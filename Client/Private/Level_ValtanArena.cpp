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
#include "MainApp.h"
#include "MapAssetObject.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "Transform.h"

#include "DataJson.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>

namespace
{
	constexpr std::string_view VALTAN_PILLAR_SET_ID =
		"encounterprop.valtan.four-pillars";
	constexpr std::array<std::string_view, 4> VALTAN_PILLAR_SLOT_IDS = {
		"pillar.valtan.slot00", "pillar.valtan.slot01",
		"pillar.valtan.slot02", "pillar.valtan.slot03" };
	/* The Server currently owns the four inner repeatable slots. Keep this
	   slot-to-placement mapping stable when applying an encounter-prop sync. */
	/* The stele bursts instead of toppling, so its twelve pieces leave along
	   the full hemisphere around the upward axis rather than the narrow cone a
	   falling wall slab uses. */
	constexpr f32_t VALTAN_PILLAR_BURST_SPEED_METERS_PER_SECOND = 7.f;
	constexpr f32_t VALTAN_PILLAR_BURST_GRAVITY_SCALE = 2.f;
	constexpr f32_t VALTAN_PILLAR_BURST_LIFETIME_SECONDS = 4.f;
	constexpr std::array<uint64_t, 4> VALTAN_PILLAR_SLOT_PLACEMENT_IDS = {
		14226635865317864635ull,
		14753860598629869201ull,
		16421721927631310369ull,
		13580356114054111471ull };
	/* All eight DEPLOY_ITR_02326 occurrences are encounter props, not permanent
	   arena dressing. Suppress every occurrence before the first rendered frame;
	   later authored trigger work can assign the remaining four stable IDs to
	   their own slots without restoring them at level load. */
	constexpr std::array<uint64_t, 8> VALTAN_ALL_PILLAR_PLACEMENT_IDS = {
		9528847514271106184ull,
		13580356114054111471ull,
		14224868475885080166ull,
		14226635865317864635ull,
		14351557328510348857ull,
		14753860598629869201ull,
		16068006371995901944ull,
		16421721927631310369ull };

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

	/* A refused level activation drops the player back to the Lobby with the
	   socket already closed, which looks exactly like a dropped connection.
	   Every refusal therefore hands the Lobby the stage that actually said no,
	   not just E_FAIL. */
	HRESULT Report_InitFailure(
		const char_t* pStage,
		const std::string& status)
	{
		const std::string detail = std::string(pStage) + " " + status;
		OutputDebugStringA((detail + "\n").c_str());
		Client::CLevelTransitionService::Report_LoadFailure(E_FAIL, detail);
		return E_FAIL;
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
#ifdef _DEBUG
	End_ReferenceCamera(false);
#endif
	End_CinematicCamera();
	Clear_ValtanSkyPresentation();
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
		return Report_InitFailure("[Level_ValtanArena][MapArea]",
			nullptr == pEntry || nullptr == pEntry->pMapAreaId ?
				"Valtan level descriptor has no map area" :
				m_MapRuntime.Get_Status());
	}
#ifdef _DEBUG
	/* A placement that misses its static batch becomes its own draw, so the
	   fallback count is what separates a heavy arena from a broken one. */
	OutputDebugStringA(("[Level_ValtanArena][MapArea] " +
		m_MapRuntime.Get_Status() + "\n").c_str());
#endif
	if (!m_DeployRuntime.Load_Area(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId))
	{
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][DeployProp]",
			m_DeployRuntime.Get_Status());
	}
	/* The authored deploy catalog contains all eight reusable pillars so later
	   triggers can raise the intended set. Their product initial state is
	   HIDDEN, therefore suppress every occurrence before the first rendered
	   Valtan frame rather than waiting for a Server sync to arrive. */
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> hiddenPillars;
	hiddenPillars.reserve(VALTAN_ALL_PILLAR_PLACEMENT_IDS.size());
	for (const uint64_t placementId : VALTAN_ALL_PILLAR_PLACEMENT_IDS)
		hiddenPillars.emplace_back(placementId, DEPLOY_PROP_STATE::DESPAWNED);
	if (!m_DeployRuntime.Set_States(hiddenPillars))
	{
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][EncounterProps]",
			m_DeployRuntime.Get_Status());
	}
	std::string destructionStatus;
	if (!m_WorldDestructionProjectionDocument.Load(
		Find_ValtanWorldDataFile(
			L"LV_LUT_HEARTRB_ED.worlddestruction.json"), destructionStatus) ||
		m_WorldDestructionProjectionDocument.Get_AreaId() !=
			pEntry->pMapAreaId)
	{
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][WorldDestruction]",
			destructionStatus);
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
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure(
			"[Level_ValtanArena][DestructionDebrisDocument]",
			destructionStatus);
	}
	if (!m_WorldDestructionDebrisPresentationRuntime.Initialize(
		ETOUI(LEVEL::VALTAN_ARENA), m_pDevice, m_pContext,
		m_DeployRuntime, destructionStatus))
	{
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure(
			"[Level_ValtanArena][DestructionDebrisRuntime]",
			destructionStatus);
	}
	auto mapLightPresentation = make_shared<CMapLightPresentationRuntime>();
	if (!mapLightPresentation->Load_Runtime(pEntry->pMapAreaId))
	{
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][MapLight]",
			mapLightPresentation->Get_Status());
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
		Clear_ValtanSkyPresentation();
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][Camera]",
			"Camera layer could not be created");
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
		Clear_ValtanSkyPresentation();
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][Replication]",
			"Client replication could not be initialized");
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
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
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

	const bool_t replicationUpdated = m_Replication.Update();
	if (!replicationUpdated)
	{
		OutputDebugStringA(
			"[Level_ValtanArena] Failed to apply replication event.\n");
		if (m_Replication.Has_FatalWorldDestructionFailure())
		{
			/* This bounce closes the socket and returns to the Lobby without
			going through the load-failure path, so the reason has to be handed
			over explicitly or the Lobby shows nothing at all. */
			std::string presentationStatus;
			if (!m_Replication.Try_Consume_PresentationFailure(
				presentationStatus))
			{
				presentationStatus =
					"World destruction projection rejected the Server sync.";
			}
			CLevelTransitionService::Report_LoadFailure(
				E_FAIL,
				"[Level_ValtanArena][WorldDestructionSync] " +
					presentationStatus);
#ifdef _DEBUG
			End_ReferenceCamera(false);
#endif
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
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
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
		return;
	}
	if (!Apply_EncounterPropPresentation())
	{
		CLevelTransitionService::Report_LoadFailure(
			E_FAIL,
			"[Level_ValtanArena][EncounterPropSync] " +
				m_DeployRuntime.Get_Status());
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
		End_CinematicCamera();
		CNetworkManager::Get().Close_ServerConnection();
		(void)CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "encounter-prop-projection-failed");
		return;
	}

#ifdef _DEBUG
	Update_AuditionTransaction();
	Render_AuditionPanel();
	/* Driven outside the panel body so a collapsed window cannot stall a
	chapter run that is already in flight. */
	{
		const VALTAN_PRESENTATION_STATE& timelineBoss =
			m_Replication.Get_ValtanPresentationState();
		Advance_EnvironmentTimeline(
			timelineBoss.isValid && !timelineBoss.strPatternId.empty());
	}
#endif
	Update_WorldDestructionPresentation(fTimeDelta);
	Bind_CameraToLocalCharacter();
#ifdef _DEBUG
	Update_ReferenceCamera();
#endif
	Update_CinematicCamera(fTimeDelta);
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	bool_t cameraAcceptsGameplay =
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() &&
		!m_bCinematicCameraApplied;
#ifdef _DEBUG
	cameraAcceptsGameplay = cameraAcceptsGameplay &&
		!m_bReferenceCameraApplied;
#endif
	m_PlayerController.Update(cameraAcceptsGameplay);
}

#ifdef _DEBUG
namespace
{
	constexpr uint64_t AUDITION_RETRY_INTERVAL_MILLISECONDS = 750u;
	constexpr uint32_t AUDITION_MAX_RETRY_COUNT = 3u;
	/* This owner is outside the uint32 Server entity range, so a Debug reference
	view can never impersonate the owner of an authoritative cinematic cue. */
	constexpr uint64_t VALTAN_REFERENCE_CAMERA_OWNER_ID =
		0x56414C54414E5246ull;
	constexpr std::string_view VALTAN_REFERENCE_PHASE_SOURCE_LEVEL =
		"VALTAN_PHASE_SPACEHOLE";
	constexpr size_t VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT = 3u;

	struct VALTAN_REFERENCE_CAMERA_POSE final
	{
		float3_t vEye;
		float3_t vLookAt;
		f32_t fFovYDegrees = 60.f;
	};

	/* Stable comparison poses for the two supplied references. The top-down
	shot keeps a small Z offset so LookAt never becomes parallel to world-up. */
	const VALTAN_REFERENCE_CAMERA_POSE VALTAN_REFERENCE_TOP_DOWN = {
		float3_t(156.03f, 132.f, -111.f),
		float3_t(156.03f, 23.f, -122.06f),
		48.f };
	const VALTAN_REFERENCE_CAMERA_POSE VALTAN_REFERENCE_EXTERIOR = {
		float3_t(156.03f, 96.f, -18.f),
		float3_t(156.03f, 23.f, -122.06f),
		54.f };

	uint64_t Get_AuditionMonotonicMilliseconds()
	{
		return static_cast<uint64_t>(GetTickCount64());
	}

