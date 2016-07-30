#include "Camera.h"
#include "Config.h"
#include "skse/GameForms.h"
#include "skse/GameRTTI.h"
#include "skse/GameMenus.h"
#include "skse/GameObjects.h"
#include "skse/GameFormComponents.h"

float NewUpDownLimit = 1.5707964f; // 0x3FC90FDB;
float DefaultNearDistance = -1.0f;
float AdjustFarClipMin = -1.0f;
float AdjustFarClipStart = -1.0f;

Camera::Camera()
{
	isEnabled = 0;
	wasEnabled = 0;
	frameCounter = 0;
	headTrackOverwrite = 0;
	findHeadPos = -1;
	timeSinceLastFrame = 0.0f;
	timeSinceLastFrame2 = 0.0f;
	_iFlags = 0;
	_nsFormId = 0;
	_nsRotX = 0.0f;
	_nsRotZ = 0.0f;
	_wasActive2 = 0;
	_targetOverwrite = 0;
	_controlOverwrite = 0;
	_lastTargetPos = NULL;
	_lastTargetRef = 0;
	_userMoved = 0;
	_hideHeadObject = 0;
	_hideHeadHelmet = 0;
	_helmetUpdateTimer = 0;
	_extraRotMouse[0] = 0.0f;
	_extraRotMouse[1] = 0.0f;
	for (int i = 0; i < 4; i++)
		_nextRotate[i] = 0.0f;
	_oldHorseMillis = -1;
	_lastWasArrow = false;
	_failedCameraTarget = 0;
}

void Camera::_UpdateFrame()
{
	LOCK_MUTEX(mtx);
	{
		// Increase frame counter.
		frameCounter++;

		// Calculate time passed.
		{
			timeSinceLastFrame2 = *((float*)0x1B4ADE0);
			timeSinceLastFrame = *((float*)0x1B4ADE4);
			//float multTime = *((float*)0x12C2410);
			if (IsGamePaused())
				timeSinceLastFrame2 = 0.0f;

			/*timeSinceLastFrame = timeSinceLastFrame2;
			if (multTime > 0.01f)
				timeSinceLastFrame /= multTime;
			else
				timeSinceLastFrame /= 0.01f;*/
		}

		// Calculate camera target for this frame. Only allowed to use pointers within this scope.
		_nsTargetObj = GetCameraTarget(true);
		_nsTargetActor = _nsTargetObj != NULL ? DYNAMIC_CAST(_nsTargetObj, TESObjectREFR, Actor) : NULL;
		if (_nsTargetObj != NULL)
		{
			_nsFormId = _nsTargetObj->formID;
			_nsRotX = _nsTargetObj->rot.x;

			float nz = _nsTargetObj->rot.z;
			if (_nsRotZ != nz)
			{
				_nsRotZ = nz;
				//SetUserMoved();
			}
		}

		_lastWasArrow = false;
		if (_nsTargetObj)
		{
			switch (_nsTargetObj->formType)
			{
			case kFormType_Arrow:
			case kFormType_Missile:
				_lastWasArrow = true;
				break;
			}
		}

		UpdateHideHead();
		if (_helmetUpdateTimer > 0)
			_helmetUpdateTimer--;
		else
			UpdateHideHelmet();

		float extraMouse = 0.0f;
		Config::GetFloat(ConfigValueNames::fExtraRotationMouse, extraMouse);
		if (extraMouse <= 0.0f)
		{
			float decay = timeSinceLastFrame * (float)M_PI;
			for (int i = 0; i < 2; i++)
			{
				if (_extraRotMouse[i] > 0.0f)
				{
					_extraRotMouse[i] -= decay;
					if (_extraRotMouse[i] < 0.0f)
						_extraRotMouse[i] = 0.0f;
				}
				else if (_extraRotMouse[i] < 0.0f)
				{
					_extraRotMouse[i] += decay;
					if (_extraRotMouse[i] > 0.0f)
						_extraRotMouse[i] = 0.0f;
				}
			}
		}

		// Update game states.
		UpdateIFlags();

		// Update profile activity.
		Config::Update();

		// Recalculate this.
		CalculateIsEnabled();
		if (IsEnabled())
			CalculateIsActivated();

		// Update config after this.
		bool activeNow = IsActivated(true);
		Config::_setActive(activeNow);
		if ((_wasActive2 != 0) != activeNow)
		{
			_wasActive2 = activeNow ? 1 : 0;
			OnActiveChanged(activeNow);
		}

		// Every ~2 seconds reset cached head index in case we had a bad one.
		if (findHeadPos > 1 && (frameCounter % 120) == 0)
			findHeadPos = -1;

		// Calculate overwrite data.
		if(IsEnabled())
		{
			bool partial = false;
			if (!IsActivated(true))
				partial = true;
			CalculateOverwrite(partial);
		}

		// Update head tracking.
		UpdateHeadTrack();

		// User didn't move this frame. This gets overwritten if they did.
		if (_userMoved > 0)
			_userMoved--;
		UpdateUserMove();
		UpdateFocus();

		// Rotate.
		_updateRotate(timeSinceLastFrame);
		
		// Reset pointers.
		_nsTargetObj = NULL;
		_nsTargetActor = NULL;

		// Set for next frame.
		wasEnabled = isEnabled;
	}
}

Point Camera::GetOverwritePosition()
{
	LOCK_MUTEX(mtx);

	Point pt;

	memcpy(&pt, &data.position, sizeof(Point));

	return pt;
}

Matrix3 Camera::GetOverwriteRotation()
{
	LOCK_MUTEX(mtx);

	Matrix3 mt;

	memcpy(&mt, &data.rotation, sizeof(Matrix3));

	return mt;
}

void Camera::_UpdatePosition(Point * pos)
{
	if (!IsActivated(true))
		return;

	Point pt = GetOverwritePosition();
	if(!pt.IsEmpty())
		memcpy(pos, &pt, sizeof(Point));
}

void Camera::_UpdateRotation(Matrix3 * rot)
{
	if (!IsActivated(true))
		return;

	Matrix3 mt = GetOverwriteRotation();
	memcpy(rot, &mt, sizeof(Matrix3));
}

