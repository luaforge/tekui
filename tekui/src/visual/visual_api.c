
#include "visual_mod.h"

/* TODO */
#define DEF_DISPLAYNAME	"display_x11"

/*****************************************************************************/

static struct TVRequest *
visi_getreq(TMOD_VIS *inst, TUINT cmd, TAPTR display, TTAGITEM *tags)
{
	TMOD_VIS *mod = (TMOD_VIS *) inst->vis_Module.tmd_ModSuper;
	struct TVRequest *req = TNULL;

	if (display == TNULL)
		display = vis_opendisplay(mod, tags);

	if (display)
	{
		if (mod == inst)
		{
			TExecLock(inst->vis_ExecBase, inst->vis_Lock);
			if (inst->vis_InitRequest)
			{
				req = inst->vis_InitRequest;
				inst->vis_InitRequest = TNULL;
				/* when using initreq, base is in LOCKED state */
			}
			else
			{
				TExecUnlock(inst->vis_ExecBase, inst->vis_Lock);
				req = TDisplayAllocReq(display);
			}

			if (req)
			{
				req->tvr_Req.io_Command = cmd;
				req->tvr_Req.io_ReplyPort = TNULL; /* syncport */
			}
		}
		else
		{
			/* try to unlink from free requests pool: */
			req = (struct TVRequest *) TRemHead(&inst->vis_ReqPool);

			if (req == TNULL)
			{
				/* peek into list of waiting requests: */
				req = (struct TVRequest *) TFIRSTNODE(&inst->vis_WaitList);
				if (req)
				{
					if (inst->vis_NumRequests >= VISUAL_MAXREQPERINSTANCE)
					{
						/* wait and unlink from waitlist: */
						TExecWaitIO(inst->vis_ExecBase, &req->tvr_Req);
						TRemove(&req->tvr_Req.io_Node);
					}
					else
						/* allocate new one: */
						req = TNULL;
				}
			}

			if (req == TNULL)
			{
				req = TDisplayAllocReq(display);
				if (req)
					inst->vis_NumRequests++;
			}

			if (req)
			{
				req->tvr_Req.io_Command = cmd;
				req->tvr_Req.io_ReplyPort = inst->vis_CmdRPort;
			}
		}
	}

	return req;
}

static void
visi_ungetreq(TMOD_VIS *inst, struct TVRequest *req)
{
	if (inst->vis_Module.tmd_ModSuper == (struct TModule *) inst)
	{
		TExecLock(inst->vis_ExecBase, inst->vis_Lock);
		if (inst->vis_InitRequest == TNULL)
		{
			inst->vis_InitRequest = req;
			/* after returning initreq, base gets UNLOCKED: */
			TExecUnlock(inst->vis_ExecBase, inst->vis_Lock);
		}
		else
			TDisplayFreeReq(req->tvr_Req.io_Device, req);
		TExecUnlock(inst->vis_ExecBase, inst->vis_Lock);
	}
	else
		TAddTail(&inst->vis_ReqPool, &req->tvr_Req.io_Node);
}

static void
visi_dosync(TMOD_VIS *inst, struct TVRequest *req)
{
	TExecDoIO(inst->vis_ExecBase, &req->tvr_Req);
	visi_ungetreq(inst, req);
}

static void
visi_doasync(TMOD_VIS *inst, struct TVRequest *req)
{
	TExecPutIO(inst->vis_ExecBase, &req->tvr_Req);
	TAddTail(&inst->vis_WaitList, &req->tvr_Req.io_Node);
}

/*****************************************************************************/

EXPORT TAPTR vis_openvisual(TMOD_VIS *mod, TTAGITEM *tags)
{
	TMOD_VIS *inst;
	TTAGITEM otags[2];

	otags[0].tti_Tag = TVisual_NewInstance;
	otags[0].tti_Value = TTRUE;
	otags[1].tti_Tag = TTAG_MORE;
	otags[1].tti_Value = (TTAG) tags;

	inst = TExecOpenModule(mod->vis_ExecBase, "visual", 0, otags);
	if (inst)
	{
		struct TVRequest *req =
			visi_getreq(inst, TVCMD_OPENVISUAL, TNULL, tags);
		if (req)
		{
			req->tvr_Op.OpenVisual.Instance = TNULL;
			req->tvr_Op.OpenVisual.IMsgPort = inst->vis_IMsgPort;
			req->tvr_Op.OpenVisual.Tags = tags;
			TExecDoIO(inst->vis_ExecBase, &req->tvr_Req);

			inst->vis_Visual = req->tvr_Op.OpenVisual.Instance;
			if (inst->vis_Visual)
			{
				inst->vis_InitRequest = req;
				inst->vis_Display = req->tvr_Req.io_Device;

				vis_setinput(inst, TITYPE_NONE, TITYPE_CLOSE);
				return inst;
			}
		}
		TExecCloseModule(mod->vis_ExecBase, inst);
	}
	TDBPRINTF(TDB_ERROR,("open failed\n"));
	return TNULL;
}

