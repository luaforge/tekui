
#include <unistd.h>
#include <dlfcn.h>
#include <sys/select.h>

#include "display_x11_mod.h"

TBOOL initlibxft(TMOD_X11 *mod);

static TTASKENTRY void x11_taskfunc(TAPTR task);
static TTASKENTRY TBOOL x11_initinstance(TAPTR task);
static void x11_exitinstance(TMOD_X11 *inst);
static void x11_processevent(TMOD_X11 *mod);
static TBOOL x11_processvisualevent(TMOD_X11 *mod, VISUAL *v,
	TAPTR msgstate, XEvent *ev);
static void x11_sendimessages(TMOD_X11 *mod, TBOOL do_interval);

/*****************************************************************************/
/*
**	Module init/exit
*/

LOCAL TBOOL
x11_init(TMOD_X11 *mod, TTAGITEM *tags)
{
	for (;;)
	{
		TTAGITEM tags[2];

		mod->x11_TimeBase =
			TExecOpenModule(mod->x11_ExecBase, "time", 0, TNULL);
		if (mod->x11_TimeBase == TNULL) break;

		mod->x11_TimeReq =
			TTimeAllocTimeRequest(mod->x11_TimeBase, TNULL);

		tags[0].tti_Tag = TTask_UserData;
		tags[0].tti_Value = (TTAG) mod;
		tags[1].tti_Tag = TTAG_DONE;

		mod->x11_Task = TExecCreateTask(mod->x11_ExecBase,
			(TTASKFUNC) x11_taskfunc, (TINITFUNC) x11_initinstance, tags);
		if (mod->x11_Task == TNULL) break;

		mod->x11_CmdPort = TExecGetUserPort(mod->x11_ExecBase, mod->x11_Task);
		mod->x11_CmdPortSignal = TExecGetPortSignal(mod->x11_ExecBase,
			mod->x11_CmdPort);

		return TTRUE;
	}

	x11_exit(mod);
	return TFALSE;
}

LOCAL void
x11_exit(TMOD_X11 *mod)
{
	if (mod->x11_Task)
	{
		TExecSignal(mod->x11_ExecBase, mod->x11_Task, TTASK_SIG_ABORT);
		x11_wake(mod);
		TDestroy(mod->x11_Task);
	}

	if (mod->x11_TimeBase)
	{
		TTimeFreeTimeRequest(mod->x11_TimeBase, mod->x11_TimeReq);
		TExecCloseModule(mod->x11_ExecBase, mod->x11_TimeBase);
	}
}

/*****************************************************************************/

static const TUINT8 endiancheck[4] = { 0x11,0x22,0x33,0x44 };

