#pragma once

#include "StringHelper.h"
#include "Input.h"
#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <mutex>
#include <list>
#include "skse/GameReferences.h"

#define IFPV_VERSION 3.4f
#define IFPV_VERSION_I 34
#define IFPV_NAME "FirstPersonPlugin"

#define LOCK_MUTEX(a) std::lock_guard<std::recursive_mutex> _gMtx(a)

enum ConfigValueNames
{
	// Internal variables.
	bWalking = 0,
	bRunning,
	bSwimming,
	bSprinting,
	bWeaponOut,
	bDead,
	bKillMove,
	sRace,
	bWereWolf,
	bVampireLord,
	bDialogue,
	bMounted,
	bSneaking,
	bCrafting,
	bSitting,
	bJumping,
	bAimMagic,
	bAimMagicLeft,
	bAimMagicRight,
	bAimBow,
	bAimCrossbow,
	bAttackAnyHanded,
	bAttackAnyHandedLeft,
	bAttackAnyHandedRight,
	bAttackOneHanded,
	bAttackOneHandedLeft,
	bAttackOneHandedRight,
	bAttackTwoHanded,
	iCameraState,
	bMounting,
	bDismounting,
	bIsInIFPV,
	bIsActive,
	bIsTargetOverwritten,
	iObjectBaseFormId,
	bIndoors,
	bGrabbing,
	sFurniture,
	bIsCameraOnArrow,
	fLookDirX,
	fLookDirZ,

	// User variables.
	bEnabled,
	bActivated,
	iDebug,
	iDebug2,
	bHeadtrack,
	fPositionOffsetHorizontal,
	fPositionOffsetVertical,
	fPositionOffsetDepth,
	fRotationFromHead,
	fExtraRotationFromCamera,
	fNearClip,
	fFOV,
	fTimeScale,
	bDisableVATS,
	fRestrictAngleHorizontal,
	fRestrictAngleVertical,
	fUpdateProfileInterval,
	fCameraViewOffsetHorizontal,
	fCameraViewOffsetVertical,
	iFaceCrosshair,
	bZoomIntoIFPV,
	bToggleIntoIFPV,
	iUnused1,
	iUnused2,
	iUnused3,
	iUnused4,
	iUnused5,
	iUnused6,
	iUnused7,
	iUnused8,
	iUnused9,
	iUnused10,
	bFocus,
	fFocusSpeed,
	fFocusMinAngle,
	iFocusDelay,
	iObjectRefFormId,
	fExtraRotationMouse,
	fTweenLength,
	bSendModEvents,
	sCameraNode,
	bHideHead,
	bHideHelmet,
	bArrowCamera,
	iIgnoreWorldCollision,
	fUpDownLimit,
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
	fAdjustFarClipMinRatio,
	fAdjustFarClipStart,
#endif

	MaxValueNames
};

enum ConfigValueTypes
{
	Bool = 0,
	Int,
	Float,
	String,
	InvalidType,
};

enum ProfileConditionOperators
{
	Equals = 0,
	NotEquals,
	Greater,
	Less,
	GreaterOrEqual,
	LessOrEqual,
	Contains,
	NotContains,
};

enum ProfileActionTypes
{
	Disable = 0,
	Enable,
	Activate,
	Deactivate,
	GoThirdPerson,
	ForceThirdPerson,
	GoFirstPerson,
	ForceFirstPerson,
	SetValue,
	AddValue,
	ReturnValue,
	SetProfile,
	RunProfile,
	BindKey,
	UnBindKey,
	BindKey2,
	Reload,
	Inhibit,
	ReturnPerson,
	AttachCamera,
	DetachCamera,
	AttachControl,
	DetachControl,
	RotateX,
	RotateY,
	SendAnimEvent,
	SetValueDelayed,
	ReturnValueDelayed,
	SetValueNoOverride,
	AddValueNoOverride,
};

enum ProfileActionTriggers
{
	ProfileActivate = 0,
	ProfileDeactivate,
	ProfileInitialize,
};

struct OBJRefHolder
{
	OBJRefHolder(int refHandle)
	{
		_obj = NULL;
		int invHandle = *((int*)0x1310630);

		if (refHandle != invHandle && refHandle != 0)
		{
			TESObjectREFR * refPtr = NULL;

			const int lookupRef = 0x4951F0;

			_asm
			{
				pushad
					pushfd
					lea eax, refPtr
					push eax
					lea eax, refHandle
					push eax
					call lookupRef
					add esp, 8
					popfd
					popad
			}

			if (refPtr != NULL)
				_obj = refPtr;
		}
	}

