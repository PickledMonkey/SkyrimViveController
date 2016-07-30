#include "Config.h"
#include "Camera.h"
#include "Conditions.h"
#include "Values.h"
#include "Actions.h"
#include "GameValues.h"
#include "Papyrus.h"
#include <ctype.h>
#include <climits>
#include "skse/GameReferences.h"
#include "skse/GameRTTI.h"
#include "skse/PapyrusEvents.h"

Config * cfgInstance = NULL;
std::recursive_mutex mtx;
void * g_modEventDispatcher = NULL;

std::tuple<ProfileActionTypes, const char*, UpdateAction> DefaultProfileActions[] =
{
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::SetValue, "Set", _SetAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::AddValue, "Modify", _AddAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::ReturnValue, "Return", _ReturnAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Enable, "Enable", _EnableAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Disable, "Disable", _DisableAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Activate, "Activate", _ActivateAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Deactivate, "Deactivate", _DeactivateAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::GoThirdPerson, "GoThirdPerson", _Go3rdPersonAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::GoFirstPerson, "GoFirstPerson", _Go1stPersonAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::ForceThirdPerson, "ForceThirdPerson", _Force3rdPersonAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::ForceFirstPerson, "ForceFirstPerson", _Force1stPersonAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::SetProfile, "SetProfile", _SetProfileAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::RunProfile, "RunProfile", _RunProfileAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Inhibit, "Inhibit", _InhibitAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::BindKey, "BindKey", _BindKeyAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::UnBindKey, "UnBindKey", _UnBindKeyAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::BindKey2, "BindKey2", _BindKeyAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Reload, "Reload", _ReloadAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::ReturnPerson, "ReturnPerson", _ReturnPersonAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::AttachCamera, "AttachCamera", _AttachCameraAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::DetachCamera, "DetachCamera", _DetachCameraAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::AttachControl, "AttachControl", _AttachControlAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::DetachControl, "DetachControl", _DetachControlAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::RotateX, "RotateX", _RotateXAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::RotateY, "RotateY", _RotateYAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::SendAnimEvent, "SendAnimEvent", _SendAnimEventAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::SetValueDelayed , "SetDelayed", _SetValueDelayedAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::ReturnValueDelayed, "ReturnDelayed", _ReturnValueDelayedAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::SetValueNoOverride, "SetNoOverride", _SetNoOverrideAction),
	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::AddValueNoOverride, "ModifyNoOverride", _AddNoOverrideAction),

	std::tuple<ProfileActionTypes, const char*, UpdateAction>(ProfileActionTypes::Activate, NULL, NULL)
};

