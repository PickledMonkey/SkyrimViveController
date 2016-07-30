#include "SkyrimInputInjector.h"

SkyrimInputInjector::SkyrimInputInjector()
{
	eventClose = CreateEvent(NULL, TRUE, FALSE, NULL);
	threadsRunning = false;
}


SkyrimInputInjector::~SkyrimInputInjector()
{
	closeThreads();
	CloseHandle(eventClose);
}


void SkyrimInputInjector::startThreads()
{
	if (threadsRunning)
	{
		return;
	}

	threadsRunning = true;
	for (unsigned int i = 0; i < NUM_THREADS; i++)
	{
		_MESSAGE("Input thread created:%d", i);
		threadParameters[i].threadNum = i;
		threadParameters[i].injector = this;
		threadHandles[i] = CreateThread(NULL, 0, sendInputThreadFunction, &threadParameters[i], 0, &threadIds[i]);
		signalThreadOpen(i);
	}
}

void SkyrimInputInjector::closeThreads()
{
	if (!threadsRunning)
	{
		return;
	}

	SetEvent(eventClose);
	for (unsigned int i = 0; i < NUM_THREADS; i++)
	{
		WaitForSingleObject(threadHandles[i], 2000);
		CloseHandle(threadHandles[i]);
	}
	openThreads.reset();
	threadsRunning = false;
}


int SkyrimInputInjector::getOpenThread()
{
	int threadNum = -1;
	if (openThreads.get(threadNum))
	{
		openThreads.pop();
		return threadNum;
	}

	return rand() % NUM_THREADS;
}

void SkyrimInputInjector::signalThreadOpen(int threadNum)
{
	openThreads.push(threadNum);
}


DWORD WINAPI SkyrimInputInjector::sendInputThreadFunction(LPVOID param)
{
	threadArgs* args = (threadArgs*)param;
	_MESSAGE("Input thread doing stuff:%d", args->threadNum);
	ConcurrentQueue<TimedInput>* inQueue = &(args->inputQueue);
	_MESSAGE("Input thread queue found:%d", args->threadNum);
	SkyrimInputInjector* __this = (SkyrimInputInjector*)args->injector;
	_MESSAGE("Input thread this injector found:%d", args->threadNum);
	HANDLE runEvents[2] = { __this->eventClose, inQueue->getNewDataEventHandle() };
	_MESSAGE("Input thread events found:%d", args->threadNum);

	TimedInput currInput;
	DWORD status = 0;

	bool parsedInput = false;
	while (true)
	{
		
		status = WaitForMultipleObjects(2, runEvents, FALSE, INFINITE);
		switch (status) 
		{
		case (WAIT_OBJECT_0) : //close event
			return 0;
			break;
		case (WAIT_OBJECT_0 + 1) : //new data
			
			while (!inQueue->isEmpty())
			{
				if (inQueue->get(currInput))
				{
					inQueue->pop();
					SendInput(1, &currInput.inputData, sizeof(INPUT));
					Sleep(currInput.waitTimeAfterwards_ms);
					parsedInput = true;
				}
			}

			if (parsedInput)
			{
				parsedInput = false;
				__this->signalThreadOpen(args->threadNum);
			}
			
			break;
		default:
			return 1;
			break;
		}
	}

	return 0;
}

void SkyrimInputInjector::scrollMouseWheel(LONG wheelDelta)
{
	INPUT scrollEvent = { 0 };

	scrollEvent.type = 0;
	scrollEvent.mi.dwFlags = MOUSEEVENTF_WHEEL;
	scrollEvent.mi.dx = 0;
	scrollEvent.mi.dy = 0;
	scrollEvent.mi.mouseData = wheelDelta;

	SendInput(1, &scrollEvent, sizeof(INPUT));
}

void SkyrimInputInjector::leftClickMouse()
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };
	
	input.inputData.type = 0;
	input.inputData.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.waitTimeAfterwards_ms = SHORT_PRESS_TIME_MS;
	
	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}

