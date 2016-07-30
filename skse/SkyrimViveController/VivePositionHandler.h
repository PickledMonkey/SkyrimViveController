#pragma once

//openvr
#include "openvr.h"

#include "VivePlayerState.h"
#include "OpenVRMathHelps.h"
#include "SkyrimInputInjector.h"

class VivePositionHandler
{
public:
	VivePositionHandler();
	~VivePositionHandler();


	virtual void updatePose(AllViveStateData& allStateData) = 0;


	virtual bool isActive() = 0;
	virtual void activateHandler(AllViveStateData& allStateData) = 0;

	virtual void deactivatehandler(AllViveStateData& allStateData) = 0;

	void setInputInjector(SkyrimInputInjector* input) { this->input = input; }

protected:
	SkyrimInputInjector* input = NULL;

private:

};

