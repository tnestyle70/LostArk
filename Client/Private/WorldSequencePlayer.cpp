#include "WorldSequencePlayer.h"

#include "DeployPropObject.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "Model.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace Client;
using namespace Engine;

namespace
{
	f32_t Clamp01(const f32_t value)
	{
		if (!std::isfinite(value))
			return 0.f;
		return value < 0.f ? 0.f : (value > 1.f ? 1.f : value);
	}

	struct OBJECT_MOTION_SAMPLE
	{
		const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr;
		f32_t localMs = 0.f;
		bool_t visible = false;
		bool_t finished = false;
		bool_t pending = false;
		bool_t holdFinalPose = false;
		f32_t emissionStartMs = 0.f;
		f32_t emissionRate = 1.f;
		std::string emissionMotionId;
	};

	bool_t Is_SingleObjectMotion(const WORLD_SEQUENCE_INSTANCE& instance)
	{
		return instance.bindings.size() == 1u &&
			instance.bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
	}

	// Resolve from the original clock on every seek; NEXT never accumulates time or pose drift.
	bool_t Resolve_ObjectMotion(const CWorldSequenceDocument& document,
		const std::string& initialId, f32_t elapsedMs, const f32_t speed,
		OBJECT_MOTION_SAMPLE& sample)
	{
		const f32_t originalElapsedMs = elapsedMs;
		const auto* motion = document.Find_Instance(initialId);
		const WORLD_SEQUENCE_TEMPLATE* previousSequence = nullptr;
		for (uint32_t depth = 0; depth <= 32u; ++depth)
		{
			if (!motion || !motion->enabled || !Is_SingleObjectMotion(*motion)) return false;
			const auto* sequence = document.Find_Template(motion->templateId);
			if (!sequence || sequence->durationMs == 0u) return false;
			const f32_t rate = motion->playbackSpeed * speed;
			if (!std::isfinite(rate) || rate <= 0.f) return false;
			const f32_t delayed = elapsedMs - motion->startDelayMs;
			const f32_t raw = (std::max)(0.f, delayed) * rate;
			const f32_t duration = static_cast<f32_t>(sequence->ObjectSpanMs());
			if (!std::isfinite(raw)) return false;
			if (delayed < 0.f && previousSequence)
			{
				sample.sequence = previousSequence;
				sample.localMs = static_cast<f32_t>(previousSequence->durationMs);
				sample.visible = true;
				sample.holdFinalPose = true;
				return true;
			}
			sample.emissionStartMs = originalElapsedMs - elapsedMs + motion->startDelayMs;
			sample.emissionRate = rate;
			sample.emissionMotionId = motion->instanceId;
			if (motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP)
				sample.emissionStartMs += std::floor(raw / duration) * duration / rate;
			if (motion->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT && raw >= duration)
			{
				elapsedMs -= motion->startDelayMs + duration / rate;
				previousSequence = sequence;
				motion = document.Find_Instance(motion->nextMotionId);
				continue;
			}
			sample.sequence = sequence;
			sample.localMs = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP ?
				std::fmod(raw, duration) : (std::min)(raw, duration);
			sample.finished = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::STOP && raw >= sequence->PresentationSpanMs();
			sample.visible = delayed >= 0.f && (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP || raw < duration);
			sample.pending = delayed < 0.f;
			sample.holdFinalPose = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD;
			return true;
		}
		return false;
	}

	/* A placement that is drawn through a shared static batch still has an
	   authored scale, so both presentations answer the same question. */
	WORLD_SEQUENCE_PLACEMENT_MAP Collect_Placements(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements)
	{
		WORLD_SEQUENCE_PLACEMENT_MAP result;
		result.reserve(placements.size());
		for (const MAP_RUNTIME_PLACED_ENTRY& placement : placements)
		{
			/* A background placement is drawn by the sky pass and has no
			   per-placement transform to drive, so it is not a sequence
			   target. This mirrors what the Map Tool admits. */
			const MAP_ASSET_ENTRY* asset =
				catalog.Find(placement.record.assetId);
			const bool_t supported = nullptr != asset &&
				MAP_ASSET_RENDER_MODE::BACKGROUND !=
					asset->renderProfile.renderMode;
			result.emplace(placement.record.placementId,
				WORLD_SEQUENCE_PLACEMENT_INFO{
					placement.record.signedScale, supported });
		}
		return result;
	}

	WORLD_SEQUENCE_DEPLOY_MAP Collect_DeployPlacements(
		const CDeployPropRuntime& deployRuntime)
	{
		WORLD_SEQUENCE_DEPLOY_MAP result;
		result.reserve(deployRuntime.Get_Entries().size());
		for (const DEPLOY_RUNTIME_ENTRY& entry : deployRuntime.Get_Entries())
		{
			WORLD_SEQUENCE_DEPLOY_INFO info;
			if (nullptr != entry.object && !entry.object->Is_StaticDeployModel())
			{
				for (const DEPLOY_PROP_ANIMATION_CLIP& clip :
					entry.object->Get_AnimationClips())
				{
					if (!clip.name.empty())
						info.animationClips.push_back(clip.name);
				}
				info.animationTargetSupported = !info.animationClips.empty();
			}
			result.emplace(entry.placement.runtimePlacementId, std::move(info));
		}
		return result;
	}
}

