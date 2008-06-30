
#include "display_x11_mod.h"

static void freepen(TMOD_X11 *mod, VISUAL *v, struct X11Pen *pen);
static int x11_seteventmask(TMOD_X11 *mod, VISUAL *v, TUINT eventmask);

#define DEF_WINWIDTH 600
#define DEF_WINHEIGHT 400

/*****************************************************************************/

LOCAL void
x11_openvisual(TMOD_X11 *mod, struct TVRequest *req)
{
	TTAGITEM *tags = req->tvr_Op.OpenVisual.Tags;
	TAPTR exec = TGetExecBase(mod);
	VISUAL *v;

	TDBPRINTF(TDB_TRACE,("openvisual\n"));

	v = TExecAlloc0(exec, mod->x11_MemMgr, sizeof(VISUAL));
	req->tvr_Op.OpenVisual.Instance = v;
	if (v == TNULL) return;

	for (;;)
	{
		XSetWindowAttributes swa;
		TUINT swa_mask;
		XGCValues gcv;
		TUINT gcv_mask;
		struct FontNode *fn;
		XWindowAttributes rootwa;

		/* gain access to root window properties: */
		XGetWindowAttributes(mod->x11_Display,
			DefaultRootWindow(mod->x11_Display), &rootwa);

		swa_mask = CWColormap | CWEventMask;

		fn = mod->x11_fm.deffont;
		v->curfont = fn;
		mod->x11_fm.defref++;

		TInitList(&v->penlist);

		TInitList(&v->imsgqueue);
		v->imsgport = req->tvr_Op.OpenVisual.IMsgPort;

		v->sizehints = XAllocSizeHints();
		if (v->sizehints == TNULL)
			break;
		v->sizehints->flags = 0;

		v->title = (TSTRPTR)
			TGetTag(tags, TVisual_Title, (TTAG) "TEKlib visual");

		/* size/position calculation: */

		v->winwidth = (TINT) TGetTag(tags,
			TVisual_Width,
			(TTAG) TMIN(WidthOfScreen(rootwa.screen), DEF_WINWIDTH));
		v->winheight = (TINT) TGetTag(tags,
			TVisual_Height,
			(TTAG) TMIN(HeightOfScreen(rootwa.screen), DEF_WINHEIGHT));

		if (TGetTag(tags, TVisual_Center, TFALSE))
		{
			v->winleft = (WidthOfScreen(rootwa.screen) - v->winwidth) / 2;
			v->wintop = (HeightOfScreen(rootwa.screen) - v->winheight) / 2;
		}
		else if (TGetTag(tags, TVisual_Fullscreen, TFALSE))
		{
			v->winwidth = WidthOfScreen(rootwa.screen);
			v->winheight = HeightOfScreen(rootwa.screen);
			v->winleft = 0;
			v->wintop = 0;
			swa_mask |= CWOverrideRedirect;
			swa.override_redirect = True;
		}
		else
		{
			v->winleft = (int) TGetTag(tags, TVisual_WinLeft, (TTAG) -1);
			v->wintop = (int) TGetTag(tags, TVisual_WinTop, (TTAG) -1);
		}

		if (v->winleft >= 0 || v->wintop >= 0)
			v->sizehints->flags |= USPosition | USSize;

		if (!TGetTag(tags, TVisual_Borderless, TFALSE))
		{
			v->sizehints->min_width = (TINT)
				TGetTag(tags, TVisual_MinWidth, (TTAG) -1);
			v->sizehints->min_height = (TINT)
				TGetTag(tags, TVisual_MinHeight, (TTAG) -1);
			v->sizehints->max_width = (TINT)
				TGetTag(tags, TVisual_MaxWidth, (TTAG) -1);
			v->sizehints->max_height = (TINT)
				TGetTag(tags, TVisual_MaxHeight, (TTAG) -1);

			if (v->sizehints->max_width > 0)
				v->winwidth = TMIN(v->winwidth, v->sizehints->max_width);
			if (v->sizehints->max_height > 0)
				v->winheight = TMIN(v->winheight, v->sizehints->max_height);
			if (v->sizehints->min_width > 0)
				v->winwidth = TMAX(v->winwidth, v->sizehints->min_width);
			if (v->sizehints->min_height > 0)
				v->winheight = TMAX(v->winheight, v->sizehints->min_height);

			v->sizehints->min_width =
				v->sizehints->min_width <= 0 ? 1 : v->sizehints->min_width;
			v->sizehints->min_height =
				v->sizehints->min_height <= 0 ? 1 : v->sizehints->min_height;
			v->sizehints->max_width =
				v->sizehints->max_width <= 0 ? 1000000 : v->sizehints->max_width;
			v->sizehints->max_height =
				v->sizehints->max_height <= 0 ? 1000000 : v->sizehints->max_height;

			v->sizehints->flags |= PMinSize | PMaxSize;
		}

		v->winleft = TMAX(v->winleft, 0);
		v->wintop = TMAX(v->wintop, 0);

		if (TGetTag(tags, TVisual_Borderless, TFALSE))
		{
			swa_mask |= CWOverrideRedirect;
			swa.override_redirect = True;
		}

		v->colormap = DefaultColormap(mod->x11_Display, mod->x11_Screen);
		if (v->colormap == TNULL)
			break;
		swa.colormap = v->colormap;

		v->base_mask = StructureNotifyMask | ExposureMask | FocusChangeMask;
		swa.event_mask = x11_seteventmask(mod, v,
			(TUINT) TGetTag(tags, TVisual_EventMask, TITYPE_FOCUS));

		v->window = XCreateWindow(mod->x11_Display,
			RootWindow(mod->x11_Display, mod->x11_Screen),
			v->winleft, v->wintop, v->winwidth, v->winheight,
			0, CopyFromParent, CopyFromParent, CopyFromParent,
			swa_mask, &swa);
		if (v->window == TNULL)
			break;

		if (v->sizehints->flags)
			XSetWMNormalHints(mod->x11_Display, v->window, v->sizehints);

		XStringListToTextProperty((char **) &v->title, 1, &v->title_prop);
		XSetWMProperties(mod->x11_Display, v->window, &v->title_prop,
			NULL, NULL, 0, NULL, NULL, NULL);

		v->atom_wm_delete_win = XInternAtom(mod->x11_Display,
			"WM_DELETE_WINDOW", True);
		XSetWMProtocols(mod->x11_Display, v->window,
			&v->atom_wm_delete_win, 1);

		gcv.function = GXcopy;
		gcv.fill_style = FillSolid;
		gcv.graphics_exposures = True;
		gcv_mask = GCFunction | GCFillStyle | GCGraphicsExposures;

		v->gc = XCreateGC(mod->x11_Display, v->window, gcv_mask, &gcv);
		XCopyGC(mod->x11_Display,
			XDefaultGC(mod->x11_Display, mod->x11_Screen),
			GCForeground | GCBackground, v->gc);

		XMapWindow(mod->x11_Display, v->window);

		if (mod->x11_use_xft == TTRUE)
		{
			v->draw = (*mod->x11_xftiface.XftDrawCreate)(mod->x11_Display,
				v->window, mod->x11_Visual, v->colormap);
			if (!v->draw) break;
		}

		v->bgpen = TVPEN_UNDEFINED;
		v->fgpen = TVPEN_UNDEFINED;

		TDBPRINTF(TDB_TRACE,("Created new window: %p\n", v->window));
		TAddTail(&mod->x11_vlist, &v->node);

		/* not yet mapped; register request in progress: */
		mod->x11_RequestInProgress = req;

		/* success: */
		return;
	}

	/* failure: */
	x11_closevisual(mod, req);
	req->tvr_Op.OpenVisual.Instance = TNULL;
}

