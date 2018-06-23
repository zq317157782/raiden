
/*
 * constant.cpp
 *
 *  Created on: 2017年9月13日
 *      Author: zhuqian
 */
#pragma once

#include "apibinder.h"
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

class LuaBinder : public APIBinder{
private:
    lua_State * _L;
    const std::string _postFix=".lua";

    bool EndWithLua(const std::string& fileName) const{
        if(fileName.compare(fileName.size()-_postFix.size(),_postFix.size(),_postFix)==0){
            return true;
        }
        return false;
    }
public:
     void Init() override;
     void Release() override;
    
     bool IsSupportedScript(const char*  fileName) const override{ 
          assert(fileName!=nullptr);
          return EndWithLua(std::string(fileName));
     }

     //执行Lua脚本
     void ExecScript(const char*  fileName) override;
};