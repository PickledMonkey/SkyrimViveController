#pragma once

#include "openvr.h"

#include "skse/GameReferences.h"

#include "VivePlayerState.h"
#include "OneHandWeaponHandler.h"
#include "BlockingHandler.h"
#include "MovementHandler.h"
#include "CameraMovementHandler.h"
#include "MenuHandler.h"
#include "OutsideMenuUIHandler.h"

#include "SkyrimInputInjector.h"

class SkyrimPlayerState
{
public:

	struct FullPhysicalPlayerPose
	{

		float coord[3];
		float rotation[2];
	};


	SkyrimPlayerState();
	~SkyrimPlayerState();

	void setPlayerState(AllViveStateData& allStateData);

	bool updatePose(AllViveStateData& allStateData);

	void setInputInjector(SkyrimInputInjector* input);

private:

	void updateHandStates(AllViveStateData& allStateData);

	bool activateHandler(VivePositionHandler* newHandler, VivePositionHandler*& currActiveHandler, AllViveStateData& currStateData);
	bool deactivateHandler(VivePositionHandler*& handler, AllViveStateData& currStateData);

	static const unsigned int NUM_HANDLERS = 7;

	SkyrimInputInjector* input = NULL;

	OneHandWeaponHandler oneHandWeaponHandlers[VivePlayerState::numHands];
	BlockingHandler blockingHandler;
	MovementHandler movementHandler;
	CameraMovementHandler cameraMovementHandler;
	MenuHandler menuHandler;
	OutsideMenuUIHandler outsideMenuUIHandler;


	VivePositionHandler* handlers[NUM_HANDLERS];

	HandStateHandler* activeRightHandHandler = NULL;
	HandStateHandler* activeLeftHandHandler = NULL;
	VivePositionHandler* activeBodyHandler = NULL;
	VivePositionHandler* activeCameraHandler = NULL;
	VivePositionHandler* activeUIHandler = NULL;
};

