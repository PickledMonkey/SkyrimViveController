#pragma once

#include <cmath>

#define M_PI 3.14159265358979323846
#define DegToRad 0.017453f
#define RadToDeg 57.295779f

#ifdef _DEBUG
#include "Windows.h"
#endif

struct Quaternion
{
	float w;
	float x;
	float y;
	float z;
};

struct Point
{
	Point()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	float x;
	float y;
	float z;

	bool IsEmpty()
	{
		return x == 0.0f && y == 0.0f && z == 0.0f;
	}

	float Length()
	{
		return sqrtf(x * x + y * y + z * z);
	}

	void Normalize()
	{
		float len = Length();
		if (len > 0.0f)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	static Point Cross(Point a, Point b)
	{
		Point res;
		res.x = (a.y * b.z) - (a.z * b.y);
		res.y = (a.z * b.x) - (a.x * b.z);
		res.z = (a.x * b.y) - (a.y * b.x);
		return res;
	}

	static float Dot(Point a, Point b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static float Distance(Point a, Point b)
	{
		Point c;
		c.x = b.x - a.x;
		c.y = b.y - a.y;
		c.z = b.z - a.z;

		return c.Length();
	}
};

struct Matrix3
{
	float data[3][3];

	static bool closeEnough(const float& a, const float& b, const float& epsilon = std::numeric_limits<float>::epsilon())
	{
		return (epsilon > std::abs(a - b));
	}

	static Matrix3 Inverse(Matrix3 mat)
	{
		float determinant, invDeterminant;
		float tmp[9];
		float m[9];
		
		memcpy(m, &mat, sizeof(Matrix3));

		tmp[0] = m[4] * m[8] - m[5] * m[7];
		tmp[1] = m[2] * m[7] - m[1] * m[8];
		tmp[2] = m[1] * m[5] - m[2] * m[4];
		tmp[3] = m[5] * m[6] - m[3] * m[8];
		tmp[4] = m[0] * m[8] - m[2] * m[6];
		tmp[5] = m[2] * m[3] - m[0] * m[5];
		tmp[6] = m[3] * m[7] - m[4] * m[6];
		tmp[7] = m[1] * m[6] - m[0] * m[7];
		tmp[8] = m[0] * m[4] - m[1] * m[3];

		determinant = m[0] * tmp[0] + m[1] * tmp[3] + m[2] * tmp[6];
		if (fabs(determinant) <= std::numeric_limits<float>::epsilon())
			return Identity();

		invDeterminant = 1.0f / determinant;
		m[0] = invDeterminant * tmp[0];
		m[1] = invDeterminant * tmp[1];
		m[2] = invDeterminant * tmp[2];
		m[3] = invDeterminant * tmp[3];
		m[4] = invDeterminant * tmp[4];
		m[5] = invDeterminant * tmp[5];
		m[6] = invDeterminant * tmp[6];
		m[7] = invDeterminant * tmp[7];
		m[8] = invDeterminant * tmp[8];

		Matrix3 res;
		memcpy(&res, m, sizeof(Matrix3));

		return res;

	}

	static Matrix3 Identity(float scale = 1.0f)
	{
		Matrix3 result;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
				result.data[i][j] = i == j ? scale : 0.0f;
		}

		return result;
	}

	static Matrix3 Multiply(Matrix3 first, Matrix3 second)
	{
		Matrix3 result;

		float temp = 0.0f;
		int a, b, c;

		for (a = 0; a < 3; a++)
		{
			for (b = 0; b < 3; b++)
			{
				for (c = 0; c < 3; c++)
					temp += first.data[b][c] * second.data[c][a];

				result.data[b][a] = temp;
				temp = 0.0f;
			}
		}

		return result;
	}

	static Matrix3 RotateZ(Matrix3 first, float rad, bool flip = false)
	{
		float c = cosf(rad);
		float s = sinf(rad);

		Matrix3 R = Identity();

		R.data[0][0] = c;	R.data[1][0] = s;
		R.data[0][1] = -s;	R.data[1][1] = c;

		if(!flip)
			return Multiply(R, first);
		return Multiply(first, R);
	}

	static Matrix3 RotateY(Matrix3 first, float rad, bool flip = false)
	{
		float c = cosf(rad);
		float s = sinf(rad);

		Matrix3 R = Identity();

		R.data[0][0] = c;	R.data[2][0] = -s;
		R.data[0][2] = s;	R.data[2][2] = c;

		if(!flip)
			return Multiply(R, first);
		return Multiply(first, R);
	}

	static Matrix3 RotateX(Matrix3 first, float rad, bool flip = false)
	{
		float c = cos(rad);
		float s = sin(rad);

		Matrix3 R = Identity();

		R.data[1][1] = c;	R.data[2][1] = s;
		R.data[1][2] = -s;	R.data[2][2] = c;

		if(!flip)
			return Multiply(R, first);
		return Multiply(first, R);
	}

	static Matrix3 Interpolate(Matrix3 a, Matrix3 b, float alpha)
	{
		Matrix3 c = Matrix3::Identity();

		Matrix3 aT = Transpose(a);
		Matrix3 delta = Multiply(b, aT);
		Point axis;
		float deltaAngle;
		GetAxisAngle(delta, axis, deltaAngle);
		Matrix3 R = GetFromAxisAngle(axis, deltaAngle * alpha);
		c = Multiply(R, a);

		return c;
	}

	static Matrix3 Transpose(Matrix3 a)
	{
		Matrix3 b;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
				b.data[i][j] = a.data[j][i];
		}

		return b;
	}

