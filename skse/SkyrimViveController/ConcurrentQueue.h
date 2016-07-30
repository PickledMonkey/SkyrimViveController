#pragma once

#include <queue>


template<typename T>
class ConcurrentQueue
{
public:
	ConcurrentQueue()
	{
		InitializeCriticalSection(&dataLock);
		eventNewData = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~ConcurrentQueue()
	{
		CloseHandle(eventNewData);
		DeleteCriticalSection(&dataLock);
	}

	bool push(const T& newData)
	{
		EnterCriticalSection(&dataLock);
		dataQueue.push(newData);
		LeaveCriticalSection(&dataLock);

		SetEvent(eventNewData);

		return true;
	}

	bool get(T& outData)
	{
		bool retrieved = false;
		EnterCriticalSection(&dataLock);
		if (!dataQueue.empty()){
			memcpy(&outData, &dataQueue.front(), sizeof(T));
			retrieved = true;
		}
		LeaveCriticalSection(&dataLock);

		return retrieved;
	}

	bool pop()
	{
		bool popped = false;
		EnterCriticalSection(&dataLock);
		if (!dataQueue.empty()){
			dataQueue.pop();
			popped = true;
		}
		LeaveCriticalSection(&dataLock);

		return popped;
	}

	bool isEmpty()
	{
		bool empty = false;

		EnterCriticalSection(&dataLock);
		empty = dataQueue.empty();
		LeaveCriticalSection(&dataLock);

		return empty;
	}

	HANDLE getNewDataEventHandle()
	{
		return eventNewData;
	}

	void reset()
	{
		std::queue<T> newQueue;

		EnterCriticalSection(&dataLock);
		dataQueue = newQueue;
		LeaveCriticalSection(&dataLock);
	}

private:
	CRITICAL_SECTION dataLock;
	std::queue<T> dataQueue;
	HANDLE eventNewData;

};

