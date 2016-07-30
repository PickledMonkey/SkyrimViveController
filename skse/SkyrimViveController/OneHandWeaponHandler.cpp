#include "OneHandWeaponHandler.h"
#include "BlockingHandler.h"

OneHandWeaponHandler::OneHandWeaponHandler()
{
	memset(averagedStandingHMDPosition, 0, 3 * sizeof(float));
}


OneHandWeaponHandler::~OneHandWeaponHandler()
{
}


bool OneHandWeaponHandler::isActive()
{
	return active;
}

OneHandWeaponHandler::HandType OneHandWeaponHandler::getHandType()
{
	return OneHandedWeapon;
}

void OneHandWeaponHandler::setAsLeftHand()
{
	HandStateHandler::setAsLeftHand();
	handSideFlag = LocationLeftX;
}

void OneHandWeaponHandler::setAsRightHand()
{
	HandStateHandler::setAsLeftHand();
	handSideFlag = LocationRightX;
}

int OneHandWeaponHandler::getWeaponLocation()
{
	int outputFlags = 0;
	vr::TrackedDevicePose_t rhPose = rightHandPoseHistory.getItem(0);
	vr::TrackedDevicePose_t hmdPose = hmdPoseHistory.getItem(0);

	float rhPosition[3];
	float hmdPosition[3];
	float relativePosition[3];

	float hmdRotatedVectors[3][3];
	float relativeProjectionVectors[3][3];

	OpenVRMathHelps::getRotatedVectorX(hmdPose, hmdRotatedVectors[x][x], hmdRotatedVectors[x][y], hmdRotatedVectors[x][z]);
	OpenVRMathHelps::getRotatedVectorY(hmdPose, hmdRotatedVectors[y][x], hmdRotatedVectors[y][y], hmdRotatedVectors[y][z]);
	OpenVRMathHelps::getRotatedVectorZ(hmdPose, hmdRotatedVectors[z][x], hmdRotatedVectors[z][y], hmdRotatedVectors[z][z]);

	OpenVRMathHelps::getTranslation(rhPose, rhPosition[x], rhPosition[y], rhPosition[z]);
	OpenVRMathHelps::getTranslation(hmdPose, hmdPosition[x], hmdPosition[y], hmdPosition[z]);

	for (int i = 0; i < 3; i++)
	{
		relativePosition[i] = rhPosition[i] - hmdPosition[i];
	}

	OpenVRMathHelps::getProjection(relativePosition, hmdRotatedVectors[x], relativeProjectionVectors[x]);
	OpenVRMathHelps::getProjection(relativePosition, hmdRotatedVectors[y], relativeProjectionVectors[y]);
	OpenVRMathHelps::getProjection(relativePosition, hmdRotatedVectors[z], relativeProjectionVectors[z]);

	debugMsg << "WepLoc: ";

	debugMsg << "X_Val:" << relativeProjectionVectors[x][x] << ", ";
	debugMsg << "Y_Val:" << relativePosition[y] << ", ";
	debugMsg << "Z_Val:" << relativeProjectionVectors[z][z] << "\t";
	if (std::abs(relativeProjectionVectors[x][x]) < 0.15)
	{
		outputFlags |= LocationMidX;
		debugMsg << "X:Mid, ";
	} 
	else
	{
		if (relativeProjectionVectors[x][x] * hmdRotatedVectors[x][x] > 0)
		{
			outputFlags |= LocationRightX;
			debugMsg << "X:Rht, ";
		}
		else
		{
			outputFlags |= LocationLeftX;
			debugMsg << "X:Lft, ";
		}
	}

	if (relativePosition[y] > -0.1524)
	{
		outputFlags |= LocationHighY;
		debugMsg << "Y:Hih, ";
	}
	else if (relativePosition[y] > -0.5588)
	{
		outputFlags |= LocationMidY;
		debugMsg << "Y:Mid, ";
	}
	else
	{
		outputFlags |= LocationLowY;
		debugMsg << "Y:Low, ";
	}
	

	if (relativeProjectionVectors[z][z] * hmdRotatedVectors[z][z] > 0)
	{
		if (std::abs(relativeProjectionVectors[z][z]) < 0.2032)
		{
			outputFlags |= LocationMidZ;
			debugMsg << "Z:Mid\t";
		}
		else
		{
			outputFlags |= LocationBackZ;
			debugMsg << "Z:Bck\t";
		}
	}
	else
	{
		if (std::abs(relativeProjectionVectors[z][z]) < 0.2032)
		{
			outputFlags |= LocationMidZ;
			debugMsg << "Z:Mid\t";
		}
		else
		{
			outputFlags |= LocationForwardZ;
			debugMsg << "Z:Fwd\t";
		}
	}

	return outputFlags;
}