LOCAL void
x11_closevisual(TMOD_X11 *mod, struct TVRequest *req)
{
	struct X11Pen *pen;
	TAPTR exec = TGetExecBase(mod);
	VISUAL *v = req->tvr_Op.OpenVisual.Instance;
	if (v == TNULL) return;

	TRemove(&v->node);

	TExecFree(TGetExecBase(mod), v->tempbuf);

	if (mod->x11_use_xft == TTRUE && v->draw)
		(*mod->x11_xftiface.XftDrawDestroy)(v->draw);

	if (v->window)
		XUnmapWindow(mod->x11_Display, v->window);
	if (v->gc)
		XFreeGC(mod->x11_Display, v->gc);
	if (v->window)
		XDestroyWindow(mod->x11_Display, v->window);

	while ((pen = (struct X11Pen *) TRemHead(&v->penlist)))
		freepen(mod, v, pen);

	if (v->colormap)
		XFreeColormap(mod->x11_Display, v->colormap);
	if (v->sizehints)
		XFree(v->sizehints);

	mod->x11_fm.defref--;

	TExecFree(exec, v);
}

static int
x11_seteventmask(TMOD_X11 *mod, VISUAL *v, TUINT eventmask)
{
	int x11_mask = v->base_mask;
	if (eventmask & TITYPE_REFRESH)
		x11_mask |= StructureNotifyMask | ExposureMask;
	if (eventmask & TITYPE_MOUSEOVER)
		x11_mask |= LeaveWindowMask | EnterWindowMask;
	if (eventmask & TITYPE_NEWSIZE)
		x11_mask |= StructureNotifyMask;
	if (eventmask & TITYPE_KEYDOWN)
		x11_mask |= KeyPressMask | KeyReleaseMask;
	if (eventmask & TITYPE_KEYUP)
		x11_mask |= KeyPressMask | KeyReleaseMask;
	if (eventmask & TITYPE_MOUSEMOVE)
		x11_mask |= PointerMotionMask | OwnerGrabButtonMask |
			ButtonMotionMask | ButtonPressMask | ButtonReleaseMask;
	if (eventmask & TITYPE_MOUSEBUTTON)
		x11_mask |= ButtonPressMask | ButtonReleaseMask | OwnerGrabButtonMask;
	v->eventmask = eventmask;
	return x11_mask;
}

LOCAL void
x11_setinput(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.SetInput.Instance;
	TUINT eventmask = req->tvr_Op.SetInput.Mask;
	XSelectInput(mod->x11_Display, v->window,
		x11_seteventmask(mod, v, eventmask));
}

