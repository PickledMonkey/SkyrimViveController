#pragma once

#include "Config.h"
#include "skse/GameReferences.h"

#define DEFACTIONQUOTE(name) #name
#define DEFACTION(name) void _##name##Action(TESObjectREFR * obj, Actor * actor, ProfileAction * action)
#if _DEBUG
#define PREACTION(name) Config::WriteDebug(2, DEFACTIONQUOTE(name)"Action()")
#define PREACTION2(name) Config::WriteDebug(2, DEFACTIONQUOTE(name)"Action("
#else
#define PREACTION(name)
#endif

DEFACTION(Set)
{
#if _DEBUG
	if(action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(Set)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Float)
		PREACTION2(Set)"%s, %f(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToFloat() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Bool)
		PREACTION2(Set)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToBool() ? 1 : 0) : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(Set)"%s, %s(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0);
	else
		PREACTION(Set);
#endif

	Config::_setValueInternal((ConfigValueNames)action->DataValueIndex, action->DataValueValue, action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(SetNoOverride)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(SetNoOverride)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Float)
		PREACTION2(SetNoOverride)"%s, %f(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToFloat() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Bool)
		PREACTION2(SetNoOverride)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToBool() ? 1 : 0) : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(SetNoOverride)"%s, %s(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0);
	else
		PREACTION(SetNoOverride);
#endif

	Config::_setValueInternal((ConfigValueNames)action->DataValueIndex, action->DataValueValue, 2000);
}

DEFACTION(Add)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(Add)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Float)
		PREACTION2(Add)"%s, %f(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToFloat() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Bool)
		PREACTION2(Add)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToBool() ? 1 : 0) : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(Add)"%s, %s(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0);
	else
		PREACTION(Add);
#endif

	if (!action->DataValueValue.HasValue)
		return;

	ConfigValue * cv = Config::_getValueInternal((ConfigValueNames)action->DataValueIndex);
	if (!cv)
		return;
	ValueInstance ci = cv->GetCurrentValue(false);
	if (!ci.HasValue || ci.Type != action->DataValueValue.Type)
		return;

	if (action->DataValueValue.Type == ConfigValueTypes::Bool)
	{
		bool prev = ci.ToBool();
		if (action->DataValueValue.ToBool())
			prev = !prev;

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), action->Profile != NULL ? action->Profile->Id : 2000);
		return;
	}

	if (action->DataValueValue.Type == ConfigValueTypes::Int)
	{
		int prev = ci.ToInt();
		prev += action->DataValueValue.ToInt();

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), action->Profile != NULL ? action->Profile->Id : 2000);
		return;
	}

	if (action->DataValueValue.Type == ConfigValueTypes::Float)
	{
		float prev = ci.ToFloat();
		prev += action->DataValueValue.ToFloat();

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), action->Profile != NULL ? action->Profile->Id : 2000);
		return;
	}
}

DEFACTION(AddNoOverride)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(AddNoOverride)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Float)
		PREACTION2(AddNoOverride)"%s, %f(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToFloat() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::Bool)
		PREACTION2(AddNoOverride)"%s, %d(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToBool() ? 1 : 0) : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(AddNoOverride)"%s, %s(%d))", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0);
	else
		PREACTION(AddNoOverride);
#endif

	if (!action->DataValueValue.HasValue)
		return;

	ConfigValue * cv = Config::_getValueInternal((ConfigValueNames)action->DataValueIndex);
	if (!cv)
		return;
	ValueInstance ci = cv->GetCurrentValue(false);
	if (!ci.HasValue || ci.Type != action->DataValueValue.Type)
		return;

	if (action->DataValueValue.Type == ConfigValueTypes::Bool)
	{
		bool prev = ci.ToBool();
		if (action->DataValueValue.ToBool())
			prev = !prev;

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), 2000);
		return;
	}

	if (action->DataValueValue.Type == ConfigValueTypes::Int)
	{
		int prev = ci.ToInt();
		prev += action->DataValueValue.ToInt();

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), 2000);
		return;
	}

	if (action->DataValueValue.Type == ConfigValueTypes::Float)
	{
		float prev = ci.ToFloat();
		prev += action->DataValueValue.ToFloat();

		Config::_setValueInternal((ConfigValueNames)cv->Id, ValueInstance(prev, true), 2000);
		return;
	}
}