static TBOOL
getprops(TMOD_X11 *inst)
{
	XVisualInfo xvi;
	int major, minor;

	inst->x11_Flags = (ImageByteOrder(inst->x11_Display) !=
		(*(TUINT *) endiancheck == 0x11223344 ? MSBFirst : LSBFirst)) ?
			TVISF_SWAPBYTEORDER : 0;

	if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 16,
		DirectColor, &xvi))
		inst->x11_Depth = 16;
	else if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 24,
		DirectColor, &xvi))
		inst->x11_Depth = 24;
	else if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 32,
		DirectColor, &xvi))
		inst->x11_Depth = 32;
	else if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 16,
		TrueColor, &xvi))
		inst->x11_Depth = 16;
	else if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 24,
		TrueColor, &xvi))
		inst->x11_Depth = 24;
	else if (XMatchVisualInfo(inst->x11_Display, inst->x11_Screen, 32,
		TrueColor, &xvi))
		inst->x11_Depth = 32;
	else
		inst->x11_Depth = 0;

	if (inst->x11_Depth > 0)
	{
		unsigned long rmsk, gmsk, bmsk;
		inst->x11_Visual = xvi.visual;

		rmsk = xvi.red_mask;
		gmsk = xvi.green_mask;
		bmsk = xvi.blue_mask;

		if ((rmsk > gmsk) && (gmsk > bmsk))
			inst->x11_PixFmt = PIXFMT_RGB;
		else if ((rmsk > bmsk) && (bmsk > gmsk))
			inst->x11_PixFmt = PIXFMT_RBG;
		else if ((bmsk > rmsk) && (rmsk > gmsk))
			inst->x11_PixFmt = PIXFMT_BRG;
		else if ((bmsk > gmsk) && (gmsk > rmsk))
			inst->x11_PixFmt = PIXFMT_BGR;
		else if ((gmsk > rmsk) && (rmsk > bmsk))
			inst->x11_PixFmt = PIXFMT_GRB;
		else if ((gmsk > bmsk) && (bmsk > rmsk))
			inst->x11_PixFmt = PIXFMT_GBR;
		else
			inst->x11_PixFmt = PIXFMT_UNDEFINED;

		if ((inst->x11_Depth == 16) && (xvi.red_mask != 0xf800))
			inst->x11_Depth = 15;
	}

	switch (inst->x11_Depth)
	{
		case 15:
		case 16:
			inst->x11_BPP = 2;
			break;
		case 24:
		case 32:
			inst->x11_BPP = 4;
			break;
	}

	XShmQueryVersion(inst->x11_Display, &major, &minor, &inst->x11_Shm);
	if (inst->x11_Shm)
		inst->x11_ShmEvent = XShmGetEventBase(inst->x11_Display) +
			ShmCompletion;

	return TTRUE;
}

/*****************************************************************************/

static TTASKENTRY TBOOL
x11_initinstance(TAPTR task)
{
	TMOD_X11 *inst = TExecGetTaskData(TGetExecBase(task), task);

	for (;;)
	{
		TTAGITEM ftags[3];
		int pipefd[2];
		XRectangle rectangle;

		/* list of free input messages: */
		TInitList(&inst->x11_imsgpool);

		/* list of all open visuals: */
		TInitList(&inst->x11_vlist);

		/* init fontmanager and default font */
		TInitList(&inst->x11_fm.openfonts);

		inst->x11_fd_sigpipe_read = -1;
		inst->x11_fd_sigpipe_write = -1;

		inst->x11_Display = XOpenDisplay(NULL);
		if (inst->x11_Display == TNULL)
			break;

		XkbSetDetectableAutoRepeat(inst->x11_Display, TTRUE, TNULL);

		inst->x11_fd_display = ConnectionNumber(inst->x11_Display);
		inst->x11_Screen = DefaultScreen(inst->x11_Display);
		inst->x11_Visual = DefaultVisual(inst->x11_Display, inst->x11_Screen);

		if (getprops(inst) == TFALSE)
			break;

		if (pipe(pipefd) != 0)
			break;
		inst->x11_fd_sigpipe_read = pipefd[0];
		inst->x11_fd_sigpipe_write = pipefd[1];
		inst->x11_fd_max =
			TMAX(inst->x11_fd_sigpipe_read, inst->x11_fd_display) + 1;

		initlibxft(inst);

		/* needed for unsetcliprect: */
		inst->x11_HugeRegion = XCreateRegion();
		rectangle.x = 0;
		rectangle.y = 0;
		rectangle.width = (unsigned short) 0xffff;
		rectangle.height = (unsigned short) 0xffff;
		XUnionRectWithRegion(&rectangle, inst->x11_HugeRegion,
			inst->x11_HugeRegion);

		ftags[0].tti_Tag = TVisual_FontName;
		ftags[0].tti_Value = (TTAG) FNT_DEFNAME;
		ftags[1].tti_Tag = TVisual_FontPxSize;
		ftags[1].tti_Value = (TTAG) FNT_DEFPXSIZE;
		ftags[2].tti_Tag = TTAG_DONE;

		inst->x11_fm.deffont = x11_hostopenfont(inst, ftags);
		if (inst->x11_fm.deffont == TNULL) break;

		TDBPRINTF(TDB_TRACE,("instance init successful\n"));
		return TTRUE;
	}

	x11_exitinstance(inst);

	return TFALSE;
}

