/* imgui.h defines its own placement-new helper and has to precede the project
headers, which is the same order Level_CharacterSelect.cpp uses. */
#ifdef _DEBUG
#include "imgui.h"
#endif

#include "Level_ValtanArena.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "Transform.h"

#include <algorithm>
#include <filesystem>

namespace
{
	std::filesystem::path Find_ValtanWorldDataFile(
		const std::filesystem::path& fileName)
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};

		const std::filesystem::path moduleDirectory =
			std::filesystem::path(modulePath).parent_path();
		const std::filesystem::path adjacent = moduleDirectory /
			L"DataFiles" / L"World" / fileName;
		if (std::filesystem::is_regular_file(adjacent))
			return adjacent;

		const std::filesystem::path parent = moduleDirectory.parent_path() /
			L"DataFiles" / L"World" / fileName;
		return std::filesystem::is_regular_file(parent) ? parent : adjacent;
	}
}

CLevel_ValtanArena::CLevel_ValtanArena(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_ValtanArena::~CLevel_ValtanArena()
{
	End_CinematicCamera();
	m_Replication.Reset();
	m_WorldDestructionDebrisPresentationRuntime.Clear();
	m_WorldDestructionDebrisPresentationDocument.Clear();
	m_WorldDestructionProjectionDocument.Clear();
}

HRESULT CLevel_ValtanArena::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA(("[Level_ValtanArena] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	if (!m_DeployRuntime.Load_Area(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_ValtanArena][DeployProp] " +
			m_DeployRuntime.Get_Status() + "\n").c_str());
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	std::string destructionStatus;
	if (!m_WorldDestructionProjectionDocument.Load(
		Find_ValtanWorldDataFile(
			L"LV_LUT_HEARTRB_ED.worlddestruction.json"), destructionStatus) ||
		m_WorldDestructionProjectionDocument.Get_AreaId() !=
			pEntry->pMapAreaId)
	{
		OutputDebugStringA(("[Level_ValtanArena][WorldDestruction] " +
			destructionStatus + "\n").c_str());
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	if (!m_WorldDestructionDebrisPresentationDocument.Load(
		Find_ValtanWorldDataFile(
			L"LV_LUT_HEARTRB_ED.worlddestructionpresentation.json"),
		destructionStatus) ||
		!m_WorldDestructionDebrisPresentationDocument.Validate_Against(
			m_WorldDestructionProjectionDocument, destructionStatus) ||
		m_WorldDestructionDebrisPresentationDocument.Get_AreaId() !=
			pEntry->pMapAreaId)
	{
		OutputDebugStringA(("[Level_ValtanArena][DestructionDebris] " +
			destructionStatus + "\n").c_str());
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	if (!m_WorldDestructionDebrisPresentationRuntime.Initialize(
		ETOUI(LEVEL::VALTAN_ARENA), m_pDevice, m_pContext,
		m_DeployRuntime, destructionStatus))
	{
		OutputDebugStringA(("[Level_ValtanArena][DestructionDebris] " +
			destructionStatus + "\n").c_str());
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	auto mapLightPresentation = make_shared<CMapLightPresentationRuntime>();
	if (!mapLightPresentation->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_ValtanArena][MapLight] " +
			mapLightPresentation->Get_Status() + "\n").c_str());
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return E_FAIL;
	}
	m_pMapLightPresentation = std::move(mapLightPresentation);
	if (!Ready_CinematicCamera())
	{
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return E_FAIL;
	}

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
	{
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return E_FAIL;
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::VALTAN_ARENA);
	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::VALTAN_ARENA);
	replicationDesc.strPlayerLayerTag =
		TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag =
		TEXT("Layer_WorldEntity");
	replicationDesc.pDeployPropRuntime = &m_DeployRuntime;
	replicationDesc.pWorldDestructionProjection =
		&m_WorldDestructionProjectionDocument;
	if (!m_Replication.Initialize(replicationDesc))
	{
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);

	return S_OK;
}