DEFACTION(Return)
{
#if _DEBUG
	PREACTION2(Return)"%s)", Config::GetConfigValueName(action->DataValueIndex));
#endif

	Config::_unsetValueInternal((ConfigValueNames)action->DataValueIndex, action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(Enable)
{
#if _DEBUG
	PREACTION(Enable);
#endif

	Config::_setValueInternal(ConfigValueNames::bEnabled, ValueInstance(true, true), action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(Disable)
{
#if _DEBUG
	PREACTION(Disable);
#endif

	Config::_setValueInternal(ConfigValueNames::bEnabled, ValueInstance(false, true), action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(Activate)
{
#if _DEBUG
	PREACTION(Activate);
#endif

	Config::_setValueInternal(ConfigValueNames::bActivated, ValueInstance(true, true), action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(Deactivate)
{
#if _DEBUG
	PREACTION(Deactivate);
#endif

	Config::_setValueInternal(ConfigValueNames::bActivated, ValueInstance(false, true), action->Profile != NULL ? action->Profile->Id : 2000);
}

DEFACTION(Go3rdPerson)
{
#if _DEBUG
	PREACTION(Go3rdPerson);
#endif

	const int goFunc = 0x83C660;
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam)
		return;
	_asm
	{
		mov ecx, cam
		call goFunc
	}
}

DEFACTION(Force3rdPerson)
{
#if _DEBUG
	PREACTION(Force3rdPerson);
#endif

	const int goFunc = 0x83C6B0;
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam)
		return;
	_asm
	{
		mov ecx, cam
			call goFunc
	}
}

DEFACTION(Go1stPerson)
{
#if _DEBUG
	PREACTION(Go1stPerson);
#endif

	const int goFunc = 0x83CE50;
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam)
		return;
	_asm
	{
		mov ecx, cam
			call goFunc
	}
}

DEFACTION(Force1stPerson)
{
#if _DEBUG
	PREACTION(Force1stPerson);
#endif

	const int goFunc = 0x83CE90;
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam)
		return;
	_asm
	{
		mov ecx, cam
			call goFunc
	}
}

DEFACTION(SetProfile)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(SetProfile)"%d(%d), %f)", action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(SetProfile)"%s(%d), %f)", action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else
		PREACTION(SetProfile);
#endif

	if (!action->DataValueValue.HasValue)
		return;

	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		Config::_setProfile(action->DataValueValue.ToInt(), action->DataTweenLength);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		Config::_setProfile(action->DataValueValue.ToString(), action->DataTweenLength);
}

DEFACTION(RunProfile)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(RunProfile)"%d(%d))", action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(RunProfile)"%s(%d))", action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0);
	else
		PREACTION(RunProfile);
#endif

	if (!action->DataValueValue.HasValue)
		return;

	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		Config::_runProfile(action->DataValueValue.ToInt());
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		Config::_runProfile(action->DataValueValue.ToString());
}

DEFACTION(Inhibit)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(Inhibit)"%s(%d), %f)", action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else
		PREACTION(Inhibit);
#endif

	if (!action->DataValueValue.HasValue)
		return;

	if (action->DataValueValue.Type == ConfigValueTypes::String)
		Config::_inhibit(action->DataValueValue.ToString(), action->DataTweenLength);
}

