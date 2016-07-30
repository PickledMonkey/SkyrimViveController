#pragma once

template<typename T>
class CircularBuffer
{
public:
	static const int BUFFER_CAPACITY = 50;

	CircularBuffer()
	{
		reset();
	}

	bool inBounds(int i)
	{
		return i < size;
	}

	T& getItem(int i)
	{
		int grabIndex = insertIndex - i-1;
		if (grabIndex < 0)
		{
			grabIndex += BUFFER_CAPACITY;
		}
		return bufferArray[grabIndex];
	}

	int getSize()
	{
		return size;
	}

	void pushItem(const T& newItem)
	{
		bufferArray[insertIndex] = newItem;
		if (size < BUFFER_CAPACITY)
		{
			size++;
		}
		insertIndex = (insertIndex + 1) % BUFFER_CAPACITY;
	}

	void reset()
	{
		size = 0;
		insertIndex = 0;
	}

private:

	T bufferArray[BUFFER_CAPACITY];

	int size = 0;
	int insertIndex = 0;


};