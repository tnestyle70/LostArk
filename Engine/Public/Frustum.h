#pragma once

/* 월드 공간의 절두체 여섯개 평면의 정보를 만들어서보관한다. */
/* 판단하고자하는 월드위치를 받아서 여섯개 평면 안에 있는지 밖에 있는지를 판단한다. */
#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CFrustum final
{
private:
	CFrustum();
public:
	~CFrustum();

public:
	HRESULT Initialize();
	void Update_InWorldSpace();
	void Update_InLocalSpace(fmatrix_t WorldMatrix);

	bool_t isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange);
	bool_t isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange);

private:
	float4_t				m_vWorldPlanes[6] = {};
	float4_t				m_vLocalPlanes[6] = {};

public:
	static unique_ptr<CFrustum> Create();
};



NS_END