void Camera::CalculateIsEnabled()
{
	// Reset.
	isEnabled = 0;

	// Make sure camera target is loaded.
	TESObjectREFR * target = _nsTargetObj;
	if (!target)
		return;

	// Make sure we have valid camera state.
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam || !cam->cameraState)
		return;

	// Check config.
	{
		bool enabled = false;

		if (Config::GetBool(ConfigValueNames::bEnabled, enabled))
		{
			if (!enabled)
				return;
		}
	}

	isEnabled = 1;
}

void Camera::CalculateIsActivated()
{
	// Check config.
	{
		bool enabled = false;

		if (Config::GetBool(ConfigValueNames::bActivated, enabled))
		{
			if (!enabled)
				return;
		}
	}

	isEnabled |= 2;

	if (IsThirdPerson(true))
		isEnabled |= 4;
}

// Check if camera should be overwritten.
bool Camera::IsEnabled()
{
	return (isEnabled & 1) != 0;
}

bool Camera::IsActivated(bool inView)
{
	if(!inView)
		return (isEnabled & 2) != 0;
	return (isEnabled & 4) != 0;
}

// This could theoretically be any object.
TESObjectREFR * Camera::GetCameraTarget(bool must3dLoaded)
{
	if (_targetOverwrite != 0)
	{
		bool isOkFail = false;

		TESForm * form = LookupFormByID(_targetOverwrite);
		if (form != NULL)
		{
			TESObjectREFR * objRef = DYNAMIC_CAST(form, TESForm, TESObjectREFR);
			if (objRef != NULL)
			{
				if (!must3dLoaded || (objRef->loadedState && objRef->loadedState->node))
				{
					_failedCameraTarget = 0;
					return objRef;
				}
				_failedCameraTarget++;
				isOkFail = true;
			}
		}

		if (!isOkFail || _failedCameraTarget > 60)
		{
			_targetOverwrite = 0;
			_failedCameraTarget = 0;
		}
	}

	return *g_thePlayer;
}

// Resolve best node for camera.
NiAVObject * Camera::ResolveTable(bool allowEye, TESObjectREFR * obj, bool allowOverwrite, bool mustHead, bool allowCache)
{
	NiAVObject * node = NULL;

	if (allowOverwrite)
	{
		std::string cameraOverwrite;
		Config::GetString(ConfigValueNames::sCameraNode, cameraOverwrite);

		if (!cameraOverwrite.empty())
		{
			node = ResolveNode(cameraOverwrite.c_str(), obj);
			if (node != NULL)
				return node;
		}
	}

	const int node_tableSize = 27;
	static const char * node_strTable[node_tableSize] = {
		"NPCEyeBone",
		"NPC Head [Head]",
		"NPC Head",
		"Head [Head]",
		"HEAD",
		"Scull",
		"FireAtronach_Head [Head]",
		"ElkScull",
		"Canine_Head",
		"DragPriestNPC Head [Head]",
		"DwarvenSpiderHead_XYZ",
		"Goat_Head",
		"ChaurusFlyerHead",
		"Boar_Reikling_Head",
		"NPC_mainbody_bone",
		"RabbitHead",
		"Horker_Head01",
		"HorseScull",
		"IW Head",
		"Mammoth Head",
		"MagicEffectsNode",
		"Sabrecat_Head [Head]",
		"SlaughterfishHead",
		"Wisp Head",
		"Witchlight Body",
		"NPC Spine2 [Spn2]",
		"NPC Root [Root]"
	};

	if (allowEye)
	{
		node = ResolveNode(node_strTable[0], obj);
		if (node != NULL)
			return node;
	}

	// Cached position.
	if (allowCache && findHeadPos >= 0)
	{
		node = ResolveNode(node_strTable[findHeadPos], obj);
		if (node != NULL)
			return node;

		findHeadPos = -1;
	}

	// Find best after eye.
	for (int i = 1; i < node_tableSize; i++)
	{
		if (mustHead)
		{
			if (!StringHelper::Contains(node_strTable[i], "head"))
				continue;
		}

		node = ResolveNode(node_strTable[i], obj);
		if (node != NULL)
		{
			// Cache position so we don't have to resolve whole list every time if player decides to become a slaughterfish.
			if(allowCache)
				findHeadPos = i;
			return node;
		}
	}

	return NULL;
}

// Get head node.
NiAVObject * Camera::GetHead(TESObjectREFR * obj)
{
	NiAVObject * node = ResolveTable(false, obj, true, false, true);
	if (node == NULL)
	{
		if (obj == NULL)
			obj = _nsTargetObj;

		if (obj != NULL && obj->loadedState != NULL)
		{
			NiNode * nd = obj->loadedState->node;
			if (nd != NULL)
			{
				node = nd;
				if (nd->m_children.m_emptyRunStart > 0)
				{
					node = nd->m_children.m_data[0];
					if (node == NULL)
						node = nd;
				}
			}
		}
	}

	return node;
}

// Get eye node.
NiAVObject * Camera::GetEye(TESObjectREFR * obj)
{
	NiAVObject * node = ResolveTable(true, obj, true, false, true);
	if (node == NULL)
	{
		if (obj == NULL)
			obj = _nsTargetObj;

		if (obj != NULL && obj->loadedState != NULL)
		{
			NiNode * nd = obj->loadedState->node;
			if (nd != NULL)
			{
				node = nd;
				if (nd->m_children.m_emptyRunStart > 0)
				{
					node = nd->m_children.m_data[0];
					if (node == NULL)
						node = nd;
				}
			}
		}
	}

	return node;
}

// Resolve node by name.
NiAVObject * Camera::ResolveNode(const char* nodeName, TESObjectREFR * obj)
{
	if (obj == NULL)
		obj = _nsTargetObj;

	if (!obj || !obj->loadedState)
		return NULL;

	NiAVObject * parentNode = obj->loadedState->node;
	if (nodeName && *nodeName && parentNode)
	{
		StringHolder bstr(nodeName);
		NiAVObject * node = parentNode->GetObjectByName(&bstr.Get()->data);
		return node;
	}

	return NULL;
}

