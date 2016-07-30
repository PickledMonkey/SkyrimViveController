#pragma once

#include "Config.h"
#include "skse/GameReferences.h"
#include "skse/GameRTTI.h"
#include "skse/GameObjects.h"
#include "skse/GameData.h"

#define DEFCONDITION(name) bool _Condition##name##(TESObjectREFR * obj, Actor * actor, ProfileCondition * con)

DEFCONDITION(Value)
{
	if (!con->DataValueValue.HasValue || con->DataValueIndex < 0)
		return false;

	ConfigValue * cv = Config::_getValueInternal((ConfigValueNames)con->DataValueIndex);
	if (cv && cv->Getter == NULL)
		cv = NULL;

	if (con->DataValueValue.Type == ConfigValueTypes::Bool)
	{
		bool res = false;
		if (!Config::GetBool((ConfigValueNames)con->DataValueIndex, res))
			return false;

		if (cv)
		{
			ValueInstance vt = cv->Getter();
			if (vt.HasValue && vt.Type == ConfigValueTypes::Bool)
				res = vt.ToBool();
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return res == con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res != con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::Greater)
			return res && res != con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::Less)
			return !res && res != con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::GreaterOrEqual)
			return res || res == con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::LessOrEqual)
			return !res || res == con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::Contains)
			return res == con->DataValueValue.ToBool();
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res != con->DataValueValue.ToBool();
		return false;
	}

	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		int res = 0;
		if (!Config::GetInt((ConfigValueNames)con->DataValueIndex, res))
			return false;

		if (cv)
		{
			ValueInstance vt = cv->Getter();
			if (vt.HasValue && vt.Type == ConfigValueTypes::Int)
				res = vt.ToInt();
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return res == con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res != con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::Greater)
			return res > con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::Less)
			return res < con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::GreaterOrEqual)
			return res >= con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::LessOrEqual)
			return res <= con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::Contains)
			return res == con->DataValueValue.ToInt();
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res != con->DataValueValue.ToInt();
		return false;
	}

	if (con->DataValueValue.Type == ConfigValueTypes::Float)
	{
		float res = 0.0f;
		if (!Config::GetFloat((ConfigValueNames)con->DataValueIndex, res))
			return false;

		if (cv)
		{
			ValueInstance vt = cv->Getter();
			if (vt.HasValue && vt.Type == ConfigValueTypes::Float)
				res = vt.ToFloat();
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return res == con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res != con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::Greater)
			return res > con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::Less)
			return res < con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::GreaterOrEqual)
			return res >= con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::LessOrEqual)
			return res <= con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::Contains)
			return res == con->DataValueValue.ToFloat();
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res != con->DataValueValue.ToFloat();
		return false;
	}

	if (con->DataValueValue.Type == ConfigValueTypes::String)
	{
		std::string res;
		if (!Config::GetString((ConfigValueNames)con->DataValueIndex, res))
			return false;

		if (cv)
		{
			ValueInstance vt = cv->Getter();
			if (vt.HasValue && vt.Type == ConfigValueTypes::String)
				res = vt.ToString();
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return StringHelper::Equals(res, con->DataValueValue.ToString());
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return !StringHelper::Equals(res, con->DataValueValue.ToString());
		if (con->Operation == ProfileConditionOperators::Contains)
			return StringHelper::Contains(res, con->DataValueValue.ToString());
		if (con->Operation == ProfileConditionOperators::NotContains)
			return !StringHelper::Contains(res, con->DataValueValue.ToString());
		return false;
	}

	return false;
}

DEFCONDITION(Faction)
{
	if (!actor)
		return false;

	if (!con->DataValueValue.HasValue)
		return false;

	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		TESForm * form = LookupFormByID(con->DataValueValue.ToInt());
		if (!form)
			return false;

		TESFaction * faction = DYNAMIC_CAST(form, TESForm, TESFaction);
		if (!faction)
			return false;

		int res = 0;
		const int getFactionRank = 0x6A90C0;
		int a3 = actor == *g_thePlayer ? 1 : 0;
		_asm
		{
			push a3
				push faction
				mov ecx, actor
				call getFactionRank
				mov eax, res
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return res >= 0;
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res < 0;
		if (con->Operation == ProfileConditionOperators::Contains)
			return res >= 0;
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res < 0;
		return false;
	}

	return false;
}

DEFCONDITION(Perk)
{
	if (!actor)
		return false;

	if (!con->DataValueValue.HasValue)
		return false;

	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		TESForm * form = LookupFormByID(con->DataValueValue.ToInt());
		if (!form || form->formType != FormType::kFormType_Perk)
			return false;

		const int hasPerk = 0x6AA190;
		int res = 0;
		_asm
		{
			pushad
				pushfd
				mov ecx, actor
				push form
				call hasPerk
				mov res, eax
				popfd
				popad
		}

		if (con->Operation == ProfileConditionOperators::Equals)
			return res != 0;
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res == 0;
		if (con->Operation == ProfileConditionOperators::Contains)
			return res != 0;
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res == 0;
		return false;
	}

	return false;
}

typedef bool(*IsMagicEffectTarget)(ActiveEffect * eff);
void * _getEffectList(Actor * actor)
{
	if (!actor)
		return NULL;

	int res = 0;
	_asm
	{
		mov ecx, actor
			lea ecx, [ecx+0x54]
			mov eax, [ecx+28]
			call eax
			mov res, eax
	}

	return (void*)res;
}
bool _hasEffectWith(Actor * actor, IsMagicEffectTarget func)
{
	void * ptr = _getEffectList(actor);
	if (!ptr)
		return false;

	while (true)
	{
		ActiveEffect * eff = *((ActiveEffect**)ptr);
		if (!eff)
			return false;

		if (func(eff))
			return true;

		ptr = (void*)((int)ptr + 4);
	}
}

