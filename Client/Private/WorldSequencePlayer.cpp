#include "WorldSequencePlayer.h"

#include "DeployPropObject.h"
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "Model.h"

#include <algorithm>
#include <cmath>

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
	Clear();
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
	if (!m_Document.Load(path, areaId,
		Collect_Placements(*targets.pCatalog, *targets.pPlacements),
		Collect_DeployPlacements(*targets.pDeployRuntime), status))
	{
		m_Document.Reset_Empty({});
		m_Status = "World sequence load failed: " + status;
		return false;
	}
	m_Status = "World sequence loaded: " +
		std::to_string(m_Document.Get_Instances().size()) + " instances";
	return true;
}

void CWorldSequencePlayer::Clear()
{
	m_Active.clear();
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

bool_t CWorldSequencePlayer::Play(
	const std::string& instanceId,
	const TARGET_SET& targets)
{
	if (!Is_Ready() || !targets.Is_Complete())
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

	const auto existing = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	if (m_Active.end() != existing)
	{
		existing->elapsedMs = 0.f;
		return true;
	}

	/* Capture the live pose of every bound target before the first sample so
	   a replay composes against the placed transform, not against whatever the
	   previous play left behind. */
	ACTIVE_INSTANCE active;
	active.instanceId = instanceId;
	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
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
		active.placementBaselines.push_back(std::move(baseline));
	}
	m_Active.push_back(std::move(active));
	m_Status = "World sequence started: " + instanceId;
	return true;
}

bool_t CWorldSequencePlayer::Is_Playing(const std::string& instanceId) const
{
	return m_Active.end() != std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
}

void CWorldSequencePlayer::Stop_All()
{
	m_Active.clear();
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

void CWorldSequencePlayer::Update(
	const f32_t timeDelta,
	const TARGET_SET& targets)
{
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
		if (APPLY_RESULT::FAILED == result)
			Release_DeployPreviews(active, targets);
		m_Active.erase(m_Active.begin() + static_cast<ptrdiff_t>(index));
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
	const f32_t delayedMs =
		active.elapsedMs - static_cast<f32_t>(instance->startDelayMs);
	const f32_t durationMs = static_cast<f32_t>(sequence->durationMs);
	const f32_t localMs = delayedMs <= 0.f ? 0.f :
		(std::min)(durationMs, delayedMs * instance->playbackSpeed);

	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
		uint64_t targetId = 0;
		if (!Try_ParseTargetId(binding, targetId))
			return APPLY_RESULT::FAILED;
		if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK* animationTrack =
				Find_AnimationTrack(*sequence, binding.slotId);
			const shared_ptr<CDeployPropObject> object =
				targets.pDeployRuntime->Find(targetId);
			if (nullptr == animationTrack || nullptr == object)
				return APPLY_RESULT::FAILED;
			f32_t normalized = 0.f;
			if (delayedMs >= 0.f)
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
				normalized = localMs * animationTrack->playbackRate /
					(clipSeconds * 1000.f);
				/* holdLastFrame wins at the end even for a looping clip. A
				   sequence whose duration rounds a hair past the clip would
				   otherwise wrap to frame 0 on its very last sample and snap
				   an unfolded prop shut. */
				if (localMs >= durationMs && animationTrack->holdLastFrame)
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
				localMs >= durationMs && animationTrack->holdLastFrame;
			if (!object->Sample_AnimationAuthoringPreview(
				animationTrack->clipName, Clamp01(normalized),
				settledOnLastFrame ? false : animationTrack->loop))
			{
				return APPLY_RESULT::FAILED;
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
		const MAP_PLACEMENT_RECORD sampled = delayedMs < 0.f ?
			baseline->record :
			Compose_SampledRecord(baseline->record, baseline->runtimeVisible,
				Sample_Track(*sequence, *track, localMs));
		if (!Apply_RuntimeRecord(targets, m_ModelCache, *entry, sampled))
			return APPLY_RESULT::FAILED;
	}
	/* Hold the settled pose: the sequence stops driving its targets once the
	   authored duration is spent, leaving the last authored frame in place. */
	return localMs < durationMs ?
		APPLY_RESULT::PLAYING : APPLY_RESULT::FINISHED;
}