void CLevel_ValtanArena::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::VALTAN_ARENA))
	{
		End_CinematicCamera();
		return;
	}
	if (nullptr != m_pMapLightPresentation &&
		!m_pMapLightPresentation->Submit_Frame() &&
		!m_bMapLightSubmissionFailureReported)
	{
		m_bMapLightSubmissionFailureReported = true;
		OutputDebugStringA(("[Level_ValtanArena][MapLight] " +
			m_pMapLightPresentation->Get_Status() + "\n").c_str());
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_ValtanArena] Failed to apply replication event.\n");
		if (m_Replication.Has_FatalWorldDestructionFailure())
		{
			End_CinematicCamera();
			CNetworkManager::Get().Close_ServerConnection();
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"world-destruction-projection-failed"))
			{
				OutputDebugStringA(
					"[Level_ValtanArena] World destruction recovery request was rejected; retrying.\n");
			}
			return;
		}
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		End_CinematicCamera();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_ValtanArena] Lobby recovery request was rejected; retrying.\n");
	}

#ifdef _DEBUG
	Render_AuditionPanel();
#endif
	Update_WorldDestructionPresentation(fTimeDelta);
	Bind_CameraToLocalCharacter();
	Update_CinematicCamera(fTimeDelta);
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() &&
		!m_bCinematicCameraApplied);
}

#ifdef _DEBUG
namespace
{
	/* The bars this build can audition are exactly the encounter's authored
	HEALTH_BAR thresholds, read from the same document the Server publishes
	from. Nothing here is a second list to keep in step. */
	std::vector<uint32_t> Collect_AuditionHealthBars(
		const Client::CEncounterPatternReference& reference)
	{
		std::vector<uint32_t> bars;
		for (const Client::ENCOUNTER_PATTERN_REFERENCE& pattern :
			reference.Get_Patterns())
		{
			if ("HEALTH_BAR" != pattern.selectionMode ||
				0u == pattern.iTriggerHealthBar)
			{
				continue;
			}
			bars.push_back(pattern.iTriggerHealthBar);
		}
		std::sort(bars.begin(), bars.end(), std::greater<uint32_t>{});
		bars.erase(std::unique(bars.begin(), bars.end()), bars.end());
		return bars;
	}

	const char_t* Describe_AuditionResult(
		const LostArk::Shared::VALTAN_AUDITION_RESULT result)
	{
		using LostArk::Shared::VALTAN_AUDITION_RESULT;
		switch (result)
		{
		case VALTAN_AUDITION_RESULT::ARMED:
			return "Armed one bar above the target. Press Cross to play it.";
		case VALTAN_AUDITION_RESULT::QUEUED:
			return "Reset complete. The Server will start the selected real pattern on its next tick.";
		case VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED:
			return "Already handled that request; the boss did not move.";
		case VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD:
			return "Release Server: auditions are Debug-only.";
		case VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD:
			return "This session is not a member of the Valtan room.";
		case VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS:
			return "No Valtan is active. Enter the encounter trigger first.";
		case VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD:
			return "Valtan is dead. Leave and re-enter to reset the room.";
		case VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR:
			return "That bar carries no authored pattern.";
		case VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE:
			return "That pattern already fired, or one is still running.";
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED:
			return "Arm the same bar before crossing it.";
		case VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			return "Move into engage range and act once, then retry.";
		default:
			return "Unknown audition verdict.";
		}
	}
}

void CLevel_ValtanArena::Submit_Audition(
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation)
{
	const std::vector<uint32_t> bars =
		Collect_AuditionHealthBars(m_ValtanEncounterReference);
	if (m_iSelectedAuditionBarIndex >= bars.size())
	{
		m_strAuditionStatus = "No authored health-bar pattern is selected.";
		return;
	}

	const uint32_t sequence = m_iNextAuditionRequestSequence;
	if (!CNetworkManager::Get().Send_ValtanAudition(
		sequence, operation, bars[m_iSelectedAuditionBarIndex]))
	{
		m_strAuditionStatus = "Could not send the audition request.";
		return;
	}
	++m_iNextAuditionRequestSequence;
	m_iPendingAuditionRequestSequence = sequence;
	m_strAuditionStatus = "Waiting for the Server verdict...";
}