void Camera::_UpdateRestrict(float * x, float * y)
{
	if (!IsActivated(false))
		return;

	// Horizontal.
	if (*x != 0.0f)
	{
		float res = 90.0f;
		Config::GetFloat(ConfigValueNames::fRestrictAngleHorizontal, res);
		if (res < 0.0f)
			res = 0.0f;
		res *= DegToRad;

		if (*x < -res)
			*x = -res;
		else if (*x > res)
			*x = res;
	}

	// Vertical.
	if (*y != 0.0f)
	{
		float res = 80.0f;
		Config::GetFloat(ConfigValueNames::fRestrictAngleVertical, res);
		if (res < 0.0f)
			res = 0.0f;
		res *= DegToRad;

		float objY = -_nsRotX;

		*y += objY;

		if (*y < -res)
			*y = -res;
		else if (*y > res)
			*y = res;

		*y -= objY;
	}
}

void Camera::_UpdateRestrict2(Actor * actor, float * x, float * y)
{
	if (!IsActivated(false))
		return;

	if (!actor || actor->formID != _nsFormId)
		return;

	float yRes = 80.0f;
	Config::GetFloat(ConfigValueNames::fRestrictAngleVertical, yRes);

	yRes *= DegToRad;

	*y = yRes;
	*x = -yRes;
}

float Camera::RestrictRad(float rad)
{
	while (rad < 0.0f)
		rad += (float)M_PI * 2.0f;
	while (rad >(float)M_PI * 2.0f)
		rad -= (float)M_PI * 2.0f;
	return rad;
}

int Camera::_CanFadeOut()
{
	if (!IsEnabled())
		return 1;

	if (IsControlOverwritten())
		return 0;

	bool block = IsActivated(true);
	//Config::GetBool(ConfigValueNames::bBlockFadeOut, block);
	if (block)
		return 0;

	return 1;
}

void Camera::UpdateHeadTrack()
{
	if (!IsEnabled())
		return;

	Actor * actor = _nsTargetActor;
	if (!actor)
		return;

	bool headTracking = false;
	Config::GetBool(ConfigValueNames::bHeadtrack, headTracking);

	if (headTracking)
	{
		switch (GetCameraStateId())
		{
		case PlayerCamera::kCameraState_Dragon:
		case PlayerCamera::kCameraState_Furniture:
		case PlayerCamera::kCameraState_Horse:
		case PlayerCamera::kCameraState_ThirdPerson1:
		case PlayerCamera::kCameraState_ThirdPerson2:
		case PlayerCamera::kCameraState_Free:
			break;

		default:
			headTracking = false;
			break;
		}
	}

	if (headTracking)
	{
		Point pLook = GetCameraLookAt();
		if (!pLook.IsEmpty())
			SetHeadTrackTarget(actor, &pLook);
		else
			SetHeadTrackTarget(actor, NULL);
	}
	else
	{
		if(headTrackOverwrite == actor->formID)
			SetHeadTrackTarget(actor, NULL);
	}
}

Point Camera::GetCameraLookAt(float distance)
{
	Matrix3 camRot = GetCameraDirection();

	// Trick to make this work when fRotationFromHead is set to > 0
	/*if (IsActivated(true))
	{
		Matrix3 headRot = data.rotation;

		float fScale = 0.0f;
		if (Config::GetFloat(ConfigValueNames::fRotationFromHead, fScale))
		{
			if (fScale < 0.0f)
				fScale = 0.0f;
			else if (fScale > 1.0f)
				fScale = 1.0f;
		}

		camRot = Matrix3::Interpolate(camRot, headRot, 1.0f - fScale);
	}*/

	Transform tf;
	memcpy(&tf.pos, &data.position, sizeof(Point));
	memcpy(&tf.rot, &camRot, sizeof(Matrix3));
	tf.scale = 1.0f;

	return Transform::Translate(tf, 0.0f, 0.0f, distance);
}

Matrix3 Camera::GetCameraDirection()
{
	Matrix3 mt = Matrix3::Identity();

	mt = Matrix3::RotateX(mt, data.cameraOffsetXAbs);
	mt = Matrix3::RotateZ(mt, -data.cameraOffsetZAbs);

	return mt;
}

Matrix3 Camera::GetCameraDirectionRel()
{
	Matrix3 mt = Matrix3::Identity();

	mt = Matrix3::RotateX(mt, data.cameraOffsetXRelNormalized);
	mt = Matrix3::RotateZ(mt, -data.cameraOffsetZRelNormalized);

	return mt;
}

int Camera::_IsHeadTrackOverwritten(Actor * actor)
{
	if (!actor)
		return 0;

	return actor->formID == headTrackOverwrite ? 1 : 0;
}

int _setHTTarget = 0x723E40;
int _clearHTTarget = 0x71F360;
int _setAG = 0x650C60;
void Camera::SetHeadTrackTarget(Actor * actor, Point * pos)
{
	if (!actor)
		return;

	bool isPlayer = actor->formID == 0x14;
	int processManager = *((int*)((int)actor + 136));
	if (processManager == 0)
		return;

	const char * npcCStr = "IsNPC";

	if (pos == NULL)
	{
		if (headTrackOverwrite == actor->formID)
		{
			headTrackOverwrite = 0;

			_asm
			{
				mov ecx, processManager
					call _clearHTTarget
			}

			if (isPlayer)
			{
				actor->actorState.flags08 &= ~8;

				StringHolder npcStr(npcCStr);
				BSFixedString * bnStr = npcStr.Get();
				_asm
				{
					mov ecx, actor
						lea ecx, [ecx + 0x20]
						push 0
						mov eax, bnStr
						push eax
						call _setAG
				}
			}
		}
	}
	else
	{
		if (headTrackOverwrite != actor->formID)
		{
			headTrackOverwrite = actor->formID;

			actor->actorState.flags08 |= 8;

			if (isPlayer)
			{
				StringHolder npcStr(npcCStr);
				BSFixedString * bnStr = npcStr.Get();
				_asm
				{
					mov ecx, actor
						lea ecx, [ecx + 0x20]
						push 1
						mov eax, bnStr
						push eax
						call _setAG
				}
			}
		}

		_asm
		{
			mov eax, pos
			push eax
			mov eax, actor
			push eax
			mov ecx, processManager
			call _setHTTarget
		}
	}
}

