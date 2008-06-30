#ifndef _TEK_VISUAL_DISPLAY_DFB_MOD_H
#define _TEK_VISUAL_DISPLAY_DFB_MOD_H

/*
**	tekui/src/display_dfb/display_dfb_mod.h - DirectFB Display Driver
**	Written by Franciska Schulze <fschulze at schulze-mueller.de>
**	See copyright notice in tekui/COPYRIGHT
*/

#include <string.h>
#include <directfb.h>
#include <directfb_keynames.h>

#include <tek/debug.h>
#include <tek/exec.h>
#include <tek/teklib.h>

#include <tek/proto/exec.h>
#include <tek/proto/time.h>
#include <tek/mod/visual.h>

/*****************************************************************************/

#define DFBDISPLAY_VERSION		1
#define DFBDISPLAY_REVISION		0
#define DFBDISPLAY_NUMVECTORS	10

#define DEF_WINWIDTH			600
#define DEF_WINHEIGHT			400

#ifndef LOCAL
#define LOCAL
#endif

#ifndef EXPORT
#define EXPORT TMODAPI
#endif

/*****************************************************************************/

struct utf8reader
{
	/* character reader callback: */
	int (*readchar)(struct utf8reader *);
	/* reader state: */
	int accu, numa, min, bufc;
	/* userdata to reader */
	void *udata;
};

LOCAL int readutf8(struct utf8reader *rd);
LOCAL unsigned char *encodeutf8(unsigned char *buf, int c);

/*****************************************************************************/
#ifndef CUR_DEFFILE
#define CUR_DEFFILE			TEKHOST_SYSDIR"cursors/cursor-green.png"
#endif

#ifndef FNT_DEFDIR
#define	FNT_DEFDIR			TEKHOST_SYSDIR"fonts/"
#endif

#define FNT_DEFNAME			"VeraMono"
#define FNT_DEFPXSIZE		14
#define	FNT_WILDCARD		"*"

#define FNTQUERY_NUMATTR	(5+1)
#define	FNTQUERY_UNDEFINED	-1

#define FNT_ITALIC			0x1
#define	FNT_BOLD			0x2
#define FNT_UNDERLINE		0x4

#define FNT_MATCH_NAME		0x01
#define FNT_MATCH_SIZE		0x02
#define FNT_MATCH_SLANT		0x04
#define	FNT_MATCH_WEIGHT	0x08
#define	FNT_MATCH_SCALE		0x10
/* all mandatory properties: */
#define FNT_MATCH_ALL		0x0f

struct FontMan
{
	/* list of opened fonts */
	struct TList openfonts;
	/* pointer to default font */
	TAPTR deffont;
	/* count of references to default font */
	TINT defref;
};

struct FontNode
{
	struct THandle handle;
	IDirectFBFont *font;
	TUINT attr;
	TUINT pxsize;
	TINT ascent;
	TINT descent;
	TINT height;
};

struct FontQueryNode
{
	struct TNode node;
	TTAGITEM tags[FNTQUERY_NUMATTR];
};

struct FontQueryHandle
{
	struct THandle handle;
	struct TList reslist;
	struct TNode **nptr;
};

/* internal structures */

struct fnt_node
{
	struct TNode node;
	TSTRPTR fname;
};

struct fnt_attr
{
	/* list of fontnames */
	struct TList fnlist;
	TSTRPTR fname;
	TINT  fpxsize;
	TBOOL fitalic;
	TBOOL fbold;
	TBOOL fscale;
	TINT  fnum;
};

/*****************************************************************************/

