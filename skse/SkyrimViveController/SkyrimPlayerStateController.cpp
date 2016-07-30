#include "SkyrimPlayerStateController.h"


SkyrimPlayerStateController::SkyrimPlayerStateController()
{
	memset(&currStateData, 0, sizeof(AllViveStateData));
}

SkyrimPlayerStateController::SkyrimPlayerStateController(const SkyrimPlayerStateController& other)
{
	deepCopy(other);
}

SkyrimPlayerStateController& SkyrimPlayerStateController::operator = (const SkyrimPlayerStateController& other){
	deepCopy(other);
	return *this;
}

SkyrimPlayerStateController::~SkyrimPlayerStateController()
{
	vr::VR_Shutdown();
	vrSystem = NULL;
}

bool SkyrimPlayerStateController::deepCopy(const SkyrimPlayerStateController& other)
{
	this->~SkyrimPlayerStateController();

	vrSystem = other.vrSystem;

	currStateData = other.currStateData;
}

int SkyrimPlayerStateController::initializeOpenVR()
{
	if (openvrInitialized)
	{
		return -3;
	}

	vr::TrackedDeviceIndex_t HMDIndex = 0;

	vr::EVRInitError initError;
	vrSystem = vr::VR_Init(&initError, vr::VRApplication_Background);

	if (initError != vr::VRInitError_None)
	{
		_MESSAGE("Error initializing openVR system\n");
		return -1;
	}

	_MESSAGE("Openvr system initialized\n");

	if (!vr::VRCompositor()){
		_MESSAGE("OpenVR Compositor init failed\n");
		vr::VR_Shutdown();
		vrSystem = NULL;
		return -2;
	}
	_MESSAGE("Openvr compositor initialized\n");


	if (vrSystem->IsInputFocusCapturedByAnotherProcess()){
		_MESSAGE("Someone else has openvr input focus. Attempting to grab the focus.");
		vrSystem->CaptureInputFocus();
		if(vrSystem->IsInputFocusCapturedByAnotherProcess()) 
		{
			_MESSAGE("Could not capture input focus");
		}
	}

	currStateData.leftHandIndex = vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	currStateData.rightHandIndex = vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	_MESSAGE("Controller indicies LH:%d, RH:%d", currStateData.leftHandIndex, currStateData.rightHandIndex);

	//Find the number of devices
	for (vr::TrackedDeviceIndex_t index = 0; index < vr::k_unMaxTrackedDeviceCount; index++){
		vr::ETrackedPropertyError propertyError;
		uint32_t deviceProperty = vrSystem->GetInt32TrackedDeviceProperty(index, vr::Prop_ModelNumber_String, &propertyError);

		if (deviceProperty == 0){
			currStateData.numDevices++;
		}
	}
	_MESSAGE("OpenVR initialized. Found %d devices.\n", currStateData.numDevices);
	openvrInitialized = true;
	return 1;
}

bool SkyrimPlayerStateController::threadRunning()
{
	return running;
}

void SkyrimPlayerStateController::startThread()
{
	if (running)
		return;

	close = false;

	input.startThreads();
	threadHandle = CreateThread(NULL, 0, controllerThread, this, 0, &threadId);
	running = true;
}


void SkyrimPlayerStateController::closeThread()
{
	if (!running)
		return;

	close = true;
	CloseHandle(threadHandle);
	input.closeThreads();
	running = false;
}


DWORD WINAPI SkyrimPlayerStateController::controllerThread(LPVOID param)
{
	_MESSAGE("In the controller thread");
	SkyrimPlayerStateController* __this = (SkyrimPlayerStateController*)param;
	_MESSAGE("Found our state controller in the thread");

	if (g_thePlayer == NULL){
		_MESSAGE("The player was NULL");
	}

	PlayerCharacter* playerActor = *g_thePlayer;
	_MESSAGE("Found the player");
	PlayerCamera * playerCamera = PlayerCamera::GetSingleton();
	_MESSAGE("Found player Camera");

	__this->playerState.setInputInjector(&__this->input);

	while (!__this->close)
	{
		//_MESSAGE("Getting Pose");
		__this->getCurrentPose();
		//_MESSAGE("Updating player state");
		__this->updatePlayerStates();
		//_MESSAGE("Updating Pose");
		__this->updatePose();
		//_MESSAGE("Update complete");
		Sleep(AllViveStateData::POLL_TIME_PERIOD);
	}
	return 1;
}