	bool_t Is_AuditionAccepted(
		const LostArk::Shared::VALTAN_AUDITION_RESULT result)
	{
		using LostArk::Shared::VALTAN_AUDITION_RESULT;
		return VALTAN_AUDITION_RESULT::ARMED == result ||
			VALTAN_AUDITION_RESULT::QUEUED == result ||
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == result;
	}

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

	/* The authored rotation script is the order the boss is meant to run its
	normal patterns in, so the Debug browser replays exactly that list rather
	than a second hand-kept order. Only this panel consumes the document, so it
	is read here instead of widening the shared encounter reference. */
	std::vector<std::string> Collect_AuthoredRotationPatternIds()
	{
		std::vector<std::string> ordered;
		const std::filesystem::path path = CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanPatternRotations.json");
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return ordered;
		const std::string text(
			(std::istreambuf_iterator<char_t>(input)),
			std::istreambuf_iterator<char_t>());
		Client::DATA_JSON_VALUE root;
		std::string parseError;
		if (!Client::CDataJson::Parse(text, root, parseError) ||
			!root.Is_Object())
		{
			return ordered;
		}
		const Client::DATA_JSON_VALUE* rotations = root.Find("rotations");
		if (nullptr == rotations || !rotations->Is_Array())
			return ordered;
		for (const Client::DATA_JSON_VALUE& rotation : rotations->Get_Array())
		{
			if (!rotation.Is_Object())
				continue;
			const Client::DATA_JSON_VALUE* ids = rotation.Find("patternIds");
			if (nullptr == ids || !ids->Is_Array())
				continue;
			for (const Client::DATA_JSON_VALUE& id : ids->Get_Array())
			{
				if (id.Is_String())
					ordered.push_back(id.Get_String());
			}
		}
		return ordered;
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
			return "Reset complete. The Server queued the requested pattern or final-arena state.";
		case VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED:
			return "Already handled that request; treating it as confirmed.";
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

bool_t CLevel_ValtanArena::Begin_ReferenceCamera(
	const REFERENCE_CAMERA_VIEW view)
{
	if (nullptr == m_pCamera || REFERENCE_CAMERA_VIEW::NONE == view ||
		m_bCinematicCameraApplied ||
		m_ValtanCinematicCameraController.Is_Active())
	{
		return false;
	}

	const VALTAN_REFERENCE_CAMERA_POSE* pose = nullptr;
	switch (view)
	{
	case REFERENCE_CAMERA_VIEW::TOP_DOWN:
		pose = &VALTAN_REFERENCE_TOP_DOWN;
		break;
	case REFERENCE_CAMERA_VIEW::EXTERIOR:
		pose = &VALTAN_REFERENCE_EXTERIOR;
		break;
	default:
		return false;
	}
	const bool_t showSpaceHole =
		REFERENCE_CAMERA_VIEW::TOP_DOWN == view;
	if (!Set_ReferencePhaseProxyVisible(showSpaceHole))
		return false;

	if (!m_bReferenceCameraApplied)
	{
		m_bReferenceCameraRestoreFollowRequested =
			m_pCamera->Is_FollowRequested();
		m_pReferenceCameraRestoreTarget = m_pCamera->Get_FollowTarget();
		/* Keep the target bound so the existing F6 shortcut remains meaningful,
		but suspend follow while the fixed presentation pose is active. */
		m_pCamera->Set_FollowEnabled(false);
		if (!m_pCamera->Begin_PresentationOverride(
			VALTAN_REFERENCE_CAMERA_OWNER_ID))
		{
			m_pCamera->Set_FollowTarget(
				m_pReferenceCameraRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(
				m_bReferenceCameraRestoreFollowRequested);
			m_pReferenceCameraRestoreTarget.reset();
			m_bReferenceCameraRestoreFollowRequested = false;
			(void)Set_ReferencePhaseProxyVisible(false);
			return false;
		}
		m_bReferenceCameraApplied = true;
	}

	if (!m_pCamera->Apply_PresentationPose(
		VALTAN_REFERENCE_CAMERA_OWNER_ID,
		pose->vEye, pose->vLookAt, pose->fFovYDegrees))
	{
		End_ReferenceCamera(false);
		return false;
	}
	m_eReferenceCameraView = view;
	return true;
}

bool_t CLevel_ValtanArena::Set_ReferencePhaseProxyVisible(
	const bool_t visible)
{
	const std::string sourceLevel(VALTAN_REFERENCE_PHASE_SOURCE_LEVEL);
	const bool_t applied = visible ?
		m_MapRuntime.Set_DebugSourceLevelVisible(
			sourceLevel, true, VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT) :
		m_MapRuntime.Restore_DebugSourceLevelVisibility(
			sourceLevel, VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT);
	if (!applied)
	{
		OutputDebugStringA(
			"[Level_ValtanArena][ReferenceCamera] SpaceHole proxy visibility failed.\n");
		return false;
	}
	m_bReferenceSpaceHoleVisible = visible;
	return true;
}

void CLevel_ValtanArena::Update_ReferenceCamera()
{
	if (!m_bReferenceCameraApplied)
	{
		if (m_bReferenceSpaceHoleVisible)
			(void)Set_ReferencePhaseProxyVisible(false);
		return;
	}
	if (nullptr == m_pCamera)
	{
		if (m_bReferenceSpaceHoleVisible)
			(void)Set_ReferencePhaseProxyVisible(false);
		m_pReferenceCameraRestoreTarget.reset();
		m_bReferenceCameraRestoreFollowRequested = false;
		m_bReferenceCameraApplied = false;
		m_eReferenceCameraView = REFERENCE_CAMERA_VIEW::NONE;
		return;
	}

	/* F6 remains the one follow/free shortcut. While a reference pose owns the
	camera, the same press dismisses it and applies the toggle the user asked for. */
	if (GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantTextInput &&
		CGameInstance::Get().Get_DIKeyPressed(DIK_F6))
	{
		End_ReferenceCamera(true);
	}
}

void CLevel_ValtanArena::End_ReferenceCamera(
	const bool_t toggleFollowRequested)
{
	if (m_bReferenceSpaceHoleVisible)
		(void)Set_ReferencePhaseProxyVisible(false);
	if (!m_bReferenceCameraApplied)
		return;

	const bool_t restoreFollowRequested =
		toggleFollowRequested ?
			!m_bReferenceCameraRestoreFollowRequested :
			m_bReferenceCameraRestoreFollowRequested;
	const shared_ptr<CTransform> restoreTarget =
		m_pReferenceCameraRestoreTarget.lock();
	if (nullptr != m_pCamera)
	{
		(void)m_pCamera->End_PresentationOverride(
			VALTAN_REFERENCE_CAMERA_OWNER_ID);
		m_pCamera->Set_FollowTarget(restoreTarget);
		m_pCamera->Set_FollowEnabled(restoreFollowRequested);
	}
	m_pReferenceCameraRestoreTarget.reset();
	m_bReferenceCameraRestoreFollowRequested = false;
	m_bReferenceCameraApplied = false;
	m_eReferenceCameraView = REFERENCE_CAMERA_VIEW::NONE;
}

const char_t* CLevel_ValtanArena::Get_ReferenceCameraViewName() const
{
	switch (m_eReferenceCameraView)
	{
	case REFERENCE_CAMERA_VIEW::TOP_DOWN:
		return "Top Down";
	case REFERENCE_CAMERA_VIEW::EXTERIOR:
		return "Exterior";
	default:
		return "none";
	}
}

void CLevel_ValtanArena::Update_AuditionTransaction()
{
	using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;

	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT result{};
	while (CNetworkManager::Get().Try_Consume_ValtanAuditionResult(result))
	{
		if (!m_PendingAuditionRequest.Is_Active() ||
			result.iRequestSequence != m_PendingAuditionRequest.iSequence)
		{
			continue;
		}
		if (result.eOperation != m_PendingAuditionRequest.eOperation ||
			result.iTargetHealthBar !=
				m_PendingAuditionRequest.iTargetHealthBar)
		{
			/* A sequence is not enough to identify a verdict. Keep waiting for
			the exact echoed request instead of completing the wrong UI action. */
			m_strAuditionStatus =
				"Ignored a mismatched Server verdict; retrying the exact request.";
			continue;
		}

		const AUDITION_PENDING_REQUEST completed =
			m_PendingAuditionRequest;
		m_PendingAuditionRequest = {};
		const bool_t accepted = Is_AuditionAccepted(result.eResult);
		m_strAuditionStatus = Describe_AuditionResult(result.eResult);

		if (!accepted && !m_EnvironmentTimeline.empty())
		{
			/* One refused chapter stops the run. Continuing would audition the
			later bars against an environment the Server never produced. */
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_bEnvironmentTimelineWaiting = false;
			m_bEnvironmentTimelinePatternStarted = false;
			m_strAuditionStatus += " Full environment timeline stopped.";
		}

		if (accepted && OPERATION::PLAY_ORDERED_1_67 == completed.eOperation)
			m_bOrderedAuditionActive = true;
		if (accepted && OPERATION::STOP_ORDERED_1_67 == completed.eOperation)
			m_bOrderedAuditionActive = false;

		if (OPERATION::STOP_ORDERED_1_67 == completed.eOperation)
		{
			const bool_t restart = m_bRestartOrderedAfterStop;
			m_bStopAuditionQueued = false;
			m_bRestartOrderedAfterStop = false;
			if (accepted && restart &&
				!Submit_Audition(OPERATION::PLAY_ORDERED_1_67))
			{
				m_strAuditionStatus =
					"The ordered audition stopped, but its restart request could not be sent.";
			}
			else if (!accepted && restart)
			{
				m_strAuditionStatus +=
					" Restart was cancelled because STOP was rejected.";
			}
			continue;
		}

		if (m_bStopAuditionQueued)
		{
			m_bStopAuditionQueued = false;
			if (!Submit_Audition(OPERATION::STOP_ORDERED_1_67))
			{
				m_bRestartOrderedAfterStop = false;
				m_strAuditionStatus =
					"The queued STOP request could not be sent.";
			}
		}
	}

	if (!m_PendingAuditionRequest.Is_Active())
		return;
	const uint64_t now = Get_AuditionMonotonicMilliseconds();
	if (now - m_PendingAuditionRequest.iLastSentAtMilliseconds <
		AUDITION_RETRY_INTERVAL_MILLISECONDS)
	{
		return;
	}

	if (m_PendingAuditionRequest.iRetryCount >= AUDITION_MAX_RETRY_COUNT)
	{
		const AUDITION_PENDING_REQUEST timedOut = m_PendingAuditionRequest;
		m_PendingAuditionRequest = {};
		m_strAuditionStatus =
			"Server verdict timed out after three bounded retries.";
		if (!m_EnvironmentTimeline.empty())
		{
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_bEnvironmentTimelineWaiting = false;
			m_bEnvironmentTimelinePatternStarted = false;
			m_strAuditionStatus += " Full environment timeline stopped.";
		}
		if (OPERATION::PLAY_ORDERED_1_67 == timedOut.eOperation)
		{
			/* The Server may have started it even though every verdict was lost.
			Keep focused reset controls disabled until an explicit STOP is acked. */
			m_bOrderedAuditionActive = true;
		}
		if (OPERATION::STOP_ORDERED_1_67 == timedOut.eOperation)
		{
			m_bStopAuditionQueued = false;
			m_bRestartOrderedAfterStop = false;
			m_strAuditionStatus +=
				" Ordered state is uncertain; send STOP again before restarting.";
			return;
		}
		if (m_bStopAuditionQueued)
		{
			m_bStopAuditionQueued = false;
			if (!Submit_Audition(OPERATION::STOP_ORDERED_1_67))
			{
				m_bRestartOrderedAfterStop = false;
				m_strAuditionStatus += " The queued STOP could not be sent.";
			}
		}
		return;
	}

	++m_PendingAuditionRequest.iRetryCount;
	m_PendingAuditionRequest.iLastSentAtMilliseconds = now;
	const bool_t sent = CNetworkManager::Get().Send_ValtanAudition(
		m_PendingAuditionRequest.iSequence,
		m_PendingAuditionRequest.eOperation,
		m_PendingAuditionRequest.iTargetHealthBar);
	m_strAuditionStatus = sent ?
		"Waiting for the Server verdict after a bounded retry..." :
		"The bounded retry could not be sent; it will not retry immediately.";
}

bool_t CLevel_ValtanArena::Submit_Audition(
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation)
{
	if (nullptr == m_Replication.Get_LocalCharacter())
	{
		m_strAuditionStatus =
			"Wait for the local character snapshot before sending an audition.";
		return false;
	}
	if (m_PendingAuditionRequest.Is_Active() || m_bStopAuditionQueued)
	{
		m_strAuditionStatus =
			"Another audition transaction must finish before this request.";
		return false;
	}

	/* These operations name an authored mechanic or a Debug state directly,
	rather than a health-bar crossing, so they carry no target bar. */
	const bool_t isBarless =
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE == operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_ORDERED_1_67 ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::STOP_ORDERED_1_67 ==
			operation;
	/* The pattern browser addresses the authored pattern order instead, and
	sends a one-based index so the wire never carries the zero that means
	"no bar". */
	const bool_t isPatternPlay =
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PATTERN == operation;
	const std::vector<Client::ENCOUNTER_PATTERN_REFERENCE>& patterns =
		m_ValtanEncounterReference.Get_Patterns();
	if (isPatternPlay && m_iSelectedAuditionPatternIndex >= patterns.size())
	{
		m_strAuditionStatus = "No authored pattern is selected.";
		return false;
	}
	const std::vector<uint32_t> bars =
		Collect_AuditionHealthBars(m_ValtanEncounterReference);
	if (!isBarless && !isPatternPlay &&
		m_iSelectedAuditionBarIndex >= bars.size())
	{
		m_strAuditionStatus = "No authored health-bar pattern is selected.";
		return false;
	}

	const uint32_t sequence = 0u == m_iNextAuditionRequestSequence ?
		1u : m_iNextAuditionRequestSequence;
	const uint32_t targetHealthBar = isPatternPlay ?
		static_cast<uint32_t>(m_iSelectedAuditionPatternIndex + 1u) :
		(isBarless ? 0u : bars[m_iSelectedAuditionBarIndex]);
	if (!CNetworkManager::Get().Send_ValtanAudition(
		sequence, operation, targetHealthBar))
	{
		m_strAuditionStatus = "Could not send the audition request.";
		return false;
	}
	m_iNextAuditionRequestSequence =
		(std::numeric_limits<uint32_t>::max)() == sequence ?
		1u : sequence + 1u;
	m_PendingAuditionRequest.iSequence = sequence;
	m_PendingAuditionRequest.eOperation = operation;
	m_PendingAuditionRequest.iTargetHealthBar = targetHealthBar;
	m_PendingAuditionRequest.iLastSentAtMilliseconds =
		Get_AuditionMonotonicMilliseconds();
	m_PendingAuditionRequest.iRetryCount = 0u;
	m_strAuditionStatus = "Waiting for the Server verdict...";
	return true;
}

void CLevel_ValtanArena::Request_OrderedAuditionStop(
	const bool_t restartAfterStop)
{
	using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
	m_EnvironmentTimeline.clear();
	m_iEnvironmentTimelineStep = 0u;
	m_bEnvironmentTimelineWaiting = false;
	m_bEnvironmentTimelinePatternStarted = false;
	m_bRestartOrderedAfterStop = restartAfterStop;

	if (m_PendingAuditionRequest.Is_Active())
	{
		if (OPERATION::STOP_ORDERED_1_67 ==
			m_PendingAuditionRequest.eOperation)
		{
			m_strAuditionStatus = restartAfterStop ?
				"Waiting for STOP before restarting the ordered audition..." :
				"Waiting for the ordered STOP verdict...";
			return;
		}
		m_bStopAuditionQueued = true;
		m_strAuditionStatus = restartAfterStop ?
			"STOP is queued; PLAY will follow only after its verdict." :
			"STOP is queued behind the current audition transaction.";
		return;
	}

	m_bStopAuditionQueued = false;
	if (!Submit_Audition(OPERATION::STOP_ORDERED_1_67))
		m_bRestartOrderedAfterStop = false;
}

void CLevel_ValtanArena::Start_AuthoredRotationPlayback(
	const std::vector<std::string>& rotationOrder)
{
	using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
	const std::vector<Client::ENCOUNTER_PATTERN_REFERENCE>& patterns =
		m_ValtanEncounterReference.Get_Patterns();
	std::vector<ENVIRONMENT_TIMELINE_STEP> staged;
	staged.reserve(rotationOrder.size());
	for (const std::string& patternId : rotationOrder)
	{
		/* The Server resolves the same one-based position in the same authored
		order, so the index is built from the encounter reference rather than
		from a second list this panel would have to keep in step. */
		const auto found = std::find_if(
			patterns.begin(), patterns.end(),
			[&patternId](const Client::ENCOUNTER_PATTERN_REFERENCE& candidate)
			{
				return candidate.patternId == patternId;
			});
		if (patterns.end() == found)
		{
			m_strAuditionStatus =
				"The rotation script names a pattern the encounter does not own: " +
				patternId;
			return;
		}
		ENVIRONMENT_TIMELINE_STEP step{};
		step.eOperation = OPERATION::PLAY_PATTERN;
		step.iTargetHealthBar = static_cast<uint32_t>(
			std::distance(patterns.begin(), found)) + 1u;
		step.waitForPattern = true;
		staged.push_back(step);
	}
	if (staged.empty())
	{
		m_strAuditionStatus = "The rotation script carries no playable step.";
		return;
	}
	m_EnvironmentTimeline = std::move(staged);
	m_iEnvironmentTimelineStep = 0u;
	m_bEnvironmentTimelineWaiting = false;
	m_bEnvironmentTimelinePatternStarted = false;
	m_strAuditionStatus.clear();
}

void CLevel_ValtanArena::Start_EnvironmentTimeline()
{
	using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
	/* Only the first step resets. Every later chapter is an ARM/CROSS pair, so
	the walls the previous chapter broke stay broken exactly as the recording
	shows them accumulating. */
	m_EnvironmentTimeline = {
		{ OPERATION::PLAY_ENTRANCE, 0u, true },
		{ OPERATION::ARM_HEALTH_BAR, 159u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 159u, true },
		{ OPERATION::ARM_HEALTH_BAR, 109u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 109u, true },
		{ OPERATION::ARM_HEALTH_BAR, 100u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 100u, true },
		{ OPERATION::ARM_HEALTH_BAR, 14u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 14u, true } };
	m_iEnvironmentTimelineStep = 0u;
	m_bEnvironmentTimelineWaiting = false;
	m_bEnvironmentTimelinePatternStarted = false;
}

void CLevel_ValtanArena::Advance_EnvironmentTimeline(
	const bool_t isBossPatternRunning)
{
	if (m_EnvironmentTimeline.empty() ||
		m_PendingAuditionRequest.Is_Active() || m_bStopAuditionQueued ||
		m_bOrderedAuditionActive)
	{
		return;
	}
	if (m_bEnvironmentTimelineWaiting)
	{
		/* A queued pattern needs a tick to start, so the step is only finished
		once the Server actually showed it running and then went idle. */
		if (isBossPatternRunning)
		{
			m_bEnvironmentTimelinePatternStarted = true;
			return;
		}
		if (!m_bEnvironmentTimelinePatternStarted)
			return;
		m_bEnvironmentTimelineWaiting = false;
		m_bEnvironmentTimelinePatternStarted = false;
		++m_iEnvironmentTimelineStep;
	}
	if (m_iEnvironmentTimelineStep >= m_EnvironmentTimeline.size())
	{
		/* The two runs finish on different notes, so the verdict is read from
		the run that just ended rather than assuming the chapter timeline. */
		const bool_t wasRotationPlayback =
			!m_EnvironmentTimeline.empty() &&
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PATTERN ==
				m_EnvironmentTimeline.front().eOperation;
		m_EnvironmentTimeline.clear();
		m_iEnvironmentTimelineStep = 0u;
		m_strAuditionStatus = wasRotationPlayback ?
			"Authored rotation playback finished." :
			"Full environment timeline finished. Pillars stay raised: the shatter has no product trigger yet.";
		return;
	}

	const ENVIRONMENT_TIMELINE_STEP& step =
		m_EnvironmentTimeline[m_iEnvironmentTimelineStep];
	if (LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PATTERN ==
		step.eOperation)
	{
		/* A rotation step carries a one-based authored pattern index, not a
		bar, so it selects out of the pattern list instead of the bar list. */
		const std::vector<Client::ENCOUNTER_PATTERN_REFERENCE>& patterns =
			m_ValtanEncounterReference.Get_Patterns();
		if (0u == step.iTargetHealthBar ||
			step.iTargetHealthBar > patterns.size())
		{
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_strAuditionStatus =
				"Rotation playback stopped: an authored pattern is missing.";
			return;
		}
		m_iSelectedAuditionPatternIndex =
			static_cast<size_t>(step.iTargetHealthBar - 1u);
	}
	else if (0u != step.iTargetHealthBar)
	{
		const std::vector<uint32_t> bars =
			Collect_AuditionHealthBars(m_ValtanEncounterReference);
		const auto selected =
			std::find(bars.begin(), bars.end(), step.iTargetHealthBar);
		if (bars.end() == selected)
		{
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_strAuditionStatus =
				"Full environment timeline stopped: an authored bar is missing.";
			return;
		}
		m_iSelectedAuditionBarIndex =
			static_cast<size_t>(std::distance(bars.begin(), selected));
	}
	if (!Submit_Audition(step.eOperation))
	{
		/* The request never left, so the chapter run stops instead of silently
		skipping the rest of the timeline. */
		m_EnvironmentTimeline.clear();
		m_iEnvironmentTimelineStep = 0u;
		return;
	}
	if (step.waitForPattern)
		m_bEnvironmentTimelineWaiting = true;
	else
		++m_iEnvironmentTimelineStep;
}

void CLevel_ValtanArena::Render_AuditionPanel()
{
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
	}
	else if (m_iSelectedAuditionBarIndex >= bars.size())
		m_iSelectedAuditionBarIndex = 0u;

	ImGui::TextUnformatted(
		"Server-authoritative repeatable audition.");
	ImGui::TextUnformatted(
		"Each Play resets boss + walls + collision/nav, then runs the real pattern.");
	ImGui::TextColored(
		ImVec4(1.f, 0.55f, 0.25f, 1.f),
		"One click: every Reset + Play activates the boss, places you at the authored bait point and resets walls/collision/nav.");
	ImGui::TextDisabled(
		"No Stage_Boss walk-through and no monster kill is required first.");
	ImGui::TextColored(
		ImVec4(1.f, 0.82f, 0.2f, 1.f),
		"109: completed outer ring only, 30 walls x 12 fragments.");
	ImGui::TextDisabled(
		"TAKEOFF -> DROP -> IMPACT breaks the 8 outer sectors; interior groups stay dormant;");
	ImGui::TextDisabled(
		"collision and nav blockers open on the persistent commit, not on the cue.");
	ImGui::TextColored(
		ImVec4(0.4f, 0.85f, 1.f, 1.f),
		"Attack/159 charge: ordinary walls only; collision/nav opens after the wall disappears.");
	ImGui::TextColored(
		ImVec4(1.f, 0.82f, 0.2f, 1.f),
		"109 outer ring is pattern-only: attack and charge must leave all 30 intact.");
	ImGui::TextDisabled(
		"No jump clip exists in this model, so the Server owns the 109 leap as an authored arc.");

	ImGui::SeparatorText("Reference Views (Debug presentation only)");
	const bool_t serverCameraOwnsPresentation =
		m_bCinematicCameraApplied ||
		m_ValtanCinematicCameraController.Is_Active();
	ImGui::BeginDisabled(nullptr == m_pCamera ||
		serverCameraOwnsPresentation);
	/* The audition panel can be docked to a narrow right rail.  Keep every
	reference action on its own full-width row so Exterior never lands outside
	the visible panel (the user's 1280-wide capture exposed this regression). */
	const f32_t referenceButtonWidth =
		(std::max)(1.f, ImGui::GetContentRegionAvail().x);
	if (ImGui::Button(
		"Reference Top Down", ImVec2(referenceButtonWidth, 0.f)))
		(void)Begin_ReferenceCamera(REFERENCE_CAMERA_VIEW::TOP_DOWN);
	if (ImGui::Button(
		"Reference Exterior (Arena Towers)",
		ImVec2(referenceButtonWidth, 0.f)))
		(void)Begin_ReferenceCamera(REFERENCE_CAMERA_VIEW::EXTERIOR);
	ImGui::EndDisabled();
	ImGui::BeginDisabled(!m_bReferenceCameraApplied);
	if (ImGui::Button(
		"Restore Camera", ImVec2(referenceButtonWidth, 0.f)))
		End_ReferenceCamera(false);
	ImGui::EndDisabled();
	ImGui::Text("Reference view: %s", Get_ReferenceCameraViewName());
	ImGui::SameLine();
	ImGui::TextDisabled("SpaceHole proxy: %s",
		m_bReferenceSpaceHoleVisible ? "visible" : "authored baseline");
	ImGui::TextDisabled(
		"F6 exits this view and toggles follow/free; a Server cinematic always takes priority.");
	if (serverCameraOwnsPresentation)
		ImGui::TextDisabled("Reference views wait until the Server cinematic ends.");

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
		/* The snapshot carries the stage index, not the stage name, so the
		authored encounter resolves the readable stage ID for the panel. */
		const char_t* stageId = "(idle)";
		if (!boss.strPatternId.empty())
		{
			const ENCOUNTER_PATTERN_REFERENCE* pattern =
				m_ValtanEncounterReference.Find_Pattern(boss.strPatternId);
			if (nullptr != pattern &&
				boss.iPatternStageIndex < pattern->stages.size())
			{
				stageId =
					pattern->stages[boss.iPatternStageIndex].stageId.c_str();
			}
		}
		ImGui::TextWrapped("Stage: %s", stageId);
		ImGui::TextWrapped("Action: %s",
			boss.strActionId.empty() ? "(idle)" : boss.strActionId.c_str());
	}
	else
	{
		ImGui::TextDisabled("Waiting for the authoritative Valtan snapshot...");
	}

	/* The 109 collapse owns the outer ring alone, so the panel counts the ring
	separately from the interior groups that must not react to it. The prefix is
	the authored group naming contract in ValtanWorldEvents.json. */
	static constexpr std::string_view OUTER_RING_GROUP_PREFIX =
		"destroyable.group.valtan.outerwall109.";
	size_t outerGroupCount = 0u;
	size_t outerPlacementCount = 0u;
	for (const WORLD_DESTRUCTION_PROJECTION_GROUP& group :
		m_WorldDestructionProjectionDocument.Get_Groups())
	{
		if (!std::string_view(group.strGroupId).starts_with(
			OUTER_RING_GROUP_PREFIX))
		{
			continue;
		}
		++outerGroupCount;
		outerPlacementCount += group.MemberPlacementIds.size();
	}

	/* The arena floor collapses on its own health-bar patterns, so its sectors
	are counted separately instead of being reported as interior walls that
	reacted to the 109 collapse by mistake. Three groups answer each stage: one
	outer rail and two brick sectors per arena half. The SL00 inner wedge and
	centre cap are Map placements, so they never appear in either count. */
	static constexpr std::string_view FLOOR_STAGE_A_GROUP_PREFIX =
		"destroyable.group.valtan.floor84.";
	static constexpr std::string_view FLOOR_STAGE_B_GROUP_PREFIX =
		"destroyable.group.valtan.floor30.";
	size_t intactCount = 0u;
	size_t breakingCount = 0u;
	size_t fracturedCount = 0u;
	size_t despawnedCount = 0u;
	size_t interiorReactedCount = 0u;
	size_t floorStageAIntact = 0u;
	size_t floorStageABreaking = 0u;
	size_t floorStageAGone = 0u;
	size_t floorStageBIntact = 0u;
	size_t floorStageBBreaking = 0u;
	size_t floorStageBGone = 0u;
	for (const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& group :
		m_Replication.Get_WorldDestructionGroupStates())
	{
		const std::string_view groupId(group.strGroupId);
		const bool isStageA = groupId.starts_with(FLOOR_STAGE_A_GROUP_PREFIX);
		const bool isStageB = groupId.starts_with(FLOOR_STAGE_B_GROUP_PREFIX);
		if (isStageA || isStageB)
		{
			size_t& intactSlot = isStageA ? floorStageAIntact : floorStageBIntact;
			size_t& breakingSlot =
				isStageA ? floorStageABreaking : floorStageBBreaking;
			size_t& goneSlot = isStageA ? floorStageAGone : floorStageBGone;
			switch (group.eState)
			{
			case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT:
				++intactSlot;
				break;
			case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING:
				++breakingSlot;
				break;
			default:
				++goneSlot;
				break;
			}
			continue;
		}
		if (!groupId.starts_with(OUTER_RING_GROUP_PREFIX))
		{
			if (LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT !=
				group.eState)
			{
				++interiorReactedCount;
			}
			continue;
		}
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
		"Outer walls: INTACT %zu | BREAKING %zu | FRACTURED %zu | GONE %zu",
		intactCount,
		breakingCount,
		fracturedCount,
		despawnedCount);
	ImGui::Text("109 outer enabled group count: %zu   Outer placement count: %zu",
		outerGroupCount,
		outerPlacementCount);
	ImGui::Text(
		"Floor Stage A (84): INTACT %zu | BREAKING %zu | GONE %zu   (expected 3)",
		floorStageAIntact,
		floorStageABreaking,
		floorStageAGone);
	ImGui::Text(
		"Floor Stage B (30): INTACT %zu | BREAKING %zu | GONE %zu   (expected 3)",
		floorStageBIntact,
		floorStageBBreaking,
		floorStageBGone);
	ImGui::TextDisabled(
		"Stage A drops the screen-right half (outer rail plus two brick sectors),"
		" stage B the screen-left half. A half is the smallest authored unit: the"
		" rail submeshes are material layers, not angular slices. The SL00 inner"
		" wedge and centre cap are Map placements and stay standing either way.");
	if (0u != interiorReactedCount)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.35f, 1.f),
			"Interior groups that left INTACT: %zu (the 109 batch must not touch them)",
			interiorReactedCount);
	}
	ImGui::Text("Destruction sync: %s  Epoch: %u  Debris actors: %u/%u",
		m_Replication.Is_WorldDestructionSynchronized() ? "READY" : "WAITING",
		m_Replication.Get_WorldDestructionEncounterEpoch(),
		m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
		CWorldDestructionDebrisPresentationRuntime::MAX_ACTIVE_ACTORS);
	/* Server-owned counters. The Client never derives passage from wall states;
	a wall that is gone while its blocker is still active must stay visible. */
	const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS& diagnostics =
		m_Replication.Get_WorldDestructionDiagnostics();
	ImGui::Text("Active player-blocking collision boxes: %u   Active nav regions: %u",
		diagnostics.iActiveWallCollisionCount,
		diagnostics.iActiveNavBlockerRegionCount);
	ImGui::Text("Nav revision: %llu   Last destruction event: %llu",
		static_cast<unsigned long long>(diagnostics.iNavigationRevision),
		static_cast<unsigned long long>(diagnostics.iLastEventSequence));
	ImGui::Text("Camera cue: %s   Sky cue: %s",
		m_ValtanCinematicCameraController.Is_Active() ? "ACTIVE" : "none",
		m_ValtanSkyState.isActive ?
			m_ValtanSkyState.strCueId.c_str() : "none");
	if (m_ValtanSkyState.isActive)
	{
		ImGui::Text("  cloud opacity %.2f   aperture %.2f   spin %.1f deg",
			m_ValtanSkyState.fCloudOpacity,
			m_ValtanSkyState.fApertureScale,
			m_ValtanSkyState.fCloudRotationDegrees);
		if (m_ValtanSkyState.strRedCloudAssetId.empty() &&
			m_ValtanSkyState.strBlackApertureAssetId.empty())
		{
			ImGui::TextDisabled(
				"  sky layer asset IDs were not resolved");
		}
	}
	/* The pillars are the one repeatable encounter prop, so the panel reports
	the live slot states and the occurrence they belong to. */
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& props =
		m_Replication.Get_EncounterPropState();
	if (props.Slots.empty())
	{
		ImGui::TextDisabled("Pillars: no encounter prop state received yet.");
	}
	else
	{
		size_t hiddenSlots = 0u;
		size_t intactSlots = 0u;
		size_t breakingSlots = 0u;
		uint32_t occurrence = 0u;
		for (const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot : props.Slots)
		{
			occurrence = (std::max)(occurrence, slot.iOccurrenceSequence);
			switch (slot.eState)
			{
			case LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT:
				++intactSlots;
				break;
			case LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING:
				++breakingSlots;
				break;
			case LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN:
				++hiddenSlots;
				break;
			default:
				break;
			}
		}
		ImGui::Text(
			"Pillars: %zu slots | HIDDEN %zu | INTACT %zu | BREAKING %zu",
			props.Slots.size(), hiddenSlots, intactSlots, breakingSlots);
		ImGui::Text("  occurrence %u   epoch %u   set %s",
			occurrence, props.iEncounterEpoch, props.strPropSetId.c_str());
	}
	const bool_t hasLocalCharacter =
		nullptr != m_Replication.Get_LocalCharacter();
	const bool_t isTransactionBusy =
		m_PendingAuditionRequest.Is_Active() || m_bStopAuditionQueued;
	const bool_t focusedControlsDisabled = !hasLocalCharacter ||
		isTransactionBusy || m_bOrderedAuditionActive ||
		!m_EnvironmentTimeline.empty();
	ImGui::SeparatorText("Ordered 1-67 audition");
	ImGui::BeginDisabled(!hasLocalCharacter);
	if (ImGui::Button(
		"Restart 1-67 Ordered Audition", ImVec2(300.f, 0.f)))
	{
		Request_OrderedAuditionStop(true);
	}
	if (ImGui::Button("Stop Ordered Audition", ImVec2(300.f, 0.f)))
	{
		Request_OrderedAuditionStop(false);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Use Restart only for the full 1-67 run; focused buttons below can be used directly.");
	ImGui::SeparatorText("Focused audition controls");
	ImGui::BeginDisabled(focusedControlsDisabled);
	if (ImGui::Button(
		"Reset + Play Entrance Whirlwind (Front Walls A/B)", ImVec2(300.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE);
	}
	if (ImGui::Button(
		"Reset + Play 109 Only (Outer Wall 30 x 12)", ImVec2(330.f, 0.f)))
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
	if (ImGui::Button(
		"Reset + Play 84 (Floor Stage A / Outer Rail)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 84u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	if (ImGui::Button(
		"Reset + Play 30 (Floor Stage B / Brick Ring)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 30u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	ImGui::TextDisabled(
		"Floor sectors stay walkable through BREAKING and turn NON-WALKABLE at the DESPAWNED commit tick.");
	if (ImGui::Button(
		"Reset + Play Attack (Down Smash / Ordinary Wall)",
		ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK);
	}
	if (ImGui::Button(
		"Reset + Play Charge (159 Impact / Nav)", ImVec2(330.f, 0.f)))
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
	if (ImGui::Button(
		"Reset + Remove All Walls (Final Arena View)", ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA);
	}
	ImGui::TextDisabled(
		"Final Arena View uses the Server destruction transaction; wait about 0.3 seconds for BREAKING -> GONE.");
	if (ImGui::Button(
		"Reset + Break Every Wall (Keep Floor)", ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL);
	}
	ImGui::TextDisabled(
		"Same transaction without the floor, so 84 and 30 can then collapse with nothing standing above them.");
	if (ImGui::Button(
		"Reset + Play Sky + Pillar Cycle", ImVec2(300.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE);
	}
	if (ImGui::Button(
		"Reset + Play Full Environment Timeline", ImVec2(300.f, 0.f)))
	{
		Start_EnvironmentTimeline();
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Pillar Cycle runs the authored 100-bar pattern: sky cues, the product raise,");
	ImGui::TextDisabled(
		"then the Debug shatter and removal. Press it again for the next cycle.");
	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.3f, 1.f),
		"Cycles 2-4 have no product trigger yet: no pattern, stage or binding is authored for them.");
	if (!m_EnvironmentTimeline.empty())
	{
		ImGui::Text("Timeline chapter %zu / %zu",
			m_iEnvironmentTimelineStep + 1u, m_EnvironmentTimeline.size());
	}
	ImGui::SeparatorText("Advanced authored health bar");
	ImGui::SeparatorText("Authored health bar");
	ImGui::TextColored(
		ImVec4(0.55f, 0.85f, 1.f, 1.f),
		"Floor collapse: 84 drops the screen-right half of the disc, 30 the"
		" screen-left half, and the Server clears every wall inside the same"
		" request. Press \"Reference Top Down\" above first: the audition stands"
		" you off-centre and the far half is easy to miss from ground level.");
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
	ImGui::BeginDisabled(focusedControlsDisabled);
	if (ImGui::Button("Reset + Play Selected"))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
	}
	/* ARM then CROSS is the only pair that leaves the current destruction
	alone, so whatever an earlier chapter already broke stays broken. */
	if (ImGui::Button("Play Selected (Keep Broken)"))
	{
		if (m_iSelectedAuditionBarIndex >= bars.size())
		{
			m_strAuditionStatus =
				"No authored health-bar pattern is selected.";
		}
		else
		{
			using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
			const uint32_t selectedBar = bars[m_iSelectedAuditionBarIndex];
			m_EnvironmentTimeline = {
				{ OPERATION::ARM_HEALTH_BAR, selectedBar, false },
				{ OPERATION::CROSS_HEALTH_BAR, selectedBar, true } };
			m_iEnvironmentTimelineStep = 0u;
			m_bEnvironmentTimelineWaiting = false;
			m_bEnvironmentTimelinePatternStarted = false;
		}
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Keep Broken replays the selected bar without resetting walls, floor or props.");
	ImGui::TextDisabled(
		"A floor-collapse bar takes every wall down with it in the same request.");

	/* The authored rotation script is what the boss is meant to run between
	the scripted mechanics, and only nine patterns are owned by a health bar.
	Replaying the script in order is therefore the only way to watch the whole
	authored order without fighting for every weighted roll. */
	ImGui::SeparatorText("New Pattern");
	const std::vector<std::string> rotationOrder =
		Collect_AuthoredRotationPatternIds();
	const std::vector<Client::ENCOUNTER_PATTERN_REFERENCE>& authoredPatterns =
		m_ValtanEncounterReference.Get_Patterns();
	if (rotationOrder.empty() || authoredPatterns.empty())
	{
		ImGui::TextDisabled(
			"ValtanPatternRotations.json carries no authored rotation step.");
	}
	else
	{
		const bool_t rotationRunning = !m_EnvironmentTimeline.empty() &&
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PATTERN ==
				m_EnvironmentTimeline.front().eOperation;
		ImGui::Text("Authored rotation script: %zu patterns in order",
			rotationOrder.size());
		ImGui::BeginChild(
			"##ValtanRotationScript", ImVec2(330.f, 150.f),
			ImGuiChildFlags_Borders);
		for (size_t index = 0; index < rotationOrder.size(); ++index)
		{
			const bool_t isCurrent = rotationRunning &&
				m_iEnvironmentTimelineStep == index;
			if (isCurrent)
			{
				ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.f, 1.f), "%2zu. %s",
					index + 1u, rotationOrder[index].c_str());
			}
			else
			{
				ImGui::Text("%2zu. %s",
					index + 1u, rotationOrder[index].c_str());
			}
		}
		ImGui::EndChild();
		ImGui::BeginDisabled(focusedControlsDisabled || rotationRunning);
		if (ImGui::Button("Play New Pattern", ImVec2(330.f, 0.f)))
			Start_AuthoredRotationPlayback(rotationOrder);
		ImGui::EndDisabled();
		if (rotationRunning)
		{
			ImGui::Text("Playing %zu / %zu",
				m_iEnvironmentTimelineStep + 1u, m_EnvironmentTimeline.size());
			if (ImGui::Button("Stop New Pattern", ImVec2(330.f, 0.f)))
			{
				m_EnvironmentTimeline.clear();
				m_iEnvironmentTimelineStep = 0u;
				m_bEnvironmentTimelineWaiting = false;
				m_bEnvironmentTimelinePatternStarted = false;
				m_strAuditionStatus = "Rotation playback stopped.";
			}
		}
		ImGui::TextDisabled(
			"Plays every authored rotation step in order. Each step resets the"
			" arena and queues its pattern through the same Brain path a"
			" health-bar crossing uses.");
	}

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
		if (nullptr == profile)
		{
			/* State-only groups (for example collapsing floor sectors) have no
			debris recipe. Their persistent projection was already committed. */
			continue;
		}
		if (profile->strMutationId != event.strMutationId)
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
	size_t pendingEmitterCount = 0u;
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& pendingCue : pendingCues)
		pendingEmitterCount += pendingCue.emitters.size();
	const uint32_t emitterShare =
		CWorldDestructionDebrisPresentationRuntime::Resolve_CueEmitterShare(
			m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
			pendingCues.size(),
			pendingEmitterCount);
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