	~OBJRefHolder()
	{
		if (_obj != NULL)
		{
			TESObjectREFR * refPtr = _obj;
			_obj = NULL;
			const int decRefFunc = 0x40B110;
			_asm
			{
				pushad
					pushfd
					lea ecx, refPtr
					call decRefFunc
					popfd
					popad
			}
		}
	}

	TESObjectREFR * GetRef()
	{
		return _obj;
	}

private:
	TESObjectREFR * _obj;
};

struct StringHolder
{
	StringHolder(const char * str)
	{
		stringRef = new BSFixedString(str);
	}

	~StringHolder()
	{
		const int releaseFunc = 0xA511B0;
		BSFixedString * strr = stringRef;
		_asm
		{
			pushad
				pushfd
				mov ecx, strr
				call releaseFunc
				popfd
				popad
		}

		delete stringRef;
	}

	BSFixedString * Get() { return stringRef; }

private:
	BSFixedString * stringRef;
};

struct ToBind
{
	ToBind()
	{
		Key = 0;
		Modifiers = 0;
		Down = true;
		Bind = true;
		ProfileId = -1;
		Duration = 0.0f;
	}

	int Key;
	int Modifiers;
	bool Down;
	bool Bind;
	std::string ProfileName;
	int ProfileId;
	float Duration;
};

struct ValueInstance
{
	ValueInstance(ValueInstance& data)
	{
		HasValue = data.HasValue;
		Data = data.Data;
		Type = data.Type;
		_data = data._data;
	}

	ValueInstance()
	{
		HasValue = false;
		Data = 0;
		Type = ConfigValueTypes::Bool;
	}

	ValueInstance(bool val, bool has)
	{
		Data = val ? 1 : 0;
		HasValue = has;
		Type = ConfigValueTypes::Bool;
	}

	ValueInstance(int val, bool has)
	{
		Data = val;
		HasValue = has;
		Type = ConfigValueTypes::Int;
	}

	ValueInstance(float val, bool has)
	{
		Data = *((int*)&val);
		HasValue = has;
		Type = ConfigValueTypes::Float;
	}

	ValueInstance(const char * val, bool has)
	{
		Data = 0;
		_data = val;
		HasValue = has;
		Type = ConfigValueTypes::String;
	}

	ValueInstance(std::string str, ConfigValueTypes type)
	{
		Data = 0;
		HasValue = false;
		Type = type;

		if (type == ConfigValueTypes::Bool)
		{
			bool v = false;
			if (StringHelper::ToBool(str, v))
			{
				HasValue = true;
				Data = v ? 1 : 0;
			}
		}
		else if (type == ConfigValueTypes::Int)
		{
			int v = 0;
			if (StringHelper::ToInt(str, v))
			{
				HasValue = true;
				Data = v;
			}
		}
		else if (type == ConfigValueTypes::Float)
		{
			float v = 0.0f;
			if (StringHelper::ToFloat(str, v))
			{
				HasValue = true;
				Data = *((int*)&v);
			}
		}
		else if (type == ConfigValueTypes::String)
		{
			HasValue = true;
			_data = str;
		}
	}

	int Data;
	bool HasValue;
	ConfigValueTypes Type;
	std::string _data;

	bool ToBool()
	{
		return Data != 0;
	}

	float ToFloat()
	{
		return *((float*)&Data);
	}

	int ToInt()
	{
		return Data;
	}

	const char * ToString()
	{
		return _data.c_str();
	}
};

struct ProfileAction;
struct ProfileCondition;
struct ConfigValue;
typedef bool(*UpdateCondition)(TESObjectREFR * obj, Actor * actor, ProfileCondition * con);
typedef void(*UpdateInternalValue)(TESObjectREFR * obj, Actor * actor, ConfigValue * val);
typedef void(*UpdateAction)(TESObjectREFR * obj, Actor * actor, ProfileAction * action);

struct Profile;

struct ProfileAction
{
	ProfileAction()
	{
		Type = ProfileActionTypes::Activate;
		Trigger = ProfileActionTriggers::ProfileActivate;
		DataValueIndex = -1;
		DataTweenLength = 0.0f;
		Func = NULL;
		Profile = NULL;
	}

