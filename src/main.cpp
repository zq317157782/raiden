/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#ifdef WIN32
  #include "getopt.h"
  #define GOOGLE_GLOG_DLL_DECL           // 使用静态glog库用这个
  #define GLOG_NO_ABBREVIATED_SEVERITIES // 没这个编译会出错,传说因为和Windows.h冲突
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


void Trim(char * str)
{
    int i , len ;

    //先去除左边的空格
    len = strlen( str ) ;
    for( i=0; i<len; i++)
    {
        if( str[i] != ' ') break ;
    }
    memmove(str,str+i,len-i+1);

    //再去除右边的空格
    len = strlen( str ) ;
    for(i = len-1; i>=0; i--)
    {
        if(str[i] != ' ') break ;
    }
    str[i+1] = 0 ;

    return ;
}

int main(int argc, char* argv[]) {
	
	FLAGS_logtostderr = 1;
	google::InitGoogleLogging(argv[0]);

	InitWrappers();
    int result;
	char *scriptName=nullptr;
    Options options;
    options.numThread=0;
    options.imageFile="raiden.png";
    while((result = getopt(argc,argv,"i:t:o:")) != -1) {
        switch(result){
            case 'i':{ 
               // strcpy(scriptName,optarg);
                scriptName=optarg;
			}break;
			case 't':{ 
				int numThread=atoi(optarg);
				options.numThread=numThread;
			}break;
			case 'o':{ 
				options.imageFile=optarg;
			}
            break;
        }
    }

	raidenInit(options);
	if (scriptName != nullptr) {
        Trim(scriptName);
		ExecScript(scriptName);
	}
	
	raidenCleanup();
}

