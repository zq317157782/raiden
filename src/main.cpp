/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */

#include "config.h"

#ifdef WIN32
  #include "getopt.h"
  #define GOOGLE_GLOG_DLL_DECL           // 使用静态glog库用这个
  #define GLOG_NO_ABBREVIATED_SEVERITIES // 没这个编译会出错,传说因为和Windows.h冲突
#else
  #include <unistd.h>
#endif


#include "api.h"
#include "parsers/luaparser.h"
#include "parsers/xmlparser.h"
static std::vector<std::unique_ptr<Parser>> parsers;

//初始化所有的binder
static void InitParsers(){
    parsers.push_back(std::unique_ptr<Parser>(new LuaParser()));
    parsers.push_back(std::unique_ptr<Parser>(new XMLParser()));
    for(int i=0;i<parsers.size();++i){
        parsers[i]->Init();
    }
}

//执行脚本
static void Parse(const char* src){
    for(int i=0;i<parsers.size();++i){
        if(parsers[i]->IsSupported(src)){
            parsers[i]->Parse(src);
            return;
        }
    }
}


static void Trim(char * str)
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

static void PrintHelp(){
     std::cout<<"*** Physically Based Renderer <Raiden[雷電 らいでん]> ***"<<std::endl;
     std::cout<<"   -h help."<<std::endl;
     std::cout<<"   -i [string] : input a scene file."<<std::endl;
     std::cout<<"   -o [string] : output image name."<<std::endl;
     std::cout<<"   -t [num] : core num for rendering."<<std::endl;
     
}

int main(int argc, char* argv[]) {
	int ret=0;

	FLAGS_logtostderr = 1;
	google::InitGoogleLogging(argv[0]);
	
    InitParsers();

    int result;
	char *scriptName=nullptr;
    Options options;
    options.numThread=0;
    options.imageFile="default.png";
    while((result = getopt(argc,argv,"i:t:o:h")) != -1) {
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
			}break;
            case 'h':
            default:
            {
                PrintHelp();
                goto MAIN_RET;
            }
            break;
        }
    } 
    if(argc==1){ 
        PrintHelp();
    }

	raidenInit(options);
	if (scriptName != nullptr) {
        Trim(scriptName);
		Parse(scriptName);
	}
	
	raidenCleanup();
MAIN_RET:
    return ret;
}

