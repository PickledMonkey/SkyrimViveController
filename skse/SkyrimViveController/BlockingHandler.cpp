#include "BlockingHandler.h"


BlockingHandler::BlockingHandler()
{

}

BlockingHandler::~BlockingHandler()
{

}

BlockingHandler::HandType BlockingHandler::getHandType()
{
	return Blocking;
}

bool BlockingHandler::isBlocking()
{
	return blocking;
}

void BlockingHandler::handleGripPressed(AllViveStateData& allStateData)
{
	input->pressInput(SkyrimInputInjector::Mouse_Right);
	blocking = true;
}

void BlockingHandler::handleGripReleased(AllViveStateData& allStateData)
{
	input->unpressInput(SkyrimInputInjector::Mouse_Right);
	blocking = false;
}

void BlockingHandler::updatePose(AllViveStateData& allStateData)
{
	if (VivePlayerState::buttonFlagHas(VivePlayerState::GripPressed, allStateData.leftHandButtonFlags))
	{
		handleGripPressed(allStateData);
	}

	if (VivePlayerState::buttonFlagHas(VivePlayerState::GripReleased, allStateData.leftHandButtonFlags))
	{
		handleGripReleased(allStateData);
	}
}

bool BlockingHandler::isActive()
{
	return active;
}

void BlockingHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void BlockingHandler::deactivatehandler(AllViveStateData& allStateData)
{
	if (blocking)
	{
		input->unpressInput(SkyrimInputInjector::Mouse_Right);
	}
	active = false;
}



