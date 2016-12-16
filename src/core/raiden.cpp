/*
 * raiden.cpp
 *
 *  Created on: 2016年11月15日
 *      Author: zhuqian
 */
#include "raiden.h"
#include "stats.h"
void RaidenMain(int argc, char* argv[]){
	//开始解析输入命令
	Options options;
	for(int i=0;i<argc;++i){
		if(!std::strcmp(argv[i],"--numThreads")||!std::strcmp(argv[i],"-numThreads")){
			if(i+1==argc){
				printf("--numThreads命令后面缺少相应的参数值\n");
				return;
			}
			options.numThread=atoi(argv[++i]);
		}
	}
}