/*****************************************************************************/

EXPORT void vis_closevisual(TMOD_VIS *mod, TMOD_VIS *inst)
{
	struct TVRequest *req = visi_getreq(mod, TVCMD_CLOSEVISUAL,
		inst->vis_Display, TNULL);
	req->tvr_Req.io_Command = TVCMD_CLOSEVISUAL;
	req->tvr_Op.CloseVisual.Instance = inst->vis_Visual;
	visi_dosync(inst, req);
	TDisplayFreeReq(inst->vis_Display, inst->vis_InitRequest);
	TExecCloseModule(mod->vis_ExecBase, inst);
}

/*****************************************************************************/

EXPORT TAPTR vis_attach(TMOD_VIS *mod, TTAGITEM *tags)
{
	TTAGITEM otags[2];
	otags[0].tti_Tag = TVisual_Attach;
	otags[0].tti_Value = (TTAG) mod;
	otags[1].tti_Tag = TTAG_MORE;
	otags[1].tti_Value = (TTAG) tags;
	return TExecOpenModule(mod->vis_ExecBase, "visual", 0, otags);
}

/*****************************************************************************/

EXPORT TAPTR vis_openfont(TMOD_VIS *mod, TTAGITEM *tags)
{
	TAPTR font = TNULL;
	struct TVRequest *req = visi_getreq(mod, TVCMD_OPENFONT, TNULL, tags);
	if (req)
	{
		req->tvr_Op.OpenFont.Tags = tags;
		TExecDoIO(mod->vis_ExecBase, &req->tvr_Req);
		font = req->tvr_Op.OpenFont.Font;
		visi_ungetreq(mod, req);
	}
	return font;
}

/*****************************************************************************/

EXPORT void vis_closefont(TMOD_VIS *mod, TAPTR font)
{
	if (font)
	{
		struct TVRequest *req = visi_getreq(mod, TVCMD_CLOSEFONT,
			TGetOwner(font), TNULL);
		if (req)
		{
			req->tvr_Op.CloseFont.Font = font;
			visi_dosync(mod, req);
		}
	}
}

/*****************************************************************************/

EXPORT TUINT vis_getfattrs(TMOD_VIS *mod, TAPTR font, TTAGITEM *tags)
{
	TUINT n = 0;
	struct TVRequest *req = visi_getreq(mod, TVCMD_GETFONTATTRS,
		TGetOwner(font), TNULL);
	if (req)
	{
		req->tvr_Op.GetFontAttrs.Font = font;
		req->tvr_Op.GetFontAttrs.Tags = tags;
		TExecDoIO(mod->vis_ExecBase, &req->tvr_Req);
		n = req->tvr_Op.TextSize.Width;
		visi_ungetreq(mod, req);
	}
	return n;
}

/*****************************************************************************/

EXPORT TINT vis_textsize(TMOD_VIS *mod, TAPTR font, TSTRPTR t)
{
	TINT size = -1;
	struct TVRequest *req = visi_getreq(mod, TVCMD_TEXTSIZE,
		TGetOwner(font), TNULL);
	if (req)
	{
		req->tvr_Op.TextSize.Font = font;
		req->tvr_Op.TextSize.Text = t;
		TExecDoIO(mod->vis_ExecBase, &req->tvr_Req);
		size = req->tvr_Op.TextSize.Width;
		visi_ungetreq(mod, req);
	}
	return size;
}

/*****************************************************************************/

