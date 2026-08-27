#include "MapEffectPresentationRuntime.h"

#include "ActionPresentationTimeline.h"
#include "ClientReplication.h"
#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "Effect_Catalog.h"
#include "EncounterPatternReference.h"
#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "WorldDestructionProjectionDocument.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

using namespace DirectX;
using namespace Engine;
using namespace Client;

namespace
{
	constexpr f32_t TIMELINE_EPSILON_SECONDS = 0.001f;
	CMapEffectPresentationRuntime* g_pMapEffectAuthoringTarget = nullptr;

	const DEPLOY_PROP_PLACEMENT* Find_DeployPlacement(
		const CDeployPropCatalog& catalog,
		const uint64_t placementId)
	{
		const auto& placements = catalog.Get_Placements();
		const auto found = std::find_if(placements.begin(), placements.end(),
			[placementId](const DEPLOY_PROP_PLACEMENT& placement)
			{
				return placement.runtimePlacementId == placementId;
			});
		return placements.end() == found ? nullptr : &*found;
	}

	const ENCOUNTER_STAGE_REFERENCE* Find_EncounterStage(
		const CEncounterPatternReference& encounter,
		const std::string& patternId,
		const std::string& stageId,
		uint32_t* outStageIndex = nullptr)
	{
		const ENCOUNTER_PATTERN_REFERENCE* pattern =
			encounter.Find_Pattern(patternId);
		if (nullptr == pattern)
			return nullptr;
		for (uint32_t index = 0u; index < pattern->stages.size(); ++index)
		{
			if (pattern->stages[index].stageId == stageId)
			{
				if (nullptr != outStageIndex)
					*outStageIndex = index;
				return &pattern->stages[index];
			}
		}
		return nullptr;
	}
}

CMapEffectPresentationRuntime::~CMapEffectPresentationRuntime()
{
	Clear();
}

bool_t CMapEffectPresentationRuntime::Load_Area(
	const uint32_t levelIndex,
	const std::string& areaId,
	CDeployPropRuntime& deployRuntime,
	const CWorldDestructionProjectionDocument& destructionProjection,
	const CEncounterPatternReference& encounterReference,
	std::string& outStatus)
{
	outStatus.clear();
	if (levelIndex >= ETOUI(LEVEL::END) || areaId.empty() ||
		!deployRuntime.Is_Loaded() || !destructionProjection.Is_Ready() ||
		!encounterReference.Is_Ready() ||
		deployRuntime.Get_Catalog().Get_AreaId() != areaId ||
		destructionProjection.Get_AreaId() != areaId)
	{
		outStatus = "Map Effect runtime dependencies are incomplete or belong to another Area.";
		m_Status = outStatus;
		return false;
	}

	CMapEffectDocument stagedDocument;
	const std::filesystem::path runtimePath =
		CMapAssetCatalog::Get_MapDataRoot() / (areaId + ".mapeffects.json");
	if (!stagedDocument.Load(runtimePath, areaId, outStatus))
	{
		m_Status = outStatus;
		return false;
	}
	if (!Commit_StagedDocument(levelIndex, std::move(stagedDocument), deployRuntime,
			destructionProjection, encounterReference, outStatus))
	{
		return false;
	}
	m_AreaId = areaId;
	m_pDeployRuntime = &deployRuntime;
	m_pDestructionProjection = &destructionProjection;
	m_pEncounterReference = &encounterReference;
	g_pMapEffectAuthoringTarget = this;
	m_Status = "Loaded published Area Map Effect presentation: " + areaId;
	outStatus = m_Status;
	return true;
}