int OneHandWeaponHandler::handleInSwingAnimation(AllViveStateData& allStateData)
{
	timeInAnimation += AllViveStateData::POLL_TIME_PERIOD;

	if (timeInAnimation > (allStateData.rightHandWeaponSpeed * 1000))
	{
		inSwingAnimation = false;
		timeInAnimation = 0;
		inPostSwingAnimation = true;
		timePostAnimation = 0;
	}
	return 0;
}

int OneHandWeaponHandler::handlePostSwingAnimation(AllViveStateData& allStateData)
{
	timePostAnimation += AllViveStateData::POLL_TIME_PERIOD;
	if (timePostAnimation >= (allStateData.rightHandWeaponSpeed * 1000) / 2)
	{
		swingOppositeSide = false;
		timePostAnimation = 0;
		inPostSwingAnimation = false;
	}

	return 0;
}

int OneHandWeaponHandler::handleInitiateSwing(int weaponLocation)
{
	if ((weaponLocation & handSideFlag) > 0 && !swingOppositeSide && !inSwingAnimation)
	{
		input->pressInput(SkyrimInputInjector::Mouse_Left);
		swingOppositeSide = true;
		swung = true;
		return 1;
	}
	else if ((weaponLocation & handSideFlag) > 0 && swingOppositeSide && !inSwingAnimation && inPostSwingAnimation)
	{
		input->pressInput(SkyrimInputInjector::Mouse_Left);
		swingOppositeSide = false;
		swung = true;
		return 1;
	}
	return 0;
}

int OneHandWeaponHandler::handleGripPressed(AllViveStateData& allStateData)
{
	gripPressed = true;

	debugMsg.str("");
	debugMsg.clear();
	

	if (otherHand == NULL)
	{
		handleInitiateSwing(getWeaponLocation());
	}


	BlockingHandler* blocking = (BlockingHandler*)otherHand;
	switch (otherHand->getHandType())
	{
	case(Unarmed) :
		//Same as blocking
	case(Blocking) :
		if (blocking->isBlocking()){
			input->pressInput(SkyrimInputInjector::Mouse_Left);
			swung = true;
		}
		else
		{
			handleInitiateSwing(getWeaponLocation());
		}
		break;
	case(SpellCasting) :
		break;
	case(OneHandedWeapon) :
		break;
	case(TwoHandedWeapon) :
		//should never happen
		break;
	}

	return 0;
}

int OneHandWeaponHandler::handleGripReleased(AllViveStateData& allStateData)
{
	if (swung)
	{
		input->unpressInput(SkyrimInputInjector::Mouse_Left);
		swung = false;
		inSwingAnimation = true;
		inPostSwingAnimation = false;
		timeInAnimation = 0;
	}
	gripPressed = false;

	return 0;
}

int OneHandWeaponHandler::handleNormalAttack(AllViveStateData& allStateData)
{
	if (inSwingAnimation)
	{
		handleInSwingAnimation(allStateData);
	}
	else if (inPostSwingAnimation)
	{
		handlePostSwingAnimation(allStateData);
	}

	if (VivePlayerState::buttonFlagHas(VivePlayerState::GripPressed, allStateData.rightHandButtonFlags))
	{
		handleGripPressed(allStateData);
	}

	if (VivePlayerState::buttonFlagHas(VivePlayerState::GripReleased, allStateData.rightHandButtonFlags))
	{
		handleGripReleased(allStateData);
	}

	return 0;
}


void OneHandWeaponHandler::updatePose(AllViveStateData& allStateData)
{
	hmdPoseHistory.pushItem(allStateData.hmdPose);
	rightHandPoseHistory.pushItem(allStateData.rightHandPose);
	rightHandStateHistory.pushItem(allStateData.rightHandState);

	handleNormalAttack(allStateData);
}

void OneHandWeaponHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void OneHandWeaponHandler::deactivatehandler(AllViveStateData& allStateData)
{
	if (swung)
	{
		input->unpressInput(SkyrimInputInjector::Mouse_Left);
	}

	active = false;
}