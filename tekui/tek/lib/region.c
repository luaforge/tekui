
/*
**	tek.lib.region - Management of rectangular regions
**	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
**	See copyright notice in COPYRIGHT
*/

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>

#include <tek/debug.h>
#include <tek/teklib.h>
#include <tek/proto/exec.h>

#define TEK_CLASS_UI_REGION_NAME "tek.lib.region*"

#define MERGE_RECTS	5

/*****************************************************************************/

struct Region
{
	TAPTR rg_ExecBase;
	struct TList rg_List;
}; /* 16 bytes on 32bit arch */

struct RectNode
{
	struct TNode rn_Node;
	TINT rn_Rect[4];
	TUINT rn_Flags;
	TUINT rn_Reserved;
}; /* 32 bytes on 32bit arch */

/*****************************************************************************/

#define OVERLAP(d0, d1, d2, d3, s0, s1, s2, s3) \
((s2) >= (d0) && (s0) <= (d2) && (s3) >= (d1) && (s1) <= (d3))

#define OVERLAPRECT(d, s) \
OVERLAP((d)[0], (d)[1], (d)[2], (d)[3], (s)[0], (s)[1], (s)[2], (s)[3])

static int lib_overlap(lua_State *L)
{
	TINT d0 = luaL_checkinteger(L, 1);
	TINT d1 = luaL_checkinteger(L, 2);
	TINT d2 = luaL_checkinteger(L, 3);
	TINT d3 = luaL_checkinteger(L, 4);
	TINT s0 = luaL_checkinteger(L, 5);
	TINT s1 = luaL_checkinteger(L, 6);
	TINT s2 = luaL_checkinteger(L, 7);
	TINT s3 = luaL_checkinteger(L, 8);

	if (OVERLAP(d0, d1, d2, d3, s0, s1, s2, s3))
	{
		lua_pushinteger(L, TMAX(s0, d0));
		lua_pushinteger(L, TMAX(s1, d1));
		lua_pushinteger(L, TMIN(s2, d2));
		lua_pushinteger(L, TMIN(s3, d3));
		return 4;
	}
	return 0;
}

/*****************************************************************************/

static void relinklist(struct TList *dlist, struct TList *slist)
{
	if (!TISLISTEMPTY(slist))
	{
		struct TNode *first = slist->tlh_Head;
		struct TNode *last = slist->tlh_TailPred;
		first->tln_Pred = (struct TNode *) dlist;
		last->tln_Succ = (struct TNode *) &dlist->tlh_Tail;
		dlist->tlh_Head = first;
		dlist->tlh_TailPred = last;
	}
}

static struct RectNode *allocrectnode(TAPTR exec,
	TINT x0, TINT y0, TINT x1, TINT y1)
{
	struct RectNode *rn = TExecAlloc(exec, TNULL, sizeof(struct RectNode));
	if (rn)
	{
		TDBPRINTF(TDB_TRACE,("allocrect: %08x\n", rn));
		rn->rn_Rect[0] = x0;
		rn->rn_Rect[1] = y0;
		rn->rn_Rect[2] = x1;
		rn->rn_Rect[3] = y1;
		rn->rn_Flags = 0;
	}
	return rn;
}

static void freelist(TAPTR exec, struct TList *list)
{
	struct TNode *next, *node = list->tlh_Head;
	for (; (next = node->tln_Succ); node = next)
	{
		TREMOVE(node);
		TExecFree(exec, node);
	}
}

/*****************************************************************************/

static TBOOL insertrect(TAPTR exec, struct TList *list,
	TINT s0, TINT s1, TINT s2, TINT s3)
{
	struct TNode *temp, *next, *node = list->tlh_Head;
	struct RectNode *rn;
	int i;

	#if defined(MERGE_RECTS)
	for (i = 0; i < MERGE_RECTS && (next = node->tln_Succ); node = next, ++i)
	{
		rn = (struct RectNode *) node;
		if (rn->rn_Rect[1] == s1 && rn->rn_Rect[3] == s3)
		{
			if (rn->rn_Rect[2] + 1 == s0)
			{
				rn->rn_Rect[2] = s2;
				return TTRUE;
			}
			else if (rn->rn_Rect[0] == s2 + 1)
			{
				rn->rn_Rect[0] = s0;
				return TTRUE;
			}
		}
		else if (rn->rn_Rect[0] == s0 && rn->rn_Rect[2] == s2)
		{
			if (rn->rn_Rect[3] + 1 == s1)
			{
				rn->rn_Rect[3] = s3;
				return TTRUE;
			}
			else if (rn->rn_Rect[1] == s3 + 1)
			{
				rn->rn_Rect[1] = s1;
				return TTRUE;
			}
		}
	}
	#endif

	rn = allocrectnode(exec, s0, s1, s2, s3);
	if (rn)
	{
		TADDHEAD(list, &rn->rn_Node, temp);
		return TTRUE;
	}

	return TFALSE;
}

