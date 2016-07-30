#pragma once
#include "VivePositionHandler.h"
#include <sstream>


class MovementHandler : public VivePositionHandler
{
public:
	MovementHandler();
	~MovementHandler();

	void updatePose(AllViveStateData& allStateData);


	bool isActive();
	void activateHandler(AllViveStateData& allStateData);

	void deactivatehandler(AllViveStateData& allStateData);

private:
	enum moveDirection
	{
		notMoving = -1,
		moveLeft = 0,
		moveRight = 1,
		moveForward = 2,
		moveBackward = 3,
		numMoveDirections = 4
	};

	void updateSprinting();
	void updateWalking();
	void startMoving(moveDirection dir, VivePlayerState::CoordinateIndex XYaxis);
	void stopMoving(moveDirection dir, VivePlayerState::CoordinateIndex XYaxis);

	VivePlayerState::ControllerHandIndex getPriorityHand(AllViveStateData& allStateData);
	int handleAxises(AllViveStateData& allStateData, VivePlayerState::ControllerHandIndex priorityHand);
	void selectMovement(VivePlayerState::ControllerHandIndex priorityHand, VivePlayerState::ControllerHandIndex secondaryHand, moveDirection handXDir[2], moveDirection handYDir[2], moveDirection selectedDir[2]);
	moveDirection handleXAxis(float axis);
	moveDirection handleYAxis(float axis);

	bool active = false;

	float threshold = 0.5;
	VivePlayerState::ControllerHandIndex default_priority_hand = VivePlayerState::leftHand;
	VivePlayerState::ControllerHandIndex default_secondary_hand = VivePlayerState::rightHand;

	bool moving[4];
	moveDirection currMovement[2];

	bool isWalking = false;
	bool isSprinting = false;

	std::ostringstream debugMsg;
};

