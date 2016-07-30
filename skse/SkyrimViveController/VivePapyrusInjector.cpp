#include "VivePapyrusInjector.h"



class ViveCommandArgument : public IFunctionArguments
{
public:
	ViveCommandArgument() {};
	~ViveCommandArgument(){};

	virtual bool Copy(Output * dst)
	{
		dst->Resize(1);
		dst->Get(0)->SetNone();
		return true;
	};
};


std::string VivePapyrusInjector::eventNames[VivePapyrusInjector::numOfCommands] = {
	"OnViveStopAnimation"
};

UInt64 VivePapyrusInjector::events[VivePapyrusInjector::numOfCommands] = {
	-1
};

bool VivePapyrusInjector::eventsReady[VivePapyrusInjector::numOfCommands] = {
	false
};

VivePapyrusInjector::VivePapyrusInjector()
{
}


VivePapyrusInjector::~VivePapyrusInjector()
{
}

bool VivePapyrusInjector::sendCommand(VivePapyrusCommand command)
{

	//SInt32	Create(VMClassRegistry* registry, UInt32 stackId, StaticFunctionTag* thisInput, BSFixedString eventName);
	//bool	Send(VMClassRegistry* registry, UInt32 stackId, StaticFunctionTag* thisInput, SInt32 handle);
	_MESSAGE("Sending command:%d", command);
	if (command > noCommand && command < numOfCommands){
		_MESSAGE("Command in range");
		if (eventsReady[command])
		{
			_MESSAGE("Command in ready, event:%lu", events[command]);

			StringCache::Ref cacheRef(eventNames[command].c_str());
			ViveCommandArgument voidArg;
			(*g_skyrimVM)->m_classRegistry->QueueEvent(events[command], &cacheRef, &voidArg);

			eventsReady[command] = false;
			_MESSAGE("Command sent");
			return true;
		}
	}
	return false;
}

bool VivePapyrusInjector::setCommandEvent(VivePapyrusCommand command, UInt32 eventNum)
{
	_MESSAGE("Setting command:%d", command);
	if (command > noCommand && command < numOfCommands){
		if (!eventsReady[command])
		{
			events[command] = eventNum;
			eventsReady[command] = true;
			_MESSAGE("Command set: eventNum=%lu", eventNum);
			return true;
		}
	}
	return false;
}

