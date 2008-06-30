
/*
**	tek.lib.visual - binding of TEKlib 'Visual' module to Lua
**	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
**	See copyright notice in COPYRIGHT
*/

#include <string.h>
#include "visual_lua.h"

#if !defined(DISPLAY_DRIVER)
#define DISPLAY_DRIVER "x11"
#endif

/*****************************************************************************/

static const struct TInitModule initmodules[] =
{
	{ "visual", tek_init_visual, TNULL, 0 },
	{ TNULL }
};

static struct TModInitNode im_visual =
{
	{ TNULL, TNULL },
	(struct TInitModule *) initmodules,
	TNULL,
};

static const luaL_Reg libfuncs[] =
{
	{ "open", tek_lib_visual_open },
	{ "close", tek_lib_visual_close },
	{ "wait", tek_lib_visual_wait },
	{ "sleep", tek_lib_visual_sleep },
	{ "openfont", tek_lib_visual_openfont },
	{ "closefont", tek_lib_visual_closefont },
	{ "textsize", tek_lib_visual_textsize_font },
	{ "gettime", tek_lib_visual_gettime },
	{ TNULL, TNULL }
};

static const luaL_Reg classmethods[] =
{
	{ "__gc", tek_lib_visual_close },
	{ "setinput", tek_lib_visual_setinput },
	{ "clearinput", tek_lib_visual_clearinput },
	{ "close", tek_lib_visual_close },
	{ "wait", tek_lib_visual_wait },
	{ "getmsg", tek_lib_visual_getmsg },
	{ "allocpen", tek_lib_visual_allocpen },
	{ "freepen", tek_lib_visual_freepen },
	{ "frect", tek_lib_visual_frect },
	{ "rect", tek_lib_visual_rect },
	{ "line", tek_lib_visual_line },
	{ "plot", tek_lib_visual_plot },
	{ "text", tek_lib_visual_text },
	{ "drawimage", tek_lib_visual_drawimage },
	{ "getattrs", tek_lib_visual_getattrs },
	{ "setattrs", tek_lib_visual_setattrs },
	{ "textsize", tek_lib_visual_textsize_visual },
	{ "setfont", tek_lib_visual_setfont },
	{ "copyarea", tek_lib_visual_copyarea },
	{ "setcliprect", tek_lib_visual_setcliprect },
	{ "unsetcliprect", tek_lib_visual_unsetcliprect },
	{ "setshift", tek_lib_visual_setshift },
	{ "drawrgb", tek_lib_visual_drawrgb },
	{ TNULL, TNULL }
};

static const luaL_Reg fontmethods[] =
{
	{ "__gc", tek_lib_visual_closefont },
	{ "getattrs", tek_lib_visual_getfontattrs },
	{ "close", tek_lib_visual_closefont },
	{ TNULL, TNULL }
};

/*****************************************************************************/
/*
**	visual_open { args }
**	args.Title - Title of Window
**	args.Width - Width of the window
**	args.Height - Height of the window
**	args.Left - Left position of the window
**	args.Top - Top position of the window
**	args.Borderless - open borderless window
**	args.CenterWindow - open window centered
**	args.MinWidth - minimum width of the window
**	args.MinHeight - minimum height of the window
**	args.MaxWidth - maximum width of the window
**	args.MaxHeight - maximum height of the window
*/

