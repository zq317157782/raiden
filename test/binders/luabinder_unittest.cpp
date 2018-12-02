#include "raiden.h"
#include "gtest.h"
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static void LuaTestTable(lua_State *L) {
	EXPECT_TRUE(lua_istable(L,-1)==true);
    lua_pushnumber(L,1);//栈顶压入第一个元素的索引
    lua_gettable(L,-2);
    EXPECT_TRUE(lua_isstring(L,-1)==true);
    EXPECT_STREQ(lua_tostring(L,-1),"float");
    //LInfo<<lua_tostring(L,-1);
    lua_pop(L,1);
    EXPECT_TRUE(lua_istable(L,-1)==true);
}

TEST(Lua,API){
   lua_State * _L=luaL_newstate();
   luaL_openlibs(_L);
   lua_register(_L, "LuaTestTable", (lua_CFunction )LuaTestTable);
   luaL_dostring(_L,"LuaTestTable({'float',1})");
}