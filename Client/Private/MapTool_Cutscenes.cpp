#include "imgui.h"
#include "MapTool_Internal.h"
#include "WorldSequenceToolPanel.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "KakulArenaHiddenPlacements.h"
#include "DeployPropObject.h"
#include "ValtanCinematicCameraController.h"
#include "WorldSequenceObject.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include "Model.h"




void Client::CMapTool::Apply_CutsceneCameraTrack(const f32_t timeDelta)
{
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
		return;
	const EDITOR_CAMERA_SHOT* bound = nullptr;
	f32_t elapsedMs = 0.f;
	/* Holding a clock means the panel is judging one shot. Letting the
	   highest priority shot that happens to be playing win would show a
	   different cutscene than the one the editor has open. */
	const bool_t editingOneShot = 0.f <= m_fCutsceneScrubMs &&
		m_iSelectedCameraShot < m_CameraShots.size();
	for (const EDITOR_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.patternOnly && !editingOneShot) continue;
		if (editingOneShot &&
			&shot != &m_CameraShots[m_iSelectedCameraShot])
		{
			continue;
		}
		/* An explicit stage play owns this preview even if an earlier intro
		   or a held cutscene still has a live clock at a higher priority. */
		if (!editingOneShot && m_bMarioIntroRunning &&
			shot.sequenceInstanceId != m_MarioIntroInstanceId)
		{
			continue;
		}
		/* A bound shot with fewer than two keys is still a shot: the product
		   level holds its single pose for the cutscene, so the preview must
		   show the same thing instead of leaving the free camera alone. */
		if (shot.sequenceInstanceId.empty())
			continue;
		f32_t candidateMs = 0.f;
		if (!m_ArenaRisePlayer.Try_GetElapsedMs(
			shot.sequenceInstanceId, candidateMs))
		{
			continue;
		}
		if (nullptr == bound || shot.priority > bound->priority)
		{
			bound = &shot;
			elapsedMs = candidateMs;
		}
	}
	if (nullptr == bound)
	{
		End_CutsceneCameraTrack();
		return;
	}
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!Sample_ShotCameraTrack(*bound, elapsedMs, pose))
	{
		End_CutsceneCameraTrack();
		return;
	}
	if (!m_bCutsceneCameraHeld)
	{
		if (!camera->Begin_PresentationOverride(
			CAMERA_SHOT_PREVIEW_OWNER_ID,
			CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
		{
			/* Another preview of equal or higher priority owns the camera.
			   Say so; a track that silently does nothing looks broken. */
			m_CameraShotStatus = "Camera track cannot take the camera: "
				"another preview holds it (stop the walkthrough or the "
				"other tool first)";
			return;
		}
		/* Where the free camera stands right now is where the glide starts,
		   exactly as the product level starts from its follow pose. */
		const float4_t* cameraPosition = CGameInstance::Get().Get_CamPosition();
		const float4x4_t* viewMatrix =
			CGameInstance::Get().Get_Transform(D3DTS::VIEW);
		if (nullptr != cameraPosition && nullptr != viewMatrix)
		{
			m_vCutsceneCameraFromEye = float3_t(
				cameraPosition->x, cameraPosition->y, cameraPosition->z);
			const vector_t forward = XMVector3Normalize(XMVectorSet(
				viewMatrix->_13, viewMatrix->_23, viewMatrix->_33, 0.f));
			XMStoreFloat3(&m_vCutsceneCameraFromLook,
				XMLoadFloat3(&m_vCutsceneCameraFromEye) + forward * 10.f);
		}
		else
		{
			m_vCutsceneCameraFromEye = pose.vEye;
			m_vCutsceneCameraFromLook = pose.vLookAt;
		}
		m_fCutsceneCameraFromFov = pose.fFovYDegrees;
		m_fCutsceneCameraBlendSeconds = 0.f;
		m_bCutsceneCameraHeld = true;
	}
	m_fCutsceneCameraBlendSeconds += (std::max)(0.f, timeDelta);
	if (0 < bound->blendInMs)
	{
		VALTAN_CINEMATIC_CAMERA_POSE fromPose{};
		fromPose.vEye = m_vCutsceneCameraFromEye;
		fromPose.vLookAt = m_vCutsceneCameraFromLook;
		fromPose.fFovYDegrees = m_fCutsceneCameraFromFov;
		VALTAN_CINEMATIC_CAMERA_POSE blended{};
		if (CValtanCinematicCameraController::Sample_BoundedTransition(
			fromPose, pose, static_cast<uint32_t>(bound->blendInMs),
			m_fCutsceneCameraBlendSeconds, blended))
		{
			pose = blended;
		}
	}
	if (!(pose.hasUp ? camera->Apply_PresentationPoseWithUp(CAMERA_SHOT_PREVIEW_OWNER_ID,
		pose.vEye, pose.vLookAt, pose.vUp, pose.fFovYDegrees) :
		camera->Apply_PresentationPose(CAMERA_SHOT_PREVIEW_OWNER_ID, pose.vEye, pose.vLookAt, pose.fFovYDegrees)))
	{
		End_CutsceneCameraTrack();
	}
}

void Client::CMapTool::End_CutsceneCameraTrack()
{
	if (!m_bCutsceneCameraHeld)
		return;
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr != camera)
		camera->End_PresentationOverride(CAMERA_SHOT_PREVIEW_OWNER_ID);
	m_bCutsceneCameraHeld = false;
}

void Client::CMapTool::Apply_CutsceneArenaVisibility(const bool_t hidden)
{
	size_t applied = 0;
	size_t missing = 0;
	m_bCutsceneArenaHidden = hidden;
	if (hidden)
	{
		/* A second hide while one is already standing would capture the hidden
		   state as the thing to restore, and the arena would never come back. */
		if (!m_CutsceneArenaRestoreVisibility.empty())
		{
			m_Status = "Cutscene arena already hidden";
			return;
		}
		/* Culling boxes and other helpers are already invisible. Capture what
		   each placement was before hiding so restoring cannot reveal
		   something the Area never showed. */
		for (const uint64_t placementId : KAKUL_ARENA_HIDDEN_PLACEMENT_IDS)
		{
			const auto found = std::find_if(m_Placements.begin(),
				m_Placements.end(),
				[placementId](const PLACED_ENTRY& value)
				{
					return value.record.placementId == placementId;
				});
			bool_t wasVisible = false;
			if (m_Placements.end() == found ||
				!CMapPlacementRuntime::Try_GetRuntimeVisible(*found, wasVisible))
			{
				++missing;
				continue;
			}
			m_CutsceneArenaRestoreVisibility.emplace_back(placementId, wasVisible);
			if (!wasVisible)
				continue;
			if (!Set_RuntimeVisible(*found, false))
			{
				++missing;
				continue;
			}
			++applied;
		}
	}
	else
	{
		for (const auto& restore : m_CutsceneArenaRestoreVisibility)
		{
			const auto found = std::find_if(m_Placements.begin(),
				m_Placements.end(),
				[&restore](const PLACED_ENTRY& value)
				{
					return value.record.placementId == restore.first;
				});
			if (m_Placements.end() == found ||
				!Set_RuntimeVisible(*found, restore.second))
			{
				++missing;
				continue;
			}
			++applied;
		}
		m_CutsceneArenaRestoreVisibility.clear();
	}
	m_Status = (hidden ? "Cutscene arena hidden: " : "Cutscene arena restored: ") +
		std::to_string(applied) + " placements, " +
		std::to_string(missing) + " unavailable";
}