LOCAL LUACFUNC TINT
tek_lib_visual_open(lua_State *L)
{
	TTAGITEM tags[14], *tp = tags;
	TEKVisual *visbase, *vis;

	vis = lua_newuserdata(L, sizeof(TEKVisual));
	/* s: visinst */

	vis->vis_Visual = TNULL;
	vis->vis_isBase = TFALSE;
	vis->vis_refBase = -1;
	vis->vis_refFont = -1;

	/* get and attach metatable: */
	luaL_newmetatable(L, TEK_LIB_VISUAL_CLASSNAME);
	/* s: visinst, vismeta */
	lua_pushvalue(L, -1);
	/* s: visinst, vismeta, vismeta */
	lua_setmetatable(L, -3);
	/* s: visinst, vismeta */

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	/* s: visinst, vismeta, visbase */

	/* get visual module base: */
	visbase = (TEKVisual *) lua_touserdata(L, -1);
	if (visbase == TNULL)
	{
		lua_pop(L, 3);
		return 0;
	}

	vis->vis_Base = visbase->vis_Base;
	vis->vis_ExecBase = visbase->vis_ExecBase;
	vis->vis_Font = visbase->vis_Font;
	vis->vis_FontHeight = visbase->vis_FontHeight;
	vis->vis_Drawdata.nump = 0;
	vis->vis_Drawdata.points = TNULL;
	vis->vis_Drawdata.pens = TNULL;
	vis->vis_RectBuffer = TNULL;
	vis->vis_ShiftX = 0;
	vis->vis_ShiftY = 0;
	vis->vis_Display = visbase->vis_Display;

	/* place ref to base in metatable: */
	vis->vis_refBase = luaL_ref(L, -2);
	/* s: visinst, vismeta */

	tp->tti_Tag = TVisual_Title;
	lua_getfield(L, 1, "Title");
	if (lua_isstring(L, -1))
		tp++->tti_Value = (TTAG) lua_tostring(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Borderless;
	lua_getfield(L, 1, "Borderless");
	if (lua_isboolean(L, -1))
		tp++->tti_Value = (TTAG) lua_toboolean(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Center;
	lua_getfield(L, 1, "Center");
	if (lua_isboolean(L, -1))
		tp++->tti_Value = (TTAG) lua_toboolean(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Fullscreen;
	lua_getfield(L, 1, "Fullscreen");
	if (lua_isboolean(L, -1))
		tp++->tti_Value = (TTAG) lua_toboolean(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Width;
	lua_getfield(L, 1, "Width");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Height;
	lua_getfield(L, 1, "Height");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_WinLeft;
	lua_getfield(L, 1, "Left");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_WinTop;
	lua_getfield(L, 1, "Top");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_MinWidth;
	lua_getfield(L, 1, "MinWidth");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_MinHeight;
	lua_getfield(L, 1, "MinHeight");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_MaxWidth;
	lua_getfield(L, 1, "MaxWidth");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_MaxHeight;
	lua_getfield(L, 1, "MaxHeight");
	if (lua_isnumber(L, -1))
		tp++->tti_Value = (TTAG) lua_tointeger(L, -1);
	lua_pop(L, 1);

	tp->tti_Tag = TVisual_Display;
	tp++->tti_Value = (TTAG) vis->vis_Display;

	tp->tti_Tag = TTAG_DONE;

	vis->vis_Visual = TVisualOpen(visbase->vis_Base, tags);
	if (vis->vis_Visual)
	{
		TVisualSetFont(vis->vis_Visual, vis->vis_Font);
		lua_pop(L, 1);
	}
	else
	{
		TDBPRINTF(TDB_ERROR,("Failed to open visual\n"));
		lua_pop(L, 2);
		lua_pushnil(L);
	}
	return 1;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_close(lua_State *L)
{
	TEKVisual *vis = luaL_checkudata(L, 1, TEK_LIB_VISUAL_CLASSNAME);

	TDBPRINTF(TDB_TRACE,("visual %08x closing\n", vis));

	TFree(vis->vis_Drawdata.pens);
	vis->vis_Drawdata.pens = TNULL;

	TFree(vis->vis_Drawdata.points);
	vis->vis_Drawdata.points = TNULL;

	TFree(vis->vis_RectBuffer);
	vis->vis_RectBuffer = TNULL;

	if (vis->vis_refBase >= 0)
	{
		lua_getmetatable(L, 1);
		luaL_unref(L, -1, vis->vis_refBase);
		vis->vis_refBase = -1;
		lua_pop(L, 1);
		TDBPRINTF(TDB_TRACE,("visual %08x unref'd\n", vis));
	}

	if (vis->vis_Visual)
	{
		TVisualClose(vis->vis_Base, vis->vis_Visual);
		vis->vis_Visual = TNULL;
		TDBPRINTF(TDB_INFO,("visual instance %08x closed\n", vis));
	}

	if (vis->vis_isBase)
	{
		if (vis->vis_Base)
		{
			TVisualCloseFont(vis->vis_Base, vis->vis_Font);
			TCloseModule(vis->vis_Base);
		}
		if (vis->vis_TimeBase)
		{
			TFreeTimeRequest(vis->vis_TimeRequest);
			TCloseModule(vis->vis_TimeBase);
		}
		/* collected visual base; remove TEKlib module: */
		TRemModules((struct TModInitNode *) &im_visual, 0);
		TDBPRINTF(TDB_INFO,("visual module removed\n"));
	}

	return 0;
}

/*****************************************************************************/

int luaopen_tek_lib_visual(lua_State *L)
{
	TEKVisual *vis;
	TAPTR exec;

	/* require "tek.lib.display.x11": */
	lua_getglobal(L, "require");
	/* s: "require" */
	lua_pushliteral(L, "tek.lib.display." DISPLAY_DRIVER);
	/* s: "require", "tek.lib.display.x11" */
	lua_call(L, 1, 1);
	/* s: displaytab */

	/* require "tek.lib.exec": */
	lua_getglobal(L, "require");
	/* s: displaytab, "require" */
	lua_pushliteral(L, "tek.lib.exec");
	/* s: displaytab, "require", "tek.lib.exec" */
	lua_call(L, 1, 1);
	/* s: displaytab, exectab */
	lua_getfield(L, -1, "base");
	/* s: displaytab, exectab, execbase */
	exec = *(TAPTR *) lua_touserdata(L, -1);

	/* register functions: */
	luaL_register(L, "tek.lib.visual", libfuncs);
	/* s: displaytab, exectab, execbase, vistab */

	/* create userdata: */
	vis = lua_newuserdata(L, sizeof(TEKVisual));
	memset(vis, 0, sizeof(TEKVisual));
	/* s: displaytab, exectab, execbase, vistab, visbase */

	vis->vis_Base = TNULL;
	vis->vis_ExecBase = exec;
	vis->vis_Visual = TNULL;
	vis->vis_refBase = -1;
	vis->vis_isBase = TTRUE;

	/* register base: */
	lua_pushvalue(L, -1);
	/* s: displaytab, exectab, execbase, vistab, visbase, visbase */
	lua_setfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	/* s: displaytab, exectab, execbase, vistab, visbase */

	/* create metatable for userdata, register methods: */
	luaL_newmetatable(L, TEK_LIB_VISUAL_CLASSNAME);
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta */
	lua_pushvalue(L, -1);
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta, vismeta */
	lua_setfield(L, -2, "__index");
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta */
	luaL_register(L, NULL, classmethods);
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta */
	lua_setmetatable(L, -2);
	/* s: displaytab, exectab, execbase, vistab, visbase */

	/* place exec reference in metatable: */
	lua_getmetatable(L, -1);
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta */
	lua_pushvalue(L, -4);
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta, execbase */
	luaL_ref(L, -2); /* index returned is always 1 */
	/* s: displaytab, exectab, execbase, vistab, visbase, vismeta */
 	lua_pop(L, 6);

	/* prepare font metatable and store reference in metatable: */
	luaL_newmetatable(L, TEK_LIB_VISUALFONT_CLASSNAME);
	/* s: fontmeta */
	lua_pushvalue(L, -1);
	/* s: fontmeta, fontmeta */
	lua_setfield(L, -2, "__index");
	/* s: fontmeta */
	luaL_register(L, NULL, fontmethods);
	lua_pop(L, 1);

	/* Add visual module to TEKlib's internal module list: */
	TAddModules((struct TModInitNode *) &im_visual, 0);

	for (;;)
	{
		TTAGITEM ftags[2];
		TTAGITEM dtags[2];

		/* Open time module: */
		vis->vis_TimeBase = TOpenModule("time", 0, TNULL);
		if (vis->vis_TimeBase == TNULL) break;

		/* Create a timerequest: */
		vis->vis_TimeRequest = TAllocTimeRequest(TNULL);
		if (vis->vis_TimeRequest == TNULL) break;

		/* Open the Visual module: */
		vis->vis_Base = TExecOpenModule(exec, "visual", 0, TNULL);
		if (vis->vis_Base == TNULL) break;

		/* Open a display: */
		dtags[0].tti_Tag = TVisual_DisplayName;
		dtags[0].tti_Value = (TTAG) "display_" DISPLAY_DRIVER;
		dtags[1].tti_Tag = TTAG_DONE;
		vis->vis_Display = TVisualOpenDisplay(vis->vis_Base, dtags);
		if (vis->vis_Display == TNULL) break;

		/* Open default font: */
		dtags[0].tti_Tag = TVisual_Display;
		dtags[0].tti_Value = (TTAG) vis->vis_Display;
		dtags[1].tti_Tag = TTAG_DONE;
		vis->vis_Font = TVisualOpenFont(vis->vis_Base, dtags);
		if (vis->vis_Font == TNULL) break;

		ftags[0].tti_Tag = TVisual_FontHeight;
		ftags[0].tti_Value = (TTAG) &vis->vis_FontHeight;
		ftags[1].tti_Tag = TTAG_DONE;
		TVisualGetFontAttrs(vis->vis_Base, vis->vis_Font, ftags);

		/* success: */
		return 0;
	}

	luaL_error(L, "Visual initialization failure");
	return 0;
}
