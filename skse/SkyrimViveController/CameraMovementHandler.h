#pragma once

#include "VivePositionHandler.h"
#include <sstream>

class CameraMovementHandler : public VivePositionHandler
{
public:
	CameraMovementHandler();
	~CameraMovementHandler();

	void updatePose(AllViveStateData& allStateData);


	bool isActive();
	void activateHandler(AllViveStateData& allStateData);

	void deactivatehandler(AllViveStateData& allStateData);

private:

	bool calculateMouseMovement(vr::TrackedDevicePose_t& hmdPose);


	std::ostringstream debugMsg;
	bool active = false;

	LONG relativeMouseMovement[2];

};

