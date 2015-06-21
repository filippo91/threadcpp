#pragma once

#include "BlockingQueue.h"
#include <future>
#include <thread>
#include <memory>

using namespace std;

template <class T>
class ThreadPool
{
private:
	BlockingQueue< shared_ptr<packaged_task<T(void)>> > bq;

public:
	ThreadPool(unsigned nthread);
	~ThreadPool(void);
	void addTask(packaged_task<T(void)>&& t);
	void close();
};

template <class T>
ThreadPool<T>::ThreadPool(unsigned nthread)
{
	for(unsigned i=0; i < nthread; i++){
		thread t([this](){
			while(1){
			shared_ptr<packaged_task<T(void)>> p;
			if( !bq.popItem(p) )return;
			p->make_ready_at_thread_exit();
			}
		});
		t.detach();
	}
}

template <class T>
ThreadPool<T>::~ThreadPool(void)
{
}

template <class T>
void ThreadPool<T>::addTask(packaged_task<T(void)>&& t)
{
	auto pointer = make_shared<packaged_task<T(void)>>(std::move(t));
	bq.addItem(pointer);
}

template <class T>
void ThreadPool<T>::close()
{
	bq.close();
}