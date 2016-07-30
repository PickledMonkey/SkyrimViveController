#include "SkyrimPlayerState.h"


SkyrimPlayerState::SkyrimPlayerState()
{
	oneHandWeaponHandlers[VivePlayerState::leftHand].setAsLeftHand();
	blockingHandler.setAsLeftHand();

	handlers[0] = &oneHandWeaponHandlers[0];
	handlers[1] = &oneHandWeaponHandlers[1];
	handlers[2] = &blockingHandler;
	handlers[3] = &movementHandler;
	handlers[4] = &cameraMovementHandler;
	handlers[5] = &menuHandler;
	handlers[6] = &outsideMenuUIHandler;
}


SkyrimPlayerState::~SkyrimPlayerState()
{
}

void SkyrimPlayerState::setInputInjector(SkyrimInputInjector* input)
{
	this->input = input;
	for (unsigned int i = 0; i < NUM_HANDLERS; i++)
	{
		handlers[i]->setInputInjector(input);
	}
}

bool SkyrimPlayerState::deactivateHandler(VivePositionHandler*& handler, AllViveStateData& currStateData)
{
	if (handler != NULL)
	{
		if (handler->isActive()){
			handler->deactivatehandler(currStateData);
			handler = NULL;
			return true;
		}
	}
	return false;
}

bool SkyrimPlayerState::activateHandler(VivePositionHandler* newHandler, VivePositionHandler*& currActiveHandler, AllViveStateData& currStateData)
{
	if (newHandler == currActiveHandler){
		return false;
	}

	if (!deactivateHandler(currActiveHandler, currStateData)){
		return false;
	}

	if (newHandler != NULL){
		if (!newHandler->isActive()){
			newHandler->activateHandler(currStateData);
			currActiveHandler = newHandler;
			return true;
		}
	}
	
	return false;
}

void SkyrimPlayerState::updateHandStates(AllViveStateData& allStateData)
{
	bool handChange = false;
	switch (allStateData.rightHandWeaponState){
	case VivePlayerState::OneHandedWeapon:
		if (activateHandler(&oneHandWeaponHandlers[VivePlayerState::rightHand], (VivePositionHandler*&)activeRightHandHandler, allStateData))
		{
			handChange = true;
		}
		break;
	default:
		//do nothing for now
		break;
	}

	switch (allStateData.leftHandWeaponState){
	case VivePlayerState::Shield:
		//Same as undefined
	case VivePlayerState::UndefinedWeaponSlot:
		if (activateHandler(&blockingHandler, (VivePositionHandler*&)activeLeftHandHandler, allStateData))
		{
			handChange = true;
		}
		break;
	default:
		break;
	}

	if (handChange){
		if (activeRightHandHandler != NULL){
			activeRightHandHandler->setAsRightHand();
			activeRightHandHandler->setOtherHandHandler(activeLeftHandHandler);
		}
		if (activeLeftHandHandler != NULL){
			activeLeftHandHandler->setAsLeftHand();
			activeLeftHandHandler->setOtherHandHandler(activeLeftHandHandler);
		}
	}
}

void SkyrimPlayerState::setPlayerState(AllViveStateData& allStateData)
{
	updateHandStates(allStateData);

	switch (allStateData.characterState){
	case VivePlayerState::HumanOnFoot:
		activateHandler(&movementHandler, activeBodyHandler, allStateData);
		//activeBodyHandler = &movementHandler;

		activateHandler(&cameraMovementHandler, activeCameraHandler, allStateData);
		//activeCameraHandler = &cameraMovementHandler;

		activateHandler(&outsideMenuUIHandler, activeUIHandler, allStateData);
		//activeUIHandler = &outsideMenuUIHandler;

		break;
	case VivePlayerState::InsideMenu:
		deactivateHandler(activeBodyHandler, allStateData);
		//activeBodyHandler = NULL;

		deactivateHandler(activeCameraHandler, allStateData);
		//activeCameraHandler = NULL;

		deactivateHandler((VivePositionHandler*&)activeRightHandHandler, allStateData);
		deactivateHandler((VivePositionHandler*&)activeLeftHandHandler, allStateData);

		activateHandler(&menuHandler, activeUIHandler, allStateData);
		//activeUIHandler = &menuHandler;

		break;
	default:
		//do nothing for now
		break;
	}
}

bool SkyrimPlayerState::updatePose(AllViveStateData& allStateData)
{
	if (activeBodyHandler != NULL){
		if (activeBodyHandler->isActive()){
			activeBodyHandler->updatePose(allStateData);
		}
	}

	if (activeRightHandHandler != NULL)
	{
		if (activeRightHandHandler->isActive()){
			activeRightHandHandler->updatePose(allStateData);
		}
	}

	if (activeCameraHandler != NULL)
	{
		if (activeCameraHandler->isActive()){
			activeCameraHandler->updatePose(allStateData);
		}
	}

	if (activeUIHandler != NULL)
	{
		if (activeUIHandler->isActive())
		{
			activeUIHandler->updatePose(allStateData);
		}
	}

	return true;
}