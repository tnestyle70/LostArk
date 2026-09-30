#include "Level_Development.h"

#include "Camera_Free.h"
#include "ArenaCameraProfile.h"
#include "Character.h"
#include "CharacterSelectionState.h"
#include "CombatHUDViewModel.h"
#include "ColosseumIntroCutscene.h"
#include "ColosseumMatchStart.h"
#include "EffectFailureDiagnostic.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#pragma push_macro("new")
#undef new
#include "imgui.h"
#pragma pop_macro("new")
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "MapLightPresentationRuntime.h"
#include "MapAssetCatalog.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "Profiler.h"
#include "Transform.h"
#include "MaharakaWaterpangPresentation.h"
#include "UILayoutRuntime.h"
#include "UILabelFont.h"
#include "WorldGameplayDocument.h"
#include "RaidEntryPreviewView.h"
#include "UITextOcclusion.h"

#ifdef _DEBUG
void Client::CLevel_Development::Set_MapAuthoringActive(bool_t active)
{
    if (active && m_Waterpang) m_Waterpang->Suspend_ForAuthoring();
    m_bMapAuthoringActive = active;
}
void Client::CLevel_Development::Set_WaterpangEffectAuthoringActive(bool_t active)
{
    if (active && m_Waterpang) m_Waterpang->Suspend_ForAuthoring();
    m_bWaterpangEffectAuthoringActive = active;
}
#endif
#include "InteractKeyPromptView.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

CLevel_Development* CLevel_Development::s_pActiveInstance = nullptr;

