
#ifndef LUA_TEK_LIB_VISUAL_H
#define LUA_TEK_LIB_VISUAL_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <tek/debug.h>
#include <tek/teklib.h>
#include <tek/inline/exec.h>
#include <tek/inline/time.h>
#include <tek/proto/visual.h>

extern TMODENTRY TUINT
tek_init_display_x11(TAPTR, struct TModule *, TUINT16, TTAGITEM *);

/*****************************************************************************/

#ifndef LUACFUNC
#define LUACFUNC TCALLBACK
#endif

#ifndef EXPORT
#define EXPORT TMODAPI
#endif

#ifndef LOCAL
#define LOCAL TMODINTERN
#endif

/*****************************************************************************/

typedef struct
{
	TINT nump;
	TINT *points;
	TVPEN *pens;

} TEKDrawdata;

typedef struct TEKVisual
{
	/* Visualbase: */
	TAPTR vis_Base;
	/* Execbase: */
	TAPTR vis_ExecBase;
	/* Timebase: */
	TAPTR vis_TimeBase;
	/* Time request */
	TAPTR vis_TimeRequest;
	/* Reference to base (stored in metatable): */
	int vis_refBase;
	/* Is base instance: */
	TBOOL vis_isBase;

	/* Reference to font (stored in metatable): */
	int vis_refFont;
	/* Font (always default font in base): */
	TAPTR vis_Font;
	/* FontHeight (always default font height in base): */
	TINT vis_FontHeight;

	/* Visual Display ptr: */
	TAPTR vis_Display;

	/* Visual instance ptr: */
	TAPTR vis_Visual;

	TEKDrawdata vis_Drawdata;

	/* ShiftX/Y: */
	TINT vis_ShiftX, vis_ShiftY;

	TINT vis_RectBufferNum;
	TINT *vis_RectBuffer;

} TEKVisual;

typedef struct
{
	/* Pen object: */
	TVPEN pen_Pen;
	/* Visual: */
	TEKVisual *pen_Visual;

} TEKPen;

typedef struct
{
	/* Visualbase: */
	TEKVisual *font_VisBase;
	/* Font object: */
	TAPTR font_Font;
	/* Font height: */
	TUINT font_Height;
	/* underline position: */
	TINT font_UlPosition;
	/* underline thickness: */
	TINT font_UlThickness;

} TEKFont;

/*****************************************************************************/

#define TExecBase vis->vis_ExecBase
#define TTimeBase vis->vis_TimeBase

#define TEK_LIB_VISUAL_BASECLASSNAME "tek.lib.visual.base*"
#define TEK_LIB_VISUAL_CLASSNAME "tek.lib.visual*"
#define TEK_LIB_VISUALPEN_CLASSNAME "tek.lib.visual.pen*"
#define TEK_LIB_VISUALFONT_CLASSNAME "tek.lib.visual.font*"

/*****************************************************************************/

LOCAL LUACFUNC TINT tek_lib_visual_open(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_close(lua_State *L);

LOCAL LUACFUNC TINT tek_lib_visual_wait(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_sleep(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_openfont(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_closefont(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_textsize_font(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_gettime(lua_State *L);

LOCAL LUACFUNC TINT tek_lib_visual_setinput(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_clearinput(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_getmsg(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_allocpen(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_freepen(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_frect(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_rect(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_line(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_plot(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_text(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_drawimage(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_getattrs(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_setattrs(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_textsize_visual(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_setfont(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_copyarea(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_setcliprect(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_unsetcliprect(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_setshift(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_drawrgb(lua_State *L);
LOCAL LUACFUNC TINT tek_lib_visual_getfontattrs(lua_State *L);

#endif
