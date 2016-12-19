/*
 * raiden.cpp
 *
 *  Created on: 2016年11月15日
 *      Author: zhuqian
 */
#include "raiden.h"
#include "stats.h"
#include "parser.h"

static int test() {
	Error("test");
}

void RaidenMain(int argc, char* argv[]) {
	//开始解析输入命令
	Options options;
//	for (int i = 0; i < argc; ++i) {
//		if (!std::strcmp(argv[i], "--numThreads")
//				|| !std::strcmp(argv[i], "-numThreads")) {
//			if (i + 1 == argc) {
//				printf("--numThreads mis value \n");
//				return;
//			}
//			options.numThread = atoi(argv[++i]);
//		}
//	}
	parse(argv[1]);
}

