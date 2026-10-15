#pragma once

/* 월드 공간의 절두체 여섯개 평면의 정보를 만들어서보관한다. */
/* 판단하고자하는 월드위치를 받아서 여섯개 평면 안에 있는지 밖에 있는지를 판단한다. */
// 보충: 현재 카메라 View/Projection에서 만든 바깥 방향 평면으로 구의 중심과 반지름을 검사한다. 아래 local 조회는 별도 좌표계 변환 뒤 사용한다.
// 평면·벡터·행렬의 엔진/DirectX 타입과 생성 결과의 unique_ptr 소유권을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 현재 카메라의 여섯 평면이다. 바깥 방향 법선과 정규화된 거리 식을 저장한다.
class CFrustum final
{
private:
	CFrustum();
public:
	~CFrustum();

public:
	// Create가 호출하는 기본 성공 단계다. 실제 카메라 평면은 이후 Update_InWorldSpace에서 계산한다.
	HRESULT Initialize();
	// Late_Update에서 현재 View * Projection으로 월드 평면을 갱신한다.
	// 보충: 현재 직접 호출자는 GameInstance::Refresh_CameraState다. 프레임 Priority_Update 뒤와 Camera의 행렬 제출 시에도 호출되므로 Late_Update에만 한정되지 않는다.
	void Update_InWorldSpace();
	// 마지막 월드 평면을 특정 객체의 local 좌표계로 바꾼다. 호출 시마다 이전 객체의 local 평면을 덮어쓴다.
	void Update_InLocalSpace(fmatrix_t WorldMatrix);

	// 같은 좌표계의 중심/비음수 반지름으로 구를 검사한다. 어느 평면에서도 완전히 밖으로 나가지 않으면 true다.
	// 바깥에서 닿는 접촉(dot == radius)은 >= 비교로 제외한다. 입력의 유한성/반지름 범위를 여기서 검증하지 않는다.
	bool_t isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange);
	bool_t isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange);

private:
	// xyz는 단위 바깥 법선, w는 평면 상수다. local 갱신과 조회를 동시에 보호하는 내부 잠금은 없다.
	float4_t				m_vWorldPlanes[6] = {};
	float4_t				m_vLocalPlanes[6] = {};

public:
	// GameInstance가 소유할 빈 평면 저장소를 만든다. 실제 카메라 행렬을 읽는 갱신은 별도로 수행한다.
	static unique_ptr<CFrustum> Create();
};

NS_END