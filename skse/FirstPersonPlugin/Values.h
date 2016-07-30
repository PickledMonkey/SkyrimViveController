#pragma once

#include "Config.h"
#include "skse/GameReferences.h"
#include "skse/GameObjects.h"
#include "skse/GameMenus.h"
#include "skse/GameRTTI.h"

#define DEFUPDATE(name) void _Update##name(TESObjectREFR * obj, Actor * actor, ConfigValue * val)

DEFUPDATE(Walking)
{
	bool result = false;
	if (actor != NULL)
		result = (actor->actorState.flags04 & 0x40) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Running)
{
	bool result = false;
	if (actor != NULL)
	{
		int res = 0;
		const int isRunning = 0x6AB210;
		_asm
		{
			mov ecx, actor
				call isRunning
				mov res, eax
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Dead)
{
	bool result = false;
	if (actor != NULL)
	{
		int dead = 0;
		const int isDead = 0x8DA830;
		_asm
		{
			push actor
				push 0
				push 0
				call isDead
				mov dead, eax
				add esp, 0xC
		}

		result = (dead & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(KillMove)
{
	bool result = false;
	if (actor != NULL)
		result = (actor->flags2 & 0x4000) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(WereWolf)
{
	bool result = false;
	if (actor != NULL && actor->race != NULL && actor->race->editorId.data)
		result = StringHelper::Contains(actor->race->editorId.data, "werewolf");

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(VampireLord)
{
	bool result = false;
	if (actor != NULL && actor->race != NULL && actor->race->editorId.data)
		result = StringHelper::Contains(actor->race->editorId.data, "vampirebeast");

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Dialogue)
{
	bool result = false;

	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
	{
		{
			StringHolder bs("BarterMenu");
			if (mm->IsMenuOpen(bs.Get()))
				result = true;
		}

		if (!result)
		{
			StringHolder bs("Crafting Menu");
			if (mm->IsMenuOpen(bs.Get()))
				result = true;
		}
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

int _attackMask = 0;
int _attackTypes = 0;
int _reCalcAttack = 1;

#define ATTACK_TYPE_UNARMED 0
#define ATTACK_TYPE_SPELL 1
#define ATTACK_TYPE_TWO_HANDED_WEAPON 2
#define ATTACK_TYPE_ONE_HANDED_WEAPON 3
#define ATTACK_TYPE_BOW 4
#define ATTACK_TYPE_CROSSBOW 5
#define ATTACK_MASK_RIGHTHAND (_attackTypes & 0xFFFF)
#define ATTACK_MASK_LEFTHAND ((_attackTypes >> 16) & 0xFFFF)

void _updateAttack(Actor * actor)
{
	_reCalcAttack = 0;
	_attackMask = 0;
	_attackTypes = 0;

	int ptr = *((int*)0x1B2EF3C);
	if (ptr == 0)
		return;

	if ((*((int*)(ptr + 0x14)) & 0xFF) == 2)
		_attackMask |= 1;
	else if ((*((int*)(ptr + 0x24)) & 0xFF) != 0)
		_attackMask |= 1;
	if (((*((int*)(ptr + 0x24)) & 0xFF00) >> 8) != 0)
		_attackMask |= 2;

	for (int i = 0; i < 2; i++)
	{
		if ((_attackMask & (1 << i)) == 0)
			continue;

		int type = 0;

		TESForm * hand = actor->GetEquippedObject(i == 1);
		if (!hand)
			type |= 1 << ATTACK_TYPE_UNARMED;
		else if (hand->formType == FormType::kFormType_Spell || hand->formType == FormType::kFormType_LeveledSpell)
			type |= 1 << ATTACK_TYPE_SPELL;
		else if (hand->IsWeapon())
		{
			TESObjectWEAP * wpn = DYNAMIC_CAST(hand, TESForm, TESObjectWEAP);
			if (!wpn || wpn->gameData.type == TESObjectWEAP::GameData::kType_HandToHandMelee)
				type |= 1 << ATTACK_TYPE_UNARMED;
			else
			{
				//  || wpn->gameData.type == TESObjectWEAP::GameData::kType_HandToHandMelee
				switch (wpn->gameData.type)
				{
				case TESObjectWEAP::GameData::kType_OneHandAxe:
				case TESObjectWEAP::GameData::kType_OneHandMace:
				case TESObjectWEAP::GameData::kType_OneHandSword:
				case TESObjectWEAP::GameData::kType_OneHandDagger:
					type |= 1 << ATTACK_TYPE_ONE_HANDED_WEAPON;
					break;

				case TESObjectWEAP::GameData::kType_TwoHandAxe:
				case TESObjectWEAP::GameData::kType_TwoHandSword:
					type |= 1 << ATTACK_TYPE_TWO_HANDED_WEAPON;
					break;

				case TESObjectWEAP::GameData::kType_Bow:
				case TESObjectWEAP::GameData::kType_Bow2:
					type |= 1 << ATTACK_TYPE_BOW;
					break;

				case TESObjectWEAP::GameData::kType_CrossBow:
				case TESObjectWEAP::GameData::kType_CBow:
					type |= 1 << ATTACK_TYPE_CROSSBOW;
					break;

				default:
					break;
				}
			}
		}

		_attackTypes |= (type << (i * 16));
	}
}

DEFUPDATE(AimMagic)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_SPELL)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_SPELL)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimMagicLeft)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_SPELL)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimMagicRight)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_SPELL)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimBow)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_BOW)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_BOW)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimCrossBow)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_CROSSBOW)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_CROSSBOW)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimOneHanded)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimOneHandedLeft)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimOneHandedRight)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimTwoHanded)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimTwoHandedLeft)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimTwoHandedRight)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimAnyHanded)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimAnyHandedLeft)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_LEFTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(AimAnyHandedRight)
{
	bool result = false;

	if (actor && _reCalcAttack == 1)
		_updateAttack(actor);

	result = ((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_TWO_HANDED_WEAPON)) != 0 ||
		((ATTACK_MASK_RIGHTHAND)& (1 << ATTACK_TYPE_ONE_HANDED_WEAPON)) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Mount)
{
	bool result = false;
	if (actor)
	{
		int res = 0;
		const int isMounted = 0x8DCB40;
		_asm
		{
			push actor
				push 0
				push 0
				call isMounted
				mov res, eax
				add esp, 0xC
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Sprinting)
{
	bool result = false;
	if (actor)
	{
		int res = 0;
		const int isSprinting = 0x6BBEB0;
		_asm
		{
			mov ecx, actor
				call isSprinting
				mov res, eax
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(WeaponOut)
{
	bool result = false;
	if (actor)
		result = actor->actorState.IsWeaponDrawn();

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Sneaking)
{
	bool result = false;
	if (actor)
	{
		int res = 0;
		const int isSneaking = 0x4D9290;
		_asm
		{
			mov ecx, actor
			call isSneaking
			mov res, eax
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Crafting)
{
	bool result = false;

	MenuManager * mm = MenuManager::GetSingleton();
	if (mm)
	{
		StringHolder bs("Crafting Menu");
		if (mm->IsMenuOpen(bs.Get()))
			result = true;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Sitting)
{
	bool result = false;

	if (actor)
	{
		int res = 0;
		const int isSitting = 0x8DA8E0;
		_asm
		{
			push actor
				push 0
				push 0
				call isSitting
				add esp, 0xC
				mov res, eax
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Jumping)
{
	bool result = false;

	if (actor)
	{
		int res = 0;
		const int isJumping = 0x6AB2F0;
		_asm
		{
			mov ecx, actor
				call isJumping
				mov res, eax
		}

		result = (res & 0xFF) != 0;
	}

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Race)
{
	std::string result;
	std::string prev;

	auto prevItr = val->Values.find(0);
	if (prevItr != val->Values.end() && prevItr->second.Type == ConfigValueTypes::String && prevItr->second.HasValue)
		prev = prevItr->second.ToString();

	if (actor && actor->race && actor->race->editorId.data)
		result = actor->race->editorId.data;

	if (StringHelper::Equals(result, prev))
		return;

	if (val->Type == ConfigValueTypes::String)
		val->Values[0] = ValueInstance(result.c_str(), true);
}

DEFUPDATE(Swimming)
{
	bool result = false;
	if (actor)
		result = (actor->actorState.flags04 & ActorState::kState_Swimming) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(CameraState)
{
	int result = Camera::GetSingleton()->GetCameraStateId();

	if (val->Type == ConfigValueTypes::Int && result >= 0)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(Mounting)
{
	bool mounting = false;
	bool dismounting = false;

	Config::_getMounting(mounting, dismounting);

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(mounting, true);
}

DEFUPDATE(Dismounting)
{
	bool mounting = false;
	bool dismounting = false;

	Config::_getMounting(mounting, dismounting);

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(dismounting, true);
}

DEFUPDATE(IFPV)
{
	bool isActivated = Config::_isActive();
	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(isActivated, true);
}

DEFUPDATE(IFPV2)
{
	bool isActivated = Camera::GetSingleton()->IsActivated(false);
	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(isActivated, true);
}

DEFUPDATE(TargetOverwrite)
{
	bool isOverwritten = Camera::GetSingleton()->IsTargetOverwritten();
	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(isOverwritten, true);
}

DEFUPDATE(TargetBaseId)
{
	int baseFormId = 0;
	if (obj && obj->baseForm)
		baseFormId = obj->baseForm->formID;

	if (val->Type == ConfigValueTypes::Int)
		val->Values[0] = ValueInstance(baseFormId, true);
}

DEFUPDATE(Indoors)
{
	bool indoors = false;
	if (obj && obj->parentCell)
		indoors = (obj->parentCell->unk2C & 1) != 0;

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(indoors, true);
}

DEFUPDATE(Grabbing)
{
	int plr = *((int*)0x1B2E8E4);
	int objRef = *((int*)(plr + 0x568)); // grabbing object reference handle.
	bool grabbing = objRef != 0 && objRef != *((int*)0x1310630);

	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(grabbing, true);
}

DEFUPDATE(Furniture)
{
	std::string value;

	if (actor && actor->processManager && actor->processManager->middleProcess)
	{
		int handle = actor->processManager->middleProcess->furnitureHandle;

		OBJRefHolder holder = OBJRefHolder(handle);
		TESObjectREFR * refr = holder.GetRef();

		if (refr != NULL)
		{
			const char * name = NULL;
			static int _getName = 0x4DE820;
			_asm
			{
				pushad
					pushfd

					mov ecx, refr
					call _getName
					mov name, eax

					popfd
					popad
			}
			if (name)
				value = name;
		}
	}

	if (val->Type == ConfigValueTypes::String)
		val->Values[0] = ValueInstance(value.c_str(), true);
}

DEFUPDATE(IsCameraOnArrow)
{
	bool result = Camera::GetSingleton()->IsArrow();
	
	if (val->Type == ConfigValueTypes::Bool)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(LookX)
{
	float temp = 0.0f;
	float result = 0.0f;
	Camera::GetSingleton()->GetLookData(result, temp);

	if (val->Type == ConfigValueTypes::Float)
		val->Values[0] = ValueInstance(result, true);
}

DEFUPDATE(LookZ)
{
	float temp = 0.0f;
	float result = 0.0f;
	Camera::GetSingleton()->GetLookData(temp, result);

	if (val->Type == ConfigValueTypes::Float)
		val->Values[0] = ValueInstance(result, true);
}

void _newCalc()
{
	_reCalcAttack = 1;
}
