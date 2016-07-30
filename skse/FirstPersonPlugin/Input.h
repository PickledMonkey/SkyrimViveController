#pragma once

#include <vector>
#include <unordered_map>

struct InputBind
{
	InputBind()
	{
		Key = 0;
		Modifiers = 0;
		IsDown = false;
		ProfileDown = -1;
		ProfileUp = -1;
		ProfileDurationDown = 0.0f;
		ProfileDurationUp = 0.0f;
	}

	int Key;
	int Modifiers;
	bool IsDown;
	int ProfileDown;
	int ProfileUp;
	float ProfileDurationDown;
	float ProfileDurationUp;
};

class InputHandler
{
	InputHandler();
	~InputHandler();

	std::vector<InputBind*> Binds;

	void DoKey(InputBind * bind, bool down);
	static bool IsKeyDown(int key);
#if _DEBUG
public:
#endif
	static bool IsKeyDown(int key, int modifiers);
#if _DEBUG
private:
#endif

	static InputHandler * Get();

public:
	static void Register(int key, int modifiers, int down, float downDur, int up, float upDur);
	static void Unregister(int key, bool down, bool up);
	static void Clear();
	static void Update();
};
