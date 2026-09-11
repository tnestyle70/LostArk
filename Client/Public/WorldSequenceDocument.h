#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <array>
#include <map>
#include <optional>
#include <algorithm>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine { struct MODEL_MATERIAL_OVERRIDE; }

NS_BEGIN(Client)

struct WORLD_SEQUENCE_PLACEMENT_INFO
{
	float3_t signedScale = float3_t(1.f, 1.f, 1.f);
	bool_t sequenceTargetSupported = true;
};

using WORLD_SEQUENCE_PLACEMENT_MAP =
	std::unordered_map<uint64_t, WORLD_SEQUENCE_PLACEMENT_INFO>;

struct WORLD_SEQUENCE_DEPLOY_INFO
{
	bool_t animationTargetSupported = false;
	std::vector<std::string> animationClips;
};

using WORLD_SEQUENCE_DEPLOY_MAP =
	std::unordered_map<uint64_t, WORLD_SEQUENCE_DEPLOY_INFO>;

enum class WORLD_SEQUENCE_TARGET_KIND
{
	MAP_PLACEMENT,
	DEPLOY_PLACEMENT,
	OBJECT_RESOURCE,
};

struct WORLD_SEQUENCE_MATERIAL_TEXTURE
{
    uint32_t expressionIndex = 0u;
    std::string assetId;
    bool_t srgb = false;
    bool operator==(const WORLD_SEQUENCE_MATERIAL_TEXTURE&) const = default;
};

struct WORLD_SEQUENCE_MATERIAL_PROFILE
{
    std::string materialName;
    std::string sourceMaterial;
    std::string family;
    std::map<std::string, std::array<float, 4>> parameters;
    std::vector<WORLD_SEQUENCE_MATERIAL_TEXTURE> textures;
    bool operator==(const WORLD_SEQUENCE_MATERIAL_PROFILE&) const = default;
};

struct WORLD_SEQUENCE_OBJECT_RESOURCE
{
	std::string objectId;
	std::string displayName;
	std::string modelAssetId;
	// Resource category and default anchor for its authored states.
	std::string anchorKind = "WORLD";
	// BOSS states follow this replicated actor and BODY bone (empty = root).
	std::string anchorBossArchetypeId;
	std::string anchorBone;
	std::string diffuseTextureAssetId;
	// Immutable source material input shared by every Motion of this resource.
	std::optional<WORLD_SEQUENCE_MATERIAL_PROFILE> materialProfile;
	f32_t modelPreScale = 0.01f;
	bool_t animated = false;
	float3_t scale = {1.f, 1.f, 1.f};
	// Existing placed curtain/roulette aliases name a sequence instead of a model.
	std::string sequenceInstanceId;
	// Empty means no initial Motion has been chosen; never infer vector order.
	std::string defaultMotionInstanceId;
};

struct WORLD_SEQUENCE_OBJECT_MOTION
{
	float3_t velocity = {};
	float3_t acceleration = {};
	float3_t angularVelocityDegrees = {};
	float3_t revolutionDegreesPerSecond = {};
	float3_t revolutionOffset = {};
	// Per-emitter position range around its captured origin, in local metres.
	float3_t spawnHalfExtents = {};
	uint32_t count = 1u;
	uint32_t intervalMs = 0u;
	f32_t spreadDegrees = 0.f;
	uint32_t seed = 1u;
};

enum class WORLD_SEQUENCE_INTERPOLATION
{
	LINEAR,
	SMOOTH_STEP,
};

struct WORLD_SEQUENCE_TRANSFORM_KEY
{
	uint32_t timeMs = 0;
	float3_t positionOffset = {};
	float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t scaleMultiplier = float3_t(1.f, 1.f, 1.f);
	bool_t visible = true;
};

struct WORLD_SEQUENCE_TRACK
{
	std::string slotId;
	std::vector<WORLD_SEQUENCE_TRANSFORM_KEY> keys;
};

struct WORLD_SEQUENCE_ANIMATION_TRACK
{
	std::string slotId;
	/* Several tracks may share one slot to play clips back to back. Each owns
	   the window from its own startMs to the next one's, so a cutscene beat
	   list stays one instance driving one target instead of several instances
	   fighting over it. The first track of a slot must start at 0. */
	uint32_t startMs = 0;
	std::string clipName;
	f32_t playbackRate = 1.f;
	bool_t loop = false;
	bool_t holdLastFrame = true;
	// Authoring label only; clipName remains the model animation lookup key.
	std::string displayName;
};

struct WORLD_SEQUENCE_EFFECT_TRACK
{
	std::string effectTrackId;
	std::string slotId;
	std::string resourceKind = "GROUP";
	std::string resourceId;
	std::string timing = "MOTION_END";
	uint32_t startMs = 0;
	uint32_t durationMs = 1000;
	float3_t positionOffset = {};
	float3_t rotationDegrees = {};
	float3_t scale = {1.f, 1.f, 1.f};
};