bool_t Client::CMapTool::Is_CutsceneOriginalPlaying() const
{
	const size_t prefixLength = strlen(KAKUL_ORIGINAL_INSTANCE_PREFIX);
	for (const WORLD_SEQUENCE_INSTANCE& instance :
		m_ArenaRisePlayer.Get_Document().Get_Instances())
	{
		if (0 != instance.instanceId.compare(0, prefixLength,
			KAKUL_ORIGINAL_INSTANCE_PREFIX))
		{
			continue;
		}
		if (m_ArenaRisePlayer.Is_Playing(instance.instanceId))
			return true;
	}
	return false;
}

void Client::CMapTool::Hide_CutsceneSet()
{
	/* The unfold ends where the arena stands, so the cutscene copies step aside
	   instead of overlapping it. */
	for (PLACED_ENTRY& entry : m_Placements)
	{
		const uint64_t placementId = entry.record.placementId;
		if (placementId < KAKUL_CUTSCENE_SET_FIRST_ID ||
			placementId >= KAKUL_CUTSCENE_SET_END_ID)
		{
			continue;
		}
		(void)Set_RuntimeVisible(entry, false);
	}
}

void Client::CMapTool::Release_CutsceneBookPreview(const uint64_t placementId)
{
	/* A finished animation instance keeps its authoring preview open, and a
	   prop under preview refuses every state change, so the next play could
	   not restore the book. Hand it back before touching its state. */
	const shared_ptr<CDeployPropObject> book = m_DeployRuntime.Find(placementId);
	if (nullptr != book)
		book->End_AnimationAuthoringPreview();
}

void Client::CMapTool::Update_CutsceneArenaRise(
	const f32_t fTimeDelta,
	const bool_t isMapAuthoringLevel)
{
	if (!isMapAuthoringLevel || !m_Catalog.Is_Ready())
		return;
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	/* Showing a hidden placement for the first time clones its model, so the
	   frame that starts a cutscene is long. Feeding that whole frame to the
	   sequence clock would skip most of the authored motion, so advance the
	   preview by at most one slow frame at a time. */
	if (0.f <= m_fCutsceneLoopStartMs && m_fCutsceneLoopEndMs >
		m_fCutsceneLoopStartMs)
	{
		/* Replay only the selected key's own stretch so its framing can be
		   judged over and over without replaying the whole cutscene. */
		m_ArenaRisePlayer.Set_Paused(true);
		f32_t next = ((std::max)(m_fCutsceneScrubMs, m_fCutsceneLoopStartMs)) +
			(std::min)(fTimeDelta, KAKUL_CUTSCENE_MAX_STEP_SECONDS) * 1000.f;
		if (next >= m_fCutsceneLoopEndMs)
			next = m_fCutsceneLoopStartMs;
		m_fCutsceneScrubMs = next;
		(void)m_ArenaRisePlayer.Seek_AllToMs(m_fCutsceneScrubMs, targets);
	}
	else if (0.f <= m_fCutsceneScrubMs)
	{
		/* Held on one frame: the clock is written, not advanced, so props,
		   boss and camera all show the same instant every frame. */
		m_ArenaRisePlayer.Set_Paused(true);
		(void)m_ArenaRisePlayer.Seek_AllToMs(m_fCutsceneScrubMs, targets);
	}
	else
	{
		m_ArenaRisePlayer.Set_Paused(false);
		/* Rewind before advancing: an instance that finishes inside Update is
		   dropped, and restarting it then would take its ending pose as the
		   new baseline. */
		Update_MarioSequenceLoop(
			(std::min)(fTimeDelta, KAKUL_CUTSCENE_MAX_STEP_SECONDS), targets);
		m_ArenaRisePlayer.Update(
			(std::min)(fTimeDelta, KAKUL_CUTSCENE_MAX_STEP_SECONDS), targets);
	}

	/* The original recycles its cutscene props on the frame the unfold ends and
	   lets the persistent arena stand in their place; do the same here. */
	if (m_bCutsceneOriginalRunning && !Is_CutsceneOriginalPlaying())
	{
		m_bCutsceneOriginalRunning = false;
		End_CutsceneCameraTrack();
		Hide_CutsceneSet();
		Apply_CutsceneArenaVisibility(false);
		m_Status = "Original cutscene finished: arena handed back";
	}
	/* The original cutscene is not the only clock a camera track can ride:
	   a stage intro has its own sequence. Drive the track whenever that
	   cutscene runs, and also while the editor is holding a clock for key
	   work, which is what Hold Cutscene and Play This Key set up. A plain
	   loop preview still leaves the free camera alone. */
	else if (m_bCutsceneOriginalRunning || 0.f <= m_fCutsceneScrubMs ||
		m_bMarioIntroRunning)
	{
		Apply_CutsceneCameraTrack(fTimeDelta);
	}

	/* The reference does not leave the book standing in the finished arena, so
	   despawn it shortly after every arena_rise instance has settled. A transform
	   track cannot target a Deploy prop, so the state is what controls its
	   visibility. */
	if (m_fCutsceneBookHoldMs < 0.f)
		return;
	for (uint32_t index = 0; index < KAKUL_ARENA_RISE_INSTANCE_COUNT; ++index)
	{
		char instanceId[64] = {};
		(void)snprintf(instanceId, sizeof(instanceId),
			"world.sequence.instance.arena_rise_%02u", index);
		if (m_ArenaRisePlayer.Is_Playing(instanceId))
		{
			m_fCutsceneBookHoldMs = 0.f;
			return;
		}
	}
	m_fCutsceneBookHoldMs += (std::max)(0.f, fTimeDelta) * 1000.f;
	if (m_fCutsceneBookHoldMs < KAKUL_BOOK_HOLD_AFTER_ARENA_MS)
		return;
	m_fCutsceneBookHoldMs = -1.f;
	if (!m_DeployRuntime.Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::DESPAWNED } }))
	{
		OutputDebugStringA(("[MapTool][CutsceneArena] book despawn failed: " +
			m_DeployRuntime.Get_Status() + "\n").c_str());
	}
}