void Camera::CalculateOverwrite(bool partial)
{
	// Get actor.
	Actor * actor = _nsTargetActor;

	// Calculate camera offsets.
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam != NULL && cam->cameraState != NULL)
	{
		switch (cam->cameraState->stateId)
		{
			case PlayerCamera::kCameraState_ThirdPerson2:
			case PlayerCamera::kCameraState_Horse:
			case PlayerCamera::kCameraState_Dragon:
				data.cameraOffsetZRel = *((float*)((int)cam->cameraState + 0xAC));
				data.cameraOffsetZAbs = data.cameraOffsetZRel + (actor != NULL ? actor->rot.z : 0.0f);
				data.cameraOffsetZRelNormalized = data.cameraOffsetZRel;
				data.cameraOffsetXRel = *((float*)((int)cam->cameraState + 0xB0));
				data.cameraOffsetXAbs = data.cameraOffsetXRel - (actor != NULL ? actor->rot.x : 0.0f);
				data.cameraOffsetXRelNormalized = data.cameraOffsetXAbs;
				break;
		}
	}

	// Calculate new position for camera.
	{
		NiAVObject * eye = GetEye(NULL);
		if (eye)
		{
			float cfgOffsetH = 0.0f;
			float cfgOffsetV = 0.0f;
			float cfgOffsetD = 0.0f;
			Config::GetFloat(ConfigValueNames::fPositionOffsetHorizontal, cfgOffsetH);
			Config::GetFloat(ConfigValueNames::fPositionOffsetVertical, cfgOffsetV);
			Config::GetFloat(ConfigValueNames::fPositionOffsetDepth, cfgOffsetD);

			Transform tf;

			memcpy(&tf, &eye->m_worldTransform, sizeof(Transform));
			Point newPos = Transform::Translate(tf, cfgOffsetH, cfgOffsetV, cfgOffsetD);

			memcpy(&data.position, &newPos, sizeof(Point));
		}
	}

	// Only wanted to update camera direction and position.
	if (partial)
		return;

	// Calculate new rotation for camera.
	{
		{
			NiAVObject * head = GetHead(NULL);
			if (head)
			{
				Matrix3 headRot;
				memcpy(&headRot, &head->m_worldTransform.rot, sizeof(Matrix3));
				if (IsArrow())
				{
					Matrix3 tempCh = Matrix3::Identity();
					tempCh.data[0][0] = -1.0f;
					tempCh.data[1][1] = -1.0f;
					headRot = Matrix3::Multiply(headRot, tempCh);
				}
				Matrix3 camRot = GetCameraDirection();

				float fScale = 1.0f;
				if (Config::GetFloat(ConfigValueNames::fRotationFromHead, fScale))
				{
					if (fScale < 0.0f)
						fScale = 0.0f;
					else if (fScale > 1.0f)
						fScale = 1.0f;
				}

				if (fScale != 1.0f)
					headRot = Matrix3::Interpolate(camRot, headRot, fScale);

				float qScale = 0.0f;
				if (Config::GetFloat(ConfigValueNames::fExtraRotationFromCamera, qScale))
				{
					if (qScale < 0.0f)
						qScale = 0.0f;
					else if (qScale > 1.0f)
						qScale = 1.0f;
				}

				if (qScale > 0.0f)
				{
					Matrix3 relCamRot = GetCameraDirectionRel();
					Matrix3 newHeadRot = Matrix3::Multiply(headRot, relCamRot);

					headRot = Matrix3::Interpolate(headRot, newHeadRot, qScale);
				}

				float mScale = 0.0f;
				Config::GetFloat(ConfigValueNames::fExtraRotationMouse, mScale);
				if (mScale > 0.0f)
				{
					Matrix3 mtm = Matrix3::Identity();

					mtm = Matrix3::RotateX(mtm, _extraRotMouse[1] * mScale);
					mtm = Matrix3::RotateZ(mtm, -_extraRotMouse[0] * mScale);

					headRot = Matrix3::Multiply(headRot, mtm);
				}

				memcpy(&data.rotation, &headRot, sizeof(Matrix3));
			}
		}
	}
}

void Camera::UpdateIFlags()
{
	_iFlags = 0;

	if(_isThirdPerson())
		_iFlags |= 1;
	if (_isGamePaused())
		_iFlags |= 2;
	if (_isPlayerCamera())
		_iFlags |= 4;
	if (_isFirstPerson())
		_iFlags |= 8;
}

bool Camera::_isThirdPerson()
{
	switch (GetCameraStateId())
	{
	case PlayerCamera::kCameraState_Bleedout:
	case PlayerCamera::kCameraState_Dragon:
	case PlayerCamera::kCameraState_Furniture:
	case PlayerCamera::kCameraState_Horse:
	case PlayerCamera::kCameraState_ThirdPerson1:
	case PlayerCamera::kCameraState_ThirdPerson2:
	case PlayerCamera::kCameraState_Transition:
	case PlayerCamera::kCameraState_VATS:
		return true;
		break;

	default:
		break;
	}

	return false;
}

bool Camera::_isFirstPerson()
{
	return GetCameraStateId() == PlayerCamera::kCameraState_FirstPerson;
}

bool Camera::_isGamePaused()
{
	PlayerCharacter * plr = *g_thePlayer;
	if (!plr || !plr->loadedState)
		return true;

	int gMain = *((int*)0x1B2E860);
	if (gMain == 0)
		return true;

	if (*((char*)(gMain + 0xA)) != 0)
		return true;

	int inputManager = *((int*)0x12E7458);
	if (inputManager == 0)
		return true;

	if (*((char*)(inputManager + 0x98)) != 0)
		return true;

	return false;
}

static const char * MenuNames[] =
{
	"Loading Menu",
	"Main Menu",
	"MapMenu",
	"RaceSex Menu",
	"StatsMenu",
	"MagicMenu",

	NULL
};
bool Camera::_isPlayerCamera()
{
	MenuManager * mm = MenuManager::GetSingleton();
	if (!mm)
		return false;

	int i = 0;
	while (MenuNames[i] != NULL)
	{
		StringHolder bs(MenuNames[i]);
		if (mm->IsMenuOpen(bs.Get()))
			return false;

		i++;
	}

	return true;
}