Config::Config()
{
	CurrentLine = 0;
	ProfileUpdateTimer = 0.0f;
	ActiveProfile = NULL;
	ActiveProfileForced = 0.0f;
	DefaultProfile = NULL;
	_nsObj = NULL;
	_nsActor = NULL;
	_iDebug = 0;
	_loading = 0;
	_reload = 0;
	_setting = 0;
	_setting2 = NULL;
	_initializingProfile = 0;
	_enteringProfile = NULL;
	_leavingProfile = NULL;
	_mounting = 0.0f;
	_dismounting = 0.0f;
	_activeSpecial = false;

	Loading = NULL;

	// Internal variables.
	int flags = 0;
	CreateValue(ConfigValueNames::bWalking, "bWalking", flags, _UpdateWalking);
	CreateValue(ConfigValueNames::bRunning, "bRunning", flags, _UpdateRunning);
	CreateValue(ConfigValueNames::bSwimming, "bSwimming", flags, _UpdateSwimming);
	CreateValue(ConfigValueNames::bSprinting, "bSprinting", flags, _UpdateSprinting);
	CreateValue(ConfigValueNames::bWeaponOut, "bWeaponOut", flags, _UpdateWeaponOut);
	CreateValue(ConfigValueNames::bDead, "bDead", flags, _UpdateDead);
	CreateValue(ConfigValueNames::bKillMove, "bKillMove", flags, _UpdateKillMove);
	CreateValue(ConfigValueNames::sRace, "sRace", flags, _UpdateRace);
	CreateValue(ConfigValueNames::bWereWolf, "bWereWolf", flags, _UpdateWereWolf);
	CreateValue(ConfigValueNames::bVampireLord, "bVampireLord", flags, _UpdateVampireLord);
	CreateValue(ConfigValueNames::bDialogue, "bDialogue", flags, _UpdateDialogue);
	CreateValue(ConfigValueNames::bMounted, "bMounted", flags, _UpdateMount);
	CreateValue(ConfigValueNames::bSneaking, "bSneaking", flags, _UpdateSneaking);
	CreateValue(ConfigValueNames::bCrafting, "bCrafting", flags, _UpdateCrafting);
	CreateValue(ConfigValueNames::bSitting, "bSitting", flags, _UpdateSitting);
	CreateValue(ConfigValueNames::bJumping, "bJumping", flags, _UpdateJumping);
	CreateValue(ConfigValueNames::bAimMagic, "bAimMagic", flags, _UpdateAimMagic);
	CreateValue(ConfigValueNames::bAimMagicLeft, "bAimMagicLeft", flags, _UpdateAimMagicLeft);
	CreateValue(ConfigValueNames::bAimMagicRight, "bAimMagicRight", flags, _UpdateAimMagicRight);
	CreateValue(ConfigValueNames::bAimBow, "bAimBow", flags, _UpdateAimBow);
	CreateValue(ConfigValueNames::bAimCrossbow, "bAimCrossbow", flags, _UpdateAimCrossBow);
	CreateValue(ConfigValueNames::bAttackAnyHanded, "bAttackAnyHanded", flags, _UpdateAimAnyHanded);
	CreateValue(ConfigValueNames::bAttackAnyHandedLeft, "bAttackAnyHandedLeft", flags, _UpdateAimAnyHandedLeft);
	CreateValue(ConfigValueNames::bAttackAnyHandedRight, "bAttackAnyHandedRight", flags, _UpdateAimAnyHandedRight);
	CreateValue(ConfigValueNames::bAttackOneHanded, "bAttackOneHanded", flags, _UpdateAimOneHanded);
	CreateValue(ConfigValueNames::bAttackOneHandedLeft, "bAttackOneHandedLeft", flags, _UpdateAimOneHandedLeft);
	CreateValue(ConfigValueNames::bAttackOneHandedRight, "bAttackOneHandedRight", flags, _UpdateAimOneHandedRight);
	CreateValue(ConfigValueNames::bAttackTwoHanded, "bAttackTwoHanded", flags, _UpdateAimTwoHanded);
	CreateValue(ConfigValueNames::iCameraState, "iCameraState", flags, _UpdateCameraState);
	CreateValue(ConfigValueNames::bMounting, "bMounting", flags, _UpdateMounting);
	CreateValue(ConfigValueNames::bDismounting, "bDismounting", flags, _UpdateDismounting);
	CreateValue(ConfigValueNames::bIsInIFPV, "bIsInIFPV", flags, _UpdateIFPV);
	CreateValue(ConfigValueNames::bIsActive, "bIsActive", flags, _UpdateIFPV2);
	CreateValue(ConfigValueNames::bIsTargetOverwritten, "bIsTargetOverwritten", flags, _UpdateTargetOverwrite);
	CreateValue(ConfigValueNames::iObjectBaseFormId, "iObjectBaseFormId", flags, _UpdateTargetBaseId);
	CreateValue(ConfigValueNames::bIndoors, "bIndoors", flags, _UpdateIndoors);
	CreateValue(ConfigValueNames::bGrabbing, "bGrabbing", flags, _UpdateGrabbing);
	CreateValue(ConfigValueNames::sFurniture, "sFurniture", flags, _UpdateFurniture);
	CreateValue(ConfigValueNames::bIsCameraOnArrow, "bIsCameraOnArrow", flags, _UpdateIsCameraOnArrow);
	CreateValue(ConfigValueNames::fLookDirX, "fLookDirX", flags, _UpdateLookX);
	CreateValue(ConfigValueNames::fLookDirZ, "fLookDirZ", flags, _UpdateLookZ);

	// User variables.
	flags = CONFIG_VALUE_FLAGS_USER_SET;
	CreateValue(ConfigValueNames::bEnabled, "bEnabled", flags);
	CreateValue(ConfigValueNames::bActivated, "bActivated", flags);
	CreateValue(ConfigValueNames::iDebug, "iDebug", flags);
	CreateValue(ConfigValueNames::iDebug2, "iDebug2", flags);
	CreateValue(ConfigValueNames::bHeadtrack, "bHeadtrack", flags);
	CreateValue(ConfigValueNames::fPositionOffsetHorizontal, "fPositionOffsetHorizontal", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fPositionOffsetVertical, "fPositionOffsetVertical", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fPositionOffsetDepth, "fPositionOffsetDepth", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fRotationFromHead, "fRotationFromHead", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fExtraRotationFromCamera, "fExtraRotationFromCamera", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fNearClip, "fNearClip", flags, NULL, _getNearClip, _setNearClip);
	CreateValue(ConfigValueNames::fFOV, "fFOV", flags | CONFIG_VALUE_FLAGS_TWEEN, NULL, _getFOV, _setFOV);
	CreateValue(ConfigValueNames::bDisableVATS, "bDisableVATS", flags, NULL, _getVATSDisable, _setVATSDisable);
	CreateValue(ConfigValueNames::fTimeScale, "fTimeScale", flags | CONFIG_VALUE_FLAGS_TWEEN | CONFIG_VALUE_FLAGS_SETTERALWAYS_NOT_ONE, NULL, _getTimeScale, _setTimeScale);
	CreateValue(ConfigValueNames::fRestrictAngleHorizontal, "fRestrictAngleHorizontal", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fRestrictAngleVertical, "fRestrictAngleVertical", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fUpdateProfileInterval, "fUpdateProfileInterval", flags);
	CreateValue(ConfigValueNames::fCameraViewOffsetHorizontal, "fCameraViewOffsetHorizontal", flags, NULL, _getCVOffsetH, _setCVOffsetH);
	CreateValue(ConfigValueNames::fCameraViewOffsetVertical, "fCameraViewOffsetVertical", flags, NULL, _getCVOffsetV, _setCVOffsetV);
	CreateValue(ConfigValueNames::iFaceCrosshair, "iFaceCrosshair", flags);
	CreateValue(ConfigValueNames::bZoomIntoIFPV, "bZoomIntoIFPV", flags);
	CreateValue(ConfigValueNames::bToggleIntoIFPV, "bToggleIntoIFPV", flags);
	CreateValue(ConfigValueNames::iUnused1, "iUnused1", flags);
	CreateValue(ConfigValueNames::iUnused2, "iUnused2", flags);
	CreateValue(ConfigValueNames::iUnused3, "iUnused3", flags);
	CreateValue(ConfigValueNames::iUnused4, "iUnused4", flags);
	CreateValue(ConfigValueNames::iUnused5, "iUnused5", flags);
	CreateValue(ConfigValueNames::iUnused6, "iUnused6", flags);
	CreateValue(ConfigValueNames::iUnused7, "iUnused7", flags);
	CreateValue(ConfigValueNames::iUnused8, "iUnused8", flags);
	CreateValue(ConfigValueNames::iUnused9, "iUnused9", flags);
	CreateValue(ConfigValueNames::iUnused10, "iUnused10", flags);
	CreateValue(ConfigValueNames::iObjectRefFormId, "iObjectRefFormId", flags, NULL, _getTargetRefId, _setTargetRefId);
	CreateValue(ConfigValueNames::bFocus, "bFocus", flags);
	CreateValue(ConfigValueNames::fFocusSpeed, "fFocusSpeed", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fFocusMinAngle, "fFocusMinAngle", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::iFocusDelay, "iFocusDelay", flags);
	CreateValue(ConfigValueNames::fExtraRotationMouse, "fExtraRotationMouse", flags | CONFIG_VALUE_FLAGS_TWEEN);
	CreateValue(ConfigValueNames::fTweenLength, "fTweenLength", flags);
	CreateValue(ConfigValueNames::bSendModEvents, "bSendModEvents", flags);
	CreateValue(ConfigValueNames::sCameraNode, "sCameraNode", flags);
	CreateValue(ConfigValueNames::bHideHead, "bHideHead", flags);
	CreateValue(ConfigValueNames::bHideHelmet, "bHideHelmet", flags);
	CreateValue(ConfigValueNames::bArrowCamera, "bArrowCamera", flags);
	CreateValue(ConfigValueNames::iIgnoreWorldCollision, "iIgnoreWorldCollision", flags);
	CreateValue(ConfigValueNames::fUpDownLimit, "fUpDownLimit", flags | CONFIG_VALUE_FLAGS_TWEEN, NULL, _getUpDownLimit, _setUpDownLimit);
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
	CreateValue(ConfigValueNames::fAdjustFarClipMinRatio, "fAdjustFarClipMinRatio", flags, NULL, _getAdjustFar, _setAdjustFar);
	CreateValue(ConfigValueNames::fAdjustFarClipStart, "fAdjustFarClipStart", flags, NULL, _getAdjustFar2, _setAdjustFar2);
#endif
}

Config::~Config()
{
	mtx.lock();

	for (auto itr = Values.begin(); itr != Values.end(); itr++)
	{
		if(*itr)
			delete *itr;
	}

	for (auto itr = Profiles.begin(); itr != Profiles.end(); itr++)
		delete *itr;

	for (auto itr = _delayedSet.begin(); itr != _delayedSet.end(); itr++)
		delete *itr;

	mtx.unlock();
}

bool Config::Load()
{
	CurrentLine = 0;
	
	FILE * file = NULL;
	errno_t err = fopen_s(&file, "Data\\SKSE\\Plugins\\FirstPersonPlugin.txt", "r");
	if (err == 2 || file == NULL)
	{
		WriteError(0, "File is missing!");
		return false;
	}

	if (err != 0)
	{
		WriteError(0, "Failed to open file - error code %d!", err);
		return false;
	}

	Loading = new ConfigLoadingData();
	Loading->ProfileName = "__Default";
	Loading->ProfileLine = 1;
	Loading->ProfileType = 2;

	char buffer[1024];
	while (fgets(buffer, 1024, file) != NULL)
	{
		CurrentLine++;

		Parse(buffer);
	}

	fclose(file);

	WriteOutLoading(true);

	WriteNormal("FirstPersonPlugin: Loaded %d profiles from configuration file.", (int)Profiles.size());
	
	if (DefaultProfile != NULL)
	{
		_initializingProfile = 1;
		this->DoProfileActions(DefaultProfile, ProfileActionTriggers::ProfileInitialize);
		_initializingProfile = 0;
		this->SetActiveProfile(DefaultProfile, 0.0f, false);
	}

	return true;
}