bool_t CMapEffectPresentationRuntime::Commit_StagedDocument(
	const uint32_t levelIndex,
	CMapEffectDocument stagedDocument,
	CDeployPropRuntime& deployRuntime,
	const CWorldDestructionProjectionDocument& destructionProjection,
	const CEncounterPatternReference& encounterReference,
	std::string& outStatus)
{
	if (!stagedDocument.Is_Ready() ||
		stagedDocument.Get_AreaId() != deployRuntime.Get_Catalog().Get_AreaId() ||
		stagedDocument.Get_AreaId() != destructionProjection.Get_AreaId())
	{
		outStatus = "Map Effect staged document does not match the live Area";
		m_Status = outStatus;
		return false;
	}

	std::vector<std::pair<uint64_t, DEPLOY_SURFACE_PRESENTATION_PACKET>> packets;
	std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE> stagedBaselines;
	std::vector<f32_t> stagedDurations;
	if (!Validate_AndStageSurfacePackets(
			stagedDocument, deployRuntime, destructionProjection,
			m_SurfaceEmissiveBaselines, packets, stagedBaselines, outStatus) ||
		!Validate_WorldEffects(stagedDocument, encounterReference,
			stagedDurations, outStatus) ||
		!Probe_WorldEffectAdmissions(levelIndex, stagedDocument,
			stagedDurations, outStatus))
	{
		m_Status = outStatus;
		return false;
	}

	/* CDeployPropRuntime validates every target and packet before mutating the
	   first live object.  The transaction is the union of previous and staged
	   owners, so removing a row restores its captured catalog/live baseline.
	   A world-only document deliberately skips this mutation seam. */
	if (!packets.empty() && !deployRuntime.Set_SurfacePresentations(packets))
	{
		outStatus = "Map Effect surface packet transaction was rejected: " +
			deployRuntime.Get_Status();
		m_Status = outStatus;
		return false;
	}

	/* Active handles intentionally keep the immutable presentation/prepared
	   revision that spawned them.  The new document is consulted only after
	   that occurrence ends and a later occurrence is admitted. */
	m_Document = std::move(stagedDocument);
	m_WorldEffectDurations = std::move(stagedDurations);
	m_SurfaceEmissiveBaselines = std::move(stagedBaselines);
	m_LastAttemptedServerSequenceByPlacement.clear();
	m_LevelActiveSpawnAttemptedPlacements.clear();
	m_iLevelIndex = levelIndex;
	m_isLoaded = true;
	m_Status = "Committed staged Area Map Effect presentation: " +
		m_Document.Get_AreaId();
	outStatus = m_Status;
	return true;
}

bool_t CMapEffectPresentationRuntime::Request_DebugApply(
	const uint32_t levelIndex,
	const CMapEffectDocument& stagedDocument,
	std::string& outStatus)
{
	outStatus.clear();
	CMapEffectPresentationRuntime* target = g_pMapEffectAuthoringTarget;
	if (nullptr == target || !target->m_isLoaded ||
		target->m_iLevelIndex != levelIndex ||
		nullptr == target->m_pDeployRuntime ||
		nullptr == target->m_pDestructionProjection ||
		nullptr == target->m_pEncounterReference)
	{
		outStatus = "No matching live Area Map Effect runtime is registered; enter Valtan Arena first";
		return false;
	}
	if (stagedDocument.Get_AreaId() != target->m_AreaId)
	{
		outStatus = "Map Effect authoring draft belongs to another Area";
		return false;
	}
	if (!target->Commit_StagedDocument(levelIndex, stagedDocument,
			*target->m_pDeployRuntime, *target->m_pDestructionProjection,
			*target->m_pEncounterReference, outStatus))
	{
		return false;
	}
	target->m_Status = "Applied Effect Tool Map Effect draft to live Area: " +
		target->m_AreaId;
	outStatus = target->m_Status;
	return true;
}

bool_t CMapEffectPresentationRuntime::Request_PublishedReload(
	const uint32_t levelIndex,
	std::string& outStatus)
{
	outStatus.clear();
	CMapEffectPresentationRuntime* target = g_pMapEffectAuthoringTarget;
	if (nullptr == target || !target->m_isLoaded ||
		target->m_iLevelIndex != levelIndex || target->m_AreaId.empty() ||
		nullptr == target->m_pDeployRuntime ||
		nullptr == target->m_pDestructionProjection ||
		nullptr == target->m_pEncounterReference)
	{
		outStatus = "No matching live Area Map Effect runtime is registered; enter Valtan Arena first";
		return false;
	}
	const std::string areaId = target->m_AreaId;
	return target->Load_Area(levelIndex, areaId,
		*target->m_pDeployRuntime, *target->m_pDestructionProjection,
		*target->m_pEncounterReference, outStatus);
}

