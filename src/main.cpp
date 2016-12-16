/*
 * main.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: zhuqian
 */
//#undef DEBUG_BUILD
#include "raiden.h"
#include "test/test.h"
#include "api.h"
int main(int argc, char* argv[]) {
#ifdef UNIT_TEST
	UnitTest(argc, argv);
#endif
	RaidenMain(argc,argv);
}