bool_t CWorldSequencePlayer::Load_Area(
	const std::string& areaId,
	const TARGET_SET& targets)
{
	if (areaId.empty() || !targets.Is_Complete())
	{
		m_Status = "World sequence area or target set is empty";
		return false;
	}
	const std::filesystem::path path = CMapAssetCatalog::Get_MapDataRoot() /
		(std::filesystem::path(areaId).wstring() + L".worldsequences.json");
	if (!std::filesystem::is_regular_file(path))
	{
		/* An Area may legitimately author no sequence yet. The caller decides
		   whether that is fatal, so report it without inventing a document. */
		m_Status = "World sequence runtime document is absent";
		return false;
	}
	/* Admit the shipped document against the targets this level created, so a
	   sequence that points at a placement the level did not load is rejected
	   here instead of failing halfway through a play. */
	std::string status;
	CWorldSequenceDocument staged;
	if (!staged.Load(path, areaId,
		Collect_Placements(*targets.pCatalog, *targets.pPlacements),
		Collect_DeployPlacements(*targets.pDeployRuntime), status))
	{
		m_Status = "World sequence load failed: " + status;
		return false;
	}
	Stop_All(targets, true);
	m_ObjectModels.clear();
	m_EffectSnapshots.clear();
	m_Document = std::move(staged);
	m_Status = "World sequence loaded: " +
		std::to_string(m_Document.Get_Instances().size()) + " instances";
	return true;
}

void CWorldSequencePlayer::Clear()
{
	for (auto& active : m_Active) Release_Objects(active);
	m_Active.clear();
	for (auto& held : m_Held) Release_Objects(held);
	m_Held.clear();
	m_ObjectModels.clear();
	m_EffectSnapshots.clear();
	m_ModelCache.clear();
	m_Document.Reset_Empty({});
	m_Status.clear();
}

bool_t CWorldSequencePlayer::Try_ParseTargetId(
	const WORLD_SEQUENCE_BINDING& binding,
	uint64_t& outTargetId)
{
	if (binding.targetId.empty() || binding.targetId.size() > 20u ||
		!std::all_of(binding.targetId.begin(), binding.targetId.end(),
			[](const unsigned char character)
			{
				return character >= '0' && character <= '9';
			}))
	{
		return false;
	}
	try
	{
		outTargetId = std::stoull(binding.targetId);
	}
	catch (...)
	{
		return false;
	}
	return 0u != outTargetId;
}

const WORLD_SEQUENCE_TRACK* CWorldSequencePlayer::Find_Track(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const std::string& slotId)
{
	const auto found = std::find_if(sequence.tracks.begin(),
		sequence.tracks.end(),
		[&slotId](const WORLD_SEQUENCE_TRACK& value)
		{
			return value.slotId == slotId;
		});
	return sequence.tracks.end() == found ? nullptr : &*found;
}

const WORLD_SEQUENCE_ANIMATION_TRACK*
CWorldSequencePlayer::Find_AnimationTrackAt(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const std::string& slotId,
	const f32_t localMs,
	f32_t& outWindowEndMs)
{
	const WORLD_SEQUENCE_ANIMATION_TRACK* found = nullptr;
	outWindowEndMs = static_cast<f32_t>(sequence.durationMs);
	for (const WORLD_SEQUENCE_ANIMATION_TRACK& track : sequence.animationTracks)
	{
		if (track.slotId != slotId)
			continue;
		const f32_t startMs = static_cast<f32_t>(track.startMs);
		if (startMs <= localMs)
		{
			/* The document orders a slot's tracks by ascending start, so the
			   last one at or before now is the clip that owns this moment. */
			found = &track;
			outWindowEndMs = static_cast<f32_t>(sequence.durationMs);
			continue;
		}
		if (nullptr != found)
		{
			outWindowEndMs = startMs;
			break;
		}
	}
	return found;
}

const WORLD_SEQUENCE_ANIMATION_TRACK* CWorldSequencePlayer::Find_AnimationTrack(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const std::string& slotId)
{
	const auto found = std::find_if(sequence.animationTracks.begin(),
		sequence.animationTracks.end(),
		[&slotId](const WORLD_SEQUENCE_ANIMATION_TRACK& value)
		{
			return value.slotId == slotId;
		});
	return sequence.animationTracks.end() == found ? nullptr : &*found;
}

MAP_RUNTIME_PLACED_ENTRY* CWorldSequencePlayer::Find_Placement(
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const uint64_t placementId)
{
	const auto found = std::find_if(placements.begin(), placements.end(),
		[placementId](const MAP_RUNTIME_PLACED_ENTRY& value)
		{
			return value.record.placementId == placementId;
		});
	return placements.end() == found ? nullptr : &*found;
}