EXPORT TAPTR vis_queryfonts(TMOD_VIS *mod, TTAGITEM *tags)
{
	TAPTR handle = TNULL;
	struct TVRequest *req = visi_getreq(mod, TVCMD_QUERYFONTS, TNULL, tags);
	if (req)
	{
		req->tvr_Op.QueryFonts.Tags = tags;
		TExecDoIO(mod->vis_ExecBase, &req->tvr_Req);
		handle = req->tvr_Op.QueryFonts.Handle;
		visi_ungetreq(mod, req);
	}
	return handle;
}

/*****************************************************************************/

EXPORT TTAGITEM *vis_getnextfont(TMOD_VIS *mod, TAPTR fqhandle)
{
	TTAGITEM *attrs;
	struct TVRequest *req = visi_getreq(mod, TVCMD_GETNEXTFONT,
		TGetOwner(fqhandle), TNULL);
	req->tvr_Op.GetNextFont.Handle = fqhandle;
	TExecDoIO(mod->vis_ExecBase, &req->tvr_Req);
	attrs = req->tvr_Op.GetNextFont.Attrs;
	visi_ungetreq(mod, req);
	return attrs;
}

/*****************************************************************************/

EXPORT TAPTR vis_getport(TMOD_VIS *inst)
{
	return inst->vis_IMsgPort;
}

/*****************************************************************************/

EXPORT TUINT vis_setinput(TMOD_VIS *inst, TUINT cmask, TUINT smask)
{
	TUINT oldmask = inst->vis_InputMask;
	TUINT newmask = (oldmask & ~cmask) | smask;
	if (newmask != oldmask)
	{
		struct TVRequest *req = visi_getreq(inst, TVCMD_SETINPUT,
			inst->vis_Display, TNULL);
		req->tvr_Op.SetInput.Instance = inst->vis_Visual;
		req->tvr_Op.SetInput.Mask = newmask;
		visi_dosync(inst, req);
		inst->vis_InputMask = newmask;
	}
	return newmask;
}

/*****************************************************************************/

EXPORT TUINT vis_getattrs(TMOD_VIS *inst, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_GETATTRS,
		inst->vis_Display, TNULL);
	req->tvr_Op.GetAttrs.Instance = inst->vis_Visual;
	req->tvr_Op.GetAttrs.Tags = tags;
	visi_dosync(inst, req);
	return req->tvr_Op.GetAttrs.Num;
}

/*****************************************************************************/

EXPORT TUINT vis_setattrs(TMOD_VIS *inst, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_SETATTRS,
		inst->vis_Display, TNULL);
	req->tvr_Op.SetAttrs.Instance = inst->vis_Visual;
	req->tvr_Op.SetAttrs.Tags = tags;
	visi_dosync(inst, req);
	return req->tvr_Op.SetAttrs.Num;
}

/*****************************************************************************/

EXPORT TVPEN vis_allocpen(TMOD_VIS *inst, TUINT rgb)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_ALLOCPEN,
		inst->vis_Display, TNULL);
	req->tvr_Op.AllocPen.Instance = inst->vis_Visual;
	req->tvr_Op.AllocPen.RGB = rgb;
	visi_dosync(inst, req);
	return req->tvr_Op.AllocPen.Pen;
}

/*****************************************************************************/

EXPORT void vis_freepen(TMOD_VIS *inst, TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_FREEPEN,
		inst->vis_Display, TNULL);
	req->tvr_Op.FreePen.Instance = inst->vis_Visual;
	req->tvr_Op.FreePen.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/
/* TODO: inst->display <-> font->display must match */