LOCAL void
x11_allocpen(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.AllocPen.Instance;
	TUINT rgb = req->tvr_Op.AllocPen.RGB;
	struct X11Pen *pen = TExecAlloc(mod->x11_ExecBase, mod->x11_MemMgr,
		sizeof(struct X11Pen));
	if (pen)
	{
		pen->color.red = ((rgb >> 16) & 0xff) << 8;
		pen->color.green = ((rgb >> 8) & 0xff) << 8;
		pen->color.blue = (rgb & 0xff) << 8;
		pen->color.flags = DoRed | DoGreen | DoBlue;
		if (XAllocColor(mod->x11_Display, v->colormap, &pen->color))
		{
			TBOOL success = TTRUE;
			if (mod->x11_use_xft == TTRUE)
			{
				XRenderColor xrcolor = { red:	((rgb >> 16) & 0xff) << 8,
										green:	((rgb >> 8) & 0xff) << 8,
										blue:	 (rgb & 0xff) << 8,
										alpha:	 0xffff };

				success = (*mod->x11_xftiface.XftColorAllocValue)
					(mod->x11_Display, mod->x11_Visual, v->colormap, &xrcolor,
					&pen->xftcolor);
			}
			if (success)
			{
				TAddTail(&v->penlist, &pen->node);
				req->tvr_Op.AllocPen.Pen = (TVPEN) pen;
				return;
			}
			XFreeColors(mod->x11_Display, v->colormap, &pen->color.pixel,
				1, 0);
		}
		TExecFree(mod->x11_ExecBase, pen);
	}
	req->tvr_Op.AllocPen.Pen = TVPEN_UNDEFINED;
}

static void
freepen(TMOD_X11 *mod, VISUAL *v, struct X11Pen *pen)
{
	TRemove(&pen->node);
	XFreeColors(mod->x11_Display, v->colormap, &pen->color.pixel, 1, 0);
	if (mod->x11_use_xft == TTRUE)
		(*mod->x11_xftiface.XftColorFree)(mod->x11_Display, mod->x11_Visual,
			v->colormap, &pen->xftcolor);
	TExecFree(mod->x11_ExecBase, pen);
}

LOCAL void
x11_freepen(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.FreePen.Instance;
	struct X11Pen *pen = (struct X11Pen *) req->tvr_Op.FreePen.Pen;
	freepen(mod, v, pen);
}

/*****************************************************************************/

static void
setbgpen(TMOD_X11 *mod, VISUAL *v, TVPEN pen)
{
	if (pen != v->bgpen && pen != TVPEN_UNDEFINED)
	{
		XGCValues gcv;
		gcv.background = ((struct X11Pen *) pen)->color.pixel;
		XChangeGC(mod->x11_Display, v->gc, GCBackground, &gcv);
		v->bgpen = pen;
	}
}

static TVPEN
setfgpen(TMOD_X11 *mod, VISUAL *v, TVPEN pen)
{
	TVPEN oldpen = v->fgpen;
	if (pen != oldpen && pen != TVPEN_UNDEFINED)
	{
		XGCValues gcv;
		gcv.foreground = ((struct X11Pen *) pen)->color.pixel;
		XChangeGC(mod->x11_Display, v->gc, GCForeground, &gcv);
		v->fgpen = pen;
		if (oldpen == (TVPEN) 0xffffffff) oldpen = pen;
	}
	return oldpen;
}

/*****************************************************************************/

LOCAL void
x11_frect(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.FRect.Instance;
	TUINT x0 = req->tvr_Op.FRect.Rect[0];
	TUINT y0 = req->tvr_Op.FRect.Rect[1];
	TUINT x1 = req->tvr_Op.FRect.Rect[2];
	TUINT y1 = req->tvr_Op.FRect.Rect[3];
	setfgpen(mod, v, req->tvr_Op.FRect.Pen);
	XFillRectangle(mod->x11_Display, v->window, v->gc,
		x0, y0, x1, y1);
}

/*****************************************************************************/

LOCAL void
x11_line(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Line.Instance;
	TUINT x0 = req->tvr_Op.Line.Rect[0];
	TUINT y0 = req->tvr_Op.Line.Rect[1];
	TUINT x1 = req->tvr_Op.Line.Rect[2];
	TUINT y1 = req->tvr_Op.Line.Rect[3];
	setfgpen(mod, v, req->tvr_Op.Line.Pen);
	XDrawLine(mod->x11_Display, v->window, v->gc,
		x0, y0, x1, y1);
}

/*****************************************************************************/

LOCAL void
x11_rect(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Rect.Instance;
	TUINT x0 = req->tvr_Op.Rect.Rect[0];
	TUINT y0 = req->tvr_Op.Rect.Rect[1];
	TUINT x1 = req->tvr_Op.Rect.Rect[2];
	TUINT y1 = req->tvr_Op.Rect.Rect[3];
	setfgpen(mod, v, req->tvr_Op.Rect.Pen);
	XDrawRectangle(mod->x11_Display, v->window, v->gc,
		x0, y0, x1 - 1, y1 - 1);
}

/*****************************************************************************/

LOCAL void
x11_plot(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Plot.Instance;
	TUINT x0 = req->tvr_Op.Plot.Rect[0];
	TUINT y0 = req->tvr_Op.Plot.Rect[1];
	setfgpen(mod, v, req->tvr_Op.Plot.Pen);
	XDrawPoint(mod->x11_Display, v->window, v->gc, x0, y0);
}

/*****************************************************************************/