bool_t CMapEffectPresentationRuntime::Validate_AndStageSurfacePackets(
	const CMapEffectDocument& stagedDocument,
	CDeployPropRuntime& deployRuntime,
	const CWorldDestructionProjectionDocument& destructionProjection,
	const std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE>&
		previousBaselines,
	std::vector<std::pair<uint64_t, DEPLOY_SURFACE_PRESENTATION_PACKET>>&
		outPackets,
	std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE>& outBaselines,
	std::string& outStatus) const
{
	outPackets.clear();
	outBaselines = previousBaselines;
	std::unordered_set<uint64_t> seenPlacements;
	std::unordered_set<uint64_t> previousPlacements;
	for (const MAP_EFFECT_SURFACE_PRESENTATION& surface :
		m_Document.Get_Surfaces())
	{
		for (const MAP_EFFECT_SURFACE_OWNER& owner : surface.owners)
			previousPlacements.insert(owner.placementId);
	}

	for (const MAP_EFFECT_SURFACE_PRESENTATION& surface :
		stagedDocument.Get_Surfaces())
	{
		for (const MAP_EFFECT_SURFACE_OWNER& owner : surface.owners)
		{
			if (surface.visibleStates.size() != 1u ||
				surface.visibleStates.front() != "INTACT")
			{
				outStatus = "Deploy surface presentation currently supports the exact INTACT visibility gate only.";
				return false;
			}
			if (!seenPlacements.insert(owner.placementId).second)
			{
				outStatus = "Map Effect surface owner is assigned more than once: " +
					std::to_string(owner.placementId);
				return false;
			}
			const DEPLOY_PROP_PLACEMENT* placement = Find_DeployPlacement(
				deployRuntime.Get_Catalog(), owner.placementId);
			const WORLD_DESTRUCTION_PROJECTION_GROUP* group =
				destructionProjection.Find_Group(owner.groupId);
			if (nullptr == placement || nullptr == group ||
				std::find(group->MemberPlacementIds.begin(),
					group->MemberPlacementIds.end(), owner.placementId) ==
					group->MemberPlacementIds.end())
			{
				outStatus = "Map Effect surface owner does not join the published Deploy/destruction placement: " +
					owner.groupId + "/" + std::to_string(owner.placementId);
				return false;
			}
			const DEPLOY_PROP_ASSET_ENTRY* asset =
				deployRuntime.Get_Catalog().Find(placement->assetId);
			if (nullptr == asset || DEPLOY_PROP_MODEL_KIND::STATIC != asset->kind ||
				!asset->deferredEmissiveOverlay || !placement->destructible ||
				1u != surface.materialIndex)
			{
				outStatus = "Map Effect surface owner is not a destructible static deferred-emissive material-1 Deploy asset: " +
					std::to_string(owner.placementId);
				return false;
			}
			DEPLOY_SURFACE_PRESENTATION_PACKET packet;
			if (!deployRuntime.Get_SurfacePresentation(owner.placementId, packet))
			{
				outStatus = "Map Effect surface owner has no live Deploy object: " +
					std::to_string(owner.placementId);
				return false;
			}
			if (!outBaselines.contains(owner.placementId))
			{
				outBaselines.emplace(owner.placementId,
					SURFACE_EMISSIVE_BASELINE{
						packet.fEmissiveIntensity,
						packet.vEmissiveColor,
						packet.fEmissiveMaskPower });
			}
			packet.fEmissiveIntensity = surface.emissiveIntensity;
			packet.vEmissiveColor = surface.emissiveColor;
			packet.fEmissiveMaskPower = surface.maskPower;
			outPackets.emplace_back(owner.placementId, packet);
		}
	}

	/* Full replacement is old U new.  Removed owners recover only the Area
	   emissive lane; destruction's transition/root/opacity lane is sampled from
	   the current live packet and survives the replacement. */
	for (const uint64_t placementId : previousPlacements)
	{
		if (seenPlacements.contains(placementId))
			continue;
		const auto baseline = outBaselines.find(placementId);
		DEPLOY_SURFACE_PRESENTATION_PACKET packet;
		if (baseline == outBaselines.end() ||
			!deployRuntime.Get_SurfacePresentation(placementId, packet))
		{
			outStatus = "Map Effect removed surface owner has no captured/live baseline: " +
				std::to_string(placementId);
			return false;
		}
		packet.fEmissiveIntensity = baseline->second.fEmissiveIntensity;
		packet.vEmissiveColor = baseline->second.vEmissiveColor;
		packet.fEmissiveMaskPower = baseline->second.fEmissiveMaskPower;
		outPackets.emplace_back(placementId, packet);
	}
	return true;
}

