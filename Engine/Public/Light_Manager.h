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
	HRESULT Replace_SceneLights(vector<LIGHT_DESC> SceneLights);
	HRESULT Render_Lights(shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer,
		bool_t bEnableSceneDirectionalShadow,
        LIGHT_RECEIVER ePassReceiver = LIGHT_RECEIVER::ALL);
	// Rendering-thread view of the currently committed scene lighting.
	const vector<LIGHT_DESC>& Get_SceneLights() const { return m_SceneLights; }
	uint32_t Get_SceneLightCount() const {
		return static_cast<uint32_t>(m_SceneLights.size());
	}

private:
	vector<LIGHT_DESC>						m_SceneLights;

public:
	static unique_ptr<CLight_Manager> Create();
};

NS_END