bool Camera::IsThirdPerson(bool recalc)
{
	if(!recalc)
		return (_iFlags & 1) != 0;

	return _isThirdPerson();
}

bool Camera::IsFirstPerson(bool recalc)
{
	if(!recalc)
		return (_iFlags & 8) != 0;

	return _isFirstPerson();
}

bool Camera::IsGamePaused()
{
	return (_iFlags & 2) != 0;
}

bool Camera::IsPlayerCamera()
{
	return (_iFlags & 4) != 0;
}

void Camera::_reload()
{
	GetSingleton()->SetTargetOverwrite(0);
	GetSingleton()->SetControlOverwrite(0, true);
}

bool Camera::IsGrabbing()
{
	int plr = *((int*)0x1B2E8E4);
	int objRef = *((int*)(plr + 0x568)); // grabbing object reference handle.
	return objRef != 0 && objRef != *((int*)0x1310630);
}

void Camera::_UpdateFollow(int * result)
{
	int iFace = 0;
	Config::GetInt(ConfigValueNames::iFaceCrosshair, iFace);

	// Special case, grabbing object must always turn, otherwise we can't move it because it depends on character rotation.
	if(iFace <= 0)
	{
		if(IsGrabbing())
			iFace = CAMERA_LOOK_FORCE_FIXED;
	}

	if (iFace == -1)
		*result = CAMERA_LOOK_FORCE_FREE;
	else if (iFace == 1)
		*result = CAMERA_LOOK_FORCE_FIXED;
	else if (iFace >= 2)
		*result = CAMERA_LOOK_FORCE_FIXED_TURN;
}

void Camera::_UpdateFollow2(int * result)
{
	int iFace = 0;
	Config::GetInt(ConfigValueNames::iFaceCrosshair, iFace);

	if (iFace != -2)
		return;

	// Special case, grabbing object must always turn, otherwise we can't move it because it depends on character rotation.
	if (IsGrabbing())
		return;
	
	int ptr = *((int*)0x1B2EF3C);
	if (ptr != 0)
	{
		if ((*((int*)(ptr + 0x14)) & 0xFF) == 2)
			return;
		else if ((*((int*)(ptr + 0x24)) & 0xFF) != 0)
			return;
		if (((*((int*)(ptr + 0x24)) & 0xFF00) >> 8) != 0)
			return;
	}

	*result = 1;
}

int Camera::_UpdateZoomIn()
{
	bool ov = false;
	Config::GetBool(ConfigValueNames::bZoomIntoIFPV, ov);

	if (ov)
	{
		Config::_setValueInternal(ConfigValueNames::bActivated, ValueInstance(true, true), 2000);
		return 1;
	}

	return 0;
}

int Camera::_UpdateMountZoom()
{
	bool ov = false;
	Config::GetBool(ConfigValueNames::bZoomIntoIFPV, ov);

	return ov ? 1 : 0;
}

int Camera::_UpdateZoomOut()
{
	bool ov = false;
	Config::GetBool(ConfigValueNames::bZoomIntoIFPV, ov);

	if (ov)
	{
		Config::_setValueInternal(ConfigValueNames::bActivated, ValueInstance(false, true), 2000);
		//return 1;
	}

	return 0;
}

int Camera::_UpdateTogglePOV()
{
	bool ov = false;
	Config::GetBool(ConfigValueNames::bToggleIntoIFPV, ov);

	if (ov)
	{
		Config::_toggleActive();
		return 1;
	}

	return 0;
}

int Camera::_UpdateMountPOV()
{
	bool ov = false;
	Config::GetBool(ConfigValueNames::bToggleIntoIFPV, ov);

	return ov ? 1 : 0;
}

void Camera::_UpdateOverShoulder(int camStatePtr)
{
	if (camStatePtr == 0 || !IsActivated(true))
		return;

	int stateId = ((TESCameraState*)camStatePtr)->stateId;
	switch (stateId)
	{
	case PlayerCamera::kCameraState_Bleedout:
	case PlayerCamera::kCameraState_Dragon:
	case PlayerCamera::kCameraState_Furniture:
	case PlayerCamera::kCameraState_Horse:
	case PlayerCamera::kCameraState_ThirdPerson1:
	case PlayerCamera::kCameraState_ThirdPerson2:
	case PlayerCamera::kCameraState_Transition:
		break;

	default:
		return;
	}

	*((float*)(camStatePtr + 0x3C)) = 0.0f;
	*((float*)(camStatePtr + 0x40)) = 0.0f;
	*((float*)(camStatePtr + 0x44)) = 0.0f;
}

int _thirdPUpdateWeapon = 0;
void Camera::OnActiveChanged(bool active)
{
	_thirdPUpdateWeapon = (_nsTargetActor != NULL ? _nsTargetActor->actorState.IsWeaponDrawn() : false) ? 1 : 0;
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam)
	{
		const int updateTP = 0x83C7E0;
		_asm
		{
			pushad
				pushfd
				mov ecx, cam
				push _thirdPUpdateWeapon
				call updateTP
				popfd
				popad
		}
	}

	if (active)
	{
		if (_oldHorseMillis < 0)
			_oldHorseMillis = *((int*)0x12B22E8);
		*((int*)0x12B22E8) = 50;
	}
	else
	{
		if (_oldHorseMillis > 0)
			*((int*)0x12B22E8) = _oldHorseMillis;
	}

	if (active)
		Config::SendModEvent("IFPV_EnteringView", "", 0);
	else
		Config::SendModEvent("IFPV_LeavingView", "", 0);
}

void Camera::SetTargetOverwrite(int formId)
{
	_targetOverwrite = formId;
}

void Camera::_helper_SetAIProcessing(Actor * actor, int enabled)
{
	static int _setAIProcessing = 0x6AA690;
	_asm
	{
		pushad
			pushfd

			mov ecx, actor
			push enabled
			call _setAIProcessing

			popfd
			popad
	}
}

