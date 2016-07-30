#ifndef _PAPYRUS_COMMANDS_H
#define _PAPYRUS_COMMANDS_H

#include "../skse/PapyrusVM.h"
#include "../skse/PapyrusNativeFunctions.h"
#include "../skse/GameReferences.h"

#include "Config.h"
#include "Camera.h"

#define PAPYRUS_PROFILE_ID 1995

struct IFPVPapyrus
{
	static float i_GetVersion(StaticFunctionTag * base)
	{
		return IFPV_VERSION;
	}

	static bool i_IsInIFPV(StaticFunctionTag * base)
	{
		return Config::_isActive();
	}

	static bool i_IsProfileActive(StaticFunctionTag * base, BSFixedString name)
	{
		return name.data != NULL && Config::IsProfileActive(name.data);
	}

	static bool i_GetBoolValue(StaticFunctionTag * base, BSFixedString name, bool error)
	{
		if (!name.data)
			return error;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Bool)
			return error;

		ValueInstance vi = cfg->GetCurrentValue(true);
		if (vi.HasValue)
			return vi.ToBool();

		return error;
	}

	static SInt32 i_GetIntValue(StaticFunctionTag * base, BSFixedString name, SInt32 error)
	{
		if (!name.data)
			return error;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Int)
			return error;

		ValueInstance vi = cfg->GetCurrentValue(true);
		if (vi.HasValue)
			return vi.ToInt();

		return error;
	}

	static float i_GetFloatValue(StaticFunctionTag * base, BSFixedString name, float error)
	{
		if (!name.data)
			return error;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Float)
			return error;

		ValueInstance vi = cfg->GetCurrentValue(true);
		if (vi.HasValue)
			return vi.ToFloat();

		return error;
	}

	static BSFixedString i_GetStringValue(StaticFunctionTag * base, BSFixedString name, BSFixedString error)
	{
		if (!name.data)
			return error;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::String)
			return error;

		ValueInstance vi = cfg->GetCurrentValue(true);
		if (vi.HasValue)
		{
			BSFixedString result(vi.ToString());
			return result;
		}

		return error;
	}

	static bool i_SetBoolValue(StaticFunctionTag * base, BSFixedString name, bool value)
	{
		if (!name.data)
			return false;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Bool)
			return false;

		Config::_setValueInternal((ConfigValueNames)cfg->Id, ValueInstance(value, true), PAPYRUS_PROFILE_ID);
		return true;
	}

	static bool i_SetIntValue(StaticFunctionTag * base, BSFixedString name, SInt32 value)
	{
		if (!name.data)
			return false;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Int)
			return false;

		Config::_setValueInternal((ConfigValueNames)cfg->Id, ValueInstance(value, true), PAPYRUS_PROFILE_ID);
		return true;
	}

	static bool i_SetFloatValue(StaticFunctionTag * base, BSFixedString name, float value)
	{
		if (!name.data)
			return false;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::Float)
			return false;

		Config::_setValueInternal((ConfigValueNames)cfg->Id, ValueInstance(value, true), PAPYRUS_PROFILE_ID);
		return true;
	}

	static bool i_SetStringValue(StaticFunctionTag * base, BSFixedString name, BSFixedString value)
	{
		if (!name.data || !value.data)
			return false;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg || cfg->Type != ConfigValueTypes::String)
			return false;

		Config::_setValueInternal((ConfigValueNames)cfg->Id, ValueInstance(value.data, true), PAPYRUS_PROFILE_ID);
		return true;
	}

	static bool i_ReturnValue(StaticFunctionTag * base, BSFixedString name)
	{
		if (!name.data)
			return false;

		ConfigValue * cfg = Config::_getValueInternal(name.data);
		if (!cfg)
			return false;

		Config::_unsetValueInternal((ConfigValueNames)cfg->Id, PAPYRUS_PROFILE_ID);
		return true;
	}

	static UInt32 i_GetCameraTargetFormId(StaticFunctionTag * base)
	{
		UInt32 result = Camera::GetSingleton()->GetTargetOverwritten();
		if (result == 0)
			result = 0x14;
		return result;
	}

	static bool _registerFunctions(VMClassRegistry * registry)
	{
		const char * className = "IFPV";

		UInt32 fl = 0;
		{
			const char * methodName = "GetVersion";
			registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, float>(methodName, className, i_GetVersion, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "IsInIFPV";
			registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, bool>(methodName, className, i_IsInIFPV, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "IsProfileActive";
			registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, bool, BSFixedString>(methodName, className, i_IsProfileActive, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "GetBoolValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, bool>(methodName, className, i_GetBoolValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "GetIntValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, SInt32, BSFixedString, SInt32>(methodName, className, i_GetIntValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "GetFloatValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, float, BSFixedString, float>(methodName, className, i_GetFloatValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "GetStringValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString>(methodName, className, i_GetStringValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "SetBoolValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, bool>(methodName, className, i_SetBoolValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "SetIntValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, SInt32>(methodName, className, i_SetIntValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "SetFloatValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, float>(methodName, className, i_SetFloatValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "SetStringValue";
			registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, BSFixedString>(methodName, className, i_SetStringValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "ReturnValue";
			registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, bool, BSFixedString>(methodName, className, i_ReturnValue, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}
		{
			const char * methodName = "GetCameraTargetFormId";
			registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, UInt32>(methodName, className, i_GetCameraTargetFormId, registry));
			registry->SetFunctionFlags(className, methodName, fl);
		}

		return true;
	}
};

#endif