bool_t CLevel_ValtanArena::Apply_EncounterPropPresentation()
{
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& props =
		m_Replication.Get_EncounterPropState();
	if (props.Slots.empty())
		return true;
	if (props.strPropSetId != VALTAN_PILLAR_SET_ID ||
		props.Slots.size() != VALTAN_PILLAR_SLOT_IDS.size())
	{
		OutputDebugStringA(
			"[Level_ValtanArena][EncounterProps] Unknown prop set or slot count.\n");
		return false;
	}
	if (props.iEncounterEpoch == m_iObservedEncounterPropEpoch &&
		props.iServerTick == m_iObservedEncounterPropServerTick)
	{
		return true;
	}

	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> states;
	states.reserve(props.Slots.size());
	/* One cue per slot. A slot that shatters on its own tick must not be
	   folded into a neighbour that is still standing. */
	std::vector<WORLD_DESTRUCTION_DEBRIS_CUE> burstCues;
	for (size_t index = 0u; index < props.Slots.size(); ++index)
	{
		const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot =
			props.Slots[index];
		if (slot.strSlotId != VALTAN_PILLAR_SLOT_IDS[index])
		{
			OutputDebugStringA(
				"[Level_ValtanArena][EncounterProps] Slot order or identity is invalid.\n");
			return false;
		}
		DEPLOY_PROP_STATE deployState = DEPLOY_PROP_STATE::INTACT;
		switch (slot.eState)
		{
		case LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN:
			deployState = DEPLOY_PROP_STATE::DESPAWNED;
			break;
		case LostArk::Shared::ENCOUNTER_PROP_STATE::SPAWNING:
		case LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT:
		case LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING:
			deployState = DEPLOY_PROP_STATE::INTACT;
			break;
		default:
			return false;
		}
		/* The shatter is an edge. The Server repeats BREAKING for as long as the
		   slot stays in it, so the burst is keyed to the slot state version and
		   thrown exactly once for it. */
		if (LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING == slot.eState &&
			0u != slot.iStateVersion &&
			m_FiredEncounterPropBurstVersions[index] != slot.iStateVersion)
		{
			WORLD_DESTRUCTION_DEBRIS_CUE burstCue{};
			burstCue.groupId = slot.strSlotId;
			burstCue.eventSequence = slot.iStateVersion;
			burstCue.randomSeed = slot.iOccurrenceSequence;
			WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE emitter{};
			emitter.sourceRuntimePlacementId =
				VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index];
			emitter.direction = float3_t(0.f, 1.f, 0.f);
			emitter.speedMetersPerSecond =
				VALTAN_PILLAR_BURST_SPEED_METERS_PER_SECOND;
			emitter.gravityScale = VALTAN_PILLAR_BURST_GRAVITY_SCALE;
			emitter.lifetimeSeconds = VALTAN_PILLAR_BURST_LIFETIME_SECONDS;
			emitter.spreadDegrees = BURST_DEBRIS_SPREAD_DEGREES;
			burstCue.emitters.push_back(std::move(emitter));
			burstCues.push_back(std::move(burstCue));
			m_FiredEncounterPropBurstVersions[index] = slot.iStateVersion;
		}
		states.emplace_back(
			VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index], deployState);
	}
	if (!m_DeployRuntime.Set_States(states))
		return false;
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& burstCue : burstCues)
	{
		std::string cueStatus;
		/* A stele with no authored twelve-piece recipe is still retired by the
		   Server's own HIDDEN edge, so a refused cue costs the shatter, never the
		   mechanic. */
		if (!m_WorldDestructionDebrisPresentationRuntime.Play_Cue(
			burstCue, 1u, cueStatus))
		{
			OutputDebugStringA((
				"[Level_ValtanArena][EncounterProps] " +
				cueStatus + "\n").c_str());
		}
	}
	m_iObservedEncounterPropEpoch = props.iEncounterEpoch;
	m_iObservedEncounterPropServerTick = props.iServerTick;
	return true;
}