bool_t CMapEffectPresentationRuntime::Validate_WorldEffects(
	const CMapEffectDocument& stagedDocument,
	const CEncounterPatternReference& encounterReference,
	std::vector<f32_t>& outDurations,
	std::string& outStatus)
{
	outDurations.clear();
	outDurations.reserve(stagedDocument.Get_WorldEffects().size());
	for (const MAP_EFFECT_WORLD_PRESENTATION& world :
		stagedDocument.Get_WorldEffects())
	{
		if (nullptr == CEffectCatalog::Find_Loaded(world.effectAssetId))
		{
			outStatus = "Map Effect world target is absent from the loaded Effect catalog: " +
				world.effectAssetId;
			return false;
		}
		f32_t durationSeconds = 0.f;
		if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(
				world.effectAssetId, durationSeconds) || durationSeconds <= 0.f)
		{
			outStatus = "Map Effect world target was not prepared during Level loading: " +
				world.effectAssetId;
			return false;
		}
		uint32_t requiredTimelineMs = 0u;
		if (MAP_EFFECT_ACTIVATION_POLICY::SERVER_PATTERN_WINDOW ==
			world.activationPolicy)
		{
			if (MAP_EFFECT_PLAYBACK_POLICY::SERVER_CLOCK_SAMPLE !=
				world.playbackPolicy || world.activationWindows.empty())
			{
				outStatus = "Server-window Map Effect must use SERVER_CLOCK_SAMPLE.";
				return false;
			}
			for (const MAP_EFFECT_ACTIVATION_WINDOW& window :
				world.activationWindows)
			{
				const ENCOUNTER_STAGE_REFERENCE* stage = Find_EncounterStage(
					encounterReference, window.patternId, window.stageId);
				if (nullptr == stage ||
					window.effectTimelineOffsetMs != stage->iStartOffsetMs)
				{
					outStatus = "Map Effect activation window does not match the encounter stage clock: " +
						window.patternId + "/" + window.stageId;
					return false;
				}
				const uint64_t required = static_cast<uint64_t>(
					window.effectTimelineOffsetMs) + stage->iDurationMs;
				if (required > (std::numeric_limits<uint32_t>::max)())
				{
					outStatus = "Map Effect activation timeline overflows uint32 milliseconds.";
					return false;
				}
				requiredTimelineMs = (std::max)(requiredTimelineMs,
					static_cast<uint32_t>(required));
			}
		}
		if (durationSeconds + TIMELINE_EPSILON_SECONDS <
			static_cast<f32_t>(requiredTimelineMs) / 1000.f)
		{
			outStatus = "Map Effect authored duration does not cover its latest Server activation sample: " +
				world.effectAssetId;
			return false;
		}
		outDurations.push_back(durationSeconds);
	}
	return true;
}

