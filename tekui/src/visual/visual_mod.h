
#ifndef _TEK_MODS_VISUAL_MOD_H
#define _TEK_MODS_VISUAL_MOD_H

/*
**	teklib/src/visual/visual_mod.h - Visual module
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/debug.h>
#include <tek/exec.h>
#include <tek/teklib.h>

#include <tek/proto/exec.h>
#include <tek/proto/time.h>
#include <tek/mod/visual.h>

#include <tek/proto/display.h>

/*****************************************************************************/

#define VISUAL_VERSION		4
#define VISUAL_REVISION		0
#define VISUAL_NUMVECTORS	43

#ifndef LOCAL
#define LOCAL
#endif

#ifndef EXPORT
#define EXPORT TMODAPI
#endif

#define VISUAL_MAXREQPERINSTANCE	64

/*****************************************************************************/

struct vis_Hash;

struct vis_HashNode
{
	struct TNode node;
	TSTRPTR key;
	TTAG value;
	TUINT hash;
};

typedef struct
{
	/* Module header: */
	struct TModule vis_Module;
	/* Exec module base ptr: */
	TAPTR vis_ExecBase;
	/* Module global memory manager (thread safe): */
	TAPTR vis_MemMgr;
	/* Locking for module base structure: */
	TAPTR vis_Lock;
	/* Number of module opens: */
	TAPTR vis_RefCount;
	/* Hash of displays: */
	struct vis_Hash *vis_Displays;

	/* Instance-specific (not in base): */

	struct TVRequest *vis_InitRequest;
	/* Display: */
	TAPTR vis_Display;
	/* Visual instance ptr from backend: */
	TAPTR vis_Visual;
	/* Current set of input types to listen for: */
	TUINT vis_InputMask;
	/* Request pool: */
	struct TList vis_ReqPool;
	/* Replyport for requests: */
	TAPTR vis_CmdRPort;
	/* Port for input messages: */
	TAPTR vis_IMsgPort;
	/* List of waiting asynchronous requests: */
	struct TList vis_WaitList;
	/* Number of requests allocated so far: */
	TINT vis_NumRequests;

} TMOD_VIS;

LOCAL struct vis_Hash *vis_createhash(TMOD_VIS *mod, TAPTR udata);
LOCAL void vis_destroyhash(TMOD_VIS *mod, struct vis_Hash *hash);
LOCAL int vis_puthash(TMOD_VIS *mod, struct vis_Hash *hash, const TSTRPTR key,
	TTAG value);
LOCAL int vis_gethash(TMOD_VIS *mod, struct vis_Hash *hash, const TSTRPTR key,
	TTAG *valp);
LOCAL int vis_remhash(TMOD_VIS *mod, struct vis_Hash *hash, const TSTRPTR key);
LOCAL TUINT vis_hashtolist(TMOD_VIS *mod, struct vis_Hash *hash,
	struct TList *list);
LOCAL void vis_hashunlist(TMOD_VIS *mod, struct vis_Hash *hash);

/*****************************************************************************/

EXPORT TAPTR vis_openvisual(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT void vis_closevisual(TMOD_VIS *mod, TMOD_VIS *inst);
EXPORT TAPTR vis_attach(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT TAPTR vis_openfont(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT void vis_closefont(TMOD_VIS *mod, TAPTR font);
EXPORT TUINT vis_getfattrs(TMOD_VIS *mod, TAPTR font, TTAGITEM *tags);
EXPORT TINT vis_textsize(TMOD_VIS *mod, TAPTR font, TSTRPTR t);
EXPORT TAPTR vis_queryfonts(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT TTAGITEM *vis_getnextfont(TMOD_VIS *mod, TAPTR fqhandle);

EXPORT TAPTR vis_getport(TMOD_VIS *mod);
EXPORT TUINT vis_setinput(TMOD_VIS *mod, TUINT cmask, TUINT smask);
EXPORT TUINT vis_getattrs(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT TUINT vis_setattrs(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT TVPEN vis_allocpen(TMOD_VIS *mod, TUINT rgb);
EXPORT void vis_freepen(TMOD_VIS *mod, TVPEN pen);
EXPORT void vis_setfont(TMOD_VIS *mod, TAPTR font);
EXPORT void vis_clear(TMOD_VIS *mod, TVPEN pen);
EXPORT void vis_rect(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h, TVPEN pen);
EXPORT void vis_frect(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h,
	TVPEN pen);
EXPORT void vis_line(TMOD_VIS *mod, TINT x1, TINT y1, TINT x2, TINT y2,
	TVPEN pen);
EXPORT void vis_plot(TMOD_VIS *mod, TINT x, TINT y, TVPEN pen);
EXPORT void vis_text(TMOD_VIS *mod, TINT x, TINT y, TSTRPTR t, TUINT l,
	TVPEN fg, TVPEN bg);

EXPORT void vis_drawstrip(TMOD_VIS *mod, TINT *array, TINT num, TTAGITEM *tags);
EXPORT void vis_drawtags(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT void vis_drawfan(TMOD_VIS *mod, TINT *array, TINT num, TTAGITEM *tags);
EXPORT void vis_drawarc(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h,
	TINT angle1, TINT angle2, TVPEN pen);

EXPORT void vis_copyarea(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h,
	TINT dx, TINT dy, TTAGITEM *tags);
EXPORT void vis_setcliprect(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h,
	TTAGITEM *tags);

EXPORT TAPTR vis_opendisplay(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT void vis_closedisplay(TMOD_VIS *mod, TAPTR display);
EXPORT TAPTR vis_querydisplays(TMOD_VIS *mod, TTAGITEM *tags);
EXPORT TTAGITEM *vis_getnextdisplay(TMOD_VIS *mod, TAPTR dqhandle);

EXPORT void vis_unsetcliprect(TMOD_VIS *mod);
EXPORT void vis_drawfarc(TMOD_VIS *mod, TINT x, TINT y, TINT w, TINT h,
	TINT angle1, TINT angle2, TVPEN pen);

EXPORT void vis_drawbuffer(TMOD_VIS *inst,
	TINT x, TINT y, TAPTR buf, TINT w, TINT h, TINT totw, TTAGITEM *tags);

#endif