static void
x11_exitinstance(TMOD_X11 *inst)
{
	struct TNode *imsg, *node, *next;

	/* free pooled input messages: */
	while ((imsg = TRemHead(&inst->x11_imsgpool)))
		TExecFree(inst->x11_ExecBase, imsg);

	/* free queued input messages in all open visuals: */
	node = inst->x11_vlist.tlh_Head;
	for (; (next = node->tln_Succ); node = next)
	{
		VISUAL *v = (VISUAL *) node;

		/* unset active font in all open visuals */
		v->curfont = TNULL;

		while ((imsg = TRemHead(&v->imsgqueue)))
			TExecFree(inst->x11_ExecBase, imsg);
	}

	/* force closing of default font */
	inst->x11_fm.defref = 0;

	/* close all fonts */
	node = inst->x11_fm.openfonts.tlh_Head;
	for (; (next = node->tln_Succ); node = next)
		x11_hostclosefont(inst, (TAPTR) node);

	/*XDestroyRegion(inst->x11_HugeRegion);*/

	if (inst->x11_fd_sigpipe_read != -1)
	{
		close(inst->x11_fd_sigpipe_read);
		close(inst->x11_fd_sigpipe_write);
	}

	if (inst->x11_Display)
		XCloseDisplay(inst->x11_Display);

	if (inst->x11_libfchandle)
		dlclose(inst->x11_libfchandle);

	if (inst->x11_libxfthandle)
		dlclose(inst->x11_libxfthandle);
}

/*****************************************************************************/
/*
**	Task
*/

static void
x11_docmd(TMOD_X11 *inst, struct TVRequest *req)
{
	switch (req->tvr_Req.io_Command)
	{
		case TVCMD_OPENVISUAL:
			x11_openvisual(inst, req);
			break;
		case TVCMD_CLOSEVISUAL:
			x11_closevisual(inst, req);
			break;
		case TVCMD_OPENFONT:
			x11_openfont(inst, req);
			break;
		case TVCMD_CLOSEFONT:
			x11_closefont(inst, req);
			break;
		case TVCMD_GETFONTATTRS:
			x11_getfontattrs(inst, req);
			break;
		case TVCMD_TEXTSIZE:
			x11_textsize(inst, req);
			break;
		case TVCMD_QUERYFONTS:
			x11_queryfonts(inst, req);
			break;
		case TVCMD_GETNEXTFONT:
			x11_getnextfont(inst, req);
			break;
		case TVCMD_SETINPUT:
			x11_setinput(inst, req);
			break;
		case TVCMD_GETATTRS:
			x11_getattrs(inst, req);
			break;
		case TVCMD_SETATTRS:
			x11_setattrs(inst, req);
			break;
		case TVCMD_ALLOCPEN:
			x11_allocpen(inst, req);
			break;
		case TVCMD_FREEPEN:
			x11_freepen(inst, req);
			break;
		case TVCMD_SETFONT:
			x11_setfont(inst, req);
			break;
		case TVCMD_CLEAR:
			x11_clear(inst, req);
			break;
		case TVCMD_RECT:
			x11_rect(inst, req);
			break;
		case TVCMD_FRECT:
			x11_frect(inst, req);
			break;
		case TVCMD_LINE:
			x11_line(inst, req);
			break;
		case TVCMD_PLOT:
			x11_plot(inst, req);
			break;
		case TVCMD_TEXT:
			x11_drawtext(inst, req);
			break;
		case TVCMD_DRAWSTRIP:
			x11_drawstrip(inst, req);
			break;
		case TVCMD_DRAWTAGS:
			x11_drawtags(inst, req);
			break;
		case TVCMD_DRAWFAN:
			x11_drawfan(inst, req);
			break;
		case TVCMD_DRAWARC:
			x11_drawarc(inst, req);
			break;
		case TVCMD_DRAWFARC:
			x11_drawfarc(inst, req);
			break;
		case TVCMD_COPYAREA:
			x11_copyarea(inst, req);
			break;
		case TVCMD_SETCLIPRECT:
			x11_setcliprect(inst, req);
			break;
		case TVCMD_UNSETCLIPRECT:
			x11_unsetcliprect(inst, req);
			break;
		case TVCMD_DRAWBUFFER:
			x11_drawbuffer(inst, req);
			break;
		default:
			TDBPRINTF(TDB_ERROR,("Unknown command code: %d\n",
			req->tvr_Req.io_Command));
	}
}

