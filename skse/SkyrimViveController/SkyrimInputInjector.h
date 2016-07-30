#pragma once

#include <stdint.h>

#include "ConcurrentQueue.h"

class SkyrimInputInjector
{
public:
	enum keyCodes
	{
		W = 0x57,
		A = 0x41,
		S = 0x53,
		D = 0x44,

		E = 0x45,
		Q = 0x51,
		R = 0x52,
		C = 0x43,
		Z = 0x5A,
		F = 0x46,

		Space = VK_SPACE,
		Shift = VK_SHIFT,
		Tab = VK_TAB,
		Ctrl = VK_CONTROL,
		Alt = VK_MENU,
		Esc = VK_ESCAPE
	};

	enum InputSelections{
		Mouse_Left = 0,
		Mouse_Right = 1,
		Key_E = 2,
		Key_R = 3,
		Key_Esc = 4,
		Key_Tab = 5,
		Key_F = 6,
		Key_W = 7,
		Key_A = 8,
		Key_S = 9,
		Key_D = 10,
		Key_Q = 11,
		Key_Z = 12,
		Key_C = 13,
		Key_Shift = 14,
		Key_Alt = 15
	};
	static const int numInputSelections = 16;

	SkyrimInputInjector();
	~SkyrimInputInjector();

	void startThreads();
	void closeThreads();

	void scrollMouseWheel(LONG wheelDelta);

	void leftClickMouse();
	void leftClickMouseLong();
	void leftMouseButtonDown();
	void leftMouseButtonUp();

	void rightClickMouse();
	void rightClickMouseLong();
	void rightMouseButtonDown();
	void rightMouseButtonUp();

	void moveMouseRelative(LONG deltaX, LONG deltaY);
	void moveMouseAbsolute(LONG deltaX, LONG deltaY);

	void tapKey(WORD keyCode);
	void tapKeyLong(WORD keyCode);
	void keyDown(WORD keyCode);
	void keyUp(WORD keyCode);

	void pressInput(InputSelections input);
	void unpressInput(InputSelections input);
	bool isPressed(InputSelections input);

private:
	static const unsigned int NUM_THREADS = 1;
	static const unsigned int SHORT_PRESS_TIME_MS = 100;
	static const unsigned int LONG_PRESS_TIME_MS = 1000;

	enum {
		x = 0,
		y = 1
	};

	struct TimedInput{
		INPUT inputData;
		uint32_t waitTimeAfterwards_ms;
	};

	struct threadArgs
	{
		SkyrimInputInjector* injector;
		int threadNum;
		ConcurrentQueue<TimedInput> inputQueue;
	};

	LONG currMouseCoord[2];

	bool threadsRunning = false;
	threadArgs threadParameters[NUM_THREADS];
	DWORD threadIds[NUM_THREADS];
	HANDLE threadHandles[NUM_THREADS];
	HANDLE eventClose;

	ConcurrentQueue<int> openThreads;

	int getOpenThread();
	void signalThreadOpen(int threadNum);
	static DWORD WINAPI sendInputThreadFunction(LPVOID param);

	bool buttonPressed[numInputSelections];
};