void Config::Parse(std::string line)
{
	line = StringHelper::Trim(line);
	if (line.empty())
		return;

	int index = 0;
	int length = line.length();

	int isBlock = -1;
	int isCon = -1;

	if (line[0] == ';' || line[0] == '#')
		return;
	else if (line[0] == '[')
	{
		WriteOutLoading();

		Loading->ProfileType = 0;
		line = line.substr(1);
		int idx = StringHelper::LastIndexOf(line, "]");
		if (idx < 0)
		{
			Loading->ProfileName = line;
			WriteError(CurrentLine, "Invalid name!");
			return;
		}

		Loading->ProfileName = line.substr(0, idx);
		return;
	}
	else if (line[0] == '{')
	{
		WriteOutLoading();

		Loading->ProfileType = 1;
		line = line.substr(1);
		int idx = StringHelper::LastIndexOf(line, "}");
		if (idx < 0)
		{
			Loading->ProfileName = line;
			WriteError(CurrentLine, "Invalid name!");
			return;
		}

		Loading->ProfileName = line.substr(0, idx);
		return;
	}
	else if (line[0] == '(')
	{
		line = line.substr(1);
		int idx = StringHelper::LastIndexOf(line, ")");
		if (idx < 0)
		{
			WriteError(CurrentLine, "Invalid syntax for condition! Ignoring line.");
			return;
		}

		if (Loading->ProfileType == 2)
		{
			WriteError(CurrentLine, "Found condition for default profile! Ignoring line.");
			return;
		}

		Loading->ProfileConditions.push_back(std::make_pair(line.substr(0, idx), CurrentLine));
		return;
	}

	Loading->ProfileActions.push_back(std::make_pair(line, CurrentLine));
}

Profile * Config::GetProfile(const char * name)
{
	for (std::vector<Profile*>::iterator itr = Profiles.begin(); itr != Profiles.end(); itr++)
	{
		if (!_stricmp((*itr)->Name.c_str(), name))
			return *itr;
	}

	return NULL;
}

Profile * Config::GetProfile(int id)
{
	for (std::vector<Profile*>::iterator itr = Profiles.begin(); itr != Profiles.end(); itr++)
	{
		if ((*itr)->Id == id)
			return *itr;
	}

	return NULL;
}

ProfileCondition * Config::ParseCondition(std::string str, int lineIndex)
{
	int idx = 0;
	int len = str.length();
	while (idx < len && isalnum(str[idx]))
		idx++;

	std::string typeStr = str.substr(0, idx);
	if (typeStr.empty())
	{
		WriteError(lineIndex, "Invalid syntax for condition!");
		return NULL;
	}

	str = StringHelper::TrimStart(str.substr(idx));
	ProfileConditionOperators op;
	len = 0;
	if (StringHelper::StartsWith(str, ">="))
	{
		op = ProfileConditionOperators::GreaterOrEqual;
		len = 2;
	}
	else if (StringHelper::StartsWith(str, "<="))
	{
		op = ProfileConditionOperators::LessOrEqual;
		len = 2;
	}
	else if (StringHelper::StartsWith(str, "!="))
	{
		op = ProfileConditionOperators::NotEquals;
		len = 2;
	}
	else if (StringHelper::StartsWith(str, "=="))
	{
		op = ProfileConditionOperators::Equals;
		len = 2;
	}
	else if (StringHelper::StartsWith(str, "~~"))
	{
		op = ProfileConditionOperators::NotContains;
		len = 2;
	}
	else if (StringHelper::StartsWith(str, ">"))
	{
		op = ProfileConditionOperators::Greater;
		len = 1;
	}
	else if (StringHelper::StartsWith(str, "<"))
	{
		op = ProfileConditionOperators::Less;
		len = 1;
	}
	else if (StringHelper::StartsWith(str, "="))
	{
		op = ProfileConditionOperators::Equals;
		len = 1;
	}
	else if (StringHelper::StartsWith(str, "&"))
	{
		op = ProfileConditionOperators::Contains;
		len = 1;
	}
	else if (StringHelper::StartsWith(str, "~"))
	{
		op = ProfileConditionOperators::NotContains;
		len = 1;
	}

	if (len == 0)
		op = ProfileConditionOperators::Equals;
	else
		str = StringHelper::TrimStart(str.substr(len));

	std::string strVal;
	if (len == 0)
		strVal = "1";
	else
		strVal = str;

	int dataIndex = -1;

	ProfileCondition * con = NULL;
	if (StringHelper::Equals(typeStr, "Faction"))
	{
		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueValue = ValueInstance(strVal, ConfigValueTypes::Int);
		con->Func = _ConditionFaction;
	}
	else if (StringHelper::Equals(typeStr, "Perk"))
	{
		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueValue = ValueInstance(strVal, ConfigValueTypes::Int);
		con->Func = _ConditionPerk;
	}
	else if (StringHelper::Equals(typeStr, "Keyword"))
	{
		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueValue = ValueInstance(strVal, ConfigValueTypes::String);
		con->Func = _ConditionKeyword;
	}
	else if (StringHelper::Equals(typeStr, "WornHasKeyword"))
	{
		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueValue = ValueInstance(strVal, ConfigValueTypes::String);
		con->Func = _ConditionWornHasKeyword;
	}
	else if (StringHelper::Equals(typeStr, "MagicEffectKeyword"))
	{
		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueValue = ValueInstance(strVal, ConfigValueTypes::String);
		con->Func = _ConditionMagicEffectKeyword;
	}
	else
	{
		ConfigValue * cVal = GetValue(typeStr.c_str());
		if (!cVal)
		{
			WriteError(lineIndex, "Unknown value '%s'!", typeStr.c_str());
			return NULL;
		}

		con = new ProfileCondition();
		con->Operation = op;
		con->DataValueIndex = cVal->Id;
		con->DataValueValue = ValueInstance(strVal, cVal->Type);
		con->Func = _ConditionValue;
		cVal->ConditionCount++;
	}

	return con;
}