DEFACTION(BindKey)
{
	if (action->DataValueIndex == 0 || !action->DataValueValue.HasValue)
	{
#if _DEBUG
		PREACTION(BindKey);
#endif
		return;
	}

	int modifiers = (action->DataValueIndex & 0x0FFF0000) >> 16;
	int key = (action->DataValueIndex & 0xFFFF);
	bool isDown = (action->DataValueIndex & 0xF0000000) == 0;

	// Bind must be queued because we can't resolve profile names during loading.
	// Actually nevermind I changed it since :P but too lazy to revert.

	if (action->DataValueValue.Type == ConfigValueTypes::Int)
	{
#if _DEBUG
		PREACTION2(BindKey)"%d, %d, %d(%d), %f)", isDown ? key : -key, modifiers, action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
#endif
		ToBind * bind = new ToBind();
		bind->Key = key;
		bind->Modifiers = modifiers;
		bind->Down = isDown;
		bind->Bind = true;
		bind->ProfileId = action->DataValueValue.ToInt();
		bind->Duration = action->DataTweenLength;

		Config::_queueBind(bind);
	}
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
	{
#if _DEBUG
		PREACTION2(BindKey)"%d, %d, %s(%d), %f)", isDown ? key : -key, modifiers, action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : "NULL", action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
#endif
		ToBind * bind = new ToBind();
		bind->Key = key;
		bind->Modifiers = modifiers;
		bind->Down = isDown;
		bind->Bind = true;
		bind->ProfileName = action->DataValueValue.ToString();
		bind->Duration = action->DataTweenLength;

		Config::_queueBind(bind);
	}
	else
	{
#if _DEBUG
		PREACTION(BindKey);
#endif
	}
}

DEFACTION(UnBindKey)
{
	if (action->DataValueIndex == 0)
	{
#if _DEBUG
		PREACTION(UnBindKey);
#endif
		return;
	}

	//int modifiers = (action->DataValueIndex & 0x0FFF0000) >> 16;
	int key = (action->DataValueIndex & 0xFFFF);
	bool isDown = (action->DataValueIndex & 0xF0000000) == 0;

#if _DEBUG
	PREACTION2(UnBindKey)"%d)", isDown ? key : -key);
#endif

	ToBind * bind = new ToBind();
	bind->Key = key;
	bind->Down = isDown;
	bind->Bind = false;

	Config::_queueBind(bind);
}

DEFACTION(Reload)
{
#if _DEBUG
	PREACTION(Reload);
#endif

	Config::_reloadFile();
}

DEFACTION(ReturnPerson)
{
#if _DEBUG
	PREACTION(ReturnPerson);
#endif

	if (action->Profile->RememberPerson == 1)
		_Go3rdPersonAction(obj, actor, action);
	else if (action->Profile->RememberPerson == 2)
		_Go1stPersonAction(obj, actor, action);
}

DEFACTION(AttachCamera)
{
#if _DEBUG
	PREACTION(AttachCamera);
#endif

	int formId = 0;

	// Get crosshair targeted ref.
	int refInfo = *((int*)0x1B115D8);
	if (refInfo != 0)
	{
		int refHandle = *((int*)(refInfo + 4));
		OBJRefHolder hld(refHandle);
		TESObjectREFR * refPtr = hld.GetRef();
		if (refPtr != NULL)
			formId = refPtr->formID;
	}

	if(formId == 0 || Camera::GetSingleton()->GetTargetOverwritten() == formId)
		Camera::GetSingleton()->SetTargetOverwrite(0);
	else
		Camera::GetSingleton()->SetTargetOverwrite(formId);
}

DEFACTION(DetachCamera)
{
#if _DEBUG
	PREACTION(DetachCamera);
#endif

	Camera::GetSingleton()->SetTargetOverwrite(0);
}

DEFACTION(AttachControl)
{
#if _DEBUG
	PREACTION(AttachControl);
#endif

	int formId = 0;

	// Get crosshair targeted ref.
	int refInfo = *((int*)0x1B115D8);
	if (refInfo != 0)
	{
		int refHandle = *((int*)(refInfo + 4));
		OBJRefHolder hld(refHandle);
		TESObjectREFR * refPtr = hld.GetRef();
		if (refPtr != NULL && refPtr->formType == kFormType_Character)
			formId = refPtr->formID;
	}

	if (formId == 0 || Camera::GetSingleton()->GetControlOverwritten() == formId)
		Camera::GetSingleton()->SetControlOverwrite(0, true);
	else
		Camera::GetSingleton()->SetControlOverwrite(formId, true);
}

DEFACTION(DetachControl)
{
#if _DEBUG
	PREACTION(DetachControl);
#endif

	Camera::GetSingleton()->SetControlOverwrite(0, true);
}