static TBOOL cutrect(TAPTR exec, struct TList *list, const TINT d[4],
	const TINT s[4])
{
	TINT d0 = d[0];
	TINT d1 = d[1];
	TINT d2 = d[2];
	TINT d3 = d[3];

	if (!OVERLAPRECT(d, s))
		return insertrect(exec, list, d[0], d[1], d[2], d[3]);

	for (;;)
	{
		if (d0 < s[0])
		{
			if (!insertrect(exec, list, d0, d1, s[0] - 1, d3))
				break;
			d0 = s[0];
		}

		if (d1 < s[1])
		{
			if (!insertrect(exec, list, d0, d1, d2, s[1] - 1))
				break;
			d1 = s[1];
		}

		if (d2 > s[2])
		{
			if (!insertrect(exec, list, s[2] + 1, d1, d2, d3))
				break;
			d2 = s[2];
		}

		if (d3 > s[3])
		{
			if (!insertrect(exec, list, d0, s[3] + 1, d2, d3))
				break;
		}

		return TTRUE;

	}
	return TFALSE;
}

static TBOOL cutrectlist(TAPTR exec, struct TList *inlist,
	struct TList *outlist, const TINT s[4])
{
	TBOOL success = TTRUE;
	struct TNode *next, *node = inlist->tlh_Head;
	for (; success && (next = node->tln_Succ); node = next)
	{
		struct RectNode *rn = (struct RectNode *) node;
		struct TList temp;

		TINITLIST(&temp);

		success = cutrect(exec, &temp, rn->rn_Rect, s);
		if (success)
		{
			struct TNode *next2, *node2 = temp.tlh_Head;
			for (; success && (next2 = node2->tln_Succ); node2 = next2)
			{
				struct RectNode *rn2 = (struct RectNode *) node2;
				success = insertrect(exec, outlist, rn2->rn_Rect[0],
					rn2->rn_Rect[1], rn2->rn_Rect[2], rn2->rn_Rect[3]);
				/* note that if unsuccessful, outlist is unusable as well */
			}
		}

		freelist(exec, &temp);
	}
	return success;
}

static TBOOL orrect(TAPTR exec, struct TList *list, TINT s[4])
{
	struct TList temp;
	TINITLIST(&temp);
	if (cutrectlist(exec, list, &temp, s))
	{
		if (insertrect(exec, &temp, s[0], s[1], s[2], s[3]))
		{
			freelist(exec, list);
			relinklist(list, &temp);
			return TTRUE;
		}
	}
	freelist(exec, &temp);
	return TFALSE;
}

/*****************************************************************************/

static int lib_new(lua_State *L)
{
	TINT x0 = luaL_checkinteger(L, 1);
	TINT y0 = luaL_checkinteger(L, 2);
	TINT x1 = luaL_checkinteger(L, 3);
	TINT y1 = luaL_checkinteger(L, 4);

	struct Region *region = lua_newuserdata(L, sizeof(struct Region));
	/* s: udata */

	TINITLIST(&region->rg_List);
	region->rg_ExecBase = TNULL;

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_CLASS_UI_REGION_NAME);
	/* s: udata, metatable */
	lua_rawgeti(L, -1, 1);
	/* s: udata, metatable, execbase */

	region->rg_ExecBase = *(TAPTR *) lua_touserdata(L, -1);

	lua_pop(L, 1);
	/* s: udata, metatable */
	lua_setmetatable(L, -2);
	/* s: udata */

	if (insertrect(region->rg_ExecBase, &region->rg_List,
		x0, y0, x1, y1) == TFALSE)
		luaL_error(L, "out of memory");

	return 1;
}

static int region_set(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);
	TINT x0 = luaL_checkinteger(L, 2);
	TINT y0 = luaL_checkinteger(L, 3);
	TINT x1 = luaL_checkinteger(L, 4);
	TINT y1 = luaL_checkinteger(L, 5);

	freelist(region->rg_ExecBase, &region->rg_List);
	if (insertrect(region->rg_ExecBase, &region->rg_List,
		x0, y0, x1, y1) == TFALSE)
		luaL_error(L, "out of memory");

	return 0;
}

static int region_collect(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);

	if (region->rg_ExecBase)
	{
		TDBPRINTF(TDB_TRACE,("region collecting...\n"));

		freelist(region->rg_ExecBase, &region->rg_List);
		region->rg_ExecBase = TNULL;

	#if 0
		/* release reference to ExecBase in metatable: */
		lua_getfield(L, LUA_REGISTRYINDEX, TEK_CLASS_UI_REGION_NAME);
		/* s: metatable */
		luaL_unref(L, -1, 1);
		lua_pop(L, 1);
		/* s: */
	#endif
	}

	return 0;
}

