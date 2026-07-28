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
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);

private:
	float4x4_t				m_TransformMatrices[ETOUI(D3DTS::END)] = {};	

public:
	static unique_ptr<CShadow> Create();
};

NS_END