bool_t Client::CMapTool::Play_CutsceneArenaRise()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	if (!targets.Is_Complete())
	{
		m_Status = "Arena rise needs a loaded Area";
		return false;
	}
	if (!m_bArenaRiseAreaLoaded)
	{
		if (!m_ArenaRisePlayer.Load_Area(KAKUL_AREA_ID, targets))
		{
			m_Status = "Arena rise document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
	}
	/* The generated instances are split by the 32-track template limit, so the
	   whole arena needs every one of them started on the same frame. Their own
	   startDelayMs staggers the rise from the floor upward. */
	m_ArenaRisePlayer.Stop_All(targets);
	size_t started = 0;
	size_t rejected = 0;
	Release_CutsceneBookPreview(KAKUL_BOOK_PLACEMENT_ID);
	/* The book carries its own covers and pages, so it is one asset. The arena
	   spreads out of it once the unfold is under way. Restore it first: a
	   previous run despawns it when the arena settles. */
	if (!m_DeployRuntime.Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::INTACT } }))
	{
		m_Status = "Arena rise could not restore the book: " +
			m_DeployRuntime.Get_Status();
		return false;
	}
	m_fCutsceneBookHoldMs = 0.f;
	if (m_ArenaRisePlayer.Play(KAKUL_BOOK_INSTANCE_ID, targets))
		++started;
	else
		++rejected;
	for (uint32_t index = 0; index < KAKUL_ARENA_RISE_INSTANCE_COUNT; ++index)
	{
		char instanceId[64] = {};
		(void)snprintf(instanceId, sizeof(instanceId),
			"world.sequence.instance.arena_rise_%02u", index);
		if (m_ArenaRisePlayer.Play(instanceId, targets))
			++started;
		else
			++rejected;
	}
	m_Status = "Arena rise started: " + std::to_string(started) +
		" instances, " + std::to_string(rejected) + " rejected";
	return 0 == rejected;
}

bool_t Client::CMapTool::Play_CutsceneOriginalRise()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	if (!targets.Is_Complete())
	{
		m_Status = "Original cutscene needs a loaded Area";
		return false;
	}
	if (!m_bArenaRiseAreaLoaded)
	{
		if (!m_ArenaRisePlayer.Load_Area(KAKUL_AREA_ID, targets))
		{
			m_Status = "Original cutscene document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
	}
	m_ArenaRisePlayer.Stop_All(targets);
	Release_CutsceneBookPreview(KAKUL_BOOK_PLACEMENT_ID);
	/* The replicated cutscene assembles on its own book, and the original
	   keeps that book under the finished arena, so restore it and leave the
	   arena_rise despawn timer disarmed. */
	if (!m_DeployRuntime.Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::INTACT } }))
	{
		m_Status = "Original cutscene could not restore the book: " +
			m_DeployRuntime.Get_Status();
		return false;
	}
	m_fCutsceneBookHoldMs = -1.f;
	std::vector<std::string> instanceIds;
	const size_t prefixLength = strlen(KAKUL_ORIGINAL_INSTANCE_PREFIX);
	for (const WORLD_SEQUENCE_INSTANCE& instance :
		m_ArenaRisePlayer.Get_Document().Get_Instances())
	{
		if (0 == instance.instanceId.compare(0, prefixLength,
			KAKUL_ORIGINAL_INSTANCE_PREFIX))
		{
			instanceIds.push_back(instance.instanceId);
		}
	}
	if (instanceIds.empty())
	{
		m_Status = "No original cutscene instances in this Area";
		return false;
	}
	size_t started = 0;
	std::string rejected;
	for (const std::string& instanceId : instanceIds)
	{
		if (m_ArenaRisePlayer.Play(instanceId, targets))
		{
			++started;
			continue;
		}
		if (!rejected.empty())
			rejected += ", ";
		rejected += instanceId.substr(prefixLength);
	}
	if (0 == started)
	{
		m_Status = "Original cutscene could not start: " + rejected;
		return false;
	}
	/* The arena the unfold builds stands here already, so clear it for the
	   cutscene set and take it back when the unfold is spent. */
	Apply_CutsceneArenaVisibility(true);
	m_bCutsceneOriginalRunning = true;
	m_Status = "Original cutscene started: " + std::to_string(started) +
		" / " + std::to_string(instanceIds.size()) +
		(rejected.empty() ? "" : "  rejected: " + rejected);
	return rejected.empty();
}

bool_t Client::CMapTool::Is_ShotCutsceneClockPlaying(
	const EDITOR_CAMERA_SHOT& shot) const
{
	/* A shot with no sequence has no clock, so its track can never
	   play. Reporting the Area cutscene instead would light up the key
	   buttons for a shot they cannot move. */
	if (shot.sequenceInstanceId.empty())
		return false;
	return m_ArenaRisePlayer.Is_Playing(shot.sequenceInstanceId);
}

