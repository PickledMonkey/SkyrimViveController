// SkyrimViveController.cpp : Defines the exported functions for the DLL application.
//
#include "SkyrimViveController.h"
#include "VivePapyrusInjector.h"

namespace SkyrimViveController {
	float InitializeSkyrimViveController(StaticFunctionTag *base){

		_MESSAGE("Initializing OpenVR");
		int openVRStatus = controller.initializeOpenVR();
		if (openVRStatus <= 0)
		{
			_MESSAGE("OpenVR failed to initialize error:%d", openVRStatus);
		}
		else {
			_MESSAGE("OpenVR initialized successfully");
		}
		

		_MESSAGE("Starting thread");
		controller.startThread();
		_MESSAGE("Thread Started");
		return 0.0;
	}

	float CloseSkyrimViveController(StaticFunctionTag *base){
		_MESSAGE("Closing thread");
		controller.closeThread();
		_MESSAGE("Thread Closed");
		return 0.0;
	}

	/*long waitForControllerCommand(StaticFunctionTag *base)
	{
		VivePapyrusInjector::VivePapyrusCommand command = VivePapyrusInjector::noCommand;
		DWORD status = WAIT_OBJECT_0;
		bool stop = false;
		do 
		{
			switch (status)
			{
			case WAIT_OBJECT_0:
				if (!VivePapyrusInjector::commands.isEmpty())
				{
					if (VivePapyrusInjector::commands.get(command))
					{
						VivePapyrusInjector::commands.pop();
						return command;
					}
				}
				break;
			case WAIT_ABANDONED:
				return -1;
				break;
			case WAIT_TIMEOUT:
				return -4;
				break;
			case WAIT_FAILED:
				return -2;
				break;
			}
			status = WaitForSingleObject(VivePapyrusInjector::commands.getNewDataEventHandle(), 10000);
		} while (!stop);

		return -3;
	}*/

	long isSkyrimViveControllerRunning(StaticFunctionTag *base)
	{
		if (controller.threadRunning()){
			return 1;
		}
		return 0;
	}

	//void setViveCommandEvent(StaticFunctionTag *base, long commandNum, UInt32  eventId)
	//{
	//	VivePapyrusInjector::setCommandEvent((VivePapyrusInjector::VivePapyrusCommand)commandNum, eventId);
	//}

	bool RegisterFuncs(VMClassRegistry* registry) {

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, float>("InitializeSkyrimViveController", "SkyrimViveController", SkyrimViveController::InitializeSkyrimViveController, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, float>("CloseSkyrimViveController", "SkyrimViveController", SkyrimViveController::CloseSkyrimViveController, registry));

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, long>("isSkyrimViveControllerRunning", "SkyrimViveController", SkyrimViveController::isSkyrimViveControllerRunning, registry));

		/*registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, long>("waitForControllerCommand", "SkyrimViveController", SkyrimViveController::waitForControllerCommand, registry));*/

		//registry->RegisterFunction(
		//	new NativeFunction2 <StaticFunctionTag, void, long, UInt32 >("setViveCommandEvent", "SkyrimViveController", SkyrimViveController::setViveCommandEvent, registry));

		return true;
	}
}


