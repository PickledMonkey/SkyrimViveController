#include "HandStateHandler.h"


HandStateHandler::HandStateHandler()
{
}


HandStateHandler::~HandStateHandler()
{
}

bool HandStateHandler::isLeftHand()
{
	return leftHand; 
}

void HandStateHandler::setAsLeftHand()
{ 
	leftHand = true; 
}

void HandStateHandler::setAsRightHand()
{
	leftHand = false;
}

void HandStateHandler::setOtherHandHandler(HandStateHandler* otherHandHandler)
{
	otherHand = otherHandHandler;
}