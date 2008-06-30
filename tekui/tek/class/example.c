
/*
**	example.c
**	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
**	See copyright notice in COPYRIGHT
**
**	Basic setup of a class for the tekUI toolkit written in C
*/

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* Name of superclass: */
#define SUPERCLASS_NAME "tek.class"

/* Name of this class: */
#define CLASS_NAME "tek.class.example"

static const luaL_Reg classfuncs[] =
{
	/* insert methods here */
	{ NULL, NULL }
};

int luaopen_tek_class_example(lua_State *L)
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