ProfileAction * Config::ParseAction(Profile * prof, std::string str, int lineIndex)
{
	int idx = 0;
	ProfileActionTriggers trigger;
	if (str.length() >= 1)
	{
		if (str[0] == '+')
		{
			trigger = ProfileActionTriggers::ProfileActivate;
			str = str.substr(1);
		}
		else if (str[0] == '-')
		{
			trigger = ProfileActionTriggers::ProfileDeactivate;
			str = str.substr(1);
		}
		else if (str[0] == '!')
		{
			trigger = ProfileActionTriggers::ProfileInitialize;
			str = str.substr(1);
		}
		else
			trigger = ProfileActionTriggers::ProfileActivate;
	}
	int len = str.length();

	while (idx < len && isalnum(str[idx]))
		idx++;

	std::string typeStr = str.substr(0, idx);
	if (typeStr.empty())
	{
		WriteError(lineIndex, "Invalid syntax for command!");
		return NULL;
	}

	if (StringHelper::Equals(typeStr, "skip"))
	{
		prof->Skip = 1;
		return NULL;
	}

	if (StringHelper::Equals(typeStr, "disable"))
	{
		prof->Skip = 2;
		return NULL;
	}

	ProfileActionTypes type;
	UpdateAction func = NULL;
	int i = 0;
	while (true)
	{
		const char * tStr = std::get<1>(DefaultProfileActions[i]);
		if (tStr == NULL)
		{
			i = -1;
			break;
		}

		if (StringHelper::Equals(typeStr, tStr))
		{
			type = std::get<0>(DefaultProfileActions[i]);
			func = std::get<2>(DefaultProfileActions[i]);
			break;
		}

		i++;
	}

	if (i < 0)
	{
		WriteError(lineIndex, "Unknown command '%s'!", typeStr.c_str());
		return NULL;
	}

	ProfileAction * action = new ProfileAction();
	action->Type = type;
	action->Trigger = trigger;
	action->Func = func;
	action->Profile = prof;

	if (type == ProfileActionTypes::Activate)
		return action;
	if (type == ProfileActionTypes::Deactivate)
		return action;
	if (type == ProfileActionTypes::Enable)
		return action;
	if (type == ProfileActionTypes::Disable)
		return action;
	if (type == ProfileActionTypes::GoThirdPerson)
		return action;
	if (type == ProfileActionTypes::GoFirstPerson)
		return action;
	if (type == ProfileActionTypes::ForceThirdPerson)
		return action;
	if (type == ProfileActionTypes::ForceFirstPerson)
		return action;
	if (type == ProfileActionTypes::ReturnPerson)
		return action;
	if (type == ProfileActionTypes::Reload)
		return action;
	if (type == ProfileActionTypes::AttachCamera || type == ProfileActionTypes::AttachControl)
		return action;
	if (type == ProfileActionTypes::DetachCamera || type == ProfileActionTypes::DetachControl)
		return action;
	if (type == ProfileActionTypes::SendAnimEvent)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Event name is expected.");
			delete action;
			return NULL;
		}

		action->DataValueValue = ValueInstance(str.c_str(), true);
		return action;
	}
	if (type == ProfileActionTypes::RotateX || type == ProfileActionTypes::RotateY)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value is expected.");
			delete action;
			return NULL;
		}

		std::string valAmt;
		if (!StringHelper::ParseNonSpace(str, valAmt) || valAmt.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value is expected.");
			delete action;
			return NULL;
		}

		ValueInstance vi(valAmt, ConfigValueTypes::Float);
		if (!vi.HasValue)
		{
			WriteError(lineIndex, "Invalid syntax for command! Value is expected.");
			delete action;
			return NULL;
		}

		action->DataValueValue = vi;

		if (!str.empty())
		{
			ValueInstance vt(str, ConfigValueTypes::Int);
			if (vt.HasValue)
				action->DataValueIndex = vt.ToInt();
		}
		
		return action;
	}
	if (type == ProfileActionTypes::ReturnValue)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		ConfigValue * val = GetValue(str.c_str());
		if (!val)
		{
			WriteError(lineIndex, "Unknown value '%s'!", str.c_str());
			delete action;
			return NULL;
		}

		action->DataValueIndex = val->Id;
		return action;
	}
	if (type == ProfileActionTypes::ReturnValueDelayed)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseWord(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		ConfigValue * val = GetValue(valName.c_str());
		if (!val)
		{
			WriteError(lineIndex, "Unknown value '%s'!", valName.c_str());
			delete action;
			return NULL;
		}

		ValueInstance vDelay(str, ConfigValueTypes::Float);
		float flt = vDelay.HasValue ? vDelay.ToFloat() : -1.0f;
		if (flt < 0.0f)
		{
			WriteError(lineIndex, "Invalid syntax for command! Expected delay in seconds.");
			delete action;
			return NULL;
		}

		action->DataValueIndex = val->Id;
		action->DataTweenLength = flt;
		return action;
	}
	if (type == ProfileActionTypes::SetValue || type == ProfileActionTypes::SetValueNoOverride)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseWord(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		ConfigValue * val = GetValue(valName.c_str());
		if (!val)
		{
			WriteError(lineIndex, "Unknown value '%s'!", valName.c_str());
			delete action;
			return NULL;
		}

		action->DataValueIndex = val->Id;
		action->DataValueValue = ValueInstance(str, val->Type);
		return action;
	}
	if (type == ProfileActionTypes::SetValueDelayed)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseWord(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		ConfigValue * val = GetValue(valName.c_str());
		if (!val)
		{
			WriteError(lineIndex, "Unknown value '%s'!", valName.c_str());
			delete action;
			return NULL;
		}

		std::string valVal;
		if (!StringHelper::ParseString(str, valVal))
		{
			WriteError(lineIndex, "Invalid syntax for command! Expected value.");
			delete action;
			return NULL;
		}

		ValueInstance vDelay(str, ConfigValueTypes::Float);
		float flt = vDelay.HasValue ? vDelay.ToFloat() : -1.0f;
		if (flt < 0.0f)
		{
			WriteError(lineIndex, "Invalid syntax for command! Expected delay in seconds.");
			delete action;
			return NULL;
		}

		action->DataValueIndex = val->Id;
		action->DataValueValue = ValueInstance(valVal, val->Type);
		action->DataTweenLength = flt;
		return action;
	}
	if (type == ProfileActionTypes::AddValue || type == ProfileActionTypes::AddValueNoOverride)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseWord(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Value name is expected.");
			delete action;
			return NULL;
		}

		ConfigValue * val = GetValue(valName.c_str());
		if (!val)
		{
			WriteError(lineIndex, "Unknown value '%s'!", valName.c_str());
			delete action;
			return NULL;
		}

		action->DataValueIndex = val->Id;
		action->DataValueValue = ValueInstance(str, val->Type);
		return action;
	}
	if (type == ProfileActionTypes::SetProfile)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseString(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		action->DataValueValue = ValueInstance(valName.c_str(), true);
		ValueInstance vt(str, ConfigValueTypes::Float);
		if (vt.HasValue)
			action->DataTweenLength = vt.ToFloat();
		else
			action->DataTweenLength = 0.0f;
		return action;
	}
	if (type == ProfileActionTypes::Inhibit)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseString(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		action->DataValueValue = ValueInstance(valName.c_str(), true);
		ValueInstance vt(str, ConfigValueTypes::Float);
		if (vt.HasValue)
			action->DataTweenLength = vt.ToFloat();
		else
			action->DataTweenLength = -1.0f;
		return action;
	}
	if (type == ProfileActionTypes::RunProfile)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseString(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		action->DataValueValue = ValueInstance(valName.c_str(), true);
		return action;
	}
	if (type == ProfileActionTypes::BindKey || type == ProfileActionTypes::BindKey2)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected.");
			delete action;
			return NULL;
		}

		std::string keyId;
		if (!StringHelper::ParseInteger(str, keyId))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected.");
			delete action;
			return NULL;
		}

		std::string keyMod;
		if (!StringHelper::ParseInteger(str, keyMod))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key modifier is expected.");
			delete action;
			return NULL;
		}

		std::string valName;
		if (!StringHelper::ParseString(str, valName))
		{
			WriteError(lineIndex, "Invalid syntax for command! Profile name is expected.");
			delete action;
			return NULL;
		}

		int keyIdN = 0;
		if (!StringHelper::ToInt(keyId, keyIdN))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected as integer value.");
			delete action;
			return NULL;
		}

		int keyModN = 0;
		if (!StringHelper::ToInt(keyMod, keyModN))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key modifier is expected as integer value.");
			delete action;
			return NULL;
		}

		if (type == ProfileActionTypes::BindKey2)
			keyModN |= 32;

		int fnValue = (keyModN & 0xFFF) << 16;
		if (keyIdN < 0)
		{
			keyIdN = -keyIdN;
			fnValue |= 0xF0000000;
		}
		fnValue |= (keyIdN & 0xFFFF);

		action->DataValueIndex = fnValue;
		action->DataValueValue = ValueInstance(valName.c_str(), true);

		if (!str.empty())
		{
			ValueInstance tw(str, ConfigValueTypes::Float);
			if (tw.HasValue && tw.ToFloat() > 0.0f)
				action->DataTweenLength = tw.ToFloat();
		}
		return action;
	}
	if (type == ProfileActionTypes::UnBindKey)
	{
		str = StringHelper::TrimStart(str.substr(idx));
		if (str.empty())
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected.");
			delete action;
			return NULL;
		}

		std::string keyId;
		if (!StringHelper::ParseInteger(str, keyId))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected.");
			delete action;
			return NULL;
		}

		int keyIdN = 0;
		if (!StringHelper::ToInt(keyId, keyIdN))
		{
			WriteError(lineIndex, "Invalid syntax for command! Key is expected as integer value.");
			delete action;
			return NULL;
		}

		int fnValue = 0;
		if (keyIdN < 0)
		{
			keyIdN = -keyIdN;
			fnValue |= 0xF0000000;
		}
		fnValue |= (keyIdN & 0xFFFF);

		action->DataValueIndex = fnValue;
		return action;
	}

	WriteError(lineIndex, "Invalid syntax for command!");
	return NULL;
}

