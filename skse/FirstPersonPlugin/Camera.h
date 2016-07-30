#pragma once

//#define CAMERA_INCLUDE_FARCLIP_CONFIG

#include <mutex>
#include <vector>
#include "skse/NiObjects.h"
#include "skse/NiNodes.h"
#include "skse/GameReferences.h"
#include "skse/GameCamera.h"
#include "GameMath.h"

#define CAMERA_LOOK_DISALLOW 0
#define CAMERA_LOOK_ALLOW 1
#define CAMERA_LOOK_ORIGINAL 2
#define CAMERA_LOOK_FORCE_FREE 3
#define CAMERA_LOOK_FORCE_FIXED 4
#define CAMERA_LOOK_FORCE_FIXED_TURN 5

extern float NewUpDownLimit;
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
extern float DefaultNearDistance;
extern float AdjustFarClipMin;
extern float AdjustFarClipStart;
#endif

struct OverwriteData
{
	OverwriteData()
	{
		position = Point();
		rotation = Matrix3::Identity();
		cameraOffsetXRel = 0.0f;
		cameraOffsetZRel = 0.0f;
		cameraOffsetXRelNormalized = 0.0f;
		cameraOffsetZRelNormalized = 0.0f;
		cameraOffsetXAbs = 0.0f;
		cameraOffsetZAbs = 0.0f;
		targetFormId = 0x14;
		targetPtr = 0;
	}

	Point position;
	Matrix3 rotation;
	float cameraOffsetXRel;
	float cameraOffsetZRel;
	float cameraOffsetXAbs;
	float cameraOffsetZAbs;
	float cameraOffsetXRelNormalized;
	float cameraOffsetZRelNormalized;
	int targetFormId;
	int targetPtr;
};

struct RotateInfo
{
	RotateInfo()
	{
		Amount = 0.0f;
		Done = 0.0f;
		Time = 0.0f;
		Axis = -1;
		Elapsed = 0.0f;
		OverwriteUser = 1;
	}

	float Amount;
	float Done;
	float Time;
	int Axis;
	float Elapsed;
	int OverwriteUser;
};