void Camera::_helper_SetPlayerControls(Actor * actor, int enabled)
{
	int func = 0x34;
	if (enabled == 0)
		func = 0x30;
	_asm
	{
		pushad
			pushfd

			mov ecx, actor
			mov ecx, [ecx + 0xD0]
			mov eax, [ecx]
			mov edx, func
			mov eax, [eax + edx]
			call eax

			popfd
			popad
	}
}

void Camera::SetControlOverwrite(int formId, bool update)
{
	if (_controlOverwrite == formId)
		return;

	Actor * targetActor = NULL;
	bool _tried = false;
	while (targetActor == NULL)
	{
		int actualForm = formId;
		if (actualForm == 0)
			actualForm = 0x14;

		TESForm * targetForm = LookupFormByID(actualForm);
		if (targetForm != NULL)
			targetActor = DYNAMIC_CAST(targetForm, TESForm, Actor);

		if (targetActor == NULL)
		{
			formId = 0;
			actualForm = 0x14;
		}

		if (_tried && targetActor == NULL)
			return;
		_tried = true;
	}

	if (_controlOverwrite == formId)
		return;

	Actor * prevActor = NULL;
	{
		int prevFormId = _controlOverwrite;
		if (prevFormId == 0)
			prevFormId = 0x14;

		TESForm * prevForm = LookupFormByID(prevFormId);
		if (prevForm != NULL)
			prevActor = DYNAMIC_CAST(prevForm, TESForm, Actor);
	}

	_controlOverwrite = formId;

	if (!update)
		return;

	// Set third person camera on the target.
	{
		static int _setTarget = 0x83D960;
		_asm
		{
			pushad
				pushfd

				mov ecx, 0x12E7288
				mov ecx, [ecx]
				push targetActor
				call _setTarget

				popfd
				popad
		}
	};

	// Set player controls to affect the target.
	//if (targetActor->formID != 0x14)
	{
		_helper_SetAIProcessing(targetActor, 0);
		_helper_SetPlayerControls(targetActor, 1);
		_helper_SetAIProcessing(targetActor, 1);
	}
	/*else
		_helper_SetAIProcessing(targetActor, 1);*/

	// Remove controls from old target.
	if (prevActor != NULL)
	{
		//if (prevActor->formID != 0x14)
		{
			_helper_SetAIProcessing(prevActor, 0);
			_helper_SetPlayerControls(prevActor, 0);
			_helper_SetAIProcessing(prevActor, 1);
		}
		/*else
			_helper_SetAIProcessing(prevActor, 0);*/
	}
}

void Camera::_RememberTargetPos(Point * pos)
{
	LOCK_MUTEX(mtx);

	if (_lastTargetPos == NULL)
		_lastTargetPos = new Point();
	
	{
		_lastTargetPos->x = pos->x;
		_lastTargetPos->y = pos->y;
		_lastTargetPos->z = pos->z;
	}
}

void Camera::_RememberTargetRef(int refHandle)
{
	LOCK_MUTEX(mtx);

	_lastTargetRef = refHandle;
}

void Camera::_updateOverwriteLook(float * msPoint)
{
	if (msPoint[0] != 0.0f || msPoint[1] != 0.0f)
	{
		MoveExtra(msPoint[0], msPoint[1]);
		SetUserMoved();
	}

	{
		LOCK_MUTEX(mtx);

		msPoint[0] += _nextRotate[0] + _nextRotate[2];
		msPoint[1] += _nextRotate[1] + _nextRotate[3];

		for (int i = 0; i < 4; i++)
			_nextRotate[i] = 0.0f;
	}
}

Point Camera::GetLastTargetPos()
{
	if (_lastTargetPos != NULL)
		return *_lastTargetPos;
	return Point();
}

void Camera::SetUserMoved()
{
	LOCK_MUTEX(mtx);

	_nextRotate[0] = 0.0f;
	_nextRotate[1] = 0.0f;

	int delay = 0;
	if (!Config::GetInt(ConfigValueNames::iFocusDelay, delay))
		delay = 3;
	if (delay < 0)
		delay = 0;

	_userMoved = delay;
	if (_lastTargetPos != NULL)
	{
		delete _lastTargetPos;
		_lastTargetPos = NULL;
	}
}

int Camera::ForceUpdateTargetPos()
{
	_lastTargetRef = 0;

	PlayerCharacter * plr = *g_thePlayer;
	if (!plr || !plr->loadedState)
		return 0;
	
	int targetRef = 0;
	const int calcTarget = 0x73DBA0;

	_asm
	{
		mov ecx, plr
			lea eax, targetRef
			push eax
			call calcTarget
	}

	return targetRef;
}

void Camera::UpdateUserMove()
{
	if (_lastTargetPos != NULL)
		return;
	if (_userMoved != 0)
		return;
	if (!IsActivated(true))
		return;

	bool isFocus = false;
	Config::GetBool(ConfigValueNames::bFocus, isFocus);

	if (!isFocus)
		return;

	ForceUpdateTargetPos();

	// More than 5000 unit away.
	if (_lastTargetPos == NULL)
	{
		_lastTargetPos = new Point();
		*_lastTargetPos = GetCameraLookAt(5000.0f);
	}
	else
	{
		float distance = Point::Distance(GetOverwritePosition(), *_lastTargetPos);
		*_lastTargetPos = GetCameraLookAt(distance);
	}
}