CLevel_Development::CLevel_Development(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
	: CLevel{ pDevice, pContext }
	, m_eLevel{ eLevel }
{
	s_pActiveInstance = this;
}

CLevel_Development::~CLevel_Development()
{
	Clear_TriggerMarkers();
	m_Waterpang.reset(); // Return poses while map and replication still exist.
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	if (m_isMapEditorWorkspace)
	{
		CCombatHUDViewModel::Get().Reset_RuntimeState();
		CMapEditorWorkspaceService::Cancel();
	}
#endif
}

HRESULT CLevel_Development::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

#ifdef _DEBUG
	m_isMapEditorWorkspace =
		LEVEL::DEVELOPMENT == m_eLevel &&
		CMapEditorWorkspaceService::Is_Requested();
	if (m_isMapEditorWorkspace)
	{
		auto previewClass =
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;
		CCharacterSelectionState::Try_Get_SelectedClass(previewClass);

		if (FAILED(Ready_Lights()) ||
			FAILED(Ready_Camera(TEXT("Layer_Camera"))) ||
			!CCombatHUDViewModel::Get().Initialize_Definitions() ||
			!CCombatHUDViewModel::Get().Apply_CharacterPreview(previewClass))
		{
			CCombatHUDViewModel::Get().Reset_RuntimeState();
			CMapEditorWorkspaceService::Cancel();
			return E_FAIL;
		}
		CMapEditorWorkspaceService::Set_Active(true);
		return S_OK;
	}
#endif

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(m_eLevel);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(m_eLevel),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_Development] " + m_MapRuntime.Get_Status() + "\n").c_str());
		Write_EffectFailureDiagnostic("map.area.load-failed",
			m_MapRuntime.Get_Status());
		return E_FAIL;
	}

	// Report what the Maharaka documents delivered: WATER assets and their water rows,
	// and assets that carry source material rows. Written once per level entry.
	if (LEVEL::MAHARAKA == m_eLevel)
	{
		const CMapAssetCatalog& catalog = m_MapRuntime.Get_Catalog();
		size_t waterAssets = 0u;
		size_t waterRows = 0u;
		size_t materialAssets = 0u;
		for (const MAP_ASSET_ENTRY& entry : catalog.Get_Entries())
		{
			if (MAP_ASSET_RENDER_MODE::WATER == entry.renderProfile.renderMode)
			{
				++waterAssets;
				if (nullptr != catalog.Find_Water(entry.id))
					++waterRows;
			}
			if (!entry.materialOverrides.empty())
				++materialAssets;
		}
		Write_EffectFailureDiagnostic("map.water.loaded",
			"area=" + catalog.Get_AreaId() +
			" assets=" + std::to_string(catalog.Get_Entries().size()) +
			" waterAssets=" + std::to_string(waterAssets) +
			" waterRows=" + std::to_string(waterRows) +
			" materialAssets=" + std::to_string(materialAssets));
	}

	// The island's EFActorMotion rows (turning and rocking props). An absent
	// document is not an error; a rejected one is reported and the island stays static.
	if (LEVEL::MAHARAKA == m_eLevel &&
		!m_MapRuntime.Load_SelfMotions(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Development] Self-motion document was rejected.\n");
	}

	if (FAILED(Ready_Lights()) ||
		FAILED(Ready_Camera(TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex = ETOUI(m_eLevel);
	replicationDesc.iLayerLevelIndex = ETOUI(m_eLevel);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	if (LEVEL::COLOSSEUM == m_eLevel)
	{
		// Match intro cutscene. A rejected document only skips it; the arena stays playable.
		auto intro = std::make_unique<CColosseumIntroCutscene>();
		// Kept even when the document was rejected, so the F1 section can retry it.
		(void)intro->Initialize(m_pDevice, m_pContext, ETOUI(m_eLevel), m_pCamera.lock());
		m_ColosseumIntro = std::move(intro);
		// Countdown banner + gate. Also kept when its document was rejected, so F1 can retry it.
		auto matchStart = std::make_unique<CColosseumMatchStart>();
		(void)matchStart->Initialize(m_pDevice, m_pContext, ETOUI(m_eLevel), &m_MapRuntime);
		m_ColosseumMatchStart = std::move(matchStart);
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (m_eLevel == LEVEL::MAHARAKA || m_eLevel == LEVEL::COLOSSEUM)
	{
		m_PartyInteraction.Initialize(m_pDevice, m_pContext, ETOUI(m_eLevel));
		m_ChatBubbleView.Initialize(m_pDevice, m_pContext, ETOUI(m_eLevel));
		if (m_eLevel == LEVEL::MAHARAKA)
			m_TravelEntryView = std::make_unique<CRaidEntryPreviewView>(m_pDevice, m_pContext, ETOUI(m_eLevel));
	}
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	if (!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(m_eLevel)))
	{
		return E_FAIL;
	}
	if (m_eLevel == LEVEL::MAHARAKA)
	{
		m_InteractPrompt = std::make_unique<CInteractKeyPromptView>();
		m_InteractPrompt->Initialize(m_pDevice,m_pContext,ETOUI(m_eLevel),pEntry->pMapAreaId);
		(void)Load_TriggerMarkers(pEntry->pMapAreaId);
		m_Waterpang = std::make_unique<CMaharakaWaterpangPresentation>();
		CWorldSequencePlayer::TARGET_SET targets;
		targets.levelIndex=ETOUI(m_eLevel); targets.pCatalog=&m_MapRuntime.Get_Catalog();
		targets.pPlacements=&m_MapRuntime.Get_MutablePlacements(); targets.pDeployRuntime=&m_WaterpangDeploy;
		targets.device=m_pDevice; targets.context=m_pContext;
		targets.previewNpc=[this](const std::string& id) { return m_Replication.Find_NpcPlacement(id); };
		if (!m_Waterpang->Initialize(targets,m_pCamera.lock()))
		{
			Write_EffectFailureDiagnostic("maharaka.waterpang.prepare",m_Waterpang->Get_Status());
			m_Waterpang.reset();
		}
	}
	return S_OK;
}

void CLevel_Development::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_isMapEditorWorkspace)
		return;

	/* Maharaka and Colosseum leave through the existing Server-approved world transfer. */
	if ((LEVEL::MAHARAKA == m_eLevel || LEVEL::COLOSSEUM == m_eLevel) &&
		SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
			CLevelTransitionService::Pump_ServerApprovedWorldTransfer(m_eLevel))
		return;

	if (LEVEL::MAHARAKA == m_eLevel)
	{
#ifdef _DEBUG
		// MapTool owns sampled poses while editing; keep replication and lights live.
		if (!m_bMapAuthoringActive)
#endif
		m_MapRuntime.Update_SelfMotions(fTimeDelta);
		Update_TriggerMarkerClocks(fTimeDelta);
		if (m_pMapLightPresentation &&
			!m_pMapLightPresentation->Submit_Frame() &&
			!m_bMapLightSubmissionFailureReported)
		{
			m_bMapLightSubmissionFailureReported = true;
			OutputDebugStringA(("[Level_Development][MapLight] " +
				m_pMapLightPresentation->Get_Status() + "\n").c_str());
		}
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Development] Failed to apply training replication.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-development.network-connection-lost",
			"Training replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_Development] Lobby recovery request was rejected; retrying.\n");
	}

	Bind_CameraToLocalCharacter();
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	bool editing = false;
#ifdef _DEBUG
	editing = m_bMapAuthoringActive || m_bWaterpangEffectAuthoringActive;