static TTASKENTRY void
x11_taskfunc(TAPTR task)
{
	TMOD_X11 *inst = TExecGetTaskData(TGetExecBase(task), task);
	TUINT sig;
	fd_set rset;
	struct TVRequest *req;
	char buf[1];
	struct timeval tv;

	/* interval time: 1/50s: */
	TTIME intt = { 0, 20000 };
	/* next absolute time to send interval message: */
	TTIME nextt;
	TTIME waitt, nowt;

	TTimeQueryTime(inst->x11_TimeBase, inst->x11_TimeReq, &nextt);
	TTimeAddTime(inst->x11_TimeBase, &nextt, &intt);

	TDBPRINTF(TDB_INFO,("Device instance running\n"));

	do
	{
		TBOOL do_interval = TFALSE;

		while (inst->x11_RequestInProgress == TNULL &&
			(req = TExecGetMsg(inst->x11_ExecBase, inst->x11_CmdPort)))
		{
			x11_docmd(inst, req);
			if (inst->x11_RequestInProgress)
				break;
			TExecReplyMsg(inst->x11_ExecBase, req);
		}

		XFlush(inst->x11_Display);

		FD_ZERO(&rset);
		FD_SET(inst->x11_fd_display, &rset);
		FD_SET(inst->x11_fd_sigpipe_read, &rset);

		/* calculate new delta to wait: */
		TTimeQueryTime(inst->x11_TimeBase, inst->x11_TimeReq, &nowt);
		waitt = nextt;
		TTimeSubTime(inst->x11_TimeBase, &waitt, &nowt);

		tv.tv_sec = waitt.ttm_Sec;
		tv.tv_usec = waitt.ttm_USec;
		/* wait for display, signal fd and timeout: */
		if (select(inst->x11_fd_max, &rset, NULL, NULL, &tv) > 0)
			if (FD_ISSET(inst->x11_fd_sigpipe_read, &rset))
				/* consume one signal: */
				read(inst->x11_fd_sigpipe_read, buf, 1);

		/* check if time interval has expired: */
		TTimeQueryTime(inst->x11_TimeBase, inst->x11_TimeReq, &nowt);
		if (TTimeCmpTime(inst->x11_TimeBase, &nowt, &nextt) > 0)
		{
			/* expired; send interval: */
			do_interval = TTRUE;
			TTimeAddTime(inst->x11_TimeBase, &nextt, &intt);
			if (TTimeCmpTime(inst->x11_TimeBase, &nowt, &nextt) >= 0)
			{
				/* nexttime expired already; create new time from now: */
				nextt = nowt;
				TTimeAddTime(inst->x11_TimeBase, &nextt, &intt);
			}
		}

		/* process input messages: */
		x11_processevent(inst);

		/* send out input messages to owners: */
		x11_sendimessages(inst, do_interval);

		/* get signal state: */
		sig = TExecSetSignal(inst->x11_ExecBase, 0, TTASK_SIG_ABORT);

	} while (!(sig & TTASK_SIG_ABORT));

	TDBPRINTF(TDB_INFO,("Device instance closedown\n"));

	/* closedown: */
	x11_exitinstance(inst);
}