void Camera::UpdateFocus()
{
	if (_lastTargetPos == NULL)
		return;
	if (_userMoved != 0)
		return;
	if (!IsActivated(true))
		return;

	bool isFocus = false;
	Config::GetBool(ConfigValueNames::bFocus, isFocus);

	if (!isFocus)
		return;

	Matrix3 curMat = GetOverwriteRotation();
	Point camPoint = GetOverwritePosition();
	Point curPoint = GetCameraLookAt();
	Point tarPoint = GetLastTargetPos();

	curPoint.x = curPoint.x - camPoint.x;
	curPoint.y = curPoint.y - camPoint.y;
	curPoint.z = curPoint.z - camPoint.z;
	curPoint.Normalize();

	tarPoint.x = tarPoint.x - camPoint.x;
	tarPoint.y = tarPoint.y - camPoint.y;
	tarPoint.z = tarPoint.z - camPoint.z;
	float distance = tarPoint.Length();
	tarPoint.Normalize();

	Matrix3 diffRot = Matrix3::GetDifference(curPoint, tarPoint);

	float euX = Matrix3::EulerAngles(curMat, 2, true);
	float euY = -Matrix3::EulerAngles(curMat, 1, true);

	curMat = Matrix3::Multiply(diffRot, curMat);

	euX -= Matrix3::EulerAngles(curMat, 2, true);
	euY -= -Matrix3::EulerAngles(curMat, 1, true);

	float minRotate = 0.0f;
	if (!Config::GetFloat(ConfigValueNames::fFocusMinAngle, minRotate))
		minRotate = 0.0005f;

	float rotateMult = 0.0f;
	if (!Config::GetFloat(ConfigValueNames::fFocusSpeed, rotateMult))
		rotateMult = 8.0f;

	const float minDist = 50.0f;
	const float maxDist = 500.0f;

	if (distance < minDist)
		distance = minDist;
	if (distance > maxDist)
		distance = maxDist;

	rotateMult *= distance / maxDist;

	if (fabs(euX) > minRotate)
	{
		RotateInfo inf;
		inf.Amount = euX * rotateMult;
		inf.Axis = 0;
		inf.OverwriteUser = 2;

		Rotate(inf);
	}

	if (fabs(euY) > minRotate)
	{
		RotateInfo inf;
		inf.Amount = euY * rotateMult;
		inf.Axis = 1;
		inf.OverwriteUser = 2;

		Rotate(inf);
	}
}

void Camera::Rotate(RotateInfo info)
{
	if (info.Axis == 0)
		_rotateCameraX.push_back(info);
	else if (info.Axis == 1)
		_rotateCameraY.push_back(info);
}

void Camera::_updateRotate(float time)
{
	if (time <= 0.0f)
		return;

	for (int axis = 0; axis < 2; axis++)
	{
		std::vector<RotateInfo> * rl = axis == 0 ? &_rotateCameraX : &_rotateCameraY;
		for (std::vector<RotateInfo>::iterator itr = rl->begin(); itr != rl->end(); )
		{
			float ratio = 1.0f;
			if (itr->Time > 0.0f)
			{
				float left = itr->Time - itr->Elapsed;
				if (left > time)
					left = time;

				ratio = left / itr->Time;
				itr->Elapsed += left;
			}

			float amt = ratio * itr->Amount;
			int ax = axis;
			if (itr->OverwriteUser == 1)
				ax += 2;

			if(ax >= 2 || _userMoved == 0)
				_nextRotate[ax] += amt;

			itr->Done += amt;

			if ((itr->Amount > 0.0f && itr->Done >= itr->Amount) || (itr->Amount <= 0.0f && itr->Done <= itr->Amount) || itr->Elapsed >= itr->Time || (itr->OverwriteUser == 2 && _userMoved != 0))
				itr = rl->erase(itr);
			else
				itr++;
		}
	}
}

void Camera::MoveExtra(float x, float y)
{
	const float moveSens = 0.05f;

	_extraRotMouse[0] += x * moveSens;
	_extraRotMouse[1] += y * moveSens;
}

int Camera::_shouldAllowDuringVATS()
{
	return IsActivated(true) ? 1 : 0;
}

int Camera::GetCameraStateId()
{
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (!cam || !cam->cameraState)
		return -1;

	return cam->cameraState->stateId;
}

void Camera::UpdateHideHead()
{
	bool shouldHide = false;
	Config::GetBool(ConfigValueNames::bHideHead, shouldHide);

	if (shouldHide && _nsTargetActor != NULL)
	{
		Actor * actor = _nsTargetActor;
		NiAVObject * head = ResolveNode("BSFaceGenNiNodeSkinned", actor);
		if (head != NULL)
		{
			_hideHeadObject = (int)head;
			return;
		}
	}

	_hideHeadObject = 0;
}

void Camera::UpdateHideHelmet()
{
	bool shouldHide = false;
	bool didSearch = false;
	Config::GetBool(ConfigValueNames::bHideHelmet, shouldHide);

	NiAVObject * helmet = NULL;
	if (shouldHide && _nsTargetActor != NULL)
	{
		if (!_lastHelmetNodeName.empty())
		{
			helmet = ResolveNode(_lastHelmetNodeName.c_str(), _nsTargetActor);
			if (helmet == NULL)
				_lastHelmetNodeName = std::string();
		}

		if (helmet == NULL)
		{
			NiAVObject * skeleton = ResolveNode("skeleton.nif", _nsTargetActor);
			if (skeleton == NULL)
				skeleton = ResolveNode("skeleton_female.nif", _nsTargetActor);

			if (skeleton != NULL)
			{
				didSearch = true;
				UInt16 count = ((NiNode*)skeleton)->m_children.m_emptyRunStart;
				UInt16 index = 0;
				while (index < count)
				{
					helmet = ((NiNode*)skeleton)->m_children.m_data[index++];
					bool isHelmet = false;
					if (helmet != NULL)
					{
						std::string helmetName;
						if (helmet->m_name && *helmet->m_name)
							helmetName = helmet->m_name;

						int idxForm = -1;
						int pareCount = 0;
						bool hadPare = false;
						int len = (int)helmetName.length();
						for (int i = 0; i < len; i++)
						{
							if (helmetName[i] == '(')
							{
								if (hadPare && pareCount == 0)
								{
									idxForm = i + 1;
									break;
								}
								hadPare = true;
								pareCount++;
							}
							else if (helmetName[i] == ')')
							{
								pareCount--;
							}
						}

						if (idxForm >= 0 && len - idxForm >= 8)
						{
							std::string formStr = helmetName.substr(idxForm, 8);
							{
								int formIdHelm = 0;
								if (StringHelper::ToInt(formStr, formIdHelm, true))
								{
									TESForm * frm = LookupFormByID(formIdHelm);
									if (frm->formType == 0x1A)
									{
										TESObjectARMO * armor = DYNAMIC_CAST(frm, TESForm, TESObjectARMO);
										static UInt32 _helmetMask =
											BGSBipedObjectForm::kPart_Head |
											BGSBipedObjectForm::kPart_Hair/* |
											BGSBipedObjectForm::kPart_LongHair |
											BGSBipedObjectForm::kPart_Circlet |
											BGSBipedObjectForm::kPart_Ears*/;
										if (armor && (armor->bipedObject.GetSlotMask() & _helmetMask) != 0)
											isHelmet = true;
									}
								}
							}
						}
					}

					if (!isHelmet)
						helmet = NULL;
					else if(helmet != NULL)
						break;
				}
			}
		}
	}

	if (didSearch)
		_helmetUpdateTimer = 10;

	if (helmet != NULL)
	{
		if (_lastHelmetNodeName.empty() && helmet->m_name && *helmet->m_name)
			_lastHelmetNodeName = helmet->m_name;
		_hideHeadHelmet = (int)helmet;
		return;
	}

	_hideHeadHelmet = 0;
	_lastHelmetNodeName = std::string();
}

