/*
 * parallel.cpp
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */
#include "parallel.h"
#include "geometry.h"
#include "stats.h"
static std::vector<std::thread> threads; //线程池,存放工作线程
static bool shutdownThreads = false;
class ParallelForLoop;
static ParallelForLoop *workList = nullptr; //工作队列
static std::mutex workListMutex; //工作队列互斥锁
static std::condition_variable workListCondition;
thread_local int ThreadIndex = 0;

//代表一个循环体
class ParallelForLoop {
public:
	std::function<void(int64_t)> func1D;
	std::function<void(Point2i)> func2D;
	const int64_t maxIndex; //最大迭代次数
	const int chunkSize;  	//单次迭代执行的最大迭代次数
	int64_t nextIndex;    	//下个迭代索引
	int activeWorkers;		//当前激活worker线程
	ParallelForLoop * next; //下一个循环体
	int numX = -1; //x维上面的个数,二维遍历下才有效的变量
public:
	ParallelForLoop(std::function<void(int64_t)> func1D, int64_t maxIndex,
			int chunkSize) :
			func1D(std::move(func1D)), maxIndex(maxIndex), chunkSize(chunkSize) {
		nextIndex=0;
		activeWorkers=0;
		func2D=nullptr;
		next=nullptr;
	}

	ParallelForLoop(std::function<void(Point2i)> func2D,const Point2i& count):
		func2D(std::move(func2D)),maxIndex(count.x*count.y),chunkSize(1),numX(count.x){
				nextIndex=0;
				activeWorkers=0;
				func1D=nullptr;
				next=nullptr;
	}

	//只有当索引大于最大索引并且没有active的工作线程的时候，才算结束
	bool Finished() const {
		return (nextIndex >= maxIndex) && (activeWorkers == 0);
	}

};
Options RaidenOptions;
//返回系统中包含的核心数
int NumSystemCores() {
	if(RaidenOptions.numThread==0){
		return std::max(1u, std::thread::hardware_concurrency());
	}
	return RaidenOptions.numThread;
}

void WorkerThreadFunc(int tIndex/*线程索引*/) {
	ThreadIndex=tIndex;

	//std::cout<<"启动工作线程 "<<ThreadIndex<<std::endl;
	//工作线程永远处于循环状态
	std::unique_lock<std::mutex> lock(workListMutex);
	while (!shutdownThreads) {
		if (!workList) {
			workListCondition.wait(lock); //没有事情做，就睡眠等待唤醒
		} else {
			ParallelForLoop& loop = *workList;
			//获得遍历区间
			int64_t startIndex = loop.nextIndex;
			int64_t endIndex = std::min(loop.nextIndex + loop.chunkSize,
					loop.maxIndex);
			loop.nextIndex = endIndex;
			if (loop.nextIndex == loop.maxIndex) {
				workList = loop.next; //这个循环体已经结束了，换下个循环体
			}
			++loop.activeWorkers;
			lock.unlock();
			//遍历区间内的循环体
			for (int64_t i = startIndex; i < endIndex; ++i) {
				if (loop.func1D) {
					loop.func1D(i);
				} else {
					loop.func2D(Point2i(i % loop.numX, i / loop.numX));
				}
			}
			//一个区间的工作完成
			lock.lock();
			--loop.activeWorkers;
			//这里实在想不通为啥要再唤醒一次工作线程
			if (loop.Finished()) {
				workListCondition.notify_all();
			}
		}
	}
	ReportThreadStats();
	//Info("work thread" <<ThreadIndex<<" over ");

}

void ParallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize) {
	Assert(count > 0);
	//首先处理单核以及遍历数非常小的情况
	if (NumSystemCores() == 1 || count < chunkSize) {
		for (int64_t i = 0; i < count; ++i) {
			func(i);
		}
		return;
	}
	//创建一个局部循环体
	//因为这个函数需要等所有遍历都结束后才返回，所以用局部变量也没啥
	ParallelForLoop loop(std::move(func), count, chunkSize);
	//访问工作队列
	workListMutex.lock();
	loop.next = workList;
	workList = &loop;
	workListMutex.unlock();
	//再次获取锁
	std::unique_lock<std::mutex> lock(workListMutex);
	workListCondition.notify_all();	//唤醒所有在等待的工作线程
	while (!loop.Finished()) {
		//获得遍历区间
		int64_t startIndex = loop.nextIndex;
		int64_t endIndex = std::min(loop.nextIndex + loop.chunkSize, loop.maxIndex);
		loop.nextIndex = endIndex;
		if (loop.nextIndex == loop.maxIndex) {
			workList = loop.next;	//这个循环体已经结束了，换下个循环体
		}
		++loop.activeWorkers;
		lock.unlock();
		//遍历区间内的循环体
		for (int64_t i = startIndex; i < endIndex; ++i) {
			if (loop.func1D) {
				loop.func1D(i);
			} else {
				loop.func2D(Point2i(i % loop.numX, i / loop.numX));
			}
		}
		//一个区间的工作完成
		lock.lock();
		--loop.activeWorkers;
	}
}

void ParallelFor2D(std::function<void(Point2i)> func,const Point2i&count) {
	Assert(count.x> 0||count.y>0);
	//首先处理单核
	if (NumSystemCores() == 1) {
		for(int y=0;y<count.y;++y){
			for(int x=0;x<count.x;++x){
				func(Point2i(x,y));
			}
		}
		return;
	}
	//创建一个局部循环体
	//因为这个函数需要等所有遍历都结束后才返回，所以用局部变量也没啥
	ParallelForLoop loop(std::move(func), count);
	//访问工作队列
	workListMutex.lock();
	loop.next = workList;
	workList = &loop;
	workListMutex.unlock();
	//再次获取锁
	std::unique_lock<std::mutex> lock(workListMutex);
	workListCondition.notify_all();	//唤醒所有在等待的工作线程
	while (!loop.Finished()) {
		//获得遍历区间
		int64_t startIndex = loop.nextIndex;
		int64_t endIndex = std::min(loop.nextIndex + loop.chunkSize, loop.maxIndex);
		loop.nextIndex = endIndex;
		if (loop.nextIndex == loop.maxIndex) {
			workList = loop.next;	//这个循环体已经结束了，换下个循环体
		}
		++loop.activeWorkers;
		lock.unlock();
		//遍历区间内的循环体
		for (int64_t i = startIndex; i < endIndex; ++i) {
			if (loop.func1D) {
				loop.func1D(i);
			} else {
				loop.func2D(Point2i(i % loop.numX, i / loop.numX));
			}
		}
		//一个区间的工作完成
		lock.lock();
		--loop.activeWorkers;
	}
}

void ParallelInit(){
	int numThread=NumSystemCores();
	for(int i=0;i<numThread-1;++i){
		threads.push_back(std::thread(WorkerThreadFunc,i+1));
	}
}

void ParallelCleanup(){
	if(threads.size()==0){
		return;
	}
	{
		std::lock_guard<std::mutex> lock(workListMutex);
		shutdownThreads=true;
		workListCondition.notify_all();
	}
	//等待所有线程完成全部工作
	for(std::thread& t:threads){
		t.join();
	}
	//抹杀>_<
	threads.erase(threads.begin(),threads.end());
	shutdownThreads=false;//重置，准备下次再启动
}

int MaxThreadIndex() {
	if (RaidenOptions.numThread == 0) {
		return NumSystemCores();
	}
	return RaidenOptions.numThread;
}
