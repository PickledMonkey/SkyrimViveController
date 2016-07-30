#include "MovementHandler.h"

using namespace VivePlayerState;

MovementHandler::MovementHandler()
{
	for (int i = 0; i < 4; i++){
		moving[i] = false;
	}
	for (int i = 0; i < 2; i++){
		currMovement[i] = notMoving;
	}
}


MovementHandler::~MovementHandler()
{

}

void MovementHandler::updateSprinting()
{
	if (isSprinting){
		debugMsg << ", Sprinting";
		input->pressInput(SkyrimInputInjector::Key_Alt);
	}
	else{
		debugMsg << ", Not Sprinting";
		input->unpressInput(SkyrimInputInjector::Key_Alt);
	}
}

void MovementHandler::updateWalking()
{
	if (isWalking){
		debugMsg << ", Walking";
		input->pressInput(SkyrimInputInjector::Key_Shift);
	}
	else{
		debugMsg << ", Running";
		input->unpressInput(SkyrimInputInjector::Key_Shift);
	}
}

void MovementHandler::startMoving(moveDirection dir, CoordinateIndex XYaxis)
{
	if (dir == currMovement[XYaxis]){
		return;
	}

	switch (dir)
	{
	case moveLeft:
		//debugMsg << ", Start MoveLeft";
		stopMoving(moveRight, XYaxis);
		currMovement[x] = moveLeft;
		debugMsg << ", Av";
		input->keyDown(SkyrimInputInjector::A);
		moving[dir] = true;
		break;
	case moveRight:
		//debugMsg << ", Start MoveRight";
		stopMoving(moveLeft, XYaxis);
		currMovement[x] = moveRight;
		debugMsg << ", Dv";
		input->keyDown(SkyrimInputInjector::D);
		moving[dir] = true;
		break;
	case moveForward:
		//debugMsg << ", Start MoveForward";
		stopMoving(moveBackward, XYaxis);
		currMovement[y] = moveForward;
		debugMsg << ", Wv";
		input->keyDown(SkyrimInputInjector::W);
		moving[dir] = true;
		break;
	case moveBackward:
		//debugMsg << ", Start MoveBackward";
		stopMoving(moveForward, XYaxis);
		currMovement[y] = moveBackward;
		debugMsg << ", Sv";
		input->keyDown(SkyrimInputInjector::S);
		moving[dir] = true;
		break;
	case notMoving:
		//debugMsg << ", Start NotMoving XY=" << XYaxis;
		stopMoving(currMovement[XYaxis], XYaxis);
		break;
	default:
		break;
	}
}

void MovementHandler::stopMoving(moveDirection dir, CoordinateIndex XYaxis)
{
	if (currMovement[XYaxis] == notMoving){
		return;
	}

	if (dir < 0 || dir >= numMoveDirections){
		return;
	}

	switch (dir)
	{
	case moveLeft:
		//debugMsg << ", Stop moving Left";
		currMovement[x] = notMoving;
		debugMsg << ", A^";
		input->keyUp(SkyrimInputInjector::A);
		break;
	case moveRight:
		//debugMsg << ", Stop moving Right";
		currMovement[x] = notMoving;
		debugMsg << ", D^";
		input->keyUp(SkyrimInputInjector::D);
		break;
	case moveForward:
		//debugMsg << ", Stop moving Forward";
		currMovement[y] = notMoving;
		debugMsg << ", W^";
		input->keyUp(SkyrimInputInjector::W);
		break;
	case moveBackward:
		//debugMsg << ", Stop moving Backward";
		currMovement[y] = notMoving;
		debugMsg << ", S^";
		input->keyUp(SkyrimInputInjector::S);
		break;
	default:
		return;
	}

	moving[dir] = false;
}




MovementHandler::moveDirection MovementHandler::handleXAxis(float axis)
{
	if (axis > 0.5 )
	{
		return moveRight;
	} 
	else if (axis < -0.5 )
	{
		return moveLeft;
	}
	return notMoving;
}

MovementHandler::moveDirection MovementHandler::handleYAxis(float axis)
{
	if (axis > 0.5)
	{
		return moveForward;
	}
	else if (axis < -0.5 )
	{
		return moveBackward;
	}
	return notMoving;
}

