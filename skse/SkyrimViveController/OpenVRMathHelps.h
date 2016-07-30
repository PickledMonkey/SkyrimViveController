#pragma once

#include "openvr.h"

#include <math.h>

class OpenVRMathHelps
{
public:
	static void getPiecewiseRotation(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ);
	static void getTranslation(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ);

	static void getRotatedVectorX(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ);
	static void getRotatedVectorY(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ);
	static void getRotatedVectorZ(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ);

	static void getProjection(float u[3], float v[3], float u_onto_v[3]);

	static float getVelocityMagnitude(const vr::TrackedDevicePose_t& pose);
	static void getNormalizedVector(float inVector[3], float outVector[3]);
	static float getVectorMagnitude(float inVector[3]);

	static void printVector(float v[3]);
	static void printPose(const vr::TrackedDevicePose_t& pose);

private:

};