WORLD_SEQUENCE_TRANSFORM_KEY CWorldSequencePlayer::Sample_Track(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const WORLD_SEQUENCE_TRACK& track,
	const f32_t timeMs)
{
	if (track.keys.empty())
		return {};
	if (timeMs <= static_cast<f32_t>(track.keys.front().timeMs))
		return track.keys.front();
	if (timeMs >= static_cast<f32_t>(track.keys.back().timeMs))
		return track.keys.back();
	const auto right = std::upper_bound(track.keys.begin(), track.keys.end(),
		timeMs,
		[](const f32_t time, const WORLD_SEQUENCE_TRANSFORM_KEY& key)
		{
			return time < static_cast<f32_t>(key.timeMs);
		});
	const WORLD_SEQUENCE_TRANSFORM_KEY& left = *(right - 1);
	const f32_t span = static_cast<f32_t>(right->timeMs - left.timeMs);
	f32_t factor = span <= 0.f ? 0.f :
		(timeMs - static_cast<f32_t>(left.timeMs)) / span;
	factor = Clamp01(factor);
	if (WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP == sequence.interpolation)
		factor = factor * factor * (3.f - 2.f * factor);

	WORLD_SEQUENCE_TRANSFORM_KEY result;
	result.timeMs = static_cast<uint32_t>((std::max)(0.f, timeMs));
	XMStoreFloat3(&result.positionOffset,
		XMVectorLerp(XMLoadFloat3(&left.positionOffset),
			XMLoadFloat3(&right->positionOffset), factor));
	XMStoreFloat4(&result.rotationQuaternion,
		XMQuaternionNormalize(XMQuaternionSlerp(
			XMLoadFloat4(&left.rotationQuaternion),
			XMLoadFloat4(&right->rotationQuaternion), factor)));
	if (result.rotationQuaternion.w < 0.f)
	{
		result.rotationQuaternion.x = -result.rotationQuaternion.x;
		result.rotationQuaternion.y = -result.rotationQuaternion.y;
		result.rotationQuaternion.z = -result.rotationQuaternion.z;
		result.rotationQuaternion.w = -result.rotationQuaternion.w;
	}
	XMStoreFloat3(&result.scaleMultiplier,
		XMVectorLerp(XMLoadFloat3(&left.scaleMultiplier),
			XMLoadFloat3(&right->scaleMultiplier), factor));
	/* Visibility is a step, not a blend: a key turns a target on or off. */
	result.visible = left.visible;
	return result;
}

MAP_PLACEMENT_RECORD CWorldSequencePlayer::Compose_SampledRecord(
	const MAP_PLACEMENT_RECORD& baseline,
	const bool_t baselineRuntimeVisible,
	const WORLD_SEQUENCE_TRANSFORM_KEY& key)
{
	MAP_PLACEMENT_RECORD sampled = baseline;
	const vector_t baselineRotation =
		XMLoadFloat4(&baseline.rotationQuaternion);
	/* The authored offset is expressed in the placement's own frame so an
	   authored nudge keeps its meaning after the placement is rotated. */
	float3_t rotatedOffset;
	XMStoreFloat3(&rotatedOffset,
		XMVector3Rotate(XMLoadFloat3(&key.positionOffset), baselineRotation));
	sampled.position.x += rotatedOffset.x;
	sampled.position.y += rotatedOffset.y;
	sampled.position.z += rotatedOffset.z;
	vector_t combined = XMQuaternionMultiply(
		XMLoadFloat4(&key.rotationQuaternion), baselineRotation);
	combined = XMQuaternionNormalize(combined);
	if (XMVectorGetW(combined) < 0.f)
		combined = XMVectorNegate(combined);
	XMStoreFloat4(&sampled.rotationQuaternion, combined);
	sampled.signedScale.x *= key.scaleMultiplier.x;
	sampled.signedScale.y *= key.scaleMultiplier.y;
	sampled.signedScale.z *= key.scaleMultiplier.z;
	/* A sequence may hide a target but must never reveal one the level has
	   already suppressed for its own reason. */
	const bool_t externalVisibilityAllowsShowing =
		baselineRuntimeVisible || !baseline.visible;
	sampled.visible = externalVisibilityAllowsShowing && key.visible;
	return sampled;
}

bool_t CWorldSequencePlayer::Apply_RuntimeRecord(
	const TARGET_SET& targets,
	std::unordered_map<std::string, shared_ptr<CModel>>& modelCache,
	MAP_RUNTIME_PLACED_ENTRY& entry,
	const MAP_PLACEMENT_RECORD& record)
{
	if (!targets.Is_Complete())
		return false;
	bool_t transformed = false;
	if (nullptr != entry.object)
	{
		entry.object->Set_PlacementTransform(record.position,
			record.rotationQuaternion, record.signedScale);
		transformed = true;
	}
	else if (nullptr != entry.batch)
	{
		const MAP_ASSET_ENTRY* asset = targets.pCatalog->Find(record.assetId);
		if (nullptr == asset)
			return false;
		shared_ptr<CModel>& model = modelCache[record.assetId];
		if (nullptr == model)
		{
			model = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					targets.levelIndex, asset->prototypeTag));
		}
		FMapStaticInstance instance{};
		if (nullptr != model &&
			SUCCEEDED(CMapPlacementRuntime::Build_StaticInstance(
				*asset, model, record, instance)) &&
			SUCCEEDED(entry.batch->Update_Instance(
				record.placementId, instance)))
		{
			transformed = true;
		}
	}
	return transformed &&
		CMapPlacementRuntime::Set_RuntimeVisible(entry, record.visible);
}

bool_t CWorldSequencePlayer::Validate_ObjectPlacement(const std::string& instanceId,
	const std::optional<OBJECT_PLACEMENT>& placement, std::string& status) const
{
	const auto* instance = m_Document.Find_Instance(instanceId);
	if (!instance) { status = "World sequence instance is unavailable: " + instanceId; return false; }
	if (!placement) return true;
	const auto validVector = [](const float3_t& value, const float minimum, const float maximum)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
			value.x >= minimum && value.x <= maximum && value.y >= minimum && value.y <= maximum &&
			value.z >= minimum && value.z <= maximum;
	};
	const auto* resource = Is_SingleObjectMotion(*instance) ?
		m_Document.Find_ObjectResource(instance->bindings.front().targetId) : nullptr;
	if (instance->anchorKind != "WORLD" || !resource || resource->anchorKind != "WORLD" ||
		!validVector(placement->position, -100000.f, 100000.f) ||
		!validVector(placement->rotationDegrees, -36000.f, 36000.f) ||
		!validVector(placement->scale, .001f, 1000.f))
	{
		status = "Independent placement requires one WORLD Object Resource and finite placement values: " + instanceId;
		return false;
	}
	return true;
}

