/*
 * statsTest.cpp
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */

#include "statsTest.h"
#include "stats.h"

STAT_COUNTER("StatsTest created",StatsTestCount);
StatsTest::StatsTest(){
	StatsTestCount++;
}