bool_t CMapEffectPresentationRuntime::Probe_WorldEffectAdmissions(
	const uint32_t levelIndex,
	const CMapEffectDocument& stagedDocument,
	const std::vector<f32_t>& durations,
	std::string& outStatus) const
{
	struct STAGED_ADMISSION_PROBE final
	{
		EFFECT_WORLD_ROOT_HANDLE Handle;
		float4x4_t Root{};
		f32_t fSampleSeconds = 0.f;
		std::string effectAssetId;
	};

	const auto& worlds = stagedDocument.Get_WorldEffects();
	if (worlds.size() != durations.size())
	{
		outStatus = "Map Effect world admission duration set is inconsistent.";
		return false;
	}
	if (worlds.empty())
	{
		outStatus.clear();
		return true;
	}
	std::vector<STAGED_ADMISSION_PROBE> probes;
	probes.reserve(worlds.size());
	const auto stopProbes = [&probes]()
	{
		for (const STAGED_ADMISSION_PROBE& probe : probes)
			CEffectPresentationService::Stop_WorldRoot(probe.Handle);
	};

	for (size_t index = 0u; index < worlds.size(); ++index)
	{
		const MAP_EFFECT_WORLD_PRESENTATION& world = worlds[index];
		f32_t sampleSeconds = 0.f;
		if (!world.activationWindows.empty())
		{
			sampleSeconds = static_cast<f32_t>(
				world.activationWindows.front().effectTimelineOffsetMs) / 1000.f;
		}
		if (!std::isfinite(sampleSeconds) || sampleSeconds < 0.f ||
			sampleSeconds > durations[index] + TIMELINE_EPSILON_SECONDS)
		{
			outStatus = "Map Effect world admission sample is outside the prepared duration: " +
				world.effectAssetId;
			return false;
		}

		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC probe;
		probe.iLevelIndex = levelIndex;
		probe.strPlacementId = world.placementId +
			".__map_effect_admission_probe__" + std::to_string(index);
		probe.strEffectAssetId = world.effectAssetId;
		probe.RootWorld = Build_WorldRoot(world);
		probe.iSpawnTick = 1u;
		probe.fInitialSampleTimeSeconds = sampleSeconds;
		probe.bExternallySampled = true;
		STAGED_ADMISSION_PROBE stagedProbe;
		stagedProbe.Root = probe.RootWorld;
		stagedProbe.fSampleSeconds = sampleSeconds;
		stagedProbe.effectAssetId = world.effectAssetId;
		std::string spawnStatus;
		if (!CEffectPresentationService::Spawn_LevelPlacement(
				probe, stagedProbe.Handle, spawnStatus))
		{
			stopProbes();
			outStatus = "Map Effect world admission probe was rejected before clone: " +
				world.effectAssetId + ": " + spawnStatus;
			return false;
		}
		probes.push_back(std::move(stagedProbe));
	}

	/* Commit only this complete candidate set so admission observes its aggregate
	   scene budget without pulling unrelated gameplay requests across the normal
	   post-update commit seam. */
	std::vector<EFFECT_WORLD_ROOT_HANDLE> probeHandles;
	probeHandles.reserve(probes.size());
	for (const STAGED_ADMISSION_PROBE& probe : probes)
		probeHandles.push_back(probe.Handle);
	CEffectPresentationService::Commit_PendingWorldRootSpawns(probeHandles);
	bool_t allAdmitted = true;
	std::string rejectedAssetId;
	for (const STAGED_ADMISSION_PROBE& probe : probes)
	{
		const bool_t probeAdmitted =
			CEffectPresentationService::Update_WorldRoot(
				probe.Handle, probe.Root) &&
			CEffectPresentationService::Seek_WorldRoot(
				probe.Handle, probe.fSampleSeconds);
		if (!probeAdmitted && rejectedAssetId.empty())
		{
			rejectedAssetId = probe.effectAssetId;
			allAdmitted = false;
		}
	}
	stopProbes();
	if (!allAdmitted)
	{
		outStatus = "Map Effect world admission probe failed during EffectObject clone/prepared attach: " +
			rejectedAssetId;
		return false;
	}
	outStatus.clear();
	return true;
}

float4x4_t CMapEffectPresentationRuntime::Build_WorldRoot(
	const MAP_EFFECT_WORLD_PRESENTATION& presentation) const
{
	float4x4_t result{};
	const vector_t rotation = XMVectorSet(
		presentation.rotationQuaternion.x,
		presentation.rotationQuaternion.y,
		presentation.rotationQuaternion.z,
		presentation.rotationQuaternion.w);
	matrix_t orientation = XMMatrixRotationQuaternion(rotation);
	if (MAP_EFFECT_ORIENTATION_POLICY::CAMERA_FACING_WORLD ==
		presentation.orientationPolicy)
	{
		const float4_t* camera = CGameInstance::Get().Get_CamPosition();
		if (nullptr != camera)
		{
			const vector_t position = XMVectorSet(presentation.position.x,
				presentation.position.y, presentation.position.z, 1.f);
			vector_t forward = XMVectorSubtract(XMLoadFloat4(camera), position);
			if (XMVectorGetX(XMVector3LengthSq(forward)) > 0.000001f)
			{
				forward = XMVector3Normalize(forward);
				vector_t upReference = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				if (std::abs(XMVectorGetX(XMVector3Dot(
					forward, upReference))) > 0.98f)
				{
					upReference = XMVectorSet(0.f, 0.f, 1.f, 0.f);
				}
				const vector_t right = XMVector3Normalize(
					XMVector3Cross(upReference, forward));
				const vector_t up = XMVector3Cross(forward, right);
				const matrix_t facing(right, up, forward,
					XMVectorSet(0.f, 0.f, 0.f, 1.f));
				orientation *= facing;
			}
		}
	}
	XMStoreFloat4x4(&result, XMMatrixScaling(
		presentation.scale.x, presentation.scale.y, presentation.scale.z) *
		orientation * XMMatrixTranslation(presentation.position.x,
			presentation.position.y, presentation.position.z));
	return result;
}