bool_t CWorldSequencePlayer::Set_ObjectPlacement(const std::string& instanceId,
	const std::optional<OBJECT_PLACEMENT>& placement, const TARGET_SET& targets)
{
	if (!targets.Is_Complete()) { m_Status = "World placement requires complete runtime targets."; return false; }
	if (!Validate_ObjectPlacement(instanceId, placement, m_Status)) return false;
	const auto active = std::find_if(m_Active.begin(), m_Active.end(),
		[&](const auto& value) { return value.instanceId == instanceId; });
	if (active == m_Active.end()) { m_Status = "World placement has no active object: " + instanceId; return false; }
	if (active->placement == placement) return true;
	const auto previous = active->placement;
	active->placement = placement;
	if (Apply_Instance(*active, targets) == APPLY_RESULT::FAILED)
	{
		const auto failure = m_Status;
		active->placement = previous;
		(void)Apply_Instance(*active, targets);
		m_Status = failure;
		return false;
	}
	m_Status = "World placement updated at the current motion clock: " + instanceId;
	return true;
}

bool_t CWorldSequencePlayer::Play(
	const std::string& instanceId,
	const TARGET_SET& targets, const f32_t playbackSpeed, const float3_t& positionOffset, const uint32_t durationMs,
	const std::optional<OBJECT_PLACEMENT>& placement)
{
	if (!Is_Ready() || !targets.Is_Complete() || !std::isfinite(playbackSpeed) || playbackSpeed <= 0.f ||
		!std::isfinite(positionOffset.x) || !std::isfinite(positionOffset.y) || !std::isfinite(positionOffset.z))
	{
		m_Status = "World sequence player is not ready";
		return false;
	}
	const WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(instanceId);
	const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		m_Document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence || !instance->enabled)
	{
		m_Status = "World sequence instance is unavailable: " + instanceId;
		return false;
	}
	if (!Validate_ObjectPlacement(instanceId, placement, m_Status)) return false;

	const auto existing = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	if (m_Active.end() != existing)
	{
		existing->durationMs = durationMs;
		existing->elapsedMs = 0.f;
		existing->playbackSpeed = playbackSpeed;
		existing->positionOffset = positionOffset;
		existing->placement = placement;
		existing->motionInstanceId.clear();
		existing->motionStartMs = 0.f;
		existing->emissionAnchors.clear();
		return true;
	}

	/* Capture the live pose of every bound target before the first sample so
	   a replay composes against the placed transform, not against whatever the
	   previous play left behind. */
	if (!Prepare_ObjectMotionChain(*instance, targets)) return false;
	// Release only completed owners sharing this new instance's explicit targets.
	for (size_t i = 0; i < m_Held.size();)
	{
		const auto* previous = m_Document.Find_Instance(m_Held[i].instanceId);
		const bool overlap = m_Held[i].instanceId == instanceId || (previous &&
			std::any_of(previous->bindings.begin(), previous->bindings.end(), [&](const auto& oldBinding)
			{
				return std::any_of(instance->bindings.begin(), instance->bindings.end(), [&](const auto& binding)
				{ return binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
					binding.targetKind == oldBinding.targetKind && binding.targetId == oldBinding.targetId; });
			}));
		if (!overlap) { ++i; continue; }
		const auto previousId = m_Held[i].instanceId;
		Stop_Instance(previousId, targets, previousId == instanceId);
	}
	ACTIVE_INSTANCE active;
	active.durationMs = durationMs;
	active.instanceId = instanceId;
	active.playbackSpeed = playbackSpeed;
	active.positionOffset = positionOffset;
	active.placement = placement;
	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
		if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
		uint64_t targetId = 0;
		if (!Try_ParseTargetId(binding, targetId))
		{
			m_Status = "World sequence binding target is invalid: " + instanceId;
			return false;
		}
		if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
		{
			const shared_ptr<CDeployPropObject> object =
				targets.pDeployRuntime->Find(targetId);
			if (nullptr == Find_AnimationTrack(*sequence, binding.slotId) ||
				nullptr == object ||
				!object->Begin_AnimationAuthoringPreview())
			{
				Release_DeployPreviews(active, targets);
				m_Status = "World sequence animated target is unavailable: " +
					instanceId;
				return false;
			}
			active.deployTargets.push_back(targetId);
			continue;
		}
		const MAP_RUNTIME_PLACED_ENTRY* entry =
			Find_Placement(*targets.pPlacements, targetId);
		if (nullptr == Find_Track(*sequence, binding.slotId) ||
			nullptr == entry)
		{
			Release_DeployPreviews(active, targets);
			m_Status = "World sequence placement target is unavailable: " +
				instanceId;
			return false;
		}
		PLACEMENT_BASELINE baseline;
		baseline.placementId = targetId;
		baseline.record = entry->record;
		baseline.runtimeVisible = entry->record.visible;
		baseline.restoreRuntimeVisible = entry->record.visible;
		(void)CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, baseline.restoreRuntimeVisible);
		active.placementBaselines.push_back(std::move(baseline));
	}
	m_Active.push_back(std::move(active));
	m_Status = "World sequence started: " + instanceId;
	return true;
}

