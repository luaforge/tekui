
/*
**	tek.ui.layout.default - Default layouter
**	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
**	See copyright notice in COPYRIGHT
*/

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <tek/type.h>

/* Name of superclass: */
#define SUPERCLASS_NAME "tek.class"

/* Name of this class: */
#define CLASS_NAME "tek.ui.layout.default"

/*****************************************************************************/

#define HUGE 1000000

static const char *ALIGN[2][6] =
{
	{ "HAlign", "VAlign", "right", "bottom", "Width", "Height" },
	{ "VAlign", "HAlign", "bottom", "right", "Height", "Width" }
};

static const int INDICES[2][6] =
{
	{ 1, 2, 3, 4, 5, 6 },
	{ 2, 1, 4, 3, 6, 5 },
};

/*****************************************************************************/
/*
**	list = layoutAxis(self, group, free, i1, i3, n, isgrid)
*/

static int lib_layoutaxis(lua_State *L)
{
	lua_Number free = lua_tonumber(L, 3);
	lua_Integer i1 = lua_tointeger(L, 4);
	lua_Integer i3 = lua_tointeger(L, 5);
	lua_Integer n = lua_tointeger(L, 6);
	int isgrid = lua_toboolean(L, 7);

	int it = 0;
	size_t len;
	int ssb, ssn;
	int i;

	lua_Integer fw0 = 0;
	lua_Integer tw0 = 0;
	lua_Number fw;
	lua_Number tw;

	lua_createtable(L, n, 0);
	lua_getfield(L, 1, "TempMinMax");
	lua_rawgeti(L, -1, i1);
	lua_rawgeti(L, -2, i3);
	lua_remove(L, -3);
	lua_getfield(L, 2, "Weights");
	lua_rawgeti(L, -1, i1);
	lua_remove(L, -2);

	for (i = 1; i <= n; ++i)
	{
		int free;
		lua_rawgeti(L, -3, i);
		lua_rawgeti(L, -3, i);
		lua_rawgeti(L, -3, i);
		free = lua_toboolean(L, -2) ?
			lua_tointeger(L, -2) > lua_tointeger(L, -3) : 0;
		lua_createtable(L, 5, 0);
		lua_pushboolean(L, free);
		lua_rawseti(L, -2, 1);
		lua_pushvalue(L, -4);
		lua_rawseti(L, -2, 2);
		lua_pushvalue(L, -3);
		lua_rawseti(L, -2, 3);
		lua_pushvalue(L, -2);
		lua_rawseti(L, -2, 4);
		lua_pushnil(L);
		lua_rawseti(L, -2, 5);
		lua_rawseti(L, -8, i);
		if (free)
		{
			if (lua_toboolean(L, -1))
				tw0 += lua_tointeger(L, -1);
			else
				fw0 += 0x100;
		}
		lua_pop(L, 3);
	}
	lua_pop(L, 3);

	if (tw0 < 0x10000)
	{
		if (fw0 == 0)
			tw0 = 0x10000;
		else
		{
			fw = 0x10000;
			fw -= tw0;
			fw *= 0x100;
			fw0 = fw / fw0;
			tw0 = 0x10000;
		}
	}
	else
		fw0 = 0;

	tw = tw0 / 0x100;
	fw = fw0 / 0x100;

	lua_getfield(L, 2, "getSameSize");
	lua_pushvalue(L, 2);
	lua_pushinteger(L, i1);
	lua_call(L, 2, 1);
	ssb = lua_toboolean(L, -1);
	lua_pop(L, 1);
	if (ssb)
	{
		lua_getfield(L, 2, "MinMax");
		lua_rawgeti(L, -1, i1);
		lua_getfield(L, 2, "MarginAndBorder");
		lua_rawgeti(L, -1, i1);
		lua_rawgeti(L, -2, i3);
		lua_getfield(L, 2, "PaddingAndBorder");
		lua_rawgeti(L, -1, i1);
		lua_rawgeti(L, -2, i3);
		ssn = lua_tonumber(L, -7);
		ssn -= lua_tonumber(L, -5);
		ssn -= lua_tonumber(L, -4);
		ssn -= lua_tonumber(L, -2);
		ssn -= lua_tonumber(L, -1);
		lua_pop(L, 8);
		ssn /= n;
	}

	lua_getglobal(L, "table");
	lua_getfield(L, -1, "insert");
	lua_getfield(L, -2, "remove");
	lua_remove(L, -3);

	len = lua_objlen(L, -3);
	lua_createtable(L, len, 0);
	for (i = 1; i <= len; ++i)
	{
		lua_rawgeti(L, -4, i);
		lua_rawseti(L, -2, i);
	}

	while ((len = lua_objlen(L, -1)) > 0)
	{
		lua_Integer rest = free;
		lua_Integer newfree = free;
		it++;

		lua_createtable(L, len, 0);

		do
		{
			lua_Integer olds, news, ti;

			lua_Integer delta = 0;

			lua_pushvalue(L, -3);
			lua_pushvalue(L, -3);
			lua_pushinteger(L, 1);
			lua_call(L, 2, 1);

			lua_rawgeti(L, -1, 1);
			if (lua_toboolean(L, -1))
			{
				lua_Number t;
				lua_rawgeti(L, -2, 4);
				if (lua_toboolean(L, -1))
				{
					t = lua_tonumber(L, -1);
					t /= 0x100;
					t *= tw;
					t *= free;
				}
				else
				{
					t = free;
					t *= 0x100;
					t *= fw;
				}
				t /= 0x10000;
				delta = t;
				lua_pop(L, 1);
			}

			if (delta == 0 && it > 1)
				delta = rest;

			lua_rawgeti(L, -2, 5);
			lua_rawgeti(L, -3, 2);

			if (lua_toboolean(L, -2))
				olds = lua_tointeger(L, -2);
			else if (ssb)
				olds = ssn;
			else
				olds = lua_tointeger(L, -1);

			ti = ssb ? ssn : lua_tointeger(L, -1);
			news = TMAX(olds + delta, ti);

			lua_rawgeti(L, -4, 3);
			if (!(ssb && isgrid) && lua_toboolean(L, -1) &&
				news > lua_tointeger(L, -1))
				news = lua_tointeger(L, -1);

			lua_pushinteger(L, news);
			lua_rawseti(L, -6, 5);

			delta = news - olds;
			newfree -= delta;
			rest -= delta;

			if (!lua_toboolean(L, -1) || lua_tointeger(L, -1) >= HUGE ||
				lua_tointeger(L, -3) < lua_tointeger(L, -1))
			{
				/* redo in next iteration: */
				lua_pushvalue(L, -9);
				lua_pushvalue(L, -7);
				lua_pushvalue(L, -7);
				lua_call(L, 2, 0);
			}

			lua_pop(L, 5);

		} while (lua_objlen(L, -2) > 0);

		free = newfree;
		if (free < 1)
		{
			lua_pop(L, 1);
			break;
		}

		lua_replace(L, -2);
	}

	lua_pop(L, 3);
	return 1;
}