#endif
	if (m_eLevel == LEVEL::MAHARAKA || m_eLevel == LEVEL::COLOSSEUM)
	{
		m_Replication.Collect_PlayerViews(m_NameplatePlayers);
		if (m_TravelEntryView)
		{
			LostArk::Shared::S2C_RAID_ENTRY_PROMPT prompt{};
			while (m_Replication.Try_Consume_RaidEntryPrompt(prompt))
				m_TravelEntryView->Open_VoteConfirm(prompt.iProposalId, prompt.eTarget);
			LostArk::Shared::S2C_RAID_ENTRY_VOTE vote{};
			while (m_Replication.Try_Consume_RaidEntryVote(vote))
				if (vote.bClosed) m_TravelEntryView->Close_VoteConfirm();
		}
		m_PartyInteraction.Register_TextOccluders();
		if (Is_TravelModalOpen())
		{
			const auto viewport = CGameInstance::Get().Get_ViewportSize();
			CUITextOcclusion::Get().Add_Occluder(UI_TEXT_LAYER::MODAL, 0.f, 0.f, viewport.x, viewport.y);
		}
		if (m_PartyInteraction.Update(m_Replication, m_pPlayerCommandSink, m_NameplatePlayers,
			!editing && !Is_TravelModalOpen() && !Is_ColosseumIntroActive() && camera &&
			camera->Is_FollowEnabled() && !camera->Is_PresentationOverrideActive()) || Is_TravelModalOpen())
		{
			CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
			CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
		}
	}
	if (m_Waterpang)
	{
		for (const auto& play:m_Replication.Consume_WorldSequencePlays()) m_Waterpang->Accept(play);
		m_Waterpang->Update(fTimeDelta,m_Replication.Get_LastServerTick(),editing);
	}
	if (m_InteractPrompt)
		m_InteractPrompt->Update(fTimeDelta,localCharacter,CCombatHUDViewModel::Get().Get_InteractPromptTriggerId(),
			!editing && camera && !camera->Is_PresentationOverrideActive());
	if (m_ColosseumIntro)
		m_ColosseumIntro->Update(fTimeDelta, m_Replication);
	if (m_ColosseumMatchStart)
	{
		// Recruitment keeps the gate closed until the Server admits combat;
		// replaying the cutscene from F1 only restarts presentation.
		const bool_t bIntroActive = Is_ColosseumIntroActive();
        const auto& match = m_Replication.Get_ColosseumMatchState();
        if (match.iMatchId && !m_bColosseumMatchAuthority)
        {
            m_bColosseumMatchAuthority = true;
            m_bColosseumMatchStartArmed = true;
            m_ColosseumMatchStart->Reset();
        }
        const bool admitted = !m_bColosseumMatchAuthority ||
            match.ePhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::ACTIVE ||
            match.ePhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED;
        if (bIntroActive && !m_bColosseumIntroWasActive)
            m_ColosseumMatchStart->Reset();
        else if (!bIntroActive && admitted && (m_bColosseumMatchStartArmed || m_bColosseumIntroWasActive))
        {
            if (m_bColosseumMatchAuthority) m_ColosseumMatchStart->Begin_ApprovedMatch();
            else m_ColosseumMatchStart->Begin(); // Preserve the direct Debug preview countdown.
            if (match.ePhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED)
                m_ColosseumMatchStart->Seek(m_ColosseumMatchStart->Get_TimelineMs());
            m_bColosseumMatchStartArmed = false;
        }
        m_bColosseumIntroWasActive = bIntroActive;
		m_ColosseumMatchStart->Update(fTimeDelta);
	}
	m_PlayerController.Update(
		nullptr != camera && camera->Is_FollowEnabled() &&
		!Is_ColosseumIntroActive() && !Is_TravelModalOpen() &&
		(LEVEL::MAHARAKA != m_eLevel || !camera->Is_PresentationOverrideActive()));
}

