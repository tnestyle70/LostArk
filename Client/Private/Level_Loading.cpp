#include "Level_Loading.h"
#include "Loader.h"

#include "Level_Lobby.h"
#include "Level_Logo.h"
#include "Level_Baren.h"
#include "Level_GamePlay.h"
#include "Level_AssetTest.h"
#include "Level_Test2.h"
#include "GameInstance.h"

CLevel_Loading::CLevel_Loading(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_eNextLevelID = eNextLevelID;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);

	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void CLevel_Loading::Update(f32_t fTimeDelta)
{
	if (nullptr == m_pLoader)
		return;

	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_RETURN) & 0x8000) &&
		true == m_pLoader->Finished())
	{
		unique_ptr<CLevel>		pNewLevel = { nullptr };

		switch (m_eNextLevelID)
		{
		case LEVEL::LOGO:
			pNewLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::LOBBY:
			pNewLevel = CLevel_Lobby::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::BAREN:
			pNewLevel = CLevel_Baren::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::GAMEPLAY:
			pNewLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::ASSET_TEST:
			pNewLevel = CLevel_AssetTest::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::TEST_LEVEL2:
			pNewLevel = CLevel_Test2::Create(m_pDevice, m_pContext);
			break;
		}	

		if (nullptr == pNewLevel)
		{
			MSG_BOX("Failed to Create Level");
			return;
		}

		if (FAILED(CGameInstance::Get().Change_Level(ETOUI(m_eNextLevelID), move(pNewLevel))))
		{
			MSG_BOX("Failed to Change Level");
			return;
		}

		return;
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	if(nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif

	return S_OK;
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID)
{
	auto pInstance = unique_ptr<CLevel_Loading>(new CLevel_Loading(pDevice, pContext));

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : CLevel_Loading");
		return nullptr;
	}

	return pInstance;
}