void Config::WriteOutLoading(bool finished)
{
	if (Loading == NULL)
		return;

	ConfigLoadingData * data = Loading;
	if (finished)
		Loading = NULL;
	else
	{
		Loading = new ConfigLoadingData();
		Loading->ProfileLine = CurrentLine;
	}

	Profile * prof = new Profile();
	prof->Id = (int)Profiles.size();
	prof->Name = data->ProfileName;
	prof->Type = data->ProfileType;

	for (std::vector<std::pair<std::string, int>>::iterator itr = data->ProfileConditions.begin(); itr != data->ProfileConditions.end(); itr++)
	{
		ProfileCondition * con = ParseCondition(itr->first, itr->second);
		if (con != NULL)
			prof->Conditions.push_back(con);
	}
	for (std::vector<std::pair<std::string, int>>::iterator itr = data->ProfileActions.begin(); itr != data->ProfileActions.end(); itr++)
	{
		ProfileAction * con = ParseAction(prof, itr->first, itr->second);
		if (con != NULL)
			prof->Actions.push_back(con);
	}

	if (prof->Skip == 2)
	{
		delete prof;
		prof = NULL;
	}

	if (prof != NULL)
	{
		if (prof->Type == 0)
			Profiles.push_back(prof);
		else if (prof->Type == 1)
			Profiles.push_back(prof);
		else if (prof->Type == 2)
		{
			Profiles.push_back(prof);
			DefaultProfile = prof;
			prof->Id = 2001;
		}
		else
		{
			WriteError(data->ProfileLine, "Invalid profile type.");
			delete prof;
		}
	}

	delete data;
}

int printConsole = 0x8486F0;
bool Config::WriteConsoleGame(const char * msg)
{
	int consoleManager = *((int*)0x1B10B58);
	if (consoleManager == 0)
		return false;

	va_list args2;
	_asm
	{
		mov ecx, consoleManager
		mov eax, args2
		push eax
		mov eax, msg // lea
		push eax
		call printConsole
	}

	return true;
}

void Config::WriteError(int line, const char * fmt, ...)
{
	std::string buf1;

	{
		char buffer[256];
		va_list args;
		va_start(args, fmt);
		_vsnprintf_s(buffer, 256, fmt, args);
		va_end(args);

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	{
		char buffer[256];
		_snprintf_s(buffer, 256, "FirstPersonPlugin(%d): %s", line, buf1.c_str());

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	ConsoleQueue.push_back(buf1);
}

void Config::WriteNormal(const char * fmt, ...)
{
	std::string buf1;

	{
		char buffer[256];
		va_list args;
		va_start(args, fmt);
		_vsnprintf_s(buffer, 256, fmt, args);
		va_end(args);

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	{
		char buffer[256];
		_snprintf_s(buffer, 256, "FirstPersonPlugin: %s", buf1.c_str());

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	ConsoleQueue.push_back(buf1);
}

bool Config::GetFloat(ConfigValueNames id, float& value)
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (!cfg)
		return false;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return false;

	ConfigValue * c = cfg->Values[(int)id];
	if (!c)
		return false;
	ValueInstance vi = c->GetCurrentValue(true);
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return false;

	value = vi.ToFloat();
	return true;
}

bool Config::GetInt(ConfigValueNames id, int& value)
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (!cfg)
		return false;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return false;

	ConfigValue * c = cfg->Values[(int)id];
	if (!c)
		return false;
	ValueInstance vi = c->GetCurrentValue(true);
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Int)
		return false;

	value = vi.ToInt();
	return true;
}

bool Config::GetBool(ConfigValueNames id, bool& value)
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (!cfg)
		return false;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return false;

	ConfigValue * c = cfg->Values[(int)id];
	if (!c)
		return false;
	ValueInstance vi = c->GetCurrentValue(true);
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Bool)
		return false;

	value = vi.ToBool();
	return true;
}

bool Config::GetString(ConfigValueNames id, std::string& value)
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (!cfg)
		return false;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return false;

	ConfigValue * c = cfg->Values[(int)id];
	if (!c)
		return false;

	ValueInstance vi = c->GetCurrentValue(true);
	if (!vi.HasValue || vi.Type != ConfigValueTypes::String)
		return false;

	value = vi.ToString();
	return true;
}

bool _firstInitialize = true;
void Config::Initialize()
{
	bool hadFirstInitialize = false;
	if (_firstInitialize)
	{
		_firstInitialize = false;
		hadFirstInitialize = true;
	}

	Config * oldCfg = cfgInstance;
	if (oldCfg != NULL && oldCfg->_loading != 0)
	{
		oldCfg->WriteError(0, "Trying to reload configuration while already loading!");
		return;
	}

	// Make sure main profile is deactivated because it will be reactivated in new profile.
	if (oldCfg != NULL)
		oldCfg->_setActive(false);

	InputHandler::Clear();
	Camera::_reload();

	cfgInstance = new Config();

	if (hadFirstInitialize)
		cfgInstance->WriteNormal("Initializing IFPV version %.1f", IFPV_VERSION);

	// Get default values from old because we might otherwise think our own set values are default.
	if (oldCfg != NULL)
	{
		for (std::vector<ConfigValue*>::iterator itr = oldCfg->Values.begin(); itr != oldCfg->Values.end(); itr++)
		{
			if (!*itr || !(*itr)->DefaultValue.HasValue || !(*itr)->IsDefaultValueFromGame)
				continue;

			ConfigValue * c = cfgInstance->_getValueInternal((ConfigValueNames)(*itr)->Id);
			if (c && !c->DefaultValue.HasValue)
				c->DefaultValue = (*itr)->DefaultValue;
		}
	}

	{
		cfgInstance->_loading = 1;
		cfgInstance->Load();
		cfgInstance->_loading = 0;
	}

	if (oldCfg != NULL)
		delete oldCfg;
}

