#include "imgui.h"

#include "Level_Loading.h"
#include "ClientLaunchOptions.h"
#include "LevelRegistry.h"
#include "Loader.h"
#include "NetworkManager.h"
#include "SceneTransitionService.h"
#include "UI_Sprite.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include "GameInstance.h"

#include <fstream>
#include <algorithm>

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

	/* TODO: every destination shows the Bern title/tip for now. Once each scene can report
	its own identity, branch on eNextLevelID (or a passed-in scene id) here instead. */
	m_strTitleText = L"\xBCA0\xB978 \xC131";
	m_strTipText = L"\xBCA0\xB978 \xC131\xC740 \xC5EC\xB7EC \xC885\xC871\xC774 \xD568\xAED8 \xC11E\xC5EC \xC788\xB294, \xBCA0\xB978\xC758 \xC218\xB3C4\xC785\xB2C8\xB2E4.";

	if (FAILED(Ready_Layer_Chrome()))
		return E_FAIL;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);

	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void CLevel_Loading::Update(f32_t fTimeDelta)
{
	if (m_isRetryRequested)
	{
		m_isRetryRequested = false;
		Retry_LobbyLoad();
		return;
	}

	if (nullptr == m_pLoader)
		return;

	if (m_pLoader->Failed())
	{
		Recover_FromFailure(m_pLoader->Get_Result());
		return;
	}

	/* No per-step byte/asset count exists in CLoader, so the bar eases toward 90% on its own
	and only snaps the rest of the way once the loader actually reports finished -- it never
	looks "done" while still waiting on the target level's real assets. */
	const f32_t fTargetProgress = m_pLoader->Finished() ? 1.f : 0.9f;
	m_fDisplayProgress += (fTargetProgress - m_fDisplayProgress) * (min)(1.f, fTimeDelta * 2.5f);

	const f32_t fTrackLeft = m_fProgressTrackX - m_fProgressTrackWidth * 0.5f;
	const f32_t fFillWidth = m_fProgressTrackWidth * m_fDisplayProgress;

	if (nullptr != m_pProgressFill)
		m_pProgressFill->Set_Rect(fTrackLeft + fFillWidth * 0.5f, m_fProgressTrackY,
			fFillWidth, m_fProgressTrackHeight);

	if (nullptr != m_pProgressGlow)
		m_pProgressGlow->Set_Rect(fTrackLeft + fFillWidth, m_fProgressTrackY,
			60.f, m_fProgressTrackHeight * 3.f);

	const bool_t isContinueRequested =
		LEVEL::LOBBY == m_eNextLevelID ||
		CClientLaunchOptions::Get().isAutoActivate ||
		(!CGameInstance::Get().IsKeyboardInputBlocked() &&
			(GetKeyState(VK_RETURN) & 0x8000));

	if (isContinueRequested &&
		true == m_pLoader->Finished())
	{
		unique_ptr<CLevel> pNewLevel =
			CLevelRegistry::Create_Level(
				m_eNextLevelID,
				m_pDevice,
				m_pContext);

		if (nullptr == pNewLevel)
		{
			Recover_FromFailure(E_FAIL);
			return;
		}

		if (FAILED(CGameInstance::Get().Change_Level(ETOUI(m_eNextLevelID), move(pNewLevel))))
		{
			Recover_FromFailure(E_FAIL);
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

	if (!m_strTitleText.empty())
		CGameInstance::Get().Draw_Text(TEXT("Font_Default"), m_strTitleText.c_str(),
			m_vTitlePos, Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.65f);

	if (!m_strTipText.empty())
	{
		CGameInstance::Get().Draw_Text(TEXT("Font_Default"), L"\xC2DC\xB098\xB9AC\xC624",
			m_vScenarioPos, Colors::Gold, 0.f, float2_t(0.5f, 0.5f), 0.55f);

		CGameInstance::Get().Draw_Text(TEXT("Font_Default"), m_strTipText.c_str(),
			m_vTipPos, Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.6f);
	}

	if (m_isFailureReported && LEVEL::LOBBY == m_eNextLevelID)
	{
		ImGui::SetNextWindowPos(
			ImVec2(24.f, 24.f), ImGuiCond_Always);
		if (ImGui::Begin(
			"Loading recovery",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped(
				"Lobby resources could not be loaded. The active world session was closed and partial resources were rolled back.");
			if (ImGui::Button("Retry Lobby"))
				m_isRetryRequested = true;
		}
		ImGui::End();
	}

#ifdef _DEBUG
	if(nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif

	return S_OK;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	CSceneTransitionService::Report_LoadFailure(result);
	CNetworkManager::Get().Close_ServerConnection();

	if (FAILED(CGameInstance::Get().Clear_Resources(
		ETOUI(m_eNextLevelID))))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to clear partial target resources.\n");
	}

	OutputDebugStringA(
		"[Level_Loading] Load failed; session closed and partial resources rolled back.\n");

	if (LEVEL::LOBBY != m_eNextLevelID)
		Retry_LobbyLoad();
}

void CLevel_Loading::Retry_LobbyLoad()
{
	unique_ptr<CLevel_Loading> pRecoveryLevel =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			LEVEL::LOBBY);
	if (nullptr == pRecoveryLevel ||
		FAILED(CGameInstance::Get().Change_Level(
			ETOUI(LEVEL::LOADING),
			move(pRecoveryLevel))))
	{
		CSceneTransitionService::Report_LoadFailure(E_FAIL);
		OutputDebugStringA(
			"[Level_Loading] Failed to start Lobby recovery.\n");
	}
}

HRESULT CLevel_Loading::Ready_Layer_Chrome()
{
	const filesystem::path layoutPath =
		CProjectDataRoot::Resolve(L"UI/Loading/LoadingLayout.json");

	ifstream stream(layoutPath);
	if (!stream.is_open())
		return S_OK;

	const string text(
		(istreambuf_iterator<char>(stream)),
		istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	string error;
	if (!CDataJson::Parse(text, root, error))
		return S_OK;

	const DATA_JSON_VALUE* pSlots = root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Array())
		return S_OK;

	for (const DATA_JSON_VALUE& slot : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE* pId = slot.Find("id");
		const DATA_JSON_VALUE* pRect = slot.Find("rect");
		const DATA_JSON_VALUE* pLayers = slot.Find("layers");
		if (nullptr == pId || !pId->Is_String() ||
			nullptr == pRect || !pRect->Is_Object())
			continue;

		const DATA_JSON_VALUE* pX = pRect->Find("x");
		const DATA_JSON_VALUE* pY = pRect->Find("y");
		const DATA_JSON_VALUE* pW = pRect->Find("width");
		const DATA_JSON_VALUE* pH = pRect->Find("height");
		if (nullptr == pX || nullptr == pY || nullptr == pW || nullptr == pH)
			continue;

		const f32_t fX = static_cast<f32_t>(pX->Get_Number());
		const f32_t fY = static_cast<f32_t>(pY->Get_Number());
		const f32_t fWidth = static_cast<f32_t>(pW->Get_Number());
		const f32_t fHeight = static_cast<f32_t>(pH->Get_Number());

		const string& strId = pId->Get_String();

		string strTexturePath;
		if (nullptr != pLayers && pLayers->Is_Array() && !pLayers->Get_Array().empty())
		{
			const DATA_JSON_VALUE* pPath = pLayers->Get_Array().front().Find("path");
			if (nullptr != pPath && pPath->Is_String())
				strTexturePath = pPath->Get_String();
		}

		/* Texture-less slots (empty "layers") are position-only markers the HUD Layout Tool
		can still drag -- pull text draw positions from them instead of creating a sprite. */
		if (strTexturePath.empty())
		{
			const float2_t vCenter(fX + fWidth * 0.5f, fY + fHeight * 0.5f);

			if ("TitleText" == strId)
				m_vTitlePos = vCenter;
			else if ("ScenarioLabel" == strId)
				m_vScenarioPos = vCenter;
			else if ("TipText" == strId)
				m_vTipPos = vCenter;

			continue;
		}

		/* The HUD Layout Tool's canvas (and this JSON) store top-left + size, but CUIObject
		positions itself by center -- convert once here instead of teaching the tool a second
		convention. */
		const wstring_t widePath(strTexturePath.begin(), strTexturePath.end());

		CUI_Sprite::UI_SPRITE_DESC Desc{};
		Desc.fX = fX + fWidth * 0.5f;
		Desc.fY = fY + fHeight * 0.5f;
		Desc.fSizeX = fWidth;
		Desc.fSizeY = fHeight;
		Desc.strTextureTag = widePath;

		shared_ptr<CGameObject> pObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC),
			TEXT("Prototype_GameObject_UI_Sprite"),
			ETOUI(LEVEL::LOADING), TEXT("Layer_Chrome"), &Desc, &pObject)))
		{
			/* One missing/renamed chrome texture should not take the whole loading screen down. */
			continue;
		}

		if ("ProgressFill" == strId)
		{
			m_pProgressFill = static_pointer_cast<CUI_Sprite>(pObject);
			m_fProgressTrackX = Desc.fX;
			m_fProgressTrackY = Desc.fY;
			m_fProgressTrackWidth = fWidth;
			m_fProgressTrackHeight = fHeight;
		}
		else if ("ProgressGlow" == strId)
		{
			m_pProgressGlow = static_pointer_cast<CUI_Sprite>(pObject);
		}
	}

	return S_OK;
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID)
{
	auto pInstance = unique_ptr<CLevel_Loading>(new CLevel_Loading(pDevice, pContext));

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		OutputDebugStringA("[Level_Loading] Create failed.\n");
		return nullptr;
	}

	return pInstance;
}
