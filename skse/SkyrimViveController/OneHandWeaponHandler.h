#pragma once

#include <sstream>

#include "CircularBuffer.h"
#include "HandStateHandler.h"
#include "VivePapyrusInjector.h"


class OneHandWeaponHandler : public HandStateHandler
{
public:
	OneHandWeaponHandler();
	~OneHandWeaponHandler();

	void updatePose(AllViveStateData& allStateData) override;

	bool isActive() override;
	void activateHandler(AllViveStateData& allStateData) override;

	void deactivatehandler(AllViveStateData& allStateData) override;

	HandType getHandType() override;
	void setAsLeftHand() override;
	void setAsRightHand() override;

protected:


private:
	enum
	{
		x = 0,
		y = 1,
		z = 2
	};

	enum
	{
		vr_y = 0,
		vr_x = 1,
		vr_z = 2
	};

	enum WeaponLocationFlags
	{
		LocationHighY = 1,
		LocationMidY = 2,
		LocationLowY = 4,
		
		LocationRightX = 8,
		LocationMidX = 16,
		LocationLeftX = 32,

		LocationForwardZ = 64,
		LocationMidZ = 128,
		LocationBackZ = 256
	};

	int handleInitiateSwing(int weaponLocation);
	int handleInSwingAnimation(AllViveStateData& allStateData);
	int handlePostSwingAnimation(AllViveStateData& allStateData);
	int handleGripPressed(AllViveStateData& allStateData);
	int handleGripReleased(AllViveStateData& allStateData);
	int handleNormalAttack(AllViveStateData& allStateData);

	int updateAverageStandingPosition(float hmdPositionCurr[3]);

	int getWeaponLocation();

	bool areSwinging(int weaponRotation, int swingDirection, int weaponLocation, double swingVelocity, double swingAcceleration);

	std::ostringstream debugMsg;

	bool gripPressed = false;
	WeaponLocationFlags handSideFlag;
	bool swingOppositeSide = false;
	bool inSwingAnimation = false;
	bool inPostSwingAnimation = false;
	int timeInAnimation = 0;
	int timePostAnimation = 0;
	bool swung = false;
	bool active = false;

	float averagedStandingHMDPosition[3];
	uint64_t numMeasurementsAtCurrStandingPos = 0;
	CircularBuffer<vr::TrackedDevicePose_t> hmdPoseHistory;
	CircularBuffer<vr::TrackedDevicePose_t> rightHandPoseHistory;
	CircularBuffer<vr::VRControllerState_t> rightHandStateHistory;
};

