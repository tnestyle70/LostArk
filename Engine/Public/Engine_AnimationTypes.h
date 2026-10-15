#pragma once

// 키 프레임에 저장할 DirectX XMFLOAT3/4 선언과 공통 타입을 가져온다.
#include "Engine_Typedef.h"

namespace Engine
{
	// CChannel이 인접한 키를 보간해 본의 로컬 변환을 만드는 입력이다. 구조체 자체는 시각/정규화 유효성을 검사하지 않는다.
	typedef struct tagKeyFrame
	{
		// 배율·quaternion XYZW 회전·본 로컬 이동을 저장한다. 회전은 오일러 각이 아니다.
		DirectX::XMFLOAT3	vScale;
		DirectX::XMFLOAT4	vRotation;
		DirectX::XMFLOAT3	vTranslation;
		// 애니메이션 track 시간이다. 초 단위 경과를 track으로 바꾸는 책임은 CAnimation의 ticks-per-second 경로에 있다.
		float		fTrackPosition;
	}KEYFRAME;
}