LOCAL void
x11_drawstrip(TMOD_X11 *mod, struct TVRequest *req)
{
	TINT i;
	XPoint tri[3];
	VISUAL *v = req->tvr_Op.Strip.Instance;
	TINT *array = req->tvr_Op.Strip.Array;
	TINT num = req->tvr_Op.Strip.Num;
	TTAGITEM *tags = req->tvr_Op.Strip.Tags;
	TVPEN pen = (TVPEN) TGetTag(tags, TVisual_Pen, TVPEN_UNDEFINED);
	TVPEN *penarray = (TVPEN *) TGetTag(tags, TVisual_PenArray, TNULL);

	if (num < 3) return;

	if (penarray)
		setfgpen(mod, v, penarray[2]);
	else
		setfgpen(mod, v, pen);

	tri[0].x = (TINT16) array[0];
	tri[0].y = (TINT16) array[1];
	tri[1].x = (TINT16) array[2];
	tri[1].y = (TINT16) array[3];
	tri[2].x = (TINT16) array[4];
	tri[2].y = (TINT16) array[5];

	XFillPolygon(mod->x11_Display, v->window, v->gc, tri, 3,
		Nonconvex, CoordModeOrigin);

	for (i = 3; i < num; i++)
	{
		tri[0].x = tri[1].x;
		tri[0].y = tri[1].y;
		tri[1].x = tri[2].x;
		tri[1].y = tri[2].y;
		tri[2].x = (TINT16) array[i*2];
		tri[2].y = (TINT16) array[i*2+1];

		if (penarray)
			setfgpen(mod, v, penarray[i]);

		XFillPolygon(mod->x11_Display, v->window, v->gc, tri, 3,
			Nonconvex, CoordModeOrigin);
	}
}

/*****************************************************************************/

LOCAL void
x11_drawfan(TMOD_X11 *mod, struct TVRequest *req)
{
	TINT i;
	XPoint tri[3];
	VISUAL *v = req->tvr_Op.Fan.Instance;
	TINT *array = req->tvr_Op.Fan.Array;
	TINT num = req->tvr_Op.Fan.Num;
	TTAGITEM *tags = req->tvr_Op.Fan.Tags;
	TVPEN pen = (TVPEN) TGetTag(tags, TVisual_Pen, TVPEN_UNDEFINED);
	TVPEN *penarray = (TVPEN *) TGetTag(tags, TVisual_PenArray, TNULL);

	if (num < 3) return;

	if (penarray)
		setfgpen(mod, v, penarray[2]);
	else
		setfgpen(mod, v, pen);

	tri[0].x = (TINT16) array[0];
	tri[0].y = (TINT16) array[1];
	tri[1].x = (TINT16) array[2];
	tri[1].y = (TINT16) array[3];
	tri[2].x = (TINT16) array[4];
	tri[2].y = (TINT16) array[5];

	XFillPolygon(mod->x11_Display, v->window, v->gc, tri, 3,
		Nonconvex, CoordModeOrigin);

	for (i = 3; i < num; i++)
	{
		tri[1].x = tri[2].x;
		tri[1].y = tri[2].y;
		tri[2].x = (TINT16) array[i*2];
		tri[2].y = (TINT16) array[i*2+1];

		if (penarray)
			setfgpen(mod, v, penarray[i]);

		XFillPolygon(mod->x11_Display, v->window, v->gc, tri, 3,
			Nonconvex, CoordModeOrigin);
	}
}

/*****************************************************************************/

LOCAL void
x11_drawarc(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Arc.Instance;
	TINT x = req->tvr_Op.Arc.Rect[0];
	TINT y = req->tvr_Op.Arc.Rect[1];
	TINT w = req->tvr_Op.Arc.Rect[2];
	TINT h = req->tvr_Op.Arc.Rect[3];
	TINT a1 = req->tvr_Op.Arc.Angle1*64;
	TINT a2 = req->tvr_Op.Arc.Angle2*64;

	setfgpen(mod, v, req->tvr_Op.Arc.Pen);
	XDrawArc(mod->x11_Display, v->window, v->gc, x, y, w, h, a1, a2);
}

/*****************************************************************************/

LOCAL void
x11_drawfarc(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Arc.Instance;
	TINT x = req->tvr_Op.Arc.Rect[0];
	TINT y = req->tvr_Op.Arc.Rect[1];
	TINT w = req->tvr_Op.Arc.Rect[2];
	TINT h = req->tvr_Op.Arc.Rect[3];
	TINT a1 = req->tvr_Op.Arc.Angle1*64;
	TINT a2 = req->tvr_Op.Arc.Angle2*64;

	setfgpen(mod, v, req->tvr_Op.Arc.Pen);
	XFillArc(mod->x11_Display, v->window, v->gc, x, y, w, h, a1, a2);
}

/*****************************************************************************/

LOCAL void
x11_copyarea(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.CopyArea.Instance;
	TINT x = req->tvr_Op.CopyArea.Rect[0];
	TINT y = req->tvr_Op.CopyArea.Rect[1];
	TINT w = req->tvr_Op.CopyArea.Rect[2];
	TINT h = req->tvr_Op.CopyArea.Rect[3];
	TINT dx = req->tvr_Op.CopyArea.DestX;
	TINT dy = req->tvr_Op.CopyArea.DestY;

	XCopyArea(mod->x11_Display, v->window, v->window, v->gc,
		x, y, w, h, dx, dy);

	mod->x11_CopyExposeHook = (struct THook *)
		TGetTag(req->tvr_Op.CopyArea.Tags, TVisual_ExposeHook, TNULL);
	if (mod->x11_CopyExposeHook)
	{
		/* register request in progress: */
		mod->x11_RequestInProgress = req;
	}
}

/*****************************************************************************/

