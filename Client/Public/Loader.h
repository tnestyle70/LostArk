#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CLoader final 
{
private:
	CLoader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CLoader();

public:
	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Start_Loading();
	bool_t Finished() const {
		return m_isFinished;
	}
	
#ifdef _DEBUG
public:
	void Print_Text();

#endif


private:
	ComPtr<ID3D11Device>		m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	LEVEL						m_eNextLevelID = { LEVEL::END };

private:
	HANDLE						m_hThread = {};
	CRITICAL_SECTION			m_CriticalSection = {};

	tchar_t						m_szLoadingText[MAX_PATH] = {};
	bool_t						m_isFinished = { false };

private:
	HRESULT Ready_For_Level_Logo();
	HRESULT Ready_For_Level_GamePlay();
	HRESULT Ready_For_Level_AssetTest();
	HRESULT Ready_For_Test_Level2();

public:
	static unique_ptr<CLoader> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID);
	void Free();
};

NS_END