static int region_iterate(lua_State *L)
{
	struct TNode *node = lua_touserdata(L, 2);
	struct TNode *next = node->tln_Succ;

	if (next)
	{
		TDBPRINTF(TDB_TRACE,("in iterate: %08x\n", node));
		lua_pushlightuserdata(L, next);
		lua_pushlightuserdata(L, node);
		return 2;
	}

	return 0;
}

static int region_getrects(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);

	lua_pushcfunction(L, region_iterate);
	/* s: func */
	lua_pushvalue(L, 1);
	/* s: func, udata */
	lua_pushlightuserdata(L, region->rg_List.tlh_Head);
	/* s: func, udata, headnode */

	return 3;
}

static int region_getrect(lua_State *L)
{
	struct RectNode *rn = lua_touserdata(L, 2);
	if (rn)
	{
		lua_pushinteger(L, rn->rn_Rect[0]);
		lua_pushinteger(L, rn->rn_Rect[1]);
		lua_pushinteger(L, rn->rn_Rect[2]);
		lua_pushinteger(L, rn->rn_Rect[3]);
		return 4;
	}
	return 0;
}

static int region_getflags(lua_State *L)
{
	struct RectNode *rn = lua_touserdata(L, 2);
	if (rn)
	{
		lua_pushinteger(L, rn->rn_Flags);
		return 1;
	}
	return 0;
}

static int region_setflags(lua_State *L)
{
	struct RectNode *rn = lua_touserdata(L, 2);
	if (rn)
		rn->rn_Flags = luaL_checkinteger(L, 3);
	return 0;
}

static int region_orrect(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);
	TINT s[4];

	s[0] = luaL_checkinteger(L, 2);
	s[1] = luaL_checkinteger(L, 3);
	s[2] = luaL_checkinteger(L, 4);
	s[3] = luaL_checkinteger(L, 5);

	if (orrect(region->rg_ExecBase, &region->rg_List, s) == TFALSE)
		luaL_error(L, "out of memory");

	return 0;
}

static TBOOL orregion(struct Region *region, struct TList *list)
{
	TBOOL success = TTRUE;
	struct TNode *next, *node = list->tlh_Head;
	for (; success && (next = node->tln_Succ); node = next)
	{
		struct RectNode *rn = (struct RectNode *) node;
		success = orrect(region->rg_ExecBase, &region->rg_List, rn->rn_Rect);
	}
	return success;
}

static int region_xorrect(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);
	TAPTR exec = region->rg_ExecBase;
	struct TNode *next, *node;
	TBOOL success;
	struct TList r1, r2;
	TINT s[4];

	s[0] = luaL_checkinteger(L, 2);
	s[1] = luaL_checkinteger(L, 3);
	s[2] = luaL_checkinteger(L, 4);
	s[3] = luaL_checkinteger(L, 5);

	TINITLIST(&r1);
	TINITLIST(&r2);

	success = insertrect(exec, &r2, s[0], s[1], s[2], s[3]);

	node = region->rg_List.tlh_Head;
	for (; success && (next = node->tln_Succ); node = next)
	{
		struct TNode *next2, *node2;
		struct RectNode *rn = (struct RectNode *) node;
		struct TList temp;

		TINITLIST(&temp);
		success = cutrect(exec, &temp, rn->rn_Rect, s);

		node2 = temp.tlh_Head;
		for (; success && (next2 = node2->tln_Succ); node2 = next2)
		{
			struct RectNode *rn2 = (struct RectNode *) node2;
			success = insertrect(exec, &r1, rn2->rn_Rect[0],
				rn2->rn_Rect[1], rn2->rn_Rect[2], rn2->rn_Rect[3]);
		}

		freelist(exec, &temp);
		TINITLIST(&temp);

		if (success)
		{
			success = cutrectlist(exec, &r2, &temp, rn->rn_Rect);
			freelist(exec, &r2);
			relinklist(&r2, &temp);
		}
	}

	if (success)
	{
		freelist(exec, &region->rg_List);
		relinklist(&region->rg_List, &r1);
		orregion(region, &r2);
		freelist(exec, &r2);
	}
	else
	{
		freelist(exec, &r1);
		freelist(exec, &r2);
		luaL_error(L, "out of memory");
	}

	return 0;
}