/*****************************************************************************/
/*
**	layout(self, group, r1, r2, r3, r4, markdamage)
*/

static int lib_layout(lua_State *L)
{
	lua_Integer ori, gs1, gs2;
	lua_getfield(L, 2, "getStructure");
	lua_pushvalue(L, 2);
	lua_call(L, 1, 3);
	ori = lua_tointeger(L, -3);
	gs1 = lua_tointeger(L, -2);
	gs2 = lua_tointeger(L, -1);
	lua_pop(L, 3);

	if (gs1 > 0 && gs2 > 0)
	{
		int isgrid = (gs1 > 1) && (gs2 > 1);
		const int *I = INDICES[ori - 1];
		int i1 = I[0], i2 = I[1], i3 = I[2], i4 = I[3], i5 = I[4], i6 = I[5];

		lua_Integer isz, osz, oszmax, t, iidx;
		lua_Integer m3, m4, oidx, goffs;
		lua_Integer r1, r2, r3, r4;
		lua_Integer cidx = 1;

		const char **A = ALIGN[ori - 1], *s;

		if (i1 == 2)
		{
			r2 = lua_tointeger(L, 3);
			r1 = lua_tointeger(L, 4);
			r4 = lua_tointeger(L, 5);
			r3 = lua_tointeger(L, 6);
			t = gs1;
			gs1 = gs2;
			gs2 = t;
		}
		else
		{
			r1 = lua_tointeger(L, 3);
			r2 = lua_tointeger(L, 4);
			r3 = lua_tointeger(L, 5);
			r4 = lua_tointeger(L, 6);
		}

		lua_getfield(L, 2, "MarginAndBorder");
		lua_getfield(L, 2, "Rect");
		lua_getfield(L, 2, "Padding");
		lua_rawgeti(L, -1, i1);
		lua_rawgeti(L, -4, i1);
		goffs = lua_tonumber(L, -1) + lua_tonumber(L, -2);
		lua_pop(L, 2);
		lua_createtable(L, 5, 0);

		lua_getfield(L, 2, "MinMax");
		lua_getfield(L, 1, "layoutAxis");
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_rawgeti(L, -4, i2);
		lua_pushinteger(L, r4 - r2 + 1 - lua_tointeger(L, -1));
		lua_remove(L, -2);
		lua_pushinteger(L, i2);
		lua_pushinteger(L, i4);
		lua_pushinteger(L, gs2);
		lua_pushboolean(L, isgrid);
		lua_call(L, 7, 1);

		/* layout on inner axis: */
		lua_getfield(L, 1, "layoutAxis");
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_rawgeti(L, -5, i1);
		lua_pushinteger(L, r3 - r1 + 1 - lua_tointeger(L, -1));
		lua_remove(L, -2);
		lua_pushinteger(L, i1);
		lua_pushinteger(L, i3);
		lua_pushinteger(L, gs1);
		lua_pushboolean(L, isgrid);
		lua_call(L, 7, 1);
		lua_remove(L, -3);

		lua_getfield(L, 2, "Children");

		/* size on outer axis: */
		lua_rawgeti(L, -6, i4);
		lua_rawgeti(L, -7, i2);
		lua_rawgeti(L, -7, i2);
		lua_rawgeti(L, -8, i4);
		oszmax = lua_tointeger(L, -4) + 1;
		oszmax -= lua_tointeger(L, -3);
		oszmax -= lua_tointeger(L, -2);
		oszmax -= lua_tointeger(L, -1);
		lua_pop(L, 4);

		/* starting position on outer axis: */
		lua_rawgeti(L, -7, i2);
		lua_rawgeti(L, -6, i2);
		lua_pushinteger(L, r2 + lua_tointeger(L, -2) + lua_tointeger(L, -1));
		lua_rawseti(L, -7, i6);
		lua_pop(L, 2);

		/* loop outer axis: */
		for (oidx = 1; oidx <= gs2; ++oidx)
		{
			if (gs2 > 1)
			{
				lua_rawgeti(L, -3, oidx);
				lua_rawgeti(L, -1, 5);
				oszmax = lua_tointeger(L, -1);
				lua_pop(L, 2);
			}

			/* starting position on inner axis: */
			lua_pushinteger(L, r1 + goffs);
			lua_rawseti(L, -5, i5);

			/* loop inner axis: */
			for (iidx = 1; iidx <= gs1; ++iidx)
			{
				lua_rawgeti(L, -1, cidx);
				if (!lua_toboolean(L, -1))
				{
					lua_pop(L, 8);
					return 0;
				}

				/* x0, y0 of child rectangle: */
				lua_rawgeti(L, -5, 5);
				lua_rawseti(L, -6, 1);
				lua_rawgeti(L, -5, 6);
				lua_rawseti(L, -6, 2);

				/* element minmax: */
				lua_getfield(L, -1, "MinMax");

				/* max per inner and outer axis: */
				lua_rawgeti(L, -1, i3);
				lua_rawgeti(L, -2, i4);
				m3 = lua_tointeger(L, -2);
				m4 = lua_tointeger(L, -1);
				lua_pop(L, 2);

				/* inner size: */
				lua_pushvalue(L, -4);
				lua_rawgeti(L, -1, iidx);
				lua_rawgeti(L, -1, 5);
				isz = lua_tointeger(L, -1);
				lua_pop(L, 3);

				lua_getfield(L, -2, A[4]);
				s = lua_tostring(L, -1);
				if (s)
				{
					if (strcmp(s, "auto") == 0)
					{
						lua_rawgeti(L, -2, i1);
						m3 = lua_tointeger(L, -1);
						lua_pop(L, 1);
					}
					else if (strcmp(s, "free") == 0 || strcmp(s, "fill") == 0)
					{
						lua_rawgeti(L, -9, i3);
						lua_rawgeti(L, -10, i1);
						lua_rawgeti(L, -10, i1);
						lua_rawgeti(L, -11, i3);
						m3 = lua_tointeger(L, -4) + 1;
						m3 -= lua_tointeger(L, -3);
						m3 -= lua_tointeger(L, -2);
						m3 -= lua_tointeger(L, -1);
						lua_pop(L, 4);
					}
				}
				lua_pop(L, 1);

				if (m3 < isz)
				{
					lua_getfield(L, -2, A[0]);
					s = lua_tostring(L, -1);
					if (s)
					{
						if (strcmp(s, "center") == 0)
						{
							lua_rawgeti(L, -7, i1);
							t = lua_tointeger(L, -1);
							t += (isz - m3) / 2;
							lua_pushinteger(L, t);
							lua_rawseti(L, -9, i1);
							lua_pop(L, 1);
						}
						else if (strcmp(s, A[2]) == 0)
						{
							lua_rawgeti(L, -7, i1);
							t = lua_tointeger(L, -1);
							t += isz - m3;
							lua_pushinteger(L, t);
							lua_rawseti(L, -9, i1);
							lua_pop(L, 1);
						}
					}
					isz = m3;
					lua_pop(L, 1);
				}

				/* outer size: */
				lua_getfield(L, -2, A[5]);
				s = lua_tostring(L, -1);
				if (s && (strcmp(s, "fill") == 0 || strcmp(s, "free") == 0))
					osz = oszmax;
				else
				{
					if (s && strcmp(s, "auto") == 0)
					{
						lua_rawgeti(L, -2, i2);
						m4 = lua_tointeger(L, -1);
						lua_pop(L, 1);
					}
					lua_rawgeti(L, -6, oidx);
					lua_rawgeti(L, -1, 5);
					osz = lua_tointeger(L, -1);
					osz = TMIN(osz, m4);
					lua_pop(L, 2);
					/* align if element does not fully occupy outer size: */
					if (osz < oszmax)
					{
						lua_getfield(L, -3, A[1]);
						s = lua_tostring(L, -1);
						if (s)
						{
							if (strcmp(s, "center") == 0)
							{
								lua_rawgeti(L, -8, i2);
								t = lua_tointeger(L, -1);
								t += (oszmax - osz) / 2;
								lua_pushinteger(L, t);
								lua_rawseti(L, -10, i2);
								lua_pop(L, 1);
							}
							else if (strcmp(s, A[3]) == 0)
							{
								/* opposite side: */
								lua_rawgeti(L, -8, i2);
								t = lua_tointeger(L, -1);
								t += oszmax - osz;
								lua_pushinteger(L, t);
								lua_rawseti(L, -10, i2);
								lua_pop(L, 1);
							}
						}
						lua_pop(L, 1);
					}
				}
				lua_pop(L, 1);

				/* x1, y1 of child rectangle: */
				lua_rawgeti(L, -6, i1);
				t = lua_tointeger(L, -1);
				lua_pushinteger(L, t + isz - 1);
				lua_rawseti(L, -8, i3);
				lua_pop(L, 1);

				lua_rawgeti(L, -6, i2);
				t = lua_tointeger(L, -1);
				lua_pushinteger(L, t + osz - 1);
				lua_rawseti(L, -8, i4);
				lua_pop(L, 1);

				/* enter recursion: */
				lua_getfield(L, -2, "layout");
				lua_pushvalue(L, -3);
				lua_rawgeti(L, -8, 1);
				lua_rawgeti(L, -9, 2);
				lua_rawgeti(L, -10, 3);
				lua_rawgeti(L, -11, 4);
				lua_pushvalue(L, 7);
				lua_call(L, 6, 0);

				/* punch a hole for the element into the background: */
				lua_getfield(L, -2, "punch");
				lua_pushvalue(L, -3);
				lua_getfield(L, 2, "FreeRegion");
				lua_call(L, 2, 0);

				/* update x0: */
				lua_rawgeti(L, -6, i5);
				lua_rawgeti(L, -5, iidx);
				lua_rawgeti(L, -1, 5);
				t = lua_tointeger(L, -3);
				t += lua_tointeger(L, -1);
				lua_pushinteger(L, t);
				lua_rawseti(L, -10, i5);
				lua_pop(L, 5);

				/* next child index: */
				cidx++;
			}

			/* update y0: */
			lua_rawgeti(L, -4, i6);
			lua_rawgeti(L, -4, oidx);
			lua_rawgeti(L, -1, 5);
			t = lua_tointeger(L, -3);
			t += lua_tointeger(L, -1);
			lua_pushinteger(L, t);
			lua_rawseti(L, -8, i6);
			lua_pop(L, 3);
		}
		lua_pop(L, 7);
	}
	return 0;
}