bool_t CLevel_Development::Is_ColosseumIntroActive() const
{
	return nullptr != m_ColosseumIntro && m_ColosseumIntro->Is_Active();
}

void Client::CLevel_Development::Render_ColosseumIntroControls()
{
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::COLOSSEUM))
		return;
	ImGui::PushID("ColosseumIntroCutscene");
	if (ImGui::CollapsingHeader("Colosseum Intro Cutscene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		CLevel_Development* const level = Get_Active(LEVEL::COLOSSEUM);
		if (nullptr == level || nullptr == level->m_ColosseumIntro)
		{
			ImGui::TextDisabled("The Colosseum intro is not loaded.");
		}
		else
		{
			CColosseumIntroCutscene& intro = *level->m_ColosseumIntro;
			if (ImGui::Button(intro.Is_Active() ? "Restart" : "Play"))
				(void)intro.Restart();
			ImGui::SameLine();
			ImGui::BeginDisabled(!intro.Is_Playing());
			if (ImGui::Button(intro.Is_Paused() ? "Resume" : "Pause"))
				intro.Set_Paused(!intro.Is_Paused());
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!intro.Is_Active());
			if (ImGui::Button("Stop"))
				intro.Stop();
			ImGui::EndDisabled();
			if (intro.Is_Playing())
			{
				f32_t fSeconds = intro.Get_ClockMs() * 0.001f;
				if (ImGui::SliderFloat("Time (s)", &fSeconds, 0.f, intro.Get_DurationMs() * 0.001f, "%.2f"))
					intro.Seek(fSeconds * 1000.f);
			}
			ImGui::Text("State: %s | FOV x %.1f | lineup actors %u", intro.Get_PhaseLabel(),
				intro.Get_FovXDegrees(), static_cast<unsigned>(intro.Get_ActorCount()));
			if (!intro.Get_Status().empty())
				ImGui::TextWrapped("%s", intro.Get_Status().c_str());
			ImGui::TextWrapped("Play re-reads Data/Camera/ColosseumIntro.cutscene.json, so edit the camera, FOV or timing there and press Play again (no rebuild). Only players connected to this room stand in the six slots. Input and HUD stay blocked while it runs; press Stop to leave early.");
		}
	}
	if (ImGui::CollapsingHeader("Colosseum Match Start (countdown + gate)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		CLevel_Development* const level = Get_Active(LEVEL::COLOSSEUM);
		if (nullptr == level || nullptr == level->m_ColosseumMatchStart)
		{
			ImGui::TextDisabled("The Colosseum match start is not loaded.");
		}
		else
		{
			CColosseumMatchStart& start = *level->m_ColosseumMatchStart;
			ImGui::BeginDisabled(level->Is_ColosseumIntroActive());
			if (ImGui::Button("Play Countdown + Gate"))
				(void)start.Restart();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!start.Is_Running());
			if (ImGui::Button(start.Is_Paused() ? "Resume##matchstart" : "Pause##matchstart"))
				start.Set_Paused(!start.Is_Paused());
			ImGui::SameLine();
			if (ImGui::Button("Stop / close gate##matchstart"))
				start.Reset();
			ImGui::EndDisabled();
			if (start.Is_Running())
			{
				f32_t fSeconds = start.Get_ClockMs() * 0.001f;
				if (ImGui::SliderFloat("Match time (s)", &fSeconds, 0.f, start.Get_TimelineMs() * 0.001f, "%.2f"))
					start.Seek(fSeconds * 1000.f);
			}
			ImGui::Text("State: %s | count %u | gate sunk %.2f m (%u leaves)", start.Get_PhaseLabel(),
				static_cast<unsigned>(start.Get_CountDigit()), start.Get_GateSinkMeters(),
				static_cast<unsigned>(start.Get_GateCount()));
			if (!start.Get_Status().empty())
				ImGui::TextWrapped("%s", start.Get_Status().c_str());
			ImGui::TextWrapped("Play re-reads Data/Camera/ColosseumMatchStart.json (count, bar, colours, text positions, gate timing and depth), so edit it and press Play again (no rebuild). The product start beat follows the Server ACTIVE phase after mercenary recruitment. This replay controls presentation only; Server combat authority is unchanged.");
		}
	}
	ImGui::PopID();
}