int Camera::FixFurnitureCamera(float * furnitureState, int craft)
{
	if (!IsActivated(true))
		return 0;

	furnitureState[0x14 / 4] = 0.0f; // translateX
	furnitureState[0x18 / 4] = 0.0f; // translateY
	furnitureState[0x1C / 4] = 0.0f; // translateZ
	furnitureState[0x20 / 4] = 0.0f; // zoom
	furnitureState[0x24 / 4] = 0.0f; // pitch
	furnitureState[0x28 / 4] = 0.0f; // yaw
	if (craft != 0)
		((UInt8*)furnitureState)[0x2D] = (UInt8)1;
	else
		((UInt8*)furnitureState)[0x2D] = (UInt8)0;

	return 1;
}

int Camera::FixDismountCamera(float * ptr, float value)
{
	if (!IsActivated(true))
	{
		*ptr = value;
		return 0;
	}

	*ptr = 0.0f;
	return 1;
}

int Camera::ShouldTurnHorseCamera()
{
	if (IsActivated(true))
		return 0;

	return 1;
}

void Camera::UpdateControlHandle()
{
	if (!IsControlOverwritten())
		return;

	int hndl = 0;
	_asm
	{
		pushad
			pushfd

			mov eax, 0x12E7288
			mov eax, [eax]
			mov eax, [eax+0x2C]
			mov hndl, eax

			popfd
			popad
	}

	OBJRefHolder objRef(hndl);
	TESObjectREFR * obj = objRef.GetRef();
	if (obj == NULL || obj->formID == 0x14)
		SetControlOverwrite(0, false);
	else
		SetControlOverwrite(obj->formID, false);
}

void Camera::_reloadGame()
{
	GetSingleton()->UpdateControlHandle();
}

int Camera::_updateCrosshair(float * pos)
{
	if (!IsActivated(true) && !IsControlOverwritten())
		return 0;

	// Actually kind of funny if player can activate with arrow camera - shoot arrow 100 ft away onto
	// wooden door then activate door to enter from 100 ft away :D
	/*if (IsArrow())
		return 0;*/

	float firstPos[3];
	firstPos[0] = 0.0f;
	firstPos[1] = 0.0f;
	firstPos[2] = 0.0f;

	static int _getFirst = 0x83BD50;
	_asm
	{
		pushad
			pushfd

			mov ecx, 0x12E7288
			mov ecx, [ecx]
			lea eax, firstPos
			push eax
			call _getFirst

			popfd
			popad
	}

	pos[0] = data.position.x - firstPos[0];
	pos[1] = data.position.y - firstPos[1];
	pos[2] = data.position.z - firstPos[2];
	return 1;
}

void Camera::_updateGeomSkip(int * ptr, int * ptr2)
{
	*ptr = _hideHeadObject;
	*ptr2 = _hideHeadHelmet;
}

void Camera::SetLastArrow(int arrowPtr)
{
	if (!arrowPtr)
		return;

	bool arrowCamera = false;
	Config::GetBool(ConfigValueNames::bArrowCamera, arrowCamera);

	if (!arrowCamera)
		return;

	SetTargetOverwrite(*((int*)(arrowPtr + 0xC)));
}

int Camera::ShouldIgnoreWorldCollision()
{
	int result = 0;
	Config::GetInt(ConfigValueNames::iIgnoreWorldCollision, result);

	return result;
}

#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG

bool Camera::UpdateNearFarDefault()
{
	if (DefaultNearDistance >= 0.0f)
		return true;

	int _ini = *((int*)0x1B97F14);
	if (_ini == 0)
		return false;

	char * iniSettings = (char*)(_ini + 4);
	if (*iniSettings == (char)0)
		return false;

	DefaultNearDistance = *((float*)0x12C8D54);
	if (DefaultNearDistance < 5.0f)
		DefaultNearDistance = 5.0f;

	return true;
}

void Camera::UpdateFarClip(int sceneGraph, float * farDist)
{
	if (sceneGraph != *((int*)0x1B2E8BC))
		return;

	if (AdjustFarClipMin <= 0.0f || AdjustFarClipStart <= 0.0f)
		return;

	if (!UpdateNearFarDefault())
		return;

	float localResult = 0;
	static int nearFunc = 0x69F270;
	_asm
	{
		pushad
			pushfd

			mov ecx, sceneGraph
			call nearFunc
			fstp localResult

			popfd
			popad
	}

	if (localResult < 1.0f)
		localResult = 1.0f;

	/*float defNear = DefaultNearDistance;
	if (defNear < 0.0f)
		defNear = 15.0f;
	else if (defNear < 5.0f)
		defNear = 5.0f;*/

	float defNear = AdjustFarClipStart;
	if (defNear < 1.0f)
		defNear = 1.0f;

	if (localResult >= defNear)
		return;

	float nearRatio = localResult / defNear;

	if (nearRatio < AdjustFarClipMin)
		nearRatio = AdjustFarClipMin;

	*farDist = *farDist * nearRatio;
}

#endif

void Camera::GetLookData(float& x, float& z)
{
	x = data.cameraOffsetXAbs;
	z = data.cameraOffsetZAbs;
}
