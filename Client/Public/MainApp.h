#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

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

private:
	HRESULT Ready_Gara();
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Start_Level(LEVEL eStartLevelID);

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END