void CLevel_ValtanArena::Render_AuditionPanel()
{
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT result{};
	while (CNetworkManager::Get().Try_Consume_ValtanAuditionResult(result))
	{
		if (result.iRequestSequence != m_iPendingAuditionRequestSequence)
			continue;
		m_iPendingAuditionRequestSequence = 0u;
		m_strAuditionStatus = Describe_AuditionResult(result.eResult);
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_FirstUseEver);
	}
	if (!ImGui::Begin(
		"Valtan Pattern Audition",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	const std::vector<uint32_t> bars =
		Collect_AuditionHealthBars(m_ValtanEncounterReference);
	if (bars.empty())
	{
		ImGui::TextDisabled(
			"No authored health-bar patterns were loaded for this encounter.");
		ImGui::End();
		return;
	}
	if (m_iSelectedAuditionBarIndex >= bars.size())
		m_iSelectedAuditionBarIndex = 0u;

	ImGui::TextUnformatted(
		"Server-authoritative repeatable audition.");
	ImGui::TextUnformatted(
		"Each Play resets boss + walls + collision/nav, then runs the real pattern.");
	ImGui::TextColored(
		ImVec4(1.f, 0.55f, 0.25f, 1.f),
		"First entry only: walk through the Debug stage triggers to Stage_Boss. No monster kill is required.");
	ImGui::TextDisabled(
		"Stage_Boss places you at the authored bait point; then either Reset + Play button is repeatable.");
	ImGui::TextColored(
		ImVec4(1.f, 0.82f, 0.2f, 1.f),
		"109: TAKEOFF -> DROP -> IMPACT -> full arena wall collapse.");
	ImGui::TextDisabled(
		"The 13 interior groups and the authored 8-sector outer ring break together;");
	ImGui::TextDisabled(
		"collision and nav blockers open on the persistent commit, not on the cue.");
	ImGui::TextColored(
		ImVec4(0.4f, 0.85f, 1.f, 1.f),
		"159: body charge -> receiver impact -> collision/nav passage opens.");
	ImGui::TextDisabled(
		"No jump clip exists in this model, so the Server owns the 109 leap as an authored arc.");

	ImGui::SeparatorText("Live authoritative state");
	const VALTAN_PRESENTATION_STATE& boss =
		m_Replication.Get_ValtanPresentationState();
	if (boss.isValid)
	{
		ImGui::Text("Server tick: %u  Sequence: %u  Stage index: %u",
			boss.iServerTick,
			boss.iPatternSequence,
			boss.iPatternStageIndex);
		ImGui::TextWrapped("Pattern: %s",
			boss.strPatternId.empty() ? "(idle)" : boss.strPatternId.c_str());
		ImGui::TextWrapped("Action: %s",
			boss.strActionId.empty() ? "(idle)" : boss.strActionId.c_str());
	}
	else
	{
		ImGui::TextDisabled("Waiting for the authoritative Valtan snapshot...");
	}

	size_t intactCount = 0u;
	size_t breakingCount = 0u;
	size_t fracturedCount = 0u;
	size_t despawnedCount = 0u;
	for (const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& group :
		m_Replication.Get_WorldDestructionGroupStates())
	{
		switch (group.eState)
		{
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT:
			++intactCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING:
			++breakingCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED:
			++fracturedCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED:
			++despawnedCount;
			break;
		default:
			break;
		}
	}
	ImGui::Text(
		"Walls: INTACT %zu | BREAKING %zu | FRACTURED %zu | GONE %zu",
		intactCount,
		breakingCount,
		fracturedCount,
		despawnedCount);
	ImGui::Text("Destruction sync: %s  Epoch: %u  Debris actors: %u/%u",
		m_Replication.Is_WorldDestructionSynchronized() ? "READY" : "WAITING",
		m_Replication.Get_WorldDestructionEncounterEpoch(),
		m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
		CWorldDestructionDebrisPresentationRuntime::MAX_ACTIVE_ACTORS);
	const bool_t isBusy = 0u != m_iPendingAuditionRequestSequence;
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Reset + Play 109 (Arena Collapse)", ImVec2(260.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 109u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	if (ImGui::Button("Reset + Play 159 (Impact/Nav)", ImVec2(260.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 159u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	ImGui::EndDisabled();
	ImGui::SeparatorText("Advanced authored health bar");
	ImGui::SeparatorText("Authored health bar");
	for (size_t index = 0; index < bars.size(); ++index)
	{
		char_t label[32]{};
		(void)snprintf(label, sizeof(label), "%u", bars[index]);
		if (ImGui::RadioButton(label, m_iSelectedAuditionBarIndex == index))
			m_iSelectedAuditionBarIndex = index;
		if (index + 1u < bars.size())
			ImGui::SameLine();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Reset + Play Selected"))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
	}
	ImGui::EndDisabled();

	if (!m_strAuditionStatus.empty())
		ImGui::TextWrapped("%s", m_strAuditionStatus.c_str());
	ImGui::End();
}
#endif

void CLevel_ValtanArena::Update_WorldDestructionPresentation(
	const f32_t fTimeDelta)
{
	const uint64_t generation =
		m_Replication.Get_WorldDestructionPresentationGeneration();
	if (generation != m_iObservedWorldDestructionPresentationGeneration)
	{
		m_WorldDestructionDebrisPresentationRuntime.Reset_Presentation();
		m_iObservedWorldDestructionPresentationGeneration = generation;
	}

	/* The 109 impact drains every group's cue on one frame. They are collected
	first so the actor budget can be split evenly, instead of the earliest
	groups spending everything and later ring sectors vanishing silently. */
	std::vector<WORLD_DESTRUCTION_DEBRIS_CUE> pendingCues;
	LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE event;
	while (m_Replication.Try_Consume_WorldDestructionLiveEvent(event))
	{
		const WORLD_DESTRUCTION_DEBRIS_PROFILE* profile =
			m_WorldDestructionDebrisPresentationDocument.Find_Group(
				event.strGroupId);
		if (nullptr == profile ||
			profile->strMutationId != event.strMutationId)
		{
			OutputDebugStringA(
				"[Level_ValtanArena][DestructionDebris] "
				"Ignored a cue whose group/mutation does not match the published presentation.\n");
			continue;
		}

		WORLD_DESTRUCTION_DEBRIS_CUE cue;
		cue.groupId = profile->strGroupId;
		cue.eventSequence = event.iEventSequence;
		cue.randomSeed = event.iRandomSeed;
		cue.emitters.reserve(profile->Emitters.size());
		for (const WORLD_DESTRUCTION_DEBRIS_EMITTER& emitter :
			profile->Emitters)
		{
			WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE emitterCue;
			emitterCue.sourceRuntimePlacementId =
				emitter.iSourceRuntimePlacementId;
			emitterCue.suppressionAliasPlacementIds =
				emitter.SuppressionAliasPlacementIds;
			emitterCue.spawnOffset = emitter.vSpawnOffset;
			emitterCue.direction = emitter.vDirection;
			emitterCue.speedMetersPerSecond =
				emitter.fSpeedMetersPerSecond;
			emitterCue.gravityScale = emitter.fGravityScale;
			emitterCue.lifetimeSeconds = emitter.fLifetimeSeconds;
			cue.emitters.push_back(std::move(emitterCue));
		}

		pendingCues.push_back(std::move(cue));
	}

	/* Server event order stays canonical; only the per-cue share is derived. */
	const uint32_t emitterShare =
		CWorldDestructionDebrisPresentationRuntime::Resolve_CueEmitterShare(
			m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
			pendingCues.size());
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& pendingCue : pendingCues)
	{
		std::string cueStatus;
		if (!m_WorldDestructionDebrisPresentationRuntime.Play_Cue(
			pendingCue, emitterShare, cueStatus))
		{
			OutputDebugStringA((
				"[Level_ValtanArena][DestructionDebris] " +
				cueStatus + "\n").c_str());
		}
	}

	std::string status;
	if (!m_WorldDestructionDebrisPresentationRuntime.Post_Physics_Update(
		fTimeDelta, status))
	{
		OutputDebugStringA((
			"[Level_ValtanArena][DestructionDebris] " + status + "\n").c_str());
	}
}

