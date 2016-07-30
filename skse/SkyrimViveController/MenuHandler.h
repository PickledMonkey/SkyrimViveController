#pragma once
#include "VivePositionHandler.h"
#include <sstream>

class MenuHandler : public VivePositionHandler
{
public:
	MenuHandler();
	~MenuHandler();


	void updatePose(AllViveStateData& allStateData);


	bool isActive();
	void activateHandler(AllViveStateData& allStateData);

	void deactivatehandler(AllViveStateData& allStateData);

private:
	

	std::ostringstream debugMsg;
	bool active = false;

	bool firstTouchScan = true;
	float lastTouchpadAxis[2];
	LONG relativeMouseMovement[2];

	void handlePressingRightTouchpad(AllViveStateData& allStateData);
	void handleLeftTouchpadUnpress(AllViveStateData& allStateData);
	void handleLeftTouchpadPress(AllViveStateData& allStateData);
	void handleRightTouchpadMovement(AllViveStateData& allStateData);
	void handleUnpressedButtons(AllViveStateData& allStateData);
	void handlePressedButtons(AllViveStateData& allStateData);

};