EXPORT void vis_setfont(TMOD_VIS *inst, TAPTR font)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_SETFONT,
		inst->vis_Display, TNULL);
	req->tvr_Op.SetFont.Instance = inst->vis_Visual;
	req->tvr_Op.SetFont.Font = font;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_clear(TMOD_VIS *inst, TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_CLEAR,
		inst->vis_Display, TNULL);
	req->tvr_Op.Clear.Instance = inst->vis_Visual;
	req->tvr_Op.Clear.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_rect(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_RECT,
		inst->vis_Display, TNULL);
	req->tvr_Op.Rect.Instance = inst->vis_Visual;
	req->tvr_Op.Rect.Rect[0] = x;
	req->tvr_Op.Rect.Rect[1] = y;
	req->tvr_Op.Rect.Rect[2] = w;
	req->tvr_Op.Rect.Rect[3] = h;
	req->tvr_Op.Rect.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_frect(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_FRECT,
		inst->vis_Display, TNULL);
	req->tvr_Op.FRect.Instance = inst->vis_Visual;
	req->tvr_Op.FRect.Rect[0] = x;
	req->tvr_Op.FRect.Rect[1] = y;
	req->tvr_Op.FRect.Rect[2] = w;
	req->tvr_Op.FRect.Rect[3] = h;
	req->tvr_Op.FRect.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_line(TMOD_VIS *inst, TINT x0, TINT y0, TINT x1, TINT y1,
	TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_LINE,
		inst->vis_Display, TNULL);
	req->tvr_Op.Line.Instance = inst->vis_Visual;
	req->tvr_Op.Line.Rect[0] = x0;
	req->tvr_Op.Line.Rect[1] = y0;
	req->tvr_Op.Line.Rect[2] = x1;
	req->tvr_Op.Line.Rect[3] = y1;
	req->tvr_Op.Line.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_plot(TMOD_VIS *inst, TINT x, TINT y, TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_PLOT,
		inst->vis_Display, TNULL);
	req->tvr_Op.Plot.Instance = inst->vis_Visual;
	req->tvr_Op.Plot.Rect[0] = x;
	req->tvr_Op.Plot.Rect[1] = y;
	req->tvr_Op.Plot.Pen = pen;
	visi_doasync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_drawtags(TMOD_VIS *inst, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWTAGS,
		inst->vis_Display, TNULL);
	req->tvr_Op.DrawTags.Instance = inst->vis_Visual;
	req->tvr_Op.DrawTags.Tags = tags;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_text(TMOD_VIS *inst, TINT x, TINT y, TSTRPTR t, TUINT l,
	TVPEN fg, TVPEN bg)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_TEXT,
		inst->vis_Display, TNULL);
	req->tvr_Op.Text.Instance = inst->vis_Visual;
	req->tvr_Op.Text.X = x;
	req->tvr_Op.Text.Y = y;
	req->tvr_Op.Text.FgPen = fg;
	req->tvr_Op.Text.BgPen = bg;
	req->tvr_Op.Text.Text = t;
	req->tvr_Op.Text.Length = l;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_drawstrip(TMOD_VIS *inst, TINT *array, TINT num, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWSTRIP,
		inst->vis_Display, TNULL);
	req->tvr_Op.Strip.Instance = inst->vis_Visual;
	req->tvr_Op.Strip.Array = array;
	req->tvr_Op.Strip.Num = num;
	req->tvr_Op.Strip.Tags = tags;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_drawfan(TMOD_VIS *inst, TINT *array, TINT num, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWFAN,
		inst->vis_Display, TNULL);
	req->tvr_Op.Fan.Instance = inst->vis_Visual;
	req->tvr_Op.Fan.Array = array;
	req->tvr_Op.Fan.Num = num;
	req->tvr_Op.Fan.Tags = tags;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_drawarc(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TINT angle1, TINT angle2, TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWARC,
		inst->vis_Display, TNULL);
	req->tvr_Op.Arc.Instance = inst->vis_Visual;
	req->tvr_Op.Arc.Rect[0] = x;
	req->tvr_Op.Arc.Rect[1] = y;
	req->tvr_Op.Arc.Rect[2] = w;
	req->tvr_Op.Arc.Rect[3] = h;
	req->tvr_Op.Arc.Angle1 = angle1;
	req->tvr_Op.Arc.Angle2 = angle2;
	req->tvr_Op.Arc.Pen = pen;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_drawfarc(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TINT angle1, TINT angle2, TVPEN pen)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWFARC,
		inst->vis_Display, TNULL);
	req->tvr_Op.Arc.Instance = inst->vis_Visual;
	req->tvr_Op.Arc.Rect[0] = x;
	req->tvr_Op.Arc.Rect[1] = y;
	req->tvr_Op.Arc.Rect[2] = w;
	req->tvr_Op.Arc.Rect[3] = h;
	req->tvr_Op.Arc.Angle1 = angle1;
	req->tvr_Op.Arc.Angle2 = angle2;
	req->tvr_Op.Arc.Pen = pen;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_copyarea(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TINT dx, TINT dy, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_COPYAREA,
		inst->vis_Display, TNULL);
	req->tvr_Op.CopyArea.Instance = inst->vis_Visual;
	req->tvr_Op.CopyArea.Rect[0] = x;
	req->tvr_Op.CopyArea.Rect[1] = y;
	req->tvr_Op.CopyArea.Rect[2] = w;
	req->tvr_Op.CopyArea.Rect[3] = h;
	req->tvr_Op.CopyArea.DestX = dx;
	req->tvr_Op.CopyArea.DestY = dy;
	req->tvr_Op.CopyArea.Tags = tags;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_setcliprect(TMOD_VIS *inst, TINT x, TINT y, TINT w, TINT h,
	TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_SETCLIPRECT,
		inst->vis_Display, TNULL);
	req->tvr_Op.ClipRect.Instance = inst->vis_Visual;
	req->tvr_Op.ClipRect.Rect[0] = x;
	req->tvr_Op.ClipRect.Rect[1] = y;
	req->tvr_Op.ClipRect.Rect[2] = w;
	req->tvr_Op.ClipRect.Rect[3] = h;
	req->tvr_Op.ClipRect.Tags = tags;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT void vis_unsetcliprect(TMOD_VIS *inst)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_UNSETCLIPRECT,
		inst->vis_Display, TNULL);
	req->tvr_Op.ClipRect.Instance = inst->vis_Visual;
	visi_dosync(inst, req);
}