bool_t CLevel_ValtanArena::Ready_CinematicCamera()
{
	std::string status;
	if (!m_ValtanEncounterReference.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanEncounter.json"), status))
	{
		OutputDebugStringA(("[Level_ValtanArena][CinematicCamera] " +
			status + "\n").c_str());
		return false;
	}
	if (!m_ValtanCinematicCameraDocument.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanCinematicCamera.json"),
		m_ValtanEncounterReference, status) ||
		!m_ValtanCinematicCameraController.Initialize(
			&m_ValtanCinematicCameraDocument,
			m_ValtanEncounterReference.Get_FixedTickHz()))
	{
		OutputDebugStringA(("[Level_ValtanArena][CinematicCamera] " +
			status + "\n").c_str());
		m_ValtanCinematicCameraDocument.Clear();
		m_ValtanEncounterReference.Clear();
		return false;
	}
	return true;
}

void CLevel_ValtanArena::Update_CinematicCamera(const f32_t fTimeDelta)
{
	if (nullptr == m_pCamera)
		return;
	const VALTAN_PRESENTATION_STATE& boss =
		m_Replication.Get_ValtanPresentationState();
	VALTAN_CINEMATIC_CAMERA_INPUT input{};
	input.isValid = boss.isValid &&
		LostArk::Shared::WORLD_ENTITY_ACTION::DEAD != boss.eAction;
	input.iNetEntityId = boss.iNetEntityId;
	input.iServerTick = boss.iServerTick;
	input.strPatternId = boss.strPatternId;
	input.strStageActionId = boss.strActionId;
	input.iPatternSequence = boss.iPatternSequence;
	input.iStageIndex = boss.iPatternStageIndex;
	input.iActionStartTick = boss.iActionStartTick;

	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!m_ValtanCinematicCameraController.Update(input, fTimeDelta, pose))
	{
		End_CinematicCamera();
		return;
	}

	if (!m_bCinematicCameraApplied)
	{
		m_bCinematicRestoreFollowRequested = m_pCamera->Is_FollowRequested();
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
		if (!m_pCamera->Begin_PresentationOverride(
			static_cast<uint64_t>(boss.iNetEntityId)))
		{
			m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(
				m_bCinematicRestoreFollowRequested);
			return;
		}
		m_bCinematicCameraApplied = true;
		m_iCinematicCameraOwnerId =
			static_cast<uint64_t>(boss.iNetEntityId);
	}
	else if (nullptr != m_pCamera->Get_FollowTarget())
	{
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
	}

	if (m_iCinematicCameraOwnerId !=
		static_cast<uint64_t>(boss.iNetEntityId))
	{
		End_CinematicCamera();
		return;
	}
	if (!m_pCamera->Apply_PresentationPose(
		m_iCinematicCameraOwnerId,
		pose.vEye, pose.vLookAt, pose.fFovYDegrees))
	{
		End_CinematicCamera();
	}
}

