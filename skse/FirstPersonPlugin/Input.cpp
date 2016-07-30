#include "Input.h"
#include "Camera.h"
#include "Config.h"

InputHandler * InputHandler::Get()
{
	static InputHandler * ins = NULL;
	if (ins == NULL)
		ins = new InputHandler();
	return ins;
}

InputHandler::InputHandler()
{
}

InputHandler::~InputHandler()
{
	Clear();
}

void InputHandler::DoKey(InputBind * bind, bool down)
{
	if (bind->IsDown == down)
		return;

	bind->IsDown = down;
	if (down)
	{
		if (bind->ProfileDown >= 0)
		{
			if ((bind->Modifiers & 32) == 0)
				Config::_setProfile(bind->ProfileDown, bind->ProfileDurationDown);
			else
				Config::_runProfile(bind->ProfileDown);
		}
	}
	else
	{
		if (bind->ProfileUp >= 0)
		{
			if ((bind->Modifiers & 32) == 0)
				Config::_setProfile(bind->ProfileUp, bind->ProfileDurationUp);
			else
				Config::_runProfile(bind->ProfileUp);
		}
	}
}

void InputHandler::Register(int key, int modifiers, int down, float downDur, int up, float upDur)
{
	Unregister(key, down != 0, up != 0);
	 
	InputHandler * ih = Get();

	InputBind * bind = new InputBind();
	bind->Key = key;
	bind->Modifiers = modifiers;
	bind->ProfileUp = up;
	bind->ProfileDurationUp = upDur;
	bind->ProfileDown = down;
	bind->ProfileDurationDown = downDur;
	bind->IsDown = IsKeyDown(key, modifiers);

	ih->Binds.push_back(bind);
}

void InputHandler::Unregister(int key, bool down, bool up)
{
	InputHandler * ih = Get();

	for (std::vector<InputBind*>::iterator itr = ih->Binds.begin(); itr != ih->Binds.end();)
	{
		if ((*itr)->Key == key)
		{
			if (down)
				(*itr)->ProfileDown = 0;
			if (up)
			{
				if ((*itr)->IsDown)
					ih->DoKey(*itr, false);
				(*itr)->ProfileUp = 0;
			}
			if ((*itr)->ProfileDown == 0 && (*itr)->ProfileUp == 0)
			{
				delete *itr;
				itr = ih->Binds.erase(itr);
				continue;
			}
		}

		itr++;
	}
}

void InputHandler::Clear()
{
	InputHandler * ih = Get();

	while (!ih->Binds.empty())
		Unregister(ih->Binds.front()->Key, true, true);
}

void InputHandler::Update()
{
	{
		const int shouldProcess = 0x772A50;
		int procResult = 0;
		_asm
		{
			pushad
				pushfd
				mov ecx, 0x12E7454
				mov ecx, [ecx]
				call shouldProcess
				mov procResult, eax
				popfd
				popad
		}

		if ((procResult & 0xFF) == 0)
			return;
	}

	int gMain = *((int*)0x1B2E860);
	if (gMain == 0)
		return;

	int hwnd = *((int*)(gMain + 0xC));
	if (hwnd == 0 || GetActiveWindow() != (HWND)hwnd)
		return;

	InputHandler * ih = Get();

	for (std::vector<InputBind*>::iterator itr = ih->Binds.begin(); itr != ih->Binds.end(); itr++)
	{
		if ((*itr)->Key <= 0)
			continue;

		bool isDown = IsKeyDown((*itr)->Key, (*itr)->Modifiers);
		ih->DoKey(*itr, isDown);
	}
}

bool InputHandler::IsKeyDown(int key)
{
	int kFunc = *((int*)0x106B2BC);
	int res = 0;
	_asm
	{
		mov eax, kFunc
			push key
			call eax
			mov res, eax
	}

	return (res & 0x8000) != 0;
}

bool InputHandler::IsKeyDown(int key, int modifiers)
{
	if (!IsKeyDown(key))
		return false;

	if ((modifiers & 1) != 0 && !IsKeyDown(0x11)) // Ctrl
		return false;
	if ((modifiers & 2) != 0 && !IsKeyDown(0x10)) // Shift
		return false;
	if ((modifiers & 4) != 0 && !IsKeyDown(0x12)) // Alt
		return false;

	return true;
}