bool SkyrimPlayerStateController::getCurrentHMDRotation(float rotationMatrix[3][3])
{
	if (rotationMatrix == NULL){
		return false;
	}

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			rotationMatrix[i][j] = currStateData.hmdPose.mDeviceToAbsoluteTracking.m[i][j];
		}
	}
	return true;
}

void SkyrimPlayerStateController::printPose(const vr::TrackedDevicePose_t& pose)
{
	_MESSAGE("BEGIN POSE---------------------------------------");
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[0][0], pose.mDeviceToAbsoluteTracking.m[0][1], pose.mDeviceToAbsoluteTracking.m[0][2], pose.mDeviceToAbsoluteTracking.m[0][3]);
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[1][0], pose.mDeviceToAbsoluteTracking.m[1][1], pose.mDeviceToAbsoluteTracking.m[1][2], pose.mDeviceToAbsoluteTracking.m[1][3]);
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[2][0], pose.mDeviceToAbsoluteTracking.m[2][1], pose.mDeviceToAbsoluteTracking.m[2][2], pose.mDeviceToAbsoluteTracking.m[2][3]);
	_MESSAGE("End POSE---------------------------------------\n");
}

VivePlayerState::ButtonState SkyrimPlayerStateController::handleButtonPressEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand)
{
	
	switch (controllerEvent.button)
	{
	case vr::k_EButton_ApplicationMenu:
		currStateData.buttonStatus[hand][VivePlayerState::MenuButtonIndex][VivePlayerState::ButtonDown] = true;
		return VivePlayerState::MenuPressed;
		break;
	case vr::k_EButton_Grip:
		currStateData.buttonStatus[hand][VivePlayerState::GripButtonIndex][VivePlayerState::ButtonDown] = true;
		return VivePlayerState::GripPressed;
		break;
	case vr::k_EButton_SteamVR_Trigger:
		currStateData.buttonStatus[hand][VivePlayerState::TriggerButtonIndex][VivePlayerState::ButtonDown] = true;
		return VivePlayerState::TriggerPressed;
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		currStateData.buttonStatus[hand][VivePlayerState::TouchpadButtonIndex][VivePlayerState::ButtonDown] = true;
		return VivePlayerState::TouchpadPressed;
		break;
	default:
		return VivePlayerState::UndefinedButton;
		break;
	}
	return VivePlayerState::UndefinedButton;
}

VivePlayerState::ButtonState SkyrimPlayerStateController::handleButtonUnpressEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand)
{
	switch (controllerEvent.button)
	{
	case vr::k_EButton_ApplicationMenu:
		currStateData.buttonStatus[hand][VivePlayerState::MenuButtonIndex][VivePlayerState::ButtonDown] = false;
		return VivePlayerState::MenuReleased;
		break;
	case vr::k_EButton_Grip:
		currStateData.buttonStatus[hand][VivePlayerState::GripButtonIndex][VivePlayerState::ButtonDown] = false;
		return VivePlayerState::GripReleased;
		break;
	case vr::k_EButton_SteamVR_Trigger:
		currStateData.buttonStatus[hand][VivePlayerState::TriggerButtonIndex][VivePlayerState::ButtonDown] = false;
		return VivePlayerState::TriggerReleased;
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		currStateData.buttonStatus[hand][VivePlayerState::TouchpadButtonIndex][VivePlayerState::ButtonDown] = false;
		return VivePlayerState::TouchpadReleased;
		break;
	default:
		return VivePlayerState::UndefinedButton;
		break;
	}
	return VivePlayerState::UndefinedButton;
}

VivePlayerState::ButtonState SkyrimPlayerStateController::handleButtonTouchEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand)
{
	switch (controllerEvent.button)
	{
	case vr::k_EButton_ApplicationMenu:
		currStateData.buttonStatus[hand][VivePlayerState::MenuButtonIndex][VivePlayerState::ButtonTouched] = true;
		return VivePlayerState::MenuTouched;
		break;
	case vr::k_EButton_Grip:
		currStateData.buttonStatus[hand][VivePlayerState::GripButtonIndex][VivePlayerState::ButtonTouched] = true;
		return VivePlayerState::GripTouched;
		break;
	case vr::k_EButton_SteamVR_Trigger:
		currStateData.buttonStatus[hand][VivePlayerState::TriggerButtonIndex][VivePlayerState::ButtonTouched] = true;
		return VivePlayerState::TriggerTouched;
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		currStateData.buttonStatus[hand][VivePlayerState::TouchpadButtonIndex][VivePlayerState::ButtonTouched] = true;
		return VivePlayerState::TouchpadTouched;
		break;
	default:
		return VivePlayerState::UndefinedButton;
		break;
	}
	return VivePlayerState::UndefinedButton;
}