LOCAL void x11_wake(TMOD_X11 *inst)
{
	char sig = 0;
	write(inst->x11_fd_sigpipe_write, &sig, 1);
}

/*****************************************************************************/
/*
**	ProcessEvents
*/

static void
x11_processevent(TMOD_X11 *mod)
{
	struct TNode *next, *node;
	XEvent ev;
	VISUAL *v;
	Window w;

	while ((XPending(mod->x11_Display)) > 0)
	{
		XNextEvent(mod->x11_Display, &ev);
		if (ev.type == mod->x11_ShmEvent)
		{
			if (mod->x11_RequestInProgress)
			{
				TExecReplyMsg(mod->x11_ExecBase, mod->x11_RequestInProgress);
				mod->x11_RequestInProgress = TNULL;
			}
			else
				TDBPRINTF(TDB_ERROR,("shm event while no request pending\n"));
			continue;
		}

		/* lookup window: */
		w = ((XAnyEvent *) &ev)->window;
		v = TNULL;
		node = mod->x11_vlist.tlh_Head;
		for (; (next = node->tln_Succ); node = next)
		{
			v = (VISUAL *) node;
			if (v->window == w)
				break;
			v = TNULL;
		}

		if (v == TNULL)
		{
			TDBPRINTF(TDB_WARN,
				("Message Type %04x from unknown window: %p\n", ev.type, w));
			continue;
		}

		/* while true, spool out messages for this particular event: */
		while (x11_processvisualevent(mod, v, TNULL, &ev));
	}
}

static TBOOL getimsg(TMOD_X11 *mod, VISUAL *v, TIMSG **msgptr, TUINT type)
{
	TIMSG *msg = (TIMSG *) TRemHead(&mod->x11_imsgpool);
	if (msg == TNULL)
		msg = TExecAllocMsg0(mod->x11_ExecBase, sizeof(TIMSG));
	if (msg)
	{
		msg->timsg_Type = type;
		msg->timsg_Qualifier = mod->x11_KeyQual;
		msg->timsg_MouseX = mod->x11_MouseX;
		msg->timsg_MouseY = mod->x11_MouseY;
		TTimeQueryTime(mod->x11_TimeBase, mod->x11_TimeReq,
			&msg->timsg_TimeStamp);
		*msgptr = msg;
		return TTRUE;
	}
	*msgptr = TNULL;
	return TFALSE;
}