/*****************************************************************************/

EXPORT TAPTR vis_opendisplay(TMOD_VIS *mod, TTAGITEM *tags)
{
	/* displaybase in taglist? */
	TAPTR display = (struct TVRequest *) TGetTag(tags, TVisual_Display, TNULL);
	if (display == TNULL)
	{
		TSTRPTR name;
		TBOOL success;
		TTAG hashval;

		/* displayname in taglist? */
		name = (TSTRPTR) TGetTag(tags, TVisual_DisplayName, TNULL);

		/* use default name: */
		if (name == TNULL)
			name = DEF_DISPLAYNAME;

		/* lookup display by name: */
		TExecLock(mod->vis_ExecBase, mod->vis_Lock);
		success = vis_gethash(mod, mod->vis_Displays, name, &hashval);
		TExecUnlock(mod->vis_ExecBase, mod->vis_Lock);

		if (success)
			display = (TAPTR) hashval;
		else
		{
			/* try to open named display: */
			TDBPRINTF(TDB_WARN,("loading module %s\n", name));
			display = TExecOpenModule(mod->vis_ExecBase, name, 0, TNULL);
			if (display)
			{
				/* store display in hash: */
				TExecLock(mod->vis_ExecBase, mod->vis_Lock);
				success = vis_puthash(mod, mod->vis_Displays, name, (TTAG) display);
				TExecUnlock(mod->vis_ExecBase, mod->vis_Lock);
				if (!success)
				{
					TExecCloseModule(mod->vis_ExecBase, display);
					display = TNULL;
				}
			}
		}
	}
	return display;
}

/*****************************************************************************/

EXPORT void vis_closedisplay(TMOD_VIS *mod, TAPTR display)
{
}

/*****************************************************************************/

EXPORT TAPTR vis_querydisplays(TMOD_VIS *mod, TTAGITEM *tags)
{
	return TNULL;
}

/*****************************************************************************/

EXPORT TTAGITEM *vis_getnextdisplay(TMOD_VIS *mod, TAPTR dqhandle)
{
	return TNULL;
}

/*****************************************************************************/

EXPORT void vis_drawbuffer(TMOD_VIS *inst,
	TINT x, TINT y, TAPTR buf, TINT w, TINT h, TINT totw, TTAGITEM *tags)
{
	struct TVRequest *req = visi_getreq(inst, TVCMD_DRAWBUFFER,
		inst->vis_Display, TNULL);
	req->tvr_Op.DrawBuffer.Instance = inst->vis_Visual;
	req->tvr_Op.DrawBuffer.Buf = buf;
	req->tvr_Op.DrawBuffer.RRect[0] = x;
	req->tvr_Op.DrawBuffer.RRect[1] = y;
	req->tvr_Op.DrawBuffer.RRect[2] = w;
	req->tvr_Op.DrawBuffer.RRect[3] = h;
	req->tvr_Op.DrawBuffer.TotWidth = totw;
	req->tvr_Op.DrawBuffer.Tags = tags;
	visi_dosync(inst, req);
}