void SkyrimInputInjector::leftClickMouseLong()
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };

	input.inputData.type = 0;
	input.inputData.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.waitTimeAfterwards_ms = LONG_PRESS_TIME_MS;

	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}

void SkyrimInputInjector::leftMouseButtonDown()
{
	INPUT clickEvent = { 0 };
	clickEvent.type = 0;
	clickEvent.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	SendInput(1, &clickEvent, sizeof(INPUT));
}

void SkyrimInputInjector::leftMouseButtonUp()
{
	INPUT clickEvent = { 0 };

	clickEvent.type = 0;
	clickEvent.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &clickEvent, sizeof(INPUT));
}

void SkyrimInputInjector::rightClickMouse()
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };

	input.inputData.type = 0;
	input.inputData.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	input.waitTimeAfterwards_ms = SHORT_PRESS_TIME_MS;

	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}

void SkyrimInputInjector::rightClickMouseLong()
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };

	input.inputData.type = 0;
	input.inputData.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	input.waitTimeAfterwards_ms = LONG_PRESS_TIME_MS;

	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}


void SkyrimInputInjector::rightMouseButtonDown()
{
	INPUT clickEvent = { 0 };

	clickEvent.type = 0;
	clickEvent.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

	SendInput(1, &clickEvent, sizeof(INPUT));
}


void SkyrimInputInjector::rightMouseButtonUp()
{
	INPUT clickEvent = { 0 };

	clickEvent.type = 0;
	clickEvent.mi.dwFlags = MOUSEEVENTF_RIGHTUP;

	SendInput(1, &clickEvent, sizeof(INPUT));
}


void SkyrimInputInjector::moveMouseRelative(LONG deltaX, LONG deltaY)
{
	currMouseCoord[x] += deltaX;
	currMouseCoord[y] += deltaY;

	INPUT moveEvent = { 0 };
	moveEvent.type = 0;
	moveEvent.mi.dx = deltaX;
	moveEvent.mi.dy = deltaY;
	SendInput(1, &moveEvent, sizeof(INPUT));
}


void SkyrimInputInjector::moveMouseAbsolute(LONG deltaX, LONG deltaY)
{
	currMouseCoord[x] = deltaX;
	currMouseCoord[y] = deltaY;

	INPUT moveEvent = { 0 };
	moveEvent.type = MOUSEEVENTF_ABSOLUTE;
	moveEvent.mi.dx = deltaX;
	moveEvent.mi.dy = deltaY;
	SendInput(1, &moveEvent, sizeof(INPUT));
}



void SkyrimInputInjector::tapKey(WORD keyCode)
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };

	input.inputData.type = INPUT_KEYBOARD;
	input.inputData.ki.wVk = keyCode;
	input.inputData.ki.dwFlags = 0;
	input.waitTimeAfterwards_ms = SHORT_PRESS_TIME_MS;

	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.ki.dwFlags = KEYEVENTF_KEYUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}

void SkyrimInputInjector::tapKeyLong(WORD keyCode)
{
	int threadNum = getOpenThread();

	TimedInput input = { 0 };

	input.inputData.type = INPUT_KEYBOARD;
	input.inputData.ki.wVk = keyCode;
	input.inputData.ki.dwFlags = 0;
	input.waitTimeAfterwards_ms = LONG_PRESS_TIME_MS;

	threadParameters[threadNum].inputQueue.push(input);

	input.inputData.ki.dwFlags = KEYEVENTF_KEYUP;
	input.waitTimeAfterwards_ms = 0;

	threadParameters[threadNum].inputQueue.push(input);
}

void SkyrimInputInjector::keyDown(WORD keyCode)
{
	INPUT keyEvent = { 0 };
	keyEvent.type = INPUT_KEYBOARD;
	//keyEvent.ki.wVk = keyCode;
	keyEvent.ki.wScan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
	keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE;

	SendInput(1, &keyEvent, sizeof(INPUT));
}