void Config::Update()
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	if (cfg->_reload != 0)
	{
		cfg->_reload = 0;

		Initialize();

		cfg = cfgInstance;
		if (cfg == NULL)
			return;
	}

	if (!cfg->ConsoleQueue.empty())
	{
		while (!cfg->ConsoleQueue.empty())
		{
			std::string str = cfg->ConsoleQueue.front();
			if (!Config::WriteConsoleGame(str.c_str()))
				break;

			cfg->ConsoleQueue.erase(cfg->ConsoleQueue.begin());
		}
	}

	cfg->ProcessBindQueue();

	Camera::GetSingleton()->_getNSConfig(&cfg->_nsObj, &cfg->_nsActor);
	{
		InputHandler::Update();

		float timePassed = Camera::GetSingleton()->GetTimeSinceLastFrame();
		float timePassedWithScale = Camera::GetSingleton()->GetTimeSinceLastFrame(true);

		for (auto itr = cfg->_delayedSet.begin(); itr != cfg->_delayedSet.end();)
		{
			DelayedSet * ds = *itr;

			ds->Delay -= timePassed;
			if (ds->Delay <= 0.0f)
			{
				itr = cfg->_delayedSet.erase(itr);

				if (!ds->Return)
					_setValueInternal(ds->Id, ds->Value, ds->Profile, 0.0f);
				else
					_unsetValueInternal(ds->Id, ds->Profile, 0.0f);

				delete ds;
				continue;
			}

			itr++;
		}

		if (cfg->_mounting > 0.0f)
		{
			cfg->_mounting -= timePassedWithScale;
			if (cfg->_mounting < 0.0f)
				cfg->_mounting = 0.0f;
		}
		if (cfg->_dismounting > 0.0f)
		{
			cfg->_dismounting -= timePassedWithScale;
			if (cfg->_dismounting < 0.0f)
				cfg->_dismounting = 0.0f;
		}

		for (auto itr = cfg->Values.begin(); itr != cfg->Values.end(); itr++)
		{
			if (*itr)
				cfg->UpdateConfigValue(*itr, timePassed);
		}
		for (auto itr = cfg->Profiles.begin(); itr != cfg->Profiles.end(); itr++)
		{
			if ((*itr)->Inhibited > 0.0f)
			{
				(*itr)->Inhibited -= timePassed;
				if ((*itr)->Inhibited < 0.0f)
					(*itr)->Inhibited = 0.0f;
			}
		}

		if (cfg->ActiveProfileForced > 0.0f)
		{
			cfg->ActiveProfileForced -= timePassed;
			if (cfg->ActiveProfileForced < 0.0f)
				cfg->ActiveProfileForced = 0.0f;
		}

		cfg->ProfileUpdateTimer -= timePassed;
		if (cfg->ProfileUpdateTimer <= 0.0f)
		{
			_newCalc();

			cfg->UpdateProfileValues();
			cfg->CalculateActiveProfile();
			Config::GetFloat(ConfigValueNames::fUpdateProfileInterval, cfg->ProfileUpdateTimer);
			if (cfg->ProfileUpdateTimer < 0.01f)
				cfg->ProfileUpdateTimer = 0.01f;
		}
	}
	cfg->_iDebug = 0;
	Config::GetInt(ConfigValueNames::iDebug, cfg->_iDebug);
	cfg->_nsObj = NULL;
	cfg->_nsActor = NULL;
}

void Config::UpdateConfigValue(ConfigValue * val, float diff)
{
	if (val->Tween)
	{
		val->Tween->TimeDone += diff;
		if (val->Tween->TimeDone >= val->Tween->TimeTotal)
		{
			delete val->Tween;
			val->Tween = NULL;
			val->ValueChanged = true;
		}
		else if (val->Type == ConfigValueTypes::Float)
			val->ValueChanged = true;
	}

	bool changed = val->ValueChanged;
	if (!changed)
	{
		if ((val->Flags & CONFIG_VALUE_FLAGS_SETTERALWAYS) != 0)
			changed = true;
		else if ((val->Flags & CONFIG_VALUE_FLAGS_SETTERALWAYS_NOT_ONE) != 0)
		{
			ValueInstance curVal = val->GetCurrentValue(true);
			float fCurVal = curVal.HasValue ? curVal.ToFloat() : 1.0f;
			if (fCurVal != 1.0f)
				changed = true;
		}
	}

	if (changed)
	{
		if (val->Setter != NULL)
		{
			ValueInstance vi = val->GetCurrentValue(true);
			val->Setter(vi);
		}

		val->ValueChanged = false;
	}
}

void Config::CreateValue(int id, const char * name, int flags, UpdateInternalValue func, _GameGetter getter, _GameSetter setter)
{
	int index = id;
	if (index < 0)
		return;

	ConfigValue * val = new ConfigValue(id, name, flags, func, getter, setter);
	if (index < Values.size())
		Values[index] = val;
	else
	{
		while (index >= Values.size())
			Values.push_back(NULL);
		Values[index] = val;
	}
}

ConfigValue * Config::GetValue(const char * name)
{
	for(std::vector<ConfigValue*>::iterator itr = Values.begin(); itr != Values.end(); itr++)
	{
		if (!*itr)
			continue;

		if (!_stricmp((*itr)->Name, name))
			return *itr;
	}

	return NULL;
}

void Config::UpdateProfileValues()
{
	for (std::vector<ConfigValue*>::iterator itr = Values.begin(); itr != Values.end(); itr++)
	{
		if (!*itr)
			continue;

		if ((*itr)->ConditionCount == 0)
			continue;

		if ((*itr)->Func == NULL)
			continue;

		(*(*itr)->Func)(_nsObj, _nsActor, *itr);
	}
}

bool Config::IsProfileConditionsOk(Profile * prof, bool allowNoConditions)
{
	if (prof->Conditions.empty())
	{
		if (!allowNoConditions)
			return false;

		return true;
	}

	for (std::vector<ProfileCondition*>::iterator itr = prof->Conditions.begin(); itr != prof->Conditions.end(); itr++)
	{
		if (!Test(prof, *itr))
			return false;
	}

	return true;
}

void Config::CalculateActiveProfile(int type)
{
	Profile * curActive = NULL;
	bool canCur = true;
	if (type > 0)
		canCur = false;
	if (ActiveProfileForced != 0.0f)
		canCur = false;

	for (std::vector<Profile*>::iterator itr = Profiles.begin(); itr != Profiles.end(); itr++)
	{
		if (type >= 0 && (*itr)->Type != type)
			continue;

		if ((*itr)->Type == 0)
		{
			if (!canCur || curActive != NULL)
				continue;

			if ((*itr)->Skip != 0)
				continue;

			if ((*itr)->Inhibited != 0.0f)
				continue;

			if (!IsProfileConditionsOk(*itr, false))
				continue;

			curActive = *itr;
		}
		else if ((*itr)->Type == 1)
		{
			bool isActive = true;

			if ((*itr)->Skip != 0)
				isActive = false;
			else if ((*itr)->Inhibited != 0.0f)
				isActive = false;
			else if (!IsProfileConditionsOk(*itr, false))
				isActive = false;

			if ((*itr)->MainData == 0 && isActive)
			{
				WriteDebug(1, "Activating profile %s.", (*itr)->Name.c_str());
				(*itr)->MainData = 1;
				SetEntering(*itr);
				DoProfileActions(*itr, ProfileActionTriggers::ProfileActivate);
				SetEntering(NULL);
			}
			else if ((*itr)->MainData == 1 && !isActive)
			{
				WriteDebug(1, "Deactivating profile %s.", (*itr)->Name.c_str());
				(*itr)->MainData = 0;
				SetLeaving(*itr);
				DoProfileActions(*itr, ProfileActionTriggers::ProfileDeactivate);
				SetLeaving(NULL);
			}
		}
	}

	if (canCur && curActive == NULL)
		curActive = DefaultProfile;

	if (curActive != NULL)
		SetActiveProfile(curActive, 0.0f, false);
}

void Config::DoProfileActions(Profile * profile, ProfileActionTriggers trigger)
{
	for (std::vector<ProfileAction*>::iterator itr = profile->Actions.begin(); itr != profile->Actions.end(); itr++)
	{
		if ((*itr)->Trigger == trigger)
			DoAction(profile, *itr);
	}
}

