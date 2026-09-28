# G03 전체 코드 · 도입15 카메라와 NPC 표정 preview

기존 PLAN의 G03 부록이다. C++/Python은 현재 구현 전문, JSON은 승인 후 저장·게시한 변경 행이다. 새 C++ 및 project/filter 등록 없음. 자동 생성 스냅샷이며 원본 파일을 직접 수정한 뒤 재생성한다.

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Public/WorldSequenceDocument.h

```cpp
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

namespace Engine { struct MODEL_MATERIAL_OVERRIDE; struct MODEL_SOURCE_CHARACTER_PARAMETERS; }

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

struct WORLD_SEQUENCE_MAP_MATERIAL_BINDING
{
    std::string materialName;
    std::string sourceAssetId;
    std::string sourceMaterialName;
    std::string diffuseTextureAssetId;
    bool unlit = false;
    bool operator==(const WORLD_SEQUENCE_MAP_MATERIAL_BINDING&) const = default;
};

struct WORLD_SEQUENCE_COMBAT_BODY
{
	uint32_t maxHp = 2000;
	// Local metres after modelPreScale, before resource/occurrence scale.
	float3_t localCenterM = {};
	float3_t halfExtentsM = {.5f, .5f, .5f};
	// ELLIPSOID interprets halfExtentsM as semiaxes; omitted authoring stays BOX.
	std::string shape = "BOX";
	std::string lifetimePolicy = "UNTIL_DESTROYED";
};

// Authoring organization only; folders never own transforms or playable motions.
struct WORLD_SEQUENCE_OBJECT_FOLDER
{
    std::string folderId;
    std::string displayName;
    std::string anchorKind = "WORLD";
    std::string parentId;
    bool operator==(const WORLD_SEQUENCE_OBJECT_FOLDER&) const = default;
};

struct WORLD_SEQUENCE_OBJECT_RESOURCE
{
	std::string objectId;
	std::string displayName;
	// Optional organizational parent: a folder or another Object resource.
	std::string parentId;
	std::string modelAssetId;
	/* Optional clip donor for a skinned body whose clips ship in a separate
	   AnimSet WModel (the Valtan bodies). Attached once at model admission;
	   empty keeps the clips embedded in modelAssetId. */
	std::string animationSetAssetId;
	/* Optional boss catalog archetype whose product presentation draws this
	   skinned body: the admitted body prototype plus its armour plates and
	   socketed weapon. The body fields above must match that catalog row;
	   empty keeps a single model. */
	std::string presentationBossArchetypeId;
	// Resource category and default anchor for its authored states.
	std::string anchorKind = "WORLD";
	// BOSS states follow this replicated actor and BODY bone (empty = root).
	std::string anchorBossArchetypeId;
	std::string anchorBone;
	std::string diffuseTextureAssetId;
	// Immutable source material input shared by every Motion of this resource.
	std::optional<WORLD_SEQUENCE_MATERIAL_PROFILE> materialProfile;
	std::optional<WORLD_SEQUENCE_COMBAT_BODY> combatBody;
    // Derived static/skinned objects retain their explicit original material owner.
    std::string materialSourceModelAssetId;
    // Reuse admitted map surface inputs, without a static placement's baked light.
    std::vector<WORLD_SEQUENCE_MAP_MATERIAL_BINDING> mapMaterialBindings;
	f32_t modelPreScale = 0.01f;
	bool_t animated = false;
	float3_t scale = {1.f, 1.f, 1.f};
	// Existing placed curtain/roulette aliases name a sequence instead of a model.
	std::string sequenceInstanceId;
	// Empty means no initial Motion has been chosen; never infer vector order.
	std::string defaultMotionInstanceId;
	// A model-less Object Resources group references existing map motions.
	std::vector<std::string> motionInstanceIds;
};

/* One authored emission of an Object motion. Offset and yaw sit in the motion's
   local frame before the WORLD placement, so every row replays the same keys,
   physics and revolution from its own lane, heading and delay instead of a
   copied motion. */
struct WORLD_SEQUENCE_OBJECT_EMISSION
{
	float3_t positionOffset = {};
	f32_t yawDegrees = 0.f;
	uint32_t startDelayMs = 0u;
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
	/* Empty keeps the seeded count/interval/spread emitter. Authored rows own
	   the count (count == emissions.size()) and force interval/spread to 0. */
	std::vector<WORLD_SEQUENCE_OBJECT_EMISSION> emissions;
	uint32_t EmissionCount() const noexcept
	{ return emissions.empty() ? count : static_cast<uint32_t>(emissions.size()); }
	uint32_t EmissionDelayMs(const uint32_t emitter) const noexcept
	{
		if (emissions.empty()) return emitter * intervalMs;
		return emissions[(std::min)(static_cast<size_t>(emitter), emissions.size() - 1u)].startDelayMs;
	}
	uint32_t LastEmissionDelayMs() const noexcept
	{
		if (emissions.empty()) return (count - 1u) * intervalMs;
		uint32_t last = 0u;
		for (const auto& emission : emissions) last = (std::max)(last, emission.startDelayMs);
		return last;
	}
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
	   fighting over it. Slot starts must increase; the first may be delayed. */
	uint32_t startMs = 0;
	std::string clipName;
	f32_t playbackRate = 1.f;
	bool_t loop = false;
	bool_t holdLastFrame = true;
	// Authoring label only; clipName remains the model animation lookup key.
	std::string displayName;
	// Native clip milliseconds before playbackRate; independent of timeline startMs.
	uint32_t sourceStartMs = 0;
	// Zero preserves the native end; otherwise loop/hold is bounded by this source out.
	uint32_t sourceEndMs = 0;
};

// Curves share the object's explicit native material profile and motion clock.
struct WORLD_SEQUENCE_MATERIAL_KEY
{
    uint32_t timeMs = 0u;
    std::array<float, 4> value{};
    bool constant = false;
    bool operator==(const WORLD_SEQUENCE_MATERIAL_KEY&) const = default;
};
struct WORLD_SEQUENCE_MATERIAL_CURVE
{
    std::string parameter;
    std::vector<WORLD_SEQUENCE_MATERIAL_KEY> keys;
    bool operator==(const WORLD_SEQUENCE_MATERIAL_CURVE&) const = default;
};
struct WORLD_SEQUENCE_MATERIAL_TRACK
{
    std::string slotId, materialName;
    std::vector<WORLD_SEQUENCE_MATERIAL_CURVE> curves;
    bool operator==(const WORLD_SEQUENCE_MATERIAL_TRACK&) const = default;
};

struct WORLD_SEQUENCE_EFFECT_TRACK
{
	std::string effectTrackId;
	std::string slotId;
	std::string resourceKind = "GROUP";
	std::string resourceId;
	// V1_EFFECT uses the same authored catalog as the Effect and Sequence tools.
	bool_t followObject = false;
	// Keep emission/placement facing while excluding the model key rotation and self-spin.
	bool_t inheritObjectRotation = true;
	bool_t fitEffectToDuration = false;
	// Repeat finite V1 sources at their native speed, or bound native infinite emitters.
	bool_t loopEffectToDuration = false;
	std::string bone;
	std::string timing = "MOTION_END";
	uint32_t startMs = 0;
	uint32_t durationMs = 1000;
	float3_t positionOffset = {};
	float3_t rotationDegrees = {};
	float3_t scale = {1.f, 1.f, 1.f};
};

struct WORLD_SEQUENCE_COLLIDER_TRACK
{
	std::string colliderTrackId;
	std::string slotId;
	uint32_t startMs = 0;
	uint32_t durationMs = 1000;
	float3_t positionOffset = {};
	// CYLINDER uses [radius, halfHeight, radius]; omitted shape remains BOX.
	float3_t halfExtents = {.5f, .5f, .5f};
	std::string shape = "BOX";
	// Ground collider yaw is independent of the visual mesh spin.
	f32_t yawDegrees = 0.f;
	std::string behavior = "DAMAGE";
	f32_t damagePercent = 20.f;
	// Bone-local metres: after model import scale, before object placement scale.
	float3_t gripLocalOffset = {};
	std::string attachmentBone;
};

struct WORLD_SEQUENCE_SOUND_TRACK
{
    std::string soundTrackId;
    std::string assetId;
    uint32_t startMs = 0u;
    uint32_t durationMs = 1u;
    f32_t volume = 1.f;
    bool_t loopToDuration = false;
    bool operator==(const WORLD_SEQUENCE_SOUND_TRACK&) const = default;
};

struct WORLD_SEQUENCE_SUBTITLE_TRACK
{
    std::string subtitleTrackId;
    std::string stringId;
    std::string text;
    std::string position = "NORMAL";
    std::string slotId;
    uint32_t startMs = 0u;
    uint32_t durationMs = 1u;
    bool operator==(const WORLD_SEQUENCE_SUBTITLE_TRACK&) const = default;
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
    std::vector<WORLD_SEQUENCE_MATERIAL_TRACK> materialTracks;
	std::vector<WORLD_SEQUENCE_COLLIDER_TRACK> colliderTracks;
    std::vector<WORLD_SEQUENCE_SOUND_TRACK> soundTracks;
    std::vector<WORLD_SEQUENCE_SUBTITLE_TRACK> subtitleTracks;
	WORLD_SEQUENCE_OBJECT_MOTION objectMotion;
	uint32_t EffectStartMs(const WORLD_SEQUENCE_EFFECT_TRACK& effect) const noexcept
	{ return effect.timing == "MOTION_END" ? durationMs : effect.startMs; }
	uint32_t ObjectSpanMs() const noexcept
	{ return durationMs + (effectTracks.empty() ? 0u : objectMotion.LastEmissionDelayMs()); }
	uint32_t PresentationSpanMs() const noexcept
	{
		uint32_t span = durationMs;
		for (const auto& effect : effectTracks)
			span = (std::max)(span, EffectStartMs(effect) + effect.durationMs);
        // Finite audio tails retain their own handles without extending the visual owner.
        return span + (effectTracks.empty() ? 0u : objectMotion.LastEmissionDelayMs());
	}
};

struct WORLD_SEQUENCE_BINDING
{
	std::string slotId;
	WORLD_SEQUENCE_TARGET_KIND targetKind =
		WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	std::string targetId;
	// One occurrence replaces only this server NPC's rendering in a live editor.
	std::string previewNpcPlacementId;
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
	bool_t loopFullPresentation = false;
	std::string nextMotionId;
	std::optional<WORLD_SEQUENCE_WALKABLE_SURFACE> walkableSurface;
	uint32_t CycleSpanMs(const WORLD_SEQUENCE_TEMPLATE& sequence) const noexcept
	{ return motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP && loopFullPresentation ?
		(std::max)(sequence.ObjectSpanMs(), sequence.PresentationSpanMs()) : sequence.ObjectSpanMs(); }
};

// Process-local clipboard values; no runtime handles or source document pointers.
struct WORLD_SEQUENCE_OBJECT_BUNDLE
{
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    std::vector<std::string> rootMotionIds;
    std::vector<WORLD_SEQUENCE_TEMPLATE> templates;
    std::vector<WORLD_SEQUENCE_INSTANCE> instances;
};

struct WORLD_SEQUENCE_PASTE_RESULT
{
    std::string objectId;
    std::vector<std::string> rootMotionIds;
    std::vector<std::string> instanceIds;
};

class CWorldSequenceDocument final
{
public:
	static constexpr uint32_t MAX_TEMPLATE_COUNT = 512;
	static constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
	static constexpr uint32_t MAX_TRACK_COUNT = 64;
	// Dense source Matinee curves retain their measured transform tolerance.
	// The existing 16 MiB document and aggregate track budgets still apply.
	static constexpr uint32_t MAX_KEY_COUNT = 4096;
	static constexpr uint32_t MAX_DURATION_MS = 600000;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus);
	// Authoring drafts use exactly the same parser and validation as file loads.
	bool_t Load_Text(std::string_view text, const std::string& expectedAreaId,
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements, std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus) const;
	bool_t Validate(
		const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
		const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
		std::string& outStatus) const;

	// Also accepts unfinished model-less tool drafts; checks only organization.
	bool_t Validate_ObjectHierarchy(std::string& outStatus) const;

    // Empty selection captures every Motion of a model Object; otherwise include NEXT closure.
    bool_t Capture_ObjectBundle(const std::string& objectId,
        const std::vector<std::string>& selectedMotionIds,
        WORLD_SEQUENCE_OBJECT_BUNDLE& outBundle, std::string& outStatus) const;
    // Empty destination creates an Object. Existing destination retains its ID/name/parent.
    // Caller marks the document dirty once after success; failure leaves document/output intact.
    bool_t Paste_ObjectBundle(const WORLD_SEQUENCE_OBJECT_BUNDLE& bundle,
        const std::string& destinationObjectId, const std::string& newObjectName,
        const std::string& parentId, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
        const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements,
        WORLD_SEQUENCE_PASTE_RESULT& outResult, std::string& outStatus);

    // Runtime projection keeps selected groups/aliases, model motion switches and NEXT closure.
    // The caller admits the resulting document through the same Validate path.
    bool_t Build_PlaybackSubset(const std::vector<std::string>& roots,
        CWorldSequenceDocument& out, std::string& status) const;

	void Reset_Empty(const std::string& areaId);
	void Touch();

	WORLD_SEQUENCE_TEMPLATE* Find_Template(const std::string& sequenceId);
	const WORLD_SEQUENCE_TEMPLATE* Find_Template(
		const std::string& sequenceId) const;
	WORLD_SEQUENCE_INSTANCE* Find_Instance(const std::string& instanceId);
	const WORLD_SEQUENCE_INSTANCE* Find_Instance(
		const std::string& instanceId) const;
	bool_t Is_Equivalent(const CWorldSequenceDocument& other) const;
    // Stage and validate before replacing one template. Failure preserves the document.
    bool_t Resize_TimelineDuration(const std::string& sequenceId, uint32_t durationMs,
        uint32_t requiredAnimationEndMs, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
        const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, std::string& outStatus);
    static bool_t Try_EffectTimeScale(const WORLD_SEQUENCE_EFFECT_TRACK& effect,
        f32_t sourceDurationSeconds, f32_t& outScale);
    bool_t Duplicate_TimelineBox(const std::string& sequenceId, bool animation, size_t index,
        const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
        const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus);
    bool_t Duplicate_ColliderTrack(const std::string& sequenceId, size_t index,
        const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
        const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus);
    WORLD_SEQUENCE_OBJECT_FOLDER* Find_ObjectFolder(const std::string& folderId);
    const WORLD_SEQUENCE_OBJECT_FOLDER* Find_ObjectFolder(const std::string& folderId) const;
    std::vector<WORLD_SEQUENCE_OBJECT_FOLDER>& Get_ObjectFolders() noexcept { return m_ObjectFolders; }
    const std::vector<WORLD_SEQUENCE_OBJECT_FOLDER>& Get_ObjectFolders() const noexcept { return m_ObjectFolders; }
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

    static bool_t Try_SampleMaterialParameters(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        const WORLD_SEQUENCE_MATERIAL_TRACK& track, f32_t timeMs,
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& out);

    static bool_t Is_ValidMaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile);
    static bool_t Build_MaterialOverride(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        const std::filesystem::path& resourceRoot, Engine::MODEL_MATERIAL_OVERRIDE& out);

	// Shared by visible Object pose, Effect socket history and Deploy pose.
	// Native-range or non-finite failure leaves outTicks unchanged.
	static bool_t Try_SampleAnimationTicks(const WORLD_SEQUENCE_ANIMATION_TRACK& track,
		f32_t localMs, f32_t windowEndMs, f32_t ticksPerSecond, f32_t durationTicks, f32_t& outTicks);

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
	std::vector<WORLD_SEQUENCE_OBJECT_FOLDER> m_ObjectFolders;
};

NS_END
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Private/WorldSequenceDocument.cpp

```cpp
#include "WorldSequenceDocument.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "SourceCharacterMaterialParameters.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.world-sequences";
	constexpr uint32_t FORMAT_VERSION = 3;
	constexpr uint32_t LEGACY_FORMAT_VERSION = 1;
	constexpr f32_t MIN_SCALE = 0.000001f;
	constexpr f32_t MIN_RUNTIME_SCALE_DETERMINANT = 0.000001f;
	constexpr f32_t MAX_COMPONENT = 100000.f;
	constexpr uintmax_t MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u;

	bool_t Is_ValidUtf8DisplayText(const std::string& value, const bool_t allowLineFeed = false)
	{
		for (size_t offset = 0u; offset < value.size();)
		{
			const uint8_t first = static_cast<uint8_t>(value[offset]);
			if (first < 0x80u)
			{
				if ((first < 0x20u && !(allowLineFeed && first == 0x0au)) || 0x7fu == first)
					return false;
				++offset;
				continue;
			}
			size_t length = 0u;
			uint32_t codePoint = 0u;
			uint32_t minimum = 0u;
			if (first >= 0xc2u && first <= 0xdfu)
			{
				length = 2u;
				codePoint = first & 0x1fu;
				minimum = 0x80u;
			}
			else if (first >= 0xe0u && first <= 0xefu)
			{
				length = 3u;
				codePoint = first & 0x0fu;
				minimum = 0x800u;
			}
			else if (first >= 0xf0u && first <= 0xf4u)
			{
				length = 4u;
				codePoint = first & 0x07u;
				minimum = 0x10000u;
			}
			else
			{
				return false;
			}
			if (offset + length > value.size())
				return false;
			for (size_t index = 1u; index < length; ++index)
			{
				const uint8_t next = static_cast<uint8_t>(value[offset + index]);
				if ((next & 0xc0u) != 0x80u)
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if (codePoint < minimum || codePoint > 0x10ffffu ||
				(codePoint >= 0xd800u && codePoint <= 0xdfffu))
			{
				return false;
			}
			offset += length;
		}
		return true;
	}

	bool_t Is_IntegerNumber(const DATA_JSON_VALUE* value)
	{
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number()) &&
			std::floor(value->Get_Number()) == value->Get_Number();
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Is_ObjectShape(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> required,
		const std::initializer_list<const char_t*> optional)
	{
		if (!value.Is_Object()) return false;
		for (const char_t* key : required)
			if (nullptr == value.Find(key)) return false;
		for (const auto& entry : value.Get_Object())
		{
			const auto matches = [&entry](const char_t* key) { return entry.first == key; };
			if (std::none_of(required.begin(), required.end(), matches) &&
				std::none_of(optional.begin(), optional.end(), matches)) return false;
		}
		return true;
	}

	bool_t Is_BoundedFloat3(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
			std::abs(value.x) <= MAX_COMPONENT && std::abs(value.y) <= MAX_COMPONENT &&
			std::abs(value.z) <= MAX_COMPONENT;
	}

	bool_t Is_ResourcePath(const std::string& value, const bool_t model)
	{
		if (value.empty() || value.size() > 1024u || value.front() == '/' ||
			value.find(':') != std::string::npos || value.find('\\') != std::string::npos ||
			!Is_ValidUtf8DisplayText(value)) return false;
		std::istringstream parts(value);
		std::string part;
		while (std::getline(parts, part, '/'))
			if (part.empty() || part == "." || part == "..") return false;
		std::string extension = value.size() > 7u ? value.substr(value.size() - 7u) : std::string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](const unsigned char character) { return static_cast<char_t>(std::tolower(character)); });
		return value.back() != '/' && (!model || extension == ".wmodel");
	}

    bool_t Validate_MaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS* out = nullptr)
    {
        Engine::MODEL_SOURCE_CHARACTER_PARAMETERS packed;
        if (profile.materialName.empty() || profile.materialName.size() > 63u ||
            !Is_ValidUtf8DisplayText(profile.materialName) || profile.sourceMaterial.empty() ||
            profile.sourceMaterial.size() > 512u || !Is_ValidUtf8DisplayText(profile.sourceMaterial) ||
            !SourceCharacterMaterial::Configure(profile.family, profile.parameters, packed)) return false;
        for (const auto& [name, values] : profile.parameters)
            for (const auto value : values)
                if (!std::isfinite(value) || std::abs(value) > 1000000.f) return false;
        const uint32_t required = packed.baseTextureMask | packed.lightTextureMask;
        uint32_t supplied = 0u;
        for (const auto& texture : profile.textures)
        {
            if (texture.expressionIndex >= Engine::SOURCE_CHARACTER_TEXTURE_COUNT ||
                !Is_ResourcePath(texture.assetId, false)) return false;
            const auto bit = 1u << texture.expressionIndex;
            if ((supplied & bit) != 0u || (required & bit) == 0u) return false;
            supplied |= bit;
        }
        if (supplied != required) return false;
        if (out) *out = packed;
        return true;
    }

	/* Authored rows must agree with the seeded fields they replace, so a
	   document never carries two different answers for the emission count. */
	bool_t Is_ValidEmissionList(const WORLD_SEQUENCE_OBJECT_MOTION& motion)
	{
		if (motion.emissions.empty()) return true;
		if (motion.emissions.size() > 128u || motion.count != motion.emissions.size() ||
			0u != motion.intervalMs || 0.f != motion.spreadDegrees) return false;
		for (const auto& emission : motion.emissions)
		{
			if (!Is_BoundedFloat3(emission.positionOffset) || !std::isfinite(emission.yawDegrees) ||
				emission.yawDegrees < -36000.f || emission.yawDegrees > 36000.f ||
				emission.startDelayMs > CWorldSequenceDocument::MAX_DURATION_MS) return false;
		}
		return true;
	}

	bool_t Read_Uint32(
		const DATA_JSON_VALUE* value,
		uint32_t& outValue,
		const uint32_t maximum = UINT32_MAX)
	{
		if (!Is_IntegerNumber(value) || value->Get_Number() < 0.0 ||
			value->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(value->Get_Number());
		return true;
	}

	bool_t Read_FiniteFloat(const DATA_JSON_VALUE* value, f32_t& outValue)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Read_Float3(const DATA_JSON_VALUE* value, float3_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		return Read_FiniteFloat(&values[0], outValue.x) &&
			Read_FiniteFloat(&values[1], outValue.y) &&
			Read_FiniteFloat(&values[2], outValue.z);
	}

	bool_t Read_Quaternion(const DATA_JSON_VALUE* value, float4_t& outValue)
	{
		if (nullptr == value || !value->Is_Array() ||
			4u != value->Get_Array().size())
		{
			return false;
		}
		const auto& values = value->Get_Array();
		if (!Read_FiniteFloat(&values[0], outValue.x) ||
			!Read_FiniteFloat(&values[1], outValue.y) ||
			!Read_FiniteFloat(&values[2], outValue.z) ||
			!Read_FiniteFloat(&values[3], outValue.w))
		{
			return false;
		}
		const vector_t raw = XMLoadFloat4(&outValue);
		const f32_t length = XMVectorGetX(XMVector4Length(raw));
		if (!std::isfinite(length) ||
			std::abs(length - 1.f) > 0.001f)
			return false;
		if (outValue.w < 0.f)
		{
			outValue.x = -outValue.x;
			outValue.y = -outValue.y;
			outValue.z = -outValue.z;
			outValue.w = -outValue.w;
		}
		return true;
	}

	bool_t Parse_Uint64String(const DATA_JSON_VALUE* value, uint64_t& outValue)
	{
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		const std::string& text = value->Get_String();
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Parse_Uint64Text(const std::string& text, uint64_t& outValue)
	{
		if (text.empty())
			return false;
		const char_t* const begin = text.data();
		const char_t* const end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end && 0u != outValue;
	}

	bool_t Is_FiniteTransform(const WORLD_SEQUENCE_TRANSFORM_KEY& key)
	{
		const auto finiteBounded = [](const f32_t value)
		{
			return std::isfinite(value) && std::abs(value) <= MAX_COMPONENT;
		};
		if (!finiteBounded(key.positionOffset.x) ||
			!finiteBounded(key.positionOffset.y) ||
			!finiteBounded(key.positionOffset.z) ||
			!finiteBounded(key.scaleMultiplier.x) ||
			!finiteBounded(key.scaleMultiplier.y) ||
			!finiteBounded(key.scaleMultiplier.z) ||
			std::abs(key.scaleMultiplier.x) < MIN_SCALE ||
			std::abs(key.scaleMultiplier.y) < MIN_SCALE ||
			std::abs(key.scaleMultiplier.z) < MIN_SCALE)
		{
			return false;
		}
		const vector_t quaternion = XMLoadFloat4(&key.rotationQuaternion);
		const f32_t length = XMVectorGetX(XMVector4Length(quaternion));
		return std::isfinite(length) && std::abs(length - 1.f) <= 0.001f &&
			key.rotationQuaternion.w >= 0.f;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) && !existsError &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CWorldSequenceDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus)
{
	CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Load");
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		if (existsError)
		{
			outStatus = "Could not inspect world sequence document";
			return false;
		}
		Reset_Empty(expectedAreaId);
		outStatus = "No world sequence document; starting empty";
		return true;
	}
	if (!std::filesystem::is_regular_file(path, existsError) || existsError)
	{
		outStatus = "World sequence document is not a regular file";
		return false;
	}
	const uintmax_t fileBytes = std::filesystem::file_size(path, existsError);
	if (existsError || fileBytes > MAX_DOCUMENT_BYTES)
	{
		outStatus = existsError ?
			"Could not inspect world sequence document size" :
			"World sequence document exceeds the 16 MiB parse limit";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open world sequence document file";
		return false;
	}
	std::string text;
	try
	{
		text.resize(static_cast<size_t>(fileBytes));
	}
	catch (const std::bad_alloc&)
	{
		outStatus = "Could not allocate bounded world sequence input";
		return false;
	}
	if (!text.empty())
		input.read(text.data(), static_cast<std::streamsize>(text.size()));
	if (input.bad() || input.gcount() != static_cast<std::streamsize>(text.size()) ||
		std::char_traits<char_t>::eof() != input.peek())
	{
		outStatus = "World sequence document changed or failed while reading";
		return false;
	}
	return Load_Text(text, expectedAreaId, availablePlacements, availableDeployPlacements, outStatus);
}

bool_t Client::CWorldSequenceDocument::Load_Text(const std::string_view text,
	const std::string& expectedAreaId, const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements, std::string& outStatus)
{
	if (text.size() > MAX_DOCUMENT_BYTES)
	{ outStatus = "World sequence document exceeds the 16 MiB parse limit"; return false; }
	DATA_JSON_VALUE root;
	std::string parseError;
	bool_t parsed = false;
	{
		CProfilerScope parseScope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Parse");
		parsed = CDataJson::Parse(text, root, parseError);
	}
	if (!parsed ||
		!Is_ObjectShape(root,
			{ "schema", "formatVersion", "areaId", "revision",
			  "templates", "instances" }, { "objectResources", "objectFolders" }))
	{
		outStatus = "World sequence JSON root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areaId = root.Find("areaId");
	const DATA_JSON_VALUE* revision = root.Find("revision");
	const DATA_JSON_VALUE* templates = root.Find("templates");
	const DATA_JSON_VALUE* instances = root.Find("instances");
	uint32_t parsedFormatVersion = 0;
	uint32_t parsedRevision = 0;
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != SCHEMA ||
		!Read_Uint32(version, parsedFormatVersion) ||
		(parsedFormatVersion < LEGACY_FORMAT_VERSION || parsedFormatVersion > FORMAT_VERSION) ||
		nullptr == areaId || !areaId->Is_String() ||
		areaId->Get_String() != expectedAreaId ||
		!Read_Uint32(revision, parsedRevision) || 0u == parsedRevision ||
		nullptr == templates || !templates->Is_Array() ||
		templates->Get_Array().size() > MAX_TEMPLATE_COUNT ||
		nullptr == instances || !instances->Is_Array() ||
		instances->Get_Array().size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence header is invalid or belongs to another Area";
		return false;
	}

	CWorldSequenceDocument staged;
	staged.m_AreaId = expectedAreaId;
	staged.m_iRevision = parsedRevision;
    if (const auto* folders = root.Find("objectFolders"))
    {
        if (parsedFormatVersion != 3u || !folders->Is_Array() || folders->Get_Array().size() > MAX_INSTANCE_COUNT)
        { outStatus = "World object folder list is invalid"; return false; }
        for (const auto& row : folders->Get_Array())
        {
            if (!Is_ObjectShape(row, {"folderId", "displayName"}, {"anchorKind", "parentId"}) ||
                !row.Find("folderId")->Is_String() || !row.Find("displayName")->Is_String())
            { outStatus = "World object folder fields are invalid"; return false; }
            WORLD_SEQUENCE_OBJECT_FOLDER folder;
            folder.folderId = row.Find("folderId")->Get_String();
            folder.displayName = row.Find("displayName")->Get_String();
            for (const char_t* key : {"anchorKind", "parentId"})
                if (const auto* value = row.Find(key))
                {
                    if (!value->Is_String()) { outStatus = "World object folder anchor and parent must be text"; return false; }
                    (std::string(key) == "anchorKind" ? folder.anchorKind : folder.parentId) = value->Get_String();
                }
            staged.m_ObjectFolders.push_back(std::move(folder));
        }
    }
	const DATA_JSON_VALUE* objects = root.Find("objectResources");
	if ((parsedFormatVersion < 3u && nullptr != objects) ||
		(parsedFormatVersion == 3u && (nullptr == objects || !objects->Is_Array() ||
			objects->Get_Array().size() > MAX_INSTANCE_COUNT)))
	{
		outStatus = "World object resource list is invalid";
		return false;
	}
	if (nullptr != objects)
	{
		for (const DATA_JSON_VALUE& row : objects->Get_Array())
		{
			WORLD_SEQUENCE_OBJECT_RESOURCE object;
			if (!Is_ObjectShape(row, { "objectId", "displayName", "modelAssetId", "modelPreScale",
				"animated", "scale" }, { "diffuseTextureAssetId", "sequenceInstanceId", "anchorKind", "defaultMotionInstanceId", "anchorBossArchetypeId", "anchorBone", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings", "motionInstanceIds", "combatBody", "animationSetAssetId", "presentationBossArchetypeId", "parentId" }) ||
				!row.Find("objectId")->Is_String() || !row.Find("displayName")->Is_String() ||
				!row.Find("modelAssetId")->Is_String() || !row.Find("animated")->Is_Boolean() ||
				!Read_FiniteFloat(row.Find("modelPreScale"), object.modelPreScale) ||
				!Read_Float3(row.Find("scale"), object.scale))
			{
				outStatus = "World object resource fields are invalid";
				return false;
			}
			object.objectId = row.Find("objectId")->Get_String();
			object.displayName = row.Find("displayName")->Get_String();
            if (const auto* parent = row.Find("parentId"))
            {
                if (!parent->Is_String()) { outStatus = "World object parent ID must be text"; return false; }
                object.parentId = parent->Get_String();
            }
			object.modelAssetId = row.Find("modelAssetId")->Get_String();
			object.animated = row.Find("animated")->Get_Boolean();
			if (const auto* value = row.Find("combatBody"))
			{
				WORLD_SEQUENCE_COMBAT_BODY body;
				if (!Is_ObjectShape(*value, {"maxHp", "localCenterM", "halfExtentsM", "lifetimePolicy"}, {"shape"}) ||
					!Read_Uint32(value->Find("maxHp"), body.maxHp, 1000000000u) ||
					!Read_Float3(value->Find("localCenterM"), body.localCenterM) ||
					!Read_Float3(value->Find("halfExtentsM"), body.halfExtentsM) ||
					!value->Find("lifetimePolicy")->Is_String())
				{ outStatus = "Invalid World Object combat body"; return false; }
				if (const auto* shape = value->Find("shape"))
				{
					if (!shape->Is_String()) { outStatus = "World Object combat shape must be text"; return false; }
					body.shape = shape->Get_String();
				}
				body.lifetimePolicy = value->Find("lifetimePolicy")->Get_String();
				object.combatBody = std::move(body);
			}
			if (const auto* members = row.Find("motionInstanceIds"))
			{
				if (!members->Is_Array() || members->Get_Array().empty() || members->Get_Array().size() > 32u)
				{ outStatus = "Object group requires 1..32 motion instance IDs"; return false; }
				for (const auto& member : members->Get_Array())
				{
					if (!member.Is_String()) { outStatus = "Object group member ID must be text"; return false; }
					object.motionInstanceIds.push_back(member.Get_String());
				}
			}
			if (const auto* motion = row.Find("defaultMotionInstanceId"))
			{
				if (!motion->Is_String()) { outStatus = "Default Motion instance ID must be text"; return false; }
				object.defaultMotionInstanceId = motion->Get_String();
			}
			if (const auto* anchor = row.Find("anchorKind"))
			{
				if (!anchor->Is_String()) { outStatus = "World object resource anchor must be WORLD, PLAYER or BOSS"; return false; }
				object.anchorKind = anchor->Get_String();
			}
			for (const char_t* key : { "anchorBossArchetypeId", "anchorBone" })
			{
				const auto* field = row.Find(key);
				if (!field) continue;
				if (!field->Is_String()) { outStatus = "World object boss anchor fields must be text"; return false; }
				(std::string(key) == "anchorBone" ? object.anchorBone : object.anchorBossArchetypeId) = field->Get_String();
			}
			for (const char_t* key : { "diffuseTextureAssetId", "sequenceInstanceId" })
			{
				const auto* field = row.Find(key);
				if (nullptr == field) continue;
				if (!field->Is_String()) { outStatus = "Invalid world object optional path/reference"; return false; }
				(std::string(key) == "sequenceInstanceId" ? object.sequenceInstanceId :
					object.diffuseTextureAssetId) = field->Get_String();
			}
            if (const auto* source = row.Find("materialSourceModelAssetId"))
            {
                if (!source->Is_String() || !Is_ResourcePath(source->Get_String(), true))
                { outStatus = "Invalid world object material source model: " + object.objectId; return false; }
                object.materialSourceModelAssetId = source->Get_String();
            }
            if (const auto* animationSet = row.Find("animationSetAssetId"))
            {
                if (!animationSet->Is_String() || !Is_ResourcePath(animationSet->Get_String(), true))
                { outStatus = "Invalid world object animation set: " + object.objectId; return false; }
                object.animationSetAssetId = animationSet->Get_String();
            }
            if (const auto* presentation = row.Find("presentationBossArchetypeId"))
            {
                if (!presentation->Is_String())
                { outStatus = "Invalid world object presentation boss: " + object.objectId; return false; }
                object.presentationBossArchetypeId = presentation->Get_String();
            }
            if (const auto* bindings = row.Find("mapMaterialBindings"))
            {
                if (!bindings->Is_Array() || bindings->Get_Array().size() > 64u)
                { outStatus = "Invalid world object map material bindings"; return false; }
                for (const auto& binding : bindings->Get_Array())
                {
                    if (!Is_ObjectShape(binding, { "materialName", "sourceAssetId", "sourceMaterialName" }, { "diffuseTextureAssetId", "unlit" }) ||
                        !binding.Find("materialName")->Is_String() || !binding.Find("sourceAssetId")->Is_String() ||
                        !binding.Find("sourceMaterialName")->Is_String())
                    { outStatus = "Invalid world object map material binding"; return false; }
                    WORLD_SEQUENCE_MAP_MATERIAL_BINDING material;
                    material.materialName = binding.Find("materialName")->Get_String();
                    material.sourceAssetId = binding.Find("sourceAssetId")->Get_String();
                    material.sourceMaterialName = binding.Find("sourceMaterialName")->Get_String();
                    if (const auto* diffuse = binding.Find("diffuseTextureAssetId"))
                    {
                        if (!diffuse->Is_String() || !Is_ResourcePath(diffuse->Get_String(), false))
                        { outStatus = "Invalid map material diffuse texture"; return false; }
                        material.diffuseTextureAssetId = diffuse->Get_String();
                    }
                    if (const auto* unlit = binding.Find("unlit"))
                    {
                        if (!unlit->Is_Boolean())
                        { outStatus = "Invalid map material unlit flag"; return false; }
                        material.unlit = unlit->Get_Boolean();
                    }
                    object.mapMaterialBindings.push_back(std::move(material));
                }
            }
            if (const auto* value = row.Find("materialProfile"))
            {
                WORLD_SEQUENCE_MATERIAL_PROFILE profile;
                if (!Is_ExactObject(*value, { "materialName", "sourceMaterial", "family", "parameters", "textures" }) ||
                    !value->Find("materialName")->Is_String() || !value->Find("sourceMaterial")->Is_String() ||
                    !value->Find("family")->Is_String() || !value->Find("textures")->Is_Array() ||
                    !SourceCharacterMaterial::Read(*value->Find("parameters"), profile.parameters))
                { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
                profile.materialName = value->Find("materialName")->Get_String();
                profile.sourceMaterial = value->Find("sourceMaterial")->Get_String();
                profile.family = value->Find("family")->Get_String();
                for (const auto& texture : value->Find("textures")->Get_Array())
                {
                    WORLD_SEQUENCE_MATERIAL_TEXTURE input;
                    if (!Is_ExactObject(texture, { "expressionIndex", "assetId", "colorSpace" }) ||
                        !Read_Uint32(texture.Find("expressionIndex"), input.expressionIndex) ||
                        !texture.Find("assetId")->Is_String() || !texture.Find("colorSpace")->Is_String() ||
                        (texture.Find("colorSpace")->Get_String() != "srgb" && texture.Find("colorSpace")->Get_String() != "linear"))
                    { outStatus = "Invalid world object material texture: " + object.objectId; return false; }
                    input.assetId = texture.Find("assetId")->Get_String();
                    input.srgb = texture.Find("colorSpace")->Get_String() == "srgb";
                    profile.textures.push_back(std::move(input));
                }
                if (!Validate_MaterialProfile(profile))
                { outStatus = "World object material input contract failed: " + object.objectId; return false; }
                object.materialProfile = std::move(profile);
            }
			staged.m_ObjectResources.push_back(std::move(object));
		}
	}
	for (const DATA_JSON_VALUE& templateValue : templates->Get_Array())
	{
		const bool_t validTemplateShape =
			LEGACY_FORMAT_VERSION == parsedFormatVersion ?
			Is_ExactObject(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks" }) :
			Is_ObjectShape(templateValue,
				{ "sequenceId", "displayName", "category", "durationMs",
				  "interpolation", "tracks", "animationTracks" }, { "objectMotion", "effectTracks", "colliderTracks", "soundTracks", "subtitleTracks", "materialTracks" });
		if (!validTemplateShape)
		{
			outStatus = "World sequence template shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* sequenceId = templateValue.Find("sequenceId");
		const DATA_JSON_VALUE* displayName = templateValue.Find("displayName");
		const DATA_JSON_VALUE* category = templateValue.Find("category");
		const DATA_JSON_VALUE* interpolation = templateValue.Find("interpolation");
		const DATA_JSON_VALUE* tracks = templateValue.Find("tracks");
		const DATA_JSON_VALUE* animationTracks =
			templateValue.Find("animationTracks");
		WORLD_SEQUENCE_TEMPLATE parsedTemplate;
		if (nullptr == sequenceId || !sequenceId->Is_String() ||
			nullptr == displayName || !displayName->Is_String() ||
			nullptr == category || !category->Is_String() ||
			!Read_Uint32(templateValue.Find("durationMs"),
				parsedTemplate.durationMs, MAX_DURATION_MS) ||
			nullptr == interpolation || !interpolation->Is_String() ||
			!Try_ParseInterpolation(interpolation->Get_String(),
				parsedTemplate.interpolation) ||
			nullptr == tracks || !tracks->Is_Array() ||
			tracks->Get_Array().size() > MAX_TRACK_COUNT ||
			(2u <= parsedFormatVersion &&
				(nullptr == animationTracks || !animationTracks->Is_Array() ||
					animationTracks->Get_Array().size() > MAX_TRACK_COUNT ||
					tracks->Get_Array().size() +
						animationTracks->Get_Array().size() > MAX_TRACK_COUNT)))
		{
			outStatus = "World sequence template fields are invalid";
			return false;
		}
		parsedTemplate.sequenceId = sequenceId->Get_String();
		parsedTemplate.displayName = displayName->Get_String();
		parsedTemplate.category = category->Get_String();
		if (const DATA_JSON_VALUE* motion = templateValue.Find("objectMotion"))
		{
			auto& value = parsedTemplate.objectMotion;
			if (parsedFormatVersion < 3u || !Is_ObjectShape(*motion,
				{ "velocity", "acceleration", "angularVelocityDegrees", "revolutionDegreesPerSecond",
				  "revolutionOffset", "count", "intervalMs", "spreadDegrees", "seed" }, { "spawnHalfExtents", "emissions" }) ||
				!Read_Float3(motion->Find("velocity"), value.velocity) ||
				!Read_Float3(motion->Find("acceleration"), value.acceleration) ||
				!Read_Float3(motion->Find("angularVelocityDegrees"), value.angularVelocityDegrees) ||
				!Read_Float3(motion->Find("revolutionDegreesPerSecond"), value.revolutionDegreesPerSecond) ||
				!Read_Float3(motion->Find("revolutionOffset"), value.revolutionOffset) ||
				(motion->Find("spawnHalfExtents") && !Read_Float3(motion->Find("spawnHalfExtents"), value.spawnHalfExtents)) ||
				!Read_Uint32(motion->Find("count"), value.count, 128u) ||
				!Read_Uint32(motion->Find("intervalMs"), value.intervalMs, MAX_DURATION_MS) ||
				!Read_FiniteFloat(motion->Find("spreadDegrees"), value.spreadDegrees) ||
				!Read_Uint32(motion->Find("seed"), value.seed))
			{
				outStatus = "World object motion is invalid";
				return false;
			}
			if (const DATA_JSON_VALUE* emissions = motion->Find("emissions"))
			{
				if (!emissions->Is_Array() || emissions->Get_Array().size() > 128u)
				{
					outStatus = "World object emissions are invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& emissionValue : emissions->Get_Array())
				{
					WORLD_SEQUENCE_OBJECT_EMISSION emission;
					if (!Is_ExactObject(emissionValue, { "positionOffset", "yawDegrees", "startDelayMs" }) ||
						!Read_Float3(emissionValue.Find("positionOffset"), emission.positionOffset) ||
						!Read_FiniteFloat(emissionValue.Find("yawDegrees"), emission.yawDegrees) ||
						!Read_Uint32(emissionValue.Find("startDelayMs"), emission.startDelayMs, MAX_DURATION_MS))
					{
						outStatus = "World object emission row is invalid";
						return false;
					}
					value.emissions.push_back(emission);
				}
			}
		}

		for (const DATA_JSON_VALUE& trackValue : tracks->Get_Array())
		{
			if (!Is_ExactObject(trackValue, { "slotId", "keys" }))
			{
				outStatus = "World sequence track shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
			const DATA_JSON_VALUE* keys = trackValue.Find("keys");
			WORLD_SEQUENCE_TRACK parsedTrack;
			if (nullptr == slotId || !slotId->Is_String() ||
				nullptr == keys || !keys->Is_Array() ||
				keys->Get_Array().size() > MAX_KEY_COUNT)
			{
				outStatus = "World sequence track fields are invalid";
				return false;
			}
			parsedTrack.slotId = slotId->Get_String();
			for (const DATA_JSON_VALUE& keyValue : keys->Get_Array())
			{
				if (!Is_ExactObject(keyValue,
					{ "timeMs", "positionOffset", "rotationQuaternion",
					  "scaleMultiplier", "visible" }))
				{
					outStatus = "World sequence key shape is invalid";
					return false;
				}
				WORLD_SEQUENCE_TRANSFORM_KEY parsedKey;
				const DATA_JSON_VALUE* visible = keyValue.Find("visible");
				if (!Read_Uint32(keyValue.Find("timeMs"), parsedKey.timeMs,
						MAX_DURATION_MS) ||
					!Read_Float3(keyValue.Find("positionOffset"),
						parsedKey.positionOffset) ||
					!Read_Quaternion(keyValue.Find("rotationQuaternion"),
						parsedKey.rotationQuaternion) ||
					!Read_Float3(keyValue.Find("scaleMultiplier"),
						parsedKey.scaleMultiplier) ||
					nullptr == visible || !visible->Is_Boolean())
				{
					outStatus = "World sequence key fields are invalid";
					return false;
				}
				parsedKey.visible = visible->Get_Boolean();
				parsedTrack.keys.push_back(parsedKey);
			}
			parsedTemplate.tracks.push_back(std::move(parsedTrack));
		}
		if (2u <= parsedFormatVersion)
		{
			for (const DATA_JSON_VALUE& trackValue :
				animationTracks->Get_Array())
			{
				if (!Is_ObjectShape(trackValue,
						{ "slotId", "clipName", "playbackRate", "loop",
						  "holdLastFrame" }, { "startMs", "displayName", "sourceStartMs", "sourceEndMs" }))
				{
					outStatus = "World sequence animation track shape is invalid";
					return false;
				}
				const DATA_JSON_VALUE* slotId = trackValue.Find("slotId");
				const DATA_JSON_VALUE* clipName = trackValue.Find("clipName");
				const DATA_JSON_VALUE* trackDisplayName = trackValue.Find("displayName");
				const DATA_JSON_VALUE* loop = trackValue.Find("loop");
				const DATA_JSON_VALUE* holdLastFrame =
					trackValue.Find("holdLastFrame");
				WORLD_SEQUENCE_ANIMATION_TRACK parsedTrack;
				if (nullptr == slotId || !slotId->Is_String() ||
					nullptr == clipName || !clipName->Is_String() ||
					(nullptr != trackDisplayName && !trackDisplayName->Is_String()) ||
					!Read_FiniteFloat(trackValue.Find("playbackRate"),
						parsedTrack.playbackRate) ||
					nullptr == loop || !loop->Is_Boolean() ||
					nullptr == holdLastFrame || !holdLastFrame->Is_Boolean())
				{
					outStatus = "World sequence animation track fields are invalid";
					return false;
				}
				const DATA_JSON_VALUE* startMs = trackValue.Find("startMs");
				if (nullptr != startMs &&
					!Read_Uint32(startMs, parsedTrack.startMs, MAX_DURATION_MS))
				{
					outStatus = "World sequence animation track start is invalid";
					return false;
				}
				if (const auto* sourceStart = trackValue.Find("sourceStartMs"))
				{
					if (!Read_Uint32(sourceStart, parsedTrack.sourceStartMs, MAX_DURATION_MS))
					{ outStatus = "World sequence animation source start is invalid"; return false; }
				}
				if (const auto* sourceEnd = trackValue.Find("sourceEndMs"))
				{
					if (!Read_Uint32(sourceEnd, parsedTrack.sourceEndMs, MAX_DURATION_MS))
					{ outStatus = "World sequence animation source end is invalid"; return false; }
				}
				parsedTrack.slotId = slotId->Get_String();
				parsedTrack.clipName = clipName->Get_String();
				if (nullptr != trackDisplayName)
					parsedTrack.displayName = trackDisplayName->Get_String();
				parsedTrack.loop = loop->Get_Boolean();
				parsedTrack.holdLastFrame = holdLastFrame->Get_Boolean();
				parsedTemplate.animationTracks.push_back(std::move(parsedTrack));
			}
		}
        if (const auto* materials = templateValue.Find("materialTracks"))
        {
            if (parsedFormatVersion < 3u || !materials->Is_Array() || materials->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World materialTracks must be a bounded v3 array"; return false; }
            for (const auto& row : materials->Get_Array())
            {
                WORLD_SEQUENCE_MATERIAL_TRACK track;
                if (!Is_ObjectShape(row, {"slotId", "materialName", "curves"}, {}) ||
                    !row.Find("slotId")->Is_String() || !row.Find("materialName")->Is_String() ||
                    !row.Find("curves")->Is_Array() || row.Find("curves")->Get_Array().size() > 64u)
                { outStatus = "World material track fields are invalid"; return false; }
                track.slotId = row.Find("slotId")->Get_String();
                track.materialName = row.Find("materialName")->Get_String();
                for (const auto& curve : row.Find("curves")->Get_Array())
                {
                    WORLD_SEQUENCE_MATERIAL_CURVE parsed;
                    if (!Is_ObjectShape(curve, {"parameter", "keys"}, {}) ||
                        !curve.Find("parameter")->Is_String() || !curve.Find("keys")->Is_Array() ||
                        curve.Find("keys")->Get_Array().size() > 4096u)
                    { outStatus = "World material curve fields are invalid"; return false; }
                    parsed.parameter = curve.Find("parameter")->Get_String();
                    for (const auto& key : curve.Find("keys")->Get_Array())
                    {
                        WORLD_SEQUENCE_MATERIAL_KEY sample;
                        if (!Is_ObjectShape(key, {"timeMs", "value", "interpolation"}, {}) ||
                            !Read_Uint32(key.Find("timeMs"), sample.timeMs, MAX_DURATION_MS) ||
                            !key.Find("interpolation")->Is_String() || !key.Find("value")->Is_Array() ||
                            key.Find("value")->Get_Array().size() != 4u)
                        { outStatus = "World material key fields are invalid"; return false; }
                        const auto& mode = key.Find("interpolation")->Get_String();
                        if (mode != "LINEAR" && mode != "CONSTANT")
                        { outStatus = "World material interpolation is invalid"; return false; }
                        sample.constant = mode == "CONSTANT";
                        for (size_t axis = 0u; axis < 4u; ++axis)
                            if (!Read_FiniteFloat(&key.Find("value")->Get_Array()[axis], sample.value[axis]))
                            { outStatus = "World material value is not finite"; return false; }
                        parsed.keys.push_back(sample);
                    }
                    track.curves.push_back(std::move(parsed));
                }
                parsedTemplate.materialTracks.push_back(std::move(track));
            }
        }
		if (const auto* effects = templateValue.Find("effectTracks"))
		{
			if (parsedFormatVersion < 3u || !effects->Is_Array() || effects->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object effectTracks must be a bounded v3 array"; return false; }
			for (const auto& row : effects->Get_Array())
			{
				WORLD_SEQUENCE_EFFECT_TRACK effect;
				if (!Is_ObjectShape(row, { "effectTrackId", "slotId", "resourceKind", "resourceId",
					"timing", "startMs", "durationMs", "positionOffset", "rotationDegrees", "scale" }, { "followObject", "inheritObjectRotation", "bone", "fitEffectToDuration", "loopEffectToDuration" }))
				{ outStatus = "World Object effect track shape is invalid"; return false; }
				for (const char* key : { "effectTrackId", "slotId", "resourceKind", "resourceId", "timing" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object effect identity must be text"; return false; }
				effect.effectTrackId = row.Find("effectTrackId")->Get_String();
				effect.slotId = row.Find("slotId")->Get_String();
				effect.resourceKind = row.Find("resourceKind")->Get_String();
				effect.resourceId = row.Find("resourceId")->Get_String();
				effect.timing = row.Find("timing")->Get_String();
				if (const auto* fit = row.Find("fitEffectToDuration"))
				{
					if (!fit->Is_Boolean()) { outStatus = "World Object effect fit must be boolean"; return false; }
					effect.fitEffectToDuration = fit->Get_Boolean();
				}
				if (const auto* loop = row.Find("loopEffectToDuration"))
				{
					if (!loop->Is_Boolean()) { outStatus = "World Object effect loop must be boolean"; return false; }
					effect.loopEffectToDuration = loop->Get_Boolean();
				}
				if (const auto* follow = row.Find("followObject"))
				{
					if (!follow->Is_Boolean()) { outStatus = "World Object effect followObject must be boolean"; return false; }
					effect.followObject = follow->Get_Boolean();
				}
				if (const auto* inherit = row.Find("inheritObjectRotation"))
				{
					if (!inherit->Is_Boolean()) { outStatus = "World Object effect inheritObjectRotation must be boolean"; return false; }
					effect.inheritObjectRotation = inherit->Get_Boolean();
				}
				if (const auto* bone = row.Find("bone"))
				{
					if (!bone->Is_String()) { outStatus = "World Object effect bone must be text"; return false; }
					effect.bone = bone->Get_String();
				}
				if (!Read_Uint32(row.Find("startMs"), effect.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), effect.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), effect.positionOffset) ||
					!Read_Float3(row.Find("rotationDegrees"), effect.rotationDegrees) ||
					!Read_Float3(row.Find("scale"), effect.scale))
				{ outStatus = "World Object effect timing or transform is invalid"; return false; }
				parsedTemplate.effectTracks.push_back(std::move(effect));
			}
		}

		if (const auto* colliders = templateValue.Find("colliderTracks"))
		{
			if (parsedFormatVersion < 3u || !colliders->Is_Array() || colliders->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object colliderTracks must be a bounded v3 array"; return false; }
			for (const auto& row : colliders->Get_Array())
			{
				WORLD_SEQUENCE_COLLIDER_TRACK collider;
				if (!Is_ObjectShape(row, { "colliderTrackId", "slotId", "startMs", "durationMs", "positionOffset",
					"halfExtents", "yawDegrees", "behavior", "damagePercent", "gripLocalOffset" }, { "attachmentBone", "shape" }))
				{ outStatus = "World Object collider track shape is invalid"; return false; }
				for (const char* key : { "colliderTrackId", "slotId", "behavior" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object collider identity must be text"; return false; }
				collider.colliderTrackId = row.Find("colliderTrackId")->Get_String();
				collider.slotId = row.Find("slotId")->Get_String();
				collider.behavior = row.Find("behavior")->Get_String();
				if (const auto* shape = row.Find("shape"))
				{
					if (!shape->Is_String()) { outStatus = "World Object collider shape must be text"; return false; }
					collider.shape = shape->Get_String();
				}
				if (const auto* bone = row.Find("attachmentBone"))
				{
					if (!bone->Is_String()) { outStatus = "World Object collider attachmentBone must be text"; return false; }
					collider.attachmentBone = bone->Get_String();
				}
				if (!Read_Uint32(row.Find("startMs"), collider.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), collider.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), collider.positionOffset) ||
					!Read_Float3(row.Find("halfExtents"), collider.halfExtents) ||
					!Read_Float3(row.Find("gripLocalOffset"), collider.gripLocalOffset) ||
					!row.Find("yawDegrees")->Is_Number() || !row.Find("damagePercent")->Is_Number())
				{ outStatus = "World Object collider timing or values are invalid"; return false; }
				collider.yawDegrees = static_cast<f32_t>(row.Find("yawDegrees")->Get_Number());
				collider.damagePercent = static_cast<f32_t>(row.Find("damagePercent")->Get_Number());
				parsedTemplate.colliderTracks.push_back(std::move(collider));
			}
		}

        if (const auto* sounds = templateValue.Find("soundTracks"))
        {
            if (parsedFormatVersion < 3u || !sounds->Is_Array() || sounds->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World soundTracks must be a bounded v3 array"; return false; }
            for (const auto& row : sounds->Get_Array())
            {
                WORLD_SEQUENCE_SOUND_TRACK sound;
                if (!Is_ObjectShape(row, { "soundTrackId", "assetId", "startMs", "durationMs", "volume" }, { "loopToDuration" }) ||
                    !row.Find("soundTrackId")->Is_String() || !row.Find("assetId")->Is_String() || !row.Find("volume")->Is_Number() ||
                    !Read_Uint32(row.Find("startMs"), sound.startMs, MAX_DURATION_MS) ||
                    !Read_Uint32(row.Find("durationMs"), sound.durationMs, MAX_DURATION_MS))
                { outStatus = "World sound track fields are invalid"; return false; }
                sound.soundTrackId = row.Find("soundTrackId")->Get_String();
                sound.assetId = row.Find("assetId")->Get_String();
                sound.volume = static_cast<f32_t>(row.Find("volume")->Get_Number());
                if (const auto* loop = row.Find("loopToDuration"))
                {
                    if (!loop->Is_Boolean()) { outStatus = "World sound loopToDuration must be boolean"; return false; }
                    sound.loopToDuration = loop->Get_Boolean();
                }
                parsedTemplate.soundTracks.push_back(std::move(sound));
            }
        }
        if (const auto* subtitles = templateValue.Find("subtitleTracks"))
        {
            if (parsedFormatVersion < 3u || !subtitles->Is_Array() || subtitles->Get_Array().size() > MAX_TRACK_COUNT)
            { outStatus = "World subtitleTracks must be a bounded v3 array"; return false; }
            for (const auto& row : subtitles->Get_Array())
            {
                WORLD_SEQUENCE_SUBTITLE_TRACK subtitle;
                if (!Is_ExactObject(row, { "subtitleTrackId", "stringId", "text", "position", "slotId", "startMs", "durationMs" }))
                { outStatus = "World subtitle track shape is invalid"; return false; }
                for (const auto* key : { "subtitleTrackId", "stringId", "text", "position", "slotId" })
                    if (!row.Find(key)->Is_String())
                    { outStatus = "World subtitle identity and text must be strings"; return false; }
                if (!Read_Uint32(row.Find("startMs"), subtitle.startMs, MAX_DURATION_MS) ||
                    !Read_Uint32(row.Find("durationMs"), subtitle.durationMs, MAX_DURATION_MS))
                { outStatus = "World subtitle timing is invalid"; return false; }
                subtitle.subtitleTrackId = row.Find("subtitleTrackId")->Get_String();
                subtitle.stringId = row.Find("stringId")->Get_String();
                subtitle.text = row.Find("text")->Get_String();
                subtitle.position = row.Find("position")->Get_String();
                subtitle.slotId = row.Find("slotId")->Get_String();
                parsedTemplate.subtitleTracks.push_back(std::move(subtitle));
            }
        }

		staged.m_Templates.push_back(std::move(parsedTemplate));
	}

	for (const DATA_JSON_VALUE& instanceValue : instances->Get_Array())
	{
		if (!Is_ObjectShape(instanceValue,
			{ "instanceId", "templateId", "enabled", "startDelayMs",
			  "playbackSpeed", "bindings" }, { "anchorKind", "position", "motionEnd", "nextMotionId", "walkableSurface", "loopFullPresentation" }))
		{
			outStatus = "World sequence instance shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* instanceId = instanceValue.Find("instanceId");
		const DATA_JSON_VALUE* templateId = instanceValue.Find("templateId");
		const DATA_JSON_VALUE* enabled = instanceValue.Find("enabled");
		const DATA_JSON_VALUE* bindings = instanceValue.Find("bindings");
		WORLD_SEQUENCE_INSTANCE parsedInstance;
		if (nullptr == instanceId || !instanceId->Is_String() ||
			nullptr == templateId || !templateId->Is_String() ||
			nullptr == enabled || !enabled->Is_Boolean() ||
			!Read_Uint32(instanceValue.Find("startDelayMs"),
				parsedInstance.startDelayMs, MAX_DURATION_MS) ||
			!Read_FiniteFloat(instanceValue.Find("playbackSpeed"),
				parsedInstance.playbackSpeed) ||
			nullptr == bindings || !bindings->Is_Array() ||
			bindings->Get_Array().size() > MAX_TRACK_COUNT)
		{
			outStatus = "World sequence instance fields are invalid";
			return false;
		}
		parsedInstance.instanceId = instanceId->Get_String();
		parsedInstance.templateId = templateId->Get_String();
		parsedInstance.enabled = enabled->Get_Boolean();
		if (const auto* loop = instanceValue.Find("loopFullPresentation"))
		{
			if (parsedFormatVersion < 3u || !loop->Is_Boolean())
			{ outStatus = "World Object full presentation loop must be a v3 boolean"; return false; }
			parsedInstance.loopFullPresentation = loop->Get_Boolean();
		}
		const DATA_JSON_VALUE* anchor = instanceValue.Find("anchorKind");
		const DATA_JSON_VALUE* position = instanceValue.Find("position");
		if ((parsedFormatVersion < 3u && (anchor || position)) ||
			(anchor && !anchor->Is_String()) ||
			(position && !Read_Float3(position, parsedInstance.position)))
		{ outStatus = "World object instance anchor is invalid"; return false; }
		if (anchor) parsedInstance.anchorKind = anchor->Get_String();
		const DATA_JSON_VALUE* motionEnd = instanceValue.Find("motionEnd");
		const DATA_JSON_VALUE* nextMotionId = instanceValue.Find("nextMotionId");
		if ((parsedFormatVersion < 3u && (motionEnd || nextMotionId)) ||
			(motionEnd && (!motionEnd->Is_String() ||
				!Try_ParseMotionEnd(motionEnd->Get_String(), parsedInstance.motionEnd))) ||
			(nextMotionId && !nextMotionId->Is_String()))
		{ outStatus = "World object motion completion is invalid"; return false; }
		if (nextMotionId) parsedInstance.nextMotionId = nextMotionId->Get_String();
		if (const DATA_JSON_VALUE* surface = instanceValue.Find("walkableSurface"))
		{
			WORLD_SEQUENCE_WALKABLE_SURFACE parsed;
			if (parsedFormatVersion < 3u || !Is_ExactObject(*surface, { "radiusM", "localHeightM" }) ||
				!Read_FiniteFloat(surface->Find("radiusM"), parsed.radiusM) ||
				!Read_FiniteFloat(surface->Find("localHeightM"), parsed.localHeightM))
			{ outStatus = "Invalid walkable surface fields: " + parsedInstance.instanceId; return false; }
			parsedInstance.walkableSurface = parsed;
		}

		for (const DATA_JSON_VALUE& bindingValue : bindings->Get_Array())
		{
			const bool_t validBindingShape =
				LEGACY_FORMAT_VERSION == parsedFormatVersion ?
				Is_ExactObject(bindingValue, { "slotId", "placementId" }) :
				Is_ObjectShape(bindingValue,
					{ "slotId", "targetKind", "targetId" }, { "previewNpcPlacementId" });
			if (!validBindingShape)
			{
				outStatus = "World sequence binding shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* slotId = bindingValue.Find("slotId");
			WORLD_SEQUENCE_BINDING parsedBinding;
			if (nullptr == slotId || !slotId->Is_String())
			{
				outStatus = "World sequence binding fields are invalid";
				return false;
			}
			parsedBinding.slotId = slotId->Get_String();
			if (LEGACY_FORMAT_VERSION == parsedFormatVersion)
			{
				uint64_t placementId = 0;
				if (!Parse_Uint64String(bindingValue.Find("placementId"),
					placementId))
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetKind =
					WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
				parsedBinding.targetId = std::to_string(placementId);
			}
			else
			{
				const DATA_JSON_VALUE* targetKind =
					bindingValue.Find("targetKind");
				const DATA_JSON_VALUE* targetId = bindingValue.Find("targetId");
				if (nullptr == targetKind || !targetKind->Is_String() ||
					!Try_ParseTargetKind(targetKind->Get_String(),
						parsedBinding.targetKind) ||
					nullptr == targetId || !targetId->Is_String())
				{
					outStatus = "World sequence binding fields are invalid";
					return false;
				}
				parsedBinding.targetId = targetId->Get_String();
				if (parsedFormatVersion < 3u && parsedBinding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object resource binding requires formatVersion 3"; return false; }
			}
			if (const auto* placement = bindingValue.Find("previewNpcPlacementId"))
			{
				if (!placement->Is_String() || !Is_ValidStableId(placement->Get_String()))
				{ outStatus = "Invalid NPC preview placement"; return false; }
				parsedBinding.previewNpcPlacementId = placement->Get_String();
			}
			parsedInstance.bindings.push_back(std::move(parsedBinding));
		}
		staged.m_Instances.push_back(std::move(parsedInstance));
	}

	if (!staged.Validate(availablePlacements, availableDeployPlacements,
		outStatus))
		return false;
	*this = std::move(staged);
	outStatus = "Loaded world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Save(
	const std::filesystem::path& path,
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	if (!Validate(availablePlacements, availableDeployPlacements, outStatus))
		return false;
	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create world sequence authoring directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create world sequence temporary file";
		return false;
	}
	output << std::setprecision(9)
		<< "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(m_AreaId) << "\",\n"
		<< "  \"revision\": " << m_iRevision << ",\n";
    if (!m_ObjectFolders.empty())
    {
        output << "  \"objectFolders\": [";
        for (size_t index = 0; index < m_ObjectFolders.size(); ++index)
        {
            const auto& folder = m_ObjectFolders[index];
            output << (index ? ",\n" : "\n") << "    {\n"
                << "      \"folderId\": \"" << CDataJson::Escape(folder.folderId) << "\",\n"
                << "      \"displayName\": \"" << CDataJson::Escape(folder.displayName) << "\",\n"
                << "      \"anchorKind\": \"" << CDataJson::Escape(folder.anchorKind) << "\"";
            if (!folder.parentId.empty())
                output << ",\n      \"parentId\": \"" << CDataJson::Escape(folder.parentId) << "\"";
            output << "\n    }";
        }
        output << "\n  ],\n";
    }
    output << "  \"objectResources\": [";
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& object = m_ObjectResources[index];
		output << (index == 0u ? "\n" : ",\n") << "    {\n"
			<< "      \"objectId\": \"" << CDataJson::Escape(object.objectId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(object.displayName) << "\",\n"
			<< "      \"modelAssetId\": \"" << CDataJson::Escape(object.modelAssetId) << "\",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(object.anchorKind) << "\",\n"
			<< "      \"diffuseTextureAssetId\": \"" << CDataJson::Escape(object.diffuseTextureAssetId) << "\",\n"
			<< "      \"modelPreScale\": " << object.modelPreScale << ",\n"
			<< "      \"animated\": " << (object.animated ? "true" : "false") << ",\n"
			<< "      \"scale\": [" << object.scale.x << ", " << object.scale.y << ", " << object.scale.z << "],\n"
			<< "      \"sequenceInstanceId\": \"" << CDataJson::Escape(object.sequenceInstanceId) << "\"";
        if (!object.parentId.empty())
            output << ",\n      \"parentId\": \"" << CDataJson::Escape(object.parentId) << "\"";
		if (object.anchorKind == "BOSS")
			output << ",\n      \"anchorBossArchetypeId\": \"" << CDataJson::Escape(object.anchorBossArchetypeId)
				<< "\",\n      \"anchorBone\": \"" << CDataJson::Escape(object.anchorBone) << "\"";
		if (!object.defaultMotionInstanceId.empty())
			output << ",\n      \"defaultMotionInstanceId\": \"" << CDataJson::Escape(object.defaultMotionInstanceId) << "\"";
		if (!object.motionInstanceIds.empty())
		{
			output << ",\n      \"motionInstanceIds\": [";
			for (size_t i = 0; i < object.motionInstanceIds.size(); ++i)
				output << (i ? ", " : "") << "\"" << CDataJson::Escape(object.motionInstanceIds[i]) << "\"";
			output << "]";
		}
        if (object.combatBody)
        {
            const auto& body = *object.combatBody;
            output << ",\n      \"combatBody\": { \"maxHp\": " << body.maxHp
                << ", \"localCenterM\": [" << body.localCenterM.x << ", " << body.localCenterM.y << ", " << body.localCenterM.z
                << "], \"halfExtentsM\": [" << body.halfExtentsM.x << ", " << body.halfExtentsM.y << ", " << body.halfExtentsM.z
                << "], \"lifetimePolicy\": \"UNTIL_DESTROYED\"";
            if (body.shape != "BOX") output << ", \"shape\": \"" << CDataJson::Escape(body.shape) << "\"";
            output << " }";
        }
        if (!object.materialSourceModelAssetId.empty())
            output << ",\n      \"materialSourceModelAssetId\": \"" << CDataJson::Escape(object.materialSourceModelAssetId) << "\"";
        if (!object.animationSetAssetId.empty())
            output << ",\n      \"animationSetAssetId\": \"" << CDataJson::Escape(object.animationSetAssetId) << "\"";
        if (!object.presentationBossArchetypeId.empty())
            output << ",\n      \"presentationBossArchetypeId\": \"" << CDataJson::Escape(object.presentationBossArchetypeId) << "\"";
        if (!object.mapMaterialBindings.empty())
        {
            output << ",\n      \"mapMaterialBindings\": [";
            for (size_t i = 0; i < object.mapMaterialBindings.size(); ++i)
            {
                const auto& binding = object.mapMaterialBindings[i];
                output << (i ? "," : "") << "\n        {\"materialName\": \"" << CDataJson::Escape(binding.materialName)
                    << "\", \"sourceAssetId\": \"" << CDataJson::Escape(binding.sourceAssetId)
                    << "\", \"sourceMaterialName\": \"" << CDataJson::Escape(binding.sourceMaterialName) << "\"";
                if (!binding.diffuseTextureAssetId.empty()) output << ", \"diffuseTextureAssetId\": \"" << CDataJson::Escape(binding.diffuseTextureAssetId) << "\"";
                if (binding.unlit) output << ", \"unlit\": true";
                output << "}";
            }
            output << "\n      ]";
        }
        if (object.materialProfile)
        {
            const auto& profile = *object.materialProfile;
            output << ",\n      \"materialProfile\": {\n        \"materialName\": \"" << CDataJson::Escape(profile.materialName)
                << "\",\n        \"sourceMaterial\": \"" << CDataJson::Escape(profile.sourceMaterial)
                << "\",\n        \"family\": \"" << CDataJson::Escape(profile.family) << "\",\n        \"parameters\": {";
            bool first = true;
            for (const auto& [name, value] : profile.parameters)
            {
                output << (first ? "" : ",") << "\n          \"" << CDataJson::Escape(name) << "\": ["
                    << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << "]";
                first = false;
            }
            output << "\n        },\n        \"textures\": [";
            for (size_t i = 0u; i < profile.textures.size(); ++i)
            {
                const auto& texture = profile.textures[i];
                output << (i ? "," : "") << "\n          {\"expressionIndex\": " << texture.expressionIndex
                    << ", \"assetId\": \"" << CDataJson::Escape(texture.assetId)
                    << "\", \"colorSpace\": \"" << (texture.srgb ? "srgb" : "linear") << "\"}";
            }
            output << "\n        ]\n      }";
        }
		output << "\n    }";
	}
	output << (m_ObjectResources.empty() ? "],\n" : "\n  ],\n")
		<< "  \"templates\": [";
	for (size_t templateIndex = 0; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& value = m_Templates[templateIndex];
		output << (0u == templateIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"sequenceId\": \"" << CDataJson::Escape(value.sequenceId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(value.displayName) << "\",\n"
			<< "      \"category\": \"" << CDataJson::Escape(value.category) << "\",\n"
			<< "      \"durationMs\": " << value.durationMs << ",\n"
			<< "      \"interpolation\": \"" << Interpolation_ToString(value.interpolation) << "\",\n"
			<< "      \"objectMotion\": {\n";
		const auto& motion = value.objectMotion;
		const auto writeVector = [&output](const char_t* name, const float3_t& vector)
		{
			output << "        \"" << name << "\": [" << vector.x << ", " << vector.y << ", " << vector.z << "],\n";
		};
		writeVector("velocity", motion.velocity);
		writeVector("acceleration", motion.acceleration);
		writeVector("angularVelocityDegrees", motion.angularVelocityDegrees);
		writeVector("revolutionDegreesPerSecond", motion.revolutionDegreesPerSecond);
		writeVector("revolutionOffset", motion.revolutionOffset);
		if (motion.spawnHalfExtents.x != 0.f || motion.spawnHalfExtents.y != 0.f || motion.spawnHalfExtents.z != 0.f)
			writeVector("spawnHalfExtents", motion.spawnHalfExtents);
		output << "        \"count\": " << motion.count << ", \"intervalMs\": " << motion.intervalMs
			<< ", \"spreadDegrees\": " << motion.spreadDegrees << ", \"seed\": " << motion.seed;
		if (!motion.emissions.empty())
		{
			output << ",\n        \"emissions\": [";
			for (size_t emissionIndex = 0; emissionIndex < motion.emissions.size(); ++emissionIndex)
			{
				const auto& emission = motion.emissions[emissionIndex];
				output << (0u == emissionIndex ? "\n" : ",\n")
					<< "          {\"positionOffset\": [" << emission.positionOffset.x << ", " << emission.positionOffset.y
					<< ", " << emission.positionOffset.z << "], \"yawDegrees\": " << emission.yawDegrees
					<< ", \"startDelayMs\": " << emission.startDelayMs << "}";
			}
			output << "\n        ]";
		}
		output << "\n      },\n"
			<< "      \"tracks\": [";
		for (size_t trackIndex = 0; trackIndex < value.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& track = value.tracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        {\n"
				<< "          \"slotId\": \"" << CDataJson::Escape(track.slotId) << "\",\n"
				<< "          \"keys\": [";
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				output << (0u == keyIndex ? "\n" : ",\n")
					<< "            {\n"
					<< "              \"timeMs\": " << key.timeMs << ",\n"
					<< "              \"positionOffset\": [" << key.positionOffset.x << ", "
					<< key.positionOffset.y << ", " << key.positionOffset.z << "],\n"
					<< "              \"rotationQuaternion\": [" << key.rotationQuaternion.x << ", "
					<< key.rotationQuaternion.y << ", " << key.rotationQuaternion.z << ", "
					<< key.rotationQuaternion.w << "],\n"
					<< "              \"scaleMultiplier\": [" << key.scaleMultiplier.x << ", "
					<< key.scaleMultiplier.y << ", " << key.scaleMultiplier.z << "],\n"
					<< "              \"visible\": " << (key.visible ? "true" : "false") << "\n"
					<< "            }";
			}
			output << (track.keys.empty() ? "]\n" : "\n          ]\n")
				<< "        }";
		}
		output << (value.tracks.empty() ? "],\n" : "\n      ],\n")
			<< "      \"animationTracks\": [";
		for (size_t trackIndex = 0;
			trackIndex < value.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& track =
				value.animationTracks[trackIndex];
			output << (0u == trackIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \""
				<< CDataJson::Escape(track.slotId)
				<< "\", \"clipName\": \""
				<< CDataJson::Escape(track.clipName)
				<< "\"";
			if (!track.displayName.empty())
				output << ", \"displayName\": \"" << CDataJson::Escape(track.displayName) << "\"";
			if (track.sourceStartMs != 0u)
				output << ", \"sourceStartMs\": " << track.sourceStartMs;
			if (track.sourceEndMs != 0u)
				output << ", \"sourceEndMs\": " << track.sourceEndMs;
			output << ", \"startMs\": " << track.startMs
					<< ", \"playbackRate\": " << track.playbackRate
				<< ", \"loop\": " << (track.loop ? "true" : "false")
				<< ", \"holdLastFrame\": "
				<< (track.holdLastFrame ? "true" : "false") << " }";
		}
		output << (value.animationTracks.empty() ? "]" : "\n      ]");
        if (!value.materialTracks.empty())
        {
            output << ",\n      \"materialTracks\": [";
            for (size_t index = 0u; index < value.materialTracks.size(); ++index)
            {
                const auto& track = value.materialTracks[index];
                output << (index ? "," : "") << "{\"slotId\":\"" << CDataJson::Escape(track.slotId)
                    << "\",\"materialName\":\"" << CDataJson::Escape(track.materialName) << "\",\"curves\":[";
                for (size_t curveIndex = 0u; curveIndex < track.curves.size(); ++curveIndex)
                {
                    const auto& curve = track.curves[curveIndex];
                    output << (curveIndex ? "," : "") << "{\"parameter\":\"" << CDataJson::Escape(curve.parameter) << "\",\"keys\":[";
                    for (size_t keyIndex = 0u; keyIndex < curve.keys.size(); ++keyIndex)
                    {
                        const auto& key = curve.keys[keyIndex];
                        output << (keyIndex ? "," : "") << "{\"timeMs\":" << key.timeMs << ",\"value\":[";
                        for (size_t axis = 0u; axis < 4u; ++axis) output << (axis ? "," : "") << key.value[axis];
                        output << "],\"interpolation\":\"" << (key.constant ? "CONSTANT" : "LINEAR") << "\"}";
                    }
                    output << "]}";
                }
                output << "]}";
            }
            output << "]";
        }
		if (!value.effectTracks.empty())
		{
			output << ",\n      \"effectTracks\": [";
			for (size_t index = 0; index < value.effectTracks.size(); ++index)
			{
				const auto& effect = value.effectTracks[index];
				output << (index ? ",\n" : "\n") << "        { \"effectTrackId\": \"" << CDataJson::Escape(effect.effectTrackId)
					<< "\", \"slotId\": \"" << CDataJson::Escape(effect.slotId)
					<< "\", \"resourceKind\": \"" << effect.resourceKind
					<< "\", \"resourceId\": \"" << CDataJson::Escape(effect.resourceId)
					<< "\", \"timing\": \"" << effect.timing
					<< "\", \"followObject\": " << (effect.followObject ? "true" : "false")
					<< ", \"bone\": \"" << CDataJson::Escape(effect.bone)
					<< "\", \"startMs\": " << effect.startMs << ", \"durationMs\": " << effect.durationMs
					<< ", \"positionOffset\": [" << effect.positionOffset.x << ", " << effect.positionOffset.y << ", " << effect.positionOffset.z
					<< "], \"rotationDegrees\": [" << effect.rotationDegrees.x << ", " << effect.rotationDegrees.y << ", " << effect.rotationDegrees.z
					<< "], \"scale\": [" << effect.scale.x << ", " << effect.scale.y << ", " << effect.scale.z << "]";
				if (!effect.inheritObjectRotation) output << ", \"inheritObjectRotation\": false";
				if (effect.fitEffectToDuration) output << ", \"fitEffectToDuration\": true";
				if (effect.loopEffectToDuration) output << ", \"loopEffectToDuration\": true";
				output << " }";
			}
			output << "\n      ]";
		}
		if (!value.colliderTracks.empty())
		{
			output << ",\n      \"colliderTracks\": [";
			for (size_t index = 0; index < value.colliderTracks.size(); ++index)
			{
				const auto& collider = value.colliderTracks[index];
				output << (index ? ",\n" : "\n") << "        { \"colliderTrackId\": \"" << CDataJson::Escape(collider.colliderTrackId)
					<< "\", \"slotId\": \"" << CDataJson::Escape(collider.slotId)
					<< "\", \"startMs\": " << collider.startMs << ", \"durationMs\": " << collider.durationMs
					<< ", \"positionOffset\": [" << collider.positionOffset.x << ", " << collider.positionOffset.y << ", " << collider.positionOffset.z
					<< "], \"halfExtents\": [" << collider.halfExtents.x << ", " << collider.halfExtents.y << ", " << collider.halfExtents.z
					<< "], \"yawDegrees\": " << collider.yawDegrees << ", \"behavior\": \"" << collider.behavior
					<< "\", \"damagePercent\": " << collider.damagePercent << ", \"gripLocalOffset\": ["
					<< collider.gripLocalOffset.x << ", " << collider.gripLocalOffset.y << ", " << collider.gripLocalOffset.z << "]";
				if (collider.shape != "BOX") output << ", \"shape\": \"" << CDataJson::Escape(collider.shape) << "\"";
				if (!collider.attachmentBone.empty()) output << ", \"attachmentBone\": \"" << CDataJson::Escape(collider.attachmentBone) << "\"";
				output << " }";
			}
			output << "\n      ]";
		}
        if (!value.soundTracks.empty())
        {
            output << ",\n      \"soundTracks\": [";
            for (size_t index = 0; index < value.soundTracks.size(); ++index)
            {
                const auto& sound = value.soundTracks[index];
                output << (index ? ",\n" : "\n") << "        { \"soundTrackId\": \"" << CDataJson::Escape(sound.soundTrackId)
                    << "\", \"assetId\": \"" << CDataJson::Escape(sound.assetId)
                    << "\", \"startMs\": " << sound.startMs << ", \"durationMs\": " << sound.durationMs
                    << ", \"volume\": " << sound.volume;
                if (sound.loopToDuration) output << ", \"loopToDuration\": true";
                output << " }";
            }
            output << "\n      ]";
        }
        if (!value.subtitleTracks.empty())
        {
            output << ",\n      \"subtitleTracks\": [";
            for (size_t index = 0; index < value.subtitleTracks.size(); ++index)
            {
                const auto& subtitle = value.subtitleTracks[index];
                output << (index ? ",\n" : "\n") << "        { \"subtitleTrackId\": \"" << CDataJson::Escape(subtitle.subtitleTrackId)
                    << "\", \"stringId\": \"" << CDataJson::Escape(subtitle.stringId)
                    << "\", \"text\": \"" << CDataJson::Escape(subtitle.text)
                    << "\", \"position\": \"" << subtitle.position << "\", \"slotId\": \"" << CDataJson::Escape(subtitle.slotId)
                    << "\", \"startMs\": " << subtitle.startMs << ", \"durationMs\": " << subtitle.durationMs << " }";
            }
            output << "\n      ]";
        }
		output << "\n    }";
	}
	output << (m_Templates.empty() ? "],\n" : "\n  ],\n")
		<< "  \"instances\": [";
	for (size_t instanceIndex = 0; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& value = m_Instances[instanceIndex];
		output << (0u == instanceIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"instanceId\": \"" << CDataJson::Escape(value.instanceId) << "\",\n"
			<< "      \"templateId\": \"" << CDataJson::Escape(value.templateId) << "\",\n"
			<< "      \"enabled\": " << (value.enabled ? "true" : "false") << ",\n"
			<< "      \"startDelayMs\": " << value.startDelayMs << ",\n"
			<< "      \"playbackSpeed\": " << value.playbackSpeed << ",\n"
			<< "      \"anchorKind\": \"" << CDataJson::Escape(value.anchorKind) << "\",\n"
			<< "      \"position\": [" << value.position.x << ", " << value.position.y << ", " << value.position.z << "],\n"
			<< "      \"motionEnd\": \"" << MotionEnd_ToString(value.motionEnd) << "\",\n"
			<< "      \"nextMotionId\": \"" << CDataJson::Escape(value.nextMotionId) << "\",\n"
			;
		if (value.loopFullPresentation) output << "      \"loopFullPresentation\": true,\n";
		if (value.walkableSurface)
			output << "      \"walkableSurface\": { \"radiusM\": " << value.walkableSurface->radiusM
				<< ", \"localHeightM\": " << value.walkableSurface->localHeightM << " },\n";
		output << "      \"bindings\": [";
		for (size_t bindingIndex = 0; bindingIndex < value.bindings.size();
			++bindingIndex)
		{
			const WORLD_SEQUENCE_BINDING& binding = value.bindings[bindingIndex];
			output << (0u == bindingIndex ? "\n" : ",\n")
				<< "        { \"slotId\": \"" << CDataJson::Escape(binding.slotId)
				<< "\", \"targetKind\": \""
				<< TargetKind_ToString(binding.targetKind)
				<< "\", \"targetId\": \""
				<< CDataJson::Escape(binding.targetId) << "\"";
			if (!binding.previewNpcPlacementId.empty())
				output << ", \"previewNpcPlacementId\": \"" << CDataJson::Escape(binding.previewNpcPlacementId) << "\"";
			output << " }";
		}
		output << (value.bindings.empty() ? "]\n" : "\n      ]\n")
			<< "    }";
	}
	output << (m_Instances.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	bool_t writeSucceeded = output.good();
	output.close();
	writeSucceeded = writeSucceeded && !output.fail();
	if (!writeSucceeded || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit world sequence document atomically";
		return false;
	}
	outStatus = "Saved world sequences: " +
		std::to_string(m_Templates.size()) + " templates, " +
		std::to_string(m_Instances.size()) + " instances";
	return true;
}

bool_t Client::CWorldSequenceDocument::Validate_ObjectHierarchy(std::string& outStatus) const
{
    if (m_ObjectFolders.size() > MAX_INSTANCE_COUNT || m_ObjectResources.size() > MAX_INSTANCE_COUNT)
    { outStatus = "World object hierarchy exceeds its limits"; return false; }
    struct NODE { const std::string* parent; const std::string* anchor; };
    std::unordered_map<std::string, NODE> nodes;
    const auto add = [&](const std::string& id, const std::string& name,
        const std::string& anchor, const std::string& parent) {
        return Is_ValidStableId(id) && !name.empty() && name.size() <= 128u &&
            Is_ValidUtf8DisplayText(name) &&
            (anchor == "WORLD" || anchor == "PLAYER" || anchor == "BOSS") &&
            (parent.empty() || Is_ValidStableId(parent)) && nodes.emplace(id, NODE{&parent, &anchor}).second;
    };
    for (const auto& folder : m_ObjectFolders)
        if (!add(folder.folderId, folder.displayName, folder.anchorKind, folder.parentId))
        { outStatus = "Invalid or duplicate Object folder: " + folder.folderId; return false; }
    for (const auto& object : m_ObjectResources)
        if (!add(object.objectId, object.displayName, object.anchorKind, object.parentId))
        { outStatus = "Invalid or duplicate Object hierarchy entry: " + object.objectId; return false; }
    for (const auto& [id, node] : nodes)
    {
        std::unordered_set<std::string> visited{id};
        const NODE* current = &node;
        size_t depth = 0;
        while (!current->parent->empty())
        {
            const auto found = nodes.find(*current->parent);
            if (found == nodes.end() || *found->second.anchor != *node.anchor)
            { outStatus = "Object parent must exist in the same anchor category: " + id; return false; }
            if (!visited.insert(found->first).second || ++depth > 64u)
            { outStatus = "Object hierarchy contains a cycle or exceeds 64 parents: " + id; return false; }
            current = &found->second;
        }
    }
    return true;
}

bool_t Client::CWorldSequenceDocument::Build_PlaybackSubset(
    const std::vector<std::string>& roots, CWorldSequenceDocument& out, std::string& status) const
{
    CWorldSequenceDocument staged;
    staged.m_AreaId = m_AreaId;
    staged.m_iRevision = m_iRevision;
    std::unordered_set<std::string> instances, objects, templates;
    std::vector<std::string> pending = roots;
    if (pending.empty()) { status = "World playback selection is empty."; return false; }
    for (size_t index = 0u; index < pending.size(); ++index)
    {
        const std::string id = pending[index];
        if (const auto* object = Find_ObjectResource(id))
        {
            if (!objects.insert(id).second) continue;
            staged.m_ObjectResources.push_back(*object);
            // Folder ancestry is authoring organization, not a playback dependency.
            staged.m_ObjectResources.back().parentId.clear();
            if (!object->sequenceInstanceId.empty()) pending.push_back(object->sequenceInstanceId);
            if (!object->defaultMotionInstanceId.empty()) pending.push_back(object->defaultMotionInstanceId);
            pending.insert(pending.end(), object->motionInstanceIds.begin(), object->motionInstanceIds.end());
            if (!object->modelAssetId.empty())
                for (const auto& motion : m_Instances)
                    if (std::any_of(motion.bindings.begin(), motion.bindings.end(), [&](const auto& binding) {
                        return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == id;
                    })) pending.push_back(motion.instanceId);
            continue;
        }
        if (!instances.insert(id).second) continue;
        const auto* instance = Find_Instance(id);
        const auto* sequence = instance ? Find_Template(instance->templateId) : nullptr;
        if (!instance || !sequence)
        { status = "World playback dependency is unavailable: " + id; return false; }
        staged.m_Instances.push_back(*instance);
        if (templates.insert(sequence->sequenceId).second) staged.m_Templates.push_back(*sequence);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
        for (const auto& binding : instance->bindings)
            if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) pending.push_back(binding.targetId);
        if (instances.size() > MAX_INSTANCE_COUNT || objects.size() > MAX_INSTANCE_COUNT || templates.size() > MAX_TEMPLATE_COUNT)
        { status = "World playback dependency closure exceeds document capacity."; return false; }
    }
    out = std::move(staged);
    status.clear();
    return true;
}

bool_t Client::CWorldSequenceDocument::Validate(
	const WORLD_SEQUENCE_PLACEMENT_MAP& availablePlacements,
	const WORLD_SEQUENCE_DEPLOY_MAP& availableDeployPlacements,
	std::string& outStatus) const
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "WorldSequence.Document.Validate");
	if (m_AreaId.empty() || m_AreaId.size() > 128u || 0u == m_iRevision ||
		m_Templates.size() > MAX_TEMPLATE_COUNT ||
		m_Instances.size() > MAX_INSTANCE_COUNT || m_ObjectResources.size() > MAX_INSTANCE_COUNT)
	{
		outStatus = "World sequence document header is invalid";
		return false;
	}
	if (!Validate_ObjectHierarchy(outStatus)) return false;
	std::unordered_set<std::string> objectIds;
	for (const WORLD_SEQUENCE_OBJECT_RESOURCE& object : m_ObjectResources)
	{
		if (!object.motionInstanceIds.empty())
		{
			if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
				object.displayName.empty() || object.displayName.size() > 128u || !Is_ValidUtf8DisplayText(object.displayName) ||
				object.motionInstanceIds.size() > 32u || object.anchorKind != "WORLD" ||
				!object.modelAssetId.empty() || !object.sequenceInstanceId.empty() || !object.defaultMotionInstanceId.empty() ||
				object.animated || !object.diffuseTextureAssetId.empty() || object.materialProfile || object.combatBody ||
				!object.materialSourceModelAssetId.empty() || !object.mapMaterialBindings.empty() ||
				!object.animationSetAssetId.empty() ||
				!object.presentationBossArchetypeId.empty() ||
				!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty() ||
				!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE || object.modelPreScale > MAX_COMPONENT ||
				!Is_BoundedFloat3(object.scale) || object.scale.x != 1.f || object.scale.y != 1.f || object.scale.z != 1.f)
			{ outStatus = "Invalid model-less Object group: " + object.objectId; return false; }
			std::unordered_set<std::string> members;
			for (const auto& id : object.motionInstanceIds)
			{
				const auto* instance = Find_Instance(id);
				if (!Is_ValidStableId(id) || !members.insert(id).second || !instance || instance->anchorKind != "WORLD" ||
					(instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && instance->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP) || instance->bindings.size() != 1u ||
					instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object group needs unique existing Map Object motions ending with Stop or Loop: " + id; return false; }
				const auto* model = Find_ObjectResource(instance->bindings.front().targetId);
				if (!model || model->modelAssetId.empty() || !model->motionInstanceIds.empty())
				{ outStatus = "Object group member must bind a model, not another group: " + id; return false; }
			}
			continue;
		}
		const bool_t alias = !object.sequenceInstanceId.empty();
		if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
			object.displayName.empty() || object.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(object.displayName) ||
			(object.anchorKind != "WORLD" && object.anchorKind != "PLAYER" && object.anchorKind != "BOSS") ||
			(object.anchorKind == "BOSS" ? !Is_ValidStableId(object.anchorBossArchetypeId) :
				(!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty())) ||
			object.anchorBone.size() > 128u || !Is_ValidUtf8DisplayText(object.anchorBone) ||
			(alias && object.anchorKind != "WORLD") ||
			!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE ||
			object.modelPreScale > MAX_COMPONENT || !Is_BoundedFloat3(object.scale) ||
			object.scale.x < MIN_SCALE || object.scale.y < MIN_SCALE || object.scale.z < MIN_SCALE ||
			(alias ? (!object.modelAssetId.empty() || !Is_ValidStableId(object.sequenceInstanceId) ||
				nullptr == Find_Instance(object.sequenceInstanceId) || !object.diffuseTextureAssetId.empty() || object.animated) :
				(!Is_ResourcePath(object.modelAssetId, true) ||
					(!object.diffuseTextureAssetId.empty() && !Is_ResourcePath(object.diffuseTextureAssetId, false)))))
		{
			outStatus = "Invalid or duplicate world object resource: " + object.objectId;
			return false;
		}
        if (object.combatBody)
        {
            const auto& body = *object.combatBody;
            if (alias || object.anchorKind != "WORLD" || body.maxHp == 0u || body.maxHp > 1000000000u ||
                (body.shape != "BOX" && body.shape != "ELLIPSOID") ||
                body.lifetimePolicy != "UNTIL_DESTROYED" || !Is_BoundedFloat3(body.localCenterM) ||
                !Is_BoundedFloat3(body.halfExtentsM) || body.halfExtentsM.x < .001f || body.halfExtentsM.y < .001f ||
                body.halfExtentsM.z < .001f || body.halfExtentsM.x > 1000.f || body.halfExtentsM.y > 1000.f || body.halfExtentsM.z > 1000.f)
            { outStatus = "Combat body requires a WORLD model with bounded HP and local bounds: " + object.objectId; return false; }
        }
        if ((!object.materialSourceModelAssetId.empty() && (alias || !Is_ResourcePath(object.materialSourceModelAssetId, true))) ||
            object.mapMaterialBindings.size() > 64u || (alias && !object.mapMaterialBindings.empty()))
        { outStatus = "Invalid world object material source: " + object.objectId; return false; }
        /* A clip donor only makes sense for a skinned body that plays clips. */
        if (!object.animationSetAssetId.empty() &&
            (alias || !object.animated || !Is_ResourcePath(object.animationSetAssetId, true)))
        { outStatus = "Invalid world object animation set: " + object.objectId; return false; }
        /* The product boss assembly borrows this skinned body's bone palette. */
        if (!object.presentationBossArchetypeId.empty() &&
            (alias || !object.animated || !Is_ValidStableId(object.presentationBossArchetypeId)))
        { outStatus = "Invalid world object presentation boss: " + object.objectId; return false; }
        std::unordered_set<std::string> materialNames;
        if (object.materialProfile) materialNames.insert(object.materialProfile->materialName);
        for (const auto& binding : object.mapMaterialBindings)
            if (binding.materialName.empty() || binding.materialName.size() > 63u || !Is_ValidUtf8DisplayText(binding.materialName) ||
                !Is_ValidStableId(binding.sourceAssetId) || binding.sourceMaterialName.empty() || binding.sourceMaterialName.size() > 63u ||
                !Is_ValidUtf8DisplayText(binding.sourceMaterialName) || !materialNames.insert(binding.materialName).second ||
                (!binding.diffuseTextureAssetId.empty() && !Is_ResourcePath(binding.diffuseTextureAssetId, false)))
            { outStatus = "Invalid or duplicate world object map material binding: " + object.objectId; return false; }
        if (object.materialProfile && (alias || !Validate_MaterialProfile(*object.materialProfile)))
        { outStatus = "Invalid world object material profile: " + object.objectId; return false; }
		if (!object.defaultMotionInstanceId.empty())
		{
			const auto* motion = Find_Instance(object.defaultMotionInstanceId);
			if (!Is_ValidStableId(object.defaultMotionInstanceId) || nullptr == motion || !motion->enabled ||
				(alias ? object.defaultMotionInstanceId != object.sequenceInstanceId :
					(motion->bindings.size() != 1u || motion->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
					 motion->bindings.front().targetId != object.objectId)))
			{
				outStatus = "Default Motion must be an enabled instance of the same Object: " + object.objectId;
				return false;
			}
		}
	}
	std::unordered_set<std::string> templateIds;
	for (const WORLD_SEQUENCE_TEMPLATE& value : m_Templates)
	{
		if (!Is_ValidStableId(value.sequenceId) ||
			!templateIds.insert(value.sequenceId).second ||
			value.displayName.empty() || value.displayName.size() > 128u ||
			!Is_ValidUtf8DisplayText(value.displayName) ||
			value.category.empty() || value.category.size() > 64u ||
			!Is_ValidUtf8DisplayText(value.category) ||
			0u == value.durationMs || value.durationMs > MAX_DURATION_MS ||
			(WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation &&
				WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP != value.interpolation) ||
			(value.tracks.empty() && value.animationTracks.empty()) ||
			value.tracks.size() + value.animationTracks.size() + value.effectTracks.size() + value.colliderTracks.size() + value.materialTracks.size() +
                value.soundTracks.size() + value.subtitleTracks.size() > MAX_TRACK_COUNT)
		{
			outStatus = "Invalid or duplicate world sequence template: " +
				value.sequenceId;
			return false;
		}
        std::unordered_set<std::string> soundIds, subtitleIds;
        for (const auto& sound : value.soundTracks)
            if (!Is_ValidStableId(sound.soundTrackId) || !soundIds.insert(sound.soundTrackId).second ||
                !Is_ResourcePath(sound.assetId, false) || !sound.assetId.starts_with("Sound/") || !sound.assetId.ends_with(".wav") ||
                sound.startMs > value.durationMs || sound.durationMs == 0u ||
                uint64_t(sound.startMs) + sound.durationMs > MAX_DURATION_MS ||
                !std::isfinite(sound.volume) || sound.volume < 0.f || sound.volume > 4.f)
            { outStatus = "Invalid World sound track: " + value.sequenceId + "/" + sound.soundTrackId; return false; }
        for (const auto& subtitle : value.subtitleTracks)
        {
            const bool balloon = subtitle.position == "BALLOON";
            const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
                [&](const auto& track) { return track.slotId == subtitle.slotId; });
            if (!Is_ValidStableId(subtitle.subtitleTrackId) || !subtitleIds.insert(subtitle.subtitleTrackId).second ||
                !Is_ValidStableId(subtitle.stringId) || subtitle.text.empty() || subtitle.text.size() > 4096u ||
                !Is_ValidUtf8DisplayText(subtitle.text, true) || subtitle.text.find_first_of("<>") != std::string::npos ||
                (!balloon && subtitle.position != "NORMAL" && subtitle.position != "UPPER") ||
                (balloon ? !Is_ValidStableId(subtitle.slotId) || !slotExists : !subtitle.slotId.empty()) ||
                !subtitle.durationMs || uint64_t(subtitle.startMs) + subtitle.durationMs > value.durationMs)
            { outStatus = "Invalid World subtitle track: " + value.sequenceId + "/" + subtitle.subtitleTrackId; return false; }
        }
		const auto& motion = value.objectMotion;
		if (!Is_BoundedFloat3(motion.velocity) || !Is_BoundedFloat3(motion.acceleration) ||
			!Is_BoundedFloat3(motion.angularVelocityDegrees) ||
			!Is_BoundedFloat3(motion.revolutionDegreesPerSecond) || !Is_BoundedFloat3(motion.revolutionOffset) ||
			!Is_BoundedFloat3(motion.spawnHalfExtents) || motion.spawnHalfExtents.x < 0.f ||
			motion.spawnHalfExtents.y < 0.f || motion.spawnHalfExtents.z < 0.f ||
			motion.count < 1u || motion.count > 128u || motion.intervalMs > MAX_DURATION_MS ||
			(value.effectTracks.empty() && motion.LastEmissionDelayMs() >= value.durationMs) ||
			!Is_ValidEmissionList(motion) ||
			!std::isfinite(motion.spreadDegrees) || motion.spreadDegrees < 0.f || motion.spreadDegrees > (value.effectTracks.empty() ? 180.f : 360.f))
		{ outStatus = "Invalid object motion in template: " + value.sequenceId; return false; }
		std::unordered_set<std::string> effectIds;
		for (const auto& effect : value.effectTracks)
		{
			const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
				[&](const auto& track) { return track.slotId == effect.slotId; }) ||
				std::any_of(value.animationTracks.begin(), value.animationTracks.end(),
					[&](const auto& track) { return track.slotId == effect.slotId; });
			if (!Is_ValidStableId(effect.effectTrackId) || !effectIds.insert(effect.effectTrackId).second ||
				!Is_ValidStableId(effect.slotId) || !slotExists || !Is_ValidStableId(effect.resourceId) ||
				(effect.resourceKind != "LEAF" && effect.resourceKind != "GROUP" && effect.resourceKind != "V1_EFFECT") ||
				((effect.fitEffectToDuration || effect.loopEffectToDuration) && effect.resourceKind != "V1_EFFECT") ||
				(effect.fitEffectToDuration && effect.loopEffectToDuration) ||
				effect.bone.size() > 256u || !Is_ValidUtf8DisplayText(effect.bone) ||
				(effect.timing != "TIME" && effect.timing != "MOTION_END") ||
				(effect.timing == "MOTION_END" && effect.startMs != 0u) ||
				effect.startMs > value.durationMs || effect.durationMs == 0u || effect.durationMs > MAX_DURATION_MS ||
				!Is_BoundedFloat3(effect.positionOffset) || !Is_BoundedFloat3(effect.rotationDegrees) ||
				!Is_BoundedFloat3(effect.scale) || effect.scale.x < MIN_SCALE || effect.scale.y < MIN_SCALE || effect.scale.z < MIN_SCALE ||
				value.PresentationSpanMs() > MAX_DURATION_MS)
			{ outStatus = "Invalid World Object effect track: " + value.sequenceId + "/" + effect.effectTrackId; return false; }
		}
		if (!value.colliderTracks.empty() && (motion.spawnHalfExtents.x != 0.f || motion.spawnHalfExtents.y != 0.f ||
			motion.spawnHalfExtents.z != 0.f || motion.spreadDegrees != 0.f))
		{ outStatus = "Collider tracks require deterministic Motion emission positions: " + value.sequenceId; return false; }
		std::unordered_set<std::string> colliderIds;
		for (const auto& collider : value.colliderTracks)
		{
			const bool hook = collider.behavior == "HOOK_CAPTURE";
			const bool damage = collider.behavior == "DAMAGE";
			const bool slotExists = std::any_of(value.tracks.begin(), value.tracks.end(),
				[&](const auto& track) { return track.slotId == collider.slotId; });
			if (!Is_ValidStableId(collider.colliderTrackId) || !colliderIds.insert(collider.colliderTrackId).second ||
				!Is_ValidStableId(collider.slotId) || !slotExists ||
				(collider.shape != "BOX" && collider.shape != "CYLINDER") ||
				(collider.shape == "CYLINDER" && (hook || std::abs(collider.halfExtents.x - collider.halfExtents.z) > .0001f)) ||
				collider.durationMs == 0u || uint64_t(collider.startMs) + collider.durationMs > value.durationMs ||
				!Is_BoundedFloat3(collider.positionOffset) || !Is_BoundedFloat3(collider.halfExtents) ||
				collider.halfExtents.x <= .001f || collider.halfExtents.y <= .001f || collider.halfExtents.z <= .001f ||
				collider.halfExtents.x > 1000.f || collider.halfExtents.y > 1000.f || collider.halfExtents.z > 1000.f ||
				!std::isfinite(collider.yawDegrees) || std::abs(collider.yawDegrees) > 36000.f ||
				(!hook && !damage && collider.behavior != "INSTANT_DEATH") || !std::isfinite(collider.damagePercent) ||
				(damage ? collider.damagePercent < 1.f || collider.damagePercent > 100.f || std::floor(collider.damagePercent) != collider.damagePercent : collider.damagePercent != 0.f) ||
				!Is_BoundedFloat3(collider.gripLocalOffset) || collider.attachmentBone.size() > 256u ||
				!Is_ValidUtf8DisplayText(collider.attachmentBone) ||
				(!hook && (!collider.attachmentBone.empty() || collider.gripLocalOffset.x != 0.f ||
					collider.gripLocalOffset.y != 0.f || collider.gripLocalOffset.z != 0.f)))
			{ outStatus = "Invalid World Object collider track: " + value.sequenceId + "/" + collider.colliderTrackId; return false; }
		}
		std::unordered_set<std::string> slotIds;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
		{
			if (!Is_ValidStableId(track.slotId) ||
				!slotIds.insert(track.slotId).second || track.keys.size() < 2u ||
				track.keys.size() > MAX_KEY_COUNT || 0u != track.keys.front().timeMs ||
				value.durationMs != track.keys.back().timeMs)
			{
				outStatus = "Invalid track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				const auto& first = track.keys.front().scaleMultiplier;
				// A reflected source actor is valid. Each axis must retain its sign
				// so interpolation never crosses a singular, zero-scale transform.
				if (!Is_FiniteTransform(key) ||
					key.timeMs > value.durationMs ||
					(0u != keyIndex &&
						track.keys[keyIndex - 1u].timeMs >= key.timeMs) ||
					std::signbit(first.x) != std::signbit(key.scaleMultiplier.x) ||
					std::signbit(first.y) != std::signbit(key.scaleMultiplier.y) ||
					std::signbit(first.z) != std::signbit(key.scaleMultiplier.z))
				{
					outStatus = "Invalid keyframe in world sequence template: " +
						value.sequenceId + "/" + track.slotId;
					return false;
				}
			}
		}
        std::unordered_set<std::string> materialTargets;
        for (const auto& track : value.materialTracks)
        {
            if (!Is_ValidStableId(track.slotId) || track.materialName.empty() || track.materialName.size() > 256u ||
                !Is_ValidUtf8DisplayText(track.materialName) || !materialTargets.insert(track.slotId + ":" + track.materialName).second ||
                track.curves.empty() || track.curves.size() > 64u ||
                (std::none_of(value.tracks.begin(), value.tracks.end(), [&](const auto& row) { return row.slotId == track.slotId; }) &&
                 std::none_of(value.animationTracks.begin(), value.animationTracks.end(), [&](const auto& row) { return row.slotId == track.slotId; })))
            { outStatus = "Invalid World material track target: " + value.sequenceId; return false; }
            std::unordered_set<std::string> parameters;
            for (const auto& curve : track.curves)
            {
                if (curve.parameter.empty() || curve.parameter.size() > 128u || !Is_ValidUtf8DisplayText(curve.parameter) ||
                    !parameters.insert(curve.parameter).second || curve.keys.empty() || curve.keys.size() > 4096u ||
                    curve.keys.front().timeMs != 0u || curve.keys.back().timeMs != value.durationMs)
                { outStatus = "World material curve must cover its motion: " + value.sequenceId; return false; }
                for (size_t index = 0u; index < curve.keys.size(); ++index)
                {
                    const auto& key = curve.keys[index];
                    if (key.timeMs > value.durationMs || (index && key.timeMs <= curve.keys[index - 1u].timeMs) ||
                        std::any_of(key.value.begin(), key.value.end(), [](float v) { return !std::isfinite(v) || std::abs(v) > 1000000.f; }))
                    { outStatus = "Invalid World material key: " + value.sequenceId; return false; }
                }
            }
        }
		/* An animation slot may carry an ordered clip chain, so its rows are
		   checked against the slot's previous start instead of a plain unique
		   set. A slot still may not be both a transform and an animation slot. */
		std::unordered_map<std::string, uint32_t> animationSlotStarts;
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
			value.animationTracks)
		{
			const auto chained = animationSlotStarts.find(track.slotId);
			const bool_t firstOfSlot = animationSlotStarts.end() == chained;
			/* A slot may carry both a transform track and a clip chain so one
			   binding can walk an animated prop while it plays. Only a second
			   animation chain on the same slot is a conflict. */
			if (!Is_ValidStableId(track.slotId) || track.clipName.empty() ||
				track.clipName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.clipName) ||
				track.displayName.size() > 128u ||
				!Is_ValidUtf8DisplayText(track.displayName) ||
				!std::isfinite(track.playbackRate) || track.playbackRate < 0.05f ||
				track.playbackRate > 8.f ||
				track.startMs >= value.durationMs || track.sourceStartMs > MAX_DURATION_MS ||
				track.sourceEndMs > MAX_DURATION_MS ||
				(track.sourceEndMs != 0u && track.sourceEndMs <= track.sourceStartMs) ||
				(!firstOfSlot && track.startMs <= chained->second))
			{
				outStatus = "Invalid animation track in world sequence template: " +
					value.sequenceId;
				return false;
			}
			animationSlotStarts[track.slotId] = track.startMs;
		}
	}

	/* One binding drives one slot, so a slot whose clips are chained still
	   needs exactly one. */
	const auto Count_BoundSlots =
		[](const WORLD_SEQUENCE_TEMPLATE& value) -> size_t
	{
		std::unordered_set<std::string> slots;
		for (const WORLD_SEQUENCE_TRACK& track : value.tracks)
			slots.insert(track.slotId);
		for (const WORLD_SEQUENCE_ANIMATION_TRACK& track : value.animationTracks)
			slots.insert(track.slotId);
		return slots.size();
	};

	std::unordered_set<std::string> instanceIds;
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const WORLD_SEQUENCE_TEMPLATE* targetTemplate = Find_Template(value.templateId);
		if (!Is_ValidStableId(value.instanceId) ||
			!instanceIds.insert(value.instanceId).second || nullptr == targetTemplate ||
			value.startDelayMs > MAX_DURATION_MS ||
			!std::isfinite(value.playbackSpeed) || value.playbackSpeed < 0.05f ||
			value.playbackSpeed > 8.f ||
			(value.anchorKind != "WORLD" && value.anchorKind != "PLAYER" && value.anchorKind != "BOSS") ||
			!Is_BoundedFloat3(value.position) ||
			value.bindings.size() != Count_BoundSlots(*targetTemplate))
		{
			outStatus = "Invalid world sequence instance: " + value.instanceId;
			return false;
		}
		if (value.loopFullPresentation && (value.motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP ||
			value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Full presentation loop requires one looping Object Resource: " + value.instanceId; return false; }
		if (!targetTemplate->colliderTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || value.anchorKind != "WORLD"))
		{ outStatus = "Collider tracks require one WORLD Object Resource binding: " + value.instanceId; return false; }
		if (!targetTemplate->effectTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Effect lanes require one Object Resource binding: " + value.instanceId; return false; }
        for (const auto& subtitle : targetTemplate->subtitleTracks)
            if (subtitle.position == "BALLOON" && std::none_of(value.bindings.begin(), value.bindings.end(), [&](const auto& binding) {
                return binding.slotId == subtitle.slotId && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; }))
            { outStatus = "World balloon subtitle requires its Object Resource binding: " + value.instanceId; return false; }
		if (value.walkableSurface)
		{
			const auto& surface = *value.walkableSurface;
			if (!std::isfinite(surface.radiusM) || surface.radiusM < 0.001f || surface.radiusM > 1000.f ||
				!std::isfinite(surface.localHeightM) || std::abs(surface.localHeightM) > 10000.f ||
				value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT ||
				value.anchorKind != "WORLD" || value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP ||
				targetTemplate->tracks.size() != 1u || !targetTemplate->animationTracks.empty())
			{ outStatus = "Walkable surface requires one static Map placement: " + value.instanceId; return false; }
			const auto& keys = targetTemplate->tracks.front().keys;
			const auto& first = keys.front();
			for (const auto& key : keys)
			{
				if (std::abs(key.rotationQuaternion.x) > 0.00001f || std::abs(key.rotationQuaternion.z) > 0.00001f ||
					key.positionOffset.x != first.positionOffset.x || key.positionOffset.y != first.positionOffset.y ||
					key.positionOffset.z != first.positionOffset.z || key.scaleMultiplier.x != first.scaleMultiplier.x ||
					key.scaleMultiplier.y != first.scaleMultiplier.y || key.scaleMultiplier.z != first.scaleMultiplier.z ||
					key.scaleMultiplier.x <= 0.f || key.scaleMultiplier.y <= 0.f ||
					std::abs(key.scaleMultiplier.x - key.scaleMultiplier.z) > 0.00001f)
				{ outStatus = "Walkable surface needs fixed position/scale and Y rotation only: " + value.instanceId; return false; }
			}
		}
		std::unordered_set<std::string> boundSlots;
		std::unordered_set<std::string> boundTargets;
		for (const WORLD_SEQUENCE_BINDING& binding : value.bindings)
		{
			if (!binding.previewNpcPlacementId.empty())
			{
				const auto* object = Find_ObjectResource(binding.targetId);
				if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || !object ||
					!object->animated || !object->sequenceInstanceId.empty() || object->combatBody ||
					!object->presentationBossArchetypeId.empty() || value.anchorKind != "WORLD" ||
					value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP ||
					value.bindings.size() != 1u || targetTemplate->objectMotion.EmissionCount() != 1u ||
					!targetTemplate->colliderTracks.empty() || !Is_ValidStableId(binding.previewNpcPlacementId))
				{ outStatus = "NPC preview requires one animated WORLD visual without combat"; return false; }
			}
			const auto transformSlot = std::find_if(targetTemplate->tracks.begin(),
				targetTemplate->tracks.end(),
				[&binding](const WORLD_SEQUENCE_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
            for (const auto& material : targetTemplate->materialTracks)
                if (material.slotId == binding.slotId)
                {
                    const auto* resource = binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
                        Find_ObjectResource(binding.targetId) : nullptr;
                    if (!resource || !resource->materialProfile || resource->materialProfile->materialName != material.materialName)
                    { outStatus = "World material track requires its exact Object material profile: " + value.instanceId; return false; }
                    for (const auto& curve : material.curves)
                    {
                        if (!resource->materialProfile->parameters.contains(curve.parameter))
                        { outStatus = "World material curve parameter is absent from its profile: " + curve.parameter; return false; }
                        for (const auto& key : curve.keys)
                        {
                            auto profile = *resource->materialProfile;
                            profile.parameters[curve.parameter] = key.value;
                            if (!Validate_MaterialProfile(profile))
                            { outStatus = "World material curve key is outside its native profile: " + curve.parameter; return false; }
                        }
                    }
                }
			const bool colliderSlot = std::any_of(targetTemplate->colliderTracks.begin(), targetTemplate->colliderTracks.end(),
				[&](const auto& collider) { return collider.slotId == binding.slotId; });
			if (colliderSlot && binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
			{ outStatus = "Collider tracks require an Object Resource binding: " + value.instanceId + "/" + binding.slotId; return false; }
			const auto animationSlot = std::find_if(
				targetTemplate->animationTracks.begin(),
				targetTemplate->animationTracks.end(),
				[&binding](const WORLD_SEQUENCE_ANIMATION_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
			uint64_t targetId = 0;
			const std::string uniqueTarget =
				std::string(TargetKind_ToString(binding.targetKind)) + ":" +
				binding.targetId;
			const bool_t hasTransformSlot =
				targetTemplate->tracks.end() != transformSlot;
			if (hasTransformSlot && binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
			{
				const auto& scale = transformSlot->keys.front().scaleMultiplier;
				if (scale.x < 0.f || scale.y < 0.f || scale.z < 0.f)
				{ outStatus = "Signed scale requires an Object Resource binding: " + value.instanceId; return false; }
			}
			const bool_t hasAnimationSlot =
				targetTemplate->animationTracks.end() != animationSlot;
			/* A Deploy target may carry a transform track alongside its clip
			   chain so one binding can walk an animated prop while it plays.
			   A map placement has no clips, so an animation slot there is
			   still a mistake. */
			const bool_t bindingShapeIsValid =
				WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT ==
					binding.targetKind ?
				hasAnimationSlot : (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
				(hasTransformSlot || hasAnimationSlot) : (hasTransformSlot && !hasAnimationSlot));
			if (!boundSlots.insert(binding.slotId).second ||
				(binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ?
					!Is_ValidStableId(binding.targetId) : !Parse_Uint64Text(binding.targetId, targetId)) ||
				!boundTargets.insert(uniqueTarget).second ||
				!bindingShapeIsValid)
			{
				outStatus = "Invalid binding in world sequence instance: " +
					value.instanceId;
				return false;
			}
			/* The binding's own kind decides which target table admits it. A
			   Deploy slot that also carries a transform track is still a
			   Deploy binding. */
			if (WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE == binding.targetKind)
			{
				const auto* object = Find_ObjectResource(binding.targetId);
				const bool colliderBone = std::any_of(targetTemplate->colliderTracks.begin(), targetTemplate->colliderTracks.end(),
					[&](const auto& collider) { return collider.slotId == binding.slotId && !collider.attachmentBone.empty(); });
				if (!object || object->modelAssetId.empty() || !object->sequenceInstanceId.empty() ||
					((hasAnimationSlot || colliderBone) && !object->animated) ||
					(colliderSlot && object->anchorKind != "WORLD") ||
					((value.anchorKind == "BOSS" || object->anchorKind == "BOSS") &&
					 (value.anchorKind != "BOSS" || object->anchorKind != "BOSS" || value.bindings.size() != 1u)))
				{ outStatus = "Invalid object resource binding: " + value.instanceId + "/" + binding.slotId; return false; }
				continue;
			}
			if (value.anchorKind != "WORLD" || value.position.x != 0.f || value.position.y != 0.f || value.position.z != 0.f)
			{ outStatus = "Placed sequences cannot use object instance anchors: " + value.instanceId; return false; }
			if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
			{
				const auto deploy = availableDeployPlacements.find(targetId);
				if (availableDeployPlacements.end() == deploy ||
					!deploy->second.animationTargetSupported)
				{
					outStatus = "Invalid animated Deploy binding in world sequence instance: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
				/* Every clip of the chain must exist on the prop, not just the
				   first, so a mistyped later beat fails here instead of part
				   way through the cutscene. */
				for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
					targetTemplate->animationTracks)
				{
					if (track.slotId != binding.slotId)
						continue;
					if (deploy->second.animationClips.end() == std::find(
						deploy->second.animationClips.begin(),
						deploy->second.animationClips.end(), track.clipName))
					{
						outStatus = "Invalid animated Deploy clip in world sequence instance: " +
							value.instanceId + "/" + track.clipName;
						return false;
					}
				}
				continue;
			}
			const auto placement = availablePlacements.find(targetId);
			if (WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind ||
				availablePlacements.end() == placement ||
				!placement->second.sequenceTargetSupported)
			{
				outStatus = "Invalid map binding in world sequence instance: " +
					value.instanceId + "/" + binding.slotId;
				return false;
			}
			const float3_t& baselineScale =
				placement->second.signedScale;
			if (value.walkableSurface && (baselineScale.x <= 0.f || baselineScale.y <= 0.f ||
				std::abs(baselineScale.x - baselineScale.z) > 0.00001f))
			{ outStatus = "Walkable surface placement scale must be positive and uniform in X/Z: " + value.instanceId; return false; }

			for (const WORLD_SEQUENCE_TRANSFORM_KEY& key : transformSlot->keys)
			{
				const double scaleX = static_cast<double>(baselineScale.x) *
					static_cast<double>(key.scaleMultiplier.x);
				const double scaleY = static_cast<double>(baselineScale.y) *
					static_cast<double>(key.scaleMultiplier.y);
				const double scaleZ = static_cast<double>(baselineScale.z) *
					static_cast<double>(key.scaleMultiplier.z);
				const f32_t composedX = static_cast<f32_t>(scaleX);
				const f32_t composedY = static_cast<f32_t>(scaleY);
				const f32_t composedZ = static_cast<f32_t>(scaleZ);
				const double determinant = static_cast<double>(composedX) *
					static_cast<double>(composedY) *
					static_cast<double>(composedZ);
				const f32_t runtimeDeterminant =
					static_cast<f32_t>(determinant);
				if (!std::isfinite(composedX) || !std::isfinite(composedY) ||
					!std::isfinite(composedZ) || !std::isfinite(determinant) ||
					!std::isfinite(runtimeDeterminant) ||
					std::abs(runtimeDeterminant) < MIN_RUNTIME_SCALE_DETERMINANT)
				{
					outStatus = "Sequence scale would create a singular map transform: " +
						value.instanceId + "/" + binding.slotId;
					return false;
				}
			}
		}
	}
	/* Resolve completion links only after every instance and binding is valid.
	   A motion changes the existing object, so it cannot switch resource or slot. */
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		const bool_t next = value.motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT;
		if (std::string_view(MotionEnd_ToString(value.motionEnd)) == "INVALID" ||
			(next ? !Is_ValidStableId(value.nextMotionId) : !value.nextMotionId.empty()) ||
			(value.motionEnd != WORLD_SEQUENCE_MOTION_END::STOP &&
				(value.bindings.size() != 1u || value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)))
		{ outStatus = "Invalid world object motion completion: " + value.instanceId; return false; }
		if (!next) continue;
		const auto* target = Find_Instance(value.nextMotionId);
		if (!target || !target->enabled || target->bindings.size() != 1u ||
			target->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
			target->bindings.front().targetId != value.bindings.front().targetId ||
			target->bindings.front().slotId != value.bindings.front().slotId ||
			Find_Template(value.templateId)->objectMotion.count != 1u ||
			Find_Template(target->templateId)->objectMotion.count != 1u)
		{ outStatus = "NEXT motion must target an enabled single object state with the same resource and slot: " + value.instanceId; return false; }
	}
	for (const WORLD_SEQUENCE_INSTANCE& value : m_Instances)
	{
		std::unordered_set<std::string> visited;
		const WORLD_SEQUENCE_INSTANCE* current = &value;
		uint32_t depth = 0u;
		while (current->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
		{
			if (!visited.insert(current->instanceId).second || ++depth > 32u)
			{ outStatus = "World object NEXT motion chain contains a cycle or exceeds 32 links: " + value.instanceId; return false; }
			current = Find_Instance(current->nextMotionId);
		}
	}
	outStatus = "World sequence document is valid";
	return true;
}

void Client::CWorldSequenceDocument::Reset_Empty(const std::string& areaId)
{
	m_AreaId = areaId;
	m_iRevision = 1;
	m_Templates.clear();
	m_Instances.clear();
	m_ObjectResources.clear();
	m_ObjectFolders.clear();
}

void Client::CWorldSequenceDocument::Touch()
{
	if (m_iRevision < (std::numeric_limits<uint32_t>::max)())
		++m_iRevision;
}

Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(const std::string& sequenceId)
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_TEMPLATE*
Client::CWorldSequenceDocument::Find_Template(
	const std::string& sequenceId) const
{
	const auto found = std::find_if(m_Templates.begin(), m_Templates.end(),
		[&sequenceId](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == sequenceId;
		});
	return m_Templates.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(const std::string& instanceId)
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_INSTANCE*
Client::CWorldSequenceDocument::Find_Instance(
	const std::string& instanceId) const
{
	const auto found = std::find_if(m_Instances.begin(), m_Instances.end(),
		[&instanceId](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	return m_Instances.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_OBJECT_FOLDER* Client::CWorldSequenceDocument::Find_ObjectFolder(const std::string& folderId)
{
    const auto found = std::find_if(m_ObjectFolders.begin(), m_ObjectFolders.end(),
        [&folderId](const auto& value) { return value.folderId == folderId; });
    return found == m_ObjectFolders.end() ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_OBJECT_FOLDER* Client::CWorldSequenceDocument::Find_ObjectFolder(const std::string& folderId) const
{
    const auto found = std::find_if(m_ObjectFolders.begin(), m_ObjectFolders.end(),
        [&folderId](const auto& value) { return value.folderId == folderId; });
    return found == m_ObjectFolders.end() ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId)
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_OBJECT_RESOURCE* Client::CWorldSequenceDocument::Find_ObjectResource(const std::string& objectId) const
{
	const auto found = std::find_if(m_ObjectResources.begin(), m_ObjectResources.end(),
		[&objectId](const auto& value) { return value.objectId == objectId; });
	return found == m_ObjectResources.end() ? nullptr : &*found;
}

bool_t Client::CWorldSequenceDocument::Is_Equivalent(
	const CWorldSequenceDocument& other) const
{
	const auto sameFloat = [](const f32_t left, const f32_t right)
	{
		return left == right;
	};
	const auto sameFloat3 = [&sameFloat](
		const float3_t& left, const float3_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z);
	};
	const auto sameFloat4 = [&sameFloat](
		const float4_t& left, const float4_t& right)
	{
		return sameFloat(left.x, right.x) && sameFloat(left.y, right.y) &&
			sameFloat(left.z, right.z) && sameFloat(left.w, right.w);
	};
	if (m_AreaId != other.m_AreaId || m_iRevision != other.m_iRevision ||
		m_Templates.size() != other.m_Templates.size() ||
		m_Instances.size() != other.m_Instances.size() ||
		m_ObjectResources.size() != other.m_ObjectResources.size() ||
        m_ObjectFolders != other.m_ObjectFolders)
	{
		return false;
	}
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& left = m_ObjectResources[index];
		const auto& right = other.m_ObjectResources[index];
		if (left.objectId != right.objectId || left.displayName != right.displayName ||
            left.parentId != right.parentId ||
			left.anchorKind != right.anchorKind || left.anchorBossArchetypeId != right.anchorBossArchetypeId ||
			left.anchorBone != right.anchorBone ||
			left.modelAssetId != right.modelAssetId || left.diffuseTextureAssetId != right.diffuseTextureAssetId ||
            left.materialProfile != right.materialProfile ||
            left.materialSourceModelAssetId != right.materialSourceModelAssetId || left.mapMaterialBindings != right.mapMaterialBindings ||
			left.animationSetAssetId != right.animationSetAssetId ||
			left.presentationBossArchetypeId != right.presentationBossArchetypeId ||
			left.modelPreScale != right.modelPreScale || left.animated != right.animated ||
			!sameFloat3(left.scale, right.scale) || left.sequenceInstanceId != right.sequenceInstanceId ||
			left.motionInstanceIds != right.motionInstanceIds ||
			left.defaultMotionInstanceId != right.defaultMotionInstanceId) return false;
		if (left.combatBody.has_value() != right.combatBody.has_value() || (left.combatBody &&
			(left.combatBody->maxHp != right.combatBody->maxHp || left.combatBody->shape != right.combatBody->shape || left.combatBody->lifetimePolicy != right.combatBody->lifetimePolicy ||
			!sameFloat3(left.combatBody->localCenterM, right.combatBody->localCenterM) ||
			!sameFloat3(left.combatBody->halfExtentsM, right.combatBody->halfExtentsM)))) return false;
	}
	for (size_t templateIndex = 0u; templateIndex < m_Templates.size();
		++templateIndex)
	{
		const WORLD_SEQUENCE_TEMPLATE& left = m_Templates[templateIndex];
		const WORLD_SEQUENCE_TEMPLATE& right = other.m_Templates[templateIndex];
		if (left.sequenceId != right.sequenceId ||
			left.displayName != right.displayName ||
			left.category != right.category || left.durationMs != right.durationMs ||
			left.interpolation != right.interpolation ||
			left.tracks.size() != right.tracks.size() ||
			left.animationTracks.size() != right.animationTracks.size() ||
			left.effectTracks.size() != right.effectTracks.size() ||
			left.colliderTracks.size() != right.colliderTracks.size() ||
            left.soundTracks != right.soundTracks || left.subtitleTracks != right.subtitleTracks ||
            left.materialTracks != right.materialTracks ||
			!sameFloat3(left.objectMotion.velocity, right.objectMotion.velocity) ||
			!sameFloat3(left.objectMotion.acceleration, right.objectMotion.acceleration) ||
			!sameFloat3(left.objectMotion.angularVelocityDegrees, right.objectMotion.angularVelocityDegrees) ||
			!sameFloat3(left.objectMotion.revolutionDegreesPerSecond, right.objectMotion.revolutionDegreesPerSecond) ||
			!sameFloat3(left.objectMotion.revolutionOffset, right.objectMotion.revolutionOffset) ||
			!sameFloat3(left.objectMotion.spawnHalfExtents, right.objectMotion.spawnHalfExtents) ||
			left.objectMotion.count != right.objectMotion.count || left.objectMotion.intervalMs != right.objectMotion.intervalMs ||
			left.objectMotion.spreadDegrees != right.objectMotion.spreadDegrees || left.objectMotion.seed != right.objectMotion.seed ||
			left.objectMotion.emissions.size() != right.objectMotion.emissions.size())
		{
			return false;
		}
		for (size_t index = 0; index < left.objectMotion.emissions.size(); ++index)
		{
			const auto& a = left.objectMotion.emissions[index]; const auto& b = right.objectMotion.emissions[index];
			if (!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat(a.yawDegrees, b.yawDegrees) ||
				a.startDelayMs != b.startDelayMs) return false;
		}
		for (size_t index = 0; index < left.effectTracks.size(); ++index)
		{
			const auto& a = left.effectTracks[index]; const auto& b = right.effectTracks[index];
			if (a.effectTrackId != b.effectTrackId || a.slotId != b.slotId || a.resourceKind != b.resourceKind ||
				a.resourceId != b.resourceId || a.fitEffectToDuration != b.fitEffectToDuration || a.loopEffectToDuration != b.loopEffectToDuration || a.followObject != b.followObject || a.inheritObjectRotation != b.inheritObjectRotation || a.bone != b.bone || a.timing != b.timing || a.startMs != b.startMs || a.durationMs != b.durationMs ||
				!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat3(a.rotationDegrees, b.rotationDegrees) ||
				!sameFloat3(a.scale, b.scale)) return false;
		}
		for (size_t index = 0; index < left.colliderTracks.size(); ++index)
		{
			const auto& a = left.colliderTracks[index]; const auto& b = right.colliderTracks[index];
			if (a.colliderTrackId != b.colliderTrackId || a.slotId != b.slotId || a.startMs != b.startMs ||
				a.durationMs != b.durationMs || !sameFloat3(a.positionOffset, b.positionOffset) ||
				!sameFloat3(a.halfExtents, b.halfExtents) || !sameFloat(a.yawDegrees, b.yawDegrees) ||
				a.shape != b.shape || a.behavior != b.behavior || !sameFloat(a.damagePercent, b.damagePercent) ||
				!sameFloat3(a.gripLocalOffset, b.gripLocalOffset) || a.attachmentBone != b.attachmentBone) return false;
		}
		for (size_t trackIndex = 0u; trackIndex < left.tracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_TRACK& leftTrack = left.tracks[trackIndex];
			const WORLD_SEQUENCE_TRACK& rightTrack = right.tracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.keys.size() != rightTrack.keys.size())
			{
				return false;
			}
			for (size_t keyIndex = 0u; keyIndex < leftTrack.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& leftKey = leftTrack.keys[keyIndex];
				const WORLD_SEQUENCE_TRANSFORM_KEY& rightKey = rightTrack.keys[keyIndex];
				if (leftKey.timeMs != rightKey.timeMs ||
					!sameFloat3(leftKey.positionOffset, rightKey.positionOffset) ||
					!sameFloat4(leftKey.rotationQuaternion,
						rightKey.rotationQuaternion) ||
					!sameFloat3(leftKey.scaleMultiplier,
						rightKey.scaleMultiplier) ||
					leftKey.visible != rightKey.visible)
				{
					return false;
				}
			}
		}
		for (size_t trackIndex = 0u;
			trackIndex < left.animationTracks.size(); ++trackIndex)
		{
			const WORLD_SEQUENCE_ANIMATION_TRACK& leftTrack =
				left.animationTracks[trackIndex];
			const WORLD_SEQUENCE_ANIMATION_TRACK& rightTrack =
				right.animationTracks[trackIndex];
			if (leftTrack.slotId != rightTrack.slotId ||
				leftTrack.startMs != rightTrack.startMs ||
				leftTrack.sourceStartMs != rightTrack.sourceStartMs ||
				leftTrack.sourceEndMs != rightTrack.sourceEndMs ||
				leftTrack.clipName != rightTrack.clipName ||
				leftTrack.displayName != rightTrack.displayName ||
				!sameFloat(leftTrack.playbackRate, rightTrack.playbackRate) ||
				leftTrack.loop != rightTrack.loop ||
				leftTrack.holdLastFrame != rightTrack.holdLastFrame)
			{
				return false;
			}
		}
	}
	for (size_t instanceIndex = 0u; instanceIndex < m_Instances.size();
		++instanceIndex)
	{
		const WORLD_SEQUENCE_INSTANCE& left = m_Instances[instanceIndex];
		const WORLD_SEQUENCE_INSTANCE& right = other.m_Instances[instanceIndex];
		if (left.instanceId != right.instanceId ||
			left.templateId != right.templateId || left.enabled != right.enabled ||
			left.startDelayMs != right.startDelayMs ||
			!sameFloat(left.playbackSpeed, right.playbackSpeed) ||
			left.bindings.size() != right.bindings.size() || left.anchorKind != right.anchorKind ||
			!sameFloat3(left.position, right.position) ||
			left.motionEnd != right.motionEnd || left.loopFullPresentation != right.loopFullPresentation || left.nextMotionId != right.nextMotionId)
		{
			return false;
		}
		if (left.walkableSurface.has_value() != right.walkableSurface.has_value() ||
			(left.walkableSurface && (!sameFloat(left.walkableSurface->radiusM, right.walkableSurface->radiusM) ||
				!sameFloat(left.walkableSurface->localHeightM, right.walkableSurface->localHeightM)))) return false;
		for (size_t bindingIndex = 0u; bindingIndex < left.bindings.size();
			++bindingIndex)
		{
			if (left.bindings[bindingIndex].slotId !=
				right.bindings[bindingIndex].slotId ||
				left.bindings[bindingIndex].targetKind !=
					right.bindings[bindingIndex].targetKind ||
				left.bindings[bindingIndex].targetId !=
					right.bindings[bindingIndex].targetId ||
				left.bindings[bindingIndex].previewNpcPlacementId !=
					right.bindings[bindingIndex].previewNpcPlacementId)
			{
				return false;
			}
		}
	}
	return true;
}

bool_t Client::CWorldSequenceDocument::Try_EffectTimeScale(
    const WORLD_SEQUENCE_EFFECT_TRACK& effect, const f32_t sourceDurationSeconds, f32_t& outScale)
{
    if (!effect.fitEffectToDuration) { outScale = 1.f; return true; }
    if (effect.resourceKind != "V1_EFFECT" || !effect.durationMs ||
        !std::isfinite(sourceDurationSeconds) || sourceDurationSeconds <= 0.f) return false;
    const double rate = double(sourceDurationSeconds) * 1000. / effect.durationMs;
    if (!std::isfinite(rate) || rate <= 0. || rate > (std::numeric_limits<f32_t>::max)()) return false;
    const f32_t scale = static_cast<f32_t>(rate);
    if (!std::isfinite(scale) || scale <= 0.f) return false;
    outScale = scale;
    return true;
}

bool_t Client::CWorldSequenceDocument::Resize_TimelineDuration(const std::string& sequenceId,
    const uint32_t durationMs, const uint32_t requiredAnimationEndMs,
    const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, std::string& outStatus)
{
    const auto reject = [&](const char* reason) { outStatus = reason; return false; };
    auto* current = Find_Template(sequenceId);
    if (!current || !durationMs || durationMs > MAX_DURATION_MS || durationMs < requiredAnimationEndMs)
        return reject("Stage duration would cut an Animation or exceed the timeline limit. Existing rows preserved.");
    if (durationMs == current->durationMs) return true;
    auto candidate = *this;
    auto* edited = candidate.Find_Template(sequenceId);
    const auto samePose = [](const auto& a, const auto& b) {
        return a.positionOffset.x == b.positionOffset.x && a.positionOffset.y == b.positionOffset.y && a.positionOffset.z == b.positionOffset.z &&
            a.rotationQuaternion.x == b.rotationQuaternion.x && a.rotationQuaternion.y == b.rotationQuaternion.y &&
            a.rotationQuaternion.z == b.rotationQuaternion.z && a.rotationQuaternion.w == b.rotationQuaternion.w &&
            a.scaleMultiplier.x == b.scaleMultiplier.x && a.scaleMultiplier.y == b.scaleMultiplier.y && a.scaleMultiplier.z == b.scaleMultiplier.z &&
            a.visible == b.visible;
    };
    for (auto& track : edited->tracks)
    {
        if (track.keys.size() < 2u) return reject("Stage requires valid Transform endpoints.");
        if (durationMs > edited->durationMs)
        {
            if (track.keys.size() >= MAX_KEY_COUNT) return reject("Stage extension exceeds the Transform key limit.");
            auto endpoint = track.keys.back(); endpoint.timeMs = durationMs;
            track.keys.push_back(endpoint); // Never retime an authored key.
        }
        else
        {
            // Only trim a constant held tail. A changed pose/visibility is authored content.
            while (track.keys.size() > 1u && track.keys.back().timeMs > durationMs)
            {
                if (!samePose(track.keys.back(), track.keys[track.keys.size() - 2u]))
                    return reject("Stage shortening would cut a Transform or visibility change. Existing rows preserved.");
                auto endpoint = track.keys.back(); track.keys.pop_back();
                if (track.keys.back().timeMs < durationMs)
                { endpoint.timeMs = durationMs; track.keys.push_back(endpoint); break; }
            }
            if (track.keys.size() < 2u) return reject("Stage shortening would remove a required Transform endpoint.");
        }
    }
    for (auto& material : edited->materialTracks)
        for (auto& curve : material.curves)
        {
            if (curve.keys.size() < 2u) return reject("Stage requires valid material endpoints.");
            if (durationMs > edited->durationMs)
            {
                if (curve.keys.size() >= MAX_KEY_COUNT) return reject("Stage extension exceeds the material key limit.");
                auto endpoint = curve.keys.back(); endpoint.timeMs = durationMs;
                curve.keys.push_back(endpoint);
            }
            else
            {
                while (curve.keys.size() > 1u && curve.keys.back().timeMs > durationMs)
                {
                    if (curve.keys.back().value != curve.keys[curve.keys.size() - 2u].value)
                        return reject("Stage shortening would cut a material change. Existing rows preserved.");
                    auto endpoint = curve.keys.back(); curve.keys.pop_back();
                    if (curve.keys.back().timeMs < durationMs)
                    { endpoint.timeMs = durationMs; curve.keys.push_back(endpoint); break; }
                }
                if (curve.keys.size() < 2u) return reject("Stage shortening would remove a required material endpoint.");
            }
        }
    bool pinnedMotionEnd = false;
    for (auto& effect : edited->effectTracks)
        if (effect.timing == "MOTION_END")
        { effect.timing = "TIME"; effect.startMs = edited->durationMs; pinnedMotionEnd = true; }
    edited->durationMs = durationMs;
    // Validation rejects clipped starts, Collider windows and emission limits, without moving them.
    if (!candidate.Validate(mapPlacements, deployPlacements, outStatus))
    { outStatus = "Stage resize refused: " + outStatus + ". Existing rows preserved."; return false; }
    *current = std::move(*edited); // Keep UI references to this template valid.
    outStatus = "Stage duration updated; Animation, Effect and Collider timings are preserved.";
    if (pinnedMotionEnd) outStatus += " Motion End Effects now use At Time to keep their previous start.";
    return true;
}

bool_t Client::CWorldSequenceDocument::Try_SampleAnimationTicks(
    const WORLD_SEQUENCE_ANIMATION_TRACK& track, const f32_t localMs, const f32_t windowEndMs,
    const f32_t ticksPerSecond, const f32_t durationTicks, f32_t& outTicks)
{
    if (!std::isfinite(localMs) || !std::isfinite(windowEndMs) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f ||
        !std::isfinite(durationTicks) || durationTicks <= 0.f ||
        !std::isfinite(track.playbackRate) || track.playbackRate <= 0.f)
        return false;
    const f32_t sourceTicks = static_cast<f32_t>(static_cast<double>(track.sourceStartMs) *
        .001 * static_cast<double>(ticksPerSecond));
    // Authored milliseconds may round a native float duration by less than one ms.
    const f32_t authoredEnd = track.sourceEndMs == 0u ? durationTicks :
        static_cast<f32_t>(static_cast<double>(track.sourceEndMs) * .001 * static_cast<double>(ticksPerSecond));
    if (!std::isfinite(sourceTicks) || sourceTicks > durationTicks || !std::isfinite(authoredEnd) ||
        (track.sourceEndMs != 0u && (track.sourceEndMs <= track.sourceStartMs ||
            static_cast<double>(track.sourceEndMs) > static_cast<double>(durationTicks) * 1000.0 / ticksPerSecond + 1.0))) return false;
    const f32_t endTicks = (std::min)(authoredEnd, durationTicks);
    if ((track.loop && sourceTicks >= endTicks) || sourceTicks > endTicks) return false;
    const f32_t elapsedTicks = (std::max)(0.f, localMs - track.startMs) * .001f *
        track.playbackRate * ticksPerSecond;
    if (!std::isfinite(elapsedTicks)) return false;
    f32_t ticks = sourceTicks + elapsedTicks;
    // Hold wins at the timeline end, including looped clips, as before.
    if (localMs >= windowEndMs && track.holdLastFrame) ticks = endTicks;
    else if (track.loop) ticks = sourceTicks + std::fmod(elapsedTicks, endTicks - sourceTicks);
    else if (ticks > endTicks) ticks = track.holdLastFrame ? endTicks : sourceTicks;
    if (!std::isfinite(ticks)) return false;
    outTicks = ticks;
    return true;
}

const char_t* Client::CWorldSequenceDocument::Interpolation_ToString(
	const WORLD_SEQUENCE_INTERPOLATION interpolation)
{
	switch (interpolation)
	{
	case WORLD_SEQUENCE_INTERPOLATION::LINEAR:
		return "LINEAR";
	case WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP:
		return "SMOOTH_STEP";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseInterpolation(
	const std::string& value,
	WORLD_SEQUENCE_INTERPOLATION& outInterpolation)
{
	if ("LINEAR" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR;
	else if ("SMOOTH_STEP" == value)
		outInterpolation = WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::TargetKind_ToString(
	const WORLD_SEQUENCE_TARGET_KIND targetKind)
{
	switch (targetKind)
	{
	case WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT:
		return "MAP_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT:
		return "DEPLOY_PLACEMENT";
	case WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE:
		return "OBJECT_RESOURCE";
	default:
		return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseTargetKind(
	const std::string& value,
	WORLD_SEQUENCE_TARGET_KIND& outTargetKind)
{
	if ("MAP_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
	else if ("DEPLOY_PLACEMENT" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT;
	else if ("OBJECT_RESOURCE" == value)
		outTargetKind = WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
	else
		return false;
	return true;
}

const char_t* Client::CWorldSequenceDocument::MotionEnd_ToString(
	const WORLD_SEQUENCE_MOTION_END motionEnd)
{
	switch (motionEnd)
	{
	case WORLD_SEQUENCE_MOTION_END::STOP: return "STOP";
	case WORLD_SEQUENCE_MOTION_END::HOLD: return "HOLD";
	case WORLD_SEQUENCE_MOTION_END::LOOP: return "LOOP";
	case WORLD_SEQUENCE_MOTION_END::NEXT: return "NEXT";
	default: return "INVALID";
	}
}

bool_t Client::CWorldSequenceDocument::Try_ParseMotionEnd(
	const std::string& value, WORLD_SEQUENCE_MOTION_END& outMotionEnd)
{
	if (value == "STOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::STOP;
	else if (value == "HOLD") outMotionEnd = WORLD_SEQUENCE_MOTION_END::HOLD;
	else if (value == "LOOP") outMotionEnd = WORLD_SEQUENCE_MOTION_END::LOOP;
	else if (value == "NEXT") outMotionEnd = WORLD_SEQUENCE_MOTION_END::NEXT;
	else return false;
	return true;
}

bool_t Client::CWorldSequenceDocument::Is_ValidStableId(
	const std::string& value)
{
	return !value.empty() && value.size() <= 128u &&
		std::all_of(value.begin(), value.end(), [](const unsigned char character)
		{
			return 0 != std::isalnum(character) || character == '_' ||
				character == '-' || character == '.';
		});
}

bool_t Client::CWorldSequenceDocument::Try_SampleMaterialParameters(
    const WORLD_SEQUENCE_MATERIAL_PROFILE& profile, const WORLD_SEQUENCE_MATERIAL_TRACK& track,
    const f32_t timeMs, Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& out)
{
    if (!std::isfinite(timeMs) || track.materialName != profile.materialName) return false;
    auto values = profile.parameters;
    for (const auto& curve : track.curves)
    {
        if (curve.keys.empty() || !values.contains(curve.parameter)) return false;
        auto right = std::upper_bound(curve.keys.begin(), curve.keys.end(), timeMs,
            [](float time, const auto& key) { return time < key.timeMs; });
        const auto left = right == curve.keys.begin() ? right : right - 1;
        const float fraction = right == curve.keys.end() || left == right || left->constant ? 0.f :
            (timeMs - left->timeMs) / (right->timeMs - left->timeMs);
        auto& value = values.at(curve.parameter);
        for (size_t axis = 0u; axis < 4u; ++axis)
            value[axis] = left->value[axis] + (fraction ? (right->value[axis] - left->value[axis]) * fraction : 0.f);
    }
    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS staged;
    if (!SourceCharacterMaterial::Configure(profile.family, values, staged)) return false;
    out = std::move(staged);
    return true;
}

bool_t Client::CWorldSequenceDocument::Is_ValidMaterialProfile(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile)
{
    return Validate_MaterialProfile(profile);
}

bool_t Client::CWorldSequenceDocument::Build_MaterialOverride(const WORLD_SEQUENCE_MATERIAL_PROFILE& profile,
    const std::filesystem::path& resourceRoot, Engine::MODEL_MATERIAL_OVERRIDE& out)
{
    Engine::MODEL_MATERIAL_OVERRIDE staged;
    if (!resourceRoot.is_absolute() || !Validate_MaterialProfile(profile, &staged.surface.sourceCharacter)) return false;
    staged.materialName = profile.materialName;
    staged.surface.family = Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
    for (const auto& texture : profile.textures)
    {
        auto& input = staged.sourceCharacterTextures[texture.expressionIndex];
        input.path = (resourceRoot / texture.assetId).lexically_normal();
        input.srgb = texture.srgb;
    }
    out = std::move(staged);
    return true;
}

bool_t CWorldSequenceDocument::Duplicate_TimelineBox(const std::string& sequenceId,
    const bool animation, const size_t index, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus)
{
    auto* current = Find_Template(sequenceId);
    if (!current) { outStatus = "Motion is unavailable: " + sequenceId; return false; }
    auto& sequence = *current;
    if ((animation && index >= sequence.animationTracks.size()) ||
        (!animation && index >= sequence.effectTracks.size())) return false;
    auto candidate = *this;
    auto* staged = candidate.Find_Template(sequence.sequenceId);
    if (!staged) return false;
    if (staged->tracks.size() + staged->animationTracks.size() + staged->effectTracks.size() + staged->colliderTracks.size() + staged->materialTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { outStatus = "Duplicate refused: Motion track limit reached. Existing draft preserved."; return false; }
    const uint32_t oldDuration = staged->durationMs;
    uint32_t duration = oldDuration;
    size_t selected = 0u;
    if (animation)
    {
        auto duplicate = staged->animationTracks[index];
        uint32_t end = oldDuration;
        for (const auto& next : staged->animationTracks)
            if (next.slotId == duplicate.slotId && next.startMs > duplicate.startMs) end = (std::min)(end, next.startMs);
        const uint32_t span = end - duplicate.startMs;
        if (oldDuration > CWorldSequenceDocument::MAX_DURATION_MS - span)
        { outStatus = "Duplicate refused: Animation exceeds the 600-second Motion limit."; return false; }
        for (auto& next : staged->animationTracks)
            if (next.slotId == duplicate.slotId && next.startMs >= end) next.startMs += span;
        duplicate.startMs = end;
        staged->animationTracks.insert(staged->animationTracks.begin() + index + 1u, duplicate);
        selected = index + 1u;
        duration += span;
    }
    else
    {
        auto duplicate = staged->effectTracks[index];
        const uint64_t start = uint64_t(staged->EffectStartMs(duplicate)) + duplicate.durationMs;
        if (start + duplicate.durationMs > CWorldSequenceDocument::MAX_DURATION_MS)
        { outStatus = "Duplicate refused: Effect exceeds the 600-second presentation limit."; return false; }
        uint32_t serial = 1u;
        do { duplicate.effectTrackId = "effect." + std::to_string(serial++); }
        while (std::any_of(staged->effectTracks.begin(), staged->effectTracks.end(),
            [&](const auto& row) { return row.effectTrackId == duplicate.effectTrackId; }));
        duplicate.timing = "TIME";
        duplicate.startMs = static_cast<uint32_t>(start);
        duration = (std::max)(duration, duplicate.startMs);
        staged->effectTracks.insert(staged->effectTracks.begin() + index + 1u, duplicate);
        selected = index + 1u;
    }
    if (duration > oldDuration)
    {
        for (auto& track : staged->tracks)
        {
            if (track.keys.empty() || track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { outStatus = "Duplicate refused: Motion endpoint cannot be extended. Existing draft preserved."; return false; }
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
        staged->durationMs = duration;
    }
    std::string status;
    if (!candidate.Validate(mapPlacements, deployPlacements, status))
    { outStatus = "Duplicate refused: " + status + ". Existing draft preserved."; return false; }
    // Preserve references held by the open Detail/Sequencer pane.
    sequence = std::move(*staged);
    outIndex = selected;
    outStatus = "Duplicated the selected box after its window.";
    return true;
}

bool_t CWorldSequenceDocument::Duplicate_ColliderTrack(const std::string& sequenceId,
    const size_t index, const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements,
    const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements, size_t& outIndex, std::string& outStatus)
{
    auto* current = Find_Template(sequenceId);
    if (!current || index >= current->colliderTracks.size())
    { outStatus = "Collider row is unavailable: " + sequenceId; return false; }
    auto candidate = *this;
    auto* staged = candidate.Find_Template(sequenceId);
    if (staged->tracks.size() + staged->animationTracks.size() + staged->effectTracks.size() + staged->colliderTracks.size() + staged->materialTracks.size() >= MAX_TRACK_COUNT)
    { outStatus = "Duplicate refused: Motion track limit reached. Existing draft preserved."; return false; }
    auto duplicate = staged->colliderTracks[index];
    uint32_t serial = 1u;
    do { duplicate.colliderTrackId = "collider." + std::to_string(serial++); }
    while (std::any_of(staged->colliderTracks.begin(), staged->colliderTracks.end(),
        [&](const auto& row) { return row.colliderTrackId == duplicate.colliderTrackId; }));
    staged->colliderTracks.insert(staged->colliderTracks.begin() + index + 1u, duplicate);
    std::string status;
    if (!candidate.Validate(mapPlacements, deployPlacements, status))
    { outStatus = "Duplicate refused: " + status + ". Existing draft preserved."; return false; }
    *current = std::move(*staged);
    outIndex = index + 1u;
    outStatus = "Duplicated the collider with its original time window.";
    return true;
}

namespace
{
    bool Validate_ObjectBundle(const WORLD_SEQUENCE_OBJECT_BUNDLE& bundle, std::string& status)
    {
        const auto& resource = bundle.resource;
        if (resource.modelAssetId.empty() || !resource.sequenceInstanceId.empty() || !resource.motionInstanceIds.empty())
        { status = "Copy requires a model Object, not a placed alias or combined Motion group."; return false; }
        CWorldSequenceDocument projection;
        projection.Reset_Empty("clipboard.world.object");
        projection.Get_ObjectResources().push_back(resource);
        projection.Get_ObjectResources().front().parentId.clear();
        projection.Get_Templates() = bundle.templates;
        projection.Get_Instances() = bundle.instances;
        if (!projection.Validate({}, {}, status)) return false;
        std::unordered_set<std::string> rootIds, visited, templateIds;
        std::vector<std::string> pending = bundle.rootMotionIds;
        for (const auto& id : pending)
            if (!rootIds.insert(id).second || !projection.Find_Instance(id))
            { status = "Copied Motion roots must be unique and present in the bundle."; return false; }
        for (size_t index = 0; index < pending.size(); ++index)
        {
            const auto& id = pending[index];
            if (!visited.insert(id).second) continue;
            const auto* instance = projection.Find_Instance(id);
            if (!instance || instance->bindings.size() != 1u ||
                instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
                instance->bindings.front().targetId != resource.objectId || instance->anchorKind != resource.anchorKind)
            { status = "Every copied Motion must bind only the copied Object in its anchor category."; return false; }
            templateIds.insert(instance->templateId);
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
        }
        if (visited.size() != bundle.instances.size() || templateIds.size() != bundle.templates.size())
        { status = "Copied Motion bundle contains unreferenced instances or templates."; return false; }
        return true;
    }

    bool Is_UnboundObjectDraft(const CWorldSequenceDocument& document,
        const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
    {
        // Only the unfinished model selection produced by Create Object is exempt.
        // A malformed assigned model or a referenced resource still gets full validation.
        return resource.modelAssetId.empty() && resource.sequenceInstanceId.empty() &&
            resource.motionInstanceIds.empty() && resource.defaultMotionInstanceId.empty() &&
            resource.animationSetAssetId.empty() && resource.presentationBossArchetypeId.empty() &&
            resource.diffuseTextureAssetId.empty() && !resource.materialProfile && !resource.combatBody &&
            resource.materialSourceModelAssetId.empty() && resource.mapMaterialBindings.empty() && !resource.animated &&
            std::isfinite(resource.modelPreScale) && resource.modelPreScale >= MIN_SCALE && resource.modelPreScale <= MAX_COMPONENT &&
            Is_BoundedFloat3(resource.scale) && resource.scale.x >= MIN_SCALE && resource.scale.y >= MIN_SCALE && resource.scale.z >= MIN_SCALE &&
            std::none_of(document.Get_Instances().begin(), document.Get_Instances().end(), [&](const auto& instance) {
                return std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
                    return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
                });
            });
    }
}

bool_t CWorldSequenceDocument::Capture_ObjectBundle(const std::string& objectId,
    const std::vector<std::string>& selectedMotionIds, WORLD_SEQUENCE_OBJECT_BUNDLE& outBundle,
    std::string& outStatus) const
{
    const auto* resource = Find_ObjectResource(objectId);
    if (!resource || resource->modelAssetId.empty() || !resource->sequenceInstanceId.empty() || !resource->motionInstanceIds.empty())
    { outStatus = "Copy requires a model Object. Placed aliases and combined Motion groups keep their existing bindings."; return false; }
    WORLD_SEQUENCE_OBJECT_BUNDLE staged;
    staged.resource = *resource;
    staged.rootMotionIds = selectedMotionIds;
    if (selectedMotionIds.empty())
        for (const auto& instance : m_Instances)
            if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
                return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == objectId;
            })) staged.rootMotionIds.push_back(instance.instanceId);
    std::unordered_set<std::string> instanceIds, templateIds;
    std::vector<std::string> pending = staged.rootMotionIds;
    for (size_t index = 0; index < pending.size(); ++index)
    {
        const auto id = pending[index];
        if (!instanceIds.insert(id).second) continue;
        const auto* instance = Find_Instance(id);
        const auto* sequence = instance ? Find_Template(instance->templateId) : nullptr;
        if (!instance || !sequence || instance->bindings.size() != 1u ||
            instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
            instance->bindings.front().targetId != objectId || instance->anchorKind != resource->anchorKind)
        { outStatus = "Copy refused: every selected and NEXT Motion must bind this model Object only."; return false; }
        if (instanceIds.size() > MAX_INSTANCE_COUNT)
        { outStatus = "Copy refused: Motion bundle capacity reached."; return false; }
        staged.instances.push_back(*instance);
        if (templateIds.insert(sequence->sequenceId).second) staged.templates.push_back(*sequence);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
    }
    if (!selectedMotionIds.empty() && !instanceIds.contains(staged.resource.defaultMotionInstanceId))
    {
        staged.resource.defaultMotionInstanceId.clear();
        for (const auto& id : staged.rootMotionIds)
            if (const auto* instance = Find_Instance(id); instance && instance->enabled)
            { staged.resource.defaultMotionInstanceId = id; break; }
    }
    if (!Validate_ObjectBundle(staged, outStatus))
    { outStatus = "Copy refused: " + outStatus + " Existing clipboard preserved."; return false; }
    outBundle = std::move(staged);
    outStatus = "Copied the Object resource and " + std::to_string(outBundle.instances.size()) + " independent Motion values.";
    return true;
}

bool_t CWorldSequenceDocument::Paste_ObjectBundle(const WORLD_SEQUENCE_OBJECT_BUNDLE& bundle,
    const std::string& destinationObjectId, const std::string& newObjectName, const std::string& parentId,
    const WORLD_SEQUENCE_PLACEMENT_MAP& mapPlacements, const WORLD_SEQUENCE_DEPLOY_MAP& deployPlacements,
    WORLD_SEQUENCE_PASTE_RESULT& outResult, std::string& outStatus)
{
    if (!Validate_ObjectBundle(bundle, outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    const bool createObject = destinationObjectId.empty();
    const auto* destination = createObject ? nullptr : Find_ObjectResource(destinationObjectId);
    if (!createObject && !destination)
    { outStatus = "Paste refused: destination Object is unavailable. Existing draft preserved."; return false; }
    if (m_Templates.size() + bundle.templates.size() > MAX_TEMPLATE_COUNT ||
        m_Instances.size() + bundle.instances.size() > MAX_INSTANCE_COUNT ||
        m_ObjectResources.size() + (createObject ? 1u : 0u) > MAX_INSTANCE_COUNT)
    { outStatus = "Paste refused: World sequence document capacity reached. Existing draft preserved."; return false; }
    const bool fillDraft = destination && Is_UnboundObjectDraft(*this, *destination);
    if (destination && (destination->anchorKind != bundle.resource.anchorKind ||
        !destination->sequenceInstanceId.empty() || !destination->motionInstanceIds.empty() ||
        (!fillDraft && (destination->modelAssetId != bundle.resource.modelAssetId ||
            destination->animationSetAssetId != bundle.resource.animationSetAssetId ||
            destination->animated != bundle.resource.animated ||
            destination->presentationBossArchetypeId != bundle.resource.presentationBossArchetypeId))))
    { outStatus = "Paste refused: destination requires the same model, animation set and anchor category. Existing draft preserved."; return false; }
    auto candidate = *this;
    WORLD_SEQUENCE_PASTE_RESULT result;
    result.objectId = destinationObjectId;
    if (createObject)
    {
        for (uint32_t serial = 1u; ; ++serial)
        {
            result.objectId = "world.object.copy." + std::to_string(serial);
            if (!candidate.Find_ObjectResource(result.objectId) && !candidate.Find_ObjectFolder(result.objectId)) break;
        }
        auto resource = bundle.resource;
        resource.objectId = result.objectId;
        resource.displayName = newObjectName;
        resource.parentId = parentId;
        candidate.Get_ObjectResources().push_back(std::move(resource));
    }
    else if (fillDraft)
    {
        auto resource = bundle.resource;
        resource.objectId = destination->objectId;
        resource.displayName = destination->displayName;
        resource.parentId = destination->parentId;
        *candidate.Find_ObjectResource(result.objectId) = std::move(resource);
    }
    std::unordered_map<std::string, std::string> templateIds, instanceIds;
    for (const auto& original : bundle.templates)
    {
        auto sequence = original;
        for (uint32_t serial = 1u; ; ++serial)
        {
            sequence.sequenceId = "world.object.motion.copy." + std::to_string(serial);
            if (!candidate.Find_Template(sequence.sequenceId)) break;
        }
        templateIds.emplace(original.sequenceId, sequence.sequenceId);
        candidate.Get_Templates().push_back(std::move(sequence));
    }
    for (const auto& original : bundle.instances)
    {
        auto instance = original;
        for (uint32_t serial = 1u; ; ++serial)
        {
            instance.instanceId = "world.object.instance.copy." + std::to_string(serial);
            if (!candidate.Find_Instance(instance.instanceId)) break;
        }
        instance.templateId = templateIds.at(original.templateId);
        instance.bindings.front().targetId = result.objectId;
        instanceIds.emplace(original.instanceId, instance.instanceId);
        result.instanceIds.push_back(instance.instanceId);
        candidate.Get_Instances().push_back(std::move(instance));
    }
    for (const auto& id : result.instanceIds)
    {
        auto* instance = candidate.Find_Instance(id);
        if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            instance->nextMotionId = instanceIds.at(instance->nextMotionId);
    }
    for (const auto& id : bundle.rootMotionIds) result.rootMotionIds.push_back(instanceIds.at(id));
    auto* resource = candidate.Find_ObjectResource(result.objectId);
    if (createObject || fillDraft || resource->defaultMotionInstanceId.empty())
        resource->defaultMotionInstanceId = bundle.resource.defaultMotionInstanceId.empty() ? std::string{} :
            instanceIds.at(bundle.resource.defaultMotionInstanceId);
    if (!candidate.Validate_ObjectHierarchy(outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    // Other Create Object drafts remain visible and unchanged. Remove only those
    // unbound placeholders from a validation copy; Save still validates everything.
    auto validation = candidate;
    auto& resources = validation.Get_ObjectResources();
    resources.erase(std::remove_if(resources.begin(), resources.end(), [&](const auto& row) {
        return row.objectId != result.objectId && Is_UnboundObjectDraft(candidate, row);
    }), resources.end());
    validation.Get_ObjectFolders().clear();
    for (auto& row : resources) row.parentId.clear();
    if (!validation.Validate(mapPlacements, deployPlacements, outStatus))
    { outStatus = "Paste refused: " + outStatus + " Existing draft preserved."; return false; }
    *this = std::move(candidate);
    outResult = std::move(result);
    outStatus = "Pasted an independent Object/Motion copy. Save to keep the resource.";
    return true;
}
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Public/WorldSequencePlayer.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeployPropRuntime.h"
#include "MapAssetCatalog.h"
#include "MapPlacementRuntime.h"
#include "WorldSequenceDocument.h"

#include <string>
#include <cmath>
#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CWorldSequenceObject;
class CNpc;
class EFFECT_V2_CATALOG_SNAPSHOT;
struct EFFECT_DOCUMENT_DESC;
struct SAYDON_WEAPON_REPLACEMENT;
struct SAYDON_HAT_REPLACEMENT;

struct WORLD_SEQUENCE_SUBTITLE_SAMPLE final
{
    std::string instanceId, subtitleTrackId, text, position;
    float3_t worldPosition{};
};

/* One playback path for authored world sequences. The Map Tool preview and the
   product level both evaluate a sequence here so a sequence can never look one
   way in the editor and another way in the game. The player only reads the
   document; starting and stopping stay with the caller that owns the gameplay
   reason for playing. */
class CWorldSequencePlayer final
{
public:

	struct OBJECT_PLACEMENT final
	{
		float3_t position{};
		float3_t rotationDegrees{};
		float3_t scale{1.f, 1.f, 1.f};
		bool operator==(const OBJECT_PLACEMENT& other) const
		{
			return position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
				rotationDegrees.x == other.rotationDegrees.x && rotationDegrees.y == other.rotationDegrees.y && rotationDegrees.z == other.rotationDegrees.z &&
				scale.x == other.scale.x && scale.y == other.scale.y && scale.z == other.scale.z;
		}
	};

	struct PLAYER_ANCHOR
	{
		uint64_t entityId = 0;
		float4x4_t world{};
		bool_t emissionOverride = false;
		bool_t liveBossAnchor = false;
		std::shared_ptr<Engine::CModel> bodyModel;
	};
	struct TARGET_SET final
	{
		uint32_t levelIndex = {};
		const CMapAssetCatalog* pCatalog = nullptr;
		std::vector<MAP_RUNTIME_PLACED_ENTRY>* pPlacements = nullptr;
		CDeployPropRuntime* pDeployRuntime = nullptr;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		// Level-owned preparation, borrowed only during the call. Live clones retain
		// a separate return token so level teardown never dereferences this owner.
		CWorldSequencePlayer* objectPreparationOwner = nullptr;
		// MapTool samples after MainApp's normal post-update Effect commit. It may
		// commit only the newly-created world roots before seeking that editor frame.
		bool_t bCommitWorldRootEffectsAfterSpawn = false;
		std::function<std::vector<PLAYER_ANCHOR>()> playerAnchors;
		// Supplied only by a live editor owner. Missing NPCs reject the preview.
		std::function<std::shared_ptr<CNpc>(const std::string&)> previewNpc;
		// Live BODY bone pose; separate from a frozen projectile emission origin.
		std::function<bool_t(const std::string&, const std::string&, PLAYER_ANCHOR&, std::string&)> bossAnchor;
		// Occurrence-local real milliseconds at birth -> frozen world origin.
		std::function<bool_t(f32_t, float4x4_t&)> objectEmissionAnchor;
		// Optional presentation-only post transform, sampled at the Object's source clock.
		// Effect frames use the current post transform without rebasing their birth history.
		std::function<bool_t(const std::string&, f32_t, float4x4_t&, std::string&)> objectWorldPostTransform;

		bool_t Is_Complete() const noexcept
		{
			return nullptr != pCatalog && nullptr != pPlacements &&
				nullptr != pDeployRuntime;
		}
	};

	/* Baseline transforms are captured when an instance starts so a sequence
	   composes against the placed pose instead of accumulating drift. */
	struct PLACEMENT_BASELINE final
	{
		uint64_t placementId = {};
		MAP_PLACEMENT_RECORD record;
		bool_t runtimeVisible = false;
		bool_t restoreRuntimeVisible = false;
	};

	CWorldSequencePlayer() = default;
	~CWorldSequencePlayer();
	CWorldSequencePlayer(const CWorldSequencePlayer&) = delete;
	CWorldSequencePlayer& operator=(const CWorldSequencePlayer&) = delete;

	/* Reads the published runtime document beside the executable. The live
	   targets are required because the document is admitted against the
	   placements and Deploy props this level actually created. */
	bool_t Load_Area(const std::string& areaId, const TARGET_SET& targets);
	// Loader-only: parse/validate against its admitted map/Deploy prototypes.
	// One bounded pending Area is replaced on the next preparation and consumed
	// once by activation. Cancellation publishes no usable stage.
	static bool_t Prepare_AreaLoad(uint32_t levelIndex, const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope, std::string& status,
		const std::function<bool_t()>& isCancellationRequested = nullptr);
	// Product activation never falls back to synchronous file parsing. Failed or
	// missing preparation preserves this player's document and reports its reason.
	bool_t Load_PreparedArea(const std::string& areaId, const TARGET_SET& targets);
	static bool_t Try_CollectPreparedAreaV1EffectTargets(uint32_t levelIndex,
		const std::string& areaId, std::vector<std::string>& outTargets);
	// CPU snapshot admitted by the Loader; lookup performs no IO or GPU work.
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> Find_PreparedLeafSnapshot(const std::string& leafId) const;
	bool_t Set_Document(const CWorldSequenceDocument& document, const TARGET_SET& targets, std::string& status);
	// Editor draft preview: same validation and stop as Set_Document, but a
	// prepared model whose resource still asks for the same inputs is kept, so
	// a key or clip edit does not reload a 40 MB body and its animation set.
	bool_t Replace_DocumentKeepingModels(const CWorldSequenceDocument& document,
		const TARGET_SET& targets, std::string& status);
	// A single request may stage several independent clocks against one document.
	// Validate/read targets once; prepare every copy before replacing any player.
	static bool_t Set_DocumentBatch(const CWorldSequenceDocument& document, const TARGET_SET& targets,
		const std::vector<CWorldSequencePlayer*>& players, std::string& status);
	bool_t Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets);
	// Prepare hidden clones through the existing Prototype/Clone/Layer path.
	// Stop/completion returns them for later occurrences; failure preserves the pool.
	bool_t Prewarm_ObjectInstances(const std::string& instanceId, uint32_t copies, const TARGET_SET& targets);
	bool_t Prewarm_HiddenObjectPose(const std::string& instanceId, f32_t elapsedMs, const TARGET_SET& targets);
	static bool_t Resolve_BossBoneAnchor(const std::shared_ptr<Engine::CModel>& model,
		const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status);
	static void Collect_ValidationTargets(const TARGET_SET& targets,
		WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy);
	bool_t Has_ActiveInstances() const { return !m_Active.empty(); }
    // Presentation controllers pack source-native parameters; only the named
    // Object clone receives the constants. Shared prototypes remain immutable.
    bool_t Set_ObjectMaterialConstants(const std::string& instanceId,
        const std::string& slotId, const std::string& materialName,
        const Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
    // Append read-only samples from the last successful World clock. Hidden or
    // missing Object targets suppress only their actor-bound balloon subtitle.
    void Collect_Subtitles(std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE>& out) const;

	/* emissionIndex selects one row of an authored emission list; a seeded
	   emitter keeps the single-object contract and answers index 0 only. */
	bool_t Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out, uint32_t emissionIndex = 0u,
		const std::string& bone = {}, bool_t boneRotation = false) const;
	// No match leaves status empty; an active but unavailable/ambiguous actor fails closed.
	bool_t Try_GetPresentationBossAnchor(const std::string& archetype, const std::string& bone,
		PLAYER_ANCHOR& out, std::string& status) const;
	bool_t Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out, uint32_t emissionIndex = 0u,
		const std::string& bone = {}, bool_t boneRotation = false) const;
	std::string Get_ObjectSampleStatus(const std::string& instanceId) const;
	// The admitted sequence retains object ownership; callers inspect the current visible pose only.
	void Collect_VisibleObjects(std::vector<std::shared_ptr<CWorldSequenceObject>>& out) const;
    struct OBJECT_INSPECTION_SAMPLE final
    {
        std::string instanceId, slotId, objectId, modelAssetId;
        uint32_t emissionIndex = 0u;
        std::shared_ptr<CWorldSequenceObject> object;
    };
    // Append the existing sampled clones, including authored-hidden and held objects.
    // Reading inspection identity never advances a clock or creates a replacement model.
    void Collect_ObjectInspectionSamples(std::vector<OBJECT_INSPECTION_SAMPLE>& out) const;
	struct OBJECT_COLLIDER_SAMPLE
	{
		std::string instanceId, colliderTrackId, behavior;
		std::string shape = "BOX";
		uint32_t emissionIndex = 0;
		float3_t center{}, halfExtents{}, gripPosition{};
		f32_t yawDegrees = 0.f;
		bool_t hasGrip = false;
	};
	// Read the last successful Apply_Objects result; never advance or reconstruct its clock.
	void Collect_ObjectColliderSamples(std::vector<OBJECT_COLLIDER_SAMPLE>& out) const;
	void Clear();

	bool_t Is_Ready() const noexcept
	{
		return !m_Document.Get_AreaId().empty();
	}
	const CWorldSequenceDocument& Get_Document() const noexcept
	{
		return m_Document;
	}
	const std::string& Get_Status() const noexcept { return m_Status; }

	/* Starts one authored instance. Restarting an already playing instance
	   rewinds it against the baseline captured by the first start. */
	bool_t Play(const std::string& instanceId, const TARGET_SET& targets, f32_t playbackSpeed = 1.f, const float3_t& positionOffset = {}, uint32_t durationMs = 0u,
		const std::optional<OBJECT_PLACEMENT>& placement = {});
	bool_t Validate_ObjectPlacement(const std::string& instanceId,
		const std::optional<OBJECT_PLACEMENT>& placement, std::string& status) const;
	// Update the existing object at its current clock; invalid edits leave its pose and motion unchanged.
	bool_t Set_ObjectPlacement(const std::string& instanceId,
		const std::optional<OBJECT_PLACEMENT>& placement, const TARGET_SET& targets);
	// A Server result changes the motion of an existing object, retaining its placement and CModel.
	bool_t Apply_ObjectMotion(const std::string& targetInstanceId,
		const std::string& motionInstanceId, const TARGET_SET& targets);
	bool_t Is_Playing(const std::string& instanceId) const;
	/* Placements the Level has taken out of every instance's hands: a popped
	   Mario ball stays hidden however its layout samples it, until the Level
	   hands the placement back. */
	void Set_PlacementSuppressed(uint64_t placementId, bool_t suppressed);
	bool_t Is_PlacementSuppressed(uint64_t placementId) const;
	/* Placements the instances that are playing or holding a pose right now manipulate. Returns an
	   order-independent signature (0 = none) so a caller can see the set change without copying it;
	   pOut (optional) receives the ids. Nothing is allocated when pOut is null. */
	uint64_t Collect_OwnedPlacements(std::unordered_set<uint64_t>* pOut) const;
	/* The camera cue runs on the cutscene's own clock. Only the player owns
	   that clock, so it hands out a read-only sample instead of letting a
	   second owner count the same time. false means the instance is not
	   playing and the caller must not pose a camera from a stale value. */
	bool_t Try_GetElapsedMs(
		const std::string& instanceId,
		f32_t& outElapsedMs) const;
	/* Returns only the pose successfully applied to the live placement. The
	   authored record remains the replay baseline, never the current pose. */
	bool_t Try_GetSampledPlacementRecord(const std::string& instanceId,
		uint64_t placementId, MAP_PLACEMENT_RECORD& outRecord) const;
	/* Authoring needs to hold a cutscene on one frame and step to any point
	   of it. Paused instances stop advancing but keep their baselines, so a
	   scrub never restarts the sequence or loses the placed pose. */
	void Set_Paused(bool_t paused);
    // For a presentation owner that supplies source milliseconds by external seek.
    // This changes audio pitch only; the owner continues to drive every track.
    void Set_ExternalSoundClockRate(f32_t rate) { if (std::isfinite(rate) && rate > 0.f && rate <= 16.f) m_ExternalSoundClockRate = rate; }
    void Update_SoundTails(f32_t timeDelta);
    // Level-owned listener audience; visual clocks continue when its sound is inaudible.
    void Set_SoundAudience(std::function<bool(const std::string&)> audience) { m_SoundAudience = std::move(audience); }
    void Retire_InstanceSoundTails(const std::string& instanceId);
	bool_t Is_Paused() const noexcept { return m_bPaused; }
	/* Moves every playing instance to the same wall-clock point and applies
	   that frame at once. false means nothing is playing to scrub. */
	bool_t Seek_AllToMs(f32_t elapsedMs, const TARGET_SET& targets);
	bool_t Seek_InstanceToMs(const std::string& instanceId, f32_t elapsedMs, const TARGET_SET& targets, bool_t discontinuous = true);
	void Stop_Instance(const std::string& instanceId, const TARGET_SET& targets, bool_t restorePlacements, bool_t preserveSoundTail = false);
	/* The longest authored span across the playing instances, so the tool can
	   size a scrub bar without guessing. */
	f32_t Get_LongestElapsedSpanMs() const;
	// Explicit duration limits births; admitted Object Effect tails finish afterwards.
	f32_t Get_InstanceElapsedSpanMs(const std::string& instanceId,
		f32_t playbackSpeed = 1.f, uint32_t durationMs = 0u) const;

	/* Stopping hands every animated Deploy target back: an authoring preview
	   left running blocks the prop's state from being set, so a second play
	   could never restore it. */
	void Stop_All(const TARGET_SET& targets, bool_t restorePlacements = false, bool_t preserveSoundTail = false);

	/* Advances every playing instance and writes the sampled presentation. A
	   target that disappears stops only its own instance. */
	void Update(f32_t timeDelta, const TARGET_SET& targets);

	/* Shared evaluation used by both the product player and the Map Tool. */
	static WORLD_SEQUENCE_TRANSFORM_KEY Sample_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const WORLD_SEQUENCE_TRACK& track,
		f32_t timeMs);
	static MAP_PLACEMENT_RECORD Compose_SampledRecord(
		const MAP_PLACEMENT_RECORD& baseline,
		bool_t baselineRuntimeVisible,
		const WORLD_SEQUENCE_TRANSFORM_KEY& key);
	static const WORLD_SEQUENCE_TRACK* Find_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId);
	static const WORLD_SEQUENCE_ANIMATION_TRACK* Find_AnimationTrack(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId);
	/* The clip a slot is playing at this point of the sequence, plus the time
	   that clip's window ends. A slot with one track answers with that track
	   and the sequence duration, so a chain and a single clip read the same. */
	static const WORLD_SEQUENCE_ANIMATION_TRACK* Find_AnimationTrackAt(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId,
		f32_t localMs,
		f32_t& outWindowEndMs);
	static MAP_RUNTIME_PLACED_ENTRY* Find_Placement(
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t placementId);
	static bool_t Try_ParseTargetId(
		const WORLD_SEQUENCE_BINDING& binding,
		uint64_t& outTargetId);
	/* Writes one composed record onto its live presentation. The model cache
	   belongs to the caller so a static batch member is cloned once per asset
	   instead of once per frame. */
	static bool_t Apply_RuntimeRecord(
		const TARGET_SET& targets,
		std::unordered_map<std::string, shared_ptr<CModel>>& modelCache,
		MAP_RUNTIME_PLACED_ENTRY& entry,
		const MAP_PLACEMENT_RECORD& record);

private:
	struct PREPARED_OBJECT_POOL
	{
		bool acceptsReturns = true;
		uint32_t levelIndex = ETOUI(LEVEL::END);
		uint32_t capacity = 0u;
		std::vector<shared_ptr<CWorldSequenceObject>> idle;
	};
	struct OBJECT_INSTANCE
	{
		std::string slotId;
		uint64_t entityId = 0;
		uint32_t emissionIndex = 0;
		uint32_t levelIndex = ETOUI(LEVEL::END);
		shared_ptr<CWorldSequenceObject> object;
		std::shared_ptr<PREPARED_OBJECT_POOL> preparationPool;
		std::shared_ptr<const SAYDON_WEAPON_REPLACEMENT> weaponReplacement;
		std::shared_ptr<const SAYDON_HAT_REPLACEMENT> hatReplacement;
		std::shared_ptr<const void> npcPreviewSuppression;
	};
	struct OBJECT_MODEL
	{
		shared_ptr<CModel> model;
		ComPtr<ID3D11ShaderResourceView> diffuse;
		ID3D11Device* deviceIdentity = nullptr;
		ID3D11DeviceContext* contextIdentity = nullptr;
		const CMapAssetCatalog* catalogIdentity = nullptr;
		// Product boss presentation drawn with this body (empty = single model).
		std::string presentationBossArchetypeId;
	};
    struct SOUND_INSTANCE
    {
        std::string key;
        uint64_t handle = 0u;
        f32_t endElapsedMs = 0.f;
        uint32_t mediaDurationMs = 0u;
        uint64_t mediaCycle = 0u;
        bool_t loopToDuration = false;
    };
    struct RETIRED_SOUND
    {
        std::string ownerId;
        uint64_t handle = 0u;
        f32_t remainingMs = 0.f;
    };
	struct ACTIVE_INSTANCE final
	{
		std::string instanceId;
		std::string motionInstanceId;
		f32_t motionStartMs = 0.f;
		f32_t elapsedMs = 0.f;
		f32_t playbackSpeed = 1.f;
		float3_t positionOffset{};
		std::optional<OBJECT_PLACEMENT> placement;
		std::vector<PLACEMENT_BASELINE> placementBaselines;
		std::unordered_map<uint64_t, MAP_PLACEMENT_RECORD> sampledPlacements;
		std::unordered_map<uint64_t, float4x4_t> sampledDeployPivots;
		std::vector<uint64_t> deployTargets;
		uint32_t durationMs = 0;
		std::string objectSampleStatus;
        std::string sampledSubtitleTemplateId;
        f32_t sampledSubtitleLocalMs = 0.f;
        bool_t hasSubtitleSample = false;

		std::vector<OBJECT_INSTANCE> objects;
		std::vector<OBJECT_COLLIDER_SAMPLE> objectColliderSamples;
		struct EFFECT_INSTANCE
		{
			std::string key;
			uint32_t handle = 0;
			uint64_t v1Handle = 0;
			std::shared_ptr<const EFFECT_DOCUMENT_DESC> sourceDocument;
			std::optional<OBJECT_PLACEMENT> sampledPlacement;
			float3_t sampledPositionOffset{};
		};
		std::vector<EFFECT_INSTANCE> effects;
        std::vector<SOUND_INSTANCE> sounds;
        bool_t seekSounds = false;
        bool_t soundPlaybackFinished = false;
		std::unordered_map<std::string, PLAYER_ANCHOR> emissionAnchors;
	};

	/* A finished sequence keeps its last authored frame; only a broken one
	   gives its targets back. Releasing on completion would snap an unfolded
	   bridge back to the folded pose the clip starts from. */
	enum class APPLY_RESULT
	{
		PLAYING,
		FINISHED,
		FAILED,
	};
	APPLY_RESULT Apply_Instance(ACTIVE_INSTANCE& active, const TARGET_SET& targets);
	bool_t Prepare_ObjectResources(const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets);
	bool_t Prepare_ObjectMotionChain(const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets);
	bool_t Apply_Objects(ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const WORLD_SEQUENCE_TEMPLATE& sequence, const TARGET_SET& targets, f32_t localMs, bool_t visible, bool_t holdFinalPose = false,
		f32_t emissionStartMs = 0.f, f32_t emissionRate = 1.f, const std::string& emissionMotionId = {});
	bool_t Get_EmissionAnchor(ACTIVE_INSTANCE& active, const TARGET_SET& targets, const std::string& key,
		f32_t birthMs, const PLAYER_ANCHOR& baseline, PLAYER_ANCHOR& out);
	static bool_t Sample_ObjectWorld(const ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const WORLD_SEQUENCE_TEMPLATE& sequence, const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
		const std::string& slotId, const PLAYER_ANCHOR& anchor, uint32_t emitter, f32_t ageMs, float4x4_t& out, std::string& status,
		bool_t inheritObjectRotation = true,
		const decltype(TARGET_SET::objectWorldPostTransform)& postTransform = {});
	bool_t Apply_ObjectEffects(ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const TARGET_SET& targets);
    void Apply_Sounds(ACTIVE_INSTANCE& active);
    void Retire_Sounds(ACTIVE_INSTANCE& active);
    void Stop_RetiredSounds(const std::string& ownerId = {});
	void Release_Objects(ACTIVE_INSTANCE& active);
	void Clear_PreparedObjects();
	static bool_t Same_ObjectModelInputs(const WORLD_SEQUENCE_OBJECT_RESOURCE& left, const WORLD_SEQUENCE_OBJECT_RESOURCE& right);
	bool_t Admit_PresentationBossModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
		const TARGET_SET& targets, OBJECT_MODEL& out);
	const OBJECT_MODEL* Find_PreparedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const;
	const OBJECT_MODEL* Find_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const;
	void Remember_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const OBJECT_MODEL& model, const TARGET_SET& targets) const;
	void Release_DeployPreviews(
		const ACTIVE_INSTANCE& active,
		const TARGET_SET& targets);

private:
	CWorldSequenceDocument m_Document;
	bool_t m_bPaused = false;
	std::vector<ACTIVE_INSTANCE> m_Active;
    std::vector<RETIRED_SOUND> m_RetiredSounds;
    f32_t m_ExternalSoundClockRate = 1.f;
    std::function<bool(const std::string&)> m_SoundAudience;
	// Finished clocks no longer tick, but own their held pose until explicit stop/replay.
	std::vector<ACTIVE_INSTANCE> m_Held;
	std::unordered_map<std::string, shared_ptr<CModel>> m_ModelCache;
	std::unordered_map<std::string, OBJECT_MODEL> m_ObjectModels;
	std::unordered_map<std::string, std::shared_ptr<PREPARED_OBJECT_POOL>> m_PreparedObjectPools;
	std::unordered_map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectSnapshots;
	std::unordered_set<uint64_t> m_SuppressedPlacements;
	std::string m_Status;
};

NS_END
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Private/WorldSequencePlayer_Objects.cpp

```cpp
#include "WorldSequencePlayer.h"
#include "ActorCatalog.h"
#include "WorldSequenceObject.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "Model.h"
#include "NpcPresentationAssetService.h"
#include "Npc.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "Effect_Playback.h"
#include "Profiler.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace Client;
using namespace Engine;

namespace
{
bool_t Apply_ObjectMaterialConstants(CModel& model, const std::string& materialName,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters, std::string& status)
{
    bool exact = false;
    for (uint32_t mesh = 0u; mesh < model.Get_NumMeshes(); ++mesh)
    {
        const auto& name = model.Get_MaterialName(mesh);
        if (name.find(materialName) == std::string::npos) continue;
        const auto* surface = model.Get_MaterialSurface(mesh);
        if (name != materialName || !surface || surface->family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER ||
            surface->sourceCharacter.program != parameters.program)
        { status = "World Object material name/program mismatch: " + materialName; return false; }
        exact = true;
    }
    if (!exact || model.Override_SourceCharacterConstants(materialName.c_str(), parameters) == 0u)
    { status = "World Object material override was rejected: " + materialName; return false; }
    return true;
}
}

CWorldSequencePlayer::~CWorldSequencePlayer() { Clear(); }

bool_t CWorldSequencePlayer::Set_ObjectMaterialConstants(const std::string& instanceId,
    const std::string& slotId, const std::string& materialName,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end() || materialName.empty())
    { m_Status = "World Object material target is inactive: " + instanceId; return false; }
    bool matched = false;
    for (const auto& entry : active->objects)
    {
        if (entry.slotId != slotId || !entry.object) continue;
        const auto& model = entry.object->Get_Model();
        if (!model) return false;
        if (!Apply_ObjectMaterialConstants(*model, materialName, parameters, m_Status)) return false;
        matched = true;
    }
    // A source actor can be hidden before its first emission. It still exists
    // once emitted, and its material is checked as soon as the clone appears.
    if (!matched)
    { m_Status = "World Object material slot has no live clone: " + slotId; return false; }
    return true;
}

namespace
{
bool_t Sample_ObjectPresentationPostTransform(
    const decltype(CWorldSequencePlayer::TARGET_SET::objectWorldPostTransform)& callback,
    const std::string& instanceId, const f32_t sourceMs, float4x4_t& out, std::string& status)
{
    XMStoreFloat4x4(&out, XMMatrixIdentity());
    if (callback && !callback(instanceId, sourceMs, out, status))
    {
        if (status.empty()) status = "World Object presentation transform is unavailable: " + instanceId;
        return false;
    }
    for (const auto& row : out.m)
        for (const float component : row)
            if (!std::isfinite(component))
            { status = "World Object presentation transform is not finite: " + instanceId; return false; }
    return true;
}

/* The product Valtan part group: a static weapon on the body's grip bone and
   skinned armour plates on its palette, as CValtan builds them. */
void Fill_PresentationParts(const std::string& archetypeId, CWorldSequenceObject::DESC& desc)
{
    desc.presentationParts.clear();
    desc.materialProfileId.clear();
    const BOSS_ACTOR_ENTRY* actor = archetypeId.empty() ? nullptr : CActorCatalog::Find_Boss(archetypeId);
    if (!actor) return;
    desc.materialProfileId = "material.valtan.monster-base.v1";
    desc.presentationParts.push_back({ CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(archetypeId),
        L"Prototype_Component_Shader_VtxMeshBinary", CValtan::WEAPON_SOCKET_BONE });
    for (const BOSS_ARMOR_PART_ENTRY& armor : actor->armorParts)
        desc.presentationParts.push_back({ CValtan::Build_ArmorModelPrototypeTag(armor.stateMask, archetypeId),
            L"Prototype_Component_Shader_VtxAnimMeshBinary", std::string() });
}

std::string Narrow_PrototypeTag(const wstring_t& tag)
{
    std::string text;
    text.reserve(tag.size());
    for (const wchar_t character : tag) text.push_back(character < 128 ? static_cast<char>(character) : '?');
    return text;
}

bool Sample_ObjectCollider(const WORLD_SEQUENCE_COLLIDER_TRACK& collider,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const WORLD_SEQUENCE_TRANSFORM_KEY& key,
    const WORLD_SEQUENCE_OBJECT_MOTION& motion, const uint32_t emitter,
    const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement,
    const CWorldSequenceObject& object, CWorldSequencePlayer::OBJECT_COLLIDER_SAMPLE& out)
{
    const float emissionYaw = motion.emissions.empty() ? 0.f : motion.emissions[emitter].yawDegrees;
    const vector_t localScale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    const vector_t placementScale = placement ? XMLoadFloat3(&placement->scale) : XMVectorReplicate(1.f);
    const float placementYaw = placement ? placement->rotationDegrees.y : 0.f;
    const matrix_t groundBasis = XMMatrixScalingFromVector(localScale) *
        XMMatrixRotationY(XMConvertToRadians(emissionYaw)) * XMMatrixScalingFromVector(placementScale) *
        XMMatrixRotationY(XMConvertToRadians(placementYaw));
    out.behavior = collider.behavior;
    out.shape = collider.shape;
    out.yawDegrees = emissionYaw + placementYaw + collider.yawDegrees;
    XMStoreFloat3(&out.halfExtents, XMVectorAbs(XMLoadFloat3(&collider.halfExtents) * localScale * placementScale));
    if (collider.shape == "CYLINDER")
    {
        float3_t scale;
        XMStoreFloat3(&scale, XMVectorAbs(localScale * placementScale));
        // Server worldTrack cylinders use the authored X radius and the larger ground scale.
        out.halfExtents.x = out.halfExtents.z = collider.halfExtents.x * (std::max)(scale.x, scale.z);
    }
    out.hasGrip = collider.behavior == "HOOK_CAPTURE";
    if (out.hasGrip)
    {
#ifdef _DEBUG
        float4x4_t attachment;
        if (!object.Try_GetAttachmentWorld(collider.attachmentBone, attachment)) return false;
        const matrix_t world = XMLoadFloat4x4(&attachment);
        XMStoreFloat3(&out.center, XMVector3TransformCoord(XMLoadFloat3(&collider.positionOffset), world));
        XMStoreFloat3(&out.gripPosition, XMVector3TransformCoord(XMLoadFloat3(&collider.gripLocalOffset), world));
#else
        return false; // Bone attachment preview is an authoring-only capability.
#endif
    }
    else
    {
        const matrix_t world = XMLoadFloat4x4(&object.Get_SampledWorld());
        XMStoreFloat3(&out.center, world.r[3] +
            XMVector3TransformNormal(XMLoadFloat3(&collider.positionOffset), groundBasis));
    }
    return true;
}

// Use the same clip windows, ticks and end policy as WorldSequenceObject::Sample,
// but sample the immutable CModel skeleton without changing the visible palette.
bool Sample_ObjectEffectBone(const std::shared_ptr<CModel>& model,
    const WORLD_SEQUENCE_TEMPLATE& sequence, const std::string& slotId,
    const std::string& bone, const float sampleMs, float4x4_t& out, std::string& error)
{
    if (!model || !model->Has_Bone(bone.c_str()))
    { error = "World Object Effect bone is unavailable: " + bone; return false; }
    f32_t windowEnd = 0.f;
    const auto* animation = CWorldSequencePlayer::Find_AnimationTrackAt(sequence, slotId, sampleMs, windowEnd);
    if (!animation) { XMStoreFloat4x4(&out, model->Get_BoneMatrix(bone.c_str())); return true; }
    uint32_t index = UINT32_MAX;
    for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
        if (animation->clipName == model->Get_AnimationName(i)) { index = i; break; }
    float position = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, position, duration) || duration <= 0.f)
    { error = "World Object Effect animation is unavailable: " + animation->clipName; return false; }
    float ticks = 0.f;
    if (!CWorldSequenceDocument::Try_SampleAnimationTicks(*animation, sampleMs, windowEnd,
        model->Get_AnimationTickPerSecond(index), duration, ticks))
    { error = "World Object Effect animation source range is invalid: " + animation->clipName; return false; }
    const uint32_t boneIndex = static_cast<uint32_t>(model->Find_BoneIndex(bone.c_str()));
    if (!model->Sample_AnimationBoneCombinedMatrices(animation->clipName.c_str(), ticks,
        std::span<const uint32_t>(&boneIndex, 1u), std::span<float4x4_t>(&out, 1u)))
    { error = "World Object Effect bone sample failed: " + bone; return false; }
    return true;
}

bool Sample_ObjectEffectAttachments(const EFFECT_DOCUMENT_DESC& document,
    const std::shared_ptr<CModel>& model, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const std::string& slotId, const float sampleMs, const float4x4_t& root,
    std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    std::unordered_map<std::string, float4x4_t> bones;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (!element.bVisible || !attachment.bEnabled || !attachment.bFollow) continue;
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "World Object Effect source attachment has no stable slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "World Object Effect camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            auto found = bones.find(attachment.strRuntimeBoneName);
            if (found == bones.end())
            {
                float4x4_t bone;
                // A static prop has no source-character skeleton (b_root, FX_* sockets).
                // Mirror the owner-anchored product path, which skips a missing bone,
                // but keep the slot present: transform-history sampling requires every
                // follow slot, so the attachment rides the object pivot instead.
                // The explicit effect-track bone (effect.bone) stays strict in the provider.
                if (model && !model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    XMStoreFloat4x4(&bone, XMMatrixIdentity());
                else if (!Sample_ObjectEffectBone(model, sequence, slotId, attachment.strRuntimeBoneName,
                    sampleMs, bone, error)) return false;
                found = bones.emplace(attachment.strRuntimeBoneName, bone).first;
            }
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = XMLoadFloat4x4(&found->second) * XMLoadFloat4x4(&root);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t yawAnchor;
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(found->second, root, root, yawAnchor))
                { error = "World Object Effect owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "World Object Effect source attachment orientation is unsupported."; return false; }
        }
        const auto& socket = attachment.SocketLocalTransform;
        const matrix_t world = XMMatrixScalingFromVector(XMLoadFloat3(&socket.vScale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(socket.vRotationDegrees.x),
                XMConvertToRadians(socket.vRotationDegrees.y), XMConvertToRadians(socket.vRotationDegrees.z)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&socket.vPosition)) * anchor;
        const float determinant = XMVectorGetX(XMMatrixDeterminant(world));
        if (XMMatrixIsNaN(world) || XMMatrixIsInfinite(world) || !std::isfinite(determinant) || std::abs(determinant) < 1.e-12f)
        { error = "World Object Effect source attachment is singular or non-finite."; return false; }
        float4x4_t value; XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = anchors.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0; r < 4; ++r) for (size_t c = 0; c < 4; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > .0001f)
                { error = "World Object Effect source slot has conflicting bones."; return false; }
    }
    return true;
}
}


bool_t CWorldSequencePlayer::Resolve_BossBoneAnchor(const std::shared_ptr<CModel>& model,
    const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status)
{
    if (!model || (!bone.empty() && !model->Has_Bone(bone.c_str())))
    { status = "World Object boss BODY bone is unavailable: " + bone; return false; }
    const matrix_t basis = bone.empty() ? XMLoadFloat4x4(&root) :
        model->Get_BoneMatrix(bone.c_str()) * XMLoadFloat4x4(&root);
    XMStoreFloat4x4(&out.world, basis);
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0u; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { status = "World Object boss BODY bone pose is not finite: " + bone; return false; }
    if (std::abs(XMVectorGetX(XMMatrixDeterminant(basis))) < .000001f)
    { status = "World Object boss BODY bone pose is singular: " + bone; return false; }
    out.bodyModel = model;
    // The object sampler preserves the socket translation and normalizes its
    // axes. Import scale belongs to the prop; boss scale is already in this pose.
    return true;
}

void CWorldSequencePlayer::Collect_ValidationTargets(const TARGET_SET& targets,
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy)
{
    placements.clear(); deploy.clear();
    if (!targets.Is_Complete()) return;
    // Load the complete placement source, not only the current rendering scope.
    std::vector<MAP_PLACEMENT_RECORD> records;
    std::string status;
    if (CMapPlacementRuntime::Read_Placements(*targets.pCatalog, records, status))
        for (const auto& row : records)
        {
            const auto* asset = targets.pCatalog->Find(row.assetId);
            placements.emplace(row.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{row.signedScale,
                asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
        }
    for (const auto& entry : *targets.pPlacements)
    {
        const auto* asset = targets.pCatalog->Find(entry.record.assetId);
        placements.insert_or_assign(entry.record.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{
            entry.record.signedScale, asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
    }
    for (const auto& entry : targets.pDeployRuntime->Get_Entries())
    {
        WORLD_SEQUENCE_DEPLOY_INFO info;
        if (entry.object && !entry.object->Is_StaticDeployModel())
        {
            for (const auto& clip : entry.object->Get_AnimationClips()) info.animationClips.push_back(clip.name);
            info.animationTargetSupported = !info.animationClips.empty();
        }
        deploy.emplace(entry.placement.runtimePlacementId, std::move(info));
    }
}

bool_t CWorldSequencePlayer::Set_Document(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    const bool_t admitted = Set_DocumentBatch(document, targets, {this}, status);
    m_Status = status;
    return admitted;
}

bool_t CWorldSequencePlayer::Set_DocumentBatch(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, const std::vector<CWorldSequencePlayer*>& players, std::string& status)
{
    if (!targets.Is_Complete())
    { status = "World Object runtime targets are unavailable."; return false; }
    std::unordered_set<CWorldSequencePlayer*> unique;
    if (players.empty() || std::any_of(players.begin(), players.end(), [&](auto* player) {
        return !player || !unique.insert(player).second; }))
    { status = "World Object document batch has an empty or duplicate player."; return false; }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    // Object-only documents own their bindings; unrelated map/deploy tables
    // are neither read nor consumed by Validate for these occurrences.
    const bool usesPlacedTargets = std::any_of(document.Get_Instances().begin(), document.Get_Instances().end(),
        [](const auto& instance) { return std::any_of(instance.bindings.begin(), instance.bindings.end(),
            [](const auto& binding) { return binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; }); });
    if (usesPlacedTargets) Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
        return false;
    std::vector<CWorldSequenceDocument> staged;
    staged.reserve(players.size());
    for (size_t i = 0; i < players.size(); ++i) staged.push_back(document);
    for (size_t i = 0; i < players.size(); ++i)
    {
        auto& player = *players[i];
        player.Stop_All(targets, true);
        player.Clear_PreparedObjects();
        player.m_ObjectModels.clear();
        player.m_EffectSnapshots.clear();
        player.m_Document = std::move(staged[i]);
        player.m_Status = "World Object document admitted.";
    }
    status = "World Object document admitted.";
    return true;
}

bool_t CWorldSequencePlayer::Replace_DocumentKeepingModels(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    if (!targets.Is_Complete())
    {
        status = "World Object runtime targets are unavailable.";
        m_Status = status;
        return false;
    }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
    {
        m_Status = status;
        return false;
    }
    CWorldSequenceDocument staged = document;
    Stop_All(targets, true);
    Clear_PreparedObjects();
    // A kept model must still be what its resource asks for. Anything renamed,
    // removed or changed in its model inputs is rebuilt on the next Play.
    size_t kept = 0u;
    for (auto entry = m_ObjectModels.begin(); entry != m_ObjectModels.end();)
    {
        const auto* before = m_Document.Find_ObjectResource(entry->first);
        const auto* after = staged.Find_ObjectResource(entry->first);
        if (nullptr == before || nullptr == after || !Same_ObjectModelInputs(*before, *after))
            entry = m_ObjectModels.erase(entry);
        else
        {
            ++kept;
            ++entry;
        }
    }
    m_EffectSnapshots.clear();
    m_Document = std::move(staged);
    status = "World Object document admitted; kept " + std::to_string(kept) + " prepared model(s).";
    m_Status = status;
    return true;
}

bool_t CWorldSequencePlayer::Same_ObjectModelInputs(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& left, const WORLD_SEQUENCE_OBJECT_RESOURCE& right)
{
    return left.modelAssetId == right.modelAssetId && left.modelPreScale == right.modelPreScale &&
        left.animated == right.animated && left.diffuseTextureAssetId == right.diffuseTextureAssetId &&
        left.materialSourceModelAssetId == right.materialSourceModelAssetId &&
        left.animationSetAssetId == right.animationSetAssetId &&
        left.presentationBossArchetypeId == right.presentationBossArchetypeId &&
        left.materialProfile == right.materialProfile && left.mapMaterialBindings == right.mapMaterialBindings;
}

bool_t CWorldSequencePlayer::Admit_PresentationBossModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const TARGET_SET& targets, OBJECT_MODEL& out)
{
    /* Reuse the product boss admission instead of decoding the body again: the
       Level owns the combined body (AnimSet attached), its armour and weapon. */
    const std::string& archetypeId = resource.presentationBossArchetypeId;
    const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
    if (!actor)
    { m_Status = "World Object presentation boss is not in the boss catalog: " + archetypeId; return false; }
    if (actor->bodyModel != resource.modelAssetId || actor->animationSetId != resource.animationSetAssetId ||
        actor->bodyModelPreScale != resource.modelPreScale)
    { m_Status = "World Object body fields must match presentation boss " + archetypeId + ": " + resource.objectId; return false; }
    const wstring_t bodyTag = CValtanPresentationAssetService::Get_BodyModelPrototypeTag(archetypeId);
    if (bodyTag.empty())
    { m_Status = "World Object presentation boss has no product assembly: " + archetypeId; return false; }
    if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(targets.device, targets.context,
        targets.levelIndex, archetypeId)))
    { m_Status = "World Object presentation boss admission failed: " + archetypeId; return false; }
    out.model = dynamic_pointer_cast<CModel>(CGameInstance::Get().Clone_Prototype(targets.levelIndex, bodyTag));
    if (!out.model || !out.model->Get_NumMeshes() || !out.model->Has_Animations())
    { m_Status = "World Object presentation boss body is unavailable: " + Narrow_PrototypeTag(bodyTag); return false; }
    CWorldSequenceObject::DESC parts;
    Fill_PresentationParts(archetypeId, parts);
    for (const auto& part : parts.presentationParts)
    {
        if (!CGameInstance::Get().Clone_Prototype(targets.levelIndex, part.modelPrototypeTag))
        { m_Status = "World Object presentation boss part is unavailable: " + Narrow_PrototypeTag(part.modelPrototypeTag); return false; }
        if (!part.socketBone.empty() && !out.model->Has_Bone(part.socketBone.c_str()))
        { m_Status = "World Object presentation boss socket bone is unavailable: " + part.socketBone; return false; }
    }
    out.presentationBossArchetypeId = archetypeId;
    out.deviceIdentity = targets.device.Get();
    out.contextIdentity = targets.context.Get();
    out.catalogIdentity = targets.pCatalog;
    return true;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_PreparedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto matches = [&](const auto& entry) {
        const auto* prepared = m_Document.Find_ObjectResource(entry.first);
        return prepared && Same_ObjectModelInputs(*prepared, resource) && entry.second.model &&
            entry.second.deviceIdentity == targets.device.Get() &&
            entry.second.contextIdentity == targets.context.Get() && entry.second.catalogIdentity == targets.pCatalog;
    };
    // Preserve the exact prototype identity used by an existing object clone pool.
    const auto exact = m_ObjectModels.find(resource.objectId);
    if (exact != m_ObjectModels.end() && matches(*exact)) return &exact->second;
    for (const auto& entry : m_ObjectModels)
        if (matches(entry)) return &entry.second;
    return nullptr;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_SharedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto* owner = targets.objectPreparationOwner;
    if (!owner || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision()) return nullptr;
    return owner->Find_PreparedObjectModel(resource, targets);
}

void CWorldSequencePlayer::Remember_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const OBJECT_MODEL& model, const TARGET_SET& targets) const
{
    auto* owner = targets.objectPreparationOwner;
    if (!owner || owner == this || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision() || !model.model ||
        model.deviceIdentity != targets.device.Get() || model.contextIdentity != targets.context.Get() ||
        model.catalogIdentity != targets.pCatalog) return;
    const auto* prepared = owner->m_Document.Find_ObjectResource(resource.objectId);
    if (!prepared || !Same_ObjectModelInputs(*prepared, resource))
    {
        const auto& resources = owner->m_Document.Get_ObjectResources();
        const auto found = std::find_if(resources.begin(), resources.end(), [&](const auto& row) {
            return Same_ObjectModelInputs(row, resource);
        });
        if (found == resources.end()) return;
        prepared = &*found;
    }
    // The admitted owner document owns this entry and clears it on reload. Each
    // visible object still clones its own CModel/Bones/Animations from the prototype.
    owner->m_ObjectModels.emplace(prepared->objectId, model);
}

bool_t CWorldSequencePlayer::Prepare_ObjectResources(
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    const auto* sequence = m_Document.Find_Template(instance.templateId);
    if (sequence)
        for (const auto& effect : sequence->effectTracks)
        {
            if (effect.resourceKind == "V1_EFFECT")
            {
                const std::vector<std::string> ids{effect.resourceId};
                std::vector<std::string> queued;
                if (!CEffectPresentationService::Queue_ProductTargets_Priority(ids, queued, m_Status)) return false;
                const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(ids);
                if (probe.iFailedCount || probe.iUnavailableCount)
                { m_Status = "World Object V1 effect preparation failed: " + effect.resourceId; return false; }
                if (!probe.bCatalogRevisionCurrent || !probe.bSettled)
                { m_Status = "World Object V1 effect is preparing: " + effect.resourceId; return false; }
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                const auto binding = std::find_if(instance.bindings.begin(), instance.bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                const auto* owner = binding == instance.bindings.end() ? nullptr :
                    m_Document.Find_ObjectResource(binding->targetId);
                if (!document || !owner)
                { m_Status = "World Object V1 effect has no prepared document or owner: " + effect.resourceId; return false; }
                for (const auto& cue : document->ModelCues)
                    if (cue.strModelAssetId != owner->modelAssetId)
                    { m_Status = "World Object V1 model cue names a different model: " + cue.strCueId; return false; }
                continue;
            }
            const auto key = effect.resourceKind + ":" + effect.resourceId;
            if (m_EffectSnapshots.contains(key)) continue;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
            std::string error;
            const auto kind = effect.resourceKind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF;
            if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(kind, effect.resourceId, snapshot, error))
            { m_Status = "World Object effect admission failed: " + effect.resourceId + " / " + error; return false; }
            EFFECT_V2_GROUP group;
            if (kind == EFFECT_V2_RESOURCE_KIND::GROUP) group = *snapshot->Find_Group(effect.resourceId);
            else
            {
                group.strGroupId = effect.resourceId;
                EFFECT_V2_GROUP_CHILD child;
                child.strChildId = "object.effect.leaf"; child.strResourceId = child.strEffectId = effect.resourceId;
                group.Children.push_back(child);
            }
            if (!CEffectV2Runtime::Prewarm_Group(group, snapshot, targets.device, targets.context))
            { m_Status = "World Object effect prewarm failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
            m_EffectSnapshots.emplace(key, std::move(snapshot));
        }
    for (const auto& binding : instance.bindings)
    {
        if (sequence && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
        {
            // Reject trimmed native ranges before releasing an existing preview owner.
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId || (animation.sourceStartMs == 0u && animation.sourceEndMs == 0u)) continue;
                uint64_t targetId = 0u;
                const auto object = Try_ParseTargetId(binding, targetId) && targets.pDeployRuntime ?
                    targets.pDeployRuntime->Find(targetId) : nullptr;
                float duration = 0.f, seconds = 0.f;
                if (object)
                    for (const auto& clip : object->Get_AnimationClips())
                        if (clip.name == animation.clipName) { duration = clip.durationSeconds; break; }
                if (!CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                    static_cast<float>(sequence->durationMs), 1.f, duration, seconds))
                { m_Status = "World sequence Deploy animation source range is unavailable: " + animation.clipName; return false; }
            }
        }
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        if (!resource || resource->modelAssetId.empty())
        { m_Status = "World Object model binding is unavailable: " + binding.targetId; return false; }
        if (!targets.device || !targets.context)
        { m_Status = "World Object render device is unavailable."; return false; }
        auto model = m_ObjectModels.find(resource->objectId);
        if (model != m_ObjectModels.end() &&
            (model->second.deviceIdentity != targets.device.Get() ||
                model->second.contextIdentity != targets.context.Get() || model->second.catalogIdentity != targets.pCatalog))
        { m_Status = "World Object prepared model belongs to different render targets; reload its owner: " + resource->objectId; return false; }
        if (model == m_ObjectModels.end())
        {
            const auto* prepared = Find_PreparedObjectModel(*resource, targets);
            if (!prepared) prepared = Find_SharedObjectModel(*resource, targets);
            if (prepared) model = m_ObjectModels.emplace(resource->objectId, *prepared).first;
        }
        if (model == m_ObjectModels.end() && !resource->presentationBossArchetypeId.empty())
        {
            OBJECT_MODEL staged;
            if (!Admit_PresentationBossModel(*resource, targets, staged)) return false;
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (model == m_ObjectModels.end())
        {
            OBJECT_MODEL staged;
            staged.deviceIdentity = targets.device.Get();
            staged.contextIdentity = targets.context.Get();
            staged.catalogIdentity = targets.pCatalog;
            const auto path = CRuntimeAssetRoot::Resolve(resource->modelAssetId);
            if (path.empty())
            { m_Status = "World Object model path is invalid: " + resource->modelAssetId; return false; }
            MODEL_ASSET_LOAD_DESC load;
            if (!CActorCatalog::Build_DerivedModelLoadDescription(resource->modelAssetId,
                resource->materialSourceModelAssetId, load, m_Status)) return false;
            for (const auto& binding : resource->mapMaterialBindings)
            {
                const auto* asset = targets.pCatalog->Find(binding.sourceAssetId);
                if (!asset) { m_Status = "World Object map material asset is missing: " + binding.sourceAssetId; return false; }
                const auto found = std::find_if(asset->materialOverrides.begin(), asset->materialOverrides.end(),
                    [&](const auto& row) { return row.materialName == binding.sourceMaterialName; });
                if (found == asset->materialOverrides.end() || found->surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
                { m_Status = "World Object map surface binding is missing or unsupported: " + binding.sourceMaterialName; return false; }
                auto material = *found;
                material.materialName = binding.materialName;
                material.surface.sourceBgUnlit = binding.unlit;
                material.surface.hasBakedLighting = false;
                material.surface.hasStaticShadow = false;
                material.bakedAveragePath.clear(); material.bakedDirectionalPath.clear(); material.staticShadowPath.clear();
                if (!binding.diffuseTextureAssetId.empty())
                {
                    material.surfaceDiffusePath = CRuntimeAssetRoot::Resolve(binding.diffuseTextureAssetId);
                    if (material.surfaceDiffusePath.empty()) { m_Status = "World Object surface texture is invalid"; return false; }
                }
                std::erase_if(load.materialOverrides, [&](const auto& prior) { return prior.materialName == material.materialName; });
                load.materialOverrides.push_back(std::move(material));
            }
            if (resource->materialProfile)
            {
                MODEL_MATERIAL_OVERRIDE material;
                if (!CWorldSequenceDocument::Build_MaterialOverride(*resource->materialProfile, load.assetRoot, material))
                { m_Status = "World Object material admission failed: " + resource->objectId; return false; }
                std::erase_if(load.materialOverrides, [&](const MODEL_MATERIAL_OVERRIDE& prior) {
                    return prior.materialName == material.materialName;
                });
                load.materialOverrides.push_back(std::move(material));
            }
            staged.model = CModel::Create(targets.device, targets.context,
                resource->animated ? MODEL::ANIM : MODEL::NONANIM, load,
                XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
            if (!staged.model || !staged.model->Get_NumMeshes())
            {
                m_Status = "World Object model admission failed: " + resource->modelAssetId;
                // This Create overload decodes the nonempty load.meshPath on this thread.
                const auto report = CModelDecoderRegistry::Get().Get_LastReport();
                if (report.meshPath == load.meshPath)
                {
                    if (!report.succeeded && !report.error.empty()) m_Status += " / " + report.error;
                    else if (report.succeeded) m_Status += " / binary decoded; model setup failed";
                }
                return false;
            }
            if (!resource->animationSetAssetId.empty())
            {
                /* The Valtan bodies ship their clips in a separate AnimSet WModel.
                   Attach it to the prototype before any clone so every occurrence
                   samples the same clip table the product boss uses. */
                const auto animationSetPath = CRuntimeAssetRoot::Resolve(resource->animationSetAssetId);
                if (animationSetPath.empty())
                { m_Status = "World Object animation set path is invalid: " + resource->animationSetAssetId; return false; }
                const unique_ptr<CModel> animationSet = CModel::Create(targets.device, targets.context,
                    MODEL::ANIM, animationSetPath.string().c_str(),
                    XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
                if (!animationSet || !animationSet->Has_Animations() ||
                    FAILED(staged.model->Attach_AnimationSet(*animationSet)))
                { m_Status = "World Object animation set does not match the body: " + resource->animationSetAssetId; return false; }
            }
            if (!resource->diffuseTextureAssetId.empty())
            {
                const auto texture = CRuntimeAssetRoot::Resolve(resource->diffuseTextureAssetId);
                if (texture.empty() || FAILED(DirectX::CreateDDSTextureFromFileEx(
                    targets.device.Get(), texture.c_str(), 0u, D3D11_USAGE_DEFAULT,
                    D3D11_BIND_SHADER_RESOURCE, 0u, 0u, DirectX::DDS_LOADER_FORCE_SRGB,
                    nullptr, &staged.diffuse)) || !staged.diffuse)
                { m_Status = "World Object texture admission failed: " + resource->diffuseTextureAssetId; return false; }
            }
            if (!staged.diffuse)
                for (uint32_t mesh = 0; mesh < staged.model->Get_NumMeshes(); ++mesh)
                    if (!staged.model->Has_MaterialTexture(mesh, aiTextureType_DIFFUSE))
                    { m_Status = "World Object diffuse texture is unavailable: " + resource->modelAssetId +
                        " (mesh " + std::to_string(mesh) + ")"; return false; }
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (sequence)
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                uint32_t index = UINT32_MAX;
                for (uint32_t i = 0; i < model->second.model->Get_NumAnimations(); ++i)
                    if (animation.clipName == model->second.model->Get_AnimationName(i)) { index = i; break; }
                if (index == UINT32_MAX) { m_Status = "World Object clip is absent: " + animation.clipName; return false; }
                if (animation.sourceStartMs != 0u || animation.sourceEndMs != 0u)
                {
                    float position = 0.f, duration = 0.f, ticks = 0.f;
                    if (!model->second.model->Get_AnimationProgress(index, position, duration) ||
                        !CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                            static_cast<float>(sequence->durationMs), model->second.model->Get_AnimationTickPerSecond(index), duration, ticks))
                    { m_Status = "World Object animation source range exceeds the native clip range: " + animation.clipName; return false; }
                }
            }
        if (sequence)
            for (const auto& effect : sequence->effectTracks)
            {
                if (effect.slotId != binding.slotId) continue;
                if (!effect.bone.empty() && !model->second.model->Has_Bone(effect.bone.c_str()))
                { m_Status = "World Object Effect bone is unavailable: " + effect.bone; return false; }
                if (effect.resourceKind != "V1_EFFECT") continue;
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                for (const auto& element : document->Elements)
                {
                    const auto& attachment = element.ActionCueAttachment;
                    if (element.bVisible && attachment.bEnabled && attachment.bFollow &&
                        attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
                        !model->second.model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    {
                        // Not a rejection: Sample_ObjectEffectAttachments anchors this slot at the
                        // object pivot, as the owner-anchored product path does for a missing bone.
                        OutputDebugStringA(("[Client][WorldObject] V1 source bone '" + attachment.strRuntimeBoneName +
                            "' is absent on " + resource->modelAssetId + "; slot '" + attachment.strRuntimeAnchorSlotId +
                            "' follows the object pivot (" + effect.resourceId + ")\n").c_str());
                        break;
                    }
                }
            }
        if (sequence)
            for (const auto& collider : sequence->colliderTracks)
                if (collider.slotId == binding.slotId && !collider.attachmentBone.empty() &&
                    !model->second.model->Has_Bone(collider.attachmentBone.c_str()))
                { m_Status = "World Object collider bone is unavailable: " + collider.attachmentBone; return false; }
        Remember_SharedObjectModel(*resource, model->second, targets);
    }
    return true;
}

bool_t CWorldSequencePlayer::Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets)
{
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance || !instance->enabled)
    { m_Status = "World Object state is absent or disabled: " + instanceId; return false; }
    if (!targets.Is_Complete())
    { m_Status = "World Object runtime targets are unavailable."; return false; }
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        uint64_t targetId = 0;
        if (!Try_ParseTargetId(binding, targetId) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
                !Find_Placement(*targets.pPlacements, targetId)) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT &&
                !targets.pDeployRuntime->Find(targetId)))
        { m_Status = "World Object placement is outside the active map scope: " + binding.targetId; return false; }
    }
    return Prepare_ObjectMotionChain(*instance, targets);
}

bool_t CWorldSequencePlayer::Prewarm_ObjectInstances(const std::string& instanceId,
    const uint32_t copies, const TARGET_SET& targets)
{
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled || instance->anchorKind != "WORLD" ||
        instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        sequence->objectMotion.EmissionCount() == 0u || sequence->objectMotion.EmissionCount() > 128u || copies == 0u || copies > 128u ||
        (targets.objectPreparationOwner && targets.objectPreparationOwner != this))
    { m_Status = "World Object prewarm requires its owner, one enabled WORLD object binding, 1..128 emissions and copies: " + instanceId; return false; }
    if (!Prepare_InstanceResources(instanceId, targets)) return false;
    const auto& objectId = instance->bindings.front().targetId;
    const auto model = m_ObjectModels.find(objectId);
    if (model == m_ObjectModels.end())
    { m_Status = "World Object prewarm model is unavailable: " + objectId; return false; }
    const auto found = m_PreparedObjectPools.find(objectId);
    auto pool = found == m_PreparedObjectPools.end() ? std::make_shared<PREPARED_OBJECT_POOL>() : found->second;
    if (found != m_PreparedObjectPools.end() && (!pool->acceptsReturns || pool->levelIndex != targets.levelIndex))
    { m_Status = "World Object prewarm belongs to another level; reload its owner: " + objectId; return false; }
    if (pool->capacity >= copies)
    { m_Status = "World Object clones already prepared: " + objectId + " / " + std::to_string(pool->capacity); return true; }
    size_t total = copies - pool->capacity;
    for (const auto& [id, prepared] : m_PreparedObjectPools) total += prepared->capacity;
    if (total > 1024u) { m_Status = "World Object prepared clone budget reached (1024)."; return false; }
    pool->idle.reserve(copies);
    std::vector<shared_ptr<CWorldSequenceObject>> staged;
    const auto rollback = [&]() {
        for (auto& object : staged)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
    };
    for (uint32_t index = pool->capacity; index < copies; ++index)
    {
        CWorldSequenceObject::DESC desc;
        desc.levelIndex = targets.levelIndex;
        desc.modelPrototype = model->second.model;
        desc.diffuseTexture = model->second.diffuse;
        Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
        shared_ptr<CGameObject> created;
        const HRESULT result = CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
            CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex, CWorldSequenceObject::LAYER_TAG, &desc, &created);
        auto object = dynamic_pointer_cast<CWorldSequenceObject>(created);
        if (FAILED(result) || !object)
        {
            if (created) CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, created);
            rollback();
            m_Status = "World Object prewarm clone/shader creation failed: " + objectId +
                " / copy " + std::to_string(index + 1u) + " / HRESULT " + std::to_string(static_cast<int32_t>(result));
            return false;
        }
        object->Hide();
        staged.push_back(std::move(object));
    }
    pool->idle.insert(pool->idle.end(), staged.begin(), staged.end());
    pool->levelIndex = targets.levelIndex;
    pool->capacity = copies;
    m_PreparedObjectPools.insert_or_assign(objectId, std::move(pool));
    m_Status = "World Object hidden clones prepared: " + objectId + " / " + std::to_string(copies);
    return true;
}

bool_t CWorldSequencePlayer::Prewarm_HiddenObjectPose(const std::string& instanceId,
    const f32_t elapsedMs, const TARGET_SET& targets)
{
    CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "WorldSequence.HiddenPose.Prewarm");
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled || instance->anchorKind != "WORLD" ||
        instance->motionEnd != WORLD_SEQUENCE_MOTION_END::HOLD || instance->bindings.size() != 1u ||
        instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        sequence->objectMotion.EmissionCount() != 1u || !sequence->effectTracks.empty() ||
        !std::isfinite(elapsedMs) || elapsedMs < instance->startDelayMs ||
        !std::isfinite(instance->playbackSpeed) || instance->playbackSpeed <= 0.f ||
        targets.objectPreparationOwner != this)
    { m_Status = "Hidden pose prewarm requires one prepared WORLD HOLD model without Effect tracks: " + instanceId; return false; }
    const auto& binding = instance->bindings.front();
    const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
    const auto pool = m_PreparedObjectPools.find(binding.targetId);
    if (!resource || pool == m_PreparedObjectPools.end() || !pool->second->acceptsReturns ||
        pool->second->levelIndex != targets.levelIndex || pool->second->idle.empty())
    { m_Status = "Hidden pose prewarm has no idle prepared clone: " + instanceId; return false; }
    const f32_t localMs = (std::min)(static_cast<f32_t>(sequence->durationMs),
        (elapsedMs - instance->startDelayMs) * instance->playbackSpeed);
    const f32_t ageMs = localMs - sequence->objectMotion.EmissionDelayMs(0u);
    if (!std::isfinite(ageMs) || ageMs < 0.f)
    { m_Status = "Hidden pose prewarm precedes the object birth: " + instanceId; return false; }
    ACTIVE_INSTANCE active; active.instanceId = instanceId;
    PLAYER_ANCHOR anchor; XMStoreFloat4x4(&anchor.world, XMMatrixIdentity());
    float4x4_t world;
    if (!Sample_ObjectWorld(active, *instance, *sequence, *resource, binding.slotId, anchor, 0u, ageMs, world, m_Status,
        true, targets.objectWorldPostTransform)) return false;
    f32_t windowEnd = 0.f;
    const auto* animation = Find_AnimationTrackAt(*sequence, binding.slotId, ageMs, windowEnd);
    // Evaluate the actual animation on the clones which combat will borrow.
    // They remain idle and hidden; no playback, sound, Effect or scene visibility is committed.
    for (const auto& object : pool->second->idle)
    {
        if (!object || !object->Sample(world, false, animation, ageMs, windowEnd))
        { m_Status = "Hidden pose prewarm sample failed: " + instanceId; return false; }
    }
    m_Status = "Hidden endpoint pose prepared: " + instanceId;
    return true;
}

void CWorldSequencePlayer::Clear_PreparedObjects()
{
    for (auto& [id, pool] : m_PreparedObjectPools)
    {
        // Borrowed clones keep this token alive. Once the owner resets, they
        // remove themselves on release instead of returning to an obsolete pool.
        pool->acceptsReturns = false;
        for (auto& object : pool->idle)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(pool->levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
        pool->idle.clear();
    }
    m_PreparedObjectPools.clear();
}

void CWorldSequencePlayer::Release_Objects(ACTIVE_INSTANCE& active)
{
    for (const auto& sound : active.sounds) CGameInstance::Get().Stop_SoundCue(sound.handle);
    active.sounds.clear();
    active.objectColliderSamples.clear();
    for (const auto& effect : active.effects)
    {
        if (effect.handle) CEffectV2Runtime::Stop_Group(effect.handle);
        if (effect.v1Handle) CEffectPresentationService::Stop_WorldRoot({effect.v1Handle});
    }
    active.effects.clear();
    active.emissionAnchors.clear();
    for (auto& entry : active.objects)
    {
        entry.weaponReplacement.reset();
        entry.hatReplacement.reset();
        entry.npcPreviewSuppression.reset();
        if (entry.object)
        {
            entry.object->Hide();
            auto& pool = entry.preparationPool;
            if (pool && pool->acceptsReturns && pool->levelIndex == entry.levelIndex && pool->idle.size() < pool->capacity)
            {
                if (entry.object->Reset_ForReuse())
                {
                    pool->idle.push_back(std::move(entry.object));
                    continue;
                }
                --pool->capacity;
                m_Status = "World Object prepared clone could not return to its pool: " +
                    (entry.object->Get_RenderStatus().empty() ? entry.slotId : entry.object->Get_RenderStatus());
            }
            CGameInstance::Get().Remove_GameObject_from_Layer(entry.levelIndex,
                CWorldSequenceObject::LAYER_TAG, entry.object);
        }
    }
    active.objects.clear();
}

bool_t CWorldSequencePlayer::Try_GetPresentationBossAnchor(const std::string& archetype,
    const std::string& bone, PLAYER_ANCHOR& out, std::string& status) const
{
    status.clear();
    const CWorldSequenceObject* selected = nullptr;
    bool declared = false;
    for (const auto& active : m_Active)
    {
        const auto* instance = m_Document.Find_Instance(active.instanceId);
        if (!instance) continue;
        for (const auto& binding : instance->bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
            const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
            if (!resource) continue;
            const auto* actor = CActorCatalog::Find_Boss(archetype);
            const bool derivedActor = actor && resource->animated &&
                resource->materialSourceModelAssetId == actor->bodyModel &&
                std::abs(resource->modelPreScale - actor->bodyModelPreScale) < .0000001f;
            if (resource->presentationBossArchetypeId != archetype && !derivedActor) continue;
            declared = true;
            for (const auto& entry : active.objects)
            {
                if (entry.slotId != binding.slotId || !entry.object || !entry.object->Is_Visible()) continue;
                if (selected && selected != entry.object.get())
                { status = "Cinematic World boss anchor is ambiguous: " + archetype; return false; }
                selected = entry.object.get();
            }
        }
    }
    if (!selected)
    {
        if (declared) status = "Cinematic World boss anchor is waiting for its sampled actor: " + archetype;
        return false;
    }
    if (!Resolve_BossBoneAnchor(selected->Get_Model(), selected->Get_SampledWorld(), bone, out, status)) return false;
    out.liveBossAnchor = true;
    return true;
}

bool_t CWorldSequencePlayer::Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out,
    const uint32_t emissionIndex, const std::string& bone, const bool_t boneRotation) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return false;
    const auto sample = [&](const CWorldSequenceObject& object) {
        const auto& root = object.Get_SampledWorld();
        if (bone.empty()) { out = root; return true; }
        const auto model = object.Get_Model();
        if (!model || !model->Has_Bone(bone.c_str())) return false;
        const matrix_t objectWorld = XMLoadFloat4x4(&root);
        const matrix_t socket = model->Get_BoneMatrix(bone.c_str()) * objectWorld;
        const vector_t forward = boneRotation ? socket.r[2] : objectWorld.r[2];
        const float x = XMVectorGetX(forward), z = XMVectorGetZ(forward);
        if (!std::isfinite(x) || !std::isfinite(z) || (!boneRotation && x*x + z*z < 0.00000001f)) return false;
        const float sx = XMVectorGetX(XMVector3Length(objectWorld.r[0]));
        const float sy = XMVectorGetX(XMVector3Length(objectWorld.r[1]));
        const float sz = XMVectorGetX(XMVector3Length(objectWorld.r[2]));
        matrix_t pivot = XMMatrixRotationY(std::atan2(x, z));
        if (boneRotation)
        {
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (XMVectorGetX(XMVector3LengthSq(socket.r[axis])) < 0.00000001f) return false;
                pivot.r[axis] = XMVector3Normalize(socket.r[axis]);
            }
        }
        pivot = XMMatrixScaling(sx, sy, sz) * pivot;
        pivot.r[3] = XMVectorSetW(socket.r[3], 1.f);
        XMStoreFloat4x4(&out, pivot);
        const auto* values = reinterpret_cast<const float*>(&out);
        return std::all_of(values, values + 16u, [](float value) { return std::isfinite(value); });
    };
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = nullptr != instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (nullptr == sequence || sequence->objectMotion.emissions.empty())
    {
        if (0u != emissionIndex || active->objects.size() != 1 || !active->objects.front().object ||
            !active->objects.front().object->Is_Visible()) return false;
        return sample(*active->objects.front().object);
    }
    // An authored row is one clone per anchor; a WORLD motion has exactly one anchor.
    const CWorldSequenceObject* found = nullptr;
    for (const auto& entry : active->objects)
    {
        if (entry.emissionIndex != emissionIndex || !entry.object) continue;
        if (found) return false;
        found = entry.object.get();
    }
    if (!found || !found->Is_Visible()) return false;
    return sample(*found);
}

void CWorldSequencePlayer::Collect_ObjectColliderSamples(std::vector<OBJECT_COLLIDER_SAMPLE>& out) const
{
    out.clear();
    for (const auto& active : m_Active)
        out.insert(out.end(), active.objectColliderSamples.begin(), active.objectColliderSamples.end());
}

std::string CWorldSequencePlayer::Get_ObjectSampleStatus(const std::string& instanceId) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return "World Object preview is not active.";
    if (!active->objectSampleStatus.empty()) return active->objectSampleStatus;
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance) return "World Object preview instance is unavailable.";
    const bool hasObjects = std::any_of(instance->bindings.begin(), instance->bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
    if (!hasObjects) return "Placed World Object state sampled at its authored map placement.";
    size_t visible = 0;
    const CWorldSequenceObject* first = nullptr;
    for (const auto& entry : active->objects)
        if (entry.object && entry.object->Is_Visible())
        { ++visible; if (!first) first = entry.object.get(); }
    if (!first) return "World Object: 0 visible (outside lifetime or hidden by the current key).";
    const auto& world = first->Get_SampledWorld();
    std::ostringstream status;
    status << "World Object: " << visible << " visible, first position (" << std::fixed << std::setprecision(2)
        << world._41 << ", " << world._42 << ", " << world._43 << ").";
    return status.str();
}

bool_t CWorldSequencePlayer::Get_EmissionAnchor(ACTIVE_INSTANCE& active, const TARGET_SET& targets,
    const std::string& key, const f32_t birthMs, const PLAYER_ANCHOR& baseline, PLAYER_ANCHOR& out)
{
    out = baseline;
    if (!targets.objectEmissionAnchor || baseline.liveBossAnchor) return true;
    const auto existing = active.emissionAnchors.find(key);
    if (existing != active.emissionAnchors.end()) { out = existing->second; return true; }
    if (!std::isfinite(birthMs) || birthMs < 0.f || !targets.objectEmissionAnchor(birthMs, out.world))
    { m_Status = "World Object emission origin is unavailable at " + std::to_string(birthMs) + " ms."; return false; }
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { m_Status = "World Object emission origin is not finite."; return false; }
    const matrix_t world = XMLoadFloat4x4(&out.world);
    if (std::abs(out.world._14) > .00001f || std::abs(out.world._24) > .00001f ||
        std::abs(out.world._34) > .00001f || std::abs(out.world._44 - 1.f) > .00001f ||
        std::abs(XMVectorGetX(XMMatrixDeterminant(world))) < .000001f)
    { m_Status = "World Object emission origin is not an invertible affine transform."; return false; }
    out.emissionOverride = true;
    active.emissionAnchors.emplace(key, out);
    return true;
}

bool_t CWorldSequencePlayer::Sample_ObjectWorld(const ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& slotId,
    const PLAYER_ANCHOR& anchor, const uint32_t emitter, const f32_t ageMs, float4x4_t& out, std::string& status,
    const bool_t inheritObjectRotation,
    const decltype(TARGET_SET::objectWorldPostTransform)& postTransform)
{
    const auto& motion = sequence.objectMotion;
    const auto* track = Find_Track(sequence, slotId);
    const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
    const float seconds = ageMs * 0.001f;
    uint32_t random = motion.seed ^ ((emitter + 1u) * 0x9e3779b9u);
    const auto randomUnit = [&random]()
    { random ^= random << 13; random ^= random >> 17; random ^= random << 5;
        return static_cast<float>(random & 0xffffffu) / 16777215.f; };
    const float spread = XMConvertToRadians(motion.spreadDegrees);
    const matrix_t direction = sequence.effectTracks.empty() ?
        XMMatrixRotationRollPitchYaw((randomUnit() - .5f) * spread, (randomUnit() - .5f) * spread * 2.f, 0.f) :
        XMMatrixRotationY((randomUnit() - .5f) * spread);
    const vector_t velocity = XMVector3TransformNormal(XMLoadFloat3(&motion.velocity), direction);
    // Sample after the existing velocity draws so zero extents preserve old paths.
    // Seed + emitter remain stable across every age sample and attached Effect.
    const float spawnX = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.x;
    const float spawnY = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.y;
    const float spawnZ = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.z;
    const vector_t spawnOffset = XMVectorSet(spawnX, spawnY, spawnZ, 0.f);
    const matrix_t revolution = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(motion.revolutionDegreesPerSecond.x * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.y * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.z * seconds));
    const vector_t orbit = XMLoadFloat3(&motion.revolutionOffset);
    const bool hasAuthoredEmissions = !motion.emissions.empty();
    const bool hasInstanceOffset = !((active.placement && instance.anchorKind == "WORLD") || anchor.emissionOverride);
    // Legacy emitters keep their existing addition order. Authored rows rotate
    // their local motion first, then add the instance's untranslated ring centre.
    const vector_t instanceOffset = hasInstanceOffset ? XMLoadFloat3(&instance.position) : XMVectorZero();
    const vector_t position = (hasAuthoredEmissions ? XMVectorZero() : instanceOffset) + XMLoadFloat3(&key.positionOffset) + spawnOffset +
        velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
        XMVector3TransformNormal(orbit, revolution) - orbit;
    const vector_t scale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    matrix_t basis = XMLoadFloat4x4(&anchor.world);
    for (int axis = 0; axis < 3; ++axis)
    {
        const float length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
        if (!std::isfinite(length) || length < 1e-6f)
        { status = "World Object anchor transform is invalid: " + resource.objectId; return false; }
        basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
    }
    // Floor colliders use emission + placement facing, without mesh upright correction or self-spin.
    // Attached effects can share that basis without changing the visible model sample.
    const matrix_t rotation = inheritObjectRotation ? XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(motion.angularVelocityDegrees.x * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.y * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.z * seconds)) : XMMatrixIdentity();
    matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position);
    /* An authored row turns the whole local motion, orbit and seeded spawn
       offset included, before the existing placement / live-anchor composition. */
    if (hasAuthoredEmissions)
    {
        const auto& emission = motion.emissions[(std::min)(static_cast<size_t>(emitter), motion.emissions.size() - 1u)];
        world *= XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&emission.positionOffset));
        if (hasInstanceOffset)
            world *= XMMatrixTranslationFromVector(instanceOffset);
    }
    const bool localPlacement = active.placement && (instance.anchorKind == "BOSS" || instance.anchorKind == "PLAYER");
    if (!anchor.emissionOverride && !localPlacement) world *= basis;
    if (active.placement)
    {
        const auto& placement = *active.placement;
        world *= XMMatrixScalingFromVector(XMLoadFloat3(&placement.scale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(placement.rotationDegrees.x),
                XMConvertToRadians(placement.rotationDegrees.y), XMConvertToRadians(placement.rotationDegrees.z)) *
            (anchor.emissionOverride ? XMMatrixIdentity() : XMMatrixTranslationFromVector(XMLoadFloat3(&placement.position)));
    }
    else if (!anchor.emissionOverride) world.r[3] += XMVectorSet(active.positionOffset.x, active.positionOffset.y, active.positionOffset.z, 0.f);
    if (anchor.emissionOverride || localPlacement) world *= basis;
    if (postTransform)
    {
        float4x4_t post;
        if (!Sample_ObjectPresentationPostTransform(postTransform, instance.instanceId, ageMs, post, status)) return false;
        world *= XMLoadFloat4x4(&post);
    }
    XMStoreFloat4x4(&out, world);
    return true;
}

bool_t CWorldSequencePlayer::Apply_Objects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const TARGET_SET& targets, const f32_t localMs, const bool_t visible, const bool_t holdFinalPose,
    const f32_t emissionStartMs, const f32_t emissionRate, const std::string& emissionMotionId)
{
    active.objectSampleStatus.clear();
    active.objectColliderSamples.clear();
    std::vector<OBJECT_COLLIDER_SAMPLE> colliderSamples;
    for (auto& entry : active.objects)
    {
        entry.object->Hide();
        entry.npcPreviewSuppression.reset();
    }
    if (!visible || std::none_of(instance.bindings.begin(), instance.bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; })) return true;
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (!resource || !targets.bossAnchor || !targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus))
        {
            if (active.objectSampleStatus.empty()) active.objectSampleStatus = "World Object Boss anchor is unavailable.";
            m_Status = active.objectSampleStatus;
            return true;
        }
        anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
        if (anchors.empty())
        {
            active.objectSampleStatus = "World Object Character anchor is waiting for a living replicated player.";
            return true;
        }
    }
    else
    {
        PLAYER_ANCHOR world;
        XMStoreFloat4x4(&world.world, XMMatrixIdentity());
        anchors.push_back(world);
    }
    const auto& motion = sequence.objectMotion;
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        const auto model = m_ObjectModels.find(binding.targetId);
        const auto* track = Find_Track(sequence, binding.slotId);
        if (!resource || model == m_ObjectModels.end())
        { m_Status = "World Object model was not prepared: " + binding.targetId; return false; }
        for (const auto& anchor : anchors)
            for (uint32_t emitter = 0; emitter < motion.EmissionCount(); ++emitter)
            {
                const f32_t delayMs = static_cast<f32_t>(motion.EmissionDelayMs(emitter));
                const f32_t ageMs = localMs - delayMs;
                if (ageMs < 0.f || (!holdFinalPose && !sequence.effectTracks.empty() && ageMs >= sequence.durationMs)) continue;
                const f32_t birthMs = emissionStartMs + delayMs / emissionRate;
                if (!sequence.effectTracks.empty() && active.durationMs && birthMs >= active.durationMs) continue;
                PLAYER_ANCHOR emissionAnchor;
                const auto emissionKey = emissionMotionId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                auto found = std::find_if(active.objects.begin(), active.objects.end(), [&](const auto& value)
                { return value.slotId == binding.slotId && value.entityId == anchor.entityId && value.emissionIndex == emitter; });
                if (found == active.objects.end())
                {
                    size_t total = 0;
                    for (const auto& value : m_Active) total += value.objects.size();
                    if (total >= 1024u) { m_Status = "World Object instance budget reached (1024)."; return false; }
                    shared_ptr<CWorldSequenceObject> object;
                    std::shared_ptr<PREPARED_OBJECT_POOL> pool;
                    const auto* prepared = Find_SharedObjectModel(*resource, targets);
                    if (prepared && prepared->model == model->second.model)
                    {
                        const auto available = targets.objectPreparationOwner->m_PreparedObjectPools.find(resource->objectId);
                        if (available != targets.objectPreparationOwner->m_PreparedObjectPools.end() &&
                            available->second->acceptsReturns && available->second->levelIndex == targets.levelIndex &&
                            !available->second->idle.empty())
                        {
                            pool = available->second;
                            object = std::move(pool->idle.back());
                            pool->idle.pop_back();
                        }
                    }
                    if (!object)
                    {
                        CWorldSequenceObject::DESC desc;
                        desc.levelIndex = targets.levelIndex;
                        desc.modelPrototype = model->second.model;
                        desc.diffuseTexture = model->second.diffuse;
                        Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
                        shared_ptr<CGameObject> staged;
                        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
                            CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex,
                            CWorldSequenceObject::LAYER_TAG, &desc, &staged)))
                        { m_Status = "World Object clone/shader creation failed: " + resource->objectId; return false; }
                        object = dynamic_pointer_cast<CWorldSequenceObject>(staged);
                        if (!object)
                        {
                            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, staged);
                            m_Status = "World Object clone type is invalid: " + resource->objectId; return false;
                        }
                    }
                    active.objects.push_back({binding.slotId, anchor.entityId, emitter, targets.levelIndex,
                        std::move(object), std::move(pool)});
                    found = active.objects.end() - 1;
                }
                const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
                float4x4_t stored;
                if (!Sample_ObjectWorld(active, instance, sequence, *resource, binding.slotId, emissionAnchor, emitter,
                    (std::min)(ageMs, static_cast<f32_t>(sequence.durationMs)), stored, m_Status,
                    true, targets.objectWorldPostTransform)) return false;
                f32_t windowEnd = 0.f;
                const auto* animation = Find_AnimationTrackAt(sequence, binding.slotId, ageMs, windowEnd);
                if (!found->object)
                { m_Status = "World Object clone type is invalid: " + resource->objectId; return false; }
                if (!found->object->Get_RenderStatus().empty())
                { m_Status = found->object->Get_RenderStatus() + " / " + resource->objectId; return false; }
                for (const auto& material : sequence.materialTracks)
                {
                    if (material.slotId != binding.slotId) continue;
                    MODEL_SOURCE_CHARACTER_PARAMETERS parameters;
                    const auto& objectModel = found->object->Get_Model();
                    if (!resource->materialProfile || !objectModel ||
                        !CWorldSequenceDocument::Try_SampleMaterialParameters(*resource->materialProfile, material, ageMs, parameters))
                    { m_Status = "World Object native material sample failed: " + material.materialName; return false; }
                    if (!Apply_ObjectMaterialConstants(*objectModel, material.materialName, parameters, m_Status)) return false;
                }
                if (!found->object->Sample(stored, key.visible, animation, ageMs, windowEnd))
                { m_Status = "World Object transform/animation sample failed: " + resource->objectId; return false; }
                if (found->object->Is_Visible() && !binding.previewNpcPlacementId.empty())
                {
                    const auto npc = targets.previewNpc ? targets.previewNpc(binding.previewNpcPlacementId) : nullptr;
                    if (!npc)
                    {
                        found->object->Hide();
                        m_Status = "NPC preview requires its live placement: " + binding.previewNpcPlacementId;
                        return false;
                    }
                    npc->Acquire_CompositionPreviewSuppression();
                    found->npcPreviewSuppression = std::shared_ptr<const void>(npc.get(),
                        [owner = std::weak_ptr<CNpc>(npc)](const void*) {
                            if (const auto current = owner.lock()) current->Release_CompositionPreviewSuppression();
                        });
                }
                if (found->object->Is_Visible())
                    for (const auto& collider : sequence.colliderTracks)
                    {
#ifndef _DEBUG
                        // Release F1 only inspects the Bingo head; authoring bone previews stay Debug-only.
                        if (collider.colliderTrackId != "collider.bingo.hammer.head") continue;
#endif
                        if (collider.slotId != binding.slotId || ageMs < collider.startMs ||
                            ageMs >= static_cast<double>(collider.startMs) + collider.durationMs) continue;
                        OBJECT_COLLIDER_SAMPLE sample;
                        if (!Sample_ObjectCollider(collider, *resource, key, motion, emitter,
                            active.placement, *found->object, sample))
                        { m_Status = "World Object collider attachment sample failed: " + collider.colliderTrackId; return false; }
                        sample.instanceId = active.instanceId;
                        sample.colliderTrackId = collider.colliderTrackId;
                        sample.emissionIndex = emitter;
                        colliderSamples.push_back(std::move(sample));
                    }
                if (anchor.liveBossAnchor &&
                    (resource->objectId == "world.object.kouku.saydon_showtime_gun_left" ||
                     resource->objectId == "world.object.kouku.saydon_showtime_gun_right"))
                    CNpcPresentationAssetService::Track_SaydonWeaponReplacement(found->weaponReplacement,
                        anchor.bodyModel, found->object);
                else found->weaponReplacement.reset();
                if (anchor.liveBossAnchor && resource->objectId == "world.object.kouku.saydon_hat_right")
                    CNpcPresentationAssetService::Track_SaydonHatReplacement(found->hatReplacement,
                        anchor.bodyModel, found->object);
                else found->hatReplacement.reset();
            }
    }
    active.objectColliderSamples = std::move(colliderSamples);
    return true;
}

bool_t CWorldSequencePlayer::Apply_ObjectEffects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (resource && targets.bossAnchor && targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus)) anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
    }
    else
    {
        PLAYER_ANCHOR anchor;
        XMStoreFloat4x4(&anchor.world, XMMatrixIdentity());
        anchors.push_back(anchor);
    }
    std::unordered_set<std::string> wanted;
    // Resolve every still-visible event from the owning clock. This preserves an
    // earlier NEXT/LOOP tail and makes a direct seek equivalent to ordinary play.
    const auto sampleChain = [&](const std::string& firstId, const f32_t firstStartMs,
        const f32_t cutoffMs) -> bool_t
    {
        const auto* motion = m_Document.Find_Instance(firstId);
        f32_t start = firstStartMs;
        for (uint32_t depth = 0; motion && depth <= 32u; ++depth)
        {
            const auto* sequence = m_Document.Find_Template(motion->templateId);
            if (!sequence) return false;
            const f32_t rate = motion->playbackSpeed * active.playbackSpeed;
            start += motion->startDelayMs;
            const f32_t localMs = (active.elapsedMs - start) * rate;
            if (localMs < 0.f || start > cutoffMs) break;
            const f32_t period = static_cast<f32_t>(motion->CycleSpanMs(*sequence));
            for (const auto& effect : sequence->effectTracks)
            {
                // Each effect owns its declared slot, including a different
                // skeleton/scale in a multi-actor cinematic or a NEXT motion.
                const auto binding = std::find_if(motion->bindings.begin(), motion->bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                if (binding == motion->bindings.end())
                { m_Status = "World Object effect slot has no object binding: " + effect.slotId; return false; }
                const auto* resource = m_Document.Find_ObjectResource(binding->targetId);
                if (!resource)
                { m_Status = "World Object effect resource is unavailable: " + binding->targetId; return false; }
                const auto snapshot = m_EffectSnapshots.find(effect.resourceKind + ":" + effect.resourceId);
                if (effect.resourceKind != "V1_EFFECT" && snapshot == m_EffectSnapshots.end())
                { m_Status = "World Object effect was not prepared: " + effect.resourceId; return false; }
                const f32_t trigger = static_cast<f32_t>(sequence->EffectStartMs(effect));
                for (uint32_t emitter = 0; emitter < sequence->objectMotion.EmissionCount(); ++emitter)
                {
                    const f32_t birth = static_cast<f32_t>(sequence->objectMotion.EmissionDelayMs(emitter)) + trigger;
                    if (localMs < birth) continue;
                    const bool loop = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
                    const uint64_t last = loop ? static_cast<uint64_t>(std::floor((localMs - birth) / period)) : 0u;
                    const uint64_t first = loop ? static_cast<uint64_t>((std::max)(0.0,
                        std::floor((localMs - birth - effect.durationMs) / period) + 1.0)) : 0u;
                    if (last < first) continue;
                    if (last - first > 1024u)
                    { m_Status = "World Object Effect overlap exceeds 1024 occurrences; increase Motion Lifetime."; return false; }
                    for (uint64_t epoch = first; epoch <= last; ++epoch)
                    {
                        const f32_t eventLocal = birth + static_cast<f32_t>(epoch) * period;
                        const f32_t ageMs = localMs - eventLocal;
                        const f32_t birthMs = start + (eventLocal - trigger) / rate;
                        if (ageMs < 0.f || ageMs >= effect.durationMs || birthMs >= cutoffMs) continue;
                        for (const auto& anchor : anchors)
                        {
                            auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) + ":" +
                                effect.effectTrackId + ":" + std::to_string(epoch) + ":" + std::to_string(emitter) + ":" + std::to_string(anchor.entityId);
                            PLAYER_ANCHOR emissionAnchor;
                            const f32_t emissionStartMs = start + static_cast<f32_t>(epoch) * period / rate;
                            const auto emissionKey = motion->instanceId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                            if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                            const auto prepared = m_ObjectModels.find(resource->objectId);
                            if (prepared == m_ObjectModels.end())
                            { m_Status = "World Object Effect model was not prepared: " + resource->objectId; return false; }
                            const bool v1 = effect.resourceKind == "V1_EFFECT";
                            const auto document = !v1 ? nullptr : CEffectCatalog::Find_Loaded(effect.resourceId);
                            if (v1 && !document)
                            { m_Status = "World Object V1 Effect document was not prepared: " + effect.resourceId; return false; }
                            float effectTimeScale = 1.f;
                            if (effect.fitEffectToDuration)
                            {
                                float sourceDuration = 0.f;
                                if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.resourceId, sourceDuration) ||
                                    !CWorldSequenceDocument::Try_EffectTimeScale(effect, sourceDuration, effectTimeScale))
                                { m_Status = "World Object Effect fit needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                            }
                            double sourceCycleMs = 0.0;
                            uint64_t firstSourceCycle = 0u, lastSourceCycle = 0u;
                            bool nativeInfiniteLoop = false;
                            if (effect.loopEffectToDuration)
                            {
                                nativeInfiniteLoop = std::any_of(document->Elements.begin(), document->Elements.end(),
                                    [](const auto& element) { return element.SourceRecipe.bEnabled &&
                                        element.SourceRecipe.iEmitterLoopCount == 0u; });
                                if (!nativeInfiniteLoop)
                                {
                                    float sourceDuration = 0.f;
                                    if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.resourceId, sourceDuration) ||
                                        !std::isfinite(sourceDuration) || sourceDuration <= 0.f)
                                    { m_Status = "World Object Effect loop needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                                    // A prepared duration includes particle/after-image tails. Repeat
                                    // emission at its authored end, retaining each still-living cycle.
                                    double emissionSeconds = 0.0;
                                    for (const auto& element : document->Elements)
                                    {
                                        if (!element.bVisible) continue;
                                        const auto& recipe = element.SourceRecipe;
                                        const auto& timing = element.Detail.Timing;
                                        const double duration = recipe.bEnabled && recipe.fEmitterDurationSeconds > 0.f ?
                                            static_cast<double>(recipe.fEmitterDurationSeconds) * recipe.iEmitterLoopCount :
                                            timing.fLifeTimeSeconds;
                                        emissionSeconds = (std::max)(emissionSeconds,
                                            timing.fStartDelaySeconds + (recipe.bEnabled ? recipe.fEmitterDelaySeconds : 0.f) + duration);
                                    }
                                    for (const auto& cue : document->ModelCues)
                                        if (cue.bVisible) emissionSeconds = (std::max)(emissionSeconds,
                                            static_cast<double>(cue.fStartDelaySeconds) + cue.fDurationSeconds);
                                    if (!std::isfinite(emissionSeconds) || emissionSeconds <= 0.0)
                                    { m_Status = "World Object Effect loop has no finite emission window: " + effect.resourceId; return false; }
                                    sourceCycleMs = (std::min)(emissionSeconds, static_cast<double>(sourceDuration)) * 1000.0;
                                    const double lastSourceEpoch = std::floor(ageMs / sourceCycleMs);
                                    const double firstSourceEpoch = (std::max)(0.0,
                                        std::floor((ageMs - sourceDuration * 1000.0) / sourceCycleMs) + 1.0);
                                    if (!std::isfinite(lastSourceEpoch) ||
                                        lastSourceEpoch >= static_cast<double>((std::numeric_limits<uint64_t>::max)()) ||
                                        lastSourceEpoch - firstSourceEpoch >= 1024.0)
                                    { m_Status = "World Object Effect source tails exceed the bounded occurrence range."; return false; }
                                    firstSourceCycle = static_cast<uint64_t>(firstSourceEpoch);
                                    lastSourceCycle = static_cast<uint64_t>(lastSourceEpoch);
                                }
                            }
                            const auto occurrenceKey = key;
                            for (uint64_t sourceCycle = firstSourceCycle; sourceCycle <= lastSourceCycle; ++sourceCycle)
                            {
                                const float sourceCycleStartMs = static_cast<float>(sourceCycle * sourceCycleMs);
                                key = occurrenceKey;
                                if (sourceCycleMs > 0.0) key += ":source-cycle:" + std::to_string(sourceCycle);
                                wanted.insert(key);
                                auto found = std::find_if(active.effects.begin(), active.effects.end(),
                                    [&](const auto& value) { return value.key == key; });
                                const float sourceSeconds = (std::max)(0.f, ageMs - sourceCycleStartMs) * .001f * effectTimeScale;
                                ACTIVE_INSTANCE placementState;
                                placementState.positionOffset = active.positionOffset;
                                placementState.placement = active.placement;
                                // This provider outlives this stack frame in PresentationService. All
                                // inputs are owned values or immutable prepared model/document handles.
                                const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
                                    [placementState, owner = *motion, sequence = *sequence, resource = *resource,
                                     effect, emissionAnchor, emitter, trigger, effectTimeScale, sourceCycleStartMs,
                                     model = prepared->second.model, document, v1]
                                    (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error) -> bool_t
                                {
                                    output.SourceAnchorWorlds.clear();
                                    if (!std::isfinite(seconds) || seconds < 0.f) return false;
                                    const float sampleMs = (std::min)(trigger + (effect.followObject ?
                                        sourceCycleStartMs + seconds * 1000.f / effectTimeScale : 0.f),
                                        static_cast<float>(sequence.durationMs));
                                    float4x4_t objectWorld;
                                    if (!Sample_ObjectWorld(placementState, owner, sequence, resource, effect.slotId,
                                        emissionAnchor, emitter, sampleMs, objectWorld, error,
                                        effect.inheritObjectRotation)) return false;
                                    matrix_t pivot = XMLoadFloat4x4(&objectWorld);
                                    // Preserve existing V2 metre sizing; V1 shares the Object's
                                    // authored owner scale so both doll variants attach proportionally.
                                    if (!v1) for (int axis = 0; axis < 3; ++axis)
                                        pivot.r[axis] = XMVectorSetW(XMVector3Normalize(pivot.r[axis]), 0.f);
                                    if (!effect.bone.empty())
                                    {
                                        float4x4_t bone;
                                        if (!Sample_ObjectEffectBone(model, sequence, effect.slotId, effect.bone, sampleMs, bone, error)) return false;
                                        pivot = XMLoadFloat4x4(&bone) * pivot;
                                    }
                                    const matrix_t local = XMMatrixScalingFromVector(XMLoadFloat3(&effect.scale)) *
                                        XMMatrixRotationRollPitchYaw(XMConvertToRadians(effect.rotationDegrees.x),
                                            XMConvertToRadians(effect.rotationDegrees.y), XMConvertToRadians(effect.rotationDegrees.z)) *
                                        XMMatrixTranslationFromVector(XMLoadFloat3(&effect.positionOffset));
                                    XMStoreFloat4x4(&output.RootWorld, local * pivot);
                                    return !document || Sample_ObjectEffectAttachments(*document, model, sequence, effect.slotId,
                                        sampleMs, output.RootWorld, output.SourceAnchorWorlds, error);
                                };
                                EFFECT_FIXED_STEP_TRANSFORM_SAMPLE frame;
                                if (!provider(sourceSeconds, frame, m_Status)) return false;
                                // Retain raw birth/history transforms. WORLD particles must receive
                                // the current presentation transform after evaluating the whole frame.
                                std::optional<float4x4_t> presentationPost;
                                if (targets.objectWorldPostTransform)
                                {
                                    float4x4_t post;
                                    const float currentSourceMs = (std::min)(trigger + ageMs,
                                        static_cast<float>(sequence->durationMs));
                                    if (!Sample_ObjectPresentationPostTransform(targets.objectWorldPostTransform,
                                        motion->instanceId, currentSourceMs, post, m_Status)) return false;
                                    if (!XMMatrixIsIdentity(XMLoadFloat4x4(&post))) presentationPost = post;
                                }
                                // V2 consumes a current pivot, after its normal metre-size conversion.
                                if (!v1 && presentationPost)
                                    XMStoreFloat4x4(&frame.RootWorld, XMLoadFloat4x4(&frame.RootWorld) * XMLoadFloat4x4(&*presentationPost));
                                if (found == active.effects.end())
                                {
                                    size_t total = active.effects.size();
                                    for (const auto& value : m_Active) if (&value != &active) total += value.effects.size();
                                    if (total >= 1024u)
                                    { m_Status = "World Object Effect occurrence budget reached (1024)."; return false; }
                                    if (v1)
                                    {
                                        EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                                        spawn.iLevelIndex = targets.levelIndex;
                                        // Handle identity is process-local; no pointer is serialized.
                                        spawn.strPlacementId = "world-object:" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ":" + key;
                                        spawn.strEffectAssetId = effect.resourceId;
                                        spawn.RootWorld = frame.RootWorld;
                                        spawn.fInitialSampleTimeSeconds = sourceSeconds;
                                        // Finite sources keep their prepared document and repeat above. Native
                                        // EmitterLoops=0 sources use the shared bounded emission policy.
                                        spawn.fSourceLoopEndSeconds = nativeInfiniteLoop ? effect.durationMs * .001f : 0.f;
                                        spawn.bExternallySampled = true;
                                        spawn.bExternalModelCueAnchors = !document->ModelCues.empty();
                                        EFFECT_WORLD_ROOT_HANDLE handle;
                                        if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_Status)) return false;
                                        /* The normal runtime keeps new requests pending until MainApp finishes
                                           Object Manager update. MapTool runs after that seam and seeks this
                                           exact frame, so commit only this editor-owned world root now. */
                                        if (targets.bCommitWorldRootEffectsAfterSpawn)
                                            CEffectPresentationService::Commit_PendingWorldRootSpawns({handle});
                                        active.effects.push_back({key, 0u, handle.iValue, document});
                                    }
                                    else
                                    {
                                        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                                        playback.PivotWorld = frame.RootWorld;
                                        playback.bExternalClock = true;
                                        playback.bProductOwned = true;
                                        // Sample_Group receives authored seconds; Motion speed is already applied.
                                        playback.fDurationSeconds = -1.f;
                                        const uint32_t handle = effect.resourceKind == "GROUP" ?
                                            CEffectV2Runtime::Play_Group(*snapshot->second->Find_Group(effect.resourceId), snapshot->second,
                                                playback, targets.device, targets.context) :
                                            CEffectV2Runtime::Play_Leaf(effect.resourceId, snapshot->second, playback, targets.device, targets.context);
                                        if (!handle)
                                        { m_Status = "World Object effect play failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                        active.effects.push_back({key, handle});
                                    }
                                    found = active.effects.end() - 1;
                                }
                                if (v1)
                                {
                                    const bool placementEdited = found->sampledPlacement != active.placement ||
                                        found->sampledPositionOffset.x != active.positionOffset.x ||
                                        found->sampledPositionOffset.y != active.positionOffset.y ||
                                        found->sampledPositionOffset.z != active.positionOffset.z;
                                    if (!CEffectPresentationService::Update_WorldRoot({found->v1Handle}, frame.RootWorld) ||
                                        !CEffectPresentationService::Seek_WorldRoot({found->v1Handle}, sourceSeconds, provider,
                                            placementEdited, 0.f, nullptr, nullptr, presentationPost ? &*presentationPost : nullptr))
                                    { m_Status = "World Object V1 effect sample failed: " + effect.resourceId + " / " + CEffectPresentationService::Get_Status(); return false; }
                                    found->sampledPlacement = active.placement;
                                    found->sampledPositionOffset = active.positionOffset;
                                }
                                else
                                {
                                    CEffectV2Runtime::Set_GroupPivot(found->handle, frame.RootWorld);
                                    if (!CEffectV2Runtime::Sample_Group(found->handle, ageMs * .001f, true, targets.device, targets.context))
                                    { m_Status = "World Object effect sample failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                }
                            }
                        }
                    }
                }
            }
            if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
            start += period / rate;
            motion = m_Document.Find_Instance(motion->nextMotionId);
        }
        return true;
    };
    const f32_t end = active.durationMs ? static_cast<f32_t>(active.durationMs) : (std::numeric_limits<f32_t>::max)();
    const bool changed = !active.motionInstanceId.empty() && active.elapsedMs >= active.motionStartMs;
    if (!sampleChain(active.instanceId, 0.f, changed ? (std::min)(end, active.motionStartMs) : end) ||
        (changed && !sampleChain(active.motionInstanceId, active.motionStartMs, end))) return false;
    for (size_t index = 0; index < active.effects.size();)
    {
        if (wanted.contains(active.effects[index].key)) { ++index; continue; }
        if (active.effects[index].handle) CEffectV2Runtime::Stop_Group(active.effects[index].handle);
        if (active.effects[index].v1Handle) CEffectPresentationService::Stop_WorldRoot({active.effects[index].v1Handle});
        active.effects.erase(active.effects.begin() + static_cast<ptrdiff_t>(index));
    }
    return true;
}

bool_t Client::CWorldSequencePlayer::Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out,
 const uint32_t emissionIndex, const std::string& bone, const bool_t boneRotation) const
{
 if (Try_GetObjectPivot(instanceId, out, emissionIndex, bone, boneRotation)) return true;
 if (!bone.empty()) return false;
 // Placed map/deploy aliases have no emission rows; only row 0 can name them.
 if (0u != emissionIndex) return false;
 const auto* instance = Get_Document().Find_Instance(instanceId);
 if (!instance) return false;
 const WORLD_SEQUENCE_BINDING* binding = nullptr;
 // Preserve the existing map-alias choice; a Deploy-only sequence resolves its animated prop.
 for (const auto kind : {WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT, WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT})
 {
  for (const auto& candidate : instance->bindings)
  {
   if (candidate.targetKind != kind) continue;
   if (candidate.slotId == "object") { binding = &candidate; break; }
   if (binding) return false;
   binding = &candidate;
  }
  if (binding) break;
 }
 if (!binding) return false;
 uint64_t placementId = 0;
 if (!CWorldSequencePlayer::Try_ParseTargetId(*binding, placementId)) return false;
 if (binding->targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
 {
  const auto active = std::find_if(m_Active.begin(), m_Active.end(),
   [&](const auto& value) { return value.instanceId == instanceId; });
  if (active == m_Active.end()) return false;
  const auto sampled = active->sampledDeployPivots.find(placementId);
  if (sampled == active->sampledDeployPivots.end()) return false;
  out = sampled->second;
  return true;
 }
 MAP_PLACEMENT_RECORD record;
 if (!Try_GetSampledPlacementRecord(instanceId, placementId, record)) return false;
 XMStoreFloat4x4(&out, XMMatrixScaling(record.signedScale.x, record.signedScale.y, record.signedScale.z) *
  XMMatrixRotationQuaternion(XMLoadFloat4(&record.rotationQuaternion)) *
  XMMatrixTranslation(record.position.x, record.position.y, record.position.z));
 return true;
}

void CWorldSequencePlayer::Set_Paused(const bool_t paused)
{
    m_bPaused = paused;
    auto& audio = CGameInstance::Get();
    for (const auto& active : m_Active)
        for (const auto& sound : active.sounds) audio.Pause_SoundCue(sound.handle, paused);
    for (const auto& sound : m_RetiredSounds) audio.Pause_SoundCue(sound.handle, paused);
}

void CWorldSequencePlayer::Stop_RetiredSounds(const std::string& ownerId)
{
    for (auto at = m_RetiredSounds.begin(); at != m_RetiredSounds.end();)
    {
        if (!ownerId.empty() && at->ownerId != ownerId) { ++at; continue; }
        CGameInstance::Get().Stop_SoundCue(at->handle);
        at = m_RetiredSounds.erase(at);
    }
}

void CWorldSequencePlayer::Retire_Sounds(ACTIVE_INSTANCE& active)
{
    // Only handles survive natural visual completion. Camera, actor and control
    // clocks retain their authored duration and explicit Stop still owns cleanup.
    for (const auto& sound : active.sounds)
        if (!sound.loopToDuration && sound.handle && sound.endElapsedMs > active.elapsedMs)
            m_RetiredSounds.push_back({active.instanceId, sound.handle, sound.endElapsedMs - active.elapsedMs});
        else CGameInstance::Get().Stop_SoundCue(sound.handle);
    active.sounds.clear();
}

void CWorldSequencePlayer::Update_SoundTails(const f32_t timeDelta)
{
    if (m_bPaused || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
    for (auto at = m_RetiredSounds.begin(); at != m_RetiredSounds.end();)
    {
        if (m_SoundAudience && !m_SoundAudience(at->ownerId))
        { CGameInstance::Get().Stop_SoundCue(at->handle); at = m_RetiredSounds.erase(at); continue; }
        at->remainingMs -= timeDelta * 1000.f;
        if (at->remainingMs > 0.f && CGameInstance::Get().Is_SoundCueActive(at->handle)) { ++at; continue; }
        CGameInstance::Get().Stop_SoundCue(at->handle);
        at = m_RetiredSounds.erase(at);
    }
}

void CWorldSequencePlayer::Retire_InstanceSoundTails(const std::string& instanceId)
{
    for (auto& active : m_Active)
        if (active.instanceId == instanceId)
        {
            Retire_Sounds(active);
            active.soundPlaybackFinished = true;
        }
}

void CWorldSequencePlayer::Apply_Sounds(ACTIVE_INSTANCE& active)
{
    if (m_SoundAudience && !m_SoundAudience(active.instanceId))
    {
        for (const auto& sound : active.sounds) CGameInstance::Get().Stop_SoundCue(sound.handle);
        active.sounds.clear();
        active.seekSounds = false;
        return;
    }
    if (active.soundPlaybackFinished && !active.seekSounds) return;
    active.soundPlaybackFinished = false;
    std::unordered_set<std::string> wanted;
    auto& audio = CGameInstance::Get();
    const auto sampleChain = [&](const std::string& firstId, const f32_t firstStartMs, const f32_t cutoffMs)
    {
        const auto* motion = m_Document.Find_Instance(firstId);
        f32_t start = firstStartMs;
        for (uint32_t depth = 0u; motion && depth <= 32u; ++depth)
        {
            const auto* sequence = m_Document.Find_Template(motion->templateId);
            if (!sequence) break;
            const f32_t rate = motion->playbackSpeed * active.playbackSpeed;
            if (!std::isfinite(rate) || rate <= 0.f) break;
            start += motion->startDelayMs;
            const f32_t localMs = (active.elapsedMs - start) * rate;
            if (localMs < 0.f || start >= cutoffMs) break;
            const f32_t period = static_cast<f32_t>(motion->CycleSpanMs(*sequence));
            const bool loop = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
            for (const auto& row : sequence->soundTracks)
            {
                if (row.loopToDuration && (active.elapsedMs >= cutoffMs || (!loop && localMs >= period))) continue;
                if (localMs < row.startMs) continue;
                const uint64_t last = loop ? static_cast<uint64_t>(std::floor((localMs - row.startMs) / period)) : 0u;
                const uint64_t first = loop ? static_cast<uint64_t>((std::max)(0.0,
                    std::floor((localMs - row.startMs - row.durationMs) / period) + 1.0)) : 0u;
                if (last < first || last - first > 1024u) continue;
                for (uint64_t epoch = first; epoch <= last; ++epoch)
                {
                    const f32_t eventMs = row.startMs + static_cast<f32_t>(epoch) * period;
                    const f32_t ageMs = localMs - eventMs;
                    const f32_t birthMs = start + eventMs / rate;
                    if (ageMs < 0.f || ageMs >= row.durationMs || birthMs >= cutoffMs) continue;
                    const auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) +
                        ":" + row.soundTrackId + ":" + std::to_string(epoch);
                    wanted.insert(key);
                    auto found = std::find_if(active.sounds.begin(), active.sounds.end(),
                        [&](const auto& sound) { return sound.key == key; });
                    if (found != active.sounds.end() && active.seekSounds)
                    {
                        audio.Stop_SoundCue(found->handle);
                        active.sounds.erase(found);
                        found = active.sounds.end();
                    }
                    if (found == active.sounds.end())
                    {
                        const auto path = CRuntimeAssetRoot::Resolve(row.assetId);
                        uint32_t mediaDurationMs = 0u;
                        const bool loopReady = !row.loopToDuration ||
                            audio.Get_SoundDurationMs(path.wstring(), mediaDurationMs);
                        const auto sampleMs = static_cast<uint32_t>(ageMs);
                        const auto cycle = row.loopToDuration && mediaDurationMs ? sampleMs / mediaDurationMs : 0u;
                        const auto offsetMs = row.loopToDuration && mediaDurationMs ? sampleMs % mediaDurationMs : sampleMs;
                        const auto handle = loopReady ? audio.Play_SoundCue(path.wstring(), row.volume,
                            offsetMs, m_bPaused, rate * m_ExternalSoundClockRate) : 0u;
                        // A missing cue is isolated and remembered, so a broken asset
                        // cannot retrigger file I/O or invalidate an otherwise valid scene.
                        active.sounds.push_back({key, handle, birthMs + row.durationMs / rate,
                            mediaDurationMs, cycle, row.loopToDuration});
                        if (!handle) m_Status = "World sequence sound unavailable: " + row.assetId;
                    }
                    else
                    {
                        const auto sampleMs = static_cast<uint32_t>(ageMs);
                        if (row.loopToDuration && found->mediaDurationMs &&
                            sampleMs / found->mediaDurationMs != found->mediaCycle)
                        {
                            // Stop the previous cycle before starting the current one.
                            // Clock modulo handles frame skips, seek and playback rate
                            // without accumulating drift or overlapping loop voices.
                            audio.Stop_SoundCue(found->handle);
                            found->mediaCycle = sampleMs / found->mediaDurationMs;
                            const auto path = CRuntimeAssetRoot::Resolve(row.assetId);
                            found->handle = audio.Play_SoundCue(path.wstring(), row.volume,
                                sampleMs % found->mediaDurationMs, m_bPaused, rate * m_ExternalSoundClockRate);
                        }
                        else audio.Set_SoundCuePlaybackRate(found->handle, rate * m_ExternalSoundClockRate);
                    }
                }
            }
            if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
            start += period / rate;
            motion = m_Document.Find_Instance(motion->nextMotionId);
        }
    };
    const f32_t end = active.durationMs ? static_cast<f32_t>(active.durationMs) : (std::numeric_limits<f32_t>::max)();
    const bool changed = !active.motionInstanceId.empty() && active.elapsedMs >= active.motionStartMs;
    sampleChain(active.instanceId, 0.f, changed ? (std::min)(end, active.motionStartMs) : end);
    if (changed) sampleChain(active.motionInstanceId, active.motionStartMs, end);
    for (auto at = active.sounds.begin(); at != active.sounds.end();)
    {
        if (wanted.contains(at->key)) { ++at; continue; }
        audio.Stop_SoundCue(at->handle);
        at = active.sounds.erase(at);
    }
    active.seekSounds = false;
}

void Client::CWorldSequencePlayer::Collect_VisibleObjects(
    std::vector<std::shared_ptr<CWorldSequenceObject>>& out) const
{
    const auto collect = [&](const auto& instances) {
        for (const auto& instance : instances)
            for (const auto& entry : instance.objects)
                if (entry.object && entry.object->Is_Visible()) out.push_back(entry.object);
    };
    collect(m_Active);
    collect(m_Held);
}

void Client::CWorldSequencePlayer::Collect_ObjectInspectionSamples(
    std::vector<OBJECT_INSPECTION_SAMPLE>& out) const
{
    const auto collect = [&](const auto& instances) {
        for (const auto& active : instances)
        {
            const auto* instance = m_Document.Find_Instance(active.instanceId);
            if (!instance) continue;
            for (const auto& entry : active.objects)
            {
                if (!entry.object) continue;
                const auto binding = std::find_if(instance->bindings.begin(), instance->bindings.end(),
                    [&entry](const auto& value) {
                        return value.slotId == entry.slotId &&
                            value.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
                    });
                if (binding == instance->bindings.end()) continue;
                const auto* resource = m_Document.Find_ObjectResource(binding->targetId);
                if (!resource) continue;
                out.push_back({active.instanceId, entry.slotId, resource->objectId,
                    resource->modelAssetId, entry.emissionIndex, entry.object});
            }
        }
    };
    collect(m_Active);
    collect(m_Held);
}
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Public/ClientReplication.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ClientReplicationEvent.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "CombatObjectProjectionRuntime.h"
#include "EstherActionSoundCueDocument.h"
#include "NetObjectRegistry.h"
#include "NpcPlacementPresentationService.h"
#include "MonsterPresentationContract.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldDestructionProjectionRuntime.h"
#include "ReplicatedPlayerHealth.h"
#include "CombatDebugVisibility.h"
#include "ValtanPresentationGenerationAdmission.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <deque>
#include <utility>
#include <vector>

//Network Event瑜??ㅼ젣 Engine GameObject ?앹꽦 ?쒓굅濡?踰덉뿭?섎뒗 ???섎굹??main-thread 寃쎄퀎

namespace Client
{
	class CCharacter;
	class CPlayableCharacterAssetService;
	class CNpc;
	class CValtan;
	class CWorldSequenceObject;
	class CDeployPropRuntime;
	struct VALTAN_PATTERN_SOUND_SOURCE_RECEIPT;

	/* Pure fail-closed admission state used by the native authoring harness and
	   the replicated primary Valtan consumer. Source saves may succeed while a
	   live presentation reload fails; that failure must remain visible and keep
	   Complete Play closed until the same authoritative consumer admits a later
	   reload (or the world is reset). */
	class CPrimaryValtanPresentationFreshnessGate final
	{
	public:
		void Admit(
			const LostArk::Shared::GameplayDataRevision& Revision,
			const std::string_view strStatus = {})
		{
			if (!Revision.Is_Valid())
			{
				Reject(
					"Authoritative primary Valtan presentation admission supplied no immutable revision.");
				return;
			}
			m_isFresh = true;
			m_Revision = Revision;
			m_strStatus.assign(strStatus);
		}

		void Reject(const std::string_view strDiagnostic)
		{
			m_isFresh = false;
			m_Revision = {};
			m_strStatus = strDiagnostic.empty() ?
				"Authoritative primary Valtan presentation reload failed." :
				std::string(strDiagnostic);
		}

		bool_t Can_Play(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus) const
		{
			if (ExpectedRevision.Is_Valid() && m_isFresh &&
				m_Revision == ExpectedRevision)
			{
				strOutStatus.clear();
				return true;
			}
			strOutStatus = !ExpectedRevision.Is_Valid() ?
				"Complete Play is blocked because no valid Server-active presentation revision was supplied." :
				(m_isFresh ?
					"Complete Play is blocked because the authoritative primary Valtan presentation cache belongs to a different immutable revision." :
					"Complete Play is blocked because the authoritative primary Valtan presentation cache is stale. " +
						m_strStatus);
			return false;
		}

	private:
		bool_t m_isFresh = false;
		LostArk::Shared::GameplayDataRevision m_Revision{};
		std::string m_strStatus;
	};

	// Level presentation이 읽는 복제 player snapshot이다. NetEntityId가 identity이며
	// nickname과 class는 Server spawn record에서만 오고 Character 수명은 소유하지 않는다.
	struct REPLICATED_PLAYER_VIEW
	{
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickname;
		/* Worn honor title from the latest snapshot (INVALID = none); the nameplate turns
		it into text through CHonorTitleCatalog. */
		LostArk::Shared::HONOR_TITLE_ID iHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
		bool_t isLocal = false;
		std::weak_ptr<CCharacter> pCharacter;
		LostArk::Shared::PLAYER_CONTROL_KIND eControlKind = LostArk::Shared::PLAYER_CONTROL_KIND::HUMAN;
	};

	struct DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW final
	{
		std::uint64_t iGeneration = 0u;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iServerTick = 0u;
	};

	enum class DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT : uint8_t
	{
		NO_PENDING,
		COMMITTED,
		RECOVERED_FAILURE,
		FATAL_FAILURE
	};

	struct VALTAN_PRESENTATION_STATE final
	{
		bool_t isValid = false;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iServerTick = 0u;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		std::string strArchetypeId;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStageIndex = 0u;
		LostArk::Shared::NET_ENTITY_ID iPatternTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PORTAL_RUSH_ROUTE_SNAPSHOT PortalRushRoute;
		std::uint32_t iActionStartTick = 0u;
		LostArk::Shared::BOSS_COMBAT_SNAPSHOT BossCombat;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
	};

	struct NPC_ACTION_EDGE_STATE final
	{
		std::string strLastActionId;
		std::uint32_t iLastActionStartTick = 0u;
		bool_t hasAppliedActionEdge = false;
	};

	struct NPC_ACTION_PLAYBACK_REQUEST final
	{
		std::string strClipName;
		bool_t isLoop = true;
		f32_t fPlaybackRate = 1.f;
		f32_t fBlendSeconds = 0.12f;
	};

	struct NPC_ACTION_PLAYBACK_RESULT final
	{
		bool_t isPrimaryPlayed = false;
		bool_t isIdleFallbackAttempted = false;
		bool_t isIdleFallbackPlayed = false;
	};

	/* Pure action-edge projection shared by product replication and its focused
	contract harness. Model playback stays injected so one bad optional binding
	falls back only that NPC to idle. Esther chains remain in the existing
	replication branch and are never flattened through this helper. */
	class CNpcActionPresentationRuntime final
	{
	public:
		static bool_t Is_NewActionEdge(
			const NPC_ACTION_EDGE_STATE& state,
			const std::string_view actionId,
			const std::uint32_t iActionStartTick)
		{
			return !state.hasAppliedActionEdge ||
				state.strLastActionId != actionId ||
				state.iLastActionStartTick != iActionStartTick;
		}

		static NPC_ACTION_PLAYBACK_REQUEST Resolve_Playback(
			const std::string_view actionId,
			const std::string& resolvedIdleClip,
			const NPC_PLACEMENT_PRESENTATION_ENTRY& placement,
			const std::map<
				std::string,
				std::vector<std::string>,
				std::less<>>* pLegacyActionClips)
		{
			NPC_ACTION_PLAYBACK_REQUEST request{};
			if (actionId.empty() || actionId == "npc.idle")
			{
				request.strClipName = resolvedIdleClip;
				return request;
			}
			if (actionId == "npc.move.walk")
			{
				request.strClipName = !placement.strWalkClip.empty() ?
					placement.strWalkClip : resolvedIdleClip;
				return request;
			}

			const auto authored = placement.ActionBindings.find(actionId);
			if (authored != placement.ActionBindings.end())
			{
				request.strClipName = authored->second.strClipName;
				request.isLoop = authored->second.isLoop;
				request.fPlaybackRate = authored->second.fPlaybackRate;
				request.fBlendSeconds = authored->second.fBlendSeconds;
				return request;
			}
			if (nullptr != pLegacyActionClips)
			{
				const auto legacy = pLegacyActionClips->find(actionId);
				if (legacy != pLegacyActionClips->end() &&
					!legacy->second.empty())
				{
					request.strClipName = legacy->second.front();
					request.isLoop = false;
				}
			}
			return request;
		}

		template <typename TRY_PLAY_ACTION, typename TRY_PLAY_IDLE>
		static NPC_ACTION_PLAYBACK_RESULT Apply_Playback(
			const NPC_ACTION_PLAYBACK_REQUEST& request,
			const std::string& resolvedIdleClip,
			TRY_PLAY_ACTION&& tryPlayAction,
			TRY_PLAY_IDLE&& tryPlayIdle,
			std::string& inOutCurrentClip)
		{
			NPC_ACTION_PLAYBACK_RESULT result{};
			result.isPrimaryPlayed = !request.strClipName.empty() &&
				std::forward<TRY_PLAY_ACTION>(tryPlayAction)(request);
			if (result.isPrimaryPlayed)
			{
				inOutCurrentClip = request.strClipName;
				return result;
			}

			result.isIdleFallbackAttempted = true;
			result.isIdleFallbackPlayed =
				std::forward<TRY_PLAY_IDLE>(tryPlayIdle)(0.12f);
			inOutCurrentClip = resolvedIdleClip;
			return result;
		}

		static void Commit_ActionEdge(
			NPC_ACTION_EDGE_STATE& state,
			const std::string_view actionId,
			const std::uint32_t iActionStartTick)
		{
			state.strLastActionId.assign(actionId);
			state.iLastActionStartTick = iActionStartTick;
			state.hasAppliedActionEdge = true;
		}
	};

	class CClientReplication final
	{
	public:
		//replication??character瑜??대뒓 layer? prototype???앹꽦?댁빞 ?섎뒗吏瑜??꾨떖?쒕떎.
		//rpelication??level baren???섎뱶肄붾뵫???꾩슂媛 ?녿떎.
		struct DESC
		{
			ComPtr<ID3D11Device> pDevice;
			ComPtr<ID3D11DeviceContext> pContext;
			std::uint32_t iPrototypeLevelIndex = 0;
			std::uint32_t iLayerLevelIndex = 0;
			/* Stable LevelRegistry area ID. Replication resolves the navigation
			   prototype that the Loader registered for this exact area and gives it
			   only to the locally controlled Character. Ground-target preview may
			   reject an invalid cell, while the Server still owns final admission. */
			std::string strMapAreaId;
			std::wstring strPlayerLayerTag;
			std::wstring strWorldEntityLayerTag;
			CDeployPropRuntime* pDeployPropRuntime = nullptr;
			const CWorldDestructionProjectionDocument*
				pWorldDestructionProjection = nullptr;
			bool_t bDeferLocalCharacterClassReplacement = false;
			/* Retire editor-only wall overrides before authoritative projection. */
			std::function<bool_t(std::string&)> beforeWorldDestructionProjection;
			/* Optional main-thread presentation edge. The reliable Server
			despawn remains authoritative; Levels may attach non-gameplay
			presentation such as a BGM transition without parsing packets. */
			std::function<void(
				std::string_view placementId,
				std::string_view archetypeId)> onWorldEntityDespawned;
		};

	public:
		CClientReplication();
		~CClientReplication();
		bool Initialize(const DESC& desc);
		bool Update();
		// Level input ownership gates this read-only, nearest-model hover projection.
		struct WORLD_COMBAT_TARGET final
		{
			LostArk::Shared::NET_ENTITY_ID iBodyNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::shared_ptr<CWorldSequenceObject> object;
		};
		void Set_WorldCombatTargets(const std::vector<WORLD_COMBAT_TARGET>& targets);
		bool Has_PendingConnectionLoss() const;
		bool Has_FatalWorldDestructionFailure() const
		{
			return m_hasFatalWorldDestructionFailure;
		}
		void Acknowledge_ConnectionLoss();
		void Reset();
		bool Has_WorldEntity(std::string_view archetypeId) const;
		bool Try_Consume_PresentationFailure(std::string& outStatus);
		/* Authoring reloads target the primary replicated Server-authoritative
		   Valtan, never only the Development preview returned by
		   CAnimationTargetService. A rejected active reload latches the freshness
		   gate consumed by Valtan Boss Tool Complete Play. */
		bool_t Reload_PrimaryValtanPresentationAuthoring(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus);
		bool_t Reload_PrimaryValtanCombatObjectSoundCues(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus);
		bool_t Can_Play_PrimaryValtanPresentation(
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			std::string& strOutStatus) const;
		bool_t Get_PrimaryValtanPatternSoundSourceReceipt(
			VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
			std::string& strOutStatus) const;
		bool Try_Consume_WorldDestructionLiveEvent(
			LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& outEvent);
		/* F1 Developer Tools writes one process-global snapshot. Every Level-owned
		   replication consumes it by revision, including while disconnected. */
		static COMBAT_DEBUG_VISIBILITY_SNAPSHOT
			Get_GlobalCombatDebugVisibility();
		static void Set_GlobalCombatDebugVisibility(
			const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility);

		std::shared_ptr<CCharacter> Get_LocalCharacter() const;
		std::shared_ptr<CValtan> Find_PrimaryValtanPresentation() const;
		/* Debug tuning only: the live CNpc body of one primary KoukuSaydon
		arena boss archetype, or null while that boss is not replicated. */
		std::shared_ptr<CNpc> Find_ArenaBossNpc(std::string_view archetypeId) const;
		std::shared_ptr<CNpc> Find_NpcPlacement(std::string_view placementId) const;
		bool_t Try_Get_DeferredLocalCharacterClassReplacement(
			DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW& OutView) const;
		DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT
			Commit_DeferredLocalCharacterClassReplacement();
		void Set_TargetedCombatPresentationPlayer(CKoukuSaydonPresentationPlayer* player)
        { m_pTargetedCombatPresentationPlayer = player; }
		void Collect_KoukuPresentationViews(std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
			std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const;
		void Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const;
		void Collect_PlayerViews(
			std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const;
		/* Minimap read model (CMinimapView): the local character's ground position and facing,
		   every other live player flagged by local party membership, and every live BOSS
		   entity. Client meters; yaw is clockwise degrees from +Z. Presentation only. */
		struct MINIMAP_MARKER
		{
			f32_t fX = 0.f;
			f32_t fZ = 0.f;
			bool_t bParty = false;
		};
		/* One received chat line: who said it and what they said, as the room
		   broadcast them. Order is arrival order. */
		struct CHAT_LINE
		{
			std::string strNickname;
			std::string strText;
		};
		struct MINIMAP_MARKER_SNAPSHOT
		{
			bool_t hasLocal = false;
			f32_t fLocalX = 0.f;
			f32_t fLocalZ = 0.f;
			f32_t fLocalYawDegrees = 0.f;
			std::vector<MINIMAP_MARKER> Players;
			std::vector<MINIMAP_MARKER> Bosses;
			/* Live NPC entities with their placement id, for the world map's function
			symbols (Data/UI/WorldMap/WorldMapNpcSymbols.json keys placement ids). */
			struct NPC_MARKER
			{
				f32_t fX = 0.f;
				f32_t fZ = 0.f;
				std::string strPlacementId;
			};
			std::vector<NPC_MARKER> Npcs;
		};
		void Collect_MinimapMarkers(MINIMAP_MARKER_SNAPSHOT& outSnapshot) const;
		const VALTAN_PRESENTATION_STATE& Get_ValtanPresentationState() const
		{
			return m_ValtanPresentationState;
		}
		uint64_t Get_WorldDestructionPresentationGeneration() const
		{
			return m_iWorldDestructionPresentationGeneration;
		}
		bool_t Is_WorldDestructionSynchronized() const
		{
			return m_WorldDestructionProjectionRuntime.Is_Synchronized();
		}
		uint32_t Get_WorldDestructionEncounterEpoch() const
		{
			return m_WorldDestructionProjectionRuntime.Get_EncounterEpoch();
		}
		uint32_t Get_WorldDestructionServerTick() const
		{
			return m_WorldDestructionProjectionRuntime.Get_ServerTick();
		}
		const std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE>&
		Get_WorldDestructionGroupStates() const
		{
			return m_WorldDestructionProjectionRuntime.Get_GroupStates();
		}
		/* Last Server-owned collision/navigation counters. Read-only; the Debug
		   panel reports them instead of deriving passage from wall states. */
		const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS&
		Get_WorldDestructionDiagnostics() const
		{
			return m_WorldDestructionDiagnostics;
		}
		/* Live pillar slot states as the Server last published them. The four
		   slots repeat, so this is a replace-in-full state, never a log. */
		const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC&
		Get_EncounterPropState() const
		{
			return m_EncounterPropState;
		}
		/* Debug-only inventory slice. Replace-in-full, same as
		   Get_EncounterPropState: the Server always answers with the whole
		   current inventory, never a delta. */
		const LostArk::Shared::S2C_INVENTORY_SNAPSHOT&
		Get_InventoryState() const
		{
			return m_InventoryState;
		}
		/* One-shot: true exactly once, the frame a party invite arrives (or a
		   newer one silently replaced an unconsumed older one -- see
		   Apply_PartyInviteReceived). */
		bool Try_Consume_PartyInviteReceived(
			LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& outInvite);
		/* Replace-in-full, same shape as Get_EncounterPropState/
		   Get_InventoryState. Empty Members means "not in a party". */
		const LostArk::Shared::S2C_PARTY_ROSTER&
		Get_PartyRoster() const
		{
			return m_PartyRoster;
		}
		const CReplicatedPlayerHealth& Get_PlayerHealth() const { return m_PlayerHealth; }
		/* Server-decided world sequence starts, in arrival order. The caller
		   takes them so one start is never played twice. */
		const LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& Get_KoukuBundleState() const { return m_KoukuBundleState; }
		std::uint32_t Get_LastServerTick() const { return m_iLastServerTick; }
		const LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& Get_KoukuRaidState() const { return m_KoukuRaidState; }
		const LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& Get_KoukuRaidReply() const { return m_KoukuRaidReply; }
		void Expect_KoukuRaidReply(std::uint32_t requestSequence);
		std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> Consume_WorldSequencePlays()
		{
			auto pending = std::move(m_PendingWorldSequencePlays);
			m_PendingWorldSequencePlays.clear();
			return pending;
		}
		/* The interact-gated box this player is standing in, or empty when the
		   Server has offered nothing. Held, not drained: the offer stands until
		   the Server withdraws it. */
		const std::string& Get_InteractPromptTriggerId() const
		{
			return m_strInteractPromptTriggerId;
		}
		bool Try_Consume_PartyTransferResult(
			LostArk::Shared::S2C_PARTY_TRANSFER_RESULT& outResult);
		// 파티 레이드 입장 투표. 프롬프트 수신 시 Bern이 수락/거절 창을 열고, vote는
		// 진행/종료 통지다. 각 한 번만 소비된다(read-only view).
		bool Try_Consume_RaidEntryPrompt(
			LostArk::Shared::S2C_RAID_ENTRY_PROMPT& outPrompt);
		bool Try_Consume_RaidEntryVote(
			LostArk::Shared::S2C_RAID_ENTRY_VOTE& outVote);
		/* Head-bubble text for whoever last chatted, while their line is still
		   within CHAT_BUBBLE_DURATION of arriving -- false (text left
		   untouched) once it has aged out, so the renderer only ever draws a
		   bubble that is still "live". */
		bool Try_Get_ActiveChatBubble(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			std::string& outText) const;
		/* Chat lines that arrived since the last drain, oldest first, each with
		   the sender's Server-replicated nickname. The room broadcasts back to
		   the sender too, so this is also where your own line comes from -- the
		   window keeps no separate local echo. */
		void Drain_ChatLines(std::vector<CHAT_LINE>& outLines);
		const LostArk::Shared::S2C_GUIDE_STATE* Get_GuideState() const
		{ return m_GuideState ? &*m_GuideState : nullptr; }

	private:
		void Sync_GlobalCombatDebugVisibility();
		void Apply_CombatDebugVisibility(
			const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility);
#ifdef _DEBUG
		struct COMBAT_OBJECT_HIT_AREA_DEBUG final
		{
			std::string strHitShape;
			bool_t bContact = false;
			f32_t fOuterRadiusM = 0.f;
			f32_t fInnerRadiusM = 0.f;
			f32_t fAngleDegrees = 0.f;
			f32_t fLengthM = 0.f;
			f32_t fHalfWidthM = 0.f;
			std::uint32_t iAtMs = 0u;
			std::uint32_t iRepeatCount = 0u;
			std::uint32_t iRepeatIntervalMs = 0u;
		};
		bool_t Load_CombatObjectHitAreaDebug(std::string& strOutStatus);
		void Draw_CombatObjectHitAreaDebug();
#endif
		/* The form picks the body: NORMAL is the class spec, CLOWN the
		KoukuSaydon colourless body on the same class (skills keep resolving
		through the class the desc carries). */
		bool Create_Character(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			LostArk::Shared::PLAYER_MADNESS_FORM madnessForm,
			std::string_view nickName,
			const float3_t& position,
			f32_t yawDegrees,
			bool_t isLocallyControlled,
			std::shared_ptr<CCharacter>& outCharacter);
		bool Apply_PlayerSnapshot(const LostArk::Shared::PLAYER_SNAPSHOT& player,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities);
		void Stage_PlayerPresentation(const LostArk::Shared::PLAYER_SNAPSHOT& player,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities);
		bool Advance_PlayerAssetPreparation();
		bool Commit_PlayerSpawn(const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);
		bool Apply_Spawn(
			const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);

		bool Apply_Despawn(
			const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned);
		bool Apply_WorldEntitySpawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned);
		bool Apply_WorldEntityDespawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned);
		bool_t Resolve_PrimaryValtan(
			std::string_view strArchetypeId,
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId,
			std::shared_ptr<CValtan>& pOutValtan,
			std::string& strOutStatus) const;
		void Remove_DependentBossPresentations(
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId);
		bool_t Prepare_ValtanGhostPresentationPool(
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId,
			f32_t collisionRadius,
			const LostArk::Shared::GameplayDataRevision& revision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
			const std::shared_ptr<CValtan>& primaryValtan,
			std::string& strOutStatus);
		std::shared_ptr<CValtan> Checkout_ValtanGhostPresentation(
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId,
			f32_t collisionRadius,
			const LostArk::Shared::GameplayDataRevision& revision,
			const VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
			const float3_t& position,
			f32_t yawDegrees,
			bool_t bHoldBodyHiddenUntilPatternSnapshot);
		bool_t Checkin_ValtanGhostPresentation(
			const std::shared_ptr<CValtan>& valtan);
		bool_t Retry_DeferredValtanGhostPresentationPoolRefresh(
			std::string& strOutStatus);
		void Clear_ValtanGhostPresentationPool();
		bool Apply_CombatObjectSpawn(
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Apply_CombatObjectPresentationEvent(
			const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event);
		bool Apply_CombatObjectDespawn(
			const LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED& despawned);
		//snapshot??netentityid瑜??ㅼ젣 client character濡??댁꽍?섎뒗 ?⑥닔
		bool Apply_WorldSnapshot(
			const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot);
		bool Apply_WorldDestructionFullSync(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& fullSync);
		bool Apply_WorldDestructionDelta(
			const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& delta);
		bool Apply_EncounterPropSync(
			const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& sync);
		bool Apply_InventorySnapshot(
			const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& snapshot);
		void Apply_PartyInviteReceived(
			const LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& received);
		void Apply_PartyRoster(
			const LostArk::Shared::S2C_PARTY_ROSTER& roster);
		void Advance_GuideBubbles();
		void Apply_GuidePrompt(const LostArk::Shared::S2C_GUIDE_PROMPT& prompt);
		void Apply_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& state);
		void Apply_ChatReceived(
			const LostArk::Shared::S2C_CHAT& received);
		enum class CHARACTER_REPLACE_RESULT
		{
			REPLACED,
			RECOVERED_FAILURE,
			FATAL_FAILURE
		};
		CHARACTER_REPLACE_RESULT Replace_CharacterClass(
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
		void Stage_LocalCharacterClassReplacement(
			const LostArk::Shared::PLAYER_SNAPSHOT& Snapshot,
			std::uint32_t iServerTick);
		void Clear_DeferredLocalCharacterClassReplacement();

		void Update_DeathPresentations();
		void Reset_World();
		bool Spawn_CombatObjectPresentation(
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
			COMBAT_OBJECT_PRESENTATION_HANDLE& outHandle,
			std::string& outStatus);
		bool Update_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle,
			const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot, std::uint32_t serverTick);
		void Stop_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
		void Release_CombatObjectPresentation(
			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
		struct COMBAT_OBJECT_PRESENTATION_SINK final
		{
			CClientReplication& Owner;
            std::uint32_t serverTick = 0u;
			bool Spawn(
				const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& message,
				COMBAT_OBJECT_PRESENTATION_HANDLE& outHandle,
				std::string& outStatus)
			{
				return Owner.Spawn_CombatObjectPresentation(
					message, outHandle, outStatus);
			}
			bool Update(
				COMBAT_OBJECT_PRESENTATION_HANDLE handle,
				const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot)
			{
				return Owner.Update_CombatObjectPresentation(handle, snapshot, serverTick);
			}
			void Stop(COMBAT_OBJECT_PRESENTATION_HANDLE handle)
			{
				Owner.Stop_CombatObjectPresentation(handle);
			}
			void Release(COMBAT_OBJECT_PRESENTATION_HANDLE handle)
			{
				Owner.Release_CombatObjectPresentation(handle);
			}
		};

	private:
		//?대뼡 layer怨?prototype???앹꽦?섏뼱???섎뒗吏
		DESC m_Desc;
		std::wstring m_strLocalPlayerNavigationPrototypeTag;
		// Stable net objects: slot table, free-slot index and the
		// handle-by-entity-id lookup, kept across frames.
		CNetObjectRegistry m_Registry;
		struct PENDING_PLAYER_PRESENTATION final
		{
			LostArk::Shared::PLAYER_SNAPSHOT Snapshot{};
			std::uint32_t ServerTick = 0u;
			// Only the referenced hand-grip owner is needed from the source packet.
			std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT> AttachmentOwners;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::S2C_PLAYER_SPAWNED> m_PendingPlayerSpawns;
		std::map<LostArk::Shared::NET_ENTITY_ID, PENDING_PLAYER_PRESENTATION> m_PendingPlayerPresentations;
		std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::CHARACTER_CLASS_ID> m_FailedPlayerSpawnClasses;
		std::unique_ptr<CPlayableCharacterAssetService> m_pPlayerAssetPreparation;
		std::optional<LostArk::Shared::CHARACTER_CLASS_ID> m_PreparingPlayerClass;
		std::unordered_set<uint8_t> m_FailedPlayerAssetClasses;
		//index slot, slotindex, generation
		OBJECT_HANDLE m_LocalCharacterHandle;
		bool m_isInitialized = false;
		bool m_wasConnected = false;
		bool m_hasPendingConnectionLoss = false;
		bool m_hasFatalWorldDestructionFailure = false;
		//留덉?留됱쑝濡??곸슜??snapshot tick
		std::uint32_t m_iLastServerTick = 0;
		std::vector<LostArk::Shared::PLAYER_SNAPSHOT> m_KoukuCardSnapshots;
		struct DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT final
		{
			bool_t isPending = false;
			std::uint64_t iGeneration = 0u;
			std::uint32_t iServerTick = 0u;
			LostArk::Shared::PLAYER_SNAPSHOT Snapshot{};
		} m_DeferredLocalCharacterClassReplacement;
		std::uint64_t m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
		std::string m_strPendingPresentationFailure;
		VALTAN_PRESENTATION_STATE m_ValtanPresentationState;
		CCombatObjectProjectionRuntime m_CombatObjectProjectionRuntime;
        // MainApp owns the player for the active Kouku level; Reset_World drops this view.
        CKoukuSaydonPresentationPlayer* m_pTargetedCombatPresentationPlayer = nullptr;
		CWorldDestructionProjectionRuntime m_WorldDestructionProjectionRuntime;
		std::deque<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>
			m_WorldDestructionLiveEvents;
		uint64_t m_iWorldDestructionPresentationGeneration = 0u;
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			m_WorldDestructionDiagnostics{};
		LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC m_EncounterPropState{};
		LostArk::Shared::S2C_INVENTORY_SNAPSHOT m_InventoryState{};
		bool m_hasPendingPartyInvite = false;
		LostArk::Shared::S2C_PARTY_INVITE_RECEIVED m_PendingPartyInvite{};
		LostArk::Shared::S2C_PARTY_ROSTER m_PartyRoster{};
		CReplicatedPlayerHealth m_PlayerHealth;
		bool m_hasPendingPartyTransferResult = false;
		LostArk::Shared::S2C_PARTY_TRANSFER_RESULT m_PendingPartyTransferResult{};
		bool m_hasPendingRaidEntryPrompt = false;
		LostArk::Shared::S2C_RAID_ENTRY_PROMPT m_PendingRaidEntryPrompt{};
		bool m_hasPendingRaidEntryVote = false;
		LostArk::Shared::S2C_RAID_ENTRY_VOTE m_PendingRaidEntryVote{};
		std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_PendingWorldSequencePlays;
		LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE m_KoukuBundleState;
		LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE m_KoukuRaidState, m_KoukuRaidReply;
		std::uint32_t m_iKoukuRaidReplyRequestSequence = 0u;
		std::uint64_t m_iKoukuRaidReplyWorldGeneration = 0u;
		std::string m_strInteractPromptTriggerId;

		struct CHAT_BUBBLE_ENTRY
		{
			std::string strText;
			std::chrono::steady_clock::time_point ExpireAt;
		};
		static constexpr std::chrono::seconds CHAT_BUBBLE_DURATION{ 5 };
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, CHAT_BUBBLE_ENTRY>
			m_ChatBubblesByNetEntityId;
		/* Bounded so a level that never drains (no chat window) cannot grow it. */
		static constexpr size_t MAX_PENDING_CHAT_LINES = 64;
		std::vector<CHAT_LINE> m_PendingChatLines;
		std::optional<LostArk::Shared::S2C_GUIDE_STATE> m_GuideState;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, std::deque<LostArk::Shared::S2C_GUIDE_PROMPT>> m_PendingGuideBubbles;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> m_GuidePromptSequences;
		/* Latest snapshot's worn honor title per player, read by Collect_PlayerViews. */
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::HONOR_TITLE_ID>
			m_HonorTitleByNetEntityId;
		COMBAT_DEBUG_VISIBILITY_SNAPSHOT m_CombatDebugVisibility{};
#ifdef _DEBUG
		bool_t m_isCombatObjectHitAreaDebugLoadAttempted = false;
		std::unordered_map<std::string,
			std::vector<COMBAT_OBJECT_HIT_AREA_DEBUG>>
			m_CombatObjectHitAreasByArchetype;
#endif

		struct WORLD_ENTITY_PRESENTATION
		{
			LostArk::Shared::WORLD_ENTITY_SNAPSHOT KoukuSnapshot{};
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			LostArk::Shared::WORLD_ENTITY_KIND eKind =
				LostArk::Shared::WORLD_ENTITY_KIND::END;
			std::string strPlacementId;
			std::string strArchetypeId;
			std::string strEncounterId;
			std::string strCurrentClip;
			/* Copied at spawn so a live entity keeps one validated placement
			binding for its entire presentation lifetime. */
			std::string strResolvedIdleClip;
			NPC_PLACEMENT_PRESENTATION_ENTRY NpcPresentation;
			NPC_ACTION_EDGE_STATE NpcActionEdge;
			MONSTER_PRESENTATION_ACTION_STATE MonsterActionState;
			std::string strActiveActionId;
			std::uint32_t iKoukuActionStartTick = 0u;
			std::size_t iActionClipIndex = 0u;
			std::uint32_t iPatternSequence = 0u;
			std::uint32_t iPatternStageIndex = 0u;
			ESTHER_ACTION_SOUND_PLAYBACK_STATE EstherActionSoundState;
			f32_t fCollisionRadius = 0.f;
			/* The Server occurrence pin and the R -> M generation admitted by the
			   concrete CValtan are tracked independently. A structural exact-reload
			   failure latches RejectedPresentationRevision; transient world-entry
			   lock contention leaves it empty so the bounded baseline recovery may
			   admit the same occurrence on a later snapshot. */
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::GameplayDataRevision AdmittedPresentationRevision{};
			LostArk::Shared::GameplayDataRevision RejectedPresentationRevision{};
			bool_t bPresentationIsolated = false;
			bool_t bUsesValtanGhostPool = false;
			std::weak_ptr<CNpc> pNpc;
			std::weak_ptr<CValtan> pValtan;
		};
		struct WORLD_COMBAT_PRESENTATION final
		{
			LostArk::Shared::NET_ENTITY_ID iBodyNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::weak_ptr<CWorldSequenceObject> object;
		};
		std::vector<WORLD_COMBAT_PRESENTATION> m_WorldCombatTargets;
		std::unordered_set<LostArk::Shared::NET_ENTITY_ID> m_PendingWorldCombatHits;
		WORLD_ENTITY_PRESENTATION* Find_ValtanPresentation(
			const std::shared_ptr<CValtan>& pValtan);
		bool_t Ensure_ValtanPresentationRevision(
			WORLD_ENTITY_PRESENTATION& Presentation,
			const std::shared_ptr<CValtan>& pValtan,
			const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
			bool_t bIsPrimary,
			std::string& strOutStatus);
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			WORLD_ENTITY_PRESENTATION> m_WorldEntities;
		static constexpr std::size_t VALTAN_GHOST_PRESENTATION_POOL_CAPACITY = 4u;
		struct VALTAN_GHOST_PRESENTATION_POOL_SLOT final
		{
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			f32_t fCollisionRadius = 0.f;
			LostArk::Shared::GameplayDataRevision AdmittedPresentationRevision{};
			VALTAN_PRESENTATION_GENERATION_RECEIPT AdmittedPresentationReceipt;
			std::shared_ptr<CValtan> pValtan;
			bool_t bCheckedOut = false;
		};
		std::vector<VALTAN_GHOST_PRESENTATION_POOL_SLOT>
			m_ValtanGhostPresentationPool;
		struct DEFERRED_VALTAN_GHOST_PRESENTATION_POOL_REFRESH final
		{
			bool_t bPending = false;
			LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			f32_t fCollisionRadius = 0.f;
			LostArk::Shared::GameplayDataRevision PresentationRevision{};
			VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
			std::weak_ptr<CValtan> pPrimaryValtan;
		};
		DEFERRED_VALTAN_GHOST_PRESENTATION_POOL_REFRESH
			m_DeferredValtanGhostPresentationPoolRefresh;
		VALTAN_PRESENTATION_GENERATION_RECEIPT
			m_RejectedValtanGhostPoolReceipt;
		CPrimaryValtanPresentationFreshnessGate
			m_PrimaryValtanJoinedPresentationFreshness;
		CPrimaryValtanPresentationFreshnessGate
			m_PrimaryValtanCombatObjectSoundFreshness;
		/* The existing Layer owns death tails; they are no longer network entities. */
		std::vector<std::weak_ptr<CValtan>> m_DeathPresentations;
	};
}
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Private/ClientReplication.cpp

```cpp
#include "ClientReplication.h"

#include "Profiler.h"
#include "LevelTransitionService.h"

#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CombatHUDViewModel.h"
#include "Effect_PresentationService.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "EffectFailureDiagnostic.h"
#include "EstherActionSoundCueDocument.h"
#include "GameInstance.h"
#include "MapNavigationContract.h"
#include "Model.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "Npc.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "PlayableCharacterAssetService.h"
#include "PlayerController.h"
#include "UIInputRouter.h"
#include "WorldSequenceObject.h"
#include "Transform.h"
#include "Valtan.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "ValtanPresentationAssetService.h"
#include "DeployPropRuntime.h"
#ifdef _DEBUG
#include "DataJson.h"
#include "HitAreaWire.h"
#include "ProjectDataRoot.h"
#endif

#include <algorithm>
#include <cmath>
#include <filesystem>
#ifdef _DEBUG
#include <fstream>
#include <iterator>
#endif
#include <limits>
#include <span>

namespace
{
	Client::CLocalMovePrediction::Snapshot LocalMoveSnapshot(
		const LostArk::Shared::PLAYER_SNAPSHOT& player, const std::uint32_t serverTick)
	{
		Client::CLocalMovePrediction::Snapshot snapshot{};
		snapshot.serverTick = serverTick;
		snapshot.processedMoveSequence = player.iLastProcessedMoveSequence;
		snapshot.position = { player.fPositionX, player.fPositionY, player.fPositionZ };
		snapshot.yawDegrees = player.fYawDegrees;
		snapshot.moveSpeed = player.fMoveSpeed;
		snapshot.canPredictMove = player.canPredictMove;
		snapshot.hasMoveGoal = player.hasMoveGoal;
		snapshot.nextWaypoint = { player.fMoveWaypointX, player.fMoveWaypointY, player.fMoveWaypointZ };
		return snapshot;
	}

	using Client::CValtan;
	constexpr std::string_view KOUKU_SAYDON_BOSS_ARCHETYPE =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	constexpr std::string_view KOUKU_SAYDON_ENCOUNTER =
		"ENCOUNTER_KAKULSAYDON_G1";

	bool Is_KoukuSaydonDependentArchetype(const std::string_view archetypeId)
	{
		return archetypeId == "BOSS_KAKULSAYDON_G1_SAYDON" ||
			archetypeId == "BOSS_KAKULSAYDON_G3_SAYDON" ||
			archetypeId == "BOSS_KAKULSAYDON_G2_KOUKU";
	}

	/* Arena bosses and their supported same-body clones share the catalog
	family and present on independent CNpc bodies keyed by NetEntityId. */
	bool Is_KoukuSaydonArenaBoss(
		const std::string_view archetypeId,
		const std::string_view encounterId,
		const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId)
	{
		return (LostArk::Shared::INVALID_NET_ENTITY_ID == ownerBossNetEntityId ||
			Is_KoukuSaydonDependentArchetype(archetypeId)) &&
			encounterId == KOUKU_SAYDON_ENCOUNTER &&
			Client::CKoukuSaydonPresentationAssetService::Is_ArenaBossArchetype(
				archetypeId);
	}

	Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT g_CombatDebugVisibility = []
	{
		Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT Visibility{};
		Visibility.iRevision = 1u;
		return Visibility;
	}();

	bool_t Can_ReloadValtanPresentationWithoutResettingLiveSound(
		std::string& strOutStatus)
	{
		Client::CValtanPatternAuditionService& Audition =
			Client::CValtanPatternAuditionService::Get();
		Client::CValtanPatternFlowService& Flow =
			Client::CValtanPatternFlowService::Get();
		Audition.Update();
		Flow.Update();
		if (Audition.Has_PatternSoundMutationBarrier() ||
			Flow.Has_PatternSoundMutationBarrier())
		{
			strOutStatus =
				"Authoritative Valtan presentation reload is blocked while a Pattern/Restart/Next/Flow occurrence owns the pinned Pattern Sound receipt; cue attempt state was preserved.";
			return false;
		}
		strOutStatus.clear();
		return true;
	}

	Client::NET_PLAYER_RECORD Make_Record(
		const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
	{
		Client::NET_PLAYER_RECORD record{};

		record.iPlayerId = spawned.iPlayerId;

		record.iNetEntityId = spawned.iNetEntityId;

		record.eCharacterClass = spawned.eCharacterClass;
		record.eControlKind = spawned.eControlKind;

		record.strNickName = spawned.strNickName;

		record.fPositionX = spawned.fPositionX;
		record.fPositionY = spawned.fPositionY;
		record.fPositionZ = spawned.fPositionZ;

		record.fYawDegrees = spawned.fYawDegrees;

		return record;
	}
	// TCP ?ъ쟾?≪씠??以묐났 Event媛 媛숈? Spawn???ㅼ떆 留뚮뱾吏 ?딄쾶 ?섍퀬,
	// 媛숈? NetEntityId???ㅻⅨ ?댁슜???ㅻ㈃ Protocol 異⑸룎濡??먮떒?쒕떎.
	bool Is_Same_Record(
		const Client::NET_PLAYER_RECORD& left,
		const Client::NET_PLAYER_RECORD& right)
	{
		return
			left.iPlayerId == right.iPlayerId &&
			left.iNetEntityId == right.iNetEntityId &&
			left.eCharacterClass == right.eCharacterClass &&
			left.eControlKind == right.eControlKind &&
			left.strNickName == right.strNickName &&
			left.fPositionX == right.fPositionX &&
			left.fPositionY == right.fPositionY &&
			left.fPositionZ == right.fPositionZ &&
			left.fYawDegrees == right.fYawDegrees;
	}

	CValtan::PATTERN_TARGET_SNAPSHOT_POSE Resolve_ValtanPatternTargetSnapshotPose(
		const std::span<const LostArk::Shared::PLAYER_SNAPSHOT> players,
		const LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId)
	{
		CValtan::PATTERN_TARGET_SNAPSHOT_POSE pose;
		pose.iNetEntityId = iTargetNetEntityId;
		if (LostArk::Shared::INVALID_NET_ENTITY_ID == iTargetNetEntityId)
			return pose;

		bool_t bFound = false;
		for (const LostArk::Shared::PLAYER_SNAPSHOT& player : players)
		{
			if (player.iNetEntityId != iTargetNetEntityId)
				continue;
			/* Duplicate identity is not a pose authority.  Keep the target ID for
			   diagnostics but force the cue occurrence down its isolated path. */
			if (bFound)
			{
				pose.bHasFinitePose = false;
				return pose;
			}
			bFound = true;
			if (!std::isfinite(player.fPositionX) ||
				!std::isfinite(player.fPositionY) ||
				!std::isfinite(player.fPositionZ) ||
				!std::isfinite(player.fYawDegrees))
			{
				continue;
			}
			pose.vPosition = float3_t(
				player.fPositionX, player.fPositionY, player.fPositionZ);
			pose.fYawDegrees = player.fYawDegrees;
			pose.bHasFinitePose = true;
		}
		return pose;
	}
}

Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT
Client::CClientReplication::Get_GlobalCombatDebugVisibility()
{
	return g_CombatDebugVisibility;
}

void Client::CClientReplication::Set_GlobalCombatDebugVisibility(
	const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility)
{
	if (g_CombatDebugVisibility.Has_SameVisibility(Visibility))
		return;
	const std::uint64_t iNextRevision =
		COMBAT_DEBUG_VISIBILITY_SNAPSHOT::Next_Revision(
			g_CombatDebugVisibility.iRevision);
	g_CombatDebugVisibility = Visibility;
	g_CombatDebugVisibility.iRevision = iNextRevision;
}

bool Client::CClientReplication::Initialize(const DESC& desc)
{
	//Layer ?뺣낫媛 ?좏븳?쒖? 寃?ы븯怨? ?ㅼ젙????ν븳??
	//?꾩옱 network ?곌껐 ?곹깭??湲곗뼲???먯뼱, ?댄썑 ?곌껐???딄꼈?붿? 媛먯??????덇쾶 ?쒕떎.
	if (nullptr == desc.pDevice ||
		nullptr == desc.pContext ||
		desc.strMapAreaId.empty() ||
		desc.strPlayerLayerTag.empty() ||
		desc.strWorldEntityLayerTag.empty() ||
		((nullptr == desc.pDeployPropRuntime) !=
			(nullptr == desc.pWorldDestructionProjection)) ||
		(nullptr != desc.pWorldDestructionProjection &&
			!desc.pWorldDestructionProjection->Is_Ready()) ||
		!CCombatHUDViewModel::Get().Initialize_Definitions())
		return false;

	MAP_NAVIGATION_CONTRACT navigationContract{};
	std::string navigationStatus;
	if (!CMapNavigationContract::Resolve_Area(
			desc.strMapAreaId, navigationContract, navigationStatus) ||
		!navigationContract.runtimeGridAvailable ||
		navigationContract.prototypeTag.empty())
	{
		OutputDebugStringA((
			"Client replication navigation unavailable: " +
			navigationStatus + "\n").c_str());
		return false;
	}

	m_Desc = desc;
	m_strLocalPlayerNavigationPrototypeTag =
		std::move(navigationContract.prototypeTag);
	m_isInitialized = true;
	m_wasConnected =
		CNetworkManager::Get().Is_Connected();
	m_hasPendingConnectionLoss = !m_wasConnected;
	m_hasFatalWorldDestructionFailure = false;
	m_WorldDestructionProjectionRuntime.Reset();
	Clear_DeferredLocalCharacterClassReplacement();
	m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
	Sync_GlobalCombatDebugVisibility();

	return true;
}

void Client::CClientReplication::Expect_KoukuRaidReply(const std::uint32_t requestSequence)
{
	m_KoukuRaidReply = {};
	m_iKoukuRaidReplyRequestSequence = requestSequence;
	m_iKoukuRaidReplyWorldGeneration = requestSequence ? CNetworkManager::Get().Get_WorldInboundGeneration() : 0u;
}

bool Client::CClientReplication::Update()
{
	m_PendingWorldCombatHits.clear();
	Engine::CProfilerScope updateScope(
		CGameInstance::Get().Get_Profiler(), "Replication.Update");
	if (!m_isInitialized)
		return false;
	/* This must precede the disconnected early return. A developer selection is a
	   process setting, not state owned by whichever Level currently has a live
	   socket. */
	Sync_GlobalCombatDebugVisibility();
	//network manager媛 留뚮뱾?대넃? replication event瑜??ㅼ젣 engine 蹂寃쎌쑝濡??곸슜?쒕떎.
	//baren main thread?먯꽌 留ㅽ봽?덉엫 ?몄텧?쒕떎.
	//珥덇린???щ? 寃??-> ?꾩옱 network ?곌껐 ?곹깭 ?뺤씤
	CNetworkManager& networkManager =
		CNetworkManager::Get();

	const bool isConnected =
		networkManager.Is_Connected();
	//?곌껐???딆뼱吏?寃쎌슦 : ?댁쟾 ?꾨젅?꾩뿉???곌껐???덉뿀?붽?? -> 洹몃젃?ㅻ㈃ reset world()
	//?⑥븘 ?덈뒗 event queue 鍮꾩슦湲?-> ?댁쟾 ?쒕쾭??spawn???ъ젒?????곸슜?섏? ?딄쾶 ??
	if (!isConnected)
	{
		if (m_wasConnected)
		{
			Reset_World();
			m_hasPendingConnectionLoss = true;
		}

		CLIENT_REPLICATION_EVENT ignored{};
		while (networkManager.Try_Consume_ReplicationEvent(ignored))
		{

		}
		m_wasConnected = false;
		return true;
	}

	m_wasConnected = true;
	bool allSucceeded = true;

	CLIENT_REPLICATION_EVENT event{};

	while (networkManager.Try_Consume_ReplicationEvent(event))
	{
		switch (event.eType)
		{
		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED:
			allSucceeded =
				Apply_Spawn(event.PlayerSpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_SPAWNED:
			allSucceeded = Apply_WorldEntitySpawn(
				event.WorldEntitySpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_SPAWNED:
			allSucceeded = Apply_CombatObjectSpawn(
				event.CombatObjectSpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_PRESENTATION:
			allSucceeded = Apply_CombatObjectPresentationEvent(
				event.CombatObjectPresentation) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED:
			allSucceeded =
				Apply_WorldEntityDespawn(event.WorldEntityDespawned) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_DESPAWNED:
			allSucceeded = Apply_CombatObjectDespawn(
				event.CombatObjectDespawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED:
			allSucceeded =
				Apply_Despawn(event.PlayerDespawned) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT:
			allSucceeded = Apply_WorldSnapshot(event.WorldSnapshot) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_DESTRUCTION_FULL_SYNC:
			allSucceeded = Apply_WorldDestructionFullSync(
				event.WorldDestructionFullSync) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_DESTRUCTION_DELTA:
			allSucceeded = Apply_WorldDestructionDelta(
				event.WorldDestructionDelta) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::ENCOUNTER_PROP_SYNC:
			allSucceeded = Apply_EncounterPropSync(
				event.EncounterPropSync) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::INVENTORY_SNAPSHOT:
			allSucceeded = Apply_InventorySnapshot(
				event.InventorySnapshot) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_INVITE_RECEIVED:
			Apply_PartyInviteReceived(event.PartyInviteReceived);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_ROSTER:
			Apply_PartyRoster(event.PartyRoster);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_TRANSFER_RESULT:
			m_PendingPartyTransferResult = event.PartyTransferResult;
			m_hasPendingPartyTransferResult = true;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::RAID_ENTRY_PROMPT:
			m_PendingRaidEntryPrompt = event.RaidEntryPrompt;
			m_hasPendingRaidEntryPrompt = true;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::RAID_ENTRY_VOTE:
			m_PendingRaidEntryVote = event.RaidEntryVote;
			m_hasPendingRaidEntryVote = true;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::GUIDE_PROMPT:
			Apply_GuidePrompt(event.GuidePrompt);
			break;
		case CLIENT_REPLICATION_EVENT_TYPE::GUIDE_STATE:
			Apply_GuideState(event.GuideState);
			break;
		case CLIENT_REPLICATION_EVENT_TYPE::CHAT_RECEIVED:
			Apply_ChatReceived(event.ChatReceived);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::INTERACT_PROMPT:
			/* Withdrawal only clears an offer that is still the current one, so
			   a late withdrawal for a box already replaced cannot blank the new
			   offer. */
			if (event.InteractPrompt.bAvailable)
			{
				m_strInteractPromptTriggerId =
					event.InteractPrompt.strTriggerPlacementId;
			}
			else if (m_strInteractPromptTriggerId ==
				event.InteractPrompt.strTriggerPlacementId)
			{
				m_strInteractPromptTriggerId.clear();
			}
			CCombatHUDViewModel::Get().Set_InteractPromptTriggerId(
				m_strInteractPromptTriggerId);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::KOUKUSAYDON_RAID_STATE:
			if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA) && event.KoukuRaidState.eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA)
			{
				// One exact local command owns this mailbox. Later state broadcasts
				// must not replace its verdict before MainApp can consume the batch.
				if (m_iKoukuRaidReplyRequestSequence && !m_KoukuRaidReply.iRequestSequence &&
					m_iKoukuRaidReplyWorldGeneration == networkManager.Get_WorldInboundGeneration() &&
					event.KoukuRaidState.iRequestSequence == m_iKoukuRaidReplyRequestSequence &&
					(!event.KoukuRaidState.iRunEpoch || event.KoukuRaidState.iOwnerPlayerId == networkManager.Get_LocalPlayerId()))
					m_KoukuRaidReply = event.KoukuRaidState;
				if (event.KoukuRaidState.iRunEpoch && event.KoukuRaidState.iRunEpoch >= m_KoukuRaidState.iRunEpoch)
					m_KoukuRaidState = event.KoukuRaidState;
			}
			break;
		case CLIENT_REPLICATION_EVENT_TYPE::KOUKUSAYDON_BUNDLE_STATE:
			if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
                event.KoukuBundleState.eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
                event.KoukuBundleState.iRunEpoch >= m_KoukuBundleState.iRunEpoch)
            {
                if (event.KoukuBundleState.iRunEpoch != m_KoukuBundleState.iRunEpoch ||
                    !CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(event.KoukuBundleState.iPinnedSourceRevision,
                        event.KoukuBundleState.DraftRowsRevision, event.KoukuBundleState.iRunEpoch))
                {
                    std::string status;
                    if (!CKoukuSaydonPresentationAssetService::Admit_RunProduct(
                        m_Desc.iLayerLevelIndex, event.KoukuBundleState.iPinnedSourceRevision,
                        event.KoukuBundleState.DraftRowsRevision, event.KoukuBundleState.iRunEpoch, status))
                    {
                        m_strPendingPresentationFailure = status;
                        OutputDebugStringA(("[KoukuSaydonAnimation] " + status + "\n").c_str());
                    }
                    // A state packet may follow the initial snapshot in this batch.
                    // Re-evaluate that action using the newly pinned cache next tick.
                    for (auto& [id, entity] : m_WorldEntities)
                        if (Is_KoukuSaydonArenaBoss(entity.strArchetypeId, entity.strEncounterId, entity.iOwnerBossNetEntityId))
                            entity.strActiveActionId.clear();
                }
                m_KoukuBundleState = event.KoukuBundleState;
            }
			break;
		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_SEQUENCE_PLAY:
			/* Queued rather than played here: the level owns the sequence
			   player and drains this on its own update. */
			m_PendingWorldSequencePlays.push_back(event.WorldSequencePlay);
			break;
		}
	}

	allSucceeded = Advance_PlayerAssetPreparation() && allSucceeded;
	Advance_GuideBubbles();
	Update_DeathPresentations();
#ifdef _DEBUG
	if (m_CombatDebugVisibility.bCombatObjectHit)
		Draw_CombatObjectHitAreaDebug();
#endif
	return allSucceeded;
}

void Client::CClientReplication::Set_WorldCombatTargets(const std::vector<WORLD_COMBAT_TARGET>& targets)
{
	m_WorldCombatTargets.clear();
	for (const auto& target : targets)
	{
		if (target.iBodyNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID ||
			m_WorldEntities.contains(target.iBodyNetEntityId) || !target.object || !target.object->Is_Visible()) continue;
		m_WorldCombatTargets.push_back({ target.iBodyNetEntityId, target.object });
		if (m_PendingWorldCombatHits.contains(target.iBodyNetEntityId)) target.object->Trigger_HitFlash();
	}
	// A stopped/dead cue cannot receive an old hit when an object pool is reused.
	m_PendingWorldCombatHits.clear();
}

bool Client::CClientReplication::Apply_WorldDestructionFullSync(
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& fullSync)
{
	if (nullptr == m_Desc.pWorldDestructionProjection ||
		nullptr == m_Desc.pDeployPropRuntime)
	{
		m_strPendingPresentationFailure =
			"World destruction projection is not configured for this level.";
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	std::string status;
	if (m_Desc.beforeWorldDestructionProjection &&
		!m_Desc.beforeWorldDestructionProjection(status))
	{
		m_strPendingPresentationFailure = "World preview cleanup failed: " + status;
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	if (!m_WorldDestructionProjectionRuntime.Apply_Full(
		*m_Desc.pWorldDestructionProjection, fullSync,
		*m_Desc.pDeployPropRuntime, status))
	{
		m_strPendingPresentationFailure = std::move(status);
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	m_WorldDestructionLiveEvents.clear();
	m_WorldDestructionDiagnostics = fullSync.Diagnostics;
	++m_iWorldDestructionPresentationGeneration;
	if (0u == m_iWorldDestructionPresentationGeneration)
		++m_iWorldDestructionPresentationGeneration;
	return true;
}

bool Client::CClientReplication::Apply_EncounterPropSync(
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& sync)
{
	/* Replace-in-full. A slot only ever holds its current state, so a late
	   joiner and a player who watched the whole cycle read the same thing, and
	   an out-of-order epoch cannot resurrect a retired pillar. */
	if (sync.iEncounterEpoch < m_EncounterPropState.iEncounterEpoch ||
		(sync.iEncounterEpoch == m_EncounterPropState.iEncounterEpoch &&
		 sync.iServerTick < m_EncounterPropState.iServerTick))
	{
		return true;
	}
	m_EncounterPropState = sync;
	return true;
}

bool Client::CClientReplication::Apply_InventorySnapshot(
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& snapshot)
{
	/* Replace-in-full: the Server always answers with the whole current
	   inventory, so the previous list is simply discarded. */
	m_InventoryState = snapshot;
	/* CCombatHUDViewModel is the level-agnostic singleton the F1 debug panel
	   already reads through (CMainApp owns no per-level CClientReplication of
	   its own), the same role it plays for Apply_LocalPlayer above. */
	CCombatHUDViewModel::Get().Apply_Inventory(snapshot);
	return true;
}

void Client::CClientReplication::Apply_PartyInviteReceived(
	const LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& received)
{
	// A newer invite silently replaces an unconsumed older one, mirroring
	// the Server's own one-pending-invite-per-target replacement rule.
	m_PendingPartyInvite = received;
	m_hasPendingPartyInvite = true;
}

bool Client::CClientReplication::Try_Consume_PartyInviteReceived(
	LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& outInvite)
{
	if (!m_hasPendingPartyInvite)
		return false;
	outInvite = m_PendingPartyInvite;
	m_hasPendingPartyInvite = false;
	return true;
}

void Client::CClientReplication::Apply_PartyRoster(
	const LostArk::Shared::S2C_PARTY_ROSTER& roster)
{
	// Replace-in-full, same shape as Apply_InventorySnapshot/
	// Apply_EncounterPropSync -- the Server always sends the whole current
	// membership, never a delta.
	m_PartyRoster = roster;
	if (m_GuideState && (!roster.GuideCompanion ||
		roster.GuideCompanion->iNetEntityId != m_GuideState->iGuideNetEntityId)) m_GuideState.reset();
}

bool Client::CClientReplication::Try_Consume_PartyTransferResult(
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT& outResult)
{
	if (!m_hasPendingPartyTransferResult)
		return false;
	outResult = m_PendingPartyTransferResult;
	m_hasPendingPartyTransferResult = false;
	return true;
}

bool Client::CClientReplication::Try_Consume_RaidEntryPrompt(
	LostArk::Shared::S2C_RAID_ENTRY_PROMPT& outPrompt)
{
	if (!m_hasPendingRaidEntryPrompt)
		return false;
	outPrompt = m_PendingRaidEntryPrompt;
	m_hasPendingRaidEntryPrompt = false;
	return true;
}

bool Client::CClientReplication::Try_Consume_RaidEntryVote(
	LostArk::Shared::S2C_RAID_ENTRY_VOTE& outVote)
{
	if (!m_hasPendingRaidEntryVote)
		return false;
	outVote = m_PendingRaidEntryVote;
	m_hasPendingRaidEntryVote = false;
	return true;
}

void Client::CClientReplication::Apply_GuidePrompt(const LostArk::Shared::S2C_GUIDE_PROMPT& prompt)
{
    using namespace LostArk::Shared;
    // Spawn and dialogue use the same reliable ordered event queue. A staged body
    // may still be loading its assets; the authoritative identity is already known.
    const auto* record = m_Registry.Find_Record(prompt.iGuideNetEntityId);
    const auto pending = m_PendingPlayerSpawns.find(prompt.iGuideNetEntityId);
    const bool committed = record && record->eControlKind == PLAYER_CONTROL_KIND::GUIDE_AI;
    const bool staged = pending != m_PendingPlayerSpawns.end() && pending->second.eControlKind == PLAYER_CONTROL_KIND::GUIDE_AI;
    if (!committed && !staged) return;
    auto& lastSequence = m_GuidePromptSequences[prompt.iGuideNetEntityId];
    if (prompt.iEventSequence <= lastSequence) return;
    lastSequence = prompt.iEventSequence;
    auto& pendingBubbles = m_PendingGuideBubbles[prompt.iGuideNetEntityId];
    if (pendingBubbles.size() >= 16u) pendingBubbles.pop_front();
    pendingBubbles.push_back(prompt);
    if (m_PendingChatLines.size() >= MAX_PENDING_CHAT_LINES) m_PendingChatLines.erase(m_PendingChatLines.begin());
    m_PendingChatLines.push_back(CHAT_LINE{ committed ? record->strNickName : pending->second.strNickName, prompt.strText });
}

void Client::CClientReplication::Advance_GuideBubbles()
{
    const auto now = std::chrono::steady_clock::now();
    for (auto& [entityId, queue] : m_PendingGuideBubbles)
    {
        if (queue.empty()) continue;
        OBJECT_HANDLE handle{};
        if (!m_Registry.Find_Handle(entityId, handle) || !m_Registry.Resolve(handle)) continue;
        const auto active = m_ChatBubblesByNetEntityId.find(entityId);
        if (active != m_ChatBubblesByNetEntityId.end() && active->second.ExpireAt > now) continue;
        // Begin the display lifetime only once the actual Character is ready.
        const auto& prompt = queue.front();
        m_ChatBubblesByNetEntityId[entityId] = CHAT_BUBBLE_ENTRY{
            prompt.strText, now + std::chrono::milliseconds(prompt.iDurationMs) };
        queue.pop_front();
    }
}

void Client::CClientReplication::Apply_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& state)
{
    const auto* record = m_Registry.Find_Record(state.iGuideNetEntityId);
    const auto pending = m_PendingPlayerSpawns.find(state.iGuideNetEntityId);
    if ((!record || record->eControlKind != LostArk::Shared::PLAYER_CONTROL_KIND::GUIDE_AI) &&
        (pending == m_PendingPlayerSpawns.end() || pending->second.eControlKind != LostArk::Shared::PLAYER_CONTROL_KIND::GUIDE_AI)) return;
    if (m_GuideState && m_GuideState->iGuideNetEntityId == state.iGuideNetEntityId &&
        state.iServerTick < m_GuideState->iServerTick) return;
    m_GuideState = state;
}

void Client::CClientReplication::Apply_ChatReceived(
	const LostArk::Shared::S2C_CHAT& received)
{
	m_ChatBubblesByNetEntityId[received.iFromNetEntityId] = CHAT_BUBBLE_ENTRY{
		received.strText,
		std::chrono::steady_clock::now() + CHAT_BUBBLE_DURATION };
	/* The same broadcast is the log's only source, sender included, so a line reads
	"<nickname> : <text>" for everyone in the room the way retail writes it. */
	if (m_PendingChatLines.size() >= MAX_PENDING_CHAT_LINES)
		m_PendingChatLines.erase(m_PendingChatLines.begin());
	m_PendingChatLines.push_back(CHAT_LINE{ received.strFromNickname, received.strText });
}

void Client::CClientReplication::Drain_ChatLines(std::vector<CHAT_LINE>& outLines)
{
	outLines = std::move(m_PendingChatLines);
	m_PendingChatLines.clear();
}

bool Client::CClientReplication::Try_Get_ActiveChatBubble(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	std::string& outText) const
{
	const auto found = m_ChatBubblesByNetEntityId.find(netEntityId);
	if (m_ChatBubblesByNetEntityId.end() == found ||
		std::chrono::steady_clock::now() >= found->second.ExpireAt)
	{
		return false;
	}
	outText = found->second.strText;
	return true;
}

bool Client::CClientReplication::Apply_WorldDestructionDelta(
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& delta)
{
	if (nullptr == m_Desc.pWorldDestructionProjection ||
		nullptr == m_Desc.pDeployPropRuntime)
	{
		m_strPendingPresentationFailure =
			"World destruction projection is not configured for this level.";
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	std::string status;
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (m_Desc.beforeWorldDestructionProjection &&
		!m_Desc.beforeWorldDestructionProjection(status))
	{
		m_strPendingPresentationFailure = "World preview cleanup failed: " + status;
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	if (!m_WorldDestructionProjectionRuntime.Apply_Delta(
		*m_Desc.pWorldDestructionProjection, delta,
		*m_Desc.pDeployPropRuntime, status, &liveEvents))
	{
		m_strPendingPresentationFailure = std::move(status);
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	constexpr size_t MAX_PENDING_PRESENTATION_EVENTS =
		LostArk::Shared::MAX_WORLD_DESTRUCTION_EVENTS * 2u;
	for (LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& event : liveEvents)
	{
		if (m_WorldDestructionLiveEvents.size() >=
			MAX_PENDING_PRESENTATION_EVENTS)
		{
			m_strPendingPresentationFailure =
				"World destruction debris cue queue reached its presentation-only limit.";
			break;
		}
		m_WorldDestructionLiveEvents.push_back(std::move(event));
	}
	m_WorldDestructionDiagnostics = delta.Diagnostics;
	return true;
}

bool Client::CClientReplication::Has_PendingConnectionLoss() const
{
	return m_hasPendingConnectionLoss;
}

void Client::CClientReplication::Acknowledge_ConnectionLoss()
{
	m_hasPendingConnectionLoss = false;
}

void Client::CClientReplication::Reset()
{
	Reset_World();
	m_wasConnected = false;
	m_hasPendingConnectionLoss = false;
}

bool Client::CClientReplication::Has_WorldEntity(
	const std::string_view archetypeId) const
{
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		const bool_t hasLivePresentation =
			(LostArk::Shared::WORLD_ENTITY_KIND::NPC == presentation.eKind &&
				!presentation.pNpc.expired()) ||
			(LostArk::Shared::WORLD_ENTITY_KIND::MONSTER == presentation.eKind &&
				!presentation.pNpc.expired()) ||
			(LostArk::Shared::WORLD_ENTITY_KIND::BOSS == presentation.eKind &&
				(!presentation.pValtan.expired() ||
				 (Is_KoukuSaydonArenaBoss(
					presentation.strArchetypeId,
					presentation.strEncounterId,
					presentation.iOwnerBossNetEntityId) &&
				  !presentation.pNpc.expired())));
		if (presentation.strArchetypeId == archetypeId &&
			hasLivePresentation)
		{
			return true;
		}
	}
	return false;
}

std::shared_ptr<Client::CValtan> Client::CClientReplication::Find_PrimaryValtanPresentation() const
{
    std::shared_ptr<CValtan> result;
    std::string status;
    return Resolve_PrimaryValtan("BOSS_VALTAN", LostArk::Shared::INVALID_NET_ENTITY_ID, result, status) ? result : nullptr;
}

bool_t Client::CClientReplication::Resolve_PrimaryValtan(
	const std::string_view strArchetypeId,
	const LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId,
	std::shared_ptr<CValtan>& pOutValtan,
	std::string& strOutStatus) const
{
	pOutValtan.reset();
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS != presentation.eKind ||
			presentation.strArchetypeId != strArchetypeId ||
			presentation.iOwnerBossNetEntityId != iOwnerBossNetEntityId)
		{
			continue;
		}

		const std::shared_ptr<CValtan> Candidate = presentation.pValtan.lock();
		if (nullptr == Candidate)
		{
			strOutStatus =
				"The primary replicated Valtan registry entry has no live presentation consumer.";
			return false;
		}
		if (nullptr != pOutValtan)
		{
			pOutValtan.reset();
			strOutStatus =
				"Multiple primary replicated Valtan presentation consumers were found.";
			return false;
		}
		pOutValtan = Candidate;
	}

	strOutStatus = nullptr == pOutValtan ?
		"No active primary replicated Valtan; the next admitted spawn will load the saved presentation source." :
		"Resolved the authoritative primary replicated Valtan presentation consumer.";
	return true;
}

Client::CClientReplication::WORLD_ENTITY_PRESENTATION*
Client::CClientReplication::Find_ValtanPresentation(
	const std::shared_ptr<CValtan>& pValtan)
{
	if (nullptr == pValtan)
		return nullptr;
	for (auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (presentation.pValtan.lock().get() == pValtan.get())
			return &presentation;
	}
	return nullptr;
}

bool_t Client::CClientReplication::Ensure_ValtanPresentationRevision(
	WORLD_ENTITY_PRESENTATION& Presentation,
	const std::shared_ptr<CValtan>& pValtan,
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	const bool_t bIsPrimary,
	std::string& strOutStatus)
{
	using LostArk::Shared::Format_GameplayDataRevision;
	if (nullptr == pValtan || !ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Valtan snapshot has no live presentation or exact occurrence revision.";
		return false;
	}

	Presentation.PinnedDefinitionRevision = ExpectedRevision;
	if (!Presentation.bPresentationIsolated &&
		Presentation.AdmittedPresentationRevision == ExpectedRevision)
	{
		strOutStatus.clear();
		return true;
	}
	if (Presentation.bPresentationIsolated &&
		Presentation.RejectedPresentationRevision == ExpectedRevision)
	{
		strOutStatus =
			"Valtan presentation remains isolated for previously rejected occurrence revision " +
			Format_GameplayDataRevision(ExpectedRevision) + ".";
		return false;
	}

	VALTAN_PRESENTATION_GENERATION_RECEIPT Receipt;
	std::string ReceiptStatus;
	std::string ReloadStatus;
	const bool_t bHasReceipt =
		CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, Receipt, ReceiptStatus);
	const bool_t bEntryReceiptRecoveryPending = !bHasReceipt &&
		CNetworkManager::Get().Get_GameplayRevisionState().
			hasPendingEntryPresentationBaselineRecovery;
	const bool_t bReloaded = bHasReceipt &&
		pValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, Receipt, ReloadStatus);
	if (!bReloaded)
	{
		Presentation.AdmittedPresentationRevision = {};
		Presentation.RejectedPresentationRevision =
			bEntryReceiptRecoveryPending ?
			LostArk::Shared::GameplayDataRevision{} : ExpectedRevision;
		Presentation.bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(pValtan);
		strOutStatus =
			"Valtan occurrence presentation revision " +
			Format_GameplayDataRevision(ExpectedRevision) +
			(bEntryReceiptRecoveryPending ?
				" is waiting for the saved entry presentation JSON after a canonical transaction: " :
				" was isolated after one exact reload attempt: ") +
			(bHasReceipt ? ReloadStatus : ReceiptStatus);
		m_strPendingPresentationFailure = strOutStatus;
		if (bIsPrimary && !bEntryReceiptRecoveryPending)
		{
			m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
			m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		}
		return false;
	}

	Presentation.AdmittedPresentationRevision = ExpectedRevision;
	Presentation.RejectedPresentationRevision = {};
	Presentation.bPresentationIsolated = false;
	strOutStatus = ReloadStatus;
	if (bIsPrimary)
	{
		m_PrimaryValtanJoinedPresentationFreshness.Admit(
			ExpectedRevision, ReloadStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Admit(
			ExpectedRevision, ReloadStatus);
		LostArk::Shared::NET_ENTITY_ID primaryEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		for (const auto& [entityId, Candidate] : m_WorldEntities)
		{
			if (&Candidate == &Presentation)
			{
				primaryEntityId = entityId;
				break;
			}
		}
		std::string PoolStatus;
		if (LostArk::Shared::INVALID_NET_ENTITY_ID == primaryEntityId ||
			!Prepare_ValtanGhostPresentationPool(
				primaryEntityId,
				Presentation.fCollisionRadius,
				ExpectedRevision,
				Receipt,
				pValtan,
				PoolStatus))
		{
			if (!m_strPendingPresentationFailure.empty())
				m_strPendingPresentationFailure += " ";
			m_strPendingPresentationFailure +=
				"Ghost presentation pool refresh failed: " + PoolStatus;
		}
	}
	return true;
}

bool_t Client::CClientReplication::Prepare_ValtanGhostPresentationPool(
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId,
	const f32_t collisionRadius,
	const LostArk::Shared::GameplayDataRevision& revision,
	const VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
	const std::shared_ptr<CValtan>& primaryValtan,
	std::string& strOutStatus)
{
	using LostArk::Shared::INVALID_NET_ENTITY_ID;
	strOutStatus.clear();
	if (INVALID_NET_ENTITY_ID == ownerBossNetEntityId ||
		nullptr == primaryValtan || !revision.Is_Valid() ||
		!receipt.Is_Valid() ||
		receipt.ServerGameplayRevision != revision ||
		!std::isfinite(collisionRadius) || collisionRadius <= 0.f)
	{
		strOutStatus = "Valtan ghost presentation pool input is invalid.";
		return false;
	}

	if (VALTAN_GHOST_PRESENTATION_POOL_CAPACITY ==
		m_ValtanGhostPresentationPool.size())
	{
		const bool_t bExactReady = std::all_of(
			m_ValtanGhostPresentationPool.begin(),
			m_ValtanGhostPresentationPool.end(),
			[ownerBossNetEntityId, collisionRadius, &revision, &receipt](
				const VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot)
			{
				return nullptr != Slot.pValtan &&
					Slot.iOwnerBossNetEntityId == ownerBossNetEntityId &&
					Slot.fCollisionRadius == collisionRadius &&
					Slot.AdmittedPresentationRevision == revision &&
					Slot.AdmittedPresentationReceipt == receipt;
			});
		if (bExactReady)
		{
			if (m_DeferredValtanGhostPresentationPoolRefresh.bPending &&
				m_DeferredValtanGhostPresentationPoolRefresh.
					PresentationReceipt == receipt)
			{
				m_DeferredValtanGhostPresentationPoolRefresh = {};
			}
			strOutStatus = "Valtan ghost presentation pool is already ready.";
			return true;
		}
	}
	if (std::any_of(
			m_ValtanGhostPresentationPool.begin(),
			m_ValtanGhostPresentationPool.end(),
			[](const VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot)
			{ return Slot.bCheckedOut; }))
	{
		m_DeferredValtanGhostPresentationPoolRefresh.bPending = true;
		m_DeferredValtanGhostPresentationPoolRefresh.
			iOwnerBossNetEntityId = ownerBossNetEntityId;
		m_DeferredValtanGhostPresentationPoolRefresh.fCollisionRadius =
			collisionRadius;
		m_DeferredValtanGhostPresentationPoolRefresh.PresentationRevision =
			revision;
		m_DeferredValtanGhostPresentationPoolRefresh.PresentationReceipt =
			receipt;
		m_DeferredValtanGhostPresentationPoolRefresh.pPrimaryValtan =
			primaryValtan;
		strOutStatus =
			"Valtan ghost presentation pool generation refresh is deferred until every checked-out slot returns.";
		return false;
	}
	if (m_RejectedValtanGhostPoolReceipt == receipt)
	{
		if (m_DeferredValtanGhostPresentationPoolRefresh.bPending &&
			m_DeferredValtanGhostPresentationPoolRefresh.
				PresentationReceipt == receipt)
		{
			m_DeferredValtanGhostPresentationPoolRefresh = {};
		}
		strOutStatus =
			"Valtan ghost presentation pool remains isolated for its rejected revision.";
		return false;
	}
	Clear_ValtanGhostPresentationPool();

	const BOSS_ACTOR_ENTRY* ghostActor =
		CActorCatalog::Find_Boss("BOSS_VALTAN_GHOST");
	if (nullptr == ghostActor ||
		ghostActor->clientPresentationId != "boss.valtan.client.v1" ||
		FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			m_Desc.pDevice,
			m_Desc.pContext,
			m_Desc.iPrototypeLevelIndex,
			"BOSS_VALTAN_GHOST")))
	{
		m_RejectedValtanGhostPoolReceipt = receipt;
		strOutStatus =
			"Valtan ghost presentation pool has no admitted ghost prototype.";
		return false;
	}

	std::vector<VALTAN_GHOST_PRESENTATION_POOL_SLOT> Staged;
	Staged.reserve(VALTAN_GHOST_PRESENTATION_POOL_CAPACITY);
	const auto Rollback = [this, &Staged]()
	{
		for (VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot : Staged)
		{
			if (nullptr == Slot.pValtan)
				continue;
			CEffectV2Runtime::Set_Ignored(
				EFFECT_V2_TARGET::From_Valtan(Slot.pValtan), false);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				Slot.pValtan);
		}
		Staged.clear();
	};

	for (std::size_t iSlot = 0u;
		iSlot < VALTAN_GHOST_PRESENTATION_POOL_CAPACITY; ++iSlot)
	{
		CValtan::VALTAN_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.vPosition = {};
		desc.fScale = ghostActor->presentationScale;
		desc.isServerAuthoritative = true;
		desc.bStartReplicationDormant = true;
		desc.strArchetypeId = "BOSS_VALTAN_GHOST";
		desc.iOwnerBossNetEntityId = ownerBossNetEntityId;
		desc.fCollisionRadius = collisionRadius;
		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
				m_Desc.iPrototypeLevelIndex,
				TEXT("Prototype_GameObject_Valtan"),
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				&desc,
				&gameObject)))
		{
			Rollback();
			m_RejectedValtanGhostPoolReceipt = receipt;
			strOutStatus =
				"Valtan ghost presentation pool could not clone a dormant slot.";
			return false;
		}
		const std::shared_ptr<CValtan> valtan =
			std::dynamic_pointer_cast<CValtan>(gameObject);
		if (nullptr != valtan)
		{
			/* A dormant Layer resident must never be discoverable by the V2
			runtime between clone commit and its first checkout. */
			CEffectV2Runtime::Set_Ignored(
				EFFECT_V2_TARGET::From_Valtan(valtan), true);
		}
		std::string CopyStatus;
		if (nullptr == valtan ||
			!valtan->Copy_AdmittedPatternPresentationFrom(
				*primaryValtan, revision, receipt, CopyStatus))
		{
			if (nullptr != valtan)
			{
				CEffectV2Runtime::Set_Ignored(
					EFFECT_V2_TARGET::From_Valtan(valtan), false);
			}
			if (nullptr != gameObject)
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					m_Desc.iLayerLevelIndex,
					m_Desc.strWorldEntityLayerTag,
					gameObject);
			}
			Rollback();
			m_RejectedValtanGhostPoolReceipt = receipt;
			strOutStatus = CopyStatus.empty() ?
				"Valtan ghost presentation pool could not admit a dormant slot." :
				CopyStatus;
			return false;
		}
#ifdef _DEBUG
		valtan->Set_CombatDebugVisibility(
			m_CombatDebugVisibility.bBossBodyCollider,
			m_CombatDebugVisibility.bBossPatternHitPulse,
			m_CombatDebugVisibility.bBossStageGeometry,
			m_CombatDebugVisibility.bCounterProxy);
#endif
		VALTAN_GHOST_PRESENTATION_POOL_SLOT Slot;
		Slot.iOwnerBossNetEntityId = ownerBossNetEntityId;
		Slot.fCollisionRadius = collisionRadius;
		Slot.AdmittedPresentationRevision = revision;
		Slot.AdmittedPresentationReceipt = receipt;
		Slot.pValtan = valtan;
		Staged.push_back(std::move(Slot));
	}

	m_ValtanGhostPresentationPool = std::move(Staged);
	m_RejectedValtanGhostPoolReceipt = {};
	strOutStatus = "Prepared four dormant Valtan ghost presentation slots.";
	return true;
}

std::shared_ptr<CValtan>
Client::CClientReplication::Checkout_ValtanGhostPresentation(
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId,
	const f32_t collisionRadius,
	const LostArk::Shared::GameplayDataRevision& revision,
	const VALTAN_PRESENTATION_GENERATION_RECEIPT& receipt,
	const float3_t& position,
	const f32_t yawDegrees,
	const bool_t bHoldBodyHiddenUntilPatternSnapshot)
{
	for (VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot :
		m_ValtanGhostPresentationPool)
	{
		if (Slot.bCheckedOut || nullptr == Slot.pValtan ||
			Slot.iOwnerBossNetEntityId != ownerBossNetEntityId ||
			Slot.fCollisionRadius != collisionRadius ||
			Slot.AdmittedPresentationRevision != revision ||
			Slot.AdmittedPresentationReceipt != receipt ||
			!Slot.pValtan->Is_ReplicationDormant())
		{
			continue;
		}
		if (!Slot.pValtan->Activate_ReplicatedPoolOccurrence(
				position, yawDegrees,
				bHoldBodyHiddenUntilPatternSnapshot))
		{
			return nullptr;
		}
		Slot.bCheckedOut = true;
		return Slot.pValtan;
	}
	return nullptr;
}

bool_t Client::CClientReplication::Checkin_ValtanGhostPresentation(
	const std::shared_ptr<CValtan>& valtan)
{
	if (nullptr == valtan)
		return false;
	bool_t bReturned = false;
	for (VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot :
		m_ValtanGhostPresentationPool)
	{
		if (Slot.pValtan.get() != valtan.get())
			continue;
		if (!Slot.bCheckedOut || !valtan->Return_ToReplicatedPool())
			return false;
		Slot.bCheckedOut = false;
		bReturned = true;
		break;
	}
	if (!bReturned)
		return false;

	const bool_t bAllSlotsReturned = std::none_of(
		m_ValtanGhostPresentationPool.begin(),
		m_ValtanGhostPresentationPool.end(),
		[](const VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot)
		{ return Slot.bCheckedOut; });
	if (bAllSlotsReturned &&
		m_DeferredValtanGhostPresentationPoolRefresh.bPending)
	{
		std::string RefreshStatus;
		if (!Retry_DeferredValtanGhostPresentationPoolRefresh(
				RefreshStatus))
		{
			if (!m_strPendingPresentationFailure.empty())
				m_strPendingPresentationFailure += " ";
			m_strPendingPresentationFailure +=
				"Deferred Valtan ghost presentation pool refresh failed: " +
				RefreshStatus;
		}
	}
	return true;
}

bool_t Client::CClientReplication::
Retry_DeferredValtanGhostPresentationPoolRefresh(
	std::string& strOutStatus)
{
	strOutStatus.clear();
	if (!m_DeferredValtanGhostPresentationPoolRefresh.bPending)
		return true;

	/* Prepare clears the pending latch while transactionally replacing the
	pool, so copy the immutable request before entering it. */
	const DEFERRED_VALTAN_GHOST_PRESENTATION_POOL_REFRESH Pending =
		m_DeferredValtanGhostPresentationPoolRefresh;
	const std::shared_ptr<CValtan> PrimaryValtan =
		Pending.pPrimaryValtan.lock();
	if (nullptr == PrimaryValtan)
	{
		m_DeferredValtanGhostPresentationPoolRefresh = {};
		strOutStatus =
			"Deferred Valtan ghost presentation donor no longer exists.";
		return false;
	}
	return Prepare_ValtanGhostPresentationPool(
		Pending.iOwnerBossNetEntityId,
		Pending.fCollisionRadius,
		Pending.PresentationRevision,
		Pending.PresentationReceipt,
		PrimaryValtan,
		strOutStatus);
}

void Client::CClientReplication::Clear_ValtanGhostPresentationPool()
{
	for (VALTAN_GHOST_PRESENTATION_POOL_SLOT& Slot :
		m_ValtanGhostPresentationPool)
	{
		if (nullptr == Slot.pValtan)
			continue;
		if (Slot.bCheckedOut)
			(void)Slot.pValtan->Return_ToReplicatedPool();
		CEffectPresentationService::Stop_BossOwner(Slot.pValtan);
		CEffectV2Runtime::Set_Ignored(
			EFFECT_V2_TARGET::From_Valtan(Slot.pValtan), false);
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			Slot.pValtan);
	}
	m_ValtanGhostPresentationPool.clear();
	m_DeferredValtanGhostPresentationPoolRefresh = {};
	m_RejectedValtanGhostPoolReceipt = {};
}

bool_t Client::CClientReplication::Reload_PrimaryValtanPresentationAuthoring(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	if (!Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Authoritative primary Valtan presentation reload requires one exact Server-active revision.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		return false;
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	if (!CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, PresentationReceipt, ReceiptStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan has no exact presentation generation receipt: " +
			ReceiptStatus;
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	std::shared_ptr<CValtan> PrimaryValtan;
	std::string ResolveStatus;
	if (!Resolve_PrimaryValtan(
		PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID, PrimaryValtan, ResolveStatus))
	{
		m_PrimaryValtanJoinedPresentationFreshness.Reject(ResolveStatus);
		m_strPendingPresentationFailure = ResolveStatus;
		strOutStatus = ResolveStatus;
		return false;
	}
	if (nullptr == PrimaryValtan)
	{
		std::string FreshnessStatus;
		if (!m_PrimaryValtanJoinedPresentationFreshness.Can_Play(
				ExpectedRevision, FreshnessStatus))
		{
			strOutStatus = ResolveStatus + " " + FreshnessStatus +
				" The next primary spawn must admit a successful joined reload.";
			return false;
		}
		strOutStatus = ResolveStatus;
		return true;
	}
	WORLD_ENTITY_PRESENTATION* const Presentation =
		Find_ValtanPresentation(PrimaryValtan);
	if (nullptr == Presentation ||
		Presentation->PinnedDefinitionRevision != ExpectedRevision)
	{
		strOutStatus =
			"Authoritative primary Valtan reload revision does not match its Server-pinned occurrence.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}

	std::string ReloadStatus;
	if (!PrimaryValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, PresentationReceipt, ReloadStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan joined presentation reload rejected: " +
			ReloadStatus;
		Presentation->AdmittedPresentationRevision = {};
		Presentation->RejectedPresentationRevision = ExpectedRevision;
		Presentation->bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(PrimaryValtan);
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	Presentation->AdmittedPresentationRevision = ExpectedRevision;
	Presentation->RejectedPresentationRevision = {};
	Presentation->bPresentationIsolated = false;
	m_PrimaryValtanJoinedPresentationFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	/* Reload_PatternPresentationAuthoring stages animation, Effect, Pattern
	   Sound, combat-object Sound and shake as one joined generation. */
	m_PrimaryValtanCombatObjectSoundFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	LostArk::Shared::NET_ENTITY_ID primaryEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	for (const auto& [entityId, Candidate] : m_WorldEntities)
	{
		if (&Candidate == Presentation)
		{
			primaryEntityId = entityId;
			break;
		}
	}
	std::string PoolStatus;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == primaryEntityId ||
		!Prepare_ValtanGhostPresentationPool(
			primaryEntityId,
			Presentation->fCollisionRadius,
			ExpectedRevision,
			PresentationReceipt,
			PrimaryValtan,
			PoolStatus))
	{
		if (!m_strPendingPresentationFailure.empty())
			m_strPendingPresentationFailure += " ";
		m_strPendingPresentationFailure +=
			"Ghost presentation pool refresh failed: " + PoolStatus;
	}
	strOutStatus =
		"Authoritative primary Valtan joined presentation reloaded. " +
		ReloadStatus;
	return true;
}

bool_t Client::CClientReplication::Reload_PrimaryValtanCombatObjectSoundCues(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	if (!Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload requires one exact Server-active revision.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		return false;
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	if (!CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, PresentationReceipt, ReceiptStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan has no exact presentation generation receipt: " +
			ReceiptStatus;
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	std::shared_ptr<CValtan> PrimaryValtan;
	std::string ResolveStatus;
	if (!Resolve_PrimaryValtan(
		PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID, PrimaryValtan, ResolveStatus))
	{
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(ResolveStatus);
		m_strPendingPresentationFailure = ResolveStatus;
		strOutStatus = ResolveStatus;
		return false;
	}
	if (nullptr == PrimaryValtan)
	{
		std::string FreshnessStatus;
		if (!m_PrimaryValtanCombatObjectSoundFreshness.Can_Play(
				ExpectedRevision, FreshnessStatus))
		{
			strOutStatus = ResolveStatus + " " + FreshnessStatus +
				" The next primary spawn must admit a successful combat-object Sound reload.";
			return false;
		}
		strOutStatus = ResolveStatus;
		return true;
	}
	WORLD_ENTITY_PRESENTATION* const Presentation =
		Find_ValtanPresentation(PrimaryValtan);
	if (nullptr == Presentation ||
		Presentation->PinnedDefinitionRevision != ExpectedRevision)
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload revision does not match its Server-pinned occurrence.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}

	std::string ReloadStatus;
	if (!PrimaryValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, PresentationReceipt, ReloadStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload rejected: " +
			ReloadStatus;
		Presentation->AdmittedPresentationRevision = {};
		Presentation->RejectedPresentationRevision = ExpectedRevision;
		Presentation->bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(PrimaryValtan);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	Presentation->AdmittedPresentationRevision = ExpectedRevision;
	Presentation->RejectedPresentationRevision = {};
	Presentation->bPresentationIsolated = false;
	/* CValtan's compatibility entry delegates to the same joined transaction. */
	m_PrimaryValtanJoinedPresentationFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	m_PrimaryValtanCombatObjectSoundFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	LostArk::Shared::NET_ENTITY_ID primaryEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	for (const auto& [entityId, Candidate] : m_WorldEntities)
	{
		if (&Candidate == Presentation)
		{
			primaryEntityId = entityId;
			break;
		}
	}
	std::string PoolStatus;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == primaryEntityId ||
		!Prepare_ValtanGhostPresentationPool(
			primaryEntityId,
			Presentation->fCollisionRadius,
			ExpectedRevision,
			PresentationReceipt,
			PrimaryValtan,
			PoolStatus))
	{
		if (!m_strPendingPresentationFailure.empty())
			m_strPendingPresentationFailure += " ";
		m_strPendingPresentationFailure +=
			"Ghost presentation pool refresh failed: " + PoolStatus;
	}
	strOutStatus =
		"Authoritative primary Valtan combat-object Sound reloaded. " +
		ReloadStatus;
	return true;
}

bool_t Client::CClientReplication::Can_Play_PrimaryValtanPresentation(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus) const
{
	if (!m_PrimaryValtanJoinedPresentationFreshness.Can_Play(
			ExpectedRevision, strOutStatus))
		return false;
	if (!m_PrimaryValtanCombatObjectSoundFreshness.Can_Play(
			ExpectedRevision, strOutStatus))
		return false;
	strOutStatus.clear();
	return true;
}

bool_t Client::CClientReplication::
	Get_PrimaryValtanPatternSoundSourceReceipt(
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
		std::string& strOutStatus) const
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::shared_ptr<CValtan> PrimaryValtan;
	if (!Resolve_PrimaryValtan(
			PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID,
			PrimaryValtan, strOutStatus) || nullptr == PrimaryValtan)
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Pattern playback requires one active primary replicated Valtan presentation consumer.";
		}
		return false;
	}
	const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& Receipt =
		PrimaryValtan->Get_PatternSoundSourceReceipt();
	if (!Receipt.Is_Valid())
	{
		strOutStatus =
			"The primary replicated Valtan has no exact admitted Pattern Sound source receipt.";
		return false;
	}
	OutReceipt = Receipt;
	strOutStatus =
		"Resolved the primary replicated Valtan's exact Pattern Sound source receipt.";
	return true;
}

bool Client::CClientReplication::Try_Consume_PresentationFailure(
	std::string& outStatus)
{
	if (m_strPendingPresentationFailure.empty())
		return false;
	outStatus = std::move(m_strPendingPresentationFailure);
	m_strPendingPresentationFailure.clear();
	return true;
}

bool Client::CClientReplication::Try_Consume_WorldDestructionLiveEvent(
	LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& outEvent)
{
	if (m_WorldDestructionLiveEvents.empty())
		return false;
	outEvent = std::move(m_WorldDestructionLiveEvents.front());
	m_WorldDestructionLiveEvents.pop_front();
	return true;
}

std::shared_ptr<CNpc> Client::CClientReplication::Find_ArenaBossNpc(
	const std::string_view archetypeId) const
{
	for (const auto& [netEntityId, presentation] : m_WorldEntities)
	{
		(void)netEntityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS != presentation.eKind ||
			presentation.strArchetypeId != archetypeId ||
			LostArk::Shared::INVALID_NET_ENTITY_ID !=
				presentation.iOwnerBossNetEntityId)
		{
			continue;
		}
		if (const std::shared_ptr<CNpc> npc = presentation.pNpc.lock())
			return npc;
	}
	return nullptr;
}

std::shared_ptr<CCharacter> Client::CClientReplication::Get_LocalCharacter() const
{
	return m_Registry.Resolve(m_LocalCharacterHandle);
}

std::shared_ptr<CNpc> Client::CClientReplication::Find_NpcPlacement(
	const std::string_view placementId) const
{
	for (const auto& [id, presentation] : m_WorldEntities)
		if (presentation.eKind == LostArk::Shared::WORLD_ENTITY_KIND::NPC &&
			presentation.strPlacementId == placementId)
			return presentation.pNpc.lock();
	return nullptr;
}

bool_t Client::CClientReplication::Try_Get_DeferredLocalCharacterClassReplacement(
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW& OutView) const
{
	OutView = {};
	if (!m_DeferredLocalCharacterClassReplacement.isPending)
		return false;

	OutView.iGeneration =
		m_DeferredLocalCharacterClassReplacement.iGeneration;
	OutView.iNetEntityId =
		m_DeferredLocalCharacterClassReplacement.Snapshot.iNetEntityId;
	OutView.eCharacterClass =
		m_DeferredLocalCharacterClassReplacement.Snapshot.eCharacterClass;
	OutView.iServerTick =
		m_DeferredLocalCharacterClassReplacement.iServerTick;
	return true;
}

Client::DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT
Client::CClientReplication::Commit_DeferredLocalCharacterClassReplacement()
{
	using namespace LostArk::Shared;
	if (!m_DeferredLocalCharacterClassReplacement.isPending)
	{
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::NO_PENDING;
	}

	const DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT Pending =
		m_DeferredLocalCharacterClassReplacement;
	const NET_ENTITY_ID iLocalEntityId =
		CNetworkManager::Get().Get_LocalEntityId();
	const NET_PLAYER_RECORD* pCurrentRecord =
		m_Registry.Find_Record(Pending.Snapshot.iNetEntityId);
	if (INVALID_NET_ENTITY_ID == iLocalEntityId ||
		Pending.Snapshot.iNetEntityId != iLocalEntityId ||
		nullptr == pCurrentRecord)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		m_strPendingPresentationFailure =
			"Deferred local class replacement lost its stable player identity.";
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}
	if (pCurrentRecord->eCharacterClass == Pending.Snapshot.eCharacterClass &&
		pCurrentRecord->eMadnessForm == Pending.Snapshot.eMadnessForm)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::COMMITTED;
	}

	const CHARACTER_REPLACE_RESULT ReplaceResult =
		Replace_CharacterClass(Pending.Snapshot);
	if (CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE == ReplaceResult)
	{
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::RECOVERED_FAILURE;
	}
	if (CHARACTER_REPLACE_RESULT::FATAL_FAILURE == ReplaceResult)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}

	const std::shared_ptr<CCharacter> pCharacter = Get_LocalCharacter();
	const float3_t Position(
		Pending.Snapshot.fPositionX,
		Pending.Snapshot.fPositionY,
		Pending.Snapshot.fPositionZ);
	const bool_t isMoving = Pending.Snapshot.eLocomotionState ==
		PLAYER_LOCOMOTION_STATE::MOVING;
	if (pCharacter)
		pCharacter->Apply_LocalMoveSnapshot(LocalMoveSnapshot(Pending.Snapshot, Pending.iServerTick),
			Pending.Snapshot.eAction == LostArk::Shared::PLAYER_ACTION_STATE::SKILL);
	if (nullptr == pCharacter || !pCharacter->Apply_NetworkState(
			Position,
			Pending.Snapshot.fYawDegrees,
			isMoving,
			Pending.iServerTick) ||
		!pCharacter->Apply_MarioPresentation(
			Pending.Snapshot.iMarioStage >= 1u && Pending.Snapshot.iMarioStage <= 4u) ||
		!pCharacter->Apply_NetworkAction(
			Pending.Snapshot.eAction,
			Pending.Snapshot.iSkillId,
			Pending.iServerTick,
			Pending.Snapshot.iActionStartTick,
			Pending.Snapshot.fYawDegrees,
			Pending.Snapshot.iComboStage,
			Pending.Snapshot.hasSkillTarget,
			float3_t(
				Pending.Snapshot.fSkillTargetX,
				Pending.Snapshot.fSkillTargetY,
				Pending.Snapshot.fSkillTargetZ), Pending.Snapshot.eKoukuHudMode, Pending.Snapshot.eAttachmentSlot, Pending.Snapshot.isKnockbackAirborne))
	{
		Clear_DeferredLocalCharacterClassReplacement();
		m_strPendingPresentationFailure =
			"Deferred local class replacement could not apply its latest snapshot.";
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}
	pCharacter->Apply_NetworkStance(Pending.Snapshot.eStance);
	pCharacter->Apply_NetworkPresentationHidden((Pending.Snapshot.CardMaze.flags & LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN) != 0u);
	CCombatHUDViewModel::Get().Apply_LocalPlayer(
		Pending.iServerTick,
		Pending.Snapshot.eCharacterClass,
		Pending.Snapshot);
	Clear_DeferredLocalCharacterClassReplacement();
	return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::COMMITTED;
}

void Client::CClientReplication::Collect_MinimapMarkers(
	MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
{
	outSnapshot = {};
	const std::shared_ptr<CCharacter> localCharacter =
		m_Registry.Resolve(m_LocalCharacterHandle);
	const auto ReadGroundXZ = [](const std::shared_ptr<Engine::CTransform>& pTransform,
		f32_t& outX, f32_t& outZ) -> bool_t
	{
		if (nullptr == pTransform)
			return false;
		float4_t vPosition{};
		XMStoreFloat4(&vPosition, pTransform->Get_State(STATE::POSITION));
		outX = vPosition.x;
		outZ = vPosition.z;
		return true;
	};

	if (nullptr != localCharacter &&
		ReadGroundXZ(localCharacter->Get_Transform(), outSnapshot.fLocalX, outSnapshot.fLocalZ))
	{
		outSnapshot.hasLocal = true;
		float4_t vLook{};
		XMStoreFloat4(&vLook, localCharacter->Get_Transform()->Get_State(STATE::LOOK));
		outSnapshot.fLocalYawDegrees =
			XMConvertToDegrees(std::atan2(vLook.x, vLook.z));
	}

	for (const LIVE_NET_PLAYER& player : m_Registry.Get_LivePlayers())
	{
		if (nullptr == player.pCharacter || player.pCharacter == localCharacter)
			continue;
		MINIMAP_MARKER marker{};
		if (!ReadGroundXZ(player.pCharacter->Get_Transform(), marker.fX, marker.fZ))
			continue;
		marker.bParty = (m_PartyRoster.GuideCompanion &&
			m_PartyRoster.GuideCompanion->iNetEntityId == player.Record.iNetEntityId) || std::any_of(
			m_PartyRoster.Members.begin(), m_PartyRoster.Members.end(),
			[&](const LostArk::Shared::PARTY_ROSTER_MEMBER& member)
			{
				return member.iNetEntityId == player.Record.iNetEntityId;
			});
		outSnapshot.Players.push_back(marker);
	}

	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS != presentation.eKind)
			continue;
		std::shared_ptr<Engine::CTransform> pTransform;
		if (const std::shared_ptr<CValtan> pValtan = presentation.pValtan.lock())
			pTransform = pValtan->Get_Transform();
		else if (const std::shared_ptr<CNpc> pNpc = presentation.pNpc.lock())
			pTransform = pNpc->Get_Transform();
		MINIMAP_MARKER marker{};
		if (ReadGroundXZ(pTransform, marker.fX, marker.fZ))
			outSnapshot.Bosses.push_back(marker);
	}

	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::NPC != presentation.eKind)
			continue;
		const std::shared_ptr<CNpc> pNpc = presentation.pNpc.lock();
		MINIMAP_MARKER_SNAPSHOT::NPC_MARKER marker{};
		if (nullptr == pNpc || !ReadGroundXZ(pNpc->Get_Transform(), marker.fX, marker.fZ))
			continue;
		marker.strPlacementId = presentation.strPlacementId;
		outSnapshot.Npcs.push_back(std::move(marker));
	}
}

void Client::CClientReplication::Collect_KoukuPresentationViews(
	std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
	std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const
{
	bosses.clear(); cards.clear();
	if (m_Desc.iLayerLevelIndex != ETOUI(LEVEL::KAKULSAYDON_ARENA)) return;
	for (const auto& [id, entity] : m_WorldEntities)
		if (entity.eKind == LostArk::Shared::WORLD_ENTITY_KIND::BOSS &&
			!entity.pNpc.expired() && !entity.bPresentationIsolated && entity.KoukuSnapshot.iNetEntityId == id)
			bosses.push_back({entity.pNpc, entity.KoukuSnapshot, m_iLastServerTick,
				entity.iOwnerBossNetEntityId, entity.strArchetypeId});
	const auto& raid = Get_KoukuRaidState();
	using LostArk::Shared::KOUKUSAYDON_RAID_PHASE;
	const bool running = raid.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING || raid.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC ||
		raid.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT || raid.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE ||
		raid.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || raid.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
	const auto live = m_Registry.Get_LivePlayers();
	for (const auto& snapshot : m_KoukuCardSnapshots)
	{
		OBJECT_HANDLE handle;
		if (m_Registry.Find_Handle(snapshot.iNetEntityId, handle))
			if (auto character = m_Registry.Resolve(handle))
			{
				const auto player = std::find_if(live.begin(), live.end(), [&](const auto& value) { return value.Record.iNetEntityId == snapshot.iNetEntityId; });
				const bool participant = !running || (player != live.end() &&
					std::find(raid.ParticipantPlayerIds.begin(), raid.ParticipantPlayerIds.end(), player->Record.iPlayerId) != raid.ParticipantPlayerIds.end());
				cards.push_back({character, snapshot, participant});
			}
	}
}

void Client::CClientReplication::Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const
{
	targets.clear();
	if (m_Desc.iLayerLevelIndex != ETOUI(LEVEL::KAKULSAYDON_ARENA)) return;
	for (const auto& [id, entity] : m_WorldEntities)
		if (entity.eKind == LostArk::Shared::WORLD_ENTITY_KIND::MONSTER &&
			!entity.pNpc.expired() && !entity.bPresentationIsolated &&
			entity.KoukuSnapshot.iNetEntityId == id && entity.KoukuSnapshot.iCurrentHp > 0u)
			targets.push_back({entity.pNpc, id, entity.strArchetypeId});
}

void Client::CClientReplication::Collect_PlayerViews(
	std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const
{
	const std::vector<LIVE_NET_PLAYER> livePlayers =
		m_Registry.Get_LivePlayers();
	const std::shared_ptr<CCharacter> localCharacter =
		m_Registry.Resolve(m_LocalCharacterHandle);

	outPlayers.resize(livePlayers.size());
	for (std::size_t index = 0u; index < livePlayers.size(); ++index)
	{
		const LIVE_NET_PLAYER& source = livePlayers[index];
		REPLICATED_PLAYER_VIEW& target = outPlayers[index];
		target.iPlayerId = source.Record.iPlayerId;
		target.iNetEntityId = source.Record.iNetEntityId;
		target.eCharacterClass = source.Record.eCharacterClass;
		target.eControlKind = source.Record.eControlKind;
		target.strNickname = source.Record.strNickName;
		if (const auto title = m_HonorTitleByNetEntityId.find(source.Record.iNetEntityId);
			m_HonorTitleByNetEntityId.end() != title)
			target.iHonorTitleId = title->second;
		target.isLocal = nullptr != localCharacter &&
			source.pCharacter == localCharacter;
		target.pCharacter = source.pCharacter;
	}

	std::sort(
		outPlayers.begin(),
		outPlayers.end(),
		[](const REPLICATED_PLAYER_VIEW& left,
			const REPLICATED_PLAYER_VIEW& right)
		{
			return left.iNetEntityId < right.iNetEntityId;
		});
}

void Client::CClientReplication::Sync_GlobalCombatDebugVisibility()
{
	const COMBAT_DEBUG_VISIBILITY_SNAPSHOT Visibility =
		Get_GlobalCombatDebugVisibility();
	if (Visibility.iRevision == m_CombatDebugVisibility.iRevision)
		return;
	Apply_CombatDebugVisibility(Visibility);
}

void Client::CClientReplication::Apply_CombatDebugVisibility(
	const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Visibility)
{
	m_CombatDebugVisibility = Visibility;
	for (const std::shared_ptr<CCharacter>& character :
		m_Registry.Get_LiveObjects())
	{
		if (nullptr != character)
		{
			character->Set_CombatColliderDebugVisible(Visibility.bPlayerBodyCollider);
#ifdef _DEBUG
			character->Set_SkillHitAreaDebugVisible(
				Visibility.bPlayerSkillHitGeometry);
#endif
		}
	}
	for (auto& [netEntityId, presentation] : m_WorldEntities)
	{
		(void)netEntityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS == presentation.eKind)
		{
			if (const std::shared_ptr<CNpc> boss = presentation.pNpc.lock())
				boss->Set_CombatColliderDebugVisible(Visibility.bBossBodyCollider);
		}
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ENTITY_KIND::NPC == presentation.eKind)
		{
			if (const std::shared_ptr<CNpc> npc = presentation.pNpc.lock())
				npc->Set_SkillHitAreaDebugVisible(Visibility.bPlayerSkillHitGeometry);
		}
		if (std::shared_ptr<CValtan> valtan = presentation.pValtan.lock())
		{
			valtan->Set_CombatDebugVisibility(
				Visibility.bBossBodyCollider,
				Visibility.bBossPatternHitPulse,
				Visibility.bBossStageGeometry,
				Visibility.bCounterProxy);
		}
#endif
	}
}

#ifdef _DEBUG
bool_t Client::CClientReplication::Load_CombatObjectHitAreaDebug(
	std::string& strOutStatus)
{
	m_isCombatObjectHitAreaDebugLoadAttempted = true;
	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Encounters") / L"Valtan" /
		L"ValtanCombatObjects.json");
	if (Path.empty())
	{
		strOutStatus =
			"Combat-object hit Debug document resolved outside Project Data root.";
		return false;
	}
	std::ifstream Input(Path, std::ios::binary);
	if (!Input.is_open())
	{
		strOutStatus = "Combat-object hit Debug document is missing.";
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Text, Root, strOutStatus) || !Root.Is_Object())
	{
		if (strOutStatus.empty())
			strOutStatus = "Combat-object hit Debug document root is invalid.";
		return false;
	}

	const auto Field = [](
		const DATA_JSON_VALUE& Object,
		const char* const pName,
		const DATA_JSON_TYPE eType) -> const DATA_JSON_VALUE*
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	};
	const auto ReadString = [&Field](
		const DATA_JSON_VALUE& Object,
		const char* const pName,
		std::string& strOutValue) -> bool_t
	{
		const DATA_JSON_VALUE* pValue = Field(
			Object, pName, DATA_JSON_TYPE::STRING);
		if (nullptr == pValue || pValue->Get_String().empty())
			return false;
		strOutValue = pValue->Get_String();
		return true;
	};
	const auto ReadU32 = [&Field](
		const DATA_JSON_VALUE& Object,
		const char* const pName,
		std::uint32_t& iOutValue) -> bool_t
	{
		const DATA_JSON_VALUE* pValue = Field(
			Object, pName, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue)
			return false;
		const double Value = pValue->Get_Number();
		if (!std::isfinite(Value) || std::floor(Value) != Value ||
			Value < 0.0 || Value > static_cast<double>(
				(std::numeric_limits<std::uint32_t>::max)()))
		{
			return false;
		}
		iOutValue = static_cast<std::uint32_t>(Value);
		return true;
	};
	const auto ReadFloat = [&Field](
		const DATA_JSON_VALUE& Object,
		const char* const pName,
		f32_t& fOutValue) -> bool_t
	{
		const DATA_JSON_VALUE* pValue = Field(
			Object, pName, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			std::abs(pValue->Get_Number()) > 100000.0)
		{
			return false;
		}
		fOutValue = static_cast<f32_t>(pValue->Get_Number());
		return std::isfinite(fOutValue);
	};

	std::string Schema;
	std::string EncounterId;
	std::uint32_t iFormatVersion = 0u;
	const DATA_JSON_VALUE* pObjects = Field(
		Root, "objects", DATA_JSON_TYPE::ARRAY);
	if (!ReadString(Root, "schema", Schema) ||
		"lostark.valtan-combat-objects" != Schema ||
		!ReadU32(Root, "formatVersion", iFormatVersion) ||
		1u != iFormatVersion ||
		!ReadString(Root, "encounterId", EncounterId) ||
		"ENCOUNTER_VALTAN" != EncounterId || nullptr == pObjects ||
		pObjects->Get_Array().size() >
			LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT)
	{
		strOutStatus = "Combat-object hit Debug document identity is invalid.";
		return false;
	}

	std::unordered_map<std::string,
		std::vector<COMBAT_OBJECT_HIT_AREA_DEBUG>> Staged;
	for (const DATA_JSON_VALUE& Object : pObjects->Get_Array())
	{
		std::string ArchetypeId;
		std::uint32_t iLifeMs = 0u;
		const DATA_JSON_VALUE* pHits = Field(
			Object, "hits", DATA_JSON_TYPE::ARRAY);
		if (!Object.Is_Object() ||
			!ReadString(Object, "combatObjectArchetypeId", ArchetypeId) ||
			!ReadU32(Object, "lifeMs", iLifeMs) || 0u == iLifeMs ||
			nullptr == pHits || pHits->Get_Array().size() > 16u)
		{
			strOutStatus =
				"Combat-object hit Debug archetype row is invalid.";
			return false;
		}
		auto [Definition, bInserted] = Staged.emplace(
			ArchetypeId, std::vector<COMBAT_OBJECT_HIT_AREA_DEBUG>{});
		if (!bInserted)
		{
			strOutStatus =
				"Combat-object hit Debug archetype is duplicated: " +
				ArchetypeId;
			return false;
		}
		for (const DATA_JSON_VALUE& Hit : pHits->Get_Array())
		{
			COMBAT_OBJECT_HIT_AREA_DEBUG Area;
			std::string HitId;
			std::string Trigger;
			if (!Hit.Is_Object() || !ReadString(Hit, "hitId", HitId) ||
				!ReadString(Hit, "hitShape", Area.strHitShape) ||
				!ReadString(Hit, "trigger", Trigger) ||
				!ReadFloat(Hit, "hitOuterRadius", Area.fOuterRadiusM) ||
				!ReadFloat(Hit, "hitInnerRadius", Area.fInnerRadiusM) ||
				!ReadFloat(Hit, "hitAngleDegrees", Area.fAngleDegrees) ||
				!ReadFloat(Hit, "hitLength", Area.fLengthM) ||
				!ReadFloat(Hit, "hitHalfWidth", Area.fHalfWidthM) ||
				!ReadU32(Hit, "atMs", Area.iAtMs) ||
				!ReadU32(Hit, "repeatCount", Area.iRepeatCount) ||
				!ReadU32(Hit, "repeatIntervalMs", Area.iRepeatIntervalMs) ||
				("CONTACT" != Trigger && "TIMED" != Trigger) ||
				0u == Area.iRepeatCount || Area.iRepeatCount > 64u ||
				(1u == Area.iRepeatCount ? 0u != Area.iRepeatIntervalMs :
					0u == Area.iRepeatIntervalMs))
			{
				strOutStatus =
					"Combat-object hit Debug row is invalid: " + ArchetypeId;
				return false;
			}
			Area.bContact = "CONTACT" == Trigger;
			const bool_t bZeroRadii = 0.f == Area.fOuterRadiusM &&
				0.f == Area.fInnerRadiusM;
			const bool_t bZeroDirectional = 0.f == Area.fAngleDegrees &&
				0.f == Area.fLengthM && 0.f == Area.fHalfWidthM;
			const bool_t bValidShape =
				("CIRCLE" == Area.strHitShape &&
				 Area.fOuterRadiusM > 0.f && 0.f == Area.fInnerRadiusM &&
				 bZeroDirectional) ||
				("RING" == Area.strHitShape &&
				 Area.fOuterRadiusM > Area.fInnerRadiusM &&
				 Area.fInnerRadiusM > 0.f && bZeroDirectional) ||
				("CONE" == Area.strHitShape && bZeroRadii &&
				 Area.fAngleDegrees > 0.f && Area.fAngleDegrees <= 180.f &&
				 Area.fLengthM > 0.f && 0.f == Area.fHalfWidthM) ||
				("BOX" == Area.strHitShape && bZeroRadii &&
				 0.f == Area.fAngleDegrees && Area.fLengthM > 0.f &&
				 Area.fHalfWidthM > 0.f);
			const std::uint64_t iLastPulseMs =
				static_cast<std::uint64_t>(Area.iAtMs) +
				static_cast<std::uint64_t>(Area.iRepeatCount - 1u) *
				Area.iRepeatIntervalMs;
			if (!bValidShape || iLastPulseMs >= iLifeMs)
			{
				strOutStatus =
					"Combat-object hit Debug shape or clock is invalid: " +
					ArchetypeId + "/" + HitId;
				return false;
			}
			Definition->second.push_back(std::move(Area));
		}
	}

	m_CombatObjectHitAreasByArchetype = std::move(Staged);
	strOutStatus = "Combat-object hit Debug geometry loaded.";
	return true;
}

void Client::CClientReplication::Draw_CombatObjectHitAreaDebug()
{
	if (0u == m_CombatObjectProjectionRuntime.Get_Count() ||
		0u == m_iLastServerTick)
	{
		return;
	}
	if (!m_isCombatObjectHitAreaDebugLoadAttempted)
	{
		std::string Status;
		if (!Load_CombatObjectHitAreaDebug(Status))
		{
			OutputDebugStringA((
				"[Client][CombatObjectDebug] " + Status + "\n").c_str());
			return;
		}
	}
	if (m_CombatObjectHitAreasByArchetype.empty())
		return;

	constexpr std::uint32_t COMBAT_OBJECT_HIT_COLOR_RGBA =
		40u | (255u << 8) | (90u << 16) | (255u << 24);
	constexpr f32_t METERS_TO_UNITS = 100.f;
	const auto ToUnits = [](const f32_t fMeters)
	{
		return static_cast<std::int32_t>(
			fMeters * METERS_TO_UNITS + 0.5f);
	};
	m_CombatObjectProjectionRuntime.Visit_Records(
		[&](const COMBAT_OBJECT_PROJECTION_RECORD& Record)
		{
			const auto Definition = m_CombatObjectHitAreasByArchetype.find(
				Record.strCombatObjectArchetypeId);
			if (m_CombatObjectHitAreasByArchetype.end() == Definition)
				return;

			float4x4_t Root{};
			XMStoreFloat4x4(
				&Root,
				XMMatrixRotationY(XMConvertToRadians(
					Record.Snapshot.fYawDegrees)) *
				XMMatrixTranslation(
					Record.Snapshot.fPositionX,
					Record.Snapshot.fPositionY,
					Record.Snapshot.fPositionZ));
			for (const COMBAT_OBJECT_HIT_AREA_DEBUG& Area :
				Definition->second)
			{
				if (!COMBAT_OBJECT_HIT_DEBUG_CLOCK::Is_Visible(
						Area.bContact, m_iLastServerTick, Record.iSpawnTick,
						Area.iAtMs, Area.iRepeatCount,
						Area.iRepeatIntervalMs))
				{
					continue;
				}

				HIT_AREA_SHAPE Shape{};
				if ("CIRCLE" == Area.strHitShape ||
					"RING" == Area.strHitShape)
				{
					Shape.iAreaType = 1;
					Shape.iAreaRange = ToUnits(Area.fOuterRadiusM);
					Shape.iAreaInner = ToUnits(Area.fInnerRadiusM);
				}
				else if ("CONE" == Area.strHitShape)
				{
					Shape.iAreaType = 3;
					Shape.iAreaRange = ToUnits(Area.fLengthM);
					Shape.iAreaAngle = static_cast<std::int32_t>(
						Area.fAngleDegrees + 0.5f);
				}
				else if ("BOX" == Area.strHitShape)
				{
					Shape.iAreaType = 2;
					Shape.iAreaRange = ToUnits(Area.fLengthM);
					Shape.iAreaAngle = ToUnits(Area.fHalfWidthM * 2.f);
				}
				CHitAreaWire::Draw(
					Root, Shape, COMBAT_OBJECT_HIT_COLOR_RGBA);
			}
		});
}
#endif

bool Client::CClientReplication::Create_Character(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const LostArk::Shared::PLAYER_MADNESS_FORM madnessForm,
	const std::string_view nickName,
	const float3_t& position,
	const f32_t yawDegrees,
	const bool_t isLocallyControlled,
	std::shared_ptr<CCharacter>& outCharacter)
{
	outCharacter.reset();
	const CHARACTER_SPEC* spec = nullptr;
	if (LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN == madnessForm)
	{
		/* The clown body is the dedicated MN_RPCZ_00-1 polymorph rig. It is
		admitted once per level like an arena boss body; the class stays the
		wearer's so its quick slots keep resolving. */
		if (FAILED(CKoukuSaydonPresentationAssetService::Ensure_ClownBodyPrototype(
			m_Desc.pDevice,
			m_Desc.pContext,
			m_Desc.iPrototypeLevelIndex)))
		{
			return false;
		}
		spec = CCharacterCatalog::Find_ClownSpec();
	}
	else
	{
		if (!CPlayableCharacterAssetService::Is_Ready(
			m_Desc.iPrototypeLevelIndex, characterClass))
		{
			return false;
		}
		spec = CCharacterCatalog::Find_Spec(characterClass);
	}
	if (nullptr == spec)
		return false;

	CCharacter::CHARACTER_DESC desc{};
	desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
	desc.pSpec = spec;
	desc.eCharacterClass = characterClass;
	desc.pNavigationPrototypeTag =
		isLocallyControlled ?
			m_strLocalPlayerNavigationPrototypeTag.c_str() : nullptr;
	desc.fSpeedPerSec = 6.f;
	desc.fRotationPerSec = 180.f;
	desc.vPosition = position;
	desc.strNickName = nickName;
	desc.isLocallyControlled = isLocallyControlled;

	std::shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		m_Desc.iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Character"),
		m_Desc.iLayerLevelIndex,
		m_Desc.strPlayerLayerTag,
		&desc,
		&gameObject)))
	{
		return false;
	}

	const std::shared_ptr<CCharacter> character =
		std::dynamic_pointer_cast<CCharacter>(gameObject);
	if (nullptr == character || nullptr == character->Get_Transform())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			gameObject);
		return false;
	}

	character->Get_Transform()->Rotation(0.f, yawDegrees, 0.f);
	character->Set_CombatColliderDebugVisible(m_CombatDebugVisibility.bPlayerBodyCollider);
#ifdef _DEBUG
	character->Set_SkillHitAreaDebugVisible(
		m_CombatDebugVisibility.bPlayerSkillHitGeometry);
#endif
	outCharacter = character;
	return true;
}

bool Client::CClientReplication::Apply_Spawn(
	const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
{
	//?쒕쾭 player瑜?client character濡??앹꽦?쒕떎.
	//spawn message -> net player record 蹂??
	//?숈씪  entity id媛 ?대? ?덈뒗吏瑜?寃?ы븳??
	//character catalog?먯꽌 class spec 寃??->
	//character desc 援ъ꽦 -> local remote ?먯젙
	//add gameobject to layer -> character 罹먯뒪??諛?transform 寃??
	//yaw 諛섏쁺 -> registry ?깅줉 -> local?대㈃ localcharacterhandle ???

	const NET_PLAYER_RECORD stagedRecord =
		Make_Record(spawned);

	if (const NET_PLAYER_RECORD* existing =
		m_Registry.Find_Record(spawned.iNetEntityId))
	{
		//媛숈? event ?ъ쟾?≪? no-op, ?ㅻⅨ ?댁슜??媛숈? id?? protocol conflict?대떎.
		return Is_Same_Record(*existing, stagedRecord);
	}

	const auto pending = m_PendingPlayerSpawns.find(spawned.iNetEntityId);
	if (pending != m_PendingPlayerSpawns.end())
		return Is_Same_Record(Make_Record(pending->second), stagedRecord);
	if (!CPlayableCharacterAssetService::Is_Ready(m_Desc.iPrototypeLevelIndex, spawned.eCharacterClass))
	{
		if (m_PendingPlayerSpawns.size() >= LostArk::Shared::MAX_WORLD_SNAPSHOT_PLAYERS) return false;
		m_PendingPlayerSpawns.emplace(spawned.iNetEntityId, spawned);
		return true;
	}
	return Commit_PlayerSpawn(spawned);
}

bool Client::CClientReplication::Commit_PlayerSpawn(
	const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Network.PlayerPresentation.CommitSpawn");
	const NET_PLAYER_RECORD stagedRecord = Make_Record(spawned);
	const bool_t isLocallyControlled =
		spawned.iPlayerId ==
		CNetworkManager::Get().Get_LocalPlayerId();
	std::shared_ptr<CCharacter> character;
	/* Spawn carries no form; the first snapshot replaces the body if the
	Server already presents this player as a clown. */
	if (!Create_Character(
		spawned.eCharacterClass,
		LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL,
		spawned.strNickName,
		float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ),
		spawned.fYawDegrees,
		isLocallyControlled,
		character))
	{
		return false;
	}

	OBJECT_HANDLE handle{};

	if (!m_Registry.Register(
		stagedRecord,
		character,
		handle))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character);
		return false;
	}

	if (isLocallyControlled)
	{
		m_LocalCharacterHandle = handle;
		CAnimationTargetService::Bind(character);
	}

	return true;
}

bool Client::CClientReplication::Apply_Despawn(
	const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned)
{
	CCombatHUDViewModel::Get().Remove_WorldHealthBar(despawned.iNetEntityId);
	m_PendingPlayerSpawns.erase(despawned.iNetEntityId);
	m_PendingPlayerPresentations.erase(despawned.iNetEntityId);
	m_FailedPlayerSpawnClasses.erase(despawned.iNetEntityId);
	m_PlayerHealth.Erase(despawned.iNetEntityId);
	m_ChatBubblesByNetEntityId.erase(despawned.iNetEntityId);
	m_GuidePromptSequences.erase(despawned.iNetEntityId);
	m_PendingGuideBubbles.erase(despawned.iNetEntityId);
	if (m_GuideState && m_GuideState->iGuideNetEntityId == despawned.iNetEntityId) m_GuideState.reset();
	OBJECT_HANDLE handle{};

	if (!m_Registry.Find_Handle(
		despawned.iNetEntityId, handle))
	{
		//?대? ?쒓굅??以묐났 despawn? no-op?쇰줈 痍④툒
		return true;
	}

	const std::shared_ptr<CCharacter> character =
		m_Registry.Resolve(handle);
	//gameobject layer?먯꽌 ?쒓굅
	if (nullptr != character &&
		FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character)))
	{
		return false;
	}

	if (!m_Registry.Unregister(despawned.iNetEntityId))
		return false;
	if (m_LocalCharacterHandle.iSlotIndex == handle.iSlotIndex &&
		m_LocalCharacterHandle.iGeneration == handle.iGeneration)
	{
		if (nullptr != character)
			CAnimationTargetService::Unbind(character);
		m_LocalCharacterHandle = {};
		if (m_DeferredLocalCharacterClassReplacement.isPending &&
			m_DeferredLocalCharacterClassReplacement.Snapshot.iNetEntityId ==
				despawned.iNetEntityId)
		{
			Clear_DeferredLocalCharacterClassReplacement();
		}
	}

	return true;
}

bool Client::CClientReplication::Apply_WorldEntitySpawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED ownerIdentity{};
	const S2C_WORLD_ENTITY_SPAWNED* owner = nullptr;
	if (INVALID_NET_ENTITY_ID != spawned.iOwnerBossNetEntityId)
	{
		const auto foundOwner = m_WorldEntities.find(spawned.iOwnerBossNetEntityId);
		if (foundOwner == m_WorldEntities.end())
		{
			m_strPendingPresentationFailure = "Dependent boss arrived before its owner.";
			return false;
		}
		const std::shared_ptr<CValtan> owningBoss = foundOwner->second.pValtan.lock();
		const bool_t liveKoukuOwner =
			Is_KoukuSaydonArenaBoss(foundOwner->second.strArchetypeId, foundOwner->second.strEncounterId,
				foundOwner->second.iOwnerBossNetEntityId) && !foundOwner->second.pNpc.expired() &&
			spawned.strArchetypeId == foundOwner->second.strArchetypeId;
		if ((!liveKoukuOwner && (nullptr == owningBoss || CValtan::DEAD == owningBoss->Get_State())) ||
			foundOwner->second.bPresentationIsolated)
		{
			m_strPendingPresentationFailure =
				"Dependent boss has no admitted live owner presentation.";
			return false;
		}
		ownerIdentity.iNetEntityId = foundOwner->first;
		ownerIdentity.iOwnerBossNetEntityId = foundOwner->second.iOwnerBossNetEntityId;
		ownerIdentity.eKind = foundOwner->second.eKind;
		ownerIdentity.strEncounterId = foundOwner->second.strEncounterId;
		ownerIdentity.PinnedDefinitionRevision =
			foundOwner->second.PinnedDefinitionRevision;
		owner = &ownerIdentity;
	}
	if (!Is_Valid_WorldEntitySpawnOwner(spawned, owner) ||
		(("BOSS_VALTAN_GHOST" == spawned.strArchetypeId && INVALID_NET_ENTITY_ID == spawned.iOwnerBossNetEntityId) ||
		 (INVALID_NET_ENTITY_ID != spawned.iOwnerBossNetEntityId && "BOSS_VALTAN_GHOST" != spawned.strArchetypeId &&
		  !Is_KoukuSaydonDependentArchetype(spawned.strArchetypeId))))
	{
		m_strPendingPresentationFailure = "Invalid dependent boss ownership graph.";
		return false;
	}
	const auto existing = m_WorldEntities.find(spawned.iNetEntityId);
	if (existing != m_WorldEntities.end())
	{
			const bool_t hasLivePresentation =
				(WORLD_ENTITY_KIND::NPC == existing->second.eKind &&
					!existing->second.pNpc.expired()) ||
				(WORLD_ENTITY_KIND::MONSTER == existing->second.eKind &&
					!existing->second.pNpc.expired()) ||
				(WORLD_ENTITY_KIND::BOSS == existing->second.eKind &&
				(!existing->second.pValtan.expired() ||
				 (Is_KoukuSaydonArenaBoss(
					existing->second.strArchetypeId,
					existing->second.strEncounterId,
					existing->second.iOwnerBossNetEntityId) &&
				  !existing->second.pNpc.expired())));
		return hasLivePresentation &&
			existing->second.eKind == spawned.eKind &&
			existing->second.strArchetypeId == spawned.strArchetypeId &&
			existing->second.strEncounterId == spawned.strEncounterId &&
			existing->second.strPlacementId == spawned.strPlacementId &&
			existing->second.iOwnerBossNetEntityId == spawned.iOwnerBossNetEntityId &&
			existing->second.fCollisionRadius == spawned.fCollisionRadius &&
			existing->second.PinnedDefinitionRevision ==
				spawned.PinnedDefinitionRevision;
	}

	if (WORLD_ENTITY_KIND::NPC == spawned.eKind)
	{
		const NPC_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Npc(spawned.strArchetypeId);
		const wstring_t modelTag =
			CNpcPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		if (nullptr == actor || modelTag.empty() ||
			FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice,
				m_Desc.pContext,
				m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			/* A silent return hid every failed NPC: the placement never
			appeared and no status named it. Report the placement, archetype
			and the failing step so a missing NPC is diagnosable. */
			Write_EffectFailureDiagnostic("npc.presentation.unavailable",
				"placement=" + spawned.strPlacementId + " archetype=" + spawned.strArchetypeId +
				" reason=" + (nullptr == actor ?
					"no catalog entry: " + CActorCatalog::Get_Status() :
					modelTag.empty() ?
						std::string("no model prototype tag") :
						std::string("model prototype preparation failed (model admission, see npc.model.unit)")));
			OutputDebugStringA(("[NpcPresentation] placement " +
				spawned.strPlacementId + " archetype " +
				spawned.strArchetypeId + " is unavailable (" +
				(nullptr == actor ?
					"no catalog entry: " + CActorCatalog::Get_Status() :
					modelTag.empty() ?
						std::string("no model prototype tag") :
						std::string("model prototype preparation failed")) +
				").\n").c_str());
			return false;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag = actor->shaderProfile == "esther" ?
			TEXT("Prototype_Component_Shader_VtxEstherNpc") :
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		NPC_PLACEMENT_PRESENTATION_ENTRY placementPresentation;
		const bool_t hasPlacementPresentation =
			CNpcPlacementPresentationService::Try_Get_Presentation(
				m_Desc.iPrototypeLevelIndex,
				spawned.strPlacementId,
				placementPresentation);
		const std::string& resolvedIdle =
			hasPlacementPresentation &&
			!placementPresentation.strIdleClip.empty() ?
				placementPresentation.strIdleClip : actor->idleClip;
		desc.pIdleClip = resolvedIdle.c_str();
		/* Only authored placement behavior suppresses root motion. Esther
		summons have no placement document and retain their action-chain travel. */
		desc.bSuppressRootMotion = hasPlacementPresentation;
		desc.bInterpolateNetworkTransform = hasPlacementPresentation;
		desc.vPosition = float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;
		if (actor->shaderProfile == "esther")
			desc.fOutlineWidth = CNpc::ESTHER_OUTLINE_WIDTH;

		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Npc"),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
		{
			Write_EffectFailureDiagnostic("npc.presentation.unavailable",
				"placement=" + spawned.strPlacementId + " archetype=" + spawned.strArchetypeId +
				" reason=object creation failed");
			return false;
		}
		const std::shared_ptr<CNpc> npc =
			std::dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == npc || !npc->Apply_NetworkState(
			desc.vPosition, spawned.fYawDegrees))
		{
			Write_EffectFailureDiagnostic("npc.presentation.unavailable",
				"placement=" + spawned.strPlacementId + " archetype=" + spawned.strArchetypeId +
				" reason=network state apply failed");
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				gameObject);
			return false;
		}

		WORLD_ENTITY_PRESENTATION presentation{};
		presentation.eKind = spawned.eKind;
		presentation.strPlacementId = spawned.strPlacementId;
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = resolvedIdle;
		presentation.strResolvedIdleClip = resolvedIdle;
		presentation.NpcPresentation = std::move(placementPresentation);
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.PinnedDefinitionRevision =
			spawned.PinnedDefinitionRevision;
#ifdef _DEBUG
		npc->Set_SkillHitAreaDebugVisible(m_CombatDebugVisibility.bPlayerSkillHitGeometry);
#endif
		presentation.pNpc = npc;
		/* A missing Bern3 ship NPC has been invisible before: name each one that really got a body. */
		if (0 == spawned.strArchetypeId.rfind("NPC_SHIP_", 0))
			Write_EffectFailureDiagnostic("npc.ship.spawned",
				"placement=" + spawned.strPlacementId + " archetype=" + spawned.strArchetypeId +
				" pos=" + std::to_string(spawned.fPositionX) + "," + std::to_string(spawned.fPositionY) + "," +
				std::to_string(spawned.fPositionZ) + " idleClip=" + resolvedIdle);
		/* An entity that spawns mid-action (a raid Esther summon) must show
		its action clip from the very first rendered frame; waiting for the
		next snapshot leaves it one interval in the idle pose at the caster's
		feet before the clip teleports it into its authored entrance. */
		const auto spawnActionClip =
			actor->actionClips.find(spawned.strActionId);
		if (spawnActionClip != actor->actionClips.end() &&
			npc->Set_Animation(
				spawnActionClip->second.front().c_str(), false))
		{
			presentation.strActiveActionId = spawned.strActionId;
			presentation.iActionClipIndex = 0u;
			presentation.strCurrentClip = spawnActionClip->second.front();
			if (const std::shared_ptr<Engine::CModel> model =
				npc->Get_Model())
			{
				model->Play_Animation(0.f);
			}
			CCombatHUDViewModel::Get().Apply_EstherCutinAction(
				spawned.strArchetypeId);
		}
		else if (hasPlacementPresentation)
		{
			const NPC_ACTION_PLAYBACK_REQUEST request =
				CNpcActionPresentationRuntime::Resolve_Playback(
					spawned.strActionId,
					presentation.strResolvedIdleClip,
					presentation.NpcPresentation,
					nullptr);
			CNpcActionPresentationRuntime::Apply_Playback(
				request,
				presentation.strResolvedIdleClip,
				[&npc](const NPC_ACTION_PLAYBACK_REQUEST& playback)
				{
					return npc->Play_NetworkAction(
						playback.strClipName.c_str(),
						playback.isLoop,
						playback.fPlaybackRate,
						playback.fBlendSeconds);
				},
				[&npc](const f32_t blendSeconds)
				{
					return npc->Play_DefaultIdle(blendSeconds);
				},
				presentation.strCurrentClip);
		}
		const auto [iter, inserted] = m_WorldEntities.emplace(
			spawned.iNetEntityId, std::move(presentation));
		(void)iter;
		if (!inserted)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				npc);
		}
		return inserted;
	}

	if (WORLD_ENTITY_KIND::MONSTER == spawned.eKind)
	{
		const MONSTER_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Monster(spawned.strArchetypeId);
		if (nullptr == actor || actor->runtimeStatus != "supported" ||
			FAILED(CMonsterPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice,
				m_Desc.pContext,
				m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			return false;
		}

		const std::wstring modelTag =
			CMonsterPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pIdleClip = actor->presentationClips.idle.c_str();
		desc.bSuppressRootMotion = true;
		desc.bInterpolateNetworkTransform = true;
		desc.vPosition = float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;

		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			CMonsterPresentationAssetService::Get_GameObjectPrototypeTag(),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
		{
			return false;
		}
		const std::shared_ptr<CNpc> monster =
			std::dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == monster || !monster->Apply_NetworkState(
			desc.vPosition, spawned.fYawDegrees))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				gameObject);
			return false;
		}

		WORLD_ENTITY_PRESENTATION presentation{};
		presentation.eKind = spawned.eKind;
		presentation.strPlacementId = spawned.strPlacementId;
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = actor->presentationClips.idle;
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.PinnedDefinitionRevision =
			spawned.PinnedDefinitionRevision;
		presentation.pNpc = monster;
		const auto [iter, inserted] = m_WorldEntities.emplace(
			spawned.iNetEntityId, std::move(presentation));
		(void)iter;
		if (!inserted)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				monster);
		}
		return inserted;
	}

	const BOSS_ACTOR_ENTRY* pBoss =
		CActorCatalog::Find_Boss(spawned.strArchetypeId);
	if (spawned.eKind == WORLD_ENTITY_KIND::BOSS && nullptr != pBoss &&
		Is_KoukuSaydonArenaBoss(
			spawned.strArchetypeId,
			spawned.strEncounterId,
			spawned.iOwnerBossNetEntityId))
	{
		const std::wstring modelTag =
			CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		if (modelTag.empty() ||
			FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice, m_Desc.pContext, m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			m_strPendingPresentationFailure =
				"KoukuSaydon boss model admission failed: " +
				CKoukuSaydonPresentationAssetService::Get_Status();
			return false;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pIdleClip = pBoss->presentationClips.idle.c_str();
		desc.bSuppressRootMotion = true;
		desc.bInterpolateNetworkTransform = true;
		desc.vPosition = float3_t(
			spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;
		desc.strEffectV2BindingOwner = std::filesystem::path(pBoss->bodyModel).stem().string();
		/* A catalog weapon rides the body's socket bone in its rest pose. The
		prototype was admitted together with the body just above. */
		const std::wstring weaponTag =
			CKoukuSaydonPresentationAssetService::Get_WeaponModelPrototypeTag(
				spawned.strArchetypeId);
		if (!weaponTag.empty())
		{
			desc.strWeaponModelTag = weaponTag;
			desc.pWeaponSocketBone =
				CKoukuSaydonPresentationAssetService::Get_WeaponSocketBone();
		}

		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag(),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
		{
			return false;
		}
		const std::shared_ptr<CNpc> boss =
			std::dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == boss || !boss->Apply_NetworkState(
			desc.vPosition, spawned.fYawDegrees))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag,
				gameObject);
			return false;
		}

		WORLD_ENTITY_PRESENTATION presentation{};
		presentation.eKind = spawned.eKind;
		presentation.strPlacementId = spawned.strPlacementId;
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = pBoss->presentationClips.idle;
		presentation.strResolvedIdleClip = pBoss->presentationClips.idle;
		presentation.iOwnerBossNetEntityId = spawned.iOwnerBossNetEntityId;
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.PinnedDefinitionRevision =
			spawned.PinnedDefinitionRevision;
		boss->Set_CombatColliderDebugVisible(m_CombatDebugVisibility.bBossBodyCollider);
		presentation.pNpc = boss;
		const auto [insertedAt, inserted] = m_WorldEntities.emplace(
			spawned.iNetEntityId, std::move(presentation));
		(void)insertedAt;
		if (!inserted)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag, boss);
		}
		return inserted;
	}
	if (spawned.eKind != WORLD_ENTITY_KIND::BOSS ||
		nullptr == pBoss ||
		pBoss->clientPresentationId != "boss.valtan.client.v1")
	{
		return false;
	}
	const bool_t isPooledGhost =
		"BOSS_VALTAN_GHOST" == spawned.strArchetypeId &&
		LostArk::Shared::INVALID_NET_ENTITY_ID !=
			spawned.iOwnerBossNetEntityId;
	const bool_t bHoldPortalRunnerBodyHiddenUntilPatternSnapshot =
		isPooledGhost &&
		"valtan.ghost.portal-once.active" == spawned.strActionId;
	if ((isPooledGhost &&
		 !CValtanPresentationAssetService::Is_Ready(
			 m_Desc.iPrototypeLevelIndex, spawned.strArchetypeId)) ||
		(!isPooledGhost &&
		 FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			 m_Desc.pDevice,
			 m_Desc.pContext,
			 m_Desc.iPrototypeLevelIndex,
			 spawned.strArchetypeId))))
	{
		m_strPendingPresentationFailure =
			"Boss model admission failed: " + spawned.strArchetypeId;
		return false;
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	const bool_t hasExactReceipt =
		CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			spawned.PinnedDefinitionRevision,
			PresentationReceipt, ReceiptStatus);
	if (isPooledGhost && !hasExactReceipt)
	{
		m_strPendingPresentationFailure =
			"Replicated ghost has no exact presentation pool receipt: " +
			ReceiptStatus;
		return false;
	}

	CValtan::VALTAN_DESC desc{};
	desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
	desc.vPosition = float3_t(
		spawned.fPositionX,
		spawned.fPositionY,
		spawned.fPositionZ);
	desc.fScale = pBoss->presentationScale;
	desc.isServerAuthoritative = true;
	desc.strArchetypeId = spawned.strArchetypeId;
	desc.iOwnerBossNetEntityId = spawned.iOwnerBossNetEntityId;
	desc.fCollisionRadius = spawned.fCollisionRadius;
	std::shared_ptr<CGameObject> gameObject;
	std::shared_ptr<CValtan> valtan;
	if (isPooledGhost)
	{
		valtan = Checkout_ValtanGhostPresentation(
			spawned.iOwnerBossNetEntityId,
			spawned.fCollisionRadius,
			spawned.PinnedDefinitionRevision,
			PresentationReceipt,
			desc.vPosition,
			spawned.fYawDegrees,
			bHoldPortalRunnerBodyHiddenUntilPatternSnapshot);
		gameObject = valtan;
		if (nullptr == valtan)
		{
			m_strPendingPresentationFailure =
				"Replicated Valtan ghost has no dormant pool slot for its exact presentation generation.";
			return false;
		}
	}
	else if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Valtan"),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
	{
		return false;
	}
	if (!isPooledGhost)
		valtan = std::dynamic_pointer_cast<CValtan>(gameObject);
	if (nullptr == valtan || !valtan->Apply_NetworkState(
		desc.vPosition,
		spawned.fYawDegrees,
		WORLD_ENTITY_ACTION::IDLE,
			{}, {}, 0u, 0u, 0u, 0u, {}, {}))
	{
		if (isPooledGhost)
			(void)Checkin_ValtanGhostPresentation(valtan);
		else
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				gameObject);
		}
		return false;
	}

	WORLD_ENTITY_PRESENTATION presentation{};
	presentation.eKind = spawned.eKind;
	presentation.strPlacementId = spawned.strPlacementId;
	presentation.strArchetypeId = spawned.strArchetypeId;
	presentation.strEncounterId = spawned.strEncounterId;
	presentation.fCollisionRadius = spawned.fCollisionRadius;
	presentation.PinnedDefinitionRevision = spawned.PinnedDefinitionRevision;
	presentation.pValtan = valtan;
	presentation.iOwnerBossNetEntityId = spawned.iOwnerBossNetEntityId;
	presentation.bUsesValtanGhostPool = isPooledGhost;
	const bool_t isPrimaryValtan = "BOSS_VALTAN" == spawned.strArchetypeId &&
		LostArk::Shared::INVALID_NET_ENTITY_ID ==
			spawned.iOwnerBossNetEntityId;
	std::string JoinedReloadStatus;
	std::string CombatObjectSoundReloadStatus;
	const bool_t entryReceiptRecoveryPending = !hasExactReceipt &&
		CNetworkManager::Get().Get_GameplayRevisionState().
			hasPendingEntryPresentationBaselineRecovery;
	const bool_t joinedReloaded = isPooledGhost ||
		(hasExactReceipt && valtan->Reload_PatternPresentationAuthoring(
			spawned.PinnedDefinitionRevision,
			PresentationReceipt, JoinedReloadStatus));
	if (isPooledGhost)
		JoinedReloadStatus =
			"Checked out an exact-revision dormant Valtan ghost presentation.";
	const bool_t combatObjectSoundReloaded = joinedReloaded;
	CombatObjectSoundReloadStatus = joinedReloaded ? JoinedReloadStatus :
		(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
	if (joinedReloaded)
	{
		presentation.AdmittedPresentationRevision =
			spawned.PinnedDefinitionRevision;
	}
	else
	{
		if (!entryReceiptRecoveryPending)
		{
			presentation.RejectedPresentationRevision =
				spawned.PinnedDefinitionRevision;
		}
		presentation.bPresentationIsolated = true;
	}
#ifdef _DEBUG
	valtan->Set_CombatDebugVisibility(
		m_CombatDebugVisibility.bBossBodyCollider,
		m_CombatDebugVisibility.bBossPatternHitPulse,
		m_CombatDebugVisibility.bBossStageGeometry,
		m_CombatDebugVisibility.bCounterProxy);
#endif
	if (!isPrimaryValtan && !joinedReloaded)
	{
		m_strPendingPresentationFailure =
			"Replicated Valtan presentation rejected its exact generation receipt: " +
			(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
		if (isPooledGhost)
			(void)Checkin_ValtanGhostPresentation(valtan);
		else
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				valtan);
		}
		return false;
	}
	const auto [iter, inserted] = m_WorldEntities.emplace(
		spawned.iNetEntityId,
		std::move(presentation));
	(void)iter;
	if (!inserted)
	{
		if (isPooledGhost)
			(void)Checkin_ValtanGhostPresentation(valtan);
		else
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				valtan);
		}
	}
	else if (isPrimaryValtan)
	{
		if (joinedReloaded)
		{
			m_PrimaryValtanJoinedPresentationFreshness.Admit(
				spawned.PinnedDefinitionRevision, JoinedReloadStatus);
		}
		else
		{
			const std::string Diagnostic =
				(entryReceiptRecoveryPending ?
					"Primary replicated Valtan spawn is waiting to recover the saved joined presentation cache: " :
					"Primary replicated Valtan spawn could not admit the joined presentation cache: ") +
				(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
			if (!entryReceiptRecoveryPending)
				m_PrimaryValtanJoinedPresentationFreshness.Reject(Diagnostic);
			m_strPendingPresentationFailure = Diagnostic;
		}
		if (combatObjectSoundReloaded)
		{
			m_PrimaryValtanCombatObjectSoundFreshness.Admit(
				spawned.PinnedDefinitionRevision,
				CombatObjectSoundReloadStatus);
		}
		else
		{
			const std::string Diagnostic =
				(entryReceiptRecoveryPending ?
					"Primary replicated Valtan spawn is waiting to recover the saved combat-object Sound cache: " :
					"Primary replicated Valtan spawn could not admit the combat-object Sound cache: ") +
				CombatObjectSoundReloadStatus;
			if (!entryReceiptRecoveryPending)
				m_PrimaryValtanCombatObjectSoundFreshness.Reject(Diagnostic);
			if (!m_strPendingPresentationFailure.empty())
				m_strPendingPresentationFailure += " ";
			m_strPendingPresentationFailure += Diagnostic;
		}
		if (joinedReloaded)
		{
			std::string PoolStatus;
			if (!Prepare_ValtanGhostPresentationPool(
					spawned.iNetEntityId,
					spawned.fCollisionRadius,
					spawned.PinnedDefinitionRevision,
					PresentationReceipt,
					valtan,
					PoolStatus))
			{
				if (!m_strPendingPresentationFailure.empty())
					m_strPendingPresentationFailure += " ";
				m_strPendingPresentationFailure +=
					"Ghost presentation pool preparation failed: " +
					PoolStatus;
			}
		}
	}
	return inserted;
}

void Client::CClientReplication::Remove_DependentBossPresentations(
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId)
{
	std::vector<LostArk::Shared::NET_ENTITY_ID> children;
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		if (presentation.iOwnerBossNetEntityId == ownerBossNetEntityId)
			children.push_back(entityId);
	}
	// Admission forbids nested ownership. Collect IDs before erasing map entries.
	for (const LostArk::Shared::NET_ENTITY_ID child : children)
	{
		LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED despawned{};
		despawned.iNetEntityId = child;
		Apply_WorldEntityDespawn(despawned);
	}
}

bool Client::CClientReplication::Apply_WorldEntityDespawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	using LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == despawned.iNetEntityId ||
		(despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::REMOVED &&
		 despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::DEAD))
	{
		return false;
	}
	CCombatHUDViewModel::Get().Remove_WorldHealthBar(despawned.iNetEntityId);
	const auto iter = m_WorldEntities.find(despawned.iNetEntityId);
	if (m_WorldEntities.end() == iter)
		return true;
	/* A DEAD despawn is the reliable terminal edge for a boss. The Server removes
	   the entity before it builds that tick's world snapshot, so a final snapshot
	   with eAction == DEAD is explicitly not guaranteed. Latch the primary boss
	   death before removing its presentation; dependent bosses must not complete
	   the raid. */
	if (WORLD_ENTITY_DESPAWN_REASON::DEAD == despawned.eReason &&
		LostArk::Shared::WORLD_ENTITY_KIND::BOSS == iter->second.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID ==
			iter->second.iOwnerBossNetEntityId)
	{
		CCombatHUDViewModel::Get().Set_BossDeadRaw(true);
	}
	Remove_DependentBossPresentations(despawned.iNetEntityId);
	const bool_t bPrimaryValtanDespawn =
		LostArk::Shared::WORLD_ENTITY_KIND::BOSS == iter->second.eKind &&
		"BOSS_VALTAN" == iter->second.strArchetypeId &&
		LostArk::Shared::INVALID_NET_ENTITY_ID ==
			iter->second.iOwnerBossNetEntityId;
	if (bPrimaryValtanDespawn)
		Clear_ValtanGhostPresentationPool();
	COMBAT_OBJECT_PRESENTATION_SINK combatObjectSink{ *this };
	const size_t removedCombatObjects =
		m_CombatObjectProjectionRuntime.Remove_Source(
			despawned.iNetEntityId, combatObjectSink);
	if (0u != removedCombatObjects)
	{
		m_strPendingPresentationFailure =
			"Combat-object owner despawned before reliable object cleanup.";
	}

	if (const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			npc);
	}
	if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS == iter->second.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId &&
		Is_KoukuSaydonArenaBoss(
			iter->second.strArchetypeId,
			iter->second.strEncounterId,
			iter->second.iOwnerBossNetEntityId))
	{
		/* Apply_Boss stops the moment the entity leaves the snapshot, so the
		bar of a despawned arena boss has to be dropped here explicitly. */
		CCombatHUDViewModel::Get().Clear_BossIfArchetype(
			iter->second.strArchetypeId);
	}
	if (const std::shared_ptr<CValtan> valtan = iter->second.pValtan.lock())
	{
		if (iter->second.bUsesValtanGhostPool)
		{
			if (!Checkin_ValtanGhostPresentation(valtan))
			{
				m_strPendingPresentationFailure =
					"Replicated Valtan ghost could not return to its dormant presentation slot.";
				return false;
			}
		}
		else if (!iter->second.bPresentationIsolated &&
			WORLD_ENTITY_DESPAWN_REASON::DEAD == despawned.eReason &&
			valtan->Begin_NetworkDeathPresentation())
		{
			// A reliable death starts even if its last DEAD snapshot never arrived.
			// Repeated despawns find no registry entry and cannot restart the clip.
			m_DeathPresentations.emplace_back(valtan);
		}
		else
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				valtan);
		}
	}
	if (m_ValtanPresentationState.isValid &&
		m_ValtanPresentationState.iNetEntityId == despawned.iNetEntityId)
	{
		m_ValtanPresentationState = {};
		CCombatHUDViewModel::Get().Clear_Boss();
	}
	if (m_Desc.onWorldEntityDespawned &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId)
	{
		m_Desc.onWorldEntityDespawned(
			iter->second.strPlacementId,
			iter->second.strArchetypeId);
	}
	m_WorldEntities.erase(iter);
	/* A despawn must not erase a reload rejection. Complete Play remains blocked
	   until Reset_World establishes a new world lifetime or a later primary spawn
	   successfully reloads the authoritative caches. */
	return true;
}

bool Client::CClientReplication::Apply_CombatObjectSpawn(
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	const auto source = m_WorldEntities.find(spawned.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		WORLD_ENTITY_KIND::BOSS != source->second.eKind ||
		(source->second.pValtan.expired() &&
		 !(source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") && !source->second.pNpc.expired())))
	{
		return false;
	}
	const bool targetedKouku = source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") &&
        CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(spawned.strCombatObjectArchetypeId);
    // The typed sink resolves the exact published visual ID. Keeping the logical
    // record lets an owner/resource that is still loading use the existing retry.
	if (!source->second.bPresentationIsolated && !targetedKouku &&
		nullptr == CActorCatalog::Find_BossCombatObjectVisual(
			source->second.strArchetypeId,
			spawned.strCombatObjectArchetypeId,
			spawned.strClientVisualId))
	{
		return false;
	}
	if (!source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") &&
		source->second.PinnedDefinitionRevision != spawned.PinnedDefinitionRevision)
	{
		m_strPendingPresentationFailure =
			"Combat-object spawn revision does not match its boss occurrence.";
		return false;
	}

	COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
	std::string status;
	if (!m_CombatObjectProjectionRuntime.Apply_Spawn(
		spawned, sink, status))
	{
		m_strPendingPresentationFailure = std::move(status);
		return false;
	}
	const COMBAT_OBJECT_PROJECTION_RECORD* record =
		m_CombatObjectProjectionRuntime.Find(spawned.iCombatObjectId);
	if (nullptr != record && !record->PresentationHandle.Is_Valid())
		m_strPendingPresentationFailure = std::move(status);
	return true;
}

bool Client::CClientReplication::Apply_CombatObjectPresentationEvent(
	const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
	const auto source = m_WorldEntities.find(event.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		LostArk::Shared::WORLD_ENTITY_KIND::BOSS != source->second.eKind)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event has no live boss owner.";
		return false;
	}
	if (source->second.bPresentationIsolated)
		return true;
	if (!source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") &&
		source->second.PinnedDefinitionRevision != event.PinnedDefinitionRevision)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event revision does not match its boss occurrence.";
		return false;
	}
	if (source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_"))
	{
		const auto* record = m_CombatObjectProjectionRuntime.Find(event.iCombatObjectId);
        if (event.strCombatObjectArchetypeId == "combatobject.kouku.pursuit")
        {
            if (source->second.pNpc.expired() || !record || !m_pTargetedCombatPresentationPlayer ||
                record->iSourceNetEntityId != event.iSourceNetEntityId ||
                record->strCombatObjectArchetypeId != event.strCombatObjectArchetypeId ||
                record->Snapshot.PinnedDefinitionRevision != event.PinnedDefinitionRevision)
            { m_strPendingPresentationFailure = "Pursuit contact has no matching replicated occurrence."; return false; }
            if (event.strHitId == "combatpresentation.kouku.pursuit.started")
            {
                if (event.eKind != LostArk::Shared::COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE ||
                    event.iRepeatIndex != 0u || !event.iEventSequence || !event.iServerTick ||
                    !std::isfinite(event.fPositionX) || !std::isfinite(event.fPositionY) ||
                    !std::isfinite(event.fPositionZ) || !std::isfinite(event.fYawDegrees))
                { m_strPendingPresentationFailure = "Pursuit lifecycle marker has an invalid kind, clock or pose."; return false; }
                // The admitted projection already owns the card; this marker starts no contact Effect.
                return true;
            }
            return m_pTargetedCombatPresentationPlayer->Play_TargetedCombatContact(event, m_strPendingPresentationFailure);
        }
		if (source->second.pNpc.expired() || !record || record->iSourceNetEntityId != event.iSourceNetEntityId ||
			record->Snapshot.PinnedDefinitionRevision != event.PinnedDefinitionRevision ||
			event.strCombatObjectArchetypeId != record->strCombatObjectArchetypeId ||
			event.eKind != LostArk::Shared::COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE ||
			event.iRepeatIndex != 0u ||
			!(CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(record->strCombatObjectArchetypeId)
				? event.strHitId == "combatpresentation.kouku.showtime.started"
				: (record->strCombatObjectArchetypeId == "combatobject.kouku.albion.bluecircle" &&
					record->strClientVisualId == "combatvisual.kouku.albion.bluecircle" &&
					event.strHitId == "combatpresentation.kouku.albion.started")))
		{ m_strPendingPresentationFailure = "Kouku combat-object lifecycle marker has no matching live occurrence."; return false; }
		// The runtime group already owns its warning and impact; do not replay it at this marker.
		return true;
	}
	const std::shared_ptr<CValtan> boss = source->second.pValtan.lock();
	if (nullptr == boss)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event boss projection expired.";
		return false;
	}
	const auto* record = m_CombatObjectProjectionRuntime.Find(event.iCombatObjectId);
	const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* visual = nullptr;
	if (record)
	{
		if (record->iSourceNetEntityId != event.iSourceNetEntityId ||
			record->strCombatObjectArchetypeId != event.strCombatObjectArchetypeId ||
			record->Snapshot.PinnedDefinitionRevision != event.PinnedDefinitionRevision)
		{
			m_strPendingPresentationFailure = "Combat-object event does not match its live occurrence.";
			return false;
		}
		visual = CActorCatalog::Find_BossCombatObjectVisual(source->second.strArchetypeId,
			record->strCombatObjectArchetypeId, record->strClientVisualId);
	}
	std::string status;
	const bool_t applied =
		boss->Apply_CombatObjectPresentationEvent(event, status);
	if (applied && visual && visual->stopActiveOnHit &&
		event.strHitId != visual->armedPresentationEventId)
	{
		COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
		m_CombatObjectProjectionRuntime.Complete_Presentation(event.iCombatObjectId, sink);
	}
	if (!applied)
		m_strPendingPresentationFailure = std::move(status);
	return applied;
}

bool Client::CClientReplication::Apply_CombatObjectDespawn(
	const LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED& despawned)
{
	COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
	// Terminal stone roots must also stop if the reliable despawn precedes the
	// self-contained hit pulse, or if an armed sequence is cancelled.
	if (const auto* record = m_CombatObjectProjectionRuntime.Find(despawned.iCombatObjectId))
	{
		const auto owner = m_WorldEntities.find(record->iSourceNetEntityId);
		if (owner != m_WorldEntities.end())
		{
			const auto* visual = CActorCatalog::Find_BossCombatObjectVisual(
				owner->second.strArchetypeId, record->strCombatObjectArchetypeId, record->strClientVisualId);
			if (visual && visual->stopActiveOnHit)
				m_CombatObjectProjectionRuntime.Complete_Presentation(despawned.iCombatObjectId, sink);
		}
	}
	std::string status;
	const bool_t applied = m_CombatObjectProjectionRuntime.Apply_Despawn(
		despawned, sink, status);
	if (!applied)
		m_strPendingPresentationFailure = std::move(status);
	return applied;
}

bool Client::CClientReplication::Spawn_CombatObjectPresentation(
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
	COMBAT_OBJECT_PRESENTATION_HANDLE& outHandle,
	std::string& outStatus)
{
	outHandle.Reset();
	const auto source = m_WorldEntities.find(spawned.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		source->second.eKind != LostArk::Shared::WORLD_ENTITY_KIND::BOSS)
	{
		outStatus = "Combat-object visual has no live boss owner.";
		return false;
	}
	if (source->second.bPresentationIsolated)
	{
		outStatus =
			"Combat-object visual is isolated with its rejected boss presentation revision.";
		return true;
	}
	if (!source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") &&
		source->second.PinnedDefinitionRevision != spawned.PinnedDefinitionRevision)
	{
		outStatus =
			"Combat-object visual revision does not match its boss occurrence.";
		return false;
	}
    const bool koukuOwner = source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") && !source->second.pNpc.expired();
    if (koukuOwner && CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(spawned.strCombatObjectArchetypeId))
    {
        if (!m_pTargetedCombatPresentationPlayer)
        { outStatus = "Targeted Kouku presentation owner is not ready."; return false; }
        if (!m_pTargetedCombatPresentationPlayer->Start_TargetedCombatVisual(spawned, outStatus)) return false;
        outHandle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::KOUKU_TARGETED_GROUP;
        outHandle.iValue = spawned.iCombatObjectId;
        return true;
    }
	const std::shared_ptr<CValtan> boss = source->second.pValtan.lock();
	const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* visual =
		CActorCatalog::Find_BossCombatObjectVisual(
			source->second.strArchetypeId,
			spawned.strCombatObjectArchetypeId,
			spawned.strClientVisualId);
	if ((!boss && !koukuOwner) || nullptr == visual)
	{
		outStatus = "Combat-object visual join is missing.";
		return false;
	}

	float actionAgeSeconds = 0.f;
	if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
		spawned.iServerTick,
		spawned.iSpawnTick,
		30.f,
		actionAgeSeconds))
	{
		outStatus = "Combat-object spawn tick is not seekable.";
		return false;
	}

	EFFECT_WORLD_ROOT_SPAWN_DESC desc;
	float4x4_t rootWorld = visual->Make_WorldRoot(
		float3_t(spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ),
		spawned.fYawDegrees);
	XMStoreFloat4x4(&rootWorld, XMMatrixScaling(spawned.fUniformScale, spawned.fUniformScale, spawned.fUniformScale) * XMLoadFloat4x4(&rootWorld));
	if (BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V2_GROUP ==
		visual->activeEffectKind)
	{
		CEffectV2Catalog& catalog = CEffectV2Catalog::Get();
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot =
			catalog.Get_Snapshot();
		if (nullptr == snapshot || !snapshot->Is_Ready())
		{
			if (!catalog.Reload_BossValtan(outStatus))
				return false;
			snapshot = catalog.Get_Snapshot();
		}
		const EFFECT_V2_GROUP* group = nullptr == snapshot ? nullptr :
			snapshot->Find_Group(visual->effectV2Group.groupId);
		if (nullptr == group)
		{
			outStatus = "Combat-object Effect V2 group is missing from the pinned catalog.";
			return false;
		}
		EFFECT_V2_GROUP_PLAYBACK_DESC playback;
		playback.PivotWorld = rootWorld;
		playback.fInitialAgeSeconds = actionAgeSeconds;
		playback.fPlaybackRate = visual->effectV2Group.playbackRate;
		playback.bProductOwned = true;
		const uint32_t groupHandle = CEffectV2Runtime::Play_Group(
			*group, std::move(snapshot), playback, m_Desc.pDevice,
			m_Desc.pContext);
		if (0u == groupHandle)
		{
			outStatus = CEffectV2Runtime::Last_Error();
			return false;
		}
		outHandle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V2_GROUP;
		outHandle.iValue = groupHandle;
		outStatus = "Spawned combat-object Effect V2 group.";
		return true;
	}

	if (koukuOwner)
	{
		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC placement;
		placement.iLevelIndex = m_Desc.iLayerLevelIndex;
		placement.strPlacementId = "combatobject.instance." + std::to_string(spawned.iCombatObjectId);
		placement.strEffectAssetId = visual->effectAssetId;
		placement.RootWorld = rootWorld;
		placement.iSpawnTick = spawned.iSpawnTick;
		placement.fInitialSampleTimeSeconds = actionAgeSeconds;
		EFFECT_WORLD_ROOT_HANDLE handle;
		if (!CEffectPresentationService::Spawn_LevelPlacement(placement, handle, outStatus))
			return false;
		outHandle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_LEVEL_ROOT;
		outHandle.iValue = handle.iValue;
		return true;
	}
	desc.strEffectAssetId = visual->effectAssetId;
	desc.pBossBudgetAndLifetimeOwner = boss;
	desc.RootWorld = rootWorld;
	desc.strOccurrenceId = "combatobject.instance." +
		std::to_string(spawned.iCombatObjectId);
	desc.iSpawnTick = spawned.iSpawnTick;
	desc.fInitialSampleTimeSeconds = actionAgeSeconds;
	EFFECT_WORLD_ROOT_HANDLE handle;
	if (!CEffectPresentationService::Spawn_WorldRoot(
		desc, handle, outStatus))
	{
		return false;
	}
	outHandle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_WORLD_ROOT;
	outHandle.iValue = handle.iValue;
	return true;
}

bool Client::CClientReplication::Update_CombatObjectPresentation(
	const COMBAT_OBJECT_PRESENTATION_HANDLE handle,
	const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot, const std::uint32_t serverTick)
{
	const COMBAT_OBJECT_PROJECTION_RECORD* record =
		m_CombatObjectProjectionRuntime.Find(snapshot.iCombatObjectId);
	const auto source = m_WorldEntities.find(snapshot.iSourceNetEntityId);
	if (nullptr == record || source == m_WorldEntities.end() ||
		record->iSourceNetEntityId != snapshot.iSourceNetEntityId ||
		record->PresentationHandle != handle)
	{
		return false;
	}
	if (source->second.bPresentationIsolated)
		return true;
	if (record->Snapshot.PinnedDefinitionRevision != snapshot.PinnedDefinitionRevision ||
		(!source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") &&
		 source->second.PinnedDefinitionRevision != snapshot.PinnedDefinitionRevision))
	{
		return false;
	}
    if (COMBAT_OBJECT_PRESENTATION_KIND::KOUKU_TARGETED_GROUP == handle.eKind)
    {
        if (!m_pTargetedCombatPresentationPlayer || handle.iValue != snapshot.iCombatObjectId ||
            !source->second.strArchetypeId.starts_with("BOSS_KAKULSAYDON_") ||
            !CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(record->strCombatObjectArchetypeId))
            return false;
        std::string status;
        const bool updated = m_pTargetedCombatPresentationPlayer->Update_TargetedCombatVisual(snapshot, serverTick, status);
        if (!updated) m_strPendingPresentationFailure = std::move(status);
        return updated;
    }
	const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* visual =
		CActorCatalog::Find_BossCombatObjectVisual(source->second.strArchetypeId,
			record->strCombatObjectArchetypeId, record->strClientVisualId);
	if (nullptr == visual)
		return false;
	float4x4_t rootWorld = visual->Make_WorldRoot(
		float3_t(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ),
		snapshot.fYawDegrees);
	XMStoreFloat4x4(&rootWorld, XMMatrixScaling(record->fUniformScale, record->fUniformScale, record->fUniformScale) * XMLoadFloat4x4(&rootWorld));
	if (COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V2_GROUP == handle.eKind)
	{
		if (BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V2_GROUP !=
			visual->activeEffectKind)
		{
			return false;
		}
		std::string failure;
		if (CEffectV2Runtime::Consume_GroupFailure(
				static_cast<uint32_t>(handle.iValue), failure))
		{
			m_strPendingPresentationFailure = std::move(failure);
			return false;
		}
		/* Natural completion is successful while the logical combat object remains
		   alive; do not restart the one-shot group on every later snapshot. */
		if (CEffectV2Runtime::Group_Seconds(
				static_cast<uint32_t>(handle.iValue)) < 0.f)
		{
			return true;
		}
		CEffectV2Runtime::Set_GroupPivot(
			static_cast<uint32_t>(handle.iValue), rootWorld);
		return true;
	}
	if ((COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_WORLD_ROOT != handle.eKind &&
		 COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_LEVEL_ROOT != handle.eKind) ||
		BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 !=
			visual->activeEffectKind)
	{
		return false;
	}
	EFFECT_WORLD_ROOT_HANDLE effectHandle;
	effectHandle.iValue = handle.iValue;
	return CEffectPresentationService::Update_WorldRoot(effectHandle, rootWorld);
}

void Client::CClientReplication::Stop_CombatObjectPresentation(
	const COMBAT_OBJECT_PRESENTATION_HANDLE handle)
{
    if (COMBAT_OBJECT_PRESENTATION_KIND::KOUKU_TARGETED_GROUP == handle.eKind)
    {
        if (m_pTargetedCombatPresentationPlayer)
            m_pTargetedCombatPresentationPlayer->Stop_TargetedCombatVisual(
                static_cast<LostArk::Shared::COMBAT_OBJECT_ID>(handle.iValue));
        return;
    }
	if (COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V2_GROUP == handle.eKind)
	{
		CEffectV2Runtime::Stop_Group(static_cast<uint32_t>(handle.iValue));
		return;
	}
	EFFECT_WORLD_ROOT_HANDLE effectHandle;
	effectHandle.iValue = handle.iValue;
	CEffectPresentationService::Stop_WorldRoot(effectHandle);
}

void Client::CClientReplication::Release_CombatObjectPresentation(
	const COMBAT_OBJECT_PRESENTATION_HANDLE handle)
{
	// Level-owned groups have no weak boss owner. Their Server lifetime includes the
	// full visual tail, so any despawn can stop both active and pending elements.
	if (COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_LEVEL_ROOT == handle.eKind ||
        COMBAT_OBJECT_PRESENTATION_KIND::KOUKU_TARGETED_GROUP == handle.eKind)
	{
		Stop_CombatObjectPresentation(handle);
		return;
	}
	/* Server despawn (lifetimeMs) releases the root pose without cutting the
	   visual. A V2 group owns a bounded clock and stops here. A V1 world root
	   was spawned with EFFECT_STOP_POLICY::NATURAL, so its elements finish on
	   their authored lifetime and CEffectPresentationService removes the
	   occurrence on Is_Finished(), boss owner loss, or a level change. */
	if (COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V2_GROUP == handle.eKind)
		CEffectV2Runtime::Stop_Group(static_cast<uint32_t>(handle.iValue));
}

bool Client::CClientReplication::Apply_WorldSnapshot(
	const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot)
{
	//server tick ?좏슚??寃??-> 留덉?留?tick蹂대떎 ??snapshot?몄? 寃??
	//->snapshot??player 諛곗뿴 ?쒗쉶 -> netentityid濡?objecthandle 寃??
	//handle濡??댁븘 ?덈뒗 character resolve
	//?꾩튂 float3 ?앹꽦 -> locomotion??bool ismoving?쇰줈 蹂??
	//character apply networkstate -> 留덉?留?servertick 媛깆떊

	//snapshot ?덉뿉 ?덈뒗 packet??낆씠??session ?뺣낫瑜?character濡??섍린吏 ?딅뒗??
	//character媛 諛쏅뒗 媛믪쓣 ?쒖닔 ?쒗쁽 媛믪씠??

	using namespace LostArk::Shared;

	if (0 == snapshot.iServerTick)
		return false;

	// ?꾩옱 TCP 寃쎈줈???쒖꽌媛 蹂댁옣?쒕떎. 以묐났 ?먮뒗 ??쟾 Tick? ?ㅼ떆 ?곸슜?섏? ?딅뒗??
	if (!CActionPresentationTimeline::Is_ForwardTick(
		snapshot.iServerTick, m_iLastServerTick))
		return true;
	if (!m_PlayerHealth.Apply_Snapshot(snapshot))
		return false;

	bool allSucceeded = true;

	// Admit the Kouku listener before remote action cues inspect its Mario audience.
	// Network array order must not leak a remote skill on the Mario-entry snapshot.
	const auto localFirst = m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA) ?
		CNetworkManager::Get().Get_LocalEntityId() : INVALID_NET_ENTITY_ID;
	for (const PLAYER_SNAPSHOT& player : snapshot.Players)
		if (player.iNetEntityId == localFirst)
			allSucceeded = Apply_PlayerSnapshot(player, snapshot.iServerTick, snapshot.Entities) && allSucceeded;
	for (const PLAYER_SNAPSHOT& player : snapshot.Players)
		if (player.iNetEntityId != localFirst)
			allSucceeded = Apply_PlayerSnapshot(player, snapshot.iServerTick, snapshot.Entities) && allSucceeded;
	if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA)) m_KoukuCardSnapshots = snapshot.Players;
	std::vector<NET_ENTITY_ID> deadBossOwners;
	for (const WORLD_ENTITY_SNAPSHOT& entity : snapshot.Entities)
	{
		const auto iter = m_WorldEntities.find(entity.iNetEntityId);
		if (iter == m_WorldEntities.end())
		{
			allSucceeded = false;
			continue;
		}
		if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA)) iter->second.KoukuSnapshot = entity;
		const bool_t expectsBossCombatState =
			WORLD_ENTITY_KIND::BOSS == iter->second.eKind;
		if (expectsBossCombatState != entity.hasBossCombatState)
		{
			allSucceeded = false;
			continue;
		}
		const float3_t position(
			entity.fPositionX,
			entity.fPositionY,
			entity.fPositionZ);
		if (WORLD_ENTITY_KIND::BOSS != iter->second.eKind)
		{
			iter->second.PinnedDefinitionRevision =
				entity.PinnedDefinitionRevision;
		}
		if (WORLD_ENTITY_KIND::NPC == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock();
			if (nullptr == npc ||
				!npc->Apply_NetworkState(
					position, entity.fYawDegrees, snapshot.iServerTick))
			{
				allSucceeded = false;
				continue;
			}
			/* Raid Esther actions retain their catalog-authored multi-clip chains.
			Town placement actions are resolved separately from the published
			placement presentation binding below. */
			const NPC_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Npc(iter->second.strArchetypeId);
			if (nullptr == actor)
			{
				allSucceeded = false;
				continue;
			}
			const auto actionClip =
				actor->actionClips.find(entity.strActionId);
			if (actionClip != actor->actionClips.end())
			{
				std::string soundStatus;
				/* Reliable spawn carries the action id but not its Server start tick.
				   The first snapshot supplies the authoritative occurrence clock;
				   subsequent snapshots advance/deduplicate the same cue cursor. */
				(void)CEstherActionSoundCueDocument::Play_Due(
					ESTHER_ACTION_SOUND_OWNER_KIND::NPC_ACTION,
					iter->second.strArchetypeId, entity.strActionId,
					snapshot.iServerTick, entity.iActionStartTick,
					iter->second.EstherActionSoundState, soundStatus);
				const std::vector<std::string>& chain = actionClip->second;
				if (iter->second.strActiveActionId != entity.strActionId)
				{
					if (!npc->Set_Animation(chain.front().c_str(), false))
					{
						allSucceeded = false;
						continue;
					}
					iter->second.strActiveActionId = entity.strActionId;
					iter->second.iActionClipIndex = 0u;
					iter->second.strCurrentClip = chain.front();
					CCombatHUDViewModel::Get().Apply_EstherCutinAction(
						iter->second.strArchetypeId);
				}
				else if (iter->second.iActionClipIndex + 1u < chain.size())
				{
					const std::shared_ptr<Engine::CModel> model =
						npc->Get_Model();
					f32_t position = 0.f;
					f32_t duration = 0.f;
					if (nullptr != model &&
						model->Get_AnimationProgress(
							model->Get_CurrentAnimIndex(),
							position, duration) &&
						duration > 0.f && position >= duration)
					{
						const std::string& next =
							chain[iter->second.iActionClipIndex + 1u];
						if (!npc->Set_Animation(next.c_str(), false))
						{
							allSucceeded = false;
						}
						else
						{
							++iter->second.iActionClipIndex;
							iter->second.strCurrentClip = next;
						}
					}
				}
				continue;
			}

			/* Placement-authored behavior uses one validated semantic-action
			binding. It is deliberately separate from the Esther chain above:
			flattening an Esther chain here would lose its cutin and root motion. */
			iter->second.strActiveActionId.clear();
			iter->second.iActionClipIndex = 0u;
			if (!CNpcActionPresentationRuntime::Is_NewActionEdge(
					iter->second.NpcActionEdge,
					entity.strActionId,
					entity.iActionStartTick))
			{
				continue;
			}
			const NPC_ACTION_PLAYBACK_REQUEST request =
				CNpcActionPresentationRuntime::Resolve_Playback(
					entity.strActionId,
					iter->second.strResolvedIdleClip,
					iter->second.NpcPresentation,
					nullptr);
			CNpcActionPresentationRuntime::Apply_Playback(
				request,
				iter->second.strResolvedIdleClip,
				[&npc](const NPC_ACTION_PLAYBACK_REQUEST& playback)
				{
					return npc->Play_NetworkAction(
						playback.strClipName.c_str(),
						playback.isLoop,
						playback.fPlaybackRate,
						playback.fBlendSeconds);
				},
				[&npc](const f32_t blendSeconds)
				{
					return npc->Play_DefaultIdle(blendSeconds);
				},
				iter->second.strCurrentClip);
			CNpcActionPresentationRuntime::Commit_ActionEdge(
				iter->second.NpcActionEdge,
				entity.strActionId,
				entity.iActionStartTick);
		}
		else if (WORLD_ENTITY_KIND::MONSTER == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> monster = iter->second.pNpc.lock();
			const MONSTER_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Monster(iter->second.strArchetypeId);
			if (nullptr == monster || nullptr == actor ||
				!monster->Apply_NetworkState(
					position, entity.fYawDegrees, snapshot.iServerTick))
			{
				allSucceeded = false;
				continue;
			}

			const MONSTER_PRESENTATION_ACTION_FRAME frame =
				CMonsterPresentationContract::Project(
					entity.eAction,
					entity.iActionStartTick,
					iter->second.MonsterActionState);
			if (!frame.shouldRestartClip)
				continue;

			const std::string* clip = &actor->presentationClips.idle;
			f32_t playbackRate = 1.f;
			const std::string* endEffect = nullptr;
			switch (frame.eKind)
			{
			case MONSTER_PRESENTATION_ACTION_KIND::CHASE:
				clip = &actor->presentationClips.chase;
				break;
			case MONSTER_PRESENTATION_ACTION_KIND::ATTACK:
				if (!actor->attackPresentations.empty())
				{
					const std::size_t attackIndex =
						CMonsterPresentationContract::Select_AttackPresentation(
							entity.iNetEntityId,
							frame.iOccurrenceStartTick,
							actor->attackPresentations.size());
					clip = &actor->attackPresentations[attackIndex].clip;
					endEffect = &actor->attackPresentations[attackIndex].endEffectAssetId;
					playbackRate =
						actor->attackPresentations[attackIndex].playbackRate;
				}
				break;
			case MONSTER_PRESENTATION_ACTION_KIND::DEAD:
				clip = &actor->presentationClips.dead;
				break;
			default:
				break;
			}
			if (monster->Play_NetworkAction(
					clip->c_str(), frame.isLoop, playbackRate, 0.12f))
			{
				iter->second.strCurrentClip = *clip;
				if (endEffect && !endEffect->empty())
					(void)monster->Schedule_ClipEndEffect(*endEffect, m_Desc.iLayerLevelIndex,
						"monster:" + std::to_string(entity.iNetEntityId) + ":" +
						std::to_string(frame.iOccurrenceStartTick) + ":clip-end");
			}
			else
			{
				/* A missing optional action clip isolates only this presentation.
				The Server entity and the rest of the snapshot remain valid. */
				monster->Play_DefaultIdle(0.12f);
				iter->second.strCurrentClip = actor->presentationClips.idle;
			}
		}
		else if (WORLD_ENTITY_KIND::BOSS == iter->second.eKind)
		{
			/* Raw, un-gated read of this tick's own eAction -- independent of
			whether Apply_NetworkState/Apply_BossCombatState/Apply_BrokenArmorMask
			below succeed. Apply_BossCombatState rejects a snapshot whose
			iStateRevision matches the last-applied one but whose content differs
			(Is_SameBossCombatState mismatch); if the Server's own BossCombat
			sub-state does not bump iStateRevision on the kill tick, that check can
			reject every post-death snapshot forever, leaving CCombatHUDViewModel's
			own Apply_Boss()-gated eAction stuck non-DEAD for the rest of the
			encounter (RaidClear/Return button never trigger, no matter how long
			you wait) even though the boss really is dead. RaidClear only needs a
			reliable death edge, not the rest of the gated boss state, so it reads
			this flag instead of Get_Boss().eAction. */
			if (INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId &&
				WORLD_ENTITY_ACTION::DEAD == entity.eAction)
			{
				CCombatHUDViewModel::Get().Set_BossDeadRaw(true);
			}

			if (Is_KoukuSaydonArenaBoss(
					iter->second.strArchetypeId,
					iter->second.strEncounterId,
					iter->second.iOwnerBossNetEntityId))
			{
				const std::shared_ptr<CNpc> boss = iter->second.pNpc.lock();
				const bool_t newPatternEdge = !entity.strPatternId.empty() &&
					entity.iPatternSequence != iter->second.iPatternSequence;
				if (nullptr == boss ||
					iter->second.PinnedDefinitionRevision !=
						entity.PinnedDefinitionRevision ||
					!boss->Apply_NetworkState(
						position, entity.fYawDegrees, snapshot.iServerTick, newPatternEdge))
				{
					allSucceeded = false;
					continue;
				}

                // A duration Logic may give the original body a child action while
                // the ordinary pattern fields retain the parent's gameplay clock.
                const bool childPresentation = !entity.strPresentationPatternId.empty();
                const auto& presentationActionId = childPresentation ?
                    entity.strPresentationActionId : entity.strActionId;
                const auto presentationActionStartTick = childPresentation ?
                    entity.iPresentationActionStartTick : entity.iActionStartTick;
                const auto presentationPatternStartTick = childPresentation ?
                    entity.iPresentationPatternStartTick : entity.iPatternStartTick;
                const auto presentationStageIndex = childPresentation ?
                    entity.iPresentationPatternStageIndex : entity.iPatternStageIndex;
				const bool_t newActionEdge =
					iter->second.strActiveActionId != presentationActionId ||
					iter->second.iKoukuActionStartTick != presentationActionStartTick ||
					iter->second.iPatternSequence != entity.iPatternSequence ||
					iter->second.iPatternStageIndex !=
						presentationStageIndex;
				if (newActionEdge)
				{
					KOUKU_SAYDON_ACTION_PRESENTATION action;
					const bool_t hasAction =
						CKoukuSaydonPresentationAssetService::Try_Resolve_Action(
							iter->second.strArchetypeId, presentationActionId, action,
                            m_KoukuBundleState.iRunEpoch ? m_KoukuBundleState.iPinnedSourceRevision : 0u);
					const bool_t played = hasAction ?
						boss->Play_NetworkAction(
							action.strClip.c_str(), action.bLoopToWindow,
							action.fPlayRate, action.bUnblendedBoneContact ? 0.f : 0.05f, action.fAnimationRootVerticalScale) :
						boss->Play_DefaultIdle(0.08f);
					const bool_t missingProductAction = !hasAction &&
						!entity.strPatternId.empty() && !presentationActionId.empty();
					if (!played || missingProductAction)
					{
						// Idle keeps the body usable, but cannot acknowledge a missing
						// Product animation as successfully presented.
						m_strPendingPresentationFailure = hasAction ?
							"KoukuSaydon Product animation clip could not start: " +
								action.strClip :
							"KoukuSaydon Server action has no admitted Product animation binding: " +
								presentationActionId;
						OutputDebugStringA(("[KoukuSaydonAnimation] " +
							m_strPendingPresentationFailure + "\n").c_str());
						allSucceeded = false;
					}
					if (played && hasAction)
					{
						// Late join and delayed snapshots enter at the Server action age.
						// All bundle members therefore share the scheduled tick origin.
						float ageSeconds = 0.f;
						const auto model = boss->Get_Model();
						if (model && CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
							snapshot.iServerTick, presentationActionStartTick, 30.f, ageSeconds))
						{
                            const float startOffsetSeconds = action.iStartOffsetMs * .001f;
                            const float animationAge = (std::max)(0.f, ageSeconds - startOffsetSeconds);
                            if (!boss->Set_NetworkAnimationWindow(ageSeconds,
                                action.bHoldAtWindowEnd ? action.iPlayMs * .001f : 0.f, startOffsetSeconds,
                                action.iSourceStartMs, action.iSourceEndMs))
                            {
                                allSucceeded = false;
                                m_strPendingPresentationFailure = "KoukuSaydon animation window could not be sampled: " + action.strActionId;
                            }
                            else
                            {
                                if (action.iBlendInMs)
                                {
                                    if (!boss->Apply_NetworkAnimationTransition(action.strBlendFromClip.c_str(),
                                        action.fBlendFromSourceMs, float(action.iBlendInMs), animationAge, action.fPlayRate))
                                    {
                                        allSucceeded = false;
                                        m_strPendingPresentationFailure = "KoukuSaydon animation transition could not be sampled: " + action.strActionId;
                                    }
                                }
                                if (!action.AnimationBlendWindows.empty())
                                {
                                    float patternAgeSeconds = 0.f;
                                    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(snapshot.iServerTick,
                                        presentationPatternStartTick, 30.f, patternAgeSeconds) ||
                                        !boss->Set_NetworkAnimationBlendWindows(action.AnimationBlendWindows, action.strOccurrenceId, patternAgeSeconds))
                                    {
                                        allSucceeded = false;
                                        m_strPendingPresentationFailure = "KoukuSaydon Logic animation blend could not be sampled: " + action.strActionId;
                                    }
                                }
                            }
						}
					}
					if (played)
					{
						iter->second.strCurrentClip = hasAction ?
							action.strClip : iter->second.strResolvedIdleClip;
					}
					iter->second.strActiveActionId = presentationActionId;
					iter->second.iKoukuActionStartTick = presentationActionStartTick;
					iter->second.iPatternSequence = entity.iPatternSequence;
					iter->second.iPatternStageIndex =
						presentationStageIndex;
				}
				if (INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId)
					CCombatHUDViewModel::Get().Apply_Boss(
						snapshot.iServerTick, iter->second.strArchetypeId, entity);
				continue;
			}

			VALTAN_PRESENTATION_STATE latest{};
			latest.isValid = true;
			latest.iNetEntityId = entity.iNetEntityId;
			latest.iServerTick = snapshot.iServerTick;
			latest.eAction = entity.eAction;
			latest.strArchetypeId = iter->second.strArchetypeId;
			latest.strPatternId = entity.strPatternId;
			latest.strActionId = entity.strActionId;
			latest.iPatternSequence = entity.iPatternSequence;
			latest.iPatternStageIndex = entity.iPatternStageIndex;
			latest.iPatternTargetNetEntityId =
				entity.iPatternTargetNetEntityId;
			latest.PortalRushRoute = entity.PortalRushRoute;
			latest.iActionStartTick = entity.iActionStartTick;
			latest.BossCombat = entity.BossCombat;
			latest.vPosition = position;
			latest.fYawDegrees = entity.fYawDegrees;

			const std::shared_ptr<CValtan> valtan =
				iter->second.pValtan.lock();
			const bool_t bIsPrimary =
				INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId;
			if (nullptr == valtan)
			{
				allSucceeded = false;
				continue;
			}
			std::string RevisionStatus;
			if (!Ensure_ValtanPresentationRevision(
					iter->second, valtan,
					entity.PinnedDefinitionRevision,
					bIsPrimary, RevisionStatus))
			{
				/* Gameplay/HUD truth continues to advance while animation, Effect,
				   Sound and combat-object presentation stay isolated. */
				if (bIsPrimary)
				{
					if (WORLD_ENTITY_ACTION::DEAD == entity.eAction)
						deadBossOwners.push_back(entity.iNetEntityId);
					m_ValtanPresentationState = std::move(latest);
					CCombatHUDViewModel::Get().Apply_Boss(
						snapshot.iServerTick,
						iter->second.strArchetypeId,
						entity);
				}
				continue;
			}
			const CValtan::PATTERN_TARGET_SNAPSHOT_POSE PatternTargetPose =
				Resolve_ValtanPatternTargetSnapshotPose(
					std::span<const PLAYER_SNAPSHOT>(snapshot.Players.data(),
						snapshot.Players.size()),
					entity.iPatternTargetNetEntityId);
			if (!valtan->Apply_NetworkState(
				position,
				entity.fYawDegrees,
				entity.eAction,
				entity.strPatternId,
				entity.strActionId,
				snapshot.iServerTick,
				entity.iActionStartTick,
				entity.iPatternSequence,
				entity.iPatternStageIndex,
				PatternTargetPose,
				entity.PortalRushRoute) ||
				!valtan->Apply_BossCombatState(entity.BossCombat) ||
				!valtan->Apply_BrokenArmorMask(entity.iBrokenArmorMask))
			{
				allSucceeded = false;
			}
			else if (bIsPrimary)
			{
				if (WORLD_ENTITY_ACTION::DEAD == entity.eAction)
					deadBossOwners.push_back(entity.iNetEntityId);
				m_ValtanPresentationState = std::move(latest);
				CCombatHUDViewModel::Get().Apply_Boss(
					snapshot.iServerTick,
					iter->second.strArchetypeId,
					entity);
			}
		}
		else
		{
			allSucceeded = false;
		}
	}
	for (const BOSS_COMBAT_EVENT& event : snapshot.BossCombatEvents)
	{
		const auto boss = m_WorldEntities.find(event.iBossNetEntityId);
		if (boss == m_WorldEntities.end() ||
			WORLD_ENTITY_KIND::BOSS != boss->second.eKind)
		{
			allSucceeded = false;
			continue;
		}
		if (boss->second.bPresentationIsolated)
			continue;
		const std::shared_ptr<CValtan> valtan = boss->second.pValtan.lock();
		if (nullptr == valtan || !valtan->Apply_BossCombatEvent(event))
			allSucceeded = false;
	}
	{
		COMBAT_OBJECT_PRESENTATION_SINK sink{ *this, snapshot.iServerTick };
		std::string status;
		if (!m_CombatObjectProjectionRuntime.Apply_Snapshot(
			snapshot.iServerTick, snapshot.CombatObjects, sink, status))
		{
			m_strPendingPresentationFailure = std::move(status);
			allSucceeded = false;
		}
		else if (status.find("failed") != std::string::npos)
		{
			m_strPendingPresentationFailure = std::move(status);
		}
	}
	for (const NET_ENTITY_ID owner : deadBossOwners)
		Remove_DependentBossPresentations(owner);
	for (const DAMAGE_EVENT& damageEvent : snapshot.DamageEvents)
	{
		if (!damageEvent.isOutgoing || damageEvent.iAmount == 0u ||
			(damageEvent.eHitFlag != DAMAGE_HIT_FLAG::NORMAL &&
			 damageEvent.eHitFlag != DAMAGE_HIT_FLAG::CRITICAL &&
			 damageEvent.eHitFlag != DAMAGE_HIT_FLAG::ABSORB) ||
			damageEvent.eCardMazeSuit != MECHANIC_CARD_SYMBOL::NONE)
			continue;
		const auto hitEntity =
			m_WorldEntities.find(damageEvent.iTargetNetEntityId);
		if (hitEntity == m_WorldEntities.end())
		{
			// Owned WORLD bodies are not duplicated as ordinary world entities.
			// Level resolves this same-frame event after consuming its reliable PLAY/STOP cues.
			if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA))
				m_PendingWorldCombatHits.insert(damageEvent.iTargetNetEntityId);
			continue;
		}
		if (hitEntity->second.bPresentationIsolated ||
			!Is_PlayerDamageableWorldArchetype(hitEntity->second.strArchetypeId))
			continue;
		if (WORLD_ENTITY_KIND::MONSTER == hitEntity->second.eKind)
		{
			if (const std::shared_ptr<CNpc> monster =
				hitEntity->second.pNpc.lock())
			{
				monster->Trigger_HitFlash();
				const MONSTER_ACTOR_ENTRY* actor = CActorCatalog::Find_Monster(
					hitEntity->second.strArchetypeId);
				const MONSTER_PRESENTATION_ACTION_KIND actionKind =
					hitEntity->second.MonsterActionState.eLastKind;
				if (nullptr != actor &&
					(MONSTER_PRESENTATION_ACTION_KIND::IDLE == actionKind ||
					 MONSTER_PRESENTATION_ACTION_KIND::CHASE == actionKind))
				{
					const std::string& returnClip =
						MONSTER_PRESENTATION_ACTION_KIND::CHASE == actionKind ?
						actor->presentationClips.chase :
						actor->presentationClips.idle;
					(void)monster->Play_TransientNetworkAction(
						actor->presentationClips.hit.c_str(),
						1.f,
						actor->hitDurationSeconds,
						returnClip.c_str(),
						true);
				}
			}
		}
		else if (WORLD_ENTITY_KIND::BOSS == hitEntity->second.eKind)
		{
			if (const std::shared_ptr<CNpc> boss = hitEntity->second.pNpc.lock())
				boss->Trigger_HitFlash();
			else if (const std::shared_ptr<CValtan> boss =
				hitEntity->second.pValtan.lock())
			{
				boss->Trigger_HitFlash();
			}
		}
	}
	CCombatHUDViewModel::Get().Apply_DamageEvents(
		snapshot.iServerTick,
		snapshot.DamageEvents,
		CNetworkManager::Get().Get_LocalPlayerId());
	CCombatHUDViewModel::Get().Apply_EstherGauge(
		snapshot.iEstherGauge,
		snapshot.iEstherGaugeMaximum);
	CCombatHUDViewModel::Get().Apply_BingoBoard(snapshot.Bingo);

	// Replace the complete read model only after the accepted snapshot has
	// resolved its presentations. No UI owns a replicated object's lifetime.
	std::vector<HUD_WORLD_HEALTH_BAR_STATE> healthBars;
	healthBars.reserve(snapshot.Players.size() + snapshot.Entities.size());
	const auto localEntityId = CNetworkManager::Get().Get_LocalEntityId();
	for (const auto& player : snapshot.Players)
	{
		if (player.iNetEntityId == localEntityId || !player.iCurrentHp || !player.iMaximumHp)
			continue;
		OBJECT_HANDLE handle{};
		if (!m_Registry.Find_Handle(player.iNetEntityId, handle)) continue;
		const auto character = m_Registry.Resolve(handle);
		if (!character) continue;
		HUD_WORLD_HEALTH_BAR_STATE state;
		state.iNetEntityId = player.iNetEntityId;
		state.isPlayer = true;
		state.iCurrentHp = player.iCurrentHp;
		state.iMaximumHp = player.iMaximumHp;
		state.iShield = player.iShield;
		state.pPresentation = character;
		healthBars.push_back(std::move(state));
	}
	for (const auto& entity : snapshot.Entities)
	{
		if (!entity.iCurrentHp || !entity.iMaximumHp) continue;
		const auto found = m_WorldEntities.find(entity.iNetEntityId);
		if (found == m_WorldEntities.end() || found->second.bPresentationIsolated) continue;
		const auto& presentation = found->second;
		if (presentation.eKind != WORLD_ENTITY_KIND::MONSTER && presentation.eKind != WORLD_ENTITY_KIND::BOSS)
			continue;
		HUD_WORLD_HEALTH_BAR_STATE state;
		state.iNetEntityId = entity.iNetEntityId;
		state.strArchetypeId = presentation.strArchetypeId;
		state.iCurrentHp = entity.iCurrentHp;
		state.iMaximumHp = entity.iMaximumHp;
		state.iShield = entity.hasBossCombatState ? entity.BossCombat.iCurrentShield : 0u;
		if (const auto npc = presentation.pNpc.lock()) state.pPresentation = npc;
		else if (const auto boss = presentation.pValtan.lock()) state.pPresentation = boss;
		if (!state.pPresentation.expired()) healthBars.push_back(std::move(state));
	}
	CCombatHUDViewModel::Get().Apply_WorldHealthBars(std::move(healthBars));

	m_iLastServerTick = snapshot.iServerTick;
	return allSucceeded;
}

Client::CClientReplication::CHARACTER_REPLACE_RESULT
Client::CClientReplication::Replace_CharacterClass(
	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Network.PlayerPresentation.Replace");
	const NET_PLAYER_RECORD* currentRecord =
		m_Registry.Find_Record(snapshot.iNetEntityId);
	if (nullptr == currentRecord)
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
	const NET_PLAYER_RECORD oldRecord = *currentRecord;
	OBJECT_HANDLE oldHandle{};
	if (!m_Registry.Find_Handle(snapshot.iNetEntityId, oldHandle))
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
	const std::shared_ptr<CCharacter> oldCharacter =
		m_Registry.Resolve(oldHandle);
	if (nullptr == oldCharacter)
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;

	const bool_t isLocallyControlled = snapshot.iNetEntityId ==
		CNetworkManager::Get().Get_LocalEntityId();
	std::shared_ptr<CCharacter> stagedCharacter;
	if (!Create_Character(
		snapshot.eCharacterClass,
		snapshot.eMadnessForm,
		oldRecord.strNickName,
		float3_t(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ),
		snapshot.fYawDegrees,
		isLocallyControlled,
		stagedCharacter))
	{
		m_strPendingPresentationFailure =
			"Class change asset admission failed; the previous character remains active.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	NET_PLAYER_RECORD newRecord = oldRecord;
	newRecord.eCharacterClass = snapshot.eCharacterClass;
	newRecord.eMadnessForm = snapshot.eMadnessForm;
	newRecord.fPositionX = snapshot.fPositionX;
	newRecord.fPositionY = snapshot.fPositionY;
	newRecord.fPositionZ = snapshot.fPositionZ;
	newRecord.fYawDegrees = snapshot.fYawDegrees;
	OBJECT_HANDLE newHandle{};
	if (!m_Registry.Replace(
		snapshot.iNetEntityId, newRecord, stagedCharacter, newHandle))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, stagedCharacter);
		m_strPendingPresentationFailure =
			"Class change registry commit failed; the previous character was kept.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
		m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, oldCharacter)))
	{
		OBJECT_HANDLE restoredHandle{};
		const bool removedStaged = SUCCEEDED(
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, stagedCharacter));
		const bool restored = m_Registry.Replace(
			snapshot.iNetEntityId, oldRecord, oldCharacter, restoredHandle);
		if (!removedStaged || !restored)
			return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
		if (isLocallyControlled)
			m_LocalCharacterHandle = restoredHandle;
		m_strPendingPresentationFailure =
			"Class change layer commit failed; the previous character was restored.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	if (isLocallyControlled)
	{
		m_LocalCharacterHandle = newHandle;
		CAnimationTargetService::Bind(stagedCharacter);
	}
	return CHARACTER_REPLACE_RESULT::REPLACED;
}

void Client::CClientReplication::Stage_LocalCharacterClassReplacement(
	const LostArk::Shared::PLAYER_SNAPSHOT& Snapshot,
	const std::uint32_t iServerTick)
{
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT& Pending =
		m_DeferredLocalCharacterClassReplacement;
	if (!Pending.isPending ||
		Pending.Snapshot.iNetEntityId != Snapshot.iNetEntityId ||
		Pending.Snapshot.eCharacterClass != Snapshot.eCharacterClass ||
		Pending.Snapshot.eMadnessForm != Snapshot.eMadnessForm)
	{
		Pending.iGeneration =
			m_iNextDeferredLocalCharacterClassReplacementGeneration++;
		if (0u == m_iNextDeferredLocalCharacterClassReplacementGeneration)
			m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
	}
	Pending.isPending = true;
	Pending.iServerTick = iServerTick;
	Pending.Snapshot = Snapshot;
}

void Client::CClientReplication::Clear_DeferredLocalCharacterClassReplacement()
{
	m_DeferredLocalCharacterClassReplacement = {};
}

void Client::CClientReplication::Update_DeathPresentations()
{
	for (auto iter = m_DeathPresentations.begin(); iter != m_DeathPresentations.end();)
	{
		const std::shared_ptr<CValtan> valtan = iter->lock();
		if (nullptr != valtan && !valtan->Is_NetworkDeathPresentationComplete())
		{
			++iter;
			continue;
		}
		if (nullptr != valtan)
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag, valtan);
		}
		iter = m_DeathPresentations.erase(iter);
	}
}

void Client::CClientReplication::Reset_World()
{
	m_WorldCombatTargets.clear();
	m_PendingWorldCombatHits.clear();
	if (const auto character = Get_LocalCharacter())
		CAnimationTargetService::Unbind(character);
	if (m_pPlayerAssetPreparation) m_pPlayerAssetPreparation->Cancel_AsyncPreparation();
	m_PreparingPlayerClass.reset();
	m_PendingPlayerSpawns.clear();
	m_PendingPlayerPresentations.clear();
	m_FailedPlayerSpawnClasses.clear();
	m_FailedPlayerAssetClasses.clear();
	m_PendingWorldSequencePlays.clear();
	m_KoukuBundleState = {};
	m_KoukuRaidState = {}; Expect_KoukuRaidReply(0u);
	//?묒냽???딄꼈?????꾩옱 registry???댁븘?덈뒗 character瑜?紐⑤몢 layer?먯꽌 ?쒓굅?섍퀬,
	//registry? local handle??珥덇린?뷀븳??
	//?뚭눼?먯뿉???몄텧?섏? ?딅뒗 ?댁쑀??留욌떎. ?꾩옱 engine? ?덈꺼 ?꾪솚 ??layer瑜?
	//癒쇱? ?쒓굅?섎?濡? level ?뚭눼?먯뿉???ㅼ떆 layer ?쒓굅瑜??쒕룄?섎㈃, ?대?
	//?щ씪吏?layer瑜?嫄대뱶由????덈떎.
	COMBAT_OBJECT_PRESENTATION_SINK combatObjectSink{ *this };
	m_CombatObjectProjectionRuntime.Reset(combatObjectSink);
    m_pTargetedCombatPresentationPlayer = nullptr;
	const std::vector<std::shared_ptr<CCharacter>> characters =
		m_Registry.Get_LiveObjects();

	for (const auto& character : characters)
	{
		if (nullptr == character)
			continue;

		//Layer?먯꽌 GameObject ?쒓굅?섍린
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character);
	}
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (const std::shared_ptr<CValtan> valtan = presentation.pValtan.lock())
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			if (!presentation.bUsesValtanGhostPool)
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					m_Desc.iLayerLevelIndex,
					m_Desc.strWorldEntityLayerTag,
					valtan);
			}
		}
		if (const std::shared_ptr<CNpc> npc = presentation.pNpc.lock())
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				npc);
		}
	}
	m_WorldEntities.clear();
	Clear_ValtanGhostPresentationPool();
	m_PrimaryValtanJoinedPresentationFreshness.Reject(
		"Replicated world reset; the next primary Valtan spawn must reload authoring sources for its exact revision.");
	m_PrimaryValtanCombatObjectSoundFreshness.Reject(
		"Replicated world reset; the next primary Valtan spawn must reload authoring sources for its exact revision.");
	for (const std::weak_ptr<CValtan>& deathPresentation : m_DeathPresentations)
	{
		if (const std::shared_ptr<CValtan> valtan = deathPresentation.lock())
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag, valtan);
		}
	}
	m_DeathPresentations.clear();
	m_Registry.Reset();
	m_LocalCharacterHandle = {};
	Clear_DeferredLocalCharacterClassReplacement();
	m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
	m_iLastServerTick = 0;
	m_KoukuCardSnapshots.clear();
#ifdef _DEBUG
	m_isCombatObjectHitAreaDebugLoadAttempted = false;
	m_CombatObjectHitAreasByArchetype.clear();
#endif
	m_ValtanPresentationState = {};
	m_WorldDestructionProjectionRuntime.Reset();
	m_WorldDestructionLiveEvents.clear();
	m_EncounterPropState = {};
	m_InventoryState = {};
	m_PlayerHealth.Reset();
	m_PartyRoster = {};
	m_hasPendingPartyInvite = false;
	m_PendingPartyInvite = {};
	m_hasPendingPartyTransferResult = false;
	m_PendingPartyTransferResult = {};
	m_hasPendingRaidEntryPrompt = false;
	m_PendingRaidEntryPrompt = {};
	m_hasPendingRaidEntryVote = false;
	m_PendingRaidEntryVote = {};
	m_ChatBubblesByNetEntityId.clear();
	m_GuideState.reset();
	m_GuidePromptSequences.clear();
	m_PendingGuideBubbles.clear();
	m_HonorTitleByNetEntityId.clear();
	++m_iWorldDestructionPresentationGeneration;
	if (0u == m_iWorldDestructionPresentationGeneration)
		++m_iWorldDestructionPresentationGeneration;
	m_hasFatalWorldDestructionFailure = false;
	m_strPendingPresentationFailure.clear();
	CCombatHUDViewModel::Get().Reset_RuntimeState();
}

bool Client::CClientReplication::Apply_PlayerSnapshot(
	const LostArk::Shared::PLAYER_SNAPSHOT& player, const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities)
{
	using namespace LostArk::Shared;
	bool allSucceeded = true;
	/* Cosmetic, so it is kept even while the body is still being staged. */
	m_HonorTitleByNetEntityId[player.iNetEntityId] = player.iHonorTitleId;
	if (m_PendingPlayerSpawns.contains(player.iNetEntityId))
	{
		Stage_PlayerPresentation(player, serverTick, entities);
		return true;
	}
	const NET_PLAYER_RECORD* record =
		m_Registry.Find_Record(player.iNetEntityId);
	if (record && record->eControlKind != player.eControlKind) return false;
	const bool_t isLocallyControlled = player.iNetEntityId ==
		CNetworkManager::Get().Get_LocalEntityId();
	if (nullptr != record &&
		(record->eCharacterClass != player.eCharacterClass ||
		 record->eMadnessForm != player.eMadnessForm))
	{
		OBJECT_HANDLE previousHandle{};
		if (m_Registry.Find_Handle(player.iNetEntityId, previousHandle))
			if (const auto previousCharacter = m_Registry.Resolve(previousHandle))
				previousCharacter->Cancel_PendingInteractionEffectAdmission();
		if (isLocallyControlled &&
			m_Desc.bDeferLocalCharacterClassReplacement)
		{
			Stage_LocalCharacterClassReplacement(
				player, serverTick);
			return true;
		}
		if (player.eMadnessForm == PLAYER_MADNESS_FORM::NORMAL &&
			!CPlayableCharacterAssetService::Is_Ready(m_Desc.iPrototypeLevelIndex, player.eCharacterClass))
		{
			Stage_PlayerPresentation(player, serverTick, entities);
			return true;
		}
		const CHARACTER_REPLACE_RESULT replaceResult =
			Replace_CharacterClass(player);
		if (CHARACTER_REPLACE_RESULT::FATAL_FAILURE == replaceResult)
		{
			return false;
		}
		if (CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE == replaceResult)
			return true;
	}
	else if (isLocallyControlled &&
		m_DeferredLocalCharacterClassReplacement.isPending)
	{
		/* A newer authoritative snapshot returned to the currently committed
		   class before presentation commit.  Drop the superseded generation. */
		Clear_DeferredLocalCharacterClassReplacement();
	}
	OBJECT_HANDLE handle{};

	if (!m_Registry.Find_Handle(
		player.iNetEntityId,
		handle))
	{
		return false;
	}

	const std::shared_ptr<CCharacter> character =
		m_Registry.Resolve(handle);

	if (nullptr == character)
	{
		return false;
	}

	const float3_t position(
		player.fPositionX,
		player.fPositionY,
		player.fPositionZ);

	const bool_t isMoving =
		player.eLocomotionState ==
		PLAYER_LOCOMOTION_STATE::MOVING;

	character->Apply_NetworkVehicle(player.iVehicleId);
	character->Apply_NetworkVehicleFlight(player.eVehicleFlightPhase, serverTick,
		player.iVehicleFlightPhaseStartTick, player.fVehicleFlightPhaseDurationSeconds);
	character->Apply_LocalMoveSnapshot(LocalMoveSnapshot(player, serverTick),
		player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::SKILL ||
		player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::VEHICLE_SKILL);
	if (!character->Apply_NetworkState(
		position,
		player.fYawDegrees,
		isMoving,
		serverTick) ||
		!character->Apply_MarioPresentation(
			player.iMarioStage >= 1u && player.iMarioStage <= 4u) ||
		!character->Apply_NetworkAction(
			player.eAction,
			player.iSkillId,
			serverTick,
			player.iActionStartTick,
			player.fYawDegrees,
			player.iComboStage,
			player.hasSkillTarget,
			float3_t(
				player.fSkillTargetX,
				player.fSkillTargetY,
				player.fSkillTargetZ), player.eKoukuHudMode, player.eAttachmentSlot, player.isKnockbackAirborne))
	{
		allSucceeded = false;
	}
	character->Apply_NetworkStance(player.eStance);
	character->Apply_NetworkVehicle(player.iVehicleId);
	character->Apply_NetworkPresentationHidden((player.CardMaze.flags & LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN) != 0u);
	if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
	{
		// Both boss presentations expose the existing weak hand-socket interface.
		// Resolve Kouku's grip from the owner in this same Server snapshot, not
		// the previous action cached before this packet's entity loop.
		std::shared_ptr<const IPlayerHandGripSocketSource> owner;
		const auto ownerIter = m_WorldEntities.find(player.iAttachmentOwnerNetEntityId);
		if (m_WorldEntities.end() != ownerIter &&
			WORLD_ENTITY_KIND::BOSS == ownerIter->second.eKind &&
			!ownerIter->second.bPresentationIsolated)
		{
			owner = ownerIter->second.pValtan.lock();
                if (!owner && Is_KoukuSaydonArenaBoss(ownerIter->second.strArchetypeId,
                    ownerIter->second.strEncounterId, ownerIter->second.iOwnerBossNetEntityId))
                {
                    const auto npc = ownerIter->second.pNpc.lock();
                    const auto state = std::find_if(entities.begin(), entities.end(),
                        [&](const auto& value) { return value.iNetEntityId == player.iAttachmentOwnerNetEntityId; });
                    if (npc)
                    {
                        npc->Clear_PlayerHandGripLocalOffset();
                        if (state != entities.end() && state->iCurrentHp &&
                            state->eAction != WORLD_ENTITY_ACTION::DEAD &&
                            state->PinnedDefinitionRevision == ownerIter->second.PinnedDefinitionRevision)
                        {
                            PLAYER_HAND_GRIP_LOCAL_OFFSET grip{};
                            if (CKoukuSaydonPresentationAssetService::Try_Resolve_AttachmentGrip(
                                ownerIter->second.strArchetypeId, state->strPatternId, player.eAttachmentSlot, grip,
                                m_KoukuBundleState.iRunEpoch ? m_KoukuBundleState.iPinnedSourceRevision : 0u))
                                (void)npc->Set_PlayerHandGripLocalOffset(grip);
                            owner = npc;
                        }
                    }
                }
		}
		/* A world-object slot has no boss socket to follow: the Server
		position is the presentation, and asking for a grip would report a
		failure that is not one. */
		if (nullptr == owner ||
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != player.eAttachmentSlot)
			character->Clear_NetworkAttachment();
		else if (!character->Apply_NetworkAttachment(
				owner, player.eAttachmentSlot) &&
			m_strPendingPresentationFailure.empty())
		{
			m_strPendingPresentationFailure =
				"Grabbed player kept its Server fallback transform because the owner presentation has no admitted CAPTURE gripLocalOffset.";
		}
	}
	else
		character->Clear_NetworkAttachment();
	if (isLocallyControlled)
	{
		const NET_PLAYER_RECORD* localRecord =
			m_Registry.Find_Record(player.iNetEntityId);
		if (nullptr == localRecord)
		{
			allSucceeded = false;
		}
		else
		{
			CCombatHUDViewModel::Get().Apply_LocalPlayer(
				serverTick,
				localRecord->eCharacterClass,
				player);
		}
	}
	m_PendingPlayerPresentations.erase(player.iNetEntityId);
	return allSucceeded;
}

Client::CClientReplication::CClientReplication() = default;
Client::CClientReplication::~CClientReplication()
{
	// The Level may already have removed its layers; release only our scene binding.
	if (const auto character = Get_LocalCharacter())
		CAnimationTargetService::Unbind(character);
}

void Client::CClientReplication::Stage_PlayerPresentation(
	const LostArk::Shared::PLAYER_SNAPSHOT& player, const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::WORLD_ENTITY_SNAPSHOT>& entities)
{
	auto& pending = m_PendingPlayerPresentations[player.iNetEntityId];
	pending.Snapshot = player;
	pending.ServerTick = serverTick;
	pending.AttachmentOwners.clear();
	const auto owner = std::find_if(entities.begin(), entities.end(), [&](const auto& entity)
		{ return entity.iNetEntityId == player.iAttachmentOwnerNetEntityId; });
	if (owner != entities.end()) pending.AttachmentOwners.push_back(*owner);
}

bool Client::CClientReplication::Advance_PlayerAssetPreparation()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Network.PlayerAssets.Advance");
	using namespace LostArk::Shared;
	if (CLevelTransitionService::Is_Pending())
	{
		if (m_pPlayerAssetPreparation)
		{
			m_pPlayerAssetPreparation->Cancel_AsyncPreparation();
			HRESULT result; std::string status;
			(void)m_pPlayerAssetPreparation->Poll_AsyncPreparation(false, result, status);
		}
		return true;
	}
	const auto spawnClass = [&](const auto& spawn)
	{
		const auto latest = m_PendingPlayerPresentations.find(spawn.iNetEntityId);
		return latest == m_PendingPlayerPresentations.end() ? spawn.eCharacterClass : latest->second.Snapshot.eCharacterClass;
	};
	const auto wanted = [&](const CHARACTER_CLASS_ID characterClass)
	{
		for (const auto& [id, pending] : m_PendingPlayerPresentations)
			if (pending.Snapshot.eCharacterClass == characterClass) return true;
		for (const auto& [id, spawn] : m_PendingPlayerSpawns)
			if (spawnClass(spawn) == characterClass) return true;
		return false;
	};
	if (m_pPlayerAssetPreparation && m_pPlayerAssetPreparation->Is_Preparing())
	{
		const bool keep = m_PreparingPlayerClass && wanted(*m_PreparingPlayerClass);
		if (!keep) m_pPlayerAssetPreparation->Cancel_AsyncPreparation();
		HRESULT result = E_PENDING; std::string status;
		if (!m_pPlayerAssetPreparation->Poll_AsyncPreparation(keep, result, status)) return true;
		if (keep && FAILED(result) && result != HRESULT_FROM_WIN32(ERROR_CANCELLED))
		{
			m_FailedPlayerAssetClasses.insert(static_cast<uint8_t>(*m_PreparingPlayerClass));
			m_strPendingPresentationFailure = status + " Existing player presentations were kept.";
		}
		m_PreparingPlayerClass.reset();
	}
	bool succeeded = true;
	for (auto it = m_PendingPlayerSpawns.begin(); it != m_PendingPlayerSpawns.end();)
	{
		const auto characterClass = spawnClass(it->second);
		const auto failed = m_FailedPlayerSpawnClasses.find(it->first);
		if ((failed != m_FailedPlayerSpawnClasses.end() && failed->second == characterClass) ||
			!CPlayableCharacterAssetService::Is_Ready(m_Desc.iPrototypeLevelIndex, characterClass)) { ++it; continue; }
		auto spawn = it->second;
		spawn.eCharacterClass = characterClass;
		const auto latest = m_PendingPlayerPresentations.find(spawn.iNetEntityId);
		if (latest != m_PendingPlayerPresentations.end())
		{
			spawn.fPositionX = latest->second.Snapshot.fPositionX;
			spawn.fPositionY = latest->second.Snapshot.fPositionY;
			spawn.fPositionZ = latest->second.Snapshot.fPositionZ;
			spawn.fYawDegrees = latest->second.Snapshot.fYawDegrees;
		}
		if (!Commit_PlayerSpawn(spawn))
		{
			m_FailedPlayerSpawnClasses[it->first] = characterClass;
			m_strPendingPresentationFailure = "Player presentation commit failed; its latest Server state remains staged.";
			succeeded = false;
			++it;
			continue;
		}
		m_FailedPlayerSpawnClasses.erase(it->first);
		it = m_PendingPlayerSpawns.erase(it);
	}
	for (auto it = m_PendingPlayerPresentations.begin(); it != m_PendingPlayerPresentations.end();)
	{
		if (m_PendingPlayerSpawns.contains(it->first) ||
			!CPlayableCharacterAssetService::Is_Ready(m_Desc.iPrototypeLevelIndex, it->second.Snapshot.eCharacterClass))
		{ ++it; continue; }
		auto pending = std::move(it->second);
		it = m_PendingPlayerPresentations.erase(it);
		succeeded = Apply_PlayerSnapshot(pending.Snapshot, pending.ServerTick, pending.AttachmentOwners) && succeeded;
	}
	std::optional<CHARACTER_CLASS_ID> requested;
	const auto consider = [&](CHARACTER_CLASS_ID characterClass)
	{
		if (!requested && !m_FailedPlayerAssetClasses.contains(static_cast<uint8_t>(characterClass)) &&
			!CPlayableCharacterAssetService::Is_Ready(m_Desc.iPrototypeLevelIndex, characterClass)) requested = characterClass;
	};
	const auto localId = CNetworkManager::Get().Get_LocalEntityId();
	if (const auto local = m_PendingPlayerPresentations.find(localId); local != m_PendingPlayerPresentations.end()) consider(local->second.Snapshot.eCharacterClass);
	if (const auto local = m_PendingPlayerSpawns.find(localId); local != m_PendingPlayerSpawns.end()) consider(spawnClass(local->second));
	for (const auto& [id, pending] : m_PendingPlayerPresentations) consider(pending.Snapshot.eCharacterClass);
	for (const auto& [id, spawn] : m_PendingPlayerSpawns) consider(spawnClass(spawn));
	if (requested)
	{
		if (!m_pPlayerAssetPreparation) m_pPlayerAssetPreparation = std::make_unique<CPlayableCharacterAssetService>();
		const HRESULT result = m_pPlayerAssetPreparation->Begin_AsyncPreparation(m_Desc.pDevice, m_Desc.pContext, m_Desc.iPrototypeLevelIndex, *requested);
		if (FAILED(result))
		{
			m_FailedPlayerAssetClasses.insert(static_cast<uint8_t>(*requested));
			m_strPendingPresentationFailure = "Player class asset preparation could not start; existing players were kept.";
		}
		else if (result == S_OK) m_PreparingPlayerClass = requested;
	}
	return succeeded;
}
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Public/Level_Development.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#ifdef _DEBUG
#include "DeployPropRuntime.h"
#endif
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class IPlayerCommandSink;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	// The current Development/Training or Maharaka owner exposes its existing camera.
	static CLevel_Development* Get_Active(LEVEL level)
	{
		return s_pActiveInstance && s_pActiveInstance->m_eLevel == level ?
			s_pActiveInstance : nullptr;
	}
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera.lock(); }
#ifdef _DEBUG
	// Borrow the existing Maharaka map; the Level remains its owner.
	CMapPlacementRuntime& Get_MapAuthoringRuntime() { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_MapAuthoringDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	void Set_MapAuthoringActive(bool_t active) { m_bMapAuthoringActive = active; }
	std::shared_ptr<CNpc> Find_MapAuthoringNpc(const std::string& placementId) const
	{ return m_Replication.Find_NpcPlacement(placementId); }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records)
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
#endif

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
	CMapPlacementRuntime m_MapRuntime;
#ifdef _DEBUG
	// Maharaka has no Deploy source pair. Runtime attach still needs a live owner.
	CDeployPropRuntime m_MapAuthoringDeploy;
	bool_t m_bMapAuthoringActive = false;
#endif
	// Maharaka only: the published source lights of the island, submitted every frame.
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	bool_t m_isMapEditorWorkspace = false;
	weak_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	static CLevel_Development* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel = LEVEL::DEVELOPMENT);
};

NS_END
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Private/MapTool_Area.cpp

```cpp
#include "imgui.h"
#include "MapTool_Internal.h"
#include "WorldSequenceToolPanel.h"
#include "Camera_Free.h"
#include "DataJson.h"
#include "ActorCatalog.h"
#include "GameInstance.h"
#include "MapEditorWorkspaceService.h"
#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_Development.h"
#include "Level_KakulSaydonArena.h"
#include "Level_ValtanArena.h"
#include "MapAssetPreview.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
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




bool_t Client::CMapTool::Ensure_AuthoringPrototypes()
{
	return Ensure_AuthoringPrototypes(m_Catalog);
}

bool_t Client::CMapTool::Admit_AuthoringPrototype(
	const MAP_ASSET_ENTRY& asset)
{
	/* This tool owns every map prototype it admits into the authoring level,
	   so its own fingerprint already answers the identity question. Probing
	   the engine instead deep clones an admitted model only to drop it, which
	   an Area the size of Bern would pay for a thousand times per switch. */
	const auto fingerprint = m_PrototypeModelPaths.find(asset.prototypeTag);
	if (fingerprint != m_PrototypeModelPaths.end())
	{
		if (fingerprint->second.lexically_normal() !=
			asset.resolvedModelPath.lexically_normal())
		{
			m_Status = "Prototype tag already belongs to another model: " +
				asset.id;
			return false;
		}
		return true;
	}

	const shared_ptr<CModel> existing =
		dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex,
				asset.prototypeTag));
	if (nullptr != existing)
	{
		m_Status = "Prototype tag already belongs to another model: " +
			asset.id;
		return false;
	}

	Engine::MODEL_ASSET_LOAD_DESC loadDesc;
	loadDesc.assetRoot = CRuntimeAssetRoot::Get();
	loadDesc.meshPath = asset.resolvedModelPath;
	loadDesc.materialOverrides = asset.materialOverrides;
	auto model = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::NONANIM,
		loadDesc,
		XMMatrixScaling(0.01f, 0.01f, 0.01f));
	if (nullptr == model ||
		FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			asset.prototypeTag,
			std::move(model))))
	{
		m_Status = "Map authoring model admission failed: " + asset.id;
		return false;
	}
	m_PrototypeModelPaths.emplace(
		asset.prototypeTag,
		asset.resolvedModelPath.lexically_normal());
	return true;
}

bool_t Client::CMapTool::Ensure_AuthoringPrototypes(
	const CMapAssetCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map authoring prototype admission is unavailable";
		return false;
	}

	for (const MAP_ASSET_ENTRY& asset : catalog.Get_Entries())
	{
		if (!Admit_AuthoringPrototype(asset))
			return false;
	}

	m_Status = "Map authoring admitted " +
		std::to_string(catalog.Get_Entries().size()) +
		" model prototypes";
	return true;
}

bool_t Client::CMapTool::Ensure_DeployAuthoringPrototypes(
	const CDeployPropCatalog& catalog)
{
	if (!catalog.Is_Ready() ||
		m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "DeployProp authoring prototype admission is unavailable";
		return false;
	}

	const matrix_t modelTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);
	const auto admitModel = [this, &modelTransform](
		const std::wstring& prototypeTag,
		const std::filesystem::path& modelPath,
		const std::filesystem::path& relativeModelPath,
		const MODEL modelKind,
		const std::string& assetId)
	{
		const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_iAuthoringLevelIndex, prototypeTag));
		if (nullptr != existing)
		{
			const auto fingerprint = m_PrototypeModelPaths.find(prototypeTag);
			return fingerprint != m_PrototypeModelPaths.end() &&
				fingerprint->second.lexically_normal() ==
					modelPath.lexically_normal();
		}

		MODEL_ASSET_LOAD_DESC loadDesc;
		if (!CActorCatalog::Build_ModelLoadDescription(
			relativeModelPath.generic_string(), loadDesc, m_Status))
			return false;

		auto model = CModel::Create(
			m_pDevice,
			m_pContext,
			modelKind,
			loadDesc,
			modelTransform);
		if (nullptr == model || FAILED(CGameInstance::Get().Add_Prototype(
			m_iAuthoringLevelIndex,
			prototypeTag,
			std::move(model))))
		{
			m_Status = "DeployProp model admission failed: " + assetId;
			return false;
		}
		m_PrototypeModelPaths.emplace(
			prototypeTag, modelPath.lexically_normal());
		return true;
	};

	for (const DEPLOY_PROP_ASSET_ENTRY& asset : catalog.Get_Assets())
	{
		const MODEL modelKind =
			DEPLOY_PROP_MODEL_KIND::ANIM == asset.kind ?
			MODEL::ANIM : MODEL::NONANIM;
		if (!admitModel(
			asset.intactPrototypeTag,
			asset.intactResolvedPath,
			asset.intactRelativePath,
			modelKind,
			asset.id))
		{
			return false;
		}
		if (DEPLOY_PROP_MODEL_KIND::STATIC == asset.kind &&
			!asset.fracturedPrototypeTag.empty() &&
			!admitModel(
				asset.fracturedPrototypeTag,
				asset.fracturedResolvedPath,
				asset.fracturedRelativePath,
				MODEL::NONANIM,
				asset.id))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CMapTool::Load_EditorAreaRegistry()
{
	const std::filesystem::path path =
		CProjectDataRoot::Resolve(L"Maps/MapCatalog.json");
	std::string text;
	std::string parseError;
	DATA_JSON_VALUE root;
	if (path.empty() || !ReadTextFile(path, text) ||
		!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_Status = "Map editor catalog parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* areas = root.Find("areas");
	if (nullptr == schema || !schema->Is_String() ||
		schema->Get_String() != "lostark.map-catalog" ||
		nullptr == version || !version->Is_Number() ||
		version->Get_Number() != 1.0 ||
		nullptr == areas || !areas->Is_Array())
	{
		m_Status = "Map editor catalog header is invalid";
		return false;
	}

	const std::array<std::pair<const char_t*, const char_t*>, 6> targets =
	{{
		{ "LV_LOBBY_CLASSSELECT_SL00", "Character Select" },
		{ "LV_BER_BERNCASTLE", "Bern" },
		{ "LV_LUT_HEARTRB_ED", "Valtan" },
		{ "LV_LUT_MIDNIGHTC_ED", "KoukuSaydon / MidnightC ED" },
		{ "LV_SHS_RCARENA_D", "Training Map" },
		{ "LV_OCN_EVENTIS_MHP", "Maharaka Paradise" },
	}};
	std::vector<EDITOR_AREA_DESCRIPTOR> staged;
	staged.reserve(targets.size());
	for (const auto& target : targets)
	{
		const DATA_JSON_VALUE* selected = nullptr;
		for (const DATA_JSON_VALUE& candidate : areas->Get_Array())
		{
			const DATA_JSON_VALUE* id = candidate.Find("id");
			if (candidate.Is_Object() && nullptr != id && id->Is_String() &&
				id->Get_String() == target.first)
			{
				if (nullptr != selected)
				{
					m_Status = "Duplicate MapCatalog area: " +
						std::string(target.first);
					return false;
				}
				selected = &candidate;
			}
		}
		if (nullptr == selected)
		{
			m_Status = "MapCatalog area is missing: " +
				std::string(target.first);
			return false;
		}

		EDITOR_AREA_DESCRIPTOR descriptor;
		descriptor.areaId = target.first;
		descriptor.label = target.second;
		std::string sourceCatalog;
		std::string sourcePlacements;
		if (!ReadRequiredString(*selected, "sourceCatalog", sourceCatalog) ||
			!ReadRequiredString(*selected, "sourcePlacements", sourcePlacements))
		{
			m_Status = "MapCatalog authoring source is missing: " +
				descriptor.areaId;
			return false;
		}
		descriptor.sourceCatalog = ResolveDataCatalogPath(sourceCatalog);
		descriptor.sourcePlacements =
			ResolveDataCatalogPath(sourcePlacements);
		const DATA_JSON_VALUE* sourceMaterials = selected->Find("sourceMaterials");
		const DATA_JSON_VALUE* runtimeMaterials = selected->Find("materials");
		if ((nullptr == sourceMaterials) != (nullptr == runtimeMaterials))
		{
			m_Status = "MapCatalog material source/runtime pair is incomplete: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceMaterials)
		{
			const std::string expectedSource = "Data/Maps/Authoring/" +
				descriptor.areaId + "/" + descriptor.areaId + ".mapmaterials.json";
			const std::string expectedRuntime = "Client/Bin/DataFiles/Map/" +
				descriptor.areaId + ".mapmaterials.json";
			if (!sourceMaterials->Is_String() || !runtimeMaterials->Is_String() ||
				sourceMaterials->Get_String() != expectedSource ||
				runtimeMaterials->Get_String() != expectedRuntime)
			{
				m_Status = "MapCatalog material paths are not canonical: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceMaterials = ResolveDataCatalogPath(expectedSource);
			if (descriptor.sourceMaterials.empty())
			{
				m_Status = "MapCatalog material path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceLights = selected->Find("sourceLights");
		if (nullptr != sourceLights)
		{
			if (!sourceLights->Is_String() ||
				sourceLights->Get_String().empty())
			{
				m_Status = "MapCatalog light authoring source is invalid: " +
					descriptor.areaId;
				return false;
			}
			descriptor.sourceLights = ResolveDataCatalogPath(
				sourceLights->Get_String());
			if (descriptor.sourceLights.empty())
			{
				m_Status = "MapCatalog light path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}
		const DATA_JSON_VALUE* sourceDeployCatalog =
			selected->Find("sourceDeployCatalog");
		const DATA_JSON_VALUE* sourceDeployPlacements =
			selected->Find("sourceDeployPlacements");
		if ((nullptr == sourceDeployCatalog) !=
			(nullptr == sourceDeployPlacements) ||
			(nullptr != sourceDeployCatalog &&
				(!sourceDeployCatalog->Is_String() ||
					sourceDeployCatalog->Get_String().empty() ||
					!sourceDeployPlacements->Is_String() ||
					sourceDeployPlacements->Get_String().empty())))
		{
			m_Status = "MapCatalog deploy authoring pair is invalid: " +
				descriptor.areaId;
			return false;
		}
		if (nullptr != sourceDeployCatalog)
		{
			descriptor.sourceDeployCatalog = ResolveDataCatalogPath(
				sourceDeployCatalog->Get_String());
			descriptor.sourceDeployPlacements = ResolveDataCatalogPath(
				sourceDeployPlacements->Get_String());
			if (descriptor.sourceDeployCatalog.empty() ||
				descriptor.sourceDeployPlacements.empty())
			{
				m_Status = "MapCatalog deploy path escapes Data root: " +
					descriptor.areaId;
				return false;
			}
		}

		/* Camera shots are an optional Client presentation layer. The document
		   is named per Area and is simply absent where none is authored. */
		descriptor.cameraShotDocument = CProjectDataRoot::Resolve(
			std::filesystem::path("Maps") / "Authoring" / descriptor.areaId /
			(descriptor.areaId + ".camerashots.json"));

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED")
		{
			std::string source;
			std::string paint;
			if (!ReadRequiredString(*selected, "navigationSource", source) ||
				!ReadRequiredString(*selected, "navigationPaint", paint))
			{
				m_Status = "MapCatalog navigation source is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.navigationSource = ResolveDataCatalogPath(source);
			descriptor.navigationPaint = ResolveDataCatalogPath(paint);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT;
			descriptor.allowNavigationBootstrap =
				descriptor.areaId == "LV_BER_BERNCASTLE";
		}
		if (descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			if (descriptor.sourceLights.empty())
			{
				m_Status = "Valtan source light presentation is missing";
				return false;
			}
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
			/* The editor area registry already declares the explicit areas
			   the Map Tool opens, so the destruction reference path is
			   declared the same way instead of adding a field to the shared
			   MapCatalog schema. Render_DestructionEncounterSource cross
			   checks the loaded encounterId against the boss placement of
			   this Area. */
			descriptor.encounterReference = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json");
			descriptor.worldEventsDocument = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanWorldEvents.json");
			descriptor.destructionSimulationDocument = CProjectDataRoot::Resolve(
				L"Maps/Authoring/LV_LUT_HEARTRB_ED/"
				L"LV_LUT_HEARTRB_ED.destructionsimulation.json");
			if (descriptor.encounterReference.empty() ||
				descriptor.worldEventsDocument.empty() ||
				descriptor.destructionSimulationDocument.empty())
			{
				m_Status = "Valtan encounter reference path is invalid";
				return false;
			}
		}

		if (descriptor.areaId == "LV_LOBBY_CLASSSELECT_SL00" ||
			descriptor.areaId == "LV_BER_BERNCASTLE" ||
			descriptor.areaId == "LV_LUT_HEARTRB_ED" ||
			descriptor.areaId == "LV_LUT_MIDNIGHTC_ED" ||
			descriptor.areaId == "LV_OCN_EVENTIS_MHP")
		{
			std::string gameplay;
			if (!ReadRequiredString(*selected, "gameplayDocument", gameplay))
			{
				m_Status = "MapCatalog gameplay document is missing: " +
					descriptor.areaId;
				return false;
			}
			descriptor.gameplayDocument = ResolveDataCatalogPath(gameplay);
			descriptor.gameplayPolicy =
				EDITOR_GAMEPLAY_POLICY::REQUIRED;
		}

		if (descriptor.sourceCatalog.empty() ||
			descriptor.sourcePlacements.empty() ||
			((!descriptor.sourceDeployCatalog.empty() ||
				!descriptor.sourceDeployPlacements.empty()) &&
				(descriptor.sourceDeployCatalog.empty() ||
					descriptor.sourceDeployPlacements.empty())) ||
			(EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy &&
				(descriptor.navigationSource.empty() ||
					descriptor.navigationPaint.empty())) ||
			(EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy &&
				descriptor.gameplayDocument.empty()))
		{
			m_Status = "MapCatalog path escapes Data root: " +
				descriptor.areaId;
			return false;
		}
		staged.push_back(std::move(descriptor));
	}

	m_EditorAreas = std::move(staged);
	return true;
}

const Client::CMapTool::EDITOR_AREA_DESCRIPTOR*
Client::CMapTool::Get_ActiveEditorArea() const
{
	return m_iActiveEditorArea < m_EditorAreas.size() ?
		&m_EditorAreas[m_iActiveEditorArea] : nullptr;
}

CWorldSequencePlayer::TARGET_SET Client::CMapTool::Runtime_AuthoringTargets() const
{
#ifdef _DEBUG
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (levelIndex == ETOUI(LEVEL::MAHARAKA))
	{
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			targets.previewNpc = [](const std::string& placementId) {
				auto* active = CLevel_Development::Get_Active(LEVEL::MAHARAKA);
				return active ? active->Find_MapAuthoringNpc(placementId) : nullptr;
			};
			return targets;
		}
	}
	if (levelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			return arena->Get_CompositionWorldTargets();
	/* Valtan lends the same map runtime. It has no World Sequence owner, so the
	   Object preparation and anchor hooks stay empty here. */
	if (levelIndex == ETOUI(LEVEL::VALTAN_ARENA))
	{
		if (auto* arena = CLevel_ValtanArena::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &arena->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &arena->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &arena->Get_MapAuthoringDeploy();
			targets.device = arena->Get_MapAuthoringDevice();
			targets.context = arena->Get_MapAuthoringContext();
			return targets;
		}
	}
	/* Bern and Character Select lend the same live map runtime. Neither Area
	   declares a DeployProp source pair, so the level-owned deploy runtime they
	   hand over stays empty; it exists because the attach contract needs a real
	   owner. Neither level drives map self motions or a World Sequence, so the
	   Object preparation and anchor hooks stay empty here too. */
	if (levelIndex == ETOUI(LEVEL::BERN))
	{
		if (auto* level = CLevel_Bern::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			return targets;
		}
	}
	if (levelIndex == ETOUI(LEVEL::CHARACTER_SELECT))
	{
		if (auto* level = CLevel_CharacterSelect::Get_Active())
		{
			CWorldSequencePlayer::TARGET_SET targets;
			targets.levelIndex = levelIndex;
			targets.pCatalog = &level->Get_MapAuthoringRuntime().Get_Catalog();
			targets.pPlacements = &level->Get_MapAuthoringRuntime().Get_MutablePlacements();
			targets.pDeployRuntime = &level->Get_MapAuthoringDeploy();
			targets.device = level->Get_MapAuthoringDevice();
			targets.context = level->Get_MapAuthoringContext();
			return targets;
		}
	}
#endif
	return {};
}

bool_t Client::CMapTool::Is_MapAuthoringLevel() const
{
	return (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::DEVELOPMENT) &&
		CMapEditorWorkspaceService::Is_Active()) ||
		((m_bOpen || m_bRuntimeAuthoring) && Runtime_AuthoringTargets().pPlacements != nullptr);
}

vector<Client::CMapTool::PLACED_ENTRY>& Client::CMapTool::Authoring_Placements()
{
	const auto targets = m_bRuntimeAuthoring ? Runtime_AuthoringTargets() : CWorldSequencePlayer::TARGET_SET{};
	return targets.pPlacements ? *targets.pPlacements : m_Placements;
}
const vector<Client::CMapTool::PLACED_ENTRY>& Client::CMapTool::Authoring_Placements() const
{ return const_cast<CMapTool*>(this)->Authoring_Placements(); }

vector<Client::CMapTool::STATIC_BATCH_ENTRY>& Client::CMapTool::Authoring_Batches()
{
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements)
	{
		if (auto* maharaka = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
				return maharaka->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* kouku = CLevel_KakulSaydonArena::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
				return kouku->Get_MapAuthoringBatches();
		if (auto* valtan = CLevel_ValtanArena::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
				return valtan->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* bern = CLevel_Bern::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN))
				return bern->Get_MapAuthoringRuntime().Get_AuthoringBatches();
		if (auto* select = CLevel_CharacterSelect::Get_Active())
			if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
				return select->Get_MapAuthoringRuntime().Get_AuthoringBatches();
	}
#endif
	return m_StaticBatches;
}
const vector<Client::CMapTool::STATIC_BATCH_ENTRY>& Client::CMapTool::Authoring_Batches() const
{ return const_cast<CMapTool*>(this)->Authoring_Batches(); }

CDeployPropRuntime& Client::CMapTool::Authoring_Deploy()
{
	const auto targets = m_bRuntimeAuthoring ? Runtime_AuthoringTargets() : CWorldSequencePlayer::TARGET_SET{};
	return targets.pDeployRuntime ? *targets.pDeployRuntime : m_DeployRuntime;
}
const CDeployPropRuntime& Client::CMapTool::Authoring_Deploy() const
{ return const_cast<CMapTool*>(this)->Authoring_Deploy(); }

bool_t Client::CMapTool::Can_ChangeRuntimeStructure()
{
	if (m_bRuntimeAuthoring && m_pWorldSequenceToolPanel && m_pWorldSequenceToolPanel->Is_PreviewActive())
	{
		m_Status = "Stop / Restore the Map Tool sequence before changing object membership.";
		return false;
	}
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && !Can_ReplaceRuntimeAuthoringTargets())
	{
		m_Status = "Stop active arena/Object/Composition playback before adding, deleting or reloading map objects.";
		return false;
	}
#endif
	return true;
}

bool_t Client::CMapTool::Can_ReplaceRuntimeAuthoringTargets() const
{
#ifdef _DEBUG
	if (!Runtime_AuthoringTargets().pPlacements)
		return false;
	if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		auto* arena = CLevel_KakulSaydonArena::Get_Active();
		return nullptr != arena && arena->Can_ReplaceMapAuthoringTargets();
	}
	/* Valtan, Bern, Character Select and Maharaka have no level-owned World Sequence or
	   Composition preview to stop before the tool replaces placements. */
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	return levelIndex == ETOUI(LEVEL::VALTAN_ARENA) ||
		levelIndex == ETOUI(LEVEL::MAHARAKA) ||
		levelIndex == ETOUI(LEVEL::BERN) ||
		levelIndex == ETOUI(LEVEL::CHARACTER_SELECT);
#else
	return false;
#endif
}

void Client::CMapTool::Apply_RuntimeAuthoringActive(const bool_t active)
{
#ifdef _DEBUG
	if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
	{
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			level->Set_MapAuthoringActive(active);
		return;
	}
	/* Both Maharaka and Kouku stop self motions while the tool owns poses. */
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KAKULSAYDON_ARENA))
		return;
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
		arena->Set_MapAuthoringActive(active);
#else
	(void)active;
#endif
}

void Client::CMapTool::Remember_RuntimePlacement(const MAP_PLACEMENT_RECORD& record)
{
	if (!m_bRuntimeAuthoring) return;
	const auto found = m_RuntimePlacementIndex.find(record.placementId);
	if (found == m_RuntimePlacementIndex.end())
	{
		m_RuntimePlacementIndex.emplace(record.placementId, m_RuntimePlacementDraft.size());
		m_RuntimePlacementDraft.push_back(record);
	}
	else m_RuntimePlacementDraft[found->second] = record;
	Rebase_RuntimeMotions();
}

void Client::CMapTool::Reset_RuntimePlacementDraft(const vector<MAP_PLACEMENT_RECORD>& records)
{
	m_RuntimePlacementDraft = records;
	m_RuntimePlacementIndex.clear();
	for (size_t index = 0; index < m_RuntimePlacementDraft.size(); ++index)
		m_RuntimePlacementIndex.emplace(m_RuntimePlacementDraft[index].placementId, index);
	Rebase_RuntimeMotions();
}

void Client::CMapTool::Forget_RuntimePlacement(uint64_t placementId)
{
	if (!m_bRuntimeAuthoring) return;
	std::erase_if(m_RuntimePlacementDraft, [placementId](const auto& record) { return record.placementId == placementId; });
	// Reset accepts a reference to the same vector; self-assignment preserves it.
	Reset_RuntimePlacementDraft(m_RuntimePlacementDraft);
}

void Client::CMapTool::Rebase_RuntimeMotions()
{
#ifdef _DEBUG
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
		if (auto* level = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
			level->Rebase_MapAuthoringSelfMotions(m_RuntimePlacementDraft);
	if (m_bRuntimeAuthoring && Runtime_AuthoringTargets().pPlacements &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			arena->Rebase_MapAuthoringSelfMotions(m_RuntimePlacementDraft);
#endif
}

const MAP_PLACEMENT_RECORD& Client::CMapTool::Authored_Placement(const PLACED_ENTRY& entry) const
{
	if (m_bRuntimeAuthoring)
	{
		const auto found = m_RuntimePlacementIndex.find(entry.record.placementId);
		if (found != m_RuntimePlacementIndex.end()) return m_RuntimePlacementDraft[found->second];
	}
	return entry.record;
}

bool_t Client::CMapTool::Begin_EditorAreaSwitch(const size_t descriptorIndex)
{
	if (descriptorIndex >= m_EditorAreas.size() ||
		m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}
	/* The preview drives this Area's actors; stop it while its targets are
	   still the ones it was staged against. */
	Stop_EditorCutscene();
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(),
			Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
			return false;
		}
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	/* Only the model catalog is read here so the admission can start. Every
	   other authoring document still loads inside the one atomic
	   Switch_EditorArea transaction that runs once the prototypes exist. */
	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	m_EditorAreaPreload.iDescriptorIndex = descriptorIndex;
	m_EditorAreaPreload.Catalog = std::move(stagedCatalog);
	m_EditorAreaPreload.iNextEntry = 0;
	Report_EditorAreaPreloadProgress();
	return true;
}

void Client::CMapTool::Report_EditorAreaPreloadProgress()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	m_Status = "Preparing " +
		m_EditorAreas[m_EditorAreaPreload.iDescriptorIndex].label + ": " +
		std::to_string(m_EditorAreaPreload.iNextEntry) + " / " +
		std::to_string(m_EditorAreaPreload.Catalog.Get_Entries().size()) +
		" model prototypes";
}

void Client::CMapTool::Update_EditorAreaPreload()
{
	if (!m_EditorAreaPreload.Is_Active())
		return;

	if (m_iAuthoringLevelIndex != ETOUI(LEVEL::DEVELOPMENT) ||
		!CMapEditorWorkspaceService::Is_Active() ||
		m_EditorAreaPreload.iDescriptorIndex >= m_EditorAreas.size())
	{
		m_EditorAreaPreload = {};
		m_Status = "Map editor Area switch was cancelled";
		return;
	}

	const std::vector<MAP_ASSET_ENTRY>& entries =
		m_EditorAreaPreload.Catalog.Get_Entries();
	const auto admissionStart = std::chrono::steady_clock::now();
	while (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		if (!Admit_AuthoringPrototype(
			entries[m_EditorAreaPreload.iNextEntry]))
		{
			/* The failing asset already named itself in the status and no
			   Area state was replaced, so the editor keeps the loaded Area. */
			m_EditorAreaPreload = {};
			return;
		}
		++m_EditorAreaPreload.iNextEntry;
		if (std::chrono::steady_clock::now() - admissionStart >=
			EDITOR_AREA_ADMISSION_FRAME_BUDGET)
		{
			break;
		}
	}

	if (m_EditorAreaPreload.iNextEntry < entries.size())
	{
		Report_EditorAreaPreloadProgress();
		return;
	}

	const size_t iCommitIndex = m_EditorAreaPreload.iDescriptorIndex;
	m_EditorAreaPreload = {};
	Switch_EditorArea(iCommitIndex);
}

bool_t Client::CMapTool::Switch_EditorArea(const size_t descriptorIndex)
{
	const auto runtimeTargets = Runtime_AuthoringTargets();
	const bool_t runtimeAttach = runtimeTargets.pPlacements && runtimeTargets.pDeployRuntime && runtimeTargets.pCatalog;
	if (descriptorIndex >= m_EditorAreas.size() || !Is_MapAuthoringLevel() ||
		(runtimeAttach && m_EditorAreas[descriptorIndex].areaId != runtimeTargets.pCatalog->Get_AreaId()))
	{
		m_Status = "Map editor Area switch is unavailable";
		return false;
	}

	const EDITOR_AREA_DESCRIPTOR& descriptor =
		m_EditorAreas[descriptorIndex];
	/* Whatever this switch replaces, a running cutscene preview must not keep
	   driving the previous catalog's actors. */
	Stop_EditorCutscene();
	const std::filesystem::path worldSequencePath =
		descriptor.sourcePlacements.parent_path() /
		std::filesystem::path(descriptor.areaId + ".worldsequences.json");
	std::string recoveryStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(worldSequencePath, recoveryStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		descriptor.sourcePlacements, worldSequencePath, recoveryStatus))
	{
		m_Status = recoveryStatus;
		return false;
	}
	std::error_code error;
	if (!std::filesystem::is_regular_file(descriptor.sourceCatalog, error) ||
		error ||
		!std::filesystem::is_regular_file(descriptor.sourcePlacements, error) ||
		error)
	{
		m_Status = "Required authoring map source is missing: " +
			descriptor.areaId;
		return false;
	}

	CMapAssetCatalog stagedCatalog;
	if (!stagedCatalog.Load_Source(
		descriptor.sourceCatalog,
		descriptor.sourcePlacements,
		descriptor.areaId,
		descriptor.sourceMaterials))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}
	if (runtimeAttach && !stagedCatalog.Bind_RuntimePrototypes(*runtimeTargets.pCatalog))
	{
		m_Status = stagedCatalog.Get_Status();
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> records;
	std::string stagedStatus;
	if (!CMapPlacementDocument::Read(
		descriptor.sourcePlacements,
		stagedCatalog,
		records,
		stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	if (runtimeAttach)
	{
		// Refuse a partial/stale runtime: Save must never delete an unloaded source row.
		std::unordered_map<uint64_t, std::string> liveIds;
		for (const auto& entry : *runtimeTargets.pPlacements)
			liveIds.emplace(entry.record.placementId, entry.record.assetId);
		if (liveIds.size() != records.size() || std::any_of(records.begin(), records.end(),
			[&liveIds](const auto& record) {
				const auto found = liveIds.find(record.placementId);
				return found == liveIds.end() || found->second != record.assetId;
			}))
		{
			m_Status = "Runtime/source placement IDs differ. Save in Test, publish and re-enter this level before runtime editing.";
			return false;
		}
	}
	/* The sequence document reference-validates Deploy animation tracks, so
	   it is loaded once this Area's Deploy runtime has been staged below. */
	auto stagedWorldSequencePanel =
		std::make_unique<CWorldSequenceToolPanel>();

	CWorldGameplayDocument stagedWorld;
	CSpawnGroupDocument stagedSpawnGroups;
	if (EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy)
	{
		error.clear();
		if (!std::filesystem::is_regular_file(
			descriptor.gameplayDocument, error) || error ||
			!stagedWorld.Load(
				descriptor.gameplayDocument,
				descriptor.areaId,
				stagedStatus))
		{
			m_Status = error ?
				"Could not inspect required gameplay document" : stagedStatus;
			return false;
		}
		const std::filesystem::path spawnGroupsPath =
			descriptor.gameplayDocument.parent_path() / L"SpawnGroups.world.json";
		if (!stagedSpawnGroups.Load(
			spawnGroupsPath, descriptor.areaId, stagedStatus))
		{
			m_Status = stagedStatus;
			return false;
		}
	}

	CNavGridPaintDocument stagedNavigation;
	CNavRuntimeBlockerDocument stagedBlockers;
	bool_t navigationLoaded = false;
	if (EDITOR_NAVIGATION_POLICY::NONE != descriptor.navigationPolicy)
	{
		error.clear();
		const bool_t hasSource = std::filesystem::is_regular_file(
			descriptor.navigationSource, error);
		if (IsFileInspectionFailure(error) ||
			(!hasSource && !descriptor.allowNavigationBootstrap))
		{
			m_Status = "Required navigation source is missing: " +
				descriptor.areaId;
			return false;
		}
		if (hasSource)
		{
			if (!stagedNavigation.Load(
				descriptor.navigationSource,
				descriptor.navigationPaint,
				stagedStatus) ||
				stagedNavigation.Get_Desc().areaId != descriptor.areaId ||
				!stagedBlockers.Load(
					descriptor.navigationBlockers,
					stagedNavigation.Get_Desc(),
					stagedStatus))
			{
				m_Status = stagedStatus;
				return false;
			}
			navigationLoaded = true;
		}
	}

	/* Pure data document: an Area without a world events path simply has no
	   destruction authoring, a missing file for Valtan starts empty so the
	   first Save can create it, and a corrupt file fails the switch like the
	   other authoring documents do. */
	CWorldDestructionDocument stagedDestruction;
	CEncounterPatternReference stagedEncounterReference;
	std::string stagedEncounterStatus;
	if (!descriptor.worldEventsDocument.empty())
	{
		if (descriptor.encounterReference.empty() ||
			!stagedEncounterReference.Load(
				descriptor.encounterReference, stagedEncounterStatus))
		{
			m_Status = descriptor.encounterReference.empty() ?
				"World destruction requires an encounter reference" :
				stagedEncounterStatus;
			return false;
		}
		std::error_code destructionError;
		const bool_t hasDocument = std::filesystem::is_regular_file(
			descriptor.worldEventsDocument, destructionError);
		if (IsFileInspectionFailure(destructionError))
		{
			m_Status = "World destruction document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string destructionStatus;
		if (!hasDocument)
		{
			stagedDestruction.Reset_Empty();
		}
		else if (!stagedDestruction.Load(
			descriptor.worldEventsDocument,
			descriptor.areaId,
			"ENCOUNTER_VALTAN",
			destructionStatus))
		{
			m_Status = destructionStatus;
			return false;
		}
	}

	CDestructionSimulationDocument stagedSimulation;
	if (!descriptor.destructionSimulationDocument.empty())
	{
		std::error_code simulationError;
		const bool_t hasSimulation = std::filesystem::is_regular_file(
			descriptor.destructionSimulationDocument, simulationError);
		if (IsFileInspectionFailure(simulationError))
		{
			m_Status = "Destruction simulation document is unreadable: " +
				descriptor.areaId;
			return false;
		}
		std::string simulationStatus;
		if (!hasSimulation)
		{
			stagedSimulation.Reset_Empty();
		}
		else if (!stagedSimulation.Load(
			descriptor.destructionSimulationDocument,
			descriptor.areaId,
			simulationStatus))
		{
			m_Status = simulationStatus;
			return false;
		}
	}
	if (stagedSimulation.Is_Ready() && stagedDestruction.Is_Ready() &&
		!stagedSimulation.Validate_GroupReferences(
			stagedDestruction, stagedStatus))
	{
		m_Status = stagedStatus;
		return false;
	}
	shared_ptr<CMapLightPresentationRuntime> stagedMapLightPresentation;
	if (!descriptor.sourceLights.empty())
	{
		stagedMapLightPresentation =
			make_shared<CMapLightPresentationRuntime>();
		if (!stagedMapLightPresentation->Load(
			descriptor.sourceLights, descriptor.areaId))
		{
			m_Status = stagedMapLightPresentation->Get_Status();
			return false;
		}
	}

	/* Read-only here: a rejected shot document must never block the Area, it
	   only leaves the list empty with a reported reason. */
	(void)Load_CameraShots(descriptor);

	if (!runtimeAttach && !Ensure_AuthoringPrototypes(stagedCatalog))
		return false;
	const bool_t stagedDebrisPrototypesReady =
		!descriptor.destructionSimulationDocument.empty() &&
		Ensure_DestructionDebrisAuthoringPrototypes(runtimeAttach);
	const std::string stagedDebrisPrototypeStatus =
		descriptor.destructionSimulationDocument.empty() ?
		"PROJECT_AUTHORED debris is not declared for this Area" : m_Status;

	CMapAssetCatalog previousCatalog = m_Catalog;
	m_Catalog = stagedCatalog;
	std::vector<PLACED_ENTRY> stagedPlacements;
	std::vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!runtimeAttach && !Stage_PlacementRuntime(records, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Map Area runtime stage rolled back: " + descriptor.areaId;
		return false;
	}
	CDeployPropRuntime stagedDeployRuntime;
	if (!runtimeAttach && !Stage_DeployProps(descriptor, stagedDeployRuntime))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Catalog = std::move(previousCatalog);
		if (m_Status.empty())
			m_Status = "Deploy prop staging failed: " + descriptor.areaId;
		return false;
	}
	if (!stagedWorldSequencePanel->Load_Area(
		worldSequencePath, descriptor.sourcePlacements, descriptor.areaId,
		stagedCatalog, records, runtimeAttach ? *runtimeTargets.pDeployRuntime : stagedDeployRuntime,
		stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> stableLinkedRecords;
	if (!CMapPlacementDocument::Read(descriptor.sourcePlacements,
		stagedCatalog, stableLinkedRecords, stagedStatus) ||
		!AreExactlySamePlacementRecords(records, stableLinkedRecords) ||
		!stagedWorldSequencePanel->Matches_LinkedSourceBaseline(stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = "Linked map/sequence source changed while the Area was loading: " +
			stagedStatus;
		return false;
	}
	if (stagedDestruction.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			stagedDestruction,
			runtimeAttach ? *runtimeTargets.pDeployRuntime : stagedDeployRuntime,
			stagedBlockers,
			stagedWorld,
			stagedEncounterReference,
			stagedStatus))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = stagedStatus;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedTriggerBoxes;
	if (!Stage_WorldTriggerBoxes(stagedWorld, stagedTriggerBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		m_Catalog = std::move(previousCatalog);
		m_Status = "World trigger box preview staging failed: " + descriptor.areaId +
			" (" + m_WorldGameplayStatus + ")";
		return false;
	}
	vector<NPC_PREVIEW_ENTRY> stagedNpcPreviews;
	if (!runtimeAttach && !Stage_WorldNpcPreviews(
		stagedWorld, stagedNpcPreviews, &stagedNavigation, &stagedBlockers))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		m_Catalog = std::move(previousCatalog);
		m_Status = "World NPC preview staging failed: " + descriptor.areaId;
		return false;
	}
	vector<TRIGGER_BOX_ENTRY> stagedSpawnAnchorBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedSpawnGroups, stagedSpawnAnchorBoxes))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		stagedDeployRuntime.Clear();
		Remove_WorldTriggerBoxes(stagedTriggerBoxes);
		Remove_WorldNpcPreviews(stagedNpcPreviews);
		m_Catalog = std::move(previousCatalog);
		m_Status = "Spawn anchor box staging failed: " + descriptor.areaId +
			" (" + m_WorldGameplayStatus + ")";
		return false;
	}

	/* The staged runtime owns preview seams into the current Deploy runtime.
	   Release them before the Area transaction move-assigns that owner. The
	   previous catalog must be used because m_Catalog currently stages the
	   destination Area. */
	if (nullptr != m_pWorldSequenceToolPanel && previousCatalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, previousCatalog, Authoring_Placements(),
			Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			const std::string restoreFailure =
				m_pWorldSequenceToolPanel->Get_Status();
			Remove_PlacementRuntime(stagedPlacements, stagedBatches);
			stagedDeployRuntime.Clear();
			Remove_WorldTriggerBoxes(stagedTriggerBoxes);
			Remove_WorldNpcPreviews(stagedNpcPreviews);
			Remove_WorldTriggerBoxes(stagedSpawnAnchorBoxes);
			m_Catalog = std::move(previousCatalog);
			m_Status = restoreFailure;
			return false;
		}
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	m_bDestructionSimulationClearRequested = false;
	if (!runtimeAttach) Remove_PlacementRuntime(Authoring_Placements(), Authoring_Batches());
	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	if (!runtimeAttach)
	{
		Authoring_Placements() = std::move(stagedPlacements);
		Authoring_Batches() = std::move(stagedBatches);
		Authoring_Deploy() = std::move(stagedDeployRuntime);
	}
	m_bRuntimeAuthoring = runtimeAttach;
	Reset_RuntimePlacementDraft(runtimeAttach ? records : vector<MAP_PLACEMENT_RECORD>{});
	if (runtimeAttach)
	{
		// These identities belong to the active arena loader, not a second prototype owner.
		for (const auto& asset : runtimeTargets.pCatalog->Get_Entries())
			m_PrototypeModelPaths.emplace(asset.prototypeTag, asset.resolvedModelPath.lexically_normal());
		for (const auto& asset : runtimeTargets.pDeployRuntime->Get_Catalog().Get_Assets())
		{
			m_PrototypeModelPaths.emplace(asset.intactPrototypeTag, asset.intactResolvedPath.lexically_normal());
			if (!asset.fracturedPrototypeTag.empty())
				m_PrototypeModelPaths.emplace(asset.fracturedPrototypeTag, asset.fracturedResolvedPath.lexically_normal());
		}
	}
	m_pWorldSequenceToolPanel = std::move(stagedWorldSequencePanel);
	m_WorldGameplayDocument = std::move(stagedWorld);
	m_WorldTriggerBoxes = std::move(stagedTriggerBoxes);
	m_WorldNpcPreviews = std::move(stagedNpcPreviews);
	m_SpawnGroupDocument = std::move(stagedSpawnGroups);
	m_SpawnAnchorBoxes = std::move(stagedSpawnAnchorBoxes);
	m_NavigationDocument = std::move(stagedNavigation);
	m_RuntimeBlockerDocument = std::move(stagedBlockers);
	m_DestructionDocument = std::move(stagedDestruction);
	m_EncounterReference = std::move(stagedEncounterReference);
	m_DestructionSimulationDocument = std::move(stagedSimulation);
	m_pMapLightPresentation = std::move(stagedMapLightPresentation);
	m_bMapLightSubmissionFailureReported = false;
	m_WorldEventsPath = descriptor.worldEventsDocument;
	m_NavigationSourcePath = descriptor.navigationSource;
	m_NavigationPaintPath = descriptor.navigationPaint;
	m_RuntimeBlockerPath = descriptor.navigationBlockers;
	m_NavigationRuntimePath.clear();
	m_NavigationBakeDesc = navigationLoaded ?
		m_NavigationDocument.Get_BakeDesc() : NAVGRID_BAKE_DESC{};
	m_iActiveEditorArea = descriptorIndex;
	m_bDestructionDebrisPrototypesReady = stagedDebrisPrototypesReady;
	m_DestructionDebrisPrototypeStatus = stagedDebrisPrototypeStatus;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_iSelectedPlacementId = 0;
	m_SelectedWorldPlacementId.clear();
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_SelectedAssetId.clear();
	Remove_WorldTriggerBoxes(m_DestructionHighlightBoxes);
	m_SelectedDestructionGroupId.clear();
	m_SelectedDestructionBindingId.clear();
	m_SelectedDestructionStageId.clear();
	m_SelectedDestructionPatternId.clear();
	m_iSelectedDeployPlacementId = 0;
	m_bDeployDirty = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_DestructionPreviewPreviousStates.clear();
	m_bDestructionPickArmed = false;
	m_bDestructionAddMemberArmed = false;
	m_bDestructionNewSettingArmed = false;
	m_bDestructionTimelinePlaying = false;
	m_fDestructionTimelineMs = 0.f;
	m_EncounterReferenceStatus = descriptor.encounterReference.empty() ?
		"Active Area declares no encounter reference" :
		stagedEncounterStatus;
	Reset_DestructionSimulationUI();
	if (!m_DestructionSimulationDocument.Get_Profiles().empty())
	{
		const DESTRUCTION_SIMULATION_PROFILE& firstProfile =
			m_DestructionSimulationDocument.Get_Profiles().front();
		m_SelectedDestructionGroupId = firstProfile.groupId;
		Select_DestructionSimulationProfile(firstProfile.profileId);
		Refresh_DestructionHighlight();
	}
	if (!m_bDestructionDebrisPrototypesReady &&
		!descriptor.destructionSimulationDocument.empty())
	{
		m_DestructionSimulationStatus = m_DestructionDebrisPrototypeStatus;
	}
	m_DestructionStatus = descriptor.worldEventsDocument.empty() ?
		"World destruction authoring disabled for this Area" :
		"World destruction authoring ready";
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
				CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_WorldGameplayStatus =
		EDITOR_GAMEPLAY_POLICY::REQUIRED == descriptor.gameplayPolicy ?
		"Gameplay authoring ready" : "Gameplay authoring disabled for this Area";
	m_NavigationStatus =
		EDITOR_NAVIGATION_POLICY::NONE == descriptor.navigationPolicy ?
		"Navigation authoring disabled for this Area" :
		(navigationLoaded ? "Navigation authoring ready" :
			"Navigation bootstrap: place Nav Bounds and Bake");
	m_Status = "Active editor Area: " + descriptor.label + " (" +
		descriptor.areaId + ") / " + std::to_string(Authoring_Placements().size()) +
		" placements. Runtime publish is separate.";
	if (!runtimeAttach) Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	Rebuild_EditorSublevelJumps();

	if (!runtimeAttach && !Focus_ActiveEditorAreaCamera())
		m_Status += " Camera focus unavailable: " + m_CameraStatus;
	return true;
}

void Client::CMapTool::Handle_LevelTransition(
	uint32_t currentLevelIndex,
	bool_t isMapAuthoringLevel)
{
	const uint32_t targetLevelIndex = isMapAuthoringLevel ?
		currentLevelIndex : ETOUI(LEVEL::END);
	if (targetLevelIndex == m_iAuthoringLevelIndex)
		return;
	/* The cutscene session's actors and camera claim belonged to the Level
	   being left; drop the tool's references before its containers go. */
	Abandon_EditorCutscene("Cutscene preview stopped: the authoring Level changed.");
	/* The old Level owns the Deploy overlay targets and may already be torn
	   down. Do not carry that preview request into the next Area. */
	m_bCameraPreviewSurroundingsCleared = false;
	m_CameraPreviewSuppressedDeployPlacementIds.clear();
	// Old level ownership is already gone during a Level transition. Never clear borrowed containers.
	m_bRuntimeAuthoring = false;
	m_RuntimePlacementDraft.clear();
	m_RuntimePlacementIndex.clear();
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(),
			Authoring_Deploy());
	}
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();
	Reset_DestructionSimulationUI();

	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldTriggerTargetPickArmed = false;
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_bWorldNpcBatchCenterValid = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchArchetypePool.clear();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldNpcBatchCopySelectedBehavior = false;
	m_bSpawnAnchorPlacementArmed = false;
	m_bAnimatedPropPlacementArmed = false;
	m_SelectedWorldPlacementId.clear();
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_SelectedDeployAssetId.clear();
	m_AnimatedPropFilter[0] = '\0';
	m_iSelectedAnimatedPropPlacementId = 0;
	Sync_AnimatedPropTransformDraft();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	Authoring_Placements().clear();
	Authoring_Batches().clear();
	Authoring_Deploy().Reset_ClearedLevelTracking();
	m_WorldTriggerBoxes.clear();
	m_SpawnAnchorBoxes.clear();
	m_iNextPlacementId = 1;
	m_bDirty = false;
	m_bDeployDirty = false;
	m_bWorldGameplayDirty = false;
	m_bSpawnGroupsDirty = false;
	m_SpawnGroupDocument.Reset();
	m_DestructionSimulationDocument.Clear();
	m_pMapLightPresentation.reset();
	m_bMapLightSubmissionFailureReported = false;
	if (nullptr != m_pWorldSequenceToolPanel)
		m_pWorldSequenceToolPanel->Reset();
	m_Catalog = CMapAssetCatalog{};
	m_EditorAreas.clear();
	m_EditorSublevelJumps.clear();
	m_iActiveEditorArea = SIZE_MAX;
	m_iPendingEditorArea = SIZE_MAX;
	m_isEditorAreaSwitchPending = false;
	m_isEditorExitPending = false;
	m_PrototypeModelPaths.clear();
	m_EditorAreaPreload = {};
	m_bDestructionDebrisPrototypesReady = false;
	/* Prototypes live under the Level index that is being left. */
	m_bWorldObjectPrototypeReady = false;
	m_DestructionDebrisPrototypeStatus =
		"PROJECT_AUTHORED debris models are not admitted";
	m_iAuthoringLevelIndex = targetLevelIndex;

	if (!isMapAuthoringLevel)
	{
		m_Status = "Current level has no map Area to author";
		m_WorldGameplayStatus = "Current level has no gameplay Area";
		m_NavigationStatus = "Current level has no navigation Area";
		m_CameraStatus = "Current level has no authoring camera";
		return;
	}

	Find_AssetTestCamera();
	if (!Load_EditorAreaRegistry() || m_EditorAreas.empty())
		return;
	const auto runtimeTargets = Runtime_AuthoringTargets();
	if (runtimeTargets.pCatalog)
	{
		const auto found = std::find_if(m_EditorAreas.begin(), m_EditorAreas.end(),
			[&runtimeTargets](const auto& area) { return area.areaId == runtimeTargets.pCatalog->Get_AreaId(); });
		if (found == m_EditorAreas.end())
			m_Status = "The current runtime Area is not registered for authoring.";
		else Switch_EditorArea(static_cast<size_t>(found - m_EditorAreas.begin()));
	}
	else Switch_EditorArea(0);
}
bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			m_iAuthoringLevelIndex,
			TEXT("Layer_Camera"),
			0);
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == camera)
	{
		m_pAssetTestCamera.reset();
		m_CameraStatus = "ASSET_TEST camera is unavailable";
		return false;
	}

	m_pAssetTestCamera = camera;
	m_CameraStatus = "Camera ready";
	return true;
}

bool_t Client::CMapTool::Focus_ActiveEditorAreaCamera()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CameraStatus = "Select an Area before focusing the camera";
		return false;
	}

	float3_t center{};
	f32_t radius = 0.f;
	bool_t hasFrame = false;
	/* Product spawn data is the stable authoring focus. Distant backdrop
	   meshes must not pull the editor camera away from the playable entry.
	   Any Area that declares player spawns uses them; one without a gameplay
	   document falls through to the placement bounds below. */
	hasFrame = TryBuildGameplaySpawnFrame(
		m_WorldGameplayDocument,
		center,
		radius);

	if (!hasFrame && !Authoring_Placements().empty())
	{
		float3_t minimum = Authoring_Placements().front().record.position;
		float3_t maximum = minimum;
		for (const PLACED_ENTRY& entry : Authoring_Placements())
		{
			minimum.x = (std::min)(minimum.x, entry.record.position.x);
			minimum.y = (std::min)(minimum.y, entry.record.position.y);
			minimum.z = (std::min)(minimum.z, entry.record.position.z);
			maximum.x = (std::max)(maximum.x, entry.record.position.x);
			maximum.y = (std::max)(maximum.y, entry.record.position.y);
			maximum.z = (std::max)(maximum.z, entry.record.position.z);
		}
		center = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		radius = (std::max)(50.f,
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z) * 0.35f);
		hasFrame = true;
	}
	if (!hasFrame)
	{
		m_CameraStatus = "Active Area has no valid camera focus target";
		return false;
	}

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	camera->Frame_Area(center, radius);
	m_CameraStatus = "Camera focused on " + descriptor->label;
	return true;
}

/* An extracted Area can span several authored sublevels that sit thousands of
   units apart, so walking between them is impractical. Grouping the committed
   placements by their authored sourceLevel gives one framing target per space
   without hard-coding any single Area's coordinates. */
void Client::CMapTool::Rebuild_EditorSublevelJumps()
{
	m_EditorSublevelJumps.clear();

	struct SUBLEVEL_BOUNDS final
	{
		float3_t minimum{};
		float3_t maximum{};
		size_t count = 0u;
	};
	std::map<std::string, SUBLEVEL_BOUNDS> bounds;
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		if (entry.record.sourceLevel.empty())
			continue;
		const float3_t& position = entry.record.position;
		const auto [iter, inserted] = bounds.try_emplace(
			entry.record.sourceLevel, SUBLEVEL_BOUNDS{ position, position, 0u });
		SUBLEVEL_BOUNDS& value = iter->second;
		if (!inserted)
		{
			value.minimum.x = (std::min)(value.minimum.x, position.x);
			value.minimum.y = (std::min)(value.minimum.y, position.y);
			value.minimum.z = (std::min)(value.minimum.z, position.z);
			value.maximum.x = (std::max)(value.maximum.x, position.x);
			value.maximum.y = (std::max)(value.maximum.y, position.y);
			value.maximum.z = (std::max)(value.maximum.z, position.z);
		}
		++value.count;
	}

	/* A single group is the whole Area, which the Focus Area button already
	   frames, so publishing one shortcut for it would only add a redundant key. */
	if (bounds.size() < 2u)
		return;

	for (const auto& [sourceLevel, value] : bounds)
	{
		EDITOR_SUBLEVEL_JUMP jump;
		jump.label = sourceLevel;
		jump.center = float3_t(
			(value.minimum.x + value.maximum.x) * 0.5f,
			(value.minimum.y + value.maximum.y) * 0.5f,
			(value.minimum.z + value.maximum.z) * 0.5f);
		/* Same framing allowance the Area focus uses, so a jump lands at a
		   comparable distance instead of inside the geometry. */
		jump.radius = (std::max)(50.f,
			(std::max)(value.maximum.x - value.minimum.x,
				value.maximum.z - value.minimum.z) * 0.35f);
		jump.placementCount = value.count;
		m_EditorSublevelJumps.push_back(std::move(jump));
	}
}

bool_t Client::CMapTool::Jump_ToEditorSublevel(const size_t jumpIndex)
{
	if (jumpIndex >= m_EditorSublevelJumps.size())
		return false;

	shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		if (!Find_AssetTestCamera())
			return false;
		camera = m_pAssetTestCamera.lock();
	}
	if (nullptr == camera)
	{
		m_CameraStatus = "ASSET_TEST camera reacquisition failed";
		return false;
	}

	const EDITOR_SUBLEVEL_JUMP& jump = m_EditorSublevelJumps[jumpIndex];
	camera->Frame_Area(jump.center, jump.radius);
	m_CameraStatus = "Camera jumped to " + jump.label + " (" +
		std::to_string(jump.placementCount) + " placements)";
	return true;
}

/* Driven from the workspace bar, which only the isolated Development editor
   shell renders. The keys therefore stop existing outside that shell, outside
   an Area that produced jump targets, and while a text field has focus. */
void Client::CMapTool::Update_EditorSublevelJumpShortcuts()
{
	if (m_EditorSublevelJumps.empty() || ImGui::GetIO().WantTextInput)
		return;

	static constexpr ImGuiKey ROW_KEYS[] = {
		ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5,
		ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9 };
	static constexpr ImGuiKey PAD_KEYS[] = {
		ImGuiKey_Keypad1, ImGuiKey_Keypad2, ImGuiKey_Keypad3,
		ImGuiKey_Keypad4, ImGuiKey_Keypad5, ImGuiKey_Keypad6,
		ImGuiKey_Keypad7, ImGuiKey_Keypad8, ImGuiKey_Keypad9 };
	const size_t bound = (std::min)(
		m_EditorSublevelJumps.size(), std::size(ROW_KEYS));
	for (size_t index = 0u; index < bound; ++index)
	{
		if (ImGui::IsKeyPressed(ROW_KEYS[index], false) ||
			ImGui::IsKeyPressed(PAD_KEYS[index], false))
		{
			(void)Jump_ToEditorSublevel(index);
			return;
		}
	}
}
```

## C:\Users\USER\source\졸업팀폴\LostArk/Client/Private/MapTool_Cutscenes.cpp

```cpp
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




bool_t Client::CMapTool::Build_CutsceneTargets(
	CWorldSequencePlayer::TARGET_SET& outTargets)
{
	if (!m_Catalog.Is_Ready())
		return false;
	outTargets.levelIndex = m_iAuthoringLevelIndex;
	outTargets.pCatalog = &m_Catalog;
	outTargets.pPlacements = &Authoring_Placements();
	outTargets.pDeployRuntime = &Authoring_Deploy();
	/* Object-resource actors build their own models, which the existing
	   Kouku callers could skip because their props were already placed.
	   Leaving these null makes every such actor fail to prepare. */
	outTargets.device = m_pDevice;
	outTargets.context = m_pContext;
	outTargets.objectPreparationOwner = &m_ArenaRisePlayer;
	/* MapTool samples cutscenes after MainApp has already committed ordinary
	   Effect requests for this frame. Its newly born V1 world roots therefore
	   need their scoped post-update commit before the same-frame seek. */
	outTargets.bCommitWorldRootEffectsAfterSpawn = true;
	/* Camera Play builds its own target set instead of using the World panel's.
	   Forward only the live NPC lookup; the editor retains its draft placements. */
	if (m_bRuntimeAuthoring)
		outTargets.previewNpc = Runtime_AuthoringTargets().previewNpc;
	return outTargets.Is_Complete();
}

bool_t Client::CMapTool::Ensure_WorldObjectPrototype()
{
	if (m_bWorldObjectPrototypeReady)
		return true;
	const std::string levelText = std::to_string(m_iAuthoringLevelIndex);
	/* Raid Levels own this factory before any editor opens: Kouku registers
	   it on activation and Valtan in its Loader rollback scope. Only isolated
	   editor Levels need the Map Tool registration below. */
	if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == m_iAuthoringLevelIndex ||
		ETOUI(LEVEL::VALTAN_ARENA) == m_iAuthoringLevelIndex)
	{
		m_bWorldObjectPrototypeReady = true;
		m_strWorldObjectPrototypeStatus =
			"World Object prototype is owned by the raid Level " +
			levelText + ".";
		return true;
	}
	if (ETOUI(LEVEL::END) <= m_iAuthoringLevelIndex)
	{
		m_strWorldObjectPrototypeStatus =
			"World Object prototype: no authoring Level is bound (index " +
			levelText + ").";
		return false;
	}
	/* A null prototype is a failure of its own - device, context or the
	   shader it binds - and is reported before Add_Prototype can fold it into
	   the same E_FAIL as a duplicate. */
	unique_ptr<CWorldSequenceObject> prototype =
		CWorldSequenceObject::Create(m_pDevice, m_pContext);
	if (nullptr == prototype)
	{
		m_strWorldObjectPrototypeStatus =
			"World Object prototype could not be created on Level " + levelText +
			" (device, context or shader initialisation failed).";
		return false;
	}
	if (SUCCEEDED(CGameInstance::Get().Add_Prototype(m_iAuthoringLevelIndex,
		CWorldSequenceObject::PROTOTYPE_TAG, std::move(prototype))))
	{
		m_bWorldObjectPrototypeReady = true;
		m_strWorldObjectPrototypeStatus =
			"World Object prototype registered by Map Tool on Level " +
			levelText + ".";
		return true;
	}
	/* Add_Prototype rejects exactly four inputs (Prototype_Manager.cpp): an
	   uninitialised manager, a Level index at or past the count the engine was
	   started with, a null prototype, or a tag already present on that Level.
	   The engine is running with LEVEL::END levels (MainApp engineDesc), the
	   index was checked against that above and the prototype is not null, so
	   the one remaining cause is this tag already sitting on this Level from
	   an earlier registration - the tool left and re-entered the Level without
	   a Change_Level to clear it. That prototype is the same class and usable. */
	m_bWorldObjectPrototypeReady = true;
	m_strWorldObjectPrototypeStatus =
		"World Object prototype was already registered on Level " + levelText +
		"; reused.";
	return true;
}

const Client::CMapTool::EDITOR_CUTSCENE* Client::CMapTool::Find_EditorCutscene(
	const std::string& cutsceneId) const
{
	const auto found = std::find_if(m_Cutscenes.begin(), m_Cutscenes.end(),
		[&cutsceneId](const EDITOR_CUTSCENE& cutscene)
		{ return cutscene.cutsceneId == cutsceneId; });
	return m_Cutscenes.end() == found ? nullptr : &*found;
}

const Client::CMapTool::EDITOR_CAMERA_SHOT* Client::CMapTool::Find_CameraShot(
	const std::string& shotId) const
{
	const auto found = std::find_if(m_CameraShots.begin(), m_CameraShots.end(),
		[&shotId](const EDITOR_CAMERA_SHOT& shot)
		{ return shot.shotId == shotId; });
	return m_CameraShots.end() == found ? nullptr : &*found;
}

const Client::CMapTool::EDITOR_CUTSCENE_CUT* Client::CMapTool::Find_CutsceneCutAt(
	const EDITOR_CUTSCENE& cutscene,
	const f32_t timeMs,
	f32_t& outLocalMs) const
{
	/* Cuts are stored in start order and the loader rejects overlap, so the
	   first cut whose half-open span [start, end) holds T owns it: at a shared
	   boundary the next cut's first key takes over, with no blend. The single
	   exception is the end instant of the last cut, which still shows its
	   final pose instead of handing the camera back one instant early. */
	const EDITOR_CUTSCENE_CUT* lastCut = cutscene.cameraCuts.empty() ?
		nullptr : &cutscene.cameraCuts.back();
	for (const EDITOR_CUTSCENE_CUT& cut : cutscene.cameraCuts)
	{
		const EDITOR_CAMERA_SHOT* shot = Find_CameraShot(cut.shotId);
		if (nullptr == shot)
			continue;
		const f32_t startMs = static_cast<f32_t>(cut.startMs);
		const f32_t endMs = startMs +
			static_cast<f32_t>((std::max)(0, shot->trackDurationMs));
		if (timeMs < startMs)
			break;
		if (timeMs < endMs || (&cut == lastCut && timeMs <= endMs))
		{
			outLocalMs = timeMs - startMs;
			return &cut;
		}
	}
	outLocalMs = 0.f;
	return nullptr;
}

bool_t Client::CMapTool::Prepare_EditorCutsceneWorld(
	const EDITOR_CUTSCENE& cutscene)
{
	m_CutsceneSessionInstanceIds.clear();
	/* Camera-only is a first-class case: a cutscene that names no World
	   instance must still play, so this reports success without touching the
	   World player at all. */
	if (cutscene.worldInstanceIds.empty())
	{
		m_CutsceneWorldSource = "World 배우 없음 (카메라만 재생)";
		return true;
	}
	m_CutsceneWorldSource.clear();
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CutsceneStatus = "No active Area for the World actors.";
		return false;
	}
	CWorldSequencePlayer::TARGET_SET targets{};
	if (!Build_CutsceneTargets(targets))
	{
		m_CutsceneStatus = "World actors need a loaded Area.";
		return false;
	}
	if (!Ensure_WorldObjectPrototype())
	{
		m_CutsceneStatus = m_strWorldObjectPrototypeStatus;
		return false;
	}
	/* The editor judges the draft it is editing, so the World Sequence
	   document the panel holds for this Area is admitted, unsaved edits
	   included. The published runtime file is used only when that draft is
	   not loaded, and the section says which one is on screen. */
	std::string admission;
	const bool_t hasDraft = nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready() &&
		m_pWorldSequenceToolPanel->Get_Document().Get_AreaId() ==
			descriptor->areaId;
	if (hasDraft)
	{
		if (!m_ArenaRisePlayer.Replace_DocumentKeepingModels(
			m_pWorldSequenceToolPanel->Get_Document(), targets, admission))
		{
			m_CutsceneStatus = "World draft could not be admitted: " + admission;
			return false;
		}
		m_CutsceneWorldSource = m_pWorldSequenceToolPanel->Is_Dirty() ?
			"저작 draft (미저장 변경 포함)" :
			"저작본 (Data/Maps/Authoring, 저장된 상태)";
	}
	else
	{
		if (!m_ArenaRisePlayer.Load_Area(descriptor->areaId, targets))
		{
			m_CutsceneStatus = "World Sequence load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_CutsceneWorldSource =
			"게시본 (Client/Bin/DataFiles/Map) - 이 Area 의 저작 draft 없음";
	}
	m_bArenaRiseAreaLoaded = true;
	m_strArenaRiseLoadedArea = descriptor->areaId;
	/* Start every instance this cutscene owns, then pause so the session
	   clock is the only thing that advances them. Every started ID is
	   recorded before the next one runs, so a failure part way through
	   releases the whole cast instead of leaving half of it on stage. */
	for (const std::string& instanceId : cutscene.worldInstanceIds)
	{
		if (!m_ArenaRisePlayer.Play(instanceId, targets))
		{
			const std::string reason = m_ArenaRisePlayer.Get_Status();
			Release_EditorCutsceneWorld(true);
			m_CutsceneStatus = "World instance could not start: " +
				instanceId + " - " + reason;
			return false;
		}
		m_CutsceneSessionInstanceIds.push_back(instanceId);
	}
	m_ArenaRisePlayer.Set_Paused(true);
	return true;
}

void Client::CMapTool::Release_EditorCutsceneWorld(
	const bool_t restorePlacements)
{
	if (m_CutsceneSessionInstanceIds.empty())
		return;
	CWorldSequencePlayer::TARGET_SET targets{};
	const bool_t hasTargets = Build_CutsceneTargets(targets);
	/* Without live targets nothing can be restored, but the actors are still
	   released: Stop_Instance then only hides and removes its own clones. */
	for (const std::string& instanceId : m_CutsceneSessionInstanceIds)
	{
		m_ArenaRisePlayer.Stop_Instance(instanceId,
			hasTargets ? targets : CWorldSequencePlayer::TARGET_SET{},
			restorePlacements && hasTargets);
	}
	m_CutsceneSessionInstanceIds.clear();
	m_bCutsceneWorldPrepared = false;
}

void Client::CMapTool::Abandon_EditorCutscene(const std::string& reason)
{
	if (EDITOR_CUTSCENE_STATE::STOPPED == m_eCutsceneState &&
		m_strCutsceneSessionId.empty() && m_CutsceneSessionInstanceIds.empty())
	{
		return;
	}
	/* The Level these actors lived in is already gone, and the targets that
	   would restore placements belong to it. Only the tool's own clones and
	   its camera claim are dropped. */
	for (const std::string& instanceId : m_CutsceneSessionInstanceIds)
	{
		m_ArenaRisePlayer.Stop_Instance(instanceId,
			CWorldSequencePlayer::TARGET_SET{}, false);
	}
	m_CutsceneSessionInstanceIds.clear();
	End_CutsceneCameraTrack();
	m_bCutsceneCameraHeld = false;
	m_bCutsceneWorldPrepared = false;
	m_bCutsceneWorldFailed = false;
	m_bCutsceneWorldPreviewStale = false;
	m_eCutsceneState = EDITOR_CUTSCENE_STATE::STOPPED;
	m_strCutsceneSessionId.clear();
	m_strCutsceneSessionArea.clear();
	m_strCutsceneActiveCutId.clear();
	m_fCutsceneSessionMs = 0.f;
    m_bCutsceneSoundNaturallyFinished = false;
    m_bCutsceneSoundSeekRequested = false;
	m_CutsceneStatus = reason;
}

bool_t Client::CMapTool::Refresh_EditorCutsceneWorldDraft()
{
    m_bCutsceneSoundNaturallyFinished = false;
    m_bCutsceneSoundSeekRequested = true;
	m_bCutsceneWorldPreviewStale = false;
	if (EDITOR_CUTSCENE_STATE::STOPPED == m_eCutsceneState)
		return true;
	const EDITOR_CUTSCENE* cutscene = Find_EditorCutscene(m_strCutsceneSessionId);
	if (nullptr == cutscene)
		return false;
	/* The edited draft replaces the running cast in place; the session clock
	   keeps its time so the change is judged at the same instant. */
	Release_EditorCutsceneWorld(true);
	m_bCutsceneWorldFailed = false;
	if (!Prepare_EditorCutsceneWorld(*cutscene))
	{
		m_bCutsceneWorldFailed = !cutscene->worldInstanceIds.empty();
		m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
		return false;
	}
	m_bCutsceneWorldPrepared = !m_CutsceneSessionInstanceIds.empty();
	return true;
}

void Client::CMapTool::Seek_EditorCutsceneWorld()
{
    const auto* source = Find_EditorCutscene(m_strCutsceneSessionId);
    if (m_bCutsceneSoundNaturallyFinished && source && m_fCutsceneSessionMs >= source->durationMs) return;
    m_bCutsceneSoundNaturallyFinished = false;
	if (!m_bCutsceneWorldPrepared || m_CutsceneSessionInstanceIds.empty())
		return;
	CWorldSequencePlayer::TARGET_SET targets{};
	if (!Build_CutsceneTargets(targets))
	{
		Release_EditorCutsceneWorld(false);
		m_bCutsceneWorldFailed = true;
		m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
		m_CutsceneStatus = "World actors lost the Area they were staged in; "
			"all were released and the preview paused.";
		return;
	}
	/* One absolute seek per frame, and only for this session's instances.
	   Calling Update as well would advance them a second time and drift them
	   away from the camera. */
    const bool_t playing = m_eCutsceneState == EDITOR_CUTSCENE_STATE::PLAYING;
	m_ArenaRisePlayer.Set_Paused(!playing);
	for (const std::string& instanceId : m_CutsceneSessionInstanceIds)
	{
        f32_t previousMs = 0.f;
        const bool_t scrubbed = m_bCutsceneSoundSeekRequested || (!playing &&
            (!m_ArenaRisePlayer.Try_GetElapsedMs(instanceId, previousMs) || previousMs != m_fCutsceneSessionMs));
		if (m_ArenaRisePlayer.Seek_InstanceToMs(
			instanceId, m_fCutsceneSessionMs, targets, scrubbed))
		{
			continue;
		}
		/* One actor that cannot be applied fails the whole cast: the player
		   has already stopped that one, and the survivors are released here
		   too, so a failure never reads as a partial success on screen. The
		   camera can go on only after the editor presses Resume. */
		const std::string reason = m_ArenaRisePlayer.Get_Status();
		Release_EditorCutsceneWorld(true);
		m_bCutsceneWorldFailed = true;
		m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
		m_CutsceneStatus = "World actor failed at " +
			std::to_string(static_cast<int32_t>(m_fCutsceneSessionMs)) +
			" ms: " + instanceId + " - " + reason +
			". Every actor was released; Resume continues camera-only.";
		return;
	}
    m_bCutsceneSoundSeekRequested = false;
}

bool_t Client::CMapTool::Play_EditorCutscene(const std::string& cutsceneId)
{
    m_bCutsceneSoundNaturallyFinished = false;
	const EDITOR_CUTSCENE* cutscene = Find_EditorCutscene(cutsceneId);
	if (nullptr == cutscene)
	{
		m_CutsceneStatus = "Unknown cutscene: " + cutsceneId;
		return false;
	}
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CutsceneStatus = "No active Area.";
		return false;
	}
	/* Whatever the previous session owned is handed back before the new one
	   takes anything, so two cutscenes never drive the camera together. */
	Stop_EditorCutscene();
	m_bCutsceneWorldFailed = false;
	m_bCutsceneWorldPreviewStale = false;
	if (!Prepare_EditorCutsceneWorld(*cutscene))
		return false;
	m_bCutsceneWorldPrepared = !m_CutsceneSessionInstanceIds.empty();
	m_strCutsceneSessionId = cutscene->cutsceneId;
	m_strCutsceneSessionArea = descriptor->areaId;
	m_fCutsceneSessionMs = 0.f;
    m_bCutsceneSoundNaturallyFinished = false;
    m_bCutsceneSoundSeekRequested = false;
	m_strCutsceneActiveCutId.clear();
	m_eCutsceneState = EDITOR_CUTSCENE_STATE::PLAYING;
	m_CutsceneStatus = "Playing " + cutscene->displayName;
	return true;
}

void Client::CMapTool::Stop_EditorCutscene()
{
	if (EDITOR_CUTSCENE_STATE::STOPPED == m_eCutsceneState &&
		m_strCutsceneSessionId.empty() && m_CutsceneSessionInstanceIds.empty())
	{
		return;
	}
	/* Release by the IDs this session started, not by the prepared flag or by
	   the cutscene row: a failed seek clears the flag, and a Reload can
	   replace the row, but neither may leave an actor on stage. */
	Release_EditorCutsceneWorld(true);
	m_bCutsceneWorldFailed = false;
	m_bCutsceneWorldPreviewStale = false;
	End_CutsceneCameraTrack();
	m_eCutsceneState = EDITOR_CUTSCENE_STATE::STOPPED;
	m_strCutsceneSessionId.clear();
	m_strCutsceneSessionArea.clear();
	m_strCutsceneActiveCutId.clear();
	m_fCutsceneSessionMs = 0.f;
    m_bCutsceneSoundNaturallyFinished = false;
    m_bCutsceneSoundSeekRequested = false;
}

void Client::CMapTool::Update_EditorCutscene(const f32_t fTimeDelta)
{
	if (EDITOR_CUTSCENE_STATE::STOPPED == m_eCutsceneState)
		return;
    m_ArenaRisePlayer.Update_SoundTails(fTimeDelta);
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	/* Changing Area mid-preview would drive another map's actors, so the
	   session ends with its own Area rather than following along. */
	if (nullptr == descriptor || descriptor->areaId != m_strCutsceneSessionArea)
	{
		Stop_EditorCutscene();
		m_CutsceneStatus = "Cutscene preview stopped: the Area changed.";
		return;
	}
	const EDITOR_CUTSCENE* cutscene = Find_EditorCutscene(m_strCutsceneSessionId);
	if (nullptr == cutscene)
	{
		Stop_EditorCutscene();
		return;
	}
	if (EDITOR_CUTSCENE_STATE::PLAYING == m_eCutsceneState)
	{
		/* Showing a hidden actor for the first time clones its model, so the
		   starting frame is long. Advancing by that whole frame would skip
		   most of the authored motion. */
		m_fCutsceneSessionMs += (std::min)(fTimeDelta,
			KAKUL_CUTSCENE_MAX_STEP_SECONDS) * 1000.f;
		if (m_fCutsceneSessionMs >= static_cast<f32_t>(cutscene->durationMs))
		{
			/* Hold the last instant instead of restarting: the editor judges
			   the ending pose, and a silent loop hides where it ends. */
			m_fCutsceneSessionMs = static_cast<f32_t>(cutscene->durationMs);
            Seek_EditorCutsceneWorld();
            for (const auto& id : m_CutsceneSessionInstanceIds) m_ArenaRisePlayer.Retire_InstanceSoundTails(id);
            m_bCutsceneSoundNaturallyFinished = true;
			m_eCutsceneState = EDITOR_CUTSCENE_STATE::PAUSED;
			m_CutsceneStatus = cutscene->displayName + " reached its end.";
		}
	}
	/* An edit to the World draft is shown at the time it was made, not after
	   the next Play, so the cast is re-admitted before this frame's seek. */
	if (m_bCutsceneWorldPreviewStale)
		(void)Refresh_EditorCutsceneWorldDraft();
	Seek_EditorCutsceneWorld();
	(void)Apply_EditorCutsceneCamera();
}

bool_t Client::CMapTool::Apply_EditorCutsceneCamera()
{
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
		return false;
	const EDITOR_CUTSCENE* cutscene = Find_EditorCutscene(m_strCutsceneSessionId);
	if (nullptr == cutscene)
		return false;
	f32_t localMs = 0.f;
	const EDITOR_CUTSCENE_CUT* cut =
		Find_CutsceneCutAt(*cutscene, m_fCutsceneSessionMs, localMs);
	if (nullptr == cut)
	{
		/* An authored gap, or the tail after the last cut. The original hands
		   the camera back there while the actors keep going, so do the same
		   instead of freezing on the last frame. */
		if (!m_strCutsceneActiveCutId.empty())
		{
			End_CutsceneCameraTrack();
			m_strCutsceneActiveCutId.clear();
		}
		return false;
	}
	const EDITOR_CAMERA_SHOT* shot = Find_CameraShot(cut->shotId);
	if (nullptr == shot)
		return false;
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!Sample_ShotCameraTrack(*shot, localMs, pose))
		return false;
	/* A cut boundary is a hard cut in the source: every director cut here has
	   transitiontime 0. Dropping the held blend state on the change keeps the
	   next cut from gliding in from the previous framing. */
	if (m_strCutsceneActiveCutId != cut->cutId)
	{
		if (!m_strCutsceneActiveCutId.empty())
			End_CutsceneCameraTrack();
		m_strCutsceneActiveCutId = cut->cutId;
	}
	if (!m_bCutsceneCameraHeld)
	{
		if (!camera->Begin_PresentationOverride(
			CAMERA_SHOT_PREVIEW_OWNER_ID,
			CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
		{
			m_CutsceneStatus = "Another preview holds the camera.";
			return false;
		}
		m_bCutsceneCameraHeld = true;
	}
	/* No blend in or out. The cut list is the only thing that decides which
	   pose is on screen, so the editor sees the source cut, not a glide. */
	if (!(pose.hasUp ?
		camera->Apply_PresentationPoseWithUp(CAMERA_SHOT_PREVIEW_OWNER_ID,
			pose.vEye, pose.vLookAt, pose.vUp, pose.fFovYDegrees) :
		camera->Apply_PresentationPose(CAMERA_SHOT_PREVIEW_OWNER_ID,
			pose.vEye, pose.vLookAt, pose.fFovYDegrees)))
	{
		return false;
	}
	return true;
}

void Client::CMapTool::Apply_CutsceneCameraTrack(const f32_t timeDelta)
{
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
		return;
	/* An editor cutscene session owns the camera outright while it runs, so
	   the per-shot clock path below never competes with it. */
	if (EDITOR_CUTSCENE_STATE::STOPPED != m_eCutsceneState)
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
			const auto found = std::find_if(Authoring_Placements().begin(),
				Authoring_Placements().end(),
				[placementId](const PLACED_ENTRY& value)
				{
					return value.record.placementId == placementId;
				});
			bool_t wasVisible = false;
			if (Authoring_Placements().end() == found ||
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
			const auto found = std::find_if(Authoring_Placements().begin(),
				Authoring_Placements().end(),
				[&restore](const PLACED_ENTRY& value)
				{
					return value.record.placementId == restore.first;
				});
			if (Authoring_Placements().end() == found ||
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
	for (PLACED_ENTRY& entry : Authoring_Placements())
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
	const shared_ptr<CDeployPropObject> book = Authoring_Deploy().Find(placementId);
	if (nullptr != book)
		book->End_AnimationAuthoringPreview();
}

void Client::CMapTool::Update_CutsceneArenaRise(
	const f32_t fTimeDelta,
	const bool_t isMapAuthoringLevel)
{
	if (!isMapAuthoringLevel || !m_Catalog.Is_Ready() ||
        m_eCutsceneState != EDITOR_CUTSCENE_STATE::STOPPED)
		return;
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
		/* The editor cutscene session is advanced from CMapTool::Update
		   before this function, so only the per-shot path runs here. */
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
	if (!Authoring_Deploy().Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::DESPAWNED } }))
	{
		OutputDebugStringA(("[MapTool][CutsceneArena] book despawn failed: " +
			Authoring_Deploy().Get_Status() + "\n").c_str());
	}
}

bool_t Client::CMapTool::Play_CutsceneArenaRise()
{
	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = m_iAuthoringLevelIndex;
	targets.pCatalog = &m_Catalog;
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
	if (!Authoring_Deploy().Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::INTACT } }))
	{
		m_Status = "Arena rise could not restore the book: " +
			Authoring_Deploy().Get_Status();
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
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
	if (!Authoring_Deploy().Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
		DEPLOY_PROP_STATE::INTACT } }))
	{
		m_Status = "Original cutscene could not restore the book: " +
			Authoring_Deploy().Get_Status();
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
	if (!Build_CutsceneTargets(targets))
	{
		m_CameraShotStatus = "Camera track needs a loaded Area";
		return false;
	}
	/* The sequence document belongs to the Area being edited. Loading Kouku's
	   here would look up this shot's instance in another map's document. */
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_CameraShotStatus = "Camera track needs a loaded Area";
		return false;
	}
	if (!m_bArenaRiseAreaLoaded ||
		m_strArenaRiseLoadedArea != descriptor->areaId)
	{
		if (!m_ArenaRisePlayer.Load_Area(descriptor->areaId, targets))
		{
			m_CameraShotStatus = "Sequence document load failed: " +
				m_ArenaRisePlayer.Get_Status();
			return false;
		}
		m_bArenaRiseAreaLoaded = true;
		m_strArenaRiseLoadedArea = descriptor->areaId;
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
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
		targets.pPlacements = &Authoring_Placements();
		targets.pDeployRuntime = &Authoring_Deploy();
		m_ArenaRisePlayer.Stop_All(targets);
		m_fCutsceneBookHoldMs = -1.f;
		(void)Authoring_Deploy().Set_States({ { KAKUL_BOOK_PLACEMENT_ID,
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
		targets.pPlacements = &Authoring_Placements();
		targets.pDeployRuntime = &Authoring_Deploy();
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
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
	if (!Ensure_WorldObjectPrototype())
	{
		m_CardMiroMarchStatus =
			"World object prototype registration failed";
		return false;
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
	targets.pPlacements = &Authoring_Placements();
	targets.pDeployRuntime = &Authoring_Deploy();
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
	if (!Authoring_Deploy().Get_Catalog().Is_Ready())
	{
		ImGui::TextWrapped("DeployProp catalog is not loaded: %s",
			Authoring_Deploy().Get_Status().c_str());
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
			Authoring_Deploy().Get_Catalog().Get_Assets())
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
		for (const DEPLOY_RUNTIME_ENTRY& entry : Authoring_Deploy().Get_Entries())
		{
			const DEPLOY_PROP_ASSET_ENTRY* asset =
				Authoring_Deploy().Get_Catalog().Find(entry.placement.assetId);
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
		Authoring_Deploy().Get_Catalog().Get_Placements().size(),
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
```

## C:\Users\USER\source\졸업팀폴\LostArk/Tools/MapPipeline/retarget_maharaka_waterpang_intro.py

```python
"""Video-guided intro15 camera + source smile in the existing MapTool player.

Camera framing and cannon clip scheduling are PROJECT_ADAPTED, not extracted
Matinee coordinates. The original importer and intro20 remain unchanged.
Prepare writes only out/. Apply requires the reviewed byte-identical candidate.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
from pathlib import Path

import build_maharaka_waterpang_sequences as b

OUT = b.ROOT / 'out/MaharakaCameraVideo20260928'
IDENTITY = b.PREFIX + '.intro15'
BIG = 'npc.maharaka.source57009.actor100'
SMALL = 'npc.maharaka.source57009.actor188'


def merge_reviewed(before, candidate, current, path=()):
    """Merge only reviewed fields; preserve unrelated Save edits by stable ID."""
    if before == candidate:
        return current
    if path == ('revision',):
        return current + 1
    if current == before:
        return candidate
    if current == candidate:
        return current
    if all(isinstance(v, dict) for v in (before, candidate, current)):
        result = copy.deepcopy(current)
        for key in set(before) | set(candidate):
            if key not in before:
                b.require(key not in current or current[key] == candidate[key], 'Concurrent added field: '+str(path+(key,)))
                result[key] = candidate[key]
            elif key not in candidate:
                b.require(key not in current or current[key] == before[key], 'Concurrent removed field: '+str(path+(key,)))
                result.pop(key, None)
            elif before[key] != candidate[key]:
                b.require(key in current, 'Concurrent removed target: '+str(path+(key,)))
                result[key] = merge_reviewed(before[key], candidate[key], current[key], path+(key,))
        return result
    identity = {'shots':'shotId','cutscenes':'cutsceneId','objectResources':'objectId',
                'templates':'sequenceId','instances':'instanceId'}.get(path[-1] if path else '')
    if identity and all(isinstance(v, list) for v in (before, candidate, current)):
        maps = [{row[identity]:row for row in rows} for rows in (before,candidate,current)]
        b.require(all(len(m)==len(rows) for m,rows in zip(maps,(before,candidate,current))), 'Duplicate stable identity')
        merged = merge_reviewed(*maps, path=path+('by-id',))
        order = [row[identity] for row in current] + [row[identity] for row in candidate if row[identity] not in maps[2]]
        return [merged[key] for key in order if key in merged]
    if path and path[-1] == 'worldInstanceIds' and candidate[:len(before)] == before:
        return current + [value for value in candidate[len(before):] if value not in current]
    raise RuntimeError('Concurrent edit of reviewed field: '+'.'.join(path))


def smooth(t):
    t = min(1., max(0., t))
    return t*t*(3.-2.*t)


def prepare():
    paths = [b.AUTHORING/(b.AREA+'.'+suffix+'.json') for suffix in ('camerashots', 'worldsequences')]
    dependencies = paths + [b.ROOT/'Data/Actors/NpcCatalog.json',
                            b.ROOT/f'Data/Worlds/{b.AREA}/Gameplay.world.json']
    original = {p: p.read_bytes() for p in dependencies}
    camera, world, npcs, gameplay = [json.loads(original[p].decode('utf-8-sig')) for p in dependencies]
    actors = {p['placementId']: p for p in gameplay['placements']}
    source_path = Path(b.load_json(b.AUDIT)['source']['physicalPackage'])
    b.require(hashlib.sha256(source_path.read_bytes()).hexdigest() == b.load_json(b.AUDIT)['source']['sha256'], 'Source package changed')
    rows, imports = b.source.extract_scene(source_path)
    b.require(imports['-70'] == 'mn_ismp_00.mat.mn_ismp_00-2_mi', 'Source smile material identity changed')
    duration = round(rows[158]['p']['interplength']*1000)
    b.require(duration == 9507, 'Source timing changed')
    scene = next(c for c in camera['cutscenes'] if c['cutsceneId'] == 'cutscene.'+IDENTITY)
    b.require(scene['durationMs'] == duration, 'Preserve user duration edit')
    big_position = b.np.array(actors[BIG]['position'])
    # Installed centimetre model: face is in front (+Z after NPC -90deg import).
    face = big_position + b.np.array([0., 1.5, 1.0])
    close_eye = face + b.np.array([-1.15, .3, 5.3])
    arena = b.np.array(actors[SMALL]['position'])
    heading = b.Rotation.from_euler('y', -45., degrees=True).as_matrix()
    source_close, _ = b.source.world_pose(rows, 206, 3, 8.101, 43, 158)
    rotated_close = arena + heading @ (source_close-arena)
    translate_close = close_eye - rotated_close
    fov_track = rows[304]['p']['floattrack']['points']
    samples = []
    for cut in scene['cameraCuts']:
        shot = next(s for s in camera['shots'] if s['shotId'] == cut['shotId'])
        start = cut['startMs']
        end = start + shot['cameraTrack']['durationMs']
        times = set(range(start, end, 50)) | {start, end}
        times.update(t for t in (6469,6602,7177,8101,8935,9502) if start <= t <= end)
        b.require(len(times) <= 128, 'Camera key capacity')
        keys = []
        for ms in sorted(times):
            p, r = b.source.world_pose(rows, 206, 3, ms/1000., 43, 158)
            p = arena + heading @ (p-arena)
            r = heading @ r
            weight = smooth((ms-6469)/(7177-6469)) * (1.-smooth((ms-8101)/(8935-8101)))
            eye = p + translate_close*weight
            look = (p+r@b.np.array([10.,0.,0.]))*(1.-weight) + face*weight
            up = (r@b.np.array([0.,1.,0.]))*(1.-weight) + b.np.array([0.,1.,0.])*weight
            up /= b.np.linalg.norm(up)
            horizontal = float(b.source.curve(fov_track, ms/1000.,90.))
            vertical = b.math.degrees(2*b.math.atan(b.math.tan(b.math.radians(horizontal)/2)/(16/9)))
            keys.append(dict(sceneId=f'{IDENTITY}.video.k{ms}',timeMs=ms-start,
                eye=eye.tolist(),lookAt=look.tolist(),up=up.tolist(),fovYDegrees=vertical))
            if weight == 1.:
                ray = (look-eye)/b.np.linalg.norm(look-eye)
                miss = b.np.linalg.norm(b.np.cross(face-eye,ray))
                b.require(miss < 1e-8, 'Close-up must aim at the actual large face')
                samples.append(dict(timeMs=ms,eye=eye.tolist(),face=face.tolist(),rayMissM=float(miss)))
        shot['cameraTrack']['keyframes'] = keys
        for key in ('eye','lookAt','fovYDegrees'): shot[key] = keys[0][key]
        shot['box']['center'] = keys[0]['eye']
        shot['displayName'] = '워터팡 / 영상 맞춤 카메라 ' + str(start)
    scene['displayName'] = '워터팡 / 도입 15 · 영상 맞춤 카메라·표정'
    camera['revision'] += 1
    # The source targets the shared smile MIC; both original bodies own this MIC.
    smile = rows[286]['p']['floattrack']['points']
    smile_keys = [dict(timeMs=0,value=[0.,0.,0.,0.],interpolation='CONSTANT')]
    smile_keys += [dict(timeMs=round(p['inval']*1000),value=[p['outval'],0.,0.,0.],interpolation='CONSTANT') for p in smile]
    smile_keys.append(dict(timeMs=duration,value=[0.,0.,0.,0.],interpolation='CONSTANT'))
    for tag, placement_id in [('mokomoko',BIG),('cannon',SMALL)]:
        placement = actors[placement_id]
        actor = next(a for a in npcs['npcs'] if a['archetypeId'] == placement['archetypeId'])
        model = b.ROOT/'Client/Bin/Resources'/actor['modelAssetId']
        original[model] = model.read_bytes()
        decoded = b.source.wm.read_wmodel(model,include_geometry=False)
        b.require(all(any(a.name == clip for a in decoded.animations) for clip in ('idle_normal_1','att_battle_2_01','att_battle_2_02')), 'Missing exact installed clip')
        profile = copy.deepcopy(next(p for p in npcs['modelMaterialOverrides'] if p['modelAssetId'] == actor['modelAssetId'] and p['materialName'] == 'mn_ismp_00-2_mi'))
        del profile['modelAssetId']
        ident = IDENTITY+'.'+tag
        resource = dict(objectId='world.object.'+ident, displayName='워터팡 / '+tag,
            modelAssetId=actor['modelAssetId'], modelPreScale=.01, animated=True, scale=[1.,1.,1.],
            anchorKind='WORLD', materialProfile=profile)
        q = b.Rotation.from_euler('y',placement['yawDegrees']-90.,degrees=True).as_quat().tolist()
        key = dict(timeMs=0,positionOffset=placement['position'],rotationQuaternion=q,scaleMultiplier=[1.,1.,1.],visible=True)
        clips = [dict(slotId='actor',clipName='idle_normal_1',startMs=0,playbackRate=1,loop=True,holdLastFrame=True)]
        if tag == 'cannon':
            # Video-guided rise, using the original installed raise/hold clips.
            # Not asserted to be a serialized Matinee animation binding.
            clips += [dict(slotId='actor',clipName=name,startMs=t,playbackRate=1,loop=loop,holdLastFrame=True)
                      for name,t,loop in [('att_battle_2_01',8101,False),('att_battle_2_02',9101,True)]]
        template = dict(sequenceId='sequence.'+ident,displayName='워터팡 / '+tag+' 표정',category='World',
            durationMs=duration,interpolation='LINEAR',tracks=[dict(slotId='actor',keys=[key,dict(key,timeMs=duration)])],
            animationTracks=clips,materialTracks=[dict(slotId='actor',materialName=profile['materialName'],
                curves=[dict(parameter='opacity_intensity',keys=smile_keys)])])
        instance = dict(instanceId='world.sequence.instance.'+ident,templateId=template['sequenceId'],enabled=True,
            startDelayMs=0,playbackSpeed=1,anchorKind='WORLD',position=[0.,0.,0.],motionEnd='STOP',
            bindings=[dict(slotId='actor',targetKind='OBJECT_RESOURCE',targetId=resource['objectId'],previewNpcPlacementId=placement_id)])
        for field,identity,row in [('objectResources','objectId',resource),('templates','sequenceId',template),('instances','instanceId',instance)]:
            b.require(not any(x[identity] == row[identity] for x in world[field]), 'Already installed: reload the reviewed candidate instead of overwriting edits')
            world[field].append(row)
        scene['worldInstanceIds'].append(instance['instanceId'])
    world['revision'] += 1
    staged = {paths[0]:(original[paths[0]],b.encoded(camera)),paths[1]:(original[paths[1]],b.encoded(world))}
    report = dict(cameraBasis='PROJECT_VIDEO_RETARGET_NOT_RAW_SOURCE_COORDINATES',
        sourceSmile=dict(package=str(source_path),matinee=43,track=286,material=imports['-70'],keys=smile_keys),
        cannonMotionBasis='PROJECT_VIDEO_SCHEDULE_OF_SOURCE_RAISE_AND_HOLD_CLIPS',
        unchanged=['intro20','stage motions','foley','gameplay NPC positions','rendering options'],
        closeupSamples=samples,visualApproval=False)
    return staged, original, report


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply-reviewed',action='store_true')
    args=parser.parse_args()
    OUT.mkdir(parents=True,exist_ok=True)
    receipt=OUT/'reviewed-inputs.json'
    if args.apply_reviewed:
        manifest=b.load_json(receipt)
        expected={b.ROOT/rel:(OUT/'before'/rel).read_bytes() for rel in manifest}
        for path,raw in expected.items():
            b.require(hashlib.sha256(raw).hexdigest()==manifest[path.relative_to(b.ROOT).as_posix()], 'Review backup changed')
        staged = {}
        for path, raw in list(expected.items()):
            candidate_path = OUT/'candidate'/path.relative_to(b.ROOT)
            if not candidate_path.exists():
                continue
            current = path.read_bytes()
            merged = merge_reviewed(json.loads(raw.decode('utf-8-sig')),
                                    b.load_json(candidate_path), json.loads(current.decode('utf-8-sig')))
            backup = OUT/'before-apply'/path.relative_to(b.ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            backup.write_bytes(current)
            staged[path] = (current, b.encoded(merged))
            expected[path] = current
        b.commit_staged_files(staged,expected=expected)
        print('Applied reviewed camera and actor candidates; runtime publish is separate.')
        return
    b.require(not receipt.exists(),'Reviewed candidate already exists; preserve it until applied or explicitly re-reviewed')
    staged,expected,report=prepare()
    for path,raw in expected.items():
        dest=OUT/'before'/path.relative_to(b.ROOT);dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(raw)
    for path,(_,raw) in staged.items():
        dest=OUT/'candidate'/path.relative_to(b.ROOT);dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(raw)
    receipt.write_bytes(b.encoded({p.relative_to(b.ROOT).as_posix():hashlib.sha256(raw).hexdigest() for p,raw in expected.items()}))
    (OUT/'comparison-result.json').write_bytes(b.encoded(report))
    print(json.dumps(dict(candidate=str(OUT/'candidate'),closeupSamples=len(report['closeupSamples']),files=len(staged))))


if __name__ == '__main__':
    main()
```

## Publisher 교체 함수 · Read-WorldSequenceDocument

```powershell
function Read-WorldSequenceDocument {
    param([string]$Path)
    if ([IO.FileInfo]::new($Path).Length -gt 16777216) { throw 'World sequence source exceeds the Client 16 MiB admission limit' }
    $raw = [IO.File]::ReadAllText($Path, [Text.UTF8Encoding]::new($false, $true))
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "World sequence JSON parse failed: $Path" }
    $rootProperties = @('schema','formatVersion','areaId','revision','templates','instances')
    if ($document.formatVersion -eq 3) { $rootProperties += 'objectResources' }
    if ($document.formatVersion -eq 3 -and $null -ne $document.PSObject.Properties['objectFolders']) { $rootProperties += 'objectFolders' }
    Assert-ExactJsonProperties $document $rootProperties 'World sequence root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.world-sequences' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -notin @(2.0, 3.0) -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        [double]$document.revision -lt 1 -or [double]$document.revision -gt 4294967295 -or
        [double]$document.revision -ne [math]::Floor([double]$document.revision)) {
        throw "World sequence header is invalid: $Path"
    }
    if ($document.templates -isnot [System.Array] -or
        $document.instances -isnot [System.Array]) {
        throw "World sequence templates and instances must be arrays: $Path"
    }
    # WorldSequenceDocument.cpp 의 상한과 동일하게 검사한다.
    $templates = @($document.templates)
    $instances = @($document.instances)
    if ($templates.Count -gt 512 -or $instances.Count -gt 2048) {
        throw "World sequence document exceeds its limits: $Path"
    }
    $stableId = '^[A-Za-z0-9._-]{1,128}$'
    function Assert-SequenceVector($Value, [string]$Label, [bool]$Positive = $false) {
        if ($Value -isnot [System.Array] -or @($Value).Count -ne 3) { throw "$Label must contain three numbers" }
        foreach ($component in $Value) {
            if (-not (Test-JsonNumber $component) -or [math]::Abs([double]$component) -gt 100000 -or
                ($Positive -and [double]$component -lt 0.000001)) { throw "$Label component is invalid" }
        }
    }
    function Assert-SequenceAssetPath([string]$Value, [bool]$Model) {
        if ([string]::IsNullOrWhiteSpace($Value) -or $Value.Length -gt 1024 -or
            $Value.StartsWith('/') -or $Value.Contains(':') -or $Value.Contains('\') -or
            $Value -match '[\x00-\x1f\x7f]' -or @($Value.Split('/') | Where-Object { $_ -in @('', '.', '..') }).Count -gt 0 -or
            ($Model -and -not $Value.EndsWith('.wmodel', [StringComparison]::OrdinalIgnoreCase))) {
            throw "Invalid Resources-relative world object asset ID: $Value"
        }
    }
    $objectResources = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $hierarchy = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    if ($null -ne $document.PSObject.Properties['objectFolders']) {
        if ($document.formatVersion -ne 3 -or $document.objectFolders -isnot [System.Array] -or $document.objectFolders.Count -gt 2048) {
            throw 'World object folder list is invalid'
        }
        foreach ($folder in $document.objectFolders) {
            $fields = @('folderId','displayName')
            foreach ($optional in @('anchorKind','parentId')) {
                if ($null -ne $folder.PSObject.Properties[$optional]) { $fields += $optional }
            }
            Assert-ExactJsonProperties $folder $fields 'World object folder'
            if ($folder.folderId -isnot [string] -or $folder.folderId -cnotmatch $stableId -or
                $hierarchy.ContainsKey($folder.folderId) -or $folder.displayName -isnot [string] -or
                [Text.UTF8Encoding]::new($false, $true).GetByteCount($folder.displayName) -notin 1..128 -or
                $folder.displayName -match '[\x00-\x1f\x7f]') {
                throw 'Invalid or duplicate World object folder'
            }
            $anchor = 'WORLD'; $parent = ''
            if ($null -ne $folder.PSObject.Properties['anchorKind']) {
                if ($folder.anchorKind -isnot [string] -or $folder.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) {
                    throw 'World object folder anchor must be WORLD, PLAYER or BOSS'
                }
                $anchor = $folder.anchorKind
            }
            if ($null -ne $folder.PSObject.Properties['parentId']) {
                if ($folder.parentId -isnot [string] -or ($folder.parentId -cne '' -and $folder.parentId -cnotmatch $stableId)) {
                    throw 'World object folder parent must be a stable ID or empty'
                }
                $parent = $folder.parentId
            }
            $hierarchy.Add($folder.folderId, [pscustomobject]@{ Anchor = $anchor; Parent = $parent })
        }
    }
    $sequenceMaterialCatalogs = $null
    if ($document.formatVersion -eq 3) {
        if ($document.objectResources -isnot [System.Array] -or @($document.objectResources).Count -gt 2048) {
            throw 'World object resource list is invalid'
        }
        foreach ($resource in $document.objectResources) {
            $fields = @('objectId','displayName','modelAssetId','modelPreScale','animated','scale')
            foreach ($optional in @('diffuseTextureAssetId','sequenceInstanceId','anchorKind','anchorBossArchetypeId','anchorBone','defaultMotionInstanceId','materialProfile','materialSourceModelAssetId','mapMaterialBindings','motionInstanceIds','combatBody','animationSetAssetId','presentationBossArchetypeId','parentId')) {
                if ($null -ne $resource.PSObject.Properties[$optional]) { $fields += $optional }
            }
            Assert-ExactJsonProperties $resource $fields 'World object resource'
            if ($resource.objectId -isnot [string] -or $resource.objectId -cnotmatch $stableId -or
                $objectResources.ContainsKey($resource.objectId) -or $resource.displayName -isnot [string] -or
                [Text.Encoding]::UTF8.GetByteCount($resource.displayName) -notin 1..128 -or
                $resource.displayName -match '[\x00-\x1f\x7f]' -or
                $resource.modelAssetId -isnot [string] -or $resource.animated -isnot [bool] -or
                -not (Test-JsonNumber $resource.modelPreScale) -or
                [double]$resource.modelPreScale -lt 0.000001 -or [double]$resource.modelPreScale -gt 100000) {
                throw "Invalid world object resource: $($resource.objectId)"
            }
            Assert-SequenceVector $resource.scale 'World object scale' $true
            if ($null -ne $resource.PSObject.Properties['anchorKind'] -and $resource.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) {
                throw 'World object resource anchor must be WORLD, PLAYER or BOSS'
            }
            $resourceAnchor = 'WORLD'
            if ($null -ne $resource.PSObject.Properties['anchorKind']) { $resourceAnchor = [string]$resource.anchorKind }
            $parent = ''
            if ($null -ne $resource.PSObject.Properties['parentId']) {
                if ($resource.parentId -isnot [string] -or ($resource.parentId -cne '' -and $resource.parentId -cnotmatch $stableId)) {
                    throw 'World object parent must be a stable ID or empty'
                }
                $parent = $resource.parentId
            }
            if ($hierarchy.ContainsKey($resource.objectId)) { throw 'Duplicate World object hierarchy ID' }
            $hierarchy.Add($resource.objectId, [pscustomobject]@{ Anchor = $resourceAnchor; Parent = $parent })
            $anchorBossArchetypeId = ''
            $anchorBone = ''
            foreach ($field in @('anchorBossArchetypeId','anchorBone')) {
                if ($null -ne $resource.PSObject.Properties[$field] -and $resource.$field -isnot [string]) {
                    throw "World object $field must be text"
                }
            }
            if ($null -ne $resource.PSObject.Properties['anchorBossArchetypeId']) { $anchorBossArchetypeId = $resource.anchorBossArchetypeId }
            if ($null -ne $resource.PSObject.Properties['anchorBone']) { $anchorBone = $resource.anchorBone }
            if ([Text.UTF8Encoding]::new($false, $true).GetByteCount($anchorBone) -gt 128 -or
                $anchorBone -match '[\x00-\x1f\x7f]' -or
                ($resourceAnchor -ceq 'BOSS' -and $anchorBossArchetypeId -cnotmatch $stableId) -or
                ($resourceAnchor -cne 'BOSS' -and ($anchorBossArchetypeId -cne '' -or $anchorBone -cne ''))) {
                throw 'World object boss anchor requires a stable boss ID and bounded optional bone; other anchors cannot carry boss fields'
            }
            $alias = $null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -ne ''
            if ($null -ne $resource.PSObject.Properties['combatBody']) {
                $body = $resource.combatBody
                $bodyFields = @('maxHp','localCenterM','halfExtentsM','lifetimePolicy')
                if ($null -ne $body.PSObject.Properties['shape']) {
                    $bodyFields += 'shape'
                    if ($body.shape -isnot [string] -or $body.shape -cnotin @('BOX','ELLIPSOID')) { throw 'World Object combat shape must be BOX or ELLIPSOID' }
                }
                Assert-ExactJsonProperties $body $bodyFields 'World Object combat body'
                if ($alias -or $resourceAnchor -cne 'WORLD' -or
                    $null -ne $resource.PSObject.Properties['motionInstanceIds'] -or
                    -not (Test-JsonNumber $body.maxHp) -or [double]$body.maxHp -lt 1 -or [double]$body.maxHp -gt 1000000000 -or
                    [double]$body.maxHp -ne [math]::Floor([double]$body.maxHp) -or
                    $body.lifetimePolicy -isnot [string] -or $body.lifetimePolicy -cne 'UNTIL_DESTROYED') {
                    throw 'Combat body requires a WORLD model, bounded HP and UNTIL_DESTROYED lifetime'
                }
                Assert-SequenceVector $body.localCenterM 'World Object combat center'
                Assert-SequenceVector $body.halfExtentsM 'World Object combat half extents' $true
                foreach ($axis in $body.halfExtentsM) {
                    if ([double]$axis -lt 0.001 -or [double]$axis -gt 1000) { throw 'Combat half extents must be 0.001..1000 metres' }
                }
            }
            if ($null -ne $resource.PSObject.Properties['motionInstanceIds']) {
                $members = $resource.motionInstanceIds
                if ($members -isnot [System.Array] -or $members.Count -lt 1 -or $members.Count -gt 32 -or
                    $resourceAnchor -cne 'WORLD' -or $alias -or $resource.modelAssetId -cne '' -or $resource.animated -or
                    @($resource.scale | Where-Object { [double]$_ -ne 1.0 }).Count -ne 0) {
                    throw 'Object group requires 1..32 Map motion IDs, no model/alias, and unit scale'
                }
                foreach ($field in @('defaultMotionInstanceId','diffuseTextureAssetId')) {
                    if ($null -ne $resource.PSObject.Properties[$field] -and
                        ($resource.$field -isnot [string] -or $resource.$field -cne '')) { throw "Object group cannot carry $field" }
                }
                if ($null -ne $resource.PSObject.Properties['materialProfile'] -or
                    $null -ne $resource.PSObject.Properties['materialSourceModelAssetId'] -or
                    ($null -ne $resource.PSObject.Properties['mapMaterialBindings'] -and
                     ($resource.mapMaterialBindings -isnot [System.Array] -or $resource.mapMaterialBindings.Count -ne 0))) {
                    throw 'Object group cannot carry material input'
                }
                if ($null -ne $resource.PSObject.Properties['animationSetAssetId']) { throw 'Object group cannot carry an animation set' }
                if ($null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) { throw 'Object group cannot carry a presentation boss' }
                $memberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($id in $members) {
                    if ($id -isnot [string] -or $id -cnotmatch $stableId -or -not $memberIds.Add($id)) {
                        throw 'Object group needs unique stable motion IDs'
                    }
                }
                $objectResources[$resource.objectId] = $resource
                continue
            }
            if ($alias) {
                if ($resource.sequenceInstanceId -isnot [string] -or $resource.sequenceInstanceId -cnotmatch $stableId -or
                    ($null -ne $resource.PSObject.Properties['anchorKind'] -and $resource.anchorKind -cne 'WORLD') -or
                    $resource.modelAssetId -ne '' -or $resource.animated -or
                    ($null -ne $resource.PSObject.Properties['diffuseTextureAssetId'] -and $resource.diffuseTextureAssetId -ne '')) {
                    throw 'World object alias must refer only to an existing sequence instance'
                }
            } else {
                Assert-SequenceAssetPath $resource.modelAssetId $true
                if ($null -ne $resource.PSObject.Properties['diffuseTextureAssetId']) {
                    if ($resource.diffuseTextureAssetId -isnot [string]) { throw 'World object diffuse path must be a string' }
                    if ($resource.diffuseTextureAssetId -ne '') { Assert-SequenceAssetPath $resource.diffuseTextureAssetId $false }
                }
            }
            if ($null -ne $resource.PSObject.Properties['animationSetAssetId']) {
                # A separate AnimSet WModel only makes sense for a skinned body that plays clips.
                if ($alias -or -not $resource.animated -or $resource.animationSetAssetId -isnot [string] -or $resource.animationSetAssetId -eq '') {
                    throw "Invalid world object animation set: $($resource.objectId)"
                }
                Assert-SequenceAssetPath $resource.animationSetAssetId $true
            }
            if ($null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) {
                # The product boss assembly borrows this skinned body's bone palette.
                if ($alias -or -not $resource.animated -or $resource.presentationBossArchetypeId -isnot [string] -or
                    $resource.presentationBossArchetypeId -cnotmatch '^[A-Za-z0-9_.-]{1,128}$') {
                    throw "Invalid world object presentation boss: $($resource.objectId)"
                }
            }
            if ($null -ne $resource.PSObject.Properties['materialSourceModelAssetId']) {
                if ($alias -or $resource.materialSourceModelAssetId -isnot [string] -or $resource.materialSourceModelAssetId -eq '') { throw 'Invalid world object material source model' }
                Assert-SequenceAssetPath $resource.materialSourceModelAssetId $true
                $sourceNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.materialSourceModelAssetId)).Names
                $targetNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.modelAssetId)).Names
                if ($null -eq $sequenceMaterialCatalogs) {
                    $sequenceMaterialCatalogs = @{}
                    foreach ($catalogName in @('CharacterCatalog','BossCatalog')) {
                        $catalogPath = Join-Path $ProjectRoot "Data\Actors\$catalogName.json"
                        $sequenceMaterialCatalogs[$catalogName] = [IO.File]::ReadAllText($catalogPath, [Text.UTF8Encoding]::new($false, $true)) | ConvertFrom-Json
                    }
                }
                $sourceModel = $resource.materialSourceModelAssetId
                $owners = @($sequenceMaterialCatalogs.CharacterCatalog.characters | Where-Object {
                    $_.bodyModel -ceq $sourceModel -or @($_.equipmentModels) -ccontains $sourceModel -or @($_.weaponModels) -ccontains $sourceModel
                })
                if ($owners.Count -gt 1 -or ($owners.Count -eq 1 -and $owners[0].runtimeStatus -cne 'supported')) { throw "World Object material source ownership is invalid: $sourceModel" }
                $sourceRows = if ($owners.Count -eq 1) { @($owners[0].modelMaterialOverrides | Where-Object { $_.modelAssetId -ceq $sourceModel }) }
                    else { @($sequenceMaterialCatalogs.BossCatalog.modelMaterialOverrides | Where-Object { $_.modelAssetId -ceq $sourceModel }) }
                if (@($sourceRows).Count -eq 0) { throw "World Object original model has no catalog material overrides: $sourceModel" }
                $boundSourceNames = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($material in $sourceRows) {
                    $name = $material.materialName
                    if (-not $boundSourceNames.Add($name) -or -not $sourceNames.ContainsKey($name) -or -not $targetNames.ContainsKey($name)) { throw "Derived World Object lost original material slot: $name" }
                    foreach ($texture in @($material.textures)) {
                        Assert-SequenceAssetPath $texture.assetId $false
                        if (-not [IO.File]::Exists((Join-Path $runtimeResourceRoot $texture.assetId))) { throw "World Object original material texture is absent: $($texture.assetId)" }
                    }
                }
            }
            if ($null -ne $resource.PSObject.Properties['mapMaterialBindings']) {
                if ($alias -or $resource.mapMaterialBindings -isnot [array] -or $resource.mapMaterialBindings.Count -gt 64) { throw 'Invalid world object map material bindings' }
                $boundMaterials = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                if ($null -ne $resource.PSObject.Properties['materialProfile']) { [void]$boundMaterials.Add($resource.materialProfile.materialName) }
                $targetNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.modelAssetId)).Names
                if ($null -eq $cinematicMapMaterials) {
                    try { $cinematicMapMaterials = ([IO.File]::ReadAllText($authoringMaterialPath, [Text.UTF8Encoding]::new($false, $true)) | ConvertFrom-Json).materials }
                    catch { throw "World Object map material source JSON parse failed: $authoringMaterialPath" }
                }
                foreach ($binding in $resource.mapMaterialBindings) {
                    $bindingFields = @('materialName','sourceAssetId','sourceMaterialName')
                    if ($null -ne $binding.PSObject.Properties['diffuseTextureAssetId']) { $bindingFields += 'diffuseTextureAssetId'; Assert-SequenceAssetPath $binding.diffuseTextureAssetId $false }
                    if ($null -ne $binding.PSObject.Properties['unlit']) {
                        $bindingFields += 'unlit'
                        if ($binding.unlit -isnot [bool]) { throw 'World Object map material unlit flag must be boolean' }
                    }
                    Assert-ExactJsonProperties $binding $bindingFields 'World object map material binding'
                    $sourceRows = @($cinematicMapMaterials | Where-Object { $_.assetId -ceq $binding.sourceAssetId -and $_.materialName -ceq $binding.sourceMaterialName })
                    if (-not $targetNames.ContainsKey($binding.materialName) -or $sourceRows.Count -ne 1 -or $sourceRows[0].family -cne 'bg-source-opaque-masked') { throw 'World Object map material slot/source is absent or unsupported' }
                    if ($null -ne $binding.PSObject.Properties['diffuseTextureAssetId'] -and -not [IO.File]::Exists((Join-Path $runtimeResourceRoot $binding.diffuseTextureAssetId))) { throw 'World Object map surface texture is absent' }
                    if ($binding.sourceAssetId -isnot [string] -or $binding.sourceAssetId -cnotmatch $stableId -or -not $boundMaterials.Add($binding.materialName)) { throw 'Invalid or duplicate map material binding' }
                    foreach ($name in @($binding.materialName,$binding.sourceMaterialName)) {
                        if ($name -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($name) -notin 1..63 -or $name -match '[\x00-\x1f\x7f]') { throw 'Invalid map material binding name' }
                    }
                }
            }
            if ($null -ne $resource.PSObject.Properties['materialProfile']) {
                if ($alias) { throw 'World object sequence alias cannot own a material profile' }
                $material = $resource.materialProfile
                Assert-ExactJsonProperties $material @('materialName','sourceMaterial','family','parameters','textures') 'World object material'
                if ($material.materialName -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($material.materialName) -notin 1..63 -or
                    $material.sourceMaterial -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($material.sourceMaterial) -notin 1..512 -or
                    $material.materialName -match '[\x00-\x1f\x7f]' -or $material.sourceMaterial -match '[\x00-\x1f\x7f]' -or
                    $material.family -isnot [string] -or $material.family -cnotmatch '^source\.character\.[a-z0-9.-]+\.v1$') {
                    throw 'World object material identity or native family is invalid'
                }
                $nativeContract = Get-SequenceNativeMaterialContract $material.family
                $parameterNames = $nativeContract.ParameterNames
                Assert-ExactJsonProperties $material.parameters $parameterNames 'World object native material parameters'
                foreach ($name in $parameterNames) {
                    $value = $material.parameters.$name
                    if ($value -is [array]) {
                        if ($value.Count -ne 4) { throw "Invalid native material vector: $name" }
                        $values = $value
                    } else { $values = @($value) }
                    foreach ($component in $values) {
                        if (-not (Test-JsonNumber $component) -or [Math]::Abs([double]$component) -gt 1000000) {
                            throw "Invalid native material parameter: $name"
                        }
                    }
                }
                if ($material.textures -isnot [array] -or $material.textures.Count -gt 16) {
                    throw 'World object native material texture expression count is invalid'
                }
                $textureMask = 0
                foreach ($texture in $material.textures) {
                    Assert-ExactJsonProperties $texture @('expressionIndex','assetId','colorSpace') 'World object native material texture'
                    if (-not (Test-JsonNumber $texture.expressionIndex) -or [double]$texture.expressionIndex % 1 -ne 0 -or $texture.expressionIndex -notin 0..15 -or
                        $texture.assetId -isnot [string] -or $texture.colorSpace -cnotin @('srgb','linear')) {
                        throw 'Invalid world object native material texture'
                    }
                    $bit = 1 -shl [int]$texture.expressionIndex
                    if (($textureMask -band $bit) -ne 0) { throw 'Duplicate world object native texture expression' }
                    $textureMask = $textureMask -bor $bit
                    Assert-SequenceAssetPath $texture.assetId $false
                    if (-not [IO.File]::Exists((Join-Path $runtimeResourceRoot $texture.assetId))) {
                        throw "World object material texture is missing: $($texture.assetId)"
                    }
                }
                if ($textureMask -ne $nativeContract.TextureMask) { throw 'World object native texture coverage is incomplete' }
            }
            $objectResources[$resource.objectId] = $resource
        }
    }
    foreach ($entry in $hierarchy.GetEnumerator()) {
        $visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        [void]$visited.Add($entry.Key)
        $current = $entry.Value; $depth = 0
        while ($current.Parent -cne '') {
            if (-not $hierarchy.ContainsKey($current.Parent) -or $hierarchy[$current.Parent].Anchor -cne $entry.Value.Anchor) {
                throw 'World object parent must exist in the same anchor category'
            }
            $depth++
            if (-not $visited.Add($current.Parent) -or $depth -gt 64) {
                throw 'World object hierarchy contains a cycle or exceeds 64 parents'
            }
            $current = $hierarchy[$current.Parent]
        }
    }
    $trackCounts = @{}
    $templateRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $templateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($template in $templates) {
        $templateProperties = @('sequenceId','displayName','category','durationMs','interpolation','tracks','animationTracks')
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['objectMotion']) { $templateProperties += 'objectMotion' }
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['effectTracks']) { $templateProperties += 'effectTracks' }
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['colliderTracks']) { $templateProperties += 'colliderTracks' }
        foreach ($lane in @('soundTracks','subtitleTracks','materialTracks')) {
            if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties[$lane]) { $templateProperties += $lane }
        }
        Assert-ExactJsonProperties $template $templateProperties 'World sequence template'
        if ($template.sequenceId -isnot [string] -or
            $template.sequenceId -notmatch $stableId -or
            -not $templateIds.Add([string]$template.sequenceId) -or
            $template.displayName -isnot [string] -or
            $template.displayName.Length -lt 1 -or
            $template.displayName.Length -gt 128 -or
            $template.category -isnot [string] -or
            $template.category.Length -lt 1 -or $template.category.Length -gt 64 -or
            -not (Test-JsonNumber $template.durationMs) -or
            [double]$template.durationMs -le 0 -or
            [double]$template.durationMs -gt 600000 -or
            $template.interpolation -isnot [string] -or
            $template.interpolation -notin @('LINEAR','SMOOTH_STEP') -or
            $template.tracks -isnot [System.Array] -or
            $template.animationTracks -isnot [System.Array]) {
            throw "World sequence template is invalid: $($template.sequenceId)"
        }
        if ([double]$template.durationMs -ne [math]::Floor([double]$template.durationMs)) { throw 'World sequence duration must be integer milliseconds' }
        if ($null -ne $template.PSObject.Properties['objectMotion']) {
            $motion = $template.objectMotion
            $motionProperties = @('velocity','acceleration','angularVelocityDegrees','revolutionDegreesPerSecond','revolutionOffset','count','intervalMs','spreadDegrees','seed')
            if ($null -ne $motion.PSObject.Properties['spawnHalfExtents']) {
                $motionProperties += 'spawnHalfExtents'
                Assert-SequenceVector $motion.spawnHalfExtents 'World object spawn half extents'
                if (@($motion.spawnHalfExtents | Where-Object { [double]$_ -lt 0 }).Count -gt 0) {
                    throw 'World object spawn half extents must be nonnegative'
                }
            }
            if ($null -ne $motion.PSObject.Properties['emissions']) { $motionProperties += 'emissions' }
            Assert-ExactJsonProperties $motion $motionProperties 'World object motion'
            foreach ($field in @('velocity','acceleration','angularVelocityDegrees','revolutionDegreesPerSecond','revolutionOffset')) {
                Assert-SequenceVector $motion.$field "World object motion $field"
            }
            foreach ($field in @('count','intervalMs','seed')) {
                if (-not (Test-JsonNumber $motion.$field) -or [double]$motion.$field -lt 0 -or
                    [double]$motion.$field -gt 4294967295 -or [double]$motion.$field -ne [math]::Floor([double]$motion.$field)) { throw "Invalid object motion $field" }
            }
            $lastEmissionMs = ([double]$motion.count - 1) * [double]$motion.intervalMs
            if ($null -ne $motion.PSObject.Properties['emissions']) {
                # Authored rows own the count; interval and spread are the seeded emitter's and must stay zero.
                $emissions = @($motion.emissions)
                if ($motion.emissions -isnot [System.Array] -or $emissions.Count -lt 1 -or $emissions.Count -gt 128 -or
                    [double]$motion.count -ne $emissions.Count -or [double]$motion.intervalMs -ne 0 -or [double]$motion.spreadDegrees -ne 0) {
                    throw "World object emissions must be 1..128 rows with count equal to the row count and zero interval/spread: $($template.sequenceId)"
                }
                $lastEmissionMs = 0
                foreach ($emission in $emissions) {
                    Assert-ExactJsonProperties $emission @('positionOffset','yawDegrees','startDelayMs') 'World object emission'
                    Assert-SequenceVector $emission.positionOffset 'World object emission positionOffset'
                    if (-not (Test-JsonNumber $emission.yawDegrees) -or [double]$emission.yawDegrees -lt -36000 -or [double]$emission.yawDegrees -gt 36000 -or
                        -not (Test-JsonNumber $emission.startDelayMs) -or [double]$emission.startDelayMs -lt 0 -or [double]$emission.startDelayMs -gt 600000 -or
                        [double]$emission.startDelayMs -ne [math]::Floor([double]$emission.startDelayMs)) {
                        throw "Invalid World object emission row: $($template.sequenceId)"
                    }
                    if ([double]$emission.startDelayMs -gt $lastEmissionMs) { $lastEmissionMs = [double]$emission.startDelayMs }
                }
            }
            if ($motion.count -lt 1 -or $motion.count -gt 128 -or $motion.intervalMs -gt 600000 -or
                (($null -eq $template.PSObject.Properties['effectTracks'] -or @($template.effectTracks).Count -eq 0) -and
                    $lastEmissionMs -ge [double]$template.durationMs) -or
                -not (Test-JsonNumber $motion.spreadDegrees) -or $motion.spreadDegrees -lt 0 -or $motion.spreadDegrees -gt $(if ($null -ne $template.PSObject.Properties['effectTracks'] -and @($template.effectTracks).Count -gt 0) { 360 } else { 180 })) {
                throw 'World object spawn count, interval or spread exceeds its lifetime'
            }
        }
        $templateRows[[string]$template.sequenceId] = $template
        $tracks = @($template.tracks)
        $animationTracks = @($template.animationTracks)
        $total = $tracks.Count + $animationTracks.Count
        if ($total -lt 1 -or $total -gt 64) {
            throw "World sequence template track count is invalid: $($template.sequenceId)"
        }
        $slotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($track in $tracks) {
            Assert-ExactJsonProperties $track @('slotId','keys') 'World sequence track'
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                -not $slotIds.Add([string]$track.slotId) -or
                $track.keys -isnot [System.Array]) {
                throw "World sequence track is invalid: $($template.sequenceId)"
            }
            $keys = @($track.keys)
            if ($keys.Count -lt 2 -or $keys.Count -gt 4096) {
                throw "World sequence key count is invalid: $($template.sequenceId)/$($track.slotId)"
            }
            $previous = -1
            foreach ($key in $keys) {
                Assert-ExactJsonProperties $key `
                    @('timeMs','positionOffset','rotationQuaternion',
                      'scaleMultiplier','visible') 'World sequence key'
                if (-not (Test-JsonNumber $key.timeMs) -or
                    [double]$key.timeMs -le $previous -or
                    [double]$key.timeMs -gt [double]$template.durationMs -or
                    $key.visible -isnot [bool] -or
                    $key.positionOffset -isnot [System.Array] -or
                    @($key.positionOffset).Count -ne 3 -or
                    $key.rotationQuaternion -isnot [System.Array] -or
                    @($key.rotationQuaternion).Count -ne 4 -or
                    $key.scaleMultiplier -isnot [System.Array] -or
                    @($key.scaleMultiplier).Count -ne 3) {
                    throw "World sequence key is invalid: $($template.sequenceId)/$($track.slotId)"
                }
                foreach ($component in (@($key.positionOffset) +
                        @($key.rotationQuaternion) + @($key.scaleMultiplier))) {
                    if (-not (Test-JsonNumber $component)) {
                        throw "World sequence key component is not finite: $($template.sequenceId)/$($track.slotId)"
                    }
                }
                Assert-SequenceVector $key.positionOffset 'World sequence key position'
                Assert-SequenceVector $key.scaleMultiplier 'World sequence key scale'
                for ($axis = 0; $axis -lt 3; $axis++) {
                    $scale = [double]$key.scaleMultiplier[$axis]
                    if ([math]::Abs($scale) -lt 0.000001 -or
                        [math]::Sign($scale) -ne [math]::Sign([double]$keys[0].scaleMultiplier[$axis])) {
                        throw 'World sequence scale is singular or changes axis sign'
                    }
                }
                $quaternionLength = 0.0
                foreach ($component in $key.rotationQuaternion) { $quaternionLength += [double]$component * [double]$component }
                if ([math]::Abs([math]::Sqrt($quaternionLength) - 1.0) -gt 0.001 -or
                    [double]$key.timeMs -ne [math]::Floor([double]$key.timeMs)) { throw 'World sequence key time or quaternion is invalid' }
                $previous = [double]$key.timeMs
            }
            if ([double]$keys[0].timeMs -ne 0 -or
                [double]$keys[$keys.Count - 1].timeMs -ne [double]$template.durationMs) {
                throw "World sequence track must span the whole duration: $($template.sequenceId)/$($track.slotId)"
            }
        }
        # An animation slot may carry an ordered clip chain, checked against
        # that slot's previous start instead of a plain unique set. A Deploy
        # slot may also carry a transform track so one binding can walk an
        # animated prop while its clips play.
        $animationSlotStarts = @{}
        foreach ($track in $animationTracks) {
            $trackProperties = @('slotId','clipName','playbackRate','loop','holdLastFrame')
            if ($null -ne $track.PSObject.Properties['displayName']) {
                $trackProperties += 'displayName'
                if ($track.displayName -isnot [string] -or
                    ([Text.UTF8Encoding]::new($false, $true)).GetByteCount($track.displayName) -gt 128 -or
                    $track.displayName -match '[\x00-\x1f\x7f]') {
                    throw "World sequence animation track displayName is invalid: $($template.sequenceId)"
                }
            }
            if ($null -ne $track.PSObject.Properties['startMs']) {
                $trackProperties += 'startMs'
            }
            if ($null -ne $track.PSObject.Properties['sourceStartMs']) {
                $trackProperties += 'sourceStartMs'
                if (-not (Test-JsonNumber $track.sourceStartMs) -or
                    [double]$track.sourceStartMs -lt 0 -or [double]$track.sourceStartMs -gt 600000 -or
                    [double]$track.sourceStartMs -ne [math]::Floor([double]$track.sourceStartMs)) {
                    throw "World sequence animation source start is invalid: $($template.sequenceId)"
                }
            }
            if ($null -ne $track.PSObject.Properties['sourceEndMs']) {
                $trackProperties += 'sourceEndMs'
                $sourceStartMs = 0
                if ($null -ne $track.PSObject.Properties['sourceStartMs']) { $sourceStartMs = [double]$track.sourceStartMs }
                if (-not (Test-JsonNumber $track.sourceEndMs) -or
                    [double]$track.sourceEndMs -lt 0 -or [double]$track.sourceEndMs -gt 600000 -or
                    [double]$track.sourceEndMs -ne [math]::Floor([double]$track.sourceEndMs) -or
                    ([double]$track.sourceEndMs -ne 0 -and [double]$track.sourceEndMs -le $sourceStartMs)) {
                    throw "World sequence animation source end is invalid: $($template.sequenceId)"
                }
            }
            Assert-ExactJsonProperties $track $trackProperties `
                'World sequence animation track'
            $startMs = 0
            if ($null -ne $track.PSObject.Properties['startMs']) {
                if (-not (Test-JsonNumber $track.startMs) -or
                    [double]$track.startMs -lt 0 -or
                    [double]$track.startMs -ne [math]::Floor([double]$track.startMs) -or
                    [double]$track.startMs -ge [double]$template.durationMs) {
                    throw "World sequence animation track start is invalid: $($template.sequenceId)"
                }
                $startMs = [int][double]$track.startMs
            }
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                $track.clipName -isnot [string] -or
                $track.clipName.Length -lt 1 -or $track.clipName.Length -gt 128 -or
                -not (Test-JsonNumber $track.playbackRate) -or
                [double]$track.playbackRate -lt 0.05 -or
                [double]$track.playbackRate -gt 8.0 -or
                $track.loop -isnot [bool] -or
                $track.holdLastFrame -isnot [bool]) {
                throw "World sequence animation track is invalid: $($template.sequenceId)"
            }
            $slot = [string]$track.slotId
            if ($animationSlotStarts.ContainsKey($slot)) {
                if ($startMs -le $animationSlotStarts[$slot]) {
                    throw "World sequence animation chain must advance: $($template.sequenceId)/$slot"
                }
            } else {
                # A slot may also carry a transform track, so only a second
                # animation chain on the same slot is a conflict.
                [void]$slotIds.Add($slot)
            }
            $animationSlotStarts[$slot] = $startMs
        }
        if ($null -ne $template.PSObject.Properties['effectTracks']) {
            if ($template.effectTracks -isnot [System.Array] -or $total + @($template.effectTracks).Count -gt 64) {
                throw 'World Object effectTracks must be a bounded array'
            }
            $effectIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            $emissionMs = 0
            if ($null -ne $template.PSObject.Properties['objectMotion']) {
                $emissionMs = ($template.objectMotion.count - 1) * $template.objectMotion.intervalMs
                if ($null -ne $template.objectMotion.PSObject.Properties['emissions']) {
                    $emissionMs = 0
                    foreach ($emission in @($template.objectMotion.emissions)) {
                        if ([double]$emission.startDelayMs -gt $emissionMs) { $emissionMs = [double]$emission.startDelayMs }
                    }
                }
            }
            foreach ($effect in $template.effectTracks) {
                $effectProperties = @('effectTrackId','slotId','resourceKind','resourceId','timing','startMs','durationMs','positionOffset','rotationDegrees','scale')
                foreach ($optional in @('followObject','inheritObjectRotation','bone','fitEffectToDuration','loopEffectToDuration')) {
                    if ($null -ne $effect.PSObject.Properties[$optional]) { $effectProperties += $optional }
                }
                Assert-ExactJsonProperties $effect $effectProperties 'World Object effect track'
                if ($null -ne $effect.PSObject.Properties['fitEffectToDuration'] -and
                    ($effect.fitEffectToDuration -isnot [bool] -or
                    ($effect.fitEffectToDuration -and $effect.resourceKind -cne 'V1_EFFECT'))) {
                    throw 'World Object Effect fit must be boolean and true requires V1_EFFECT'
                }
                if ($null -ne $effect.PSObject.Properties['loopEffectToDuration'] -and
                    ($effect.loopEffectToDuration -isnot [bool] -or
                    ($effect.loopEffectToDuration -and $effect.resourceKind -cne 'V1_EFFECT'))) {
                    throw 'World Object Effect loop must be boolean and true requires V1_EFFECT'
                }
                if ($null -ne $effect.PSObject.Properties['fitEffectToDuration'] -and $effect.fitEffectToDuration -and
                    $null -ne $effect.PSObject.Properties['loopEffectToDuration'] -and $effect.loopEffectToDuration) {
                    throw 'World Object Effect fit and loop are mutually exclusive'
                }
                if (($null -ne $effect.PSObject.Properties['followObject'] -and $effect.followObject -isnot [bool]) -or
                    ($null -ne $effect.PSObject.Properties['inheritObjectRotation'] -and $effect.inheritObjectRotation -isnot [bool]) -or
                    ($null -ne $effect.PSObject.Properties['bone'] -and ($effect.bone -isnot [string] -or
                    [Text.Encoding]::UTF8.GetByteCount([string]$effect.bone) -gt 256 -or $effect.bone -match '[\x00-\x1f\x7f]'))) {
                    throw 'Invalid World Object effect followObject or bone'
                }
                if ($effect.effectTrackId -isnot [string] -or $effect.effectTrackId -notmatch $stableId -or
                    -not $effectIds.Add([string]$effect.effectTrackId) -or $effect.slotId -isnot [string] -or
                    -not $slotIds.Contains([string]$effect.slotId) -or $effect.resourceKind -cnotin @('LEAF','GROUP','V1_EFFECT') -or
                    $effect.resourceId -isnot [string] -or $effect.resourceId -notmatch $stableId -or
                    $effect.timing -cnotin @('TIME','MOTION_END')) {
                    throw 'Invalid World Object effect identity, resource kind or slot'
                }
                foreach ($field in @('startMs','durationMs')) {
                    if (-not (Test-JsonNumber $effect.$field) -or $effect.$field -lt 0 -or $effect.$field -gt 600000 -or
                        $effect.$field -ne [math]::Floor([double]$effect.$field)) { throw "Invalid World Object effect $field" }
                }
                $effectStart = if ($effect.timing -ceq 'MOTION_END') { $template.durationMs } else { $effect.startMs }
                if ($effect.startMs -gt $template.durationMs -or $effect.durationMs -lt 1 -or
                    ($effect.timing -ceq 'MOTION_END' -and $effect.startMs -ne 0) -or
                    [math]::Max($template.durationMs, $effectStart + $effect.durationMs) + $emissionMs -gt 600000) {
                    throw 'World Object effect exceeds its motion or presentation span'
                }
                Assert-SequenceVector $effect.positionOffset 'World Object effect position'
                Assert-SequenceVector $effect.rotationDegrees 'World Object effect rotation'
                Assert-SequenceVector $effect.scale 'World Object effect scale' $true
            }
        }
        if ($null -ne $template.PSObject.Properties['colliderTracks']) {
            if ($template.colliderTracks -isnot [System.Array] -or
                @($template.tracks).Count + @($template.animationTracks | Where-Object { $null -ne $_ }).Count + @($template.effectTracks | Where-Object { $null -ne $_ }).Count + @($template.colliderTracks).Count -gt 64) {
                throw 'World Object colliderTracks exceed the combined 64-track limit'
            }
            if (@($template.colliderTracks).Count -gt 0 -and ([double]$template.objectMotion.spreadDegrees -ne 0 -or @($template.objectMotion.spawnHalfExtents | Where-Object { $null -ne $_ -and [double]$_ -ne 0 }).Count)) { throw 'World Object collider publication requires zero random spread and spawn extents' }
            $colliderIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($row in $template.colliderTracks) {
                $fields = @('colliderTrackId','slotId','startMs','durationMs','positionOffset','halfExtents','yawDegrees','behavior','damagePercent','gripLocalOffset')
                if ($null -ne $row.PSObject.Properties['attachmentBone']) { $fields += 'attachmentBone' }
                if ($null -ne $row.PSObject.Properties['shape']) { $fields += 'shape' }
                Assert-ExactJsonProperties $row $fields 'World Object collider track'
                $colliderShape = 'BOX'
                if ($null -ne $row.PSObject.Properties['shape']) {
                    if ($row.shape -isnot [string] -or $row.shape -cnotin @('BOX','CYLINDER')) { throw 'Invalid World Object collider shape' }
                    $colliderShape = $row.shape
                }
                if ($row.colliderTrackId -isnot [string] -or $row.colliderTrackId -notmatch $stableId -or -not $colliderIds.Add($row.colliderTrackId) -or
                    $row.slotId -isnot [string] -or @($template.tracks | Where-Object { $_.slotId -ceq $row.slotId }).Count -ne 1 -or
                    $row.behavior -cnotin @('DAMAGE','INSTANT_DEATH','HOOK_CAPTURE')) { throw 'Invalid collider identity, transform slot or behavior' }
                foreach ($field in @('startMs','durationMs','damagePercent')) {
                    if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Collider $field must be an integer" }
                }
                if ($row.startMs -lt 0 -or $row.durationMs -lt 1 -or $row.startMs + $row.durationMs -gt $template.durationMs -or
                    $row.damagePercent -lt 0 -or $row.damagePercent -gt 100 -or
                    ($row.behavior -ceq 'DAMAGE' -and $row.damagePercent -lt 1) -or
                    ($row.behavior -cne 'DAMAGE' -and $row.damagePercent -ne 0) -or
                    -not (Test-JsonNumber $row.yawDegrees) -or [math]::Abs([double]$row.yawDegrees) -gt 36000) { throw 'Invalid collider clock, damage or yaw' }
                foreach ($field in @('positionOffset','halfExtents','gripLocalOffset')) {
                    Assert-SequenceVector $row.$field "Collider $field"
                    $maximum = if ($field -ceq 'halfExtents') { 1000 } else { 100000 }
                    if (@($row.$field | Where-Object { [math]::Abs([double]$_) -gt $maximum -or ($field -ceq 'halfExtents' -and [double]$_ -le 0.001) }).Count) { throw 'Invalid collider vector range' }
                }
                if ($colliderShape -ceq 'CYLINDER' -and ([math]::Abs([double]$row.halfExtents[0] - [double]$row.halfExtents[2]) -gt 0.0001 -or $row.behavior -ceq 'HOOK_CAPTURE')) {
                    throw 'Cylinder Collider requires equal X/Z radii and cannot capture a hook'
                }
                if ($row.behavior -cne 'HOOK_CAPTURE' -and (@($row.gripLocalOffset | Where-Object { [double]$_ -ne 0 }).Count -or -not [string]::IsNullOrEmpty([string]$row.attachmentBone))) { throw 'Only hook capture carries a grip or bone' }
                if ($null -ne $row.PSObject.Properties['attachmentBone'] -and ($row.attachmentBone -isnot [string] -or
                    [Text.Encoding]::UTF8.GetByteCount([string]$row.attachmentBone) -gt 256 -or $row.attachmentBone -match '[\x00-\x1f\x7f]')) { throw 'Invalid collider attachmentBone' }
            }
        }
        $totalTracks = 0
        foreach ($lane in @('tracks','animationTracks','effectTracks','colliderTracks','soundTracks','subtitleTracks','materialTracks')) {
            if ($null -ne $template.PSObject.Properties[$lane]) {
                if ($template.$lane -isnot [System.Array]) { throw "World $lane must be an array" }
                $totalTracks += @($template.$lane).Count
                if ($lane -in @('soundTracks','subtitleTracks','materialTracks')) {
                    for ($rowIndex = 0; $rowIndex -lt $template.$lane.Count; ++$rowIndex) {
                        if ($null -eq $template.$lane[$rowIndex]) { throw "World $lane cannot contain null rows" }
                    }
                }
            }
        }
        if ($totalTracks -gt 64) { throw 'World sequence exceeds the combined 64-track limit' }
        $soundIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.soundTracks | Where-Object { $null -ne $_ })) {
            $soundFields = @('soundTrackId','assetId','startMs','durationMs','volume')
            if ($row.PSObject.Properties['loopToDuration']) {
                if ($row.loopToDuration -isnot [bool]) { throw 'World sound loopToDuration must be boolean' }
                $soundFields += 'loopToDuration'
            }
            Assert-ExactJsonProperties $row $soundFields 'World sound track'
            if ($row.soundTrackId -isnot [string] -or $row.soundTrackId -cnotmatch $stableId -or -not $soundIds.Add($row.soundTrackId) -or
                $row.assetId -isnot [string] -or -not $row.assetId.StartsWith('Sound/', [StringComparison]::Ordinal) -or
                -not $row.assetId.EndsWith('.wav', [StringComparison]::Ordinal)) { throw 'Invalid World sound identity or asset ID' }
            Assert-SequenceAssetPath $row.assetId $false
            if ([Text.UTF8Encoding]::new($false, $true).GetByteCount($row.assetId) -gt 1024) { throw 'World sound asset ID exceeds its byte limit' }
            foreach ($field in @('startMs','durationMs')) {
                if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Sound $field must be integer milliseconds" }
            }
            if ($row.startMs -lt 0 -or $row.startMs -gt $template.durationMs -or $row.durationMs -lt 1 -or
                [double]$row.startMs + [double]$row.durationMs -gt 600000 -or
                -not (Test-JsonNumber $row.volume) -or $row.volume -lt 0 -or $row.volume -gt 4) { throw 'Invalid World sound time or volume' }
            if (-not (Test-Path -LiteralPath (Join-Path $runtimeResourceRoot $row.assetId) -PathType Leaf)) { throw "World sound asset is missing: $($row.assetId)" }
        }
        $subtitleIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.subtitleTracks | Where-Object { $null -ne $_ })) {
            Assert-ExactJsonProperties $row @('subtitleTrackId','stringId','text','position','slotId','startMs','durationMs') 'World subtitle track'
            foreach ($field in @('subtitleTrackId','stringId','text','position','slotId')) {
                if ($row.$field -isnot [string]) { throw "Subtitle $field must be text" }
            }
            if ($row.subtitleTrackId -cnotmatch $stableId -or -not $subtitleIds.Add($row.subtitleTrackId) -or $row.stringId -cnotmatch $stableId -or
                [Text.UTF8Encoding]::new($false, $true).GetByteCount($row.text) -notin 1..4096 -or
                $row.text -match '[\x00-\x09\x0b-\x1f\x7f<>]' -or $row.position -cnotin @('NORMAL','UPPER','BALLOON')) { throw 'Invalid World subtitle identity or plain UTF-8 text' }
            if ($row.position -ceq 'BALLOON') {
                if ($row.slotId -cnotmatch $stableId -or @($template.tracks | Where-Object { $_.slotId -ceq $row.slotId }).Count -ne 1) { throw 'Balloon subtitle requires an existing transform slot' }
            } elseif ($row.slotId -cne '') { throw 'Screen subtitle cannot bind an Object slot' }
            foreach ($field in @('startMs','durationMs')) {
                if (-not (Test-JsonNumber $row.$field) -or [double]$row.$field -ne [math]::Floor([double]$row.$field)) { throw "Subtitle $field must be integer milliseconds" }
            }
            if ($row.startMs -lt 0 -or $row.durationMs -lt 1 -or [double]$row.startMs + [double]$row.durationMs -gt $template.durationMs) { throw 'World subtitle exceeds its template clock' }
        }
        $materialTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($row in @($template.materialTracks | Where-Object { $null -ne $_ })) {
            Assert-ExactJsonProperties $row @('slotId','materialName','curves') 'World material track'
            if ($row.slotId -isnot [string] -or -not $slotIds.Contains($row.slotId) -or
                $row.materialName -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($row.materialName) -notin 1..256 -or
                $row.materialName -match '[\x00-\x1f\x7f]' -or -not $materialTargets.Add("$($row.slotId):$($row.materialName)") -or
                $row.curves -isnot [System.Array] -or $row.curves.Count -notin 1..64) { throw 'Invalid World material target or curves' }
            $parameters = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($curve in $row.curves) {
                Assert-ExactJsonProperties $curve @('parameter','keys') 'World material curve'
                if ($curve.parameter -isnot [string] -or [Text.Encoding]::UTF8.GetByteCount($curve.parameter) -notin 1..128 -or
                    $curve.parameter -match '[\x00-\x1f\x7f]' -or -not $parameters.Add($curve.parameter) -or
                    $curve.keys -isnot [System.Array] -or $curve.keys.Count -notin 1..4096) { throw 'Invalid World material curve' }
                $previous = -1
                foreach ($key in $curve.keys) {
                    Assert-ExactJsonProperties $key @('timeMs','value','interpolation') 'World material key'
                    if (-not (Test-JsonNumber $key.timeMs) -or $key.timeMs -ne [math]::Floor([double]$key.timeMs) -or
                        $key.timeMs -le $previous -or $key.timeMs -gt $template.durationMs -or
                        $key.value -isnot [System.Array] -or $key.value.Count -ne 4 -or
                        $key.interpolation -cnotin @('LINEAR','CONSTANT')) { throw 'Invalid World material key time or interpolation' }
                    foreach ($value in $key.value) {
                        if (-not (Test-JsonNumber $value) -or [math]::Abs([double]$value) -gt 1000000) { throw 'Invalid World material key value' }
                    }
                    $previous = $key.timeMs
                }
                if ($curve.keys[0].timeMs -ne 0 -or $curve.keys[-1].timeMs -ne $template.durationMs) { throw 'World material curve must cover its motion' }
            }
        }
        # One binding per slot, so a chained slot and a slot that also carries
        # a transform track each still count once.
        $trackCounts[[string]$template.sequenceId] = $slotIds.Count
    }
    $instanceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $instanceRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($instance in $instances) {
        $instanceProperties = @('instanceId','templateId','enabled','startDelayMs','playbackSpeed','bindings')
        foreach ($optional in @('anchorKind','position','motionEnd','nextMotionId','walkableSurface','loopFullPresentation')) {
            if ($document.formatVersion -eq 3 -and $null -ne $instance.PSObject.Properties[$optional]) { $instanceProperties += $optional }
        }
        Assert-ExactJsonProperties $instance $instanceProperties 'World sequence instance'
        if ($null -ne $instance.PSObject.Properties['anchorKind'] -and $instance.anchorKind -cnotin @('WORLD','PLAYER','BOSS')) { throw 'Invalid world object anchor' }
        $instanceAnchor = 'WORLD'
        if ($null -ne $instance.PSObject.Properties['anchorKind']) { $instanceAnchor = [string]$instance.anchorKind }
        if ($null -ne $instance.PSObject.Properties['position']) { Assert-SequenceVector $instance.position 'World object instance position' }
        if ($instance.instanceId -isnot [string] -or
            $instance.instanceId -notmatch $stableId -or
            -not $instanceIds.Add([string]$instance.instanceId) -or
            $instance.templateId -isnot [string] -or
            -not $trackCounts.ContainsKey([string]$instance.templateId) -or
            $instance.enabled -isnot [bool] -or
            -not (Test-JsonNumber $instance.startDelayMs) -or
            [double]$instance.startDelayMs -lt 0 -or
            [double]$instance.startDelayMs -gt 600000 -or
            -not (Test-JsonNumber $instance.playbackSpeed) -or
            [double]$instance.playbackSpeed -lt 0.05 -or
            [double]$instance.playbackSpeed -gt 8.0 -or
            $instance.bindings -isnot [System.Array]) {
            throw "World sequence instance is invalid: $($instance.instanceId)"
        }
        if ([double]$instance.startDelayMs -ne [math]::Floor([double]$instance.startDelayMs)) { throw 'World sequence start delay must be integer milliseconds' }
        $bindings = @($instance.bindings)
        if ($bindings.Count -ne $trackCounts[[string]$instance.templateId]) {
            throw "World sequence instance binding count does not match its template: $($instance.instanceId)"
        }
        if ($instanceAnchor -ceq 'BOSS' -and ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')) {
            throw 'Boss-anchored world sequence requires exactly one Object Resource binding'
        }
        $motionEnd = 'STOP'
        if ($null -ne $instance.PSObject.Properties['motionEnd']) {
            if ($instance.motionEnd -isnot [string] -or $instance.motionEnd -cnotin @('STOP','HOLD','LOOP','NEXT')) { throw 'Invalid world object motion completion' }
            $motionEnd = [string]$instance.motionEnd
        }
        if ($null -ne $instance.PSObject.Properties['loopFullPresentation'] -and
            ($instance.loopFullPresentation -isnot [bool] -or
            ($instance.loopFullPresentation -and ($motionEnd -cne 'LOOP' -or
             $bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')))) {
            throw 'Full presentation loop requires a boolean and one looping Object Resource'
        }
        $nextMotionId = ''
        if ($null -ne $instance.PSObject.Properties['nextMotionId']) {
            if ($instance.nextMotionId -isnot [string]) { throw 'Invalid world object next motion ID' }
            $nextMotionId = [string]$instance.nextMotionId
        }
        if (($motionEnd -eq 'NEXT' -and $nextMotionId -cnotmatch $stableId) -or
            ($motionEnd -ne 'NEXT' -and $nextMotionId -ne '') -or
            ($motionEnd -ne 'STOP' -and ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE'))) {
            throw "Invalid world object motion completion: $($instance.instanceId)"
        }
        if ($null -ne $instance.PSObject.Properties['walkableSurface']) {
            $surface = $instance.walkableSurface
            Assert-ExactJsonProperties $surface @('radiusM','localHeightM') 'Walkable surface'
            foreach ($field in @('radiusM','localHeightM')) {
                if ($surface.$field -isnot [ValueType] -or $surface.$field -is [bool] -or
                    [double]::IsNaN([double]$surface.$field) -or [double]::IsInfinity([double]$surface.$field)) {
                    throw "Walkable surface $field must be finite"
                }
            }
            $surfaceTemplate = $templateRows[$instance.templateId]
            if ([double]$surface.radiusM -lt 0.001 -or [double]$surface.radiusM -gt 1000 -or
                [math]::Abs([double]$surface.localHeightM) -gt 10000 -or $bindings.Count -ne 1 -or
                $bindings[0].targetKind -cne 'MAP_PLACEMENT' -or $motionEnd -cne 'STOP' -or
                @($surfaceTemplate.tracks).Count -ne 1 -or @($surfaceTemplate.animationTracks).Count -ne 0) {
                throw "Walkable surface requires one fixed Map placement: $($instance.instanceId)"
            }
            $first = $surfaceTemplate.tracks[0].keys[0]
            foreach ($key in $surfaceTemplate.tracks[0].keys) {
                if ([math]::Abs([double]$key.rotationQuaternion[0]) -gt 0.00001 -or
                    [math]::Abs([double]$key.rotationQuaternion[2]) -gt 0.00001 -or
                    [double]$key.scaleMultiplier[0] -le 0 -or [double]$key.scaleMultiplier[1] -le 0 -or
                    [math]::Abs([double]$key.scaleMultiplier[0] - [double]$key.scaleMultiplier[2]) -gt 0.00001) {
                    throw "Walkable surface must be horizontal with uniform X/Z scale: $($instance.instanceId)"
                }
                foreach ($axis in 0..2) {
                    if ($key.positionOffset[$axis] -ne $first.positionOffset[$axis] -or
                        $key.scaleMultiplier[$axis] -ne $first.scaleMultiplier[$axis]) {
                        throw "Walkable surface position and scale must stay fixed: $($instance.instanceId)"
                    }
                }
            }
        }
        $instanceRows.Add([string]$instance.instanceId, $instance)
        $boundSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $boundTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $template = $templateRows[[string]$instance.templateId]
        foreach ($subtitle in @($template.subtitleTracks | Where-Object { $null -ne $_ -and $_.position -ceq 'BALLOON' })) {
            if (@($bindings | Where-Object { $_.slotId -ceq $subtitle.slotId -and $_.targetKind -ceq 'OBJECT_RESOURCE' }).Count -ne 1) {
                throw "World balloon subtitle requires its Object Resource binding: $($instance.instanceId)"
            }
        }
        if ($null -ne $template.PSObject.Properties['effectTracks'] -and @($template.effectTracks).Count -gt 0 -and
            ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')) {
            throw 'World Object effect lanes require one Object Resource binding'
        }
        foreach ($binding in $bindings) {
            $bindingFields = @('slotId','targetKind','targetId')
            if ($null -ne $binding.PSObject.Properties['previewNpcPlacementId']) { $bindingFields += 'previewNpcPlacementId' }
            Assert-ExactJsonProperties $binding $bindingFields 'World sequence binding'
            if ($null -ne $binding.PSObject.Properties['previewNpcPlacementId']) {
                $resource = $objectResources[$binding.targetId]
                if ($binding.targetKind -cne 'OBJECT_RESOURCE' -or $null -eq $resource -or -not $resource.animated -or
                    $binding.previewNpcPlacementId -isnot [string] -or $binding.previewNpcPlacementId -cnotmatch $stableId -or
                    $bindings.Count -ne 1 -or $instanceAnchor -cne 'WORLD' -or $motionEnd -cne 'STOP' -or
                    -not [string]::IsNullOrEmpty($resource.sequenceInstanceId) -or
                    ($null -ne $template.objectMotion -and $template.objectMotion.count -ne 1) -or
                    @($template.colliderTracks | Where-Object { $null -ne $_ }).Count -ne 0 -or
                    $null -ne $resource.PSObject.Properties['combatBody'] -or
                    $null -ne $resource.PSObject.Properties['presentationBossArchetypeId']) {
                    throw "Invalid editor NPC replacement: $($instance.instanceId)"
                }
                $world = Get-Content -LiteralPath (Join-Path $ProjectRoot "Data/Worlds/$AreaId/Gameplay.world.json") -Raw -Encoding UTF8 | ConvertFrom-Json
                $npc = @($world.placements | Where-Object { $_.placementId -ceq $binding.previewNpcPlacementId -and $_.kind -ceq 'npc' -and $_.enabled })
                $npcCatalog = Get-Content -LiteralPath (Join-Path $ProjectRoot 'Data/Actors/NpcCatalog.json') -Raw -Encoding UTF8 | ConvertFrom-Json
                $actor = @($npcCatalog.npcs | Where-Object { $npc.Count -eq 1 -and $_.archetypeId -ceq $npc[0].archetypeId })
                if ($npc.Count -ne 1 -or $actor.Count -ne 1 -or $actor[0].modelAssetId -cne $resource.modelAssetId) {
                    throw "NPC preview must match one enabled placement and its exact model: $($instance.instanceId)"
                }
            }
            if ($binding.slotId -isnot [string] -or
                -not $boundSlots.Add([string]$binding.slotId) -or
                $binding.targetKind -isnot [string] -or
                $binding.targetKind -cnotin @('MAP_PLACEMENT','DEPLOY_PLACEMENT','OBJECT_RESOURCE') -or
                $binding.targetId -isnot [string] -or
                ($binding.targetKind -eq 'OBJECT_RESOURCE' -and $binding.targetId -cnotmatch $stableId) -or
                ($binding.targetKind -ne 'OBJECT_RESOURCE' -and $binding.targetId -notmatch '^[0-9]{1,20}$') -or
                -not $boundTargets.Add("$($binding.targetKind):$($binding.targetId)")) {
                throw "World sequence binding is invalid: $($instance.instanceId)"
            }
            $template = $templateRows[$instance.templateId]
            $transformTracks = @($template.tracks | Where-Object { $_.slotId -ceq $binding.slotId })
            $animationTracks = @($template.animationTracks | Where-Object { $_.slotId -ceq $binding.slotId })
            foreach ($material in @($template.materialTracks | Where-Object { $null -ne $_ -and $_.slotId -ceq $binding.slotId })) {
                if ($binding.targetKind -cne 'OBJECT_RESOURCE' -or -not $objectResources.ContainsKey($binding.targetId)) { throw 'World material track requires an Object Resource' }
                $profile = $objectResources[$binding.targetId].materialProfile
                if ($null -eq $profile -or $profile.materialName -cne $material.materialName) { throw 'World material track requires its exact Object material profile' }
                foreach ($curve in $material.curves) {
                    if ($null -eq $profile.parameters.PSObject.Properties[$curve.parameter]) { throw "World material parameter is absent: $($curve.parameter)" }
                }
            }
            if ($binding.targetKind -eq 'OBJECT_RESOURCE') {
                if (-not $objectResources.ContainsKey($binding.targetId)) { throw 'Unknown world object binding resource' }
                $resource = $objectResources[$binding.targetId]
                $resourceAnchor = 'WORLD'
                if ($null -ne $resource.PSObject.Properties['anchorKind']) { $resourceAnchor = [string]$resource.anchorKind }
                if (($instanceAnchor -ceq 'BOSS') -ne ($resourceAnchor -ceq 'BOSS')) {
                    throw 'Boss-anchored world sequence and Object Resource anchors must match'
                }
                if ($resource.modelAssetId -eq '' -or ($animationTracks.Count -gt 0 -and -not $resource.animated) -or
                    ($transformTracks.Count -eq 0 -and $animationTracks.Count -eq 0)) { throw 'Invalid object resource slot or animation binding' }
            } else {
                foreach ($track in $transformTracks) {
                    if (@($track.keys[0].scaleMultiplier | Where-Object { [double]$_ -lt 0 }).Count -gt 0) {
                        throw 'Signed scale requires an Object Resource binding'
                    }
                }
                $numericTarget = [uint64]0
                if (-not [uint64]::TryParse($binding.targetId, [ref]$numericTarget) -or $numericTarget -eq 0) { throw 'Placed sequence target ID is outside uint64 range' }
                if (($null -ne $instance.PSObject.Properties['anchorKind'] -and $instance.anchorKind -ne 'WORLD') -or
                    ($null -ne $instance.PSObject.Properties['position'] -and @($instance.position | Where-Object { $_ -ne 0 }).Count -gt 0)) {
                    throw 'Placed sequences cannot use object instance anchors'
                }
                if (($binding.targetKind -eq 'MAP_PLACEMENT' -and ($transformTracks.Count -ne 1 -or $animationTracks.Count -gt 0)) -or
                    ($binding.targetKind -eq 'DEPLOY_PLACEMENT' -and $animationTracks.Count -eq 0)) { throw 'Invalid placed sequence slot binding' }
            }
        }
    }
    foreach ($instance in $instances) {
        if ($null -eq $instance.PSObject.Properties['motionEnd'] -or $instance.motionEnd -cne 'NEXT') { continue }
        if (-not $instanceRows.ContainsKey($instance.nextMotionId)) { throw "Unknown NEXT motion: $($instance.instanceId)" }
        $target = $instanceRows[$instance.nextMotionId]
        if (-not $target.enabled -or @($target.bindings).Count -ne 1 -or
            $target.bindings[0].targetKind -cne 'OBJECT_RESOURCE' -or
            $target.bindings[0].targetId -cne $instance.bindings[0].targetId -or
            $target.bindings[0].slotId -cne $instance.bindings[0].slotId) {
            throw "NEXT motion must target an enabled object state with the same resource and slot: $($instance.instanceId)"
        }
        foreach ($state in @($instance, $target)) {
            $stateTemplate = $templateRows[$state.templateId]
            if ($null -ne $stateTemplate.PSObject.Properties['objectMotion'] -and $stateTemplate.objectMotion.count -ne 1) {
                throw "NEXT motion requires a single object emission: $($instance.instanceId)"
            }
        }
    }
    foreach ($instance in $instances) {
        $visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $current = $instance
        $depth = 0
        while ($null -ne $current.PSObject.Properties['motionEnd'] -and $current.motionEnd -ceq 'NEXT') {
            $depth += 1
            if (-not $visited.Add([string]$current.instanceId) -or $depth -gt 32) {
                throw "World object NEXT motion chain contains a cycle or exceeds 32 links: $($instance.instanceId)"
            }
            $current = $instanceRows[$current.nextMotionId]
        }
    }
    foreach ($resource in $objectResources.Values) {
        if ($null -ne $resource.PSObject.Properties['motionInstanceIds']) {
            foreach ($id in $resource.motionInstanceIds) {
                if (-not $instanceRows.ContainsKey($id)) { throw "Unknown Object group motion: $id" }
                $member = $instanceRows[$id]
                if (($null -ne $member.PSObject.Properties['anchorKind'] -and $member.anchorKind -cne 'WORLD') -or
                    ($null -ne $member.PSObject.Properties['motionEnd'] -and $member.motionEnd -cnotin @('STOP', 'LOOP')) -or
                    @($member.bindings).Count -ne 1 -or $member.bindings[0].targetKind -cne 'OBJECT_RESOURCE') {
                    throw 'Object group member must be one Map Object motion ending with Stop or Loop'
                }
                $model = $objectResources[$member.bindings[0].targetId]
                if ($null -eq $model -or $model.modelAssetId -ceq '' -or $null -ne $model.PSObject.Properties['motionInstanceIds']) {
                    throw 'Object group member must bind a model, not a group'
                }
            }
            continue
        }
        if ($null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -ne '' -and
            -not $instanceIds.Contains($resource.sequenceInstanceId)) { throw 'Unknown world object sequence alias' }
        if ($null -eq $resource.PSObject.Properties['defaultMotionInstanceId']) { continue }
        $defaultId = $resource.defaultMotionInstanceId
        if ($defaultId -isnot [string]) { throw 'Default Motion instance ID must be text' }
        if ($defaultId -ceq '') { continue }
        if ($defaultId -cnotmatch $stableId -or -not $instanceRows.ContainsKey($defaultId)) { throw 'Default Motion instance does not exist' }
        $motion = $instanceRows[$defaultId]
        $alias = $null -ne $resource.PSObject.Properties['sequenceInstanceId'] -and $resource.sequenceInstanceId -cne ''
        if (-not $motion.enabled -or ($alias -and $defaultId -cne $resource.sequenceInstanceId) -or
            (-not $alias -and (@($motion.bindings).Count -ne 1 -or $motion.bindings[0].targetKind -cne 'OBJECT_RESOURCE' -or
                              $motion.bindings[0].targetId -cne $resource.objectId))) {
            throw 'Default Motion must be an enabled instance of the same Object'
        }
    }
    # Publish the exact snapshot just validated, even if another editor saves
    # the source while this asynchronous process is checking it.
    $reader = [IO.StringReader]::new($raw)
    $validatedLines = [Collections.Generic.List[string]]::new()
    try {
        while ($null -ne ($line = $reader.ReadLine())) { $validatedLines.Add($line) }
    } finally { $reader.Dispose() }
    return $validatedLines.ToArray()
}
```

## 저장·게시 JSON 변경 행 · Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.camerashots.json

revision: 3 → 4

### shots / maharaka.waterpang.source.intro15.camera.1

```json
{
  "shotId": "maharaka.waterpang.source.intro15.camera.1",
  "displayName": "워터팡 / 영상 맞춤 카메라 0",
  "defaultHoldMs": 4784,
  "transitionEasing": "LINEAR",
  "activation": "PATTERN_ONLY",
  "sequenceInstanceId": "",
  "box": {
    "center": [
      53.045359675341075,
      35.44,
      -984.3777417407425
    ],
    "halfExtents": [
      1,
      1,
      1
    ],
    "yawDegrees": 0
  },
  "eye": [
    53.045359675341075,
    35.44,
    -984.3777417407425
  ],
  "lookAt": [
    61.2497257660573,
    29.722749654568027,
    -984.4154980271553
  ],
  "fovYDegrees": 29.39495760413827,
  "blendInMs": 0,
  "blendOutMs": 0,
  "priority": 100,
  "cameraTrack": {
    "durationMs": 4784,
    "interpolation": "LINEAR",
    "easing": "LINEAR",
    "keyframes": [
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k0",
        "timeMs": 0,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k50",
        "timeMs": 50,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.39762847567784
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k100",
        "timeMs": 100,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.4055206796816
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k150",
        "timeMs": 150,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.418454064198443
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k200",
        "timeMs": 200,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.43624903666468
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k250",
        "timeMs": 250,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.45872649241747
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k300",
        "timeMs": 300,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.485707746169542
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k350",
        "timeMs": 350,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.517014466321257
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k400",
        "timeMs": 400,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.55246861200007
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k450",
        "timeMs": 450,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.59189237272997
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k500",
        "timeMs": 500,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.635108110646474
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k550",
        "timeMs": 550,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.681938305183834
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k600",
        "timeMs": 600,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.732205500173038
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k650",
        "timeMs": 650,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.78573225329943
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k700",
        "timeMs": 700,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.842341087879447
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k750",
        "timeMs": 750,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.901854446925004
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k800",
        "timeMs": 800,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 29.964094649474106
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k850",
        "timeMs": 850,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 30.0288838491741
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k900",
        "timeMs": 900,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 30.096043995112943
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k950",
        "timeMs": 950,
        "eye": [
          53.045359675341075,
          35.44,
          -984.3777417407425
        ],
        "lookAt": [
          61.2497257660573,
          29.722749654568027,
          -984.4154980271553
        ],
        "up": [
          0.5716970700236409,
          0.8202821674693244,
          0.01730970416302795
        ],
        "fovYDegrees": 30.165396794900907
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1000",
        "timeMs": 1000,
        "eye": [
          53.045090028507815,
          35.44004096051621,
          -984.3777416038927
        ],
        "lookAt": [
          61.24948148088599,
          29.722827019422063,
          -984.415499356775
        ],
        "up": [
          0.5716934363690077,
          0.8202847114048016,
          0.017309160815499165
        ],
        "fovYDegrees": 30.2367636800132
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1050",
        "timeMs": 1050,
        "eye": [
          53.011960161697516,
          35.44507551796634,
          -984.3777249197843
        ],
        "lookAt": [
          61.21946733592371,
          29.732336525938628,
          -984.4156616301292
        ],
        "up": [
          0.5712467660491456,
          0.8205972295532337,
          0.01724294429470909
        ],
        "fovYDegrees": 30.30996577340992
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1100",
        "timeMs": 1100,
        "eye": [
          52.92662224831608,
          35.45805125117352,
          -984.3776824281014
        ],
        "lookAt": [
          61.142147251482236,
          29.7568518276833,
          -984.4160758643435
        ],
        "up": [
          0.5700948883711556,
          0.8214014130999104,
          0.017074449055537018
        ],
        "fovYDegrees": 30.384823859456592
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1150",
        "timeMs": 1150,
        "eye": [
          52.79354657562409,
          35.478299010150785,
          -984.3776170444381
        ],
        "lookAt": [
          61.02154831308368,
          29.795125713733377,
          -984.416715525867
        ],
        "up": [
          0.5682954135377674,
          0.8226527582750985,
          0.016815536099403477
        ],
        "fovYDegrees": 30.46115835617439
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1200",
        "timeMs": 1200,
        "eye": [
          52.61720343088173,
          35.50514964491114,
          -984.3775316843884
        ],
        "lookAt": [
          60.86168648836438,
          29.84591848790149,
          -984.4175545080368
        ],
        "up": [
          0.5659051823155471,
          0.8243056444526186,
          0.016478141641605513
        ],
        "fovYDegrees": 30.538789289854176
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1250",
        "timeMs": 1250,
        "eye": [
          52.40206310134944,
          35.53793400546761,
          -984.3774292635462
        ],
        "lookAt": [
          60.66657104599424,
          29.907994740330583,
          -984.4185668834507
        ],
        "up": [
          0.56298060077174,
          0.8263137793705866,
          0.016074239546188276
        ],
        "fovYDegrees": 30.61753627207359
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1300",
        "timeMs": 1300,
        "eye": [
          52.15259587428763,
          35.57598294183321,
          -984.3773126975058
        ],
        "lookAt": [
          60.44020855168244,
          29.980120437888537,
          -984.419726686128
        ],
        "up": [
          0.559577941493775,
          0.8286306016183659,
          0.015615807861726911
        ],
        "fovYDegrees": 30.69721847916084
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1350",
        "timeMs": 1350,
        "eye": [
          51.87327203695641,
          35.61862730402097,
          -984.3771849018608
        ],
        "lookAt": [
          60.18660645535417,
          30.0610603536375,
          -984.4210077256926
        ],
        "up": [
          0.5557536092047332,
          0.8312096418459051,
          0.015114799314415528
        ],
        "fovYDegrees": 30.777654634152565
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1400",
        "timeMs": 1400,
        "eye": [
          51.568561876616364,
          35.665197942043896,
          -984.3770487922055
        ],
        "lookAt": [
          59.90977627780488,
          30.149575848065325,
          -984.4223834348171
        ],
        "up": [
          0.5515643694793466,
          0.8340048435456731,
          0.01458311568933157
        ],
        "fovYDegrees": 30.858662991296796
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1450",
        "timeMs": 1450,
        "eye": [
          51.242935680527765,
          35.715025705915004,
          -984.3769072841338
        ],
        "lookAt": [
          59.61373640002515,
          30.24442300802531,
          -984.4238267502868
        ],
        "up": [
          0.5470675399672918,
          0.8369708437225418,
          0.014032586094950434
        ],
        "fovYDegrees": 30.940061323154715
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1500",
        "timeMs": 1500,
        "eye": [
          50.900863735950956,
          35.76744144564732,
          -984.3767632932398
        ],
        "lookAt": [
          59.30251445400924,
          30.344351143396793,
          -984.4253100272905
        ],
        "up": [
          0.5423211441478142,
          0.8400632133091923,
          0.013474949159100122
        ],
        "fovYDegrees": 31.02166691035747
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1550",
        "timeMs": 1550,
        "eye": [
          50.54681633014621,
          35.82177601125386,
          -984.3766197351175
        ],
        "lookAt": [
          58.98014931022769,
          30.448101636333654,
          -984.4268049858982
        ],
        "up": [
          0.5373840281740992,
          0.8432386568071197,
          0.01292183924615663
        ],
        "fovYDegrees": 31.103296534076136
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1600",
        "timeMs": 1600,
        "eye": [
          50.185263750374105,
          35.87736025274764,
          -984.376479525361
        ],
        "lookAt": [
          58.65069265408394,
          30.554407133608063,
          -984.4282826881639
        ],
        "up": [
          0.5323159418174351,
          0.846455170337006,
          0.012384776815400364
        ],
        "fovYDegrees": 31.184766471264293
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1650",
        "timeMs": 1650,
        "eye": [
          49.820676283894656,
          35.93352502014168,
          -984.3763455795639
        ],
        "lookAt": [
          58.318210141565956,
          30.661991068992855,
          -984.4297135438798
        ],
        "up": [
          0.5271775848887584,
          0.8496721570659876,
          0.011875163059406276
        ],
        "fovYDegrees": 31.265892492733506
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1700",
        "timeMs": 1700,
        "eye": [
          49.45752421796848,
          35.989601163448995,
          -984.3762208133207
        ],
        "lookAt": [
          57.98678212293406,
          30.769567499879976,
          -984.431067342732
        ],
        "up": [
          0.5220306207962302,
          0.8528504988403791,
          0.011404278969724296
        ],
        "fovYDegrees": 31.346489864122344
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1750",
        "timeMs": 1750,
        "eye": [
          49.1002778398559,
          36.04491953268262,
          -984.3761081422252
        ],
        "lookAt": [
          57.66050392259726,
          30.875841240431644,
          -984.4323133104424
        ],
        "up": [
          0.5169376590887719,
          0.8559525827866263,
          0.010983288975723421
        ],
        "fovYDegrees": 31.426373349819205
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1800",
        "timeMs": 1800,
        "eye": [
          48.75340743681703,
          36.09881097785555,
          -984.3760104818714
        ],
        "lookAt": [
          57.343485663303596,
          30.979508272539224,
          -984.4334201864726
        ],
        "up": [
          0.5119622089333814,
          0.8589422816455874,
          0.01062324929225671
        ],
        "fovYDegrees": 31.505357219898524
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1850",
        "timeMs": 1850,
        "eye": [
          48.42138329611253,
          36.15060634898081,
          -984.3759307478534
        ],
        "lookAt": [
          57.039851623326285,
          31.07925641575607,
          -984.4343563209763
        ],
        "up": [
          0.5071686054747278,
          0.8617848866699842,
          0.010335121093746432
        ],
        "fovYDegrees": 31.583255260128603
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1900",
        "timeMs": 1900,
        "eye": [
          48.10867570500258,
          36.19963649607143,
          -984.3758718557651
        ],
        "lookAt": [
          56.75373911642457,
          31.173766238207477,
          -984.4350897889565
        ],
        "up": [
          0.5026219109253667,
          0.8644469920362399,
          0.010129788607373296
        ],
        "fovYDegrees": 31.65988078510709
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k1950",
        "timeMs": 1950,
        "eye": [
          47.81975495074748,
          36.24523226914041,
          -984.3758367212006
        ],
        "lookAt": [
          56.48929688594913,
          31.26171219229505,
          -984.4355885199961
        ],
        "up": [
          0.498387792030684,
          0.8668963298942216,
          0.010018082187160061
        ],
        "fovYDegrees": 31.735046654577918
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2000",
        "timeMs": 2000,
        "eye": [
          47.5582946600197,
          36.28684323467909,
          -984.3758277551572
        ],
        "lookAt": [
          56.24996063340505,
          31.342013061893887,
          -984.4358041622539
        ],
        "up": [
          0.494519322638901,
          0.8691088358190506,
          0.01002352423138487
        ],
        "fovYDegrees": 31.808565292980223
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2050",
        "timeMs": 2050,
        "eye": [
          47.30151044040241,
          36.32786117074474,
          -984.3758305206044
        ],
        "lookAt": [
          56.015137857421664,
          31.42182673912793,
          -984.4351886974687
        ],
        "up": [
          0.4906393504881767,
          0.8712986937144501,
          0.01056475669540123
        ],
        "fovYDegrees": 31.880248712275943
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2100",
        "timeMs": 2100,
        "eye": [
          47.03784708905524,
          36.370003146116,
          -984.3758374483407
        ],
        "lookAt": [
          55.77430172296381,
          31.50471472624796,
          -984.4335351186179
        ],
        "up": [
          0.48656254326433407,
          0.8735659612948444,
          0.011806894560274448
        ],
        "fovYDegrees": 31.94990853809892
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2150",
        "timeMs": 2150,
        "eye": [
          46.76818084328226,
          36.41313350315297,
          -984.3758488031334
        ],
        "lookAt": [
          55.52818044174609,
          31.59033603509734,
          -984.4309089250886
        ],
        "up": [
          0.4823078664810791,
          0.875894576890797,
          0.013704455617119774
        ],
        "fovYDegrees": 32.01735603926261
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2200",
        "timeMs": 2200,
        "eye": [
          46.49338794038735,
          36.457116584215726,
          -984.3758648497497
        ],
        "lookAt": [
          55.277505088504746,
          31.678347650813187,
          -984.4273761038266
        ],
        "up": [
          0.47789378511901887,
          0.8782680124884216,
          0.016211982736695833
        ],
        "fovYDegrees": 32.0824021606587
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2250",
        "timeMs": 2250,
        "eye": [
          46.21434461767464,
          36.50181673166437,
          -984.375885852957
        ],
        "lookAt": [
          55.02301007741076,
          31.768404351438974,
          -984.423003335708
        ],
        "up": [
          0.4733385080239316,
          0.8806695074376512,
          0.019284073511672575
        ],
        "fovYDegrees": 32.14485755957243
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2300",
        "timeMs": 2300,
        "eye": [
          45.93192711244818,
          36.547098287859,
          -984.3759120775227
        ],
        "lookAt": [
          54.765433584122704,
          31.860158567730235,
          -984.4178581710648
        ],
        "up": [
          0.4686602079622218,
          0.8830822867913745,
          0.022875406622040784
        ],
        "fovYDegrees": 32.20453264543353
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2350",
        "timeMs": 2350,
        "eye": [
          45.647011662011934,
          36.592825595159695,
          -984.375943788214
        ],
        "lookAt": [
          54.50551791257703,
          31.95326028183692,
          -984.4120091748239
        ],
        "up": [
          0.46387721687293737,
          0.8854897643572343,
          0.026940766224957367
        ],
        "fovYDegrees": 32.26123762301446
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2400",
        "timeMs": 2400,
        "eye": [
          45.36047450367006,
          36.63886299592656,
          -984.3759812497982
        ],
        "lookAt": [
          54.244009805440584,
          32.04735696332144,
          -984.405526042025
        ],
        "up": [
          0.4590081961728564,
          0.8878757303336305,
          0.0314350653039408
        ],
        "fovYDegrees": 32.3147825390795
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2450",
        "timeMs": 2450,
        "eye": [
          45.073191874726334,
          36.68507483251967,
          -984.3760247270426
        ],
        "lookAt": [
          53.98166069702036,
          32.1420935407808,
          -984.3984796847574
        ],
        "up": [
          0.45407228226168844,
          0.8902245232210121,
          0.03631336857940037
        ],
        "fovYDegrees": 32.36497733247953
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2500",
        "timeMs": 2500,
        "eye": [
          44.78604001248485,
          36.731325447299135,
          -984.3760744847143
        ],
        "lookAt": [
          53.71922690733901,
          32.23711240718649,
          -984.3909422917901
        ],
        "up": [
          0.44908920764045657,
          0.892521185550542,
          0.04153091528394114
        ],
        "fovYDegrees": 32.41163188767821
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2550",
        "timeMs": 2550,
        "eye": [
          44.49989515424978,
          36.77747918262504,
          -984.3761307875809
        ],
        "lookAt": [
          53.457469776030656,
          32.33205345693696,
          -984.3829873623791
        ],
        "up": [
          0.44407939829857274,
          0.8947516028554452,
          0.04704314184224039
        ],
        "fovYDegrees": 32.45455609168511
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2600",
        "timeMs": 2600,
        "eye": [
          44.21563353732495,
          36.82340038085748,
          -984.3761939004095
        ],
        "lookAt": [
          53.197155734699386,
          32.42655415253664,
          -984.3746897158908
        ],
        "up": [
          0.43906404824387796,
          0.8969026252256203,
          0.05280570426656841
        ],
        "fovYDegrees": 32.493559894361205
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2650",
        "timeMs": 2650,
        "eye": [
          43.93413139901457,
          36.86895338435654,
          -984.3762640879676
        ],
        "lookAt": [
          52.93905631640303,
          32.52024961877249,
          -984.3661254790178
        ],
        "up": [
          0.4340651722440818,
          0.8989621707324468,
          0.05877449988492368
        ],
        "fovYDegrees": 32.52845337205064
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2700",
        "timeMs": 2700,
        "eye": [
          43.656264976622445,
          36.91400253548232,
          -984.3763416150222
        ],
        "lookAt": [
          52.68394810096929,
          32.61277276225716,
          -984.3573720524388
        ],
        "up": [
          0.4291056380160248,
          0.9009193099864536,
          0.06490568785855937
        ],
        "fovYDegrees": 32.55904679448134
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2750",
        "timeMs": 2750,
        "eye": [
          43.38291050745279,
          36.958412176594905,
          -984.3764267463408
        ],
        "lookAt": [
          52.43261259493412,
          32.70375441424507,
          -984.3485080588293
        ],
        "up": [
          0.42420917923893464,
          0.9027643310932781,
          0.07115570781838616
        ],
        "fovYDegrees": 32.58515069486455
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2800",
        "timeMs": 2800,
        "eye": [
          43.11494422880959,
          37.0020466500544,
          -984.3765197466907
        ],
        "lookAt": [
          52.18583604498621,
          32.79282349470823,
          -984.3396132741194
        ],
        "up": [
          0.41940039087699593,
          0.9044887843002811,
          0.0774812958540489
        ],
        "fovYDegrees": 32.606575943110826
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2850",
        "timeMs": 2850,
        "eye": [
          42.85324237799672,
          37.04477029822088,
          -984.3766208808391
        ],
        "lookAt": [
          51.944409183927824,
          32.879607195780565,
          -984.3307685438631
        ],
        "up": [
          0.4147047083725348,
          0.9060855056739647,
          0.08383949702381689
        ],
        "fovYDegrees": 32.623133822066315
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2900",
        "timeMs": 2900,
        "eye": [
          42.59868119231831,
          37.086447463454455,
          -984.3767304135533
        ],
        "lookAt": [
          51.7091269082986,
          32.96373118284568,
          -984.3220556864875
        ],
        "up": [
          0.41014837231140416,
          0.9075486192133533,
          0.0901876735160726
        ],
        "fovYDegrees": 32.63463610665966
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k2950",
        "timeMs": 2950,
        "eye": [
          42.352136909078425,
          37.126942488115205,
          -984.3768486096008
        ],
        "lookAt": [
          51.48078788696215,
          33.04481981175202,
          -984.313557385059
        ],
        "up": [
          0.4057583801642355,
          0.9088735168829212,
          0.096483507582209
        ],
        "fovYDegrees": 32.64089514583556
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3000",
        "timeMs": 3000,
        "eye": [
          42.11448576558101,
          37.16611971456322,
          -984.3769757337485
        ],
        "lookAt": [
          51.26019410012361,
          33.122496360893884,
          -984.3053570690174
        ],
        "up": [
          0.40156242666885594,
          0.9100568161366152,
          0.10268499837418653
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3050",
        "timeMs": 3050,
        "eye": [
          41.88660399913008,
          37.20384348515861,
          -984.377112050764
        ],
        "lookAt": [
          51.04815030842166,
          33.19638327719436,
          -984.297538787107
        ],
        "up": [
          0.3975888343383191,
          0.9110962945982387,
          0.10875045185587175
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3100",
        "timeMs": 3100,
        "eye": [
          41.66936784702972,
          37.23997814226144,
          -984.3772578254146
        ],
        "lookAt": [
          50.84546345192366,
          33.26610243536983,
          -984.2901870724446
        ],
        "up": [
          0.39386647545403275,
          0.9119908016593619,
          0.11463846301365226
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3150",
        "timeMs": 3150,
        "eye": [
          41.46365354658393,
          37.27438802823183,
          -984.3774133224675
        ],
        "lookAt": [
          50.652941979046645,
          33.33127541024271,
          -984.283386800348
        ],
        "up": [
          0.3904246867330461,
          0.9127401468508362,
          0.12030788966695828
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3200",
        "timeMs": 3200,
        "eye": [
          41.27033733509667,
          37.306937485429856,
          -984.3775788066899
        ],
        "lookAt": [
          50.47139510562809,
          33.39152376230061,
          -984.2772230391702
        ],
        "up": [
          0.38729317764183235,
          0.9133449649352289,
          0.12571781727168935
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3250",
        "timeMs": 3250,
        "eye": [
          41.0902954498719,
          37.33749085621562,
          -984.3777545428493
        ],
        "lookAt": [
          50.30163200458041,
          33.446469337175635,
          -984.2717808939615
        ],
        "up": [
          0.38450193306531116,
          0.9138065577531063,
          0.13082751421798935
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3300",
        "timeMs": 3300,
        "eye": [
          40.92440412821395,
          37.3659124829492,
          -984.3779407957129
        ],
        "lookAt": [
          50.14446092678368,
          33.495734580235286,
          -984.2671453423175
        ],
        "up": [
          0.38208111072934964,
          0.9141267129348952,
          0.135596377247588
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3350",
        "timeMs": 3350,
        "eye": [
          40.7735396074265,
          37.3920667079907,
          -984.378137830048
        ],
        "lookAt": [
          50.00068825410315,
          33.53894286803698,
          -984.2634010612526
        ],
        "up": [
          0.3800609334177996,
          0.914307499661877,
          0.13998386675484284
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3400",
        "timeMs": 3400,
        "eye": [
          40.638578124813726,
          37.4158178737002,
          -984.3783459106218
        ],
        "lookAt": [
          49.87111748567412,
          33.57571885899907,
          -984.260632243382
        ],
        "up": [
          0.3784715756219703,
          0.9143510417256293,
          0.14394943189009005
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3450",
        "timeMs": 3450,
        "eye": [
          40.52039591767956,
          37.437030322437806,
          -984.3785653022017
        ],
        "lookAt": [
          49.756548158862905,
          33.605688866281895,
          -984.2589224000931
        ],
        "up": [
          0.3773430438123198,
          0.9142592681971385,
          0.14745242555511362
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3500",
        "timeMs": 3500,
        "eye": [
          40.419869223328156,
          37.455568396563606,
          -984.3787962695551
        ],
        "lookAt": [
          49.65777470662252,
          33.62848125655051,
          -984.2583541487402
        ],
        "up": [
          0.3767050490305575,
          0.9140336420784021,
          0.15045200957041752
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3550",
        "timeMs": 3550,
        "eye": [
          40.333175589803695,
          37.4719573481387,
          -984.3790902767647
        ],
        "lookAt": [
          49.57203599281678,
          33.64716483616377,
          -984.2590040828151
        ],
        "up": [
          0.3762996438444484,
          0.9137651324087568,
          0.15307469038527757
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3600",
        "timeMs": 3600,
        "eye": [
          40.251980063081696,
          37.487368057296074,
          -984.3795337810529
        ],
        "lookAt": [
          49.49294170386372,
          33.66761260661742,
          -984.2608061490297
        ],
        "up": [
          0.37564113249925635,
          0.9136082760401009,
          0.1556073826195629
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3650",
        "timeMs": 3650,
        "eye": [
          40.17588040066113,
          37.50186404065307,
          -984.3801158324555
        ],
        "lookAt": [
          49.41992505668655,
          33.68950759054708,
          -984.2635967578661
        ],
        "up": [
          0.3747659685161443,
          0.9135477420006389,
          0.1580537627761215
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3700",
        "timeMs": 3700,
        "eye": [
          40.10465262961639,
          37.51548382782249,
          -984.3808234239867
        ],
        "lookAt": [
          49.352551562006916,
          33.71239854161967,
          -984.2672116193282
        ],
        "up": [
          0.3737216165615458,
          0.9135646673300658,
          0.16041119636961582
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3750",
        "timeMs": 3750,
        "eye": [
          40.0380727770215,
          37.52826594841713,
          -984.3816435486603
        ],
        "lookAt": [
          49.290387348867824,
          33.73583395163364,
          -984.271486327504
        ],
        "up": [
          0.3725553100446305,
          0.9136403701121538,
          0.16267702683192883
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3800",
        "timeMs": 3800,
        "eye": [
          39.97591686995086,
          37.54024893204979,
          -984.3825631994906
        ],
        "lookAt": [
          49.23300013111473,
          33.75936169541753,
          -984.2762560983592
        ],
        "up": [
          0.3713140907020528,
          0.9137564294919585,
          0.16484851715503931
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3850",
        "timeMs": 3850,
        "eye": [
          39.917960935478796,
          37.55147130833327,
          -984.3835693694915
        ],
        "lookAt": [
          49.17995988906143,
          33.78252878744571,
          -984.2813555833377
        ],
        "up": [
          0.370044840548026,
          0.9138947381096817,
          0.16692280742673368
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3900",
        "timeMs": 3900,
        "eye": [
          39.8639810006794,
          37.561971606880356,
          -984.384649051677
        ],
        "lookAt": [
          49.13083926569054,
          33.804881248948725,
          -984.2866187577272
        ],
        "up": [
          0.368794306534391,
          0.914037526924131,
          0.1688968881958349
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k3950",
        "timeMs": 3950,
        "eye": [
          39.813753092626854,
          37.57178835730385,
          -984.3857892390611
        ],
        "lookAt": [
          49.08521367633719,
          33.82596408359111,
          -984.2918788819655
        ],
        "up": [
          0.3676091183417809,
          0.9141673623806446,
          0.17076758963572058
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4000",
        "timeMs": 4000,
        "eye": [
          39.76705323839561,
          37.58096008921655,
          -984.386976924658
        ],
        "lookAt": [
          49.04266113062515,
          33.84532135947339,
          -984.2969685336492
        ],
        "up": [
          0.3665357997438748,
          0.9142671158734857,
          0.17253158649504247
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4050",
        "timeMs": 4050,
        "eye": [
          39.72365746505971,
          37.58952533223126,
          -984.3881991014815
        ],
        "lookAt": [
          49.00276176545922,
          33.862496395288915,
          -984.3017197079726
        ],
        "up": [
          0.36562077395197573,
          0.9143199054600538,
          0.17418541883371483
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4100",
        "timeMs": 4100,
        "eye": [
          39.68334179969363,
          37.59752261596076,
          -984.389442762546
        ],
        "lookAt": [
          48.96509708812362,
          33.87703204893221,
          -984.305963984684
        ],
        "up": [
          0.3649103632565383,
          0.9143090098029427,
          0.17572552854136955
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4150",
        "timeMs": 4150,
        "eye": [
          39.645882269371356,
          37.60499047001786,
          -984.3906949008651
        ],
        "lookAt": [
          48.92924892897162,
          33.888471107715006,
          -984.3095327603662
        ],
        "up": [
          0.3644507831357617,
          0.9142177543452142,
          0.17714831162538275
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4200",
        "timeMs": 4200,
        "eye": [
          39.61105490116726,
          37.611967424015354,
          -984.3919425094532
        ],
        "lookAt": [
          48.89479810383785,
          33.896356780599625,
          -984.3122575459826
        ],
        "up": [
          0.3642881307989108,
          0.9140293697639682,
          0.17845018623671682
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4250",
        "timeMs": 4250,
        "eye": [
          39.578635722155546,
          37.61849200756604,
          -984.393172581324
        ],
        "lookAt": [
          48.86132278713409,
          33.90023329450657,
          -984.3139703311209
        ],
        "up": [
          0.3644683678737867,
          0.9137268227976306,
          0.17962767637411936
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4300",
        "timeMs": 4300,
        "eye": [
          39.54840075941056,
          37.62460275028271,
          -984.394372109492
        ],
        "lookAt": [
          48.828396597635766,
          33.89964659879223,
          -984.3145040182658
        ],
        "up": [
          0.36503729663405254,
          0.9132926196042286,
          0.18067751116989839
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4350",
        "timeMs": 4350,
        "eye": [
          39.52012604000641,
          37.630338181778164,
          -984.3955280869708
        ],
        "lookAt": [
          48.79558640022571,
          33.894145184417965,
          -984.3136929326844
        ],
        "up": [
          0.36604052879350113,
          0.9127085818830143,
          0.1815967396118966
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4400",
        "timeMs": 4400,
        "eye": [
          39.49358759101739,
          37.6357368316652,
          -984.3966275067745
        ],
        "lookAt": [
          48.76244982837432,
          33.88328102713786,
          -984.311373416153
        ],
        "up": [
          0.36752344547168403,
          0.9119555960828135,
          0.18238286049369454
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4450",
        "timeMs": 4450,
        "eye": [
          39.468561439517764,
          37.64083722955661,
          -984.3976573619174
        ],
        "lookAt": [
          48.72853253392756,
          33.866610667208164,
          -984.3073845157315
        ],
        "up": [
          0.369531146459879,
          0.9110133361311489,
          0.18303396730454755
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4500",
        "timeMs": 4500,
        "eye": [
          39.44482361258178,
          37.6456779050652,
          -984.3986046454132
        ],
        "lookAt": [
          48.69336517290908,
          33.84369644165252,
          -984.3015687821202
        ],
        "up": [
          0.37210838638992094,
          0.9098599602535689,
          0.18354890766676696
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4550",
        "timeMs": 4550,
        "eye": [
          39.42211283870031,
          37.65030711663042,
          -984.3994804952588
        ],
        "lookAt": [
          48.65710753118532,
          33.815752610537146,
          -984.2942912868134
        ],
        "up": [
          0.3751468201031787,
          0.9085347736394547,
          0.18392778054002168
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4600",
        "timeMs": 4600,
        "eye": [
          39.400116458983156,
          37.65479437206746,
          -984.4003904981424
        ],
        "lookAt": [
          48.62253028731826,
          33.79028539555775,
          -984.2879025014081
        ],
        "up": [
          0.3779525507210322,
          0.907321491254354,
          0.18417269317529378
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4650",
        "timeMs": 4650,
        "eye": [
          39.37879911599205,
          37.6591477175417,
          -984.4013454892083
        ],
        "lookAt": [
          48.58998805374398,
          33.76814376323847,
          -984.2826585875168
        ],
        "up": [
          0.3804472556361358,
          0.9062554999290627,
          0.18428471050865158
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4700",
        "timeMs": 4700,
        "eye": [
          39.358155482630956,
          37.66336798951601,
          -984.4023417244025
        ],
        "lookAt": [
          48.55944603083296,
          33.74917386737736,
          -984.2785006093245
        ],
        "up": [
          0.3826454291468241,
          0.9053335743062305,
          0.18426501237876908
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4750",
        "timeMs": 4750,
        "eye": [
          39.33818023180399,
          37.667456024453294,
          -984.403375459671
        ],
        "lookAt": [
          48.53086384345828,
          33.73322384393337,
          -984.2753709059078
        ],
        "up": [
          0.38456125408487274,
          0.9045519791741121,
          0.18411506953225337
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4784",
        "timeMs": 4784,
        "eye": [
          39.32497612307712,
          37.670160781238664,
          -984.4040979092101
        ],
        "lookAt": [
          48.51252444796812,
          33.724026148129106,
          -984.2738011895048
        ],
        "up": [
          0.38570983944678827,
          0.9040985282055871,
          0.18393958532742166
        ],
        "fovYDegrees": 32.64206322821485
      }
    ]
  }
}
```

### shots / maharaka.waterpang.source.intro15.camera.2

```json
{
  "shotId": "maharaka.waterpang.source.intro15.camera.2",
  "displayName": "워터팡 / 영상 맞춤 카메라 4784",
  "defaultHoldMs": 4723,
  "transitionEasing": "LINEAR",
  "activation": "PATTERN_ONLY",
  "sequenceInstanceId": "",
  "box": {
    "center": [
      39.32497612307712,
      37.670160781238664,
      -984.4040979092101
    ],
    "halfExtents": [
      1,
      1,
      1
    ],
    "yawDegrees": 0
  },
  "eye": [
    39.32497612307712,
    37.670160781238664,
    -984.4040979092101
  ],
  "lookAt": [
    48.51252444796812,
    33.724026148129106,
    -984.2738011895048
  ],
  "fovYDegrees": 32.64206322821485,
  "blendInMs": 0,
  "blendOutMs": 0,
  "priority": 100,
  "cameraTrack": {
    "durationMs": 4723,
    "interpolation": "LINEAR",
    "easing": "LINEAR",
    "keyframes": [
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4784",
        "timeMs": 0,
        "eye": [
          39.32497612307712,
          37.670160781238664,
          -984.4040979092101
        ],
        "lookAt": [
          48.51252444796812,
          33.724026148129106,
          -984.2738011895048
        ],
        "up": [
          0.38570983944678827,
          0.9040985282055871,
          0.18393958532742166
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4834",
        "timeMs": 50,
        "eye": [
          39.306111762548866,
          37.674028540998954,
          -984.4051862159891
        ],
        "lookAt": [
          48.487130340804036,
          33.712812594352094,
          -984.2722723726017
        ],
        "up": [
          0.3871824684252247,
          0.9035430336854308,
          0.18357484147529188
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4884",
        "timeMs": 100,
        "eye": [
          39.28790150793759,
          37.6777663054427,
          -984.4063019887778
        ],
        "lookAt": [
          48.46356469699191,
          33.70422046388529,
          -984.2716227918862
        ],
        "up": [
          0.3884094174811122,
          0.9031157248844834,
          0.18308498539846804
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4934",
        "timeMs": 150,
        "eye": [
          39.270340032147324,
          37.68137491103281,
          -984.4074414835225
        ],
        "lookAt": [
          48.441771343647474,
          33.698103805560464,
          -984.2717985745898
        ],
        "up": [
          0.38940394217057234,
          0.9028114043789088,
          0.18247229363769182
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k4984",
        "timeMs": 200,
        "eye": [
          39.253422008081934,
          37.68485519423214,
          -984.4086009561686
        ],
        "lookAt": [
          48.42169096592497,
          33.69431789752685,
          -984.2727466214724
        ],
        "up": [
          0.3901791060609603,
          0.9026245781303703,
          0.18173919816165388
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5034",
        "timeMs": 250,
        "eye": [
          39.23714210864555,
          37.68820799150359,
          -984.4097766626625
        ],
        "lookAt": [
          48.40326154526101,
          33.69271909048225,
          -984.2744144983935
        ],
        "up": [
          0.3907478058726359,
          0.9025494937578017,
          0.18088826253590023
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5084",
        "timeMs": 300,
        "eye": [
          39.22149500674229,
          37.691434139310026,
          -984.4109648589503
        ],
        "lookAt": [
          48.38641877055285,
          33.693164655653774,
          -984.2767503306619
        ],
        "up": [
          0.3911227960515731,
          0.9025801764108943,
          0.17992215972158251
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5134",
        "timeMs": 350,
        "eye": [
          39.20647537527592,
          37.69453447411432,
          -984.4121618009775
        ],
        "lookAt": [
          48.37109642289902,
          33.69551263866922,
          -984.279702701264
        ],
        "up": [
          0.3913167126235106,
          0.9027104623082539,
          0.17884365144099337
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5184",
        "timeMs": 400,
        "eye": [
          39.19207788715092,
          37.69750983237937,
          -984.413363744691
        ],
        "lookAt": [
          48.357226734452404,
          33.699621720300556,
          -984.2832205539077
        ],
        "up": [
          0.39134209620084015,
          0.9029340299961097,
          0.17765556905460908
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5234",
        "timeMs": 450,
        "eye": [
          39.17829721527106,
          37.70036105056803,
          -984.4145669460361
        ],
        "lookAt": [
          48.34474072185206,
          33.70535108491243,
          -984.2872531016537
        ],
        "up": [
          0.391211414032348,
          0.9032444293764483,
          0.17636079590210862
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5284",
        "timeMs": 500,
        "eye": [
          39.165128032540295,
          37.703088965143195,
          -984.4157676609591
        ],
        "lookAt": [
          48.333568494632814,
          33.71256029731117,
          -984.2917497417881
        ],
        "up": [
          0.39093708100338054,
          0.9036351085465704,
          0.17496225106743596
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5334",
        "timeMs": 550,
        "eye": [
          39.15256501186283,
          37.70569441256774,
          -984.4169621454059
        ],
        "lookAt": [
          48.32363953892984,
          33.72110918856202,
          -984.2966599774372
        ],
        "up": [
          0.39053147951002704,
          0.904099438485421,
          0.17346287453531964
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5384",
        "timeMs": 600,
        "eye": [
          39.1406028261427,
          37.70817822930454,
          -984.4181466553227
        ],
        "lookAt": [
          48.31488297673016,
          33.730857751225244,
          -984.3019333463265
        ],
        "up": [
          0.39000697814560364,
          0.9046307356156734,
          0.1718656137136808
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5434",
        "timeMs": 650,
        "eye": [
          39.129236148283844,
          37.71054125181649,
          -984.4193174466554
        ],
        "lookAt": [
          48.30722780085861,
          33.741666044354155,
          -984.3075193569633
        ],
        "up": [
          0.3893759491510904,
          0.9052222822645247,
          0.1701734113029791
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5484",
        "timeMs": 700,
        "eye": [
          39.118459651190484,
          37.71278431656644,
          -984.4204707753503
        ],
        "lookAt": [
          48.300603085825415,
          33.75339410850058,
          -984.3133674324388
        ],
        "up": [
          0.38865078459328706,
          0.9058673450405215,
          0.1683891944997049
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5534",
        "timeMs": 750,
        "eye": [
          39.108268007766405,
          37.71490826001729,
          -984.421602897353
        ],
        "lookAt": [
          48.294938174608085,
          33.76590189088485,
          -984.3194268619408
        ],
        "up": [
          0.38784391124534817,
          0.9065591911385257,
          0.16651586552689923
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5584",
        "timeMs": 800,
        "eye": [
          39.098655890915715,
          37.71691391863191,
          -984.4227100686095
        ],
        "lookAt": [
          48.29016284139639,
          33.77904918080898,
          -984.3256467600102
        ],
        "up": [
          0.3869678041540268,
          0.9072911025801683,
          0.16455629348971546
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5634",
        "timeMs": 850,
        "eye": [
          39.08961797354261,
          37.71880212887318,
          -984.4237885450663
        ],
        "lookAt": [
          48.286207430283575,
          33.7926955553224,
          -984.3319760334886
        ],
        "up": [
          0.3860349988864601,
          0.9080563883928598,
          0.1625133075586317
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5684",
        "timeMs": 900,
        "eye": [
          39.08114892855106,
          37.720573727203984,
          -984.4248345826692
        ],
        "lookAt": [
          48.28300296985532,
          33.80670033508963,
          -984.338363356057
        ],
        "up": [
          0.3850581024566469,
          0.9088483947266492,
          0.16038969148695084
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5734",
        "timeMs": 950,
        "eye": [
          39.07324342884509,
          37.72222955008719,
          -984.425844437364
        ],
        "lookAt": [
          48.28048126359954,
          33.820922550358894,
          -984.3447571502152
        ],
        "up": [
          0.38404980293791907,
          0.9096605129049327,
          0.15818817947267905
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5784",
        "timeMs": 1000,
        "eye": [
          39.06589614732866,
          37.723770433985685,
          -984.426814365097
        ],
        "lookAt": [
          48.27857495603691,
          33.835220916889526,
          -984.3511055765117
        ],
        "up": [
          0.38302287777268634,
          0.9104861854022608,
          0.15591145337777385
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5834",
        "timeMs": 1050,
        "eye": [
          39.05910175690588,
          37.72519721536235,
          -984.4277406218141
        ],
        "lookAt": [
          48.27721757445766,
          33.84945382166435,
          -984.3573565298046
        ],
        "up": [
          0.38199020079454293,
          0.9113189097402389,
          0.15356214132007637
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5884",
        "timeMs": 1100,
        "eye": [
          39.0528549304808,
          37.72651073068005,
          -984.4286194634614
        ],
        "lookAt": [
          48.276343546139515,
          33.863479318190684,
          -984.3634576423195
        ],
        "up": [
          0.38096474798044716,
          0.9121522402907839,
          0.15114281765501938
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5934",
        "timeMs": 1150,
        "eye": [
          39.04715034095744,
          37.72771181640167,
          -984.4294471459848
        ],
        "lookAt": [
          48.27588819091957,
          33.877155131181354,
          -984.3693562932575
        ],
        "up": [
          0.37995960195211387,
          0.9129797879747904,
          0.14865600436544135
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k5984",
        "timeMs": 1200,
        "eye": [
          39.04198266123984,
          37.7288013089901,
          -984.4302199253304
        ],
        "lookAt": [
          48.27578768899365,
          33.89033867040315,
          -984.374999624711
        ],
        "up": [
          0.3789879552459888,
          0.9137952178435452,
          0.14610417387854502
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6034",
        "timeMs": 1250,
        "eye": [
          39.03734656423204,
          37.729780044908196,
          -984.4309340574443
        ],
        "lookAt": [
          48.2759790238248,
          33.90288705348718,
          -984.3803345636562
        ],
        "up": [
          0.378063112370176,
          0.9145922445300371,
          0.14348975332923444
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6084",
        "timeMs": 1300,
        "eye": [
          39.03323672283807,
          37.73064886061886,
          -984.4315857982725
        ],
        "lookAt": [
          48.27639990005505,
          33.91465713751115,
          -984.3853078498131
        ],
        "up": [
          0.37719849066446204,
          0.915364625557602,
          0.14081513028876919
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6134",
        "timeMs": 1350,
        "eye": [
          39.029647809961965,
          37.73140859258495,
          -984.4321714037609
        ],
        "lookAt": [
          48.276988636332426,
          33.92550555918939,
          -984.3898660691967
        ],
        "up": [
          0.37640761997610683,
          0.9161061524941481,
          0.1380826599768838
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6184",
        "timeMs": 1400,
        "eye": [
          39.026574498507856,
          37.73206007726935,
          -984.4326871298556
        ],
        "lookAt": [
          48.277684032987736,
          33.935288783541964,
          -984.3939556932286
        ],
        "up": [
          0.3757041411593328,
          0.9168106399414909,
          0.13529467397426403
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6234",
        "timeMs": 1450,
        "eye": [
          39.0240114613796,
          37.73260415113496,
          -984.4331292325026
        ],
        "lookAt": [
          48.27842521452209,
          33.9438631609594,
          -984.3975231233253
        ],
        "up": [
          0.3751018034004343,
          0.917471912351115,
          0.13245349045053473
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6284",
        "timeMs": 1500,
        "eye": [
          39.02195337148133,
          37.73304165064462,
          -984.4334939676479
        ],
        "lookAt": [
          48.27915144690016,
          33.95108499263428,
          -984.4005147409549
        ],
        "up": [
          0.37461446036314644,
          0.9180837886599564,
          0.12956142592071615
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6334",
        "timeMs": 1550,
        "eye": [
          39.020394901717225,
          37.733373412261244,
          -984.4337775912377
        ],
        "lookAt": [
          48.27980192967741,
          33.95681060439621,
          -984.4028769632165
        ],
        "up": [
          0.3742560651403296,
          0.918640064742578,
          0.12662080854042024
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6384",
        "timeMs": 1600,
        "eye": [
          39.01933072499117,
          37.73360027244769,
          -984.4339763592178
        ],
        "lookAt": [
          48.280315563033454,
          33.96089642906066,
          -984.40455630409
        ],
        "up": [
          0.37404066398817093,
          0.9191344936794055,
          0.12363399294688861
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6434",
        "timeMs": 1650,
        "eye": [
          39.01875551420712,
          37.733723067666844,
          -984.4340865275343
        ],
        "lookAt": [
          48.28063068982976,
          33.96319909748672,
          -984.4054994415987
        ],
        "up": [
          0.3739823888079547,
          0.9195607638445036,
          0.12060337664928279
        ],
        "fovYDegrees": 32.64206322821485
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6469",
        "timeMs": 1685,
        "eye": [
          39.018684948661644,
          37.73373437224929,
          -984.434108179318
        ],
        "lookAt": [
          48.280744319297995,
          33.963661594223794,
          -984.4056922823848
        ],
        "up": [
          0.3740425227220624,
          0.9198151708171116,
          0.11845776770817207
        ],
        "fovYDegrees": 32.64202487325161
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6484",
        "timeMs": 1700,
        "eye": [
          39.064585630926786,
          37.71964759210546,
          -984.4560549884562
        ],
        "lookAt": [
          48.35963571303894,
          33.942522145236126,
          -984.433820504572
        ],
        "up": [
          0.37347409511248975,
          0.9201123997077431,
          0.11794181694363226
        ],
        "fovYDegrees": 32.60482235016393
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6534",
        "timeMs": 1750,
        "eye": [
          39.80373996895229,
          37.49254199470264,
          -984.8271480206794
        ],
        "lookAt": [
          49.638770437341506,
          33.599663609846246,
          -984.9097360671987
        ],
        "up": [
          0.3634272434181147,
          0.9237082125459652,
          0.1211766347801768
        ],
        "fovYDegrees": 32.01002545032794
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6584",
        "timeMs": 1800,
        "eye": [
          41.34062673398383,
          37.02027580033611,
          -985.6029034966348
        ],
        "lookAt": [
          52.2041627522796,
          32.91180759344981,
          -985.9066555152626
        ],
        "up": [
          0.34238826936642713,
          0.9304937352827285,
          0.13019862364809537
        ],
        "fovYDegrees": 30.784945400349812
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6602",
        "timeMs": 1818,
        "eye": [
          42.06305747235106,
          36.79828078529821,
          -985.9677475951653
        ],
        "lookAt": [
          53.363561163241705,
          32.600840596058454,
          -986.3764698697515
        ],
        "up": [
          0.3325299782067942,
          0.9334859339272773,
          0.13426773534139852
        ],
        "fovYDegrees": 30.213996751107455
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6634",
        "timeMs": 1850,
        "eye": [
          43.53954160577769,
          36.34457865896629,
          -986.7132751019362
        ],
        "lookAt": [
          55.63996032191079,
          31.99008133073808,
          -987.3383975345848
        ],
        "up": [
          0.3124964436338485,
          0.9392724461045117,
          0.14182117156843727
        ],
        "fovYDegrees": 29.05636583395695
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6684",
        "timeMs": 1900,
        "eye": [
          46.26478026409026,
          35.507180220553664,
          -988.088216522197
        ],
        "lookAt": [
          59.51262169253665,
          30.950335060822418,
          -989.1191503299879
        ],
        "up": [
          0.27606568376586493,
          0.9489670550999811,
          0.15247710182763397
        ],
        "fovYDegrees": 26.950581377312545
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6734",
        "timeMs": 1950,
        "eye": [
          49.38063838867788,
          34.54981013505868,
          -989.6576814430307
        ],
        "lookAt": [
          63.41755794834333,
          29.900720766871217,
          -991.1625211637577
        ],
        "up": [
          0.23554291958818963,
          0.9587994019137236,
          0.15881826066846824
        ],
        "fovYDegrees": 24.58870045812387
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6784",
        "timeMs": 2000,
        "eye": [
          52.75141165929718,
          33.514198052441756,
          -991.3516235500509
        ],
        "lookAt": [
          67.01495770844134,
          28.93215628339793,
          -993.3808002431036
        ],
        "up": [
          0.19338583808115115,
          0.968281032487833,
          0.15822060470730942
        ],
        "fovYDegrees": 22.084122692088428
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6834",
        "timeMs": 2050,
        "eye": [
          56.24139575570441,
          32.442073622663365,
          -993.099996528871
        ],
        "lookAt": [
          70.05479818094338,
          28.111671529744875,
          -995.6844399911031
        ],
        "up": [
          0.15191228132959464,
          0.9770713928674318,
          0.1491782558596761
        ],
        "fovYDegrees": 19.541986252792537
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6884",
        "timeMs": 2100,
        "eye": [
          59.714886357656276,
          31.375166495683928,
          -994.8327540651048
        ],
        "lookAt": [
          72.39106089781829,
          27.47862708730653,
          -997.98175234559
        ],
        "up": [
          0.11316682179429094,
          0.9848254302380093,
          0.13157561476770382
        ],
        "fovYDegrees": 17.060102793570138
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6934",
        "timeMs": 2150,
        "eye": [
          63.03617914490893,
          30.355206321463932,
          -996.4798498443656
        ],
        "lookAt": [
          73.98518138894205,
          27.043792211197783,
          -1000.1788273602366
        ],
        "up": [
          0.07881597603465575,
          0.9911507597416328,
          0.10680923829561026
        ],
        "fovYDegrees": 14.730797871652737
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k6984",
        "timeMs": 2200,
        "eye": [
          66.06956979721886,
          29.423922749963808,
          -997.971237552267
        ],
        "lookAt": [
          74.89876718359146,
          26.791269229405998,
          -1002.1796761908416
        ],
        "up": [
          0.050068256271152196,
          0.9957189420538627,
          0.07769786450799436
        ],
        "fovYDegrees": 12.643099774914258
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7034",
        "timeMs": 2250,
        "eye": [
          68.67935399434253,
          28.623045431144,
          -999.2368708744224
        ],
        "lookAt": [
          75.27561786280118,
          26.683253476741093,
          -1003.8866007861885
        ],
        "up": [
          0.02763701713500883,
          0.9984585103438239,
          0.048133132101196796
        ],
        "fovYDegrees": 10.88481931203326
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7084",
        "timeMs": 2300,
        "eye": [
          70.72982741603641,
          27.994304014964953,
          -1000.2067034964455
        ],
        "lookAt": [
          75.31307416565603,
          26.667621715863014,
          -1005.2007915415651
        ],
        "up": [
          0.01179532455218786,
          0.9996766410616791,
          0.022527397416202796
        ],
        "fovYDegrees": 9.544208586955312
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7134",
        "timeMs": 2350,
        "eye": [
          72.0852857420569,
          27.579428151387116,
          -1000.8106891039498
        ],
        "lookAt": [
          75.22271020506648,
          26.68834680567081,
          -1006.0231530468044
        ],
        "up": [
          0.002568966020094447,
          0.9999829808224591,
          0.005238175160967974
        ],
        "fovYDegrees": 8.711073412052965
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7177",
        "timeMs": 2393,
        "eye": [
          72.59163702300107,
          27.42550199674566,
          -1000.9841036252606
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7184",
        "timeMs": 2400,
        "eye": [
          72.61942034140895,
          27.417293818415484,
          -1000.9836744151763
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7234",
        "timeMs": 2450,
        "eye": [
          72.80781652298593,
          27.363618802015043,
          -1000.980764964321
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7284",
        "timeMs": 2500,
        "eye": [
          72.97912366958515,
          27.318042185911857,
          -1000.9781211003956
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7334",
        "timeMs": 2550,
        "eye": [
          73.13411734153033,
          27.279730864730414,
          -1000.9757306470336
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7384",
        "timeMs": 2600,
        "eye": [
          73.27357309914521,
          27.24785173309521,
          -1000.9735814278685
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7434",
        "timeMs": 2650,
        "eye": [
          73.39826650275334,
          27.22157168563075,
          -1000.9716612665335
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7484",
        "timeMs": 2700,
        "eye": [
          73.50897311267856,
          27.20005761696152,
          -1000.9699579866619
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7534",
        "timeMs": 2750,
        "eye": [
          73.60646848924455,
          27.182476421712018,
          -1000.9684594118874
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7584",
        "timeMs": 2800,
        "eye": [
          73.69152819277507,
          27.16799499450675,
          -1000.967153365843
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7634",
        "timeMs": 2850,
        "eye": [
          73.76492778359373,
          27.1557802299702,
          -1000.9660276721626
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7684",
        "timeMs": 2900,
        "eye": [
          73.82744282202421,
          27.144999022726875,
          -1000.965070154479
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7734",
        "timeMs": 2950,
        "eye": [
          73.87984886839038,
          27.134818267401272,
          -1000.9642686364259
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7784",
        "timeMs": 3000,
        "eye": [
          73.92292148301588,
          27.12440485861788,
          -1000.9636109416368
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7834",
        "timeMs": 3050,
        "eye": [
          73.95743622622439,
          27.112925691001198,
          -1000.9630848937448
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7884",
        "timeMs": 3100,
        "eye": [
          73.98416865833958,
          27.099547659175723,
          -1000.9626783163835
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7934",
        "timeMs": 3150,
        "eye": [
          74.00389433968529,
          27.083437657765955,
          -1000.9623790331863
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k7984",
        "timeMs": 3200,
        "eye": [
          74.01738883058509,
          27.063762581396386,
          -1000.9621748677864
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8034",
        "timeMs": 3250,
        "eye": [
          74.02542769136276,
          27.039689324691512,
          -1000.9620536438173
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.470562218266851
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8084",
        "timeMs": 3300,
        "eye": [
          74.0287864823419,
          27.01038478227584,
          -1000.9620031849123
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.472624583195936
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8101",
        "timeMs": 3317,
        "eye": [
          74.029,
          26.99908152,
          -1000.962
        ],
        "lookAt": [
          75.179,
          26.69908152,
          -1006.262
        ],
        "up": [
          0.0,
          1.0,
          0.0
        ],
        "fovYDegrees": 8.505731890479693
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8134",
        "timeMs": 3350,
        "eye": [
          73.98728010065192,
          26.979268973299558,
          -1000.886667915423
        ],
        "lookAt": [
          75.20647214951775,
          26.69179103979373,
          -1006.1589995955559
        ],
        "up": [
          0.0010342872262308726,
          0.9999960837162675,
          0.0026005388021573712
        ],
        "fovYDegrees": 8.680619841914963
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8184",
        "timeMs": 3400,
        "eye": [
          73.77564531570474,
          26.95939741539101,
          -1000.5050040668111
        ],
        "lookAt": [
          75.34137043860225,
          26.65383458971101,
          -1005.637538199279
        ],
        "up": [
          0.006350028307688819,
          0.999858905036036,
          0.015551435966836837
        ],
        "fovYDegrees": 9.202732924649561
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8234",
        "timeMs": 3450,
        "eye": [
          73.40454192431986,
          26.95050682266688,
          -999.8388667646096
        ],
        "lookAt": [
          75.5595800337537,
          26.58525532411158,
          -1004.7287897570858
        ],
        "up": [
          0.015909993634012222,
          0.9991817206246425,
          0.03718549814302097
        ],
        "fovYDegrees": 10.001312555692175
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8284",
        "timeMs": 3500,
        "eye": [
          72.89579781792048,
          26.95119319412086,
          -998.9309383532966
        ],
        "lookAt": [
          75.82134741622798,
          26.488351782697322,
          -1003.4930632399263
        ],
        "up": [
          0.029439450913942893,
          0.9974870805073582,
          0.06444255543344393
        ],
        "fovYDegrees": 11.039317284965072
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8334",
        "timeMs": 3550,
        "eye": [
          72.27124088792989,
          26.96005252874666,
          -997.8239011773504
        ],
        "lookAt": [
          76.08418616800388,
          26.365824962486055,
          -1001.9908791949175
        ],
        "up": [
          0.04655406561285753,
          0.9945060312268816,
          0.093757521449058
        ],
        "fovYDegrees": 12.280174550543867
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8384",
        "timeMs": 3600,
        "eye": [
          71.55269902577116,
          26.975680825537964,
          -996.5604375812492
        ],
        "lookAt": [
          76.30736675780423,
          26.220868988756873,
          -1000.2825776951954
        ],
        "up": [
          0.06667534520951457,
          0.9903616757985144,
          0.12140078027241147
        ],
        "fovYDegrees": 13.687598735213172
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8434",
        "timeMs": 3650,
        "eye": [
          70.76200012286748,
          26.996674083488475,
          -995.1832299094712
        ],
        "lookAt": [
          76.4556086792413,
          26.05723487343251,
          -998.4279988337038
        ],
        "up": [
          0.08903872005406482,
          0.9855700613543054,
          0.1439574954394689
        ],
        "fovYDegrees": 15.225337975609829
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8484",
        "timeMs": 3700,
        "eye": [
          69.92097207064225,
          27.02162830159188,
          -993.7349605064945
        ],
        "lookAt": [
          76.50197501238172,
          25.87926735044237,
          -996.4862357597945
        ],
        "up": [
          0.11276961033664769,
          0.9808512396250031,
          0.158788729797223
        ],
        "fovYDegrees": 16.856844664078622
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8534",
        "timeMs": 3750,
        "eye": [
          69.05144276051851,
          27.04913947884188,
          -992.2583117167976
        ],
        "lookAt": [
          76.42996915652168,
          25.69191487273873,
          -994.5154602390437
        ],
        "up": [
          0.13697412499236908,
          0.9768500351239012,
          0.16432315101959247
        ],
        "fovYDegrees": 18.54488507433057
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8584",
        "timeMs": 3800,
        "eye": [
          68.1752400839195,
          27.077803614232174,
          -990.7959658848583
        ],
        "lookAt": [
          76.23483323999001,
          25.500712922431653,
          -992.5728207046577
        ],
        "up": [
          0.16079985862435603,
          0.9739091082356133,
          0.16013885962532567
        ],
        "fovYDegrees": 20.25112019342283
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8634",
        "timeMs": 3850,
        "eye": [
          67.31419193226833,
          27.106216706756456,
          -989.3906053551551
        ],
        "lookAt": [
          75.92404755262322,
          25.311740824696724,
          -990.7144127629231
        ],
        "up": [
          0.18345583389837178,
          0.9719900544825261,
          0.14689891420874832
        ],
        "fovYDegrees": 21.93570326913252
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8684",
        "timeMs": 3900,
        "eye": [
          66.49012619698833,
          27.132974755408423,
          -988.0849124721659
        ],
        "lookAt": [
          75.51703026871505,
          25.13155226933996,
          -988.9953221147263
        ],
        "up": [
          0.20420890576608222,
          0.9707552718790186,
          0.12622569043151322
        ],
        "fovYDegrees": 23.556949883013626
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8734",
        "timeMs": 3950,
        "eye": [
          65.72487076950276,
          27.156673759181764,
          -986.9215695803694
        ],
        "lookAt": [
          75.04403673339546,
          24.96707973336836,
          -987.4697398593767
        ],
        "up": [
          0.22238038603969118,
          0.9697606998222034,
          0.10055420918682835
        ],
        "fovYDegrees": 25.071142896375104
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8784",
        "timeMs": 4000,
        "eye": [
          65.0402535412346,
          27.17590971707018,
          -985.9432590242433
        ],
        "lookAt": [
          74.54425767421148,
          24.825512965982437,
          -986.1911501549224
        ],
        "up": [
          0.23735444177030712,
          0.9686799977174151,
          0.0729515660839071
        ],
        "fovYDegrees": 26.43253593497412
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8834",
        "timeMs": 4050,
        "eye": [
          64.45810240360711,
          27.18927862806737,
          -985.1926631482659
        ],
        "lookAt": [
          74.06311587274452,
          24.714151646244495,
          -985.2125902200371
        ],
        "up": [
          0.24859073680611649,
          0.9674739680712335,
          0.04686967760390692
        ],
        "fovYDegrees": 27.593613052542505
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8884",
        "timeMs": 4100,
        "eye": [
          64.0002452480437,
          27.195376491167025,
          -984.7124642969154
        ],
        "lookAt": [
          73.64876108878711,
          24.64023225487381,
          -984.5869826758062
        ],
        "up": [
          0.2556201654487522,
          0.9664325586682455,
          0.02581551010343953
        ],
        "fovYDegrees": 28.50564645329664
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8934",
        "timeMs": 4150,
        "eye": [
          63.688509965967356,
          27.192799305362843,
          -984.5453448146703
        ],
        "lookAt": [
          73.34776337204205,
          24.610729115803988,
          -984.3675402411845
        ],
        "up": [
          0.2580039086008766,
          0.9660570666078859,
          0.012950953773263751
        ],
        "fovYDegrees": 29.11956666641885
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8935",
        "timeMs": 4151,
        "eye": [
          63.683915450131835,
          27.19264962082167,
          -984.5454889923734
        ],
        "lookAt": [
          73.34319039662473,
          24.61063771229546,
          -984.367663200421
        ],
        "up": [
          0.2580010280457236,
          0.9660599105974423,
          0.012795259427277342
        ],
        "fovYDegrees": 29.128470194288262
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k8984",
        "timeMs": 4200,
        "eye": [
          63.463836613263915,
          27.185269784183365,
          -984.5560899104422
        ],
        "lookAt": [
          73.1240362431995,
          24.606720765284955,
          -984.3782470952075
        ],
        "up": [
          0.2578016677719327,
          0.9661835230196791,
          0.005263073179358821
        ],
        "fovYDegrees": 29.387119582519873
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9034",
        "timeMs": 4250,
        "eye": [
          63.366073842625276,
          27.186613692242272,
          -984.5571382799601
        ],
        "lookAt": [
          73.02634559182441,
          24.60830267062068,
          -984.3797629507577
        ],
        "up": [
          0.25778765081403593,
          0.9661900034454126,
          0.004733321237183073
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9084",
        "timeMs": 4300,
        "eye": [
          63.271669425918226,
          27.19398382343151,
          -984.5574102363009
        ],
        "lookAt": [
          72.93197531175431,
          24.615672801809918,
          -984.3819039177104
        ],
        "up": [
          0.25778856176185133,
          0.9661900034454126,
          0.00468344605070114
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9134",
        "timeMs": 4350,
        "eye": [
          63.17455241464303,
          27.205756032701817,
          -984.5576171124503
        ],
        "lookAt": [
          72.83491207638113,
          24.627445011080226,
          -984.3850961524249
        ],
        "up": [
          0.25778999679016196,
          0.9661900034454126,
          0.004603780737826993
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9184",
        "timeMs": 4400,
        "eye": [
          63.07805842911205,
          27.22061530206961,
          -984.55776764522
        ],
        "lookAt": [
          72.7384846513686,
          24.64230428044802,
          -984.38901492636
        ],
        "up": [
          0.2577917729803288,
          0.9661900034454126,
          0.004503223935392498
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9234",
        "timeMs": 4450,
        "eye": [
          62.98552308963758,
          27.237246613551292,
          -984.5578705714214
        ],
        "lookAt": [
          72.64602206680979,
          24.6589355919297,
          -984.3933355057386
        ],
        "up": [
          0.2577937144701006,
          0.9661900034454126,
          0.004390674419942896
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9284",
        "timeMs": 4500,
        "eye": [
          62.900282016532145,
          27.254334949163287,
          -984.5579346278663
        ],
        "lookAt": [
          72.56085382992109,
          24.6760239275417,
          -984.3977331480361
        ],
        "up": [
          0.25779565812941896,
          0.9661900034454126,
          0.004275031201455512
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9334",
        "timeMs": 4550,
        "eye": [
          62.8256708301082,
          27.270565290921986,
          -984.5579685513665
        ],
        "lookAt": [
          72.48631002262086,
          24.6922542693004,
          -984.4018831008175
        ],
        "up": [
          0.2577974561643467,
          0.9661900034454126,
          0.004165193554374853
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9384",
        "timeMs": 4600,
        "eye": [
          62.76502515067801,
          27.284622620843816,
          -984.5579810787332
        ],
        "lookAt": [
          72.4257212839882,
          24.706311599222225,
          -984.4054606026322
        ],
        "up": [
          0.2577989756489756,
          0.9661900034454126,
          0.004070060993717839
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9434",
        "timeMs": 4650,
        "eye": [
          62.721680598553895,
          27.295191920945175,
          -984.5579809467781
        ],
        "lookAt": [
          72.38241867759643,
          24.716880899323584,
          -984.4081408856392
        ],
        "up": [
          0.25780009498516854,
          0.9661900034454126,
          0.003998533205034368
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9484",
        "timeMs": 4700,
        "eye": [
          62.698972794048586,
          27.30095817324249,
          -984.5579768923133
        ],
        "lookAt": [
          72.3597334437172,
          24.722647151620897,
          -984.409599179705
        ],
        "up": [
          0.257800697290034,
          0.9661900034454126,
          0.00395950993507235
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9502",
        "timeMs": 4718,
        "eye": [
          62.69649222066097,
          27.30160499706388,
          -984.5579761549756
        ],
        "lookAt": [
          72.35725538836007,
          24.72329397544229,
          -984.409762480124
        ],
        "up": [
          0.25780076448453726,
          0.9661900034454126,
          0.003955132531513471
        ],
        "fovYDegrees": 29.39495760413827
      },
      {
        "sceneId": "maharaka.waterpang.source.intro15.video.k9507",
        "timeMs": 4723,
        "eye": [
          62.69637530034117,
          27.3016357421875,
          -984.5579761157427
        ],
        "lookAt": [
          72.3571385876591,
          24.72332472056591,
          -984.4097702380093
        ],
        "up": [
          0.2578007676766064,
          0.9661900034454126,
          0.0039549244627511545
        ],
        "fovYDegrees": 29.39495760413827
      }
    ]
  }
}
```

### cutscenes / cutscene.maharaka.waterpang.source.intro15

```json
{
  "cutsceneId": "cutscene.maharaka.waterpang.source.intro15",
  "displayName": "워터팡 / 도입 15 · 영상 맞춤 카메라·표정",
  "durationMs": 9507,
  "cameraCuts": [
    {
      "cutId": "maharaka.waterpang.source.intro15.cut.0",
      "shotId": "maharaka.waterpang.source.intro15.camera.1",
      "startMs": 0
    },
    {
      "cutId": "maharaka.waterpang.source.intro15.cut.1",
      "shotId": "maharaka.waterpang.source.intro15.camera.2",
      "startMs": 4784
    }
  ],
  "worldInstanceIds": [
    "world.sequence.instance.maharaka.waterpang.source.intro15.stage",
    "world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko",
    "world.sequence.instance.maharaka.waterpang.source.intro15.cannon"
  ]
}
```

## 저장·게시 JSON 변경 행 · Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json

revision: 3 → 4

### objectResources / world.object.maharaka.waterpang.source.intro15.mokomoko

```json
{
  "objectId": "world.object.maharaka.waterpang.source.intro15.mokomoko",
  "displayName": "워터팡 / mokomoko",
  "modelAssetId": "Character/NPC/Maharaka/MN_ISMP_00/MN_ISMP_00.wmodel",
  "modelPreScale": 0.01,
  "animated": true,
  "scale": [
    1.0,
    1.0,
    1.0
  ],
  "anchorKind": "WORLD",
  "materialProfile": {
    "materialName": "mn_ismp_00-2_mi",
    "sourceMaterial": "mn_ismp_00.mat.mn_ismp_00-2_mi",
    "family": "source.character.maharaka-ismp-2.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0,
        0,
        0,
        0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0,
        0,
        0,
        1
      ],
      "constantoutline": [
        0,
        0,
        0,
        0
      ],
      "constantoutline_blink": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "constantoutline_color": [
        0,
        0,
        0,
        1
      ],
      "diffusecolor": [
        1,
        1,
        1,
        1
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0,
        0,
        0,
        0
      ],
      "fx_color_desaturation_buffsettool": [
        0,
        0,
        0,
        0
      ],
      "fx_color_intensity_actiontool": [
        0,
        0,
        0,
        1
      ],
      "fx_color_intensity_buffsettool": [
        0,
        0,
        0,
        1
      ],
      "hit_color": [
        0,
        0,
        0,
        0
      ],
      "ibl_color_bottom": [
        1,
        1,
        1,
        1
      ],
      "ibl_color_top": [
        1,
        1,
        1,
        1
      ],
      "ibl_exposer": [
        5,
        5,
        5,
        5
      ],
      "ibl_intensity": [
        1,
        1,
        1,
        1
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80,
        80,
        80,
        80
      ],
      "metalicness_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "normaltex_intensity": [
        1,
        1,
        1,
        1
      ],
      "opacity_intensity": [
        0,
        0,
        0,
        0
      ],
      "orennayar": [
        1,
        1,
        1,
        1
      ],
      "orennayar_brightness": [
        1,
        1,
        1,
        1
      ],
      "pbr_specular_intensity": [
        10,
        10,
        10,
        10
      ],
      "pbr_specular_power": [
        6,
        6,
        6,
        6
      ],
      "roughness_power": [
        3,
        3,
        3,
        3
      ],
      "selectioncolor": [
        0,
        0,
        0,
        1
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0,
        0,
        0,
        0
      ],
      "state_noise": [
        1,
        0,
        0,
        1
      ],
      "trans_rim_hard": [
        1,
        1,
        1,
        1
      ],
      "trans_rim_inradius": [
        0,
        0,
        0,
        0
      ],
      "transcolor": [
        0,
        0,
        0,
        1
      ],
      "transcolor_rimlight ": [
        1,
        1,
        1,
        1
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/flat_black.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  }
}
```

### objectResources / world.object.maharaka.waterpang.source.intro15.cannon

```json
{
  "objectId": "world.object.maharaka.waterpang.source.intro15.cannon",
  "displayName": "워터팡 / cannon",
  "modelAssetId": "Character/NPC/Maharaka/MN_ISMP_00-1/MN_ISMP_00-1.wmodel",
  "modelPreScale": 0.01,
  "animated": true,
  "scale": [
    1.0,
    1.0,
    1.0
  ],
  "anchorKind": "WORLD",
  "materialProfile": {
    "materialName": "mn_ismp_00-2_mi",
    "sourceMaterial": "mn_ismp_00.mat.mn_ismp_00-2_mi",
    "family": "source.character.maharaka-ismp-2.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0,
        0,
        0,
        0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0,
        0,
        0,
        1
      ],
      "constantoutline": [
        0,
        0,
        0,
        0
      ],
      "constantoutline_blink": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "constantoutline_color": [
        0,
        0,
        0,
        1
      ],
      "diffusecolor": [
        1,
        1,
        1,
        1
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0,
        0,
        0,
        0
      ],
      "fx_color_desaturation_buffsettool": [
        0,
        0,
        0,
        0
      ],
      "fx_color_intensity_actiontool": [
        0,
        0,
        0,
        1
      ],
      "fx_color_intensity_buffsettool": [
        0,
        0,
        0,
        1
      ],
      "hit_color": [
        0,
        0,
        0,
        0
      ],
      "ibl_color_bottom": [
        1,
        1,
        1,
        1
      ],
      "ibl_color_top": [
        1,
        1,
        1,
        1
      ],
      "ibl_exposer": [
        5,
        5,
        5,
        5
      ],
      "ibl_intensity": [
        1,
        1,
        1,
        1
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80,
        80,
        80,
        80
      ],
      "metalicness_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "normaltex_intensity": [
        1,
        1,
        1,
        1
      ],
      "opacity_intensity": [
        0,
        0,
        0,
        0
      ],
      "orennayar": [
        1,
        1,
        1,
        1
      ],
      "orennayar_brightness": [
        1,
        1,
        1,
        1
      ],
      "pbr_specular_intensity": [
        10,
        10,
        10,
        10
      ],
      "pbr_specular_power": [
        6,
        6,
        6,
        6
      ],
      "roughness_power": [
        3,
        3,
        3,
        3
      ],
      "selectioncolor": [
        0,
        0,
        0,
        1
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0,
        0,
        0,
        0
      ],
      "state_noise": [
        1,
        0,
        0,
        1
      ],
      "trans_rim_hard": [
        1,
        1,
        1,
        1
      ],
      "trans_rim_inradius": [
        0,
        0,
        0,
        0
      ],
      "transcolor": [
        0,
        0,
        0,
        1
      ],
      "transcolor_rimlight ": [
        1,
        1,
        1,
        1
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/NPC/Maharaka/Textures/mn_ismp_00-2_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/flat_black.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  }
}
```

### templates / sequence.maharaka.waterpang.source.intro15.mokomoko

```json
{
  "sequenceId": "sequence.maharaka.waterpang.source.intro15.mokomoko",
  "displayName": "워터팡 / mokomoko 표정",
  "category": "World",
  "durationMs": 9507,
  "interpolation": "LINEAR",
  "tracks": [
    {
      "slotId": "actor",
      "keys": [
        {
          "timeMs": 0,
          "positionOffset": [
            75.179,
            25.19908152,
            -1007.262
          ],
          "rotationQuaternion": [
            0.0,
            -0.7071067811865476,
            0.0,
            0.7071067811865476
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        },
        {
          "timeMs": 9507,
          "positionOffset": [
            75.179,
            25.19908152,
            -1007.262
          ],
          "rotationQuaternion": [
            0.0,
            -0.7071067811865476,
            0.0,
            0.7071067811865476
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        }
      ]
    }
  ],
  "animationTracks": [
    {
      "slotId": "actor",
      "clipName": "idle_normal_1",
      "startMs": 0,
      "playbackRate": 1,
      "loop": true,
      "holdLastFrame": true
    }
  ],
  "materialTracks": [
    {
      "slotId": "actor",
      "materialName": "mn_ismp_00-2_mi",
      "curves": [
        {
          "parameter": "opacity_intensity",
          "keys": [
            {
              "timeMs": 0,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6501,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6602,
              "value": [
                1.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9502,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9507,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            }
          ]
        }
      ]
    }
  ]
}
```

### templates / sequence.maharaka.waterpang.source.intro15.cannon

```json
{
  "sequenceId": "sequence.maharaka.waterpang.source.intro15.cannon",
  "displayName": "워터팡 / cannon 표정",
  "category": "World",
  "durationMs": 9507,
  "interpolation": "LINEAR",
  "tracks": [
    {
      "slotId": "actor",
      "keys": [
        {
          "timeMs": 0,
          "positionOffset": [
            75.05,
            23.66841349,
            -984.32
          ],
          "rotationQuaternion": [
            0.0,
            -0.9275104074026151,
            0.0,
            0.37379733032732426
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        },
        {
          "timeMs": 9507,
          "positionOffset": [
            75.05,
            23.66841349,
            -984.32
          ],
          "rotationQuaternion": [
            0.0,
            -0.9275104074026151,
            0.0,
            0.37379733032732426
          ],
          "scaleMultiplier": [
            1.0,
            1.0,
            1.0
          ],
          "visible": true
        }
      ]
    }
  ],
  "animationTracks": [
    {
      "slotId": "actor",
      "clipName": "idle_normal_1",
      "startMs": 0,
      "playbackRate": 1,
      "loop": true,
      "holdLastFrame": true
    },
    {
      "slotId": "actor",
      "clipName": "att_battle_2_01",
      "startMs": 8101,
      "playbackRate": 1,
      "loop": false,
      "holdLastFrame": true
    },
    {
      "slotId": "actor",
      "clipName": "att_battle_2_02",
      "startMs": 9101,
      "playbackRate": 1,
      "loop": true,
      "holdLastFrame": true
    }
  ],
  "materialTracks": [
    {
      "slotId": "actor",
      "materialName": "mn_ismp_00-2_mi",
      "curves": [
        {
          "parameter": "opacity_intensity",
          "keys": [
            {
              "timeMs": 0,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6501,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 6602,
              "value": [
                1.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9502,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            },
            {
              "timeMs": 9507,
              "value": [
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "interpolation": "CONSTANT"
            }
          ]
        }
      ]
    }
  ]
}
```

### instances / world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko

```json
{
  "instanceId": "world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko",
  "templateId": "sequence.maharaka.waterpang.source.intro15.mokomoko",
  "enabled": true,
  "startDelayMs": 0,
  "playbackSpeed": 1,
  "anchorKind": "WORLD",
  "position": [
    0.0,
    0.0,
    0.0
  ],
  "motionEnd": "STOP",
  "bindings": [
    {
      "slotId": "actor",
      "targetKind": "OBJECT_RESOURCE",
      "targetId": "world.object.maharaka.waterpang.source.intro15.mokomoko",
      "previewNpcPlacementId": "npc.maharaka.source57009.actor100"
    }
  ]
}
```

### instances / world.sequence.instance.maharaka.waterpang.source.intro15.cannon

```json
{
  "instanceId": "world.sequence.instance.maharaka.waterpang.source.intro15.cannon",
  "templateId": "sequence.maharaka.waterpang.source.intro15.cannon",
  "enabled": true,
  "startDelayMs": 0,
  "playbackSpeed": 1,
  "anchorKind": "WORLD",
  "position": [
    0.0,
    0.0,
    0.0
  ],
  "motionEnd": "STOP",
  "bindings": [
    {
      "slotId": "actor",
      "targetKind": "OBJECT_RESOURCE",
      "targetId": "world.object.maharaka.waterpang.source.intro15.cannon",
      "previewNpcPlacementId": "npc.maharaka.source57009.actor188"
    }
  ]
}
```