void SkyrimInputInjector::keyUp(WORD keyCode)
{
	INPUT keyEvent = { 0 };
	keyEvent.type = INPUT_KEYBOARD;
	//keyEvent.ki.wVk = keyCode;
	keyEvent.ki.wScan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
	keyEvent.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	
	SendInput(1, &keyEvent, sizeof(INPUT));
}


void SkyrimInputInjector::pressInput(InputSelections inState)
{
	if (buttonPressed[inState]){
		return;
	}

	switch (inState){
	case Mouse_Left:
		buttonPressed[Mouse_Left] = true;
		leftMouseButtonDown();
		break;
	case Mouse_Right:
		buttonPressed[Mouse_Right] = true;
		rightMouseButtonDown();
		break;
	case Key_E:
		buttonPressed[Key_E] = true;
		keyDown(E);
		break;
	case Key_R:
		buttonPressed[Key_R] = true;
		keyDown(R);
		break;
	case Key_Esc:
		buttonPressed[Key_Esc] = true;
		keyDown(Esc);
		break;
	case Key_Tab:
		buttonPressed[Key_Tab] = true;
		keyDown(Tab);
		break;
	case Key_F:
		buttonPressed[Key_F] = true;
		keyDown(F);
		break;
	case Key_W:
		buttonPressed[Key_W] = true;
		keyDown(W);
		break;
	case Key_A:
		buttonPressed[Key_A] = true;
		keyDown(A);
		break;
	case Key_S:
		buttonPressed[Key_S] = true;
		keyDown(S);
		break;
	case Key_D:
		buttonPressed[Key_D] = true;
		keyDown(D);
		break;
	case Key_Q:
		buttonPressed[Key_Q] = true;
		keyDown(Q);
		break;
	case Key_Z:
		buttonPressed[Key_Z] = true;
		keyDown(Z);
		break;
	case Key_C:
		buttonPressed[Key_C] = true;
		keyDown(C);
	case Key_Shift:
		buttonPressed[Key_Shift] = true;
		keyDown(Shift);
		break;
	case Key_Alt:
		buttonPressed[Key_Alt] = true;
		keyDown(Alt);
		break;
	}
}

void SkyrimInputInjector::unpressInput(InputSelections inState)
{
	if (!buttonPressed[inState]){
		return;
	}

	switch (inState){
	case Mouse_Left:
		buttonPressed[Mouse_Left] = false;
		leftMouseButtonUp();
		break;
	case Mouse_Right:
		buttonPressed[Mouse_Right] = false;
		rightMouseButtonUp();
		break;
	case Key_E:
		buttonPressed[Key_E] = false;
		keyUp(E);
		break;
	case Key_R:
		buttonPressed[Key_R] = false;
		keyUp(R);
		break;
	case Key_Esc:
		buttonPressed[Key_Esc] = false;
		keyUp(Esc);
		break;
	case Key_Tab:
		buttonPressed[Key_Tab] = false;
		keyUp(Tab);
		break;
	case Key_F:
		buttonPressed[Key_F] = false;
		keyUp(F);
		break;
	case Key_W:
		buttonPressed[Key_W] = false;
		keyUp(W);
		break;
	case Key_A:
		buttonPressed[Key_A] = false;
		keyUp(A);
		break;
	case Key_S:
		buttonPressed[Key_S] = false;
		keyUp(S);
		break;
	case Key_D:
		buttonPressed[Key_D] = false;
		keyUp(D);
		break;
	case Key_Q:
		buttonPressed[Key_Q] = false;
		keyUp(Q);
		break;
	case Key_Z:
		buttonPressed[Key_Z] = false;
		keyUp(Z);
		break;
	case Key_C:
		buttonPressed[Key_C] = false;
		keyUp(C);
		break;
	case Key_Shift:
		buttonPressed[Key_Shift] = false;
		keyUp(Shift);
		break;
	case Key_Alt:
		buttonPressed[Key_Alt] = false;
		keyUp(Alt);
		break;
	}
}

bool SkyrimInputInjector::isPressed(InputSelections input)
{
	return buttonPressed[input];
}