static TBOOL processkey(TMOD_X11 *mod, VISUAL *v, XKeyEvent *ev, TBOOL keydown)
{
	KeySym keysym;
	XComposeStatus compose;
	char buffer[10];

	TIMSG *imsg;
	TUINT evtype = 0;
	TUINT newqual;
	TUINT evmask = v->eventmask;
	TBOOL newkey = TFALSE;

	mod->x11_MouseX = (TINT) ev->x;
	mod->x11_MouseY = (TINT) ev->y;

	XLookupString(ev, buffer, 10, &keysym, &compose);

	switch (keysym)
	{
		case XK_Shift_L:
			newqual = TKEYQ_LSHIFT;
			break;
		case XK_Shift_R:
			newqual = TKEYQ_RSHIFT;
			break;
		case XK_Control_L:
			newqual = TKEYQ_LCTRL;
			break;
		case XK_Control_R:
			newqual = TKEYQ_RCTRL;
			break;
		case XK_Alt_L:
			newqual = TKEYQ_LALT;
			break;
		case XK_Alt_R:
			newqual = TKEYQ_RALT;
			break;
		default:
			newqual = 0;
	}

	if (newqual != 0)
	{
		if (keydown)
			mod->x11_KeyQual |= newqual;
		else
			mod->x11_KeyQual &= ~newqual;
	}

	if (keydown && (evmask & TITYPE_KEYDOWN))
		evtype = TITYPE_KEYDOWN;
	else if (!keydown && (evmask & TITYPE_KEYUP))
		evtype = TITYPE_KEYUP;

	if (evtype && getimsg(mod, v, &imsg, evtype))
	{
		imsg->timsg_Qualifier = mod->x11_KeyQual;

		if (keysym >= XK_F1 && keysym <= XK_F12)
		{
			imsg->timsg_Code = (TUINT) (keysym - XK_F1) + TKEYC_F1;
			newkey = TTRUE;
		}
		else if (keysym < 256)
		{
			/* cooked ASCII/Latin-1 code */
			imsg->timsg_Code = keysym;
			newkey = TTRUE;
		}
		else if (keysym >= XK_KP_0 && keysym <= XK_KP_9)
		{
			imsg->timsg_Code = (TUINT) (keysym - XK_KP_0) + 48;
			imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
			newkey = TTRUE;
		}
		else
		{
			newkey = TTRUE;
			switch (keysym)
			{
				case XK_Left:
					imsg->timsg_Code = TKEYC_CRSRLEFT;
					break;
				case XK_Right:
					imsg->timsg_Code = TKEYC_CRSRRIGHT;
					break;
				case XK_Up:
					imsg->timsg_Code = TKEYC_CRSRUP;
					break;
				case XK_Down:
					imsg->timsg_Code = TKEYC_CRSRDOWN;
					break;

				case XK_Escape:
					imsg->timsg_Code = TKEYC_ESC;
					break;
				case XK_Delete:
					imsg->timsg_Code = TKEYC_DEL;
					break;
				case XK_BackSpace:
					imsg->timsg_Code = TKEYC_BCKSPC;
					break;
				case XK_ISO_Left_Tab:
				case XK_Tab:
					imsg->timsg_Code = TKEYC_TAB;
					break;
				case XK_Return:
					imsg->timsg_Code = TKEYC_RETURN;
					break;

				case XK_Help:
					imsg->timsg_Code = TKEYC_HELP;
					break;
				case XK_Insert:
					imsg->timsg_Code = TKEYC_INSERT;
					break;
				case XK_Page_Up:
					imsg->timsg_Code = TKEYC_PAGEUP;
					break;
				case XK_Page_Down:
					imsg->timsg_Code = TKEYC_PAGEDOWN;
					break;
				case XK_Home:
					imsg->timsg_Code = TKEYC_POSONE;
					break;
				case XK_End:
					imsg->timsg_Code = TKEYC_POSEND;
					break;
				case XK_Print:
					imsg->timsg_Code = TKEYC_PRINT;
					break;
				case XK_Scroll_Lock:
					imsg->timsg_Code = TKEYC_SCROLL;
					break;
				case XK_Pause:
					imsg->timsg_Code = TKEYC_PAUSE;
					break;
				case XK_KP_Enter:
					imsg->timsg_Code = TKEYC_RETURN;
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				case XK_KP_Decimal:
					imsg->timsg_Code = '.';
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				case XK_KP_Add:
					imsg->timsg_Code = '+';
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				case XK_KP_Subtract:
					imsg->timsg_Code = '-';
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				case XK_KP_Multiply:
					imsg->timsg_Code = '*';
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				case XK_KP_Divide:
					imsg->timsg_Code = '/';
					imsg->timsg_Qualifier |= TKEYQ_NUMBLOCK;
					break;
				default:
					newkey = TFALSE;
			}
		}

		if (!newkey && newqual)
		{
			imsg->timsg_Code = TKEYC_NONE;
			newkey = TTRUE;
		}

		if (newkey)
		{
			ptrdiff_t len =
				(ptrdiff_t) encodeutf8(imsg->timsg_KeyCode, imsg->timsg_Code) -
				(ptrdiff_t) imsg->timsg_KeyCode;
			imsg->timsg_KeyCode[len] = 0;
			imsg->timsg_MouseX = mod->x11_MouseX;
			imsg->timsg_MouseY = mod->x11_MouseY;
			TAddTail(&v->imsgqueue, &imsg->timsg_Node);
		}
		else
		{
			/* put back message: */
			TAddTail(&mod->x11_imsgpool, &imsg->timsg_Node);
		}
	}

	return newkey;
}