bool_t CMapEffectPresentationRuntime::Resolve_WorldSample(
	const MAP_EFFECT_WORLD_PRESENTATION& presentation,
	const ACTIVE_WORLD_EFFECT* active,
	const VALTAN_PRESENTATION_STATE& boss,
	const ENCOUNTER_STAGE_REFERENCE* activeStage,
	const f32_t timeDelta,
	f32_t& outSampleSeconds,
	uint32_t& outOccurrenceSequence,
	uint32_t& outSampleStartTick) const
{
	outSampleSeconds = 0.f;
	outOccurrenceSequence = 0u;
	outSampleStartTick = 0u;
	if (MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE ==
		presentation.activationPolicy)
	{
		const f32_t delta = std::isfinite(timeDelta) ?
			(std::clamp)(timeDelta, 0.f, 0.1f) : 0.f;
		outSampleSeconds = nullptr == active ? 0.f :
			active->fLastSampleSeconds + delta;
		return true;
	}

	if (!boss.isValid || nullptr == activeStage ||
		0u == boss.iActionStartTick || 0u == boss.iServerTick)
	{
		return false;
	}
	const auto found = std::find_if(presentation.activationWindows.begin(),
		presentation.activationWindows.end(),
		[&boss, activeStage](const MAP_EFFECT_ACTIVATION_WINDOW& window)
		{
			return window.patternId == boss.strPatternId &&
				window.stageId == activeStage->stageId;
		});
	if (presentation.activationWindows.end() == found)
		return false;

	const f32_t fixedTickHz = static_cast<f32_t>(
		m_pEncounterReference->Get_FixedTickHz());
	f32_t actionAgeSeconds = 0.f;
	if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			boss.iServerTick, boss.iActionStartTick, fixedTickHz,
			actionAgeSeconds) ||
		actionAgeSeconds > static_cast<f32_t>(activeStage->iDurationMs) /
			1000.f + TIMELINE_EPSILON_SECONDS)
	{
		return false;
	}

	const f32_t authoritativeSample = static_cast<f32_t>(
		found->effectTimelineOffsetMs) / 1000.f + actionAgeSeconds;
	outSampleSeconds = authoritativeSample;
	outOccurrenceSequence = boss.iPatternSequence;
	outSampleStartTick = boss.iActionStartTick;
	/* A render frame can repeat the latest Server snapshot.  Advance locally by
	   at most one fixed tick, then correct to the next authoritative sample.
	   actionStartTick may change at every stage without replacing the handle. */
	if (nullptr != active && active->iPatternSequence == boss.iPatternSequence &&
		active->iActionStartTick == boss.iActionStartTick &&
		active->iLastServerTick == boss.iServerTick && fixedTickHz > 0.f)
	{
		const f32_t tickSeconds = 1.f / fixedTickHz;
		const f32_t delta = std::isfinite(timeDelta) ?
			(std::clamp)(timeDelta, 0.f, tickSeconds) : 0.f;
		outSampleSeconds = (std::min)(authoritativeSample + tickSeconds,
			(std::max)(authoritativeSample,
				active->fLastSampleSeconds + delta));
	}
	return true;
}

