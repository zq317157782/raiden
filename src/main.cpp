/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifdef WIN32
  #include "getopt.h"
#else
  #include <unistd.h>
#endif
#include "api.h"
#include "wrapper/luawrapper.h"
static std::vector<std::unique_ptr<APIWrapper>> wrappers;

//初始化所有的Wrapper
void InitWrappers(){
    wrappers.push_back(std::unique_ptr<APIWrapper>(new LuaWrapper()));
    for(int i=0;i<wrappers.size();++i){
        wrappers[i]->Init();
    }
}

//执行脚本
void ExecScript(const char* src){
    for(int i=0;i<wrappers.size();++i){
        if(wrappers[i]->IsSupportedScript(src)){
            wrappers[i]->ExecScript(src);
            return;
        }
    }
}


int main(int argc, char* argv[]) {
	InitWrappers();
    int result;
    char *scriptName=nullptr;
    while((result = getopt(argc,argv,"i:l:")) != -1) {
        switch(result){
            case 'i':{ 
               // strcpy(scriptName,optarg);
                scriptName=optarg;
            }
            break;
        }
    }

	if (scriptName != nullptr) {
		ExecScript(scriptName);
	}
}

