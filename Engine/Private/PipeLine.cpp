#include "PipeLine.h"

#include "GameInstance.h"

CPipeLine::CPipeLine()
{
}

CPipeLine::~CPipeLine()
{
}

void CPipeLine::Set_Transform(D3DTS eType, fmatrix_t TransformMatrix)
{
	XMStoreFloat4x4(&m_TransformMatrices[ETOUI(eType)], TransformMatrix);
}

const float4_t* CPipeLine::Get_CamPosition()
{
	return &m_vCamPosition;
}

const float4x4_t* CPipeLine::Get_Transform(D3DTS eType)
{
	return &m_TransformMatrices[ETOUI(eType)];
}

const float4x4_t* CPipeLine::Get_InverseTransform(D3DTS eType)
{
	return &m_TransformInverseMatrices[ETOUI(eType)];
}

HRESULT CPipeLine::Bind_CamPos_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_RawValue(pConstantName, &m_vCamPosition, sizeof(float4_t));
}

HRESULT CPipeLine::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);
}

HRESULT CPipeLine::Bind_Inverse_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return pShader->Bind_Matrix(pConstantName, &m_TransformInverseMatrices[ETOUI(eType)]);
}

void CPipeLine::Update()
{
	for (size_t i = 0; i < ETOUI(D3DTS::END); i++)
	{
		XMStoreFloat4x4(&m_TransformInverseMatrices[i],
			XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrices[i])));
	}

	memcpy(&m_vCamPosition, &m_TransformInverseMatrices[ETOUI(D3DTS::VIEW)]._41, sizeof(float4_t));
}

unique_ptr<CPipeLine> CPipeLine::Create()
{
	return unique_ptr<CPipeLine>(new CPipeLine());
}