bool_t CWorldSequencePlayer::Prepare_ObjectMotionChain(
	const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
	const auto* motion = &instance;
	for (uint32_t depth = 0; depth <= 32u; ++depth)
	{
		if (!Prepare_ObjectResources(*motion, targets)) return false;
		if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) return true;
		motion = m_Document.Find_Instance(motion->nextMotionId);
		if (!motion || !motion->enabled) break;
	}
	m_Status = "World Object completion motion is unavailable or exceeds the chain limit.";
	return false;
}

bool_t CWorldSequencePlayer::Apply_ObjectMotion(const std::string& targetInstanceId,
	const std::string& motionInstanceId, const TARGET_SET& targets)
{
	const auto active = std::find_if(m_Active.begin(), m_Active.end(),
		[&](const ACTIVE_INSTANCE& value) { return value.instanceId == targetInstanceId; });
	const auto* target = m_Document.Find_Instance(targetInstanceId);
	const auto* motion = m_Document.Find_Instance(motionInstanceId);
	const auto* targetSequence = target ? m_Document.Find_Template(target->templateId) : nullptr;
	const auto* motionSequence = motion ? m_Document.Find_Template(motion->templateId) : nullptr;
	if (!targets.Is_Complete() || active == m_Active.end() || !target || !motion ||
		!motion->enabled || !targetSequence || !motionSequence ||
		!Is_SingleObjectMotion(*target) || !Is_SingleObjectMotion(*motion) ||
		target->bindings.front().targetId != motion->bindings.front().targetId ||
		target->bindings.front().slotId != motion->bindings.front().slotId ||
		target->anchorKind != "WORLD" || targetSequence->objectMotion.count != 1u ||
		motionSequence->objectMotion.count != 1u ||
		(active->durationMs != 0u && active->elapsedMs >= active->durationMs))
	{
		m_Status = "World Object motion target must be an active single world object with the same parent: " + targetInstanceId;
		return false;
	}
	if (!Prepare_ObjectMotionChain(*motion, targets)) return false;
	const auto previousMotion = active->motionInstanceId;
	const auto previousStart = active->motionStartMs;
	active->motionInstanceId = motionInstanceId;
	active->motionStartMs = active->elapsedMs;
	if (Apply_Instance(*active, targets) == APPLY_RESULT::FAILED)
	{
		const auto failure = m_Status;
		active->motionInstanceId = previousMotion;
		active->motionStartMs = previousStart;
		(void)Apply_Instance(*active, targets);
		m_Status = failure;
		return false;
	}
	m_Status = "World Object motion applied: " + targetInstanceId + " -> " + motionInstanceId;
	return true;
}

void CWorldSequencePlayer::Set_PlacementSuppressed(const uint64_t placementId, const bool_t suppressed)
{
	if (suppressed)
		m_SuppressedPlacements.insert(placementId);
	else
		m_SuppressedPlacements.erase(placementId);
}

bool_t CWorldSequencePlayer::Is_PlacementSuppressed(const uint64_t placementId) const
{
	return m_SuppressedPlacements.contains(placementId);
}

bool_t CWorldSequencePlayer::Is_Playing(const std::string& instanceId) const
{
	return m_Active.end() != std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
}

bool_t CWorldSequencePlayer::Try_GetElapsedMs(
	const std::string& instanceId,
	f32_t& outElapsedMs) const
{
	const auto found = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	if (m_Active.end() == found)
		return false;
	outElapsedMs = found->elapsedMs;
	return true;
}

bool_t CWorldSequencePlayer::Try_GetSampledPlacementRecord(
	const std::string& instanceId, const uint64_t placementId,
	MAP_PLACEMENT_RECORD& outRecord) const
{
	const auto active = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value) { return value.instanceId == instanceId; });
	if (active == m_Active.end()) return false;
	const auto sampled = active->sampledPlacements.find(placementId);
	if (sampled == active->sampledPlacements.end()) return false;
	outRecord = sampled->second;
	return true;
}

void CWorldSequencePlayer::Stop_Instance(
	const std::string& instanceId, const TARGET_SET& targets, const bool_t restorePlacements)
{
	const auto held = std::find_if(m_Held.begin(), m_Held.end(),
		[&](const ACTIVE_INSTANCE& value) { return value.instanceId == instanceId; });
	if (held != m_Held.end())
	{
		m_Active.push_back(std::move(*held));
		m_Held.erase(held);
	}
	const auto found = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value) { return value.instanceId == instanceId; });
	if (found == m_Active.end()) return;
	if (restorePlacements && targets.Is_Complete())
		for (const auto& baseline : found->placementBaselines)
			if (auto* entry = Find_Placement(*targets.pPlacements, baseline.placementId))
			{
				auto record = baseline.record;
				record.visible = baseline.restoreRuntimeVisible &&
					!m_SuppressedPlacements.contains(baseline.placementId);
				(void)Apply_RuntimeRecord(targets, m_ModelCache, *entry, record);
			}
	Release_DeployPreviews(*found, targets);
	Release_Objects(*found);
	m_Active.erase(found);
}

void CWorldSequencePlayer::Stop_All(const TARGET_SET& targets, const bool_t restorePlacements)
{
	while (!m_Active.empty()) Stop_Instance(m_Active.back().instanceId, targets, restorePlacements);
	while (!m_Held.empty())
	{
		const auto id = m_Held.back().instanceId;
		Stop_Instance(id, targets, restorePlacements);
	}
}

