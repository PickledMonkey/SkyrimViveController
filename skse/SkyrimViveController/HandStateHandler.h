#pragma once
#include "VivePositionHandler.h"
class HandStateHandler : public VivePositionHandler
{
public:

	enum HandType{
		OneHandedWeapon = 0,
		TwoHandedWeapon = 1,
		Blocking = 2,
		Unarmed = 3,
		SpellCasting = 4
	};


	HandStateHandler();
	~HandStateHandler();

	virtual void updatePose(AllViveStateData& allStateData) = 0;
	virtual bool isActive() = 0;
	virtual void activateHandler(AllViveStateData& allStateData) = 0;
	virtual void deactivatehandler(AllViveStateData& allStateData) = 0;

	virtual bool isLeftHand();
	virtual void setAsLeftHand();
	virtual void setAsRightHand();
	virtual void setOtherHandHandler(HandStateHandler* otherHandHandler);

	virtual HandType getHandType() = 0;

protected:

	bool active = false;
	bool leftHand = false;
	HandStateHandler* otherHand;

};

