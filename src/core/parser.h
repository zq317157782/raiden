/*
 * apilua.h
 *
 *  Created on: 2016年12月19日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_APILUA_H_
#define SRC_CORE_APILUA_H_

#include "lua/lua.hpp"
extern "C" {
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include "raiden.h"
#include "api.h"
static lua_State *L;

#define PARAM_TYPR_WRONG(x) lua_getglobal(L, "debug");  \
							lua_getfield(L, -1, "traceback");\
int iError = lua_pcall( L,0,1,0);\
Error("[wrong param]");\
Error(lua_tostring(L, -1));\
exit(1);

static void Init() {
	Options options;
	if (lua_isinteger(L, 1)) {
		int nThreads = lua_tointeger(L, 1);
		Assert(nThreads >= 0);
		options.numThread = nThreads;
	} else {
		PARAM_TYPR_WRONG()
	}
	if (lua_isstring(L, 2)) {
		options.imageFile = lua_tostring(L, 2);
	} else {
		PARAM_TYPR_WRONG()
	}
	raidenInit(options);
}

static void CleanUp() {
	raidenCleanup();
}


static void WorldBegin(){
	raidenWorldBegin();
}

static void WorldEnd(){
	raidenWorldEnd();
}



void parse(char* filename) {
	int status, result;
	L = luaL_newstate(); /* create state */
	luaL_openlibs(L);
	lua_register(L, "Init", (lua_CFunction )Init);
	lua_register(L, "WorldBegin", (lua_CFunction )WorldBegin);
	lua_register(L, "WorldEnd", (lua_CFunction )WorldEnd);
	luaL_dofile(L, filename);
}

#endif /* SRC_CORE_APILUA_H_ */