bool_t Client::CMapTool::Ensure_ShotCutsceneClock(
	const EDITOR_CAMERA_SHOT& shot)
{
	if (shot.sequenceInstanceId.empty())
	{
		/* No sequence means no clock of its own. Starting the Area
		   cutscene here would preview a different shot entirely, so
		   report the gap instead of showing the wrong thing. */
		m_CameraShotStatus = "This shot names no Sequence Instance, so it has no clock to play: " + shot.shotId;
		return false;
	}
	if (m_ArenaRisePlayer.Is_Playing(shot.sequenceInstanceId))
		return true;
	/* The original cutscene owns prop and arena state besides its
	   clock, so a shot bound to it still starts through that path. */
	const size_t originalLength = strlen(KAKUL_ORIGINAL_INSTANCE_PREFIX);
	if (shot.sequenceInstanceId.size() >= originalLength &&
		0 == shot.sequenceInstanceId.compare(0, originalLength,
			KAKUL_ORIGINAL_INSTANCE_PREFIX))
	{
		return Play_CutsceneOriginalRise();
	}
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	if (!targets.Is_Complete())
	{
		m_CameraShotStatus = "Camera track needs a loaded Area";
		return false;
	}
	if (!m_bArenaRiseAreaLoaded)
	{
		if (!m_ArenaRisePlayer.Load_Area(KAKUL_AREA_ID, targets))
		{
			m_CameraShotStatus = "Sequence document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
	}
	/* Only this shot's own sequence starts. Stopping the others would
	   throw away whatever preview the editor already has running. */
	if (!m_ArenaRisePlayer.Play(shot.sequenceInstanceId, targets))
	{
		m_CameraShotStatus = "Camera track sequence could not start: " +
			shot.sequenceInstanceId;
		return false;
	}
	m_CameraShotStatus = "Camera track clock started: " +
		shot.sequenceInstanceId;
	return true;
}

vector<std::string> Client::CMapTool::Collect_MarioIntroStages() const
{
	/* A stage has an intro once a camera shot with a track is bound to
	   `world.sequence.instance.mario_<stage>_intro`. */
	static constexpr std::string_view INTRO_SUFFIX = "_intro";
	vector<std::string> stages;
	for (const EDITOR_CAMERA_SHOT& shot : m_CameraShots)
	{
		const std::string& id = shot.sequenceInstanceId;
		if (shot.keyframes.size() < 2u ||
			0 != id.rfind(MARIO_SEQUENCE_ROOT, 0) ||
			!id.ends_with(INTRO_SUFFIX) ||
			id.size() <= MARIO_SEQUENCE_ROOT.size() + INTRO_SUFFIX.size())
		{
			continue;
		}
		std::string stage = id.substr(MARIO_SEQUENCE_ROOT.size(),
			id.size() - MARIO_SEQUENCE_ROOT.size() - INTRO_SUFFIX.size());
		if (stages.end() == std::find(stages.begin(), stages.end(), stage))
			stages.push_back(std::move(stage));
	}
	std::sort(stages.begin(), stages.end());
	return stages;
}

bool_t Client::CMapTool::Play_MarioIntro(const std::string& stageToken)
{
	Stop_MarioIntro();
	const std::string instanceId =
		std::string(MARIO_SEQUENCE_ROOT) + stageToken + "_intro";
	const auto found = std::find_if(
		m_CameraShots.begin(), m_CameraShots.end(),
		[&instanceId](const EDITOR_CAMERA_SHOT& shot)
		{
			return shot.sequenceInstanceId == instanceId &&
				2u <= shot.keyframes.size();
		});
	if (m_CameraShots.end() == found)
	{
		m_MarioWalkStatus = stageToken +
			" has no intro camera shot bound to " + instanceId;
		return false;
	}
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	if (!targets.Is_Complete())
	{
		m_MarioWalkStatus = "Stage intro needs a loaded Area";
		return false;
	}
	if (!m_bArenaRiseAreaLoaded)
	{
		if (!m_ArenaRisePlayer.Load_Area(KAKUL_AREA_ID, targets))
		{
			m_MarioWalkStatus = "Sequence document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
	}
	/* A held clock or a key loop would pin the sequence on one frame, and
	   the intro is meant to run through, so the editor's hold is released.
	   Play on an instance that is already running rewinds it to 0. */
	m_fCutsceneScrubMs = -1.f;
	m_fCutsceneLoopStartMs = -1.f;
	m_fCutsceneLoopEndMs = -1.f;
	if (!m_ArenaRisePlayer.Play(instanceId, targets))
	{
		m_MarioWalkStatus = "Stage intro could not start: " +
			m_ArenaRisePlayer.Get_Status();
		return false;
	}
	m_bMarioIntroRunning = true;
	m_MarioIntroInstanceId = instanceId;
	m_iSelectedCameraShot = static_cast<size_t>(found - m_CameraShots.begin());
	m_iCutsceneSelectedKey = -1;
	m_MarioWalkStatus = stageToken + " intro started: " + found->shotId +
		" (" + std::to_string(found->trackDurationMs) + " ms)";
	return true;
}

void Client::CMapTool::Update_MarioIntro()
{
	if (!m_bMarioIntroRunning)
		return;
	/* The camera is driven by Apply_CutsceneCameraTrack while this flag
	   holds its gate open; only the end of the run is watched here. */
	if (m_ArenaRisePlayer.Is_Playing(m_MarioIntroInstanceId))
		return;
	m_bMarioIntroRunning = false;
	End_CutsceneCameraTrack();
	m_MarioWalkStatus = "Intro finished: " +
		m_MarioIntroInstanceId;
}

void Client::CMapTool::Stop_MarioIntro()
{
	if (!m_bMarioIntroRunning)
		return;
	m_bMarioIntroRunning = false;
	End_CutsceneCameraTrack();
}

bool_t Client::CMapTool::Toggle_MarioSequenceLoop()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	if (!targets.Is_Complete())
	{
		m_MarioWalkStatus = "Sequence loop needs a loaded Area";
		return false;
	}
	if (m_bMarioSequenceLoopRunning)
	{
		/* Stopping mid sequence would leave the props wherever the lap got
		   to. Rewinding to the opening frame first puts the stage back in
		   the state a trigger is supposed to find it in, and does it without
		   reloading the Area, which would discard unsaved authoring. */
		(void)m_ArenaRisePlayer.Seek_AllToMs(0.f, targets);
		m_ArenaRisePlayer.Stop_All(targets);
		m_MarioLoopInstanceIds.clear();
		m_bMarioSequenceLoopRunning = false;
		m_MarioWalkStatus = "Sequence loop stopped at the opening frame";
		return true;
	}
	if (!m_bArenaRiseAreaLoaded)
	{
		if (!m_ArenaRisePlayer.Load_Area(KAKUL_AREA_ID, targets))
		{
			m_MarioWalkStatus = "Sequence document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
	}
	const size_t prefixLength = strlen(KAKUL_MARIO_INSTANCE_PREFIX);
	m_MarioLoopInstanceIds.clear();
	for (const WORLD_SEQUENCE_INSTANCE& instance :
		m_ArenaRisePlayer.Get_Document().Get_Instances())
	{
		if (instance.instanceId.size() < prefixLength ||
			0 != instance.instanceId.compare(0, prefixLength,
				KAKUL_MARIO_INSTANCE_PREFIX))
		{
			continue;
		}
		if (instance.enabled)
			m_MarioLoopInstanceIds.push_back(instance.instanceId);
	}
	if (m_MarioLoopInstanceIds.empty())
	{
		m_MarioWalkStatus = "No Mario sequences in this Area";
		return false;
	}
	/* A held scrub writes the clock instead of advancing it, which would
	   freeze the loop on one frame. */
	m_fCutsceneScrubMs = -1.f;
	m_fCutsceneLoopStartMs = -1.f;
	m_fCutsceneLoopEndMs = -1.f;
	size_t started = 0;
	std::string rejected;
	for (const std::string& instanceId : m_MarioLoopInstanceIds)
	{
		if (m_ArenaRisePlayer.Play(instanceId, targets))
		{
			++started;
			continue;
		}
		if (!rejected.empty())
			rejected += ", ";
		rejected += instanceId.substr(prefixLength);
	}
	if (0 == started)
	{
		m_MarioLoopInstanceIds.clear();
		m_MarioWalkStatus = "Sequence loop could not start: " + rejected;
		return false;
	}
	m_bMarioSequenceLoopRunning = true;
	m_MarioWalkStatus = "Sequence loop running: " + std::to_string(started) +
		" of " + std::to_string(m_MarioLoopInstanceIds.size()) + " sequences" +
		(rejected.empty() ? "" : ", unavailable: " + rejected);
	return true;
}

void Client::CMapTool::Update_MarioSequenceLoop(
	const f32_t fTimeDelta,
	const CWorldSequencePlayer::TARGET_SET& targets)
{
	if (!m_bMarioSequenceLoopRunning || !std::isfinite(fTimeDelta))
		return;
	const f32_t stepMs = (std::max)(0.f, fTimeDelta) * 1000.f;
	bool_t interrupted = false;
	for (const std::string& instanceId : m_MarioLoopInstanceIds)
	{
		const WORLD_SEQUENCE_INSTANCE* instance =
			m_ArenaRisePlayer.Get_Document().Find_Instance(instanceId);
		const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ?
			nullptr :
			m_ArenaRisePlayer.Get_Document().Find_Template(instance->templateId);
		if (nullptr == sequence)
			continue;
		f32_t elapsedMs = 0.f;
		if (!m_ArenaRisePlayer.Try_GetElapsedMs(instanceId, elapsedMs))
		{
			/* Something else took the player -- a cutscene preview stops every
			   instance. Playing again here would adopt the pose that stop left
			   behind as the new baseline and drift the props a little further
			   each lap, so end the loop and let it be started again. */
			interrupted = true;
			break;
		}
		const f32_t rewindAtMs =
			static_cast<f32_t>(sequence->durationMs) -
			KAKUL_MARIO_LOOP_REWIND_MARGIN_MS;
		if (elapsedMs + stepMs >= rewindAtMs)
		{
			/* Still active, so this rewinds the clock and keeps the baseline
			   the first start captured. */
			(void)m_ArenaRisePlayer.Play(instanceId, targets);
		}
	}
	if (interrupted)
	{
		m_MarioLoopInstanceIds.clear();
		m_bMarioSequenceLoopRunning = false;
		m_MarioWalkStatus = "Sequence loop ended: another preview took the sequence player";
	}
}

void Client::CMapTool::Render_CutsceneArenaPreview()
{
	if (!ImGui::CollapsingHeader("Cutscene Arena Preview"))
		return;
	ImGui::TextWrapped(
		"The pop-up book cutscene raises the tent arena, so the product level "
		"opens with these placements hidden. Authoring keeps them visible; this "
		"toggle previews the hidden state without changing any saved document.");
	ImGui::Text("Arena placements: %zu",
		KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.size());
	if (ImGui::Checkbox("Hide arena (cutscene start state)",
		&m_bCutsceneArenaHidden))
	{
		Apply_CutsceneArenaVisibility(m_bCutsceneArenaHidden);
	}
	ImGui::Separator();
	ImGui::TextWrapped(
		"Play raises the arena from the floor upward using the generated "
		"arena_rise instances. Hide it first, otherwise there is nothing to "
		"raise.");
	ImGui::BeginDisabled(!m_bCutsceneArenaHidden);
	if (ImGui::Button("Play arena rise"))
		(void)Play_CutsceneArenaRise();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Play 2 (original)"))
		(void)Play_CutsceneOriginalRise();
	ImGui::SameLine();
	if (ImGui::Button("Stop arena rise"))
	{
		CWorldSequencePlayer::TARGET_SET targets{};
		targets.levelIndex = m_iAuthoringLevelIndex;
		targets.pCatalog = &m_Catalog;
		targets.pPlacements = &m_Placements;
		targets.pDeployRuntime = &m_DeployRuntime;
		m_ArenaRisePlayer.Stop_All(targets);
		m_fCutsceneBookHoldMs = -1.f;
		(void)m_DeployRuntime.Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
			DEPLOY_PROP_STATE::DESPAWNED } });
		if (m_bCutsceneOriginalRunning)
		{
			m_bCutsceneOriginalRunning = false;
			End_CutsceneCameraTrack();
			Hide_CutsceneSet();
			Apply_CutsceneArenaVisibility(false);
		}
		m_Status = "Arena rise stopped";
	}
	ImGui::TextDisabled("%s", m_Status.c_str());

	ImGui::SeparatorText("Stage Intro Camera");
	ImGui::TextWrapped(
		"Plays one stage's intro camera shot from the top with no player and "
		"no Server. The shot bound to that stage's mario_<stage>_intro "
		"sequence drives the camera exactly as the product level will, then "
		"hands it back. A stage gets a button as soon as such a shot exists.");
	const vector<std::string> introStages = Collect_MarioIntroStages();
	ImGui::BeginDisabled(m_bMarioIntroRunning);
	if (introStages.empty())
	{
		ImGui::TextDisabled(
			"No camera shot is bound to a mario_<stage>_intro sequence yet.");
	}
	for (size_t index = 0; index < introStages.size(); ++index)
	{
		if (0u != index)
			ImGui::SameLine();
		const std::string label = "Play " + introStages[index];
		if (ImGui::Button(label.c_str()))
			(void)Play_MarioIntro(introStages[index]);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Stop Intro"))
	{
		Stop_MarioIntro();
		CWorldSequencePlayer::TARGET_SET targets{};
		targets.levelIndex = m_iAuthoringLevelIndex;
		targets.pCatalog = &m_Catalog;
		targets.pPlacements = &m_Placements;
		targets.pDeployRuntime = &m_DeployRuntime;
		m_ArenaRisePlayer.Stop_All(targets);
		m_MarioWalkStatus = "Intro stopped";
	}
	if (m_bMarioIntroRunning)
	{
		f32_t elapsedMs = 0.f;
		(void)m_ArenaRisePlayer.Try_GetElapsedMs(
			m_MarioIntroInstanceId, elapsedMs);
		ImGui::Text("%s at %.0f ms", m_MarioIntroInstanceId.c_str(),
			static_cast<double>(elapsedMs));
	}
	ImGui::SeparatorText("Sequence Loop");
	ImGui::TextWrapped(
		"Replays every authored Mario sequence for as long as it is on, so the motion a trigger is meant to start can be watched while the trigger box is placed. Editor preview only: nothing is saved, and stopping rewinds the props to the opening frame each stage begins on.");
	if (ImGui::Button(m_bMarioSequenceLoopRunning ?
		"Stop Sequence Loop" : "Loop All Mario Sequences"))
	{
		(void)Toggle_MarioSequenceLoop();
	}
	if (m_bMarioSequenceLoopRunning)
	{
		ImGui::SameLine();
		ImGui::Text("looping %zu sequences",
			m_MarioLoopInstanceIds.size());
	}
	ImGui::TextDisabled("%s", m_MarioWalkStatus.c_str());
	ImGui::SeparatorText("Card Maze March");
	ImGui::TextWrapped(
		"Runs the four card soldiers across the maze the way the march does: "
		"3 to 9, 6 to 12, 9 to 3, then 12 to 6 o'clock, one after another "
		"along the centre lanes. Editor preview only: nothing is saved and "
		"no Server is involved.");
	if (ImGui::Button("CardMiro_Play"))
		(void)Play_CardMiroMarch();
	ImGui::SameLine();
	if (ImGui::Button("CardMiro_Stop"))
		Stop_CardMiroMarch();
	ImGui::TextDisabled("%s", m_CardMiroMarchStatus.c_str());
	for (const std::string& instanceId :
		Collect_CardMiroMarchInstanceIds(m_ArenaRisePlayer.Get_Document()))
	{
		if (m_ArenaRisePlayer.Is_Playing(instanceId))
		{
			ImGui::TextDisabled("%s: %s", instanceId.c_str(),
				m_ArenaRisePlayer.Get_ObjectSampleStatus(instanceId).c_str());
		}
	}
}