typedef struct DFBDisplay
{
	/* Module header: */
	struct TModule dfb_Module;
	/* Exec module base ptr: */
	TAPTR dfb_ExecBase;
	/* Locking for module base structure: */
	TAPTR dfb_Lock;
	/* Number of module opens: */
	TAPTR dfb_RefCount;
	/* Task: */
	TAPTR dfb_Task;
	/* Command message port: */
	TAPTR dfb_CmdPort;
	/* Command message port signal: */
	TUINT dfb_CmdPortSignal;

	/* DirectFB super interface: */
	IDirectFB *dfb_DFB;
	/* DirectFB primary Layer */
	IDirectFBDisplayLayer *dfb_Layer;
	/* input interfaces: device and its buffer: */
	IDirectFBEventBuffer *dfb_Events;
	/* DirectFB root window, used capture background events */
	IDirectFBWindow *dfb_RootWindow;

	DFBDisplayLayerConfig dfb_LayerConfig;
	DFBGraphicsDeviceDescription dfb_DevDsc;

	/* cursor image */
	IDirectFBSurface *dfb_CursorSurface;

	TINT dfb_ScrWidth;
	TINT dfb_ScrHeight;
	TINT dfb_MouseX;
	TINT dfb_MouseY;
	TINT dfb_KeyQual;

	TAPTR dfb_Focused;
	TAPTR dfb_Active;

	/* Input file descriptor: */
	int dfb_FDInput;

	/* Self-pipe: */
	int dfb_FDSigPipeRead;
	int dfb_FDSigPipeWrite;
	int dfb_FDMax;

	/* pooled input messages: */
	struct TList dfb_imsgpool;
	/* list of all visuals: */
	struct TList dfb_vlist;
	/* Time module base ptr: */
	TAPTR dfb_TimeBase;
	/* Timerequest: */
	TAPTR dfb_TimeReq;
	/* Module global memory manager (thread safe): */
	TAPTR dfb_MemMgr;

	struct FontMan dfb_fm;

} TMOD_DFB;

typedef struct
{
	struct TNode node;

	TINT winwidth, winheight;
	TINT winleft, wintop;
	TINT wminwidth, wminheight;
	TINT wmaxwidth, wmaxheight;
	TBOOL borderless;
	TSTRPTR title;

	TUINT base_mask;
	TUINT eventmask;

	TVPEN bgpen, fgpen;

	struct TList imsgqueue;
	TAPTR imsgport;

	/* list of allocated pens: */
	struct TList penlist;

	IDirectFBWindow *window;
	IDirectFBSurface *winsurface;
	DFBWindowID winid;

	/* current active font */
	TAPTR curfont;
} VISUAL;

struct DFBPen
{
	struct TNode node;
	TUINT8 r;
	TUINT8 g;
	TUINT8 b;
	TUINT8 a;
};

struct attrdata
{
	TMOD_DFB *mod;
	VISUAL *v;
	TAPTR font;
	TINT num;
	TBOOL sizechanged;
};

/*****************************************************************************/

LOCAL TBOOL dfb_init(TMOD_DFB *mod, TTAGITEM *tags);
LOCAL void dfb_exit(TMOD_DFB *mod);
LOCAL void dfb_wake(TMOD_DFB *inst);
LOCAL void dfb_openvisual(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_closevisual(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_setinput(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_allocpen(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_freepen(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_frect(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_rect(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_line(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_plot(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawstrip(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_clear(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_getattrs(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_setattrs(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawtext(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_openfont(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_getfontattrs(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_textsize(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_setfont(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_closefont(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_queryfonts(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_getnextfont(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawtags(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawfan(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawarc(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_copyarea(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_setcliprect(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawfarc(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_unsetcliprect(TMOD_DFB *mod, struct TVRequest *req);
LOCAL void dfb_drawbuffer(TMOD_DFB *mod, struct TVRequest *req);

LOCAL TAPTR dfb_hostopenfont(TMOD_DFB *mod, TTAGITEM *tags);
LOCAL void dfb_hostclosefont(TMOD_DFB *mod, TAPTR font);
LOCAL void dfb_hostsetfont(TMOD_DFB *mod, VISUAL *v, TAPTR font);
LOCAL TTAGITEM *dfb_hostgetnextfont(TMOD_DFB *mod, TAPTR fqhandle);
LOCAL TINT dfb_hosttextsize(TMOD_DFB *mod, TAPTR font, TSTRPTR text);
LOCAL THOOKENTRY TTAG dfb_hostgetfattrfunc(struct THook *hook, TAPTR obj, TTAG msg);
LOCAL TAPTR dfb_hostqueryfonts(TMOD_DFB *mod, TTAGITEM *tags);

#endif /* _TEK_VISUAL_DISPLAY_DFB_MOD_H */
