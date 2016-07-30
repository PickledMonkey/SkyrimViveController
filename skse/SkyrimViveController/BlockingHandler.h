#pragma once
#include "HandStateHandler.h"
class BlockingHandler : public HandStateHandler
{
public:
	BlockingHandler();
	~BlockingHandler();

	virtual void updatePose(AllViveStateData& allStateData);


	bool isActive();
	void activateHandler(AllViveStateData& allStateData);

	void deactivatehandler(AllViveStateData& allStateData);

	HandType getHandType() override;
	bool isBlocking();

protected:

private:

	bool active;
	bool blocking = false;

	void handleGripPressed(AllViveStateData& allStateData);
	void handleGripReleased(AllViveStateData& allStateData);
};