void Config::SetActiveProfile(Profile * prof, float forcedTime, bool manualSet)
{
	if (_setting != 0)
	{
		_setting2 = prof;
		return;
	}

	_setting = 1;
	_setting2 = prof;

	if (ActiveProfile == prof)
	{
		ActiveProfileForced = forcedTime;

		_setting = 0;
		_setting2 = NULL;
		return;
	}

	std::string prevProfile = "None";

	if (ActiveProfile != NULL)
	{
		prevProfile = ActiveProfile->Name;

		SetLeaving(ActiveProfile);
		DoProfileActions(ActiveProfile, ProfileActionTriggers::ProfileDeactivate);
		SetLeaving(NULL);
	}

	ActiveProfile = _setting2;

	std::string curProfile = "None";
	if (ActiveProfile)
		curProfile = ActiveProfile->Name;

	WriteDebug(1, "Changed profile from %s to %s.", prevProfile.c_str(), curProfile.c_str());

	_setting = 0;
	_setting2 = NULL;

	if (manualSet && forcedTime == 0.0f && _setting2 != NULL && _setting2->Conditions.empty())
		ProfileUpdateTimer = 0.0f;

	if (ActiveProfile != NULL)
	{
		int curP = 0;
		if (Camera::GetSingleton()->IsThirdPerson(true))
			curP = 1;
		else if (Camera::GetSingleton()->IsFirstPerson(true))
			curP = 2;
		ActiveProfile->RememberPerson = curP;
		SetEntering(ActiveProfile);
		DoProfileActions(ActiveProfile, ProfileActionTriggers::ProfileActivate);
		SetEntering(NULL);
		ActiveProfileForced = forcedTime;
	}
	else
		ActiveProfileForced = 0.0f;
}

void Config::DoAction(Profile * profile, ProfileAction * action)
{
	// profile may be NULL.

	if (action->Func != NULL)
		action->Func(_nsObj, _nsActor, action);
}

bool Config::Test(Profile * profile, ProfileCondition * con)
{
	if (con->Func != NULL)
		return con->Func(_nsObj, _nsActor, con);
	return false;
}

void Config::_setValueInternal(ConfigValueNames id, ValueInstance vi, int profileId, float delay)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return;

	ConfigValue * c = cfg->Values[index];
	if (!c || c->Type != vi.Type || !(c->Flags & CONFIG_VALUE_FLAGS_USER_SET))
		return;

	for (auto itr = cfg->_delayedSet.begin(); itr != cfg->_delayedSet.end();)
	{
		DelayedSet * ds = *itr;
		if (ds->Id == id && ds->Profile == profileId && !ds->Return)
		{
			itr = cfg->_delayedSet.erase(itr);
			delete ds;
		}
		else
			itr++;
	}

	if (delay > 0.0f)
	{
		DelayedSet * ds = new DelayedSet();
		ds->Id = id;
		ds->Delay = delay;
		ds->Value = vi;
		ds->Return = false;
		ds->Profile = profileId;
		cfg->_delayedSet.push_back(ds);
		return;
	}

	c->UpdateDefaultValue();

	Profile * prof = cfg->GetProfile(profileId);
	if (prof != NULL)
	{
		for (auto itr = prof->Modified.begin(); itr != prof->Modified.end(); itr++)
		{
			if (*itr == c)
			{
				_unsetValueInternal(id, prof->Id);
				break;
			}
		}
	}

	float tweenLength = 0.0f;
	if ((c->Flags & CONFIG_VALUE_FLAGS_TWEEN) != 0)
	{
		if(!Config::GetFloat(ConfigValueNames::fTweenLength, tweenLength))
			tweenLength = 0.2f;
	}

	bool wouldAffect = c->ThisAddValueWouldAffect(profileId);

	if (wouldAffect)
	{
		ValueInstance curVi = c->GetCurrentValue(true);
		if (!curVi.HasValue || curVi.Type != ConfigValueTypes::Float)
			tweenLength = 0.0f;

		if (tweenLength > 0.0f && vi.HasValue)
		{
			float curValue = curVi.ToFloat();
			if (c->Getter != NULL)
			{
				ValueInstance vi = c->Getter();
				if (vi.HasValue && vi.Type == ConfigValueTypes::Float)
					curValue = vi.ToFloat();
			}

			if (c->Tween != NULL)
				delete c->Tween;
			c->Tween = new TweenData(curValue, vi.ToFloat(), tweenLength);
		}
		else
			c->ValueChanged = true; // Tween value change is handled elsewhere.
	}

	c->Values[profileId] = vi;

	if (prof != NULL)
		prof->Modified.insert(c);

	if (cfg->_initializingProfile != 0 && (c->Flags & CONFIG_VALUE_FLAGS_USER_SET) && !c->DefaultValue.HasValue)
		c->DefaultValue = vi;
}

void Config::_unsetValueInternal(ConfigValueNames id, int profileId, float delay)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return;

	ConfigValue * c = cfg->Values[(int)id];
	if (!c || !(c->Flags & CONFIG_VALUE_FLAGS_USER_SET))
		return;

	for (auto itr = cfg->_delayedSet.begin(); itr != cfg->_delayedSet.end();)
	{
		DelayedSet * ds = *itr;
		if (ds->Id == id && ds->Profile == profileId && ds->Return)
		{
			itr = cfg->_delayedSet.erase(itr);
			delete ds;
		}
		else
			itr++;
	}

	if (delay > 0.0f)
	{
		DelayedSet * ds = new DelayedSet();
		ds->Id = id;
		ds->Delay = delay;
		ds->Value = ValueInstance();
		ds->Return = true;
		ds->Profile = profileId;
		cfg->_delayedSet.push_back(ds);
		return;
	}

	c->UpdateDefaultValue();

	Profile * prof = cfg->GetProfile(profileId);
	if (prof != NULL)
		prof->Modified.erase(c);

	auto itr = c->Values.find(profileId);
	if (itr == c->Values.end())
		return;

	bool wouldAffect = c->ThisRemoveValueWouldAffect(profileId);
	ValueInstance pVi = c->GetCurrentValue(true);
	c->Values.erase(itr);
	ValueInstance nVi = c->GetCurrentValue(false);

	if (wouldAffect)
	{
		float tweenLength = 0.0f;
		if ((c->Flags & CONFIG_VALUE_FLAGS_TWEEN) != 0 && c->Type == ConfigValueTypes::Float)
		{
			if (!Config::GetFloat(ConfigValueNames::fTweenLength, tweenLength))
				tweenLength = 0.2f;
		}

		if (!pVi.HasValue || pVi.Type != ConfigValueTypes::Float || !nVi.HasValue || nVi.Type != ConfigValueTypes::Float)
			tweenLength = 0.0f;

		if (tweenLength > 0.0f)
		{
			float curPrev = pVi.ToFloat();
			if (c->Getter != NULL)
			{
				ValueInstance vi = c->Getter();
				if (vi.HasValue && vi.Type == ConfigValueTypes::Float)
					curPrev = vi.ToFloat();
			}
			float curNext = nVi.ToFloat();

			if (c->Tween != NULL)
				delete c->Tween;
			
			c->Tween = new TweenData(curPrev, curNext, tweenLength);
		}
		else
			c->ValueChanged = true; // Tween value change is handled elsewhere.
	}
}

ConfigValue * Config::_getValueInternal(ConfigValueNames id)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return NULL;

	int index = (int)id;
	if (index < 0 || index >= cfg->Values.size())
		return NULL;

	return cfg->Values[(int)id];
}

ConfigValue * Config::_getValueInternal(const char * name)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return NULL;

	for (auto itr = cfg->Values.begin(); itr != cfg->Values.end(); itr++)
	{
		if (!_stricmp((*itr)->Name, name))
			return *itr;
	}

	return NULL;
}