DEFACTION(RotateX)
{
	if (!action->DataValueValue.HasValue || action->DataValueValue.Type != ConfigValueTypes::Float)
	{
#if _DEBUG
		PREACTION(RotateX);
#endif
		return;
	}

#if _DEBUG
	PREACTION2(RotateX)"%f, %f, %d)", action->DataValueValue.ToFloat(), action->DataTweenLength, action->DataValueIndex);
#endif

	RotateInfo inf;
	inf.Axis = 0;
	inf.Amount = action->DataValueValue.ToFloat();
	inf.Time = action->DataTweenLength;
	if (action->DataValueIndex == 0 || action->DataValueIndex == 1 || action->DataValueIndex == 2)
		inf.OverwriteUser = action->DataValueIndex;

	Camera::GetSingleton()->Rotate(inf);
}

DEFACTION(RotateY)
{
	if (!action->DataValueValue.HasValue || action->DataValueValue.Type != ConfigValueTypes::Float)
	{
#if _DEBUG
		PREACTION(RotateY);
#endif
		return;
	}

#if _DEBUG
	PREACTION2(RotateY)"%f, %f, %d)", action->DataValueValue.ToFloat(), action->DataTweenLength, action->DataValueIndex);
#endif

	RotateInfo inf;
	inf.Axis = 1;
	inf.Amount = action->DataValueValue.ToFloat();
	inf.Time = action->DataTweenLength;
	if (action->DataValueIndex == 0 || action->DataValueIndex == 1 || action->DataValueIndex == 2)
		inf.OverwriteUser = action->DataValueIndex;

	Camera::GetSingleton()->Rotate(inf);
}

DEFACTION(SendAnimEvent)
{
	if (!action->DataValueValue.HasValue || action->DataValueValue.Type != ConfigValueTypes::String)
	{
#if _DEBUG
		PREACTION(SendAnimEvent);
#endif
		return;
	}

#if _DEBUG
	PREACTION2(SendAnimEvent)"%s)", action->DataValueValue.ToString());
#endif

	if (!actor)
		return;

	StringHolder bs(action->DataValueValue.ToString());
	BSFixedString * bf = bs.Get();
	_asm
	{
		pushad
			pushfd
			mov ecx, actor
			lea ecx, [ecx+0x20]
			mov edx, [ecx]
			mov edx, [edx+4]
			mov eax, bf
			push eax
			call edx
			popfd
			popad
	}
}

DEFACTION(SetValueDelayed)
{
#if _DEBUG
	if (action->DataValueValue.Type == ConfigValueTypes::Int)
		PREACTION2(SetDelayed)"%s, %d(%d), %f)", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToInt() : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else if (action->DataValueValue.Type == ConfigValueTypes::Float)
		PREACTION2(SetDelayed)"%s, %f(%d), %f)", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? action->DataValueValue.ToFloat() : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else if (action->DataValueValue.Type == ConfigValueTypes::Bool)
		PREACTION2(SetDelayed)"%s, %d(%d), %f)", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToBool() ? 1 : 0) : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else if (action->DataValueValue.Type == ConfigValueTypes::String)
		PREACTION2(SetDelayed)"%s, %s(%d), %f)", Config::GetConfigValueName(action->DataValueIndex), action->DataValueValue.HasValue ? (action->DataValueValue.ToString() ? action->DataValueValue.ToString() : "NULL") : 0, action->DataValueValue.HasValue ? 1 : 0, action->DataTweenLength);
	else
		PREACTION(SetDelayed);
#endif

	Config::_setValueInternal((ConfigValueNames)action->DataValueIndex, action->DataValueValue, action->Profile != NULL ? action->Profile->Id : 2000, action->DataTweenLength);
}

DEFACTION(ReturnValueDelayed)
{
#if _DEBUG
	PREACTION2(Return)"%s, %f)", Config::GetConfigValueName(action->DataValueIndex), action->DataTweenLength);
#endif

	Config::_unsetValueInternal((ConfigValueNames)action->DataValueIndex, action->Profile != NULL ? action->Profile->Id : 2000, action->DataTweenLength);
}
