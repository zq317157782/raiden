
/*
 * constant.cpp
 *
 *  Created on: 2017年9月13日
 *      Author: zhuqian
 */
#pragma once
#include "api.h"
class APIBinder{
private:

public:
    //初始化Wrapper
    virtual void Init()=0;

    //释放Wrapper所占的资源
    virtual void Release()=0;

    //执行脚本
    virtual void ExecScript(const char*  fileName){
    }
    //判断脚本是否被当前Wrapper支持
    virtual bool IsSupportedScript(const char*  fileName) const{
        return false;
    }
};