const char * _tempKw = NULL;
bool _isMEKeywordStr(ActiveEffect * eff)
{
	if (!eff->effect)
		return false;

	EffectSetting * set = eff->effect->mgef;
	if (!set)
		return false;

	for (UInt32 i = 0; i < set->keywordForm.numKeywords; i++)
	{
		const char * kwStr = set->keywordForm.keywords[i]->keyword.Get();
		if (!kwStr)
			continue;

		if (!_stricmp(kwStr, _tempKw))
			return true;
	}

	return false;
}

DEFCONDITION(MagicEffectKeyword)
{
	if (!actor)
		return false;

	if (!con->DataValueValue.HasValue)
		return false;

	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		TESForm * form = LookupFormByID(con->DataValueValue.ToInt());
		if (!form || form->formType != FormType::kFormType_Keyword)
			return false;

		const int hasMEKeyword = 0x6635B0;
		int res = 0;
		_asm
		{
			pushad
				pushfd
				mov ecx, actor
				lea ecx, [ecx+0x54]
				push 0
				push form
				call hasMEKeyword
				mov res, eax
				popfd
				popad
		}

		res = res & 0xFF;

		if (con->Operation == ProfileConditionOperators::Equals)
			return res != 0;
		if (con->Operation == ProfileConditionOperators::NotEquals)
			return res == 0;
		if (con->Operation == ProfileConditionOperators::Contains)
			return res != 0;
		if (con->Operation == ProfileConditionOperators::NotContains)
			return res == 0;
		return false;
	}

	if (con->DataValueValue.Type == ConfigValueTypes::String)
	{
		_tempKw = con->DataValueValue.ToString();
		return _hasEffectWith(actor, _isMEKeywordStr);
	}

	return false;
}

struct KeywordCache
{
	KeywordCache() { HasInitialized = false; }
	
	BGSKeyword * Get(std::string str)
	{
		Init();

		int formId = 0;
		for (std::unordered_map<int, std::string>::iterator itr = All.begin(); itr != All.end(); itr++)
		{
			if (StringHelper::Equals(itr->second, str))
			{
				formId = itr->first;
				break;
			}
		}

		return Get(formId);
	}

	BGSKeyword * Get(int formId)
	{
		if (formId == 0)
			return NULL;

		TESForm * form = LookupFormByID(formId);
		return DYNAMIC_CAST(form, TESForm, BGSKeyword);
	}

private:
	void Init()
	{
		if (HasInitialized)
			return;

		HasInitialized = true;

		DataHandler * dh = DataHandler::GetSingleton();
		if (!dh)
			return;

		for (int i = 0; i < (int)dh->keywords.count; i++)
		{
			BGSKeyword * kw = NULL;
			if (!dh->keywords.GetNthItem(i, kw))
				continue;

			const char * kwStr = kw->keyword.Get();
			if(kwStr)
				All[kw->formID] = kwStr;
		}
	}

	bool HasInitialized;

	std::unordered_map<int, std::string> All;
};

KeywordCache kwCache;

DEFCONDITION(Keyword)
{
	if (!actor)
		return false;

	if (!con->DataValueValue.HasValue)
		return false;

	bool has = false;
	if (con->DataValueValue.Type == ConfigValueTypes::String)
	{
		std::string kwComp = con->DataValueValue.ToString();
		BGSKeyword * kwForm = kwCache.Get(kwComp);
		int kwId = kwForm != NULL ? kwForm->formID : 0;

		con->DataValueValue = ValueInstance(kwId, true);
	}
	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		int kwId = con->DataValueValue.ToInt();
		BGSKeyword * kwForm = kwCache.Get(kwId);
		if (kwForm != NULL)
		{
			int res = 0;
			const int hasKeyword = 0x6A9550;
			_asm
			{
				mov ecx, actor
					push kwForm
					call hasKeyword
					mov res, eax
			}

			has = (res & 0xFF) != 0;
		}
	}

	switch (con->Operation)
	{
	case ProfileConditionOperators::Equals:
	case ProfileConditionOperators::Contains:
		return has;

		case ProfileConditionOperators::NotEquals:
		case ProfileConditionOperators::NotContains:
			return !has;
	}

	return false;
}

DEFCONDITION(WornHasKeyword)
{
	if (!actor)
		return false;

	if (!con->DataValueValue.HasValue)
		return false;

	bool has = false;
	if (con->DataValueValue.Type == ConfigValueTypes::String)
	{
		std::string kwComp = con->DataValueValue.ToString();
		BGSKeyword * kwForm = kwCache.Get(kwComp);
		int kwId = kwForm != NULL ? kwForm->formID : 0;

		con->DataValueValue = ValueInstance(kwId, true);
	}
	if (con->DataValueValue.Type == ConfigValueTypes::Int)
	{
		int kwId = con->DataValueValue.ToInt();
		BGSKeyword * kwForm = kwCache.Get(kwId);
		if (kwForm != NULL)
		{
			int resCont = 0;
			int resHas = 0;
			const int getContainer = 0x476800;
			const int hasWornKeyword = 0x4769C0;
			_asm
			{
				push actor
				call getContainer
				add esp, 4
				mov resCont, eax
			}

			if (resCont != 0)
			{
				_asm
				{
					push kwForm
						push resCont
						call hasWornKeyword
						add esp, 8
						mov resHas, eax
				}
			}

			has = (resHas & 0xFF) != 0;
		}
	}

	switch (con->Operation)
	{
	case ProfileConditionOperators::Equals:
	case ProfileConditionOperators::Contains:
		return has;

	case ProfileConditionOperators::NotEquals:
	case ProfileConditionOperators::NotContains:
		return !has;
	}

	return false;
}
