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
	HRESULT Ready_For_Lobby();
	HRESULT Ready_For_Baren();
	HRESULT Ready_For_Level_GamePlay();
	HRESULT Ready_For_Level_AssetTest();
	HRESULT Ready_For_Test_Level2();
	/* The character GameObject and its part classes are shared by every playable
	class, and Add_Prototype rejects a duplicate tag, so this registers them once
	per level -- call it before any of the per-class model helpers below. */
	HRESULT Ready_Character_Shared_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_LanceMaster_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_GunSlinger_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_Artist_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_Slayer_Prototypes(uint32_t iLevelIndex);
	HRESULT Ready_Npc_Prototypes(uint32_t iLevelIndex);

public:
	static unique_ptr<CLoader> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID);
	void Free();
};

NS_END