	ProfileActionTypes Type;
	ProfileActionTriggers Trigger;
	int DataValueIndex;
	ValueInstance DataValueValue;
	float DataTweenLength;
	UpdateAction Func;
	Profile * Profile;
};

struct ProfileCondition
{
	ProfileCondition()
	{
		Operation = ProfileConditionOperators::Equals;
		DataValueIndex = -1;
		Status = 0;
		Func = NULL;
	}

	ProfileConditionOperators Operation;
	int DataValueIndex;
	ValueInstance DataValueValue;
	int Status;
	UpdateCondition Func;
};

struct Profile
{
	Profile()
	{
		Id = 0;
		Inhibited = 0.0f;
		Skip = 0;
		Main = false;
		MainData = 0;
		RememberPerson = 0;
		Type = 0;
	}

	~Profile()
	{
	}

	int Id;
	std::string Name;
	std::vector<ProfileCondition*> Conditions;
	std::vector<ProfileAction*> Actions;
	std::set<ConfigValue*> Modified;
	float Inhibited;
	int RememberPerson;
	int Skip;
	bool Main;
	int MainData;
	int Type;
};

struct TweenData
{
	TweenData(float prev, float next, float time)
	{
		PreviousValue = prev;
		NextValue = next;
		TimeDone = 0.0f;
		TimeTotal = time;
	}

	TweenData(TweenData * prev, float next, float time)
	{
		PreviousValue = prev->GetFloatValue();
		NextValue = next;
		TimeDone = 0.0f;
		TimeTotal = time;
	}

	float PreviousValue;
	float NextValue;
	float TimeDone;
	float TimeTotal;

	float GetFloatValue()
	{
		float scale = TimeDone / TimeTotal;
		if (scale < 0.0f)
			scale = 0.0f;
		else if (scale > 1.0f)
			scale = 1.0f;
		return (NextValue - PreviousValue) * scale + PreviousValue;
	}
};

struct DelayedSet
{
	ConfigValueNames Id;
	float Delay;
	ValueInstance Value;
	bool Return;
	int Profile;
};

typedef void(*_GameSetter)(ValueInstance vi);
typedef ValueInstance(*_GameGetter)();

#define CONFIG_VALUE_FLAGS_USER_SET 1
#define CONFIG_VALUE_FLAGS_TWEEN 2
#define CONFIG_VALUE_FLAGS_SETTERALWAYS 4
#define CONFIG_VALUE_FLAGS_SETTERALWAYS_NOT_ONE 8

struct ConfigValue
{
	ConfigValue(int id, const char * name, int flags, UpdateInternalValue func, _GameGetter getter, _GameSetter setter)
	{
		ConditionCount = 0;
		Func = func;
		Id = id;
		Name = name;
		Flags = flags;
		switch (Name[0])
		{
		case 'f': Type = ConfigValueTypes::Float; break;
		case 'b': Type = ConfigValueTypes::Bool; break;
		case 'i': Type = ConfigValueTypes::Int; break;
		case 's': Type = ConfigValueTypes::String; break;
		default: Type = ConfigValueTypes::InvalidType; break;
		}
		Tween = NULL;
		Getter = getter;
		Setter = setter;
		ValueChanged = false;
		IsDefaultValueFromGame = false;
	}

	~ConfigValue()
	{
		if (Tween != NULL)
			delete Tween;
	}

	int Id;
	const char * Name;
	ConfigValueTypes Type;
	std::map<int, ValueInstance> Values;
	TweenData * Tween;
	UpdateInternalValue Func;
	int Flags;
	int ConditionCount;
	_GameGetter Getter;
	_GameSetter Setter;
	ValueInstance DefaultValue;
	bool ValueChanged;
	bool IsDefaultValueFromGame;

	bool ThisAddValueWouldAffect(int profileId)
	{
		return Values.empty() || Values.begin()->first > profileId;
	}

	bool ThisRemoveValueWouldAffect(int profileId)
	{
		return !Values.empty() && Values.begin()->first == profileId;
	}

	ValueInstance GetCurrentValue(bool allowTween)
	{
		UpdateDefaultValue();

		if (allowTween && Tween != NULL && Type == ConfigValueTypes::Float)
			return ValueInstance(Tween->GetFloatValue(), true);

		if (!Values.empty())
			return Values.begin()->second;

		if (DefaultValue.HasValue)
			return DefaultValue;

		return ValueInstance();
	}

