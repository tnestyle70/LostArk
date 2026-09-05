#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

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
};

struct WORLD_SEQUENCE_BINDING
{
	std::string slotId;
	WORLD_SEQUENCE_TARGET_KIND targetKind =
		WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	std::string targetId;
};

struct WORLD_SEQUENCE_INSTANCE
{
	std::string instanceId;
	std::string templateId;
	bool_t enabled = true;
	uint32_t startDelayMs = 0;
	f32_t playbackSpeed = 1.f;
	std::vector<WORLD_SEQUENCE_BINDING> bindings;
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
	static bool_t Is_ValidStableId(const std::string& value);

private:
	std::string m_AreaId;
	uint32_t m_iRevision = 1;
	std::vector<WORLD_SEQUENCE_TEMPLATE> m_Templates;
	std::vector<WORLD_SEQUENCE_INSTANCE> m_Instances;
};

NS_END