struct WORLD_SEQUENCE_TEMPLATE
{
	std::string sequenceId;
	std::string displayName;
	std::string category = "World";
	uint32_t durationMs = 1000;
	WORLD_SEQUENCE_INTERPOLATION interpolation =
		WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP;
	std::vector<WORLD_SEQUENCE_TRACK> tracks;
	std::vector<WORLD_SEQUENCE_ANIMATION_TRACK> animationTracks;
	std::vector<WORLD_SEQUENCE_EFFECT_TRACK> effectTracks;
	WORLD_SEQUENCE_OBJECT_MOTION objectMotion;
	uint32_t EffectStartMs(const WORLD_SEQUENCE_EFFECT_TRACK& effect) const noexcept
	{ return effect.timing == "MOTION_END" ? durationMs : effect.startMs; }
	uint32_t ObjectSpanMs() const noexcept
	{ return durationMs + (effectTracks.empty() ? 0u : (objectMotion.count - 1u) * objectMotion.intervalMs); }
	uint32_t PresentationSpanMs() const noexcept
	{
		uint32_t span = durationMs;
		for (const auto& effect : effectTracks)
			span = (std::max)(span, EffectStartMs(effect) + effect.durationMs);
		return span + (effectTracks.empty() ? 0u : (objectMotion.count - 1u) * objectMotion.intervalMs);
	}
};

struct WORLD_SEQUENCE_BINDING
{
	std::string slotId;
	WORLD_SEQUENCE_TARGET_KIND targetKind =
		WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	std::string targetId;
};

enum class WORLD_SEQUENCE_MOTION_END
{
	STOP,
	HOLD,
	LOOP,
	NEXT,
};

// A horizontal circle in the bound placement local space, measured after model import scale.
struct WORLD_SEQUENCE_WALKABLE_SURFACE
{
	f32_t radiusM = 1.f;
	f32_t localHeightM = 0.f;
};

struct WORLD_SEQUENCE_INSTANCE
{
	std::string instanceId;
	std::string templateId;
	bool_t enabled = true;
	uint32_t startDelayMs = 0;
	f32_t playbackSpeed = 1.f;
	std::vector<WORLD_SEQUENCE_BINDING> bindings;
	std::string anchorKind = "WORLD";
	float3_t position = {};
	WORLD_SEQUENCE_MOTION_END motionEnd = WORLD_SEQUENCE_MOTION_END::STOP;
	std::string nextMotionId;
	std::optional<WORLD_SEQUENCE_WALKABLE_SURFACE> walkableSurface;
};

class CWorldSequenceDocument final
{
public:
	static constexpr uint32_t MAX_TEMPLATE_COUNT = 256;
	static constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
	static constexpr uint32_t MAX_TRACK_COUNT = 32;
	static constexpr uint32_t MAX_KEY_COUNT = 256;
	static constexpr uint32_t MAX_DURATION_MS = 600000;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus) const;
	bool_t Validate(
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus) const;

	void Reset_Empty(const std::string& areaId);
	void Touch();

	WORLD_SEQUENCE_TEMPLATE* Find_Template(const std::string& sequenceId);
	const WORLD_SEQUENCE_TEMPLATE* Find_Template(
		const std::string& sequenceId) const;
	WORLD_SEQUENCE_INSTANCE* Find_Instance(const std::string& instanceId);
	const WORLD_SEQUENCE_INSTANCE* Find_Instance(
		const std::string& instanceId) const;
	bool_t Is_Equivalent(const CWorldSequenceDocument& other) const;
	WORLD_SEQUENCE_OBJECT_RESOURCE* Find_ObjectResource(const std::string& objectId);
	const WORLD_SEQUENCE_OBJECT_RESOURCE* Find_ObjectResource(const std::string& objectId) const;
	std::vector<WORLD_SEQUENCE_OBJECT_RESOURCE>& Get_ObjectResources() noexcept { return m_ObjectResources; }
	const std::vector<WORLD_SEQUENCE_OBJECT_RESOURCE>& Get_ObjectResources() const noexcept { return m_ObjectResources; }

	const std::string& Get_AreaId() const noexcept { return m_AreaId; }
	uint32_t Get_Revision() const noexcept { return m_iRevision; }
	std::vector<WORLD_SEQUENCE_TEMPLATE>& Get_Templates() noexcept
	{
		return m_Templates;
	}
	const std::vector<WORLD_SEQUENCE_TEMPLATE>& Get_Templates() const noexcept
	{
		return m_Templates;
	}
	std::vector<WORLD_SEQUENCE_INSTANCE>& Get_Instances() noexcept
	{
		return m_Instances;
	}
	const std::vector<WORLD_SEQUENCE_INSTANCE>& Get_Instances() const noexcept
	{
		return m_Instances;
	}

    static bool_t Build_MaterialOverride(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        const std::filesystem::path& resourceRoot, Engine::MODEL_MATERIAL_OVERRIDE& out);

	static const char_t* Interpolation_ToString(
		WORLD_SEQUENCE_INTERPOLATION interpolation);
	static bool_t Try_ParseInterpolation(
		const std::string& value,
		WORLD_SEQUENCE_INTERPOLATION& outInterpolation);
	static const char_t* TargetKind_ToString(
		WORLD_SEQUENCE_TARGET_KIND targetKind);
	static bool_t Try_ParseTargetKind(
		const std::string& value,
		WORLD_SEQUENCE_TARGET_KIND& outTargetKind);
	static const char_t* MotionEnd_ToString(WORLD_SEQUENCE_MOTION_END motionEnd);
	static bool_t Try_ParseMotionEnd(const std::string& value,
		WORLD_SEQUENCE_MOTION_END& outMotionEnd);
	static bool_t Is_ValidStableId(const std::string& value);

private:
	std::string m_AreaId;
	uint32_t m_iRevision = 1;
	std::vector<WORLD_SEQUENCE_TEMPLATE> m_Templates;
	std::vector<WORLD_SEQUENCE_INSTANCE> m_Instances;
	std::vector<WORLD_SEQUENCE_OBJECT_RESOURCE> m_ObjectResources;
};

NS_END
