#pragma once
#include "SkyrimPlayerStateController.h"

#include "skse/PapyrusNativeFunctions.h"

namespace SkyrimViveController
{
	static SkyrimPlayerStateController controller;

	//long waitForControllerCommand(StaticFunctionTag *base);

	float InitializeSkyrimViveController(StaticFunctionTag *base);
	float CloseSkyrimViveController(StaticFunctionTag *base);

	long isSkyrimViveControllerRunning(StaticFunctionTag *base);

	//void setViveCommandEvent(StaticFunctionTag *base, long commandNum, UInt32  eventId);

	bool RegisterFuncs(VMClassRegistry* registry);
}