bool_t CLevel_Development::Load_TriggerMarkers(const char* pAreaId)
{
	if (nullptr == pAreaId)
		return false;
	/* The Server publishes this document from the same Gameplay.world.json, and the interact
	   prompt reads it the same way: the marker sits on the exact trigger box centre, nothing
	   is copied or guessed. */
	CWorldGameplayDocument world;
	std::string strStatus;
	const std::filesystem::path path = CMapAssetCatalog::Get_MapDataRoot().parent_path() /
		"World" / (std::string(pAreaId) + ".viewer.world.json");
	if (!world.Load(path, pAreaId, strStatus))
	{
		OutputDebugStringA(("[MaharakaTriggerMarker] " + strStatus + "\n").c_str());
		return false;
	}
	std::vector<TRIGGER_MARKER> staged;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : world.Get_Placements())
	{
		/* The box the player steps on to go somewhere: jump1/jump2/jump3. The disabled
		   arrival boxes (jump*_1) have no event, and the Bern exit and the match start are
		   changeLevel / playSequence boxes, so none of those show the marker. */
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind || !placement.isEnabled ||
			1u != placement.triggerEvents.size() ||
			WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER != placement.triggerEvents.front().eKind)
			continue;
		TRIGGER_MARKER marker;
		marker.placementId = placement.placementId;
		XMStoreFloat4x4(&marker.rootWorld, XMMatrixTranslation(
			placement.position.x, placement.position.y, placement.position.z));
		staged.push_back(std::move(marker));
	}
	Clear_TriggerMarkers();
	m_TriggerMarkers = std::move(staged);
	return true;
}

void CLevel_Development::Clear_TriggerMarkers()
{
	for (auto& marker : m_TriggerMarkers)
	{
		EFFECT_WORLD_ROOT_HANDLE handle;
		handle.iValue = marker.iHandleValue;
		CEffectPresentationService::Stop_WorldRoot(handle);
	}
	m_TriggerMarkers.clear();
}

void CLevel_Development::Update_TriggerMarkerClocks(const f32_t deltaSeconds)
{
	if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
		return;
	for (auto& marker : m_TriggerMarkers)
	{
		if (marker.retired)
			continue;
		if (marker.clockStarted)
			marker.seconds = std::fmod(marker.seconds + deltaSeconds, 7.f);
		marker.clockStarted = true;
	}
}