VivePlayerState::ButtonState SkyrimPlayerStateController::handleButtonUntouchEvent(const vr::VREvent_Controller_t& controllerEvent, VivePlayerState::ControllerHandIndex hand)
{
	switch (controllerEvent.button)
	{
	case vr::k_EButton_ApplicationMenu:
		currStateData.buttonStatus[hand][VivePlayerState::MenuButtonIndex][VivePlayerState::ButtonTouched] = false;
		return VivePlayerState::MenuUntouched;
		break;
	case vr::k_EButton_Grip:
		currStateData.buttonStatus[hand][VivePlayerState::GripButtonIndex][VivePlayerState::ButtonTouched] = false;
		return VivePlayerState::GripUntouched;
		break;
	case vr::k_EButton_SteamVR_Trigger:
		currStateData.buttonStatus[hand][VivePlayerState::TriggerButtonIndex][VivePlayerState::ButtonTouched] = false;
		return VivePlayerState::TriggerUntouched;
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		currStateData.buttonStatus[hand][VivePlayerState::TouchpadButtonIndex][VivePlayerState::ButtonTouched] = false;
		return VivePlayerState::TouchpadUntouched;
		break;
	default:
		return VivePlayerState::UndefinedButton;
		break;
	}
	return VivePlayerState::UndefinedButton;
}


void SkyrimPlayerStateController::handleVREvent(const vr::VREvent_t & vrevent, uint64_t& currButtonFlags, VivePlayerState::ControllerHandIndex hand)
{
	switch (vrevent.eventType)
	{
	case vr::VREvent_ButtonPress:
		currButtonFlags |= VivePlayerState::getButtonStateFlag(handleButtonPressEvent(vrevent.data.controller, hand));
		break;
	case vr::VREvent_ButtonUnpress:
		currButtonFlags |= VivePlayerState::getButtonStateFlag(handleButtonUnpressEvent(vrevent.data.controller, hand));
		break;
	case vr::VREvent_ButtonTouch:
		currButtonFlags |= VivePlayerState::getButtonStateFlag(handleButtonTouchEvent(vrevent.data.controller, hand));
		break;
	case vr::VREvent_ButtonUntouch:
		currButtonFlags |= VivePlayerState::getButtonStateFlag(handleButtonUntouchEvent(vrevent.data.controller, hand));
		break;
	}
}

void SkyrimPlayerStateController::getTrackpadAxis(const vr::VRControllerState_t& controllerState, float outAxis[2])
{
	//vr::k_eControllerAxis_TrackPad is incorrect. It's actually 0 for the trackpads
	outAxis[VivePlayerState::x] = controllerState.rAxis[0].x;
	outAxis[VivePlayerState::y] = controllerState.rAxis[0].y;
}

void SkyrimPlayerStateController::getCurrentPose()
{
	memset(currStateData.devicePoses, 0, sizeof(vr::TrackedDevicePose_t) * vr::k_unMaxTrackedDeviceCount);
	vr::VRCompositor()->WaitGetPoses(currStateData.devicePoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0 /*As soon as possible*/, currStateData.devicePoses, vr::k_unMaxTrackedDeviceCount);
	currStateData.leftHandPose = currStateData.devicePoses[currStateData.leftHandIndex];
	currStateData.rightHandPose = currStateData.devicePoses[currStateData.rightHandIndex];
	currStateData.hmdPose = currStateData.devicePoses[currStateData.HMDIndex];
	
	currStateData.leftHandButtonFlags = 0;
	currStateData.rightHandButtonFlags = 0;
	vr::VREvent_t event;
	while (vrSystem->PollNextEvent(&event, sizeof(event)))
	{
		if (event.trackedDeviceIndex == currStateData.leftHandIndex)
		{
			handleVREvent(event, currStateData.leftHandButtonFlags, VivePlayerState::leftHand);
		}
		else if (event.trackedDeviceIndex == currStateData.rightHandIndex)
		{
			handleVREvent(event, currStateData.rightHandButtonFlags, VivePlayerState::rightHand);
		}
	}

	memset(&currStateData.leftHandState, 0, sizeof(vr::VRControllerState_t));
	memset(&currStateData.rightHandState, 0, sizeof(vr::VRControllerState_t));

	vrSystem->GetControllerState(currStateData.leftHandIndex, &currStateData.leftHandState);
	vrSystem->GetControllerState(currStateData.rightHandIndex, &currStateData.rightHandState);

	getTrackpadAxis(currStateData.leftHandState, currStateData.leftTrackpadAxis);
	getTrackpadAxis(currStateData.rightHandState, currStateData.rightTrackpadAxis);
}