	static void GetAxisAngle(Matrix3 a, Point& axis, float& angle)
	{
		float c2 = a.data[0][0] + a.data[1][1] + a.data[2][2] - 1.0f;
		axis.x = a.data[2][1] - a.data[1][2];
		axis.y = a.data[0][2] - a.data[2][0];
		axis.z = a.data[1][0] - a.data[0][1];
		float s2 = axis.Length();

		if (s2 > 0.0f)
		{
			angle = atan2(s2, c2);
			axis.x *= 1.0f / s2;
			axis.y *= 1.0f / s2;
			axis.z *= 1.0f / s2;
			return;
		}

		if (c2 >= 2.0f)
		{
			axis.x = 1.0f;
			axis.y = 0.0f;
			axis.z = 0.0f;
			angle = 0.0f;
			return;
		}

		angle = (float)M_PI;

		if (a.data[2][2] > a.data[0][0] && a.data[2][2] > a.data[1][1])
		{
			axis.z = 0.5f * sqrtf(a.data[2][2] - a.data[0][0] - a.data[1][1] + 1.0f);
			float dbla = 0.5f / axis.z;
			axis.x = dbla * a.data[0][2];
			axis.y = dbla * a.data[1][2];
			return;
		}
		if (a.data[1][1] > a.data[0][0] && a.data[1][1] > a.data[2][2])
		{
			axis.y = 0.5f * sqrtf(a.data[1][1] - a.data[0][0] - a.data[2][2] + 1.0f);
			float dbla = 0.5f / axis.y;
			axis.x = dbla * a.data[0][1];
			axis.z = dbla * a.data[1][2];
			return;
		}

		{
			axis.x = 0.5f * sqrtf(a.data[0][0] - a.data[1][1] - a.data[2][2] + 1.0f);
			float dbla = 0.5f / axis.x;
			axis.y = dbla * a.data[0][1];
			axis.z = dbla * a.data[0][2];
			return;
		}
	}

	static Matrix3 GetFromAxisAngle(Point axis, float angle)
	{
		Matrix3 R = Matrix3::Identity();

		float c = cosf(angle);
		float s = sinf(angle);
		float t = 1.0f - c;
		
		R.data[0][0] = c + axis.x * axis.x * t;
		R.data[1][1] = c + axis.y * axis.y * t;
		R.data[2][2] = c + axis.z * axis.z * t;

		// No idea what any of this is, I copied from google and game didn't crash.
		float tmp1 = axis.x * axis.y * t;
		float tmp2 = axis.z * s;
		R.data[1][0] = tmp1 + tmp2;
		R.data[0][1] = tmp1 - tmp2;
		tmp1 = axis.x * axis.z * t;
		tmp2 = axis.y * s;
		R.data[2][0] = tmp1 - tmp2;
		R.data[0][2] = tmp1 + tmp2;
		tmp1 = axis.y * axis.z * t;
		tmp2 = axis.x * s;
		R.data[2][1] = tmp1 + tmp2;
		R.data[1][2] = tmp1 - tmp2;

		return R;
	}

	static Matrix3 TurnByDifference(Matrix3 sourceMatrix, Point sourceVector, Point targetVector)
	{
		Matrix3 R = GetDifference(sourceVector, targetVector);
		Matrix3 c = Matrix3::Multiply(R, sourceMatrix);
		return c;
	}

	static Matrix3 GetDifference(Point sourceVector, Point targetVector)
	{
		float angle = acos(Point::Dot(sourceVector, targetVector));
		Point axis = Point::Cross(sourceVector, targetVector);
		axis.Normalize();

		return Matrix3::GetFromAxisAngle(axis, angle);
	}

