#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CPipeLine final
{
private:
	CPipeLine();
public:
	~CPipeLine();

public:
	void Set_Transform(D3DTS eType, fmatrix_t TransformMatrix);
	const float4_t* Get_CamPosition();
	const float4x4_t* Get_Transform(D3DTS eType);
	const float4x4_t* Get_InverseTransform(D3DTS eType);

public:
	HRESULT Bind_CamPos_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_Inverse_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);

public:
	void Update();

private:
	float4x4_t				m_TransformMatrices[ETOUI(D3DTS::END)] = {};
	float4x4_t				m_TransformInverseMatrices[ETOUI(D3DTS::END)] = {};
	float4_t				m_vCamPosition = {};

public:
	static unique_ptr<CPipeLine> Create();
};

NS_END