void CLevel_Development::Submit_TriggerMarkers()
{
	if (LEVEL::MAHARAKA != m_eLevel ||
		CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::MAHARAKA))
		return;
	for (auto& marker : m_TriggerMarkers)
	{
		if (marker.retired) continue;
		EFFECT_WORLD_ROOT_HANDLE handle;
		handle.iValue = marker.iHandleValue;
		// The complete fixed marker footprint stays inside the existing 8m sphere.
		const float3_t center{ marker.rootWorld._41, marker.rootWorld._42, marker.rootWorld._43 };
		const bool_t visible = CEffectPresentationService::Is_WorldPresentationVisible(
			center, 8.f, marker.active);
		if (!visible)
		{
			if (marker.active && handle.Is_Valid())
				(void)CEffectPresentationService::Submit_LevelPlacementSample(handle, false);
			marker.active = false;
			continue;
		}
		const bool_t firstSample = !marker.started;
		if (firstSample)
		{
			// Reuse the Loader's prepared target and commit only this marker.
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
			desc.iLevelIndex = ETOUI(LEVEL::MAHARAKA);
			desc.strPlacementId = "maharaka.trigger." + marker.placementId;
			desc.strEffectAssetId = "effect.world.move_destination";
			desc.RootWorld = marker.rootWorld;
			desc.bExternallySampled = true;
			std::string strStatus;
			if (!CEffectPresentationService::Spawn_LevelPlacement(desc, handle, strStatus))
			{
				marker.retired = true;
				OutputDebugStringA(("[MaharakaTriggerMarker] " + marker.placementId +
					": " + strStatus + "\n").c_str());
				continue;
			}
			marker.iHandleValue = handle.iValue;
			CEffectPresentationService::Commit_PendingWorldRootSpawns({ handle });
			marker.started = true;
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
			[root = marker.rootWorld](f32_t, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& sample,
				std::string& status)
			{
				sample.RootWorld = root;
				sample.SourceAnchorWorlds.clear();
				status.clear();
				return true;
			};
		if (auto* profiler = CGameInstance::Get().Get_Profiler())
		{
			profiler->Add_Counter(EProfilerCounter::EffectMarkerSamples);
			if (firstSample || !marker.active)
				profiler->Add_Counter(EProfilerCounter::EffectMarkerHistoryRequests);
		}
		const bool_t sampled = CEffectPresentationService::Seek_WorldRoot(handle,
			marker.seconds, provider, firstSample || !marker.active);
		const HRESULT submitted = sampled ?
			CEffectPresentationService::Submit_LevelPlacementSample(handle, true) : E_FAIL;
		if (S_OK != submitted)
		{
			CEffectPresentationService::Stop_WorldRoot(handle);
			marker.iHandleValue = 0u;
			marker.retired = true;
			marker.active = false;
			OutputDebugStringA(("[MaharakaTriggerMarker] Sample/submission failed: " +
				marker.placementId + ": " + CEffectPresentationService::Get_Status() + "\n").c_str());
			continue;
		}
		marker.active = true;
	}
}

HRESULT CLevel_Development::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	if (m_eLevel == LEVEL::MAHARAKA || m_eLevel == LEVEL::COLOSSEUM)
	{
		m_PlayerNameplateView.Render(m_NameplatePlayers, &m_Replication.Get_PartyRoster(),
            m_eLevel == LEVEL::COLOSSEUM ? &m_Replication.Get_ColosseumMatchState() : nullptr,
            m_eLevel == LEVEL::COLOSSEUM ? &m_Replication.Get_PlayerHealth() : nullptr);
		m_ChatBubbleView.Render(m_Replication, m_NameplatePlayers);
		m_PartyInteraction.Render(m_pPlayerCommandSink);
	}
	if (m_InteractPrompt) m_InteractPrompt->Render_Text();
	if (m_Waterpang) m_Waterpang->Render();
	if (m_ColosseumIntro) m_ColosseumIntro->Render();
	if (m_ColosseumMatchStart) m_ColosseumMatchStart->Render();

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(m_isMapEditorWorkspace ?
		TEXT("LostArk Map Editor Workspace") :
		LEVEL::MAHARAKA == m_eLevel ?
		TEXT("LostArk Maharaka Paradise") :
		TEXT("LostArk Test Training Ground"));
#endif
	return S_OK;
}