bool_t CWorldSequencePlayer::Seek_InstanceToMs(
	const std::string& instanceId, const f32_t elapsedMs, const TARGET_SET& targets)
{
	if (!targets.Is_Complete() || !std::isfinite(elapsedMs) || elapsedMs < 0.f) return false;
	const auto found = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value) { return value.instanceId == instanceId; });
	if (found == m_Active.end()) return false;
	found->elapsedMs = elapsedMs;
	return APPLY_RESULT::FAILED != Apply_Instance(*found, targets);
}

void CWorldSequencePlayer::Release_DeployPreviews(
	const ACTIVE_INSTANCE& active,
	const TARGET_SET& targets)
{
	if (nullptr == targets.pDeployRuntime)
		return;
	for (const uint64_t targetId : active.deployTargets)
	{
		const shared_ptr<CDeployPropObject> object =
			targets.pDeployRuntime->Find(targetId);
		if (nullptr != object)
			object->End_AnimationAuthoringPreview();
	}
}

bool_t CWorldSequencePlayer::Seek_AllToMs(
	const f32_t elapsedMs,
	const TARGET_SET& targets)
{
	if (m_Active.empty() || !targets.Is_Complete() ||
		!std::isfinite(elapsedMs) || elapsedMs < 0.f)
	{
		return false;
	}
	/* A scrub rewrites the clock rather than advancing it, and applies the
	   frame in place. A finished instance stays in the list here so the
	   authoring scrub can step back into it. */
	bool_t succeeded = true;
	for (size_t index = 0; index < m_Active.size();)
	{
		ACTIVE_INSTANCE& active = m_Active[index];
		active.elapsedMs = elapsedMs;
		if (APPLY_RESULT::FAILED == Apply_Instance(active, targets))
		{
			const auto id = active.instanceId;
			Stop_Instance(id, targets, true);
			succeeded = false;
		}
		else ++index;
	}
	return succeeded;
}