LOCAL void
x11_setcliprect(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.ClipRect.Instance;
	TINT x = req->tvr_Op.ClipRect.Rect[0];
	TINT y = req->tvr_Op.ClipRect.Rect[1];
	TINT w = req->tvr_Op.ClipRect.Rect[2];
	TINT h = req->tvr_Op.ClipRect.Rect[3];
	Region region;
	XRectangle rectangle;

	region = XCreateRegion();

	rectangle.x = (short) x;
	rectangle.y = (short) y;
	rectangle.width = (unsigned short) w;
	rectangle.height = (unsigned short) h;

	/* union rect into region */
	XUnionRectWithRegion(&rectangle, region, region);
	/* set clip region */
	XSetRegion(mod->x11_Display, v->gc, region);

	if (mod->x11_use_xft == TTRUE)
		(*mod->x11_xftiface.XftDrawSetClip)(v->draw, region);

	XDestroyRegion(region);
}

/*****************************************************************************/

LOCAL void
x11_unsetcliprect(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.ClipRect.Instance;
	/*XSetClipMask(mod->x11_Display, v->gc, None);*/
	XSetRegion(mod->x11_Display, v->gc, mod->x11_HugeRegion);
	if (mod->x11_use_xft)
		(*mod->x11_xftiface.XftDrawSetClip)(v->draw, mod->x11_HugeRegion);
}

/*****************************************************************************/

LOCAL void
x11_clear(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Clear.Instance;
	setfgpen(mod, v, req->tvr_Op.Clear.Pen);
	XFillRectangle(mod->x11_Display, v->window, v->gc,
		0, 0, v->winwidth, v->winheight);
}

/*****************************************************************************/

static THOOKENTRY TTAG
getattrfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	struct attrdata *data = hook->thk_Data;
	TTAGITEM *item = obj;
	VISUAL *v = data->v;

	switch (item->tti_Tag)
	{
		default:
			return TTRUE;
		case TVisual_Width:
			*((TINT *) item->tti_Value) = v->winwidth;
			break;
		case TVisual_Height:
			*((TINT *) item->tti_Value) = v->winheight;
			break;
		case TVisual_WinLeft:
			*((TINT *) item->tti_Value) = v->winleft;
			break;
		case TVisual_WinTop:
			*((TINT *) item->tti_Value) = v->wintop;
			break;
		case TVisual_MinWidth:
			*((TINT *) item->tti_Value) = v->sizehints->min_width;
			break;
		case TVisual_MinHeight:
			*((TINT *) item->tti_Value) = v->sizehints->min_height;
			break;
		case TVisual_MaxWidth:
			*((TINT *) item->tti_Value) = v->sizehints->max_width;
			break;
		case TVisual_MaxHeight:
			*((TINT *) item->tti_Value) = v->sizehints->max_height;
			break;
	}
	data->num++;
	return TTRUE;
}

static THOOKENTRY TTAG
setattrfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	struct attrdata *data = hook->thk_Data;
	TTAGITEM *item = obj;
	VISUAL *v = data->v;

	switch (item->tti_Tag)
	{
		default:
			return TTRUE;
		case TVisual_Width:
			data->neww = (TINT) item->tti_Value;
			break;
		case TVisual_Height:
			data->newh = (TINT) item->tti_Value;
			break;
		case TVisual_MinWidth:
			v->sizehints->min_width = (TINT) item->tti_Value;
			break;
		case TVisual_MinHeight:
			v->sizehints->min_height = (TINT) item->tti_Value;
			break;
		case TVisual_MaxWidth:
			v->sizehints->max_width = (TINT) item->tti_Value;
			break;
		case TVisual_MaxHeight:
			v->sizehints->max_height = (TINT) item->tti_Value;
			break;
		#if 0
		case TVisualHost_GrabButton:
			switch (item->tti_Value)
			{
				case 0:
				{
					XUngrabButton(mod->x11_Display,
						AnyButton, AnyModifier, v->window);
					break;
				}
				case 1:
				{
					TUINT mask = ButtonPressMask | Button1MotionMask |
						ButtonReleaseMask;
					res = XGrabButton(mod->x11_Display, Button1, AnyModifier,
						v->window, True, mask, GrabModeAsync, GrabModeSync,
						None, None);
					break;
				}
			}
			TDBPRINTF(TDB_WARN,("GrabButton(%d): %d\n", item->tti_Value, res));
			break;
		case TVisualHost_GrabPointer:
			switch (item->tti_Value)
			{
				case 0:
				{
					XUngrabPointer(mod->x11_Display, None);
					break;
				}
				case 1:
				{
					TUINT mask = PointerMotionMask;
					res = XGrabPointer(mod->x11_Display, v->window, True,
						mask, GrabModeAsync, GrabModeSync, None, None, None);
					break;
				}
			}
			TDBPRINTF(TDB_WARN,("GrabPointer(%d): %d\n",
				item->tti_Value, res));
			break;
		#endif
	}
	data->num++;
	return TTRUE;
}

/*****************************************************************************/

LOCAL void
x11_getattrs(TMOD_X11 *mod, struct TVRequest *req)
{
	struct attrdata data;
	struct THook hook;

	data.v = req->tvr_Op.GetAttrs.Instance;
	data.num = 0;
	data.mod = mod;
	TInitHook(&hook, getattrfunc, &data);

	TForEachTag(req->tvr_Op.GetAttrs.Tags, &hook);
	req->tvr_Op.GetAttrs.Num = data.num;
}

/*****************************************************************************/