void Config::_setProfile(int id, float duration)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	Profile * prof = NULL;
	for (std::vector<Profile*>::iterator itr = cfg->Profiles.begin(); itr != cfg->Profiles.end(); itr++)
	{
		if ((*itr)->Id == id && (*itr)->Type == 0)
		{
			prof = *itr;
			break;
		}
	}

	if (prof == NULL)
		prof = cfg->DefaultProfile;

	if(prof != NULL && cfg->IsProfileConditionsOk(prof, true))
		cfg->SetActiveProfile(prof, duration, true);
}

void Config::_setProfile(const char * name, float duration)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	Profile * prof = cfg->GetProfile(name);
	if (prof != NULL && prof->Type != 0)
		prof = NULL;
	
	if (prof == NULL)
		prof = cfg->DefaultProfile;

	if (prof != NULL && cfg->IsProfileConditionsOk(prof, true))
		cfg->SetActiveProfile(prof, duration, true);
}

void Config::_queueBind(ToBind * bind)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	cfg->BindQueue.push_back(bind);
}

void Config::ProcessBindQueue()
{
	while (!BindQueue.empty())
	{
		ToBind * bind = BindQueue.front();
		BindQueue.erase(BindQueue.begin());

		int profId = bind->ProfileId;
		if (profId < 0)
		{
			Profile * prof = GetProfile(bind->ProfileName.c_str());
			if (prof != NULL)
				profId = prof->Id;
		}

		if (profId >= 0 && bind->Bind)
			InputHandler::Register(bind->Key, bind->Modifiers, bind->Down ? profId : 0, bind->Down ? bind->Duration : 0.0f, !bind->Down ? profId : 0, !bind->Down ? bind->Duration : 0.0f);
		else if (!bind->Bind)
			InputHandler::Unregister(bind->Key, bind->Down, !bind->Down);

		delete bind;
	}
}

void Config::_runProfile(int id)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	Profile * prof = cfg->GetProfile(id);
	
	if (prof != NULL)
	{
		cfg->SetEntering(prof);
		cfg->DoProfileActions(prof, ProfileActionTriggers::ProfileActivate);
		cfg->SetEntering(NULL);
		cfg->SetLeaving(prof);
		cfg->DoProfileActions(prof, ProfileActionTriggers::ProfileDeactivate);
		cfg->SetLeaving(NULL);
	}
}

void Config::_runProfile(const char * name)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	Profile * prof = cfg->GetProfile(name);

	if (prof != NULL)
	{
		cfg->SetEntering(prof);
		cfg->DoProfileActions(prof, ProfileActionTriggers::ProfileActivate);
		cfg->SetEntering(NULL);
		cfg->SetLeaving(prof);
		cfg->DoProfileActions(prof, ProfileActionTriggers::ProfileDeactivate);
		cfg->SetLeaving(NULL);
	}
}

void Config::_reloadFile()
{
	Config * cfg = cfgInstance;
	if (cfg != NULL)
		cfg->_reload = 1;
}

void Config::_inhibit(const char * name, float duration)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	Profile * prof = cfg->GetProfile(name);
	if (prof == NULL)
		return;

	prof->Inhibited = duration;
}

int Config::_getDebug()
{
	int result = 0;
	Config::GetInt(ConfigValueNames::iDebug, result);

	switch (result)
	{
	case 1:
	case 2:
		return result;

	default:
		break;
	}

	return 0;
}

void Config::WriteDebug(int level, const char * fmt, ...)
{
	if (level > _getDebug())
		return;

	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	std::string buf1;

	{
		char buffer[256];
		va_list args;
		va_start(args, fmt);
		_vsnprintf_s(buffer, 256, fmt, args);
		va_end(args);

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	{
		char buffer[256];
		_snprintf_s(buffer, 256, "FirstPersonPlugin(DEBUG): %s", buf1.c_str());

		buf1 = buffer;
		buf1 = StringHelper::Replace(buf1, "%", "%%");
	}

	cfg->ConsoleQueue.push_back(buf1);
}

void Config::_setActive(bool active)
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	cfg->_activeSpecial = active;
}

int _updateGraphics = 0;
int Config::_ShouldUpdateGraphics()
{
	int x = _updateGraphics;
	_updateGraphics = 0;
	return x;
}

void Config::_setUpdateGraphics()
{
	_updateGraphics = 1;
}

void Config::_mount(Actor * actor)
{
	if (!actor || actor->formID != Camera::GetSingleton()->GetLastTargetFormId())
		return;

	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (cfg)
		cfg->_mounting = 1.2f;
}

void Config::_dismount(Actor * actor)
{
	if (!actor || actor->formID != Camera::GetSingleton()->GetLastTargetFormId())
		return;

	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (cfg)
		cfg->_dismounting = 2.5f;
}

void Config::_getMounting(bool& mount, bool& dismount)
{
	Config * cfg = cfgInstance;
	if (cfg)
	{
		mount = cfg->_mounting > 0.0f;
		dismount = cfg->_dismounting > 0.0f;
	}
}

void Config::_toggleActive()
{
	LOCK_MUTEX(mtx);

	ConfigValue * cv = _getValueInternal(ConfigValueNames::bActivated);
	if (cv)
	{
		ValueInstance vi = cv->GetCurrentValue(true);
		bool had = vi.HasValue && vi.Type == ConfigValueTypes::Bool && vi.ToBool();
		_setValueInternal(ConfigValueNames::bActivated, ValueInstance(!had, true), 2000);
	}
}

bool Config::_isActive()
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return false;

	return cfg->_activeSpecial;
}

void Config::SetEntering(Profile * prof)
{
	_enteringProfile = prof;

	if(prof != NULL)
		SendModEvent("IFPV_EnteringProfile", prof->Name, prof->Id);
}

void Config::SetLeaving(Profile * prof)
{
	_leavingProfile = prof;

	if (prof != NULL)
		SendModEvent("IFPV_LeavingProfile", prof->Name, prof->Id);
}

const char * Config::GetConfigValueName(int id)
{
	ConfigValue * cv = _getValueInternal((ConfigValueNames)id);
	if (cv != NULL)
		return cv->Name;
	return "";
}

bool Config::IsProfileActive(const char * name)
{
	LOCK_MUTEX(mtx);

	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return false;

	Profile * prof = cfg->GetProfile(name);
	return prof != NULL && prof->MainData == 1;
}

void Config::SetModEventDispatcher(void * dispatcher)
{
	g_modEventDispatcher = dispatcher;
}

void Config::SendModEvent(const char * name, std::string strArg, float numArg)
{
	if (g_modEventDispatcher == NULL)
		return;

	int plr = *((int*)0x1B2E8E4);
	if (plr == 0)
		return;

	bool sending = false;
	Config::GetBool(ConfigValueNames::bSendModEvents, sending);

	if (!sending)
		return;

	LOCK_MUTEX(mtx);

	EventDispatcher<SKSEModCallbackEvent> * dispatcher = (EventDispatcher<SKSEModCallbackEvent>*)g_modEventDispatcher;

	BSFixedString eventName = BSFixedString(name);
	BSFixedString eventArgStr = BSFixedString(strArg.c_str());

	SKSEModCallbackEvent evn(eventName, eventArgStr, numArg, (TESForm*)plr);
	dispatcher->SendEvent(&evn);
}

void Config::ResetPapyrus()
{
	Config * cfg = cfgInstance;
	if (cfg == NULL)
		return;

	LOCK_MUTEX(mtx);

	for (int i = 0; i < (int)cfg->Values.size(); i++)
	{
		auto itr = cfg->Values[i]->Values.find(PAPYRUS_PROFILE_ID);
		if (itr == cfg->Values[i]->Values.end())
			continue;

		_unsetValueInternal((ConfigValueNames)i, PAPYRUS_PROFILE_ID);
	}
}
