#pragma once

//openvr
#include "openvr.h"

//skse game api
#include "skse/GameAPI.h"
#include "skse/GameReferences.h"
#include "skse/GameObjects.h"
#include "skse/GameCamera.h"

//skse papyrus functions
#include "skse/PapyrusActor.h"

//standard library includes
#include <stddef.h>
#include <string>

//src
#include "SkyrimPlayerState.h"
#include "SkyrimInputInjector.h"

class SkyrimPlayerStateController
{
public:
	SkyrimPlayerStateController();
	SkyrimPlayerStateController(const SkyrimPlayerStateController& other);
	SkyrimPlayerStateController& operator=(const SkyrimPlayerStateController& other);
	~SkyrimPlayerStateController();

	int initializeOpenVR();

	bool getCurrentHMDRotation(float rotationMatrix[3][3]);

	void getCurrentPose();
	void updatePlayerStates();
	void updatePose();

	VivePlayerState::WeaponDrawnState getWeaponDrawnState(PlayerCharacter* playerActor);
	VivePlayerState::WeaponState getWeaponState(TESForm* equippedObject, bool leftHand);
	VivePlayerState::CharacterState getCharacterState(PlayerCharacter* playerActor);
	VivePlayerState::MovementSpeed getMovementSpeed(PlayerCharacter* playerActor);
	VivePlayerState::MovementState getMovementState(PlayerCharacter* playerActor);

	bool threadRunning();
	void startThread();
	void closeThread();
	static DWORD WINAPI controllerThread(LPVOID param);

private:
	bool deepCopy(const SkyrimPlayerStateController& other);
	
	static void printPose(const vr::TrackedDevicePose_t& pose);

	VivePlayerState::WeaponState translateSlotType(uint8_t slotType);

	bool isWerewolf(PlayerCharacter* playerActor);
	bool isVampireLord(PlayerCharacter* playerActor);

	VivePlayerState::CharacterState getMountedState(PlayerCharacter* playerActor);
	VivePlayerState::CharacterState getTransformedState(PlayerCharacter* playerActor, VivePlayerState::WeaponState leftHand, VivePlayerState::WeaponState rightHand);

	void getTrackpadAxis(const vr::VRControllerState_t& controllerState, float outAxis[2]);
	void handleVREvent(const vr::VREvent_t & vrevent, uint64_t& currButtonFlags, VivePlayerState::ControllerHandIndex hand);
	VivePlayerState::ButtonState handleButtonPressEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand);
	VivePlayerState::ButtonState handleButtonUnpressEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand);
	VivePlayerState::ButtonState handleButtonTouchEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand);
	VivePlayerState::ButtonState handleButtonUntouchEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand);

	bool close = false;
	bool running = false;
	bool openvrInitialized = false;

	vr::IVRSystem* vrSystem;

	AllViveStateData currStateData;

	SkyrimPlayerState playerState;

	HANDLE threadHandle;
	DWORD threadId;

	SkyrimInputInjector input;
};