static TBOOL
x11_processvisualevent(TMOD_X11 *mod, VISUAL *v, TAPTR msgstate, XEvent *ev)
{
	TIMSG *imsg;

	switch (ev->type)
	{
		case ClientMessage:
			if ((v->eventmask & TITYPE_CLOSE) &&
				ev->xclient.data.l[0] == v->atom_wm_delete_win)
			{
				if (getimsg(mod, v, &imsg, TITYPE_CLOSE))
					TAddTail(&v->imsgqueue, &imsg->timsg_Node);
			}
			break;

		case ConfigureNotify:
			v->winleft = ev->xconfigure.x;
			v->wintop = ev->xconfigure.y;
			if ((v->winwidth != ev->xconfigure.width ||
				v->winheight != ev->xconfigure.height))
			{
				v->waitforexpose = TTRUE;
				v->winwidth = ev->xconfigure.width;
				v->winheight = ev->xconfigure.height;
				if (v->eventmask & TITYPE_NEWSIZE)
				{
					if (getimsg(mod, v, &imsg, TITYPE_NEWSIZE))
						TAddTail(&v->imsgqueue, &imsg->timsg_Node);
					TDBPRINTF(TDB_TRACE, ("Configure: NEWSIZE: %d %d\n",
						v->winwidth, v->winheight));
				}
			}
			break;

		case EnterNotify:
		case LeaveNotify:
			if (v->eventmask & TITYPE_MOUSEOVER)
			{
				if (getimsg(mod, v, &imsg, TITYPE_MOUSEOVER))
				{
					imsg->timsg_Code = (ev->type == EnterNotify);
					TAddTail(&v->imsgqueue, &imsg->timsg_Node);
				}
			}
			break;

		case MapNotify:
			if (mod->x11_RequestInProgress)
			{
				TExecReplyMsg(mod->x11_ExecBase, mod->x11_RequestInProgress);
				mod->x11_RequestInProgress = TNULL;
				v->waitforexpose = TFALSE;
			}
			break;

		case Expose:
			if (v->waitforexpose)
				v->waitforexpose = TFALSE;
			else if ((v->eventmask & TITYPE_REFRESH) &&
				getimsg(mod, v, &imsg, TITYPE_REFRESH))
			{
				imsg->timsg_X = ev->xexpose.x;
				imsg->timsg_Y = ev->xexpose.y;
				imsg->timsg_Width = ev->xexpose.width;
				imsg->timsg_Height = ev->xexpose.height;
				TAddTail(&v->imsgqueue, &imsg->timsg_Node);
					TDBPRINTF(TDB_TRACE, ("Expose: REFRESH: %d %d %d %d\n",
						imsg->timsg_X, imsg->timsg_Y,
						imsg->timsg_Width, imsg->timsg_Height));
			}
			break;

		case GraphicsExpose:
			if (mod->x11_CopyExposeHook)
			{
				TINT rect[4];
				rect[0] = ev->xgraphicsexpose.x;
				rect[1] = ev->xgraphicsexpose.y;
				rect[2] = rect[0] + ev->xgraphicsexpose.width - 1;
				rect[3] = rect[1] + ev->xgraphicsexpose.height - 1;
				TCallHookPkt(mod->x11_CopyExposeHook,
					mod->x11_RequestInProgress->tvr_Op.CopyArea.Instance,
					(TTAG) rect);
			}

			if (ev->xgraphicsexpose.count > 0)
				break;

			/* no more graphics expose events, fallthru: */

		case NoExpose:
			if (mod->x11_RequestInProgress)
			{
				TExecReplyMsg(mod->x11_ExecBase, mod->x11_RequestInProgress);
				mod->x11_RequestInProgress = TNULL;
				mod->x11_CopyExposeHook = TNULL;
			}
			else
				TDBPRINTF(TDB_TRACE,("Map: TITYPE_REFRESH NOT SET\n"));
			break;

		case FocusIn:
		case FocusOut:
			mod->x11_KeyQual = 0;
			if (v->eventmask & TITYPE_FOCUS)
			{
				if (getimsg(mod, v, &imsg, TITYPE_FOCUS))
				{
					imsg->timsg_Code = (ev->type == FocusIn);
					TAddTail(&v->imsgqueue, &imsg->timsg_Node);
				}
			}
			break;

		case MotionNotify:
			mod->x11_MouseX = (TINT) ev->xmotion.x;
			mod->x11_MouseY = (TINT) ev->xmotion.y;
			if (v->eventmask & TITYPE_MOUSEMOVE)
			{
				if (getimsg(mod, v, &imsg, TITYPE_MOUSEMOVE))
				{
					imsg->timsg_MouseX = mod->x11_MouseX;
					imsg->timsg_MouseY = mod->x11_MouseY;
					TAddTail(&v->imsgqueue, &imsg->timsg_Node);
				}
			}
			break;

		case ButtonRelease:
		case ButtonPress:
			mod->x11_MouseX = (TINT) ev->xbutton.x;
			mod->x11_MouseY = (TINT) ev->xbutton.y;
			if (v->eventmask & TITYPE_MOUSEBUTTON)
			{
				if (getimsg(mod, v, &imsg, TITYPE_MOUSEBUTTON))
				{
					unsigned int button = ev->xbutton.button;
					if (ev->type == ButtonPress)
					{
						switch (button)
						{
							case Button1:
								imsg->timsg_Code = TMBCODE_LEFTDOWN;
								break;
							case Button2:
								imsg->timsg_Code = TMBCODE_MIDDLEDOWN;
								break;
							case Button3:
								imsg->timsg_Code = TMBCODE_RIGHTDOWN;
								break;
							case Button4:
								imsg->timsg_Code = TMBCODE_WHEELUP;
								break;
							case Button5:
								imsg->timsg_Code = TMBCODE_WHEELDOWN;
								break;
						}
					}
					else
					{
						switch (button)
						{
							case Button1:
								imsg->timsg_Code = TMBCODE_LEFTUP;
								break;
							case Button2:
								imsg->timsg_Code = TMBCODE_MIDDLEUP;
								break;
							case Button3:
								imsg->timsg_Code = TMBCODE_RIGHTUP;
								break;
						}
					}
					imsg->timsg_MouseX = mod->x11_MouseX;
					imsg->timsg_MouseY = mod->x11_MouseY;
					TAddTail(&v->imsgqueue, &imsg->timsg_Node);
				}
			}
			break;

		case KeyRelease:
			processkey(mod, v, (XKeyEvent *) ev, TFALSE);
			break;

		case KeyPress:
			processkey(mod, v, (XKeyEvent *) ev, TTRUE);
			break;
	}
	return TFALSE;
}

/*****************************************************************************/

static void
x11_sendimessages(TMOD_X11 *mod, TBOOL do_interval)
{
	struct TNode *next, *node = mod->x11_vlist.tlh_Head;
	for (; (next = node->tln_Succ); node = next)
	{
		VISUAL *v = (VISUAL *) node;
		TIMSG *imsg;

		if (do_interval && (v->eventmask & TITYPE_INTERVAL) &&
			getimsg(mod, v, &imsg, TITYPE_INTERVAL))
			TExecPutMsg(mod->x11_ExecBase, v->imsgport, TNULL, imsg);

		while ((imsg = (TIMSG *) TRemHead(&v->imsgqueue)))
			TExecPutMsg(mod->x11_ExecBase, v->imsgport, TNULL, imsg);
	}
}

