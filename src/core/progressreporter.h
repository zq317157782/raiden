/*
 * progressreporter.h
 *
 *  Created on: 2017年1月27日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PROGRESSREPORTER_H_
#define SRC_CORE_PROGRESSREPORTER_H_
#include "raiden.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <algorithm>
//进度条
class ProgressReporter{
private:
	std::atomic<uint64_t> _doneNum;//已经完成了个工作数
	uint64_t _totalNum;//工作数总数
	const std::string _title;//进度条标题
	const const std::chrono::system_clock::time_point _startTime;
	std::atomic<bool> _threadExit;
	std::thread _updateThread;
	void PrintBar(){
		//线程的睡眠时间
		std::chrono::milliseconds sleepDuration(250);
		int totalLength=80;
		char buf[totalLength];
		snprintf(buf,_title.size()+1,_title.c_str());
		buf[_title.size()]='[';
		char* barStart=buf+_title.size()+1;
		for(int i=0;i<50;++i){
			barStart[i]=' ';
		}
		barStart[50]=']';
		barStart[52]='\n';
		barStart[53]='\0';
		uint64_t it=0;
		while(!_threadExit){
			for(int i=0;i<_title.size()+54;++i){
					printf("\b");
			}
			std::this_thread::sleep_for(sleepDuration);
			Float percent=(Float)(_doneNum)/(_totalNum);
			Float num=50*percent;
			for(int i=0;i<num;++i){
				barStart[i]='+';
			}
			fputs(buf, stdout);
			fflush(stdout);
			it++;
		}
		printf("\n");
	}
public:
	ProgressReporter(uint64_t totalNum,const std::string& title):_totalNum(std::max((uint64_t)1,totalNum)),_title(title){
		_startTime=std::chrono::system_clock::now();
		_doneNum=0;
		_threadExit=false;
		//开启更新进程
		_updateThread=std::thread([this](){
			this->PrintBar();
			if(_totalNum==_doneNum){
				_threadExit=true;
			}
		});
	}
	~ProgressReporter(){
		_doneNum = _totalNum;
		_threadExit = true;
		_updateThread.join();
	}
	//更新进度
	void Update(int num=1){
		Assert(num>=1);
		//更新当前完成任务个数
		_doneNum+=num;
	}
	//完成进度
	void Done(){
		_doneNum=_totalNum;
	}
};


#endif /* SRC_CORE_PROGRESSREPORTER_H_ */