LOCAL void
x11_setattrs(TMOD_X11 *mod, struct TVRequest *req)
{
	struct attrdata data;
	struct THook hook;
	VISUAL *v = req->tvr_Op.SetAttrs.Instance;
	TINT neww, newh;

	data.v = v;
	data.num = 0;
	data.mod = mod;
	data.neww = -1;
	data.newh = -1;
	TInitHook(&hook, setattrfunc, &data);

	TForEachTag(req->tvr_Op.SetAttrs.Tags, &hook);
	req->tvr_Op.SetAttrs.Num = data.num;

	if (v->sizehints->max_width < 0) v->sizehints->max_width = 1000000;
	if (v->sizehints->max_height < 0) v->sizehints->max_height = 1000000;

	v->sizehints->min_width = TMAX(v->sizehints->min_width, 0);
	v->sizehints->max_width = TMAX(v->sizehints->max_width,
		v->sizehints->min_width);
	v->sizehints->min_height = TMAX(v->sizehints->min_height, 0);
	v->sizehints->max_height = TMAX(v->sizehints->max_height,
		v->sizehints->min_height);

	neww = data.neww < 0 ? v->winwidth : data.neww;
	newh = data.newh < 0 ? v->winheight : data.newh;
	if (neww < v->sizehints->min_width || newh < v->sizehints->min_height)
	{
		neww = TMAX(neww, v->sizehints->min_width);
		newh = TMAX(newh, v->sizehints->min_height);
		XResizeWindow(mod->x11_Display, v->window, neww, newh);
	}

	XSetWMNormalHints(mod->x11_Display, v->window, v->sizehints);
}

/*****************************************************************************/

LOCAL void
x11_drawtext(TMOD_X11 *mod, struct TVRequest *req)
{
	VISUAL *v = req->tvr_Op.Text.Instance;
	TSTRPTR text = req->tvr_Op.Text.Text;
	TINT len = req->tvr_Op.Text.Length;
	TUINT x = req->tvr_Op.Text.X;
	TUINT y = req->tvr_Op.Text.Y;
	TAPTR exec = TGetExecBase(mod);

	struct X11Pen *fgpen = (struct X11Pen *) req->tvr_Op.Text.FgPen;
	struct X11Pen *bgpen = (struct X11Pen *) req->tvr_Op.Text.BgPen;

	setfgpen(mod, v, (TVPEN) fgpen);
	setbgpen(mod, v, (TVPEN) bgpen);

	if ((TVPEN) bgpen == TVPEN_UNDEFINED)
	{
		if (mod->x11_use_xft == TTRUE)
		{
			XftFont *f = ((struct FontNode *) v->curfont)->xftfont;
			(*mod->x11_xftiface.XftDrawStringUtf8)(v->draw, &fgpen->xftcolor,
				f, x, y + f->ascent, (FcChar8 *)text, len);
		}
		else
		{
			TSTRPTR latin = utf8tolatin(mod, text, len);
			if (latin)
			{
				XFontStruct *f = ((struct FontNode *) v->curfont)->font;
				XDrawString(mod->x11_Display, v->window, v->gc,
					x, y + f->ascent, (char *) latin, strlen(latin));
				TExecFree(exec, latin);
			}
		}
	}
	else
	{
		if (mod->x11_use_xft == TTRUE)
		{
			XftFont *f = ((struct FontNode *) v->curfont)->xftfont;
			TINT w = x11_hosttextsize(mod, v->curfont, text);

			(*mod->x11_xftiface.XftDrawRect)(v->draw, &bgpen->xftcolor,
							x, y, w, f->height);

			(*mod->x11_xftiface.XftDrawStringUtf8)(v->draw, &fgpen->xftcolor,
				f, x, y + f->ascent, (FcChar8 *)text, len);
		}
		else
		{
			TSTRPTR latin = utf8tolatin(mod, text, len);
			if (latin)
			{
				XFontStruct *f = ((struct FontNode *) v->curfont)->font;
				XDrawImageString(mod->x11_Display, v->window, v->gc,
					x, y + f->ascent, (char *) latin, strlen(latin));
				TExecFree(exec, latin);
			}
		}
	}
}

/*****************************************************************************/

LOCAL void
x11_openfont(TMOD_X11 *mod, struct TVRequest *req)
{
	req->tvr_Op.OpenFont.Font =
		x11_hostopenfont(mod, req->tvr_Op.OpenFont.Tags);
}

/*****************************************************************************/

LOCAL void
x11_textsize(TMOD_X11 *mod, struct TVRequest *req)
{
	req->tvr_Op.TextSize.Width =
		x11_hosttextsize(mod, req->tvr_Op.TextSize.Font,
			req->tvr_Op.TextSize.Text);
}

/*****************************************************************************/

LOCAL void
x11_getfontattrs(TMOD_X11 *mod, struct TVRequest *req)
{
	struct attrdata data;
	struct THook hook;

	data.mod = mod;
	data.font = req->tvr_Op.GetFontAttrs.Font;
	data.num = 0;
	TInitHook(&hook, x11_hostgetfattrfunc, &data);

	TForEachTag(req->tvr_Op.GetFontAttrs.Tags, &hook);
	req->tvr_Op.GetFontAttrs.Num = data.num;
}

/*****************************************************************************/

LOCAL void
x11_setfont(TMOD_X11 *mod, struct TVRequest *req)
{
	x11_hostsetfont(mod, req->tvr_Op.SetFont.Instance,
		req->tvr_Op.SetFont.Font);
}

/*****************************************************************************/

LOCAL void
x11_closefont(TMOD_X11 *mod, struct TVRequest *req)
{
	x11_hostclosefont(mod, req->tvr_Op.CloseFont.Font);
}

/*****************************************************************************/