bool_t Client::CMapTool::Play_CardMiroMarch()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	/* A world object builds its own model, which the placement previews
	   never ask for, so this is the path that needs the device. */
	targets.device = m_pDevice;
	targets.context = m_pContext;
	if (!targets.Is_Complete() || nullptr == m_pWorldSequenceToolPanel ||
		!m_pWorldSequenceToolPanel->Is_Ready())
	{
		m_CardMiroMarchStatus =
			"Card maze march needs a loaded Area with its world sequences";
		return false;
	}
	/* The arena Level registers this prototype for itself; the editor
	   Level does not, so without it the clone fails and nothing appears. */
	if (!m_bWorldObjectPrototypeReady)
	{
		if (FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex, CWorldSequenceObject::PROTOTYPE_TAG,
			CWorldSequenceObject::Create(m_pDevice, m_pContext))))
		{
			m_CardMiroMarchStatus =
				"World object prototype registration failed";
			return false;
		}
		m_bWorldObjectPrototypeReady = true;
	}
	/* Admit the edited document rather than the file, so a march can be
	   checked before Save. */
	std::string status;
	if (!m_ArenaRisePlayer.Set_Document(
		m_pWorldSequenceToolPanel->Get_Document(), targets, status))
	{
		m_CardMiroMarchStatus = status;
		return false;
	}
	m_bArenaRiseAreaLoaded = true;
	m_fCutsceneScrubMs = m_fCutsceneLoopStartMs = m_fCutsceneLoopEndMs = -1.f;
	m_ArenaRisePlayer.Set_Paused(false);
	const std::vector<std::string> marchIds =
		Collect_CardMiroMarchInstanceIds(m_ArenaRisePlayer.Get_Document());
	if (marchIds.empty())
	{
		m_CardMiroMarchStatus =
			"This Area authors no card maze march instances";
		return false;
	}
	for (const std::string& instanceId : marchIds)
	{
		if (!m_ArenaRisePlayer.Play(instanceId, targets))
		{
			m_CardMiroMarchStatus = m_ArenaRisePlayer.Get_Status();
			return false;
		}
	}
	m_CardMiroMarchStatus =
		"Card maze march started: 3->9, 6->12, 9->3, 12->6";
	return true;
}

