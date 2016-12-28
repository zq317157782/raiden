/*
 * stats.h
 *
 *  Created on: 2016年11月22日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_STATS_H_
#define SRC_CORE_STATS_H_

#include "raiden.h"
#include <map>
#include <functional>
#include <mutex>
//统计数据累积器
class StatsAccumulator{
private:
	std::map<std::string,uint64_t> _counters;//统计个数的map
public:
	void ReportCounter(std::string name,uint64_t count){
		_counters[name]+=count;
	}

	void LogStats(){
		std::cout<<"--------Stats--------"<<std::endl;
		std::cout<<"[Counter]:"<<std::endl;
		for(auto item:_counters){
			if(item.second==0) continue;
			std::cout<<"--"<<item.first<<":"<<item.second<<std::endl;
		}
	}
};

//状态注册器
//使用构造器来注册状态
class StatRegisterer{
public:
	static std::vector<std::function<void(StatsAccumulator&)>> * statsCallbackFuncs;
public:
	static void CallCallBacks(StatsAccumulator& accum);

	StatRegisterer(std::function<void(StatsAccumulator&)> func){
		if(statsCallbackFuncs==nullptr){
			statsCallbackFuncs=new std::vector<std::function<void(StatsAccumulator&)>>();
		}
		statsCallbackFuncs->push_back(func);
	}
};

//每个线程完成后，都需要调用这个函数来向累加器报告统计数据
void ReportThreadStats();

void LogStats();//Log打印stats

#define STAT_COUNTER(name, var)                           \
    static thread_local uint64_t var;                  \
    static void STATS_FUNC##var(StatsAccumulator &accum) { \
        accum.ReportCounter(name, var);                   \
        var = 0;                                           \
    }                                                      \
    static StatRegisterer STATS_REG##var(STATS_FUNC##var)

#endif /* SRC_CORE_STATS_H_ */