ControllerHandIndex MovementHandler::getPriorityHand(AllViveStateData& allStateData)
{
	uint8_t handFlags = 0;
	if (allStateData.buttonStatus[leftHand][TouchpadButtonIndex][ButtonDown]){
		handFlags |= 1;
	}

	if (allStateData.buttonStatus[rightHand][TouchpadButtonIndex][ButtonDown]){
		handFlags |= 2;
	}

	switch (handFlags){
	case 0: 
		debugMsg << "PriorityHand: NoHand";
		return noHand;;
	case 1:
		debugMsg << "PriorityHand: LeftHand";
		return leftHand;
	case 2: 
		debugMsg << "PriorityHand: RightHand";
		return rightHand;
	case 3:
		debugMsg << "PriorityHand: BothHands";
		return bothHands;
	}
	return noHand;
}

void MovementHandler::selectMovement(ControllerHandIndex priorityHand, ControllerHandIndex secondaryHand, moveDirection handXDir[2], moveDirection handYDir[2], moveDirection selectedDir[2])
{
	if (handXDir[priorityHand] != notMoving){
		selectedDir[x] = handXDir[priorityHand];
	}
	else{
		selectedDir[x] = handXDir[secondaryHand];
	}

	if (handYDir[priorityHand] != notMoving){
		selectedDir[y] = handYDir[priorityHand];
	}
	else{
		selectedDir[y] = handYDir[secondaryHand];
	}
}

int MovementHandler::handleAxises(AllViveStateData& allStateData, ControllerHandIndex priorityHand)
{
	moveDirection handXDir[2];
	moveDirection handYDir[2];

	debugMsg << ", LHAxis X:" << allStateData.leftTrackpadAxis[x] << " Y:" << allStateData.leftTrackpadAxis[y];
	debugMsg << ", RHAxis X:" << allStateData.rightTrackpadAxis[x] << " Y:" << allStateData.rightTrackpadAxis[y] << ", ";

	handXDir[leftHand] = handleXAxis(allStateData.leftTrackpadAxis[x]);
	handYDir[leftHand] = handleYAxis(allStateData.leftTrackpadAxis[y]);

	handXDir[rightHand] = handleXAxis(allStateData.rightTrackpadAxis[x]);
	handYDir[rightHand] = handleYAxis(allStateData.rightTrackpadAxis[y]);
	
	moveDirection selectedMoveDir[2] = { notMoving, notMoving };

	switch (priorityHand)
	{
	case noHand:
		isWalking = true;
		selectMovement(default_priority_hand, default_secondary_hand, handXDir, handYDir, selectedMoveDir);
		break;
	case leftHand:
		isWalking = false;
		selectMovement(leftHand, rightHand, handXDir, handYDir, selectedMoveDir);
		break;
	case rightHand:
		isWalking = false;
		selectMovement(rightHand, leftHand, handXDir, handYDir, selectedMoveDir);
		break;
	case bothHands:
		isSprinting = handYDir[leftHand] == moveForward && handYDir[rightHand] == moveForward && handXDir[leftHand] == notMoving && handXDir[rightHand] == notMoving;
		isWalking = false;
		selectMovement(default_priority_hand, default_secondary_hand, handXDir, handYDir, selectedMoveDir);
		break;
	}
	debugMsg << ", SelectedDir - X:" << selectedMoveDir[x] << " Y:" << selectedMoveDir[y];

	updateWalking();
	updateSprinting();
	startMoving(selectedMoveDir[x], x);
	startMoving(selectedMoveDir[y], y);

	return 0;
}

void MovementHandler::updatePose(AllViveStateData& allStateData)
{
	if (!(allStateData.buttonStatus[leftHand][TouchpadButtonIndex][ButtonTouched] ||
		allStateData.buttonStatus[rightHand][TouchpadButtonIndex][ButtonTouched]))
	{
		isWalking = false;
		isSprinting = false;

		updateWalking();
		updateSprinting();

		startMoving(notMoving, x);
		startMoving(notMoving, y);
		return;
	}

	debugMsg.str("");
	debugMsg.clear();
	ControllerHandIndex priorityHand = getPriorityHand(allStateData);
	handleAxises(allStateData, priorityHand);
	//_MESSAGE(debugMsg.str().c_str());
}

bool MovementHandler::isActive()
{
	return active;
}

void MovementHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void MovementHandler::deactivatehandler(AllViveStateData& allStateData)
{
	isWalking = false;
	isSprinting = false;

	updateWalking();
	updateSprinting();

	startMoving(notMoving, x);
	startMoving(notMoving, y);

	active = false;
}