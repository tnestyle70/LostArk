#include "imgui.h"

#include "Level_Loading.h"

#include "AnimationEffectCueDocument.h"
#include "ActorCatalog.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "CharacterSpec.h"
#include "DataJson.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "Loader.h"
#include "NetworkManager.h"
#include "ProjectDataRoot.h"
#include "UI_Sprite.h"
#include "ValtanPatternEffectCueDocument.h"

#include <algorithm>
#include <fstream>

namespace
{
	constexpr size_t VALTAN_PATTERN_EFFECT_CUE_COUNT = 98u;
	constexpr size_t VALTAN_COMBAT_OBJECT_EFFECT_TARGET_COUNT = 2u;
	constexpr size_t VALTAN_PRODUCT_EFFECT_TARGET_COUNT =
		VALTAN_PATTERN_EFFECT_CUE_COUNT +
		VALTAN_COMBAT_OBJECT_EFFECT_TARGET_COUNT;
}

CLevel_Loading::CLevel_Loading(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
}

HRESULT CLevel_Loading::Initialize(
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken &&
		LEVEL::LOBBY != eNextLevelID)
	{
		return E_INVALIDARG;
	}

	m_eNextLevelID = eNextLevelID;
	m_iLobbyCommandToken = lobbyCommandToken;

	/* No per-scenario text source exists yet, so every target level shows a fixed
	placeholder title/tip. Written as \x escapes -- this source file has no BOM and the
	project builds without /utf-8, so literal Korean text here would be misread as CP949.
	Valtan Arena: "Revived Heart of the Beast" / "From the Revived Heart of the Beast, the
	howls of beasts can be heard." Everything else keeps Bern's placeholder: "Bern Castle" /
	"Bern Castle is the capital of Bern, where many races live mixed together." */
	if (LEVEL::VALTAN_ARENA == m_eNextLevelID)
	{
		m_strTitleText = L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5";
		m_strTipText = L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5\xC5D0\xC11C \xC9D0\xC2B9\xB4E4\xC758 \xC6B8\xBD80\xC9D6\xC74C\xC774 \xB4E4\xB824\xC635\xB2C8\xB2E4.";
	}
	else
	{
		m_strTitleText = L"\xBCA0\xB978 \xC131";
		m_strTipText = L"\xBCA0\xB978 \xC131\xC740 \xC5EC\xB7EC \xC885\xC871\xC774 \xD568\xAED8 \xC11E\xC5EC \xC788\xB294, \xBCA0\xB978\xC758 \xC218\xB3C4\xC785\xB2C8\xB2E4.";
	}

	if (FAILED(Ready_Layer_Chrome()))
		return E_FAIL;

	m_pLoader = CLoader::Create(m_pDevice, m_pContext, m_eNextLevelID);
	if (nullptr == m_pLoader)
		return E_FAIL;

	/* The target class/encounter is already fixed at this point and the loader
	   worker is live. Register its priority Product targets now so the regular
	   main-frame preparation step overlaps the remaining level load. */
	if (LEVEL::CHARACTER_SELECT == m_eNextLevelID ||
		LEVEL::VALTAN_ARENA == m_eNextLevelID)
	{
		Advance_TargetEffectPreparation();
	}
	return S_OK;
}

