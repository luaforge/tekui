
#ifndef _TEK_VISUAL_DISPLAY_X11_MOD_H
#define _TEK_VISUAL_DISPLAY_X11_MOD_H

/*
**	teklib/src/visual/display_x11_mod.h - X11 Display Driver
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/debug.h>
#include <tek/exec.h>
#include <tek/teklib.h>

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include <tek/proto/exec.h>
#include <tek/proto/time.h>
#include <tek/mod/visual.h>

/*****************************************************************************/

#define X11DISPLAY_VERSION		1
#define X11DISPLAY_REVISION		1
#define X11DISPLAY_NUMVECTORS	10

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

#define DEFFONTNAME			"-misc-fixed-medium-r-normal-*-14-*-*-*-*-*-*-*"

#define FNT_LENGTH			41
#define FNT_DEFNAME			"fixed"
#define FNT_WGHT_MEDIUM		"medium"
#define	FNT_WGHT_BOLD		"bold"
#define FNT_SLANT_R			"r"
#define FNT_SLANT_I			"i"
#define FNT_DEFPXSIZE		14
#define	FNT_DEFREGENC		"iso8859-1"
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
	struct TList openfonts;		/* list of opened fonts */
	TAPTR deffont;				/* pointer to default font */
	TINT defref;				/* count of references to default font */
};

struct FontNode
{
	struct THandle handle;
	XFontStruct *font;
	XftFont *xftfont;
	TUINT attr;
	TUINT pxsize;
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
	struct TList fnlist;	/* list of fontnames */
	TSTRPTR fname;
	TINT  fpxsize;
	TBOOL fitalic;
	TBOOL fbold;
	TBOOL fscale;
	TINT  fnum;
};

struct XftInterface
{
	XftFont *(*XftFontOpen)(Display *dpy, int screen, ...);
	void (*XftFontClose)(Display *dpy, XftFont *pub);
	void (*XftTextExtentsUtf8)(Display *dpy, XftFont *pub, _Xconst FcChar8 *string,
		int len, XGlyphInfo *extents);
	void (*XftDrawStringUtf8)(XftDraw *draw, _Xconst XftColor *color, XftFont *pub,
		int x, int y, _Xconst FcChar8  *string, int len);
	void (*XftDrawRect)(XftDraw *draw, _Xconst XftColor *color, int x, int y,
		unsigned int width, unsigned int height);
	FT_Face (*XftLockFace)(XftFont *pub);
	void (*XftUnlockFace)(XftFont *pub);
	Bool (*XftColorAllocValue)(Display *dpy, Visual *visual, Colormap cmap,
		_Xconst XRenderColor *color, XftColor *result);
	void (*XftColorFree)(Display *dpy, Visual *visual, Colormap  cmap,
		XftColor *color);
	XftDraw *(*XftDrawCreate)(Display *dpy, Drawable  drawable, Visual *visual,
		Colormap colormap);
	void (*XftDrawDestroy)(XftDraw *draw);
	Bool (*XftDrawSetClip)(XftDraw *d, Region r);
};

#define LIBXFT_NUMSYMS	(sizeof(struct XftInterface) / sizeof(void (*)(void)))


struct FcInterface
{
	void (*FcDefaultSubstitute)(FcPattern *pattern);
	void (*FcFontSetDestroy)(FcFontSet *s);
	FcFontSet *(*FcFontSort)(FcConfig *config, FcPattern *p, FcBool trim,
		FcCharSet **csp, FcResult *result);
	FcBool (*FcPatternAddBool)(FcPattern *p, const char *object, FcBool b);
	FcBool (*FcPatternAddDouble)(FcPattern *p, const char *object, double d);
	FcBool (*FcPatternAddInteger)(FcPattern *p, const char *object, int i);
	FcPattern *(*FcPatternBuild)(FcPattern *orig, ...);
	void (*FcPatternDestroy)(FcPattern *p);
	void (*FcPatternPrint)(const FcPattern *p);
	FcResult (*FcPatternGetString)(const FcPattern *p, const char *object,
		int n, FcChar8 **s);
	FcResult (*FcPatternGetDouble)(const FcPattern *p, const char *object,
		int n, double *d);
	FcResult (*FcPatternGetInteger)(const FcPattern *p, const char *object,
		int n, int *i);
	FcResult (*FcPatternGetBool)(const FcPattern *p, const char *object,
		int n, FcBool *b);
	FcBool (*FcInit)(void);
};

#define LIBFC_NUMSYMS	(sizeof(struct FcInterface) / sizeof(void (*)(void)))

/*****************************************************************************/