/*****************************************************************************/
/*
**	m1, m2, m3, m4 = askMinMax(self, group, m1, m2, m3, m4)
*/

static int lib_askMinMax(lua_State *L)
{
	int m[5] = { 0, 0, 0, 0, 0 };
	int ori, gw, gh;

	lua_getfield(L, 2, "getStructure");
	lua_pushvalue(L, 2);
	lua_call(L, 1, 3);
	ori = lua_tointeger(L, -3);
	gw = lua_tointeger(L, -2);
	gh = lua_tointeger(L, -1);
	lua_pop(L, 3);

	if (gw > 0 && gh > 0)
	{
		int gss, i1, gs, y, x;
		int cidx = 1;

		lua_getfield(L, 2, "getSameSize");
		lua_pushvalue(L, 2);
		lua_pushinteger(L, ori);
		lua_call(L, 2, 1);
		gss = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_createtable(L, 4, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, 1, "TempMinMax");
		lua_createtable(L, gw, 0);
		lua_createtable(L, gh, 0);
		lua_createtable(L, gw, 0);
		lua_createtable(L, gh, 0);
		lua_getfield(L, 2, "Children");

		for (y = 1; y <= gh; ++y)
		{
			for (x = 1; x <= gw; ++x)
			{
				int mm1, mm2, mm3, mm4;
				int minxx, minyy;
				const char *s;

				lua_rawgeti(L, -1, cidx);

				if (!lua_toboolean(L, -1))
				{
					lua_pop(L, 1);
					break;
				}
				cidx++;

				lua_getfield(L, -1, "askMinMax");
				lua_pushvalue(L, -2);
				lua_pushvalue(L, 3);
				lua_pushvalue(L, 4);
				lua_pushvalue(L, 5);
				lua_pushvalue(L, 6);
				lua_call(L, 5, 4);
				mm1 = lua_tointeger(L, -4);
				mm2 = lua_tointeger(L, -3);
				mm3 = lua_tointeger(L, -2);
				mm4 = lua_tointeger(L, -1);
				lua_pop(L, 4);

				lua_getfield(L, -1, "Width");
				s = lua_tostring(L, -1);
				if (s)
				{
					if (strcmp(s, "auto") == 0)
						mm3 = mm1;
					else if (strcmp(s, "fill") == 0)
						mm3 = -1; /* nil */
					else if (strcmp(s, "free") == 0)
						mm3 = HUGE;
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "Height");
				s = lua_tostring(L, -1);
				if (s)
				{
					if (strcmp(s, "auto") == 0)
						mm4 = mm2;
					else if (strcmp(s, "fill") == 0)
						mm4 = -1; /* nil */
					else if (strcmp(s, "free") == 0)
						mm4 = HUGE;
				}
				lua_pop(L, 2);

				lua_getfield(L, 2, "Width");
				s = lua_tostring(L, -1);
				if (s && strcmp(s, "auto") == 0)
					mm3 = gss ? -1 : mm1;
				lua_pop(L, 1);

				lua_getfield(L, 2, "Height");
				s = lua_tostring(L, -1);
				if (s && strcmp(s, "auto") == 0)
					mm4 = gss ? -1 : mm2;
				lua_pop(L, 1);

				if (mm3 < 0 && ori == 2)
					mm3 = mm1;
				if (mm4 < 0 && ori == 1)
					mm4 = mm2;

				lua_rawgeti(L, -5, x);
				minxx = lua_isnil(L, -1) ? 0 : lua_tointeger(L, -1);
				minxx = TMAX(minxx, mm1);
				lua_pushinteger(L, minxx);
				lua_rawseti(L, -7, x);
				lua_pop(L, 1);

				lua_rawgeti(L, -4, y);
				minyy = lua_isnil(L, -1) ? 0 : lua_tointeger(L, -1);
				minyy = TMAX(minyy, mm2);
				lua_pushinteger(L, minyy);
				lua_rawseti(L, -6, y);
				lua_pop(L, 1);

				if (mm3 >= 0)
				{
					lua_rawgeti(L, -3, x);
					if (lua_isnil(L, -1) || mm3 > lua_tointeger(L, -1))
					{
						lua_pushinteger(L, TMAX(mm3, minxx));
						lua_rawseti(L, -5, x);
					}
					lua_pop(L, 1);
				}

				if (mm4 >= 0)
				{
					lua_rawgeti(L, -2, y);
					if (lua_isnil(L, -1) || mm4 > lua_tointeger(L, -1))
					{
						lua_pushinteger(L, TMAX(mm4, minyy));
						lua_rawseti(L, -4, y);
					}
					lua_pop(L, 1);
				}

			}
		}

		lua_pop(L, 1);
		lua_rawseti(L, -5, 4);
		lua_rawseti(L, -4, 3);
		lua_rawseti(L, -3, 2);
		lua_rawseti(L, -2, 1);

		gs = gw;
		for (i1 = 1; i1 <= 2; i1++)
		{
			int t1, ss, n;
			int i3 = i1 + 2;
			lua_getfield(L, 2, "getSameSize");
			lua_pushvalue(L, 2);
			lua_pushinteger(L, i1);
			lua_call(L, 2, 1);
			ss = lua_toboolean(L, -1);
			lua_pop(L, 1);

			for (n = 1; n <= gs; ++n)
			{
				lua_rawgeti(L, -1, i1);
				lua_rawgeti(L, -1, n);
				t1 = lua_tointeger(L, -1);
				m[i1] = ss ? TMAX(m[i1], t1) : m[i1] + t1;
				lua_pop(L, 2);

				lua_rawgeti(L, -1, i3);
				lua_rawgeti(L, -1, n);
				if (!lua_isnil(L, -1))
				{
					int t2 = lua_tointeger(L, -1); /* tmm[i3][n] */
					lua_pushinteger(L, TMAX(t2, t1));
					lua_rawseti(L, -3, n);
					m[i3] = TMAX(m[i3], 0) + t2;
				}
				else if (ori == i1)
				{
					/* if on primary axis, we must reserve at least min: */
					m[i3] = TMAX(m[i3], 0) + t1;
				}
				lua_pop(L, 2);
			}

			if (ss)
			{
				m[i1] *= gs;
				if (m[i3] >= 0 && m[i1] > m[i3])
					m[i3] = m[i1];
			}
			gs = gh;
		}

		lua_pop(L, 1);
	}

	lua_pushinteger(L, m[1]);
	lua_pushinteger(L, m[2]);
	lua_pushinteger(L, m[3]);
	lua_pushinteger(L, m[4]);

	return 4;
}