bool_t CLevel_ValtanArena::Ready_CinematicCamera()
{
	std::string status;
	if (!m_ValtanEncounterReference.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanEncounter.json"), status))
	{
		(void)Report_InitFailure(
			"[Level_ValtanArena][EncounterReference]", status);
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
		Clear_ValtanSkyPresentation();
		m_ValtanCinematicCameraDocument.Clear();
		m_ValtanEncounterReference.Clear();
		(void)Report_InitFailure(
			"[Level_ValtanArena][CinematicCamera]", status);
		return false;
	}
	if (!Ready_ValtanSkyPresentation(status))
	{
		Clear_ValtanSkyPresentation();
		m_ValtanCinematicCameraController.Reset();
		m_ValtanCinematicCameraDocument.Clear();
		m_ValtanEncounterReference.Clear();
		(void)Report_InitFailure(
			"[Level_ValtanArena][CinematicSky]", status);
		return false;
	}
	return true;
}

bool_t CLevel_ValtanArena::Ready_ValtanSkyPresentation(
	std::string& outStatus)
{
	using SKY_LAYER_ARRAY = std::array<
		VALTAN_SKY_PRESENTATION_LAYER, VALTAN_SKY_LAYER_COUNT>;
	const std::vector<VALTAN_CINEMATIC_SKY_CUE>& cues =
		m_ValtanCinematicCameraDocument.Get_SkyCues();
	if (cues.empty())
	{
		outStatus = "Cinematic sky document has no presentation cue";
		return false;
	}

	const std::string redCloudSeed = cues.front().strRedCloudAssetId;
	const std::string blackApertureSeed =
		cues.front().strBlackApertureAssetId;
	if (redCloudSeed.empty() || blackApertureSeed.empty() ||
		redCloudSeed == blackApertureSeed)
	{
		outStatus = "Cinematic sky seed asset IDs are missing or ambiguous";
		return false;
	}
	for (const VALTAN_CINEMATIC_SKY_CUE& cue : cues)
	{
		if (cue.strRedCloudAssetId != redCloudSeed ||
			cue.strBlackApertureAssetId != blackApertureSeed)
		{
			outStatus = "Cinematic sky cues do not share one stable asset pair";
			return false;
		}
	}

	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements =
		m_MapRuntime.Get_Placements();
	auto stageGroup = [&placements, &outStatus](
		const std::string& seedAssetId,
		const char_t* groupLabel,
		SKY_LAYER_ARRAY& outLayers,
		std::string& outSourceLevel)
	{
		const MAP_RUNTIME_PLACED_ENTRY* pSeed = nullptr;
		for (const MAP_RUNTIME_PLACED_ENTRY& entry : placements)
		{
			if (entry.record.assetId != seedAssetId)
				continue;
			if (nullptr != pSeed)
			{
				outStatus = std::string(groupLabel) +
					" sky seed resolves to multiple placements: " + seedAssetId;
				return false;
			}
			pSeed = &entry;
		}
		if (nullptr == pSeed || pSeed->record.sourceLevel.empty() ||
			nullptr == pSeed->object || nullptr != pSeed->batch)
		{
			outStatus = std::string(groupLabel) +
				" sky seed is missing or not a non-batched map object: " +
				seedAssetId;
			return false;
		}
		outSourceLevel = pSeed->record.sourceLevel;

		size_t layerCount = 0u;
		for (const MAP_RUNTIME_PLACED_ENTRY& entry : placements)
		{
			if (entry.record.sourceLevel != outSourceLevel)
				continue;
			if (layerCount >= outLayers.size() || nullptr == entry.object ||
				nullptr != entry.batch || 0u == entry.record.placementId)
			{
				outStatus = std::string(groupLabel) +
					" sky source group must contain exactly three non-batched objects: " +
					outSourceLevel;
				return false;
			}
			VALTAN_SKY_PRESENTATION_LAYER& layer = outLayers[layerCount++];
			layer.pObject = entry.object;
			layer.strAssetId = entry.record.assetId;
			layer.iPlacementId = entry.record.placementId;
			layer.vBasePosition = entry.record.position;
			layer.vBaseRotationQuaternion = entry.record.rotationQuaternion;
			layer.vBaseSignedScale = entry.record.signedScale;
			layer.bBaseVisible = entry.object->Is_Visible();
		}
		if (outLayers.size() != layerCount)
		{
			outStatus = std::string(groupLabel) +
				" sky source group does not contain three objects: " +
				outSourceLevel;
			return false;
		}
		std::sort(outLayers.begin(), outLayers.end(),
			[](const VALTAN_SKY_PRESENTATION_LAYER& left,
				const VALTAN_SKY_PRESENTATION_LAYER& right)
			{
				return left.iPlacementId < right.iPlacementId;
			});
		for (size_t left = 0u; left < outLayers.size(); ++left)
		{
			for (size_t right = left + 1u; right < outLayers.size(); ++right)
			{
				if (outLayers[left].iPlacementId ==
						outLayers[right].iPlacementId ||
					outLayers[left].pObject == outLayers[right].pObject)
				{
					outStatus = std::string(groupLabel) +
						" sky source group contains a duplicate object";
					return false;
				}
			}
		}
		return true;
	};

	SKY_LAYER_ARRAY stagedRedCloudLayers{};
	SKY_LAYER_ARRAY stagedBlackApertureLayers{};
	std::string redCloudSourceLevel;
	std::string blackApertureSourceLevel;
	if (!stageGroup(redCloudSeed, "Red cloud", stagedRedCloudLayers,
			redCloudSourceLevel) ||
		!stageGroup(blackApertureSeed, "Black aperture",
			stagedBlackApertureLayers, blackApertureSourceLevel))
	{
		return false;
	}
	if (redCloudSourceLevel == blackApertureSourceLevel)
	{
		outStatus = "Cinematic sky seeds resolve to the same source group";
		return false;
	}
	for (const VALTAN_SKY_PRESENTATION_LAYER& redLayer :
		stagedRedCloudLayers)
	{
		for (const VALTAN_SKY_PRESENTATION_LAYER& apertureLayer :
			stagedBlackApertureLayers)
		{
			if (redLayer.iPlacementId == apertureLayer.iPlacementId ||
				redLayer.pObject == apertureLayer.pObject)
			{
				outStatus = "Cinematic sky source groups overlap";
				return false;
			}
		}
	}
	const auto hasExactAssetSet = [](const SKY_LAYER_ARRAY& layers,
		const std::array<std::string_view, VALTAN_SKY_LAYER_COUNT>& expected)
	{
		return std::all_of(expected.begin(), expected.end(),
			[&layers](const std::string_view assetId)
			{
				return 1u == static_cast<size_t>(std::count_if(
					layers.begin(), layers.end(),
					[assetId](const VALTAN_SKY_PRESENTATION_LAYER& layer)
					{
						return std::string_view(layer.strAssetId) == assetId;
					}));
			});
	};
	constexpr std::array<std::string_view, VALTAN_SKY_LAYER_COUNT>
		RED_CLOUD_ASSET_IDS = {
			"VALTAN_PHASE_CHAOS_CLOUD",
			"VALTAN_PHASE_CHAOS_ELECTRIC",
			"VALTAN_PHASE_CHAOS_RING"
		};
	constexpr std::array<std::string_view, VALTAN_SKY_LAYER_COUNT>
		BLACK_APERTURE_ASSET_IDS = {
			"VALTAN_PHASE_SPACEHOLE_CLOUD",
			"VALTAN_PHASE_SPACEHOLE_CORE",
			"VALTAN_PHASE_SPACEHOLE_STREAK"
		};
	if (!hasExactAssetSet(stagedRedCloudLayers, RED_CLOUD_ASSET_IDS) ||
		!hasExactAssetSet(stagedBlackApertureLayers,
			BLACK_APERTURE_ASSET_IDS))
	{
		outStatus = "Cinematic sky source groups do not contain the exact six Valtan phase assets";
		return false;
	}

	size_t visiblePolicyCount = 0u;
	auto attachPolicies = [&outStatus, &visiblePolicyCount](auto& layers)
	{
		for (VALTAN_SKY_PRESENTATION_LAYER& layer : layers)
		{
			const VALTAN_CINEMATIC_SKY_LAYER_POLICY* pPolicy =
				CValtanCinematicCameraController::Find_SkyLayerPolicy(
					layer.strAssetId);
			if (nullptr == pPolicy || pPolicy->strAssetId != layer.strAssetId ||
				!std::isfinite(pPolicy->fAbsoluteUniformScale) ||
				!std::isfinite(pPolicy->fOpacityMultiplier) ||
				!std::isfinite(pPolicy->fRotationMultiplier) ||
				pPolicy->fAbsoluteUniformScale <
					CValtanCinematicCameraController::
						SKY_MIN_ABSOLUTE_UNIFORM_SCALE ||
				pPolicy->fAbsoluteUniformScale >
					CValtanCinematicCameraController::
						SKY_MAX_ABSOLUTE_UNIFORM_SCALE ||
				pPolicy->fOpacityMultiplier < 0.f ||
				pPolicy->fOpacityMultiplier > 1.f ||
				pPolicy->eProfile >=
					VALTAN_CINEMATIC_SKY_LAYER_PROFILE::END ||
				pPolicy->isPresentationVisible !=
					(VALTAN_CINEMATIC_SKY_LAYER_PROFILE::NONE !=
						pPolicy->eProfile))
			{
				outStatus = "Cinematic sky layer policy is missing or invalid: " +
					layer.strAssetId;
				return false;
			}
			layer.Policy = *pPolicy;
			if (pPolicy->isPresentationVisible)
				++visiblePolicyCount;
		}
		return true;
	};
	if (!attachPolicies(stagedRedCloudLayers) ||
		!attachPolicies(stagedBlackApertureLayers) ||
		4u != visiblePolicyCount)
	{
		if (outStatus.empty())
			outStatus = "Cinematic sky policy must expose exactly four radial layers";
		return false;
	}
	const VALTAN_CINEMATIC_SKY_LAYER_POLICY* pElectricPolicy =
		CValtanCinematicCameraController::Find_SkyLayerPolicy(
			"VALTAN_PHASE_CHAOS_ELECTRIC");
	const VALTAN_CINEMATIC_SKY_LAYER_POLICY* pStreakPolicy =
		CValtanCinematicCameraController::Find_SkyLayerPolicy(
			"VALTAN_PHASE_SPACEHOLE_STREAK");
	if (nullptr == pElectricPolicy || pElectricPolicy->isPresentationVisible ||
		nullptr == pStreakPolicy || pStreakPolicy->isPresentationVisible)
	{
		outStatus = "Cinematic sky raw electric and streak layers must stay hidden";
		return false;
	}

	/* Resolve every dependency before changing the currently committed set. */
	Clear_ValtanSkyPresentation();
	m_ValtanRedCloudLayers = std::move(stagedRedCloudLayers);
	m_ValtanBlackApertureLayers = std::move(stagedBlackApertureLayers);
	m_strValtanRedCloudSeedAssetId = redCloudSeed;
	m_strValtanBlackApertureSeedAssetId = blackApertureSeed;
	Reset_ValtanSkyPresentation();
	outStatus = "Cinematic sky presentation cached four masked radial layers";
	return true;
}

void CLevel_ValtanArena::Update_CinematicCamera(const f32_t fTimeDelta)
{
	const VALTAN_PRESENTATION_STATE& boss =
		m_Replication.Get_ValtanPresentationState();
	VALTAN_CINEMATIC_CAMERA_INPUT input{};
	/* Death is no longer a reason to stop: it selects the clear shot instead,
	   and the same restore path still runs when that cue finishes, on level exit
	   and on disconnect. */
	input.isBossDead = boss.isValid &&
		LostArk::Shared::WORLD_ENTITY_ACTION::DEAD == boss.eAction;
	input.isValid = boss.isValid;
	input.iNetEntityId = boss.iNetEntityId;
	input.iServerTick = boss.iServerTick;
	input.strPatternId = boss.strPatternId;
	input.strStageActionId = boss.strActionId;
	input.iPatternSequence = boss.iPatternSequence;
	input.iStageIndex = boss.iPatternStageIndex;
	input.iActionStartTick = boss.iActionStartTick;

	/* One authoritative tuple drives both layers, so the sky can never run on a
	   clock of its own. It is resolved before the camera early-outs because the
	   sky is authored on stages that carry no camera cue. */
	m_ValtanSkyState = m_ValtanCinematicCameraController.Resolve_SkyState(
		input, fTimeDelta);
	Apply_ValtanSkyPresentation(m_ValtanSkyState);
	if (nullptr == m_pCamera)
	{
		End_CinematicCameraOverride();
		return;
	}

	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!m_ValtanCinematicCameraController.Update(input, fTimeDelta, pose))
	{
		/* A sky-authored stage is allowed to have no camera cue. Restore only
		   the camera and leave the independently resolved sky state intact. */
		End_CinematicCameraOverride();
		return;
	}

#ifdef _DEBUG
	/* A Server-authored cue always preempts the local comparison aid before it
	tries to acquire the single camera presentation owner. */
	End_ReferenceCamera(false);
#endif

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
			End_CinematicCamera();
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

void CLevel_ValtanArena::Apply_ValtanSkyPresentation(
	const VALTAN_CINEMATIC_SKY_STATE& state)
{
	if (!state.isActive || state.strCueId.empty() ||
		state.strRedCloudAssetId != m_strValtanRedCloudSeedAssetId ||
		state.strBlackApertureAssetId !=
			m_strValtanBlackApertureSeedAssetId ||
		!std::isfinite(state.fCloudOpacity) ||
		!std::isfinite(state.fApertureScale) ||
		!std::isfinite(state.fCloudRotationDegrees))
	{
		Reset_ValtanSkyPresentation();
		return;
	}

	const f32_t cloudOpacity =
		(std::clamp)(state.fCloudOpacity, 0.f, 1.f);
	const f32_t apertureScale =
		(std::clamp)(state.fApertureScale, 0.f, 1.1f);
	const f32_t apertureOpacity =
		(std::clamp)(apertureScale, 0.f, 1.f);
	constexpr f32_t VISIBLE_EPSILON = 0.001f;
	const VALTAN_CINEMATIC_SKY_PRESENTATION_FRAME& frame =
		CValtanCinematicCameraController::Get_SkyPresentationFrame();

	const auto toMapProfile = [](
		const VALTAN_CINEMATIC_SKY_LAYER_PROFILE profile)
	{
		switch (profile)
		{
		case VALTAN_CINEMATIC_SKY_LAYER_PROFILE::DARK_APERTURE:
			return CMapAssetObject::PRESENTATION_VORTEX_PROFILE::DARK_APERTURE;
		case VALTAN_CINEMATIC_SKY_LAYER_PROFILE::RED_RING:
			return CMapAssetObject::PRESENTATION_VORTEX_PROFILE::RED_RING;
		case VALTAN_CINEMATIC_SKY_LAYER_PROFILE::RED_CLOUD_DISC:
			return CMapAssetObject::PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC;
		default:
			return CMapAssetObject::PRESENTATION_VORTEX_PROFILE::NONE;
		}
	};

	auto applyGroup = [
		&frame, &toMapProfile,
		cloudOpacity, apertureOpacity, apertureScale,
		visibleEpsilon = VISIBLE_EPSILON,
		rotationDegrees = state.fCloudRotationDegrees](auto& layers)
	{
		for (VALTAN_SKY_PRESENTATION_LAYER& layer : layers)
		{
			if (nullptr == layer.pObject)
				continue;

			const VALTAN_CINEMATIC_SKY_LAYER_POLICY& policy = layer.Policy;
			const CMapAssetObject::PRESENTATION_VORTEX_PROFILE mapProfile =
				toMapProfile(policy.eProfile);
			const f32_t sourceOpacity =
				VALTAN_CINEMATIC_SKY_LAYER_PROFILE::DARK_APERTURE ==
					policy.eProfile ? apertureOpacity : cloudOpacity;
			const f32_t opacity = (std::clamp)(
				sourceOpacity * policy.fOpacityMultiplier, 0.f, 1.f);
			const bool_t visible = policy.isPresentationVisible &&
				CMapAssetObject::PRESENTATION_VORTEX_PROFILE::NONE !=
					mapProfile &&
				opacity > visibleEpsilon;
			if (!visible)
			{
				layer.pObject->Set_Visible(false);
				layer.pObject->Set_PresentationOpacityMultiplier(1.f);
				layer.pObject->Set_PresentationVortexProfile(
					CMapAssetObject::PRESENTATION_VORTEX_PROFILE::NONE,
					0.f);
				layer.pObject->Set_PlacementTransform(
					layer.vBasePosition,
					layer.vBaseRotationQuaternion,
					layer.vBaseSignedScale);
				continue;
			}

			const vector_t facingQuaternion =
				XMLoadFloat4(&frame.vFacingQuaternion);
			const vector_t yawQuaternion = XMQuaternionRotationRollPitchYaw(
				0.f,
				XMConvertToRadians(
					rotationDegrees * policy.fRotationMultiplier),
				0.f);
			float4_t presentationQuaternion{};
			/* DirectXMath defines Multiply(Q1, Q2) as Q2*Q1: face every
			   plane toward the camera first, then rotate it around world Y. */
			XMStoreFloat4(&presentationQuaternion,
				XMQuaternionMultiply(facingQuaternion, yawQuaternion));

			f32_t uniformScale = policy.fAbsoluteUniformScale;
			if (policy.scalesWithAperture)
			{
				/* Keep the dark centre readable during the TAKEOFF fade while
				   preserving the authored aperture growth through DROP. */
				uniformScale *= 0.5f + 0.5f * apertureScale;
			}
			uniformScale = (std::clamp)(uniformScale,
				CValtanCinematicCameraController::
					SKY_MIN_ABSOLUTE_UNIFORM_SCALE,
				CValtanCinematicCameraController::
					SKY_MAX_ABSOLUTE_UNIFORM_SCALE);
			const float3_t presentationScale(
				uniformScale, uniformScale, uniformScale);
			const float3_t presentationPosition(
				frame.vAnchor.x,
				frame.vAnchor.y,
				frame.vAnchor.z);
			layer.pObject->Set_PresentationVortexProfile(mapProfile, 1.f);
			layer.pObject->Set_PlacementTransform(
				presentationPosition,
				presentationQuaternion,
				presentationScale);
			layer.pObject->Set_PresentationOpacityMultiplier(opacity);
			layer.pObject->Set_Visible(true);
		}
	};

	applyGroup(m_ValtanRedCloudLayers);
	applyGroup(m_ValtanBlackApertureLayers);
}

void CLevel_ValtanArena::Reset_ValtanSkyPresentation()
{
	auto resetGroup = [](auto& layers)
	{
		for (VALTAN_SKY_PRESENTATION_LAYER& layer : layers)
		{
			if (nullptr == layer.pObject)
				continue;
			/* Hide first: an object already queued by Late_Update must fail its
			   Render re-check before its base transform is restored. */
			layer.pObject->Set_Visible(false);
			layer.pObject->Set_PresentationOpacityMultiplier(1.f);
			layer.pObject->Set_PresentationVortexProfile(
				CMapAssetObject::PRESENTATION_VORTEX_PROFILE::NONE, 0.f);
			layer.pObject->Set_PlacementTransform(
				layer.vBasePosition,
				layer.vBaseRotationQuaternion,
				layer.vBaseSignedScale);
			layer.pObject->Set_Visible(layer.bBaseVisible);
		}
	};
	resetGroup(m_ValtanRedCloudLayers);
	resetGroup(m_ValtanBlackApertureLayers);
}

void CLevel_ValtanArena::Clear_ValtanSkyPresentation()
{
	Reset_ValtanSkyPresentation();
	m_ValtanRedCloudLayers = {};
	m_ValtanBlackApertureLayers = {};
	m_strValtanRedCloudSeedAssetId.clear();
	m_strValtanBlackApertureSeedAssetId.clear();
}

void CLevel_ValtanArena::End_CinematicCameraOverride()
{
	if (m_bCinematicCameraApplied && nullptr != m_pCamera)
	{
		if (0u != m_iCinematicCameraOwnerId)
			m_pCamera->End_PresentationOverride(m_iCinematicCameraOwnerId);
		m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
		m_pCamera->Set_FollowEnabled(m_bCinematicRestoreFollowRequested);
	}
	m_pCinematicRestoreTarget.reset();
	m_bCinematicRestoreFollowRequested = false;
	m_bCinematicCameraApplied = false;
	m_iCinematicCameraOwnerId = 0u;
}

void CLevel_ValtanArena::End_CinematicCamera()
{
	/* Disconnect, presentation abort and level exit all restore the cached map
	   layers. Ordinary camera-cue completion uses the narrower helper above. */
	m_ValtanSkyState = VALTAN_CINEMATIC_SKY_STATE{};
	Reset_ValtanSkyPresentation();
	End_CinematicCameraOverride();
}

HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PlayerNameplateView.Render(m_NameplatePlayers);

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(TEXT("Valtan Arena Map"));
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
#ifdef _DEBUG
		if (m_bReferenceCameraApplied)
		{
			m_pReferenceCameraRestoreTarget.reset();
			m_pCamera->Set_FollowTarget(nullptr);
			m_pCamera->Set_FollowEnabled(false);
			return true;
		}
#endif
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
#ifdef _DEBUG
	if (m_bReferenceCameraApplied)
	{
		m_pReferenceCameraRestoreTarget = transform;
		m_pCamera->Set_FollowTarget(transform);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
#endif
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