	static Matrix3 LookAt(Point sourcePos, Point targetPos)
	{
		/*Matrix3 mat = Matrix3::Identity();

		Point up;
		up.x = 0.0f;
		up.y = 1.0f;
		up.z = 0.0f;

		Point forward;
		forward.x = targetPos.x - sourcePos.x;
		forward.y = targetPos.y - sourcePos.y;
		forward.z = targetPos.z - sourcePos.z;
		forward.Normalize();

		Point side = Point::Cross(forward, up);
		side.Normalize();

		up = Point::Cross(side, forward);

		mat.data[0][0] = side.x;
		mat.data[1][0] = side.y;
		mat.data[2][0] = side.z;
		mat.data[0][1] = up.x;
		mat.data[1][1] = up.y;
		mat.data[2][1] = up.z;
		mat.data[0][2] = -forward.x;
		mat.data[1][2] = -forward.y;
		mat.data[2][2] = -forward.z;

		return mat;*/

		Point newForward;
		newForward.x = targetPos.x - sourcePos.x;
		newForward.y = targetPos.y - sourcePos.y;
		newForward.z = targetPos.z - sourcePos.z;
		newForward.Normalize();

		Point oldForward;
		float temp = 0.0f;
		Matrix3::GetAxisAngle(Matrix3::Identity(), oldForward, temp);

		float dot = Point::Dot(oldForward, newForward);
		if (closeEnough(dot, -1.0f))
		{
			Point up;
			up.y = 1.0f;

			return Matrix3::GetFromAxisAngle(up, (float)M_PI);
		}
		else if (closeEnough(dot, 1.0f))
			return Matrix3::Identity();

		float rotAngle = acos(dot);
		Point rotAxis = Point::Cross(oldForward, newForward);
		rotAxis.Normalize();

		return Matrix3::GetFromAxisAngle(rotAxis, rotAngle);
	}

	static float EulerAngles(Matrix3 R, int axis, bool flipped)
	{
		if (!flipped)
		{
			if (closeEnough(R.data[0][2], -1.0f))
			{
				if (axis == 0)
					return 0.0f;
				if (axis == 1)
					return (float)M_PI * 0.5f;
				return atan2(R.data[1][0], R.data[2][0]);
			}
			else if (closeEnough(R.data[0][2], 1.0f))
			{
				float x = 0;
				if (axis == 0)
					return 0.0f;
				if (axis == 1)
					return -(float)M_PI * 0.5f;
				return atan2(-R.data[1][0], -R.data[2][0]);
			}
			else
			{
				float x1 = -asin(R.data[0][2]);
				if (axis == 0)
					return x1;

				if (axis == 1)
					return atan2(R.data[1][2] / cos(x1), R.data[2][2] / cos(x1));

				return atan2(R.data[0][1] / cos(x1), R.data[0][0] / cos(x1));
			}
		}

		if (closeEnough(R.data[2][0], -1.0f))
		{
			if (axis == 0)
				return 0.0f;
			if (axis == 1)
				return (float)M_PI * 0.5f;
			return atan2(R.data[0][1], R.data[0][2]);
		}
		else if (closeEnough(R.data[2][0], 1.0f))
		{
			float x = 0;
			if (axis == 0)
				return 0.0f;
			if (axis == 1)
				return -(float)M_PI * 0.5f;
			return atan2(-R.data[0][1], -R.data[0][2]);
		}
		else
		{
			float x1 = -asin(R.data[2][0]);
			if (axis == 0)
				return x1;

			if (axis == 1)
				return atan2(R.data[2][1] / cos(x1), R.data[2][2] / cos(x1));

			return atan2(R.data[1][0] / cos(x1), R.data[0][0] / cos(x1));
		}
	}
};

struct Matrix4
{
	float data[4][4];

	static Matrix4 Identity()
	{
		Matrix4 result;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
				result.data[i][j] = i == j ? 1.0f : 0.0f;
		}

		return result;
	}

	static Matrix4 Multiply(Matrix4 first, Matrix4 second)
	{
		Matrix4 result;

		float temp = 0.0f;
		int a, b, c;

		for (a = 0; a < 4; a++)
		{
			for (b = 0; b < 4; b++)
			{
				for (c = 0; c < 4; c++)
					temp += first.data[b][c] * second.data[c][a];

				result.data[b][a] = temp;
				temp = 0.0f;
			}
		}

		return result;
	}

	static Matrix4 GetMatrix4(Matrix3 first)
	{
		Matrix4 result = Identity();
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
				result.data[i][j] = first.data[i][j];
		}

		return result;
	}

	static Matrix4 GetMatrix4(Matrix3 first, Point pos)
	{
		Matrix4 result = Identity();
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
				result.data[i][j] = first.data[i][j];
		}

		result.data[3][0] = pos.x;
		result.data[3][1] = pos.y;
		result.data[3][2] = pos.z;

		return result;
	}
};

struct Transform
{
	Matrix3 rot;
	Point pos;
	float scale;

	static Point Translate(Transform transform, float moveHorizontal, float moveVertical, float moveDepth)
	{
		Point move = transform.pos;
		
		Point origPos;
		origPos.x = moveHorizontal;
		origPos.y = moveDepth;
		origPos.z = moveVertical;

		Point newPos;

		float * tempData = (float*)&transform.rot;

		newPos.x = tempData[0] * origPos.x + tempData[1] * origPos.y + tempData[2] * origPos.z + move.x;
		newPos.y = tempData[3] * origPos.x + tempData[4] * origPos.y + tempData[5] * origPos.z + move.y;
		newPos.z = tempData[6] * origPos.x + tempData[7] * origPos.y + tempData[8] * origPos.z + move.z;

		return newPos;
	}
};