void Client::CMapTool::Stop_CardMiroMarch()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &m_Placements;
	targets.pDeployRuntime = &m_DeployRuntime;
	targets.device = m_pDevice;
	targets.context = m_pContext;
	for (const std::string& instanceId :
		Collect_CardMiroMarchInstanceIds(m_ArenaRisePlayer.Get_Document()))
		m_ArenaRisePlayer.Stop_Instance(instanceId, targets, true);
	m_CardMiroMarchStatus = "Card maze march stopped";
}

void Client::CMapTool::Render_AnimatedPropsAuthoring()
{
	if (!ImGui::CollapsingHeader("Animated Props (Deploy ANIM)",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active || active->sourceDeployCatalog.empty() ||
		active->sourceDeployPlacements.empty())
	{
		ImGui::TextDisabled(
			"This Area declares no Deploy prop catalog, so animated props cannot be authored here.");
		return;
	}
	if (!m_DeployRuntime.Get_Catalog().Is_Ready())
	{
		ImGui::TextWrapped("DeployProp catalog is not loaded: %s",
			m_DeployRuntime.Get_Status().c_str());
		if (ImGui::Button("Reload Deploy Props"))
			(void)Load_DeployProps();
		return;
	}

	ImGui::TextDisabled(
		"Only cooked .wmodel assets registered in the Area catalog are listed; raw glTF/PSA bundles are never runtime assets.");
	/* Korean: only cooked .wmodel assets registered in the catalog appear
	   here, and a raw glTF/PSA bundle cannot be placed. MapTool.cpp compiles
	   without /utf-8, so operator help is written as explicit UTF-8 bytes. */
	static const char_t* const ANIMATED_PROP_HELP_ASSETS =
		"\xEC\xB9\xB4\xED\x83\x88\xEB\xA1\x9C\xEA\xB7\xB8\xEC\x97\x90 "
		"\xEB\x93\xB1\xEB\xA1\x9D\xEB\x90\x9C \xEC\xA1\xB0\xEB\xA6\xAC "
		"\xEC\x99\x84\xEB\xA3\x8C .wmodel \xEC\x9E\x90\xEC\x82\xB0\xEB"
		"\xA7\x8C \xEB\xB3\xB4\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4. \xEC"
		"\x9B\x90\xEB\xB3\xB8 glTF/PSA\xEB\x8A\x94 \xEB\xB0\xB0\xEC\xB9"
		"\x98\xED\x95\xA0 \xEC\x88\x98 \xEC\x97\x86\xEC\x8A\xB5\xEB\x8B"
		"\x88\xEB\x8B\xA4.";
	/* Korean: arm placement, then click the ground in the viewport to drop
	   the prop there; Esc cancels. */
	static const char_t* const ANIMATED_PROP_HELP_ARM =
		"\xEB\xB0\xB0\xEC\xB9\x98\xEB\xA5\xBC \xEC\x8B\x9C\xEC\x9E\x91"
		"\xED\x95\x9C \xEB\x92\xA4 \xEB\xB7\xB0\xED\x8F\xAC\xED\x8A\xB8"
		"\xEC\x97\x90\xEC\x84\x9C \xEC\xA7\x80\xEB\xA9\xB4\xEC\x9D\x84 "
		"\xED\x81\xB4\xEB\xA6\xAD\xED\x95\x98\xEB\xA9\xB4 \xEA\xB7\xB8 "
		"\xEC\x9E\x90\xEB\xA6\xAC\xEC\x97\x90 \xEB\x86\x93\xEC\x9E\x85"
		"\xEB\x8B\x88\xEB\x8B\xA4. Esc\xEB\xA1\x9C \xEC\xB7\xA8\xEC\x86"
		"\x8C\xED\x95\xA9\xEB\x8B\x88\xEB\x8B\xA4.";
	/* Korean: only PROJECT rows authored here can be edited or removed;
	   SOURCE rows extracted from the original data are read-only. */
	static const char_t* const ANIMATED_PROP_HELP_PLACED =
		"\xEC\x97\xAC\xEA\xB8\xB0\xEC\x84\x9C \xEB\xA7\x8C\xEB\x93\xA0 PR"
		"OJECT \xEB\xB0\xB0\xEC\xB9\x98\xEB\xA7\x8C \xED\x8E\xB8\xEC\xA7"
		"\x91\xEA\xB3\xBC \xEC\x82\xAD\xEC\xA0\x9C\xEA\xB0\x80 \xEB\x90"
		"\x98\xEA\xB3\xA0 SOURCE \xEB\xB0\xB0\xEC\xB9\x98\xEB\x8A\x94 "
		"\xEC\x9D\xBD\xEA\xB8\xB0 \xEC\xA0\x84\xEC\x9A\xA9\xEC\x9E\x85"
		"\xEB\x8B\x88\xEB\x8B\xA4.";
	/* Korean: Apply is what writes the draft into the world object and the
	   authoring document. */
	static const char_t* const ANIMATED_PROP_HELP_APPLY =
		"Apply\xEB\xA5\xBC \xEB\x88\x8C\xEB\x9F\xAC\xEC\x95\xBC \xEC\x8B"
		"\xA4\xEC\xA0\x9C \xEC\x9B\x94\xEB\x93\x9C \xEC\x98\xA4\xEB\xB8"
		"\x8C\xEC\xA0\x9D\xED\x8A\xB8\xEC\x99\x80 \xEB\xAC\xB8\xEC\x84"
		"\x9C\xEC\x97\x90 \xEB\xB0\x98\xEC\x98\x81\xEB\x90\xA9\xEB\x8B"
		"\x88\xEB\x8B\xA4.";
	/* Korean: Save replaces only the Area .deployplacements source; runtime
	   uptake is the separate publish step. */
	static const char_t* const ANIMATED_PROP_HELP_SAVE =
		"Save\xEB\x8A\x94 Area\xEC\x9D\x98 .deployplacements \xEC\x9B\x90"
		"\xEB\xB3\xB8\xEB\xA7\x8C \xEA\xB5\x90\xEC\xB2\xB4\xED\x95\xA9"
		"\xEB\x8B\x88\xEB\x8B\xA4. \xEB\x9F\xB0\xED\x83\x80\xEC\x9E\x84 "
		"\xEB\xB0\x98\xEC\x98\x81\xEC\x9D\x80 publish \xEB\x8B\xA8\xEA"
		"\xB3\x84\xEC\x9E\x85\xEB\x8B\x88\xEB\x8B\xA4.";

	const DEPLOY_PROP_ASSET_ENTRY* selectedAsset = Get_SelectedDeployAsset();
	if (nullptr == selectedAsset ||
		DEPLOY_PROP_MODEL_KIND::ANIM != selectedAsset->kind)
	{
		m_bAnimatedPropPlacementArmed = false;
	}
	const DEPLOY_RUNTIME_ENTRY* selectedPlacement = Get_SelectedAnimatedProp();
	const bool_t hasSelectedPlacement = nullptr != selectedPlacement;
	const bool_t isSelectionEditable = hasSelectedPlacement &&
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED ==
			selectedPlacement->placement.provenance;
	if (hasSelectedPlacement && m_iAnimatedPropDraftPlacementId !=
		selectedPlacement->placement.runtimePlacementId)
	{
		Sync_AnimatedPropTransformDraft();
	}
	else if (!hasSelectedPlacement && 0u != m_iAnimatedPropDraftPlacementId)
	{
		Sync_AnimatedPropTransformDraft();
	}

	if (ImGui::BeginTable("##animated-prop-authoring", 2,
		ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Catalog Assets",
			ImGuiTableColumnFlags_WidthStretch, 1.f);
		ImGui::TableSetupColumn("Placed Animated Props",
			ImGuiTableColumnFlags_WidthStretch, 1.f);
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputTextWithHint("##animated-prop-filter", "Filter assets",
			m_AnimatedPropFilter, std::size(m_AnimatedPropFilter));
		ImGui::BeginChild("AnimatedPropAssets", ImVec2(0.f, 170.f), true);
		size_t listedAssets = 0;
		for (const DEPLOY_PROP_ASSET_ENTRY& asset :
			m_DeployRuntime.Get_Catalog().Get_Assets())
		{
			if (DEPLOY_PROP_MODEL_KIND::ANIM != asset.kind ||
				!MatchesAnimatedPropFilter(
					asset.label, asset.id, m_AnimatedPropFilter))
			{
				continue;
			}
			++listedAssets;
			ImGui::PushID(asset.id.c_str());
			if (ImGui::Selectable(asset.label.c_str(),
				asset.id == m_SelectedDeployAssetId))
			{
				m_SelectedDeployAssetId = asset.id;
				m_Status = "Selected animated prop asset " + asset.id;
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			{
				ImGui::SetTooltip(
					"%s\ncooked model: %s\nintact clip: %s\nfractured clip: %s\n%s",
					asset.id.c_str(),
					asset.intactRelativePath.generic_string().c_str(),
					asset.animationRoles.intactClip.empty() ?
						"(none)" : asset.animationRoles.intactClip.c_str(),
					asset.animationRoles.fracturedClip.empty() ?
						"(none)" : asset.animationRoles.fracturedClip.c_str(),
					asset.evidence.c_str());
			}
			ImGui::PopID();
		}
		if (0 == listedAssets)
			ImGui::TextDisabled("No cooked ANIM asset matches the filter.");
		ImGui::EndChild();
		ShowAuthoringHelp(ANIMATED_PROP_HELP_ASSETS);
		ImGui::BeginDisabled(nullptr == selectedAsset);
		if (ImGui::Button(m_bAnimatedPropPlacementArmed ?
			"Cancel Placement" : "Place In Viewport"))
		{
			m_bAnimatedPropPlacementArmed = !m_bAnimatedPropPlacementArmed;
			m_Status = m_bAnimatedPropPlacementArmed ?
				"Animated prop placement armed. Click the ground in the viewport." :
				"Animated prop placement cancelled";
		}
		ImGui::EndDisabled();
		ShowAuthoringHelp(ANIMATED_PROP_HELP_ARM);

		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("AnimatedPropPlacements", ImVec2(0.f, 190.f), true);
		size_t listedPlacements = 0;
		for (const DEPLOY_RUNTIME_ENTRY& entry : m_DeployRuntime.Get_Entries())
		{
			const DEPLOY_PROP_ASSET_ENTRY* asset =
				m_DeployRuntime.Get_Catalog().Find(entry.placement.assetId);
			if (nullptr == asset ||
				DEPLOY_PROP_MODEL_KIND::ANIM != asset->kind)
			{
				continue;
			}
			++listedPlacements;
			const bool_t authored =
				DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED ==
				entry.placement.provenance;
			const std::string label = "#" +
				std::to_string(entry.placement.runtimePlacementId) + "  " +
				asset->label + (authored ? "  [PROJECT]" : "  [SOURCE]");
			ImGui::PushID(reinterpret_cast<void*>(static_cast<uintptr_t>(
				entry.placement.runtimePlacementId)));
			if (ImGui::Selectable(label.c_str(),
				m_iSelectedAnimatedPropPlacementId ==
					entry.placement.runtimePlacementId))
			{
				m_iSelectedAnimatedPropPlacementId =
					entry.placement.runtimePlacementId;
				Sync_AnimatedPropTransformDraft();
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			{
				std::string clipSummary;
				if (nullptr != entry.object)
				{
					for (const DEPLOY_PROP_ANIMATION_CLIP& clip :
						entry.object->Get_AnimationClips())
					{
						clipSummary += "\n  " + clip.name + "  " +
							std::to_string(clip.durationSeconds) + " s";
					}
				}
				if (clipSummary.empty())
					clipSummary = "\n  (no clip is readable on this instance)";
				ImGui::SetTooltip("%s\n%s\nclips:%s",
					entry.placement.assetId.c_str(),
					entry.placement.sourcePlacementId.c_str(),
					clipSummary.c_str());
			}
			ImGui::PopID();
		}
		if (0 == listedPlacements)
		{
			ImGui::TextDisabled(
				"No animated prop is placed in this Area yet.");
		}
		ImGui::EndChild();
		ShowAuthoringHelp(ANIMATED_PROP_HELP_PLACED);
		ImGui::EndTable();
	}

	ImGui::BeginDisabled(!isSelectionEditable);
	bool_t transformEdited = false;
	ImGui::DragFloat3("Position##animated-prop",
		&m_AnimatedPropDraftPosition.x, 0.05f);
	transformEdited |= ImGui::IsItemDeactivatedAfterEdit();
	float3_t rotationDegrees =
		DeployQuaternionToEulerDegrees(m_AnimatedPropDraftRotation);
	if (ImGui::DragFloat3("Rotation (deg)##animated-prop",
		&rotationDegrees.x, 0.25f, -360.f, 360.f, "%.1f"))
	{
		m_AnimatedPropDraftRotation =
			DeployEulerDegreesToQuaternion(rotationDegrees);
	}
	transformEdited |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::DragFloat("Uniform Scale##animated-prop",
		&m_fAnimatedPropDraftScale, 0.01f, 0.01f, 100.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	transformEdited |= ImGui::IsItemDeactivatedAfterEdit();
	if (transformEdited)
		(void)Apply_AnimatedPropTransform();
	if (ImGui::Button("Apply Transform"))
		(void)Apply_AnimatedPropTransform();
	ShowAuthoringHelp(ANIMATED_PROP_HELP_APPLY);
	ImGui::SameLine();
	if (ImGui::Button("Revert Draft"))
		Sync_AnimatedPropTransformDraft();
	ImGui::SameLine();
	if (ImGui::Button("Remove Placement"))
		(void)Remove_SelectedAnimatedProp();
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(!hasSelectedPlacement);
	if (ImGui::Button("Focus"))
	{
		const DEPLOY_RUNTIME_ENTRY* target = Get_SelectedAnimatedProp();
		float3_t center{};
		float3_t halfExtents{};
		shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
		if (nullptr == camera && Find_AssetTestCamera())
			camera = m_pAssetTestCamera.lock();
		if (nullptr == target || nullptr == target->object ||
			!target->object->Get_WorldBounds(center, halfExtents))
		{
			m_Status =
				"Animated prop focus needs a placed prop with model bounds";
		}
		else if (nullptr == camera)
		{
			m_Status = "ASSET_TEST camera is unavailable";
		}
		else
		{
			const f32_t radius = (std::max)(2.f,
				(std::max)(halfExtents.x, halfExtents.z) * 3.f);
			camera->Frame_Area(center, radius);
			m_Status = "Camera framed animated prop #" +
				std::to_string(target->placement.runtimePlacementId);
		}
	}
	ImGui::EndDisabled();
	if (hasSelectedPlacement && !isSelectionEditable)
	{
		ImGui::TextDisabled(
			"The selected placement came from the source extraction and is read-only.");
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bDeployDirty);
	if (ImGui::Button("Save Animated Props"))
		(void)Save_DeployPlacements();
	ImGui::EndDisabled();
	ShowAuthoringHelp(ANIMATED_PROP_HELP_SAVE);
	ImGui::SameLine();
	ImGui::Text("Placements: %zu%s",
		m_DeployRuntime.Get_Catalog().Get_Placements().size(),
		m_bDeployDirty ? "  *unsaved" : "");
	ImGui::SameLine();
	if (ImGui::Button("Reload Animated Props"))
	{
		if (m_bDeployDirty)
		{
			m_Status =
				"Save or discard the animated prop changes before reloading";
		}
		else if (Load_DeployProps())
		{
			m_iSelectedAnimatedPropPlacementId = 0u;
			Sync_AnimatedPropTransformDraft();
		}
	}
}
