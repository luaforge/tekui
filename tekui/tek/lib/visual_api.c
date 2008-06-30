
#include "visual_lua.h"

static TAPTR checkinstptr(lua_State *L, int n, const char *classname)
{
	TAPTR p = luaL_checkudata(L, n, classname);
	if (p == TNULL) luaL_argerror(L, n, "Closed handle");
	return p;
}

#define getvisptr(L, n) luaL_checkudata(L, n, TEK_LIB_VISUAL_CLASSNAME)
#define getpenptr(L, n) luaL_checkudata(L, n, TEK_LIB_VISUALPEN_CLASSNAME)
#define checkvisptr(L, n) checkinstptr(L, n, TEK_LIB_VISUAL_CLASSNAME)
#define checkpenptr(L, n) checkinstptr(L, n, TEK_LIB_VISUALPEN_CLASSNAME)
#define checkfontptr(L, n) checkinstptr(L, n, TEK_LIB_VISUALFONT_CLASSNAME)

/*****************************************************************************/
/*
**	visual_wait(table[, numv])
**	Waits for one or a set of visuals. Places true in the table for
**	each visual that has messages pending. If specified, no more than
**	numv entries in the table will be processed. Alternatively, accepts
**	a single visual as its sole argument.
*/

LOCAL LUACFUNC TINT
tek_lib_visual_wait(lua_State *L)
{
	TEKVisual *vis;
	TUINT sig = 0;
	int i, n;

	if (lua_istable(L, 1))
	{
		n = lua_isnone(L, 2) ? 32 : luaL_checknumber(L, 2);
		for (i = 1; i <= n; ++i)
		{
			lua_rawgeti(L, 1, i);
			if (lua_isnil(L, -1) || !lua_isuserdata(L, -1))
			{
				n = i - 1;
				lua_pop(L, 1);
				break;
			}
			vis = checkvisptr(L, -1);
			sig |= TGetPortSignal(TVisualGetPort(vis->vis_Visual));
			lua_pop(L, 1);
		}
	}
	else
	{
		vis = checkvisptr(L, 1);
		sig = TGetPortSignal(TVisualGetPort(vis->vis_Visual));
		n = 1;
	}

	if (sig)
	{
		sig = TWait(sig);
		if (lua_istable(L, 1))
		{
			for (i = 1; i <= n; ++i)
			{
				lua_rawgeti(L, 1, i);
				vis = lua_touserdata(L, -1);
				lua_pushboolean(L,
					sig & TGetPortSignal(TVisualGetPort(vis->vis_Visual)));
				lua_rawseti(L, 1, i);
				lua_pop(L, 1);
			}
		}
	}

	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_sleep(lua_State *L)
{
	TDOUBLE sec = luaL_checknumber(L, 1);
	TEKVisual *vis;
	TTIME dt;

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	lua_pop(L, 1);

	dt.ttm_Sec = sec;
	dt.ttm_USec = (sec - dt.ttm_Sec) * 1000000;

	TDelay(vis->vis_TimeRequest, &dt);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_gettime(lua_State *L)
{
	TEKVisual *vis;
	TTIME dt;
	lua_Number t;

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	lua_pop(L, 1);

	TQueryTime(vis->vis_TimeRequest, &dt);

	t = dt.ttm_USec;
	t *= 0.000001;
	t += dt.ttm_Sec;

	lua_pushnumber(L, t);

	return 1;
}

/*****************************************************************************/
/*
**	openfont(name, pxsize)
*/

LOCAL LUACFUNC TINT
tek_lib_visual_openfont(lua_State *L)
{
	TTAGITEM ftags[5], *tp = ftags;
	TEKVisual *vis;
	TEKFont *font;
	TSTRPTR name = (TSTRPTR) luaL_optstring(L, 1, "");
	TINT size = luaL_optnumber(L, 2, -1);

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (name && name[0] != 0)
	{
		tp->tti_Tag = TVisual_FontName;
		tp++->tti_Value = (TTAG) name;
	}

	if (size > 0)
	{
		tp->tti_Tag = TVisual_FontPxSize;
		tp++->tti_Value = (TTAG) size;
	}

	tp->tti_Tag = TVisual_Display;
	tp++->tti_Value = (TTAG) vis->vis_Display;

	tp->tti_Tag = TTAG_DONE;

	/* reserve userdata for a collectable object: */
	font = lua_newuserdata(L, sizeof(TEKFont));
	/* s: fontdata */
	font->font_Font = TVisualOpenFont(vis->vis_Base, ftags);
	if (font->font_Font)
	{
		font->font_VisBase = vis->vis_Base;

		ftags[0].tti_Tag = TVisual_FontHeight;
		ftags[0].tti_Value = (TTAG) &font->font_Height;
		ftags[1].tti_Tag = TVisual_FontUlPosition;
		ftags[1].tti_Value = (TTAG) &font->font_UlPosition;
		ftags[2].tti_Tag = TVisual_FontUlThickness;
		ftags[2].tti_Value = (TTAG) &font->font_UlThickness;
		ftags[3].tti_Tag = TTAG_DONE;
		TVisualGetFontAttrs(vis->vis_Base, font->font_Font, ftags);

		/* attach class metatable to userdata object: */
		luaL_newmetatable(L, TEK_LIB_VISUALFONT_CLASSNAME);
		/* s: fontdata, meta */
		lua_setmetatable(L, -2);
		/* s: fontdata */
		lua_pushinteger(L, font->font_Height);
		/* s: fontdata, height */
		return 2;
	}

	lua_pop(L, 1);
	lua_pushnil(L);
	return 1;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_closefont(lua_State *L)
{
	TEKFont *font = luaL_checkudata(L, 1, TEK_LIB_VISUALFONT_CLASSNAME);
	if (font->font_Font)
	{
		TVisualCloseFont(font->font_VisBase, font->font_Font);
		font->font_Font = TNULL;
	}
	return 0;
}

/*****************************************************************************/
/*
**	return width, height of the specified font and text
*/

LOCAL LUACFUNC TINT
tek_lib_visual_textsize_font(lua_State *L)
{
	TEKFont *font = checkfontptr(L, 1);
	TSTRPTR s = (TSTRPTR) luaL_checkstring(L, 2);
	lua_pushinteger(L,
		TVisualTextSize(font->font_VisBase, font->font_Font, s));
	lua_pushinteger(L, font->font_Height);
	return 2;
}

/*****************************************************************************/
/*
**	set font attributes in passed (or newly created) table
*/

LOCAL LUACFUNC TINT
tek_lib_visual_getfontattrs(lua_State *L)
{
	TEKFont *font = checkfontptr(L, 1);
	if (lua_type(L, 2) == LUA_TTABLE)
		lua_pushvalue(L, 2);
	else
		lua_newtable(L);

	lua_pushinteger(L, font->font_Height);
	lua_setfield(L, -2, "Height");
	lua_pushinteger(L, font->font_Height - font->font_UlPosition);
	lua_setfield(L, -2, "UlPosition");
	lua_pushinteger(L, font->font_UlThickness);
	lua_setfield(L, -2, "UlThickness");

	return 1;
}

/*****************************************************************************/

static int c_upper(c)
{
	if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
	return c;
}

static int
strcmp_nocase(TSTRPTR s1, TSTRPTR s2)
{
	TINT c1, c2;
	do
	{
		c1 = c_upper(*s1++);
		c2 = c_upper(*s2++);
		if (!c1 || !c2) break;
	} while (c1 == c2);
	return c1 - c2;
}

static const
struct tek_lib_visual_itypes { TUINT mask; TSTRPTR name; } itypes[] =
{
	{ TITYPE_CLOSE, "close" },
	{ TITYPE_FOCUS, "focus" },
	{ TITYPE_NEWSIZE, "newsize" },
	{ TITYPE_REFRESH, "refresh" },
	{ TITYPE_MOUSEOVER, "mouseover" },
	{ TITYPE_KEYDOWN, "keydown" },
	{ TITYPE_KEYUP, "keyup" },
	{ TITYPE_MOUSEMOVE, "mousemove" },
	{ TITYPE_MOUSEBUTTON, "mousebutton" },
	{ TITYPE_INTERVAL, "interval" },
	/* { TITYPE_ALL, "all" }, */
	{ 0, TNULL }
};

/*****************************************************************************/

static TUINT
tek_lib_visual_getinputmask(lua_State *L, TEKVisual *vis)
{
	TUINT i, sig = 0, narg = lua_gettop(L) - 1;
	for (i = 0; i < narg; ++i)
	{
 		const struct tek_lib_visual_itypes *fp = itypes;
		TUINT newsig = 0;
		while (fp->mask)
		{
			if (strcmp_nocase(fp->name,
 				(TSTRPTR) luaL_checkstring(L, 2 + i)) == 0)
			{
				newsig = fp->mask;
				break;
			}
			fp++;
		}
		if (newsig == 0)
			luaL_argerror(L, i + 2, "Unknown keyword");
		sig |= newsig;
	}
	return sig;
}

LOCAL LUACFUNC TINT
tek_lib_visual_setinput(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TVisualSetInput(vis->vis_Visual, 0, tek_lib_visual_getinputmask(L, vis));
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_clearinput(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TVisualSetInput(vis->vis_Visual, tek_lib_visual_getinputmask(L, vis), 0);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_getmsg(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TIMSG *imsg = (TIMSG *) TGetMsg(TVisualGetPort(vis->vis_Visual));
	if (imsg)
	{
		const struct tek_lib_visual_itypes *fp = itypes;

		while (fp->mask)
		{
			if (fp->mask == imsg->timsg_Type)
				break;
			fp++;
		}

		if (fp->name == TNULL)
			TDBPRINTF(TDB_ERROR,("Unknown message at port\n"));

		lua_newtable(L);
		lua_pushnumber(L, (lua_Number) imsg->timsg_TimeStamp.ttm_Sec * 1000 +
			imsg->timsg_TimeStamp.ttm_USec / 1000);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, imsg->timsg_Type);
		lua_rawseti(L, -2, 2);

		lua_pushnumber(L, imsg->timsg_Code);
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, imsg->timsg_MouseX);
		lua_rawseti(L, -2, 4);
		lua_pushnumber(L, imsg->timsg_MouseY);
		lua_rawseti(L, -2, 5);
		lua_pushnumber(L, imsg->timsg_Qualifier);
		lua_rawseti(L, -2, 6);

		/* extra information depending on event type: */
		switch (imsg->timsg_Type)
		{
			case TITYPE_REFRESH:
				lua_pushnumber(L, imsg->timsg_X);
				lua_rawseti(L, -2, 7);
				lua_pushnumber(L, imsg->timsg_Y);
				lua_rawseti(L, -2, 8);
				lua_pushnumber(L, imsg->timsg_X + imsg->timsg_Width - 1);
				lua_rawseti(L, -2, 9);
				lua_pushnumber(L, imsg->timsg_Y + imsg->timsg_Height - 1);
				lua_rawseti(L, -2, 10);
				break;
			case TITYPE_KEYUP:
			case TITYPE_KEYDOWN:
				/* UTF-8 representation of keycode: */
				lua_pushstring(L, (const char *) imsg->timsg_KeyCode);
				lua_rawseti(L, -2, 7);
				break;
		}

		TAckMsg(imsg);
		return 1;
	}
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_allocpen(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT r = luaL_checknumber(L, 2);
	TINT g = luaL_checknumber(L, 3);
	TINT b = luaL_checknumber(L, 4);
	TEKPen *pen = lua_newuserdata(L, sizeof(TEKPen));
	/* s: pendata */
	pen->pen_Pen = TVPEN_UNDEFINED;
	/* attach class metatable to userdata object: */
	luaL_newmetatable(L, TEK_LIB_VISUALPEN_CLASSNAME);
	/* s: pendata, meta */
	lua_setmetatable(L, -2);
	/* s: pendata */
	r = TCLAMP(0, r, 255);
	g = TCLAMP(0, g, 255);
	b = TCLAMP(0, b, 255);
	pen->pen_Pen = TVisualAllocPen(vis->vis_Visual, (r << 16) | (g << 8) | b);
	pen->pen_Visual = vis;
	return 1;
}

LOCAL LUACFUNC TINT
tek_lib_visual_freepen(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TEKPen *pen = getpenptr(L, 2);
	if (pen->pen_Pen != TVPEN_UNDEFINED)
	{
		if (vis != pen->pen_Visual)
			luaL_argerror(L, 2, "Pen not from visual");
		TVisualFreePen(vis->vis_Visual, pen->pen_Pen);
		pen->pen_Pen = TVPEN_UNDEFINED;
	}
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_rect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen = checkpenptr(L, 6);
	TVisualRect(vis->vis_Visual, x0, y0, x1 - x0 + 1, y1 - y0 + 1,
		pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_frect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen = checkpenptr(L, 6);
	TVisualFRect(vis->vis_Visual, x0, y0, x1 - x0 + 1, y1 - y0 + 1,
		pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_line(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen = checkpenptr(L, 6);
	TVisualLine(vis->vis_Visual, x0, y0, x1, y1, pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_plot(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TEKPen *pen = checkpenptr(L, 4);
	TVisualPlot(vis->vis_Visual, x0, y0, pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_text(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	size_t tlen;
	TSTRPTR text = (TSTRPTR) luaL_checklstring(L, 4, &tlen);
	TEKPen *fpen = checkpenptr(L, 5);
	TVPEN bpen = TVPEN_UNDEFINED;
	if (lua_isuserdata(L, 6))
		bpen = ((TEKPen *) checkpenptr(L, 6))->pen_Pen;
	TVisualText(vis->vis_Visual, x0, y0, text, tlen, fpen->pen_Pen, bpen);
	return 0;
}

/*****************************************************************************/
/*
** Layout of data table (L[2])
** ----------------------------
**	data =
**	{
**		Coords =
**		{
**			x1, y1,
**			x2, y2,
**			...
**		},
**
**		Primitives =
**		{
**			{
**				format code,
**				number of points,
**				Points = { index1, index2, index3 },
**				Pen = pentable_index,
**					OR
**				Pens = { pentable_index0, pentable_index1, ... }
**			},
**
**			{
**				...
**			}
**		},
**
**		MinMax = { mm1, mm2, mm3, mm4, },
**		Rect = { r1, r2, r3, r4},
**		PenTable = ...,
**	},
**
** Format Codes:
** ---------------
** 0x1000 Strip
** 0x2000 Fan
** 0x4000 Triangle
**
** 0x0000 pen
** 0x0001 pens
** 0x0002 color
** 0x0004 colors
**
*/

#define u_ld(b) ({ int __i = 0, __j = b; while (__j >>= 1) __i++; __i; })

static lua_Number
igetnumber(lua_State *L, TINT index, TINT n)
{
	lua_Number result;
	lua_rawgeti(L, index, n);
	result = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	return result;
}

static lua_Integer
igetinteger(lua_State *L, TINT index, TINT n)
{
	lua_Integer result;
	lua_rawgeti(L, index, n);
	result = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	return result;
}

static TEKPen*
getpen(lua_State *L, TINT index)
{
	TEKPen *pen;
	lua_rawget(L, index);
	pen = checkpenptr(L, -1);
	lua_pop(L, 1);
	return pen;
}

LOCAL LUACFUNC TINT
tek_lib_visual_drawimage(lua_State *L)
{
	TINT pidx;
	TINT i, j;
	TINT fmtcode;
	TINT primcount, nump;
	lua_Number rect[4], minmax[4];
	lua_Number xoffs, yoffs;
	lua_Number scalex, scaley;
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TTAGITEM tags[2];

	luaL_checktype(L, 2, LUA_TTABLE);

	/* get rect */
	lua_getfield(L, 2, "Rect");
	luaL_checktype(L, -1, LUA_TTABLE);

	/* s: rect */
	for (i = 0; i < 4; i++)
	{
		/* get s:rect[i+1] */
		rect[i] = igetnumber(L, -1, i+1);
	}
	lua_pop(L, 1);
	/* s: */

	/* get minmax */
	lua_getfield(L, 2, "MinMax");
	luaL_checktype(L, -1, LUA_TTABLE);

	/* s: minmax */
	for (i = 0; i < 4; i++)
	{
		/* get s:minmax[i+1] */
		minmax[i] = igetnumber(L, -1, i+1);
	}
	lua_pop(L, 1);
	/* s: */

	xoffs = minmax[0];
	yoffs = minmax[1];
	scalex = (rect[2] - rect[0]) * 65536 / (minmax[2] - xoffs);
	scaley = (rect[3] - rect[1]) * 65536 / (minmax[3] - yoffs);

	/* get coordinates */
	lua_getfield(L, 2, "Coords");
	luaL_checktype(L, -1, LUA_TTABLE);

	/* s: coords */
	lua_getfield(L, 2, "Primitives");
	luaL_checktype(L, -1, LUA_TTABLE);

	/* s: primitives, coords */
	primcount = lua_objlen(L, -1);

	for (i = 0; i < primcount; i++)
	{
		lua_rawgeti(L, -1, i+1);
		/* s: primitives[i+1], primitives, coords */

		/* get format code */
		fmtcode = igetinteger(L, -1, 1);

		/* get number of points */
		nump = igetinteger(L, -1, 2);

		/* (re-)allocate memory for points and pens */
		if (vis->vis_Drawdata.nump < nump)
		{
			TINT newsize = 1<<(u_ld((nump*2))+1);

			if (vis->vis_Drawdata.points)
				TFree(vis->vis_Drawdata.points);
			if (vis->vis_Drawdata.pens)
				TFree(vis->vis_Drawdata.pens);

			vis->vis_Drawdata.points = TAlloc(TNULL, newsize*sizeof(TINT));
			vis->vis_Drawdata.pens = TAlloc(TNULL, (newsize>>1)*sizeof(TVPEN));
			vis->vis_Drawdata.nump = newsize>>1;
		}

		if (!vis->vis_Drawdata.points || !vis->vis_Drawdata.pens)
		{
			lua_pop(L, 1);
			/* s: primitives, points */
			break;
		}

		/* get indices */
		lua_getfield(L, -1, "Points");
		luaL_checktype(L, -1, LUA_TTABLE);

		/* s: primitives[i+1].points, primitives[i+1], primitives, coords */
		for (j = 0; j < nump; j++)
		{
			TINT px, py;

			/* get s:primitives[i+1].points[j+1] */
			pidx = igetinteger(L, -1, j+1);

			/* get s:points[pidx*2-1] */
			px = igetnumber(L, -4, pidx*2-1);

			/* get s:points[pidx*2] */
			py = igetnumber(L, -4, pidx*2);

			vis->vis_Drawdata.points[j*2] = rect[0] +
				(px - xoffs) * scalex / 65536 + sx;
			vis->vis_Drawdata.points[j*2+1] = rect[3] -
				(py - yoffs) * scaley / 65536 + sy;
		}
		lua_pop(L, 1);
		/* s: primitives[i+1], primitives, coords */

		/* get pens/colors */
		if ((fmtcode & 0xf) < 0x2)
		{
			TEKPen *pen;

			lua_getfield(L, 2, "PenTable");
			luaL_checktype(L, -1, LUA_TTABLE);
			/* s: pentab, primitives[i+1], primitives, coords */

			if ((fmtcode & 0xf) == 0x1)
			{
				/* pens */

				lua_getfield(L, -2, "Pens");
				luaL_checktype(L, -1, LUA_TTABLE);
				/* s: primitives[i+1].pens, pentab, primitives[i+1], primitives, coords */
				for (j = 0; j < nump; j++)
				{
					lua_rawgeti(L, -1, j+1);
					/* s: primitives[i+1].pens[j+1], primitives[i+1].pens,
					      pentab, primitives[i+1], primitives, coords */

					/* get s:pentab[primitives[i+1].pens[j+1]] */
					pen = getpen(L, -3);

					vis->vis_Drawdata.pens[j] = pen->pen_Pen;
				}

				lua_pop(L, 1);
				/* s: pentab, primitives[i+1], primitives, coords */

				tags[0].tti_Tag = TVisual_PenArray;
				tags[0].tti_Value = (TTAG) vis->vis_Drawdata.pens;
				tags[1].tti_Tag = TTAG_DONE;
			}
			else
			{
				/* pen */

				lua_getfield(L, -2, "Pen");
				/* s: primitives[i+1].pen, pentab, primitives[i+1], primitives, coords */

				/* get s: pentab[primitives[i+1].pen] */
				pen = getpen(L, -2);

				tags[0].tti_Tag = TVisual_Pen;
				tags[0].tti_Value = (TTAG) pen->pen_Pen;
				tags[1].tti_Tag = TTAG_DONE;
			}

			lua_pop(L, 1);
			/* s: primitives[i+1], primitives, coords */
		}
		else
		{
			/* color / colors */

		}

		lua_pop(L, 1);
		/* s: primitives, coords */

		switch ((fmtcode & 0xf000))
		{
			case 0x1000: /* Strip */
			case 0x4000: /* Triangle */
				TVisualDrawStrip(vis->vis_Visual, vis->vis_Drawdata.points, nump, tags);
				break;
			case 0x2000: /* Fan */
				TVisualDrawFan(vis->vis_Visual, vis->vis_Drawdata.points, nump, tags);
				break;
			default:
				break;
		}

	} /* end for i < primcount */

	lua_pop(L, 2);
	/* s: */

	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_getattrs(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TTAGITEM tags[5];
	TINT pw, ph, x, y;

	tags[0].tti_Tag = TVisual_Width;
	tags[0].tti_Value = (TTAG) &pw;
	tags[1].tti_Tag = TVisual_Height;
	tags[1].tti_Value = (TTAG) &ph;
	tags[2].tti_Tag = TVisual_WinLeft;
	tags[2].tti_Value = (TTAG) &x;
	tags[3].tti_Tag = TVisual_WinTop;
	tags[3].tti_Value = (TTAG) &y;
	tags[4].tti_Tag = TTAG_DONE;

	TVisualGetAttrs(vis->vis_Visual, tags);

	lua_pushnumber(L, pw);
	lua_pushnumber(L, ph);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 4;
}

LOCAL LUACFUNC TINT
tek_lib_visual_setattrs(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TTAGITEM tags[7], *tp = tags;

	lua_getfield(L, 2, "MinWidth");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MinWidth;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MinHeight");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MinHeight;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MaxWidth");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MaxWidth;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MaxHeight");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MaxHeight;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	#if 0
	lua_getfield(L, 2, "GrabButton");
	if (lua_isnumber(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisualHost_GrabButton;
		tp->tti_Value = (TTAG) lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "GrabPointer");
	if (lua_isnumber(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisualHost_GrabPointer;
		tp->tti_Value = (TTAG) lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}
	#endif

	tp->tti_Tag = TTAG_DONE;
	lua_pushnumber(L, TVisualSetAttrs(vis->vis_Visual, tags));

	return 1;
}

/*****************************************************************************/
/*
**	textsize_visual: return text width, height using the current font
*/

LOCAL LUACFUNC TINT
tek_lib_visual_textsize_visual(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TSTRPTR s = (TSTRPTR) luaL_checkstring(L, 2);
	lua_pushinteger(L,
		TVisualTextSize(vis->vis_Base, vis->vis_Font, s));
	lua_pushinteger(L, vis->vis_FontHeight);
	return 2;
}

/*****************************************************************************/
/*
**	setfont(font): attach a font to a visual
*/

LOCAL LUACFUNC TINT
tek_lib_visual_setfont(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TEKFont *font = checkfontptr(L, 2);
	if (font->font_Font && vis->vis_Font != font->font_Font)
	{
		lua_getmetatable(L, 1);
		/* s: vismeta */

		if (vis->vis_refFont != -1)
		{
			/* unreference old current font: */
			luaL_unref(L, -1, vis->vis_refFont);
			vis->vis_refFont = -1;
		}

		TVisualSetFont(vis->vis_Visual, font->font_Font);
		vis->vis_Font = font->font_Font;
		vis->vis_FontHeight = font->font_Height;

		/* reference new font: */
		lua_pushvalue(L, 2);
		/* s: vismeta, font */
		vis->vis_refFont = luaL_ref(L, -2);
		/* s: vismeta */
		lua_pop(L, 1);
	}
	return 0;
}

/*****************************************************************************/

static TTAG hookfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	TEKVisual *vis = hook->thk_Data;
	TINT *rect = (TINT *) msg;
	TINT *newbuf = vis->vis_RectBuffer ?
		TExecRealloc(vis->vis_ExecBase, vis->vis_RectBuffer,
			(vis->vis_RectBufferNum + 4) * sizeof(TINT)) :
		TExecAlloc(vis->vis_ExecBase, TNULL, sizeof(TINT) * 4);

	if (newbuf)
	{
		vis->vis_RectBuffer = newbuf;
		newbuf += vis->vis_RectBufferNum;
		vis->vis_RectBufferNum += 4;
		newbuf[0] = rect[0];
		newbuf[1] = rect[1];
		newbuf[2] = rect[2];
		newbuf[3] = rect[3];
	}

	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_copyarea(lua_State *L)
{
	TTAGITEM tags[2], *tp = TNULL;
	struct THook hook;

	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x = luaL_checkinteger(L, 2) + sx;
	TINT y = luaL_checkinteger(L, 3) + sy;
	TINT w = luaL_checkinteger(L, 4);
	TINT h = luaL_checkinteger(L, 5);
	TINT dx = luaL_checkinteger(L, 6) + sx;
	TINT dy = luaL_checkinteger(L, 7) + sy;

	if (lua_istable(L, 8))
	{
		vis->vis_RectBuffer = TNULL;
		vis->vis_RectBufferNum = 0;
		TInitHook(&hook, hookfunc, vis);
		tags[0].tti_Tag = TVisual_ExposeHook;
		tags[0].tti_Value = (TTAG) &hook;
		tags[1].tti_Tag = TTAG_DONE;
		tp = tags;
	}

	TVisualCopyArea(vis->vis_Visual, x, y, w, h, dx, dy, tp);

	if (tp)
	{
		TINT i;
		for (i = 0; i < vis->vis_RectBufferNum; ++i)
		{
			lua_pushinteger(L, vis->vis_RectBuffer[i]);
			lua_rawseti(L, 8, i + 1);
		}
		TExecFree(vis->vis_ExecBase, vis->vis_RectBuffer);
		vis->vis_RectBuffer = TNULL;
	}

	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setcliprect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT x = luaL_checkinteger(L, 2);
	TINT y = luaL_checkinteger(L, 3);
	TINT w = luaL_checkinteger(L, 4);
	TINT h = luaL_checkinteger(L, 5);
	TVisualSetClipRect(vis->vis_Visual, x, y, w, h, TNULL);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_unsetcliprect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TVisualUnsetClipRect(vis->vis_Visual);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setshift(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = luaL_checkinteger(L, 2);
	TINT sy = luaL_checkinteger(L, 3);
	vis->vis_ShiftX += sx;
	vis->vis_ShiftY += sy;
	return 0;
}

/*****************************************************************************/
/*
**	drawrgb(visual, x0, y0, table, width, height, pixwidth, pixheight)
*/

LOCAL LUACFUNC TINT
tek_lib_visual_drawrgb(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT w = luaL_checkinteger(L, 5);
	TINT h = luaL_checkinteger(L, 6);
	TINT pw = luaL_checkinteger(L, 7);
	TINT ph = luaL_checkinteger(L, 8);
	TUINT *buf;
	TINT bw = w * pw;
	TINT bh = h * ph;

	luaL_checktype(L, 4, LUA_TTABLE);

	buf = TExecAlloc(vis->vis_ExecBase, TNULL, bw * bh * sizeof(TUINT));
	if (buf)
	{
		TUINT rgb;
		TUINT *p = buf;
		TINT i = 0;
		TINT xx, yy, x, y;

		for (y = 0; y < h; ++y)
		{
			TUINT *lp = p;
			for (x = 0; x < w; ++x)
			{
				lua_rawgeti(L, 4, i++);
				rgb = lua_tointeger(L, -1);
				lua_pop(L, 1);
				for (xx = 0; xx < pw; ++xx)
					*p++ = rgb;
			}

			for (yy = 0; yy < ph - 1; ++yy)
			{
				TExecCopyMem(vis->vis_ExecBase, lp, p, bw * sizeof(TUINT));
				p += bw;
			}
		}

		TVisualDrawBuffer(vis->vis_Visual, x0, y0, buf, bw, bh, bw, TNULL);

		TExecFree(vis->vis_ExecBase, buf);
	}

	return 0;
}
