#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CLight_Manager final 
{
private:
	CLight_Manager();
public:
	~CLight_Manager();

public:
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);

private:
	list<unique_ptr<class CLight>>			m_Lights;

public:
	static unique_ptr<CLight_Manager> Create();
};

NS_END