void CLevel_Development::Render_PartyInviteText()
{
	m_PartyInteraction.Render_InvitePopupText();
	m_PartyInteraction.Render_ContextMenuText();
    if (m_eLevel == LEVEL::COLOSSEUM && !Is_ColosseumIntroActive() &&
        m_Replication.Get_ColosseumMatchState().iMatchId != 0u)
    {
        using namespace LostArk::Shared;
        const auto& match = m_Replication.Get_ColosseumMatchState();
        const auto local = CNetworkManager::Get().Get_LocalEntityId();
        std::uint8_t ownTeam = COLOSSEUM_NO_TEAM;
        std::uint32_t count[2]{};
        for (const auto& row : match.Players)
        {
            if (row.iNetEntityId == local) ownTeam = row.iTeam;
            if (row.iTeam < 2u && row.bParticipant) ++count[row.iTeam];
        }
        std::wstring text;
        if (match.ePhase == COLOSSEUM_MATCH_PHASE::RECRUITING)
            text = L"\uC6A9\uBCD1 \uBAA8\uC9D1 \u00B7 1\uD300 " + std::to_wstring(count[0]) + L"/4 \u00B7 2\uD300 " + std::to_wstring(count[1]) +
                L"/4  |  \uAC19\uC740 \uD300 \uD6C4\uBCF4\uB97C \uC6B0\uD074\uB9AD\uD558\uC5EC \uC6A9\uBCD1 2\uBA85\uC744 \uCD08\uB300\uD558\uC138\uC694";
        else if (match.ePhase == COLOSSEUM_MATCH_PHASE::FINISHED)
            text = match.iWinnerTeam == COLOSSEUM_NO_TEAM ? L"\uBB34\uC2B9\uBD80" :
                match.iWinnerTeam == ownTeam ? L"\uC2B9\uB9AC" : L"\uD328\uBC30";
        else text = L"\uD300 " + std::to_wstring(static_cast<unsigned>(ownTeam) + 1u) + L" \u00B7 4 \uB300 4 \uB300\uC804";
        const auto viewport = CGameInstance::Get().Get_ViewportSize();
        CUITextLayerScope hudText(UI_TEXT_LAYER::HUD);
        (void)UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), text.c_str(),
            viewport.x * .5f, viewport.y * .13f, viewport.y / 1080.f * 22.f,
            XMVectorSet(1.f, .92f, .7f, 1.f), viewport.x * .9f);
    }
}

bool_t CLevel_Development::Is_TravelModalOpen() const
{
	return m_TravelEntryView && m_TravelEntryView->Is_Open();
}

void CLevel_Development::Render_TravelModalText()
{
	if (m_TravelEntryView) m_TravelEntryView->RenderText();
}

void CLevel_Development::Render_TravelModal()
{
	if (!m_TravelEntryView || !m_pPlayerCommandSink) return;
	m_TravelEntryView->Render();
	const auto intent = m_TravelEntryView->Consume_Intent();
	if (intent.eKind == CRaidEntryPreviewView::RAID_ENTRY_INTENT::RESPOND)
		m_pPlayerCommandSink->Request_RaidEntryRespond(m_iNextTravelVoteSequence++, intent.iProposalId, intent.bAccepted);
}

HRESULT CLevel_Development::Ready_Lights()
{
	if (LEVEL::MAHARAKA != m_eLevel)
		return S_OK;

	// The island's Area declares a light pair, so a missing or rejected document
	// fails the load like Character Select does instead of showing an unlit island.
	const CLIENT_LEVEL_DESCRIPTOR* pEntry = CLevelRegistry::Find(m_eLevel);
	auto staged = make_shared<CMapLightPresentationRuntime>();
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!staged->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_Development][MapLight] " +
			staged->Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	m_pMapLightPresentation = std::move(staged);
	m_bMapLightSubmissionFailureReported = false;
	return S_OK;
}

HRESULT CLevel_Development::Ready_Camera(
	const wstring_t& strLayerTag)
{
	// Training/Maharaka share the saved Character Select sizes, independent
	// of whichever arena last wrote the process presentation profile.
	ARENA_CAMERA_PROFILE sizeProfile = CArenaCameraProfile::Default(ARENA_CAMERA_MAP::CHARACTER_SELECT);
	std::string sizeStatus;
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::CHARACTER_SELECT, sizeProfile, sizeStatus))
		OutputDebugStringA(("[Level_Development][CharacterSize] " + sizeStatus + "\n").c_str());
	CCharacter::Set_MapPresentationSizeProfile(sizeProfile);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	cameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 4000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(m_eLevel),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(m_eLevel),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(m_eLevel),
			strLayerTag,
			gameObject);
		return E_FAIL;
	}

	m_pCamera = camera;
	return S_OK;
}

bool_t CLevel_Development::Bind_CameraToLocalCharacter()
{
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr == camera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		camera->Set_FollowTarget(nullptr);
		camera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;
	camera->Set_PositionOffset(float3_t(0.4f, 7.5f, 4.5f));
	camera->Set_FollowTarget(transform);
	camera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_Development> CLevel_Development::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eLevel)
{
	auto instance = unique_ptr<CLevel_Development>(
		new CLevel_Development(pDevice, pContext, eLevel));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