VivePlayerState::WeaponDrawnState SkyrimPlayerStateController::getWeaponDrawnState(PlayerCharacter* playerActor)
{
	if (playerActor->actorState.IsWeaponDrawn())
	{
		return VivePlayerState::Drawn;
	}
	return VivePlayerState::Sheathed;
}

VivePlayerState::WeaponState SkyrimPlayerStateController::translateSlotType(uint8_t slotType)
{
	//TESObjectWEAP::GameData slot types
	switch (slotType){
	case(TESObjectWEAP::GameData::kType_HandToHandMelee) :
		return VivePlayerState::Unarmed;
		break;
	case(TESObjectWEAP::GameData::kType_OneHandSword) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_OneHandDagger) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_OneHandAxe) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_OneHandMace) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_TwoHandSword) :
		return VivePlayerState::TwoHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_TwoHandAxe) :
		return VivePlayerState::TwoHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_Bow) :
		return VivePlayerState::Bow;
		break;
	case(TESObjectWEAP::GameData::kType_Staff) :
		return VivePlayerState::Staff;
		break;
	case(TESObjectWEAP::GameData::kType_CrossBow) :
		return VivePlayerState::Crossbow;
		break;
	case(TESObjectWEAP::GameData::kType_H2H) :
		return VivePlayerState::Unarmed;
		break;
	case(TESObjectWEAP::GameData::kType_1HS) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_1HD) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_1HA) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_1HM) :
		return VivePlayerState::OneHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_2HS) :
		return VivePlayerState::TwoHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_2HA) :
		return VivePlayerState::TwoHandedWeapon;
		break;
	case(TESObjectWEAP::GameData::kType_Bow2) :
		return VivePlayerState::Bow;
		break;
	case(TESObjectWEAP::GameData::kType_Staff2) :
		return VivePlayerState::Staff;
		break;
	case(TESObjectWEAP::GameData::kType_CBow) :
		return VivePlayerState::Crossbow;
		break;
	default:
		return VivePlayerState::UndefinedWeaponSlot;
		break;
	}
	return VivePlayerState::UndefinedWeaponSlot;
}


VivePlayerState::WeaponState SkyrimPlayerStateController::getWeaponState(TESForm* equippedObject, bool leftHand)
{
	if (equippedObject == NULL){
		return VivePlayerState::UndefinedWeaponSlot;
	}

	if (equippedObject->IsWeapon()){
		TESObjectWEAP* weapon = (TESObjectWEAP*)equippedObject;
		uint8_t equipSlotType = weapon->gameData.type;
		if(leftHand){
			currStateData.leftHandWeaponSpeed = weapon->speed();
		}
		else{
			currStateData.rightHandWeaponSpeed = weapon->speed();
		}

		return translateSlotType(equipSlotType);
	}

	if (equippedObject->formType == kFormType_Spell || equippedObject->formType == kFormType_ScrollItem){
		SpellItem* spellItem = (SpellItem*) equippedObject;
		if (spellItem->data.castTime <= 0){
			return VivePlayerState::SpellcastingInstant;
		}
		else{
			return VivePlayerState::SpellcastingCharged;
		}
	}
	return VivePlayerState::UndefinedWeaponSlot;
}

bool SkyrimPlayerStateController::isWerewolf(PlayerCharacter* playerActor)
{
	TESRace	* race = playerActor->race;
	if (strcmp(race->fullName.name.data, "Vampire Lord") == 0){
		return true;
	}
	return false;
}

bool SkyrimPlayerStateController::isVampireLord(PlayerCharacter* playerActor)
{
	TESRace	* race = playerActor->race;
	if (strcmp(race->fullName.name.data, "Werewolf") == 0){
		return true;
	}
	return false;
}

