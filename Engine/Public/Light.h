#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CLight final 
{
private:
	CLight();
public:
	~CLight();

public:
	HRESULT Initialize(const LIGHT_DESC& LightDesc);
	HRESULT Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);
	static HRESULT Render_Desc(
		const LIGHT_DESC& LightDesc,
		shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer);

private:
	LIGHT_DESC				m_LightDesc;

public:
	static unique_ptr<CLight> Create(const LIGHT_DESC& LightDesc);
};

NS_END
