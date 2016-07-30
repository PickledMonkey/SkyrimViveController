#pragma once


namespace VivePlayerState
{
	enum CoordinateIndex
	{
		x = 0,
		y = 1,
		z = 2
	};

	enum WeaponDrawnState{
		UndefinedWeaponDrawnState = -1,
		Drawn = 0,
		Sheathed = 1
	};

	enum WeaponState{
		UndefinedWeaponSlot = -1,
		SpellcastingInstant = 0,
		SpellcastingCharged = 1,
		OneHandedWeapon = 2,
		TwoHandedWeapon = 3,
		Shield = 4,
		Bow = 5,
		Crossbow = 6,
		Unarmed = 7,
		Staff = 8,
		Torch = 9
	};

	enum MovementSpeed{
		UndefinedMovementSpeed = -1,
		Running = 0,
		Walking = 1,
		Sprinting = 2,
	};

	enum MovementState{
		UndefinedMovementState = -1,
		NormalMovement = 0,
		Sneaking = 1,
		Swimming = 2,
		Immovable = 3 //Sitting, Sleeping, Menu, Stuck in Animation, Killmove, etc.
	};

	enum CharacterState{
		UndefinedCharacterState = -1,
		HumanOnFoot = 0,
		VampireLord = 1,
		Werewolf = 2,
		Horseback = 3,
		DragonRider = 4,
		InsideMenu = 5
	};

	enum ButtonIndex{
		MenuButtonIndex = 0,
		GripButtonIndex = 1,
		TriggerButtonIndex = 2,
		TouchpadButtonIndex = 3,
		numButtonIndicies = 4
	};

	enum ButtonStatusIndex{
		ButtonTouched = 0,
		ButtonDown = 1,
		numButtonStatusIndicies
	};

	enum ButtonState{
		UndefinedButton = 0,

		MenuTouched = 1,
		MenuPressed = 2,
		MenuReleased = 3,
		MenuUntouched = 4, 

		GripTouched = 5,
		GripPressed = 6, 
		GripReleased = 7,
		GripUntouched = 8,

		TriggerTouched = 9,
		TriggerPressed = 10,
		TriggerReleased = 11, 
		TriggerUntouched = 12,

		TouchpadTouched = 13,
		TouchpadPressed = 14,
		TouchpadReleased = 15,
		TouchpadUntouched = 16
	};

	enum ControllerHandIndex{
		bothHands = -2,
		noHand = -1,
		leftHand = 0,
		rightHand = 1,
		numHands = 2
	};

	inline uint64_t getButtonStateFlag(const ButtonState button){ if (button > 0) { return (uint64_t)1 << (button - 1); } return 0; }
	inline bool buttonFlagHas(const ButtonState button, const uint64_t& flags){ if (button > 0){ return (flags&((uint64_t)1 << (button - 1))) > 0; } return false; }

}

struct AllViveStateData
{
	static const int POLL_TIME_PERIOD = 10; //ms

	int numDevices = 0;

	vr::TrackedDeviceIndex_t HMDIndex = 0;
	vr::TrackedDeviceIndex_t leftHandIndex;
	vr::TrackedDeviceIndex_t rightHandIndex;

	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
	vr::VRControllerState_t controllerStates[vr::k_unMaxTrackedDeviceCount];

	vr::TrackedDevicePose_t leftHandPose;
	vr::TrackedDevicePose_t rightHandPose;
	vr::TrackedDevicePose_t hmdPose;

	vr::VRControllerState_t leftHandState;
	vr::VRControllerState_t rightHandState;

	float leftTrackpadAxis[2];
	float rightTrackpadAxis[2];

	uint64_t leftHandButtonFlags = 0;
	uint64_t rightHandButtonFlags = 0;

	float leftHandWeaponSpeed = 0.0;
	float rightHandWeaponSpeed = 0.0;
	VivePlayerState::WeaponState leftHandWeaponState;
	VivePlayerState::WeaponState rightHandWeaponState;

	VivePlayerState::MovementState movementState;
	VivePlayerState::MovementSpeed movementSpeed;

	VivePlayerState::WeaponDrawnState weaponDrawnState;

	VivePlayerState::CharacterState characterState;

	bool buttonStatus[VivePlayerState::numHands][VivePlayerState::numButtonIndicies][VivePlayerState::numButtonStatusIndicies];
	
};