static int region_subrect(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);
	TAPTR exec = region->rg_ExecBase;
	struct TNode *next, *node;
	TBOOL success;
	struct TList r1;
	TINT s[4];

	s[0] = luaL_checkinteger(L, 2);
	s[1] = luaL_checkinteger(L, 3);
	s[2] = luaL_checkinteger(L, 4);
	s[3] = luaL_checkinteger(L, 5);

	TINITLIST(&r1);

	success = TTRUE;
	node = region->rg_List.tlh_Head;
	for (; success && (next = node->tln_Succ); node = next)
	{
		struct TNode *next2, *node2;
		struct RectNode *rn = (struct RectNode *) node;
		struct TList temp;

		TINITLIST(&temp);
		success = cutrect(exec, &temp, rn->rn_Rect, s);

		node2 = temp.tlh_Head;
		for (; success && (next2 = node2->tln_Succ); node2 = next2)
		{
			struct RectNode *rn2 = (struct RectNode *) node2;
			success = insertrect(exec, &r1, rn2->rn_Rect[0],
				rn2->rn_Rect[1], rn2->rn_Rect[2], rn2->rn_Rect[3]);
		}

		freelist(exec, &temp);
	}

	if (success)
	{
		freelist(exec, &region->rg_List);
		relinklist(&region->rg_List, &r1);
	}
	else
	{
		freelist(exec, &r1);
		luaL_error(L, "out of memory");
	}

	return 0;
}

static int region_overlaprect(lua_State *L)
{
	struct RectNode *rn = lua_touserdata(L, 2);

	TINT s0 = lua_tointeger(L, 3);
	TINT s1 = lua_tointeger(L, 4);
	TINT s2 = lua_tointeger(L, 5);
	TINT s3 = lua_tointeger(L, 6);
	TINT d0 = rn->rn_Rect[0];
	TINT d1 = rn->rn_Rect[1];
	TINT d2 = rn->rn_Rect[2];
	TINT d3 = rn->rn_Rect[3];

	if (OVERLAP(d0, d1, d2, d3, s0, s1, s2, s3))
	{
		lua_pushinteger(L, TMAX(s0, d0));
		lua_pushinteger(L, TMAX(s1, d1));
		lua_pushinteger(L, TMIN(s2, d2));
		lua_pushinteger(L, TMIN(s3, d3));
		return 4;
	}

	return 0;
}

static int region_checkoverlap(lua_State *L)
{
	struct Region *region = luaL_checkudata(L, 1, TEK_CLASS_UI_REGION_NAME);
	struct TNode *next, *node;
	TINT s[4];

	s[0] = luaL_checkinteger(L, 2);
	s[1] = luaL_checkinteger(L, 3);
	s[2] = luaL_checkinteger(L, 4);
	s[3] = luaL_checkinteger(L, 5);

	node = region->rg_List.tlh_Head;
	for (; (next = node->tln_Succ); node = next)
	{
		struct RectNode *rn = (struct RectNode *) node;
		if (OVERLAPRECT(rn->rn_Rect, s))
		{
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	lua_pushboolean(L, 1);
	return 1;
}

/*****************************************************************************/

static const luaL_Reg libfuncs[] =
{
	{ "new", lib_new },
	{ "overlapCoords", lib_overlap },
	{ NULL, NULL }
};

static const luaL_Reg regionmethods[] =
{
	{ "__gc", region_collect },
	{ "setRect", region_set },
	{ "getRects", region_getrects },
	{ "getRect", region_getrect },
	{ "orRect", region_orrect },
	{ "xorRect", region_xorrect },
	{ "subRect", region_subrect },
	{ "setFlags", region_setflags },
	{ "getFlags", region_getflags },
	{ "overlapRect", region_overlaprect },
	{ "checkOverlap", region_checkoverlap },
	{ NULL, NULL }
};

int luaopen_tek_lib_region(lua_State *L)
{
	luaL_register(L, "tek.lib.region", libfuncs);
	/* s: libtab */
	lua_pop(L, 1);
	/* s: */

	/* require "tek.lib.exec": */
	lua_getglobal(L, "require");
	/* s: "require" */
	lua_pushliteral(L, "tek.lib.exec");
	/* s: "require", "tek.lib.exec" */
	lua_call(L, 1, 1);
	/* s: exectab */
	lua_getfield(L, -1, "base");
	/* s: exectab, execbase */
	lua_remove(L, -2);
	/* s: execbase */

	luaL_newmetatable(L, TEK_CLASS_UI_REGION_NAME);
	/* s: execbase, metatable */
	luaL_register(L, NULL, regionmethods);
	/* s: execbase, metatable */
	lua_pushvalue(L, -1);
	/* s: execbase, metatable, metatable */
	lua_pushvalue(L, -3);
	/* s: execbase, metatable, metatable, execbase */
	luaL_ref(L, -2);
	/* s: execbase, metatable, metatable */
	lua_setfield(L, -2, "__index");
	/* s: execbase, metatable */
	lua_pop(L, 2);
	/* s: */

	return 0;
}
