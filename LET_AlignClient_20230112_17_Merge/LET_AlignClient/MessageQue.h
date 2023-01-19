
#pragma once

#include "Infra/Guard.h"
#include "Infra/Semaphore.h"
#include <list>

template<typename T>
class TMessageQue
{
public:
	
	void push_back(T const& msg)
	{
		Infra::CGuard g(_mutex);
		_list.push_back(msg);
		_sem.post();
	}
	
	void get(T& msg)
	{
		_mutex.enter();
		while (_list.empty())
		{
			_mutex.leave();
			_sem.pend();
			_mutex.enter();
		}

		msg = _list.front();
		_list.pop_front();
		_mutex.leave();
	}
	
	bool get(T& msg, int timeout)
	{
		_mutex.enter();
		while (_list.empty())
		{
			_mutex.leave();
			if (_sem.pend(timeout) < 0)
			{
				return false;
			}
			_mutex.enter();
		}
		
		msg = _list.front();
		_list.pop_front();
		_mutex.leave();
		return true;
	}
	
private:
	
	Dahua::Infra::CSemaphore _sem;
	Dahua::Infra::CMutex     _mutex;
	std::list<T>             _list;
};