void CLevel_Loading::Update(const f32_t fTimeDelta)
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

	bool_t bTargetPresentationReady = true;
	if ((LEVEL::CHARACTER_SELECT == m_eNextLevelID ||
		 LEVEL::VALTAN_ARENA == m_eNextLevelID) &&
		!m_isActivationRequested)
	{
		bTargetPresentationReady =
			Advance_TargetEffectPreparation();
	}

	/* The worker owns the first 90%. Selected Product targets prepare alongside
	   it and use the remaining 10% only when they outlive the worker. */
	f32_t fTargetProgress = m_pLoader->Finished() ? 1.f : 0.9f;
	if (m_pLoader->Finished() && !bTargetPresentationReady)
	{
		const uint32_t iSettledCount =
			m_iEffectPreparationPreparedCount +
			m_iEffectPreparationFailedCount;
		const f32_t fPreparationRatio =
			0u == m_iEffectPreparationTargetCount ? 0.f :
			static_cast<f32_t>(iSettledCount) /
			static_cast<f32_t>(m_iEffectPreparationTargetCount);
		fTargetProgress = 0.9f + 0.09f * (min)(1.f, fPreparationRatio);
	}
	m_fDisplayProgress += (fTargetProgress - m_fDisplayProgress) * (min)(1.f, fTimeDelta * 2.5f);

	const f32_t fTrackLeft = m_fProgressTrackX - m_fProgressTrackWidth * 0.5f;
	const f32_t fFillWidth = m_fProgressTrackWidth * m_fDisplayProgress;

	if (nullptr != m_pProgressFill)
		m_pProgressFill->Set_Rect(fTrackLeft + fFillWidth * 0.5f, m_fProgressTrackY,
			fFillWidth, m_fProgressTrackHeight);

	if (nullptr != m_pProgressGlow)
		m_pProgressGlow->Set_Rect(fTrackLeft + fFillWidth, m_fProgressTrackY,
			m_fProgressGlowWidth, m_fProgressGlowHeight);

	if (m_pLoader->Finished() && bTargetPresentationReady &&
		!m_isActivationRequested)
	{
		if (CLevelTransitionService::Request_Activation(
			m_eNextLevelID,
			"loading.complete",
			m_iLobbyCommandToken))
		{
			m_isActivationRequested = true;
			m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
		}
		else
		{
			OutputDebugStringA(
				"[Level_Loading] Activation request was rejected; retrying.\n");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (!m_strTitleText.empty())
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), m_strTitleText.c_str(),
			m_vTitlePos, Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.65f);

	if (!m_strTipText.empty())
	{
		/* "Scenario" label, as \x escapes for the same reason as the title/tip text above. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), L"\xC2DC\xB098\xB9AC\xC624",
			m_vScenarioPos, Colors::Gold, 0.f, float2_t(0.5f, 0.5f), 0.55f);

		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), m_strTipText.c_str(),
			m_vTipPos, Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.6f);
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport && nullptr != m_pLoader)
	{
		const std::string loadingStatus =
			(LEVEL::CHARACTER_SELECT == m_eNextLevelID ||
			 LEVEL::VALTAN_ARENA == m_eNextLevelID) &&
			m_pLoader->Finished() &&
			!m_strEffectPreparationStatus.empty() ?
			m_strEffectPreparationStatus : CLoader::Get_ActiveStatus();
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(
				viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
				viewport->WorkPos.y + 16.f),
			ImGuiCond_Always,
			ImVec2(0.5f, 0.f));
		ImGui::SetNextWindowBgAlpha(0.82f);
		if (ImGui::Begin(
			"Loading progress",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing))
		{
			ImGui::TextUnformatted(loadingStatus.c_str());
		}
		ImGui::End();
	}

	if (m_isFailureReported && LEVEL::LOBBY == m_eNextLevelID)
	{
		ImGui::SetNextWindowPos(ImVec2(24.f, 24.f), ImGuiCond_Always);
		if (ImGui::Begin(
			"Loading recovery",
			nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped(
				"Lobby resources could not be loaded. Partial resources were rolled back.");
			if (ImGui::Button("Retry Lobby"))
				m_isRetryRequested = true;
		}
		ImGui::End();
	}

#ifdef _DEBUG
	if (nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif
	return S_OK;
}

bool_t CLevel_Loading::Advance_TargetEffectPreparation()
{
	const bool_t bCharacterSelect =
		LEVEL::CHARACTER_SELECT == m_eNextLevelID;
	const bool_t bValtanArena = LEVEL::VALTAN_ARENA == m_eNextLevelID;
	if ((!bCharacterSelect && !bValtanArena) ||
		m_isEffectPreparationComplete)
	{
		return true;
	}
	const std::string TargetLabel =
		bCharacterSelect ? "CHARACTER SELECT" : "VALTAN ARENA";

	const auto IsolateFailure = [this, &TargetLabel](const std::string& Status)
	{
		m_isEffectPreparationRegistered = true;
		m_EffectPreparationTargets.clear();
		m_iEffectPreparationTargetCount = 0u;
		m_iEffectPreparationPendingCount = 0u;
		m_iEffectPreparationPreparedCount = 0u;
		m_iEffectPreparationFailedCount = 1u;
		m_strEffectPreparationRegistrationFailure = Status;
		m_strEffectPreparationStatus =
			TargetLabel + ": Effect preparation isolated: " + Status;
		OutputDebugStringA(("[Level_Loading] " +
			m_strEffectPreparationStatus + "\n").c_str());
		return false;
	};

	if (!m_isEffectPreparationRegistered)
	{
		std::string Status;
		using LostArk::Shared::CHARACTER_CLASS_ID;
		CHARACTER_CLASS_ID SelectedClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		if (!CCharacterSelectionState::Try_Get_SelectedClass(SelectedClass) ||
			!LostArk::Shared::Is_Supported_Playable_Character_Class(
				SelectedClass))
		{
			SelectedClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		}
		const CHARACTER_SPEC* pSpec =
			CCharacterCatalog::Find_Spec(SelectedClass);
		if (nullptr == pSpec || nullptr == pSpec->pAssetName)
		{
			return IsolateFailure(
				"selected class has no animation asset spec.");
		}
		ANIMATION_EFFECT_CUE_DOCUMENT PlayerCueDocument;
		std::vector<std::string> PlayerEffectAssetIds;
		if (!CAnimationEffectCueDocument::Load_ForProductPrewarm(
				pSpec->pAssetName, PlayerCueDocument, Status) ||
			!CEffectPresentationService::Queue_ProductCues_Priority(
				PlayerCueDocument.Cues, PlayerEffectAssetIds, Status))
		{
			return IsolateFailure(Status);
		}
		m_EffectPreparationTargets = std::move(PlayerEffectAssetIds);

		if (bValtanArena)
		{
			VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
			if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
					CueDocument, Status) ||
				CueDocument.Cues.size() != VALTAN_PATTERN_EFFECT_CUE_COUNT)
			{
				return IsolateFailure(Status);
			}
			const BOSS_ACTOR_ENTRY* pBossActor = CActorCatalog::Find_Boss(
				CueDocument.strOwnerArchetypeId);
			if (nullptr == pBossActor ||
				pBossActor->combatObjectVisuals.size() !=
					VALTAN_COMBAT_OBJECT_EFFECT_TARGET_COUNT)
			{
				return IsolateFailure(nullptr == pBossActor ?
					CActorCatalog::Get_Status() :
					"Valtan BossCatalog must declare exactly two combat-object visuals.");
			}
			std::vector<std::string> EffectAssetIds;
			EffectAssetIds.reserve(CueDocument.Cues.size() +
				pBossActor->combatObjectVisuals.size());
			for (const VALTAN_PATTERN_EFFECT_CUE& Cue : CueDocument.Cues)
				EffectAssetIds.push_back(Cue.strEffectAssetId);
			for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
				pBossActor->combatObjectVisuals)
			{
				EffectAssetIds.push_back(Visual.effectAssetId);
			}
			std::vector<std::string> BossEffectAssetIds;
			if (!CEffectPresentationService::Queue_ProductTargets_Priority(
					EffectAssetIds, BossEffectAssetIds, Status) ||
				BossEffectAssetIds.size() != VALTAN_PRODUCT_EFFECT_TARGET_COUNT)
			{
				return IsolateFailure(Status);
			}
			m_EffectPreparationTargets.insert(
				m_EffectPreparationTargets.end(),
				BossEffectAssetIds.begin(), BossEffectAssetIds.end());
			std::sort(m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end());
			m_EffectPreparationTargets.erase(std::unique(
				m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end()),
				m_EffectPreparationTargets.end());
		}

		m_isEffectPreparationRegistered = true;
		m_iEffectPreparationTargetCount = static_cast<uint32_t>(
			m_EffectPreparationTargets.size());
		m_strEffectPreparationStatus =
			TargetLabel + ": queued " +
			std::to_string(m_iEffectPreparationTargetCount) +
			" Product Effects for the target presentation.";
	}

	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE Probe =
		CEffectPresentationService::Get_ProductCuePreparationProbe(
			m_EffectPreparationTargets);
	m_iEffectPreparationTargetCount = Probe.iTargetCount;
	m_iEffectPreparationPendingCount = Probe.iPendingCount;
	m_iEffectPreparationPreparedCount = Probe.iPreparedCount;
	m_iEffectPreparationFailedCount =
		Probe.iFailedCount + Probe.iUnavailableCount +
		(m_strEffectPreparationRegistrationFailure.empty() ? 0u : 1u);
	const bool_t bActivationReady =
		Is_ProductPrewarmTargetActivationReady(
			Probe, !m_strEffectPreparationRegistrationFailure.empty());
	if (!bActivationReady)
	{
		if (!m_strEffectPreparationRegistrationFailure.empty())
		{
			m_strEffectPreparationStatus =
				TargetLabel + ": Effect preparation isolated; waiting for "
				"the current Product Effect catalog revision.";
		}
		else
		{
			m_strEffectPreparationStatus =
				TargetLabel + ": preparing Product Effects " +
				std::to_string(m_iEffectPreparationPreparedCount +
					m_iEffectPreparationFailedCount) + "/" +
				std::to_string(m_iEffectPreparationTargetCount) +
				" (selected pending " +
				std::to_string(Probe.iPendingCount) +
				", background pending " +
				std::to_string(Probe.iQueuePendingCount) + ").";
		}
		return false;
	}

	m_isEffectPreparationComplete = true;
	if (!m_strEffectPreparationRegistrationFailure.empty())
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": Effect preparation isolated: " +
			m_strEffectPreparationRegistrationFailure +
			" Continuing without blocking on unrelated Effect work.";
	}
	else if (0u != m_iEffectPreparationFailedCount)
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": Product Effect preparation settled with " +
			std::to_string(m_iEffectPreparationFailedCount) +
			" isolated failure(s); continuing.";
	}
	else
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": prepared " +
			std::to_string(m_iEffectPreparationPreparedCount) +
			" Product Effects for the target presentation.";
	}
	return true;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	CCharacterSelectionState::Cancel_PendingCreation();
	Cancel_LobbyCommand("target level loading failed");
	/* The loader's live progress line names the stage that refused, and it is
	the only record of it once the loading Level is torn down. */
	CLevelTransitionService::Report_LoadFailure(
		result, "[Loader] " + CLoader::Get_ActiveStatus());
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

void CLevel_Loading::Cancel_LobbyCommand(const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == m_iLobbyCommandToken)
		return;

	CLobbyCommandService::Cancel(m_iLobbyCommandToken, pReason);
	m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
}

void CLevel_Loading::Retry_LobbyLoad()
{
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"loading.recovery"))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to stage Lobby recovery.\n");
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

		/* Valtan Arena reuses every other chrome slot from the shared JSON as-is; only its
		Background art is swapped here instead of forking a second layout document for one
		texture path. */
		if ("Background" == strId && LEVEL::VALTAN_ARENA == m_eNextLevelID)
			strTexturePath = "UI/Loading/Loading_Background_Valtan.png";

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
			m_fProgressGlowWidth = fWidth;
			m_fProgressGlowHeight = fHeight;
		}
	}

	return S_OK;
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	auto instance = unique_ptr<CLevel_Loading>(
		new CLevel_Loading(pDevice, pContext));
	if (FAILED(instance->Initialize(
		eNextLevelID,
		lobbyCommandToken)))
		return nullptr;
	return instance;
}
