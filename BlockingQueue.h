#pragma once

#include "stdafx.h"

#include <limits> 
#include <mutex>
#include <condition_variable>
#include <list>
#include <atomic>

template<class T>
class BlockingQueue
{
private:
	std::mutex m;
	std::condition_variable cv;
	unsigned maxItem;
	std::atomic<bool> closed;
	std::list<T> l;

	BlockingQueue(const BlockingQueue&);
	BlockingQueue& operator =(const BlockingQueue&);

public:
	BlockingQueue(void){
		this->maxItem = UINT_MAX;
		closed = false;
	}

	BlockingQueue(int){
		this->maxItem = maxItem;
		closed = false;
	}

	~BlockingQueue(void){
	}

	void addItem(T t)
	{
		std::unique_lock<std::mutex> ul(m);

		cv.wait(ul, [this]()->bool{return l.size() < maxItem || closed == true;});

		if(closed==true){
			throw std::exception("The queue is closed");
		}

		l.push_back(t);	

		cv.notify_all();
	}

	bool popItem(T& retValue)
	{
		std::unique_lock<std::mutex> ul(m);


		cv.wait(ul, [this]()->bool{return l.size() > 0 || closed == true;});

		if (l.size()>0) {

			retValue = l.front();	

			l.pop_front();

			cv.notify_all();

			return true;
		} else {
			return false;
		}
	}

	void close(){
		/*
		std::lock_guard<std::mutex> lg(m);	
		closed = true;
		cv.notify_all();
		*/
		closed.store(true);
		cv.notify_all();
	}

	bool isClosed()
	{
		return closed.load();
	}
};