f32_t CWorldSequencePlayer::Get_InstanceElapsedSpanMs(const std::string& instanceId,
    const f32_t playbackSpeed, const uint32_t durationMs) const
{
    if (!std::isfinite(playbackSpeed) || playbackSpeed <= 0.f) return 0.f;
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance || !instance->enabled) return 0.f;
    bool hasEffects = false;
    const auto* probe = instance;
    for (uint32_t depth = 0; probe && depth <= 32u; ++depth)
    {
        const auto* sequence = m_Document.Find_Template(probe->templateId);
        if (!sequence) return 0.f;
        hasEffects |= !sequence->effectTracks.empty();
        if (probe->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
        probe = m_Document.Find_Instance(probe->nextMotionId);
    }
    if (!hasEffects)
    {
        const auto* sequence = m_Document.Find_Template(instance->templateId);
        return durationMs ? static_cast<f32_t>(durationMs) : instance->startDelayMs +
            sequence->durationMs / (instance->playbackSpeed * playbackSpeed);
    }
    const double cutoff = durationMs ? durationMs : (std::numeric_limits<double>::max)();
    double start = 0., span = 0.;
    for (uint32_t depth = 0; instance && depth <= 32u; ++depth)
    {
        const auto* sequence = m_Document.Find_Template(instance->templateId);
        if (!sequence || !instance->enabled) return 0.f;
        const double rate = instance->playbackSpeed * playbackSpeed;
        if (!std::isfinite(rate) || rate <= 0.) return 0.f;
        start += instance->startDelayMs;
        if (start >= cutoff) break;
        const double period = sequence->ObjectSpanMs() / rate;
        const bool loop = instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
        if (loop && !durationMs) return static_cast<f32_t>(CWorldSequenceDocument::MAX_DURATION_MS);
        double tail = sequence->durationMs;
        for (const auto& effect : sequence->effectTracks)
            tail = (std::max)(tail, static_cast<double>(sequence->EffectStartMs(effect) + effect.durationMs));
        for (uint32_t emitter = 0; emitter < sequence->objectMotion.EmissionCount(); ++emitter)
        {
            double birth = start + static_cast<double>(sequence->objectMotion.EmissionDelayMs(emitter)) / rate;
            if (birth >= cutoff) continue;
            if (loop) birth += (std::max)(0., std::ceil((cutoff - birth) / period) - 1.) * period;
            span = (std::max)(span, birth + tail / rate);
        }
        if (instance->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
        start += period;
        instance = m_Document.Find_Instance(instance->nextMotionId);
    }
    return std::isfinite(span) ? static_cast<f32_t>(span) : 0.f;
}


f32_t CWorldSequencePlayer::Get_LongestElapsedSpanMs() const
{
	f32_t longest = 0.f;
	for (const ACTIVE_INSTANCE& active : m_Active)
	{
		const f32_t span = Get_InstanceElapsedSpanMs(active.instanceId, active.playbackSpeed, active.durationMs);
		longest = (std::max)(longest, span);
	}
	return longest;
}

void CWorldSequencePlayer::Update(
	const f32_t timeDelta,
	const TARGET_SET& targets)
{
	if (m_bPaused)
		return;
	if (m_Active.empty() || !targets.Is_Complete() ||
		!std::isfinite(timeDelta) || timeDelta < 0.f)
	{
		return;
	}
	for (size_t index = 0; index < m_Active.size();)
	{
		ACTIVE_INSTANCE& active = m_Active[index];
		active.elapsedMs += timeDelta * 1000.f;
		const APPLY_RESULT result = Apply_Instance(active, targets);
		if (APPLY_RESULT::PLAYING == result)
		{
			++index;
			continue;
		}
		/* Only a broken instance hands its animated targets back; a finished
		   one leaves them holding the authored final frame. */
		if (APPLY_RESULT::FAILED == result || active.durationMs != 0u)
		{
			const auto id = active.instanceId;
			Stop_Instance(id, targets, true);
		}
		else
		{
			Release_Objects(active);
			m_Held.push_back(std::move(active));
			m_Active.erase(m_Active.begin() + static_cast<ptrdiff_t>(index));
		}
	}
}

CWorldSequencePlayer::APPLY_RESULT CWorldSequencePlayer::Apply_Instance(
	ACTIVE_INSTANCE& active,
	const TARGET_SET& targets)
{
	const WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(active.instanceId);
	const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		m_Document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence)
	{
		m_Status = "World sequence disappeared while playing: " +
			active.instanceId;
		return APPLY_RESULT::FAILED;
	}
	bool hasObjectEffects = false;
	for (const auto& motionId : {active.instanceId, active.motionInstanceId})
	{
		const auto* motion = m_Document.Find_Instance(motionId);
		for (uint32_t depth = 0; motion && depth <= 32u; ++depth)
		{
			const auto* motionSequence = m_Document.Find_Template(motion->templateId);
			if (!motionSequence) break;
			if (!motionSequence->effectTracks.empty()) { hasObjectEffects = true; break; }
			if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
			motion = m_Document.Find_Instance(motion->nextMotionId);
		}
	}
	if (Is_SingleObjectMotion(*instance) && !Apply_ObjectEffects(active, *instance, targets))
		return APPLY_RESULT::FAILED;
	if (hasObjectEffects && active.durationMs && active.effects.empty() &&
		active.elapsedMs >= Get_InstanceElapsedSpanMs(active.instanceId, active.playbackSpeed, active.durationMs))
	{
		(void)Apply_Objects(active, *instance, *sequence, targets, 0.f, false);
		return APPLY_RESULT::FINISHED;
	}
	if (active.durationMs && active.elapsedMs >= active.durationMs && !hasObjectEffects)
	{
		(void)Apply_Objects(active, *instance, *sequence, targets, 0.f, false);
		return active.effects.empty() ? APPLY_RESULT::FINISHED : APPLY_RESULT::PLAYING;
	}
	if (Is_SingleObjectMotion(*instance))
	{
		OBJECT_MOTION_SAMPLE sample;
		const bool_t appliedMotion = !active.motionInstanceId.empty() &&
			active.elapsedMs >= active.motionStartMs;
		const auto& motionId = appliedMotion ? active.motionInstanceId : active.instanceId;
		if (!Resolve_ObjectMotion(m_Document, motionId,
			active.elapsedMs - (appliedMotion ? active.motionStartMs : 0.f), active.playbackSpeed, sample))
		{
			m_Status = "World Object motion clock or completion target is invalid: " + motionId;
			return APPLY_RESULT::FAILED;
		}
		if (appliedMotion && sample.pending)
		{
			// An action's start delay leaves the existing object's base motion visible.
			sample = {};
			if (!Resolve_ObjectMotion(m_Document, active.instanceId,
				active.elapsedMs, active.playbackSpeed, sample))
				return APPLY_RESULT::FAILED;
		}
		if (appliedMotion && sample.finished)
		{
			// STOP stops the applied motion, not the target object's lifetime.
			sample.visible = true;
			sample.finished = false;
			sample.holdFinalPose = true;
		}
		// Keep the target's binding/anchor/placement and reuse its existing object. Only the motion changes.
		if (!Apply_Objects(active, *instance, *sample.sequence, targets, sample.localMs, sample.visible, sample.holdFinalPose,
			sample.emissionStartMs + (appliedMotion ? active.motionStartMs : 0.f), sample.emissionRate, sample.emissionMotionId))
			return APPLY_RESULT::FAILED;
		return sample.finished && (active.durationMs == 0u || hasObjectEffects) && active.effects.empty() ? APPLY_RESULT::FINISHED : APPLY_RESULT::PLAYING;
	}
	const f32_t delayedMs =
		active.elapsedMs - static_cast<f32_t>(instance->startDelayMs);
	const f32_t durationMs = static_cast<f32_t>(sequence->durationMs);
	const f32_t localMs = delayedMs <= 0.f ? 0.f :
		(std::min)(durationMs, delayedMs * instance->playbackSpeed * active.playbackSpeed);

	if (!Apply_Objects(active, *instance, *sequence, targets, localMs, delayedMs >= 0.f && localMs < durationMs))
		return APPLY_RESULT::FAILED;
	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
		if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
		uint64_t targetId = 0;
		if (!Try_ParseTargetId(binding, targetId))
			return APPLY_RESULT::FAILED;
		if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
		{
			f32_t windowEndMs = durationMs;
			const WORLD_SEQUENCE_ANIMATION_TRACK* animationTrack =
				Find_AnimationTrackAt(*sequence, binding.slotId, localMs,
					windowEndMs);
			const shared_ptr<CDeployPropObject> object =
				targets.pDeployRuntime->Find(targetId);
			if (nullptr == animationTrack || nullptr == object)
				return APPLY_RESULT::FAILED;
			/* Before its start delay an instance must not pose its target. The
			   placement branch already holds the baseline here; parking a
			   Deploy prop on frame 0 instead would let a later beat of a
			   chained cutscene overwrite the beat that is actually playing. */
			if (delayedMs < 0.f)
				continue;
			f32_t normalized = 0.f;
			{
				f32_t clipSeconds = 0.f;
				for (const DEPLOY_PROP_ANIMATION_CLIP& clip :
					object->Get_AnimationClips())
				{
					if (clip.name == animationTrack->clipName)
					{
						clipSeconds = clip.durationSeconds;
						break;
					}
				}
				if (!std::isfinite(clipSeconds) || clipSeconds <= 0.f)
					return APPLY_RESULT::FAILED;
				const f32_t windowMs = (std::max)(0.f,
					localMs - static_cast<f32_t>(animationTrack->startMs));
				normalized = windowMs * animationTrack->playbackRate /
					(clipSeconds * 1000.f);
				/* holdLastFrame wins at the end even for a looping clip. A
				   sequence whose duration rounds a hair past the clip would
				   otherwise wrap to frame 0 on its very last sample and snap
				   an unfolded prop shut. */
				if (localMs >= windowEndMs && animationTrack->holdLastFrame)
					normalized = 1.f;
				else if (animationTrack->loop)
					normalized = std::fmod(normalized, 1.f);
				else if (normalized > 1.f)
					normalized = animationTrack->holdLastFrame ? 1.f : 0.f;
			}
			/* The sampler rewinds a looping clip to frame 0 the moment it is
			   asked for 1.0, so the settled frame must be requested as a
			   non-looping seek or the hold would fold the prop shut. */
			const bool_t settledOnLastFrame =
				localMs >= windowEndMs && animationTrack->holdLastFrame;
			if (!object->Sample_AnimationAuthoringPreview(
				animationTrack->clipName, Clamp01(normalized),
				settledOnLastFrame ? false : animationTrack->loop))
			{
				return APPLY_RESULT::FAILED;
			}
			/* A cutscene may also walk the prop while its clips play. The
			   offset is authored in the placement's own frame, exactly as a
			   map placement composes, so the same authored numbers mean the
			   same thing for both target kinds. */
			const WORLD_SEQUENCE_TRACK* const moveTrack =
				Find_Track(*sequence, binding.slotId);
			if (nullptr != moveTrack)
			{
				float3_t placedPosition{};
				float4_t placedRotation(0.f, 0.f, 0.f, 1.f);
				if (!object->Get_PlacedRootPose(placedPosition, placedRotation))
					return APPLY_RESULT::FAILED;
				const WORLD_SEQUENCE_TRANSFORM_KEY key =
					Sample_Track(*sequence, *moveTrack, localMs);
				const vector_t placed =
					XMQuaternionNormalize(XMLoadFloat4(&placedRotation));
				float3_t rotatedOffset;
				XMStoreFloat3(&rotatedOffset, XMVector3Rotate(
					XMLoadFloat3(&key.positionOffset), placed));
				const float3_t posedPosition(
					placedPosition.x + rotatedOffset.x + active.positionOffset.x,
					placedPosition.y + rotatedOffset.y + active.positionOffset.y,
					placedPosition.z + rotatedOffset.z + active.positionOffset.z);
				vector_t combined = XMQuaternionNormalize(XMQuaternionMultiply(
					XMLoadFloat4(&key.rotationQuaternion), placed));
				if (XMVectorGetW(combined) < 0.f)
					combined = XMVectorNegate(combined);
				float4_t posedRotation;
				XMStoreFloat4(&posedRotation, combined);
				if (!object->Apply_AnimationAuthoringPose(
					posedPosition, posedRotation))
				{
					return APPLY_RESULT::FAILED;
				}
			}
			continue;
		}
		const WORLD_SEQUENCE_TRACK* track = Find_Track(*sequence, binding.slotId);
		const auto baseline = std::find_if(active.placementBaselines.begin(),
			active.placementBaselines.end(),
			[targetId](const PLACEMENT_BASELINE& value)
			{
				return value.placementId == targetId;
			});
		MAP_RUNTIME_PLACED_ENTRY* entry =
			Find_Placement(*targets.pPlacements, targetId);
		if (nullptr == track || active.placementBaselines.end() == baseline ||
			nullptr == entry)
		{
			return APPLY_RESULT::FAILED;
		}
		MAP_PLACEMENT_RECORD sampled = delayedMs < 0.f ?
			baseline->record :
			Compose_SampledRecord(baseline->record, baseline->runtimeVisible,
				Sample_Track(*sequence, *track, localMs));
		if (delayedMs >= 0.f)
		{
			sampled.position.x += active.positionOffset.x;
			sampled.position.y += active.positionOffset.y;
			sampled.position.z += active.positionOffset.z;
		}
		if (m_SuppressedPlacements.contains(targetId))
			sampled.visible = false;
		if (!Apply_RuntimeRecord(targets, m_ModelCache, *entry, sampled))
			return APPLY_RESULT::FAILED;
		active.sampledPlacements[targetId] = std::move(sampled);
	}
	/* Hold the settled pose: the sequence stops driving its targets once the
	   authored duration is spent, leaving the last authored frame in place. */
	return (localMs < durationMs || (active.durationMs && active.elapsedMs < active.durationMs)) ?
		APPLY_RESULT::PLAYING : APPLY_RESULT::FINISHED;
}
