#pragma once
#include "VivePositionHandler.h"
class OutsideMenuUIHandler : public VivePositionHandler
{
public:
	OutsideMenuUIHandler();
	~OutsideMenuUIHandler();

	void updatePose(AllViveStateData& allStateData);


	bool isActive();
	void activateHandler(AllViveStateData& allStateData);

	void deactivatehandler(AllViveStateData& allStateData);

private:

	bool active = false;

	void handleMenuButtonPress(AllViveStateData& allStateData);
	void handleMenuButtonUnpress(AllViveStateData& allStateData);

	void handleUnpressedButtons(AllViveStateData& allStateData);
	void handlePressedButtons(AllViveStateData& allStateData);

	int timeHoldingMenuButton = 0;
	int threshholdToEscapeKey = 1000; //ms
	bool escapeKeyTapped = false;
};

