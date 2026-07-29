#include "Level_Logo.h"

#include "Level_Loading.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"

CLevel_Logo::CLevel_Logo(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_Logo::~CLevel_Logo()
{
}

HRESULT CLevel_Logo::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

#ifdef _WIN64
	const filesystem::path soundPath =
		CRuntimeAssetRoot::Resolve(L"Sound/Yasuo/Yasuo_Q.wav");
	if (FAILED(CGameInstance::Get().Play_Sound(
		soundPath.wstring(), 0.8f)))
		return E_FAIL;
#endif

	return S_OK;
}

void CLevel_Logo::Update(f32_t fTimeDelta)
{
	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_F2) & 0x8000))
	{
		if (FAILED(CGameInstance::Get().Change_Level(
			ETOUI(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ASSET_TEST))))
		{
			MSG_BOX("Failed to Change Level");
			return;
		}

		return;
	}

	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_SPACE) & 0x8000))
	{
		if (FAILED(CGameInstance::Get().Change_Level(ETOUI(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::GAMEPLAY))))
		{
			MSG_BOX("Failed to Change Level");
			return;
		}

		return;
	}


	__super::Update(fTimeDelta);		
}

HRESULT CLevel_Logo::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("로고레벨입니다."));
#endif

	return S_OK;
}

HRESULT CLevel_Logo::Ready_Layer_BackGround(const wstring_t& strLayerTag)
{
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_BackGround"),
		ETOUI(LEVEL::LOGO), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CLevel_Logo> CLevel_Logo::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Logo>(new CLevel_Logo(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Logo");
		return nullptr;
	}

	return pInstance;
}