LOCAL void
x11_queryfonts(TMOD_X11 *mod, struct TVRequest *req)
{
	req->tvr_Op.QueryFonts.Handle =
		x11_hostqueryfonts(mod, req->tvr_Op.QueryFonts.Tags);
}

/*****************************************************************************/

LOCAL void
x11_getnextfont(TMOD_X11 *mod, struct TVRequest *req)
{
	req->tvr_Op.GetNextFont.Attrs =
		x11_hostgetnextfont(mod, req->tvr_Op.GetNextFont.Handle);
}

/*****************************************************************************/

struct drawdata
{
	VISUAL *v;
	TMOD_X11 *mod;
	Display *display;
	Window window;
	GC gc;
	TINT x0, x1, y0, y1;
};

static THOOKENTRY TTAG
drawtagfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	struct drawdata *data = hook->thk_Data;
	TTAGITEM *item = obj;

	switch (item->tti_Tag)
	{
		case TVisualDraw_X0:
			data->x0 = item->tti_Value;
			break;
		case TVisualDraw_Y0:
			data->y0 = item->tti_Value;
			break;
		case TVisualDraw_X1:
			data->x1 = item->tti_Value;
			break;
		case TVisualDraw_Y1:
			data->y1 = item->tti_Value;
			break;
		case TVisualDraw_NewX:
			data->x0 = data->x1;
			data->x1 = item->tti_Value;
			break;
		case TVisualDraw_NewY:
			data->y0 = data->y1;
			data->y1 = item->tti_Value;
			break;
		case TVisualDraw_FgPen:
			setfgpen(data->mod, data->v, item->tti_Value);
			break;
		case TVisualDraw_BgPen:
			setbgpen(data->mod, data->v, item->tti_Value);
			break;
		case TVisualDraw_Command:
			switch (item->tti_Value)
			{
				case TVCMD_FRECT:
					XFillRectangle(data->display, data->window, data->gc,
						data->x0, data->y0, data->x1, data->y1);
					break;
				case TVCMD_RECT:
					XDrawRectangle(data->display, data->window, data->gc,
						data->x0, data->y0, data->x1 - 1, data->y1 - 1);
					break;
				case TVCMD_LINE:
					XDrawLine(data->display, data->window, data->gc,
						data->x0, data->y0, data->x1, data->y1);
					break;
			}
			break;
	}
	return TTRUE;
}

LOCAL void
x11_drawtags(TMOD_X11 *mod, struct TVRequest *req)
{
	struct THook hook;
	struct drawdata data;
	data.v = req->tvr_Op.DrawTags.Instance;
	data.mod = mod;
	data.display = mod->x11_Display;
	data.window = data.v->window;
	data.gc = data.v->gc;

	TInitHook(&hook, drawtagfunc, &data);
	TForEachTag(req->tvr_Op.DrawTags.Tags, &hook);
}

/*****************************************************************************/

/* HACK: */
static TBOOL shm_available = TTRUE;

static int
shm_errhandler(Display *d, XErrorEvent *evt)
{
	TDBPRINTF(TDB_ERROR,("Remote display - fallback to normal XPutImage\n"));
	shm_available = TFALSE;
	return 0;
}

