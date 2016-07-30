#include "MenuHandler.h"

using namespace VivePlayerState;

MenuHandler::MenuHandler()
{
}


MenuHandler::~MenuHandler()
{

}


void MenuHandler::handleRightTouchpadMovement(AllViveStateData& allStateData)
{
	relativeMouseMovement[x] = (allStateData.rightTrackpadAxis[x] - lastTouchpadAxis[x]) * 200;
	relativeMouseMovement[y] = (allStateData.rightTrackpadAxis[y] - lastTouchpadAxis[y]) * -200; //y in inverse

	lastTouchpadAxis[x] = allStateData.rightTrackpadAxis[x];
	lastTouchpadAxis[y] = allStateData.rightTrackpadAxis[y];

	if (firstTouchScan){
		firstTouchScan = false;
	}
	else{
		input->moveMouseRelative(relativeMouseMovement[x], relativeMouseMovement[y]);
	}
}



void MenuHandler::handlePressingRightTouchpad(AllViveStateData& allStateData)
{
	input->scrollMouseWheel(allStateData.leftTrackpadAxis[y] * -30);
}

void MenuHandler::handleLeftTouchpadUnpress(AllViveStateData& allStateData)
{
	input->unpressInput(SkyrimInputInjector::Key_W);
	input->unpressInput(SkyrimInputInjector::Key_A);
	input->unpressInput(SkyrimInputInjector::Key_S);
	input->unpressInput(SkyrimInputInjector::Key_D);
}

void MenuHandler::handleLeftTouchpadPress(AllViveStateData& allStateData)
{
	
	if (allStateData.leftTrackpadAxis[x] > 0.5)
	{
		input->pressInput(SkyrimInputInjector::Key_D);
	}
	else if (allStateData.leftTrackpadAxis[x]  < -0.5)
	{
		input->pressInput(SkyrimInputInjector::Key_A);
	}

	if (allStateData.leftTrackpadAxis[y] > 0.5)
	{
		input->pressInput(SkyrimInputInjector::Key_W);
	}
	else if (allStateData.leftTrackpadAxis[y] < -0.5)
	{
		input->pressInput(SkyrimInputInjector::Key_S);
	}
}

void  MenuHandler::handleUnpressedButtons(AllViveStateData& allStateData)
{
	if (buttonFlagHas(TouchpadReleased, allStateData.leftHandButtonFlags)){
		handleLeftTouchpadUnpress(allStateData);
	}

	if (buttonFlagHas(TriggerReleased, allStateData.rightHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_E);
	}

	if (buttonFlagHas(TriggerReleased, allStateData.leftHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_R);
	}

	if (buttonFlagHas(GripReleased, allStateData.rightHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Mouse_Left);
	}

	if (buttonFlagHas(GripReleased, allStateData.leftHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Mouse_Right);
	}

	if (buttonFlagHas(MenuReleased, allStateData.rightHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_Tab);
	}

	if (buttonFlagHas(MenuReleased, allStateData.leftHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_F);
	}
}

void MenuHandler::handlePressedButtons(AllViveStateData& allStateData)
{
	if (buttonFlagHas(TouchpadPressed, allStateData.leftHandButtonFlags)){
		handleLeftTouchpadPress(allStateData);
	}

	if (buttonFlagHas(TriggerPressed, allStateData.rightHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_E);
	}

	if (buttonFlagHas(TriggerPressed, allStateData.leftHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_R);
	}

	if (buttonFlagHas(GripPressed, allStateData.rightHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Mouse_Left);
	}

	if (buttonFlagHas(GripPressed, allStateData.leftHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Mouse_Right);
	}

	if (buttonFlagHas(MenuPressed, allStateData.rightHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_Tab);
	}

	if (buttonFlagHas(MenuPressed, allStateData.leftHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_F);
	}
}


void MenuHandler::updatePose(AllViveStateData& allStateData)
{
	if (allStateData.buttonStatus[rightHand][TouchpadButtonIndex][ButtonDown]){
		handlePressingRightTouchpad(allStateData);
	}

	else if (allStateData.buttonStatus[rightHand][TouchpadButtonIndex][ButtonTouched])
	{
		handleRightTouchpadMovement(allStateData);
	}
	else if (!firstTouchScan)
	{
		firstTouchScan = true;
	}

	handlePressedButtons(allStateData);
	handleUnpressedButtons(allStateData);
}


bool MenuHandler::isActive()
{
	return active;
}

void MenuHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void MenuHandler::deactivatehandler(AllViveStateData& allStateData)
{
	for (int i = 0; i < SkyrimInputInjector::numInputSelections; i++){
		input->unpressInput((SkyrimInputInjector::InputSelections) i);
	}
	active = false;
}