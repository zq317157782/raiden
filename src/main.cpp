/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */
//#undef DEBUG_BUILD
#include "raiden.h"
#include "test/test.h"

int main(int argc, char* argv[]) {
#ifdef UNIT_TEST
	//UnitTest(argc, argv);
#endif
	//lua_exec(argc,argv);
	RaidenMain(argc,argv);
}