LOCAL void
x11_drawbuffer(TMOD_X11 *mod, struct TVRequest *req)
{
	TAPTR exec = TGetExecBase(mod);
	VISUAL *v = req->tvr_Op.DrawBuffer.Instance;
	TINT x0 = req->tvr_Op.DrawBuffer.RRect[0];
	TINT y0 = req->tvr_Op.DrawBuffer.RRect[1];
	TINT w = req->tvr_Op.DrawBuffer.RRect[2];
	TINT h = req->tvr_Op.DrawBuffer.RRect[3];
	TINT totw = req->tvr_Op.DrawBuffer.TotWidth;
	TAPTR buf = req->tvr_Op.DrawBuffer.Buf;

	if (mod->x11_Depth <= 0)
		return; /* unsupported */

	if (w != v->imw || h != v->imh)
	{
		if (shm_available && mod->x11_Shm)
		{
			if (v->image)
			{
				XShmDetach(mod->x11_Display, &v->shminfo);
				XDestroyImage(v->image);
				shmdt(v->shminfo.shmaddr);
				shmctl(v->shminfo.shmid, IPC_RMID, 0);
				v->image = TNULL;
			}

			v->image = XShmCreateImage(mod->x11_Display, mod->x11_Visual,
				mod->x11_Depth, ZPixmap, TNULL, &v->shminfo, w, h);
			if (v->image)
			{
				v->shminfo.shmid = shmget(IPC_PRIVATE,
					v->image->bytes_per_line * v->image->height,
						IPC_CREAT|0777);
				if (v->shminfo.shmid != -1)
				{
					v->shminfo.shmaddr = v->image->data =
						shmat(v->shminfo.shmid, 0, 0);
					if (v->shminfo.shmaddr)
					{
						XErrorHandler oldhnd;
						v->shminfo.readOnly = False;
						XSync(mod->x11_Display, 0);
						oldhnd = XSetErrorHandler(shm_errhandler);
						XShmAttach(mod->x11_Display, &v->shminfo);
						XSync(mod->x11_Display, 0);
						XSetErrorHandler(oldhnd);
						if (shm_available)
						{
							v->imw = w;
							v->imh = h;
						}
					}
				}
			}
		}
		else
		{
			TBOOL okay = TTRUE;

			if (v->image)
			{
				v->image->data = NULL;
				XDestroyImage(v->image);
				v->image = NULL;
			}

			if (v->tempbuf)
			{
				TExecFree(exec, v->tempbuf);
				v->tempbuf = NULL;
			}

			if (!(((mod->x11_Depth << 9) + (mod->x11_PixFmt << 1) +
				(mod->x11_Flags & TVISF_SWAPBYTEORDER)) == (24 << 9) +
					(PIXFMT_RGB << 1) + 0))
			{
				/* formats differ, need tempbuf */
				TINT sz = w * h * mod->x11_BPP * 8;
				v->tempbuf = TExecAlloc(exec, TNULL, sz);
				if (v->tempbuf == TNULL) okay = TFALSE;
			}

			if (okay)
			{
				v->image = XCreateImage(mod->x11_Display, mod->x11_Visual,
					mod->x11_Depth, ZPixmap, 0, v->tempbuf ? v->tempbuf : (char *) buf,
					w, h, mod->x11_BPP * 8, 0);
				if (v->image)
				{
					v->imw = w;
					v->imh = h;
				}
			}
		}
	}


	if (v->image)
	{
		int xx, yy;
		TUINT p;
		TUINT *sp = buf;

		switch ((mod->x11_Depth << 9) + (mod->x11_PixFmt << 1) +
			(mod->x11_Flags & TVISF_SWAPBYTEORDER))
		{
			default:
				TDBPRINTF(TDB_ERROR,("Cannot convert rgb to screen mode\n"));
				break;

			case (32 << 9) + (PIXFMT_RGB << 1) + 0:
			case (24 << 9) + (PIXFMT_RGB << 1) + 0:
				if (mod->x11_Shm)
					TExecCopyMem(exec, buf, v->image->data, w*h*4);
				else
					v->image->data = (char *) buf;
				break;


			case (15 << 9) + (PIXFMT_RGB << 1) + 0:
			{
				TUINT16 *dp = (TUINT16 *)
					(v->tempbuf ? v->tempbuf : v->image->data);

				for (yy = 0; yy < h; ++yy)
				{
					for (xx = 0; xx < w; xx++)
					{
						p = sp[xx];
						dp[xx] = ((p & 0xf80000) >> 9) |
							((p & 0x00f800) >> 6) |
							((p & 0x0000f8) >> 3);

					}
					sp += totw;
					dp += totw;
				}
				break;
			}

			case (15 << 9) + (PIXFMT_RGB << 1) + 1:
			{
				TUINT16 *dp = (TUINT16 *)
					(v->tempbuf ? v->tempbuf : v->image->data);

				for (yy = 0; yy < h; ++yy)
				{
					for (xx = 0; xx < w; xx++)
					{
						p = sp[xx];

						/*		24->15 bit, host-swapped
						**		........rrrrrrrrGGggggggbbbbbbbb
						** ->	................gggbbbbb0rrrrrGG */

						dp[xx] = ((p & 0xf80000) >> 17) |
							((p & 0x00c000) >> 14) |
							((p & 0x003800) << 2) |
							((p & 0x0000f8) << 5);
					}
					sp += totw;
					dp += totw;
				}
				break;
			}

			case (16 << 9) + (PIXFMT_RGB << 1) + 0:
			{
				TUINT16 *dp = (TUINT16 *)
					(v->tempbuf ? v->tempbuf : v->image->data);
				for (yy = 0; yy < h; ++yy)
				{
					for (xx = 0; xx < w; xx++)
					{
						p = sp[xx];
						dp[xx] = ((p & 0xf80000) >> 8) |
							((p & 0x00fc00) >> 5) |
							((p & 0x0000f8) >> 3);

					}
					sp += totw;
					dp = (TUINT16 *)
						(((char*) dp) + v->image->bytes_per_line);
				}
				break;
			}

			case (16 << 9) + (PIXFMT_RGB << 1) + 1:
			{
				TUINT16 *dp = (TUINT16 *)
					(v->tempbuf ? v->tempbuf : v->image->data);
				for (yy = 0; yy < h; ++yy)
				{
					for (xx = 0; xx < w; xx++)
					{
						p = sp[xx];

						/*		24->16 bit, host-swapped
						**		........rrrrrrrrGGGgggggbbbbbbbb
						** ->	................gggbbbbbrrrrrGGG */

						dp[xx] = ((p & 0xf80000) >> 16) |
							((p & 0x0000f8) << 5) |
							((p & 0x00e000) >> 13) |
							((p & 0x001c00) << 3);
					}
					sp += totw;
					dp = (TUINT16 *)
						(((char*) dp) + v->image->bytes_per_line);
				}
				break;
			}

			case (24 << 9) + (PIXFMT_RGB << 1) + 1:
			{
				TUINT *dp = (TUINT *)
					(v->tempbuf ? v->tempbuf : v->image->data);

				for (yy = 0; yy < h; ++yy)
				{
					for (xx = 0; xx < w; xx++)
					{
						p = sp[xx];

						/*	24->24 bit, host-swapped */
						dp[xx] = ((p & 0x00ff0000) >> 8) |
							((p & 0x0000ff00) << 8) |
							((p & 0x000000ff) << 24);
					}
					sp += totw;
					dp += totw;
				}
				break;
			}
		}

		if (shm_available && mod->x11_Shm)
		{
			XShmPutImage(mod->x11_Display, v->window, v->gc, v->image, 0, 0,
				x0, y0, w, h, 1);
			mod->x11_RequestInProgress = req;
		}
		else
		{
			XPutImage(mod->x11_Display, v->window, v->gc, v->image, 0, 0,
				x0, y0, w, h);
		}
	}
}