typedef struct X11Display
{
	/* Module header: */
	struct TModule x11_Module;
	/* Exec module base ptr: */
	TAPTR x11_ExecBase;
	/* Time module base ptr: */
	TAPTR x11_TimeBase;
	/* Timerequest: */
	TAPTR x11_TimeReq;
	/* Module global memory manager (thread safe): */
	TAPTR x11_MemMgr;
	/* Locking for module base structure: */
	TAPTR x11_Lock;
	/* Number of module opens: */
	TAPTR x11_RefCount;
	/* Task: */
	TAPTR x11_Task;
	/* Command message port: */
	TAPTR x11_CmdPort;
	/* Command message port signal: */
	TUINT x11_CmdPortSignal;

	/* X11 display: */
	Display *x11_Display;
	/* default X11 screen number: */
	int x11_Screen;
	/* default X11 visual: */
	Visual *x11_Visual;

	TUINT x11_Flags;
	TUINT x11_PixFmt;
	TINT x11_Depth, x11_BPP;

	int x11_fd_display;
	int x11_fd_sigpipe_read;
	int x11_fd_sigpipe_write;
	int x11_fd_max;

	TBOOL x11_use_xft;
	TAPTR x11_libxfthandle;
	struct XftInterface x11_xftiface;
	TAPTR x11_libfchandle;
	struct FcInterface x11_fciface;

	struct FontMan x11_fm;

	/* list of all visuals: */
	struct TList x11_vlist;

	struct TList x11_imsgpool;

	struct TVRequest *x11_RequestInProgress;
	struct THook *x11_CopyExposeHook;

	Region x11_HugeRegion;
	TINT x11_Shm, x11_ShmEvent;

	TINT x11_KeyQual;
	TINT x11_MouseX, x11_MouseY;

} TMOD_X11;

struct X11Pen
{
	struct TNode node;
	XColor color;
	XftColor xftcolor;
};

typedef struct
{
	struct TNode node;

	TINT winwidth, winheight;
	TINT winleft, wintop;
	TSTRPTR title;

	Window window;

	XTextProperty title_prop;
	Colormap colormap;
	GC gc;

	XftDraw *draw;
	TAPTR curfont;				/* current active font */

	Atom atom_wm_delete_win;

	TUINT base_mask;
	TUINT eventmask;

	TVPEN bgpen, fgpen;

	XImage *image;
	char *tempbuf;
	int imw, imh;

	XSizeHints *sizehints;

	struct TList imsgqueue;
	TAPTR imsgport;

	/* list of allocated pens: */
	struct TList penlist;

	/* HACK to consume an Expose event after ConfigureNotify: */
	TBOOL waitforexpose;

	TINT shm, shmevent;
	XShmSegmentInfo shminfo;

} VISUAL;

struct attrdata
{
	TMOD_X11 *mod;
	VISUAL *v;
	TAPTR font;
	TINT num;
	TBOOL sizechanged;
	TINT neww, newh;
};

/*****************************************************************************/

#define TVISF_SWAPBYTEORDER	0x00000001

#define PIXFMT_UNDEFINED	0
#define PIXFMT_RGB			1
#define PIXFMT_RBG			2
#define PIXFMT_BRG			3
#define PIXFMT_BGR			4
#define PIXFMT_GRB			5
#define PIXFMT_GBR			6

/*****************************************************************************/

LOCAL TBOOL x11_init(TMOD_X11 *mod, TTAGITEM *tags);
LOCAL void x11_exit(TMOD_X11 *mod);
LOCAL void x11_openvisual(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_closevisual(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_setinput(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_allocpen(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_freepen(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_frect(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_rect(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_line(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_plot(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawstrip(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_clear(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_getattrs(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_setattrs(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawtext(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_openfont(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_getfontattrs(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_textsize(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_setfont(TMOD_X11 *mod, struct TVRequest *req);

LOCAL void x11_closefont(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_queryfonts(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_getnextfont(TMOD_X11 *mod, struct TVRequest *req);

LOCAL void x11_drawtags(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawfan(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawarc(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_copyarea(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_setcliprect(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_unsetcliprect(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawfarc(TMOD_X11 *mod, struct TVRequest *req);
LOCAL void x11_drawbuffer(TMOD_X11 *mod, struct TVRequest *req);

LOCAL void x11_wake(TMOD_X11 *inst);

LOCAL void x11_hostsetfont(TMOD_X11 *mod, VISUAL *v, TAPTR font);
LOCAL TAPTR x11_hostopenfont(TMOD_X11 *mod, TTAGITEM *tags);
LOCAL TAPTR x11_hostqueryfonts(TMOD_X11 *mod, TTAGITEM *tags);
LOCAL void x11_hostclosefont(TMOD_X11 *mod, TAPTR font);
LOCAL TINT  x11_hosttextsize(TMOD_X11 *mod, TAPTR font, TSTRPTR text);
LOCAL THOOKENTRY TTAG x11_hostgetfattrfunc(struct THook *hook, TAPTR obj, TTAG msg);
LOCAL TTAGITEM *x11_hostgetnextfont(TMOD_X11 *mod, TAPTR fqhandle);

TSTRPTR utf8tolatin(TMOD_X11 *mod, TSTRPTR utf8string, TINT len);

#endif
