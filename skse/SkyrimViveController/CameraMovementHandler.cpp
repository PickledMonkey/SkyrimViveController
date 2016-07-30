#include "CameraMovementHandler.h"

using namespace VivePlayerState;

CameraMovementHandler::CameraMovementHandler()
{
}


CameraMovementHandler::~CameraMovementHandler()
{
}

bool CameraMovementHandler::calculateMouseMovement(vr::TrackedDevicePose_t& hmdPose)
{
	float hmdRotatedVectors[3][3];
	float projectedAngularVelocity[3][3];

	OpenVRMathHelps::getRotatedVectorX(hmdPose, hmdRotatedVectors[x][x], hmdRotatedVectors[x][y], hmdRotatedVectors[x][z]);
	OpenVRMathHelps::getRotatedVectorY(hmdPose, hmdRotatedVectors[y][x], hmdRotatedVectors[y][y], hmdRotatedVectors[y][z]);
	OpenVRMathHelps::getRotatedVectorZ(hmdPose, hmdRotatedVectors[z][x], hmdRotatedVectors[z][y], hmdRotatedVectors[z][z]);

	for (int i = 0; i < 3; i++)
	{
		OpenVRMathHelps::getProjection(hmdPose.vAngularVelocity.v, hmdRotatedVectors[i], projectedAngularVelocity[i]);
	}


	relativeMouseMovement[x] = projectedAngularVelocity[y][y] * -15;
	relativeMouseMovement[y] = projectedAngularVelocity[x][x] * -15;
	return true;
}

void CameraMovementHandler::updatePose(AllViveStateData& allStateData)
{
	if (calculateMouseMovement(allStateData.hmdPose)){
		input->moveMouseRelative(relativeMouseMovement[x], relativeMouseMovement[y]);
	}
}

bool CameraMovementHandler::isActive()
{
	return active;
}

void CameraMovementHandler::activateHandler(AllViveStateData& allStateData)
{
	active = true;
}

void CameraMovementHandler::deactivatehandler(AllViveStateData& allStateData)
{
	active = false;
}