/*****************************************************************************/

static int lib_new(lua_State *L)
{
	lua_newtable(L);
	lua_setfield(L, 2, "TempMinMax");
	lua_getglobal(L, "require");
	lua_pushliteral(L, SUPERCLASS_NAME);
	lua_call(L, 1, 1);
	lua_getfield(L, -1, "new");
	lua_remove(L, -2);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_call(L, 2, 1);
	return 1;
}

/*****************************************************************************/

static const luaL_Reg classfuncs[] =
{
	{ "new", lib_new },
	{ "askMinMax", lib_askMinMax },
	{ "layoutAxis", lib_layoutaxis },
	{ "layout", lib_layout },
	{ NULL, NULL }
};

int luaopen_tek_ui_layout_default(lua_State *L)
{
	lua_getglobal(L, "require");
	/* s: <require> */
	lua_pushliteral(L, SUPERCLASS_NAME);
	/* s: <require>, "superclass" */
	lua_call(L, 1, 1);
	/* s: superclass */
	lua_pushvalue(L, -1);
	/* s: superclass, superclass */
	luaL_register(L, CLASS_NAME, classfuncs);
	/* s: superclass, superclass, class */
	lua_call(L, 1, 1);
	/* s: superclass, class */
	luaL_newmetatable(L, CLASS_NAME "*");
	/* s: superclass, class, meta */
	lua_getfield(L, -3, "newClass");
	/* s: superclass, class, meta, <newClass> */
	lua_setfield(L, -2, "__call");
	/* s: superclass, class, meta */
	lua_pushvalue(L, -3);
	/* s: superclass, class, meta, superclass */
	lua_setfield(L, -2, "__index");
	/* s: superclass, class, meta */
	lua_setmetatable(L, -2);
	/* s: superclass, class */
	lua_pop(L, 2);
	return 0;
}