void CMapEffectPresentationRuntime::Update_ServerPresentation(
	const VALTAN_PRESENTATION_STATE& boss,
	const f32_t timeDelta)
{
	if (!m_isLoaded || nullptr == m_pEncounterReference ||
		m_iLevelIndex >= ETOUI(LEVEL::END))
		return;

	const ENCOUNTER_PATTERN_REFERENCE* activePattern =
		boss.isValid ? m_pEncounterReference->Find_Pattern(boss.strPatternId) :
		nullptr;
	const ENCOUNTER_STAGE_REFERENCE* activeStage =
		(nullptr != activePattern &&
		 boss.iPatternStageIndex < activePattern->stages.size()) ?
		&activePattern->stages[boss.iPatternStageIndex] : nullptr;

	/* Existing occurrences consume their immutable spawn revision first. */
	for (size_t index = m_ActiveWorldEffects.size(); index-- > 0u;)
	{
		ACTIVE_WORLD_EFFECT& active = m_ActiveWorldEffects[index];
		f32_t sampleSeconds = 0.f;
		uint32_t occurrenceSequence = 0u;
		uint32_t sampleStartTick = 0u;
		const bool_t shouldPresent = Resolve_WorldSample(
			active.Presentation, &active, boss, activeStage, timeDelta,
			sampleSeconds, occurrenceSequence, sampleStartTick);
		if (!shouldPresent || !std::isfinite(sampleSeconds) ||
			sampleSeconds < 0.f || active.fDurationSeconds <= 0.f)
		{
			Stop_WorldEffect(index);
			m_ActiveWorldEffects.erase(m_ActiveWorldEffects.begin() + index);
			continue;
		}
		if (MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE ==
			active.Presentation.activationPolicy)
		{
			sampleSeconds = std::fmod(sampleSeconds,
				active.fDurationSeconds);
		}
		else if (active.iPatternSequence != occurrenceSequence ||
			sampleSeconds > active.fDurationSeconds +
				TIMELINE_EPSILON_SECONDS)
		{
			Stop_WorldEffect(index);
			m_ActiveWorldEffects.erase(m_ActiveWorldEffects.begin() + index);
			continue;
		}

		const float4x4_t root = Build_WorldRoot(active.Presentation);
		if (!CEffectPresentationService::Update_WorldRoot(
				active.Handle, root) ||
			!CEffectPresentationService::Seek_WorldRoot(
				active.Handle, sampleSeconds))
		{
			Stop_WorldEffect(index);
			m_ActiveWorldEffects.erase(m_ActiveWorldEffects.begin() + index);
			m_Status = "Map Effect world occurrence lost its admitted level-owned handle; the committed definition was preserved.";
			continue;
		}
		active.iPatternSequence = occurrenceSequence;
		active.iActionStartTick = sampleStartTick;
		active.iLastServerTick = boss.iServerTick;
		active.fLastSampleSeconds = sampleSeconds;
	}

	/* The current document is a next-spawn definition set. */
	const auto& worlds = m_Document.Get_WorldEffects();
	for (size_t index = 0u; index < worlds.size(); ++index)
	{
		const MAP_EFFECT_WORLD_PRESENTATION& world = worlds[index];
		if (std::any_of(m_ActiveWorldEffects.begin(),
			m_ActiveWorldEffects.end(), [&world](const ACTIVE_WORLD_EFFECT& active)
			{
				return active.Presentation.placementId == world.placementId;
			}))
		{
			continue;
		}
		if (index >= m_WorldEffectDurations.size() ||
			m_WorldEffectDurations[index] <= 0.f)
		{
			continue;
		}

		f32_t sampleSeconds = 0.f;
		uint32_t occurrenceSequence = 0u;
		uint32_t sampleStartTick = 0u;
		if (!Resolve_WorldSample(world, nullptr, boss, activeStage, timeDelta,
				sampleSeconds, occurrenceSequence, sampleStartTick) ||
			!std::isfinite(sampleSeconds) || sampleSeconds < 0.f ||
			sampleSeconds > m_WorldEffectDurations[index] +
				TIMELINE_EPSILON_SECONDS)
		{
			continue;
		}

		if (MAP_EFFECT_ACTIVATION_POLICY::LEVEL_ACTIVE ==
			world.activationPolicy)
		{
			if (!m_LevelActiveSpawnAttemptedPlacements.insert(
					world.placementId).second)
			{
				continue;
			}
		}
		else
		{
			const auto previousAttempt =
				m_LastAttemptedServerSequenceByPlacement.find(world.placementId);
			if (previousAttempt !=
					m_LastAttemptedServerSequenceByPlacement.end() &&
				previousAttempt->second == occurrenceSequence)
			{
				continue;
			}
			m_LastAttemptedServerSequenceByPlacement.insert_or_assign(
				world.placementId, occurrenceSequence);
		}

		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
		spawn.iLevelIndex = m_iLevelIndex;
		spawn.strPlacementId = world.placementId;
		spawn.strEffectAssetId = world.effectAssetId;
		spawn.RootWorld = Build_WorldRoot(world);
		spawn.iSpawnTick = sampleStartTick;
		spawn.fInitialSampleTimeSeconds = sampleSeconds;
		spawn.bExternallySampled = true;
		EFFECT_WORLD_ROOT_HANDLE handle;
		std::string status;
		if (!CEffectPresentationService::Spawn_LevelPlacement(
				spawn, handle, status))
		{
			m_Status = "Map Effect world spawn rejected; previous committed document/occurrences remain: " +
				status;
			continue;
		}

		ACTIVE_WORLD_EFFECT active;
		active.Presentation = world;
		active.Handle = handle;
		active.fDurationSeconds = m_WorldEffectDurations[index];
		active.iPatternSequence = occurrenceSequence;
		active.iActionStartTick = sampleStartTick;
		active.iLastServerTick = boss.iServerTick;
		active.fLastSampleSeconds = sampleSeconds;
		m_ActiveWorldEffects.push_back(std::move(active));
	}
}

