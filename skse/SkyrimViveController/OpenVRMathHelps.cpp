#include "OpenVRMathHelps.h"

/*
R(x)						R(y)						R(z)
(1 | 0		| 0			*	(cos(b) | 0 | sin(b)	*	(cos(c) | -sin(c)	| 0
 0 | cos(a) | -sin(a)		 0		| 1 | 0				 sin(c) | cos(c)	| 0
 0 | sin(a) | cos(a))		 -sin(b)| 0 | cos(b))		 0		| 0			| 1 )
 

 =
M
(cos(b) cos(c)						| -cos(b) sin(c)						| sin(b)
 cos(c) sin(a) sin(b)+cos(a) sin(c)	| cos(a) cos(c)-sin(a) sin(b) sin(c)	| -cos(b) sin(a)
 sin(a) sin(c)-cos(a) cos(c) sin(b)	| cos(c) sin(a)+cos(a) sin(b) sin(c)	| cos(a) cos(b))

 y = asin(M[0][2])
 x = acos(M[2][2]/cos(y))
 z = acos(M[0][0]/cos(y))

*/
void OpenVRMathHelps::getPiecewiseRotation(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ)
{
	outY= asin(pose.mDeviceToAbsoluteTracking.m[0][2]);

	float cosX = cos(outX);

	outX = acos((pose.mDeviceToAbsoluteTracking.m[2][2] / cosX));
	outZ = acos((pose.mDeviceToAbsoluteTracking.m[0][0] / cosX));
}


/*
M
(1	|	0	|	0	|	x
 0	|	1	|	0	|	y
 0	|	0	|	1	|	z)

*/
void OpenVRMathHelps::getTranslation(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ)
{
	outX = pose.mDeviceToAbsoluteTracking.m[0][3];
	outY = pose.mDeviceToAbsoluteTracking.m[1][3];
	outZ = pose.mDeviceToAbsoluteTracking.m[2][3];
}

/*
M
(cos(b) cos(c)						| -cos(b) sin(c)						| sin(b)			* (1
cos(c) sin(a) sin(b)+cos(a) sin(c)	| cos(a) cos(c)-sin(a) sin(b) sin(c)	| -cos(b) sin(a)	   0
sin(a) sin(c)-cos(a) cos(c) sin(b)	| cos(c) sin(a)+cos(a) sin(b) sin(c)	| cos(a) cos(b))	   0

=

x = M[0][0]
y = M[1][0]
z = M[2][0]

*/
void OpenVRMathHelps::getRotatedVectorX(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ)
{
	outX = pose.mDeviceToAbsoluteTracking.m[0][0];
	outY = pose.mDeviceToAbsoluteTracking.m[1][0];
	outZ = pose.mDeviceToAbsoluteTracking.m[2][0];
}

void OpenVRMathHelps::getRotatedVectorY(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ)
{
	outX = pose.mDeviceToAbsoluteTracking.m[0][1];
	outY = pose.mDeviceToAbsoluteTracking.m[1][1];
	outZ = pose.mDeviceToAbsoluteTracking.m[2][1];
}

void OpenVRMathHelps::getRotatedVectorZ(const vr::TrackedDevicePose_t& pose, float& outX, float& outY, float& outZ)
{
	outX = pose.mDeviceToAbsoluteTracking.m[0][2];
	outY = pose.mDeviceToAbsoluteTracking.m[1][2];
	outZ = pose.mDeviceToAbsoluteTracking.m[2][2];
}


/*
projection a_onto_b = (a dot b)/(b dot b) * b
*/
void OpenVRMathHelps::getProjection(float u[3], float v[3], float u_onto_v[3])
{
	double coef = ((u[0] * v[0]) + (u[1] * v[1]) + (u[2] * v[2])) / ((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));

	u_onto_v[0] = coef*v[0];
	u_onto_v[1] = coef*v[1];
	u_onto_v[2] = coef*v[2];
}



float OpenVRMathHelps::getVelocityMagnitude(const vr::TrackedDevicePose_t& pose)
{
	return sqrt(pose.vVelocity.v[0] * pose.vVelocity.v[0] + pose.vVelocity.v[1] * pose.vVelocity.v[1] + pose.vVelocity.v[2] * pose.vVelocity.v[2]);
}

void OpenVRMathHelps::getNormalizedVector(float inVector[3], float outVector[3])
{
	float magnitude = getVectorMagnitude(inVector);
	for (int i = 0; i < 3; i++)
	{
		outVector[i] = inVector[i] / magnitude;
	}
}

float OpenVRMathHelps::getVectorMagnitude(float inVector[3])
{
	return sqrt(inVector[0] * inVector[0] + inVector[1] * inVector[1] + inVector[2] * inVector[2]);
}


void OpenVRMathHelps::printVector(float v[3])
{
	_MESSAGE("VECTOR: [%g, %g, %g]", v[0], v[1], v[2]);
}

void OpenVRMathHelps::printPose(const vr::TrackedDevicePose_t& pose)
{
	_MESSAGE("BEGIN POSE---------------------------------------");
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[0][0], pose.mDeviceToAbsoluteTracking.m[0][1], pose.mDeviceToAbsoluteTracking.m[0][2], pose.mDeviceToAbsoluteTracking.m[0][3]);
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[1][0], pose.mDeviceToAbsoluteTracking.m[1][1], pose.mDeviceToAbsoluteTracking.m[1][2], pose.mDeviceToAbsoluteTracking.m[1][3]);
	_MESSAGE("[%g, %g, %g, %g]", pose.mDeviceToAbsoluteTracking.m[2][0], pose.mDeviceToAbsoluteTracking.m[2][1], pose.mDeviceToAbsoluteTracking.m[2][2], pose.mDeviceToAbsoluteTracking.m[2][3]);
	_MESSAGE("End POSE---------------------------------------\n");
}