VivePlayerState::CharacterState SkyrimPlayerStateController::getCharacterState(PlayerCharacter* playerActor){
	if ((playerActor->race->data.raceFlags & TESRace::kRace_Playable) != TESRace::kRace_Playable){
		return VivePlayerState::UndefinedCharacterState;
	}

	VivePlayerState::CharacterState getCharacterState(PlayerCharacter* playerActor);

	PlayerCamera * playerCamera = PlayerCamera::GetSingleton();
	if (playerCamera) {
		for (int i = 0; i < PlayerCamera::kNumCameraStates; i++) {
			if (playerCamera->cameraState == playerCamera->cameraStates[i])
			{
				switch (i){
				case PlayerCamera::kCameraState_Horse:
					return VivePlayerState::Horseback;
					break;
				case PlayerCamera::kCameraState_Dragon:
					return VivePlayerState::DragonRider;
					break;
				case PlayerCamera::kCameraState_TweenMenu:
					return VivePlayerState::InsideMenu;
				case PlayerCamera::kCameraState_FirstPerson:
				case PlayerCamera::kCameraState_ThirdPerson1:
				case PlayerCamera::kCameraState_ThirdPerson2:
					return VivePlayerState::HumanOnFoot;
				}
			}
		}
	}

	return VivePlayerState::UndefinedCharacterState;
}

VivePlayerState::CharacterState SkyrimPlayerStateController::getTransformedState(PlayerCharacter* playerActor, VivePlayerState::WeaponState leftHand, VivePlayerState::WeaponState rightHand){
	if (leftHand == VivePlayerState::SpellcastingCharged && rightHand == VivePlayerState::SpellcastingCharged){
		if (isVampireLord(playerActor)){
			return VivePlayerState::VampireLord;
		}
	}
	
	if (leftHand == VivePlayerState::UndefinedWeaponSlot && rightHand == VivePlayerState::UndefinedWeaponSlot){
		if (isWerewolf(playerActor)){
			return VivePlayerState::Werewolf;
		}
	}

	return VivePlayerState::UndefinedCharacterState;
}

VivePlayerState::MovementSpeed SkyrimPlayerStateController::getMovementSpeed(PlayerCharacter* playerActor){
	if ((playerActor->actorState.flags04 & ActorState::kState_Sprinting) == ActorState::kState_Sprinting){
		return VivePlayerState::Sprinting;
	}
	if ((playerActor->actorState.flags04 & ActorState::kState_Walking) == ActorState::kState_Walking){
		return VivePlayerState::Walking;
	}
	if ((playerActor->actorState.flags04 & ActorState::kState_Running) == ActorState::kState_Running){
		return VivePlayerState::Running;
	}
	return VivePlayerState::UndefinedMovementSpeed;
}

VivePlayerState::MovementState SkyrimPlayerStateController::getMovementState(PlayerCharacter* playerActor)
{
	if ((playerActor->actorState.flags04 & ActorState::kState_Swimming) == ActorState::kState_Swimming){
		return VivePlayerState::Swimming;
	} 
	else if ((playerActor->actorState.flags04 & ActorState::kState_Sneaking) == ActorState::kState_Sneaking){
		return VivePlayerState::Sneaking;
	}

	if (papyrusActor::GetFurnitureReference(playerActor) != NULL){
		return VivePlayerState::Immovable;
	}

	return VivePlayerState::NormalMovement;
}

void SkyrimPlayerStateController::updatePlayerStates()
{
	PlayerCharacter* playerActor = *g_thePlayer;

	currStateData.leftHandWeaponState = getWeaponState(playerActor->GetEquippedObject(true), true);
	currStateData.rightHandWeaponState = getWeaponState(playerActor->GetEquippedObject(false), false);

	currStateData.movementState = getMovementState(playerActor);
	currStateData.movementSpeed = getMovementSpeed(playerActor);

	currStateData.weaponDrawnState = getWeaponDrawnState(playerActor);

	currStateData.characterState = getCharacterState(playerActor);
	if (currStateData.characterState == VivePlayerState::HumanOnFoot){
		VivePlayerState::CharacterState transformedState = getTransformedState(playerActor, currStateData.leftHandWeaponState, currStateData.rightHandWeaponState);
		if (transformedState != VivePlayerState::UndefinedCharacterState){
			currStateData.characterState = transformedState;
		}
	}

	playerState.setPlayerState(currStateData);
}


void SkyrimPlayerStateController::updatePose()
{
	playerState.updatePose(currStateData);
}