class Camera
{
	Camera();
	~Camera() { }

public:
	bool IsEnabled();
	bool IsActivated(bool inView);
	bool WasEnabled() { return (wasEnabled & 1) != 0; }
	bool WasActivated() { return (wasEnabled & 2) != 0; }
	Point GetOverwritePosition();
	Matrix3 GetOverwriteRotation();
	int GetLastTargetFormId() { return _nsFormId; }
	float GetTimeSinceLastFrame(bool allowScale = false) { return !allowScale ? timeSinceLastFrame : timeSinceLastFrame2; }
	void _getNSConfig(TESObjectREFR ** obj, Actor ** actor)
	{
		*obj = _nsTargetObj;
		*actor = _nsTargetActor;
	}
	bool IsThirdPerson(bool recalc = false);
	bool IsFirstPerson(bool recalc = false);
	bool IsGamePaused();
	bool IsPlayerCamera();
	void SetTargetOverwrite(int formId);
	bool IsTargetOverwritten() { return _targetOverwrite != 0; }
	int GetTargetOverwritten() { return _targetOverwrite; }
	void SetControlOverwrite(int formId, bool update);
	bool IsControlOverwritten() { return _controlOverwrite != 0; }
	int GetControlOverwritten() { return _controlOverwrite; }
	void Rotate(RotateInfo info);
	int ForceUpdateTargetPos();
	int GetLastTargetRef() { return _lastTargetRef; }
	int GetCameraStateId();
	bool IsGrabbing();
	bool IsArrow() { return _lastWasArrow; }
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
	bool UpdateNearFarDefault();
#endif
	void GetLookData(float& x, float& z);

private:
	void _updateRotate(float time);
	Point GetCameraLookAt(float distance = 200.0f);
	Matrix3 GetCameraDirection();
	Matrix3 GetCameraDirectionRel();
	TESObjectREFR * GetCameraTarget(bool must3dLoaded);
	int _IsHeadTrackOverwritten(Actor * actor);
	void UpdateHeadTrack();
	void SetHeadTrackTarget(Actor * actor, Point * pos);
	int _CanFadeOut();
	void _UpdateRestrict(float * x, float * y);
	void _UpdateRestrict2(Actor * actor, float * x, float * y);
	void _UpdateFrame();
	void _UpdatePosition(Point * pos);
	void _UpdateRotation(Matrix3 * rot);
	void CalculateIsEnabled();
	void CalculateIsActivated();
	void CalculateOverwrite(bool partial = false);
	NiAVObject * ResolveNode(const char* nodeName, TESObjectREFR * obj);
	NiAVObject * GetHead(TESObjectREFR * obj);
	NiAVObject * GetEye(TESObjectREFR * obj);
	NiAVObject * ResolveTable(bool allowEye, TESObjectREFR * obj, bool allowOverwrite, bool mustHead, bool allowCache);
	void UpdateIFlags();
	bool _isThirdPerson();
	bool _isFirstPerson();
	bool _isGamePaused();
	bool _isPlayerCamera();
	void _UpdateFollow(int * result);
	void _UpdateFollow2(int * result);
	int _UpdateZoomIn();
	int _UpdateZoomOut();
	int _UpdateTogglePOV();
	int _UpdateMountPOV();
	int _UpdateMountZoom();
	void _RememberTargetPos(Point * pos);
	void _RememberTargetRef(int refHandle);
	void _RememberMouse(float * msPoint);
	void _UpdateOverShoulder(int camStatePtr);
	void OnActiveChanged(bool active);
	void SetUserMoved();
	void UpdateUserMove();
	void UpdateFocus();
	void _updateOverwriteLook(float * amt);
	void MoveExtra(float x, float y);
	int _shouldAllowDuringVATS();
	Point GetLastTargetPos();
	void UpdateHideHead();
	void UpdateHideHelmet();
	int FixFurnitureCamera(float * furnitureState, int craft);
	int FixDismountCamera(float * ptr, float value);
	int ShouldTurnHorseCamera();
	void UpdateControlHandle();
	int _updateCrosshair(float * pos);
	void _updateGeomSkip(int * ptr, int * ptr2);
	void SetLastArrow(int arrowPtr);
	int ShouldIgnoreWorldCollision();
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
	void UpdateFarClip(int sceneGraph, float * farDist);
#endif

	float timeSinceLastFrame;
	float timeSinceLastFrame2;
	int headTrackOverwrite;
	int findHeadPos;
	int isEnabled;
	int wasEnabled;
	std::recursive_mutex mtx;
	unsigned long long frameCounter;
	OverwriteData data;
	TESObjectREFR * _nsTargetObj;
	Actor * _nsTargetActor;
	int _iFlags;
	int _nsFormId;
	float _nsRotX;
	float _nsRotZ;
	int _wasActive2;
	int _targetOverwrite;
	int _controlOverwrite;
	Point * _lastTargetPos;
	int _lastTargetRef;
	int _userMoved;
	int _hideHeadObject;
	int _hideHeadHelmet;
	std::string _lastHelmetNodeName;
	std::vector<RotateInfo> _rotateCameraX;
	std::vector<RotateInfo> _rotateCameraY;
	float _nextRotate[4];
	float _extraRotMouse[2];
	int _oldHorseMillis;
	int _helmetUpdateTimer;
	bool _lastWasArrow;
	int _failedCameraTarget;

	static void _helper_SetAIProcessing(Actor * actor, int enabled);
	static void _helper_SetPlayerControls(Actor * actor, int enabled);

public:
	static float RestrictRad(float rad);
	static void _reload();
	static void _reloadGame();

	static Camera * GetSingleton()
	{
		static Camera * cam = NULL;
		if (cam == NULL)
			cam = new Camera();

		return cam;
	}
};