	void UpdateDefaultValue()
	{
		if (!DefaultValue.HasValue && Getter != NULL)
		{
			DefaultValue = Getter();
			if (DefaultValue.HasValue)
				IsDefaultValueFromGame = true;
		}
	}
};

struct ConfigLoadingData
{
	ConfigLoadingData()
	{
		ProfileLine = -1;
		ProfileType = -1;
	}

	int ProfileLine;
	std::string ProfileName;
	std::vector<std::pair<std::string, int>> ProfileConditions;
	std::vector<std::pair<std::string, int>> ProfileActions;
	int ProfileType;
};

class Config
{
	Config();
	~Config();

	bool Load();
	void Parse(std::string line);
	void WriteError(int line, const char * fmt, ...);
	void WriteNormal(const char * fmt, ...);
	static bool WriteConsoleGame(const char * msg);
	std::string Trim(char * line, int index, int length);
	void WriteOutLoading(bool finished = false);
	ProfileCondition * ParseCondition(std::string str, int lineIndex);
	ProfileAction * ParseAction(Profile * prof, std::string str, int lineIndex);

	void UpdateProfileValues();
	void CalculateActiveProfile(int type = -1);
	void SetActiveProfile(Profile * prof, float forcedTime, bool manualSet);
	void DoAction(Profile * profile, ProfileAction * action);
	bool Test(Profile * profile, ProfileCondition * con);
	void DoProfileActions(Profile * profile, ProfileActionTriggers trigger);
	bool IsProfileConditionsOk(Profile * prof, bool allowNoConditions);
	
	void SetEntering(Profile * prof);
	void SetLeaving(Profile * prof);

	Profile * GetProfile(const char * name);
	Profile * GetProfile(int id);
	ConfigValue * GetValue(const char * name);
	void CreateValue(int id, const char * name, int flags, UpdateInternalValue func = NULL, _GameGetter getter = NULL, _GameSetter setter = NULL);
	void ProcessBindQueue();

	void UpdateConfigValue(ConfigValue * val, float diff);

	std::vector<Profile*> Profiles;
	std::vector<ConfigValue*> Values;
	ConfigLoadingData * Loading;
	int CurrentLine;
	std::vector<std::string> ConsoleQueue;
	std::vector<ToBind*> BindQueue;
	float ProfileUpdateTimer;
	Profile * ActiveProfile;
	float ActiveProfileForced;
	Profile * DefaultProfile;
	TESObjectREFR * _nsObj;
	Actor * _nsActor;
	int _iDebug;
	int _loading;
	int _reload;
	int _setting;
	int _initializingProfile;
	Profile * _enteringProfile;
	Profile * _leavingProfile;
	Profile * _setting2;
	float _mounting;
	float _dismounting;
	bool _activeSpecial;
	std::list<DelayedSet*> _delayedSet;

public:
	static bool IsProfileActive(const char * name);
	static void Update();
	static bool GetFloat(ConfigValueNames id, float& value);
	static bool GetInt(ConfigValueNames id, int& value);
	static bool GetBool(ConfigValueNames id, bool& value);
	static bool GetString(ConfigValueNames id, std::string& value);
	static void Initialize();
	static void _setProfile(int id, float duration = -1.0f);
	static void _setProfile(const char * name, float duration = -1.0f);
	static void _runProfile(int id);
	static void _runProfile(const char * name);
	static void _setValueInternal(ConfigValueNames id, ValueInstance vi, int profileId, float delay = 0.0f);
	static void _unsetValueInternal(ConfigValueNames id, int profileId, float delay = 0.0f);
	static ConfigValue * _getValueInternal(ConfigValueNames id);
	static void _queueBind(ToBind * bind);
	static void _reloadFile();
	static void _inhibit(const char * name, float duration);
	static int _getDebug();
	static void WriteDebug(int level, const char * fmt, ...);
	static void _setActive(bool active);
	static bool _isActive();
	static int _ShouldUpdateGraphics();
	static void _setUpdateGraphics();
	static void _mount(Actor * actor);
	static void _dismount(Actor * actor);
	static void _getMounting(bool& mount, bool& dismount);
	static void _toggleActive();
	static const char * GetConfigValueName(int id);
	static void SetModEventDispatcher(void * dispatcher);
	static void SendModEvent(const char * name, std::string strArg = "", float numArg = 0.0f);
	static ConfigValue * _getValueInternal(const char * name);
	static void ResetPapyrus();
};
