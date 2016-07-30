#pragma once

#include "ConcurrentQueue.h"

#include "skse/PapyrusNativeFunctions.h"
#include "skse/PapyrusModEvent.h"

class VivePapyrusInjector
{
public:
	VivePapyrusInjector();
	~VivePapyrusInjector();

	enum VivePapyrusCommand
	{
		noCommand = -1,
		stopAnimation = 0
	};
	static const int numOfCommands = 1;

	static bool eventsReady[numOfCommands];
	static std::string eventNames[numOfCommands];
	static UInt64 events[numOfCommands];

	static bool sendCommand(VivePapyrusCommand command);
	static bool setCommandEvent(VivePapyrusCommand command, UInt32 eventNum);
};

