#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;

class CMainApp final
{
private:
	CMainApp();
public:
	~CMainApp();

public:
	HRESULT Initialize();
	void Update(f32_t fTimeDelta);
	HRESULT Render();

private:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

#ifdef _DEBUG
	std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
	std::unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	bool m_bF1Down = false;
#endif

private:
	HRESULT Ready_Gara();
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Start_Level(LEVEL eStartLevelID);

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	void UpdateDebugToolShortcut();
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END

