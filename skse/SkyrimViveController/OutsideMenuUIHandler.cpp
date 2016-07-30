#include "OutsideMenuUIHandler.h"

using namespace VivePlayerState;


OutsideMenuUIHandler::OutsideMenuUIHandler()
{
}


OutsideMenuUIHandler::~OutsideMenuUIHandler()
{
}

void OutsideMenuUIHandler::handleMenuButtonPress(AllViveStateData& allStateData)
{
	if (timeHoldingMenuButton > threshholdToEscapeKey && !escapeKeyTapped){
		input->tapKey(SkyrimInputInjector::Esc);
		escapeKeyTapped = true;
	}
	else{
		timeHoldingMenuButton += AllViveStateData::POLL_TIME_PERIOD;
	}
}

void OutsideMenuUIHandler::handleMenuButtonUnpress(AllViveStateData& allStateData)
{
	if (escapeKeyTapped){
		input->tapKey(SkyrimInputInjector::Tab);
	}
	escapeKeyTapped = false;
	timeHoldingMenuButton = 0;
}

void  OutsideMenuUIHandler::handleUnpressedButtons(AllViveStateData& allStateData)
{
	if (buttonFlagHas(TriggerReleased, allStateData.rightHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_E);
	}

	if (buttonFlagHas(TriggerReleased, allStateData.leftHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_Z);
	}

	if (buttonFlagHas(MenuReleased, allStateData.rightHandButtonFlags)){
		handleMenuButtonUnpress(allStateData);
	}

	if (buttonFlagHas(MenuReleased, allStateData.leftHandButtonFlags)){
		input->unpressInput(SkyrimInputInjector::Key_Q);
	}
}

void OutsideMenuUIHandler::handlePressedButtons(AllViveStateData& allStateData)
{
	if (buttonFlagHas(TriggerPressed, allStateData.rightHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_E);
	}

	if (buttonFlagHas(TriggerPressed, allStateData.leftHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_Z);
	}

	if (buttonFlagHas(MenuPressed, allStateData.rightHandButtonFlags)){
		handleMenuButtonPress(allStateData);
	}

	if (buttonFlagHas(MenuPressed, allStateData.leftHandButtonFlags)){
		input->pressInput(SkyrimInputInjector::Key_Q);
	}
}


void OutsideMenuUIHandler::updatePose(AllViveStateData& allStateData)
{
	handlePressedButtons(allStateData);
	handleUnpressedButtons(allStateData);
}


bool OutsideMenuUIHandler::isActive()
{
	return active;
}

void OutsideMenuUIHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void OutsideMenuUIHandler::deactivatehandler(AllViveStateData& allStateData)
{
	active = false;
	input->unpressInput(SkyrimInputInjector::Key_E);
	input->unpressInput(SkyrimInputInjector::Key_Z);
	input->unpressInput(SkyrimInputInjector::Key_Q);
	input->unpressInput(SkyrimInputInjector::Key_Tab);
	input->unpressInput(SkyrimInputInjector::Key_Esc);
}

