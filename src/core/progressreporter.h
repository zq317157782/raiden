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
	const std::chrono::system_clock::time_point _startTime;
	std::atomic<bool> _threadExit;
	std::thread _updateThread;
	void PrintBar(){
		//线程的睡眠时间
		std::chrono::milliseconds sleepDuration(500);
		char buf[128];
		char buf2[128];
		for(int i=0;i<128;++i){
			buf[i]=' ';
			buf2[i]='\b';
		}

		int titleSize=_title.size()+1;
		snprintf(buf,titleSize,"%s",_title.c_str());
		buf[_title.size()]='[';
		buf[_title.size()+51]=']';
		//到这里已经写了titleSize+52个字符了
		char* barStart=&buf[_title.size()+1];
		int it = 0;
		while(!_threadExit){
			if (it == 100) {
				sleepDuration = std::chrono::milliseconds(2000);
			}
			//清空上次一的数据
			/*for(int i=0;i<89;++i){
				printf("\b");
			}*/
			//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			std::this_thread::sleep_for(sleepDuration);
			Float percent=(Float)(_doneNum)/(_totalNum);
			Float num=50*percent;
			for(int i=0;i<num;++i){
				barStart[i]='+';
			}

			//统计剩余时间
			 std::chrono::system_clock::time_point now =std::chrono::system_clock::now();
			 long long ep= std::chrono::duration_cast<std::chrono::milliseconds>(now-_startTime).count();
			 int seconds=ep/1000;//已经经过的时间
			 int s=seconds%3600;
			 int m=seconds/60%60;
			 int h=seconds/3600;
		
			 int nokotaSeconds=seconds/percent-seconds;//剩余的时间
			 int ns=nokotaSeconds%3600;
			 int nm=nokotaSeconds/60%60;
			 int nh=nokotaSeconds/3600;

			 snprintf(buf+52+_title.size(),34+6,"[percent:%.1f|%2d:%2d:%2d=>%2d:%2d:%2d]",percent,nh,nm,ns,h,m,s);//seconds,nokotaSeconds);
			 buf[_title.size()+51+34+6+1]='\0';
			 buf2[_title.size()+51+34+6+1]='\0';
			 printf(buf2);
			 fputs(buf, stdout);
			 fflush(stdout);
			 it++;
		}
		printf("\n");
	}
public:
	ProgressReporter(uint64_t totalNum,const std::string& title):_totalNum(std::max((uint64_t)1,totalNum)),_title(title),_startTime(std::chrono::system_clock::now()){
		_doneNum=0;
		_threadExit=false;
		//开启更新进程
		_updateThread=std::thread([this](){
			this->PrintBar();
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
