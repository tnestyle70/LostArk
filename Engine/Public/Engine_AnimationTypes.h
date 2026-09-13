#pragma once

#include "Engine_Typedef.h"

namespace Engine
{
	typedef struct tagKeyFrame
	{
		DirectX::XMFLOAT3	vScale;
		DirectX::XMFLOAT4	vRotation;
		DirectX::XMFLOAT3	vTranslation;
		float		fTrackPosition;
	}KEYFRAME;
}
