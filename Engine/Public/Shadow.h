#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CShadow final 
{
public:

private:
	CShadow();
public:
	~CShadow();

public:	
	HRESULT Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc) {
		return Apply_Shadow_Light(ShadowLightDesc);
	}
	bool_t Is_Enabled() const { return m_ShadowLightDesc.Settings.bEnabled; }
	const SHADOW_LIGHT_DESC& Get_Desc() const { return m_ShadowLightDesc; }
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_LightingShaderResources(shared_ptr<class CShader> pShader);

private:
	SHADOW_LIGHT_DESC		m_ShadowLightDesc = {};
	float4x4_t				m_TransformMatrices[ETOUI(D3DTS::END)] = {};

public:
	static unique_ptr<CShadow> Create();
};

NS_END