void CLevel_ValtanArena::End_CinematicCamera()
{
	if (!m_bCinematicCameraApplied || nullptr == m_pCamera)
		return;
	if (0u != m_iCinematicCameraOwnerId)
		m_pCamera->End_PresentationOverride(m_iCinematicCameraOwnerId);
	m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
	m_pCamera->Set_FollowEnabled(m_bCinematicRestoreFollowRequested);
	m_pCinematicRestoreTarget.reset();
	m_bCinematicRestoreFollowRequested = false;
	m_bCinematicCameraApplied = false;
	m_iCinematicCameraOwnerId = 0u;
}

HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PlayerNameplateView.Render(m_NameplatePlayers);

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("Valtan Arena Map"));
#endif

	return S_OK;
}

HRESULT CLevel_ValtanArena::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	const float3_t focus(151.25f, 22.97f, -121.75f);
	const f32_t span = 180.f;

	const f32_t distance = (std::max)(40.f, span * 0.7f);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	cameraDesc.vAt = focus;
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = (std::max)(20.f, span * 0.08f);
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::VALTAN_ARENA),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::VALTAN_ARENA),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::VALTAN_ARENA), strLayerTag, gameObject);
		return E_FAIL;
	}

	return S_OK;
}

bool_t CLevel_ValtanArena::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		if (m_bCinematicCameraApplied)
		{
			m_pCinematicRestoreTarget.reset();
			return true;
		}
		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;
	if (m_bCinematicCameraApplied)
	{
		m_pCinematicRestoreTarget = transform;
		return true;
	}
	m_pCamera->Set_PositionOffset(
		float3_t(0.4f, 7.5f, 4.5f));
	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_ValtanArena> CLevel_ValtanArena::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_ValtanArena>(
		new CLevel_ValtanArena(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