void CMapEffectPresentationRuntime::Stop_WorldEffect(const size_t index)
{
	if (index >= m_ActiveWorldEffects.size())
		return;
	if (m_ActiveWorldEffects[index].Handle.Is_Valid())
	{
		CEffectPresentationService::Stop_WorldRoot(
			m_ActiveWorldEffects[index].Handle);
	}
	m_ActiveWorldEffects[index] = {};
}

void CMapEffectPresentationRuntime::Restore_CurrentSurfaceBaselines()
{
	if (nullptr == m_pDeployRuntime || !m_pDeployRuntime->Is_Loaded())
		return;
	std::vector<CDeployPropRuntime::DEPLOY_SURFACE_PRESENTATION_UPDATE> updates;
	std::unordered_set<uint64_t> seen;
	for (const MAP_EFFECT_SURFACE_PRESENTATION& surface :
		m_Document.Get_Surfaces())
	{
		for (const MAP_EFFECT_SURFACE_OWNER& owner : surface.owners)
		{
			if (!seen.insert(owner.placementId).second)
				continue;
			const auto baseline = m_SurfaceEmissiveBaselines.find(
				owner.placementId);
			DEPLOY_SURFACE_PRESENTATION_PACKET packet;
			if (baseline == m_SurfaceEmissiveBaselines.end() ||
				!m_pDeployRuntime->Get_SurfacePresentation(
					owner.placementId, packet))
			{
				continue;
			}
			packet.fEmissiveIntensity = baseline->second.fEmissiveIntensity;
			packet.vEmissiveColor = baseline->second.vEmissiveColor;
			packet.fEmissiveMaskPower = baseline->second.fEmissiveMaskPower;
			updates.emplace_back(owner.placementId, packet);
		}
	}
	if (!updates.empty() &&
		!m_pDeployRuntime->Set_SurfacePresentations(updates))
	{
		OutputDebugStringA((
			"[Client][MapEffect] surface baseline restore failed during clear: " +
			m_pDeployRuntime->Get_Status() + "\n").c_str());
	}
}

void CMapEffectPresentationRuntime::Clear()
{
	if (g_pMapEffectAuthoringTarget == this)
		g_pMapEffectAuthoringTarget = nullptr;
	Restore_CurrentSurfaceBaselines();
	for (size_t index = 0u; index < m_ActiveWorldEffects.size(); ++index)
		Stop_WorldEffect(index);
	m_ActiveWorldEffects.clear();
	m_WorldEffectDurations.clear();
	m_SurfaceEmissiveBaselines.clear();
	m_LastAttemptedServerSequenceByPlacement.clear();
	m_LevelActiveSpawnAttemptedPlacements.clear();
	m_Document.Clear();
	m_AreaId.clear();
	m_pDeployRuntime = nullptr;
	m_pDestructionProjection = nullptr;
	m_pEncounterReference = nullptr;
	m_iLevelIndex = ETOUI(LEVEL::END);
	m_isLoaded = false;
	m_Status = "Map Effect presentation not loaded";
}
