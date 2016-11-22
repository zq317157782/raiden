/*
 * stats.cpp
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */
#include "stats.h"

static StatsAccumulator statsAccumulator;

std::vector<std::function<void(StatsAccumulator&)>> * StatRegisterer::statsCallbackFuncs=nullptr;


void StatRegisterer::CallCallBacks(StatsAccumulator& accum){
	for(auto func:*statsCallbackFuncs){
		func(accum);
	}
}

void LogStats(){
	statsAccumulator.LogStats();
}
void ReportThreadStats(){
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex); //上锁，直到out of scope
	StatRegisterer::CallCallBacks(statsAccumulator);//调用所有
}
