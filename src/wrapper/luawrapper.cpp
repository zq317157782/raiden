#include "luawrapper.h"

void LuaWrapper::Init(){
    //初始化Lua
    _L=luaL_newstate();
    luaL_openlibs(_L);
}

void LuaWrapper::Release(){
    
}


void LuaWrapper::ExecScript(const char*  fileName){
    int ret = luaL_dofile(_L, fileName);
	if (ret != LUA_OK) {
		lua_error(_L);
	}
}