#pragma once

#include "Config.h"

ValueInstance _getNearClip()
{
	return ValueInstance(*((float*)0x12C8D54), true);
}

void _setNearClip(ValueInstance vi)
{
#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG
	if (!Camera::GetSingleton()->UpdateNearFarDefault())
		return;
#endif

	ConfigValue * cv1 = Config::_getValueInternal(ConfigValueNames::fNearClip);
	if (!cv1)
		return;

	ValueInstance vi1 = cv1->GetCurrentValue(true);

	float cur = 500.0f;
	if (vi1.HasValue && vi1.Type == ConfigValueTypes::Float)
	{
		float fcur = vi1.ToFloat();
		if (fcur < cur)
			cur = fcur;
	}

	if (cur < 1.0f)
		cur = 1.0f;

	{
		float prev = *((float*)0x12C8D54);
		if (cur != prev)
		{
			*((float*)0x12C8D54) = cur;
			Config::_setUpdateGraphics();
		}
	}
}

#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG

ValueInstance _getAdjustFar()
{
	return ValueInstance(AdjustFarClipMin, true);
}

void _setAdjustFar(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return;

	float cur = vi.ToFloat();
	if (AdjustFarClipMin != cur)
	{
		AdjustFarClipMin = cur;
		Config::_setUpdateGraphics();
	}
}

ValueInstance _getAdjustFar2()
{
	return ValueInstance(AdjustFarClipStart, true);
}

void _setAdjustFar2(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return;

	float cur = vi.ToFloat();
	if (AdjustFarClipStart != cur)
	{
		AdjustFarClipStart = cur;
		Config::_setUpdateGraphics();
	}
}

#endif

ValueInstance _getTimeScale()
{
	return ValueInstance(*((float*)0x12C2414), true);
}

void _setTimeScale(ValueInstance vi)
{
	if (vi.HasValue && vi.Type == ConfigValueTypes::Float)
	{
		float cur = vi.ToFloat();
		if (cur < 0.0f)
			cur = 0.0f;
		*((float*)0x12C2414) = cur;
	}
}

ValueInstance _getUpDownLimit()
{
	return ValueInstance(NewUpDownLimit, true);
}

void _setUpDownLimit(ValueInstance vi)
{
	if (vi.HasValue && vi.Type == ConfigValueTypes::Float)
	{
		float cur = vi.ToFloat();
		NewUpDownLimit = cur;
	}
}

ValueInstance _getVATSDisable()
{
	int val = *((int*)0x1278034);
	return ValueInstance((val & 0xFF) != 0, true);
}

void _setVATSDisable(ValueInstance vi)
{
	if (vi.HasValue && vi.Type == ConfigValueTypes::Bool)
		*((int*)0x1278034) = vi.ToBool() ? 1 : 0;
}

ValueInstance _getFOV()
{
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam)
		return ValueInstance(cam->worldFOV, true);
	return ValueInstance(90.0f, false);
}

void _setFOV(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return;

	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam)
	{
		float cur = vi.ToFloat();
		if (cam->worldFOV != cur)
		{
			cam->worldFOV = vi.ToFloat();
			Config::_setUpdateGraphics();
		}
	}
}

ValueInstance _getCVOffsetH()
{
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam && cam->cameraState)
	{
		switch (cam->cameraState->stateId)
		{
		case PlayerCamera::kCameraState_ThirdPerson2:
		case PlayerCamera::kCameraState_Horse:
		case PlayerCamera::kCameraState_Dragon:
			return ValueInstance(*((float*)((int)cam->cameraState + 0xAC)), true);
		}
	}

	return ValueInstance(0.0f, false);
}

void _setCVOffsetH(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return;

	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam && cam->cameraState)
	{
		switch (cam->cameraState->stateId)
		{
		case PlayerCamera::kCameraState_ThirdPerson2:
		case PlayerCamera::kCameraState_Horse:
		case PlayerCamera::kCameraState_Dragon:
			*((float*)((int)cam->cameraState + 0xAC)) = vi.ToFloat();
			return;
		}
	}
}

ValueInstance _getCVOffsetV()
{
	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam && cam->cameraState)
	{
		switch (cam->cameraState->stateId)
		{
		case PlayerCamera::kCameraState_ThirdPerson2:
		case PlayerCamera::kCameraState_Horse:
		case PlayerCamera::kCameraState_Dragon:
			return ValueInstance(*((float*)((int)cam->cameraState + 0xB0)), true);
		}
	}

	return ValueInstance(0.0f, false);
}

void _setCVOffsetV(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Float)
		return;

	PlayerCamera * cam = PlayerCamera::GetSingleton();
	if (cam && cam->cameraState)
	{
		switch (cam->cameraState->stateId)
		{
		case PlayerCamera::kCameraState_ThirdPerson2:
		case PlayerCamera::kCameraState_Horse:
		case PlayerCamera::kCameraState_Dragon:
			*((float*)((int)cam->cameraState + 0xB0)) = vi.ToFloat();
			return;
		}
	}
}

ValueInstance _getTargetRefId()
{
	return ValueInstance(Camera::GetSingleton()->GetTargetOverwritten(), true);
}

void _setTargetRefId(ValueInstance vi)
{
	if (!vi.HasValue || vi.Type != ConfigValueTypes::Int)
		return;

	Camera::GetSingleton()->SetTargetOverwrite(vi.ToInt());
}
