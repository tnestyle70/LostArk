#include "Shadow.h"
#include "GameInstance.h"

CShadow::CShadow()
{
}

CShadow::~CShadow()
{
}

HRESULT CShadow::Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	XMStoreFloat4x4(&m_TransformMatrices[ETOUI(D3DTS::VIEW)], XMMatrixLookAtLH(XMLoadFloat4(&ShadowLightDesc.vEye), XMLoadFloat4(&ShadowLightDesc.vAt), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&m_TransformMatrices[ETOUI(D3DTS::PROJ)], XMMatrixPerspectiveFovLH(XMConvertToRadians(ShadowLightDesc.fFovy), CGameInstance::Get().Get_ViewportSize().x / CGameInstance::Get().Get_ViewportSize().y, ShadowLightDesc.fNear, ShadowLightDesc.fFar));

	return S_OK;
}

HRESULT CShadow::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);
	
}

unique_ptr<CShadow> CShadow::Create()
{
	return unique_ptr<CShadow>(new CShadow());
}
