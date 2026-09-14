# G01. 외곽불 6모션 통합 미리보기

현재 저장된 WORLD revision 1699의 D/E/F CW/CCW를 변경 없이 참조한다. 각10개, 36000ms, 기존 위치·크기·공전·emissions를 유지한다.

## 소유자와 연결

`C:/Users/USER/source/졸업팀폴/LostArk/Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`의 독립 Sequence workspace에 한 Sequence를 추가한다. World Object Tool 원본과 제품 Boss Composition/Server/Resources는 수정하지 않는다. MainApp::Begin_KoukuWorldPreview → Level::Debug_BeginCompositionWorldPreview → 기존 CWorldSequencePlayer 6개를 사용한다. WORLD placement override를 넣지 않고 anchor NONE/offset0으로 저장된 instance Map Position을 보존한다.

## 정확한 추가 블록

revision 5→6, nextPatternOrdinal 4→5, nextWorldOrdinal 15→21. worlds 배열 앞에 다음6행을 추가한다.

```json
[
  {
    "worldId": "kakulsaydon.g1.world.15",
    "displayName": "3관문_추가불_D_CW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_d_cw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  },
  {
    "worldId": "kakulsaydon.g1.world.16",
    "displayName": "3관문_추가불_D_CCW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_d_ccw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  },
  {
    "worldId": "kakulsaydon.g1.world.17",
    "displayName": "3관문_추가불_E_CW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_e_cw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  },
  {
    "worldId": "kakulsaydon.g1.world.18",
    "displayName": "3관문_추가불_E_CCW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_e_ccw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  },
  {
    "worldId": "kakulsaydon.g1.world.19",
    "displayName": "3관문_추가불_F_CW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_f_cw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  },
  {
    "worldId": "kakulsaydon.g1.world.20",
    "displayName": "3관문_추가불_F_CCW_외곽공전",
    "sequenceInstanceId": "world.sequence.instance.kouku.g3.fire_hook.extra_fire_f_ccw",
    "positionOffset": [
      0,
      0,
      0
    ],
    "anchorKind": "NONE",
    "anchorPosition": [
      0,
      0,
      0
    ],
    "companionEffectResourceId": ""
  }
]
```

patterns 배열 앞에 다음1행을 추가한다.

```json
{
  "patternId": "KAKULSAYDON_G1_PATTERN_4",
  "actorProfileId": "MN_RPCT_05",
  "gateId": "GATE3",
  "targetBossPlacementId": "boss.kakulsaydon.g3.saydon",
  "displayName": "3관문_외곽불_전체",
  "authoringStatus": "DRAFT",
  "category": "MECHANIC",
  "nextStageOrdinal": 2,
  "nextAnimationOrdinal": 1,
  "nextLogicOccurrenceOrdinal": 1,
  "nextSummonOccurrenceOrdinal": 1,
  "nextWorldOccurrenceOrdinal": 7,
  "nextSceneProfileOccurrenceOrdinal": 1,
  "stages": [
    {
      "stageId": "STAGE_1",
      "actionId": "KAKULSAYDON_G1_PATTERN_4.stage.1",
      "stageKind": "ACTIVE",
      "durationMs": 36000,
      "animationOccurrences": []
    }
  ],
  "logicOccurrences": [],
  "summonOccurrences": [],
  "worldOccurrences": [
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.1",
      "worldId": "kakulsaydon.g1.world.15",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    },
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.2",
      "worldId": "kakulsaydon.g1.world.16",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    },
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.3",
      "worldId": "kakulsaydon.g1.world.17",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    },
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.4",
      "worldId": "kakulsaydon.g1.world.18",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    },
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.5",
      "worldId": "kakulsaydon.g1.world.19",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    },
    {
      "occurrenceId": "KAKULSAYDON_G1_PATTERN_4.world.6",
      "worldId": "kakulsaydon.g1.world.20",
      "startMs": 0,
      "durationMs": 36000,
      "playbackSpeed": 1
    }
  ],
  "sceneProfileOccurrences": [],
  "resetBossToSpawn": false,
  "nextPresentationOccurrenceOrdinal": 1,
  "presentationOccurrences": []
}
```

## 검증과 사용

기존 모든 JSON 값의 semantic equality, WORLD source/runtime 무변경, stable ID/reference/동시0ms/36000ms/60개를 검사한다. 기존 native --kouku-sequence-document-contract와 WorldSequences Validate/Check, diff check를 실행한다. 새 C++/프로젝트 파일/런타임 생성물이 없어 빌드 및 독립 Sequence 게시 단계는 없다. 사용자가 F1 → Sequencer Benchmark → Reload Patterns → GATE3 → 3관문_외곽불_전체 → Play Sequence로 확인한다. 기존 Object preview는 Stop / Restore한다. Complete Play는 해당 Gate 모든 Sequence를 포함하므로 단독 확인에는 Play Sequence를 사용한다. 개별 모션 저장 후 다시 재생하여 반경 변경을 확인한다. Client/UI 실행 및 시각 PASS는 사용자 몫이다.


# G02. Object Resources 안의 묶음으로 교정

G01의 별도 Benchmark 항목은 사용자 의도와 달라 제거한다. objectResources의 모델 없는 motionInstanceIds 묶음으로 기존6개를 참조한다. 동일 CWorldSequencePlayer에6개 독립 instance를 stage하며 새 runtime을 만들지 않는다. Object Detail에서각member반경·위치를 편집하고 Save한다. 실패입력은 codec/publisher가 거부하며 미리보기의 stage 실패는 이전 group을 유지한다. 기존 객체의 모델/좌표/반경/개수는 수정하지 않는다. 새 C++파일과 project등록은 없다.

## Client/Public/WorldSequenceDocument.h 전체 반영 코드

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

struct WORLD_SEQUENCE_MAP_MATERIAL_BINDING
{
    std::string materialName;
    std::string sourceAssetId;
    std::string sourceMaterialName;
    std::string diffuseTextureAssetId;
    bool operator==(const WORLD_SEQUENCE_MAP_MATERIAL_BINDING&) const = default;
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
    // Baked cinematic meshes retain an explicit original actor material owner.
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
	{ return durationMs + (effectTracks.empty() ? 0u : objectMotion.LastEmissionDelayMs()); }
	uint32_t PresentationSpanMs() const noexcept
	{
		uint32_t span = durationMs;
		for (const auto& effect : effectTracks)
			span = (std::max)(span, EffectStartMs(effect) + effect.durationMs);
		return span + (effectTracks.empty() ? 0u : objectMotion.LastEmissionDelayMs());
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

```

## Client/Private/WorldSequenceDocument.cpp 전체 반영 코드

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

	bool_t Is_ValidUtf8DisplayText(const std::string& value)
	{
		for (size_t offset = 0u; offset < value.size();)
		{
			const uint8_t first = static_cast<uint8_t>(value[offset]);
			if (first < 0x80u)
			{
				if (first < 0x20u || 0x7fu == first)
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
            profile.family != "source.character.monster-pbr-masked.v1" ||
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
			key.scaleMultiplier.x < MIN_SCALE ||
			key.scaleMultiplier.y < MIN_SCALE ||
			key.scaleMultiplier.z < MIN_SCALE)
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
			  "templates", "instances" }, { "objectResources" }))
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
				"animated", "scale" }, { "diffuseTextureAssetId", "sequenceInstanceId", "anchorKind", "defaultMotionInstanceId", "anchorBossArchetypeId", "anchorBone", "materialProfile", "materialSourceModelAssetId", "mapMaterialBindings", "motionInstanceIds" }) ||
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
			object.modelAssetId = row.Find("modelAssetId")->Get_String();
			object.animated = row.Find("animated")->Get_Boolean();
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
            if (const auto* bindings = row.Find("mapMaterialBindings"))
            {
                if (!bindings->Is_Array() || bindings->Get_Array().size() > 64u)
                { outStatus = "Invalid world object map material bindings"; return false; }
                for (const auto& binding : bindings->Get_Array())
                {
                    if (!Is_ObjectShape(binding, { "materialName", "sourceAssetId", "sourceMaterialName" }, { "diffuseTextureAssetId" }) ||
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
				  "interpolation", "tracks", "animationTracks" }, { "objectMotion", "effectTracks" });
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
						  "holdLastFrame" }, { "startMs", "displayName" }))
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
				parsedTrack.slotId = slotId->Get_String();
				parsedTrack.clipName = clipName->Get_String();
				if (nullptr != trackDisplayName)
					parsedTrack.displayName = trackDisplayName->Get_String();
				parsedTrack.loop = loop->Get_Boolean();
				parsedTrack.holdLastFrame = holdLastFrame->Get_Boolean();
				parsedTemplate.animationTracks.push_back(std::move(parsedTrack));
			}
		}
		if (const auto* effects = templateValue.Find("effectTracks"))
		{
			if (parsedFormatVersion < 3u || !effects->Is_Array() || effects->Get_Array().size() > MAX_TRACK_COUNT)
			{ outStatus = "World Object effectTracks must be a bounded v3 array"; return false; }
			for (const auto& row : effects->Get_Array())
			{
				WORLD_SEQUENCE_EFFECT_TRACK effect;
				if (!Is_ExactObject(row, { "effectTrackId", "slotId", "resourceKind", "resourceId",
					"timing", "startMs", "durationMs", "positionOffset", "rotationDegrees", "scale" }))
				{ outStatus = "World Object effect track shape is invalid"; return false; }
				for (const char* key : { "effectTrackId", "slotId", "resourceKind", "resourceId", "timing" })
					if (!row.Find(key)->Is_String())
					{ outStatus = "World Object effect identity must be text"; return false; }
				effect.effectTrackId = row.Find("effectTrackId")->Get_String();
				effect.slotId = row.Find("slotId")->Get_String();
				effect.resourceKind = row.Find("resourceKind")->Get_String();
				effect.resourceId = row.Find("resourceId")->Get_String();
				effect.timing = row.Find("timing")->Get_String();
				if (!Read_Uint32(row.Find("startMs"), effect.startMs, MAX_DURATION_MS) ||
					!Read_Uint32(row.Find("durationMs"), effect.durationMs, MAX_DURATION_MS) ||
					!Read_Float3(row.Find("positionOffset"), effect.positionOffset) ||
					!Read_Float3(row.Find("rotationDegrees"), effect.rotationDegrees) ||
					!Read_Float3(row.Find("scale"), effect.scale))
				{ outStatus = "World Object effect timing or transform is invalid"; return false; }
				parsedTemplate.effectTracks.push_back(std::move(effect));
			}
		}

		staged.m_Templates.push_back(std::move(parsedTemplate));
	}

	for (const DATA_JSON_VALUE& instanceValue : instances->Get_Array())
	{
		if (!Is_ObjectShape(instanceValue,
			{ "instanceId", "templateId", "enabled", "startDelayMs",
			  "playbackSpeed", "bindings" }, { "anchorKind", "position", "motionEnd", "nextMotionId", "walkableSurface" }))
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
				Is_ExactObject(bindingValue,
					{ "slotId", "targetKind", "targetId" });
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
		<< "  \"revision\": " << m_iRevision << ",\n"
		<< "  \"objectResources\": [";
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
        if (!object.materialSourceModelAssetId.empty())
            output << ",\n      \"materialSourceModelAssetId\": \"" << CDataJson::Escape(object.materialSourceModelAssetId) << "\"";
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
			output << ", \"startMs\": " << track.startMs
					<< ", \"playbackRate\": " << track.playbackRate
				<< ", \"loop\": " << (track.loop ? "true" : "false")
				<< ", \"holdLastFrame\": "
				<< (track.holdLastFrame ? "true" : "false") << " }";
		}
		output << (value.animationTracks.empty() ? "]" : "\n      ]");
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
					<< "\", \"startMs\": " << effect.startMs << ", \"durationMs\": " << effect.durationMs
					<< ", \"positionOffset\": [" << effect.positionOffset.x << ", " << effect.positionOffset.y << ", " << effect.positionOffset.z
					<< "], \"rotationDegrees\": [" << effect.rotationDegrees.x << ", " << effect.rotationDegrees.y << ", " << effect.rotationDegrees.z
					<< "], \"scale\": [" << effect.scale.x << ", " << effect.scale.y << ", " << effect.scale.z << "] }";
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
				<< CDataJson::Escape(binding.targetId) << "\" }";
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
	std::unordered_set<std::string> objectIds;
	for (const WORLD_SEQUENCE_OBJECT_RESOURCE& object : m_ObjectResources)
	{
		if (!object.motionInstanceIds.empty())
		{
			if (!Is_ValidStableId(object.objectId) || !objectIds.insert(object.objectId).second ||
				object.displayName.empty() || object.displayName.size() > 128u || !Is_ValidUtf8DisplayText(object.displayName) ||
				object.motionInstanceIds.size() > 32u || object.anchorKind != "WORLD" ||
				!object.modelAssetId.empty() || !object.sequenceInstanceId.empty() || !object.defaultMotionInstanceId.empty() ||
				object.animated || !object.diffuseTextureAssetId.empty() || object.materialProfile ||
				!object.materialSourceModelAssetId.empty() || !object.mapMaterialBindings.empty() ||
				!object.anchorBossArchetypeId.empty() || !object.anchorBone.empty() ||
				!std::isfinite(object.modelPreScale) || object.modelPreScale < MIN_SCALE || object.modelPreScale > MAX_COMPONENT ||
				!Is_BoundedFloat3(object.scale) || object.scale.x != 1.f || object.scale.y != 1.f || object.scale.z != 1.f)
			{ outStatus = "Invalid model-less Object group: " + object.objectId; return false; }
			std::unordered_set<std::string> members;
			for (const auto& id : object.motionInstanceIds)
			{
				const auto* instance = Find_Instance(id);
				if (!Is_ValidStableId(id) || !members.insert(id).second || !instance || instance->anchorKind != "WORLD" ||
					instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP || instance->bindings.size() != 1u ||
					instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				{ outStatus = "Object group needs unique existing Map Object motions ending with Stop: " + id; return false; }
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
        if ((!object.materialSourceModelAssetId.empty() && (alias || !Is_ResourcePath(object.materialSourceModelAssetId, true))) ||
            object.mapMaterialBindings.size() > 64u || (alias && !object.mapMaterialBindings.empty()))
        { outStatus = "Invalid world object material source: " + object.objectId; return false; }
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
			value.tracks.size() + value.animationTracks.size() + value.effectTracks.size() > MAX_TRACK_COUNT)
		{
			outStatus = "Invalid or duplicate world sequence template: " +
				value.sequenceId;
			return false;
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
				(effect.resourceKind != "LEAF" && effect.resourceKind != "GROUP") ||
				(effect.timing != "TIME" && effect.timing != "MOTION_END") ||
				(effect.timing == "MOTION_END" && effect.startMs != 0u) ||
				effect.startMs > value.durationMs || effect.durationMs == 0u || effect.durationMs > MAX_DURATION_MS ||
				!Is_BoundedFloat3(effect.positionOffset) || !Is_BoundedFloat3(effect.rotationDegrees) ||
				!Is_BoundedFloat3(effect.scale) || effect.scale.x < MIN_SCALE || effect.scale.y < MIN_SCALE || effect.scale.z < MIN_SCALE ||
				value.PresentationSpanMs() > MAX_DURATION_MS)
			{ outStatus = "Invalid World Object effect track: " + value.sequenceId + "/" + effect.effectTrackId; return false; }
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
			bool_t mirrored = false;
			for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = track.keys[keyIndex];
				const bool_t keyMirrored = key.scaleMultiplier.x *
					key.scaleMultiplier.y * key.scaleMultiplier.z < 0.f;
				if (!Is_FiniteTransform(key) || keyMirrored ||
					key.timeMs > value.durationMs ||
					(0u != keyIndex &&
						track.keys[keyIndex - 1u].timeMs >= key.timeMs) ||
					(0u != keyIndex && keyMirrored != mirrored))
				{
					outStatus = "Invalid keyframe in world sequence template: " +
						value.sequenceId + "/" + track.slotId;
					return false;
				}
				mirrored = keyMirrored;
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
				track.startMs >= value.durationMs ||
				(firstOfSlot && 0u != track.startMs) ||
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
		if (!targetTemplate->effectTracks.empty() && (value.bindings.size() != 1u ||
			value.bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE))
		{ outStatus = "Effect lanes require one Object Resource binding: " + value.instanceId; return false; }
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
			const auto transformSlot = std::find_if(targetTemplate->tracks.begin(),
				targetTemplate->tracks.end(),
				[&binding](const WORLD_SEQUENCE_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
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
				if (!object || object->modelAssetId.empty() || !object->sequenceInstanceId.empty() ||
					(hasAnimationSlot && !object->animated) ||
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
		m_ObjectResources.size() != other.m_ObjectResources.size())
	{
		return false;
	}
	for (size_t index = 0u; index < m_ObjectResources.size(); ++index)
	{
		const auto& left = m_ObjectResources[index];
		const auto& right = other.m_ObjectResources[index];
		if (left.objectId != right.objectId || left.displayName != right.displayName ||
			left.anchorKind != right.anchorKind || left.anchorBossArchetypeId != right.anchorBossArchetypeId ||
			left.anchorBone != right.anchorBone ||
			left.modelAssetId != right.modelAssetId || left.diffuseTextureAssetId != right.diffuseTextureAssetId ||
            left.materialProfile != right.materialProfile ||
            left.materialSourceModelAssetId != right.materialSourceModelAssetId || left.mapMaterialBindings != right.mapMaterialBindings ||
			left.modelPreScale != right.modelPreScale || left.animated != right.animated ||
			!sameFloat3(left.scale, right.scale) || left.sequenceInstanceId != right.sequenceInstanceId ||
			left.motionInstanceIds != right.motionInstanceIds ||
			left.defaultMotionInstanceId != right.defaultMotionInstanceId) return false;
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
				a.resourceId != b.resourceId || a.timing != b.timing || a.startMs != b.startMs || a.durationMs != b.durationMs ||
				!sameFloat3(a.positionOffset, b.positionOffset) || !sameFloat3(a.rotationDegrees, b.rotationDegrees) ||
				!sameFloat3(a.scale, b.scale)) return false;
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
			left.motionEnd != right.motionEnd || left.nextMotionId != right.nextMotionId)
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
					right.bindings[bindingIndex].targetId)
			{
				return false;
			}
		}
	}
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

```

## Client/Public/WorldObjectTool.h 전체 반영 코드

```cpp
#pragma once

#include "WorldSequenceDocument.h"
#include "PhysicalResourceCatalog.h"
#include "CompositionResourceTree.h"
#include "EffectV2_Catalog.h"

#include <array>
#include <filesystem>

#ifdef _DEBUG
namespace Client
{
class CLevel_KakulSaydonArena;

class CWorldObjectTool final
{
public:
    ~CWorldObjectTool();
    void Open();
    void Deactivate();
    void Update(f32_t seconds, bool_t active);
    void Render();
    bool_t Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    // Only a saved document may become a Workbench resource inventory.
    const CWorldSequenceDocument* Get_SavedDocument() const { return m_Ready ? &m_SavedDocument : nullptr; }
    uint64_t Get_SavedGeneration() const { return m_SavedGeneration; }

private:
    bool Load_Source();
    bool Save_Source();
    bool Matches_SourceBaseline();
    void Start_Publish();
    void Poll_Publish();
    void Mark_Dirty();
    void Stop_Preview();
    bool Begin_Preview();
    const WORLD_SEQUENCE_INSTANCE* Preview_Instance() const;
    void Seek(f32_t clockMs);
    f32_t SpanMs() const;
    f32_t PreviewSpanMs() const;
    void Select_Object(const std::string& id);
    void Select_State(const std::string& id);
    std::vector<std::string> StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const;
    bool Create_Object();
    void Create_State();
    bool Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& stateName,
        WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance);
    void Change_ResourceAnchor(WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind);
    void Render_WindowMenu();
    void Render_Toolbar();
    void Render_Resources();
    void Refresh_AnimationResources();
    void Render_AnimationResources();
    void Render_EffectResources();
    bool Append_SelectedEffect();
    void Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Append_SelectedAnimation();
    bool Stage_SelectedModel(CWorldSequenceDocument& candidate);
    bool Assign_SelectedModel();
    void Render_Detail();
    void Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    const WORLD_SEQUENCE_OBJECT_RESOURCE* Preview_Group() const;
    void Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_PhysicalResources();
    void Rebuild_PhysicalTree();

    bool m_Open = false;
    bool m_ResourcesOpen = true;
    bool m_SequencerOpen = true;
    bool m_DetailOpen = true;
    bool m_ResetLayoutRequested = false;
    bool m_InteractionRequested = false;
    bool m_PreviewAtCharacter = true;
    bool m_Ready = false;
    bool m_Dirty = false;
    bool m_PreviewActive = false;
    bool m_PreviewDirty = false;
    bool m_Playing = false;
    CLevel_KakulSaydonArena* m_PreviewLevel = nullptr;
    f32_t m_ClockMs = 0.f;
    f32_t m_VerticalArcHeight = 2.f;
    // Distribute on Ring preset inputs; rows are the saved truth, not these.
    int m_RingCount = 10;
    f32_t m_RingStartDegrees = 0.f;
    f32_t m_Zoom = 100.f;
    CWorldSequenceDocument m_Document;
    CWorldSequenceDocument m_SavedDocument;
    WORLD_SEQUENCE_PLACEMENT_MAP m_MapTargets;
    WORLD_SEQUENCE_DEPLOY_MAP m_DeployTargets;
    std::filesystem::path m_SourcePath;
    std::filesystem::path m_PlacementPath;
    std::filesystem::path m_DeployPath;
    std::string m_SourceBytes;
    std::string m_PlacementBytes;
    std::string m_DeployBytes;
    std::string m_Status;
    std::string m_PreviewStatus;
    std::string m_SelectedObject;
    std::string m_SelectedInstance;
    size_t m_SelectedTrack = 0;
    size_t m_SelectedKey = 0;
    uint64_t m_SavedGeneration = 0;
    std::array<char, 128> m_NewObjectName{};
    int m_NewObjectAnchor = 0;
    bool m_CreateObjectFailed = false;
    std::array<char, 128> m_NewStateName{};
    std::string m_PristinePatternId;
    struct ANIMATION_RESOURCE
    {
        std::string clipName;
        double durationMs = 0.;
    };
    std::vector<ANIMATION_RESOURCE> m_AnimationResources;
    std::string m_AnimationObjectId;
    std::string m_AnimationModelAssetId;
    std::string m_AnimationCandidateModelAssetId;
    std::string m_AnimationCandidateObjectId;
    bool m_AnimationCatalogReady = false;
    std::string m_AnimationResourceStatus;
    std::string m_SelectedAnimationClip;
    std::array<char, 256> m_AnimationSearch{};
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> m_EffectResources;
    std::string m_SelectedEffectResource;
    EFFECT_V2_RESOURCE_KIND m_SelectedEffectKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    std::string m_EffectResourceStatus;
    bool m_EffectInventoryLoaded = false;
    size_t m_SelectedEffectRow = 0;
    std::array<char, 256> m_EffectSearch{};
    std::array<char, 256> m_ObjectSearch{};
    std::array<char, 256> m_PhysicalSearch{};
    std::vector<PHYSICAL_RESOURCE_ASSET> m_PhysicalAssets;
    COMPOSITION_RESOURCE_TREE_NODE m_PhysicalTree;
    std::string m_PhysicalStatus;
    std::string m_SelectedPhysical;
    bool m_PhysicalScanned = false;
    bool m_PhysicalScanRunning = false;
    CPhysicalResourceScan m_PhysicalScan;
    int m_PhysicalSlot = 0;
    HANDLE m_PublishProcess = nullptr;
    std::filesystem::path m_PublishLog;
};
}
#endif

```

## Client/Private/WorldObjectTool.cpp 전체 반영 코드

```cpp
#include "imgui.h"
#include "WorldObjectTool.h"

#ifdef _DEBUG
#include "CompositionTimeline.h"
#include "Animation.h"
#include "BinaryAsset/WModelDecoder.h"
#include "RuntimeAssetRoot.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldSequencePlayer.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>

namespace
{
constexpr const char* AREA_ID = "LV_LUT_MIDNIGHTC_ED";

bool ReadSource(const std::filesystem::path& path, std::string& bytes,
    std::string& status, const bool optional = false)
{
    std::error_code error;
    if (optional && !std::filesystem::exists(path, error) && !error)
    { bytes.clear(); return true; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 64u * 1024u * 1024u)
    { status = "Cannot read bounded authoring source: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot open source: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (input.bad() || bytes.size() != size)
    { status = "Source changed or failed while reading: " + path.string(); return false; }
    return true;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool EditText(const char* label, std::string& value, const size_t maxBytes = 128)
{
    char text[1024]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, (std::min)(sizeof(text), maxBytes + 1))) return false;
    value = text;
    return true;
}

bool EditUInt(const char* label, uint32_t& value, const int maximum, const int minimum = 0)
{
    int number = static_cast<int>((std::min)(value, static_cast<uint32_t>(INT_MAX)));
    if (!ImGui::DragInt(label, &number, 1.f, minimum, maximum, "%d", ImGuiSliderFlags_AlwaysClamp)) return false;
    value = static_cast<uint32_t>((std::clamp)(number, minimum, maximum));
    return true;
}

const char* MotionEndLabel(const Client::WORLD_SEQUENCE_MOTION_END motionEnd)
{
    switch (motionEnd)
    {
    case Client::WORLD_SEQUENCE_MOTION_END::STOP: return "Stop";
    case Client::WORLD_SEQUENCE_MOTION_END::HOLD: return "Hold Last Pose";
    case Client::WORLD_SEQUENCE_MOTION_END::LOOP: return "Loop";
    case Client::WORLD_SEQUENCE_MOTION_END::NEXT: return "Play Motion";
    default: return "Unknown";
    }
}

float3_t QuaternionEuler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
}

using namespace Client;

CWorldObjectTool::~CWorldObjectTool()
{
    Stop_Preview();
    if (m_PublishProcess) CloseHandle(m_PublishProcess);
}

void CWorldObjectTool::Open()
{
    m_Open = true;
    m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
    if (!m_Ready) Load_Source();
}

bool CWorldObjectTool::Consume_InteractionRequest()
{
    const bool requested = m_InteractionRequested;
    m_InteractionRequested = false;
    return requested;
}

void CWorldObjectTool::Deactivate()
{
    Stop_Preview();
}

bool CWorldObjectTool::Load_Source()
{
    const auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { m_Status = "Enter KoukuSaydon, then Reload Source to edit and preview world objects."; return false; }
    const auto directory = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / AREA_ID);
    const auto sourcePath = directory / (std::string(AREA_ID) + ".worldsequences.json");
    const auto placementPath = directory / (std::string(AREA_ID) + ".mapplacements");
    const auto deployPath = directory / (std::string(AREA_ID) + ".deployplacements");
    std::string sourceBefore, mapBefore, deployBefore;
    if (!ReadSource(sourcePath, sourceBefore, m_Status) ||
        !ReadSource(placementPath, mapBefore, m_Status) ||
        !ReadSource(deployPath, deployBefore, m_Status, true)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP map;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    level->Get_WorldObjectValidationTargets(map, deploy);
    CWorldSequenceDocument staged;
    if (!staged.Load(sourcePath, AREA_ID, map, deploy, m_Status)) return false;
    std::string sourceAfter, mapAfter, deployAfter;
    if (!ReadSource(sourcePath, sourceAfter, m_Status) ||
        !ReadSource(placementPath, mapAfter, m_Status) ||
        !ReadSource(deployPath, deployAfter, m_Status, true)) return false;
    if (sourceBefore != sourceAfter || mapBefore != mapAfter || deployBefore != deployAfter)
    { m_Status = "Linked authoring source changed during Reload; existing draft preserved."; return false; }
    Stop_Preview();
    m_Document = std::move(staged);
    m_SavedDocument = m_Document;
    m_MapTargets = std::move(map);
    m_DeployTargets = std::move(deploy);
    m_SourcePath = sourcePath; m_PlacementPath = placementPath; m_DeployPath = deployPath;
    m_SourceBytes = std::move(sourceAfter); m_PlacementBytes = std::move(mapAfter); m_DeployBytes = std::move(deployAfter);
    m_Ready = true; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_AnimationObjectId.clear();
    m_AnimationCandidateObjectId.clear();
    m_AnimationCandidateModelAssetId.clear();
    if (!m_Document.Find_ObjectResource(m_SelectedObject))
        m_SelectedObject = m_Document.Get_ObjectResources().empty() ? "" : m_Document.Get_ObjectResources().front().objectId;
    Select_Object(m_SelectedObject);
    m_Status = "Source loaded. Save stores Object edits and applies them for the next play.";
    return true;
}

bool CWorldObjectTool::Matches_SourceBaseline()
{
    std::string source, map, deploy;
    if (!ReadSource(m_SourcePath, source, m_Status) ||
        !ReadSource(m_PlacementPath, map, m_Status) ||
        !ReadSource(m_DeployPath, deploy, m_Status, true)) return false;
    if (source != m_SourceBytes || map != m_PlacementBytes || deploy != m_DeployBytes)
    { m_Status = "Save conflict: linked source changed on disk. Draft preserved; Reload Source before saving."; return false; }
    return true;
}

bool CWorldObjectTool::Save_Source()
{
    if (!m_Ready || m_PublishProcess || !Matches_SourceBaseline()) return false;
    auto stagedPath = m_SourcePath;
    stagedPath += L".world-object-" + std::to_wstring(GetCurrentProcessId()) + L".stage";
    // The document owns codec/validation. A separate staging destination permits
    // a readback and the final linked-source CAS immediately before promotion.
    if (!m_Document.Save(stagedPath, m_MapTargets, m_DeployTargets, m_Status)) return false;
    CWorldSequenceDocument verified;
    std::string stagedBytes;
    const bool ready = verified.Load(stagedPath, AREA_ID, m_MapTargets, m_DeployTargets, m_Status) &&
        m_Document.Is_Equivalent(verified) && ReadSource(stagedPath, stagedBytes, m_Status) && Matches_SourceBaseline();
    if (!ready || !MoveFileExW(stagedPath.c_str(), m_SourcePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        std::error_code ignored; std::filesystem::remove(stagedPath, ignored);
        if (ready) m_Status = "Atomic source replacement failed; draft preserved.";
        else if (m_Status.empty()) m_Status = "Staged source readback differs; draft preserved.";
        return false;
    }
    m_SourceBytes = std::move(stagedBytes); m_Document = std::move(verified);
    m_SavedDocument = m_Document; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_Status = "Saved; applying World Object runtime data.";
    Start_Publish();
    return true;
}

void CWorldObjectTool::Start_Publish()
{
    if (!m_Ready || m_Dirty || m_PublishProcess) return;
    if (!Matches_SourceBaseline())
    { m_Status = "Saved; apply stopped because linked source changed. Reload Source before retrying."; return; }
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    if (!std::filesystem::is_regular_file(script)) { m_Status = "Saved; apply failed: publisher is missing. Save to retry."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { m_Status = "Saved; apply failed: log folder is unavailable. Save to retry."; return; }
    m_PublishLog = std::filesystem::path(temporary) / (L"LostArk-WorldObject-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(m_PublishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == log) { m_Status = "Saved; apply failed: cannot create log. Save to retry."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == input) { CloseHandle(log); m_Status = "Saved; apply failed: cannot prepare input. Save to retry."; return; }
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
        script.wstring() + L"\" -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = !!CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
    CloseHandle(log); CloseHandle(input);
    if (!started) { m_Status = "Saved; apply failed: cannot start publisher. Save to retry."; return; }
    CloseHandle(process.hThread); m_PublishProcess = process.hProcess;
    m_Status = "Saved; applying World Object runtime data. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Poll_Publish()
{
    if (!m_PublishProcess || WaitForSingleObject(m_PublishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_PublishProcess, &code);
    CloseHandle(m_PublishProcess); m_PublishProcess = nullptr;
    if (code != 0)
    { m_Status = "Saved; apply failed (" + std::to_string(code) + "). Previous runtime preserved. Save to retry. Log: " + m_PublishLog.string(); return; }
    ++m_SavedGeneration;
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        std::string status;
        if (!level->Reload_WorldObjectRuntime(status))
        { m_Status = "Saved and runtime files applied; next-play reload pending: " + status; return; }
    }
    m_Status = "Saved and applied for the next play. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Mark_Dirty()
{
    m_PristinePatternId.clear();
    m_Document.Touch(); m_Dirty = true; m_PreviewDirty = m_PreviewActive;
}

void CWorldObjectTool::Stop_Preview()
{
    if (m_PreviewActive && m_PreviewLevel && m_PreviewLevel == CLevel_KakulSaydonArena::Get_Active())
        m_PreviewLevel->Debug_StopWorldObjectPreview();
    m_PreviewLevel = nullptr; m_PreviewActive = false; m_Playing = false; m_PreviewDirty = false;
}

const WORLD_SEQUENCE_INSTANCE* CWorldObjectTool::Preview_Instance() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance.empty() && resource ?
        resource->defaultMotionInstanceId : m_SelectedInstance);
    return instance && instance->enabled ? instance : nullptr;
}

const WORLD_SEQUENCE_OBJECT_RESOURCE* CWorldObjectTool::Preview_Group() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    return m_SelectedInstance.empty() && resource && !resource->motionInstanceIds.empty() ? resource : nullptr;
}

bool CWorldObjectTool::Begin_Preview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) { m_Status = "World object preview requires the active KoukuSaydon arena."; return false; }
    if (const auto* group = Preview_Group())
    {
        if (SpanMs() <= 0.f)
        {
            Stop_Preview(); m_ClockMs = 0.f;
            m_Status = m_PreviewStatus = "All group motions are disabled.";
            return false;
        }
        if (!level->Debug_BeginWorldObjectPreview(m_Document, group->objectId, m_Status, m_PreviewAtCharacter)) return false;
        m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
        return true;
    }
    const auto* instance = Preview_Instance();
    if (!instance) { m_Status = "Choose an enabled Default Motion or select a connected Motion."; return false; }
    if (!level->Debug_BeginWorldObjectPreview(m_Document, instance->instanceId, m_Status, m_PreviewAtCharacter)) return false;
    m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
    return true;
}

f32_t CWorldObjectTool::SpanMs() const
{
    if (const auto* group = Preview_Group())
    {
        float span = 0.f;
        for (const auto& id : group->motionInstanceIds)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance && instance->enabled ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (sequence) span = (std::max)(span, static_cast<float>(instance->startDelayMs) +
                static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed));
        }
        return span;
    }
    const auto* instance = Preview_Instance();
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    return !sequence ? 0.f : static_cast<float>(instance->startDelayMs) +
        static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
}

f32_t CWorldObjectTool::PreviewSpanMs() const
{
    const auto* instance = Preview_Instance();
    if (instance && (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP ||
        instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        return (std::max)(SpanMs(), static_cast<float>(CWorldSequenceDocument::MAX_DURATION_MS));
    return SpanMs();
}

void CWorldObjectTool::Seek(const f32_t clockMs)
{
    m_ClockMs = (std::clamp)(clockMs, 0.f, PreviewSpanMs());
    if ((!m_PreviewActive || m_PreviewDirty) && !Begin_Preview()) { m_Playing = false; return; }
    if (!m_PreviewLevel->Debug_SampleWorldObjectPreview(m_ClockMs, m_PreviewStatus))
    { m_Status = m_PreviewStatus; Stop_Preview(); }
}

void CWorldObjectTool::Update(const f32_t seconds, const bool_t active)
{
    Poll_Publish();
    if (m_PhysicalScanRunning && m_PhysicalScan.Advance())
    {
        if (m_PhysicalScan.Commit(m_PhysicalAssets, m_PhysicalStatus)) Rebuild_PhysicalTree();
        m_PhysicalScanRunning = false;
    }
    if (!active || !m_Open || (m_PreviewLevel && m_PreviewLevel != CLevel_KakulSaydonArena::Get_Active()))
    { Stop_Preview(); return; }
    if (m_PreviewActive && m_PreviewDirty) Seek(m_ClockMs);
    if (!m_Playing || !m_PreviewActive) return;
    const float span = PreviewSpanMs();
    m_ClockMs += (std::max)(0.f, seconds) * 1000.f;
    if (span <= 0.f) { Stop_Preview(); return; }
    if (m_ClockMs >= span) { m_ClockMs = span; m_Playing = false; }
    Seek(m_ClockMs);
}

std::vector<std::string> CWorldObjectTool::StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const
{
    if (!resource.motionInstanceIds.empty()) return resource.motionInstanceIds;
    if (!resource.sequenceInstanceId.empty()) return {resource.sequenceInstanceId};
    std::vector<std::string> ids;
    for (const auto& instance : m_Document.Get_Instances())
        if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
            return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
        })) ids.push_back(instance.instanceId);
    return ids;
}

void CWorldObjectTool::Select_Object(const std::string& id)
{
    Stop_Preview(); m_SelectedObject = id; m_SelectedInstance.clear(); m_ClockMs = 0.f;
    if (Preview_Group()) m_PreviewAtCharacter = false;
    m_SelectedTrack = 0; m_SelectedKey = 0;
}

void CWorldObjectTool::Select_State(const std::string& id)
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto motions = resource ? StateIds(*resource) : std::vector<std::string>{};
    if (std::find(motions.begin(), motions.end(), id) == motions.end())
    { m_Status = "The selected motion does not belong to this object."; return; }
    Stop_Preview(); m_SelectedInstance = id; m_SelectedTrack = 0; m_SelectedKey = 0; m_ClockMs = 0.f;
}

bool CWorldObjectTool::Create_Object()
{
    if (!m_NewObjectName[0]) { m_Status = "Enter an object name."; return false; }
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        resource.objectId = "world.object.resource." + std::to_string(index);
        if (!m_Document.Find_ObjectResource(resource.objectId)) break;
    }
    resource.displayName = m_NewObjectName.data();
    resource.anchorKind = m_NewObjectAnchor == 2 ? "BOSS" : m_NewObjectAnchor == 1 ? "PLAYER" : "WORLD";
    if (m_Document.Get_ObjectResources().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World object resource capacity reached."; return false; }
    m_Document.Get_ObjectResources().push_back(resource);
    Mark_Dirty(); Select_Object(resource.objectId);
    m_Status = "Object created. Assign its shared model, then Create Motion in Object Detail. Assign Model before Save.";
    m_NewObjectName[0] = 0;
    m_NewStateName[0] = 0;
    return true;
}

void CWorldObjectTool::Create_State()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty() || !m_NewStateName[0]) return;
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(*resource, m_NewStateName.data(), sequence, instance)) return;
    const bool firstMotion = StateIds(*resource).empty();
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(instance);
    if (firstMotion) resource->defaultMotionInstanceId = instance.instanceId;
    Mark_Dirty(); Select_State(instance.instanceId);
    m_PristinePatternId = instance.instanceId;
    m_NewStateName[0] = 0;
    m_Status = "Motion created. Append Clip or edit Transform/Physics, then Save.";
}

bool CWorldObjectTool::Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const std::string& stateName, WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance)
{
    if (m_Document.Get_Templates().size() >= CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        m_Document.Get_Instances().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World sequence document capacity reached."; return false; }
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        sequence.sequenceId = resource.objectId + ".state." + std::to_string(index);
        instance.instanceId = sequence.sequenceId + ".instance";
        if (!m_Document.Find_Template(sequence.sequenceId) && !m_Document.Find_Instance(instance.instanceId)) break;
    }
    sequence.displayName = stateName; sequence.durationMs = 2000;
    WORLD_SEQUENCE_TRACK track; track.slotId = "object"; track.keys.push_back({});
    WORLD_SEQUENCE_TRANSFORM_KEY end; end.timeMs = sequence.durationMs; track.keys.push_back(end);
    sequence.tracks.push_back(track);
    instance.templateId = sequence.sequenceId;
    instance.anchorKind = resource.anchorKind;
    instance.bindings.push_back({"object", WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE, resource.objectId});
    if (instance.anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level)
        { m_Status = "Map state creation requires the active KoukuSaydon arena."; return false; }
        if (!level->Try_Get_AuthoringPreviewPlacement(instance.position, m_Status))
        {
            m_Status = "Map state needs the current character placement: " + m_Status;
            return false;
        }
    }
    return true;
}

void CWorldObjectTool::Change_ResourceAnchor(
    WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind)
{
    if (resource.anchorKind == anchorKind || !resource.sequenceInstanceId.empty()) return;
    float3_t position{};
    if (anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level || !level->Try_Get_AuthoringPreviewPlacement(position, m_Status))
        { m_Status = "Map anchor needs the current character placement: " + m_Status; return; }
    }
    resource.anchorKind = anchorKind;
    if (anchorKind != "BOSS") { resource.anchorBossArchetypeId.clear(); resource.anchorBone.clear(); }
    for (const auto& id : StateIds(resource))
    {
        auto* instance = m_Document.Find_Instance(id);
        if (!instance || instance->anchorKind == anchorKind) continue;
        instance->anchorKind = anchorKind;
        instance->position = position;
    }
    Mark_Dirty();
    m_Status = anchorKind == "BOSS" ?
        "Boss anchor applied to this resource's states. Choose the boss and BODY bone below." : anchorKind == "PLAYER" ?
        "Character anchor applied to this resource's states; offsets start at the character origin." :
        "Map anchor applied to this resource's states at the current character position.";
}

void CWorldObjectTool::Render()
{
    if (!m_Open) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport ? viewport->WorkPos : ImVec2(0.f, 0.f);
    const ImVec2 available = viewport ? viewport->WorkSize : ImVec2(1600.f, 900.f);
    constexpr float margin = 8.f, gap = 8.f;
    const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
    const float height = (std::max)(1.f, available.y - margin * 2.f);
    const float leftWidth = width * .23f, rightWidth = width * .24f;
    const float centerWidth = width - leftWidth - rightWidth;
    const float leftX = origin.x + margin, centerX = leftX + leftWidth + gap;
    const float rightX = centerX + centerWidth + gap, topY = origin.y + margin;
    const ImGuiCond condition = m_ResetLayoutRequested ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    m_ResetLayoutRequested = false;
    const auto beginPane = [&](const char* name, bool& visible, const ImVec2 position, const ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(name, &visible, ImGuiWindowFlags_MenuBar);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0))
            m_InteractionRequested = true;
        if (expanded) Render_WindowMenu();
        return expanded;
    };

    if (m_ResourcesOpen)
    {
        if (beginPane("Object Resources###WorldObjectResourcesWindow", m_ResourcesOpen,
            {leftX, topY}, {leftWidth, height}))
        {
            if (m_Ready) Render_Resources();
            else
            {
                if (ImGui::Button("Reload Source")) Load_Source();
                ImGui::TextWrapped("%s", m_Status.c_str());
            }
            if (m_Ready) Render_EffectResources();
            Render_PhysicalResources();
            if (m_Ready) Render_AnimationResources();
        }
        ImGui::End();
    }
    if (m_SequencerOpen)
    {
        if (beginPane("Object Sequencer###WorldObjectSequencerWindow", m_SequencerOpen,
            {centerX, topY + height * .57f}, {centerWidth, height * .43f}))
        {
            Render_Toolbar();
            auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (const auto* group = Preview_Group()) Render_GroupSequence(*group);
            else if (sequence) Render_Sequence(*sequence);
            else
            {
                ImGui::TextWrapped("Select a Motion beneath an Object to open its Lifetime timeline. The parent Object edits shared resources only.");
                ImGui::BeginDisabled(); ImGui::Button("Play"); ImGui::SameLine(); ImGui::Button("Append Clip"); ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }
    if (m_DetailOpen)
    {
        if (beginPane("Object Detail###WorldObjectDetailWindow", m_DetailOpen,
            {rightX, topY}, {rightWidth, height}))
        {
            if (m_Ready) Render_Detail();
            else ImGui::TextDisabled("Load Object Resources to edit an object.");
        }
        ImGui::End();
    }
    if (!m_ResourcesOpen && !m_SequencerOpen && !m_DetailOpen) m_Open = false;
    if (!m_Open) Stop_Preview();
}

void CWorldObjectTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Object Resources", nullptr, &m_ResourcesOpen);
        ImGui::MenuItem("Object Sequencer", nullptr, &m_SequencerOpen);
        ImGui::MenuItem("Object Detail", nullptr, &m_DetailOpen);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All")) m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
            m_ResetLayoutRequested = true;
        }
        if (ImGui::MenuItem("Close World Object Tool")) m_Open = false;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void CWorldObjectTool::Render_Toolbar()
{
    if (ImGui::Button("Reload Source"))
    {
        if (m_Dirty) ImGui::OpenPopup("Reload object source?");
        else Load_Source();
    }
    if (ImGui::BeginPopupModal("Reload object source?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload discards this tool's unsaved object/state edits.");
        if (ImGui::Button("Discard and Reload")) { Load_Source(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Ready || m_PublishProcess);
    if (ImGui::Button(m_Dirty ? "Save *" : "Save")) Save_Source();
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CWorldObjectTool::Render_Resources()
{
    if (ImGui::Button("Create Object"))
    {
        m_CreateObjectFailed = false;
        ImGui::OpenPopup("Create Object Resource");
    }
    ImGui::SameLine(); ImGui::TextDisabled("%zu resources", m_Document.Get_ObjectResources().size());
    if (ImGui::BeginPopupModal("Create Object Resource", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputTextWithHint("Name", "New object name", m_NewObjectName.data(), m_NewObjectName.size());
        ImGui::Combo("Anchor Type", &m_NewObjectAnchor, "Map\0Character\0Boss\0");
        ImGui::TextUnformatted(m_NewObjectAnchor == 2 ?
            "Boss: motions follow the chosen boss BODY bone. Select the boss and bone in Object Detail." : m_NewObjectAnchor == 0 ?
            "Map: motions created later use a fixed world anchor." :
            "Character: motions created later use each living character as their anchor.");
        ImGui::TextUnformatted("Creates the parent Object only. Add its motions from Object Detail.");
        ImGui::BeginDisabled(!m_NewObjectName[0]);
        if (ImGui::Button("Create"))
        {
            m_CreateObjectFailed = !Create_Object();
            if (!m_CreateObjectFailed) ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (m_CreateObjectFailed) ImGui::TextWrapped("%s", m_Status.c_str());
        ImGui::EndPopup();
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectSearch", "Search object or motion", m_ObjectSearch.data(), m_ObjectSearch.size());
    const float treeHeight = (std::max)(120.f, ImGui::GetContentRegionAvail().y * .28f);
    if (ImGui::BeginChild("ObjectResourceTree", ImVec2(0.f, treeHeight), true))
    {
        const auto search = Lower(m_ObjectSearch.data());
        const auto stateMatches = [&search](const std::string& id, const WORLD_SEQUENCE_TEMPLATE* sequence)
        {
            const auto searchable = sequence ? sequence->displayName + " " + sequence->sequenceId + " " + id : id;
            return Lower(searchable).find(search) != std::string::npos;
        };
        for (const char* anchor : {"WORLD", "PLAYER", "BOSS"})
        {
            const char* category = std::string(anchor) == "WORLD" ? "Map" : std::string(anchor) == "BOSS" ? "Boss" : "Character";
            size_t count = 0;
            for (const auto& resource : m_Document.Get_ObjectResources()) if (resource.anchorKind == anchor) ++count;
            const std::string categoryLabel = std::string(category) + " (" + std::to_string(count) + ")";
            if (!ImGui::TreeNodeEx(anchor, ImGuiTreeNodeFlags_DefaultOpen, "%s", categoryLabel.c_str())) continue;
            for (const auto& resource : m_Document.Get_ObjectResources())
            {
                if (resource.anchorKind != anchor) continue;
                const auto states = StateIds(resource);
                const bool resourceMatches = search.empty() || Lower(resource.displayName + " " + resource.objectId).find(search) != std::string::npos;
                bool matches = resourceMatches;
                if (!matches)
                    for (const auto& id : states)
                    {
                        const auto* state = m_Document.Find_Instance(id);
                        const auto* sequence = state ? m_Document.Find_Template(state->templateId) : nullptr;
                        if (stateMatches(id, sequence)) { matches = true; break; }
                    }
                if (!matches) continue;
                ImGui::PushID(resource.objectId.c_str());
                const auto label = resource.displayName + (!resource.motionInstanceIds.empty() ? " [Group]" :
                    resource.sequenceInstanceId.empty() && resource.modelAssetId.empty() ? " [assign model]" : "");
                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth | (m_SelectedObject == resource.objectId && m_SelectedInstance.empty() ? ImGuiTreeNodeFlags_Selected : 0);
                if (!search.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                const bool open = ImGui::TreeNodeEx("Resource", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked()) Select_Object(resource.objectId);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", resource.objectId.c_str());
                if (open)
                {
                    for (const auto& id : states)
                    {
                        const auto* instance = m_Document.Find_Instance(id);
                        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
                        if (!resourceMatches && !stateMatches(id, sequence)) continue;
                        ImGui::PushID(id.c_str());
                        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), id == m_SelectedInstance))
                        {
                            m_SelectedObject = resource.objectId;
                            if (!resource.motionInstanceIds.empty() && instance && !instance->bindings.empty())
                                m_SelectedObject = instance->bindings.front().targetId;
                            Select_State(id);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
                        ImGui::PopID();
                    }
                    if (states.empty()) ImGui::TextDisabled("No motions - select Object to Create Motion");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_EffectResources()
{
    if (!ImGui::CollapsingHeader("V2 Effects", ImGuiTreeNodeFlags_DefaultOpen)) return;
    const bool reload = ImGui::Button("Reload V2 Effects");
    if (!m_EffectInventoryLoaded || reload)
    {
        std::vector<EFFECT_V2_RESOURCE_SUMMARY> staged;
        if (CEffectV2Catalog::Get().Read_Inventory(staged, m_EffectResourceStatus))
            m_EffectResources = std::move(staged);
        m_EffectInventoryLoaded = true;
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectEffectSearch", "Search smoke / group / leaf", m_EffectSearch.data(), m_EffectSearch.size());
    const auto search = Lower(m_EffectSearch.data());
    if (ImGui::BeginChild("ObjectV2Effects", ImVec2(0.f, 135.f), true))
        for (const auto& effect : m_EffectResources)
        {
            if (!search.empty() && Lower(effect.strDisplayName + " " + effect.strResourceId).find(search) == std::string::npos) continue;
            ImGui::PushID(effect.strResourceId.c_str());
            const auto label = std::string(effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "[Group] " : "[Leaf] ") +
                (effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName);
            ImGui::BeginDisabled(!effect.strStatus.empty());
            if (ImGui::Selectable(label.c_str(), m_SelectedEffectResource == effect.strResourceId && m_SelectedEffectKind == effect.eKind))
            { m_SelectedEffectResource = effect.strResourceId; m_SelectedEffectKind = effect.eKind; }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s\n%s", effect.strResourceId.c_str(), effect.strStatus.c_str());
            ImGui::PopID();
        }
    ImGui::EndChild();
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const bool target = instance && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
    ImGui::BeginDisabled(!target || m_SelectedEffectResource.empty());
    if (ImGui::Button("Append Effect at Motion End")) Append_SelectedEffect();
    ImGui::EndDisabled();
    if (!target) ImGui::TextWrapped("Select an Object's child Motion, then append an Effect row.");
    if (!m_EffectResourceStatus.empty()) ImGui::TextWrapped("%s", m_EffectResourceStatus.c_str());
}

bool CWorldObjectTool::Append_SelectedEffect()
{
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence || instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
    { m_Status = "Select a child Object Motion before appending an Effect."; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(m_SelectedEffectKind, m_SelectedEffectResource, snapshot, m_EffectResourceStatus))
    { m_Status = "Effect Append failed: " + m_EffectResourceStatus; return false; }
    // Match the existing V2 authoring preview span, including particles/trails
    // remaining after emission ends. The row does not stretch leaf envelopes.
    const auto leafSpan = [](const EFFECT_V2_DOCUMENT& document, const uint32_t explicitMs, const bool tailEnabled)
    {
        const auto& params = document.Desc.Params;
        const double rate = (std::max)(.001, static_cast<double>(params.fPlayRate));
        const double emission = explicitMs ? explicitMs : params.fLifetime > 0.f ?
            std::ceil(params.fLifetime * 1000.0 / rate) : 3000.0;
        const double tail = !tailEnabled ? 0.0 : document.eType == EFFECT_V2_TYPE::PARTICLE ?
            params.Particle.vLifetime.y * 1000.0 / rate : document.eType == EFFECT_V2_TYPE::TRAIL ?
            params.Trail.fPointLifetime * 1000.0 / rate : 0.0;
        return emission + std::ceil(tail);
    };
    double span = 0.;
    if (m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        const auto* group = snapshot->Find_Group(m_SelectedEffectResource);
        if (!group) { m_Status = "Selected Effect group is unavailable."; return false; }
        span = group->iDurationMs;
        if (!group->iDurationMs)
            for (const auto& child : group->Children)
            {
                const auto* leaf = snapshot->Find_Document(child.strEffectId);
                if (!leaf) { m_Status = "Selected Effect group child is unavailable."; return false; }
                span = (std::max)(span, child.iStartMs + leafSpan(*leaf, child.iDurationMs,
                    child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE));
            }
    }
    else
    {
        const auto* leaf = snapshot->Find_Document(m_SelectedEffectResource);
        if (!leaf) { m_Status = "Selected Effect leaf is unavailable."; return false; }
        span = leafSpan(*leaf, 0u, true);
    }
    CWorldSequenceDocument staged = m_Document;
    auto* edited = staged.Find_Template(sequence->sequenceId);
    WORLD_SEQUENCE_EFFECT_TRACK row;
    uint32_t serial = 1u;
    do { row.effectTrackId = "effect." + std::to_string(serial++); }
    while (std::any_of(edited->effectTracks.begin(), edited->effectTracks.end(),
        [&](const auto& value) { return value.effectTrackId == row.effectTrackId; }));
    row.slotId = instance->bindings.front().slotId;
    row.resourceKind = m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.resourceId = m_SelectedEffectResource;
    row.durationMs = static_cast<uint32_t>((std::clamp)(std::ceil(span), 1., 600000.));
    edited->effectTracks.push_back(row);
    if (!staged.Validate(m_MapTargets, m_DeployTargets, m_Status)) return false;
    m_SelectedEffectRow = edited->effectTracks.size() - 1u;
    m_Document = std::move(staged);
    Mark_Dirty();
    m_Status = "Effect row appended at Motion End. Adjust it in Effect Rows, then Save.";
    return true;
}

void CWorldObjectTool::Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!ImGui::CollapsingHeader("Effect Rows", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (sequence.effectTracks.empty())
    { ImGui::TextWrapped("Choose a V2 Group or Leaf in Object Resources and Append Effect at Motion End."); return; }
    m_SelectedEffectRow = (std::min)(m_SelectedEffectRow, sequence.effectTracks.size() - 1u);
    for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
    {
        const auto& row = sequence.effectTracks[index];
        const auto label = row.resourceId + "##" + row.effectTrackId;
        if (ImGui::Selectable(label.c_str(), m_SelectedEffectRow == index)) m_SelectedEffectRow = index;
    }
    auto& row = sequence.effectTracks[m_SelectedEffectRow];
    bool changed = false;
    int timing = row.timing == "MOTION_END" ? 0 : 1;
    if (ImGui::Combo("Effect Trigger", &timing, "Motion End\0At Time\0"))
    {
        row.timing = timing == 0 ? "MOTION_END" : "TIME";
        row.startMs = timing == 0 ? 0u : sequence.durationMs;
        changed = true;
    }
    if (timing == 1) changed |= EditUInt("Effect Start (ms)", row.startMs, sequence.durationMs);
    else ImGui::TextDisabled("Follows Motion Lifetime: %u ms", sequence.durationMs);
    changed |= EditUInt("Effect Window (ms)", row.durationMs, CWorldSequenceDocument::MAX_DURATION_MS, 1);
    changed |= ImGui::DragFloat3("Effect Offset (m)", &row.positionOffset.x, .01f);
    changed |= ImGui::DragFloat3("Effect Rotation (deg)", &row.rotationDegrees.x, .5f);
    changed |= ImGui::DragFloat3("Effect Scale", &row.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextWrapped("Each emitted object triggers this Effect at its own trajectory position. The Effect stays there after the model ends; its original envelope is preserved.");
    if (ImGui::Button("Remove Selected Effect Row"))
    {
        sequence.effectTracks.erase(sequence.effectTracks.begin() + static_cast<ptrdiff_t>(m_SelectedEffectRow));
        m_SelectedEffectRow = 0;
        changed = true;
    }
    if (changed) Mark_Dirty();
}


void CWorldObjectTool::Refresh_AnimationResources()
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
    {
        m_AnimationResources.clear();
        m_SelectedAnimationClip.clear();
    }
    m_AnimationObjectId = m_SelectedObject;
    m_AnimationModelAssetId = modelAssetId;
    m_AnimationCatalogReady = false;
    if (!resource)
    { m_AnimationResourceStatus = "Select or Create Object first."; return; }
    if (!resource->sequenceInstanceId.empty())
    { m_AnimationResourceStatus = "This placed object uses its existing sequence tracks; it has no separate native clip catalog."; return; }
    if (modelAssetId.empty())
    { m_AnimationResourceStatus = "Select a WModel in Physical Resources to list its native animations."; return; }
    const auto path = CRuntimeAssetRoot::Resolve(modelAssetId);
    std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> catalog;
    std::string status;
    if (path.empty() || !Engine::CWModelDecoder::Read_AnimationCatalog(path, catalog, status))
    {
        m_AnimationResourceStatus = "Animation catalog unavailable: " + modelAssetId + ": " +
            (path.empty() ? "invalid Resources-relative path" : status) + ". Existing object and patterns are unchanged.";
        return;
    }
    std::vector<ANIMATION_RESOURCE> staged;
    for (const auto& clip : catalog)
    {
        // Match the CAnimation clock used by WorldSequenceObject::Sample.
        const double duration = static_cast<double>(clip.durationTicks) / Engine::CAnimation::COOKED_TICK_RATE * 1000.;
        if (clip.name.empty() || !std::isfinite(duration) || duration <= 0.)
        { m_AnimationResourceStatus = "Invalid animation timing: " + modelAssetId + ". Existing patterns are unchanged."; return; }
        staged.push_back({clip.name, duration});
    }
    m_AnimationResources = std::move(staged);
    m_AnimationCatalogReady = true;
    if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == m_SelectedAnimationClip;
    })) m_SelectedAnimationClip.clear();
    m_AnimationResourceStatus = m_AnimationResources.empty() ?
        "This model has no native animation. Assign Model on the parent Object, then author Transform keys or Physics on a child Motion." :
        std::to_string(m_AnimationResources.size()) + " native clips. Select one and Append Clip to the selected Motion.";
}

bool CWorldObjectTool::Stage_SelectedModel(CWorldSequenceDocument& candidate)
{
    auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || !resource->sequenceInstanceId.empty() || !m_AnimationCatalogReady ||
        m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId.empty())
    { m_Status = "Select an available physical WModel for this object first."; return false; }
    // A resource can own several saved patterns. Keep all of their clip bindings valid.
    for (const auto& instance : candidate.Get_Instances())
    {
        const auto* sequence = candidate.Find_Template(instance.templateId);
        if (!sequence) continue;
        for (const auto& binding : instance.bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource->objectId) continue;
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
                    return clip.clipName == animation.clipName;
                }))
                {
                    m_Status = "Model change refused: pattern '" + sequence->displayName + "' uses '" + animation.clipName +
                        "', which is absent from " + m_AnimationModelAssetId + ". Existing model and patterns are unchanged.";
                    return false;
                }
            }
        }
    }
    resource->modelAssetId = m_AnimationModelAssetId;
    resource->animated = !m_AnimationResources.empty();
    return true;
}

bool CWorldObjectTool::Assign_SelectedModel()
{
    if (!m_SelectedInstance.empty())
    { m_Status = "Select the parent Object to assign its shared model."; return false; }
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    CWorldSequenceDocument candidate = m_Document;
    if (!Stage_SelectedModel(candidate)) return false;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Model assignment refused: " + status + ". Existing draft preserved."; return false; }
    const auto pristinePattern = m_PristinePatternId;
    m_Document = std::move(candidate);
    Mark_Dirty();
    // Assigning a model does not edit the newly created pattern's default timing.
    m_PristinePatternId = pristinePattern;
    m_Status = "Shared model assigned. Create or select a Motion, then Append Clip or edit Transform/Physics.";
    return true;
}

bool CWorldObjectTool::Append_SelectedAnimation()
{
    const auto selectedClip = m_SelectedAnimationClip;
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    const auto found = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == selectedClip;
    });
    if (selectedClip.empty() || found == m_AnimationResources.end())
    { m_Status = "Select an available native animation first; the current object pattern is unchanged."; return false; }
    const double nativeMs = std::ceil(found->durationMs);
    if (!std::isfinite(nativeMs) || nativeMs < 1. || nativeMs > CWorldSequenceDocument::MAX_DURATION_MS)
    { m_Status = "The selected animation exceeds the supported 600-second pattern lifetime."; return false; }
    const uint32_t clipMs = static_cast<uint32_t>(nativeMs);
    CWorldSequenceDocument candidate = m_Document;
    auto* instance = candidate.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? candidate.Find_Template(instance->templateId) : nullptr;
    if (!sequence)
    { m_Status = "Select or create a Motion before appending a clip."; return false; }
    const auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || resource->modelAssetId != m_AnimationModelAssetId || !resource->animated)
    { m_Status = "Assign the animated model on the parent Object before appending its clips."; return false; }
    std::string slotId;
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != m_SelectedObject) continue;
        if (!slotId.empty())
        { m_Status = "This pattern has multiple bindings for the selected object; choose a single-object pattern."; return false; }
        slotId = binding.slotId;
    }
    if (slotId.empty())
    { m_Status = "The selected pattern does not bind the selected object. Existing draft preserved."; return false; }
    if (sequence->tracks.size() + sequence->animationTracks.size() + sequence->effectTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { m_Status = "The pattern has reached its track limit. Existing draft preserved."; return false; }
    const bool firstOfSlot = std::none_of(sequence->animationTracks.begin(), sequence->animationTracks.end(), [&](const auto& clip) {
        return clip.slotId == slotId;
    });
    const bool pristine = firstOfSlot && sequence->animationTracks.empty() && m_PristinePatternId == m_SelectedInstance;
    const uint32_t oldDuration = sequence->durationMs;
    const uint32_t startMs = firstOfSlot ? 0u : oldDuration;
    if (startMs > CWorldSequenceDocument::MAX_DURATION_MS - clipMs)
    { m_Status = "Appending this clip would exceed the 600-second pattern lifetime. Existing draft preserved."; return false; }
    const uint32_t duration = pristine ? clipMs : (std::max)(oldDuration, startMs + clipMs);
    for (auto& track : sequence->tracks)
    {
        if (track.keys.empty())
        { m_Status = "The pattern has an empty Transform track. Existing draft preserved."; return false; }
        if (pristine)
            track.keys.back().timeMs = duration;
        else if (duration > oldDuration)
        {
            if (track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { m_Status = "Extending this pattern would exceed its Transform key limit. Existing draft preserved."; return false; }
            // Preserve every authored time; extend only the final held pose.
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
    }
    WORLD_SEQUENCE_ANIMATION_TRACK animation;
    animation.slotId = slotId; animation.clipName = selectedClip; animation.startMs = startMs;
    animation.playbackRate = 1.f; animation.loop = false; animation.holdLastFrame = true;
    sequence->animationTracks.push_back(std::move(animation));
    sequence->durationMs = duration;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Animation append refused: " + status + ". Existing draft preserved."; return false; }
    m_Document = std::move(candidate);
    Mark_Dirty();
    m_Status = "Appended " + selectedClip + " at " + std::to_string(startMs) + " ms. Play in Object Sequencer, tune in Object Detail, then Save.";
    return true;
}

void CWorldObjectTool::Render_AnimationResources()
{
    ImGui::SeparatorText("Animation Resources");
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
        Refresh_AnimationResources();
    if (!resource || !resource->sequenceInstanceId.empty())
    { ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str()); return; }
    if (ImGui::Button("Refresh Animations")) Refresh_AnimationResources();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_SelectedInstance.empty() || !m_AnimationCatalogReady || m_AnimationModelAssetId.empty());
    if (ImGui::Button("Assign Model")) { Assign_SelectedModel(); resource = m_Document.Find_ObjectResource(m_SelectedObject); }
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str());
    if (!modelAssetId.empty()) ImGui::TextWrapped("Model: %s", modelAssetId.c_str());
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (sequence) ImGui::TextWrapped("Selected Motion: %s", sequence->displayName.c_str());
    else ImGui::TextWrapped("Select a child Motion to append clips. Shared model assignment belongs to the parent Object.");
    ImGui::BeginDisabled(!m_AnimationCatalogReady || m_SelectedAnimationClip.empty() || !sequence);
    if (ImGui::Button("Append Clip")) Append_SelectedAnimation();
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##AnimationSearch", "Search native animation", m_AnimationSearch.data(), m_AnimationSearch.size());
    const auto search = Lower(m_AnimationSearch.data());
    const float height = (std::max)(100.f, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("NativeAnimationCatalog", ImVec2(0.f, height), true))
    {
        for (const auto& clip : m_AnimationResources)
        {
            if (!search.empty() && Lower(clip.clipName).find(search) == std::string::npos) continue;
            ImGui::PushID(clip.clipName.c_str());
            const float nameWidth = (std::max)(80.f, ImGui::GetContentRegionAvail().x - 82.f);
            if (ImGui::Selectable(clip.clipName.c_str(), clip.clipName == m_SelectedAnimationClip, 0, ImVec2(nameWidth, 0.f)))
                m_SelectedAnimationClip = clip.clipName;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%.3f ms\n%s", clip.clipName.c_str(), clip.durationMs, m_AnimationModelAssetId.c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%.0f ms", clip.durationMs);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText("Object / Shared Resources");
    bool changed = EditText("Object Name", resource.displayName);
    ImGui::TextDisabled("%s", resource.objectId.c_str());
    const bool alias = !resource.sequenceInstanceId.empty();
    int resourceAnchor = resource.anchorKind == "BOSS" ? 2 : resource.anchorKind == "PLAYER" ? 1 : 0;
    ImGui::BeginDisabled(alias);
    if (ImGui::Combo("Anchor Type", &resourceAnchor, "Map\0Character\0Boss\0"))
        Change_ResourceAnchor(resource, resourceAnchor == 2 ? "BOSS" : resourceAnchor == 1 ? "PLAYER" : "WORLD");
    ImGui::EndDisabled();
    if (resource.anchorKind == "BOSS")
    {
        static constexpr const char* actorIds[] = {
            "BOSS_KAKULSAYDON_G1_KOUKU", "BOSS_KAKULSAYDON_G1_SAYDON",
            "BOSS_KAKULSAYDON_G2_KOUKU", "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "BOSS_KAKULSAYDON_G3_SAYDON"};
        static constexpr const char* actorLabels[] = {
            "Kouku / Gate 1", "Saydon / Gate 1", "Kouku / Gate 2", "Big Saydon / Gate 2", "Saydon / Gate 3"};
        const char* selected = resource.anchorBossArchetypeId.empty() ? "Choose Boss" : resource.anchorBossArchetypeId.c_str();
        for (size_t i = 0u; i < std::size(actorIds); ++i)
            if (resource.anchorBossArchetypeId == actorIds[i]) selected = actorLabels[i];
        if (ImGui::BeginCombo("Boss Actor", selected))
        {
            for (size_t i = 0u; i < std::size(actorIds); ++i)
                if (ImGui::Selectable(actorLabels[i], resource.anchorBossArchetypeId == actorIds[i]))
                { resource.anchorBossArchetypeId = actorIds[i]; changed = true; }
            ImGui::EndCombo();
        }
        changed |= EditText("Boss Archetype ID", resource.anchorBossArchetypeId);
        changed |= EditText("BODY Bone", resource.anchorBone);
        ImGui::TextWrapped("Follows the named BODY bone every frame. Empty bone uses the boss root. Hand props use b_wp_1 or b_wp_2. Transform keys edit the local grip offset and rotation.");
    }
    if (alias) ImGui::TextDisabled("Placed objects keep their Map anchor.");
    if (alias)
        ImGui::TextWrapped("Placed object sequence: %s. This editor updates its existing tracks and bindings.", resource.sequenceInstanceId.c_str());
    else
    {
        ImGui::TextWrapped("Model: %s", resource.modelAssetId.empty() ? "Choose a WModel in Physical Resources" : resource.modelAssetId.c_str());
        ImGui::TextWrapped("Diffuse: %s", resource.diffuseTextureAssetId.empty() ? "Embedded model material" : resource.diffuseTextureAssetId.c_str());
        if (!resource.diffuseTextureAssetId.empty() && ImGui::SmallButton("Clear Diffuse Override")) { resource.diffuseTextureAssetId.clear(); changed = true; }
        changed |= ImGui::DragFloat("Model Import Scale", &resource.modelPreScale, .001f, .000001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Object Scale", &resource.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::Checkbox("Animated Model", &resource.animated);
    }
    if (changed) Mark_Dirty();
    if (!alias)
    {
        const auto* initial = m_Document.Find_Instance(resource.defaultMotionInstanceId);
        const auto* initialSequence = initial ? m_Document.Find_Template(initial->templateId) : nullptr;
        if (initialSequence && !initialSequence->tracks.empty() && !initialSequence->tracks.front().keys.empty())
        {
            const auto& first = initialSequence->tracks.front().keys.front();
            auto position = first.positionOffset;
            auto rotation = QuaternionEuler(first.rotationQuaternion);
            ImGui::SeparatorText("Object Transform / All Motion Keys");
            bool transformChanged = ImGui::DragFloat3("Object Position (m)", &position.x, .01f);
            transformChanged |= ImGui::DragFloat3("Object Rotation (deg)", &rotation.x, .25f);
            ImGui::TextWrapped("Position and Rotation adjust every connected Motion key relative to its current pose. Object Scale above applies to all Motions. Use a child Motion for individual keys.");
            if (transformChanged)
            {
                auto candidate = m_Document;
                const auto translationDelta = XMLoadFloat3(&position) - XMLoadFloat3(&first.positionOffset);
                const auto rotationDelta = XMMatrixTranspose(XMMatrixRotationQuaternion(XMLoadFloat4(&first.rotationQuaternion))) *
                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
                std::vector<std::string> editedTemplates;
                for (const auto& id : StateIds(resource))
                {
                    const auto* motion = candidate.Find_Instance(id);
                    auto* sequence = motion ? candidate.Find_Template(motion->templateId) : nullptr;
                    if (!sequence || std::find(editedTemplates.begin(), editedTemplates.end(), sequence->sequenceId) != editedTemplates.end()) continue;
                    // Shared templates cannot be rewritten on behalf of a different Object.
                    for (const auto& other : candidate.Get_Instances())
                        if (other.templateId == sequence->sequenceId)
                            for (const auto& binding : other.bindings)
                                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource.objectId)
                                { m_Status = "This Motion template is shared with another target. Edit its keys separately; Object transform was preserved."; return; }
                    for (auto& track : sequence->tracks)
                        for (auto& key : track.keys)
                        {
                            XMStoreFloat3(&key.positionOffset, XMLoadFloat3(&key.positionOffset) + translationDelta);
                            XMStoreFloat4(&key.rotationQuaternion, XMQuaternionNormalize(XMQuaternionRotationMatrix(
                                XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) * rotationDelta)));
                        }
                    editedTemplates.push_back(sequence->sequenceId);
                }
                std::string status;
                if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
                { m_Status = "Object transform refused: " + status + ". Existing draft preserved."; return; }
                m_Document = std::move(candidate);
                Mark_Dirty();
                m_Status = "Updated the Object transform in every connected Motion. Save preserves these keys.";
                return;
            }
        }
        else ImGui::TextDisabled("Choose a Default Motion to edit Object Position and Rotation here.");
    }
    ImGui::SeparatorText("Connected Motions");
    const auto motions = StateIds(resource);
    const auto* defaultMotion = m_Document.Find_Instance(resource.defaultMotionInstanceId);
    const auto* defaultSequence = defaultMotion ? m_Document.Find_Template(defaultMotion->templateId) : nullptr;
    const char* defaultLabel = defaultSequence ? defaultSequence->displayName.c_str() : "None - choose a Default Motion";
    if (ImGui::BeginCombo("Default Motion", defaultLabel))
    {
        if (ImGui::Selectable("None", resource.defaultMotionInstanceId.empty()))
        { resource.defaultMotionInstanceId.clear(); Mark_Dirty(); }
        for (const auto& id : motions)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            ImGui::BeginDisabled(!instance || !instance->enabled || !sequence);
            if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), resource.defaultMotionInstanceId == id))
            { resource.defaultMotionInstanceId = id; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (!defaultMotion || !defaultMotion->enabled)
        ImGui::TextWrapped("Choose an enabled Default Motion to Append or Preview this Object.");
    ImGui::BeginDisabled(!Preview_Instance());
    if (ImGui::Button("Preview Default")) { Stop_Preview(); m_PreviewAtCharacter = true; m_ClockMs = 0.f; Seek(0.f); m_Playing = m_PreviewActive; }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Stop Preview")) { Stop_Preview(); m_ClockMs = 0.f; }
    if (motions.empty()) ImGui::TextDisabled("No motions yet.");
    for (const auto& id : motions)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        ImGui::PushID(id.c_str());
        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str()))
        { Select_State(id); ImGui::PopID(); return; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
        if (sequence && instance)
        {
            ImGui::TextDisabled("%zu clips | %u ms | %s", sequence->animationTracks.size(),
                sequence->durationMs, MotionEndLabel(instance->motionEnd));
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            {
                const auto* next = m_Document.Find_Instance(instance->nextMotionId);
                const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
                ImGui::TextDisabled("  Next: %s", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion");
            }
        }
        ImGui::PopID();
    }
    ImGui::BeginDisabled(alias);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##NewMotion", "Motion name", m_NewStateName.data(), m_NewStateName.size());
    ImGui::BeginDisabled(!m_NewStateName[0]);
    if (ImGui::Button("Create Motion")) Create_State();
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    if (alias) ImGui::TextWrapped("This placed Object retains its existing Motion and bindings.");
}

void CWorldObjectTool::Render_Detail()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource) { ImGui::TextUnformatted("Select an object resource."); return; }
    if (Preview_Group()) { Render_GroupDetail(*resource); return; }
    if (m_SelectedInstance.empty()) { Render_ObjectDetail(*resource); return; }
    ImGui::TextWrapped("Object: %s", resource->displayName.c_str());
    if (ImGui::Button("Edit Parent Object")) { Select_Object(resource->objectId); return; }
    const bool alias = !resource->sequenceInstanceId.empty();
    bool changed = false;
    auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence) { ImGui::TextWrapped("The selected Motion is unavailable. Select its parent Object to continue."); return; }
    ImGui::SeparatorText("Motion");
    changed |= EditText("Motion Name", sequence->displayName);
    ImGui::TextDisabled("%s", instance->instanceId.c_str());
    changed |= ImGui::Checkbox("Enabled", &instance->enabled);
    if (!alias)
    {
        ImGui::TextDisabled("Creation Anchor: %s", instance->anchorKind == "BOSS" ? "Boss / BODY Bone" : instance->anchorKind == "PLAYER" ? "Character" : "Map");
        changed |= ImGui::DragFloat3(instance->anchorKind == "BOSS" ? "Bone Offset" : instance->anchorKind == "PLAYER" ? "Character Offset" : "Map Position", &instance->position.x, .01f);
        if (ImGui::Button("Use Current Character Position"))
        {
            if (instance->anchorKind == "PLAYER" || instance->anchorKind == "BOSS") { instance->position = {}; changed = true; }
            else if (auto* level = CLevel_KakulSaydonArena::Get_Active()) changed |= level->Try_Get_AuthoringPreviewPlacement(instance->position, m_Status);
            else m_Status = "Player placement requires the active KoukuSaydon arena.";
        }
    }
    const bool supportsSurface = alias && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
    if (supportsSurface || instance->walkableSurface)
    {
        ImGui::SeparatorText("Walkable Surface");
        bool enabled = instance->walkableSurface.has_value();
        if (ImGui::Checkbox("Enable Circular Walking Surface", &enabled))
        {
            if (enabled) instance->walkableSurface = WORLD_SEQUENCE_WALKABLE_SURFACE{};
            else instance->walkableSurface.reset();
            changed = true;
        }
        if (instance->walkableSurface)
        {
            changed |= ImGui::DragFloat("Surface Radius (local m)", &instance->walkableSurface->radiusM,
                .01f, .001f, 1000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Surface Height (local m)", &instance->walkableSurface->localHeightM,
                .001f, -10000.f, 10000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("A horizontal circle centered on the placed model. Values use meters after model import scale; placement scale is applied automatically. Fixed position/scale and Y rotation only. Server Complete Play uses its visible lifetime; blocked ground stays blocked. Save, then publish the Map and Kouku gameplay data.");
        }
    }
    changed |= EditUInt("Start Delay (ms)", instance->startDelayMs, CWorldSequenceDocument::MAX_DURATION_MS);
    changed |= ImGui::DragFloat("Playback Speed", &instance->playbackSpeed, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    uint32_t duration = sequence->durationMs;
    if (EditUInt("Lifetime (ms)", duration, CWorldSequenceDocument::MAX_DURATION_MS, 1))
    {
        // Preserve the key order and endpoint contract even when shortening a state.
        bool valid = true;
        for (const auto& track : sequence->tracks) if (duration + 1u < track.keys.size()) valid = false;
        for (const auto& animation : sequence->animationTracks)
            if (static_cast<size_t>(duration) < std::count_if(sequence->animationTracks.begin(), sequence->animationTracks.end(),
                [&](const auto& other) { return other.slotId == animation.slotId; })) valid = false;
        if (valid)
        {
            for (auto& track : sequence->tracks)
            {
                for (size_t key = 1; key + 1 < track.keys.size(); ++key)
                {
                    const auto scaled = static_cast<uint32_t>(std::llround(static_cast<double>(track.keys[key].timeMs) * duration / sequence->durationMs));
                    track.keys[key].timeMs = (std::clamp)(scaled, track.keys[key - 1].timeMs + 1,
                        duration - static_cast<uint32_t>(track.keys.size() - key - 1));
                }
                if (!track.keys.empty()) track.keys.back().timeMs = duration;
            }
            std::unordered_map<std::string, uint32_t> previousStarts;
            for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
            {
                auto& animation = sequence->animationTracks[index];
                const auto previous = previousStarts.find(animation.slotId);
                const auto remaining = std::count_if(sequence->animationTracks.begin() + index + 1, sequence->animationTracks.end(),
                    [&](const auto& other) { return other.slotId == animation.slotId; });
                const uint32_t scaled = static_cast<uint32_t>(static_cast<double>(animation.startMs) * duration / sequence->durationMs);
                animation.startMs = previous == previousStarts.end() ? 0 : (std::clamp)(scaled,
                    previous->second + 1, duration - static_cast<uint32_t>(remaining) - 1);
                previousStarts[animation.slotId] = animation.startMs;
            }
            for (auto& effect : sequence->effectTracks)
                if (effect.timing == "TIME")
                    effect.startMs = static_cast<uint32_t>(static_cast<uint64_t>(effect.startMs) * duration / sequence->durationMs);
            sequence->durationMs = duration;
            if (sequence->objectMotion.count > 1)
                sequence->objectMotion.intervalMs = (std::min)(sequence->objectMotion.intervalMs, (sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration) / (sequence->objectMotion.count - 1));
            for (auto& emission : sequence->objectMotion.emissions)
                emission.startDelayMs = (std::min)(emission.startDelayMs, sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration);
            changed = true;
        }
        else m_Status = "Lifetime must leave at least one millisecond between every existing key or clip.";
    }
    ImGui::BeginDisabled(alias);
    int motionEnd = static_cast<int>(instance->motionEnd);
    if (ImGui::Combo("On Complete", &motionEnd, "Stop\0Hold Last Pose\0Loop\0Play Motion\0"))
    {
        instance->motionEnd = static_cast<WORLD_SEQUENCE_MOTION_END>(motionEnd);
        if (instance->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) instance->nextMotionId.clear();
        changed = true;
    }
    ImGui::EndDisabled();
    if (alias) ImGui::TextDisabled("Placed Object motions use Stop.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
    {
        const auto* next = m_Document.Find_Instance(instance->nextMotionId);
        const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
        if (ImGui::BeginCombo("Next Motion", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion"))
        {
            for (const auto& id : StateIds(*resource))
            {
                const auto* candidate = m_Document.Find_Instance(id);
                const auto* candidateSequence = candidate ? m_Document.Find_Template(candidate->templateId) : nullptr;
                if (!candidateSequence || candidate->instanceId == instance->instanceId) continue;
                const bool compatible = candidate->enabled && sequence->objectMotion.count == 1u &&
                    candidateSequence->objectMotion.count == 1u && instance->bindings.size() == 1u &&
                    candidate->bindings.size() == 1u && instance->bindings.front().slotId == candidate->bindings.front().slotId;
                ImGui::PushID(id.c_str());
                ImGui::BeginDisabled(!compatible);
                if (ImGui::Selectable(candidateSequence->displayName.c_str(), instance->nextMotionId == id))
                { instance->nextMotionId = id; changed = true; }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("%s%s", id.c_str(), compatible ? "" : "\nRequires an enabled Motion with Count 1 and the same object slot.");
                ImGui::EndDisabled();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::TextWrapped("At Lifetime, continue the same object with this Motion. Each Motion in the link must use Count 1; links cannot form a cycle.");
    }
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD)
        ImGui::TextWrapped("Keep the final transform and animation pose after Lifetime.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP)
        ImGui::TextWrapped("Repeat this Motion after Lifetime. This Loop is saved and also used by Play.");
    else ImGui::TextWrapped("Standalone Play ends the object at Lifetime. A Motion applied by a Result stops at its final pose; the target WORLD cue keeps its original lifetime.");
    int interpolation = sequence->interpolation == WORLD_SEQUENCE_INTERPOLATION::LINEAR ? 0 : 1;
    if (ImGui::Combo("Interpolation", &interpolation, "Linear\0Smooth Step\0"))
    { sequence->interpolation = interpolation == 0 ? WORLD_SEQUENCE_INTERPOLATION::LINEAR : WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP; changed = true; }
    if (!alias && ImGui::CollapsingHeader("Physics / Motion / Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& motion = sequence->objectMotion;
        ImGui::TextWrapped("Motion uses the Lifetime timeline and works without animation clips. Play or drag the ruler to preview, then Save.");
        ImGui::DragFloat("Arc Height (m)", &m_VerticalArcHeight, .05f, .01f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Apply Vertical Arc"))
        {
            const float seconds = sequence->durationMs * .001f;
            const float velocityY = 4.f * m_VerticalArcHeight / seconds;
            const float accelerationY = -8.f * m_VerticalArcHeight / (seconds * seconds);
            if (!std::isfinite(m_VerticalArcHeight) || m_VerticalArcHeight <= 0.f ||
                !std::isfinite(velocityY) || !std::isfinite(accelerationY) ||
                std::abs(velocityY) > 100000.f || std::abs(accelerationY) > 100000.f)
                m_Status = "Vertical arc is too fast for this Lifetime. Increase Lifetime or reduce Arc Height; motion preserved.";
            else
            {
                motion.velocity = {0.f, velocityY, 0.f};
                motion.acceleration = {0.f, accelerationY, 0.f};
                motion.spreadDegrees = 0.f;
                changed = true;
                m_Status = "Vertical arc applied. Count and interval preserved. First emission returns at Lifetime. Effect-bearing motions give every emission a full Lifetime. Save to keep the motion.";
            }
        }
        ImGui::TextDisabled("First emission: apex at %.0f ms; return at %u ms.", sequence->durationMs * .5f, sequence->durationMs);
        ImGui::TextWrapped("Arc Height sets the physics offset above the Transform track. With Effect rows, every emission has a full Lifetime and its own Effect tail. Apply again after changing Lifetime. Velocity and Acceleration remain editable.");
        changed |= ImGui::DragFloat3("Velocity (m/s)", &motion.velocity.x, .05f);
        changed |= ImGui::DragFloat3("Acceleration (m/s2)", &motion.acceleration.x, .05f);
        changed |= ImGui::DragFloat3("Self Rotation (deg/s)", &motion.angularVelocityDegrees.x, .5f);
        changed |= ImGui::DragFloat3("Revolution (deg/s)", &motion.revolutionDegreesPerSecond.x, .5f);
        changed |= ImGui::DragFloat3("Revolution Offset (m)", &motion.revolutionOffset.x, .05f);
        auto& emissions = motion.emissions;
        ImGui::BeginDisabled(!emissions.empty());
        changed |= EditUInt("Count", motion.count, 128, 1);
        const uint32_t maxInterval = motion.count > 1 ? (sequence->effectTracks.empty() ? sequence->durationMs - 1 :
            CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs) / (motion.count - 1) : CWorldSequenceDocument::MAX_DURATION_MS;
        if (motion.intervalMs > maxInterval) { motion.intervalMs = maxInterval; changed = true; }
        changed |= EditUInt("Creation Interval (ms)", motion.intervalMs, maxInterval);
        changed |= ImGui::DragFloat(sequence->effectTracks.empty() ? "Spread (deg)" : "Horizontal Spread (deg)", &motion.spreadDegrees, .5f, 0.f, sequence->effectTracks.empty() ? 180.f : 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        changed |= ImGui::DragFloat3("Spawn Half Extents (m)", &motion.spawnHalfExtents.x, .05f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextDisabled("Width / height / depth = twice these values. Set Y to 0 for a ground rectangle.");
        changed |= EditUInt("Seed", motion.seed, INT_MAX);
        ImGui::SeparatorText("Authored Emissions");
        ImGui::TextWrapped("Rows replace the seeded spread. Each row replays this Motion's keys, physics and revolution from its own local offset, yaw and delay. Leave the list empty to keep Count / Creation Interval / Spread.");
        const uint32_t maxDelay = sequence->effectTracks.empty() ? sequence->durationMs - 1 : CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs;
        size_t removeRow = emissions.size(), duplicateRow = emissions.size();
        if (!emissions.empty() && ImGui::BeginTable("AuthoredEmissions", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.f);
            ImGui::TableSetupColumn("Offset X (m)");
            ImGui::TableSetupColumn("Offset Y (m)");
            ImGui::TableSetupColumn("Offset Z (m)");
            ImGui::TableSetupColumn("Yaw (deg)");
            ImGui::TableSetupColumn("Start Delay (ms)");
            ImGui::TableSetupColumn("##rowActions", ImGuiTableColumnFlags_WidthFixed, 84.f);
            ImGui::TableHeadersRow();
            for (size_t row = 0; row < emissions.size(); ++row)
            {
                auto& emission = emissions[row];
                ImGui::PushID(static_cast<int>(row));
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row);
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetX", &emission.positionOffset.x, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetY", &emission.positionOffset.y, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetZ", &emission.positionOffset.z, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##yaw", &emission.yawDegrees, .5f, -36000.f, 36000.f, "%.1f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= EditUInt("##delay", emission.startDelayMs, static_cast<int>(maxDelay));
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Dup")) duplicateRow = row;
                ImGui::SameLine();
                if (ImGui::SmallButton("Del")) removeRow = row;
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (duplicateRow < emissions.size() && emissions.size() < 128u)
        { emissions.insert(emissions.begin() + duplicateRow + 1, emissions[duplicateRow]); changed = true; }
        if (removeRow < emissions.size()) { emissions.erase(emissions.begin() + removeRow); changed = true; }
        ImGui::BeginDisabled(emissions.size() >= 128u);
        if (ImGui::Button("Add Emission"))
        {
            WORLD_SEQUENCE_OBJECT_EMISSION emission;
            if (!emissions.empty()) emission = emissions.back();
            emissions.push_back(emission);
            changed = true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(emissions.empty());
        if (ImGui::Button("Clear Emissions")) { emissions.clear(); changed = true; }
        ImGui::EndDisabled();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragInt("Ring Count", &m_RingCount, 1.f, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragFloat("Ring Start (deg)", &m_RingStartDegrees, .5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        if (ImGui::Button("Distribute on Ring"))
        {
            /* A row's yaw also turns Revolution Offset, so a row at phase p rides
               R(p)*o and an offset of R(p)*o puts every row on one circle centred
               on the saved position itself. */
            const vector_t orbitOffset = XMLoadFloat3(&motion.revolutionOffset);
            emissions.clear();
            for (int index = 0; index < m_RingCount; ++index)
            {
                WORLD_SEQUENCE_OBJECT_EMISSION emission;
                emission.yawDegrees = m_RingStartDegrees + 360.f * static_cast<float>(index) / static_cast<float>(m_RingCount);
                XMStoreFloat3(&emission.positionOffset, XMVector3TransformNormal(orbitOffset,
                    XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees))));
                emissions.push_back(emission);
            }
            changed = true;
        }
        ImGui::TextDisabled("Ring rows circle the saved position at this Motion's Revolution Offset radius; use Revolution (deg/s) for the orbit speed.");
        if (!emissions.empty())
        {
            motion.count = static_cast<uint32_t>(emissions.size());
            motion.intervalMs = 0u;
            motion.spreadDegrees = 0.f;
        }
    }
    if (changed) Mark_Dirty();
    if (!alias) Render_EffectRows(*sequence);
    ImGui::SeparatorText("Selected Key");
    Render_KeyEditor(*sequence);
}

void CWorldObjectTool::Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    bool changed = EditText("Group Name", resource.displayName);
    ImGui::TextWrapped("Shared motions: edits below also change the original motions. Save keeps the group and all member settings together.");
    for (const auto& id : resource.motionInstanceIds)
    {
        auto* instance = m_Document.Find_Instance(id);
        auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) { ImGui::TextWrapped("Missing motion: %s", id.c_str()); continue; }
        ImGui::PushID(id.c_str());
        if (ImGui::CollapsingHeader(sequence->displayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::Checkbox("Enabled", &instance->enabled);
            changed |= ImGui::DragFloat3("Map Position", &instance->position.x, .05f, -100000.f, 100000.f, "%.3f");
            auto& motion = sequence->objectMotion;
            const float oldRadius = std::hypot(motion.revolutionOffset.x, motion.revolutionOffset.z);
            float radius = oldRadius;
            ImGui::BeginDisabled(oldRadius < .001f);
            if (ImGui::DragFloat("Orbit Radius (m)", &radius, .05f, .01f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                const float ratio = radius / oldRadius;
                motion.revolutionOffset.x *= ratio; motion.revolutionOffset.z *= ratio;
                for (auto& emission : motion.emissions)
                { emission.positionOffset.x *= ratio; emission.positionOffset.z *= ratio; }
                changed = true;
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Scales the horizontal orbit and authored emission offsets together, keeping the saved centre and phases.");
            ImGui::Text("Count: %u | Lifetime: %u ms", motion.EmissionCount(), sequence->durationMs);
        }
        ImGui::PopID();
    }
    if (changed) Mark_Dirty();
}

void CWorldObjectTool::Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    ImGui::SameLine();
    if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter)) m_PreviewDirty = m_PreviewActive;
    ImGui::TextWrapped("%zu motions play together. Edit each row in Object Detail. Map Position and Orbit Radius remain independent.", resource.motionInstanceIds.size());
    if (!m_PreviewStatus.empty()) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    float clock = m_ClockMs;
    if (ImGui::SliderFloat("Time (ms)", &clock, 0.f, PreviewSpanMs(), "%.0f")) Seek(clock);
    for (const auto& id : resource.motionInstanceIds)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) continue;
        ImGui::TextUnformatted(sequence->displayName.c_str());
        const float start = static_cast<float>(instance->startDelayMs);
        const float duration = static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
        ImGui::ProgressBar(instance->enabled ? (std::clamp)((m_ClockMs - start) / (std::max)(1.f, duration), 0.f, 1.f) : 0.f,
            ImVec2(-1.f, 12.f), instance->enabled ? "" : "Disabled");
    }
}

void CWorldObjectTool::Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    ImGui::SeparatorText(sequence.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine(); if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* selectedInstance = m_Document.Find_Instance(m_SelectedInstance);
    if (selectedInstance && selectedInstance->anchorKind == "WORLD")
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter))
        {
            m_PreviewDirty = m_PreviewActive;
            if (m_PreviewActive) Seek(m_ClockMs);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Preview at the current character without changing the saved Map position. Clear to preview the authored position.");
    }
    if (selectedInstance) ImGui::TextDisabled("On Complete: %s", MotionEndLabel(selectedInstance->motionEnd));
    if (m_PreviewActive) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    if (m_ClockMs >= SpanMs() && selectedInstance &&
        (selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP || selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        ImGui::TextWrapped("Completion motion active. This timeline still edits the selected Motion's first Lifetime; seek to 0 to restart it.");
    float clock = (std::min)(m_ClockMs, SpanMs());
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, (std::max)(1.f, SpanMs()), "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    const float rowHeight = 32.f;
    const uint32_t timelineDuration = sequence.PresentationSpanMs();
    const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, timelineDuration * m_Zoom * .001f);
    const float pixelsPerMs = width / timelineDuration;
    const bool showPhysics = resource && resource->sequenceInstanceId.empty();
    const float tracksHeight = rowHeight * static_cast<float>(sequence.tracks.size() + sequence.animationTracks.size() + sequence.effectTracks.size());
    const float height = 28.f + tracksHeight + (showPhysics ? 64.f : 0.f);
    if (ImGui::BeginChild("ObjectTimeline", ImVec2(0, (std::max)(110.f, ImGui::GetContentRegionAvail().y)), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), timelineDuration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
        {
            const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
            if (instance) Seek(instance->startDelayMs + local / instance->playbackSpeed);
        }
        for (size_t index = 0; index < sequence.tracks.size(); ++index)
        {
            auto& track = sequence.tracks[index]; ImGui::PushID(track.slotId.c_str());
            const auto row = ImVec2(origin.x, origin.y + 28.f + rowHeight * index);
            CompositionTimeline::DrawBox(draw, row, ImVec2(row.x + sequence.durationMs * pixelsPerMs, row.y + 25.f),
                IM_COL32(61, 107, 141, 255), m_SelectedTrack == index, track.slotId.c_str(), false, false);
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, ImVec2(row.x + width, row.y + 25.f)) && ImGui::IsMouseClicked(0))
            { m_SelectedTrack = index; m_SelectedKey = 0; }
            for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
            {
                auto& key = track.keys[keyIndex]; const float x = row.x + key.timeMs * pixelsPerMs;
                const float y = row.y + 12.f;
                const ImU32 color = m_SelectedTrack == index && m_SelectedKey == keyIndex ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE;
                draw->AddQuadFilled(ImVec2(x, y - 6), ImVec2(x + 6, y), ImVec2(x, y + 6), ImVec2(x - 6, y), color);
                ImGui::PushID(static_cast<int>(keyIndex)); ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, row.x, row.x + width - 14.f), row.y + 4.f));
                ImGui::InvisibleButton("Key", ImVec2(14, 18));
                if (ImGui::IsItemClicked()) { m_SelectedTrack = index; m_SelectedKey = keyIndex; }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                {
                    const auto moved = static_cast<int>((ImGui::GetIO().MousePos.x - row.x) / pixelsPerMs);
                    const uint32_t time = static_cast<uint32_t>((std::clamp)(moved, static_cast<int>(track.keys[keyIndex - 1].timeMs + 1), static_cast<int>(track.keys[keyIndex + 1].timeMs - 1)));
                    if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            const auto& track = sequence.animationTracks[index];
            uint32_t end = sequence.durationMs;
            for (const auto& next : sequence.animationTracks) if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, next.startMs);
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + index);
            CompositionTimeline::DrawBox(draw, ImVec2(origin.x + track.startMs * pixelsPerMs, y),
                ImVec2(origin.x + end * pixelsPerMs, y + 25.f), IM_COL32(113, 82, 147, 255), false,
                track.displayName.empty() ? track.clipName.c_str() : track.displayName.c_str());
        }
        for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
        {
            const auto& effect = sequence.effectTracks[index];
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + sequence.animationTracks.size() + index);
            const float x = origin.x + sequence.EffectStartMs(effect) * pixelsPerMs;
            const float endX = x + effect.durationMs * pixelsPerMs;
            CompositionTimeline::DrawBox(draw, ImVec2(x, y), ImVec2(endX, y + 25.f),
                IM_COL32(167, 95, 51, 255), m_SelectedEffectRow == index, effect.resourceId.c_str());
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(ImVec2(x, y), ImVec2(endX, y + 25.f)) && ImGui::IsMouseClicked(0))
                m_SelectedEffectRow = index;
        }
        if (showPhysics)
        {
            const float top = origin.y + 28.f + tracksHeight;
            draw->AddRectFilled(ImVec2(origin.x, top), ImVec2(origin.x + width, top + 60.f), IM_COL32(28, 49, 48, 255));
            draw->AddText(ImVec2(origin.x + 5.f, top + 3.f), IM_COL32(136, 227, 198, 255), "Physics Y offset / first emission");
            const auto& motion = sequence.objectMotion;
            const float seconds = sequence.durationMs * .001f;
            const float physicsWidth = sequence.durationMs * pixelsPerMs;
            const auto sampleY = [&motion](float time) { return motion.velocity.y * time + .5f * motion.acceleration.y * time * time; };
            float minimum = (std::min)(0.f, sampleY(seconds));
            float maximum = (std::max)(0.f, sampleY(seconds));
            if (motion.acceleration.y != 0.f)
            {
                const float apex = -motion.velocity.y / motion.acceleration.y;
                if (apex > 0.f && apex < seconds)
                { minimum = (std::min)(minimum, sampleY(apex)); maximum = (std::max)(maximum, sampleY(apex)); }
            }
            const float range = (std::max)(.01f, maximum - minimum);
            if (std::isfinite(motion.velocity.y) && std::isfinite(motion.acceleration.y) &&
                std::isfinite(minimum) && std::isfinite(maximum) && std::isfinite(range))
                for (int segment = 0; segment < 64; ++segment)
                {
                    const float from = static_cast<float>(segment) / 64.f, to = static_cast<float>(segment + 1) / 64.f;
                    draw->AddLine(ImVec2(origin.x + physicsWidth * from, top + 56.f - 30.f * (sampleY(seconds * from) - minimum) / range),
                        ImVec2(origin.x + physicsWidth * to, top + 56.f - 30.f * (sampleY(seconds * to) - minimum) / range), IM_COL32(91, 217, 171, 255), 2.f);
                }
            ImGui::SetCursorScreenPos(ImVec2(origin.x, top));
            ImGui::InvisibleButton("PhysicsSeek", ImVec2(width, 60.f));
            if (ImGui::IsItemActive() && selectedInstance)
            {
                const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
                Seek(selectedInstance->startDelayMs + local / selectedInstance->playbackSpeed);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Drag to seek. Y offset range: %.3f to %.3f m. Added to Transform keys before spread, revolution and anchor rotation.", minimum, maximum);
        }
        const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
        const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
        const float cursorX = origin.x + (std::clamp)(local, 0.f, static_cast<float>(timelineDuration)) * pixelsPerMs;
        draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, origin.y + height), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!sequence.tracks.empty())
    {
        m_SelectedTrack = (std::min)(m_SelectedTrack, sequence.tracks.size() - 1);
        if (ImGui::BeginCombo("Target Track", sequence.tracks[m_SelectedTrack].slotId.c_str()))
        {
            for (size_t index = 0; index < sequence.tracks.size(); ++index)
                if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(), index == m_SelectedTrack)) { m_SelectedTrack = index; m_SelectedKey = 0; }
            ImGui::EndCombo();
        }
        auto& track = sequence.tracks[m_SelectedTrack];
        if (!track.keys.empty())
        {
            m_SelectedKey = (std::min)(m_SelectedKey, track.keys.size() - 1);
            auto& key = track.keys[m_SelectedKey];
            if (ImGui::BeginCombo("Keyframe", (std::to_string(key.timeMs) + " ms").c_str()))
            {
                for (size_t index = 0; index < track.keys.size(); ++index)
                    if (ImGui::Selectable((std::to_string(track.keys[index].timeMs) + " ms").c_str(), m_SelectedKey == index)) m_SelectedKey = index;
                ImGui::EndCombo();
            }
            auto& selected = track.keys[m_SelectedKey];
            const bool endpoint = m_SelectedKey == 0 || m_SelectedKey + 1 == track.keys.size();
            bool changed = false;
            ImGui::BeginDisabled(endpoint);
            if (!endpoint) changed |= EditUInt("Key Time (ms)", selected.timeMs, track.keys[m_SelectedKey + 1].timeMs - 1, track.keys[m_SelectedKey - 1].timeMs + 1);
            else ImGui::Text("Endpoint: %u ms", selected.timeMs);
            ImGui::EndDisabled();
            changed |= ImGui::DragFloat3("Position Offset (m)", &selected.positionOffset.x, .01f);
            auto degrees = QuaternionEuler(selected.rotationQuaternion);
            if (ImGui::DragFloat3("Rotation Offset (deg)", &degrees.x, .25f))
            {
                XMStoreFloat4(&selected.rotationQuaternion, XMQuaternionRotationRollPitchYaw(
                    XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
                changed = true;
            }
            changed |= ImGui::DragFloat3("Scale Multiplier", &selected.scaleMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Visible", &selected.visible);
            if (changed) Mark_Dirty();
            ImGui::BeginDisabled(endpoint);
            if (ImGui::Button("Delete Key")) { track.keys.erase(track.keys.begin() + m_SelectedKey); m_SelectedKey = 0; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT || sequence.durationMs < 2);
            if (ImGui::Button("Add Key at Cursor"))
            {
                const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
                const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
                const uint32_t time = static_cast<uint32_t>((std::clamp)(local, 1.f, static_cast<float>(sequence.durationMs - 1)));
                auto at = std::lower_bound(track.keys.begin(), track.keys.end(), time, [](const auto& value, uint32_t clock) { return value.timeMs < clock; });
                m_SelectedKey = at - track.keys.begin();
                if (at == track.keys.end() || at->timeMs != time)
                {
                    auto added = CWorldSequencePlayer::Sample_Track(sequence, track, static_cast<float>(time)); added.timeMs = time;
                    track.keys.insert(at, added); Mark_Dirty();
                }
            }
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Animation Clips"))
    {
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            auto& clip = sequence.animationTracks[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Slot: %s", clip.slotId.c_str());
            bool changed = EditText("Clip display name", clip.displayName);
            ImGui::TextWrapped("Native clip: %s", clip.clipName.c_str());
            uint32_t minimum = 0, maximum = sequence.durationMs - 1;
            bool first = true;
            for (size_t other = 0; other < sequence.animationTracks.size(); ++other)
            {
                const auto& next = sequence.animationTracks[other];
                if (other == index || next.slotId != clip.slotId) continue;
                if (next.startMs < clip.startMs) { minimum = (std::max)(minimum, next.startMs + 1); first = false; }
                if (next.startMs > clip.startMs) maximum = (std::min)(maximum, next.startMs - 1);
            }
            ImGui::BeginDisabled(first); changed |= EditUInt("Clip Start (ms)", clip.startMs, maximum, minimum); ImGui::EndDisabled();
            changed |= ImGui::DragFloat("Clip Speed", &clip.playbackRate, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Clip Loop", &clip.loop); changed |= ImGui::Checkbox("Hold Last Pose", &clip.holdLastFrame);
            if (changed) Mark_Dirty();
            if (ImGui::SmallButton("Remove Clip"))
            {
                const auto slot = clip.slotId; sequence.animationTracks.erase(sequence.animationTracks.begin() + index);
                for (auto& remaining : sequence.animationTracks) if (remaining.slotId == slot) { remaining.startMs = 0; break; }
                Mark_Dirty(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        ImGui::TextWrapped("Select a native clip in Object Resources, then Append Clip to this Motion.");
    }
}

void CWorldObjectTool::Rebuild_PhysicalTree()
{
    m_PhysicalTree = {};
    const auto search = Lower(m_PhysicalSearch.data());
    for (size_t index = 0; index < m_PhysicalAssets.size(); ++index)
    {
        const auto& asset = m_PhysicalAssets[index];
        if ((m_PhysicalSlot == 0) != (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)) continue;
        if (!search.empty() && Lower(asset.assetId).find(search) == std::string::npos) continue;
        std::vector<std::string> segments;
        for (const auto& segment : std::filesystem::path(asset.assetId).parent_path()) segments.push_back(segment.string());
        InsertResourceTree(m_PhysicalTree, segments, index);
    }
    FinalizeResourceTree(m_PhysicalTree);
}

void CWorldObjectTool::Render_PhysicalResources()
{
    ImGui::SeparatorText("Physical Resources");
    if (!m_SelectedInstance.empty()) ImGui::TextWrapped("Select the parent Object to change its shared model or texture.");
    if (!m_PhysicalScanned)
    {
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
        m_PhysicalScanned = true;
    }
    ImGui::BeginDisabled(m_PhysicalScanRunning);
    if (ImGui::Button("Refresh Files"))
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
    ImGui::EndDisabled();
    if (m_PhysicalScanRunning)
    { ImGui::SameLine(); ImGui::TextDisabled("Scanning files..."); }
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##AssignSlot", &m_PhysicalSlot, "Model (.wmodel)\0Diffuse (.dds)\0")) Rebuild_PhysicalTree();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##PhysicalSearch", "Search full relative path", m_PhysicalSearch.data(), m_PhysicalSearch.size())) Rebuild_PhysicalTree();
    ImGui::TextDisabled("%zu matching files", m_PhysicalTree.iRecursiveLeafCount);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_PhysicalStatus.c_str());
    if (ImGui::BeginChild("PhysicalResourceFolders", ImVec2(0, (std::max)(100.f, ImGui::GetContentRegionAvail().y * .40f)), true))
    {
        RenderResourceTree(m_PhysicalTree, [this](size_t index) {
            if (index >= m_PhysicalAssets.size()) return;
            const auto& asset = m_PhysicalAssets[index];
            ImGui::PushID(asset.assetId.c_str());
            auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
            ImGui::BeginDisabled(!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty());
            if (ImGui::Selectable(asset.fileName.c_str(), asset.assetId == m_SelectedPhysical &&
                (asset.kind != PHYSICAL_RESOURCE_KIND::MODEL || m_AnimationCandidateObjectId == m_SelectedObject)))
            {
                m_SelectedPhysical = asset.assetId;
                if (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)
                {
                    m_AnimationCandidateModelAssetId = asset.assetId;
                    m_AnimationCandidateObjectId = m_SelectedObject;
                    Refresh_AnimationResources();
                    m_Status = "Model candidate selected. Assign Model updates the parent Object; its child motions keep their clip bindings.";
                }
                else
                {
                    resource->diffuseTextureAssetId = asset.assetId;
                    const auto pristinePattern = m_PristinePatternId;
                    Mark_Dirty();
                    m_PristinePatternId = pristinePattern;
                    m_Status = "Assigned diffuse " + asset.assetId + ". Save stores it on " + resource->displayName + ".";
                }
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", asset.assetId.c_str());
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
}
#endif

```

## Client/Public/Level_KakulSaydonArena.h 전체 반영 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "ArenaCameraProfile.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Effect_PresentationService.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "StatusEffectTextView.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanCinematicCameraController.h"
#include "WorldSequencePlayer.h"

#include <array>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CNpc;
class CTrigger_Box;
class IPlayerCommandSink;
class IWorldEntityCommandSink;

class CUILayoutRuntime;
class CKoukuMadnessGaugeView;

class CLevel_KakulSaydonArena final : public CLevel
{
public:
	void Set_MapLightAuthoringOverride(std::shared_ptr<CMapLightPresentationRuntime> lights) { m_pMapLightAuthoringOverride = std::move(lights); }
	bool_t Reload_MapLights();
	struct KAKUL_STAGE_MARKER final
	{
		std::string strStageId;
		std::string strPlacementId;
		std::string strDisplayNameKo;
		std::string strSourceLevelId;
	};

	/* One authored camera shot. While the local Character stands inside the
	   box the camera holds this exact pose - the reference footage keeps the
	   background pinned while the party walks - and leaving the box hands the
	   camera back to the ordinary follow view. */
	struct KAKUL_CAMERA_SHOT final
	{
		std::string strShotId;
		std::string strDisplayName;
		uint32_t iDefaultHoldMs = 3000u;
		bool_t bPatternOnly = false;
		VALTAN_CINEMATIC_CAMERA_EASING eTransitionEasing = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		/* Empty means the box decides. When it names a sequence the
		   shot holds for exactly as long as that sequence plays, so a
		   trigger that starts the sequence also starts the shot. */
		std::string strSequenceInstanceId;
		float3_t vCenter = {};
		float3_t vHalfExtents = {};
		f32_t fYawDegrees = 0.f;
		float3_t vEye = {};
		float3_t vLookAt = {};
		f32_t fFovYDegrees = 60.f;
		uint32_t iBlendInMs = 0u;
		uint32_t iBlendOutMs = 0u;
		uint32_t iPriority = 0u;
		/* A side scrolling stage keeps one framing and slides it with the
		   local Character instead of pinning it in place. Both offsets are
		   added to that Character's position, so the authored eye and lookAt
		   stay as the pose used while no Character exists. */
		bool_t followsPlayer = false;
		float3_t vFollowEyeOffset = {};
		float3_t vFollowLookAtOffset = {};
		/* A shot without a track keeps the single authored pose. With one
		   it is sampled on the bound sequence's own clock by the one
		   cinematic sampler this project owns, so a second easing or
		   spline implementation can never drift from it. */
		bool_t hasCameraTrack = false;
		VALTAN_CINEMATIC_CAMERA_CUE CameraTrack;
	};

private:
	CLevel_KakulSaydonArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	using WORLD_EMISSION_ANCHOR = std::function<bool_t(f32_t, float4x4_t&)>;
	using WORLD_EMISSION_RESOLVER = std::function<bool_t(std::uint32_t, std::string_view, std::string_view, WORLD_EMISSION_ANCHOR&)>;
	void Set_CompositionWorldEmissionResolver(WORLD_EMISSION_RESOLVER resolver)
	{ m_WorldEmissionResolver = std::move(resolver); }
	virtual ~CLevel_KakulSaydonArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	const ARENA_CAMERA_PROFILE& Get_FollowCameraProfile() const
	{ return m_FollowCameraProfile; }
	const std::string& Get_FollowCameraProfileStatus() const
	{ return m_strFollowCameraProfileStatus; }
	bool_t Set_FollowCameraProfile(const ARENA_CAMERA_PROFILE& profile,
		std::string& outStatus);


	static CLevel_KakulSaydonArena* Get_Active()
	{
		return s_pActiveInstance;
	}
	void Collect_MinimapMarkers(
		CClientReplication::MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
	{
		m_Replication.Collect_MinimapMarkers(outSnapshot);
	}

	/* Read-only replicated presentation, including the current madness avatar. */
	shared_ptr<CCharacter> Get_LocalCharacter() const
	{
		return m_Replication.Get_LocalCharacter();
	}

#ifdef _DEBUG
	void Set_DebugGazeView(bool visible, float halfAngleDegrees, float distanceM)
	{ m_bDebugGazeView = visible; m_fDebugGazeHalfAngle = halfAngleDegrees; m_fDebugGazeDistance = distanceM; }
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
	struct COMPOSITION_WORLD_PREVIEW_CUE final
	{
		std::string occurrenceId;
		std::string instanceId;
		uint32_t startMs = 0u;
		uint32_t durationMs = 0u;
		f32_t playbackSpeed = 1.f;
		float3_t positionOffset{};
		std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
		WORLD_EMISSION_ANCHOR emissionAnchor;
		std::string bossArchetypeId;
		std::string actorProfileId;
	};
	bool_t Debug_BeginCompositionWorldPreview(const std::string& patternId,
		std::vector<COMPOSITION_WORLD_PREVIEW_CUE> cues, std::string& status,
		const CWorldSequenceDocument* sourceDocument = nullptr);
	bool_t Debug_HasVisibleCompositionWorldBox(std::string_view occurrenceId) const;
	bool_t Debug_SetCompositionWorldPlacement(const std::string& occurrenceId,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement, std::string& status);
	bool_t Debug_SampleCompositionWorldPreview(const std::string& patternId,
		bool_t playing, uint32_t clockMs, std::string& status);
	void Debug_StopCompositionWorldPreview();
	// Applies immediately and remembers this arena's value until process exit.
	bool_t Set_DebugCameraSpeed(f32_t metersPerSecond);

	/* One F1 "KoukuSaydon Arena" gate button. The Server raises the named
	   disabled boss placements, moves only this player to the fixed position
	   through the Debug teleport contract, and the HUD follows one archetype.
	   Positions are Debug authoring values captured from Move Player; the
	   Server still validates navigation, height and collision. A gate with a
	   deferred reason has no navigation yet and only reports that reason. */
	struct KAKUL_DEBUG_GATE final
	{
		const char_t* pLabel = nullptr;
		std::array<const char_t*, 2> BossPlacementIds = { nullptr, nullptr };
		float3_t vPlayerPosition = {};
		const char_t* pHudFocusArchetypeId = nullptr;
		/* Placement of pHudFocusArchetypeId: the boss the Kouku Boss Tool and
		   Complete Play target after this gate is raised. Null keeps the
		   Gate 1 Kouku target. */
		const char_t* pAuditionPlacementId = nullptr;
		const char_t* pDeferredReason = nullptr;
	};
	static constexpr size_t NO_ACTIVE_DEBUG_GATE = static_cast<size_t>(-1);
	static const std::array<KAKUL_DEBUG_GATE, 9>& Get_DebugGates();
	/* Despawns the previous gate bosses, requests this gate's placements,
	   submits the player teleport, points the HUD and the pattern audition at
	   the gate boss. Every step is a typed Server command; nothing local is
	   spawned or moved. */
	bool_t Debug_ActivateGate(size_t gateIndex, std::string& outStatus);
	bool_t Debug_DespawnArenaBosses(std::string& outStatus);
	size_t Get_ActiveDebugGate() const { return m_iActiveDebugGate; }
	bool_t Is_DebugGatePending() const { return NO_ACTIVE_DEBUG_GATE != m_iPendingDebugGate; }
	const std::string& Get_DebugGateStatus() const { return m_strDebugGateStatus; }
	/* Debug tuning only: the live body of one arena boss archetype. */
	std::shared_ptr<CNpc> Debug_FindArenaBossNpc(std::string_view archetypeId) const
	{
		return m_Replication.Find_ArenaBossNpc(archetypeId);
	}
#endif

	// The level owns the replicated player anchor used by local authoring previews.
	bool_t Try_Get_AuthoringPreviewPlacement(
		float3_t& outPosition, std::string& outStatus) const;
	bool_t Try_Get_AuthoringForwardPlacement(
		float3_t& outPosition, std::string& outStatus) const;

	/* The F1 stage selector submits only stable authored placement IDs through
	   the typed Server command sink. Until an authored StageMarkers contract is
	   loaded, the empty allow-list rejects every request instead of inventing a
	   waypoint or teleporting the local Character. */
	bool_t Request_StageTeleport(
		std::uint32_t requestSequence,
		std::string_view placementId,
		std::string& outStatus);
	const std::vector<KAKUL_STAGE_MARKER>& Get_StageMarkers() const
	{
		return m_StageMarkers;
	}
	void Collect_KoukuPresentationViews(std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
		std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const
	{ m_Replication.Collect_KoukuPresentationViews(bosses, cards); }
	void Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const
	{ m_Replication.Collect_KoukuMazeTargets(targets); }
	bool_t Sample_CompositionCamera(std::string_view shotId, float seconds, const float3_t& offset, std::string_view ownerKey, uint32_t durationMs, bool_t preview);
	void Stop_CompositionCamera(bool_t force = false);
	bool_t Try_GetCompositionWorldPivot(std::string_view instanceId, float4x4_t& out,
		std::string_view occurrenceId = {}, std::uint32_t emissionIndex = 0u) const;
    bool_t Create_CompositionPreviewActor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        std::shared_ptr<CNpc>& outActor, std::string& status);
    void Release_CompositionPreviewActor(const std::shared_ptr<CNpc>& actor);
    CWorldSequencePlayer::TARGET_SET Get_CompositionWorldTargets() { return Make_WorldSequenceTargets(); }
	// Authoring inventory reads the placed centre even before any sequence plays.
	bool_t Try_GetWorldSequencePlacementBaseline(const WORLD_SEQUENCE_INSTANCE& instance,
		float3_t& outPosition) const;
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const { return m_pPlayerCommandSink; }
	const CWorldSequenceDocument& Get_WorldSequenceDocument() const { return m_SequencePlayer.Get_Document(); }
	const LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& Get_KoukuBundleState() const { return m_Replication.Get_KoukuBundleState(); }
	std::uint32_t Get_PresentationServerTick() const { return m_Replication.Get_LastServerTick(); }
    bool_t Can_StartCompositionWorld(const std::string& instanceId, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr) const;
	bool_t Try_GetOwnedCompositionWorldPivot(std::uint32_t runEpoch, const std::string& memberId,
		const std::string& sequenceId, const std::string& cueId, float4x4_t& out, std::uint32_t emissionIndex = 0u) const;
	void Get_WorldObjectValidationTargets(WORLD_SEQUENCE_PLACEMENT_MAP&, WORLD_SEQUENCE_DEPLOY_MAP&) const;
	bool_t Reload_WorldObjectRuntime(std::string& status);
#ifdef _DEBUG
	bool_t Debug_BeginWorldObjectPreview(const CWorldSequenceDocument&, const std::string& instanceId,
		std::string& status, bool_t previewAtCharacter = true);
	bool_t Debug_SampleWorldObjectPreview(f32_t clockMs, std::string& status);
	void Debug_StopWorldObjectPreview();
#endif
	const std::vector<KAKUL_CAMERA_SHOT>& Get_PublishedCameraShots() const { return m_CameraShots; }
	bool_t Reload_PublishedCameraShots(std::string& outStatus) { return Load_CameraShots(outStatus); }
	bool_t Ensure_CameraShotAuthoring(std::string& outStatus);
	bool_t Create_CameraShot(std::string_view name, std::string& outShotId, std::string& outStatus);
	bool_t Update_CameraShot(const KAKUL_CAMERA_SHOT& shot, std::string& outStatus);
	bool_t Capture_CameraShot(std::string_view shotId, std::string& outStatus);
	bool_t Save_CameraShots(std::string& outStatus);
	static bool_t Parse_CameraShots(std::string_view text, std::vector<KAKUL_CAMERA_SHOT>& outShots, std::string& outStatus);
	static VALTAN_CINEMATIC_CAMERA_CUE CameraShot_ToCue(const KAKUL_CAMERA_SHOT& shot);
	static bool_t Stage_PatternCameraTracks(std::string_view baseline,
		const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues, const std::map<std::string, std::string>& names,
		std::string& outText, std::string& outStatus);
	bool_t Save_CameraShotSource(std::string_view expectedSource, const std::string& text, std::string& outStatus);

	const std::vector<KAKUL_CAMERA_SHOT>& Get_CameraShots() const
	{
		return m_bCameraAuthoringLoaded ? m_AuthoringCameraShots : m_CameraShots;
	}

	/* Raises one paper stage bridge: the Deploy prop leaves DESPAWNED and its
	   authored unfold sequence starts on the same frame, so the bridge is
	   never visible in its finished pose before it has unfolded. Playing an
	   already raised bridge is a no-op rather than a rewind. */
	bool_t Request_PaperBridgeUnfold(
		uint64_t leverPlacementId,
		std::string& outStatus);

private:
	CWorldSequencePlayer::TARGET_SET Make_WorldSequenceTargets();
	/* The cutscene is one show spread over several instances. Starting the
	   named one starts them all and swaps the arena for the cutscene copy. */
	bool_t Start_PopupBookCutscene(
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus);
	void Apply_CutsceneSetVisible(bool_t cutsceneVisible);
	/* The cutscene boss is presentation only, so it is taken off the arena
	   as soon as its sequence stops playing. */
	void Update_CutsceneBossRetire(
		const CWorldSequencePlayer::TARGET_SET& targets);
	bool_t Start_ServerRequestedSequence(
		const std::string& instanceId, f32_t playbackSpeed, const float3_t& positionOffset,
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus, uint32_t durationMs = 0u,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement = {});
	bool_t Load_StageMarkers(std::string& outStatus);
	bool_t Load_CameraShots(std::string& outStatus);
	void Update_CameraShots(f32_t fTimeDelta);
	void Update_CompositionCamera(f32_t fTimeDelta);
	bool_t Resolve_CompositionFollowPose(VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	/* The arena and the Mario gimmick are more than a kilometre apart, so a
	   trigger move between them is hidden behind a black screen instead of
	   letting the camera travel that distance on screen. Server owns the
	   move; this only reads the action state it already replicates. */
	void Update_TriggerMoveFade(f32_t fTimeDelta);
	bool_t Load_EntranceTriggerMarkers();
	void Clear_EntranceTriggerMarkers();
	void Update_EntranceTriggerMarkers(f32_t deltaSeconds);
	void Retire_EntranceTriggerMarker(const std::string& sequenceInstanceId);
	/* Turns replicated player state into floating status words. Reads the
	   snapshots only; it never decides that a status is on. */
	void Update_StatusEffectText(f32_t fTimeDelta);
	void Update_CardMazePresentation(f32_t fTimeDelta);
	void Update_MarioBallBouncePresentation(f32_t fTimeDelta);
	void Update_MarioLayoutPresentation();
	std::string m_strMarioLayoutInstance;
	bool_t m_bMarioLayoutFailed = false;
	bool_t m_bMarioLayoutStarted = false;
	std::uint32_t m_iMarioBallBounceSnapshotTick = 0u;
	f32_t m_fMarioBallBounceSnapshotSeconds = 0.f;
	bool_t m_bMarioBallBounceRunning = false;
	bool_t m_bMarioBallBounceFailed = false;
	/* Server-popped source balls of the current layout: a newly set slot bit
	   hides that binding through the sequence player and plays the ball's
	   smoke leaf once; a newly set curse bit queues the centred notice. */
	void Update_MarioBallPresentation(f32_t timeDelta);
	std::string m_strMarioBallLayoutInstance;
	std::uint16_t m_iMarioPoppedBallsSeen = 0u;
	std::uint8_t m_iMarioCurseSeen = 0u;
	std::uint8_t m_iMarioCurseNoticeQueue = 0u;
	std::int32_t m_iMarioCurseNoticeColor = -1;
	f32_t m_fMarioCurseNoticeSeconds = 0.f;
	std::array<bool_t, 3u> m_bMarioBallSmokeFailed = {};
	// Presentation-only launch markers use published world positions and the existing object player.
	struct MARIO_BOMB_EMITTER
	{
		std::uint8_t stage = 0u;
		std::uint32_t seed = 1u, phaseMs = 0u, durationMs = 0u;
		std::vector<std::string> slots;
		std::vector<std::int64_t> births;
		bool_t failed = false;
	};
	bool_t Ready_MarioBombPresentation(std::string& status);
	void Update_MarioBombPresentation(f32_t timeDelta);
	std::unique_ptr<CWorldSequencePlayer> m_pMarioBombPlayer;
	std::vector<MARIO_BOMB_EMITTER> m_MarioBombEmitters;
	std::uint8_t m_iMarioBombStage = 0u;
	double m_fMarioBombStageStartMs = 0.;
	std::uint32_t m_iMarioBombSnapshotTick = 0u;
	f32_t m_fMarioBombSnapshotSeconds = 0.f;
	bool_t m_bMarioBombLoadAttempted = false;
	std::uint32_t m_iCardMazeLastSnapshotTick = 0u;
	f32_t m_fCardMazeSnapshotSeconds = 0.f;
	bool m_bCardMazeMarchPlaying = false;
	void Update_DeadScene(f32_t fTimeDelta);
	const KAKUL_CAMERA_SHOT* Find_ActiveCameraShot(
		const float3_t& vPosition) const;
	void Release_CameraShot();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

#ifdef _DEBUG
	/* The three arena-side `_go` boxes are the only way into the Mario
	   stages and each is 2x1x2m of empty air, so nobody can find them
	   without a wire. This draws them through the same authoring box the
	   Map Editor and Bern already use; it reads the authored document and
	   owns no gameplay state. */
	bool_t Ready_DebugStageEntryTriggers(const std::string& areaId);
#endif

private:
	CMapPlacementRuntime m_MapRuntime;
	/* The authored deploy catalog carries both paper levers and both paper
	   stage bridges. A bridge stays DESPAWNED until its lever is pulled, so
	   suppress the bridges before the first rendered frame instead of letting
	   them appear already unfolded. */
	CDeployPropRuntime m_DeployRuntime;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightAuthoringOverride;
	CWorldSequencePlayer m_SequencePlayer;
	bool_t m_bWorldObjectReloadPending = false;
	struct OWNED_WORLD_CUE final
	{
		std::uint32_t runEpoch = 0, startTick = 0, durationMs = 0;
		std::string memberId, cueId, occurrenceId, sequenceId;
		float clockMs = 0.f;
		WORLD_EMISSION_ANCHOR emissionAnchor;
		std::shared_ptr<CWorldSequencePlayer> player;
	};
	WORLD_EMISSION_RESOLVER m_WorldEmissionResolver;
	std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_PendingOwnedWorldCues;
	std::map<std::string, OWNED_WORLD_CUE> m_OwnedWorldCues;
	std::set<std::string> m_StoppedWorldOwners;
	std::set<std::string> m_ConsumedWorldCueIds;
	std::uint32_t m_iLatestWorldRunEpoch = 0u;
	void Consume_OwnedWorldCue(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play,
		const CWorldSequencePlayer::TARGET_SET& targets);
#ifdef _DEBUG
	unique_ptr<CWorldSequencePlayer> m_pWorldObjectPreview;
	std::vector<std::string> m_WorldObjectPreviewInstances;
#endif
#ifdef _DEBUG
	struct COMPOSITION_WORLD_PREVIEW_PLAYBACK final
	{
		COMPOSITION_WORLD_PREVIEW_CUE cue;
		unique_ptr<CWorldSequencePlayer> player;
	};
	std::map<std::string, COMPOSITION_WORLD_PREVIEW_PLAYBACK> m_CompositionWorldPreviewCues;
	struct COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE final
	{
		uint64_t placementId = 0u;
		DEPLOY_PROP_STATE previousState = DEPLOY_PROP_STATE::INTACT;
		DEPLOY_PROP_STATE appliedState = DEPLOY_PROP_STATE::INTACT;
	};
	// Preview borrows bound Deploy states, a replaced book and exclusive arena visibility.
	std::vector<COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE> m_CompositionWorldPreviewDeployStates;
	std::vector<std::pair<uint64_t, bool_t>> m_CompositionWorldPreviewArenaVisibility;
	std::string m_strCompositionWorldPreviewPattern;
	bool_t m_bCompositionWorldPreviewClockBound = false;
	bool_t m_bCompositionWorldPreviewStandingArenaVisible = false;
#endif
	bool_t m_bCutsceneBossVisible = false;
	bool_t m_bCutsceneSetVisible = false;
	std::unordered_set<uint64_t> m_RaisedPaperBridges;
	shared_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	ARENA_CAMERA_PROFILE m_FollowCameraProfile =
		CArenaCameraProfile::Default(ARENA_CAMERA_MAP::KOUKU_SAYDON);
	std::string m_strFollowCameraProfileStatus;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	std::vector<KAKUL_STAGE_MARKER> m_StageMarkers;
	std::unordered_set<std::string> m_StageMarkerPlacementIds;
	std::vector<KAKUL_CAMERA_SHOT> m_CameraShots;
	std::vector<KAKUL_CAMERA_SHOT> m_AuthoringCameraShots;
	std::string m_strCameraAuthoringBaseline;
	std::set<std::string> m_DirtyCameraShotIds;
	bool_t m_bCameraAuthoringLoaded = false;
	struct COMPOSITION_CAMERA_TRANSITION final
	{
		std::string ownerKey;
		std::string cancelledOwnerKey;
		VALTAN_CINEMATIC_CAMERA_POSE fromPose;
		VALTAN_CINEMATIC_CAMERA_POSE entryPose;
		f32_t lastSeconds = -1.f;
		VALTAN_CINEMATIC_CAMERA_POSE appliedPose;
		VALTAN_CINEMATIC_CAMERA_EASING easing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		uint32_t blendOutMs = 0u;
		f32_t returnSeconds = 0.f;
		bool_t returning = false;
		bool_t followAtStart = true;
	} m_CompositionCamera;
	std::string m_strActiveCameraShotId;
	/* The pose written last frame. A hand-over starts from this, so entering,
	   swapping and leaving all begin at what the player already sees. */
	float3_t m_vCameraEyeApplied = {};
	float3_t m_vCameraLookApplied = {};
	f32_t m_fCameraFovApplied = 60.f;
	float3_t m_vCameraEyeFrom = {};
	float3_t m_vCameraLookFrom = {};
	f32_t m_fCameraFovFrom = 60.f;
	float3_t m_vCameraEyeTo = {};
	float3_t m_vCameraLookTo = {};
	f32_t m_fCameraFovTo = 60.f;
	f32_t m_fCameraBlendSeconds = 0.f;
	f32_t m_fCameraBlendElapsed = 0.f;
	bool_t m_bCameraShotHeld = false;
	std::string m_strCameraShotStatus;
#ifdef _DEBUG
	/* Debug gate command sequence and the accumulated Server replies shown in
	   the F1 arena panel. Session state only; never persisted. */
	bool m_bDebugGazeView = false;
	float m_fDebugGazeHalfAngle = 45.f;
	float m_fDebugGazeDistance = 30.f;
	std::uint32_t m_iNextDebugGateRequestSequence = 1u;
	/* Index into Get_DebugGates() of the gate whose bosses are raised now;
	   that button stays disabled until another gate or Despawn is chosen. */
	size_t m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	size_t m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
	std::unordered_set<std::string> m_DebugGatePendingPlacements;
	bool_t m_bDebugGateFailed = false;
	std::string m_strDebugGateStatus =
		"Choose a gate. The Server raises its bosses and moves only your player.";
	/* Last F1 status-word preview serial already turned into a word. */
	std::uint32_t m_iStatusEffectTextPreviewSerial = 0u;
#endif
	/* One full-screen slot, black, whose alpha is the whole effect. Built
	   hidden so the first rendered frame after activation cannot flash it. */
	unique_ptr<CUILayoutRuntime> m_pTriggerMoveFadeView;
	unique_ptr<CUILayoutRuntime> m_pDeadSceneView;
	/* Madness gauge under the local character. Reads CCombatHUDViewModel's
	   KoukuSaydon gimmick state only; hidden while that state is invalid. */
	unique_ptr<CKoukuMadnessGaugeView> m_pMadnessGaugeView;
	/* Floating status word over a head (currently the Server FEAR state). Owns no
	   gameplay truth: Update submits one word per replicated FEAR occurrence and
	   Render draws whatever is still inside its motion. */
	CStatusEffectTextView m_StatusEffectTextView;
	f32_t m_fTriggerMoveFadeAlpha = 0.f;
	/* Speed gate. The short hops share TRIGGER_MOVE with the stage
	   transition, so the fade arms only once the character is seen moving
	   far faster than any hop can. */
	float3_t m_vTriggerMoveFadeLastPosition = {};
	bool_t m_bTriggerMoveFadeHasLastPosition = false;
	bool_t m_bTriggerMoveFadeArmed = false;
	struct ENTRANCE_TRIGGER_MARKER final
	{
		std::string placementId;
		std::string sequenceInstanceId;
		EFFECT_WORLD_ROOT_HANDLE handle;
		float4x4_t rootWorld{};
		f32_t seconds = 0.f;
		bool_t started = false;
		bool_t retired = false;
	};
	std::vector<ENTRANCE_TRIGGER_MARKER> m_EntranceTriggerMarkers;

#ifdef _DEBUG
	std::vector<shared_ptr<CTrigger_Box>> m_DebugStageEntryTriggers;
#endif

	static CLevel_KakulSaydonArena* s_pActiveInstance;

public:
	static unique_ptr<CLevel_KakulSaydonArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END

```

## Client/Private/Level_KakulSaydonArena_WorldObjects.cpp 전체 반영 코드

```cpp
#include "Level_KakulSaydonArena.h"
#include "Character.h"
#include "Npc.h"
#include "CombatHUDViewModel.h"
#include "DeployPropObject.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Transform.h"
#include "WorldGameplayDocument.h"
#include <cmath>
#include <algorithm>
#include <set>

using namespace Client;

namespace
{
    constexpr std::uint32_t MARIO_BOMB_INTERVAL_MS = 4000u;
    constexpr float MARIO_BOMB_SPEED_MPS = 3.f;
    constexpr float MARIO_BOMB_LOW_BOTTOM_M = .05f;
    constexpr float MARIO_BOMB_HIGH_BOTTOM_M = .90f;
}

bool_t CLevel_KakulSaydonArena::Ready_MarioBombPresentation(std::string& status)
{
    struct BINDING { std::uint8_t stage; const char* marker; const char* arrival; const char* exit; };
    static constexpr BINDING bindings[] = {
        {2u, "Mario2_Boom", "Mario2_go", "Mario2_Trigger_2"},
        {2u, "Mario2_Boom_1", "Mario2_Trigger_2", "Mario2_Trigger_4"},
        {2u, "Mario2_Boom_2", "Mario2_Trigger_4", "Mario2_Trigger_7"},
        {3u, "Mario3_Boom", "Mario3_Trigger_4", "Mario3_Trigger_5"},
        {3u, "Mario3_Boom_1", "Mario3_Trigger_4", "Mario3_Trigger_5"},
        {3u, "Mario3_Boom_2", "Mario3_Trigger_8", "Mario3_Trigger_10"},
        {4u, "Mario4_Boom", "Mario4_Tigger_6", "Mario4_Tigger_7"}
    };
    CWorldGameplayDocument world;
    const auto path = CMapAssetCatalog::Get_MapDataRoot().parent_path() / "World" /
        "LV_LUT_MIDNIGHTC_ED.viewer.world.json";
    if (!world.Load(path, "LV_LUT_MIDNIGHTC_ED", status)) return false;
    const auto& source = m_SequencePlayer.Get_Document();
    const auto* base = source.Find_Instance("world.object.instance.mario.clown_face_ball.show");
    const auto* motion = base ? source.Find_Template(base->templateId) : nullptr;
    if (!base || !motion || base->bindings.size() != 1u ||
        base->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        motion->tracks.size() != 1u || motion->tracks.front().keys.empty() ||
        !motion->animationTracks.empty() || !motion->effectTracks.empty())
    { status = "Mario clown face ball show motion is missing or incompatible."; return false; }
    const auto* resource = source.Find_ObjectResource(base->bindings.front().targetId);
    if (!resource || resource->animated || !resource->sequenceInstanceId.empty())
    { status = "Mario clown face ball model resource is missing or incompatible."; return false; }

    CWorldSequenceDocument stagedDocument;
    stagedDocument.Reset_Empty("LV_LUT_MIDNIGHTC_ED");
    auto copiedResource = *resource;
    copiedResource.defaultMotionInstanceId.clear();
    stagedDocument.Get_ObjectResources().push_back(std::move(copiedResource));
    std::vector<MARIO_BOMB_EMITTER> stagedEmitters;
    for (const auto& binding : bindings)
    {
        const auto* marker = world.Find(binding.marker);
        const auto* arrival = world.Find(binding.arrival);
        const auto* exit = world.Find(binding.exit);
        if (!marker || !arrival || !exit || marker->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
            arrival->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX || exit->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
            arrival->triggerEvents.size() != 1u ||
            arrival->triggerEvents.front().eKind != WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER)
        { status = std::string("Mario bomb published marker/lane is missing: ") + binding.marker; return false; }
        const float3_t start = marker->position;
        const auto& laneStart = arrival->triggerEvents.front().targetPosition;
        const auto distance = [&start](const float3_t& p)
        { return std::hypot(double(p.x) - start.x, double(p.z) - start.z); };
        const auto& finish = distance(laneStart) > distance(exit->position) ? laneStart : exit->position;
        const double length = distance(finish);
        if (!std::isfinite(length) || length < .1 || length > 200.)
        { status = std::string("Mario bomb lane length is invalid: ") + binding.marker; return false; }

        MARIO_BOMB_EMITTER emitter;
        emitter.stage = binding.stage;
        // Stable marker identity, never vector order, supplies the random sequence and launch phase.
        emitter.seed = 2166136261u;
        for (const char* c = binding.marker; *c; ++c)
            emitter.seed = (emitter.seed ^ static_cast<unsigned char>(*c)) * 16777619u;
        emitter.phaseMs = emitter.seed % MARIO_BOMB_INTERVAL_MS;
        emitter.durationMs = static_cast<std::uint32_t>(std::ceil(length / MARIO_BOMB_SPEED_MPS * 1000.));
        const auto slotCount = (emitter.durationMs + MARIO_BOMB_INTERVAL_MS - 1u) / MARIO_BOMB_INTERVAL_MS;
        if (!slotCount || slotCount > 32u)
        { status = std::string("Mario bomb slot budget exceeded: ") + binding.marker; return false; }
        WORLD_SEQUENCE_TEMPLATE flight;
        flight.sequenceId = std::string("sequence.mario.bomb.") + binding.marker;
        flight.displayName = std::string(binding.marker) + " / flying clown bomb";
        flight.category = "WorldObject";
        flight.durationMs = emitter.durationMs;
        flight.interpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR;
        WORLD_SEQUENCE_TRACK track;
        track.slotId = base->bindings.front().slotId;
        auto first = motion->tracks.front().keys.front();
        first.timeMs = 0u;
        first.visible = true;
        // ClownFaceBall's nose points along cooked-model +X, not +Z.
        // Keep world-up and turn that face into the actual flight direction;
        // the Mario side camera then sees a profile on either travel side.
        const float flightYaw = static_cast<float>(std::atan2(
            -(double(finish.z) - start.z), double(finish.x) - start.x));
        const vector_t flightRotation = XMQuaternionRotationAxis(
            XMVectorSet(0.f, 1.f, 0.f, 0.f), flightYaw);
        XMStoreFloat4(&first.rotationQuaternion, flightRotation);
        // The show key recenters this asymmetric mesh at its bottom center.
        // Rotate that correction too so all headings share the same launch line.
        XMStoreFloat3(&first.positionOffset, XMVector3Rotate(
            XMLoadFloat3(&first.positionOffset), flightRotation));
        auto last = first;
        last.timeMs = emitter.durationMs;
        last.positionOffset.x += finish.x - start.x;
        last.positionOffset.z += finish.z - start.z;
        track.keys = {first, last};
        flight.tracks.push_back(std::move(track));
        stagedDocument.Get_Templates().push_back(flight);
        for (std::uint32_t slot = 0; slot < slotCount; ++slot)
        {
            WORLD_SEQUENCE_INSTANCE instance;
            instance.instanceId = std::string("world.object.instance.mario.bomb.") + binding.marker + ".slot" + std::to_string(slot);
            instance.templateId = flight.sequenceId;
            instance.position = start;
            instance.bindings = base->bindings;
            emitter.slots.push_back(instance.instanceId);
            emitter.births.push_back(-1);
            stagedDocument.Get_Instances().push_back(std::move(instance));
        }
        stagedEmitters.push_back(std::move(emitter));
    }
    auto stagedPlayer = std::make_unique<CWorldSequencePlayer>();
    const auto targets = Make_WorldSequenceTargets();
    if (!stagedPlayer->Set_Document(stagedDocument, targets, status)) return false;
    for (const auto& emitter : stagedEmitters)
        if (!stagedPlayer->Prepare_InstanceResources(emitter.slots.front(), targets))
        { status = stagedPlayer->Get_Status(); return false; }
    if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(targets, true);
    m_pMarioBombPlayer = std::move(stagedPlayer);
    m_MarioBombEmitters = std::move(stagedEmitters);
    status = "7 Mario bomb markers ready; 4000ms interval, 3m/s, face follows flight, no damage.";
    OutputDebugStringA(("[MarioBomb] " + status + "\n").c_str());
    return true;
}

void CLevel_KakulSaydonArena::Update_MarioBombPresentation(const f32_t timeDelta)
{
    if (!std::isfinite(timeDelta) || timeDelta < 0.f) return;
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const std::uint8_t stage = player.iCurrentHp && player.iMarioStage >= 2u && player.iMarioStage <= 4u ?
        player.iMarioStage : 0u;
    const auto tick = m_Replication.Get_LastServerTick();
    if (tick != m_iMarioBombSnapshotTick)
    { m_iMarioBombSnapshotTick = tick; m_fMarioBombSnapshotSeconds = 0.f; }
    else m_fMarioBombSnapshotSeconds = (std::min)(.1f, m_fMarioBombSnapshotSeconds + timeDelta);
    const double clockMs = double(tick) * (1000. / 30.) + double(m_fMarioBombSnapshotSeconds) * 1000.;
    const auto targets = Make_WorldSequenceTargets();
    if (stage != m_iMarioBombStage || clockMs < m_fMarioBombStageStartMs)
    {
        if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(targets, true);
        for (auto& emitter : m_MarioBombEmitters)
        { std::fill(emitter.births.begin(), emitter.births.end(), -1); emitter.failed = false; }
        m_iMarioBombStage = stage;
        m_fMarioBombStageStartMs = clockMs;
        m_bMarioBombLoadAttempted = false;
    }
    if (!stage) return;
    if (!m_pMarioBombPlayer)
    {
        if (m_bMarioBombLoadAttempted) return;
        m_bMarioBombLoadAttempted = true;
        std::string status;
        if (!Ready_MarioBombPresentation(status))
        { OutputDebugStringA(("[MarioBomb] " + status + "\n").c_str()); return; }
    }
    for (auto& emitter : m_MarioBombEmitters)
    {
        if (emitter.stage != stage || emitter.failed) continue;
        const auto latest = static_cast<std::int64_t>(std::floor((clockMs - emitter.phaseMs) / MARIO_BOMB_INTERVAL_MS));
        const auto count = static_cast<std::int64_t>(emitter.slots.size());
        for (std::int64_t slot = 0; slot < count; ++slot)
        {
            const auto& id = emitter.slots[static_cast<size_t>(slot)];
            const auto birth = latest < slot ? -1 : latest - (latest - slot) % count;
            const double birthMs = double(birth) * MARIO_BOMB_INTERVAL_MS + emitter.phaseMs;
            const double age = clockMs - birthMs;
            if (birth < 0 || birthMs < m_fMarioBombStageStartMs || age < 0. || age >= emitter.durationMs)
            {
                if (m_pMarioBombPlayer->Is_Playing(id)) m_pMarioBombPlayer->Stop_Instance(id, targets, true);
                emitter.births[static_cast<size_t>(slot)] = -1;
                continue;
            }
            if (emitter.births[static_cast<size_t>(slot)] != birth)
            {
                m_pMarioBombPlayer->Stop_Instance(id, targets, true);
                std::uint32_t random = emitter.seed ^ (static_cast<std::uint32_t>(birth) * 0x9e3779b9u);
                random ^= random << 13; random ^= random >> 17; random ^= random << 5;
                const float3_t heightOffset{0.f, (random & 1u) ? MARIO_BOMB_HIGH_BOTTOM_M : MARIO_BOMB_LOW_BOTTOM_M, 0.f};
                if (!m_pMarioBombPlayer->Play(id, targets, 1.f, heightOffset))
                { emitter.failed = true; break; }
                emitter.births[static_cast<size_t>(slot)] = birth;
            }
            if (!m_pMarioBombPlayer->Seek_InstanceToMs(id, static_cast<float>(age), targets))
            { emitter.failed = true; break; }
        }
        if (emitter.failed)
        {
            const auto status = m_pMarioBombPlayer->Get_Status();
            for (const auto& id : emitter.slots) m_pMarioBombPlayer->Stop_Instance(id, targets, true);
            OutputDebugStringA(("[MarioBomb] " + emitter.slots.front() + ": " + status + "\n").c_str());
        }
    }
}

void CLevel_KakulSaydonArena::Update_MarioLayoutPresentation()
{
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    std::string desired;
    if (player.iCurrentHp && player.iMarioStage >= 1u && player.iMarioStage <= 4u &&
        player.iMarioLayoutVariant >= 1u && player.iMarioLayoutVariant <= 3u)
        desired = "world.sequence.instance.mario" + std::to_string(player.iMarioStage) +
            ".source.layout" + std::to_string(player.iMarioLayoutVariant);
    const auto targets = Make_WorldSequenceTargets();
    if (desired != m_strMarioLayoutInstance)
    {
        if (!m_strMarioLayoutInstance.empty())
            m_SequencePlayer.Stop_Instance(m_strMarioLayoutInstance, targets, true);
        m_strMarioLayoutInstance = desired;
        m_bMarioLayoutFailed = false;
        m_bMarioLayoutStarted = false;
    }
    if (desired.empty() || m_bMarioLayoutFailed || m_bMarioLayoutStarted) return;
    if (!m_SequencePlayer.Play(desired, targets))
    {
        m_SequencePlayer.Stop_Instance(desired, targets, true);
        m_bMarioLayoutFailed = true;
        OutputDebugStringA(("[MarioLayout] " + desired + ": " + m_SequencePlayer.Get_Status() + "\n").c_str());
    }
    else m_bMarioLayoutStarted = true;
}

void CLevel_KakulSaydonArena::Update_MarioBallBouncePresentation(const f32_t timeDelta)
{
    Update_MarioLayoutPresentation();
    static const std::string instanceId = "world.sequence.instance.mario.striped_ball.bounce";
    const auto stage = CCombatHUDViewModel::Get().Get_Player().iMarioStage;
    if (stage < 1u || stage > 4u)
    {
        if (m_bMarioBallBounceRunning)
            m_SequencePlayer.Stop_Instance(instanceId, Make_WorldSequenceTargets(), true);
        m_bMarioBallBounceRunning = false;
        m_bMarioBallBounceFailed = false;
        m_iMarioBallBounceSnapshotTick = 0u;
        m_fMarioBallBounceSnapshotSeconds = 0.f;
        return;
    }
    if (m_bMarioBallBounceFailed || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
    const auto* instance = m_SequencePlayer.Get_Document().Find_Instance(instanceId);
    const auto* sequence = instance ? m_SequencePlayer.Get_Document().Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled)
    {
        if (m_bMarioBallBounceRunning)
            m_SequencePlayer.Stop_Instance(instanceId, Make_WorldSequenceTargets(), true);
        m_bMarioBallBounceRunning = false;
        return;
    }
    const auto tick = m_Replication.Get_LastServerTick();
    if (tick != m_iMarioBallBounceSnapshotTick)
    {
        m_iMarioBallBounceSnapshotTick = tick;
        m_fMarioBallBounceSnapshotSeconds = 0.f;
    }
    else m_fMarioBallBounceSnapshotSeconds = (std::min)(.1f, m_fMarioBallBounceSnapshotSeconds + timeDelta);
    const double periodMs = static_cast<double>(sequence->durationMs) / instance->playbackSpeed;
    if (!std::isfinite(periodMs) || periodMs <= 0.) return;
    // Sample authored placement-relative tracks; never accumulate displacement.
    // Per-track phase keys share one clock, so neighbours rise and fall out of phase.
    const double clockMs = static_cast<double>(tick) * (1000. / 30.) +
        static_cast<double>(m_fMarioBallBounceSnapshotSeconds) * 1000.;
    const f32_t elapsedMs = static_cast<f32_t>(instance->startDelayMs + std::fmod(clockMs, periodMs));
    const auto targets = Make_WorldSequenceTargets();
    if ((!m_SequencePlayer.Is_Playing(instanceId) && !m_SequencePlayer.Play(instanceId, targets)) ||
        !m_SequencePlayer.Seek_InstanceToMs(instanceId, elapsedMs, targets))
    {
        const auto failure = m_SequencePlayer.Get_Status();
        m_SequencePlayer.Stop_Instance(instanceId, targets, true);
        m_bMarioBallBounceRunning = false;
        m_bMarioBallBounceFailed = true;
        OutputDebugStringA(("[MarioBallBounce] " + failure + "\n").c_str());
        return;
    }
    m_bMarioBallBounceRunning = true;
}

void CLevel_KakulSaydonArena::Update_MarioBallPresentation(const f32_t timeDelta)
{
    constexpr f32_t CURSE_NOTICE_SECONDS = 3.f;
    constexpr float BALL_CENTRE_HEIGHT_M = .47f;
    static constexpr const char* SMOKE_LEAVES[3] = {
        "boss.kouku.ball.smoke.red_1", "boss.kouku.ball.smoke.blue_1", "boss.kouku.ball.smoke.yellow_1" };
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const auto& document = m_SequencePlayer.Get_Document();
    // Follows the layout the bounce update chose this frame; empty outside Mario.
    const bool_t layoutChanged = m_strMarioLayoutInstance != m_strMarioBallLayoutInstance;
    if (layoutChanged)
    {
        /* Hand the previous layout's slots back. Its stop already restored
           the authored visibility, so nothing is shown here. */
        if (const auto* previous = document.Find_Instance(m_strMarioBallLayoutInstance))
            for (const auto& binding : previous->bindings)
                if (uint64_t placementId = 0u; CWorldSequencePlayer::Try_ParseTargetId(binding, placementId))
                    m_SequencePlayer.Set_PlacementSuppressed(placementId, false);
        m_strMarioBallLayoutInstance = m_strMarioLayoutInstance;
        m_iMarioPoppedBallsSeen = 0u;
        m_iMarioCurseSeen = 0u;
        m_iMarioCurseNoticeQueue = 0u;
        m_iMarioCurseNoticeColor = -1;
        m_fMarioCurseNoticeSeconds = 0.f;
    }
    const auto* layout = m_strMarioBallLayoutInstance.empty() ?
        nullptr : document.Find_Instance(m_strMarioBallLayoutInstance);
    const std::uint16_t popped = layout ? player.iMarioPoppedBallMask : std::uint16_t{};
    const std::uint8_t curse = layout ? player.iMarioCurseReleasedMask : std::uint8_t{};
    /* Balls popped before this player arrived are hidden silently: the smoke
       and the notice belong to the pop itself. */
    const std::uint16_t changed = static_cast<std::uint16_t>(popped ^ m_iMarioPoppedBallsSeen);
    auto& placements = m_MapRuntime.Get_MutablePlacements();
    for (size_t slot = 0u; layout && slot < layout->bindings.size() && slot < 16u; ++slot)
    {
        const std::uint16_t bit = static_cast<std::uint16_t>(1u << slot);
        uint64_t placementId = 0u;
        if (!(changed & bit) || !CWorldSequencePlayer::Try_ParseTargetId(layout->bindings[slot], placementId))
            continue;
        const bool_t hidden = 0u != (popped & bit);
        m_SequencePlayer.Set_PlacementSuppressed(placementId, hidden);
        auto* entry = CWorldSequencePlayer::Find_Placement(placements, placementId);
        if (nullptr == entry) continue;
        (void)CMapPlacementRuntime::Set_RuntimeVisible(*entry, !hidden);
        if (!hidden || layoutChanged) continue;
        const std::string& asset = entry->record.assetId;
        const int color = asset == "MAP_MARIO_RED_STAR_BALL" ? 0 :
            asset == "MAP_MARIO_BLUE_BALL" ? 1 : asset == "MAP_MARIO_YELLOW_BALL" ? 2 : -1;
        if (color < 0 || m_bMarioBallSmokeFailed[color]) continue;
        const auto smoke = m_SequencePlayer.Find_PreparedLeafSnapshot(SMOKE_LEAVES[color]);
        if (!smoke)
        {
            m_bMarioBallSmokeFailed[color] = true;
            OutputDebugStringA(("[MarioBall] Loader snapshot unavailable: " + std::string(SMOKE_LEAVES[color]) + "\n").c_str());
            continue;
        }
        /* The authored cloud sits at its own local offset (2.3 m up, 1.35 m
           forward) because it was written for a taller anchor. Cancel that
           offset so the puff lands on the ball; reading it back keeps this
           correct if the effect is re-authored. */
        const auto* document = smoke->Find_Document(SMOKE_LEAVES[color]);
        const float3_t authored = nullptr == document ? float3_t{} :
            document->Desc.Params.Position.vStart;
        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
        XMStoreFloat4x4(&playback.PivotWorld, XMMatrixTranslation(
            entry->record.position.x - authored.x,
            entry->record.position.y + BALL_CENTRE_HEIGHT_M - authored.y,
            entry->record.position.z - authored.z));
        playback.bProductOwned = true;
        if (0u == CEffectV2Runtime::Play_Leaf(SMOKE_LEAVES[color], smoke, playback, m_pDevice, m_pContext))
            OutputDebugStringA(("[MarioBall] smoke: " + CEffectV2Runtime::Last_Error() + "\n").c_str());
    }
    m_iMarioPoppedBallsSeen = popped;
    if (!layoutChanged)
        m_iMarioCurseNoticeQueue |= static_cast<std::uint8_t>(curse & ~m_iMarioCurseSeen);
    m_iMarioCurseSeen = curse;
    if (m_iMarioCurseNoticeColor >= 0)
    {
        m_fMarioCurseNoticeSeconds -= std::isfinite(timeDelta) && timeDelta > 0.f ? timeDelta : 0.f;
        if (m_fMarioCurseNoticeSeconds <= 0.f) m_iMarioCurseNoticeColor = -1;
    }
    for (int color = 0; m_iMarioCurseNoticeColor < 0 && color < 3; ++color)
        if (m_iMarioCurseNoticeQueue & (1u << color))
        {
            m_iMarioCurseNoticeQueue &= static_cast<std::uint8_t>(~(1u << color));
            m_iMarioCurseNoticeColor = color;
            m_fMarioCurseNoticeSeconds = CURSE_NOTICE_SECONDS;
        }
}

CWorldSequencePlayer::TARGET_SET CLevel_KakulSaydonArena::Make_WorldSequenceTargets()
{
    CWorldSequencePlayer::TARGET_SET targets;
    targets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
    targets.pCatalog = &m_MapRuntime.Get_Catalog();
    targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
    targets.pDeployRuntime = &m_DeployRuntime;
    targets.device = m_pDevice;
    targets.context = m_pContext;
    targets.playerAnchors = [this]()
    {
        std::vector<CWorldSequencePlayer::PLAYER_ANCHOR> anchors;
        std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
        std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
        m_Replication.Collect_KoukuPresentationViews(bosses, players);
        for (const auto& player : players)
        {
            const auto character = player.pCharacter.lock();
            if (!player.Snapshot.iCurrentHp || !character || !character->Get_Transform()) continue;
            anchors.push_back({player.Snapshot.iNetEntityId, *character->Get_Transform()->Get_WorldMatrixPtr()});
        }
        return anchors;
    };
    targets.bossAnchor = [this](const std::string& archetype, const std::string& bone,
        CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
    {
        std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
        std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
        m_Replication.Collect_KoukuPresentationViews(bosses, players);
        const KOUKU_BOSS_PRESENTATION_VIEW* selected = nullptr;
        for (const auto& boss : bosses)
        {
            if (boss.strArchetypeId != archetype || boss.iOwnerBossNetEntityId || !boss.Snapshot.iCurrentHp) continue;
            if (selected) { status = "World Object Boss anchor is ambiguous: " + archetype; return false; }
            selected = &boss;
        }
        const auto actor = selected ? selected->pNpc.lock() : nullptr;
        if (!actor || !actor->Get_Transform())
        { status = "World Object Boss anchor is waiting for its living replicated actor: " + archetype; return false; }
        out.entityId = selected->Snapshot.iNetEntityId;
        return CWorldSequencePlayer::Resolve_BossBoneAnchor(actor->Get_Model(),
            *actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
    };
    return targets;
}

void CLevel_KakulSaydonArena::Get_WorldObjectValidationTargets(
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy) const
{
    CWorldSequencePlayer::Collect_ValidationTargets(
        const_cast<CLevel_KakulSaydonArena*>(this)->Make_WorldSequenceTargets(), placements, deploy);
}

bool_t CLevel_KakulSaydonArena::Try_GetWorldSequencePlacementBaseline(
    const WORLD_SEQUENCE_INSTANCE& instance, float3_t& outPosition) const
{
    if (instance.anchorKind != "WORLD" || instance.bindings.empty()) return false;
    std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> seen;
    double sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    size_t count = 0u;
    for (const auto& binding : instance.bindings)
    {
        if (!seen.emplace(binding.targetKind, binding.targetId).second) continue;
        float3_t position{};
        if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
            position = instance.position;
        else
        {
            uint64_t id = 0u;
            if (!CWorldSequencePlayer::Try_ParseTargetId(binding, id)) return false;
            if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT)
            {
                const auto& placements = m_MapRuntime.Get_Placements();
                const auto entry = std::find_if(placements.begin(), placements.end(),
                    [id](const auto& value) { return value.record.placementId == id; });
                if (entry == placements.end()) return false;
                position = entry->record.position;
            }
            else if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
            {
                const auto object = m_DeployRuntime.Find(id);
                float4_t rotation;
                if (!object || !object->Get_PlacedRootPose(position, rotation)) return false;
            }
            else return false;
        }
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return false;
        sumX += position.x; sumY += position.y; sumZ += position.z;
        ++count;
    }
    if (!count) return false;
    // One common translation preserves the relative placement of a curtain group.
    outPosition = {float(sumX / count), float(sumY / count), float(sumZ / count)};
    return true;
}

bool_t CLevel_KakulSaydonArena::Reload_WorldObjectRuntime(std::string& status)
{
    if (m_SequencePlayer.Has_ActiveInstances())
    {
        m_bWorldObjectReloadPending = true;
        status = "Saved World Objects are ready for the next play; the active Server sequence keeps its current revision.";
        return true;
    }
    m_bWorldObjectReloadPending = false;
    // Every owned cue and authoring preview has its own admitted document copy.
    // Replacing the idle base does not stop or rewrite any of those active poses.
    const bool_t result = m_SequencePlayer.Load_Area("LV_LUT_MIDNIGHTC_ED", Make_WorldSequenceTargets());
    status = m_SequencePlayer.Get_Status();
    return result;
}

#ifdef _DEBUG
bool_t CLevel_KakulSaydonArena::Debug_BeginWorldObjectPreview(
    const CWorldSequenceDocument& document, const std::string& instanceId, std::string& status,
    const bool_t previewAtCharacter)
{
    if (m_SequencePlayer.Has_ActiveInstances() || !m_CompositionWorldPreviewCues.empty())
    { status = "Stop the active pattern/world preview before previewing this object."; return false; }
    const auto* group = document.Find_ObjectResource(instanceId);
    const bool isGroup = group && !group->motionInstanceIds.empty();
    std::vector<std::string> ids = isGroup ? group->motionInstanceIds : std::vector<std::string>{instanceId};
    if (isGroup)
        std::erase_if(ids, [&document](const auto& id) {
            const auto* motion = document.Find_Instance(id); return motion && !motion->enabled;
        });
    if (ids.empty()) { status = "Object group has no enabled motions."; return false; }
    for (const auto& id : ids)
        if (!Can_StartCompositionWorld(id, status, &document)) return false;
    const auto targets = Make_WorldSequenceTargets();
    auto staged = std::make_unique<CWorldSequencePlayer>();
    if (!staged->Set_Document(document, targets, status))
    { status = staged->Get_Status(); return false; }
    for (const auto& id : ids)
        if (!staged->Prepare_InstanceResources(id, targets))
        { status = staged->Get_Status(); return false; }
    float3_t previewOffset{};
    const auto* instance = document.Find_Instance(ids.front());
    if (previewAtCharacter && instance && instance->anchorKind == "WORLD")
    {
        float3_t previewPosition{}, baseline{};
        if (!Try_GetWorldSequencePlacementBaseline(*instance, baseline))
        { status = "World preview cannot resolve every saved placement in this object group."; return false; }
        if (!Try_Get_AuthoringForwardPlacement(previewPosition, status)) return false;
        previewOffset = {previewPosition.x - baseline.x,
            previewPosition.y - baseline.y, previewPosition.z - baseline.z};
    }
    // The preview offset never edits the saved map anchor or placed curtain/roulette.
    // All resources and the preview placement are admitted before releasing the old presentation.
    // Model-only groups stage independent instances before replacing the old
    // preview. A shared offset preserves every member's relative placement.
    if (!isGroup) Debug_StopWorldObjectPreview();
    for (const auto& id : ids)
        if (!staged->Play(id, targets, 1.f, previewOffset))
        { status = staged->Get_Status(); staged->Stop_All(targets, true); return false; }
    if (isGroup) Debug_StopWorldObjectPreview();
    m_pWorldObjectPreview = std::move(staged);
    m_WorldObjectPreviewInstances = std::move(ids);
    return Debug_SampleWorldObjectPreview(0.f, status);
}

bool_t CLevel_KakulSaydonArena::Debug_SampleWorldObjectPreview(const f32_t clockMs, std::string& status)
{
    if (!m_pWorldObjectPreview || !std::isfinite(clockMs) || clockMs < 0.f)
    { status = "World Object preview is not active."; return false; }
    for (const auto& id : m_WorldObjectPreviewInstances)
    {
        if (!m_pWorldObjectPreview->Seek_InstanceToMs(id, clockMs, Make_WorldSequenceTargets()))
        {
            status = m_pWorldObjectPreview->Get_Status();
            Debug_StopWorldObjectPreview();
            return false;
        }
    }
    status = std::to_string(m_WorldObjectPreviewInstances.size()) + " motion(s). " +
        m_pWorldObjectPreview->Get_ObjectSampleStatus(m_WorldObjectPreviewInstances.front());
    return true;
}

void CLevel_KakulSaydonArena::Debug_StopWorldObjectPreview()
{
    if (m_pWorldObjectPreview) m_pWorldObjectPreview->Stop_All(Make_WorldSequenceTargets(), true);
    m_pWorldObjectPreview.reset();
    m_WorldObjectPreviewInstances.clear();
}
#endif

```

## Tools/MapPipeline/Publish-MapAuthoring.ps1 전체 반영 코드

```powershell
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9_.-]+$')]
    [string]$AreaId,

    [string]$ProjectRoot,

    [ValidateRange(0, 65537)]
    [int]$FailureAfterPromote = 0,

    [ValidateSet('Validate', 'Check', 'Publish')]
    [string]$Mode = 'Publish',

    [ValidateSet('Area', 'WorldSequences')]
    [string]$Scope = 'Area'
)

$ErrorActionPreference = 'Stop'
if ($Mode -ne 'Publish' -and $FailureAfterPromote -ne 0) {
    throw 'FailureAfterPromote is only supported in Publish mode.'
}
if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
$ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
$authoringPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapplacements"
$authoringDeployPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.deployplacements"
$authoringLightPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.maplights.json"
$authoringEffectPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapeffects.json"
$authoringWaterPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapwater.json"
$authoringMaterialPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapmaterials.json"
$authoringSequencePath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.worldsequences.json"
$authoringCameraShotPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.camerashots.json"
$importRoot = Join-Path $ProjectRoot "Data\Maps\Imported\$AreaId"
$sourceCatalogPath = Join-Path $importRoot "$AreaId.mapassets"
$sourceShardSetPath = Join-Path $importRoot "$AreaId.mapset"
$sourceDeployCatalogPath = Join-Path $importRoot "$AreaId.deployassets"
$runtimeRoot = Join-Path $ProjectRoot 'Client\Bin\DataFiles\Map'
$runtimePath = Join-Path $runtimeRoot "$AreaId.mapplacements"
$runtimeLightPath = Join-Path $runtimeRoot "$AreaId.maplights.json"
$runtimeEffectPath = Join-Path $runtimeRoot "$AreaId.mapeffects.json"
$runtimeWaterPath = Join-Path $runtimeRoot "$AreaId.mapwater.json"
$runtimeSequencePath = Join-Path $runtimeRoot "$AreaId.worldsequences.json"
$runtimeCameraShotPath = Join-Path $runtimeRoot "$AreaId.camerashots.json"
$mapCatalogPath = Join-Path $ProjectRoot 'Data\Maps\MapCatalog.json'
$worldDestructionPath = Join-Path $ProjectRoot "Client\Bin\DataFiles\World\$AreaId.worlddestruction.json"
$worldDestructionSourcePath = Join-Path $ProjectRoot 'Data\Encounters\Valtan\ValtanWorldEvents.json'
$effectCatalogPath = Join-Path $ProjectRoot 'Data\Effects\EffectCatalog.json'
$valtanEncounterPath = Join-Path $ProjectRoot 'Data\Encounters\Valtan\ValtanEncounter.json'
$runtimeResourceRoot = Join-Path $ProjectRoot 'Client\Bin\Resources'
$shardSetPath = Join-Path $runtimeRoot "$AreaId.mapset"
$utf8 = [Text.UTF8Encoding]::new($false)
$script:mapMaterialModels = [Collections.Generic.Dictionary[string,string]]::new([StringComparer]::Ordinal)
$importedPlacementMask = [uint64]::Parse(
    '9223372036854775808',
    [Globalization.CultureInfo]::InvariantCulture)

function Read-PlacementDocument {
    param([string]$Path, [string[]]$Lines)
    if (-not $PSBoundParameters.ContainsKey('Lines')) {
        $Lines = @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
    }
    if ($lines.Count -lt 1) {
        throw "Placement document is empty: $Path"
    }
    $header = [regex]::Match(
        $lines[0],
        '^LOSTARK_MAP_PLACEMENTS\s+2\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
    if (-not $header.Success -or $header.Groups['area'].Value -cne $AreaId) {
        throw "Placement header does not match area '$AreaId': $Path"
    }
    $rows = @($lines | Select-Object -Skip 1)
    if ([uint32]$header.Groups['count'].Value -ne $rows.Count -or
        $rows.Count -gt 65536) {
        throw "Placement count mismatch: $Path"
    }
    return $rows
}

function Parse-PlacementRow {
    param([string]$Row, [string]$Context)
	$number = '-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?'
    $match = [regex]::Match(
        $Row,
		('^(?<id>[0-9]+)\s+"(?<source>[^"\r\n]{1,256})"\s+' +
		 '"(?<level>[A-Za-z0-9_.-]{1,128})"\s+' +
		 '"(?<transform>[A-Za-z0-9_.-]{1,32})"\s+' +
		 '"(?<asset>[A-Za-z0-9_.:-]+)"\s+' +
		 "(?<px>$number)\s+(?<py>$number)\s+(?<pz>$number)\s+" +
		 "(?<qx>$number)\s+(?<qy>$number)\s+(?<qz>$number)\s+(?<qw>$number)\s+" +
		 "(?<sx>$number)\s+(?<sy>$number)\s+(?<sz>$number)\s+(?<visible>[01])$") )
    if (-not $match.Success) {
        throw "Invalid placement row in $Context"
    }
	$values = @('px','py','pz','qx','qy','qz','qw','sx','sy','sz') | ForEach-Object {
		$value = 0.0
		if (-not [double]::TryParse(
			$match.Groups[$_].Value,
			[Globalization.NumberStyles]::Float,
			[Globalization.CultureInfo]::InvariantCulture,
			[ref]$value) -or
			[double]::IsNaN($value) -or [double]::IsInfinity($value) -or
			[math]::Abs($value) -gt [single]::MaxValue) {
			throw "Non-finite placement value in $Context"
		}
		$value
	}
	$placementId = [uint64]$match.Groups['id'].Value
	$transformSource = $match.Groups['transform'].Value
	$importedSource = $transformSource -in @('actor','component')
	$editorSource = $transformSource -in @('editor','legacy','overlay')
	$validIdDomain =
		($importedSource -and 0 -ne ($placementId -band $importedPlacementMask)) -or
		($editorSource -and $placementId -le [uint64]0x7fffffffffffffff)
	$quaternionLength = [math]::Sqrt(
		$values[3] * $values[3] + $values[4] * $values[4] +
		$values[5] * $values[5] + $values[6] * $values[6])
	if (0 -eq $placementId -or -not $validIdDomain -or
		$quaternionLength -lt 0.000001 -or
		[math]::Abs($values[7]) -lt 0.000001 -or
		[math]::Abs($values[8]) -lt 0.000001 -or
		[math]::Abs($values[9]) -lt 0.000001) {
		throw "Placement transform or ID domain is invalid in $Context"
	}
    return [pscustomobject]@{
		PlacementId = $placementId
		SourcePlacementId = $match.Groups['source'].Value
        AssetId = $match.Groups['asset'].Value
        Row = $Row
    }
}

function Read-MapAssetCatalog {
    param([string]$Path, [string[]]$Lines, [switch]$Published)
    if (-not $PSBoundParameters.ContainsKey('Lines')) {
        $Lines = @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
    }
    $header = if ($lines.Count -gt 0) {
        [regex]::Match($lines[0],
            '^LOSTARK_MAP_ASSET_CATALOG\s+(?<version>[1-5])\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)(?:\s+"(?<materials>[A-Za-z0-9_.-]+)")?$')
    }
    if ($null -eq $header -or -not $header.Success -or
        $header.Groups['area'].Value -cne $AreaId -or
        [uint32]$header.Groups['count'].Value -ne ($lines.Count - 1) -or
        $lines.Count -lt 2 -or $lines.Count -gt 2049) {
        throw "Asset catalog header/count is invalid: $Path"
    }
    $version = [int]$header.Groups['version'].Value
    if (($version -eq 5 -and (-not $Published -or
            $header.Groups['materials'].Value -cne "$AreaId.mapmaterials.json")) -or
        ($version -lt 5 -and $header.Groups['materials'].Success)) {
        throw "Asset catalog material reference/version is invalid: $Path"
    }
    $assetIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $assets = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $resourcePrefix = [IO.Path]::GetFullPath($runtimeResourceRoot).TrimEnd('\') + '\'
    foreach ($row in @($lines | Select-Object -Skip 1)) {
        $match = [regex]::Match($row,
            '^"(?<id>[A-Za-z0-9_.:-]+)"\s+"(?:[^"\\\r\n]|\\.)+"\s+"(?<model>(?:[^"\\\r\n]|\\.)+)"\s+')
        if (-not $match.Success) { throw "Invalid asset catalog row: $Path" }
        # Match std::quoted's escape handling before checking the runtime path.
        $modelPath = [regex]::Replace($match.Groups['model'].Value, '\\(.)', '$1')
        if ([IO.Path]::IsPathRooted($modelPath) -or $modelPath.Contains(':') -or
            '..' -in @($modelPath -split '[\\/]') -or
            -not [IO.Path]::GetFullPath((Join-Path $runtimeResourceRoot $modelPath)).StartsWith(
                $resourcePrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Asset model path must stay Resources-relative: $modelPath"
        }
        $assetId = $match.Groups['id'].Value
        if (-not $assetIds.Add($assetId)) {
            throw "Duplicate asset ID in catalog: $Path"
        }
        if ($script:mapMaterialModels.ContainsKey($assetId) -and
            $script:mapMaterialModels[$assetId] -cne $modelPath) {
            throw "Conflicting model paths for map asset: $assetId"
        }
        $script:mapMaterialModels[$assetId] = $modelPath
        if ($Published) {
            $tokens = @(Split-DeployAuthoringTokens $row $Path)
            $expectedCount = if ($version -eq 1) { 8 } elseif ($version -eq 2) { 11 } elseif ($version -eq 3) { 25 } else { 26 }
            $renderMode = if ($version -lt 3) { 'Opaque' } else { $tokens[11] }
            if ($tokens.Count -ne $expectedCount -or
                $renderMode -cnotin @('Opaque','Alpha','Sky','Additive','Water')) {
                throw "Asset catalog sequence target row is invalid: $Path/$assetId"
            }
            $assets.Add($assetId, [pscustomobject]@{ RenderMode = $renderMode })
        }
    }
    if ($script:mapMaterialsDeclared -and -not $Published) {
        if ($header.Groups['version'].Value -ne '4') {
            throw "Map material references require an imported v4 catalog: $Path"
        }
        $lines[0] = "LOSTARK_MAP_ASSET_CATALOG 5 `"$AreaId`" $($assetIds.Count) `"$AreaId.mapmaterials.json`""
    }
    return [pscustomobject]@{ Lines = $lines; AssetIds = $assetIds; Assets = $assets }
}

function Assert-ImportedLeaf {
	param([string]$Name, [string]$Extension)
	if ($Name.Length -gt 260 -or $Name -notmatch '^[A-Za-z0-9_.-]+$' -or
		[IO.Path]::GetFileName($Name) -ne $Name -or
		[IO.Path]::GetExtension($Name) -ne $Extension) {
		throw "Shard path must be a leaf $Extension filename: $Name"
	}
	$fullPath = [IO.Path]::GetFullPath((Join-Path $importRoot $Name))
	if ([IO.Path]::GetDirectoryName($fullPath) -ne
		[IO.Path]::GetFullPath($importRoot).TrimEnd('\')) {
		throw "Shard path escapes the imported map root: $Name"
	}
	return $fullPath
}

function Test-JsonNumber {
    param([object]$Value)
    if ($Value -is [bool] -or $null -eq $Value) { return $false }
    if ($Value -isnot [byte] -and $Value -isnot [sbyte] -and
        $Value -isnot [int16] -and $Value -isnot [uint16] -and
        $Value -isnot [int32] -and $Value -isnot [uint32] -and
        $Value -isnot [int64] -and $Value -isnot [uint64] -and
        $Value -isnot [single] -and $Value -isnot [double] -and
        $Value -isnot [decimal]) {
        return $false
    }
    $number = [double]$Value
    return -not [double]::IsNaN($number) -and
        -not [double]::IsInfinity($number)
}

function Assert-ExactJsonProperties {
    param([object]$Value, [string[]]$Expected, [string]$Context)
    if ($null -eq $Value) { throw "$Context is null" }
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if ($actual.Count -ne $expectedSorted.Count -or
        (Compare-Object $actual $expectedSorted)) {
        throw "$Context has unexpected properties"
    }
}

function Split-DeployAuthoringTokens {
    param([string]$Line, [string]$Context)
    if ($null -eq $Line -or $Line.Length -eq 0) {
        throw "Deploy authoring row is empty: $Context"
    }
    $pattern = '"(?:\\.|[^"\\\r\n])*"|[^\s"]+'
    $matches = [regex]::Matches($Line, $pattern)
    $residue = [regex]::Replace($Line, $pattern, '')
    if ($matches.Count -eq 0 -or $residue -match '\S') {
        throw "Deploy authoring row has malformed quoting: $Context"
    }
    $tokens = [Collections.Generic.List[string]]::new()
    foreach ($match in $matches) {
        $raw = [string]$match.Value
        if ($raw.StartsWith('"') -and $raw.EndsWith('"')) {
            $body = $raw.Substring(1, $raw.Length - 2)
            $tokens.Add([regex]::Replace($body, '\\(.)', '$1'))
        }
        else {
            $tokens.Add($raw)
        }
    }
    return $tokens.ToArray()
}

function Convert-DeployUInt64 {
    param([string]$Value, [string]$Context, [bool]$AllowZero = $false)
    $parsed = [uint64]0
    if ($Value -cnotmatch '^[0-9]{1,20}$' -or
        -not [uint64]::TryParse(
            $Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed) -or (-not $AllowZero -and 0 -eq $parsed)) {
        throw "Deploy unsigned integer is invalid: $Context"
    }
    return $parsed
}

function Convert-DeployUInt32 {
    param([string]$Value, [string]$Context, [bool]$AllowZero = $true)
    $parsed = [uint32]0
    if ($Value -cnotmatch '^[0-9]{1,10}$' -or
        -not [uint32]::TryParse(
            $Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed) -or (-not $AllowZero -and 0 -eq $parsed)) {
        throw "Deploy unsigned integer is invalid: $Context"
    }
    return $parsed
}

function Convert-DeployFiniteNumber {
    param([string]$Value, [string]$Context)
    $parsed = 0.0
    if (-not [double]::TryParse(
        $Value,
        [Globalization.NumberStyles]::Float,
        [Globalization.CultureInfo]::InvariantCulture,
        [ref]$parsed) -or [double]::IsNaN($parsed) -or
        [double]::IsInfinity($parsed) -or
        [math]::Abs($parsed) -gt [single]::MaxValue) {
        throw "Deploy floating-point value is invalid: $Context"
    }
    return $parsed
}

function Assert-DeployWModelPath {
    param([string]$Value, [string]$Context)
    if ([string]::IsNullOrWhiteSpace($Value) -or
        [IO.Path]::IsPathRooted($Value) -or $Value.Contains(':') -or
        '..' -in @($Value -split '[\\/]') -or
        [IO.Path]::GetExtension($Value) -cne '.wmodel') {
        throw "Deploy model path must be a Resources-relative WModel: $Context"
    }
}

function Read-DeployAuthoringPair {
    param([string[]]$CatalogLines, [string[]]$PlacementLines,
        [string]$CatalogPath = $sourceDeployCatalogPath,
        [string]$PlacementPath = $authoringDeployPath)
    if (-not $PSBoundParameters.ContainsKey('CatalogLines')) {
        $CatalogLines = @([IO.File]::ReadAllLines($CatalogPath, [Text.Encoding]::UTF8))
    }
    if ($catalogLines.Count -lt 2) {
        throw "Deploy catalog is empty: $CatalogPath"
    }
    $catalogHeader = @(Split-DeployAuthoringTokens `
        $catalogLines[0] "$CatalogPath header")
    if ($catalogHeader.Count -ne 4 -or
        $catalogHeader[0] -cne 'LOSTARK_DEPLOY_PROP_CATALOG' -or
        $catalogHeader[1] -notin @('2','3') -or
        $catalogHeader[2] -cne $AreaId) {
        throw "Deploy catalog header is invalid: $CatalogPath"
    }
    $catalogVersion = [uint32]$catalogHeader[1]
    $assetCount = Convert-DeployUInt32 $catalogHeader[3] `
        "$CatalogPath asset count" $false
    if ($assetCount -gt 64 -or $assetCount -ne ($catalogLines.Count - 1)) {
        throw "Deploy catalog count is invalid: $CatalogPath"
    }

    $assets = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::Ordinal)
    $prototypeTags = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    for ($index = 0; $index -lt $assetCount; ++$index) {
        $context = "$CatalogPath row $index"
        $tokens = @(Split-DeployAuthoringTokens $catalogLines[$index + 1] $context)
        $expectedCount = if ($catalogVersion -eq 3) { 12 } else { 10 }
        if ($tokens.Count -ne $expectedCount -or
            $tokens[0] -cnotmatch '^[A-Za-z0-9_.:-]{1,160}$' -or
            $tokens[1] -notin @('STATIC','ANIM') -or
            [string]::IsNullOrWhiteSpace($tokens[2]) -or
            [string]::IsNullOrWhiteSpace($tokens[4]) -or
            [string]::IsNullOrWhiteSpace($tokens[9]) -or
            $assets.ContainsKey($tokens[0]) -or
            -not $prototypeTags.Add($tokens[4])) {
            throw "Deploy catalog identity is invalid or duplicated: $context"
        }
        Assert-DeployWModelPath $tokens[3] "$context intact model"
        $declaresFractured = $tokens[5].Length -ne 0 -or $tokens[6].Length -ne 0
        if ($declaresFractured) {
            if ($tokens[1] -ne 'STATIC' -or $tokens[5].Length -eq 0 -or
                $tokens[6].Length -eq 0 -or -not $prototypeTags.Add($tokens[6])) {
                throw "Deploy fractured model pair is invalid: $context"
            }
            Assert-DeployWModelPath $tokens[5] "$context fractured model"
        }
        $emissive = Convert-DeployFiniteNumber $tokens[7] "$context emissive"
        if ($emissive -lt 0.0 -or $tokens[8] -notin @('0','1') -or
            ($tokens[8] -eq '1' -and $tokens[1] -ne 'STATIC')) {
            throw "Deploy catalog presentation fields are invalid: $context"
        }
        $intactClip = if ($catalogVersion -eq 3) { $tokens[10] } `
            elseif ($tokens[1] -eq 'ANIM') { 'on' } else { '' }
        $fracturedClip = if ($catalogVersion -eq 3) { $tokens[11] } `
            elseif ($tokens[1] -eq 'ANIM') { 'off' } else { '' }
        if (($tokens[1] -eq 'STATIC' -and
                ($intactClip.Length -ne 0 -or $fracturedClip.Length -ne 0)) -or
            $intactClip.Length -gt 256 -or $fracturedClip.Length -gt 256 -or
            $intactClip -match '[\x00-\x1f]' -or
            $fracturedClip -match '[\x00-\x1f]') {
            throw "Deploy animation roles are invalid: $context"
        }
        $assets.Add($tokens[0], [pscustomobject]@{
            Kind = $tokens[1]
            DeferredEmissiveOverlay = $tokens[8] -eq '1'
            IntactClip = $intactClip
            FracturedClip = $fracturedClip
        })
    }

    if (-not $PSBoundParameters.ContainsKey('PlacementLines')) {
        $PlacementLines = @([IO.File]::ReadAllLines($PlacementPath, [Text.Encoding]::UTF8))
    }
    if ($placementLines.Count -lt 1) {
        throw "Deploy placement document is empty: $PlacementPath"
    }
    $placementHeader = @(Split-DeployAuthoringTokens `
        $placementLines[0] "$PlacementPath header")
    if ($placementHeader.Count -ne 4 -or
        $placementHeader[0] -cne 'LOSTARK_DEPLOY_PROP_PLACEMENTS' -or
        $placementHeader[1] -notin @('1','2') -or
        $placementHeader[2] -cne $AreaId) {
        throw "Deploy placement header is invalid: $PlacementPath"
    }
    $placementVersion = [uint32]$placementHeader[1]
    $placementCount = Convert-DeployUInt32 $placementHeader[3] `
        "$PlacementPath placement count" $true
    if ($placementCount -gt 4096 -or
        $placementCount -ne ($placementLines.Count - 1)) {
        throw "Deploy placement count is invalid: $PlacementPath"
    }

    $placements = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::Ordinal)
    for ($index = 0; $index -lt $placementCount; ++$index) {
        $context = "$PlacementPath row $index"
        $tokens = @(Split-DeployAuthoringTokens $placementLines[$index + 1] $context)
        $expectedCount = if ($placementVersion -eq 2) { 17 } else { 16 }
        if ($tokens.Count -ne $expectedCount) {
            throw "Deploy placement row width is invalid: $context"
        }
        $runtimeId = Convert-DeployUInt64 $tokens[0] "$context runtime ID"
        $deployActorId = Convert-DeployUInt32 $tokens[1] "$context actor ID" $true
        $definitionId = Convert-DeployUInt32 $tokens[2] "$context definition ID" $true
        if ([string]::IsNullOrWhiteSpace($tokens[3]) -or
            $tokens[3].Length -gt 256 -or -not $assets.ContainsKey($tokens[4]) -or
            $placements.ContainsKey($tokens[0])) {
            throw "Deploy placement identity is invalid or duplicated: $context"
        }
        $numeric = for ($field = 5; $field -le 12; ++$field) {
            Convert-DeployFiniteNumber $tokens[$field] "$context field $field"
        }
        $quaternionLength = [math]::Sqrt(
            $numeric[3] * $numeric[3] + $numeric[4] * $numeric[4] +
            $numeric[5] * $numeric[5] + $numeric[6] * $numeric[6])
        if ($quaternionLength -le 0.000001 -or $numeric[7] -le 0.000001 -or
            $tokens[13] -notin @('0','1')) {
            throw "Deploy placement transform is invalid: $context"
        }
        $stateOffAction = Convert-DeployUInt32 $tokens[14] `
            "$context state-off action" $true
        $occurrenceCount = Convert-DeployUInt32 $tokens[15] `
            "$context occurrence count" $true
        $provenance = if ($placementVersion -eq 2) { $tokens[16] } `
            else { 'SOURCE_EXACT' }
        if ($provenance -eq 'SOURCE_EXACT') {
            if (0 -eq $deployActorId -or 0 -eq $definitionId) {
                throw "SOURCE_EXACT Deploy placement lacks source IDs: $context"
            }
        }
        elseif ($provenance -eq 'PROJECT_AUTHORED') {
            if ($placementVersion -ne 2 -or 0 -ne $deployActorId -or
                0 -ne $definitionId -or 0 -ne $stateOffAction -or
                0 -ne $occurrenceCount) {
                throw "PROJECT_AUTHORED Deploy placement impersonates source data: $context"
            }
        }
        else {
            throw "Deploy placement provenance is invalid: $context"
        }
        $placements.Add($tokens[0], [pscustomobject]@{
            AssetId = $tokens[4]
            Destructible = $tokens[13] -eq '1'
            Provenance = $provenance
        })
    }
    return [pscustomobject]@{
        CatalogLines = $catalogLines
        PlacementLines = $placementLines
        Assets = $assets
        Placements = $placements
    }
}

function Read-MapLightDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map light JSON parse failed: $Path" }
    if ((Test-JsonNumber $document.formatVersion) -and
        [double]$document.formatVersion -eq 2.0) {
        # Validate original JSON before PowerShell can collapse duplicate keys.
        # The existing map publisher still owns staging and atomic promotion.
        $validator = Join-Path $PSScriptRoot '..\RenderingPipeline\light_resources_pipeline.py'
        $python = Get-Command python.exe -ErrorAction SilentlyContinue
        if ($null -eq $python) { $python = Get-Command python -ErrorAction SilentlyContinue }
        if ($null -eq $python -or -not (Test-Path -LiteralPath $validator -PathType Leaf)) {
            throw 'Map light v2 requires the strict light pipeline and Python.'
        }
        $previousErrorActionPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $validationOutput = & $python.Source $validator --mode Validate --source $Path --map-lights-area $AreaId 2>&1
            $validationExitCode = $LASTEXITCODE
        }
        finally { $ErrorActionPreference = $previousErrorActionPreference }
        if ($validationExitCode -ne 0) {
            throw "Map light v2 validation failed: $($validationOutput -join ' ')"
        }
        return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
    }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','provenance','lights') `
        'Map light root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-light-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        $document.provenance -isnot [string] -or
        $document.provenance -notin @(
            'SOURCE_EXACT',
            'SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED',
            'PROJECT_AUTHORED')) {
        throw "Map light header is invalid: $Path"
    }
    if ($document.lights -isnot [System.Array]) {
        throw "Map light lights property must be an array: $Path"
    }
    $lights = @($document.lights)
    if ($lights.Count -lt 1 -or $lights.Count -gt 64) {
        throw "Map light count is invalid: $Path"
    }
    $lightIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $sourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($light in $lights) {
        Assert-ExactJsonProperties $light `
            @('lightId','sourceLevel','sourceObjectId','position','radiusMeters',
              'falloffExponent','color','brightness') 'Map point light'
        if ($light.lightId -isnot [string] -or
            $light.lightId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            $light.sourceLevel -isnot [string] -or
            $light.sourceLevel -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            $light.sourceObjectId -isnot [string] -or
            $light.sourceObjectId -notmatch '^[A-Za-z0-9._:-]{1,256}$' -or
            -not $lightIds.Add($light.lightId) -or
            -not $sourceIds.Add($light.sourceObjectId)) {
            throw "Map point light identity is invalid or duplicated: $Path"
        }
        if ($light.position -isnot [System.Array] -or
            $light.color -isnot [System.Array]) {
            throw "Map point light vectors must be arrays: $($light.lightId)"
        }
        $position = @($light.position)
        $color = @($light.color)
        if ($position.Count -ne 3 -or $color.Count -ne 4) {
            throw "Map point light vector width is invalid: $($light.lightId)"
        }
        foreach ($component in $position) {
            if (-not (Test-JsonNumber $component) -or
                [double]$component -lt -100000.0 -or
                [double]$component -gt 100000.0) {
                throw "Map point light position is invalid: $($light.lightId)"
            }
        }
        foreach ($component in $color) {
            if (-not (Test-JsonNumber $component) -or
                [double]$component -lt 0.0 -or [double]$component -gt 1.0) {
                throw "Map point light color is invalid: $($light.lightId)"
            }
        }
        if (-not (Test-JsonNumber $light.radiusMeters) -or
            [double]$light.radiusMeters -lt 0.01 -or
            [double]$light.radiusMeters -gt 1000.0 -or
            -not (Test-JsonNumber $light.falloffExponent) -or
            [double]$light.falloffExponent -lt 0.01 -or
            [double]$light.falloffExponent -gt 64.0 -or
            -not (Test-JsonNumber $light.brightness) -or
            [double]$light.brightness -lt 0.0 -or
            [double]$light.brightness -gt 64.0) {
            throw "Map point light scalar is invalid: $($light.lightId)"
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-MapLightPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapLightsDeclared) {
        if (-not [IO.File]::Exists($authoringLightPath)) {
            throw "Declared map light authoring source is missing: $authoringLightPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.maplights.json"
            Lines = Read-MapLightDocument $authoringLightPath
        })
    }
}

function Get-MapPublishSha256 {
    param([string]$Path)
    $stream = [IO.File]::OpenRead($Path)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString($algorithm.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $algorithm.Dispose()
        $stream.Dispose()
    }
}

function Read-MapEffectDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map Effect JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','presentations') `
        'Map Effect root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-effect-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        $document.presentations -isnot [System.Array]) {
        throw "Map Effect header is invalid: $Path"
    }
    $presentations = @($document.presentations)
    if ($presentations.Count -lt 1 -or $presentations.Count -gt 64) {
        throw "Map Effect presentation count is invalid: $Path"
    }
    $independentIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $worldPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $surfacePlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($presentation in $presentations) {
        if ($presentation.independentEffectId -isnot [string] -or
            $presentation.independentEffectId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            -not $independentIds.Add($presentation.independentEffectId) -or
            $presentation.displayName -isnot [string] -or
            [string]::IsNullOrWhiteSpace($presentation.displayName) -or
            $presentation.displayName.Length -gt 160 -or
            $presentation.presentationKind -isnot [string]) {
            throw "Map Effect identity is invalid or duplicated: $Path"
        }
        if ($presentation.presentationKind -eq 'DEPLOY_SURFACE_OVERLAY') {
            Assert-ExactJsonProperties $presentation `
                @('independentEffectId','displayName','presentationKind','owners',
                  'visibleStates','materialIndex','emissiveIntensity',
                  'emissiveColor','maskPower') 'Map Effect surface row'
            if ($presentation.owners -isnot [System.Array] -or
                $presentation.visibleStates -isnot [System.Array]) {
                throw "Map Effect surface arrays are invalid: $Path"
            }
            $owners = @($presentation.owners)
            $states = @($presentation.visibleStates)
            if ($owners.Count -lt 1 -or $owners.Count -gt 256 -or
                $states.Count -ne 1 -or $states[0] -isnot [string] -or
                $states[0] -cne 'INTACT' -or
                -not (Test-JsonNumber $presentation.materialIndex) -or
                [double]$presentation.materialIndex -lt 0 -or
                [double]$presentation.materialIndex -gt 255 -or
                [math]::Floor([double]$presentation.materialIndex) -ne
                    [double]$presentation.materialIndex -or
                -not (Test-JsonNumber $presentation.emissiveIntensity) -or
                [double]$presentation.emissiveIntensity -lt 0 -or
                [double]$presentation.emissiveIntensity -gt 64 -or
                -not (Test-JsonNumber $presentation.maskPower) -or
                [double]$presentation.maskPower -lt 0.01 -or
                [double]$presentation.maskPower -gt 32) {
                throw "Map Effect surface values are invalid: $Path"
            }
            $color = @($presentation.emissiveColor)
            if ($presentation.emissiveColor -isnot [System.Array] -or
                $color.Count -ne 4 -or
                @($color | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt 0 -or [double]$_ -gt 16
                }).Count -gt 0) {
                throw "Map Effect surface color is invalid: $Path"
            }
            $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($owner in $owners) {
                Assert-ExactJsonProperties $owner @('groupId','placementId') 'Map Effect surface owner'
                if ($owner.groupId -isnot [string] -or
                    $owner.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                    -not $groupIds.Add($owner.groupId) -or
                    $owner.placementId -isnot [string] -or
                    $owner.placementId -notmatch '^[1-9][0-9]{0,19}$' -or
                    -not $surfacePlacementIds.Add($owner.placementId)) {
                    throw "Map Effect surface owner is invalid or duplicated: $Path"
                }
                $parsedPlacementId = [uint64]0
                if (-not [uint64]::TryParse(
                    $owner.placementId,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedPlacementId) -or 0 -eq $parsedPlacementId) {
                    throw "Map Effect surface placement ID overflows uint64: $($owner.placementId)"
                }
            }
            continue
        }
        if ($presentation.presentationKind -eq 'EFFECT_DOCUMENT') {
            Assert-ExactJsonProperties $presentation `
                @('independentEffectId','displayName','presentationKind','placementId',
                  'effectAssetId','position','rotationQuaternion','scale',
                  'orientationPolicy','activationPolicy','activationSetId',
                  'activationWindows','playbackPolicy') 'Map Effect world row'
            if ($presentation.placementId -isnot [string] -or
                $presentation.placementId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                -not $worldPlacementIds.Add($presentation.placementId) -or
                $presentation.effectAssetId -isnot [string] -or
                $presentation.effectAssetId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                $presentation.orientationPolicy -notin @('WORLD','CAMERA_FACING_WORLD') -or
                $presentation.activationPolicy -notin @('LEVEL_ACTIVE','SERVER_PATTERN_WINDOW') -or
                $presentation.playbackPolicy -notin @('LOCAL_LOOP','SERVER_CLOCK_SAMPLE') -or
                (($presentation.activationPolicy -eq 'SERVER_PATTERN_WINDOW') -ne
                 ($presentation.playbackPolicy -eq 'SERVER_CLOCK_SAMPLE'))) {
                throw "Map Effect world identity/policy is invalid: $Path"
            }
            $position = @($presentation.position)
            $rotation = @($presentation.rotationQuaternion)
            $scale = @($presentation.scale)
            if ($presentation.position -isnot [System.Array] -or
                $presentation.rotationQuaternion -isnot [System.Array] -or
                $presentation.scale -isnot [System.Array] -or
                $position.Count -ne 3 -or $rotation.Count -ne 4 -or $scale.Count -ne 3 -or
                @($position | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt -100000 -or [double]$_ -gt 100000
                }).Count -gt 0 -or
                @($rotation | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt -1 -or [double]$_ -gt 1
                }).Count -gt 0 -or
                @($scale | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt 0.001 -or [double]$_ -gt 1000
                }).Count -gt 0) {
                throw "Map Effect world transform is invalid: $Path"
            }
            $quaternionLengthSquared = 0.0
            foreach ($component in $rotation) {
                $quaternionLengthSquared += [double]$component * [double]$component
            }
            if ($quaternionLengthSquared -lt 0.999 -or $quaternionLengthSquared -gt 1.001) {
                throw "Map Effect world quaternion is not normalized: $Path"
            }
            if ($presentation.activationWindows -isnot [System.Array]) {
                throw "Map Effect activation windows must be an array: $Path"
            }
            $windows = @($presentation.activationWindows)
            if ($presentation.activationPolicy -eq 'LEVEL_ACTIVE') {
                if (-not [string]::IsNullOrEmpty($presentation.activationSetId) -or
                    $windows.Count -ne 0) {
                    throw "LEVEL_ACTIVE Map Effect cannot declare a Server set: $Path"
                }
            }
            elseif ($presentation.activationSetId -isnot [string] -or
                $presentation.activationSetId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                $windows.Count -lt 1 -or $windows.Count -gt 32) {
                throw "Map Effect Server activation set is invalid: $Path"
            }
            $windowKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            $previousOffset = -1
            foreach ($window in $windows) {
                Assert-ExactJsonProperties $window `
                    @('patternId','stageId','effectTimelineOffsetMs') `
                    'Map Effect activation window'
                $windowKey = "$($window.patternId)`n$($window.stageId)"
                if ($window.patternId -isnot [string] -or
                    $window.patternId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
                    $window.stageId -isnot [string] -or
                    $window.stageId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
                    -not $windowKeys.Add($windowKey) -or
                    -not (Test-JsonNumber $window.effectTimelineOffsetMs) -or
                    [double]$window.effectTimelineOffsetMs -lt 0 -or
                    [double]$window.effectTimelineOffsetMs -gt 3600000 -or
                    [math]::Floor([double]$window.effectTimelineOffsetMs) -ne
                        [double]$window.effectTimelineOffsetMs -or
                    [double]$window.effectTimelineOffsetMs -le $previousOffset) {
                    throw "Map Effect activation window is invalid or unordered: $Path"
                }
                $previousOffset = [double]$window.effectTimelineOffsetMs
            }
            continue
        }
        throw "Unsupported Map Effect presentation kind: $($presentation.presentationKind)"
    }
    Assert-MapEffectDomainJoins $document $Path
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Read-MapEffectDeployCatalog {
    if (-not [IO.File]::Exists($sourceDeployCatalogPath)) {
        throw "Map Effect surface rows require the Deploy catalog: $sourceDeployCatalogPath"
    }
    $pair = Read-DeployAuthoringPair
    return ,$pair.Assets
}

function Read-MapEffectDeployPlacements {
    if (-not [IO.File]::Exists($authoringDeployPath)) {
        throw "Map Effect surface rows require Deploy placements: $authoringDeployPath"
    }
    $pair = Read-DeployAuthoringPair
    return ,$pair.Placements
}

function Read-MapEffectWorldDestruction {
    if (-not [IO.File]::Exists($worldDestructionPath)) {
        throw "Map Effect surface rows require the published destruction projection: $worldDestructionPath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $worldDestructionPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "World destruction projection JSON parse failed: $worldDestructionPath" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','combatRuntimeRevision','groups') `
        'World destruction projection root'
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.world-destruction-client-projection' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 3.0 -or
        $document.areaId -isnot [string] -or $document.areaId -cne $AreaId -or
        $document.combatRuntimeRevision -isnot [string] -or
        $document.combatRuntimeRevision -notmatch '^[0-9a-f]{64}$' -or
        $document.groups -isnot [System.Array]) {
        throw "World destruction projection header is invalid: $worldDestructionPath"
    }
    $groups = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.groups)) {
        Assert-ExactJsonProperties $group `
            @('groupId','mutationId','removesGround','suppressionAliasPlacementIds',
              'memberPlacementIds') 'World destruction projection group'
        if ($group.groupId -isnot [string] -or
            $group.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.mutationId -isnot [string] -or
            $group.mutationId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.removesGround -isnot [bool] -or
            $group.memberPlacementIds -isnot [System.Array] -or
            $group.suppressionAliasPlacementIds -isnot [System.Array] -or
            $groups.ContainsKey($group.groupId)) {
            throw "World destruction projection group is invalid or duplicated: $worldDestructionPath"
        }
        $members = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($member in @($group.memberPlacementIds)) {
            $parsedId = [uint64]0
            if ($member -isnot [string] -or $member -notmatch '^[1-9][0-9]{0,19}$' -or
                -not [uint64]::TryParse(
                    $member,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedId) -or 0 -eq $parsedId -or
                -not $members.Add($member) -or -not $ownedMembers.Add($member)) {
                throw "World destruction member is invalid or multiply owned: $($group.groupId)/$member"
            }
        }
        if (0 -eq $members.Count) {
            throw "World destruction projection group has no members: $($group.groupId)"
        }
        $aliases = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($alias in @($group.suppressionAliasPlacementIds)) {
            if ($alias -isnot [string] -or -not $members.Contains($alias) -or
                -not $aliases.Add($alias)) {
                throw "World destruction suppression alias is invalid: $($group.groupId)/$alias"
            }
        }
        $groups.Add($group.groupId, [pscustomobject]@{
            MutationId = [string]$group.mutationId
            RemovesGround = [bool]$group.removesGround
            Members = $members
        })
    }
    return ,$groups
}

function Read-MapEffectWorldDestructionSource {
    if (-not [IO.File]::Exists($worldDestructionSourcePath)) {
        throw "Map Effect surface rows require the destruction source document: $worldDestructionSourcePath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $worldDestructionSourcePath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "World destruction source JSON parse failed: $worldDestructionSourcePath" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','encounterId','provenance','groups',
          'mutations','bindings') 'World destruction source root'
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.world-destruction-events' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -cne $AreaId -or
        $document.encounterId -isnot [string] -or
        $document.encounterId -cne 'ENCOUNTER_VALTAN' -or
        $document.groups -isnot [System.Array] -or
        $document.mutations -isnot [System.Array] -or
        $document.bindings -isnot [System.Array]) {
        throw "World destruction source header is invalid: $worldDestructionSourcePath"
    }
    $mutationsByGroup = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($mutation in @($document.mutations)) {
        Assert-ExactJsonProperties $mutation `
            @('mutationId','groupId','targetState','breakingDurationMs') `
            'World destruction source mutation'
        if ($mutation.mutationId -isnot [string] -or
            $mutation.mutationId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $mutation.groupId -isnot [string] -or
            $mutation.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $mutation.targetState -isnot [string] -or
            $mutation.targetState -notin @('FRACTURED','DESPAWNED') -or
            -not (Test-JsonNumber $mutation.breakingDurationMs) -or
            [double]$mutation.breakingDurationMs -lt 0.0 -or
            [math]::Floor([double]$mutation.breakingDurationMs) -ne
                [double]$mutation.breakingDurationMs -or
            $mutationsByGroup.ContainsKey($mutation.groupId)) {
            throw "World destruction source mutation is invalid or duplicated for its group."
        }
        $mutationsByGroup.Add($mutation.groupId, $mutation)
    }
    $groups = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.groups)) {
        Assert-ExactJsonProperties $group `
            @('groupId','memberPlacementIds','navigationRegionIds','navPolarity',
              'initialState') 'World destruction source group'
        if ($group.groupId -isnot [string] -or
            $group.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.memberPlacementIds -isnot [System.Array] -or
            $group.navigationRegionIds -isnot [System.Array] -or
            $group.navPolarity -isnot [string] -or
            $group.navPolarity -notin @('BLOCK_WHILE_INTACT','BLOCK_WHILE_FRACTURED') -or
            $group.initialState -isnot [string] -or $group.initialState -cne 'INTACT' -or
            $groups.ContainsKey($group.groupId) -or
            -not $mutationsByGroup.ContainsKey($group.groupId)) {
            throw "World destruction source group is invalid or has no exact mutation: $($group.groupId)"
        }
        $members = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($member in @($group.memberPlacementIds)) {
            $parsedId = [uint64]0
            if ($member -isnot [string] -or $member -notmatch '^[1-9][0-9]{0,19}$' -or
                -not [uint64]::TryParse(
                    $member,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedId) -or 0 -eq $parsedId -or
                -not $members.Add($member) -or -not $ownedMembers.Add($member)) {
                throw "World destruction source member is invalid or multiply owned: $($group.groupId)/$member"
            }
        }
        if (0 -eq $members.Count) {
            throw "World destruction source group has no members: $($group.groupId)"
        }
        $mutation = $mutationsByGroup[$group.groupId]
        $groups.Add($group.groupId, [pscustomobject]@{
            MutationId = [string]$mutation.mutationId
            TargetState = [string]$mutation.targetState
            RemovesGround = $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED'
            Members = $members
        })
    }
    return ,$groups
}

function Add-MapEffectRuntimeResourceIds {
    param(
        [object]$Value,
        [string]$Context,
        [Collections.Generic.HashSet[string]]$ResourceIds)
    if ($null -eq $Value) { return }
    if ($Value -is [System.Array]) {
        for ($index = 0; $index -lt $Value.Count; ++$index) {
            Add-MapEffectRuntimeResourceIds $Value[$index] "$Context[$index]" $ResourceIds
        }
        return
    }
    if ($Value -isnot [pscustomobject]) { return }
    foreach ($property in $Value.PSObject.Properties) {
        $field = "$Context.$($property.Name)"
        if ($property.Name -in @('assetId','modelAssetId','textureAssetId') -and
            -not [string]::IsNullOrEmpty([string]$property.Value)) {
            if ($property.Value -isnot [string]) {
                throw "Map Effect runtime resource ID must be a string: $field"
            }
            $resourceId = [string]$property.Value
            if ($resourceId.Contains('\') -or $resourceId.Contains(':') -or
                $resourceId.StartsWith('/') -or
                $resourceId -match '(^|/)\.\.?(/|$)' -or
                $resourceId -notmatch '^[A-Za-z0-9_.\/-]+\.(dds|wmodel)$') {
                throw "Map Effect runtime resource ID is unsafe or unsupported: $field=$resourceId"
            }
            [void]$ResourceIds.Add($resourceId)
        }
        if ($property.Value -is [pscustomobject] -or
            $property.Value -is [System.Array]) {
            Add-MapEffectRuntimeResourceIds $property.Value $field $ResourceIds
        }
    }
}

function Assert-MapEffectRuntimeResourceFiles {
    param([Collections.Generic.HashSet[string]]$ResourceIds, [string]$EffectAssetId)
    if (0 -eq $ResourceIds.Count) {
        throw "Map Effect direct-authored document has no runtime resource binding: $EffectAssetId"
    }
    $root = [IO.Path]::GetFullPath($runtimeResourceRoot)
    $rootPrefix = $root.TrimEnd('\') + '\'
    foreach ($resourceId in $ResourceIds) {
        $relative = $resourceId.Replace('/', [IO.Path]::DirectorySeparatorChar)
        $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
        if (-not $path.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase) -or
            -not [IO.File]::Exists($path)) {
            throw "Map Effect runtime resource file is missing or outside Resources: $resourceId"
        }
        $stream = [IO.File]::OpenRead($path)
        try {
            $magic = [byte[]]::new(4)
            if ($stream.Read($magic, 0, 4) -ne 4) {
                throw "Map Effect runtime resource file is truncated: $resourceId"
            }
            $expected = if ($resourceId.EndsWith('.dds', [StringComparison]::OrdinalIgnoreCase)) {
                [Text.Encoding]::ASCII.GetBytes('DDS ')
            }
            else { [Text.Encoding]::ASCII.GetBytes('WINT') }
            $matchesMagic = $true
            for ($index = 0; $index -lt 4; ++$index) {
                if ($magic[$index] -ne $expected[$index]) {
                    $matchesMagic = $false
                    break
                }
            }
            if (-not $matchesMagic) {
                throw "Map Effect runtime resource file magic is invalid: $resourceId"
            }
        }
        finally { $stream.Dispose() }
    }
}

function Resolve-MapEffectAuthoredDocument {
    param([string]$EffectAssetId)
    if (-not [IO.File]::Exists($effectCatalogPath)) {
        throw "Effect catalog is missing for Map Effect validation: $effectCatalogPath"
    }
    try {
        $catalog = [IO.File]::ReadAllText(
            $effectCatalogPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Effect catalog JSON parse failed: $effectCatalogPath" }
    if (-not (Test-JsonNumber $catalog.formatVersion) -or
        [double]$catalog.formatVersion -ne 1.0 -or
        $catalog.effects -isnot [System.Array]) {
        throw "Effect catalog header is invalid: $effectCatalogPath"
    }
    $matches = @($catalog.effects | Where-Object {
        $_.effectAssetId -is [string] -and $_.effectAssetId -ceq $EffectAssetId
    })
    if ($matches.Count -ne 1) {
        throw "Map Effect target must resolve exactly once in EffectCatalog: $EffectAssetId"
    }
    $entry = $matches[0]
    Assert-ExactJsonProperties $entry `
        @('effectAssetId','payloadKind','authoringPath') `
        'Map Effect direct-authored catalog row'
    $expectedAuthoringPath = "Effects/Authored/$EffectAssetId.effect.json"
    if ($entry.payloadKind -isnot [string] -or
        $entry.payloadKind -cne 'DIRECT_AUTHORED_DOCUMENT' -or
        $entry.authoringPath -isnot [string] -or
        $entry.authoringPath -cne $expectedAuthoringPath -or
        $entry.authoringPath.Contains('\') -or
        $entry.authoringPath -match '(^|/)\.\.(/|$)' -or
        $entry.authoringPath -match '^[A-Za-z]:') {
        throw "Map Effect target must be a canonical direct-authored catalog row: $EffectAssetId"
    }
    $dataRoot = [IO.Path]::GetFullPath((Join-Path $ProjectRoot 'Data'))
    $relativePath = $entry.authoringPath.Replace('/', [IO.Path]::DirectorySeparatorChar)
    $authoredPath = [IO.Path]::GetFullPath((Join-Path $dataRoot $relativePath))
    $rootPrefix = $dataRoot.TrimEnd('\') + '\'
    if (-not $authoredPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase) -or
        -not [IO.File]::Exists($authoredPath)) {
        throw "Map Effect authored document is missing or outside Data: $authoredPath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $authoredPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Map Effect authored document JSON parse failed: $authoredPath" }
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.effect-authoring' -or
        -not (Test-JsonNumber $document.version) -or
        [double]$document.version -ne 13.0 -or
        $document.effectAssetId -isnot [string] -or
        $document.effectAssetId -cne $EffectAssetId -or
        $document.elements -isnot [System.Array]) {
        throw "Map Effect authored identity is invalid: $authoredPath"
    }
    $maximumDurationSeconds = 0.0
    $drawableCount = 0
    $elementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($element in @($document.elements)) {
        if ($element -isnot [pscustomobject] -or
            $element.id -isnot [string] -or
            $element.id -notmatch '^[a-z0-9][a-z0-9._-]{0,127}$' -or
            -not $elementIds.Add($element.id) -or
            $element.visible -isnot [bool] -or
            $element.kind -isnot [string] -or
            $element.kind -notin @('particle','trail','mesh','decal','light','screenPost') -or
            $element.material -isnot [pscustomobject] -or
            $element.material.templateId -isnot [string] -or
            [string]::IsNullOrWhiteSpace($element.material.templateId) -or
            $element.material.renderProfile -isnot [string] -or
            [string]::IsNullOrWhiteSpace($element.material.renderProfile) -or
            $element.detail -isnot [pscustomobject]) {
            throw "Map Effect authored element identity/material/detail is invalid: $EffectAssetId"
        }
        if (-not $element.visible) { continue }
        ++$drawableCount
        if ($null -eq $element.detail -or $null -eq $element.detail.timing -or
            -not (Test-JsonNumber $element.detail.timing.startDelaySeconds) -or
            -not (Test-JsonNumber $element.detail.timing.lifeTimeSeconds) -or
            -not (Test-JsonNumber $element.detail.timing.afterImageSeconds)) {
            throw "Map Effect authored visible element has no finite timing: $EffectAssetId"
        }
        $endSeconds = [double]$element.detail.timing.startDelaySeconds +
            [double]$element.detail.timing.lifeTimeSeconds +
            [double]$element.detail.timing.afterImageSeconds
        if ($endSeconds -le 0.0) {
            throw "Map Effect authored visible element has no positive lifetime: $EffectAssetId"
        }
        $maximumDurationSeconds = [Math]::Max($maximumDurationSeconds, $endSeconds)
    }
    if (0 -eq $drawableCount) {
        throw "Map Effect direct-authored document has no visible drawable element: $EffectAssetId"
    }
    $resourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    Add-MapEffectRuntimeResourceIds $document $EffectAssetId $resourceIds
    Assert-MapEffectRuntimeResourceFiles $resourceIds $EffectAssetId
    return [pscustomobject]@{
        Path = $authoredPath
        DurationSeconds = $maximumDurationSeconds
    }
}

function Assert-MapEffectActivationWindows {
    param([object]$Presentation, [double]$EffectDurationSeconds)
    if (-not [IO.File]::Exists($valtanEncounterPath)) {
        throw "Valtan encounter is missing for Map Effect activation validation: $valtanEncounterPath"
    }
    try {
        $encounter = [IO.File]::ReadAllText(
            $valtanEncounterPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Valtan encounter JSON parse failed: $valtanEncounterPath" }
    if ($encounter.schema -isnot [string] -or
        $encounter.schema -cne 'lostark.encounter-profile' -or
        -not (Test-JsonNumber $encounter.formatVersion) -or
        [double]$encounter.formatVersion -ne 4.0 -or
        $encounter.encounterId -isnot [string] -or
        $encounter.encounterId -cne 'ENCOUNTER_VALTAN' -or
        $encounter.patterns -isnot [System.Array]) {
        throw "Valtan encounter header is invalid for Map Effect activation validation."
    }
    $windows = @($Presentation.activationWindows)
    $patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($window in $windows) { [void]$patternIds.Add([string]$window.patternId) }
    if ($patternIds.Count -ne 1) {
        throw "A Server-clock Map Effect activation set must target exactly one encounter pattern."
    }
    $patternId = [string]$windows[0].patternId
    $matches = @($encounter.patterns | Where-Object {
        $_.patternId -is [string] -and $_.patternId -ceq $patternId
    })
    if ($matches.Count -ne 1 -or $matches[0].stages -isnot [System.Array]) {
        throw "Map Effect activation pattern does not resolve exactly once: $patternId"
    }
    $stages = @($matches[0].stages)
    if ($stages.Count -eq 0 -or $windows.Count -ne $stages.Count) {
        throw "Map Effect activation windows must exactly cover every pattern stage: $patternId"
    }
    $expectedOffsetMs = [uint64]0
    for ($index = 0; $index -lt $stages.Count; ++$index) {
        $stage = $stages[$index]
        $window = $windows[$index]
        if ($stage.stageId -isnot [string] -or
            $stage.stageId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            -not (Test-JsonNumber $stage.durationMs) -or
            [double]$stage.durationMs -lt 1.0 -or
            [math]::Floor([double]$stage.durationMs) -ne [double]$stage.durationMs -or
            $window.patternId -cne $patternId -or
            $window.stageId -cne $stage.stageId -or
            [uint64][double]$window.effectTimelineOffsetMs -ne $expectedOffsetMs) {
            throw "Map Effect activation window does not exactly join encounter stage offset/duration: $patternId/$($stage.stageId)"
        }
        $expectedOffsetMs += [uint64][double]$stage.durationMs
        if ($expectedOffsetMs -gt 3600000) {
            throw "Map Effect activation pattern duration exceeds the supported timeline: $patternId"
        }
    }
    if (($EffectDurationSeconds * 1000.0) + 0.001 -lt [double]$expectedOffsetMs) {
        throw "Map Effect authored duration does not cover the complete activation pattern: $patternId"
    }
}

function Assert-MapEffectDomainJoins {
    param([object]$Document, [string]$DocumentPath)
    $surfaceRows = @($Document.presentations | Where-Object {
        $_.presentationKind -ceq 'DEPLOY_SURFACE_OVERLAY'
    })
    if ($surfaceRows.Count -gt 0) {
        $assets = Read-MapEffectDeployCatalog
        $placements = Read-MapEffectDeployPlacements
        $groups = Read-MapEffectWorldDestruction
        $sourceGroups = Read-MapEffectWorldDestructionSource
        foreach ($surface in $surfaceRows) {
            if (@($surface.visibleStates).Count -ne 1 -or
                [string]$surface.visibleStates[0] -cne 'INTACT' -or
                [uint32][double]$surface.materialIndex -ne [uint32]1) {
                throw "Map Effect surface supports only INTact material-1 deferred overlay presentation: $($surface.independentEffectId)"
            }
            foreach ($owner in @($surface.owners)) {
                $placementId = [string]$owner.placementId
                $groupId = [string]$owner.groupId
                if (-not $placements.ContainsKey($placementId)) {
                    throw "Map Effect surface owner does not resolve to a Deploy placement: $placementId"
                }
                $placement = $placements[$placementId]
                if (-not $assets.ContainsKey([string]$placement.AssetId)) {
                    throw "Map Effect surface owner references an unknown Deploy asset: $placementId/$($placement.AssetId)"
                }
                $asset = $assets[[string]$placement.AssetId]
                if ($asset.Kind -cne 'STATIC' -or
                    -not [bool]$asset.DeferredEmissiveOverlay -or
                    -not [bool]$placement.Destructible) {
                    throw "Map Effect surface owner must be a static destructible deferred-emissive Deploy placement: $placementId"
                }
                if (-not $groups.ContainsKey($groupId)) {
                    throw "Map Effect surface owner references an unknown destruction group: $groupId"
                }
                $group = $groups[$groupId]
                if (-not [bool]$group.RemovesGround -or
                    -not $group.Members.Contains($placementId)) {
                    throw "Map Effect surface owner must be a member of a removesGround destruction group: $groupId/$placementId"
                }
                if (-not $sourceGroups.ContainsKey($groupId)) {
                    throw "Map Effect destruction projection has no current source group: $groupId"
                }
                $sourceGroup = $sourceGroups[$groupId]
                if (-not [bool]$sourceGroup.RemovesGround -or
                    $sourceGroup.TargetState -cne 'DESPAWNED' -or
                    $sourceGroup.MutationId -cne $group.MutationId -or
                    -not $sourceGroup.Members.SetEquals($group.Members)) {
                    throw "Map Effect destruction projection is stale against its current source group/mutation: $groupId"
                }
            }
        }
    }

    foreach ($world in @($Document.presentations | Where-Object {
        $_.presentationKind -ceq 'EFFECT_DOCUMENT'
    })) {
        $authored = Resolve-MapEffectAuthoredDocument ([string]$world.effectAssetId)
        if ($world.activationPolicy -ceq 'SERVER_PATTERN_WINDOW') {
            Assert-MapEffectActivationWindows $world ([double]$authored.DurationSeconds)
        }
    }
}

function Add-MapEffectPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapEffectsDeclared) {
        if (-not [IO.File]::Exists($authoringEffectPath)) {
            throw "Declared Map Effect authoring source is missing: $authoringEffectPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.mapeffects.json"
            Lines = Read-MapEffectDocument $authoringEffectPath
        })
    }
}

function Read-MapWaterDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map water JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','revision','unclassifiedAssetCount',
          'waters','deferred') `
        'Map water root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-water-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        -not (Test-JsonNumber $document.unclassifiedAssetCount)) {
        throw "Map water header is invalid: $Path"
    }
    if ($document.waters -isnot [System.Array] -or
        $document.deferred -isnot [System.Array]) {
        throw "Map water waters/deferred properties must be arrays: $Path"
    }
    $waters = @($document.waters)
    if ($waters.Count -lt 1 -or $waters.Count -gt 64) {
        throw "Map water material count is invalid: $Path"
    }
    $scalarNames = @(
        'opacity','opacityPower','fresnelIntensity','fresnelPower',
        'screenDistortionIntensity','normalIntensity','detailNormalIntensity',
        'normalDistortionIntensity','reflectionIntensity','reflectionUv',
        'depthBias','diffuseTiling')
    $vectorNames = @(
        'diffuseColor','reflectionColor','normalTilingPanning',
        'detailNormalTilingPanning','reflectionTilingPanning')
    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($water in $waters) {
        $required = @(
            'assetId','materialName','provenance','sourceParentChain',
            'sourceBlendMode','twoSided','detailNormalTexture','reflectionTexture',
            'foamTexture') + $scalarNames + $vectorNames
        $present = @($water.PSObject.Properties.Name)
        $extra = @($present | Where-Object {
            $_ -ne 'missingSourceTextures' -and $required -notcontains $_ })
        if ($extra.Count -gt 0) {
            throw "Map water row has unknown properties: $($extra -join ', ')"
        }
        foreach ($name in $required) {
            if ($present -notcontains $name) {
                throw "Map water row is missing $name in $Path"
            }
        }
        if ($water.assetId -isnot [string] -or
            [string]::IsNullOrWhiteSpace($water.assetId) -or
            $water.materialName -isnot [string] -or
            [string]::IsNullOrWhiteSpace($water.materialName)) {
            throw "Map water row identity is invalid: $Path"
        }
        # The runtime picks one shader pass per asset, so two rows for the same
        # asset would silently make one of them unreachable.
        if (-not $keys.Add($water.assetId)) {
            throw "Duplicate map water asset row: $($water.assetId)"
        }
        if ($water.sourceBlendMode -ne 'BLEND_Translucent') {
            throw "Map water row is not translucent in the source: $($water.assetId)"
        }
        if ($water.provenance -isnot [string] -or
            $water.provenance -notin @('SOURCE_MATERIAL_EXACT','PROJECT_AUTHORED')) {
            throw "Map water provenance is invalid: $($water.assetId)"
        }
        foreach ($name in @('detailNormalTexture','reflectionTexture','foamTexture')) {
            $value = $water.$name
            if ($value -isnot [string]) {
                throw "Map water $name must be a string: $($water.assetId)"
            }
            if ([string]::IsNullOrEmpty($value)) { continue }
            if ($value -notlike 'Map/*' -or $value -match '\.\.' -or
                $value -match '^[A-Za-z]:' -or $value.Contains('\')) {
                throw "Map water $name is not a Resources-relative id: $value"
            }
        }
        foreach ($name in $scalarNames) {
            if (-not (Test-JsonNumber $water.$name)) {
                throw "Map water $name is not finite: $($water.assetId)"
            }
        }
        foreach ($name in $vectorNames) {
            $vector = @($water.$name)
            if ($vector.Count -ne 4) {
                throw "Map water $name must have four components: $($water.assetId)"
            }
            foreach ($component in $vector) {
                if (-not (Test-JsonNumber $component)) {
                    throw "Map water $name component is not finite: $($water.assetId)"
                }
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Read-WorldSequenceDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.UTF8Encoding]::new($false, $true))
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "World sequence JSON parse failed: $Path" }
    $rootProperties = @('schema','formatVersion','areaId','revision','templates','instances')
    if ($document.formatVersion -eq 3) { $rootProperties += 'objectResources' }
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
    if ($templates.Count -gt 256 -or $instances.Count -gt 2048) {
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
    if ($document.formatVersion -eq 3) {
        if ($document.objectResources -isnot [System.Array] -or @($document.objectResources).Count -gt 2048) {
            throw 'World object resource list is invalid'
        }
        foreach ($resource in $document.objectResources) {
            $fields = @('objectId','displayName','modelAssetId','modelPreScale','animated','scale')
            foreach ($optional in @('diffuseTextureAssetId','sequenceInstanceId','anchorKind','anchorBossArchetypeId','anchorBone','defaultMotionInstanceId','materialProfile','materialSourceModelAssetId','mapMaterialBindings','motionInstanceIds')) {
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
            if ($null -ne $resource.PSObject.Properties['materialSourceModelAssetId']) {
                if ($alias -or $resource.materialSourceModelAssetId -isnot [string] -or $resource.materialSourceModelAssetId -eq '') { throw 'Invalid world object material source model' }
                Assert-SequenceAssetPath $resource.materialSourceModelAssetId $true
                $sourceNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.materialSourceModelAssetId)).Names
                $targetNames = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $resource.modelAssetId)).Names
                foreach ($name in $sourceNames.Keys) {
                    if (-not $targetNames.ContainsKey($name)) { throw "Cinematic model lost source material slot: $name" }
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
                    $material.family -cne 'source.character.monster-pbr-masked.v1') {
                    throw 'World object material identity or native family is invalid'
                }
                # Exact named inputs of the selected native Base/Light pair, program 21.
                $parameterNames = @('1.use_dyeing_sp','1.use_emissive_flickerspeed_fixed','beckmannspecular_constant_max','buffcolor','constantoutline','constantoutline_blink','constantoutline_color','diffusecolor','emissive_color','emissive_flicker_speed','emissive_intensity','emissive_intensitymin','fresnel_radius','fresnel_rimlightintensity','fx_color_desaturation_actiontool','fx_color_desaturation_buffsettool','fx_color_intensity_actiontool','fx_color_intensity_buffsettool','hit_color','ibl_color_bottom','ibl_color_top','ibl_exposer','ibl_intensity','ibl_normal_smooth','ibl_reflect_lodbias','metalicness_power','normaltex_intensity','orennayar','orennayar_brightness','pbr_specular_intensity','pbr_specular_power','roughness_power','selectioncolor','shadowfactor','specular_power_limit','state','state_noise','trans_rim_hard','trans_rim_inradius','transcolor','transcolor_rimlight ')
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
                if ($material.textures -isnot [array] -or $material.textures.Count -ne 8) {
                    throw 'World object native material requires eight texture expressions'
                }
                $textureMask = 0
                foreach ($texture in $material.textures) {
                    Assert-ExactJsonProperties $texture @('expressionIndex','assetId','colorSpace') 'World object native material texture'
                    if (-not (Test-JsonNumber $texture.expressionIndex) -or [double]$texture.expressionIndex % 1 -ne 0 -or $texture.expressionIndex -notin 0..7 -or
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
                if ($textureMask -ne 255) { throw 'World object native texture coverage is incomplete' }
            }
            $objectResources[$resource.objectId] = $resource
        }
    }
    $trackCounts = @{}
    $templateRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $templateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($template in $templates) {
        $templateProperties = @('sequenceId','displayName','category','durationMs','interpolation','tracks','animationTracks')
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['objectMotion']) { $templateProperties += 'objectMotion' }
        if ($document.formatVersion -eq 3 -and $null -ne $template.PSObject.Properties['effectTracks']) { $templateProperties += 'effectTracks' }
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
        if ($total -lt 1 -or $total -gt 32) {
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
            if ($keys.Count -lt 2 -or $keys.Count -gt 256) {
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
                Assert-SequenceVector $key.scaleMultiplier 'World sequence key scale' $true
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
                if (0 -ne $startMs) {
                    throw "World sequence animation track is invalid: $($template.sequenceId)"
                }
                [void]$slotIds.Add($slot)
            }
            $animationSlotStarts[$slot] = $startMs
        }
        if ($null -ne $template.PSObject.Properties['effectTracks']) {
            if ($template.effectTracks -isnot [System.Array] -or $total + @($template.effectTracks).Count -gt 32) {
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
                Assert-ExactJsonProperties $effect @('effectTrackId','slotId','resourceKind','resourceId','timing','startMs','durationMs','positionOffset','rotationDegrees','scale') 'World Object effect track'
                if ($effect.effectTrackId -isnot [string] -or $effect.effectTrackId -notmatch $stableId -or
                    -not $effectIds.Add([string]$effect.effectTrackId) -or $effect.slotId -isnot [string] -or
                    -not $slotIds.Contains([string]$effect.slotId) -or $effect.resourceKind -cnotin @('LEAF','GROUP') -or
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
        # One binding per slot, so a chained slot and a slot that also carries
        # a transform track each still count once.
        $trackCounts[[string]$template.sequenceId] = $slotIds.Count
    }
    $instanceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $instanceRows = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($instance in $instances) {
        $instanceProperties = @('instanceId','templateId','enabled','startDelayMs','playbackSpeed','bindings')
        foreach ($optional in @('anchorKind','position','motionEnd','nextMotionId','walkableSurface')) {
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
        if ($null -ne $template.PSObject.Properties['effectTracks'] -and @($template.effectTracks).Count -gt 0 -and
            ($bindings.Count -ne 1 -or $bindings[0].targetKind -cne 'OBJECT_RESOURCE')) {
            throw 'World Object effect lanes require one Object Resource binding'
        }
        foreach ($binding in $bindings) {
            Assert-ExactJsonProperties $binding `
                @('slotId','targetKind','targetId') 'World sequence binding'
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
                    ($null -ne $member.PSObject.Properties['motionEnd'] -and $member.motionEnd -cne 'STOP') -or
                    @($member.bindings).Count -ne 1 -or $member.bindings[0].targetKind -cne 'OBJECT_RESOURCE') {
                    throw 'Object group member must be one Map Object motion ending with Stop'
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

function Assert-WorldSequencePlacementTargets {
    param([object]$Document, [Collections.Generic.List[object]]$Files)
    $bindings = @($Document.instances | ForEach-Object { $_.bindings })
    $mapBindings = @($bindings | Where-Object { $_.targetKind -ceq 'MAP_PLACEMENT' })
    $deployBindings = @($bindings | Where-Object { $_.targetKind -ceq 'DEPLOY_PLACEMENT' })
    if ($mapBindings.Count -eq 0 -and $deployBindings.Count -eq 0) { return }

    function Read-SequenceTargetLines([string]$Name, [string]$Extension) {
        if ($Name -notmatch '^[A-Za-z0-9_.-]+$' -or $Name.Length -gt 260 -or
            [IO.Path]::GetExtension($Name) -cne $Extension) {
            throw "World sequence target filename is invalid: $Name"
        }
        if ($Scope -eq 'Area') {
            $staged = @($Files | Where-Object { $_.Name -ceq $Name })
            if ($staged.Count -ne 1) { throw "World sequence target file is not staged: $Name" }
            return $staged[0].Lines
        }
        $path = Join-Path $runtimeRoot $Name
        if (-not [IO.File]::Exists($path)) {
            throw "World sequence target runtime is missing: $path. Publish Scope Area first."
        }
        return [IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8)
    }

    $placements = [Collections.Generic.Dictionary[uint64,object]]::new()
    if ($mapBindings.Count -gt 0) {
        $sharded = if ($Scope -eq 'Area') {
            @($Files | Where-Object { $_.Name -ceq "$AreaId.mapset" }).Count -eq 1
        } else {
            if ($areaEntry.catalogType -cnotin @('single','shard-set')) {
                throw "World sequence Map target catalog type is invalid: $AreaId"
            }
            $areaEntry.catalogType -ceq 'shard-set'
        }
        $pairs = [Collections.Generic.List[object]]::new()
        if ($sharded) {
            $set = @(Read-SequenceTargetLines "$AreaId.mapset" '.mapset')
            $header = @(Split-DeployAuthoringTokens $set[0] 'World sequence target shard set')
            if ($header.Count -ne 4 -or $header[0] -cne 'LOSTARK_MAP_SHARD_SET' -or
                $header[1] -cne '1' -or $header[2] -cne $AreaId -or
                $header[3] -notmatch '^[0-9]+$' -or [uint32]$header[3] -notin 1..64 -or
                [uint32]$header[3] -ne $set.Count - 1) { throw 'Invalid world sequence target shard set' }
            $shardIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($line in $set[1..($set.Count - 1)]) {
                $row = @(Split-DeployAuthoringTokens $line 'World sequence target shard')
                if ($row.Count -ne 5 -or $row[0] -cnotmatch '^[A-Za-z0-9_.-]+$' -or
                    -not $shardIds.Add($row[0]) -or $row[3] -notmatch '^[0-9]+$' -or
                    $row[4] -notmatch '^[0-9]+$') { throw 'Invalid world sequence target shard row' }
                $pairs.Add([pscustomobject]@{ Catalog = $row[1]; Placements = $row[2];
                    AssetCount = [uint32]$row[3]; PlacementCount = [uint32]$row[4] })
            }
        } else {
            $pairs.Add([pscustomobject]@{ Catalog = "$AreaId.mapassets";
                Placements = "$AreaId.mapplacements"; AssetCount = -1; PlacementCount = -1 })
        }
        foreach ($pair in $pairs) {
            $catalogLines = @(Read-SequenceTargetLines $pair.Catalog '.mapassets')
            $placementLines = @(Read-SequenceTargetLines $pair.Placements '.mapplacements')
            $catalog = Read-MapAssetCatalog $pair.Catalog -Lines $catalogLines -Published
            $rows = @(Read-PlacementDocument $pair.Placements -Lines $placementLines)
            if ($sharded -and ($catalog.AssetIds.Count -ne $pair.AssetCount -or
                    $rows.Count -ne $pair.PlacementCount)) { throw 'World sequence target shard count mismatch' }
            foreach ($row in $rows) {
                $parsed = Parse-PlacementRow $row $pair.Placements
                if ($placements.ContainsKey($parsed.PlacementId) -or
                    -not $catalog.Assets.ContainsKey($parsed.AssetId)) {
                    throw "Invalid world sequence Map target placement/asset: $($parsed.PlacementId)/$($parsed.AssetId)"
                }
                $placements.Add($parsed.PlacementId, $catalog.Assets[$parsed.AssetId])
            }
        }
    }
    $deploy = $null
    if ($deployBindings.Count -gt 0) {
        $catalogName = "$AreaId.deployassets"
        $placementName = "$AreaId.deployplacements"
        $deploy = Read-DeployAuthoringPair -CatalogPath $catalogName -PlacementPath $placementName `
            -CatalogLines @(Read-SequenceTargetLines $catalogName '.deployassets') `
            -PlacementLines @(Read-SequenceTargetLines $placementName '.deployplacements')
    }
    foreach ($instance in $Document.instances) {
        foreach ($binding in $instance.bindings) {
            $context = "$($instance.instanceId)/$($binding.slotId) -> $($binding.targetId)"
            if ($binding.targetKind -ceq 'MAP_PLACEMENT') {
                $target = [uint64]$binding.targetId
                if (-not $placements.ContainsKey($target) -or $placements[$target].RenderMode -ceq 'Sky') {
                    throw "Invalid Map binding: $context. Publish Scope Area with its placements/assets first."
                }
            } elseif ($binding.targetKind -ceq 'DEPLOY_PLACEMENT') {
                # Clip existence remains the native CModel validator's contract;
                # catalog roles do not enumerate every clip on an animated prop.
                $target = ([uint64]$binding.targetId).ToString([Globalization.CultureInfo]::InvariantCulture)
                if (-not $deploy.Placements.ContainsKey($target) -or
                    $deploy.Assets[$deploy.Placements[$target].AssetId].Kind -cne 'ANIM') {
                    throw "Invalid animated Deploy binding: $context. Publish Scope Area with its placements/assets first."
                }
            }
        }
    }
}

function Add-WorldSequencePublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:worldSequencesDeclared) {
        if (-not [IO.File]::Exists($authoringSequencePath)) {
            throw "Declared world sequence authoring source is missing: $authoringSequencePath"
        }
        $lines = @(Read-WorldSequenceDocument $authoringSequencePath)
        # Join the exact validated snapshot against the files this operation will
        # leave installed. A sequence-only save cannot publish new Map targets.
        Assert-WorldSequencePlacementTargets ($lines -join "`n" | ConvertFrom-Json) $Files
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.worldsequences.json"
            Lines = $lines
        })
    }
}

function Read-CameraShotDocument {
    param([string]$Path)
    if ((Get-Item -LiteralPath $Path).Length -gt 262144) { throw "Camera document exceeds 256 KiB" }
    $text = [IO.File]::ReadAllText($Path, [Text.UTF8Encoding]::new($false, $true))
    try { $document = $text | ConvertFrom-Json }
    catch { throw "Camera shot JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','revision','shots') 'Camera shot root'
    if ($document.schema -ne 'lostark.camera-shots' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1 -or
        $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        [double]$document.revision -lt 1 -or
        [math]::Floor([double]$document.revision) -ne [double]$document.revision) {
        throw "Camera shot header is invalid: $Path"
    }
    # Level_KakulSaydonArena.cpp 의 상한과 동일하게 검사한다.
    $shots = @($document.shots)
    if ($shots.Count -gt 64) {
        throw "Camera shot document exceeds its limits: $Path"
    }
    $stableId = '^[A-Za-z0-9._-]{1,128}$'
    $shotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($shot in $shots) {
        $shotProperties = @('shotId','sequenceInstanceId','box','eye','lookAt','fovYDegrees','blendInMs','blendOutMs','priority')
        # A shot may carry an optional keyframed camera track. Without it the
        # shot is the single authored pose it has always been.
        if ($null -ne $shot.PSObject.Properties['cameraTrack']) {
            $shotProperties += 'cameraTrack'
        }
        # A side scrolling stage slides one framing with the player instead
        # of pinning it, so that shot carries two offsets and no track.
        if ($null -ne $shot.PSObject.Properties['follow']) {
            $shotProperties += 'follow'
        }

        foreach ($optional in @('displayName','defaultHoldMs','transitionEasing','activation')) {
            if ($null -ne $shot.PSObject.Properties[$optional]) { $shotProperties += $optional }
        }
        Assert-ExactJsonProperties $shot $shotProperties 'Camera shot'
        if ($null -ne $shot.PSObject.Properties['displayName']) {
            if ($shot.displayName -isnot [string] -or $shot.displayName.Length -eq 0 -or
                [Text.Encoding]::UTF8.GetByteCount($shot.displayName) -gt 128 -or $shot.displayName.Contains([char]0)) {
                throw "Camera displayName requires 1..128 UTF-8 bytes: $($shot.shotId)"
            }
        }
        if ($null -ne $shot.PSObject.Properties['defaultHoldMs']) {
            $hold = $shot.defaultHoldMs
            if (-not (Test-JsonNumber $hold) -or [double]$hold -lt 0 -or [double]$hold -gt 600000 -or
                [math]::Floor([double]$hold) -ne [double]$hold -or
                [double]$hold + [double]$shot.blendInMs -lt 1 -or [double]$hold + [double]$shot.blendInMs -gt 600000) {
                throw "Camera entry plus default hold must be 1..600000 ms: $($shot.shotId)"
            }
        }
        if ($null -ne $shot.PSObject.Properties['transitionEasing'] -and
            ($shot.transitionEasing -isnot [string] -or $shot.transitionEasing -cnotin @('LINEAR','SMOOTHSTEP'))) {
            throw "Camera transitionEasing must be LINEAR or SMOOTHSTEP: $($shot.shotId)"
        }
        if ($null -ne $shot.PSObject.Properties['activation'] -and
            ($shot.activation -isnot [string] -or $shot.activation -cnotin @('AUTO','PATTERN_ONLY'))) {
            throw "Camera activation must be AUTO or PATTERN_ONLY: $($shot.shotId)"
        }
        if ($shot.shotId -isnot [string] -or
            $shot.shotId -notmatch $stableId -or
            -not $shotIds.Add([string]$shot.shotId)) {
            throw "Camera shot id is invalid or duplicated: $($shot.shotId)"
        }
        if ($shot.sequenceInstanceId -isnot [string] -or
            ($shot.sequenceInstanceId -ne '' -and
                $shot.sequenceInstanceId -notmatch $stableId)) {
            throw "Camera shot sequence binding is invalid: $($shot.shotId)"
        }
        Assert-ExactJsonProperties $shot.box `
            @('center','halfExtents','yawDegrees') "Camera shot box $($shot.shotId)"
        foreach ($triplet in @(@($shot.box.center), @($shot.box.halfExtents),
                @($shot.eye), @($shot.lookAt))) {
            if ($triplet.Count -ne 3) {
                throw "Camera shot vector must hold three numbers: $($shot.shotId)"
            }
            foreach ($component in $triplet) {
                if (-not (Test-JsonNumber $component)) {
                    throw "Camera shot vector component is not finite: $($shot.shotId)"
                }
            }
        }
        foreach ($extent in @($shot.box.halfExtents)) {
            if ([double]$extent -le 0 -or [double]$extent -gt 1000) {
                throw "Camera shot half extent is out of range: $($shot.shotId)"
            }
        }
        # An eye sitting on its own target has no direction and the engine
        # would reject the pose every frame.
        $delta = 0.0
        for ($axis = 0; $axis -lt 3; $axis++) {
            $step = [double]$shot.lookAt[$axis] - [double]$shot.eye[$axis]
            $delta += $step * $step
        }
        if ($delta -le 0.000001) {
            throw "Camera shot eye and lookAt coincide: $($shot.shotId)"
        }
        if (-not (Test-JsonNumber $shot.box.yawDegrees) -or
            [math]::Abs([double]$shot.box.yawDegrees) -gt 360) {
            throw "Camera shot yaw is out of range: $($shot.shotId)"
        }
        if (-not (Test-JsonNumber $shot.fovYDegrees) -or
            [double]$shot.fovYDegrees -le 1 -or [double]$shot.fovYDegrees -ge 179) {
            throw "Camera shot field of view is out of range: $($shot.shotId)"
        }
        foreach ($pair in @(@('blendInMs', 10000), @('blendOutMs', 10000), @('priority', 1000))) {
            $value = $shot.($pair[0])
            if (-not (Test-JsonNumber $value) -or
                [double]$value -lt 0 -or [double]$value -gt $pair[1] -or
                [math]::Floor([double]$value) -ne [double]$value) {
                throw "Camera shot $($pair[0]) is out of range: $($shot.shotId)"
            }
        }
        if ($null -ne $shot.PSObject.Properties['follow']) {
            Assert-ExactJsonProperties $shot.follow `
                @('eyeOffset','lookAtOffset') "Camera shot follow $($shot.shotId)"
            foreach ($triplet in @(@($shot.follow.eyeOffset), @($shot.follow.lookAtOffset))) {
                if ($triplet.Count -ne 3) {
                    throw "Camera shot follow offset must hold three numbers: $($shot.shotId)"
                }
                foreach ($component in $triplet) {
                    if (-not (Test-JsonNumber $component)) {
                        throw "Camera shot follow offset is not finite: $($shot.shotId)"
                    }
                }
            }
            # The follow pose needs a direction for the same reason the
            # authored pose does.
            $followDelta = 0.0
            for ($axis = 0; $axis -lt 3; $axis++) {
                $step = [double]$shot.follow.lookAtOffset[$axis] - [double]$shot.follow.eyeOffset[$axis]
                $followDelta += $step * $step
            }
            if ($followDelta -le 0.000001) {
                throw "Camera shot follow offsets coincide: $($shot.shotId)"
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-CameraShotPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:cameraShotsDeclared) {
        if (-not [IO.File]::Exists($authoringCameraShotPath)) {
            throw "Declared camera shot authoring source is missing: $authoringCameraShotPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.camerashots.json"
            Lines = Read-CameraShotDocument $authoringCameraShotPath
        })
    }
}

function Read-WModelMaterialNames {
    param([string]$Path)
    $stream = [IO.File]::OpenRead($Path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ($stream.Length -lt 48) { throw "Truncated WModel: $Path" }
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WINT' -or
            $reader.ReadUInt16() -ne 1) { throw "Invalid WModel container: $Path" }
        $stream.Position = 16
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WMOD') {
            throw "Invalid WModel metadata: $Path"
        }
        $sectionCount = $reader.ReadUInt32()
        if ($sectionCount -lt 1 -or $sectionCount -gt 4096 -or
            48L + 64L * $sectionCount -gt $stream.Length) {
            throw "Invalid WModel section count: $Path"
        }
        $sections = [Collections.Generic.List[object]]::new()
        for ($index = 0; $index -lt $sectionCount; $index++) {
            $stream.Position = 48L + 64L * $index
            $type = $reader.ReadUInt32()
            $null = $reader.ReadUInt32()
            $offset = $reader.ReadUInt64()
            $size = $reader.ReadUInt64()
            if ($offset -gt [long]::MaxValue -or $size -gt [long]::MaxValue) {
                throw "Invalid WModel section range: $Path"
            }
            $sections.Add([pscustomobject]@{ Type = $type; Offset = [long]$offset; Size = [long]$size })
        }
        $base = 0L
        foreach ($candidate in @(16L, 48L)) {
            $position = $candidate + $sections[0].Offset
            if ($position -ge 0 -and $position -le $stream.Length - 4) {
                $stream.Position = $position
                if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ceq 'WINT') {
                    $base = $candidate
                    break
                }
            }
        }
        if (0 -eq $base) { throw "Cannot resolve WModel section base: $Path" }
        $materials = @($sections | Where-Object { $_.Type -eq 2 })
        if ($materials.Count -ne 1) { throw "WModel must have one material section: $Path" }
        $section = $materials[0]
        if ($section.Offset -gt $stream.Length - $base -or $section.Size -lt 24 -or
            $section.Size -gt $stream.Length - $base - $section.Offset) {
            throw "WModel material section is outside the file: $Path"
        }
        $start = $base + $section.Offset
        $stream.Position = $start
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WINT') {
            throw "Invalid WModel material header: $Path"
        }
        $stream.Position = $start + 16
        $magic = [Text.Encoding]::ASCII.GetString($reader.ReadBytes(4))
        $count = $reader.ReadUInt32()
        $stride = switch ($magic) { 'WMAT' { 596 } 'WMA2' { 4756 } 'WMA3' { 5340 } default { 0 } }
        if (0 -eq $stride -or $count -lt 1 -or $count -gt 4096 -or
            24L + [long]$count * $stride -gt $section.Size) {
            throw "Invalid WModel material table: $Path"
        }
        $names = [Collections.Generic.Dictionary[string,int]]::new([StringComparer]::Ordinal)
        $strictUtf8 = [Text.UTF8Encoding]::new($false, $true)
        for ($index = 0; $index -lt $count; $index++) {
            $stream.Position = $start + 24L + [long]$index * $stride + 12
            $name = $strictUtf8.GetString($reader.ReadBytes(64)).Split([char]0)[0]
            # The decoder preserves unused empty material slots. Only the
            # named row requested by the override must be unambiguous.
            if ([string]::IsNullOrEmpty($name)) { continue }
            if ($names.ContainsKey($name)) { $names[$name]++ }
            else { $names[$name] = 1 }
        }
        return [pscustomobject]@{ Names = $names }
    }
    finally { $reader.Dispose(); $stream.Dispose() }
}

function Read-MapMaterialDocument {
    param([string]$Path)
    $document = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8) | ConvertFrom-Json
    $rootFields = @('schema','formatVersion','areaId','materials')
    $placementLighting = $document.PSObject.Properties['placementLighting']
    if ($null -ne $placementLighting) {
        $rootFields += 'placementLighting'
        if ($document.formatVersion -ne 2 -or $placementLighting.Value -isnot [array]) {
            throw 'Placement lighting requires formatVersion 2 and an array'
        }
    }
    Assert-ExactJsonProperties $document $rootFields 'Map material root'
    if ($document.schema -cne 'lostark.map-materials' -or
        -not (Test-JsonNumber $document.formatVersion) -or $document.formatVersion -notin @(1,2) -or
        $document.areaId -cne $AreaId -or $document.materials -isnot [array] -or
        $document.materials.Count -lt 1 -or $document.materials.Count -gt 32768) {
        throw "Invalid map material document header: $Path"
    }
    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $modelNames = @{}
    $bakedAssets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $shadowAssets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $validateLightingTexture = {
        param($Value)
        if ($Value -isnot [string] -or [string]::IsNullOrWhiteSpace($Value) -or
            [IO.Path]::IsPathRooted($Value) -or $Value.Contains(':') -or
            '..' -in @($Value -split '[\\/]') -or [IO.Path]::GetExtension($Value) -cne '.dds') {
            throw 'Lighting texture must be a Resources-relative DDS'
        }
        $resolved = [IO.Path]::GetFullPath((Join-Path $runtimeResourceRoot $Value))
        $prefix = [IO.Path]::GetFullPath($runtimeResourceRoot).TrimEnd('\') + '\'
        if (-not $resolved.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase) -or -not [IO.File]::Exists($resolved)) {
            throw "Lighting texture is missing: $Value"
        }
    }
    foreach ($row in $document.materials) {
        # Render state is validated separately from each strict material family.
        $materialFields = [ordered]@{}
        foreach ($property in $row.PSObject.Properties) {
            if ($property.Name -cin @('renderMode','cullMode')) {
                if ($document.formatVersion -ne 2 -or $row.family -ceq 'diffuse-sampler' -or $property.Value -isnot [string]) { throw 'Invalid per-material draw state' }
                if ($property.Name -ceq 'renderMode' -and $property.Value -cnotin @('deferred','translucent','background','additive','water')) { throw 'Unknown material render mode' }
                if ($property.Name -ceq 'cullMode' -and $property.Value -cnotin @('back','front','none')) { throw 'Unknown material cull mode' }
            } else { $materialFields[$property.Name] = $property.Value }
        }
        $row = [pscustomobject]$materialFields
        if ($null -ne $row.PSObject.Properties['bakedLighting'] -and $null -ne $row.bakedLighting.PSObject.Properties['staticShadow']) {
            $shadow = $row.bakedLighting.staticShadow
            Assert-ExactJsonProperties $shadow @('texture','lightGuid','lightChannel','penumbraWidth','penumbraBasis','shadowExponent') 'Static shadow'
            if ($shadow.lightGuid -isnot [string] -or $shadow.lightGuid -cnotmatch '^[0-9a-f]{32}$' -or
                $shadow.penumbraBasis -cne 'PROJECT_ADAPTER' -or
                -not (Test-JsonNumber $shadow.lightChannel) -or $shadow.lightChannel -lt 1 -or $shadow.lightChannel -gt 15 -or [Math]::Floor($shadow.lightChannel) -ne $shadow.lightChannel -or
                -not (Test-JsonNumber $shadow.penumbraWidth) -or $shadow.penumbraWidth -le 0 -or $shadow.penumbraWidth -gt 1 -or
                -not (Test-JsonNumber $shadow.shadowExponent) -or $shadow.shadowExponent -le 0 -or $shadow.shadowExponent -gt 128) { throw 'Invalid static shadow transfer or light identity' }
            & $validateLightingTexture $shadow.texture
            [void]$shadowAssets.Add($row.assetId)
            $bakedFields = [ordered]@{}
            foreach ($field in $row.bakedLighting.PSObject.Properties) { if ($field.Name -cne 'staticShadow') { $bakedFields[$field.Name] = $field.Value } }
            $row.bakedLighting = [pscustomobject]$bakedFields
        }

        if ($row.family -ceq 'diffuse-sampler') {
            Assert-ExactJsonProperties $row @('assetId','materialName','sourceMaterial','family','sourceTexture','addressU') 'Map diffuse sampler'
            foreach ($key in @('assetId','materialName','sourceMaterial','sourceTexture')) {
                if ($row.$key -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$key) -or
                    $row.$key -match '[\x00-\x1F\x7F]') { throw "Invalid map diffuse sampler $key" }
            }
            if ($row.addressU -isnot [string] -or $row.addressU -cnotin @('WRAP','MIRROR') -or
                [Text.Encoding]::UTF8.GetByteCount($row.materialName) -gt 63 -or
                [Text.Encoding]::UTF8.GetByteCount($row.sourceMaterial) -gt 512 -or
                [Text.Encoding]::UTF8.GetByteCount($row.sourceTexture) -gt 512 -or
                -not $keys.Add($row.assetId + "`n" + $row.materialName) -or
                -not $script:mapMaterialModels.ContainsKey($row.assetId)) {
                throw "Invalid, duplicate, or unknown map diffuse sampler: $($row.assetId)/$($row.materialName)"
            }
            $modelPath = [string]$script:mapMaterialModels[$row.assetId]
            if (-not $modelNames.ContainsKey($modelPath)) {
                $modelNames[$modelPath] = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $modelPath)).Names
            }
            if (-not $modelNames[$modelPath].ContainsKey($row.materialName) -or $modelNames[$modelPath][$row.materialName] -lt 1) {
                throw "Map diffuse sampler material does not exist: $($row.assetId)/$($row.materialName)"
            }
            continue
        }
        if ($row.family -cin @('bg-source-snowice-opaque','bg-source-vertexblend-opaque','bg-source-wet-opaque')) {
            if ($document.formatVersion -ne 2) { throw 'Source special requires formatVersion 2' }
            $ice = $row.family -ceq 'bg-source-snowice-opaque'
            $blend = $row.family -ceq 'bg-source-vertexblend-opaque'
            $fields = @('assetId','materialName','sourceMaterial','family','textureColorSpace','castsShadow','diffuseColor','specularColor','reflectionColor','normalIntensity','diffuseBrightness','specularIntensity','specularPower','uvTiling','detailNormalIntensity','detailNormalTiling','diffuseTexture','normalTexture')
            $flags = 0
            if ($ice -or $blend) {
                if (-not (Test-JsonNumber $row.sourceSpecialFlags) -or $row.sourceSpecialFlags -lt 0 -or $row.sourceSpecialFlags -gt 3 -or [Math]::Floor($row.sourceSpecialFlags) -ne $row.sourceSpecialFlags) { throw 'Invalid source special flags' }
                $flags = [int]$row.sourceSpecialFlags
                $fields += 'sourceSpecialFlags'
            }
            $scalars = @('normalIntensity','diffuseBrightness','specularIntensity','specularPower','detailNormalIntensity','detailNormalTiling')
            $vectors = @('diffuseColor','specularColor','reflectionColor')
            if ($ice) {
                $scalars += 'sourceNormalTiling'
                $vectors += @('sourceIceCoreColor','sourceIceOuterColor','sourceIceBlend')
                $fields += @('sourceNormalTiling','sourceIceCoreColor','sourceIceOuterColor','sourceIceBlend','sourceIceBumpOffset')
                if (-not (Test-JsonNumber $row.sourceIceBumpOffset) -or [Math]::Abs([double]$row.sourceIceBumpOffset) -gt 1000000) { throw 'Invalid ice bump offset' }
            } elseif ($blend) {
                $scalars += 'sourceBlendSharpness'; $vectors += 'sourceRimlight'
                $fields += @('sourceBlendDiffuse','sourceBlendSpecular','sourceBlendLayers','sourceBlendSharpness','sourceRimlight')
                foreach ($key in @('sourceBlendDiffuse','sourceBlendSpecular','sourceBlendLayers')) {
                    if ($row.$key -isnot [array] -or $row.$key.Count -ne 4) { throw 'Invalid source blend layers' }
                    foreach ($layer in $row.$key) {
                        if ($layer -isnot [array] -or $layer.Count -ne 4) { throw 'Invalid source blend layer arity' }
                        foreach ($value in $layer) { if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt 1000000) { throw 'Invalid source blend layer value' } }
                    }
                }
                for ($i=0; $i -lt 4; $i++) {
                    if (($i -lt 2 -or ($flags -band (1 -shl ($i - 2)))) -and ($row.sourceBlendLayers[$i][0] -le 0 -or $row.sourceBlendLayers[$i][1] -le 0)) { throw 'Invalid source blend UV' }
                }
            } else {
                $scalars += @('sourceNormalTiling','sourceWetSpecularPower'); $vectors += 'sourceWetParameters'
                $fields += @('sourceNormalTiling','sourceWetParameters','sourceWetSpecularPower')
            }
            $textures = @(@('diffuseTexture','diffuse'),@('normalTexture','normal'))
            if (-not $blend) {
                $textures += ,@('reflectionTexture','reflection')
                if (-not $ice -or ($flags -band 2)) { $textures += ,@('specularTexture','specular') }
            }
            if ($ice) { $textures += ,@('specialMaskTexture','specialMask') }
            if ($blend -or ($ice -and ($flags -band 1))) { $textures += ,@('detailNormalTexture','detailNormal') }
            if ($blend) {
                $textures += @(@('overlayDiffuseTexture','overlay'),@('overlayNormalTexture','overlayNormal'))
                if ($flags -band 1) { $textures += @(@('blendDiffuseGTexture','blendG'),@('blendNormalGTexture','blendGNormal')) }
                if ($flags -band 2) { $textures += @(@('blendDiffuseBTexture','blendB'),@('blendNormalBTexture','blendBNormal')) }
            }
            $spaces = @()
            foreach ($texture in $textures) { $fields += $texture[0]; $spaces += $texture[1] }
            if ($null -ne $row.PSObject.Properties['bakedLighting']) { $fields += 'bakedLighting' }
            Assert-ExactJsonProperties $row @($fields | Select-Object -Unique) 'Source special'
            Assert-ExactJsonProperties $row.textureColorSpace $spaces 'Source special color spaces'
            foreach ($texture in $textures) {
                & $validateLightingTexture $row.($texture[0])
                $space = $row.textureColorSpace.($texture[1])
                if ($space -cnotin @('srgb','linear') -or ($texture[1] -match '[Nn]ormal' -and $space -cne 'linear')) { throw 'Invalid source special color space' }
            }
            foreach ($key in $scalars) { if (-not (Test-JsonNumber $row.$key) -or $row.$key -lt 0 -or $row.$key -gt [single]::MaxValue) { throw 'Invalid source special scalar' } }
            foreach ($key in $vectors) {
                if ($row.$key -isnot [array] -or $row.$key.Count -ne 4) { throw 'Invalid source special vector' }
                foreach ($value in $row.$key) { if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) { throw 'Invalid source special vector value' } }
            }
            if ($row.uvTiling -isnot [array] -or $row.uvTiling.Count -ne 2 -or $row.castsShadow -isnot [bool] -or $row.detailNormalTiling -le 0 -or (-not $blend -and $row.sourceNormalTiling -le 0)) { throw 'Invalid source special UV or shadow' }
            foreach ($value in $row.uvTiling) { if (-not (Test-JsonNumber $value) -or $value -le 0 -or $value -gt 1000000) { throw 'Invalid source special UV value' } }
            foreach ($key in @('assetId','materialName','sourceMaterial')) {
                if ($row.$key -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$key) -or $row.$key -match '[\x00-\x1f\x7f]') { throw 'Invalid source special identity' }
            }
            if ([Text.Encoding]::UTF8.GetByteCount($row.materialName) -gt 63 -or [Text.Encoding]::UTF8.GetByteCount($row.sourceMaterial) -gt 512 -or
                -not $keys.Add($row.assetId + "`n" + $row.materialName) -or -not $script:mapMaterialModels.ContainsKey($row.assetId)) { throw 'Invalid or duplicate source special identity' }
            if ($null -ne $row.PSObject.Properties['bakedLighting']) {
                Assert-ExactJsonProperties $row.bakedLighting @('averageTexture','directionalTexture','colorSpace') 'Source special baked lighting'
                if ($row.bakedLighting.colorSpace -cnotin @('linear','srgb')) { throw 'Invalid source special lightmap color space' }
                & $validateLightingTexture $row.bakedLighting.averageTexture
                & $validateLightingTexture $row.bakedLighting.directionalTexture
                [void]$bakedAssets.Add($row.assetId)
            }
            $modelPath = [string]$script:mapMaterialModels[$row.assetId]
            if (-not $modelNames.ContainsKey($modelPath)) { $modelNames[$modelPath] = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $modelPath)).Names }
            if (-not $modelNames[$modelPath].ContainsKey($row.materialName)) { throw 'Source special material does not exist in WModel' }
            continue
        }
        if ($row.family -cin @('bg-source-foliage-masked','bg-source-grass-masked')) {
            if ($document.formatVersion -ne 2) { throw 'Source foliage requires formatVersion 2' }
            $fields = @('assetId','materialName','sourceMaterial','family','sourceFlags','transmissionColor','diffuseBrightness','normalIntensity','specularIntensity','specularPower','diffuseSaturation','diffuseColor','specularColor','castsShadow','diffuseTexture','textureColorSpace')
            foreach ($optional in @('normalTexture','specularTexture','maskTexture','emissive','bakedLighting')) {
                if ($null -ne $row.PSObject.Properties[$optional]) { $fields += $optional }
            }
            Assert-ExactJsonProperties $row $fields 'Source foliage'
            foreach ($key in @('assetId','materialName','sourceMaterial','family')) {
                if ($row.$key -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$key) -or $row.$key -match '[\x00-\x1f\x7f]') { throw "Invalid source foliage identity: $key" }
            }
            if ([Text.Encoding]::UTF8.GetByteCount($row.materialName) -gt 63 -or [Text.Encoding]::UTF8.GetByteCount($row.sourceMaterial) -gt 512 -or
                -not $keys.Add($row.assetId + "`n" + $row.materialName) -or -not $script:mapMaterialModels.ContainsKey($row.assetId)) { throw 'Invalid source foliage material identity' }
            if (-not (Test-JsonNumber $row.sourceFlags) -or $row.sourceFlags -lt 0 -or $row.sourceFlags -gt 127 -or [Math]::Floor($row.sourceFlags) -ne $row.sourceFlags) { throw 'Invalid source foliage flags' }
            $flags = [int]$row.sourceFlags
            if (($flags -band 8) -ne 0 -and ($flags -band 4) -eq 0) { throw 'Source foliage specular branch mismatch' }
            foreach ($key in @('diffuseBrightness','normalIntensity','specularIntensity','specularPower','diffuseSaturation')) {
                if (-not (Test-JsonNumber $row.$key) -or $row.$key -lt 0 -or $row.$key -gt [single]::MaxValue) { throw "Invalid source foliage scalar: $key" }
            }
            foreach ($key in @('diffuseColor','specularColor','transmissionColor')) {
                if ($row.$key -isnot [array] -or $row.$key.Count -ne 4) { throw "Invalid source foliage color: $key" }
                foreach ($value in $row.$key) { if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) { throw 'Invalid foliage color component' } }
            }
            if ($row.castsShadow -isnot [bool]) { throw 'Invalid foliage shadow policy' }
            Assert-ExactJsonProperties $row.textureColorSpace @('diffuse','normal','specular','mask') 'Foliage color spaces'
            foreach ($key in @('diffuse','normal','specular','mask')) { if ($row.textureColorSpace.$key -cnotin @('srgb','linear')) { throw 'Invalid foliage texture color space' } }
            if ($row.textureColorSpace.normal -cne 'linear') { throw 'Foliage normal must be linear' }
            & $validateLightingTexture $row.diffuseTexture
            if (($flags -band 1) -ne 0) { & $validateLightingTexture $row.normalTexture }
            if (($flags -band 8) -ne 0) { & $validateLightingTexture $row.specularTexture }
            if ($row.family -ceq 'bg-source-foliage-masked') { & $validateLightingTexture $row.maskTexture }
            if ($null -ne $row.PSObject.Properties['bakedLighting']) {
                Assert-ExactJsonProperties $row.bakedLighting @('averageTexture','directionalTexture','colorSpace') 'Foliage baked lighting'
                if ($row.bakedLighting.colorSpace -cnotin @('linear','srgb')) { throw 'Invalid foliage lightmap color space' }
                & $validateLightingTexture $row.bakedLighting.averageTexture
                & $validateLightingTexture $row.bakedLighting.directionalTexture
                [void]$bakedAssets.Add($row.assetId)
            }
            if ($null -ne $row.PSObject.Properties['emissive']) {
                $emissive = $row.emissive
                if (($flags -band 32) -eq 0) { throw 'Unexpected foliage emissive input' }
                Assert-ExactJsonProperties $emissive @('texture','color','intensity','uvTiling','colorSpace','flicker') 'Foliage emissive'
                Assert-ExactJsonProperties $emissive.flicker @('minimum','speed','phaseOffset') 'Foliage flicker'
                & $validateLightingTexture $emissive.texture
                if ($emissive.colorSpace -cnotin @('linear','srgb') -or $emissive.color -isnot [array] -or $emissive.color.Count -ne 4 -or $emissive.uvTiling -isnot [array] -or $emissive.uvTiling.Count -ne 2) { throw 'Invalid foliage emissive shape' }
                foreach ($value in @($emissive.color) + @($emissive.intensity,$emissive.flicker.minimum,$emissive.flicker.speed)) {
                    if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) { throw 'Invalid foliage emissive value' }
                }
                foreach ($value in $emissive.uvTiling) { if (-not (Test-JsonNumber $value) -or $value -le 0 -or $value -gt 1000000) { throw 'Invalid foliage emissive UV' } }
                if (-not (Test-JsonNumber $emissive.flicker.phaseOffset) -or [Math]::Abs([double]$emissive.flicker.phaseOffset) -gt 1000000) { throw 'Invalid foliage emissive phase' }
            } elseif (($flags -band 32) -ne 0) { throw 'Missing foliage emissive input' }
            $modelPath = [string]$script:mapMaterialModels[$row.assetId]
            if (-not $modelNames.ContainsKey($modelPath)) { $modelNames[$modelPath] = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $modelPath)).Names }
            if (-not $modelNames[$modelPath].ContainsKey($row.materialName) -or $modelNames[$modelPath][$row.materialName] -lt 1) { throw 'Foliage material does not exist in WModel' }
            continue
        }
        if ($row.family -is [string] -and $row.family.StartsWith('source.', [StringComparison]::Ordinal)) {
            if ($document.formatVersion -ne 2) { throw 'Native map material requires formatVersion 2' }
            $nativeFields = @('assetId','materialName','sourceMaterial','family','parameters','textures')
            if ($null -ne $row.PSObject.Properties['bakedLighting']) {
                $supportsNativeBaked = $row.family -cin @('source.character.monster-862fa1000fe2.v1','source.character.monster-3c300c108ac5.v1','source.character.monster-7373ec8df226.v1','source.character.monster-a2e0ec089348.v1')
                if ($row.family -cmatch '^source\.map\.translucent-(\d+)\.v1$') {
                    $nativeProgram = [int]$Matches[1]
                    $supportsNativeBaked = $nativeProgram -ge 44 -and $nativeProgram -le 63 -and $nativeProgram -notin @(47,53,55)
                }
                elseif ($row.family -cmatch '^source\.map\.water-(\d+)\.v1$') {
                    $nativeProgram = [int]$Matches[1]
                    $supportsNativeBaked = $nativeProgram -ge 40 -and $nativeProgram -le 43
                }
                if (-not $supportsNativeBaked) { throw 'Unsupported native baked lighting family' }
                $nativeFields += 'bakedLighting'
                Assert-ExactJsonProperties $row.bakedLighting @('averageTexture','directionalTexture','colorSpace') 'Native baked lighting'
                if ($row.bakedLighting.colorSpace -cnotin @('linear','srgb')) { throw 'Invalid native lightmap color space' }
                & $validateLightingTexture $row.bakedLighting.averageTexture
                & $validateLightingTexture $row.bakedLighting.directionalTexture
                [void]$bakedAssets.Add($row.assetId)
            }
            Assert-ExactJsonProperties $row $nativeFields 'Native map material'
            foreach ($key in @('assetId','materialName','sourceMaterial','family')) {
                if ($row.$key -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$key) -or $row.$key -match '[\x00-\x1F\x7F]') {
                    throw "Invalid native map identity: $key"
                }
            }
            if ([Text.Encoding]::UTF8.GetByteCount($row.materialName) -gt 63 -or
                [Text.Encoding]::UTF8.GetByteCount($row.sourceMaterial) -gt 512 -or
                -not $keys.Add($row.assetId + "`n" + $row.materialName) -or
                -not $script:mapMaterialModels.ContainsKey($row.assetId) -or
                $row.parameters -isnot [pscustomobject] -or $row.textures -isnot [array]) { throw 'Invalid native map material contract' }
            $texturelessHelper = $row.family -cin @('source.map.black.v1','source.map.shadow-modulate.v1')
            if ($texturelessHelper -ne ($row.textures.Count -eq 0)) { throw 'Invalid textureless native helper' }
            foreach ($parameter in $row.parameters.PSObject.Properties) {
                $components = @($parameter.Value)
                if ($components.Count -ne 1 -and $components.Count -ne 4) { throw 'Invalid native parameter arity' }
                foreach ($value in $components) {
                    if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt 1000000) { throw 'Invalid native material parameter' }
                }
            }
            $textureIndices = [Collections.Generic.HashSet[int]]::new()
            foreach ($texture in $row.textures) {
                Assert-ExactJsonProperties $texture @('expressionIndex','assetId','colorSpace') 'Native map texture'
                if (-not (Test-JsonNumber $texture.expressionIndex) -or $texture.expressionIndex -lt 0 -or
                    $texture.expressionIndex -ge 16 -or [Math]::Floor($texture.expressionIndex) -ne $texture.expressionIndex -or
                    -not $textureIndices.Add([int]$texture.expressionIndex) -or $texture.colorSpace -cnotin @('srgb','linear')) {
                    throw 'Invalid native map texture index or color space'
                }
                & $validateLightingTexture $texture.assetId
            }
            $modelPath = [string]$script:mapMaterialModels[$row.assetId]
            if (-not $modelNames.ContainsKey($modelPath)) {
                $modelNames[$modelPath] = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $modelPath)).Names
            }
            if (-not $modelNames[$modelPath].ContainsKey($row.materialName) -or $modelNames[$modelPath][$row.materialName] -lt 1) {
                throw 'Native map material does not exist in WModel'
            }
            continue
        }
        $fields = @('assetId','materialName','sourceMaterial','family','diffuseBrightness',
            'normalIntensity','specularIntensity','specularPower','reflectionIntensity',
            'reflectionContrast','diffuseColor','specularColor','reflectionColor',
            'reflectionTexture','textureColorSpace')
        $scalarFields = @('diffuseBrightness','normalIntensity','specularIntensity',
            'specularPower','reflectionIntensity','reflectionContrast')
        $isSourceBg = $row.family -ceq 'bg-source-opaque-masked'
        $isSourceOverlay = $row.family -ceq 'bg_base_opa_overlay'
        $isSourceSpecular = $row.family -ceq 'bg_seamless-specular_opa'
        $isPBR = $row.family -cin @('bg_base_pbr_seamless_opa','bg_base_pbr_opa')
        if ($isPBR) {
            if ($document.formatVersion -ne 2) { throw 'PBR map material requires formatVersion 2' }
            $fields = @('assetId','materialName','sourceMaterial','family','textureColorSpace','diffuseColor','reflectionColor','uvTiling','reflectionOriginOffset','diffuseBrightness','normalIntensity','reflectionIntensity','reflectionContrast','reflectionTiling','diffuseSaturation','detailNormalIntensity','detailNormalTiling','metallicIntensity','metallicPower','roughnessIntensity','roughnessPower','aoIntensity','aoPower','specularPBRIntensity','nonmetallicBrightness','metallicBrightness','minimumRoughness','vertexAlpha','uvFixedNormal','useWorldReflection','castsShadow','diffuseTexture','normalTexture','detailNormalTexture','ormTexture','reflectionTexture')
            $scalarFields = @('diffuseBrightness','normalIntensity','reflectionIntensity','reflectionContrast','reflectionTiling','diffuseSaturation','detailNormalIntensity','detailNormalTiling','metallicIntensity','metallicPower','roughnessIntensity','roughnessPower','aoIntensity','aoPower','specularPBRIntensity','nonmetallicBrightness','metallicBrightness','minimumRoughness','vertexAlpha')
        }
        switch -CaseSensitive ($row.family) {
            'bg_seamless-specular_msk' { $fields += 'reflectionTiling'; $scalarFields += 'reflectionTiling' }
            'bg_base_msk' { $fields += 'diffuseSaturation'; $scalarFields += 'diffuseSaturation' }
            'bg_base_pbr_seamless_opa' { }
            'bg_base_pbr_opa' { }
            'bg-source-opaque-masked' {
                if ($document.formatVersion -ne 2) { throw 'Source BG requires formatVersion 2' }
                $fields = @('assetId','materialName','sourceMaterial','family','textureColorSpace',
                    'diffuseBrightness','normalIntensity','specularIntensity','specularPower',
                    'reflectionIntensity','reflectionContrast','reflectionTiling','diffuseSaturation',
                    'diffuseColor','specularColor','reflectionColor','uvTiling','reflectionOriginOffset',
                    'castsShadow','diffuseTexture','sourceFlags','sourceBump','sourceUV','flickerMode','addressU')
                $scalarFields += @('reflectionTiling','diffuseSaturation')
                foreach ($optional in @('normalTexture','specularTexture','reflectionTexture','bakedLighting','emissive','detailNormalTexture','detailNormalIntensity','detailNormalTiling','sourceSubspecular','sourceRimlight','sourceSpecularSaturation','sourcePanning')) {
                    if ($null -ne $row.PSObject.Properties[$optional]) { $fields += $optional }
                }
            }
            'bg_base_opa_overlay' {
                if ($document.formatVersion -ne 2) { throw 'Source overlay requires formatVersion 2' }
                $fields = @('assetId','materialName','sourceMaterial','family','textureColorSpace',
                    'diffuseBrightness','diffuseSaturation','normalIntensity','specularIntensity','specularPower',
                    'diffuseColor','specularColor','overlayColor','overlayTiling','overlayNormalIntensity',
                    'overlaySharpness','overlayBrightness','overlaySaturation','overlaySpecularIntensity',
                    'castsShadow','diffuseTexture','normalTexture','overlayDiffuseTexture','overlayNormalTexture')
                $scalarFields = @('diffuseBrightness','diffuseSaturation','normalIntensity','specularIntensity',
                    'specularPower','overlayTiling','overlayNormalIntensity','overlaySharpness',
                    'overlayBrightness','overlaySaturation','overlaySpecularIntensity')
                foreach ($optional in @('bakedLighting','uvTiling','specularTexture')) {
                    if ($null -ne $row.PSObject.Properties[$optional]) { $fields += $optional }
                }
                foreach ($optional in @('sourceOverlayFlags','sourceDirection','sourceUV','detailNormalTexture','detailNormalIntensity','detailNormalTiling')) {
                    if ($null -ne $row.PSObject.Properties[$optional]) { $fields += $optional }
                }
                if ($null -ne $row.PSObject.Properties['sourceOverlayFlags']) {
                    if (-not (Test-JsonNumber $row.sourceOverlayFlags) -or $row.sourceOverlayFlags -lt 0 -or $row.sourceOverlayFlags -gt 511 -or [Math]::Floor($row.sourceOverlayFlags) -ne $row.sourceOverlayFlags) { throw 'Invalid source overlay flags' }
                    if (($row.sourceOverlayFlags -band 1) -eq 0) { $fields = @($fields | Where-Object { $_ -cne 'normalTexture' }) }
                    if (($row.sourceOverlayFlags -band 2) -eq 0) { $fields = @($fields | Where-Object { $_ -cne 'overlayNormalTexture' }) }
                }
            }
            'bg_seamless-specular_opa' {
                if ($document.formatVersion -ne 2) { throw 'Source specular requires formatVersion 2' }
                $fields += @('reflectionTiling','diffuseSaturation','uvTiling','reflectionOriginOffset','castsShadow','diffuseTexture','normalTexture','specularTexture')
                $scalarFields += @('reflectionTiling','diffuseSaturation')
                if ($null -ne $row.PSObject.Properties['bakedLighting']) { $fields += 'bakedLighting' }
            }
            default { throw "Unsupported map material family: $($row.family)" }
        }
        if ($isPBR) {
            foreach ($optional in @('bakedLighting','environment','emissive')) {
                if ($null -ne $row.PSObject.Properties[$optional]) { $fields += $optional }
            }
        }
        Assert-ExactJsonProperties $row $fields "Map material row $($row.assetId)"
        $identityFields = if ($isSourceOverlay -or $isSourceBg) { @('assetId','materialName','sourceMaterial') } else { @('assetId','materialName','sourceMaterial','reflectionTexture') }
        foreach ($key in $identityFields) {
            if ($row.$key -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$key) -or
                $row.$key -match '[\x00-\x1F\x7F]') { throw "Invalid map material $key" }
        }
        if ([Text.Encoding]::UTF8.GetByteCount($row.materialName) -gt 63 -or
            [Text.Encoding]::UTF8.GetByteCount($row.sourceMaterial) -gt 512 -or
            -not $keys.Add($row.assetId + "`n" + $row.materialName) -or
            -not $script:mapMaterialModels.ContainsKey($row.assetId)) {
            throw "Invalid, duplicate, or unknown map material identity: $($row.assetId)/$($row.materialName)"
        }
        foreach ($key in $scalarFields) {
            $allowSigned = $isSourceBg -and $key -ceq 'normalIntensity'
            if (-not (Test-JsonNumber $row.$key) -or (-not $allowSigned -and [double]$row.$key -lt 0) -or
                [Math]::Abs([double]$row.$key) -gt [single]::MaxValue) { throw "Invalid map material number: $key" }
        }
        if ((-not $isPBR -and -not $isSourceBg -and -not $isSourceOverlay -and [double]$row.specularPower -lt 1) -or
            (($isPBR -or $isSourceSpecular -or $row.family -ceq 'bg_seamless-specular_msk') -and [double]$row.reflectionTiling -le 0)) {
            throw "Invalid map material power or tiling: $($row.assetId)"
        }
        $colors = if ($isSourceOverlay) { @('diffuseColor','specularColor','overlayColor') } elseif ($isPBR) { @('diffuseColor','reflectionColor') } else { @('diffuseColor','specularColor','reflectionColor') }
        foreach ($key in $colors) {
            if ($row.$key -isnot [array] -or $row.$key.Count -ne 4) { throw "Invalid map material color: $key" }
            foreach ($component in $row.$key) {
                if (-not (Test-JsonNumber $component) -or [double]$component -lt 0 -or
                    [double]$component -gt [single]::MaxValue) { throw "Invalid map material color component: $key" }
            }
        }
        if ($isPBR) {
            foreach ($key in @('uvFixedNormal','useWorldReflection','castsShadow')) {
                if ($row.$key -isnot [bool]) { throw "Invalid PBR boolean: $key" }
            }
            foreach ($key in @('uvTiling','reflectionOriginOffset')) {
                if ($row.$key -isnot [array] -or $row.$key.Count -ne 2) { throw "Invalid PBR vector: $key" }
                foreach ($value in $row.$key) {
                    if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt [single]::MaxValue) {
                        throw "Invalid PBR vector component: $key"
                    }
                }
            }
            if ($row.uvTiling[0] -le 0 -or $row.uvTiling[1] -le 0 -or
                $row.detailNormalTiling -le 0 -or $row.minimumRoughness -le 0 -or
                $row.minimumRoughness -gt 1 -or $row.vertexAlpha -gt 1 -or
                $row.textureColorSpace.normal -cne 'linear' -or $row.textureColorSpace.detailNormal -cne 'linear') {
                throw 'Invalid PBR bounds or normal color space'
            }
        }
        if ($isSourceSpecular -or $isSourceBg) {
            if ($row.castsShadow -isnot [bool] -or $row.textureColorSpace.normal -cne 'linear') {
                throw 'Invalid source specular shadow or normal color space'
            }
            foreach ($key in @('uvTiling','reflectionOriginOffset')) {
                if ($row.$key -isnot [array] -or $row.$key.Count -ne 2) { throw "Invalid source specular vector: $key" }
                foreach ($value in $row.$key) {
                    if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt [single]::MaxValue) {
                        throw "Invalid source specular vector component: $key"
                    }
                }
            }
            if (-not $isSourceBg -and ($row.uvTiling[0] -le 0 -or $row.uvTiling[1] -le 0)) { throw 'Non-positive source specular UV tiling' }
        }
        if ($isSourceOverlay -and ($row.castsShadow -isnot [bool] -or $row.overlayTiling -le 0 -or
            $row.textureColorSpace.normal -cne 'linear' -or $row.textureColorSpace.overlayNormal -cne 'linear')) {
            throw 'Invalid source overlay shadow, tiling or normal color space'
        }
        if ($isSourceBg) {
            if (-not (Test-JsonNumber $row.sourceFlags) -or $row.sourceFlags -lt 0 -or $row.sourceFlags -gt 65535 -or
                [Math]::Floor($row.sourceFlags) -ne $row.sourceFlags -or
                -not (Test-JsonNumber $row.flickerMode) -or $row.flickerMode -lt 0 -or $row.flickerMode -gt 2 -or
                [Math]::Floor($row.flickerMode) -ne $row.flickerMode -or $row.addressU -cnotin @('WRAP','MIRROR')) {
                throw 'Invalid source BG flags, flicker or sampler'
            }
            $flags = [uint32]$row.sourceFlags
            foreach ($key in @('sourceSubspecular','sourcePanning','sourceRimlight')) {
                if ($null -eq $row.PSObject.Properties[$key]) { continue }
                $count = if ($key -ceq 'sourceRimlight') { 4 } else { 2 }
                if ($row.$key -isnot [array] -or $row.$key.Count -ne $count) { throw 'Invalid source BG branch vector' }
                foreach ($value in $row.$key) {
                    if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt 1000000 -or ($key -cne 'sourcePanning' -and $value -lt 0)) { throw 'Invalid source BG branch component' }
                }
            }
            if ($null -ne $row.PSObject.Properties['sourceSpecularSaturation'] -and
                (-not (Test-JsonNumber $row.sourceSpecularSaturation) -or $row.sourceSpecularSaturation -lt 0 -or $row.sourceSpecularSaturation -gt 1000000)) { throw 'Invalid source specular saturation' }
            if ((($flags -band 8) -ne 0 -and ($flags -band 4) -eq 0) -or
                (($flags -band 32) -ne 0 -and ($flags -band 16) -eq 0)) { throw 'Inconsistent source BG branches' }
            foreach ($vector in @('sourceBump','sourceUV')) {
                if ($row.$vector -isnot [array] -or $row.$vector.Count -ne 4) { throw "Invalid source BG vector: $vector" }
                foreach ($value in $row.$vector) {
                    if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt [single]::MaxValue) {
                        throw "Invalid source BG vector value: $vector"
                    }
                }
            }
            if ([Math]::Abs($row.sourceUV[0]*$row.sourceUV[0]+$row.sourceUV[1]*$row.sourceUV[1]-1) -gt 0.0001) {
                throw 'Source BG UV rotation must be unit length'
            }
        }
        $spaceKeys = if ($isSourceOverlay) { @('diffuse','normal','overlayDiffuse','overlayNormal') } elseif ($isSourceSpecular -or $isSourceBg) { @('diffuse','normal','specular','reflection') } elseif ($isPBR) { @('diffuse','normal','detailNormal','reflection','orm') } else { @('diffuse','specular','reflection') }
        if ($isSourceOverlay -and $null -ne $row.PSObject.Properties['specularTexture']) { $spaceKeys += 'specular' }
        if ($isSourceOverlay -and $null -ne $row.PSObject.Properties['uvTiling']) {
            if ($row.uvTiling -isnot [array] -or $row.uvTiling.Count -ne 2) { throw 'Invalid overlay UV tiling' }
            foreach ($value in $row.uvTiling) {
                if (-not (Test-JsonNumber $value) -or $value -le 0 -or $value -gt [single]::MaxValue) { throw 'Invalid overlay UV value' }
            }
        }
        Assert-ExactJsonProperties $row.textureColorSpace $spaceKeys 'Map material color spaces'
        foreach ($key in $spaceKeys) {
            if ($row.textureColorSpace.$key -isnot [string] -or
                $row.textureColorSpace.$key -cnotin @('srgb','linear')) { throw "Invalid map material color space: $key" }
        }
        $textureKeys = if ($isSourceOverlay) { @('diffuseTexture','normalTexture','overlayDiffuseTexture','overlayNormalTexture') } elseif ($isSourceSpecular -or $isSourceBg) { @('diffuseTexture','normalTexture','specularTexture','reflectionTexture') } elseif ($isPBR) { @('diffuseTexture','normalTexture','detailNormalTexture','ormTexture','reflectionTexture') } else { @('reflectionTexture') }
        if ($isSourceOverlay -and $null -ne $row.PSObject.Properties['specularTexture']) { $textureKeys += 'specularTexture' }
        if ($isSourceOverlay -and $null -ne $row.PSObject.Properties['sourceOverlayFlags']) {
            $hasOverlayDetail = ($row.sourceOverlayFlags -band 32) -ne 0
            foreach ($key in @('detailNormalTexture','detailNormalIntensity','detailNormalTiling')) {
                if ($hasOverlayDetail -ne ($null -ne $row.PSObject.Properties[$key])) { throw 'Inconsistent source overlay detail branch' }
            }
            if (($row.sourceOverlayFlags -band 1) -eq 0) { $textureKeys = @($textureKeys | Where-Object { $_ -cne 'normalTexture' }) }
            if (($row.sourceOverlayFlags -band 2) -eq 0) { $textureKeys = @($textureKeys | Where-Object { $_ -cne 'overlayNormalTexture' }) }
            if (($row.sourceOverlayFlags -band 32) -ne 0) {
                $textureKeys += 'detailNormalTexture'
                foreach ($key in @('detailNormalIntensity','detailNormalTiling')) {
                    if (-not (Test-JsonNumber $row.$key) -or $row.$key -lt 0 -or $row.$key -gt [single]::MaxValue) { throw 'Invalid overlay detail parameter' }
                }
            }
            foreach ($key in @('sourceDirection','sourceUV')) {
                if ($row.$key -isnot [array] -or $row.$key.Count -ne 4) { throw 'Invalid source overlay vector' }
                foreach ($value in $row.$key) { if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt 1000000) { throw 'Invalid overlay vector component' } }
            }
        }
        if ($isSourceBg) {
            $textureKeys = @('diffuseTexture')
            if (($flags -band 1) -ne 0) { $textureKeys += 'normalTexture' }
            if (($flags -band 8) -ne 0) { $textureKeys += 'specularTexture' }
            if (($flags -band 16) -ne 0) { $textureKeys += 'reflectionTexture' }
            if (($flags -band 32768) -ne 0) {
                $textureKeys += 'detailNormalTexture'
                foreach ($key in @('detailNormalIntensity','detailNormalTiling')) {
                    if (-not (Test-JsonNumber $row.$key) -or $row.$key -lt 0 -or $row.$key -gt [single]::MaxValue) { throw 'Invalid source detail normal parameter' }
                }
            }
        }
        foreach ($textureKey in $textureKeys) {
        if ($row.$textureKey -isnot [string] -or [string]::IsNullOrWhiteSpace($row.$textureKey)) { throw "Invalid map texture: $textureKey" }
        $texture = [string]$row.$textureKey
        if ([IO.Path]::IsPathRooted($texture) -or $texture.Contains(':') -or
            '..' -in @($texture -split '[\\/]') -or [IO.Path]::GetExtension($texture) -cne '.dds') {
            throw "Map reflection texture must be a Resources-relative DDS: $texture"
        }
        $reflectionPath = [IO.Path]::GetFullPath((Join-Path $runtimeResourceRoot $texture))
        $resourcePrefix = [IO.Path]::GetFullPath($runtimeResourceRoot).TrimEnd('\') + '\'
        if (-not $reflectionPath.StartsWith($resourcePrefix, [StringComparison]::OrdinalIgnoreCase) -or
            -not [IO.File]::Exists($reflectionPath)) { throw "Map reflection texture is missing: $texture" }
        }
        if ($null -ne $row.PSObject.Properties['bakedLighting']) {
            $baked = $row.bakedLighting
            Assert-ExactJsonProperties $baked @('averageTexture','directionalTexture','colorSpace') 'Baked lighting'
            if ($baked.colorSpace -isnot [string] -or $baked.colorSpace -cnotin @('linear','srgb')) {
                throw 'Invalid baked lighting color space'
            }
            & $validateLightingTexture $baked.averageTexture
            & $validateLightingTexture $baked.directionalTexture
            [void]$bakedAssets.Add($row.assetId)
        }
        if ($null -ne $row.PSObject.Properties['emissive']) {
            $emissive = $row.emissive
            Assert-ExactJsonProperties $emissive @('texture','color','intensity','uvTiling','colorSpace','flicker') 'Source emissive'
            & $validateLightingTexture $emissive.texture
            if ($emissive.colorSpace -cnotin @('linear','srgb') -or
                $emissive.color -isnot [array] -or $emissive.color.Count -ne 4 -or
                $emissive.uvTiling -isnot [array] -or $emissive.uvTiling.Count -ne 2) {
                throw 'Invalid source emissive color space or vector'
            }
            Assert-ExactJsonProperties $emissive.flicker @('minimum','speed','phaseOffset') 'Source emissive flicker'
            foreach ($value in @($emissive.color) + @($emissive.intensity,$emissive.flicker.minimum,$emissive.flicker.speed)) {
                if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) {
                    throw 'Invalid source emissive nonnegative value'
                }
            }
            if (-not $isSourceBg -and $emissive.flicker.minimum -gt 1) { throw 'Source emissive minimum must be in [0,1]' }
            foreach ($value in $emissive.uvTiling) {
                if (-not (Test-JsonNumber $value) -or $value -le 0 -or $value -gt [single]::MaxValue) {
                    throw 'Invalid source emissive UV tiling'
                }
            }
            if (-not (Test-JsonNumber $emissive.flicker.phaseOffset) -or
                [Math]::Abs([double]$emissive.flicker.phaseOffset) -gt [single]::MaxValue) {
                throw 'Invalid source emissive phase offset'
            }
        }
        if ($null -ne $row.PSObject.Properties['environment']) {
            $environment = $row.environment
            Assert-ExactJsonProperties $environment @('cubeTexture','brdfTexture','color','rotation') 'Environment lighting'
            & $validateLightingTexture $environment.cubeTexture
            & $validateLightingTexture $environment.brdfTexture
            if ($environment.color -isnot [array] -or $environment.color.Count -ne 4 -or
                $environment.rotation -isnot [array] -or $environment.rotation.Count -ne 2) {
                throw 'Invalid environment color or rotation'
            }
            foreach ($value in $environment.color) {
                if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) { throw 'Invalid environment color value' }
            }
            foreach ($value in $environment.rotation) {
                if (-not (Test-JsonNumber $value) -or [Math]::Abs([double]$value) -gt 1) { throw 'Invalid environment rotation value' }
            }
            if ([Math]::Abs($environment.rotation[0]*$environment.rotation[0]+$environment.rotation[1]*$environment.rotation[1]-1) -gt 0.0001) {
                throw 'Environment rotation must have unit length'
            }
        }
        $modelPath = [string]$script:mapMaterialModels[$row.assetId]
        if (-not $modelNames.ContainsKey($modelPath)) {
            $modelNames[$modelPath] = (Read-WModelMaterialNames (Join-Path $runtimeResourceRoot $modelPath)).Names
        }
        if (-not $modelNames[$modelPath].ContainsKey($row.materialName) -or
            $modelNames[$modelPath][$row.materialName] -lt 1) {
            throw "Map material name does not exist in WModel: $($row.assetId)/$($row.materialName)"
        }
    }
    if ($null -ne $placementLighting) {
        if ($placementLighting.Value.Count -gt 65536) { throw 'Too many placement lighting rows' }
        $sourcePlacements = [Collections.Generic.Dictionary[string,string]]::new([StringComparer]::Ordinal)
        foreach ($placementLine in (Read-PlacementDocument $authoringPath)) {
            $parsed = Parse-PlacementRow $placementLine $authoringPath
            $sourcePlacements.Add($parsed.SourcePlacementId, $parsed.AssetId)
        }
        $lightingSources = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($lighting in $placementLighting.Value) {
            $lightingFields = @('sourcePlacementId','assetId','coordinateScale','coordinateBias','averageScale','directionalScale')
            $hasShadowScale = $null -ne $lighting.PSObject.Properties['shadowCoordinateScale']
            $hasShadowBias = $null -ne $lighting.PSObject.Properties['shadowCoordinateBias']
            if ($hasShadowScale -ne $hasShadowBias) { throw 'Incomplete placement shadow coordinates' }
            if ($hasShadowScale) {
                $lightingFields += @('shadowCoordinateScale','shadowCoordinateBias')
                if (-not $shadowAssets.Contains($lighting.assetId) -or $lighting.shadowCoordinateScale -isnot [array] -or $lighting.shadowCoordinateScale.Count -ne 2 -or $lighting.shadowCoordinateBias -isnot [array] -or $lighting.shadowCoordinateBias.Count -ne 2) { throw 'Invalid placement shadow asset or coordinates' }
                for ($axis=0; $axis -lt 2; $axis++) {
                    $scale=$lighting.shadowCoordinateScale[$axis]; $bias=$lighting.shadowCoordinateBias[$axis]
                    if (-not (Test-JsonNumber $scale) -or -not (Test-JsonNumber $bias) -or $scale -le 0 -or $bias -lt 0 -or $scale+$bias -gt 1.00001) { throw 'Placement shadow coordinates exceed atlas bounds' }
                }
            }
            Assert-ExactJsonProperties $lighting $lightingFields 'Placement lighting'
            if ($lighting.sourcePlacementId -isnot [string] -or [string]::IsNullOrWhiteSpace($lighting.sourcePlacementId) -or
                [Text.Encoding]::UTF8.GetByteCount($lighting.sourcePlacementId) -gt 512 -or
                $lighting.sourcePlacementId -match '[\x00-\x1F\x7F]' -or
                $lighting.assetId -isnot [string] -or -not $bakedAssets.Contains($lighting.assetId) -or
                -not $lightingSources.Add($lighting.sourcePlacementId) -or
                -not $sourcePlacements.ContainsKey($lighting.sourcePlacementId) -or
                $sourcePlacements[$lighting.sourcePlacementId] -cne $lighting.assetId) {
                throw 'Invalid, duplicate, dangling, or mismatched placement lighting identity'
            }
            foreach ($field in @('coordinateScale','coordinateBias','averageScale','directionalScale')) {
                $count = if ($field -cin @('coordinateScale','coordinateBias')) { 2 } else { 3 }
                if ($lighting.$field -isnot [array] -or $lighting.$field.Count -ne $count) { throw "Invalid lighting vector: $field" }
                foreach ($value in $lighting.$field) {
                    if (-not (Test-JsonNumber $value) -or $value -lt 0 -or $value -gt [single]::MaxValue) {
                        throw "Invalid lighting vector value: $field"
                    }
                }
            }
            for ($axis=0; $axis -lt 2; ++$axis) {
                if ($lighting.coordinateScale[$axis] -le 0 -or
                    $lighting.coordinateScale[$axis]+$lighting.coordinateBias[$axis] -gt 1.00001) {
                    throw 'Lightmap atlas coordinates exceed the texture'
                }
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-MapMaterialPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapMaterialsDeclared) {
        if (-not [IO.File]::Exists($authoringMaterialPath)) {
            throw "Declared map material authoring source is missing: $authoringMaterialPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.mapmaterials.json"
            Lines = Read-MapMaterialDocument $authoringMaterialPath
        })
    }
}


function Add-MapWaterPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapWaterDeclared) {
        if (-not [IO.File]::Exists($authoringWaterPath)) {
            throw "Declared map water authoring source is missing: $authoringWaterPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.mapwater.json"
            Lines = Read-MapWaterDocument $authoringWaterPath
        })
    }
}

if (-not [IO.File]::Exists($mapCatalogPath)) {
    throw "Map catalog is missing: $mapCatalogPath"
}
try { $mapCatalog = [IO.File]::ReadAllText(
        $mapCatalogPath, [Text.Encoding]::UTF8) | ConvertFrom-Json }
catch { throw "Map catalog JSON parse failed: $mapCatalogPath" }
$areaEntries = @($mapCatalog.areas | Where-Object { $_.id -eq $AreaId })
if (1 -ne $areaEntries.Count) {
    throw "Map catalog must declare Area exactly once: $AreaId"
}
$areaEntry = $areaEntries[0]
if ($Scope -eq 'Area') {
$sourceLightsProperty = $areaEntry.PSObject.Properties['sourceLights']
$runtimeLightsProperty = $areaEntry.PSObject.Properties['lights']
if (($null -eq $sourceLightsProperty) -ne ($null -eq $runtimeLightsProperty)) {
    throw "Map catalog light source/runtime declaration is incomplete: $AreaId"
}
$script:mapLightsDeclared = $null -ne $sourceLightsProperty
if ($AreaId -eq 'LV_LUT_HEARTRB_ED' -and -not $script:mapLightsDeclared) {
    throw "Valtan source/runtime map light declarations are required: $AreaId"
}
if ($script:mapLightsDeclared) {
    $expectedSourceLights = "Data/Maps/Authoring/$AreaId/$AreaId.maplights.json"
    $expectedRuntimeLights = "Client/Bin/DataFiles/Map/$AreaId.maplights.json"
    if ($areaEntry.sourceLights -isnot [string] -or
        $areaEntry.sourceLights -ne $expectedSourceLights -or
        $areaEntry.lights -isnot [string] -or
        $areaEntry.lights -ne $expectedRuntimeLights) {
        throw "Map catalog light paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringLightPath)) {
    throw "Map light source exists without a MapCatalog declaration: $AreaId"
}

$sourceEffectsProperty = $areaEntry.PSObject.Properties['sourceEffects']
$runtimeEffectsProperty = $areaEntry.PSObject.Properties['effects']
if (($null -eq $sourceEffectsProperty) -ne ($null -eq $runtimeEffectsProperty)) {
    throw "Map catalog Effect source/runtime declaration is incomplete: $AreaId"
}
$script:mapEffectsDeclared = $null -ne $sourceEffectsProperty
if ($AreaId -eq 'LV_LUT_HEARTRB_ED' -and -not $script:mapEffectsDeclared) {
    throw "Valtan source/runtime Map Effect declarations are required: $AreaId"
}
if ($script:mapEffectsDeclared) {
    $expectedSourceEffects = "Data/Maps/Authoring/$AreaId/$AreaId.mapeffects.json"
    $expectedRuntimeEffects = "Client/Bin/DataFiles/Map/$AreaId.mapeffects.json"
    if ($areaEntry.sourceEffects -isnot [string] -or
        $areaEntry.sourceEffects -ne $expectedSourceEffects -or
        $areaEntry.effects -isnot [string] -or
        $areaEntry.effects -ne $expectedRuntimeEffects) {
        throw "Map catalog Effect paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringEffectPath)) {
    throw "Map Effect source exists without a MapCatalog declaration: $AreaId"
}

$sourceMaterialsProperty = $areaEntry.PSObject.Properties['sourceMaterials']
$runtimeMaterialsProperty = $areaEntry.PSObject.Properties['materials']
if (($null -eq $sourceMaterialsProperty) -ne ($null -eq $runtimeMaterialsProperty)) {
    throw "Map catalog material source/runtime declaration is incomplete: $AreaId"
}
$script:mapMaterialsDeclared = $null -ne $sourceMaterialsProperty
if ($script:mapMaterialsDeclared) {
    $expectedSourceMaterials = "Data/Maps/Authoring/$AreaId/$AreaId.mapmaterials.json"
    $expectedRuntimeMaterials = "Client/Bin/DataFiles/Map/$AreaId.mapmaterials.json"
    if ($areaEntry.sourceMaterials -isnot [string] -or
        $areaEntry.sourceMaterials -cne $expectedSourceMaterials -or
        $areaEntry.materials -isnot [string] -or
        $areaEntry.materials -cne $expectedRuntimeMaterials) {
        throw "Map catalog material paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringMaterialPath)) {
    throw "Map material source exists without a MapCatalog declaration: $AreaId"
}

$sourceWaterProperty = $areaEntry.PSObject.Properties['sourceWater']
$runtimeWaterProperty = $areaEntry.PSObject.Properties['water']
if (($null -eq $sourceWaterProperty) -ne ($null -eq $runtimeWaterProperty)) {
    throw "Map catalog water source/runtime declaration is incomplete: $AreaId"
}
$script:mapWaterDeclared = $null -ne $sourceWaterProperty
if ($script:mapWaterDeclared) {
    $expectedSourceWater = "Data/Maps/Authoring/$AreaId/$AreaId.mapwater.json"
    $expectedRuntimeWater = "Client/Bin/DataFiles/Map/$AreaId.mapwater.json"
    if ($areaEntry.sourceWater -isnot [string] -or
        $areaEntry.sourceWater -ne $expectedSourceWater -or
        $areaEntry.water -isnot [string] -or
        $areaEntry.water -ne $expectedRuntimeWater) {
        throw "Map catalog water paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringWaterPath)) {
    throw "Map water source exists without a MapCatalog declaration: $AreaId"
}
}

$sourceSequencesProperty = $areaEntry.PSObject.Properties['sourceSequences']
$runtimeSequencesProperty = $areaEntry.PSObject.Properties['sequences']
if (($null -eq $sourceSequencesProperty) -ne ($null -eq $runtimeSequencesProperty)) {
    throw "Map catalog world sequence source/runtime declaration is incomplete: $AreaId"
}
$script:worldSequencesDeclared = $null -ne $sourceSequencesProperty
if ($script:worldSequencesDeclared) {
    $expectedSourceSequences = "Data/Maps/Authoring/$AreaId/$AreaId.worldsequences.json"
    $expectedRuntimeSequences = "Client/Bin/DataFiles/Map/$AreaId.worldsequences.json"
    if ($areaEntry.sourceSequences -isnot [string] -or
        $areaEntry.sourceSequences -ne $expectedSourceSequences -or
        $areaEntry.sequences -isnot [string] -or
        $areaEntry.sequences -ne $expectedRuntimeSequences) {
        throw "Map catalog world sequence paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringSequencePath)) {
    throw "World sequence source exists without a MapCatalog declaration: $AreaId"
}

if ($Scope -eq 'Area') {
$sourceCameraShotsProperty = $areaEntry.PSObject.Properties['sourceCameraShots']
$runtimeCameraShotsProperty = $areaEntry.PSObject.Properties['cameraShots']
if (($null -eq $sourceCameraShotsProperty) -ne ($null -eq $runtimeCameraShotsProperty)) {
    throw "Map catalog camera shot source/runtime declaration is incomplete: $AreaId"
}
$script:cameraShotsDeclared = $null -ne $sourceCameraShotsProperty
if ($script:cameraShotsDeclared) {
    $expectedSourceCameraShots = "Data/Maps/Authoring/$AreaId/$AreaId.camerashots.json"
    $expectedRuntimeCameraShots = "Client/Bin/DataFiles/Map/$AreaId.camerashots.json"
    if ($areaEntry.sourceCameraShots -isnot [string] -or
        $areaEntry.sourceCameraShots -ne $expectedSourceCameraShots -or
        $areaEntry.cameraShots -isnot [string] -or
        $areaEntry.cameraShots -ne $expectedRuntimeCameraShots) {
        throw "Map catalog camera shot paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringCameraShotPath)) {
    throw "Camera shot source exists without a MapCatalog declaration: $AreaId"
}
}

function Invoke-FileSetTransaction {
    param([object[]]$Files)
    $transactionId = [Guid]::NewGuid().ToString('N')
    $stagingRoot = [IO.Path]::GetFullPath(
        (Join-Path $runtimeRoot ".map-publish.staging.$AreaId.$transactionId"))
    if ([IO.Path]::GetDirectoryName($stagingRoot) -ne
        [IO.Path]::GetFullPath($runtimeRoot).TrimEnd('\')) {
        throw 'Map publish staging directory escapes the runtime root.'
    }
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    $entries = [Collections.Generic.List[object]]::new()
    $committed = $false
    try {
        foreach ($file in $Files) {
            $staged = Join-Path $stagingRoot $file.Name
            [IO.File]::WriteAllBytes($staged, $file.Bytes)
            $entries.Add([ordered]@{
                Staged = $staged
                Destination = Join-Path $runtimeRoot $file.Name
                Rollback = Join-Path $runtimeRoot ".$($file.Name).rollback.$transactionId"
                HadPrevious = $false
                Promoted = $false
            })
        }

        $promotedCount = 0
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Destination)) {
                [IO.File]::Move($entry.Destination, $entry.Rollback)
                $entry.HadPrevious = $true
            }
            [IO.File]::Move($entry.Staged, $entry.Destination)
            $entry.Promoted = $true
            ++$promotedCount
            if ($FailureAfterPromote -eq $promotedCount) {
                throw "Injected map publish failure after $promotedCount promotion(s)."
            }
        }
        $committed = $true
    }
    catch {
        $failure = $_
        for ($index = $entries.Count - 1; $index -ge 0; --$index) {
            $entry = $entries[$index]
            if ($entry.Promoted -and [IO.File]::Exists($entry.Destination)) {
                [IO.File]::Delete($entry.Destination)
            }
            if ($entry.HadPrevious -and [IO.File]::Exists($entry.Rollback)) {
                [IO.File]::Move($entry.Rollback, $entry.Destination)
            }
        }
        throw $failure
    }
    finally {
        if ([IO.Directory]::Exists($stagingRoot)) {
            try {
                Remove-Item -LiteralPath $stagingRoot -Recurse -Force
            }
            catch {
                Write-Warning "Map publish staging cleanup failed: $($_.Exception.Message)"
            }
        }
    }

    if ($committed) {
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Rollback)) {
                try {
                    [IO.File]::Delete($entry.Rollback)
                }
                catch {
                    Write-Warning (
                        "Map publish committed, but backup cleanup failed for " +
                        "'$($entry.Rollback)': $($_.Exception.Message)")
                }
            }
        }
    }
}

function Add-DeployPublishFiles {
    param([Collections.Generic.List[object]]$Files)
    $hasCatalog = [IO.File]::Exists($sourceDeployCatalogPath)
    $hasPlacements = [IO.File]::Exists($authoringDeployPath)
    if ($hasCatalog -ne $hasPlacements) {
        throw "Deploy authoring pair is incomplete for $AreaId"
    }
    if ($hasCatalog) {
        $pair = Read-DeployAuthoringPair
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.deployassets"
            Lines = $pair.CatalogLines
        })
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.deployplacements"
            Lines = $pair.PlacementLines
        })
    }
}

function Complete-MapPublish {
    param([object[]]$Files, [string]$CatalogType, [string]$RuntimeEntry,
        [int]$ShardCount = 0)
    # Every mode consumes the same normalized bytes. This stage performs no I/O.
    $expectedFiles = [Collections.Generic.List[object]]::new()
    $names = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($file in $Files) {
        if ($file.Name -notmatch '^[A-Za-z0-9_.-]+$' -or
            [IO.Path]::GetFileName($file.Name) -ne $file.Name -or
            -not $names.Add($file.Name)) {
            throw "Invalid or duplicate map output filename: $($file.Name)"
        }
        $expectedFiles.Add([pscustomobject]@{
            Name = $file.Name
            Bytes = $utf8.GetBytes((([string[]]$file.Lines -join "`n") + "`n"))
        })
    }
    if ($Mode -eq 'Check') {
        foreach ($file in $expectedFiles) {
            $destination = Join-Path $runtimeRoot $file.Name
            if (-not [IO.File]::Exists($destination)) {
                throw "Map runtime output is missing: $destination"
            }
            $actual = [IO.File]::ReadAllBytes($destination)
            if ($actual.Length -ne $file.Bytes.Length -or
                [Convert]::ToBase64String($actual) -cne
                [Convert]::ToBase64String($file.Bytes)) {
                throw "Map runtime output differs from authoring: $destination"
            }
        }
    }
    elseif ($Mode -eq 'Publish') {
        Invoke-FileSetTransaction $expectedFiles
    }
    $result = [ordered]@{
        AreaId = $AreaId
        Mode = $Mode
        Scope = $Scope
        CatalogType = $CatalogType
        PlacementCount = $authoringRows.Count
        FileCount = $expectedFiles.Count
        RuntimePath = $RuntimeEntry
    }
    if ($ShardCount -gt 0) { $result.ShardCount = $ShardCount }
    if ($Mode -eq 'Publish') { $result.Sha256 = Get-MapPublishSha256 $RuntimeEntry }
    [pscustomobject]$result
}

if ($Scope -eq 'WorldSequences') {
    if (-not $script:worldSequencesDeclared) { throw "Map catalog does not declare World Sequences: $AreaId" }
    $authoringRows = @()
    $files = [Collections.Generic.List[object]]::new()
    Add-WorldSequencePublishFile $files
    Complete-MapPublish $files 'world-sequences' $runtimeSequencePath
    return
}

if (-not [IO.File]::Exists($authoringPath)) {
    throw "Authoring placement is missing: $authoringPath"
}
if (-not [IO.Directory]::Exists($importRoot)) {
    throw "Imported map source is missing: $importRoot"
}
$authoringRows = @(Read-PlacementDocument $authoringPath)
$parsedAuthoringRows = [Collections.Generic.List[object]]::new()
$seenPlacementIds = [Collections.Generic.HashSet[uint64]]::new()
$seenSourcePlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($row in $authoringRows) {
    $parsed = Parse-PlacementRow $row $authoringPath
    if (-not $seenPlacementIds.Add($parsed.PlacementId)) {
        throw "Duplicate authoring placement ID: $($parsed.PlacementId)"
    }
    if (-not $seenSourcePlacementIds.Add($parsed.SourcePlacementId)) {
        throw "Duplicate authoring source placement ID: $($parsed.SourcePlacementId)"
    }
    $parsedAuthoringRows.Add($parsed)
}

if (-not [IO.File]::Exists($sourceShardSetPath)) {
    if (-not [IO.File]::Exists($sourceCatalogPath)) {
        throw "Imported map catalog is missing: $sourceCatalogPath"
    }
    $catalog = Read-MapAssetCatalog $sourceCatalogPath
    foreach ($parsed in $parsedAuthoringRows) {
        if (-not $catalog.AssetIds.Contains($parsed.AssetId)) {
            throw "Authoring placement references an asset outside the catalog: $($parsed.AssetId)"
        }
    }
    $lines = @("LOSTARK_MAP_PLACEMENTS 2 `"$AreaId`" $($authoringRows.Count)") + $authoringRows
    $files = [Collections.Generic.List[object]]::new()
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapassets"
        Lines = $catalog.Lines
    })
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapplacements"
        Lines = $lines
    })

    Add-DeployPublishFiles $files
    Add-MapLightPublishFile $files
    Add-MapEffectPublishFile $files
    Add-MapWaterPublishFile $files
    Add-MapMaterialPublishFile $files
    Add-WorldSequencePublishFile $files
    Add-CameraShotPublishFile $files

    Complete-MapPublish $files 'single' $runtimePath
    return
}

$mapSetLines = @([IO.File]::ReadAllLines($sourceShardSetPath, [Text.Encoding]::UTF8))
if ($mapSetLines.Count -lt 2 -or $mapSetLines.Count -gt 65) {
    throw "Shard set count is invalid: $sourceShardSetPath"
}
$mapSetHeader = [regex]::Match(
    $mapSetLines[0],
    '^LOSTARK_MAP_SHARD_SET\s+1\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
if (-not $mapSetHeader.Success -or
    $mapSetHeader.Groups['area'].Value -cne $AreaId -or
    [uint32]$mapSetHeader.Groups['count'].Value -ne ($mapSetLines.Count - 1)) {
    throw "Shard set header is invalid: $sourceShardSetPath"
}

$shards = [Collections.Generic.List[object]]::new()
$assetShards = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
$existingPlacementShard = @{}
$seenShardIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$seenBaselineSources = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($line in @($mapSetLines | Select-Object -Skip 1)) {
    $match = [regex]::Match(
        $line,
        '^"(?<id>[A-Za-z0-9_.-]+)"\s+"(?<catalog>[^"]+)"\s+"(?<placements>[^"]+)"\s+(?<assets>[0-9]+)\s+(?<count>[0-9]+)$')
    if (-not $match.Success) {
        throw "Invalid shard-set row: $line"
    }
    if (-not $seenShardIds.Add($match.Groups['id'].Value)) {
        throw "Duplicate shard ID: $($match.Groups['id'].Value)"
    }
    $shard = [pscustomobject]@{
        Id = $match.Groups['id'].Value
        CatalogName = $match.Groups['catalog'].Value
        PlacementName = $match.Groups['placements'].Value
        AssetCount = [uint32]$match.Groups['assets'].Value
        PlacementCount = [uint32]$match.Groups['count'].Value
        Rows = [Collections.Generic.List[string]]::new()
        CatalogLines = @()
    }
	$catalogPath = Assert-ImportedLeaf $shard.CatalogName '.mapassets'
	$placementPath = Assert-ImportedLeaf $shard.PlacementName '.mapplacements'
    if (-not [IO.File]::Exists($catalogPath) -or
        -not [IO.File]::Exists($placementPath)) {
        throw "Shard source is missing: $($shard.Id)"
    }
    $catalog = Read-MapAssetCatalog $catalogPath
    $shard.CatalogLines = $catalog.Lines
    if ($catalog.AssetIds.Count -ne $shard.AssetCount) {
        throw "Shard asset count mismatch: $($shard.Id)"
    }
    foreach ($assetId in $catalog.AssetIds) {
        if (-not $assetShards.ContainsKey($assetId)) {
            $assetShards[$assetId] = [Collections.Generic.List[string]]::new()
        }
        $assetShards[$assetId].Add($shard.Id)
    }
    $baselineRows = @(Read-PlacementDocument $placementPath)
    if ($baselineRows.Count -ne $shard.PlacementCount) {
        throw "Shard placement count mismatch: $($shard.Id)"
    }
    foreach ($existingRow in $baselineRows) {
        $parsed = Parse-PlacementRow $existingRow $placementPath
        if ($existingPlacementShard.ContainsKey($parsed.PlacementId)) {
            throw "Duplicate placement ID across current shards: $($parsed.PlacementId)"
        }
        if (-not $seenBaselineSources.Add($parsed.SourcePlacementId)) {
            throw "Duplicate source placement ID across current shards: $($parsed.SourcePlacementId)"
        }
        if (-not $catalog.AssetIds.Contains($parsed.AssetId)) {
            throw "Baseline placement references an asset outside its shard: $($parsed.AssetId)"
        }
        $existingPlacementShard[$parsed.PlacementId] = $shard.Id
    }
    $shards.Add($shard)
}

$shardsById = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
foreach ($shard in $shards) { $shardsById[$shard.Id] = $shard }
foreach ($parsed in $parsedAuthoringRows) {
    if (-not $assetShards.ContainsKey($parsed.AssetId)) {
        throw "Authoring placement references an asset outside the shard set: $($parsed.AssetId)"
    }
    if ($existingPlacementShard.ContainsKey($parsed.PlacementId)) {
        $targetShardId = $existingPlacementShard[$parsed.PlacementId]
        if ($targetShardId -notin @($assetShards[$parsed.AssetId])) {
            throw "Existing placement changed to an asset unavailable in its shard: $($parsed.PlacementId)"
        }
    }
    else {
        $candidates = @($assetShards[$parsed.AssetId] | Sort-Object)
        $targetShardId = $candidates[[int]($parsed.PlacementId % [uint64]$candidates.Count)]
    }
    $shardsById[$targetShardId].Rows.Add($parsed.Row)
}

$files = [Collections.Generic.List[object]]::new()
$newMapSetLines = [Collections.Generic.List[string]]::new()
$newMapSetLines.Add("LOSTARK_MAP_SHARD_SET 1 `"$AreaId`" $($shards.Count)")
foreach ($shard in $shards) {
    $placementLines = [Collections.Generic.List[string]]::new()
    $placementLines.Add("LOSTARK_MAP_PLACEMENTS 2 `"$AreaId`" $($shard.Rows.Count)")
    foreach ($row in $shard.Rows) { $placementLines.Add($row) }
    $files.Add([pscustomobject]@{
        Name = $shard.PlacementName
        Lines = $placementLines
    })
    $files.Add([pscustomobject]@{
        Name = $shard.CatalogName
        Lines = $shard.CatalogLines
    })
    $newMapSetLines.Add(
        "`"$($shard.Id)`" `"$($shard.CatalogName)`" `"$($shard.PlacementName)`" $($shard.AssetCount) $($shard.Rows.Count)")
}
$files.Add([pscustomobject]@{
    Name = "$AreaId.mapset"
    Lines = $newMapSetLines
})
Add-DeployPublishFiles $files
Add-MapLightPublishFile $files
Add-MapEffectPublishFile $files
Add-MapWaterPublishFile $files
Add-MapMaterialPublishFile $files
Add-WorldSequencePublishFile $files
Add-CameraShotPublishFile $files
Complete-MapPublish $files 'shard-set' $shardSetPath $shards.Count

```

## Tools/MapPipeline/test_world_sequence_authoring_contract.py 전체 반영 코드

```python
from __future__ import annotations

import re
import copy
import json
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class WorldSequenceAuthoringContractTests(unittest.TestCase):
    """Fast source/project integration guards; the Product build compiles behavior."""

    def setUp(self) -> None:
        self.document_h = read("Client/Public/WorldSequenceDocument.h")
        self.document_cpp = read("Client/Private/WorldSequenceDocument.cpp")
        self.panel_h = read("Client/Public/WorldSequenceToolPanel.h")
        self.panel_cpp = read("Client/Private/WorldSequenceToolPanel.cpp")
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")

    def test_walkable_surface_accepts_fixed_roulette_and_rejects_unsupported_geometry(self) -> None:
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.8")
        instance["walkableSurface"] = {"radiusM": 2.5, "localHeightM": .026313}
        source["instances"] = [instance]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == instance["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", []) if row.get("sequenceInstanceId") == instance["instanceId"]]
        keys = source["templates"][0]["tracks"][0]["keys"]
        source["templates"][0]["tracks"][0]["keys"] = [keys[0], keys[-1]]
        cases = [("valid", source, True)]
        for name, mutate in (
            ("negative_radius", lambda d: d["instances"][0]["walkableSurface"].update(radiusM=-1)),
            ("boolean_height", lambda d: d["instances"][0]["walkableSurface"].update(localHeightM=True)),
            ("unknown_field", lambda d: d["instances"][0]["walkableSurface"].update(height=0)),
            ("moving_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(positionOffset=[0, 1, 0])),
            ("tilted_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(rotationQuaternion=[1, 0, 0, 0])),
            ("nonuniform_scale", lambda d: d["templates"][0]["tracks"][0]["keys"][0].update(scaleMultiplier=[1, 1, 2])),
        ):
            document = copy.deepcopy(source); mutate(document); cases.append((name, document, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = [re.search(r"(?ms)^function " + name + r" \{.*?^\}", publisher).group(0)
                       for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument")]
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_v3_object_publisher_accepts_source_and_rejects_invalid_resources(self) -> None:
        """Exercise the actual publisher function on a preserved source plus one model state."""
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        # Keep the real placed curtain state to cover compatibility without scanning every roulette key per mutant.
        curtain = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.curtain_drop")
        source["instances"] = [curtain]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == curtain["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", [])
                                     if row.get("sequenceInstanceId") == curtain["instanceId"]]
        source["formatVersion"] = 3
        source.setdefault("objectResources", []).append({
            "objectId": "test.world.object", "displayName": "Test object", "modelAssetId": "Map/Test/test.wmodel",
            "diffuseTextureAssetId": "", "modelPreScale": 0.01, "animated": False,
            "scale": [1, 1, 1], "sequenceInstanceId": "",
        })
        source["templates"].append({
            "sequenceId": "test.world.template", "displayName": "Test state", "category": "Object",
            "durationMs": 1000, "interpolation": "LINEAR", "animationTracks": [],
            "objectMotion": {"velocity": [0, 1, 0], "acceleration": [0, -2, 0], "angularVelocityDegrees": [0, 180, 0],
                             "revolutionDegreesPerSecond": [0, 0, 0], "revolutionOffset": [0, 0, 0],
                             "count": 2, "intervalMs": 100, "spreadDegrees": 30, "seed": 7},
            "tracks": [{"slotId": "object", "keys": [
                {"timeMs": time, "positionOffset": [0, 0, 0], "rotationQuaternion": [0, 0, 0, 1],
                 "scaleMultiplier": [1, 1, 1], "visible": True} for time in (0, 1000)]}],
        })
        source["instances"].append({
            "instanceId": "test.world.instance", "templateId": "test.world.template", "enabled": True,
            "startDelayMs": 0, "playbackSpeed": 1, "anchorKind": "PLAYER", "position": [1, 2, 3],
            "bindings": [{"slotId": "object", "targetKind": "OBJECT_RESOURCE", "targetId": "test.world.object"}],
        })
        cases = [("valid", source, True)]
        with_default = copy.deepcopy(source)
        with_default["objectResources"][-1]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_valid", with_default, True))
        grouped = copy.deepcopy(source)
        grouped["instances"][-1]["anchorKind"] = "WORLD"
        grouped["objectResources"].append({
            "objectId": "test.world.group", "displayName": "Combined motions", "modelAssetId": "",
            "modelPreScale": .01, "animated": False, "scale": [1, 1, 1], "anchorKind": "WORLD",
            "motionInstanceIds": ["test.world.instance"],
        })
        cases.append(("object_group_valid", grouped, True))
        empty_materials = copy.deepcopy(grouped)
        empty_materials["objectResources"][-1]["mapMaterialBindings"] = []
        cases.append(("object_group_empty_material_bindings", empty_materials, True))
        for name, mutate in (
            ("missing", lambda d: d["objectResources"][-1].update(motionInstanceIds=["missing.motion"])),
            ("duplicate", lambda d: d["objectResources"][-1].update(motionInstanceIds=["test.world.instance"] * 2)),
            ("empty", lambda d: d["objectResources"][-1].update(motionInstanceIds=[])),
            ("model", lambda d: d["objectResources"][-1].update(modelAssetId="Map/Test/test.wmodel")),
            ("default", lambda d: d["objectResources"][-1].update(defaultMotionInstanceId="test.world.instance")),
            ("loop", lambda d: d["instances"][-1].update(motionEnd="LOOP")),
            ("player", lambda d: d["instances"][-1].update(anchorKind="PLAYER")),
            ("nested", lambda d: d["instances"][-1]["bindings"][0].update(targetId="test.world.group")),
        ):
            candidate = copy.deepcopy(grouped)
            mutate(candidate)
            cases.append(("object_group_invalid_" + name, candidate, False))
        with_effect = copy.deepcopy(source)
        with_effect["templates"][-1]["effectTracks"] = [{
            "effectTrackId": "effect.smoke", "slotId": "object", "resourceKind": "GROUP",
            "resourceId": "boss.kouku.ball.smoke", "timing": "MOTION_END", "startMs": 0,
            "durationMs": 2000, "positionOffset": [0, -2.3, -1.35],
            "rotationDegrees": [0, 0, 0], "scale": [1, 1, 1],
        }]
        cases.append(("effect_motion_end", with_effect, True))
        ten_balls = copy.deepcopy(with_effect)
        ten_balls["templates"][-1]["objectMotion"].update(count=10, intervalMs=300, spreadDegrees=360)
        cases.append(("effect_ten_complete_emissions", ten_balls, True))
        at_time = copy.deepcopy(with_effect)
        at_time["templates"][-1]["effectTracks"][0].update(timing="TIME", startMs=1000)
        cases.append(("effect_time_includes_model_end", at_time, True))
        for name, fields in (
            ("unknown_timing", {"timing": "FINISH"}), ("end_offset", {"startMs": 1}),
            ("missing_slot", {"slotId": "absent"}), ("unknown_kind", {"resourceKind": "WORLD"}),
            ("path_identity", {"resourceId": "../smoke"}), ("zero_window", {"durationMs": 0}),
            ("boolean_time", {"startMs": True}), ("time_past_end", {"timing": "TIME", "startMs": 1001}),
            ("negative_scale", {"scale": [-1, 1, 1]}), ("excessive_tail", {"durationMs": 600000}),
        ):
            invalid = copy.deepcopy(with_effect)
            invalid["templates"][-1]["effectTracks"][0].update(fields)
            cases.append(("effect_invalid_" + name, invalid, False))
        duplicate_effect = copy.deepcopy(with_effect)
        duplicate_effect["templates"][-1]["effectTracks"] *= 2
        cases.append(("effect_duplicate_track_id", duplicate_effect, False))
        for name, default_id in (("missing", "missing.instance"), ("foreign", curtain["instanceId"]),
                                 ("number", 4), ("null", None), ("empty", "")):
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["defaultMotionInstanceId"] = default_id
            cases.append(("default_motion_" + name, candidate, name == "empty"))
        disabled_default = copy.deepcopy(with_default)
        disabled_default["instances"][-1]["enabled"] = False
        cases.append(("default_motion_disabled", disabled_default, False))
        alias_default = copy.deepcopy(source)
        alias_default["objectResources"][0]["defaultMotionInstanceId"] = curtain["instanceId"]
        cases.append(("default_motion_alias_valid", alias_default, True))
        alias_default_other = copy.deepcopy(alias_default)
        alias_default_other["objectResources"][0]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_alias_foreign", alias_default_other, False))
        for end in ("STOP", "HOLD", "LOOP"):
            completed = copy.deepcopy(source)
            completed["instances"][-1].update(motionEnd=end, nextMotionId="")
            cases.append(("motion_end_" + end, completed, True))
        for name, fields in (
            ("unknown", {"motionEnd": "FINISH"}), ("lowercase", {"motionEnd": "hold"}),
            ("number", {"motionEnd": 1}), ("null", {"motionEnd": None}),
            ("boolean", {"motionEnd": True}), ("next_missing", {"motionEnd": "NEXT"}),
            ("next_empty", {"motionEnd": "NEXT", "nextMotionId": ""}),
            ("stop_target", {"motionEnd": "STOP", "nextMotionId": "test.world.instance"}),
            ("implicit_stop_target", {"nextMotionId": "test.world.instance"}),
            ("target_number", {"nextMotionId": 7}), ("target_null", {"nextMotionId": None}),
        ):
            invalid = copy.deepcopy(source)
            invalid["instances"][-1].update(fields)
            cases.append(("motion_end_invalid_" + name, invalid, False))
        placed_completion = copy.deepcopy(source)
        placed_completion["instances"][0]["motionEnd"] = "HOLD"
        cases.append(("motion_end_placed_target", placed_completion, False))
        next_source = copy.deepcopy(source)
        next_source["templates"][-1]["objectMotion"]["count"] = 1
        next_state = copy.deepcopy(next_source["instances"][-1])
        next_state.update(instanceId="test.world.next", motionEnd="HOLD")
        next_source["instances"][-1].update(motionEnd="NEXT", nextMotionId=next_state["instanceId"])
        next_source["instances"].append(next_state)
        cases.append(("motion_next_valid", next_source, True))
        for name, mutate in (
            ("unknown", lambda d: d["instances"][-2].update(nextMotionId="missing.motion")),
            ("case_mismatch", lambda d: d["instances"][-2].update(nextMotionId="TEST.WORLD.NEXT")),
            ("disabled", lambda d: d["instances"][-1].update(enabled=False)),
            ("self", lambda d: d["instances"][-2].update(nextMotionId="test.world.instance")),
            ("cycle", lambda d: d["instances"][-1].update(motionEnd="NEXT", nextMotionId="test.world.instance")),
            ("multiple_emissions", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
        ):
            invalid = copy.deepcopy(next_source)
            mutate(invalid)
            cases.append(("motion_next_invalid_" + name, invalid, False))
        other_resource = copy.deepcopy(next_source)
        resource_copy = copy.deepcopy(other_resource["objectResources"][-1])
        resource_copy["objectId"] = "test.other.object"
        other_resource["objectResources"].append(resource_copy)
        other_resource["instances"][-1]["bindings"][0]["targetId"] = resource_copy["objectId"]
        cases.append(("motion_next_different_resource", other_resource, False))
        other_slot = copy.deepcopy(next_source)
        template_copy = copy.deepcopy(other_slot["templates"][-1])
        template_copy["sequenceId"] = "test.other.slot.template"
        template_copy["tracks"][0]["slotId"] = "another.slot"
        other_slot["templates"].append(template_copy)
        other_slot["instances"][-1].update(templateId=template_copy["sequenceId"])
        other_slot["instances"][-1]["bindings"][0]["slotId"] = "another.slot"
        cases.append(("motion_next_different_slot", other_slot, False))
        for depth in (32, 33):
            chain = copy.deepcopy(next_source)
            prototype = chain["instances"][-1]
            chain["instances"] = [chain["instances"][0]]
            for index in range(depth + 1):
                state = copy.deepcopy(prototype)
                state.update(instanceId=f"test.motion.{index}", motionEnd="NEXT" if index < depth else "STOP")
                if index < depth:
                    state["nextMotionId"] = f"test.motion.{index + 1}"
                chain["instances"].append(state)
            cases.append((f"motion_next_depth_{depth}", chain, depth == 32))
        animation_source = copy.deepcopy(source)
        animation_source["objectResources"][-1]["animated"] = True
        animation_source["templates"][-1]["animationTracks"] = [{
            "slotId": "object", "clipName": "mn_rhoc_00_sk.ao_idle_normal_1",
            "playbackRate": 1, "loop": False, "holdLastFrame": True,
        }]
        cases.append(("animation_label_absent", animation_source, True))
        for name, label, valid in (
            ("empty", "", True), ("korean", "카드_등장", True),
            ("maximum", "a" * 128, True), ("bytes_over", "한" * 43, False),
            ("long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
            ("control", "card\nlabel", False),
        ):
            labeled = copy.deepcopy(animation_source)
            labeled["templates"][-1]["animationTracks"][0]["displayName"] = label
            cases.append(("animation_label_" + name, labeled, valid))
        for anchor in ("WORLD", "PLAYER"):
            anchored = copy.deepcopy(source)
            anchored["objectResources"][-1]["anchorKind"] = anchor
            cases.append(("resource_anchor_" + anchor, anchored, True))
        boss_source = copy.deepcopy(source)
        boss_source["objectResources"][-1].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="bip001_R_Hand")
        boss_source["instances"][-1]["anchorKind"] = "BOSS"
        cases.append(("boss_hand_anchor", boss_source, True))
        for name, bone, valid in (
            ("root", "", True), ("maximum", "a" * 128, True), ("korean", "오른손", True),
            ("bytes_over", "한" * 43, False), ("too_long", "a" * 129, False),
            ("control", "hand\n", False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBone"] = bone
            cases.append(("boss_bone_" + name, candidate, valid))
        boss_root = copy.deepcopy(boss_source)
        del boss_root["objectResources"][-1]["anchorBone"]
        cases.append(("boss_bone_absent_root", boss_root, True))
        for name, boss_id, valid in (
            ("maximum", "a" * 128, True), ("empty", "", False),
            ("space", "boss kouku", False), ("path", "../kouku", False),
            ("too_long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBossArchetypeId"] = boss_id
            cases.append(("boss_archetype_" + name, candidate, valid))
        missing_boss = copy.deepcopy(boss_source)
        del missing_boss["objectResources"][-1]["anchorBossArchetypeId"]
        cases.append(("boss_archetype_absent", missing_boss, False))
        for anchor in ("WORLD", "PLAYER"):
            empty_fields = copy.deepcopy(source)
            empty_fields["objectResources"][-1].update(
                anchorKind=anchor, anchorBossArchetypeId="", anchorBone="")
            cases.append(("nonboss_empty_fields_" + anchor, empty_fields, True))
            for field, value in (("anchorBossArchetypeId", "BOSS_KOUKU"), ("anchorBone", "hand"),
                                 ("anchorBossArchetypeId", None), ("anchorBone", None)):
                candidate = copy.deepcopy(empty_fields)
                candidate["objectResources"][-1][field] = value
                cases.append((f"nonboss_{anchor}_{field}_{value}", candidate, False))
            candidate = copy.deepcopy(boss_source)
            candidate["instances"][-1]["anchorKind"] = anchor
            cases.append(("boss_resource_instance_mismatch_" + anchor, candidate, False))
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["anchorKind"] = anchor
            candidate["instances"][-1]["anchorKind"] = "BOSS"
            cases.append(("boss_instance_resource_mismatch_" + anchor, candidate, False))
        implicit_instance = copy.deepcopy(boss_source)
        del implicit_instance["instances"][-1]["anchorKind"]
        cases.append(("boss_resource_implicit_world_instance", implicit_instance, False))
        multiple_boss_bindings = copy.deepcopy(boss_source)
        second_object = copy.deepcopy(multiple_boss_bindings["objectResources"][-1])
        second_object["objectId"] = "test.second.boss.object"
        multiple_boss_bindings["objectResources"].append(second_object)
        second_track = copy.deepcopy(multiple_boss_bindings["templates"][-1]["tracks"][0])
        second_track["slotId"] = "second.object"
        multiple_boss_bindings["templates"][-1]["tracks"].append(second_track)
        multiple_boss_bindings["instances"][-1]["bindings"].append({
            "slotId": "second.object", "targetKind": "OBJECT_RESOURCE", "targetId": second_object["objectId"]})
        cases.append(("boss_anchor_multiple_resources", multiple_boss_bindings, False))
        placed_boss = copy.deepcopy(source)
        placed_boss["instances"][0]["anchorKind"] = "BOSS"
        cases.append(("boss_anchor_placed_instance", placed_boss, False))
        alias_boss = copy.deepcopy(source)
        alias_boss["objectResources"][0].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="")
        cases.append(("boss_anchor_placed_alias", alias_boss, False))
        for name, mutate in (
            ("bad_count", lambda d: d["templates"][-1]["objectMotion"].update(count=0)),
            ("spawn_after_lifetime", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=1000)),
            ("unknown_resource", lambda d: d["instances"][-1]["bindings"][0].update(targetId="missing.object")),
            ("path_escape", lambda d: d["objectResources"][-1].update(modelAssetId="Map/../test.wmodel")),
            ("unknown_property", lambda d: d["objectResources"][-1].update(velocty=1)),
            ("unknown_alias", lambda d: d["objectResources"][-1].update(modelAssetId="", sequenceInstanceId="missing.instance")),
            ("invalid_anchor", lambda d: d["instances"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor", lambda d: d["objectResources"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor_type", lambda d: d["objectResources"][-1].update(anchorKind=0)),
            ("placed_alias_character_anchor", lambda d: d["objectResources"][0].update(anchorKind="PLAYER")),
        ):
            invalid = copy.deepcopy(source)
            mutate(invalid)
            cases.append((name, invalid, False))
        # Authored emission rows replace the seeded emitter: count follows the rows, interval/spread stay 0,
        # every delay stays inside the lifetime, and NEXT keeps requiring a single emission.
        emitted = copy.deepcopy(source)
        emitted["templates"][-1]["objectMotion"].update(count=3, intervalMs=0, spreadDegrees=0, emissions=[
            {"positionOffset": [0, 0, 0], "yawDegrees": 0, "startDelayMs": 0},
            {"positionOffset": [4, 0, 0], "yawDegrees": 90, "startDelayMs": 200},
            {"positionOffset": [-4, 0, 0], "yawDegrees": -90, "startDelayMs": 400}])
        cases.append(("authored_emissions_valid", emitted, True))
        for name, mutate in (
            ("count_mismatch", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
            ("interval_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=100)),
            ("spread_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(spreadDegrees=30)),
            ("delay_after_lifetime", lambda d: d["templates"][-1]["objectMotion"]["emissions"][-1].update(startDelayMs=1000)),
            ("unknown_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].update(phase=1)),
            ("missing_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].pop("yawDegrees")),
            ("fractional_delay", lambda d: d["templates"][-1]["objectMotion"]["emissions"][1].update(startDelayMs=200.5)),
            ("empty_rows", lambda d: d["templates"][-1]["objectMotion"].update(count=1, emissions=[])),
        ):
            invalid = copy.deepcopy(emitted)
            mutate(invalid)
            cases.append(("authored_emissions_invalid_" + name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match)
            definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            malformed_utf8 = json.dumps(animation_source).replace(
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1"',
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1", "displayName": "LABEL_BYTE"',
            ).encode("utf-8").replace(b"LABEL_BYTE", b"\xc0\x80")
            (folder / "animation_label_invalid_utf8.json").write_bytes(malformed_utf8)
            cases.append(("animation_label_invalid_utf8", animation_source, False))
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"
            script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_world_sequences_only_scope_preserves_other_layers_and_rolls_back(self):
        area = "LV_LUT_MIDNIGHTC_ED"
        source = json.loads(read(f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"))
        card = next(row for row in source["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
        source["instances"] = [card]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == card["templateId"]]
        source["objectResources"] = [row for row in source["objectResources"] if row["objectId"] == "world.object.kouku.card"]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog = root / "Data/Maps/MapCatalog.json"
            catalog.parent.mkdir(parents=True)
            # Deliberately incomplete unrelated declarations must not block this scope.
            catalog.write_text(json.dumps({"areas": [{"id": area, "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json", "sourceLights": "unrelated.invalid"}]}), encoding="utf-8")
            authoring = root / f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"
            authoring.parent.mkdir(parents=True)
            authoring.write_text(json.dumps(source), encoding="utf-8")
            runtime = root / "Client/Bin/DataFiles/Map"
            runtime.mkdir(parents=True)
            preserved = {}
            for suffix in ("maplights.json", "mapplacements", "mapmaterials.json", "camerashots.json"):
                path = runtime / f"{area}.{suffix}"; path.write_bytes(b"preserved unrelated runtime\r\n"); preserved[path] = path.read_bytes()
            output = runtime / f"{area}.worldsequences.json"
            output.write_bytes(b"previous sequence runtime")
            def run(mode, fail=0):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", "WorldSequences", "-Mode", mode, "-FailureAfterPromote", str(fail)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, run("Validate").returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            failed = run("Publish", 1)
            self.assertNotEqual(0, failed.returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            applied = run("Publish")
            self.assertEqual(0, applied.returncode, applied.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("Check").returncode)
            self.assertTrue(all(path.read_bytes() == before for path, before in preserved.items()))
            prior = output.read_bytes()
            source["objectResources"][0]["defaultMotionInstanceId"] = "missing.motion"
            authoring.write_text(json.dumps(source), encoding="utf-8")
            self.assertNotEqual(0, run("Publish").returncode)
            self.assertEqual(prior, output.read_bytes())

    def test_sequence_publish_joins_installed_or_staged_map_and_deploy_targets(self):
        area = "TEST_SEQUENCE_JOIN"
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if len(row["bindings"]) == 1 and row["bindings"][0]["targetKind"] == "MAP_PLACEMENT")
        template = next(row for row in source["templates"] if row["sequenceId"] == instance["templateId"])
        instance.pop("walkableSurface", None)
        instance["bindings"][0]["targetId"] = "2"
        source.update(areaId=area, instances=[instance], templates=[template], objectResources=[])
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            authoring = root / f"Data/Maps/Authoring/{area}"
            imported = root / f"Data/Maps/Imported/{area}"
            runtime = root / "Client/Bin/DataFiles/Map"
            for directory in (authoring, imported, runtime):
                directory.mkdir(parents=True)
            catalog = root / "Data/Maps/MapCatalog.json"
            entry = {"id": area, "catalogType": "single", "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                     "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json"}
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            sequence = authoring / f"{area}.worldsequences.json"
            sequence.write_text(json.dumps(source), encoding="utf-8")
            output = runtime / sequence.name
            output.write_bytes(b"previous sequence runtime")
            asset = lambda name: f'"{name}" "Fixture" "Map/{name}.wmodel" "Prototype_{name}" 1 1 1 Origin'
            placement = lambda number, name: f'{number} "editor.{number}" "fixture" "editor" "{name}" 0 0 0 0 0 0 1 1 1 1 1'
            def write_pair(folder, names, ids):
                (folder / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 1 "{area}" {len(names)}\n' + "\n".join(map(asset, names)) + "\n", encoding="utf-8")
                (folder / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" {len(ids)}\n' + "\n".join(placement(number, name) for number, name in ids) + "\n", encoding="utf-8")
            write_pair(runtime, ["OLD"], [(1, "OLD")])
            write_pair(imported, ["OLD", "NEW"], [(1, "OLD"), (2, "NEW")])
            (authoring / f"{area}.mapplacements").write_bytes((imported / f"{area}.mapplacements").read_bytes())
            def run(scope, mode="Publish"):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", scope, "-Mode", mode], capture_output=True, text=True, timeout=30)
            snapshot = lambda: {path.name: path.read_bytes() for path in runtime.iterdir() if path.is_file()}
            def reject(expected):
                before = snapshot()
                result = run("WorldSequences")
                self.assertNotEqual(0, result.returncode)
                self.assertIn(expected, result.stderr)
                self.assertEqual(before, snapshot(), "A rejected join must preserve every installed file")
            reject("Invalid Map binding")
            before = snapshot()
            result = run("Area", "Validate")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(before, snapshot())
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            write_pair(runtime, ["OLD"], [(1, "OLD"), (2, "NEW")])
            reject("Invalid world sequence Map target placement/asset")
            # Sky is specifically excluded by native Collect_Placements.
            sky = asset("NEW") + ' "group" "Group" "evidence" Sky Back 1 1 0 0 1 0 1 50 1 1 1 1'
            (runtime / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 3 "{area}" 1\n{sky}\n', encoding="utf-8")
            (runtime / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" 1\n{placement(2, "NEW")}\n', encoding="utf-8")
            reject("Invalid Map binding")
            self.assertEqual(0, run("Area").returncode)
            # Single-scope joins must also consume the installed shard set.
            (runtime / f"{area}.mapset").write_text(f'LOSTARK_MAP_SHARD_SET 1 "{area}" 1\n"one" "{area}.mapassets" "{area}.mapplacements" 2 2\n', encoding="utf-8")
            entry["catalogType"] = "shard-set"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            self.assertEqual(0, run("WorldSequences", "Validate").returncode)
            entry["catalogType"] = "single"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            # Deploy roles are not the complete native clip list: an arbitrary
            # named animation remains valid here and is checked by CModel later.
            instance["bindings"][0].update(targetKind="DEPLOY_PLACEMENT", targetId="11")
            template["tracks"] = []
            template["animationTracks"] = [{"slotId": instance["bindings"][0]["slotId"], "clipName": "additional.native.clip",
                "playbackRate": 1, "loop": False, "holdLastFrame": True}]
            sequence.write_text(json.dumps(source), encoding="utf-8")
            deploy_asset = '"ANIMATED" ANIM "Fixture" "Map/animated.wmodel" "Prototype_Animated" "" "" 1 0 "fixture" "on" "off"'
            deploy_catalog = f'LOSTARK_DEPLOY_PROP_CATALOG 3 "{area}" 1\n{deploy_asset}\n'
            (imported / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            deploy_row = '11 0 0 "editor.11" "ANIMATED" 0 0 0 0 0 0 1 1 0 0 0 PROJECT_AUTHORED'
            deploy_source = f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 1\n{deploy_row}\n'
            (authoring / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployplacements").write_text(f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 0\n', encoding="utf-8")
            reject("Invalid animated Deploy binding")
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            (runtime / f"{area}.deployplacements").write_text(deploy_source.replace('"ANIMATED"', '"ABSENT"'), encoding="utf-8")
            reject("Deploy placement identity is invalid")
            (runtime / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog.replace(' ANIM ', ' STATIC ').replace('"on" "off"', '"" ""'), encoding="utf-8")
            reject("Invalid animated Deploy binding")

    def test_camera_pattern_authoring_fields_validate_without_changing_legacy_shots(self):
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json"))
        source["shots"] = [source["shots"][0]]
        cases = [("legacy", copy.deepcopy(source), True)]
        shot = source["shots"][0]
        shot.update(displayName="패턴 카메라", defaultHoldMs=3000, transitionEasing="LINEAR", activation="PATTERN_ONLY")
        cases.append(("pattern", copy.deepcopy(source), True))
        for name, fields in (
            ("name_number", {"displayName": 7}), ("name_empty", {"displayName": ""}),
            ("name_too_long", {"displayName": "가" * 43}), ("name_null", {"displayName": None}),
            ("bad_hold", {"defaultHoldMs": -1}), ("fraction_hold", {"defaultHoldMs": 1.5}),
            ("long_entry_hold", {"defaultHoldMs": 600000, "blendInMs": 1}),
            ("zero_entry_hold", {"defaultHoldMs": 0, "blendInMs": 0}),
            ("bad_easing", {"transitionEasing": "HOLD"}), ("easing_type", {"transitionEasing": True}),
            ("bad_activation", {"activation": "pattern_only"}), ("activation_null", {"activation": None}),
        ):
            invalid = copy.deepcopy(source); invalid["shots"][0].update(fields)
            cases.append((name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-CameraShotDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match); definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-CameraShotDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual({name: valid for name, _, valid in cases}, {r["name"]: r["valid"] for r in json.loads(result.stdout)})

    def test_client_project_registers_each_source_once(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        includes = [
            item.attrib.get("Include", "")
            for kind in ("ClInclude", "ClCompile")
            for item in project.findall(f".//msb:{kind}", namespace)
        ]
        expected = (
            r"..\Public\WorldSequenceDocument.h",
            r"..\Public\WorldSequenceToolPanel.h",
            r"..\Private\WorldSequenceDocument.cpp",
            r"..\Private\WorldSequenceToolPanel.cpp",
        )
        for path in expected:
            self.assertEqual(1, includes.count(path), path)
        panel_compile = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\WorldSequenceToolPanel.cpp']",
            namespace,
        )
        self.assertIsNotNone(panel_compile)
        options = " ".join(
            value.text or ""
            for value in panel_compile.findall("msb:AdditionalOptions", namespace)
        )
        self.assertIn("/utf-8", options)

    def test_document_is_strict_versioned_and_transactional(self) -> None:
        self.assertIn('SCHEMA = "lostark.world-sequences"', self.document_cpp)
        self.assertIn("FORMAT_VERSION = 1", self.document_cpp)
        self.assertIn("Is_ExactObject", self.document_cpp)
        self.assertIn("parse -> exact schema validation", read(
            ".md/GB/08-31/2026-08-31_WORLD_SEQUENCE_MAP_TOOL_PLAN.md"
        ))
        self.assertIn("ReplaceFileW", self.document_cpp)
        self.assertIn("MoveFileExW", self.document_cpp)
        self.assertIn("writeSucceeded = writeSucceeded && !output.fail()", self.document_cpp)

    def test_document_reads_are_bounded_and_display_text_is_safe_utf8(self) -> None:
        self.assertIn("MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u", self.document_cpp)
        self.assertIn("std::filesystem::file_size", self.document_cpp)
        self.assertIn("World sequence document exceeds the 16 MiB parse limit", self.document_cpp)
        self.assertIn("catch (const std::bad_alloc&)", self.document_cpp)
        self.assertIn("input.gcount()", self.document_cpp)
        self.assertIn("input.peek()", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.displayName)", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.category)", self.document_cpp)

    def test_document_rejects_invalid_refs_enums_and_runtime_scales(self) -> None:
        self.assertIn("availablePlacements.find(targetId)", self.document_cpp)
        self.assertIn("availableDeployPlacements.find(targetId)", self.document_cpp)
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind",
            self.document_cpp,
        )
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind",
            self.document_cpp,
        )
        self.assertIn("animationTargetSupported", self.document_cpp)
        self.assertIn("WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation", self.document_cpp)
        self.assertIn("MIN_RUNTIME_SCALE_DETERMINANT", self.document_cpp)
        self.assertIn("std::isfinite(composedX)", self.document_cpp)
        self.assertIn("Sequence scale would create a singular map transform", self.document_cpp)
        self.assertIn("boundTargets.insert(uniqueTarget).second", self.document_cpp)
        self.assertIn("sequenceTargetSupported", self.document_cpp)
        self.assertIn("MAP_ASSET_RENDER_MODE::BACKGROUND", self.panel_cpp)

    def test_preview_uses_runtime_baseline_and_never_edits_map_records(self) -> None:
        self.assertIn("Try_GetRuntimeVisible", self.panel_cpp)
        self.assertIn("baseline->runtimeVisible || !baseline->baseline.visible", self.panel_cpp)
        self.assertIn("restoredRecord.visible = target.runtimeVisible", self.panel_cpp)
        self.assertIn("Stop_AndRestore", self.panel_cpp)
        self.assertNotRegex(self.panel_cpp, r"entry\.record\s*=")
        runtime_h = read("Client/Public/MapPlacementRuntime.h")
        batch_h = read("Client/Public/MapStaticBatchObject.h")
        self.assertIn("Try_GetRuntimeVisible", runtime_h)
        self.assertIn("Try_GetInstanceVisible", batch_h)

    def test_paused_live_edit_is_validated_then_resampled(self) -> None:
        update = re.search(
            r"void Client::CWorldSequenceToolPanel::Update\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(update)
        body = update.group(0)
        self.assertIn("m_bPreviewNeedsRefresh", body)
        self.assertIn("Validate(catalog, placements, deployRuntime, validation)", body)
        self.assertIn("Apply_Preview", body)
        self.assertLess(
            body.index("Validate(catalog, placements, deployRuntime, validation)"),
            body.index("Apply_Preview"),
        )
        self.assertIn("if (m_bPreviewActive)\n\t\tm_bPreviewNeedsRefresh = true", self.panel_cpp)

    def test_selection_cannot_orphan_an_active_preview(self) -> None:
        template_list = re.search(
            r"void Client::CWorldSequenceToolPanel::Render_TemplateList\(\).*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(template_list)
        self.assertIn("ImGui::BeginDisabled(m_bPreviewActive)", template_list.group(0))
        self.assertIn("Is_PreviewActive", self.panel_h)
        self.assertIn("sequenceOwnsPreviewTargets", self.map_tool_cpp)
        self.assertIn("연출 미리보기를 Stop / Restore한 뒤", self.map_tool_cpp)

    def test_map_and_sequence_save_share_validation_and_rollback(self) -> None:
        self.assertIn("Save_PlacementsAndWorldSequences", self.map_tool_h)
        self.assertIn("PrepareAuthoringBackup", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("WriteAuthoringTransactionMarker", self.map_tool_cpp)
        self.assertIn("RecoverAuthoringTransaction", self.map_tool_cpp)
        self.assertIn("Linked save verification failed", self.map_tool_cpp)
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("m_pWorldSequenceToolPanel->Validate", body)
        self.assertIn("Save_PlacementsAndWorldSequences", body)
        toolbar = re.search(
            r"void Client::CMapTool::Render_Toolbar\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(toolbar)
        self.assertIn('ImGui::Button("Save")', toolbar.group(0))
        self.assertIn("Save_AllAuthoring()", toolbar.group(0))

    def test_linked_save_uses_area_lock_and_stale_source_cas(self) -> None:
        self.assertIn("SCOPED_AUTHORING_SAVE_LOCK", self.map_tool_cpp)
        self.assertIn(".linked-save.lock", self.map_tool_cpp)
        self.assertIn("FILE_FLAG_DELETE_ON_CLOSE", self.map_tool_cpp)
        self.assertIn("GetLastError()", self.map_tool_cpp)
        self.assertIn("ERROR_SHARING_VIOLATION", self.map_tool_cpp)
        self.assertIn("Could not open Area authoring lock", self.map_tool_cpp)
        self.assertIn("Matches_LinkedSourceBaseline", self.panel_h)
        self.assertIn("m_PlacementBaselineBytes", self.panel_h)
        self.assertIn("m_SequenceBaselineBytes", self.panel_h)
        self.assertIn("Save conflict: linked map/sequence source changed after Reload", self.panel_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("Matches_LinkedSourceBaseline"))
        self.assertLess(body.index("Matches_LinkedSourceBaseline"),
                        body.index("PrepareAuthoringBackup"))

        map_save = re.search(
            r"bool_t Client::CMapTool::Save_Placements\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(map_save)
        map_body = map_save.group(0)
        self.assertIn("return Save_PlacementsAndWorldSequences()", map_body)
        self.assertLess(map_body.index("return Save_PlacementsAndWorldSequences()"),
                        map_body.index("authoringLock.Acquire"))
        self.assertNotIn("Refresh_LinkedSourceBaseline", map_body)

    def test_linked_save_verifies_exact_intended_content(self) -> None:
        self.assertIn("AreExactlySamePlacementRecords", self.map_tool_cpp)
        self.assertIn("Has_SameDocument", self.panel_h)
        self.assertIn("Is_Equivalent", self.document_h)
        self.assertIn("Linked save verification found different Map Placement content", self.map_tool_cpp)
        self.assertIn("Linked save verification found different World Sequence content", self.map_tool_cpp)
        self.assertIn("stableVerifiedPlacements", self.map_tool_cpp)
        self.assertIn("return left == right", self.map_tool_cpp)
        self.assertIn("return left == right", self.document_cpp)
        self.assertIn("outStoredRecords", read("Client/Public/MapPlacementDocument.h"))
        placement_document = read("Client/Private/MapPlacementDocument.cpp")
        self.assertIn("UNIT_QUATERNION_TOLERANCE", placement_document)
        self.assertIn("std::abs(length - 1.f) <=", placement_document)
        self.assertIn("Linked save verification source changed during final read", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("MAX_TRANSACTION_MARKER_BYTES = 512u", self.map_tool_cpp)
        self.assertIn("transaction marker exceeds its bounded read limit", self.map_tool_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertIn("Adopt_VerifiedLinkedSourceBaseline", body)
        self.assertLess(body.rindex("ClearAuthoringTransactionMarker"),
                        body.index("Adopt_VerifiedLinkedSourceBaseline"))
        self.assertIn("editor baseline stayed unchanged", body)

    def test_reload_stages_placements_and_sequences_before_commit(self) -> None:
        reload_body = re.search(
            r"bool_t Client::CMapTool::Load_Placements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(reload_body)
        body = reload_body.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedDocument", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)
        self.assertLess(body.index("stagedWorldSequencePanel->Load_Area"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertLess(body.index("Stage_PlacementRuntime"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertIn("Is_PreviewActive", body)
        render = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(render)
        self.assertIn("Consume_ReloadAllRequest", render.group(0))
        self.assertIn("Load_Placements()", render.group(0))
        self.assertLess(render.group(0).index("Load_Placements()"),
                        render.group(0).index("return;", render.group(0).index("Load_Placements()")))

    def test_initial_area_load_recovers_an_interrupted_linked_save(self) -> None:
        switch = re.search(
            r"bool_t Client::CMapTool::Switch_EditorArea\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(switch)
        body = switch.group(0)
        self.assertIn("worldSequencePath", body)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedRecords", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)

    def test_ui_rejects_duplicate_and_background_bindings(self) -> None:
        self.assertIn("A map object can be bound to only one target slot", self.panel_cpp)
        self.assertIn("IsSequenceTargetSupported", self.panel_cpp)
        self.assertIn("[Background - unavailable]", self.panel_cpp)
        self.assertIn("sequenceTargetSupported", self.document_h)

    def test_loaded_selection_keeps_template_and_instance_coherent(self) -> None:
        load = re.search(
            r"bool_t Client::CWorldSequenceToolPanel::Load_Area\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(load)
        body = load.group(0)
        self.assertIn("m_SelectedInstanceId", body)
        self.assertIn("m_Document.Get_Instances().front().templateId", body)
        self.assertIn("placementBefore", body)
        self.assertIn("placementAfter", body)
        self.assertIn("Linked map/sequence source changed while the Area was loading", body)

    def test_visible_labels_are_english_and_help_is_korean(self) -> None:
        for label in (
            "New Sequence",
            "Sequence List",
            "Placed Instances",
            "Map Objects",
            "Add Target Track",
            "Add Key at Preview Time",
            "Stop / Restore",
        ):
            self.assertIn(label, self.panel_cpp)
        self.assertIn("ShowKoreanHelp", self.panel_cpp)
        self.assertIn("새 재사용 연출 템플릿을 만듭니다", self.panel_cpp)
        self.assertIn("미리보기는 원본 맵 배치를 수정하지 않으며", self.panel_cpp)

    def test_product_runtime_boundary_is_explicit(self) -> None:
        self.assertIn("Authoring preview only; product runtime publish is separate", self.panel_cpp)
        self.assertIn("서버 상호작용·제품 재생·길 개방은 아직 연결되지 않았습니다", self.panel_cpp)


class AnimatedPropAuthoringContractTests(unittest.TestCase):
    """MapTool owns creating the Deploy ANIM placements a sequence binds to."""

    def setUp(self) -> None:
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")
        self.catalog_h = read("Client/Public/DeployPropCatalog.h")
        self.catalog_cpp = read("Client/Private/DeployPropCatalog.cpp")

    def test_every_map_tool_declaration_has_a_definition(self) -> None:
        declared = set()
        for match in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?;",
            self.map_tool_h,
            flags=re.DOTALL,
        ):
            declared.add(match.group(1))
        defined = set(re.findall(r"CMapTool::([A-Za-z_]\w*)", self.map_tool_cpp))
        ignored = {"ETOI", "ETOUI", "float2_t", "float3_t", "float4_t"}
        missing = sorted(declared - defined - ignored)
        self.assertEqual([], missing)

    def test_animated_prop_authoring_is_reachable_from_the_sequence_mode(self) -> None:
        panel = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(panel)
        self.assertIn("Render_AnimatedPropsAuthoring();", panel.group(0))
        for label in (
            "Animated Props (Deploy ANIM)",
            "Place In Viewport",
            "Apply Transform",
            "Remove Placement",
            "Save Animated Props",
        ):
            self.assertIn(label, self.map_tool_cpp)

    def test_armed_viewport_click_places_only_in_sequence_mode(self) -> None:
        interaction = re.search(
            r"void Client::CMapTool::Update_WorldInteraction\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(interaction)
        body = interaction.group(0)
        self.assertIn(
            "if (m_bAnimatedPropPlacementArmed && mousePressed)\n"
            "\t\t\t(void)Try_PlaceSelectedDeploy();",
            body,
        )
        self.assertIn("m_bAnimatedPropPlacementArmed = false;", body)
        consumes = re.search(
            r"bool_t Client::CMapTool::ConsumesWorldLeftMouse\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(consumes)
        self.assertIn("m_bAnimatedPropPlacementArmed", consumes.group(0))

    def test_project_authored_placement_ids_stay_in_the_editor_domain(self) -> None:
        allocate = re.search(
            r"uint64_t Client::CMapTool::Allocate_AnimatedPropPlacementId\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(allocate)
        body = allocate.group(0)
        self.assertEqual(
            2, body.count("CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID")
        )
        self.assertIn("0u : candidate", body)
        # A project row may never carry extractor-only evidence fields.
        self.assertIn("0u == row.deployActorId && 0u == row.propDefinitionId", self.catalog_cpp)
        self.assertIn("PROJECT_AUTHORED", self.catalog_h)

    def test_deploy_placement_save_is_atomic_and_read_back_verified(self) -> None:
        self.assertIn("MoveFileExW", self.catalog_cpp)
        self.assertIn("MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", self.catalog_cpp)
        save = re.search(
            r"bool_t Client::CMapTool::Save_DeployPlacements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save)
        body = save.group(0)
        self.assertIn("CDeployPropCatalog verification;", body)
        self.assertIn("DeployProp placement save verification found different content", body)
        self.assertLess(body.index("verification.Load"), body.index("m_bDeployDirty = false"))

    def test_unsaved_deploy_authoring_blocks_and_saves_before_sequences(self) -> None:
        unsaved = re.search(
            r"bool_t Client::CMapTool::Has_UnsavedAuthoring\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(unsaved)
        self.assertIn("m_bDeployDirty", unsaved.group(0))
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\treturn true;\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("if (m_bDeployDirty && !Save_DeployPlacements())", body)
        self.assertLess(
            body.index("Save_DeployPlacements()"),
            body.index("Save_PlacementsAndWorldSequences()"),
        )

    def test_removing_a_bound_animated_prop_rolls_the_runtime_back(self) -> None:
        remove = re.search(
            r"bool_t Client::CMapTool::Remove_SelectedAnimatedProp\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(remove)
        body = remove.group(0)
        self.assertIn("CDeployPropCatalog restore = m_DeployRuntime.Get_Catalog();", body)
        self.assertIn("m_pWorldSequenceToolPanel->Validate(", body)
        self.assertIn("Commit_DeployCatalog(std::move(restore)", body)
        self.assertLess(body.index("Validate("), body.index("std::move(restore)"))
        self.assertIn("Source-extracted Deploy placements cannot be removed", body)

    def test_commit_releases_preview_seams_before_rebuilding_the_runtime(self) -> None:
        commit = re.search(
            r"bool_t Client::CMapTool::Commit_DeployCatalog\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(commit)
        body = commit.group(0)
        self.assertIn("Stop_AndRestore(", body)
        self.assertIn("Restore_DestructionPreview();", body)
        self.assertIn("m_pDestructionSimulationController->Clear();", body)
        self.assertLess(body.index("Stop_AndRestore("), body.index("stagedRuntime.Load("))
        self.assertLess(
            body.index("stagedRuntime.Load("),
            body.index("m_DeployRuntime = std::move(stagedRuntime);"),
        )

    def test_korean_help_remains_valid_escaped_utf8(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        map_tool = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\MapTool.cpp']", namespace
        )
        self.assertIsNotNone(map_tool)
        # MapTool now explicitly compiles as UTF-8; escaped help remains valid.

        blocks = re.findall(
            r"static const char_t\* const (ANIMATED_PROP_HELP_\w+)\s*=\s*"
            r"((?:\s*\"(?:[^\"\\]|\\.)*\")+)\s*;",
            self.map_tool_cpp,
        )
        self.assertEqual(5, len(blocks))
        for name, body in blocks:
            data = bytearray()
            for part in re.findall(r"\"((?:[^\"\\]|\\.)*)\"", body):
                index = 0
                while index < len(part):
                    if part[index] == "\\" and part[index + 1] == "x":
                        data.append(int(part[index + 2:index + 4], 16))
                        index += 4
                    elif part[index] == "\\":
                        data.append(ord(part[index + 1]))
                        index += 2
                    else:
                        data.append(ord(part[index]))
                        index += 1
                # An \xHH escape followed by another hex digit would swallow it.
                self.assertIsNone(
                    re.search(r"\\x[0-9A-Fa-f]{2}[0-9A-Fa-f]", part), name
                )
            text = data.decode("utf-8")
            self.assertTrue(any("\uac00" <= ch <= "\ud7a3" for ch in text), name)



if __name__ == "__main__":
    unittest.main(verbosity=2)

```

## 데이터 추가

WORLD resource 배열에 다음행 추가, revision 1699→1700.

```json
{
  "objectId": "world.object.group.kouku.g3.outer_fire.all",
  "displayName": "외곽불_전체",
  "modelAssetId": "",
  "anchorKind": "WORLD",
  "modelPreScale": 0.01,
  "animated": false,
  "scale": [
    1,
    1,
    1
  ],
  "motionInstanceIds": [
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_d_cw",
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_d_ccw",
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_e_cw",
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_e_ccw",
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_f_cw",
    "world.sequence.instance.kouku.g3.fire_hook.extra_fire_f_ccw"
  ]
}
```

Benchmark의 G01 pattern/world6행만 제거하며 다른내용보존, revision증가, counter유지. 코드 반영 후 focused publisher 정상/오류입력, WorldSequences Validate/Publish/Check, Debug Client 최소빌드 및 diff check. 사용자 화면확인은 남긴다.

# G03. 한 항목 클릭 → 여섯 행 동시 편집

사용자는 G02의 접히는 그룹/개별 자식 선택 대신 `외곽불_전체` 한 항목을 클릭하면 Object Sequencer에 여섯 행이 동시에 펼쳐지고 각각 수정되는 형태를 승인했다. 데이터와 런타임 그룹 참조 계약은 유지하며 이번 변경은 `Client/Private/WorldObjectTool.cpp`의 선택·표시 위치만 수정한다. 기존 원본 모션/모델과 저작 값은 변경하지 않는다.

## 파일과 함수 책임

- `Render_Resources`: motionInstanceIds가 있는 항목은 트리 자식 없이 Selectable 한 줄이다. 선택 시 Select_Object로 통합 선택을 유지하고 Object Sequencer를 열며 Seek(0)으로 기존 전체 preview를 정지 상태로 sample한다. 모델 기반 일반 Object는 기존 자식 선택 흐름을 유지한다.
- `Render_GroupDetail`: 통합 이름 편집과 Object Sequencer 안내만 담당한다. 여섯 개 편집기를 서로 다른 창으로 분리하지 않는다.
- `Render_GroupSequence`: 기존 단일 Play/Pause/Stop/seek 아래에 모든 member의 공통 시간축, Enabled, Map Position, Orbit Radius를 접힘 없이 표시한다. 각 행의 시간막대를 드래그해도 모든 member가 같은 시각으로 이동한다. Save/Mark_Dirty 및 원 중심을 유지하는 반경 비례 편집은 G02의 동일 경로를 재사용한다.
- H 선언, 문서 codec, publisher, 데이터, Level/player, Server, Shader는 바뀌지 않는다. 새 C++ 파일 및 프로젝트/filter 등록은 없다.

## 적용과 확인

기존 파일의 BOM/UTF-8/CRLF를 보존하여 동시 변경 검사 후 반영한다. WorldSequences authoring/runtime bytes 불변과 JSON/XML parse, scoped diff 검사를 확인한다. Client의 C++ 컴파일과 `_BuildLinkAction`만 실행하며 셰이더 컴파일을 새로 시작하지 않는다. 실행 중인 Client/빌드는 임의 종료하지 않는다. 사용자 수동 절차는 Object Resources → Map → 외곽불_전체 클릭, 여섯 행과 아레나 동시 표시 확인, 각 행 반경 편집, Play/Pause/seek, Save/Reload다. 실제 화면 판정은 사용자가 수행한다.

## Client/Private/WorldObjectTool.cpp G03 전체 반영 코드

```cpp
#include "imgui.h"
#include "WorldObjectTool.h"

#ifdef _DEBUG
#include "CompositionTimeline.h"
#include "Animation.h"
#include "BinaryAsset/WModelDecoder.h"
#include "RuntimeAssetRoot.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldSequencePlayer.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>

namespace
{
constexpr const char* AREA_ID = "LV_LUT_MIDNIGHTC_ED";

bool ReadSource(const std::filesystem::path& path, std::string& bytes,
    std::string& status, const bool optional = false)
{
    std::error_code error;
    if (optional && !std::filesystem::exists(path, error) && !error)
    { bytes.clear(); return true; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 64u * 1024u * 1024u)
    { status = "Cannot read bounded authoring source: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot open source: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (input.bad() || bytes.size() != size)
    { status = "Source changed or failed while reading: " + path.string(); return false; }
    return true;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool EditText(const char* label, std::string& value, const size_t maxBytes = 128)
{
    char text[1024]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, (std::min)(sizeof(text), maxBytes + 1))) return false;
    value = text;
    return true;
}

bool EditUInt(const char* label, uint32_t& value, const int maximum, const int minimum = 0)
{
    int number = static_cast<int>((std::min)(value, static_cast<uint32_t>(INT_MAX)));
    if (!ImGui::DragInt(label, &number, 1.f, minimum, maximum, "%d", ImGuiSliderFlags_AlwaysClamp)) return false;
    value = static_cast<uint32_t>((std::clamp)(number, minimum, maximum));
    return true;
}

const char* MotionEndLabel(const Client::WORLD_SEQUENCE_MOTION_END motionEnd)
{
    switch (motionEnd)
    {
    case Client::WORLD_SEQUENCE_MOTION_END::STOP: return "Stop";
    case Client::WORLD_SEQUENCE_MOTION_END::HOLD: return "Hold Last Pose";
    case Client::WORLD_SEQUENCE_MOTION_END::LOOP: return "Loop";
    case Client::WORLD_SEQUENCE_MOTION_END::NEXT: return "Play Motion";
    default: return "Unknown";
    }
}

float3_t QuaternionEuler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
}

using namespace Client;

CWorldObjectTool::~CWorldObjectTool()
{
    Stop_Preview();
    if (m_PublishProcess) CloseHandle(m_PublishProcess);
}

void CWorldObjectTool::Open()
{
    m_Open = true;
    m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
    if (!m_Ready) Load_Source();
}

bool CWorldObjectTool::Consume_InteractionRequest()
{
    const bool requested = m_InteractionRequested;
    m_InteractionRequested = false;
    return requested;
}

void CWorldObjectTool::Deactivate()
{
    Stop_Preview();
}

bool CWorldObjectTool::Load_Source()
{
    const auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { m_Status = "Enter KoukuSaydon, then Reload Source to edit and preview world objects."; return false; }
    const auto directory = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / AREA_ID);
    const auto sourcePath = directory / (std::string(AREA_ID) + ".worldsequences.json");
    const auto placementPath = directory / (std::string(AREA_ID) + ".mapplacements");
    const auto deployPath = directory / (std::string(AREA_ID) + ".deployplacements");
    std::string sourceBefore, mapBefore, deployBefore;
    if (!ReadSource(sourcePath, sourceBefore, m_Status) ||
        !ReadSource(placementPath, mapBefore, m_Status) ||
        !ReadSource(deployPath, deployBefore, m_Status, true)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP map;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    level->Get_WorldObjectValidationTargets(map, deploy);
    CWorldSequenceDocument staged;
    if (!staged.Load(sourcePath, AREA_ID, map, deploy, m_Status)) return false;
    std::string sourceAfter, mapAfter, deployAfter;
    if (!ReadSource(sourcePath, sourceAfter, m_Status) ||
        !ReadSource(placementPath, mapAfter, m_Status) ||
        !ReadSource(deployPath, deployAfter, m_Status, true)) return false;
    if (sourceBefore != sourceAfter || mapBefore != mapAfter || deployBefore != deployAfter)
    { m_Status = "Linked authoring source changed during Reload; existing draft preserved."; return false; }
    Stop_Preview();
    m_Document = std::move(staged);
    m_SavedDocument = m_Document;
    m_MapTargets = std::move(map);
    m_DeployTargets = std::move(deploy);
    m_SourcePath = sourcePath; m_PlacementPath = placementPath; m_DeployPath = deployPath;
    m_SourceBytes = std::move(sourceAfter); m_PlacementBytes = std::move(mapAfter); m_DeployBytes = std::move(deployAfter);
    m_Ready = true; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_AnimationObjectId.clear();
    m_AnimationCandidateObjectId.clear();
    m_AnimationCandidateModelAssetId.clear();
    if (!m_Document.Find_ObjectResource(m_SelectedObject))
        m_SelectedObject = m_Document.Get_ObjectResources().empty() ? "" : m_Document.Get_ObjectResources().front().objectId;
    Select_Object(m_SelectedObject);
    m_Status = "Source loaded. Save stores Object edits and applies them for the next play.";
    return true;
}

bool CWorldObjectTool::Matches_SourceBaseline()
{
    std::string source, map, deploy;
    if (!ReadSource(m_SourcePath, source, m_Status) ||
        !ReadSource(m_PlacementPath, map, m_Status) ||
        !ReadSource(m_DeployPath, deploy, m_Status, true)) return false;
    if (source != m_SourceBytes || map != m_PlacementBytes || deploy != m_DeployBytes)
    { m_Status = "Save conflict: linked source changed on disk. Draft preserved; Reload Source before saving."; return false; }
    return true;
}

bool CWorldObjectTool::Save_Source()
{
    if (!m_Ready || m_PublishProcess || !Matches_SourceBaseline()) return false;
    auto stagedPath = m_SourcePath;
    stagedPath += L".world-object-" + std::to_wstring(GetCurrentProcessId()) + L".stage";
    // The document owns codec/validation. A separate staging destination permits
    // a readback and the final linked-source CAS immediately before promotion.
    if (!m_Document.Save(stagedPath, m_MapTargets, m_DeployTargets, m_Status)) return false;
    CWorldSequenceDocument verified;
    std::string stagedBytes;
    const bool ready = verified.Load(stagedPath, AREA_ID, m_MapTargets, m_DeployTargets, m_Status) &&
        m_Document.Is_Equivalent(verified) && ReadSource(stagedPath, stagedBytes, m_Status) && Matches_SourceBaseline();
    if (!ready || !MoveFileExW(stagedPath.c_str(), m_SourcePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        std::error_code ignored; std::filesystem::remove(stagedPath, ignored);
        if (ready) m_Status = "Atomic source replacement failed; draft preserved.";
        else if (m_Status.empty()) m_Status = "Staged source readback differs; draft preserved.";
        return false;
    }
    m_SourceBytes = std::move(stagedBytes); m_Document = std::move(verified);
    m_SavedDocument = m_Document; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_Status = "Saved; applying World Object runtime data.";
    Start_Publish();
    return true;
}

void CWorldObjectTool::Start_Publish()
{
    if (!m_Ready || m_Dirty || m_PublishProcess) return;
    if (!Matches_SourceBaseline())
    { m_Status = "Saved; apply stopped because linked source changed. Reload Source before retrying."; return; }
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    if (!std::filesystem::is_regular_file(script)) { m_Status = "Saved; apply failed: publisher is missing. Save to retry."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { m_Status = "Saved; apply failed: log folder is unavailable. Save to retry."; return; }
    m_PublishLog = std::filesystem::path(temporary) / (L"LostArk-WorldObject-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(m_PublishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == log) { m_Status = "Saved; apply failed: cannot create log. Save to retry."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == input) { CloseHandle(log); m_Status = "Saved; apply failed: cannot prepare input. Save to retry."; return; }
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
        script.wstring() + L"\" -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = !!CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
    CloseHandle(log); CloseHandle(input);
    if (!started) { m_Status = "Saved; apply failed: cannot start publisher. Save to retry."; return; }
    CloseHandle(process.hThread); m_PublishProcess = process.hProcess;
    m_Status = "Saved; applying World Object runtime data. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Poll_Publish()
{
    if (!m_PublishProcess || WaitForSingleObject(m_PublishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_PublishProcess, &code);
    CloseHandle(m_PublishProcess); m_PublishProcess = nullptr;
    if (code != 0)
    { m_Status = "Saved; apply failed (" + std::to_string(code) + "). Previous runtime preserved. Save to retry. Log: " + m_PublishLog.string(); return; }
    ++m_SavedGeneration;
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        std::string status;
        if (!level->Reload_WorldObjectRuntime(status))
        { m_Status = "Saved and runtime files applied; next-play reload pending: " + status; return; }
    }
    m_Status = "Saved and applied for the next play. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Mark_Dirty()
{
    m_PristinePatternId.clear();
    m_Document.Touch(); m_Dirty = true; m_PreviewDirty = m_PreviewActive;
}

void CWorldObjectTool::Stop_Preview()
{
    if (m_PreviewActive && m_PreviewLevel && m_PreviewLevel == CLevel_KakulSaydonArena::Get_Active())
        m_PreviewLevel->Debug_StopWorldObjectPreview();
    m_PreviewLevel = nullptr; m_PreviewActive = false; m_Playing = false; m_PreviewDirty = false;
}

const WORLD_SEQUENCE_INSTANCE* CWorldObjectTool::Preview_Instance() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance.empty() && resource ?
        resource->defaultMotionInstanceId : m_SelectedInstance);
    return instance && instance->enabled ? instance : nullptr;
}

const WORLD_SEQUENCE_OBJECT_RESOURCE* CWorldObjectTool::Preview_Group() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    return m_SelectedInstance.empty() && resource && !resource->motionInstanceIds.empty() ? resource : nullptr;
}

bool CWorldObjectTool::Begin_Preview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) { m_Status = "World object preview requires the active KoukuSaydon arena."; return false; }
    if (const auto* group = Preview_Group())
    {
        if (SpanMs() <= 0.f)
        {
            Stop_Preview(); m_ClockMs = 0.f;
            m_Status = m_PreviewStatus = "All group motions are disabled.";
            return false;
        }
        if (!level->Debug_BeginWorldObjectPreview(m_Document, group->objectId, m_Status, m_PreviewAtCharacter)) return false;
        m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
        return true;
    }
    const auto* instance = Preview_Instance();
    if (!instance) { m_Status = "Choose an enabled Default Motion or select a connected Motion."; return false; }
    if (!level->Debug_BeginWorldObjectPreview(m_Document, instance->instanceId, m_Status, m_PreviewAtCharacter)) return false;
    m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
    return true;
}

f32_t CWorldObjectTool::SpanMs() const
{
    if (const auto* group = Preview_Group())
    {
        float span = 0.f;
        for (const auto& id : group->motionInstanceIds)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance && instance->enabled ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (sequence) span = (std::max)(span, static_cast<float>(instance->startDelayMs) +
                static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed));
        }
        return span;
    }
    const auto* instance = Preview_Instance();
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    return !sequence ? 0.f : static_cast<float>(instance->startDelayMs) +
        static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
}

f32_t CWorldObjectTool::PreviewSpanMs() const
{
    const auto* instance = Preview_Instance();
    if (instance && (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP ||
        instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        return (std::max)(SpanMs(), static_cast<float>(CWorldSequenceDocument::MAX_DURATION_MS));
    return SpanMs();
}

void CWorldObjectTool::Seek(const f32_t clockMs)
{
    m_ClockMs = (std::clamp)(clockMs, 0.f, PreviewSpanMs());
    if ((!m_PreviewActive || m_PreviewDirty) && !Begin_Preview()) { m_Playing = false; return; }
    if (!m_PreviewLevel->Debug_SampleWorldObjectPreview(m_ClockMs, m_PreviewStatus))
    { m_Status = m_PreviewStatus; Stop_Preview(); }
}

void CWorldObjectTool::Update(const f32_t seconds, const bool_t active)
{
    Poll_Publish();
    if (m_PhysicalScanRunning && m_PhysicalScan.Advance())
    {
        if (m_PhysicalScan.Commit(m_PhysicalAssets, m_PhysicalStatus)) Rebuild_PhysicalTree();
        m_PhysicalScanRunning = false;
    }
    if (!active || !m_Open || (m_PreviewLevel && m_PreviewLevel != CLevel_KakulSaydonArena::Get_Active()))
    { Stop_Preview(); return; }
    if (m_PreviewActive && m_PreviewDirty) Seek(m_ClockMs);
    if (!m_Playing || !m_PreviewActive) return;
    const float span = PreviewSpanMs();
    m_ClockMs += (std::max)(0.f, seconds) * 1000.f;
    if (span <= 0.f) { Stop_Preview(); return; }
    if (m_ClockMs >= span) { m_ClockMs = span; m_Playing = false; }
    Seek(m_ClockMs);
}

std::vector<std::string> CWorldObjectTool::StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const
{
    if (!resource.motionInstanceIds.empty()) return resource.motionInstanceIds;
    if (!resource.sequenceInstanceId.empty()) return {resource.sequenceInstanceId};
    std::vector<std::string> ids;
    for (const auto& instance : m_Document.Get_Instances())
        if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
            return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
        })) ids.push_back(instance.instanceId);
    return ids;
}

void CWorldObjectTool::Select_Object(const std::string& id)
{
    Stop_Preview(); m_SelectedObject = id; m_SelectedInstance.clear(); m_ClockMs = 0.f;
    if (Preview_Group()) m_PreviewAtCharacter = false;
    m_SelectedTrack = 0; m_SelectedKey = 0;
}

void CWorldObjectTool::Select_State(const std::string& id)
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto motions = resource ? StateIds(*resource) : std::vector<std::string>{};
    if (std::find(motions.begin(), motions.end(), id) == motions.end())
    { m_Status = "The selected motion does not belong to this object."; return; }
    Stop_Preview(); m_SelectedInstance = id; m_SelectedTrack = 0; m_SelectedKey = 0; m_ClockMs = 0.f;
}

bool CWorldObjectTool::Create_Object()
{
    if (!m_NewObjectName[0]) { m_Status = "Enter an object name."; return false; }
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        resource.objectId = "world.object.resource." + std::to_string(index);
        if (!m_Document.Find_ObjectResource(resource.objectId)) break;
    }
    resource.displayName = m_NewObjectName.data();
    resource.anchorKind = m_NewObjectAnchor == 2 ? "BOSS" : m_NewObjectAnchor == 1 ? "PLAYER" : "WORLD";
    if (m_Document.Get_ObjectResources().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World object resource capacity reached."; return false; }
    m_Document.Get_ObjectResources().push_back(resource);
    Mark_Dirty(); Select_Object(resource.objectId);
    m_Status = "Object created. Assign its shared model, then Create Motion in Object Detail. Assign Model before Save.";
    m_NewObjectName[0] = 0;
    m_NewStateName[0] = 0;
    return true;
}

void CWorldObjectTool::Create_State()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty() || !m_NewStateName[0]) return;
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(*resource, m_NewStateName.data(), sequence, instance)) return;
    const bool firstMotion = StateIds(*resource).empty();
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(instance);
    if (firstMotion) resource->defaultMotionInstanceId = instance.instanceId;
    Mark_Dirty(); Select_State(instance.instanceId);
    m_PristinePatternId = instance.instanceId;
    m_NewStateName[0] = 0;
    m_Status = "Motion created. Append Clip or edit Transform/Physics, then Save.";
}

bool CWorldObjectTool::Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const std::string& stateName, WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance)
{
    if (m_Document.Get_Templates().size() >= CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        m_Document.Get_Instances().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World sequence document capacity reached."; return false; }
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        sequence.sequenceId = resource.objectId + ".state." + std::to_string(index);
        instance.instanceId = sequence.sequenceId + ".instance";
        if (!m_Document.Find_Template(sequence.sequenceId) && !m_Document.Find_Instance(instance.instanceId)) break;
    }
    sequence.displayName = stateName; sequence.durationMs = 2000;
    WORLD_SEQUENCE_TRACK track; track.slotId = "object"; track.keys.push_back({});
    WORLD_SEQUENCE_TRANSFORM_KEY end; end.timeMs = sequence.durationMs; track.keys.push_back(end);
    sequence.tracks.push_back(track);
    instance.templateId = sequence.sequenceId;
    instance.anchorKind = resource.anchorKind;
    instance.bindings.push_back({"object", WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE, resource.objectId});
    if (instance.anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level)
        { m_Status = "Map state creation requires the active KoukuSaydon arena."; return false; }
        if (!level->Try_Get_AuthoringPreviewPlacement(instance.position, m_Status))
        {
            m_Status = "Map state needs the current character placement: " + m_Status;
            return false;
        }
    }
    return true;
}

void CWorldObjectTool::Change_ResourceAnchor(
    WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind)
{
    if (resource.anchorKind == anchorKind || !resource.sequenceInstanceId.empty()) return;
    float3_t position{};
    if (anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level || !level->Try_Get_AuthoringPreviewPlacement(position, m_Status))
        { m_Status = "Map anchor needs the current character placement: " + m_Status; return; }
    }
    resource.anchorKind = anchorKind;
    if (anchorKind != "BOSS") { resource.anchorBossArchetypeId.clear(); resource.anchorBone.clear(); }
    for (const auto& id : StateIds(resource))
    {
        auto* instance = m_Document.Find_Instance(id);
        if (!instance || instance->anchorKind == anchorKind) continue;
        instance->anchorKind = anchorKind;
        instance->position = position;
    }
    Mark_Dirty();
    m_Status = anchorKind == "BOSS" ?
        "Boss anchor applied to this resource's states. Choose the boss and BODY bone below." : anchorKind == "PLAYER" ?
        "Character anchor applied to this resource's states; offsets start at the character origin." :
        "Map anchor applied to this resource's states at the current character position.";
}

void CWorldObjectTool::Render()
{
    if (!m_Open) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport ? viewport->WorkPos : ImVec2(0.f, 0.f);
    const ImVec2 available = viewport ? viewport->WorkSize : ImVec2(1600.f, 900.f);
    constexpr float margin = 8.f, gap = 8.f;
    const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
    const float height = (std::max)(1.f, available.y - margin * 2.f);
    const float leftWidth = width * .23f, rightWidth = width * .24f;
    const float centerWidth = width - leftWidth - rightWidth;
    const float leftX = origin.x + margin, centerX = leftX + leftWidth + gap;
    const float rightX = centerX + centerWidth + gap, topY = origin.y + margin;
    const ImGuiCond condition = m_ResetLayoutRequested ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    m_ResetLayoutRequested = false;
    const auto beginPane = [&](const char* name, bool& visible, const ImVec2 position, const ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(name, &visible, ImGuiWindowFlags_MenuBar);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0))
            m_InteractionRequested = true;
        if (expanded) Render_WindowMenu();
        return expanded;
    };

    if (m_ResourcesOpen)
    {
        if (beginPane("Object Resources###WorldObjectResourcesWindow", m_ResourcesOpen,
            {leftX, topY}, {leftWidth, height}))
        {
            if (m_Ready) Render_Resources();
            else
            {
                if (ImGui::Button("Reload Source")) Load_Source();
                ImGui::TextWrapped("%s", m_Status.c_str());
            }
            if (m_Ready) Render_EffectResources();
            Render_PhysicalResources();
            if (m_Ready) Render_AnimationResources();
        }
        ImGui::End();
    }
    if (m_SequencerOpen)
    {
        if (beginPane("Object Sequencer###WorldObjectSequencerWindow", m_SequencerOpen,
            {centerX, topY + height * .57f}, {centerWidth, height * .43f}))
        {
            Render_Toolbar();
            auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (const auto* group = Preview_Group()) Render_GroupSequence(*group);
            else if (sequence) Render_Sequence(*sequence);
            else
            {
                ImGui::TextWrapped("Select a Motion beneath an Object to open its Lifetime timeline. The parent Object edits shared resources only.");
                ImGui::BeginDisabled(); ImGui::Button("Play"); ImGui::SameLine(); ImGui::Button("Append Clip"); ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }
    if (m_DetailOpen)
    {
        if (beginPane("Object Detail###WorldObjectDetailWindow", m_DetailOpen,
            {rightX, topY}, {rightWidth, height}))
        {
            if (m_Ready) Render_Detail();
            else ImGui::TextDisabled("Load Object Resources to edit an object.");
        }
        ImGui::End();
    }
    if (!m_ResourcesOpen && !m_SequencerOpen && !m_DetailOpen) m_Open = false;
    if (!m_Open) Stop_Preview();
}

void CWorldObjectTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Object Resources", nullptr, &m_ResourcesOpen);
        ImGui::MenuItem("Object Sequencer", nullptr, &m_SequencerOpen);
        ImGui::MenuItem("Object Detail", nullptr, &m_DetailOpen);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All")) m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
            m_ResetLayoutRequested = true;
        }
        if (ImGui::MenuItem("Close World Object Tool")) m_Open = false;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void CWorldObjectTool::Render_Toolbar()
{
    if (ImGui::Button("Reload Source"))
    {
        if (m_Dirty) ImGui::OpenPopup("Reload object source?");
        else Load_Source();
    }
    if (ImGui::BeginPopupModal("Reload object source?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload discards this tool's unsaved object/state edits.");
        if (ImGui::Button("Discard and Reload")) { Load_Source(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Ready || m_PublishProcess);
    if (ImGui::Button(m_Dirty ? "Save *" : "Save")) Save_Source();
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CWorldObjectTool::Render_Resources()
{
    if (ImGui::Button("Create Object"))
    {
        m_CreateObjectFailed = false;
        ImGui::OpenPopup("Create Object Resource");
    }
    ImGui::SameLine(); ImGui::TextDisabled("%zu resources", m_Document.Get_ObjectResources().size());
    if (ImGui::BeginPopupModal("Create Object Resource", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputTextWithHint("Name", "New object name", m_NewObjectName.data(), m_NewObjectName.size());
        ImGui::Combo("Anchor Type", &m_NewObjectAnchor, "Map\0Character\0Boss\0");
        ImGui::TextUnformatted(m_NewObjectAnchor == 2 ?
            "Boss: motions follow the chosen boss BODY bone. Select the boss and bone in Object Detail." : m_NewObjectAnchor == 0 ?
            "Map: motions created later use a fixed world anchor." :
            "Character: motions created later use each living character as their anchor.");
        ImGui::TextUnformatted("Creates the parent Object only. Add its motions from Object Detail.");
        ImGui::BeginDisabled(!m_NewObjectName[0]);
        if (ImGui::Button("Create"))
        {
            m_CreateObjectFailed = !Create_Object();
            if (!m_CreateObjectFailed) ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (m_CreateObjectFailed) ImGui::TextWrapped("%s", m_Status.c_str());
        ImGui::EndPopup();
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectSearch", "Search object or motion", m_ObjectSearch.data(), m_ObjectSearch.size());
    const float treeHeight = (std::max)(120.f, ImGui::GetContentRegionAvail().y * .28f);
    if (ImGui::BeginChild("ObjectResourceTree", ImVec2(0.f, treeHeight), true))
    {
        const auto search = Lower(m_ObjectSearch.data());
        const auto stateMatches = [&search](const std::string& id, const WORLD_SEQUENCE_TEMPLATE* sequence)
        {
            const auto searchable = sequence ? sequence->displayName + " " + sequence->sequenceId + " " + id : id;
            return Lower(searchable).find(search) != std::string::npos;
        };
        for (const char* anchor : {"WORLD", "PLAYER", "BOSS"})
        {
            const char* category = std::string(anchor) == "WORLD" ? "Map" : std::string(anchor) == "BOSS" ? "Boss" : "Character";
            size_t count = 0;
            for (const auto& resource : m_Document.Get_ObjectResources()) if (resource.anchorKind == anchor) ++count;
            const std::string categoryLabel = std::string(category) + " (" + std::to_string(count) + ")";
            if (!ImGui::TreeNodeEx(anchor, ImGuiTreeNodeFlags_DefaultOpen, "%s", categoryLabel.c_str())) continue;
            for (const auto& resource : m_Document.Get_ObjectResources())
            {
                if (resource.anchorKind != anchor) continue;
                const auto states = StateIds(resource);
                const bool resourceMatches = search.empty() || Lower(resource.displayName + " " + resource.objectId).find(search) != std::string::npos;
                bool matches = resourceMatches;
                if (!matches)
                    for (const auto& id : states)
                    {
                        const auto* state = m_Document.Find_Instance(id);
                        const auto* sequence = state ? m_Document.Find_Template(state->templateId) : nullptr;
                        if (stateMatches(id, sequence)) { matches = true; break; }
                    }
                if (!matches) continue;
                ImGui::PushID(resource.objectId.c_str());
                if (!resource.motionInstanceIds.empty())
                {
                    // One authoring entry opens every member; it is not a folder of solo previews.
                    if (ImGui::Selectable(resource.displayName.c_str(), m_SelectedObject == resource.objectId && m_SelectedInstance.empty()))
                    {
                        Select_Object(resource.objectId);
                        m_SequencerOpen = true;
                        Seek(0.f);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open all %zu motions together in Object Sequencer.", states.size());
                    ImGui::PopID();
                    continue;
                }
                const auto label = resource.displayName +
                    (resource.sequenceInstanceId.empty() && resource.modelAssetId.empty() ? " [assign model]" : "");
                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth | (m_SelectedObject == resource.objectId && m_SelectedInstance.empty() ? ImGuiTreeNodeFlags_Selected : 0);
                if (!search.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                const bool open = ImGui::TreeNodeEx("Resource", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked()) Select_Object(resource.objectId);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", resource.objectId.c_str());
                if (open)
                {
                    for (const auto& id : states)
                    {
                        const auto* instance = m_Document.Find_Instance(id);
                        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
                        if (!resourceMatches && !stateMatches(id, sequence)) continue;
                        ImGui::PushID(id.c_str());
                        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), id == m_SelectedInstance))
                        {
                            m_SelectedObject = resource.objectId;
                            Select_State(id);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
                        ImGui::PopID();
                    }
                    if (states.empty()) ImGui::TextDisabled("No motions - select Object to Create Motion");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_EffectResources()
{
    if (!ImGui::CollapsingHeader("V2 Effects", ImGuiTreeNodeFlags_DefaultOpen)) return;
    const bool reload = ImGui::Button("Reload V2 Effects");
    if (!m_EffectInventoryLoaded || reload)
    {
        std::vector<EFFECT_V2_RESOURCE_SUMMARY> staged;
        if (CEffectV2Catalog::Get().Read_Inventory(staged, m_EffectResourceStatus))
            m_EffectResources = std::move(staged);
        m_EffectInventoryLoaded = true;
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectEffectSearch", "Search smoke / group / leaf", m_EffectSearch.data(), m_EffectSearch.size());
    const auto search = Lower(m_EffectSearch.data());
    if (ImGui::BeginChild("ObjectV2Effects", ImVec2(0.f, 135.f), true))
        for (const auto& effect : m_EffectResources)
        {
            if (!search.empty() && Lower(effect.strDisplayName + " " + effect.strResourceId).find(search) == std::string::npos) continue;
            ImGui::PushID(effect.strResourceId.c_str());
            const auto label = std::string(effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "[Group] " : "[Leaf] ") +
                (effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName);
            ImGui::BeginDisabled(!effect.strStatus.empty());
            if (ImGui::Selectable(label.c_str(), m_SelectedEffectResource == effect.strResourceId && m_SelectedEffectKind == effect.eKind))
            { m_SelectedEffectResource = effect.strResourceId; m_SelectedEffectKind = effect.eKind; }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s\n%s", effect.strResourceId.c_str(), effect.strStatus.c_str());
            ImGui::PopID();
        }
    ImGui::EndChild();
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const bool target = instance && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
    ImGui::BeginDisabled(!target || m_SelectedEffectResource.empty());
    if (ImGui::Button("Append Effect at Motion End")) Append_SelectedEffect();
    ImGui::EndDisabled();
    if (!target) ImGui::TextWrapped("Select an Object's child Motion, then append an Effect row.");
    if (!m_EffectResourceStatus.empty()) ImGui::TextWrapped("%s", m_EffectResourceStatus.c_str());
}

bool CWorldObjectTool::Append_SelectedEffect()
{
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence || instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
    { m_Status = "Select a child Object Motion before appending an Effect."; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(m_SelectedEffectKind, m_SelectedEffectResource, snapshot, m_EffectResourceStatus))
    { m_Status = "Effect Append failed: " + m_EffectResourceStatus; return false; }
    // Match the existing V2 authoring preview span, including particles/trails
    // remaining after emission ends. The row does not stretch leaf envelopes.
    const auto leafSpan = [](const EFFECT_V2_DOCUMENT& document, const uint32_t explicitMs, const bool tailEnabled)
    {
        const auto& params = document.Desc.Params;
        const double rate = (std::max)(.001, static_cast<double>(params.fPlayRate));
        const double emission = explicitMs ? explicitMs : params.fLifetime > 0.f ?
            std::ceil(params.fLifetime * 1000.0 / rate) : 3000.0;
        const double tail = !tailEnabled ? 0.0 : document.eType == EFFECT_V2_TYPE::PARTICLE ?
            params.Particle.vLifetime.y * 1000.0 / rate : document.eType == EFFECT_V2_TYPE::TRAIL ?
            params.Trail.fPointLifetime * 1000.0 / rate : 0.0;
        return emission + std::ceil(tail);
    };
    double span = 0.;
    if (m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        const auto* group = snapshot->Find_Group(m_SelectedEffectResource);
        if (!group) { m_Status = "Selected Effect group is unavailable."; return false; }
        span = group->iDurationMs;
        if (!group->iDurationMs)
            for (const auto& child : group->Children)
            {
                const auto* leaf = snapshot->Find_Document(child.strEffectId);
                if (!leaf) { m_Status = "Selected Effect group child is unavailable."; return false; }
                span = (std::max)(span, child.iStartMs + leafSpan(*leaf, child.iDurationMs,
                    child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE));
            }
    }
    else
    {
        const auto* leaf = snapshot->Find_Document(m_SelectedEffectResource);
        if (!leaf) { m_Status = "Selected Effect leaf is unavailable."; return false; }
        span = leafSpan(*leaf, 0u, true);
    }
    CWorldSequenceDocument staged = m_Document;
    auto* edited = staged.Find_Template(sequence->sequenceId);
    WORLD_SEQUENCE_EFFECT_TRACK row;
    uint32_t serial = 1u;
    do { row.effectTrackId = "effect." + std::to_string(serial++); }
    while (std::any_of(edited->effectTracks.begin(), edited->effectTracks.end(),
        [&](const auto& value) { return value.effectTrackId == row.effectTrackId; }));
    row.slotId = instance->bindings.front().slotId;
    row.resourceKind = m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.resourceId = m_SelectedEffectResource;
    row.durationMs = static_cast<uint32_t>((std::clamp)(std::ceil(span), 1., 600000.));
    edited->effectTracks.push_back(row);
    if (!staged.Validate(m_MapTargets, m_DeployTargets, m_Status)) return false;
    m_SelectedEffectRow = edited->effectTracks.size() - 1u;
    m_Document = std::move(staged);
    Mark_Dirty();
    m_Status = "Effect row appended at Motion End. Adjust it in Effect Rows, then Save.";
    return true;
}

void CWorldObjectTool::Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!ImGui::CollapsingHeader("Effect Rows", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (sequence.effectTracks.empty())
    { ImGui::TextWrapped("Choose a V2 Group or Leaf in Object Resources and Append Effect at Motion End."); return; }
    m_SelectedEffectRow = (std::min)(m_SelectedEffectRow, sequence.effectTracks.size() - 1u);
    for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
    {
        const auto& row = sequence.effectTracks[index];
        const auto label = row.resourceId + "##" + row.effectTrackId;
        if (ImGui::Selectable(label.c_str(), m_SelectedEffectRow == index)) m_SelectedEffectRow = index;
    }
    auto& row = sequence.effectTracks[m_SelectedEffectRow];
    bool changed = false;
    int timing = row.timing == "MOTION_END" ? 0 : 1;
    if (ImGui::Combo("Effect Trigger", &timing, "Motion End\0At Time\0"))
    {
        row.timing = timing == 0 ? "MOTION_END" : "TIME";
        row.startMs = timing == 0 ? 0u : sequence.durationMs;
        changed = true;
    }
    if (timing == 1) changed |= EditUInt("Effect Start (ms)", row.startMs, sequence.durationMs);
    else ImGui::TextDisabled("Follows Motion Lifetime: %u ms", sequence.durationMs);
    changed |= EditUInt("Effect Window (ms)", row.durationMs, CWorldSequenceDocument::MAX_DURATION_MS, 1);
    changed |= ImGui::DragFloat3("Effect Offset (m)", &row.positionOffset.x, .01f);
    changed |= ImGui::DragFloat3("Effect Rotation (deg)", &row.rotationDegrees.x, .5f);
    changed |= ImGui::DragFloat3("Effect Scale", &row.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextWrapped("Each emitted object triggers this Effect at its own trajectory position. The Effect stays there after the model ends; its original envelope is preserved.");
    if (ImGui::Button("Remove Selected Effect Row"))
    {
        sequence.effectTracks.erase(sequence.effectTracks.begin() + static_cast<ptrdiff_t>(m_SelectedEffectRow));
        m_SelectedEffectRow = 0;
        changed = true;
    }
    if (changed) Mark_Dirty();
}


void CWorldObjectTool::Refresh_AnimationResources()
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
    {
        m_AnimationResources.clear();
        m_SelectedAnimationClip.clear();
    }
    m_AnimationObjectId = m_SelectedObject;
    m_AnimationModelAssetId = modelAssetId;
    m_AnimationCatalogReady = false;
    if (!resource)
    { m_AnimationResourceStatus = "Select or Create Object first."; return; }
    if (!resource->sequenceInstanceId.empty())
    { m_AnimationResourceStatus = "This placed object uses its existing sequence tracks; it has no separate native clip catalog."; return; }
    if (modelAssetId.empty())
    { m_AnimationResourceStatus = "Select a WModel in Physical Resources to list its native animations."; return; }
    const auto path = CRuntimeAssetRoot::Resolve(modelAssetId);
    std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> catalog;
    std::string status;
    if (path.empty() || !Engine::CWModelDecoder::Read_AnimationCatalog(path, catalog, status))
    {
        m_AnimationResourceStatus = "Animation catalog unavailable: " + modelAssetId + ": " +
            (path.empty() ? "invalid Resources-relative path" : status) + ". Existing object and patterns are unchanged.";
        return;
    }
    std::vector<ANIMATION_RESOURCE> staged;
    for (const auto& clip : catalog)
    {
        // Match the CAnimation clock used by WorldSequenceObject::Sample.
        const double duration = static_cast<double>(clip.durationTicks) / Engine::CAnimation::COOKED_TICK_RATE * 1000.;
        if (clip.name.empty() || !std::isfinite(duration) || duration <= 0.)
        { m_AnimationResourceStatus = "Invalid animation timing: " + modelAssetId + ". Existing patterns are unchanged."; return; }
        staged.push_back({clip.name, duration});
    }
    m_AnimationResources = std::move(staged);
    m_AnimationCatalogReady = true;
    if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == m_SelectedAnimationClip;
    })) m_SelectedAnimationClip.clear();
    m_AnimationResourceStatus = m_AnimationResources.empty() ?
        "This model has no native animation. Assign Model on the parent Object, then author Transform keys or Physics on a child Motion." :
        std::to_string(m_AnimationResources.size()) + " native clips. Select one and Append Clip to the selected Motion.";
}

bool CWorldObjectTool::Stage_SelectedModel(CWorldSequenceDocument& candidate)
{
    auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || !resource->sequenceInstanceId.empty() || !m_AnimationCatalogReady ||
        m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId.empty())
    { m_Status = "Select an available physical WModel for this object first."; return false; }
    // A resource can own several saved patterns. Keep all of their clip bindings valid.
    for (const auto& instance : candidate.Get_Instances())
    {
        const auto* sequence = candidate.Find_Template(instance.templateId);
        if (!sequence) continue;
        for (const auto& binding : instance.bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource->objectId) continue;
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
                    return clip.clipName == animation.clipName;
                }))
                {
                    m_Status = "Model change refused: pattern '" + sequence->displayName + "' uses '" + animation.clipName +
                        "', which is absent from " + m_AnimationModelAssetId + ". Existing model and patterns are unchanged.";
                    return false;
                }
            }
        }
    }
    resource->modelAssetId = m_AnimationModelAssetId;
    resource->animated = !m_AnimationResources.empty();
    return true;
}

bool CWorldObjectTool::Assign_SelectedModel()
{
    if (!m_SelectedInstance.empty())
    { m_Status = "Select the parent Object to assign its shared model."; return false; }
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    CWorldSequenceDocument candidate = m_Document;
    if (!Stage_SelectedModel(candidate)) return false;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Model assignment refused: " + status + ". Existing draft preserved."; return false; }
    const auto pristinePattern = m_PristinePatternId;
    m_Document = std::move(candidate);
    Mark_Dirty();
    // Assigning a model does not edit the newly created pattern's default timing.
    m_PristinePatternId = pristinePattern;
    m_Status = "Shared model assigned. Create or select a Motion, then Append Clip or edit Transform/Physics.";
    return true;
}

bool CWorldObjectTool::Append_SelectedAnimation()
{
    const auto selectedClip = m_SelectedAnimationClip;
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    const auto found = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == selectedClip;
    });
    if (selectedClip.empty() || found == m_AnimationResources.end())
    { m_Status = "Select an available native animation first; the current object pattern is unchanged."; return false; }
    const double nativeMs = std::ceil(found->durationMs);
    if (!std::isfinite(nativeMs) || nativeMs < 1. || nativeMs > CWorldSequenceDocument::MAX_DURATION_MS)
    { m_Status = "The selected animation exceeds the supported 600-second pattern lifetime."; return false; }
    const uint32_t clipMs = static_cast<uint32_t>(nativeMs);
    CWorldSequenceDocument candidate = m_Document;
    auto* instance = candidate.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? candidate.Find_Template(instance->templateId) : nullptr;
    if (!sequence)
    { m_Status = "Select or create a Motion before appending a clip."; return false; }
    const auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || resource->modelAssetId != m_AnimationModelAssetId || !resource->animated)
    { m_Status = "Assign the animated model on the parent Object before appending its clips."; return false; }
    std::string slotId;
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != m_SelectedObject) continue;
        if (!slotId.empty())
        { m_Status = "This pattern has multiple bindings for the selected object; choose a single-object pattern."; return false; }
        slotId = binding.slotId;
    }
    if (slotId.empty())
    { m_Status = "The selected pattern does not bind the selected object. Existing draft preserved."; return false; }
    if (sequence->tracks.size() + sequence->animationTracks.size() + sequence->effectTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { m_Status = "The pattern has reached its track limit. Existing draft preserved."; return false; }
    const bool firstOfSlot = std::none_of(sequence->animationTracks.begin(), sequence->animationTracks.end(), [&](const auto& clip) {
        return clip.slotId == slotId;
    });
    const bool pristine = firstOfSlot && sequence->animationTracks.empty() && m_PristinePatternId == m_SelectedInstance;
    const uint32_t oldDuration = sequence->durationMs;
    const uint32_t startMs = firstOfSlot ? 0u : oldDuration;
    if (startMs > CWorldSequenceDocument::MAX_DURATION_MS - clipMs)
    { m_Status = "Appending this clip would exceed the 600-second pattern lifetime. Existing draft preserved."; return false; }
    const uint32_t duration = pristine ? clipMs : (std::max)(oldDuration, startMs + clipMs);
    for (auto& track : sequence->tracks)
    {
        if (track.keys.empty())
        { m_Status = "The pattern has an empty Transform track. Existing draft preserved."; return false; }
        if (pristine)
            track.keys.back().timeMs = duration;
        else if (duration > oldDuration)
        {
            if (track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { m_Status = "Extending this pattern would exceed its Transform key limit. Existing draft preserved."; return false; }
            // Preserve every authored time; extend only the final held pose.
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
    }
    WORLD_SEQUENCE_ANIMATION_TRACK animation;
    animation.slotId = slotId; animation.clipName = selectedClip; animation.startMs = startMs;
    animation.playbackRate = 1.f; animation.loop = false; animation.holdLastFrame = true;
    sequence->animationTracks.push_back(std::move(animation));
    sequence->durationMs = duration;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Animation append refused: " + status + ". Existing draft preserved."; return false; }
    m_Document = std::move(candidate);
    Mark_Dirty();
    m_Status = "Appended " + selectedClip + " at " + std::to_string(startMs) + " ms. Play in Object Sequencer, tune in Object Detail, then Save.";
    return true;
}

void CWorldObjectTool::Render_AnimationResources()
{
    ImGui::SeparatorText("Animation Resources");
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
        Refresh_AnimationResources();
    if (!resource || !resource->sequenceInstanceId.empty())
    { ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str()); return; }
    if (ImGui::Button("Refresh Animations")) Refresh_AnimationResources();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_SelectedInstance.empty() || !m_AnimationCatalogReady || m_AnimationModelAssetId.empty());
    if (ImGui::Button("Assign Model")) { Assign_SelectedModel(); resource = m_Document.Find_ObjectResource(m_SelectedObject); }
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str());
    if (!modelAssetId.empty()) ImGui::TextWrapped("Model: %s", modelAssetId.c_str());
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (sequence) ImGui::TextWrapped("Selected Motion: %s", sequence->displayName.c_str());
    else ImGui::TextWrapped("Select a child Motion to append clips. Shared model assignment belongs to the parent Object.");
    ImGui::BeginDisabled(!m_AnimationCatalogReady || m_SelectedAnimationClip.empty() || !sequence);
    if (ImGui::Button("Append Clip")) Append_SelectedAnimation();
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##AnimationSearch", "Search native animation", m_AnimationSearch.data(), m_AnimationSearch.size());
    const auto search = Lower(m_AnimationSearch.data());
    const float height = (std::max)(100.f, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("NativeAnimationCatalog", ImVec2(0.f, height), true))
    {
        for (const auto& clip : m_AnimationResources)
        {
            if (!search.empty() && Lower(clip.clipName).find(search) == std::string::npos) continue;
            ImGui::PushID(clip.clipName.c_str());
            const float nameWidth = (std::max)(80.f, ImGui::GetContentRegionAvail().x - 82.f);
            if (ImGui::Selectable(clip.clipName.c_str(), clip.clipName == m_SelectedAnimationClip, 0, ImVec2(nameWidth, 0.f)))
                m_SelectedAnimationClip = clip.clipName;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%.3f ms\n%s", clip.clipName.c_str(), clip.durationMs, m_AnimationModelAssetId.c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%.0f ms", clip.durationMs);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText("Object / Shared Resources");
    bool changed = EditText("Object Name", resource.displayName);
    ImGui::TextDisabled("%s", resource.objectId.c_str());
    const bool alias = !resource.sequenceInstanceId.empty();
    int resourceAnchor = resource.anchorKind == "BOSS" ? 2 : resource.anchorKind == "PLAYER" ? 1 : 0;
    ImGui::BeginDisabled(alias);
    if (ImGui::Combo("Anchor Type", &resourceAnchor, "Map\0Character\0Boss\0"))
        Change_ResourceAnchor(resource, resourceAnchor == 2 ? "BOSS" : resourceAnchor == 1 ? "PLAYER" : "WORLD");
    ImGui::EndDisabled();
    if (resource.anchorKind == "BOSS")
    {
        static constexpr const char* actorIds[] = {
            "BOSS_KAKULSAYDON_G1_KOUKU", "BOSS_KAKULSAYDON_G1_SAYDON",
            "BOSS_KAKULSAYDON_G2_KOUKU", "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "BOSS_KAKULSAYDON_G3_SAYDON"};
        static constexpr const char* actorLabels[] = {
            "Kouku / Gate 1", "Saydon / Gate 1", "Kouku / Gate 2", "Big Saydon / Gate 2", "Saydon / Gate 3"};
        const char* selected = resource.anchorBossArchetypeId.empty() ? "Choose Boss" : resource.anchorBossArchetypeId.c_str();
        for (size_t i = 0u; i < std::size(actorIds); ++i)
            if (resource.anchorBossArchetypeId == actorIds[i]) selected = actorLabels[i];
        if (ImGui::BeginCombo("Boss Actor", selected))
        {
            for (size_t i = 0u; i < std::size(actorIds); ++i)
                if (ImGui::Selectable(actorLabels[i], resource.anchorBossArchetypeId == actorIds[i]))
                { resource.anchorBossArchetypeId = actorIds[i]; changed = true; }
            ImGui::EndCombo();
        }
        changed |= EditText("Boss Archetype ID", resource.anchorBossArchetypeId);
        changed |= EditText("BODY Bone", resource.anchorBone);
        ImGui::TextWrapped("Follows the named BODY bone every frame. Empty bone uses the boss root. Hand props use b_wp_1 or b_wp_2. Transform keys edit the local grip offset and rotation.");
    }
    if (alias) ImGui::TextDisabled("Placed objects keep their Map anchor.");
    if (alias)
        ImGui::TextWrapped("Placed object sequence: %s. This editor updates its existing tracks and bindings.", resource.sequenceInstanceId.c_str());
    else
    {
        ImGui::TextWrapped("Model: %s", resource.modelAssetId.empty() ? "Choose a WModel in Physical Resources" : resource.modelAssetId.c_str());
        ImGui::TextWrapped("Diffuse: %s", resource.diffuseTextureAssetId.empty() ? "Embedded model material" : resource.diffuseTextureAssetId.c_str());
        if (!resource.diffuseTextureAssetId.empty() && ImGui::SmallButton("Clear Diffuse Override")) { resource.diffuseTextureAssetId.clear(); changed = true; }
        changed |= ImGui::DragFloat("Model Import Scale", &resource.modelPreScale, .001f, .000001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Object Scale", &resource.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::Checkbox("Animated Model", &resource.animated);
    }
    if (changed) Mark_Dirty();
    if (!alias)
    {
        const auto* initial = m_Document.Find_Instance(resource.defaultMotionInstanceId);
        const auto* initialSequence = initial ? m_Document.Find_Template(initial->templateId) : nullptr;
        if (initialSequence && !initialSequence->tracks.empty() && !initialSequence->tracks.front().keys.empty())
        {
            const auto& first = initialSequence->tracks.front().keys.front();
            auto position = first.positionOffset;
            auto rotation = QuaternionEuler(first.rotationQuaternion);
            ImGui::SeparatorText("Object Transform / All Motion Keys");
            bool transformChanged = ImGui::DragFloat3("Object Position (m)", &position.x, .01f);
            transformChanged |= ImGui::DragFloat3("Object Rotation (deg)", &rotation.x, .25f);
            ImGui::TextWrapped("Position and Rotation adjust every connected Motion key relative to its current pose. Object Scale above applies to all Motions. Use a child Motion for individual keys.");
            if (transformChanged)
            {
                auto candidate = m_Document;
                const auto translationDelta = XMLoadFloat3(&position) - XMLoadFloat3(&first.positionOffset);
                const auto rotationDelta = XMMatrixTranspose(XMMatrixRotationQuaternion(XMLoadFloat4(&first.rotationQuaternion))) *
                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
                std::vector<std::string> editedTemplates;
                for (const auto& id : StateIds(resource))
                {
                    const auto* motion = candidate.Find_Instance(id);
                    auto* sequence = motion ? candidate.Find_Template(motion->templateId) : nullptr;
                    if (!sequence || std::find(editedTemplates.begin(), editedTemplates.end(), sequence->sequenceId) != editedTemplates.end()) continue;
                    // Shared templates cannot be rewritten on behalf of a different Object.
                    for (const auto& other : candidate.Get_Instances())
                        if (other.templateId == sequence->sequenceId)
                            for (const auto& binding : other.bindings)
                                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource.objectId)
                                { m_Status = "This Motion template is shared with another target. Edit its keys separately; Object transform was preserved."; return; }
                    for (auto& track : sequence->tracks)
                        for (auto& key : track.keys)
                        {
                            XMStoreFloat3(&key.positionOffset, XMLoadFloat3(&key.positionOffset) + translationDelta);
                            XMStoreFloat4(&key.rotationQuaternion, XMQuaternionNormalize(XMQuaternionRotationMatrix(
                                XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) * rotationDelta)));
                        }
                    editedTemplates.push_back(sequence->sequenceId);
                }
                std::string status;
                if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
                { m_Status = "Object transform refused: " + status + ". Existing draft preserved."; return; }
                m_Document = std::move(candidate);
                Mark_Dirty();
                m_Status = "Updated the Object transform in every connected Motion. Save preserves these keys.";
                return;
            }
        }
        else ImGui::TextDisabled("Choose a Default Motion to edit Object Position and Rotation here.");
    }
    ImGui::SeparatorText("Connected Motions");
    const auto motions = StateIds(resource);
    const auto* defaultMotion = m_Document.Find_Instance(resource.defaultMotionInstanceId);
    const auto* defaultSequence = defaultMotion ? m_Document.Find_Template(defaultMotion->templateId) : nullptr;
    const char* defaultLabel = defaultSequence ? defaultSequence->displayName.c_str() : "None - choose a Default Motion";
    if (ImGui::BeginCombo("Default Motion", defaultLabel))
    {
        if (ImGui::Selectable("None", resource.defaultMotionInstanceId.empty()))
        { resource.defaultMotionInstanceId.clear(); Mark_Dirty(); }
        for (const auto& id : motions)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            ImGui::BeginDisabled(!instance || !instance->enabled || !sequence);
            if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), resource.defaultMotionInstanceId == id))
            { resource.defaultMotionInstanceId = id; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (!defaultMotion || !defaultMotion->enabled)
        ImGui::TextWrapped("Choose an enabled Default Motion to Append or Preview this Object.");
    ImGui::BeginDisabled(!Preview_Instance());
    if (ImGui::Button("Preview Default")) { Stop_Preview(); m_PreviewAtCharacter = true; m_ClockMs = 0.f; Seek(0.f); m_Playing = m_PreviewActive; }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Stop Preview")) { Stop_Preview(); m_ClockMs = 0.f; }
    if (motions.empty()) ImGui::TextDisabled("No motions yet.");
    for (const auto& id : motions)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        ImGui::PushID(id.c_str());
        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str()))
        { Select_State(id); ImGui::PopID(); return; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
        if (sequence && instance)
        {
            ImGui::TextDisabled("%zu clips | %u ms | %s", sequence->animationTracks.size(),
                sequence->durationMs, MotionEndLabel(instance->motionEnd));
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            {
                const auto* next = m_Document.Find_Instance(instance->nextMotionId);
                const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
                ImGui::TextDisabled("  Next: %s", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion");
            }
        }
        ImGui::PopID();
    }
    ImGui::BeginDisabled(alias);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##NewMotion", "Motion name", m_NewStateName.data(), m_NewStateName.size());
    ImGui::BeginDisabled(!m_NewStateName[0]);
    if (ImGui::Button("Create Motion")) Create_State();
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    if (alias) ImGui::TextWrapped("This placed Object retains its existing Motion and bindings.");
}

void CWorldObjectTool::Render_Detail()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource) { ImGui::TextUnformatted("Select an object resource."); return; }
    if (Preview_Group()) { Render_GroupDetail(*resource); return; }
    if (m_SelectedInstance.empty()) { Render_ObjectDetail(*resource); return; }
    ImGui::TextWrapped("Object: %s", resource->displayName.c_str());
    if (ImGui::Button("Edit Parent Object")) { Select_Object(resource->objectId); return; }
    const bool alias = !resource->sequenceInstanceId.empty();
    bool changed = false;
    auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence) { ImGui::TextWrapped("The selected Motion is unavailable. Select its parent Object to continue."); return; }
    ImGui::SeparatorText("Motion");
    changed |= EditText("Motion Name", sequence->displayName);
    ImGui::TextDisabled("%s", instance->instanceId.c_str());
    changed |= ImGui::Checkbox("Enabled", &instance->enabled);
    if (!alias)
    {
        ImGui::TextDisabled("Creation Anchor: %s", instance->anchorKind == "BOSS" ? "Boss / BODY Bone" : instance->anchorKind == "PLAYER" ? "Character" : "Map");
        changed |= ImGui::DragFloat3(instance->anchorKind == "BOSS" ? "Bone Offset" : instance->anchorKind == "PLAYER" ? "Character Offset" : "Map Position", &instance->position.x, .01f);
        if (ImGui::Button("Use Current Character Position"))
        {
            if (instance->anchorKind == "PLAYER" || instance->anchorKind == "BOSS") { instance->position = {}; changed = true; }
            else if (auto* level = CLevel_KakulSaydonArena::Get_Active()) changed |= level->Try_Get_AuthoringPreviewPlacement(instance->position, m_Status);
            else m_Status = "Player placement requires the active KoukuSaydon arena.";
        }
    }
    const bool supportsSurface = alias && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
    if (supportsSurface || instance->walkableSurface)
    {
        ImGui::SeparatorText("Walkable Surface");
        bool enabled = instance->walkableSurface.has_value();
        if (ImGui::Checkbox("Enable Circular Walking Surface", &enabled))
        {
            if (enabled) instance->walkableSurface = WORLD_SEQUENCE_WALKABLE_SURFACE{};
            else instance->walkableSurface.reset();
            changed = true;
        }
        if (instance->walkableSurface)
        {
            changed |= ImGui::DragFloat("Surface Radius (local m)", &instance->walkableSurface->radiusM,
                .01f, .001f, 1000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Surface Height (local m)", &instance->walkableSurface->localHeightM,
                .001f, -10000.f, 10000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("A horizontal circle centered on the placed model. Values use meters after model import scale; placement scale is applied automatically. Fixed position/scale and Y rotation only. Server Complete Play uses its visible lifetime; blocked ground stays blocked. Save, then publish the Map and Kouku gameplay data.");
        }
    }
    changed |= EditUInt("Start Delay (ms)", instance->startDelayMs, CWorldSequenceDocument::MAX_DURATION_MS);
    changed |= ImGui::DragFloat("Playback Speed", &instance->playbackSpeed, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    uint32_t duration = sequence->durationMs;
    if (EditUInt("Lifetime (ms)", duration, CWorldSequenceDocument::MAX_DURATION_MS, 1))
    {
        // Preserve the key order and endpoint contract even when shortening a state.
        bool valid = true;
        for (const auto& track : sequence->tracks) if (duration + 1u < track.keys.size()) valid = false;
        for (const auto& animation : sequence->animationTracks)
            if (static_cast<size_t>(duration) < std::count_if(sequence->animationTracks.begin(), sequence->animationTracks.end(),
                [&](const auto& other) { return other.slotId == animation.slotId; })) valid = false;
        if (valid)
        {
            for (auto& track : sequence->tracks)
            {
                for (size_t key = 1; key + 1 < track.keys.size(); ++key)
                {
                    const auto scaled = static_cast<uint32_t>(std::llround(static_cast<double>(track.keys[key].timeMs) * duration / sequence->durationMs));
                    track.keys[key].timeMs = (std::clamp)(scaled, track.keys[key - 1].timeMs + 1,
                        duration - static_cast<uint32_t>(track.keys.size() - key - 1));
                }
                if (!track.keys.empty()) track.keys.back().timeMs = duration;
            }
            std::unordered_map<std::string, uint32_t> previousStarts;
            for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
            {
                auto& animation = sequence->animationTracks[index];
                const auto previous = previousStarts.find(animation.slotId);
                const auto remaining = std::count_if(sequence->animationTracks.begin() + index + 1, sequence->animationTracks.end(),
                    [&](const auto& other) { return other.slotId == animation.slotId; });
                const uint32_t scaled = static_cast<uint32_t>(static_cast<double>(animation.startMs) * duration / sequence->durationMs);
                animation.startMs = previous == previousStarts.end() ? 0 : (std::clamp)(scaled,
                    previous->second + 1, duration - static_cast<uint32_t>(remaining) - 1);
                previousStarts[animation.slotId] = animation.startMs;
            }
            for (auto& effect : sequence->effectTracks)
                if (effect.timing == "TIME")
                    effect.startMs = static_cast<uint32_t>(static_cast<uint64_t>(effect.startMs) * duration / sequence->durationMs);
            sequence->durationMs = duration;
            if (sequence->objectMotion.count > 1)
                sequence->objectMotion.intervalMs = (std::min)(sequence->objectMotion.intervalMs, (sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration) / (sequence->objectMotion.count - 1));
            for (auto& emission : sequence->objectMotion.emissions)
                emission.startDelayMs = (std::min)(emission.startDelayMs, sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration);
            changed = true;
        }
        else m_Status = "Lifetime must leave at least one millisecond between every existing key or clip.";
    }
    ImGui::BeginDisabled(alias);
    int motionEnd = static_cast<int>(instance->motionEnd);
    if (ImGui::Combo("On Complete", &motionEnd, "Stop\0Hold Last Pose\0Loop\0Play Motion\0"))
    {
        instance->motionEnd = static_cast<WORLD_SEQUENCE_MOTION_END>(motionEnd);
        if (instance->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) instance->nextMotionId.clear();
        changed = true;
    }
    ImGui::EndDisabled();
    if (alias) ImGui::TextDisabled("Placed Object motions use Stop.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
    {
        const auto* next = m_Document.Find_Instance(instance->nextMotionId);
        const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
        if (ImGui::BeginCombo("Next Motion", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion"))
        {
            for (const auto& id : StateIds(*resource))
            {
                const auto* candidate = m_Document.Find_Instance(id);
                const auto* candidateSequence = candidate ? m_Document.Find_Template(candidate->templateId) : nullptr;
                if (!candidateSequence || candidate->instanceId == instance->instanceId) continue;
                const bool compatible = candidate->enabled && sequence->objectMotion.count == 1u &&
                    candidateSequence->objectMotion.count == 1u && instance->bindings.size() == 1u &&
                    candidate->bindings.size() == 1u && instance->bindings.front().slotId == candidate->bindings.front().slotId;
                ImGui::PushID(id.c_str());
                ImGui::BeginDisabled(!compatible);
                if (ImGui::Selectable(candidateSequence->displayName.c_str(), instance->nextMotionId == id))
                { instance->nextMotionId = id; changed = true; }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("%s%s", id.c_str(), compatible ? "" : "\nRequires an enabled Motion with Count 1 and the same object slot.");
                ImGui::EndDisabled();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::TextWrapped("At Lifetime, continue the same object with this Motion. Each Motion in the link must use Count 1; links cannot form a cycle.");
    }
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD)
        ImGui::TextWrapped("Keep the final transform and animation pose after Lifetime.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP)
        ImGui::TextWrapped("Repeat this Motion after Lifetime. This Loop is saved and also used by Play.");
    else ImGui::TextWrapped("Standalone Play ends the object at Lifetime. A Motion applied by a Result stops at its final pose; the target WORLD cue keeps its original lifetime.");
    int interpolation = sequence->interpolation == WORLD_SEQUENCE_INTERPOLATION::LINEAR ? 0 : 1;
    if (ImGui::Combo("Interpolation", &interpolation, "Linear\0Smooth Step\0"))
    { sequence->interpolation = interpolation == 0 ? WORLD_SEQUENCE_INTERPOLATION::LINEAR : WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP; changed = true; }
    if (!alias && ImGui::CollapsingHeader("Physics / Motion / Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& motion = sequence->objectMotion;
        ImGui::TextWrapped("Motion uses the Lifetime timeline and works without animation clips. Play or drag the ruler to preview, then Save.");
        ImGui::DragFloat("Arc Height (m)", &m_VerticalArcHeight, .05f, .01f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Apply Vertical Arc"))
        {
            const float seconds = sequence->durationMs * .001f;
            const float velocityY = 4.f * m_VerticalArcHeight / seconds;
            const float accelerationY = -8.f * m_VerticalArcHeight / (seconds * seconds);
            if (!std::isfinite(m_VerticalArcHeight) || m_VerticalArcHeight <= 0.f ||
                !std::isfinite(velocityY) || !std::isfinite(accelerationY) ||
                std::abs(velocityY) > 100000.f || std::abs(accelerationY) > 100000.f)
                m_Status = "Vertical arc is too fast for this Lifetime. Increase Lifetime or reduce Arc Height; motion preserved.";
            else
            {
                motion.velocity = {0.f, velocityY, 0.f};
                motion.acceleration = {0.f, accelerationY, 0.f};
                motion.spreadDegrees = 0.f;
                changed = true;
                m_Status = "Vertical arc applied. Count and interval preserved. First emission returns at Lifetime. Effect-bearing motions give every emission a full Lifetime. Save to keep the motion.";
            }
        }
        ImGui::TextDisabled("First emission: apex at %.0f ms; return at %u ms.", sequence->durationMs * .5f, sequence->durationMs);
        ImGui::TextWrapped("Arc Height sets the physics offset above the Transform track. With Effect rows, every emission has a full Lifetime and its own Effect tail. Apply again after changing Lifetime. Velocity and Acceleration remain editable.");
        changed |= ImGui::DragFloat3("Velocity (m/s)", &motion.velocity.x, .05f);
        changed |= ImGui::DragFloat3("Acceleration (m/s2)", &motion.acceleration.x, .05f);
        changed |= ImGui::DragFloat3("Self Rotation (deg/s)", &motion.angularVelocityDegrees.x, .5f);
        changed |= ImGui::DragFloat3("Revolution (deg/s)", &motion.revolutionDegreesPerSecond.x, .5f);
        changed |= ImGui::DragFloat3("Revolution Offset (m)", &motion.revolutionOffset.x, .05f);
        auto& emissions = motion.emissions;
        ImGui::BeginDisabled(!emissions.empty());
        changed |= EditUInt("Count", motion.count, 128, 1);
        const uint32_t maxInterval = motion.count > 1 ? (sequence->effectTracks.empty() ? sequence->durationMs - 1 :
            CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs) / (motion.count - 1) : CWorldSequenceDocument::MAX_DURATION_MS;
        if (motion.intervalMs > maxInterval) { motion.intervalMs = maxInterval; changed = true; }
        changed |= EditUInt("Creation Interval (ms)", motion.intervalMs, maxInterval);
        changed |= ImGui::DragFloat(sequence->effectTracks.empty() ? "Spread (deg)" : "Horizontal Spread (deg)", &motion.spreadDegrees, .5f, 0.f, sequence->effectTracks.empty() ? 180.f : 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        changed |= ImGui::DragFloat3("Spawn Half Extents (m)", &motion.spawnHalfExtents.x, .05f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextDisabled("Width / height / depth = twice these values. Set Y to 0 for a ground rectangle.");
        changed |= EditUInt("Seed", motion.seed, INT_MAX);
        ImGui::SeparatorText("Authored Emissions");
        ImGui::TextWrapped("Rows replace the seeded spread. Each row replays this Motion's keys, physics and revolution from its own local offset, yaw and delay. Leave the list empty to keep Count / Creation Interval / Spread.");
        const uint32_t maxDelay = sequence->effectTracks.empty() ? sequence->durationMs - 1 : CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs;
        size_t removeRow = emissions.size(), duplicateRow = emissions.size();
        if (!emissions.empty() && ImGui::BeginTable("AuthoredEmissions", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.f);
            ImGui::TableSetupColumn("Offset X (m)");
            ImGui::TableSetupColumn("Offset Y (m)");
            ImGui::TableSetupColumn("Offset Z (m)");
            ImGui::TableSetupColumn("Yaw (deg)");
            ImGui::TableSetupColumn("Start Delay (ms)");
            ImGui::TableSetupColumn("##rowActions", ImGuiTableColumnFlags_WidthFixed, 84.f);
            ImGui::TableHeadersRow();
            for (size_t row = 0; row < emissions.size(); ++row)
            {
                auto& emission = emissions[row];
                ImGui::PushID(static_cast<int>(row));
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row);
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetX", &emission.positionOffset.x, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetY", &emission.positionOffset.y, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetZ", &emission.positionOffset.z, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##yaw", &emission.yawDegrees, .5f, -36000.f, 36000.f, "%.1f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= EditUInt("##delay", emission.startDelayMs, static_cast<int>(maxDelay));
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Dup")) duplicateRow = row;
                ImGui::SameLine();
                if (ImGui::SmallButton("Del")) removeRow = row;
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (duplicateRow < emissions.size() && emissions.size() < 128u)
        { emissions.insert(emissions.begin() + duplicateRow + 1, emissions[duplicateRow]); changed = true; }
        if (removeRow < emissions.size()) { emissions.erase(emissions.begin() + removeRow); changed = true; }
        ImGui::BeginDisabled(emissions.size() >= 128u);
        if (ImGui::Button("Add Emission"))
        {
            WORLD_SEQUENCE_OBJECT_EMISSION emission;
            if (!emissions.empty()) emission = emissions.back();
            emissions.push_back(emission);
            changed = true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(emissions.empty());
        if (ImGui::Button("Clear Emissions")) { emissions.clear(); changed = true; }
        ImGui::EndDisabled();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragInt("Ring Count", &m_RingCount, 1.f, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragFloat("Ring Start (deg)", &m_RingStartDegrees, .5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        if (ImGui::Button("Distribute on Ring"))
        {
            /* A row's yaw also turns Revolution Offset, so a row at phase p rides
               R(p)*o and an offset of R(p)*o puts every row on one circle centred
               on the saved position itself. */
            const vector_t orbitOffset = XMLoadFloat3(&motion.revolutionOffset);
            emissions.clear();
            for (int index = 0; index < m_RingCount; ++index)
            {
                WORLD_SEQUENCE_OBJECT_EMISSION emission;
                emission.yawDegrees = m_RingStartDegrees + 360.f * static_cast<float>(index) / static_cast<float>(m_RingCount);
                XMStoreFloat3(&emission.positionOffset, XMVector3TransformNormal(orbitOffset,
                    XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees))));
                emissions.push_back(emission);
            }
            changed = true;
        }
        ImGui::TextDisabled("Ring rows circle the saved position at this Motion's Revolution Offset radius; use Revolution (deg/s) for the orbit speed.");
        if (!emissions.empty())
        {
            motion.count = static_cast<uint32_t>(emissions.size());
            motion.intervalMs = 0u;
            motion.spreadDegrees = 0.f;
        }
    }
    if (changed) Mark_Dirty();
    if (!alias) Render_EffectRows(*sequence);
    ImGui::SeparatorText("Selected Key");
    Render_KeyEditor(*sequence);
}

void CWorldObjectTool::Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (EditText("Motion Name", resource.displayName)) Mark_Dirty();
    ImGui::TextWrapped("All %zu rows are open together in Object Sequencer. Edit each row's Map Position and Orbit Radius there, then Save.", resource.motionInstanceIds.size());
    ImGui::TextWrapped("The original motion settings are shared, not copied. No need to select individual resources.");
}

void CWorldObjectTool::Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    ImGui::SameLine();
    if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter)) m_PreviewDirty = m_PreviewActive;
    ImGui::TextWrapped("%zu rows / one timeline. Edit below; Play and Save apply to all rows together.", resource.motionInstanceIds.size());
    if (!m_PreviewStatus.empty()) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    const float span = PreviewSpanMs();
    float clock = m_ClockMs;
    if (ImGui::SliderFloat("Time (ms)", &clock, 0.f, span, "%.0f")) Seek(clock);
    bool changed = false;
    for (const auto& id : resource.motionInstanceIds)
    {
        auto* instance = m_Document.Find_Instance(id);
        auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) { ImGui::TextWrapped("Missing motion: %s", id.c_str()); continue; }
        ImGui::PushID(id.c_str());
        ImGui::SeparatorText(sequence->displayName.c_str());
        const float start = static_cast<float>(instance->startDelayMs);
        const float duration = static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
        const float timelineSpan = (std::max)(1.f, span);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = (std::max)(1.f, ImGui::GetContentRegionAvail().x);
        ImGui::InvisibleButton("Timeline", ImVec2(width, 16.f));
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + 16.f), IM_COL32(30, 35, 43, 255));
        const float left = origin.x + width * (std::clamp)(start / timelineSpan, 0.f, 1.f);
        const float right = origin.x + width * (std::clamp)((start + duration) / timelineSpan, 0.f, 1.f);
        draw->AddRectFilled(ImVec2(left, origin.y + 2.f), ImVec2(right, origin.y + 14.f),
            instance->enabled ? IM_COL32(65, 111, 143, 255) : IM_COL32(65, 65, 65, 255));
        const float cursor = origin.x + width * (std::clamp)(m_ClockMs / timelineSpan, 0.f, 1.f);
        draw->AddLine(ImVec2(cursor, origin.y), ImVec2(cursor, origin.y + 16.f), IM_COL32(255, 216, 48, 255), 2.f);
        if (ImGui::IsItemActive() && ImGui::IsMouseDown(0))
            Seek((std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / width, 0.f, 1.f) * span);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%.0f - %.0f ms | Drag to seek every row", start, start + duration);
        changed |= ImGui::Checkbox("Enabled", &instance->enabled);
        ImGui::SameLine();
        ImGui::Text("Count: %u | %.0f - %.0f ms", sequence->objectMotion.EmissionCount(), start, start + duration);
        if (ImGui::BeginTable("RowOffsets", 2, ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableNextColumn();
            changed |= ImGui::DragFloat3("Map Position", &instance->position.x, .05f, -100000.f, 100000.f, "%.3f");
            ImGui::TableNextColumn();
            auto& motion = sequence->objectMotion;
            const float oldRadius = std::hypot(motion.revolutionOffset.x, motion.revolutionOffset.z);
            float radius = oldRadius;
            ImGui::BeginDisabled(oldRadius < .001f);
            if (ImGui::DragFloat("Orbit Radius (m)", &radius, .05f, .01f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
            {
                const float ratio = radius / oldRadius;
                motion.revolutionOffset.x *= ratio; motion.revolutionOffset.z *= ratio;
                for (auto& emission : motion.emissions)
                { emission.positionOffset.x *= ratio; emission.positionOffset.z *= ratio; }
                changed = true;
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("Scales the horizontal orbit and authored emission offsets together, keeping the saved centre and phases.");
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
    if (changed) Mark_Dirty();
}

void CWorldObjectTool::Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    ImGui::SeparatorText(sequence.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine(); if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* selectedInstance = m_Document.Find_Instance(m_SelectedInstance);
    if (selectedInstance && selectedInstance->anchorKind == "WORLD")
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter))
        {
            m_PreviewDirty = m_PreviewActive;
            if (m_PreviewActive) Seek(m_ClockMs);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Preview at the current character without changing the saved Map position. Clear to preview the authored position.");
    }
    if (selectedInstance) ImGui::TextDisabled("On Complete: %s", MotionEndLabel(selectedInstance->motionEnd));
    if (m_PreviewActive) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    if (m_ClockMs >= SpanMs() && selectedInstance &&
        (selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP || selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        ImGui::TextWrapped("Completion motion active. This timeline still edits the selected Motion's first Lifetime; seek to 0 to restart it.");
    float clock = (std::min)(m_ClockMs, SpanMs());
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, (std::max)(1.f, SpanMs()), "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    const float rowHeight = 32.f;
    const uint32_t timelineDuration = sequence.PresentationSpanMs();
    const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, timelineDuration * m_Zoom * .001f);
    const float pixelsPerMs = width / timelineDuration;
    const bool showPhysics = resource && resource->sequenceInstanceId.empty();
    const float tracksHeight = rowHeight * static_cast<float>(sequence.tracks.size() + sequence.animationTracks.size() + sequence.effectTracks.size());
    const float height = 28.f + tracksHeight + (showPhysics ? 64.f : 0.f);
    if (ImGui::BeginChild("ObjectTimeline", ImVec2(0, (std::max)(110.f, ImGui::GetContentRegionAvail().y)), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), timelineDuration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
        {
            const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
            if (instance) Seek(instance->startDelayMs + local / instance->playbackSpeed);
        }
        for (size_t index = 0; index < sequence.tracks.size(); ++index)
        {
            auto& track = sequence.tracks[index]; ImGui::PushID(track.slotId.c_str());
            const auto row = ImVec2(origin.x, origin.y + 28.f + rowHeight * index);
            CompositionTimeline::DrawBox(draw, row, ImVec2(row.x + sequence.durationMs * pixelsPerMs, row.y + 25.f),
                IM_COL32(61, 107, 141, 255), m_SelectedTrack == index, track.slotId.c_str(), false, false);
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, ImVec2(row.x + width, row.y + 25.f)) && ImGui::IsMouseClicked(0))
            { m_SelectedTrack = index; m_SelectedKey = 0; }
            for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
            {
                auto& key = track.keys[keyIndex]; const float x = row.x + key.timeMs * pixelsPerMs;
                const float y = row.y + 12.f;
                const ImU32 color = m_SelectedTrack == index && m_SelectedKey == keyIndex ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE;
                draw->AddQuadFilled(ImVec2(x, y - 6), ImVec2(x + 6, y), ImVec2(x, y + 6), ImVec2(x - 6, y), color);
                ImGui::PushID(static_cast<int>(keyIndex)); ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, row.x, row.x + width - 14.f), row.y + 4.f));
                ImGui::InvisibleButton("Key", ImVec2(14, 18));
                if (ImGui::IsItemClicked()) { m_SelectedTrack = index; m_SelectedKey = keyIndex; }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                {
                    const auto moved = static_cast<int>((ImGui::GetIO().MousePos.x - row.x) / pixelsPerMs);
                    const uint32_t time = static_cast<uint32_t>((std::clamp)(moved, static_cast<int>(track.keys[keyIndex - 1].timeMs + 1), static_cast<int>(track.keys[keyIndex + 1].timeMs - 1)));
                    if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            const auto& track = sequence.animationTracks[index];
            uint32_t end = sequence.durationMs;
            for (const auto& next : sequence.animationTracks) if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, next.startMs);
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + index);
            CompositionTimeline::DrawBox(draw, ImVec2(origin.x + track.startMs * pixelsPerMs, y),
                ImVec2(origin.x + end * pixelsPerMs, y + 25.f), IM_COL32(113, 82, 147, 255), false,
                track.displayName.empty() ? track.clipName.c_str() : track.displayName.c_str());
        }
        for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
        {
            const auto& effect = sequence.effectTracks[index];
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + sequence.animationTracks.size() + index);
            const float x = origin.x + sequence.EffectStartMs(effect) * pixelsPerMs;
            const float endX = x + effect.durationMs * pixelsPerMs;
            CompositionTimeline::DrawBox(draw, ImVec2(x, y), ImVec2(endX, y + 25.f),
                IM_COL32(167, 95, 51, 255), m_SelectedEffectRow == index, effect.resourceId.c_str());
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(ImVec2(x, y), ImVec2(endX, y + 25.f)) && ImGui::IsMouseClicked(0))
                m_SelectedEffectRow = index;
        }
        if (showPhysics)
        {
            const float top = origin.y + 28.f + tracksHeight;
            draw->AddRectFilled(ImVec2(origin.x, top), ImVec2(origin.x + width, top + 60.f), IM_COL32(28, 49, 48, 255));
            draw->AddText(ImVec2(origin.x + 5.f, top + 3.f), IM_COL32(136, 227, 198, 255), "Physics Y offset / first emission");
            const auto& motion = sequence.objectMotion;
            const float seconds = sequence.durationMs * .001f;
            const float physicsWidth = sequence.durationMs * pixelsPerMs;
            const auto sampleY = [&motion](float time) { return motion.velocity.y * time + .5f * motion.acceleration.y * time * time; };
            float minimum = (std::min)(0.f, sampleY(seconds));
            float maximum = (std::max)(0.f, sampleY(seconds));
            if (motion.acceleration.y != 0.f)
            {
                const float apex = -motion.velocity.y / motion.acceleration.y;
                if (apex > 0.f && apex < seconds)
                { minimum = (std::min)(minimum, sampleY(apex)); maximum = (std::max)(maximum, sampleY(apex)); }
            }
            const float range = (std::max)(.01f, maximum - minimum);
            if (std::isfinite(motion.velocity.y) && std::isfinite(motion.acceleration.y) &&
                std::isfinite(minimum) && std::isfinite(maximum) && std::isfinite(range))
                for (int segment = 0; segment < 64; ++segment)
                {
                    const float from = static_cast<float>(segment) / 64.f, to = static_cast<float>(segment + 1) / 64.f;
                    draw->AddLine(ImVec2(origin.x + physicsWidth * from, top + 56.f - 30.f * (sampleY(seconds * from) - minimum) / range),
                        ImVec2(origin.x + physicsWidth * to, top + 56.f - 30.f * (sampleY(seconds * to) - minimum) / range), IM_COL32(91, 217, 171, 255), 2.f);
                }
            ImGui::SetCursorScreenPos(ImVec2(origin.x, top));
            ImGui::InvisibleButton("PhysicsSeek", ImVec2(width, 60.f));
            if (ImGui::IsItemActive() && selectedInstance)
            {
                const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
                Seek(selectedInstance->startDelayMs + local / selectedInstance->playbackSpeed);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Drag to seek. Y offset range: %.3f to %.3f m. Added to Transform keys before spread, revolution and anchor rotation.", minimum, maximum);
        }
        const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
        const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
        const float cursorX = origin.x + (std::clamp)(local, 0.f, static_cast<float>(timelineDuration)) * pixelsPerMs;
        draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, origin.y + height), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!sequence.tracks.empty())
    {
        m_SelectedTrack = (std::min)(m_SelectedTrack, sequence.tracks.size() - 1);
        if (ImGui::BeginCombo("Target Track", sequence.tracks[m_SelectedTrack].slotId.c_str()))
        {
            for (size_t index = 0; index < sequence.tracks.size(); ++index)
                if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(), index == m_SelectedTrack)) { m_SelectedTrack = index; m_SelectedKey = 0; }
            ImGui::EndCombo();
        }
        auto& track = sequence.tracks[m_SelectedTrack];
        if (!track.keys.empty())
        {
            m_SelectedKey = (std::min)(m_SelectedKey, track.keys.size() - 1);
            auto& key = track.keys[m_SelectedKey];
            if (ImGui::BeginCombo("Keyframe", (std::to_string(key.timeMs) + " ms").c_str()))
            {
                for (size_t index = 0; index < track.keys.size(); ++index)
                    if (ImGui::Selectable((std::to_string(track.keys[index].timeMs) + " ms").c_str(), m_SelectedKey == index)) m_SelectedKey = index;
                ImGui::EndCombo();
            }
            auto& selected = track.keys[m_SelectedKey];
            const bool endpoint = m_SelectedKey == 0 || m_SelectedKey + 1 == track.keys.size();
            bool changed = false;
            ImGui::BeginDisabled(endpoint);
            if (!endpoint) changed |= EditUInt("Key Time (ms)", selected.timeMs, track.keys[m_SelectedKey + 1].timeMs - 1, track.keys[m_SelectedKey - 1].timeMs + 1);
            else ImGui::Text("Endpoint: %u ms", selected.timeMs);
            ImGui::EndDisabled();
            changed |= ImGui::DragFloat3("Position Offset (m)", &selected.positionOffset.x, .01f);
            auto degrees = QuaternionEuler(selected.rotationQuaternion);
            if (ImGui::DragFloat3("Rotation Offset (deg)", &degrees.x, .25f))
            {
                XMStoreFloat4(&selected.rotationQuaternion, XMQuaternionRotationRollPitchYaw(
                    XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
                changed = true;
            }
            changed |= ImGui::DragFloat3("Scale Multiplier", &selected.scaleMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Visible", &selected.visible);
            if (changed) Mark_Dirty();
            ImGui::BeginDisabled(endpoint);
            if (ImGui::Button("Delete Key")) { track.keys.erase(track.keys.begin() + m_SelectedKey); m_SelectedKey = 0; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT || sequence.durationMs < 2);
            if (ImGui::Button("Add Key at Cursor"))
            {
                const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
                const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
                const uint32_t time = static_cast<uint32_t>((std::clamp)(local, 1.f, static_cast<float>(sequence.durationMs - 1)));
                auto at = std::lower_bound(track.keys.begin(), track.keys.end(), time, [](const auto& value, uint32_t clock) { return value.timeMs < clock; });
                m_SelectedKey = at - track.keys.begin();
                if (at == track.keys.end() || at->timeMs != time)
                {
                    auto added = CWorldSequencePlayer::Sample_Track(sequence, track, static_cast<float>(time)); added.timeMs = time;
                    track.keys.insert(at, added); Mark_Dirty();
                }
            }
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Animation Clips"))
    {
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            auto& clip = sequence.animationTracks[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Slot: %s", clip.slotId.c_str());
            bool changed = EditText("Clip display name", clip.displayName);
            ImGui::TextWrapped("Native clip: %s", clip.clipName.c_str());
            uint32_t minimum = 0, maximum = sequence.durationMs - 1;
            bool first = true;
            for (size_t other = 0; other < sequence.animationTracks.size(); ++other)
            {
                const auto& next = sequence.animationTracks[other];
                if (other == index || next.slotId != clip.slotId) continue;
                if (next.startMs < clip.startMs) { minimum = (std::max)(minimum, next.startMs + 1); first = false; }
                if (next.startMs > clip.startMs) maximum = (std::min)(maximum, next.startMs - 1);
            }
            ImGui::BeginDisabled(first); changed |= EditUInt("Clip Start (ms)", clip.startMs, maximum, minimum); ImGui::EndDisabled();
            changed |= ImGui::DragFloat("Clip Speed", &clip.playbackRate, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Clip Loop", &clip.loop); changed |= ImGui::Checkbox("Hold Last Pose", &clip.holdLastFrame);
            if (changed) Mark_Dirty();
            if (ImGui::SmallButton("Remove Clip"))
            {
                const auto slot = clip.slotId; sequence.animationTracks.erase(sequence.animationTracks.begin() + index);
                for (auto& remaining : sequence.animationTracks) if (remaining.slotId == slot) { remaining.startMs = 0; break; }
                Mark_Dirty(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        ImGui::TextWrapped("Select a native clip in Object Resources, then Append Clip to this Motion.");
    }
}

void CWorldObjectTool::Rebuild_PhysicalTree()
{
    m_PhysicalTree = {};
    const auto search = Lower(m_PhysicalSearch.data());
    for (size_t index = 0; index < m_PhysicalAssets.size(); ++index)
    {
        const auto& asset = m_PhysicalAssets[index];
        if ((m_PhysicalSlot == 0) != (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)) continue;
        if (!search.empty() && Lower(asset.assetId).find(search) == std::string::npos) continue;
        std::vector<std::string> segments;
        for (const auto& segment : std::filesystem::path(asset.assetId).parent_path()) segments.push_back(segment.string());
        InsertResourceTree(m_PhysicalTree, segments, index);
    }
    FinalizeResourceTree(m_PhysicalTree);
}

void CWorldObjectTool::Render_PhysicalResources()
{
    ImGui::SeparatorText("Physical Resources");
    if (!m_SelectedInstance.empty()) ImGui::TextWrapped("Select the parent Object to change its shared model or texture.");
    if (!m_PhysicalScanned)
    {
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
        m_PhysicalScanned = true;
    }
    ImGui::BeginDisabled(m_PhysicalScanRunning);
    if (ImGui::Button("Refresh Files"))
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
    ImGui::EndDisabled();
    if (m_PhysicalScanRunning)
    { ImGui::SameLine(); ImGui::TextDisabled("Scanning files..."); }
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##AssignSlot", &m_PhysicalSlot, "Model (.wmodel)\0Diffuse (.dds)\0")) Rebuild_PhysicalTree();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##PhysicalSearch", "Search full relative path", m_PhysicalSearch.data(), m_PhysicalSearch.size())) Rebuild_PhysicalTree();
    ImGui::TextDisabled("%zu matching files", m_PhysicalTree.iRecursiveLeafCount);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_PhysicalStatus.c_str());
    if (ImGui::BeginChild("PhysicalResourceFolders", ImVec2(0, (std::max)(100.f, ImGui::GetContentRegionAvail().y * .40f)), true))
    {
        RenderResourceTree(m_PhysicalTree, [this](size_t index) {
            if (index >= m_PhysicalAssets.size()) return;
            const auto& asset = m_PhysicalAssets[index];
            ImGui::PushID(asset.assetId.c_str());
            auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
            ImGui::BeginDisabled(!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty());
            if (ImGui::Selectable(asset.fileName.c_str(), asset.assetId == m_SelectedPhysical &&
                (asset.kind != PHYSICAL_RESOURCE_KIND::MODEL || m_AnimationCandidateObjectId == m_SelectedObject)))
            {
                m_SelectedPhysical = asset.assetId;
                if (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)
                {
                    m_AnimationCandidateModelAssetId = asset.assetId;
                    m_AnimationCandidateObjectId = m_SelectedObject;
                    Refresh_AnimationResources();
                    m_Status = "Model candidate selected. Assign Model updates the parent Object; its child motions keep their clip bindings.";
                }
                else
                {
                    resource->diffuseTextureAssetId = asset.assetId;
                    const auto pristinePattern = m_PristinePatternId;
                    Mark_Dirty();
                    m_PristinePatternId = pristinePattern;
                    m_Status = "Assigned diffuse " + asset.assetId + ". Save stores it on " + resource->displayName + ".";
                }
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", asset.assetId.c_str());
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
}
#endif

```

# G04. 기존 전체 Motion 편집기와 통합 타임라인 연결

G03의 별도 위치·반경 간이 편집은 사용자 요청을 충족하지 않았다. 첨부한 원래 Object Detail의 Physics / Motion / Emission, Authored Emissions와 Selected Key 등 기존 전체 편집기를 그대로 사용한다. 통합 타임라인의 행/키를 선택해도 나머지 모션은 계속 표시·재생한다.

## 파일·소유자·불변식

- `Client/Public/WorldObjectTool.h`: `m_SelectedGroup`은 재생할 통합 resource stable ID다. 기존 `m_SelectedObject/m_SelectedInstance`는 상세 편집 대상 member다. preview scope와 editing selection의 수명을 분리하는 UI 상태이며 저장 데이터 필드가 아니다.
- `Client/Private/WorldObjectTool.cpp`: `Preview_Group/Preview_Instance`는 통합 scope 우선, `Select_Object`는 그룹의 첫 member를 편집 대상으로 선택, `Select_State` 그룹 분기는 member만 바꾸고 Stop/clock reset을 하지 않는다. 일반 resource 선택은 그룹에서 나간다. Load Source는 기존 통합 선택을 다시 연다.
- `Render_Detail`은 통합 이름/member 선택 다음 기존 전체 Motion 편집기로 이어진다. 기존 Mark_Dirty/Save/publisher 경로를 그대로 사용하며 간이 편집 필드를 따로 만들지 않는다. 그룹의 기존 WORLD/STOP 계약은 유지한다.
- `Render_GroupSequence`는 기존 CompositionTimeline DrawRuler/DrawBox로 모든 member Transform 키, animation/effect 행과 단일 시계를 표시한다. 키 선택/내부 키 시간 드래그는 해당 member를 편집한다. 시간은 member delay와 playbackSpeed를 고려해 전역/로컬로 변환한다. 재생은 기존 다중 instance Object preview가 소유한다.
- 기존 `Render_Sequence/Render_KeyEditor/Physics/Emission`은 재사용한다. 데이터·모델·셰이더·Level/player·Server 변경은 없으며 새 C++ 파일이나 프로젝트/filter 등록도 없다.
- 기존 Python source integration guard에 통합 선택과 전체 편집기의 연결을 검사하는 회귀 사례를 추가한다. 이는 화면 실행 하네스가 아니며 실제 UI는 사용자가 판정한다.

## 검증과 사용

동시 수정 hash 검사와 기존 UTF-8/BOM/CRLF 보존 후 반영한다. Client Debug C++ 컴파일 및 실제 Link, source integration 검사, 공식 WorldSequences Check, JSON/XML parse와 diff check를 수행한다. user가 Client 종료를 확인한 뒤 빌드하며 FXC target은 실행하지 않는다.

사용자는 외곽불_전체를 클릭하여 전체 표시를 확인하고 타임라인 행/키 또는 Object Detail의 Editing Motion을 선택한다. 오른쪽 기존 전체 편집기에서 수정하고 공통 Play/seek 및 Save/Reload를 확인한다. 멤버 선택만으로 시간/재생 상태가 초기화되지 않는지와 개수·오프셋·키 수정 동안 전체 표시가 유지되는지를 수동 확인한다.


## Client/Public/WorldObjectTool.h G04 전체 반영 코드

```cpp
#pragma once

#include "WorldSequenceDocument.h"
#include "PhysicalResourceCatalog.h"
#include "CompositionResourceTree.h"
#include "EffectV2_Catalog.h"

#include <array>
#include <filesystem>

#ifdef _DEBUG
namespace Client
{
class CLevel_KakulSaydonArena;

class CWorldObjectTool final
{
public:
    ~CWorldObjectTool();
    void Open();
    void Deactivate();
    void Update(f32_t seconds, bool_t active);
    void Render();
    bool_t Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    // Only a saved document may become a Workbench resource inventory.
    const CWorldSequenceDocument* Get_SavedDocument() const { return m_Ready ? &m_SavedDocument : nullptr; }
    uint64_t Get_SavedGeneration() const { return m_SavedGeneration; }

private:
    bool Load_Source();
    bool Save_Source();
    bool Matches_SourceBaseline();
    void Start_Publish();
    void Poll_Publish();
    void Mark_Dirty();
    void Stop_Preview();
    bool Begin_Preview();
    const WORLD_SEQUENCE_INSTANCE* Preview_Instance() const;
    void Seek(f32_t clockMs);
    f32_t SpanMs() const;
    f32_t PreviewSpanMs() const;
    void Select_Object(const std::string& id);
    void Select_State(const std::string& id);
    std::vector<std::string> StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const;
    bool Create_Object();
    void Create_State();
    bool Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& stateName,
        WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance);
    void Change_ResourceAnchor(WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind);
    void Render_WindowMenu();
    void Render_Toolbar();
    void Render_Resources();
    void Refresh_AnimationResources();
    void Render_AnimationResources();
    void Render_EffectResources();
    bool Append_SelectedEffect();
    void Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Append_SelectedAnimation();
    bool Stage_SelectedModel(CWorldSequenceDocument& candidate);
    bool Assign_SelectedModel();
    void Render_Detail();
    void Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    const WORLD_SEQUENCE_OBJECT_RESOURCE* Preview_Group() const;
    void Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_PhysicalResources();
    void Rebuild_PhysicalTree();

    bool m_Open = false;
    bool m_ResourcesOpen = true;
    bool m_SequencerOpen = true;
    bool m_DetailOpen = true;
    bool m_ResetLayoutRequested = false;
    bool m_InteractionRequested = false;
    bool m_PreviewAtCharacter = true;
    bool m_Ready = false;
    bool m_Dirty = false;
    bool m_PreviewActive = false;
    bool m_PreviewDirty = false;
    bool m_Playing = false;
    CLevel_KakulSaydonArena* m_PreviewLevel = nullptr;
    f32_t m_ClockMs = 0.f;
    f32_t m_VerticalArcHeight = 2.f;
    // Distribute on Ring preset inputs; rows are the saved truth, not these.
    int m_RingCount = 10;
    f32_t m_RingStartDegrees = 0.f;
    f32_t m_Zoom = 100.f;
    CWorldSequenceDocument m_Document;
    CWorldSequenceDocument m_SavedDocument;
    WORLD_SEQUENCE_PLACEMENT_MAP m_MapTargets;
    WORLD_SEQUENCE_DEPLOY_MAP m_DeployTargets;
    std::filesystem::path m_SourcePath;
    std::filesystem::path m_PlacementPath;
    std::filesystem::path m_DeployPath;
    std::string m_SourceBytes;
    std::string m_PlacementBytes;
    std::string m_DeployBytes;
    std::string m_Status;
    std::string m_PreviewStatus;
    std::string m_SelectedObject;
    // Preview scope stays on the combined resource while the editor selects a member.
    std::string m_SelectedGroup;
    std::string m_SelectedInstance;
    size_t m_SelectedTrack = 0;
    size_t m_SelectedKey = 0;
    uint64_t m_SavedGeneration = 0;
    std::array<char, 128> m_NewObjectName{};
    int m_NewObjectAnchor = 0;
    bool m_CreateObjectFailed = false;
    std::array<char, 128> m_NewStateName{};
    std::string m_PristinePatternId;
    struct ANIMATION_RESOURCE
    {
        std::string clipName;
        double durationMs = 0.;
    };
    std::vector<ANIMATION_RESOURCE> m_AnimationResources;
    std::string m_AnimationObjectId;
    std::string m_AnimationModelAssetId;
    std::string m_AnimationCandidateModelAssetId;
    std::string m_AnimationCandidateObjectId;
    bool m_AnimationCatalogReady = false;
    std::string m_AnimationResourceStatus;
    std::string m_SelectedAnimationClip;
    std::array<char, 256> m_AnimationSearch{};
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> m_EffectResources;
    std::string m_SelectedEffectResource;
    EFFECT_V2_RESOURCE_KIND m_SelectedEffectKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    std::string m_EffectResourceStatus;
    bool m_EffectInventoryLoaded = false;
    size_t m_SelectedEffectRow = 0;
    std::array<char, 256> m_EffectSearch{};
    std::array<char, 256> m_ObjectSearch{};
    std::array<char, 256> m_PhysicalSearch{};
    std::vector<PHYSICAL_RESOURCE_ASSET> m_PhysicalAssets;
    COMPOSITION_RESOURCE_TREE_NODE m_PhysicalTree;
    std::string m_PhysicalStatus;
    std::string m_SelectedPhysical;
    bool m_PhysicalScanned = false;
    bool m_PhysicalScanRunning = false;
    CPhysicalResourceScan m_PhysicalScan;
    int m_PhysicalSlot = 0;
    HANDLE m_PublishProcess = nullptr;
    std::filesystem::path m_PublishLog;
};
}
#endif

```

## Client/Private/WorldObjectTool.cpp G04 전체 반영 코드

```cpp
#include "imgui.h"
#include "WorldObjectTool.h"

#ifdef _DEBUG
#include "CompositionTimeline.h"
#include "Animation.h"
#include "BinaryAsset/WModelDecoder.h"
#include "RuntimeAssetRoot.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldSequencePlayer.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>

namespace
{
constexpr const char* AREA_ID = "LV_LUT_MIDNIGHTC_ED";

bool ReadSource(const std::filesystem::path& path, std::string& bytes,
    std::string& status, const bool optional = false)
{
    std::error_code error;
    if (optional && !std::filesystem::exists(path, error) && !error)
    { bytes.clear(); return true; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 64u * 1024u * 1024u)
    { status = "Cannot read bounded authoring source: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot open source: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (input.bad() || bytes.size() != size)
    { status = "Source changed or failed while reading: " + path.string(); return false; }
    return true;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool EditText(const char* label, std::string& value, const size_t maxBytes = 128)
{
    char text[1024]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, (std::min)(sizeof(text), maxBytes + 1))) return false;
    value = text;
    return true;
}

bool EditUInt(const char* label, uint32_t& value, const int maximum, const int minimum = 0)
{
    int number = static_cast<int>((std::min)(value, static_cast<uint32_t>(INT_MAX)));
    if (!ImGui::DragInt(label, &number, 1.f, minimum, maximum, "%d", ImGuiSliderFlags_AlwaysClamp)) return false;
    value = static_cast<uint32_t>((std::clamp)(number, minimum, maximum));
    return true;
}

const char* MotionEndLabel(const Client::WORLD_SEQUENCE_MOTION_END motionEnd)
{
    switch (motionEnd)
    {
    case Client::WORLD_SEQUENCE_MOTION_END::STOP: return "Stop";
    case Client::WORLD_SEQUENCE_MOTION_END::HOLD: return "Hold Last Pose";
    case Client::WORLD_SEQUENCE_MOTION_END::LOOP: return "Loop";
    case Client::WORLD_SEQUENCE_MOTION_END::NEXT: return "Play Motion";
    default: return "Unknown";
    }
}

float3_t QuaternionEuler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
}

using namespace Client;

CWorldObjectTool::~CWorldObjectTool()
{
    Stop_Preview();
    if (m_PublishProcess) CloseHandle(m_PublishProcess);
}

void CWorldObjectTool::Open()
{
    m_Open = true;
    m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
    if (!m_Ready) Load_Source();
}

bool CWorldObjectTool::Consume_InteractionRequest()
{
    const bool requested = m_InteractionRequested;
    m_InteractionRequested = false;
    return requested;
}

void CWorldObjectTool::Deactivate()
{
    Stop_Preview();
}

bool CWorldObjectTool::Load_Source()
{
    const auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { m_Status = "Enter KoukuSaydon, then Reload Source to edit and preview world objects."; return false; }
    const auto directory = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / AREA_ID);
    const auto sourcePath = directory / (std::string(AREA_ID) + ".worldsequences.json");
    const auto placementPath = directory / (std::string(AREA_ID) + ".mapplacements");
    const auto deployPath = directory / (std::string(AREA_ID) + ".deployplacements");
    std::string sourceBefore, mapBefore, deployBefore;
    if (!ReadSource(sourcePath, sourceBefore, m_Status) ||
        !ReadSource(placementPath, mapBefore, m_Status) ||
        !ReadSource(deployPath, deployBefore, m_Status, true)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP map;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    level->Get_WorldObjectValidationTargets(map, deploy);
    CWorldSequenceDocument staged;
    if (!staged.Load(sourcePath, AREA_ID, map, deploy, m_Status)) return false;
    std::string sourceAfter, mapAfter, deployAfter;
    if (!ReadSource(sourcePath, sourceAfter, m_Status) ||
        !ReadSource(placementPath, mapAfter, m_Status) ||
        !ReadSource(deployPath, deployAfter, m_Status, true)) return false;
    if (sourceBefore != sourceAfter || mapBefore != mapAfter || deployBefore != deployAfter)
    { m_Status = "Linked authoring source changed during Reload; existing draft preserved."; return false; }
    Stop_Preview();
    m_Document = std::move(staged);
    m_SavedDocument = m_Document;
    m_MapTargets = std::move(map);
    m_DeployTargets = std::move(deploy);
    m_SourcePath = sourcePath; m_PlacementPath = placementPath; m_DeployPath = deployPath;
    m_SourceBytes = std::move(sourceAfter); m_PlacementBytes = std::move(mapAfter); m_DeployBytes = std::move(deployAfter);
    m_Ready = true; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_AnimationObjectId.clear();
    m_AnimationCandidateObjectId.clear();
    m_AnimationCandidateModelAssetId.clear();
    if (Preview_Group()) m_SelectedObject = m_SelectedGroup;
    if (!m_Document.Find_ObjectResource(m_SelectedObject))
        m_SelectedObject = m_Document.Get_ObjectResources().empty() ? "" : m_Document.Get_ObjectResources().front().objectId;
    Select_Object(m_SelectedObject);
    m_Status = "Source loaded. Save stores Object edits and applies them for the next play.";
    return true;
}

bool CWorldObjectTool::Matches_SourceBaseline()
{
    std::string source, map, deploy;
    if (!ReadSource(m_SourcePath, source, m_Status) ||
        !ReadSource(m_PlacementPath, map, m_Status) ||
        !ReadSource(m_DeployPath, deploy, m_Status, true)) return false;
    if (source != m_SourceBytes || map != m_PlacementBytes || deploy != m_DeployBytes)
    { m_Status = "Save conflict: linked source changed on disk. Draft preserved; Reload Source before saving."; return false; }
    return true;
}

bool CWorldObjectTool::Save_Source()
{
    if (!m_Ready || m_PublishProcess || !Matches_SourceBaseline()) return false;
    auto stagedPath = m_SourcePath;
    stagedPath += L".world-object-" + std::to_wstring(GetCurrentProcessId()) + L".stage";
    // The document owns codec/validation. A separate staging destination permits
    // a readback and the final linked-source CAS immediately before promotion.
    if (!m_Document.Save(stagedPath, m_MapTargets, m_DeployTargets, m_Status)) return false;
    CWorldSequenceDocument verified;
    std::string stagedBytes;
    const bool ready = verified.Load(stagedPath, AREA_ID, m_MapTargets, m_DeployTargets, m_Status) &&
        m_Document.Is_Equivalent(verified) && ReadSource(stagedPath, stagedBytes, m_Status) && Matches_SourceBaseline();
    if (!ready || !MoveFileExW(stagedPath.c_str(), m_SourcePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        std::error_code ignored; std::filesystem::remove(stagedPath, ignored);
        if (ready) m_Status = "Atomic source replacement failed; draft preserved.";
        else if (m_Status.empty()) m_Status = "Staged source readback differs; draft preserved.";
        return false;
    }
    m_SourceBytes = std::move(stagedBytes); m_Document = std::move(verified);
    m_SavedDocument = m_Document; m_Dirty = false; ++m_SavedGeneration;
    m_PristinePatternId.clear();
    m_Status = "Saved; applying World Object runtime data.";
    Start_Publish();
    return true;
}

void CWorldObjectTool::Start_Publish()
{
    if (!m_Ready || m_Dirty || m_PublishProcess) return;
    if (!Matches_SourceBaseline())
    { m_Status = "Saved; apply stopped because linked source changed. Reload Source before retrying."; return; }
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    if (!std::filesystem::is_regular_file(script)) { m_Status = "Saved; apply failed: publisher is missing. Save to retry."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { m_Status = "Saved; apply failed: log folder is unavailable. Save to retry."; return; }
    m_PublishLog = std::filesystem::path(temporary) / (L"LostArk-WorldObject-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(m_PublishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == log) { m_Status = "Saved; apply failed: cannot create log. Save to retry."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == input) { CloseHandle(log); m_Status = "Saved; apply failed: cannot prepare input. Save to retry."; return; }
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
        script.wstring() + L"\" -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = !!CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
    CloseHandle(log); CloseHandle(input);
    if (!started) { m_Status = "Saved; apply failed: cannot start publisher. Save to retry."; return; }
    CloseHandle(process.hThread); m_PublishProcess = process.hProcess;
    m_Status = "Saved; applying World Object runtime data. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Poll_Publish()
{
    if (!m_PublishProcess || WaitForSingleObject(m_PublishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_PublishProcess, &code);
    CloseHandle(m_PublishProcess); m_PublishProcess = nullptr;
    if (code != 0)
    { m_Status = "Saved; apply failed (" + std::to_string(code) + "). Previous runtime preserved. Save to retry. Log: " + m_PublishLog.string(); return; }
    ++m_SavedGeneration;
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        std::string status;
        if (!level->Reload_WorldObjectRuntime(status))
        { m_Status = "Saved and runtime files applied; next-play reload pending: " + status; return; }
    }
    m_Status = "Saved and applied for the next play. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Mark_Dirty()
{
    m_PristinePatternId.clear();
    m_Document.Touch(); m_Dirty = true; m_PreviewDirty = m_PreviewActive;
}

void CWorldObjectTool::Stop_Preview()
{
    if (m_PreviewActive && m_PreviewLevel && m_PreviewLevel == CLevel_KakulSaydonArena::Get_Active())
        m_PreviewLevel->Debug_StopWorldObjectPreview();
    m_PreviewLevel = nullptr; m_PreviewActive = false; m_Playing = false; m_PreviewDirty = false;
}

const WORLD_SEQUENCE_INSTANCE* CWorldObjectTool::Preview_Instance() const
{
    if (Preview_Group()) return nullptr;
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance.empty() && resource ?
        resource->defaultMotionInstanceId : m_SelectedInstance);
    return instance && instance->enabled ? instance : nullptr;
}

const WORLD_SEQUENCE_OBJECT_RESOURCE* CWorldObjectTool::Preview_Group() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedGroup);
    return resource && !resource->motionInstanceIds.empty() ? resource : nullptr;
}

bool CWorldObjectTool::Begin_Preview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) { m_Status = "World object preview requires the active KoukuSaydon arena."; return false; }
    if (const auto* group = Preview_Group())
    {
        if (SpanMs() <= 0.f)
        {
            Stop_Preview(); m_ClockMs = 0.f;
            m_Status = m_PreviewStatus = "All group motions are disabled.";
            return false;
        }
        if (!level->Debug_BeginWorldObjectPreview(m_Document, group->objectId, m_Status, m_PreviewAtCharacter)) return false;
        m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
        return true;
    }
    const auto* instance = Preview_Instance();
    if (!instance) { m_Status = "Choose an enabled Default Motion or select a connected Motion."; return false; }
    if (!level->Debug_BeginWorldObjectPreview(m_Document, instance->instanceId, m_Status, m_PreviewAtCharacter)) return false;
    m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
    return true;
}

f32_t CWorldObjectTool::SpanMs() const
{
    if (const auto* group = Preview_Group())
    {
        float span = 0.f;
        for (const auto& id : group->motionInstanceIds)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance && instance->enabled ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (sequence) span = (std::max)(span, static_cast<float>(instance->startDelayMs) +
                static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed));
        }
        return span;
    }
    const auto* instance = Preview_Instance();
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    return !sequence ? 0.f : static_cast<float>(instance->startDelayMs) +
        static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
}

f32_t CWorldObjectTool::PreviewSpanMs() const
{
    const auto* instance = Preview_Instance();
    if (instance && (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP ||
        instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        return (std::max)(SpanMs(), static_cast<float>(CWorldSequenceDocument::MAX_DURATION_MS));
    return SpanMs();
}

void CWorldObjectTool::Seek(const f32_t clockMs)
{
    m_ClockMs = (std::clamp)(clockMs, 0.f, PreviewSpanMs());
    if ((!m_PreviewActive || m_PreviewDirty) && !Begin_Preview()) { m_Playing = false; return; }
    if (!m_PreviewLevel->Debug_SampleWorldObjectPreview(m_ClockMs, m_PreviewStatus))
    { m_Status = m_PreviewStatus; Stop_Preview(); }
}

void CWorldObjectTool::Update(const f32_t seconds, const bool_t active)
{
    Poll_Publish();
    if (m_PhysicalScanRunning && m_PhysicalScan.Advance())
    {
        if (m_PhysicalScan.Commit(m_PhysicalAssets, m_PhysicalStatus)) Rebuild_PhysicalTree();
        m_PhysicalScanRunning = false;
    }
    if (!active || !m_Open || (m_PreviewLevel && m_PreviewLevel != CLevel_KakulSaydonArena::Get_Active()))
    { Stop_Preview(); return; }
    if (m_PreviewActive && m_PreviewDirty) Seek(m_ClockMs);
    if (!m_Playing || !m_PreviewActive) return;
    const float span = PreviewSpanMs();
    m_ClockMs += (std::max)(0.f, seconds) * 1000.f;
    if (span <= 0.f) { Stop_Preview(); return; }
    if (m_ClockMs >= span) { m_ClockMs = span; m_Playing = false; }
    Seek(m_ClockMs);
}

std::vector<std::string> CWorldObjectTool::StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const
{
    if (!resource.motionInstanceIds.empty()) return resource.motionInstanceIds;
    if (!resource.sequenceInstanceId.empty()) return {resource.sequenceInstanceId};
    std::vector<std::string> ids;
    for (const auto& instance : m_Document.Get_Instances())
        if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
            return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
        })) ids.push_back(instance.instanceId);
    return ids;
}

void CWorldObjectTool::Select_Object(const std::string& id)
{
    Stop_Preview(); m_SelectedObject = id; m_SelectedInstance.clear(); m_ClockMs = 0.f;
    m_SelectedGroup.clear();
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (resource && !resource->motionInstanceIds.empty())
    {
        m_SelectedGroup = resource->objectId;
        m_PreviewAtCharacter = false;
        Select_State(resource->motionInstanceIds.front());
    }
    m_SelectedTrack = 0; m_SelectedKey = 0;
}

void CWorldObjectTool::Select_State(const std::string& id)
{
    if (const auto* group = Preview_Group())
    {
        const auto* instance = m_Document.Find_Instance(id);
        if (std::find(group->motionInstanceIds.begin(), group->motionInstanceIds.end(), id) == group->motionInstanceIds.end() ||
            !instance || instance->bindings.size() != 1u ||
            !m_Document.Find_ObjectResource(instance->bindings.front().targetId))
        { m_Status = "The selected motion does not belong to this combined motion."; return; }
        // Editing another row must not stop, rewind or solo the combined preview.
        m_SelectedObject = instance->bindings.front().targetId;
        m_SelectedInstance = id;
        m_SelectedTrack = 0; m_SelectedKey = 0; m_SelectedEffectRow = 0;
        return;
    }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto motions = resource ? StateIds(*resource) : std::vector<std::string>{};
    if (std::find(motions.begin(), motions.end(), id) == motions.end())
    { m_Status = "The selected motion does not belong to this object."; return; }
    Stop_Preview(); m_SelectedInstance = id; m_SelectedTrack = 0; m_SelectedKey = 0; m_ClockMs = 0.f;
}

bool CWorldObjectTool::Create_Object()
{
    if (!m_NewObjectName[0]) { m_Status = "Enter an object name."; return false; }
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        resource.objectId = "world.object.resource." + std::to_string(index);
        if (!m_Document.Find_ObjectResource(resource.objectId)) break;
    }
    resource.displayName = m_NewObjectName.data();
    resource.anchorKind = m_NewObjectAnchor == 2 ? "BOSS" : m_NewObjectAnchor == 1 ? "PLAYER" : "WORLD";
    if (m_Document.Get_ObjectResources().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World object resource capacity reached."; return false; }
    m_Document.Get_ObjectResources().push_back(resource);
    Mark_Dirty(); Select_Object(resource.objectId);
    m_Status = "Object created. Assign its shared model, then Create Motion in Object Detail. Assign Model before Save.";
    m_NewObjectName[0] = 0;
    m_NewStateName[0] = 0;
    return true;
}

void CWorldObjectTool::Create_State()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty() || !m_NewStateName[0]) return;
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(*resource, m_NewStateName.data(), sequence, instance)) return;
    const bool firstMotion = StateIds(*resource).empty();
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(instance);
    if (firstMotion) resource->defaultMotionInstanceId = instance.instanceId;
    Mark_Dirty(); Select_State(instance.instanceId);
    m_PristinePatternId = instance.instanceId;
    m_NewStateName[0] = 0;
    m_Status = "Motion created. Append Clip or edit Transform/Physics, then Save.";
}

bool CWorldObjectTool::Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const std::string& stateName, WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance)
{
    if (m_Document.Get_Templates().size() >= CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        m_Document.Get_Instances().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World sequence document capacity reached."; return false; }
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        sequence.sequenceId = resource.objectId + ".state." + std::to_string(index);
        instance.instanceId = sequence.sequenceId + ".instance";
        if (!m_Document.Find_Template(sequence.sequenceId) && !m_Document.Find_Instance(instance.instanceId)) break;
    }
    sequence.displayName = stateName; sequence.durationMs = 2000;
    WORLD_SEQUENCE_TRACK track; track.slotId = "object"; track.keys.push_back({});
    WORLD_SEQUENCE_TRANSFORM_KEY end; end.timeMs = sequence.durationMs; track.keys.push_back(end);
    sequence.tracks.push_back(track);
    instance.templateId = sequence.sequenceId;
    instance.anchorKind = resource.anchorKind;
    instance.bindings.push_back({"object", WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE, resource.objectId});
    if (instance.anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level)
        { m_Status = "Map state creation requires the active KoukuSaydon arena."; return false; }
        if (!level->Try_Get_AuthoringPreviewPlacement(instance.position, m_Status))
        {
            m_Status = "Map state needs the current character placement: " + m_Status;
            return false;
        }
    }
    return true;
}

void CWorldObjectTool::Change_ResourceAnchor(
    WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind)
{
    if (resource.anchorKind == anchorKind || !resource.sequenceInstanceId.empty()) return;
    float3_t position{};
    if (anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level || !level->Try_Get_AuthoringPreviewPlacement(position, m_Status))
        { m_Status = "Map anchor needs the current character placement: " + m_Status; return; }
    }
    resource.anchorKind = anchorKind;
    if (anchorKind != "BOSS") { resource.anchorBossArchetypeId.clear(); resource.anchorBone.clear(); }
    for (const auto& id : StateIds(resource))
    {
        auto* instance = m_Document.Find_Instance(id);
        if (!instance || instance->anchorKind == anchorKind) continue;
        instance->anchorKind = anchorKind;
        instance->position = position;
    }
    Mark_Dirty();
    m_Status = anchorKind == "BOSS" ?
        "Boss anchor applied to this resource's states. Choose the boss and BODY bone below." : anchorKind == "PLAYER" ?
        "Character anchor applied to this resource's states; offsets start at the character origin." :
        "Map anchor applied to this resource's states at the current character position.";
}

void CWorldObjectTool::Render()
{
    if (!m_Open) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport ? viewport->WorkPos : ImVec2(0.f, 0.f);
    const ImVec2 available = viewport ? viewport->WorkSize : ImVec2(1600.f, 900.f);
    constexpr float margin = 8.f, gap = 8.f;
    const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
    const float height = (std::max)(1.f, available.y - margin * 2.f);
    const float leftWidth = width * .23f, rightWidth = width * .24f;
    const float centerWidth = width - leftWidth - rightWidth;
    const float leftX = origin.x + margin, centerX = leftX + leftWidth + gap;
    const float rightX = centerX + centerWidth + gap, topY = origin.y + margin;
    const ImGuiCond condition = m_ResetLayoutRequested ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    m_ResetLayoutRequested = false;
    const auto beginPane = [&](const char* name, bool& visible, const ImVec2 position, const ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(name, &visible, ImGuiWindowFlags_MenuBar);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0))
            m_InteractionRequested = true;
        if (expanded) Render_WindowMenu();
        return expanded;
    };

    if (m_ResourcesOpen)
    {
        if (beginPane("Object Resources###WorldObjectResourcesWindow", m_ResourcesOpen,
            {leftX, topY}, {leftWidth, height}))
        {
            if (m_Ready) Render_Resources();
            else
            {
                if (ImGui::Button("Reload Source")) Load_Source();
                ImGui::TextWrapped("%s", m_Status.c_str());
            }
            if (m_Ready) Render_EffectResources();
            Render_PhysicalResources();
            if (m_Ready) Render_AnimationResources();
        }
        ImGui::End();
    }
    if (m_SequencerOpen)
    {
        if (beginPane("Object Sequencer###WorldObjectSequencerWindow", m_SequencerOpen,
            {centerX, topY + height * .57f}, {centerWidth, height * .43f}))
        {
            Render_Toolbar();
            auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (const auto* group = Preview_Group()) Render_GroupSequence(*group);
            else if (sequence) Render_Sequence(*sequence);
            else
            {
                ImGui::TextWrapped("Select a Motion beneath an Object to open its Lifetime timeline. The parent Object edits shared resources only.");
                ImGui::BeginDisabled(); ImGui::Button("Play"); ImGui::SameLine(); ImGui::Button("Append Clip"); ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }
    if (m_DetailOpen)
    {
        if (beginPane("Object Detail###WorldObjectDetailWindow", m_DetailOpen,
            {rightX, topY}, {rightWidth, height}))
        {
            if (m_Ready) Render_Detail();
            else ImGui::TextDisabled("Load Object Resources to edit an object.");
        }
        ImGui::End();
    }
    if (!m_ResourcesOpen && !m_SequencerOpen && !m_DetailOpen) m_Open = false;
    if (!m_Open) Stop_Preview();
}

void CWorldObjectTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Object Resources", nullptr, &m_ResourcesOpen);
        ImGui::MenuItem("Object Sequencer", nullptr, &m_SequencerOpen);
        ImGui::MenuItem("Object Detail", nullptr, &m_DetailOpen);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All")) m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
            m_ResetLayoutRequested = true;
        }
        if (ImGui::MenuItem("Close World Object Tool")) m_Open = false;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void CWorldObjectTool::Render_Toolbar()
{
    if (ImGui::Button("Reload Source"))
    {
        if (m_Dirty) ImGui::OpenPopup("Reload object source?");
        else Load_Source();
    }
    if (ImGui::BeginPopupModal("Reload object source?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload discards this tool's unsaved object/state edits.");
        if (ImGui::Button("Discard and Reload")) { Load_Source(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Ready || m_PublishProcess);
    if (ImGui::Button(m_Dirty ? "Save *" : "Save")) Save_Source();
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CWorldObjectTool::Render_Resources()
{
    if (ImGui::Button("Create Object"))
    {
        m_CreateObjectFailed = false;
        ImGui::OpenPopup("Create Object Resource");
    }
    ImGui::SameLine(); ImGui::TextDisabled("%zu resources", m_Document.Get_ObjectResources().size());
    if (ImGui::BeginPopupModal("Create Object Resource", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputTextWithHint("Name", "New object name", m_NewObjectName.data(), m_NewObjectName.size());
        ImGui::Combo("Anchor Type", &m_NewObjectAnchor, "Map\0Character\0Boss\0");
        ImGui::TextUnformatted(m_NewObjectAnchor == 2 ?
            "Boss: motions follow the chosen boss BODY bone. Select the boss and bone in Object Detail." : m_NewObjectAnchor == 0 ?
            "Map: motions created later use a fixed world anchor." :
            "Character: motions created later use each living character as their anchor.");
        ImGui::TextUnformatted("Creates the parent Object only. Add its motions from Object Detail.");
        ImGui::BeginDisabled(!m_NewObjectName[0]);
        if (ImGui::Button("Create"))
        {
            m_CreateObjectFailed = !Create_Object();
            if (!m_CreateObjectFailed) ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (m_CreateObjectFailed) ImGui::TextWrapped("%s", m_Status.c_str());
        ImGui::EndPopup();
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectSearch", "Search object or motion", m_ObjectSearch.data(), m_ObjectSearch.size());
    const float treeHeight = (std::max)(120.f, ImGui::GetContentRegionAvail().y * .28f);
    if (ImGui::BeginChild("ObjectResourceTree", ImVec2(0.f, treeHeight), true))
    {
        const auto search = Lower(m_ObjectSearch.data());
        const auto stateMatches = [&search](const std::string& id, const WORLD_SEQUENCE_TEMPLATE* sequence)
        {
            const auto searchable = sequence ? sequence->displayName + " " + sequence->sequenceId + " " + id : id;
            return Lower(searchable).find(search) != std::string::npos;
        };
        for (const char* anchor : {"WORLD", "PLAYER", "BOSS"})
        {
            const char* category = std::string(anchor) == "WORLD" ? "Map" : std::string(anchor) == "BOSS" ? "Boss" : "Character";
            size_t count = 0;
            for (const auto& resource : m_Document.Get_ObjectResources()) if (resource.anchorKind == anchor) ++count;
            const std::string categoryLabel = std::string(category) + " (" + std::to_string(count) + ")";
            if (!ImGui::TreeNodeEx(anchor, ImGuiTreeNodeFlags_DefaultOpen, "%s", categoryLabel.c_str())) continue;
            for (const auto& resource : m_Document.Get_ObjectResources())
            {
                if (resource.anchorKind != anchor) continue;
                const auto states = StateIds(resource);
                const bool resourceMatches = search.empty() || Lower(resource.displayName + " " + resource.objectId).find(search) != std::string::npos;
                bool matches = resourceMatches;
                if (!matches)
                    for (const auto& id : states)
                    {
                        const auto* state = m_Document.Find_Instance(id);
                        const auto* sequence = state ? m_Document.Find_Template(state->templateId) : nullptr;
                        if (stateMatches(id, sequence)) { matches = true; break; }
                    }
                if (!matches) continue;
                ImGui::PushID(resource.objectId.c_str());
                if (!resource.motionInstanceIds.empty())
                {
                    // One authoring entry opens every member; it is not a folder of solo previews.
                    if (ImGui::Selectable(resource.displayName.c_str(), m_SelectedGroup == resource.objectId))
                    {
                        Select_Object(resource.objectId);
                        m_SequencerOpen = true;
                        m_DetailOpen = true;
                        Seek(0.f);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open all %zu motions together in Object Sequencer.", states.size());
                    ImGui::PopID();
                    continue;
                }
                const auto label = resource.displayName +
                    (resource.sequenceInstanceId.empty() && resource.modelAssetId.empty() ? " [assign model]" : "");
                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth | (m_SelectedObject == resource.objectId && m_SelectedInstance.empty() ? ImGuiTreeNodeFlags_Selected : 0);
                if (!search.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                const bool open = ImGui::TreeNodeEx("Resource", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked()) Select_Object(resource.objectId);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", resource.objectId.c_str());
                if (open)
                {
                    for (const auto& id : states)
                    {
                        const auto* instance = m_Document.Find_Instance(id);
                        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
                        if (!resourceMatches && !stateMatches(id, sequence)) continue;
                        ImGui::PushID(id.c_str());
                        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), id == m_SelectedInstance))
                        {
                            m_SelectedGroup.clear();
                            m_SelectedObject = resource.objectId;
                            Select_State(id);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
                        ImGui::PopID();
                    }
                    if (states.empty()) ImGui::TextDisabled("No motions - select Object to Create Motion");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_EffectResources()
{
    if (!ImGui::CollapsingHeader("V2 Effects", ImGuiTreeNodeFlags_DefaultOpen)) return;
    const bool reload = ImGui::Button("Reload V2 Effects");
    if (!m_EffectInventoryLoaded || reload)
    {
        std::vector<EFFECT_V2_RESOURCE_SUMMARY> staged;
        if (CEffectV2Catalog::Get().Read_Inventory(staged, m_EffectResourceStatus))
            m_EffectResources = std::move(staged);
        m_EffectInventoryLoaded = true;
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectEffectSearch", "Search smoke / group / leaf", m_EffectSearch.data(), m_EffectSearch.size());
    const auto search = Lower(m_EffectSearch.data());
    if (ImGui::BeginChild("ObjectV2Effects", ImVec2(0.f, 135.f), true))
        for (const auto& effect : m_EffectResources)
        {
            if (!search.empty() && Lower(effect.strDisplayName + " " + effect.strResourceId).find(search) == std::string::npos) continue;
            ImGui::PushID(effect.strResourceId.c_str());
            const auto label = std::string(effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "[Group] " : "[Leaf] ") +
                (effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName);
            ImGui::BeginDisabled(!effect.strStatus.empty());
            if (ImGui::Selectable(label.c_str(), m_SelectedEffectResource == effect.strResourceId && m_SelectedEffectKind == effect.eKind))
            { m_SelectedEffectResource = effect.strResourceId; m_SelectedEffectKind = effect.eKind; }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s\n%s", effect.strResourceId.c_str(), effect.strStatus.c_str());
            ImGui::PopID();
        }
    ImGui::EndChild();
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const bool target = instance && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
    ImGui::BeginDisabled(!target || m_SelectedEffectResource.empty());
    if (ImGui::Button("Append Effect at Motion End")) Append_SelectedEffect();
    ImGui::EndDisabled();
    if (!target) ImGui::TextWrapped("Select an Object's child Motion, then append an Effect row.");
    if (!m_EffectResourceStatus.empty()) ImGui::TextWrapped("%s", m_EffectResourceStatus.c_str());
}

bool CWorldObjectTool::Append_SelectedEffect()
{
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence || instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
    { m_Status = "Select a child Object Motion before appending an Effect."; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(m_SelectedEffectKind, m_SelectedEffectResource, snapshot, m_EffectResourceStatus))
    { m_Status = "Effect Append failed: " + m_EffectResourceStatus; return false; }
    // Match the existing V2 authoring preview span, including particles/trails
    // remaining after emission ends. The row does not stretch leaf envelopes.
    const auto leafSpan = [](const EFFECT_V2_DOCUMENT& document, const uint32_t explicitMs, const bool tailEnabled)
    {
        const auto& params = document.Desc.Params;
        const double rate = (std::max)(.001, static_cast<double>(params.fPlayRate));
        const double emission = explicitMs ? explicitMs : params.fLifetime > 0.f ?
            std::ceil(params.fLifetime * 1000.0 / rate) : 3000.0;
        const double tail = !tailEnabled ? 0.0 : document.eType == EFFECT_V2_TYPE::PARTICLE ?
            params.Particle.vLifetime.y * 1000.0 / rate : document.eType == EFFECT_V2_TYPE::TRAIL ?
            params.Trail.fPointLifetime * 1000.0 / rate : 0.0;
        return emission + std::ceil(tail);
    };
    double span = 0.;
    if (m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        const auto* group = snapshot->Find_Group(m_SelectedEffectResource);
        if (!group) { m_Status = "Selected Effect group is unavailable."; return false; }
        span = group->iDurationMs;
        if (!group->iDurationMs)
            for (const auto& child : group->Children)
            {
                const auto* leaf = snapshot->Find_Document(child.strEffectId);
                if (!leaf) { m_Status = "Selected Effect group child is unavailable."; return false; }
                span = (std::max)(span, child.iStartMs + leafSpan(*leaf, child.iDurationMs,
                    child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE));
            }
    }
    else
    {
        const auto* leaf = snapshot->Find_Document(m_SelectedEffectResource);
        if (!leaf) { m_Status = "Selected Effect leaf is unavailable."; return false; }
        span = leafSpan(*leaf, 0u, true);
    }
    CWorldSequenceDocument staged = m_Document;
    auto* edited = staged.Find_Template(sequence->sequenceId);
    WORLD_SEQUENCE_EFFECT_TRACK row;
    uint32_t serial = 1u;
    do { row.effectTrackId = "effect." + std::to_string(serial++); }
    while (std::any_of(edited->effectTracks.begin(), edited->effectTracks.end(),
        [&](const auto& value) { return value.effectTrackId == row.effectTrackId; }));
    row.slotId = instance->bindings.front().slotId;
    row.resourceKind = m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.resourceId = m_SelectedEffectResource;
    row.durationMs = static_cast<uint32_t>((std::clamp)(std::ceil(span), 1., 600000.));
    edited->effectTracks.push_back(row);
    if (!staged.Validate(m_MapTargets, m_DeployTargets, m_Status)) return false;
    m_SelectedEffectRow = edited->effectTracks.size() - 1u;
    m_Document = std::move(staged);
    Mark_Dirty();
    m_Status = "Effect row appended at Motion End. Adjust it in Effect Rows, then Save.";
    return true;
}

void CWorldObjectTool::Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!ImGui::CollapsingHeader("Effect Rows", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (sequence.effectTracks.empty())
    { ImGui::TextWrapped("Choose a V2 Group or Leaf in Object Resources and Append Effect at Motion End."); return; }
    m_SelectedEffectRow = (std::min)(m_SelectedEffectRow, sequence.effectTracks.size() - 1u);
    for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
    {
        const auto& row = sequence.effectTracks[index];
        const auto label = row.resourceId + "##" + row.effectTrackId;
        if (ImGui::Selectable(label.c_str(), m_SelectedEffectRow == index)) m_SelectedEffectRow = index;
    }
    auto& row = sequence.effectTracks[m_SelectedEffectRow];
    bool changed = false;
    int timing = row.timing == "MOTION_END" ? 0 : 1;
    if (ImGui::Combo("Effect Trigger", &timing, "Motion End\0At Time\0"))
    {
        row.timing = timing == 0 ? "MOTION_END" : "TIME";
        row.startMs = timing == 0 ? 0u : sequence.durationMs;
        changed = true;
    }
    if (timing == 1) changed |= EditUInt("Effect Start (ms)", row.startMs, sequence.durationMs);
    else ImGui::TextDisabled("Follows Motion Lifetime: %u ms", sequence.durationMs);
    changed |= EditUInt("Effect Window (ms)", row.durationMs, CWorldSequenceDocument::MAX_DURATION_MS, 1);
    changed |= ImGui::DragFloat3("Effect Offset (m)", &row.positionOffset.x, .01f);
    changed |= ImGui::DragFloat3("Effect Rotation (deg)", &row.rotationDegrees.x, .5f);
    changed |= ImGui::DragFloat3("Effect Scale", &row.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextWrapped("Each emitted object triggers this Effect at its own trajectory position. The Effect stays there after the model ends; its original envelope is preserved.");
    if (ImGui::Button("Remove Selected Effect Row"))
    {
        sequence.effectTracks.erase(sequence.effectTracks.begin() + static_cast<ptrdiff_t>(m_SelectedEffectRow));
        m_SelectedEffectRow = 0;
        changed = true;
    }
    if (changed) Mark_Dirty();
}


void CWorldObjectTool::Refresh_AnimationResources()
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
    {
        m_AnimationResources.clear();
        m_SelectedAnimationClip.clear();
    }
    m_AnimationObjectId = m_SelectedObject;
    m_AnimationModelAssetId = modelAssetId;
    m_AnimationCatalogReady = false;
    if (!resource)
    { m_AnimationResourceStatus = "Select or Create Object first."; return; }
    if (!resource->sequenceInstanceId.empty())
    { m_AnimationResourceStatus = "This placed object uses its existing sequence tracks; it has no separate native clip catalog."; return; }
    if (modelAssetId.empty())
    { m_AnimationResourceStatus = "Select a WModel in Physical Resources to list its native animations."; return; }
    const auto path = CRuntimeAssetRoot::Resolve(modelAssetId);
    std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> catalog;
    std::string status;
    if (path.empty() || !Engine::CWModelDecoder::Read_AnimationCatalog(path, catalog, status))
    {
        m_AnimationResourceStatus = "Animation catalog unavailable: " + modelAssetId + ": " +
            (path.empty() ? "invalid Resources-relative path" : status) + ". Existing object and patterns are unchanged.";
        return;
    }
    std::vector<ANIMATION_RESOURCE> staged;
    for (const auto& clip : catalog)
    {
        // Match the CAnimation clock used by WorldSequenceObject::Sample.
        const double duration = static_cast<double>(clip.durationTicks) / Engine::CAnimation::COOKED_TICK_RATE * 1000.;
        if (clip.name.empty() || !std::isfinite(duration) || duration <= 0.)
        { m_AnimationResourceStatus = "Invalid animation timing: " + modelAssetId + ". Existing patterns are unchanged."; return; }
        staged.push_back({clip.name, duration});
    }
    m_AnimationResources = std::move(staged);
    m_AnimationCatalogReady = true;
    if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == m_SelectedAnimationClip;
    })) m_SelectedAnimationClip.clear();
    m_AnimationResourceStatus = m_AnimationResources.empty() ?
        "This model has no native animation. Assign Model on the parent Object, then author Transform keys or Physics on a child Motion." :
        std::to_string(m_AnimationResources.size()) + " native clips. Select one and Append Clip to the selected Motion.";
}

bool CWorldObjectTool::Stage_SelectedModel(CWorldSequenceDocument& candidate)
{
    auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || !resource->sequenceInstanceId.empty() || !m_AnimationCatalogReady ||
        m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId.empty())
    { m_Status = "Select an available physical WModel for this object first."; return false; }
    // A resource can own several saved patterns. Keep all of their clip bindings valid.
    for (const auto& instance : candidate.Get_Instances())
    {
        const auto* sequence = candidate.Find_Template(instance.templateId);
        if (!sequence) continue;
        for (const auto& binding : instance.bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource->objectId) continue;
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
                    return clip.clipName == animation.clipName;
                }))
                {
                    m_Status = "Model change refused: pattern '" + sequence->displayName + "' uses '" + animation.clipName +
                        "', which is absent from " + m_AnimationModelAssetId + ". Existing model and patterns are unchanged.";
                    return false;
                }
            }
        }
    }
    resource->modelAssetId = m_AnimationModelAssetId;
    resource->animated = !m_AnimationResources.empty();
    return true;
}

bool CWorldObjectTool::Assign_SelectedModel()
{
    if (!m_SelectedInstance.empty())
    { m_Status = "Select the parent Object to assign its shared model."; return false; }
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    CWorldSequenceDocument candidate = m_Document;
    if (!Stage_SelectedModel(candidate)) return false;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Model assignment refused: " + status + ". Existing draft preserved."; return false; }
    const auto pristinePattern = m_PristinePatternId;
    m_Document = std::move(candidate);
    Mark_Dirty();
    // Assigning a model does not edit the newly created pattern's default timing.
    m_PristinePatternId = pristinePattern;
    m_Status = "Shared model assigned. Create or select a Motion, then Append Clip or edit Transform/Physics.";
    return true;
}

bool CWorldObjectTool::Append_SelectedAnimation()
{
    const auto selectedClip = m_SelectedAnimationClip;
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    const auto found = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == selectedClip;
    });
    if (selectedClip.empty() || found == m_AnimationResources.end())
    { m_Status = "Select an available native animation first; the current object pattern is unchanged."; return false; }
    const double nativeMs = std::ceil(found->durationMs);
    if (!std::isfinite(nativeMs) || nativeMs < 1. || nativeMs > CWorldSequenceDocument::MAX_DURATION_MS)
    { m_Status = "The selected animation exceeds the supported 600-second pattern lifetime."; return false; }
    const uint32_t clipMs = static_cast<uint32_t>(nativeMs);
    CWorldSequenceDocument candidate = m_Document;
    auto* instance = candidate.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? candidate.Find_Template(instance->templateId) : nullptr;
    if (!sequence)
    { m_Status = "Select or create a Motion before appending a clip."; return false; }
    const auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || resource->modelAssetId != m_AnimationModelAssetId || !resource->animated)
    { m_Status = "Assign the animated model on the parent Object before appending its clips."; return false; }
    std::string slotId;
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != m_SelectedObject) continue;
        if (!slotId.empty())
        { m_Status = "This pattern has multiple bindings for the selected object; choose a single-object pattern."; return false; }
        slotId = binding.slotId;
    }
    if (slotId.empty())
    { m_Status = "The selected pattern does not bind the selected object. Existing draft preserved."; return false; }
    if (sequence->tracks.size() + sequence->animationTracks.size() + sequence->effectTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { m_Status = "The pattern has reached its track limit. Existing draft preserved."; return false; }
    const bool firstOfSlot = std::none_of(sequence->animationTracks.begin(), sequence->animationTracks.end(), [&](const auto& clip) {
        return clip.slotId == slotId;
    });
    const bool pristine = firstOfSlot && sequence->animationTracks.empty() && m_PristinePatternId == m_SelectedInstance;
    const uint32_t oldDuration = sequence->durationMs;
    const uint32_t startMs = firstOfSlot ? 0u : oldDuration;
    if (startMs > CWorldSequenceDocument::MAX_DURATION_MS - clipMs)
    { m_Status = "Appending this clip would exceed the 600-second pattern lifetime. Existing draft preserved."; return false; }
    const uint32_t duration = pristine ? clipMs : (std::max)(oldDuration, startMs + clipMs);
    for (auto& track : sequence->tracks)
    {
        if (track.keys.empty())
        { m_Status = "The pattern has an empty Transform track. Existing draft preserved."; return false; }
        if (pristine)
            track.keys.back().timeMs = duration;
        else if (duration > oldDuration)
        {
            if (track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { m_Status = "Extending this pattern would exceed its Transform key limit. Existing draft preserved."; return false; }
            // Preserve every authored time; extend only the final held pose.
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
    }
    WORLD_SEQUENCE_ANIMATION_TRACK animation;
    animation.slotId = slotId; animation.clipName = selectedClip; animation.startMs = startMs;
    animation.playbackRate = 1.f; animation.loop = false; animation.holdLastFrame = true;
    sequence->animationTracks.push_back(std::move(animation));
    sequence->durationMs = duration;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Animation append refused: " + status + ". Existing draft preserved."; return false; }
    m_Document = std::move(candidate);
    Mark_Dirty();
    m_Status = "Appended " + selectedClip + " at " + std::to_string(startMs) + " ms. Play in Object Sequencer, tune in Object Detail, then Save.";
    return true;
}

void CWorldObjectTool::Render_AnimationResources()
{
    ImGui::SeparatorText("Animation Resources");
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
        Refresh_AnimationResources();
    if (!resource || !resource->sequenceInstanceId.empty())
    { ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str()); return; }
    if (ImGui::Button("Refresh Animations")) Refresh_AnimationResources();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_SelectedInstance.empty() || !m_AnimationCatalogReady || m_AnimationModelAssetId.empty());
    if (ImGui::Button("Assign Model")) { Assign_SelectedModel(); resource = m_Document.Find_ObjectResource(m_SelectedObject); }
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str());
    if (!modelAssetId.empty()) ImGui::TextWrapped("Model: %s", modelAssetId.c_str());
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (sequence) ImGui::TextWrapped("Selected Motion: %s", sequence->displayName.c_str());
    else ImGui::TextWrapped("Select a child Motion to append clips. Shared model assignment belongs to the parent Object.");
    ImGui::BeginDisabled(!m_AnimationCatalogReady || m_SelectedAnimationClip.empty() || !sequence);
    if (ImGui::Button("Append Clip")) Append_SelectedAnimation();
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##AnimationSearch", "Search native animation", m_AnimationSearch.data(), m_AnimationSearch.size());
    const auto search = Lower(m_AnimationSearch.data());
    const float height = (std::max)(100.f, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("NativeAnimationCatalog", ImVec2(0.f, height), true))
    {
        for (const auto& clip : m_AnimationResources)
        {
            if (!search.empty() && Lower(clip.clipName).find(search) == std::string::npos) continue;
            ImGui::PushID(clip.clipName.c_str());
            const float nameWidth = (std::max)(80.f, ImGui::GetContentRegionAvail().x - 82.f);
            if (ImGui::Selectable(clip.clipName.c_str(), clip.clipName == m_SelectedAnimationClip, 0, ImVec2(nameWidth, 0.f)))
                m_SelectedAnimationClip = clip.clipName;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%.3f ms\n%s", clip.clipName.c_str(), clip.durationMs, m_AnimationModelAssetId.c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%.0f ms", clip.durationMs);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText("Object / Shared Resources");
    bool changed = EditText("Object Name", resource.displayName);
    ImGui::TextDisabled("%s", resource.objectId.c_str());
    const bool alias = !resource.sequenceInstanceId.empty();
    int resourceAnchor = resource.anchorKind == "BOSS" ? 2 : resource.anchorKind == "PLAYER" ? 1 : 0;
    ImGui::BeginDisabled(alias);
    if (ImGui::Combo("Anchor Type", &resourceAnchor, "Map\0Character\0Boss\0"))
        Change_ResourceAnchor(resource, resourceAnchor == 2 ? "BOSS" : resourceAnchor == 1 ? "PLAYER" : "WORLD");
    ImGui::EndDisabled();
    if (resource.anchorKind == "BOSS")
    {
        static constexpr const char* actorIds[] = {
            "BOSS_KAKULSAYDON_G1_KOUKU", "BOSS_KAKULSAYDON_G1_SAYDON",
            "BOSS_KAKULSAYDON_G2_KOUKU", "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "BOSS_KAKULSAYDON_G3_SAYDON"};
        static constexpr const char* actorLabels[] = {
            "Kouku / Gate 1", "Saydon / Gate 1", "Kouku / Gate 2", "Big Saydon / Gate 2", "Saydon / Gate 3"};
        const char* selected = resource.anchorBossArchetypeId.empty() ? "Choose Boss" : resource.anchorBossArchetypeId.c_str();
        for (size_t i = 0u; i < std::size(actorIds); ++i)
            if (resource.anchorBossArchetypeId == actorIds[i]) selected = actorLabels[i];
        if (ImGui::BeginCombo("Boss Actor", selected))
        {
            for (size_t i = 0u; i < std::size(actorIds); ++i)
                if (ImGui::Selectable(actorLabels[i], resource.anchorBossArchetypeId == actorIds[i]))
                { resource.anchorBossArchetypeId = actorIds[i]; changed = true; }
            ImGui::EndCombo();
        }
        changed |= EditText("Boss Archetype ID", resource.anchorBossArchetypeId);
        changed |= EditText("BODY Bone", resource.anchorBone);
        ImGui::TextWrapped("Follows the named BODY bone every frame. Empty bone uses the boss root. Hand props use b_wp_1 or b_wp_2. Transform keys edit the local grip offset and rotation.");
    }
    if (alias) ImGui::TextDisabled("Placed objects keep their Map anchor.");
    if (alias)
        ImGui::TextWrapped("Placed object sequence: %s. This editor updates its existing tracks and bindings.", resource.sequenceInstanceId.c_str());
    else
    {
        ImGui::TextWrapped("Model: %s", resource.modelAssetId.empty() ? "Choose a WModel in Physical Resources" : resource.modelAssetId.c_str());
        ImGui::TextWrapped("Diffuse: %s", resource.diffuseTextureAssetId.empty() ? "Embedded model material" : resource.diffuseTextureAssetId.c_str());
        if (!resource.diffuseTextureAssetId.empty() && ImGui::SmallButton("Clear Diffuse Override")) { resource.diffuseTextureAssetId.clear(); changed = true; }
        changed |= ImGui::DragFloat("Model Import Scale", &resource.modelPreScale, .001f, .000001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Object Scale", &resource.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::Checkbox("Animated Model", &resource.animated);
    }
    if (changed) Mark_Dirty();
    if (!alias)
    {
        const auto* initial = m_Document.Find_Instance(resource.defaultMotionInstanceId);
        const auto* initialSequence = initial ? m_Document.Find_Template(initial->templateId) : nullptr;
        if (initialSequence && !initialSequence->tracks.empty() && !initialSequence->tracks.front().keys.empty())
        {
            const auto& first = initialSequence->tracks.front().keys.front();
            auto position = first.positionOffset;
            auto rotation = QuaternionEuler(first.rotationQuaternion);
            ImGui::SeparatorText("Object Transform / All Motion Keys");
            bool transformChanged = ImGui::DragFloat3("Object Position (m)", &position.x, .01f);
            transformChanged |= ImGui::DragFloat3("Object Rotation (deg)", &rotation.x, .25f);
            ImGui::TextWrapped("Position and Rotation adjust every connected Motion key relative to its current pose. Object Scale above applies to all Motions. Use a child Motion for individual keys.");
            if (transformChanged)
            {
                auto candidate = m_Document;
                const auto translationDelta = XMLoadFloat3(&position) - XMLoadFloat3(&first.positionOffset);
                const auto rotationDelta = XMMatrixTranspose(XMMatrixRotationQuaternion(XMLoadFloat4(&first.rotationQuaternion))) *
                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
                std::vector<std::string> editedTemplates;
                for (const auto& id : StateIds(resource))
                {
                    const auto* motion = candidate.Find_Instance(id);
                    auto* sequence = motion ? candidate.Find_Template(motion->templateId) : nullptr;
                    if (!sequence || std::find(editedTemplates.begin(), editedTemplates.end(), sequence->sequenceId) != editedTemplates.end()) continue;
                    // Shared templates cannot be rewritten on behalf of a different Object.
                    for (const auto& other : candidate.Get_Instances())
                        if (other.templateId == sequence->sequenceId)
                            for (const auto& binding : other.bindings)
                                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource.objectId)
                                { m_Status = "This Motion template is shared with another target. Edit its keys separately; Object transform was preserved."; return; }
                    for (auto& track : sequence->tracks)
                        for (auto& key : track.keys)
                        {
                            XMStoreFloat3(&key.positionOffset, XMLoadFloat3(&key.positionOffset) + translationDelta);
                            XMStoreFloat4(&key.rotationQuaternion, XMQuaternionNormalize(XMQuaternionRotationMatrix(
                                XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) * rotationDelta)));
                        }
                    editedTemplates.push_back(sequence->sequenceId);
                }
                std::string status;
                if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
                { m_Status = "Object transform refused: " + status + ". Existing draft preserved."; return; }
                m_Document = std::move(candidate);
                Mark_Dirty();
                m_Status = "Updated the Object transform in every connected Motion. Save preserves these keys.";
                return;
            }
        }
        else ImGui::TextDisabled("Choose a Default Motion to edit Object Position and Rotation here.");
    }
    ImGui::SeparatorText("Connected Motions");
    const auto motions = StateIds(resource);
    const auto* defaultMotion = m_Document.Find_Instance(resource.defaultMotionInstanceId);
    const auto* defaultSequence = defaultMotion ? m_Document.Find_Template(defaultMotion->templateId) : nullptr;
    const char* defaultLabel = defaultSequence ? defaultSequence->displayName.c_str() : "None - choose a Default Motion";
    if (ImGui::BeginCombo("Default Motion", defaultLabel))
    {
        if (ImGui::Selectable("None", resource.defaultMotionInstanceId.empty()))
        { resource.defaultMotionInstanceId.clear(); Mark_Dirty(); }
        for (const auto& id : motions)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            ImGui::BeginDisabled(!instance || !instance->enabled || !sequence);
            if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), resource.defaultMotionInstanceId == id))
            { resource.defaultMotionInstanceId = id; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (!defaultMotion || !defaultMotion->enabled)
        ImGui::TextWrapped("Choose an enabled Default Motion to Append or Preview this Object.");
    ImGui::BeginDisabled(!Preview_Instance());
    if (ImGui::Button("Preview Default")) { Stop_Preview(); m_PreviewAtCharacter = true; m_ClockMs = 0.f; Seek(0.f); m_Playing = m_PreviewActive; }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Stop Preview")) { Stop_Preview(); m_ClockMs = 0.f; }
    if (motions.empty()) ImGui::TextDisabled("No motions yet.");
    for (const auto& id : motions)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        ImGui::PushID(id.c_str());
        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str()))
        { Select_State(id); ImGui::PopID(); return; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
        if (sequence && instance)
        {
            ImGui::TextDisabled("%zu clips | %u ms | %s", sequence->animationTracks.size(),
                sequence->durationMs, MotionEndLabel(instance->motionEnd));
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            {
                const auto* next = m_Document.Find_Instance(instance->nextMotionId);
                const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
                ImGui::TextDisabled("  Next: %s", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion");
            }
        }
        ImGui::PopID();
    }
    ImGui::BeginDisabled(alias);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##NewMotion", "Motion name", m_NewStateName.data(), m_NewStateName.size());
    ImGui::BeginDisabled(!m_NewStateName[0]);
    if (ImGui::Button("Create Motion")) Create_State();
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    if (alias) ImGui::TextWrapped("This placed Object retains its existing Motion and bindings.");
}

void CWorldObjectTool::Render_Detail()
{
    if (auto* group = m_Document.Find_ObjectResource(m_SelectedGroup)) Render_GroupDetail(*group);
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource) { ImGui::TextUnformatted("Select an object resource."); return; }
    if (m_SelectedInstance.empty()) { Render_ObjectDetail(*resource); return; }
    ImGui::TextWrapped("Object: %s", resource->displayName.c_str());
    if (!Preview_Group() && ImGui::Button("Edit Parent Object")) { Select_Object(resource->objectId); return; }
    const bool alias = !resource->sequenceInstanceId.empty();
    bool changed = false;
    auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence) { ImGui::TextWrapped("The selected Motion is unavailable. Select its parent Object to continue."); return; }
    ImGui::SeparatorText("Motion");
    changed |= EditText("Motion Name", sequence->displayName);
    ImGui::TextDisabled("%s", instance->instanceId.c_str());
    changed |= ImGui::Checkbox("Enabled", &instance->enabled);
    if (!alias)
    {
        ImGui::TextDisabled("Creation Anchor: %s", instance->anchorKind == "BOSS" ? "Boss / BODY Bone" : instance->anchorKind == "PLAYER" ? "Character" : "Map");
        changed |= ImGui::DragFloat3(instance->anchorKind == "BOSS" ? "Bone Offset" : instance->anchorKind == "PLAYER" ? "Character Offset" : "Map Position", &instance->position.x, .01f);
        if (ImGui::Button("Use Current Character Position"))
        {
            if (instance->anchorKind == "PLAYER" || instance->anchorKind == "BOSS") { instance->position = {}; changed = true; }
            else if (auto* level = CLevel_KakulSaydonArena::Get_Active()) changed |= level->Try_Get_AuthoringPreviewPlacement(instance->position, m_Status);
            else m_Status = "Player placement requires the active KoukuSaydon arena.";
        }
    }
    const bool supportsSurface = alias && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
    if (supportsSurface || instance->walkableSurface)
    {
        ImGui::SeparatorText("Walkable Surface");
        bool enabled = instance->walkableSurface.has_value();
        if (ImGui::Checkbox("Enable Circular Walking Surface", &enabled))
        {
            if (enabled) instance->walkableSurface = WORLD_SEQUENCE_WALKABLE_SURFACE{};
            else instance->walkableSurface.reset();
            changed = true;
        }
        if (instance->walkableSurface)
        {
            changed |= ImGui::DragFloat("Surface Radius (local m)", &instance->walkableSurface->radiusM,
                .01f, .001f, 1000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Surface Height (local m)", &instance->walkableSurface->localHeightM,
                .001f, -10000.f, 10000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("A horizontal circle centered on the placed model. Values use meters after model import scale; placement scale is applied automatically. Fixed position/scale and Y rotation only. Server Complete Play uses its visible lifetime; blocked ground stays blocked. Save, then publish the Map and Kouku gameplay data.");
        }
    }
    changed |= EditUInt("Start Delay (ms)", instance->startDelayMs, CWorldSequenceDocument::MAX_DURATION_MS);
    changed |= ImGui::DragFloat("Playback Speed", &instance->playbackSpeed, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    uint32_t duration = sequence->durationMs;
    if (EditUInt("Lifetime (ms)", duration, CWorldSequenceDocument::MAX_DURATION_MS, 1))
    {
        // Preserve the key order and endpoint contract even when shortening a state.
        bool valid = true;
        for (const auto& track : sequence->tracks) if (duration + 1u < track.keys.size()) valid = false;
        for (const auto& animation : sequence->animationTracks)
            if (static_cast<size_t>(duration) < std::count_if(sequence->animationTracks.begin(), sequence->animationTracks.end(),
                [&](const auto& other) { return other.slotId == animation.slotId; })) valid = false;
        if (valid)
        {
            for (auto& track : sequence->tracks)
            {
                for (size_t key = 1; key + 1 < track.keys.size(); ++key)
                {
                    const auto scaled = static_cast<uint32_t>(std::llround(static_cast<double>(track.keys[key].timeMs) * duration / sequence->durationMs));
                    track.keys[key].timeMs = (std::clamp)(scaled, track.keys[key - 1].timeMs + 1,
                        duration - static_cast<uint32_t>(track.keys.size() - key - 1));
                }
                if (!track.keys.empty()) track.keys.back().timeMs = duration;
            }
            std::unordered_map<std::string, uint32_t> previousStarts;
            for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
            {
                auto& animation = sequence->animationTracks[index];
                const auto previous = previousStarts.find(animation.slotId);
                const auto remaining = std::count_if(sequence->animationTracks.begin() + index + 1, sequence->animationTracks.end(),
                    [&](const auto& other) { return other.slotId == animation.slotId; });
                const uint32_t scaled = static_cast<uint32_t>(static_cast<double>(animation.startMs) * duration / sequence->durationMs);
                animation.startMs = previous == previousStarts.end() ? 0 : (std::clamp)(scaled,
                    previous->second + 1, duration - static_cast<uint32_t>(remaining) - 1);
                previousStarts[animation.slotId] = animation.startMs;
            }
            for (auto& effect : sequence->effectTracks)
                if (effect.timing == "TIME")
                    effect.startMs = static_cast<uint32_t>(static_cast<uint64_t>(effect.startMs) * duration / sequence->durationMs);
            sequence->durationMs = duration;
            if (sequence->objectMotion.count > 1)
                sequence->objectMotion.intervalMs = (std::min)(sequence->objectMotion.intervalMs, (sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration) / (sequence->objectMotion.count - 1));
            for (auto& emission : sequence->objectMotion.emissions)
                emission.startDelayMs = (std::min)(emission.startDelayMs, sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration);
            changed = true;
        }
        else m_Status = "Lifetime must leave at least one millisecond between every existing key or clip.";
    }
    ImGui::BeginDisabled(alias || Preview_Group());
    int motionEnd = static_cast<int>(instance->motionEnd);
    if (ImGui::Combo("On Complete", &motionEnd, "Stop\0Hold Last Pose\0Loop\0Play Motion\0"))
    {
        instance->motionEnd = static_cast<WORLD_SEQUENCE_MOTION_END>(motionEnd);
        if (instance->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) instance->nextMotionId.clear();
        changed = true;
    }
    ImGui::EndDisabled();
    if (Preview_Group()) ImGui::TextDisabled("Combined preview members use Stop; each Lifetime remains editable.");
    if (alias) ImGui::TextDisabled("Placed Object motions use Stop.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
    {
        const auto* next = m_Document.Find_Instance(instance->nextMotionId);
        const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
        if (ImGui::BeginCombo("Next Motion", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion"))
        {
            for (const auto& id : StateIds(*resource))
            {
                const auto* candidate = m_Document.Find_Instance(id);
                const auto* candidateSequence = candidate ? m_Document.Find_Template(candidate->templateId) : nullptr;
                if (!candidateSequence || candidate->instanceId == instance->instanceId) continue;
                const bool compatible = candidate->enabled && sequence->objectMotion.count == 1u &&
                    candidateSequence->objectMotion.count == 1u && instance->bindings.size() == 1u &&
                    candidate->bindings.size() == 1u && instance->bindings.front().slotId == candidate->bindings.front().slotId;
                ImGui::PushID(id.c_str());
                ImGui::BeginDisabled(!compatible);
                if (ImGui::Selectable(candidateSequence->displayName.c_str(), instance->nextMotionId == id))
                { instance->nextMotionId = id; changed = true; }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("%s%s", id.c_str(), compatible ? "" : "\nRequires an enabled Motion with Count 1 and the same object slot.");
                ImGui::EndDisabled();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::TextWrapped("At Lifetime, continue the same object with this Motion. Each Motion in the link must use Count 1; links cannot form a cycle.");
    }
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD)
        ImGui::TextWrapped("Keep the final transform and animation pose after Lifetime.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP)
        ImGui::TextWrapped("Repeat this Motion after Lifetime. This Loop is saved and also used by Play.");
    else ImGui::TextWrapped("Standalone Play ends the object at Lifetime. A Motion applied by a Result stops at its final pose; the target WORLD cue keeps its original lifetime.");
    int interpolation = sequence->interpolation == WORLD_SEQUENCE_INTERPOLATION::LINEAR ? 0 : 1;
    if (ImGui::Combo("Interpolation", &interpolation, "Linear\0Smooth Step\0"))
    { sequence->interpolation = interpolation == 0 ? WORLD_SEQUENCE_INTERPOLATION::LINEAR : WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP; changed = true; }
    if (!alias && ImGui::CollapsingHeader("Physics / Motion / Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& motion = sequence->objectMotion;
        ImGui::TextWrapped("Motion uses the Lifetime timeline and works without animation clips. Play or drag the ruler to preview, then Save.");
        ImGui::DragFloat("Arc Height (m)", &m_VerticalArcHeight, .05f, .01f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Apply Vertical Arc"))
        {
            const float seconds = sequence->durationMs * .001f;
            const float velocityY = 4.f * m_VerticalArcHeight / seconds;
            const float accelerationY = -8.f * m_VerticalArcHeight / (seconds * seconds);
            if (!std::isfinite(m_VerticalArcHeight) || m_VerticalArcHeight <= 0.f ||
                !std::isfinite(velocityY) || !std::isfinite(accelerationY) ||
                std::abs(velocityY) > 100000.f || std::abs(accelerationY) > 100000.f)
                m_Status = "Vertical arc is too fast for this Lifetime. Increase Lifetime or reduce Arc Height; motion preserved.";
            else
            {
                motion.velocity = {0.f, velocityY, 0.f};
                motion.acceleration = {0.f, accelerationY, 0.f};
                motion.spreadDegrees = 0.f;
                changed = true;
                m_Status = "Vertical arc applied. Count and interval preserved. First emission returns at Lifetime. Effect-bearing motions give every emission a full Lifetime. Save to keep the motion.";
            }
        }
        ImGui::TextDisabled("First emission: apex at %.0f ms; return at %u ms.", sequence->durationMs * .5f, sequence->durationMs);
        ImGui::TextWrapped("Arc Height sets the physics offset above the Transform track. With Effect rows, every emission has a full Lifetime and its own Effect tail. Apply again after changing Lifetime. Velocity and Acceleration remain editable.");
        changed |= ImGui::DragFloat3("Velocity (m/s)", &motion.velocity.x, .05f);
        changed |= ImGui::DragFloat3("Acceleration (m/s2)", &motion.acceleration.x, .05f);
        changed |= ImGui::DragFloat3("Self Rotation (deg/s)", &motion.angularVelocityDegrees.x, .5f);
        changed |= ImGui::DragFloat3("Revolution (deg/s)", &motion.revolutionDegreesPerSecond.x, .5f);
        changed |= ImGui::DragFloat3("Revolution Offset (m)", &motion.revolutionOffset.x, .05f);
        auto& emissions = motion.emissions;
        ImGui::BeginDisabled(!emissions.empty());
        changed |= EditUInt("Count", motion.count, 128, 1);
        const uint32_t maxInterval = motion.count > 1 ? (sequence->effectTracks.empty() ? sequence->durationMs - 1 :
            CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs) / (motion.count - 1) : CWorldSequenceDocument::MAX_DURATION_MS;
        if (motion.intervalMs > maxInterval) { motion.intervalMs = maxInterval; changed = true; }
        changed |= EditUInt("Creation Interval (ms)", motion.intervalMs, maxInterval);
        changed |= ImGui::DragFloat(sequence->effectTracks.empty() ? "Spread (deg)" : "Horizontal Spread (deg)", &motion.spreadDegrees, .5f, 0.f, sequence->effectTracks.empty() ? 180.f : 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        changed |= ImGui::DragFloat3("Spawn Half Extents (m)", &motion.spawnHalfExtents.x, .05f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextDisabled("Width / height / depth = twice these values. Set Y to 0 for a ground rectangle.");
        changed |= EditUInt("Seed", motion.seed, INT_MAX);
        ImGui::SeparatorText("Authored Emissions");
        ImGui::TextWrapped("Rows replace the seeded spread. Each row replays this Motion's keys, physics and revolution from its own local offset, yaw and delay. Leave the list empty to keep Count / Creation Interval / Spread.");
        const uint32_t maxDelay = sequence->effectTracks.empty() ? sequence->durationMs - 1 : CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs;
        size_t removeRow = emissions.size(), duplicateRow = emissions.size();
        if (!emissions.empty() && ImGui::BeginTable("AuthoredEmissions", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.f);
            ImGui::TableSetupColumn("Offset X (m)");
            ImGui::TableSetupColumn("Offset Y (m)");
            ImGui::TableSetupColumn("Offset Z (m)");
            ImGui::TableSetupColumn("Yaw (deg)");
            ImGui::TableSetupColumn("Start Delay (ms)");
            ImGui::TableSetupColumn("##rowActions", ImGuiTableColumnFlags_WidthFixed, 84.f);
            ImGui::TableHeadersRow();
            for (size_t row = 0; row < emissions.size(); ++row)
            {
                auto& emission = emissions[row];
                ImGui::PushID(static_cast<int>(row));
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row);
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetX", &emission.positionOffset.x, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetY", &emission.positionOffset.y, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetZ", &emission.positionOffset.z, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##yaw", &emission.yawDegrees, .5f, -36000.f, 36000.f, "%.1f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= EditUInt("##delay", emission.startDelayMs, static_cast<int>(maxDelay));
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Dup")) duplicateRow = row;
                ImGui::SameLine();
                if (ImGui::SmallButton("Del")) removeRow = row;
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (duplicateRow < emissions.size() && emissions.size() < 128u)
        { emissions.insert(emissions.begin() + duplicateRow + 1, emissions[duplicateRow]); changed = true; }
        if (removeRow < emissions.size()) { emissions.erase(emissions.begin() + removeRow); changed = true; }
        ImGui::BeginDisabled(emissions.size() >= 128u);
        if (ImGui::Button("Add Emission"))
        {
            WORLD_SEQUENCE_OBJECT_EMISSION emission;
            if (!emissions.empty()) emission = emissions.back();
            emissions.push_back(emission);
            changed = true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(emissions.empty());
        if (ImGui::Button("Clear Emissions")) { emissions.clear(); changed = true; }
        ImGui::EndDisabled();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragInt("Ring Count", &m_RingCount, 1.f, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragFloat("Ring Start (deg)", &m_RingStartDegrees, .5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        if (ImGui::Button("Distribute on Ring"))
        {
            /* A row's yaw also turns Revolution Offset, so a row at phase p rides
               R(p)*o and an offset of R(p)*o puts every row on one circle centred
               on the saved position itself. */
            const vector_t orbitOffset = XMLoadFloat3(&motion.revolutionOffset);
            emissions.clear();
            for (int index = 0; index < m_RingCount; ++index)
            {
                WORLD_SEQUENCE_OBJECT_EMISSION emission;
                emission.yawDegrees = m_RingStartDegrees + 360.f * static_cast<float>(index) / static_cast<float>(m_RingCount);
                XMStoreFloat3(&emission.positionOffset, XMVector3TransformNormal(orbitOffset,
                    XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees))));
                emissions.push_back(emission);
            }
            changed = true;
        }
        ImGui::TextDisabled("Ring rows circle the saved position at this Motion's Revolution Offset radius; use Revolution (deg/s) for the orbit speed.");
        if (!emissions.empty())
        {
            motion.count = static_cast<uint32_t>(emissions.size());
            motion.intervalMs = 0u;
            motion.spreadDegrees = 0.f;
        }
    }
    if (changed) Mark_Dirty();
    if (!alias) Render_EffectRows(*sequence);
    ImGui::SeparatorText("Selected Key");
    Render_KeyEditor(*sequence);
}

void CWorldObjectTool::Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (EditText("Combined Motion Name", resource.displayName)) Mark_Dirty();
    const auto* current = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = current ? m_Document.Find_Template(current->templateId) : nullptr;
    if (ImGui::BeginCombo("Editing Motion", sequence ? sequence->displayName.c_str() : "Choose a row"))
    {
        for (const auto& id : resource.motionInstanceIds)
        {
            const auto* member = m_Document.Find_Instance(id);
            const auto* motion = member ? m_Document.Find_Template(member->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            if (ImGui::Selectable(motion ? motion->displayName.c_str() : id.c_str(), id == m_SelectedInstance)) Select_State(id);
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::TextWrapped("All %zu motions stay in preview. The full editor below changes the selected row; Save keeps all rows.", resource.motionInstanceIds.size());
}

void CWorldObjectTool::Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    ImGui::SameLine();
    if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter)) m_PreviewDirty = m_PreviewActive;
    ImGui::TextWrapped("Click a motion row or key to edit it in Object Detail. All %zu motions keep playing together.", resource.motionInstanceIds.size());
    if (!m_PreviewStatus.empty()) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    // Disabled rows remain on the authoring timeline, even when playback has a shorter span.
    float extent = 1.f;
    for (const auto& id : resource.motionInstanceIds)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        if (sequence) extent = (std::max)(extent, instance->startDelayMs +
            sequence->PresentationSpanMs() / (std::max)(.05f, instance->playbackSpeed));
    }
    float clock = m_ClockMs;
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, extent, "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    if (ImGui::BeginChild("CombinedObjectTimeline", ImVec2(0.f, (std::max)(110.f, ImGui::GetContentRegionAvail().y)),
        true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        auto* draw = ImGui::GetWindowDrawList();
        const uint32_t duration = static_cast<uint32_t>(std::ceil(extent));
        const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, extent * m_Zoom * .001f);
        const float pixelsPerMs = width / extent;
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), duration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
            Seek((std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, extent));
        float y = origin.y + 28.f;
        for (const auto& id : resource.motionInstanceIds)
        {
            auto* instance = m_Document.Find_Instance(id);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (!sequence) continue;
            ImGui::PushID(id.c_str());
            const float speed = (std::max)(.05f, instance->playbackSpeed);
            const auto timeX = [&](float localMs) { return origin.x + (instance->startDelayMs + localMs / speed) * pixelsPerMs; };
            ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
            if (ImGui::Selectable(sequence->displayName.c_str(), id == m_SelectedInstance, 0, ImVec2(width, 22.f)))
                Select_State(id);
            y += 24.f;
            for (size_t trackIndex = 0; trackIndex < sequence->tracks.size(); ++trackIndex)
            {
                auto& track = sequence->tracks[trackIndex];
                ImGui::PushID(track.slotId.c_str());
                const ImVec2 row(timeX(0.f), y);
                const ImVec2 end(timeX(static_cast<float>(sequence->durationMs)), y + 25.f);
                CompositionTimeline::DrawBox(draw, row, end,
                    instance->enabled ? IM_COL32(61, 107, 141, 255) : IM_COL32(65, 65, 65, 255),
                    id == m_SelectedInstance && m_SelectedTrack == trackIndex, track.slotId.c_str(), false, false);
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, end) && ImGui::IsMouseClicked(0))
                {
                    if (m_SelectedInstance != id) Select_State(id);
                    m_SelectedTrack = trackIndex; m_SelectedKey = 0;
                }
                for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
                {
                    auto& key = track.keys[keyIndex];
                    const float x = timeX(static_cast<float>(key.timeMs)), centreY = y + 12.f;
                    const bool selected = id == m_SelectedInstance && m_SelectedTrack == trackIndex && m_SelectedKey == keyIndex;
                    draw->AddQuadFilled(ImVec2(x, centreY - 6.f), ImVec2(x + 6.f, centreY),
                        ImVec2(x, centreY + 6.f), ImVec2(x - 6.f, centreY), selected ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE);
                    ImGui::PushID(static_cast<int>(keyIndex));
                    ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, origin.x, origin.x + width - 14.f), y + 4.f));
                    ImGui::InvisibleButton("Key", ImVec2(14.f, 18.f));
                    if (ImGui::IsItemClicked())
                    {
                        if (m_SelectedInstance != id) Select_State(id);
                        m_SelectedTrack = trackIndex; m_SelectedKey = keyIndex;
                    }
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                    {
                        const float local = ((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs - instance->startDelayMs) * speed;
                        const uint32_t time = static_cast<uint32_t>((std::clamp)(local,
                            static_cast<float>(track.keys[keyIndex - 1].timeMs + 1), static_cast<float>(track.keys[keyIndex + 1].timeMs - 1)));
                        if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                    ImGui::PopID();
                }
                ImGui::PopID();
                y += 32.f;
            }
            for (const auto& clip : sequence->animationTracks)
            {
                uint32_t end = sequence->durationMs;
                for (const auto& next : sequence->animationTracks)
                    if (next.slotId == clip.slotId && next.startMs > clip.startMs) end = (std::min)(end, next.startMs);
                const ImVec2 first(timeX(static_cast<float>(clip.startMs)), y);
                const ImVec2 last(timeX(static_cast<float>(end)), y + 25.f);
                CompositionTimeline::DrawBox(draw, first, last, IM_COL32(113, 82, 147, 255), id == m_SelectedInstance,
                    clip.displayName.empty() ? clip.clipName.c_str() : clip.displayName.c_str());
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(first, last) && ImGui::IsMouseClicked(0)) Select_State(id);
                y += 32.f;
            }
            for (size_t index = 0; index < sequence->effectTracks.size(); ++index)
            {
                const auto& effect = sequence->effectTracks[index];
                const float start = static_cast<float>(sequence->EffectStartMs(effect));
                const ImVec2 first(timeX(start), y), last(timeX(start + effect.durationMs), y + 25.f);
                CompositionTimeline::DrawBox(draw, first, last, IM_COL32(167, 95, 51, 255),
                    id == m_SelectedInstance && m_SelectedEffectRow == index, effect.resourceId.c_str());
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(first, last) && ImGui::IsMouseClicked(0))
                { if (m_SelectedInstance != id) Select_State(id); m_SelectedEffectRow = index; }
                y += 32.f;
            }
            ImGui::PopID();
            y += 8.f;
        }
        const float cursor = origin.x + (std::clamp)(m_ClockMs, 0.f, extent) * pixelsPerMs;
        draw->AddLine(ImVec2(cursor, origin.y), ImVec2(cursor, y), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, y - origin.y));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    ImGui::SeparatorText(sequence.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine(); if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* selectedInstance = m_Document.Find_Instance(m_SelectedInstance);
    if (selectedInstance && selectedInstance->anchorKind == "WORLD")
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter))
        {
            m_PreviewDirty = m_PreviewActive;
            if (m_PreviewActive) Seek(m_ClockMs);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Preview at the current character without changing the saved Map position. Clear to preview the authored position.");
    }
    if (selectedInstance) ImGui::TextDisabled("On Complete: %s", MotionEndLabel(selectedInstance->motionEnd));
    if (m_PreviewActive) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    if (m_ClockMs >= SpanMs() && selectedInstance &&
        (selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP || selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        ImGui::TextWrapped("Completion motion active. This timeline still edits the selected Motion's first Lifetime; seek to 0 to restart it.");
    float clock = (std::min)(m_ClockMs, SpanMs());
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, (std::max)(1.f, SpanMs()), "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    const float rowHeight = 32.f;
    const uint32_t timelineDuration = sequence.PresentationSpanMs();
    const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, timelineDuration * m_Zoom * .001f);
    const float pixelsPerMs = width / timelineDuration;
    const bool showPhysics = resource && resource->sequenceInstanceId.empty();
    const float tracksHeight = rowHeight * static_cast<float>(sequence.tracks.size() + sequence.animationTracks.size() + sequence.effectTracks.size());
    const float height = 28.f + tracksHeight + (showPhysics ? 64.f : 0.f);
    if (ImGui::BeginChild("ObjectTimeline", ImVec2(0, (std::max)(110.f, ImGui::GetContentRegionAvail().y)), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), timelineDuration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
        {
            const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
            if (instance) Seek(instance->startDelayMs + local / instance->playbackSpeed);
        }
        for (size_t index = 0; index < sequence.tracks.size(); ++index)
        {
            auto& track = sequence.tracks[index]; ImGui::PushID(track.slotId.c_str());
            const auto row = ImVec2(origin.x, origin.y + 28.f + rowHeight * index);
            CompositionTimeline::DrawBox(draw, row, ImVec2(row.x + sequence.durationMs * pixelsPerMs, row.y + 25.f),
                IM_COL32(61, 107, 141, 255), m_SelectedTrack == index, track.slotId.c_str(), false, false);
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, ImVec2(row.x + width, row.y + 25.f)) && ImGui::IsMouseClicked(0))
            { m_SelectedTrack = index; m_SelectedKey = 0; }
            for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
            {
                auto& key = track.keys[keyIndex]; const float x = row.x + key.timeMs * pixelsPerMs;
                const float y = row.y + 12.f;
                const ImU32 color = m_SelectedTrack == index && m_SelectedKey == keyIndex ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE;
                draw->AddQuadFilled(ImVec2(x, y - 6), ImVec2(x + 6, y), ImVec2(x, y + 6), ImVec2(x - 6, y), color);
                ImGui::PushID(static_cast<int>(keyIndex)); ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, row.x, row.x + width - 14.f), row.y + 4.f));
                ImGui::InvisibleButton("Key", ImVec2(14, 18));
                if (ImGui::IsItemClicked()) { m_SelectedTrack = index; m_SelectedKey = keyIndex; }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                {
                    const auto moved = static_cast<int>((ImGui::GetIO().MousePos.x - row.x) / pixelsPerMs);
                    const uint32_t time = static_cast<uint32_t>((std::clamp)(moved, static_cast<int>(track.keys[keyIndex - 1].timeMs + 1), static_cast<int>(track.keys[keyIndex + 1].timeMs - 1)));
                    if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            const auto& track = sequence.animationTracks[index];
            uint32_t end = sequence.durationMs;
            for (const auto& next : sequence.animationTracks) if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, next.startMs);
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + index);
            CompositionTimeline::DrawBox(draw, ImVec2(origin.x + track.startMs * pixelsPerMs, y),
                ImVec2(origin.x + end * pixelsPerMs, y + 25.f), IM_COL32(113, 82, 147, 255), false,
                track.displayName.empty() ? track.clipName.c_str() : track.displayName.c_str());
        }
        for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
        {
            const auto& effect = sequence.effectTracks[index];
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + sequence.animationTracks.size() + index);
            const float x = origin.x + sequence.EffectStartMs(effect) * pixelsPerMs;
            const float endX = x + effect.durationMs * pixelsPerMs;
            CompositionTimeline::DrawBox(draw, ImVec2(x, y), ImVec2(endX, y + 25.f),
                IM_COL32(167, 95, 51, 255), m_SelectedEffectRow == index, effect.resourceId.c_str());
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(ImVec2(x, y), ImVec2(endX, y + 25.f)) && ImGui::IsMouseClicked(0))
                m_SelectedEffectRow = index;
        }
        if (showPhysics)
        {
            const float top = origin.y + 28.f + tracksHeight;
            draw->AddRectFilled(ImVec2(origin.x, top), ImVec2(origin.x + width, top + 60.f), IM_COL32(28, 49, 48, 255));
            draw->AddText(ImVec2(origin.x + 5.f, top + 3.f), IM_COL32(136, 227, 198, 255), "Physics Y offset / first emission");
            const auto& motion = sequence.objectMotion;
            const float seconds = sequence.durationMs * .001f;
            const float physicsWidth = sequence.durationMs * pixelsPerMs;
            const auto sampleY = [&motion](float time) { return motion.velocity.y * time + .5f * motion.acceleration.y * time * time; };
            float minimum = (std::min)(0.f, sampleY(seconds));
            float maximum = (std::max)(0.f, sampleY(seconds));
            if (motion.acceleration.y != 0.f)
            {
                const float apex = -motion.velocity.y / motion.acceleration.y;
                if (apex > 0.f && apex < seconds)
                { minimum = (std::min)(minimum, sampleY(apex)); maximum = (std::max)(maximum, sampleY(apex)); }
            }
            const float range = (std::max)(.01f, maximum - minimum);
            if (std::isfinite(motion.velocity.y) && std::isfinite(motion.acceleration.y) &&
                std::isfinite(minimum) && std::isfinite(maximum) && std::isfinite(range))
                for (int segment = 0; segment < 64; ++segment)
                {
                    const float from = static_cast<float>(segment) / 64.f, to = static_cast<float>(segment + 1) / 64.f;
                    draw->AddLine(ImVec2(origin.x + physicsWidth * from, top + 56.f - 30.f * (sampleY(seconds * from) - minimum) / range),
                        ImVec2(origin.x + physicsWidth * to, top + 56.f - 30.f * (sampleY(seconds * to) - minimum) / range), IM_COL32(91, 217, 171, 255), 2.f);
                }
            ImGui::SetCursorScreenPos(ImVec2(origin.x, top));
            ImGui::InvisibleButton("PhysicsSeek", ImVec2(width, 60.f));
            if (ImGui::IsItemActive() && selectedInstance)
            {
                const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
                Seek(selectedInstance->startDelayMs + local / selectedInstance->playbackSpeed);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Drag to seek. Y offset range: %.3f to %.3f m. Added to Transform keys before spread, revolution and anchor rotation.", minimum, maximum);
        }
        const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
        const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
        const float cursorX = origin.x + (std::clamp)(local, 0.f, static_cast<float>(timelineDuration)) * pixelsPerMs;
        draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, origin.y + height), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!sequence.tracks.empty())
    {
        m_SelectedTrack = (std::min)(m_SelectedTrack, sequence.tracks.size() - 1);
        if (ImGui::BeginCombo("Target Track", sequence.tracks[m_SelectedTrack].slotId.c_str()))
        {
            for (size_t index = 0; index < sequence.tracks.size(); ++index)
                if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(), index == m_SelectedTrack)) { m_SelectedTrack = index; m_SelectedKey = 0; }
            ImGui::EndCombo();
        }
        auto& track = sequence.tracks[m_SelectedTrack];
        if (!track.keys.empty())
        {
            m_SelectedKey = (std::min)(m_SelectedKey, track.keys.size() - 1);
            auto& key = track.keys[m_SelectedKey];
            if (ImGui::BeginCombo("Keyframe", (std::to_string(key.timeMs) + " ms").c_str()))
            {
                for (size_t index = 0; index < track.keys.size(); ++index)
                    if (ImGui::Selectable((std::to_string(track.keys[index].timeMs) + " ms").c_str(), m_SelectedKey == index)) m_SelectedKey = index;
                ImGui::EndCombo();
            }
            auto& selected = track.keys[m_SelectedKey];
            const bool endpoint = m_SelectedKey == 0 || m_SelectedKey + 1 == track.keys.size();
            bool changed = false;
            ImGui::BeginDisabled(endpoint);
            if (!endpoint) changed |= EditUInt("Key Time (ms)", selected.timeMs, track.keys[m_SelectedKey + 1].timeMs - 1, track.keys[m_SelectedKey - 1].timeMs + 1);
            else ImGui::Text("Endpoint: %u ms", selected.timeMs);
            ImGui::EndDisabled();
            changed |= ImGui::DragFloat3("Position Offset (m)", &selected.positionOffset.x, .01f);
            auto degrees = QuaternionEuler(selected.rotationQuaternion);
            if (ImGui::DragFloat3("Rotation Offset (deg)", &degrees.x, .25f))
            {
                XMStoreFloat4(&selected.rotationQuaternion, XMQuaternionRotationRollPitchYaw(
                    XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
                changed = true;
            }
            changed |= ImGui::DragFloat3("Scale Multiplier", &selected.scaleMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Visible", &selected.visible);
            if (changed) Mark_Dirty();
            ImGui::BeginDisabled(endpoint);
            if (ImGui::Button("Delete Key")) { track.keys.erase(track.keys.begin() + m_SelectedKey); m_SelectedKey = 0; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT || sequence.durationMs < 2);
            if (ImGui::Button("Add Key at Cursor"))
            {
                const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
                const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
                const uint32_t time = static_cast<uint32_t>((std::clamp)(local, 1.f, static_cast<float>(sequence.durationMs - 1)));
                auto at = std::lower_bound(track.keys.begin(), track.keys.end(), time, [](const auto& value, uint32_t clock) { return value.timeMs < clock; });
                m_SelectedKey = at - track.keys.begin();
                if (at == track.keys.end() || at->timeMs != time)
                {
                    auto added = CWorldSequencePlayer::Sample_Track(sequence, track, static_cast<float>(time)); added.timeMs = time;
                    track.keys.insert(at, added); Mark_Dirty();
                }
            }
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Animation Clips"))
    {
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            auto& clip = sequence.animationTracks[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Slot: %s", clip.slotId.c_str());
            bool changed = EditText("Clip display name", clip.displayName);
            ImGui::TextWrapped("Native clip: %s", clip.clipName.c_str());
            uint32_t minimum = 0, maximum = sequence.durationMs - 1;
            bool first = true;
            for (size_t other = 0; other < sequence.animationTracks.size(); ++other)
            {
                const auto& next = sequence.animationTracks[other];
                if (other == index || next.slotId != clip.slotId) continue;
                if (next.startMs < clip.startMs) { minimum = (std::max)(minimum, next.startMs + 1); first = false; }
                if (next.startMs > clip.startMs) maximum = (std::min)(maximum, next.startMs - 1);
            }
            ImGui::BeginDisabled(first); changed |= EditUInt("Clip Start (ms)", clip.startMs, maximum, minimum); ImGui::EndDisabled();
            changed |= ImGui::DragFloat("Clip Speed", &clip.playbackRate, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Clip Loop", &clip.loop); changed |= ImGui::Checkbox("Hold Last Pose", &clip.holdLastFrame);
            if (changed) Mark_Dirty();
            if (ImGui::SmallButton("Remove Clip"))
            {
                const auto slot = clip.slotId; sequence.animationTracks.erase(sequence.animationTracks.begin() + index);
                for (auto& remaining : sequence.animationTracks) if (remaining.slotId == slot) { remaining.startMs = 0; break; }
                Mark_Dirty(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        ImGui::TextWrapped("Select a native clip in Object Resources, then Append Clip to this Motion.");
    }
}

void CWorldObjectTool::Rebuild_PhysicalTree()
{
    m_PhysicalTree = {};
    const auto search = Lower(m_PhysicalSearch.data());
    for (size_t index = 0; index < m_PhysicalAssets.size(); ++index)
    {
        const auto& asset = m_PhysicalAssets[index];
        if ((m_PhysicalSlot == 0) != (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)) continue;
        if (!search.empty() && Lower(asset.assetId).find(search) == std::string::npos) continue;
        std::vector<std::string> segments;
        for (const auto& segment : std::filesystem::path(asset.assetId).parent_path()) segments.push_back(segment.string());
        InsertResourceTree(m_PhysicalTree, segments, index);
    }
    FinalizeResourceTree(m_PhysicalTree);
}

void CWorldObjectTool::Render_PhysicalResources()
{
    ImGui::SeparatorText("Physical Resources");
    if (!m_SelectedInstance.empty()) ImGui::TextWrapped("Select the parent Object to change its shared model or texture.");
    if (!m_PhysicalScanned)
    {
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
        m_PhysicalScanned = true;
    }
    ImGui::BeginDisabled(m_PhysicalScanRunning);
    if (ImGui::Button("Refresh Files"))
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
    ImGui::EndDisabled();
    if (m_PhysicalScanRunning)
    { ImGui::SameLine(); ImGui::TextDisabled("Scanning files..."); }
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##AssignSlot", &m_PhysicalSlot, "Model (.wmodel)\0Diffuse (.dds)\0")) Rebuild_PhysicalTree();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##PhysicalSearch", "Search full relative path", m_PhysicalSearch.data(), m_PhysicalSearch.size())) Rebuild_PhysicalTree();
    ImGui::TextDisabled("%zu matching files", m_PhysicalTree.iRecursiveLeafCount);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_PhysicalStatus.c_str());
    if (ImGui::BeginChild("PhysicalResourceFolders", ImVec2(0, (std::max)(100.f, ImGui::GetContentRegionAvail().y * .40f)), true))
    {
        RenderResourceTree(m_PhysicalTree, [this](size_t index) {
            if (index >= m_PhysicalAssets.size()) return;
            const auto& asset = m_PhysicalAssets[index];
            ImGui::PushID(asset.assetId.c_str());
            auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
            ImGui::BeginDisabled(!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty());
            if (ImGui::Selectable(asset.fileName.c_str(), asset.assetId == m_SelectedPhysical &&
                (asset.kind != PHYSICAL_RESOURCE_KIND::MODEL || m_AnimationCandidateObjectId == m_SelectedObject)))
            {
                m_SelectedPhysical = asset.assetId;
                if (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)
                {
                    m_AnimationCandidateModelAssetId = asset.assetId;
                    m_AnimationCandidateObjectId = m_SelectedObject;
                    Refresh_AnimationResources();
                    m_Status = "Model candidate selected. Assign Model updates the parent Object; its child motions keep their clip bindings.";
                }
                else
                {
                    resource->diffuseTextureAssetId = asset.assetId;
                    const auto pristinePattern = m_PristinePatternId;
                    Mark_Dirty();
                    m_PristinePatternId = pristinePattern;
                    m_Status = "Assigned diffuse " + asset.assetId + ". Save stores it on " + resource->displayName + ".";
                }
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", asset.assetId.c_str());
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
}
#endif

```

## Tools/MapPipeline/test_world_sequence_authoring_contract.py G04 전체 반영 코드

```python
from __future__ import annotations

import re
import copy
import json
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class WorldSequenceAuthoringContractTests(unittest.TestCase):
    """Fast source/project integration guards; the Product build compiles behavior."""

    def test_combined_object_selection_retains_full_motion_editor(self) -> None:
        code = read("Client/Private/WorldObjectTool.cpp")
        header = read("Client/Public/WorldObjectTool.h")
        self.assertIn("std::string m_SelectedGroup;", header)
        group_scope = code.split("CWorldObjectTool::Preview_Group() const", 1)[1].split("bool CWorldObjectTool::Begin_Preview", 1)[0]
        self.assertIn("Find_ObjectResource(m_SelectedGroup)", group_scope)
        self.assertNotIn("m_SelectedInstance.empty()", group_scope)
        select = code.split("void CWorldObjectTool::Select_State", 1)[1]
        grouped = select.split("const auto* resource =", 1)[0]
        self.assertIn("m_SelectedObject = instance->bindings.front().targetId", grouped)
        self.assertIn("m_SelectedInstance = id", grouped)
        self.assertNotIn("Stop_Preview()", grouped)
        self.assertNotIn("m_ClockMs =", grouped)
        detail = code.split("void CWorldObjectTool::Render_Detail()", 1)[1].split("void CWorldObjectTool::Render_GroupDetail", 1)[0]
        self.assertIn("Render_GroupDetail(*group);", detail)
        self.assertNotIn("Render_GroupDetail(*resource); return;", detail)
        for control in ("Physics / Motion / Emission", "Authored Emissions", "Revolution Offset (m)", "Render_KeyEditor(*sequence)"):
            self.assertIn(control, detail)
        timeline = code.split("void CWorldObjectTool::Render_GroupSequence", 1)[1].split("void CWorldObjectTool::Render_Sequence", 1)[0]
        for component in ("resource.motionInstanceIds", "DrawRuler", "track.keys", "Select_State(id)", "Mark_Dirty()", "sequence->animationTracks", "sequence->effectTracks"):
            self.assertIn(component, timeline)
        self.assertNotIn("Orbit Radius", timeline)

    def setUp(self) -> None:
        self.document_h = read("Client/Public/WorldSequenceDocument.h")
        self.document_cpp = read("Client/Private/WorldSequenceDocument.cpp")
        self.panel_h = read("Client/Public/WorldSequenceToolPanel.h")
        self.panel_cpp = read("Client/Private/WorldSequenceToolPanel.cpp")
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")

    def test_walkable_surface_accepts_fixed_roulette_and_rejects_unsupported_geometry(self) -> None:
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.8")
        instance["walkableSurface"] = {"radiusM": 2.5, "localHeightM": .026313}
        source["instances"] = [instance]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == instance["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", []) if row.get("sequenceInstanceId") == instance["instanceId"]]
        keys = source["templates"][0]["tracks"][0]["keys"]
        source["templates"][0]["tracks"][0]["keys"] = [keys[0], keys[-1]]
        cases = [("valid", source, True)]
        for name, mutate in (
            ("negative_radius", lambda d: d["instances"][0]["walkableSurface"].update(radiusM=-1)),
            ("boolean_height", lambda d: d["instances"][0]["walkableSurface"].update(localHeightM=True)),
            ("unknown_field", lambda d: d["instances"][0]["walkableSurface"].update(height=0)),
            ("moving_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(positionOffset=[0, 1, 0])),
            ("tilted_plane", lambda d: d["templates"][0]["tracks"][0]["keys"][-1].update(rotationQuaternion=[1, 0, 0, 0])),
            ("nonuniform_scale", lambda d: d["templates"][0]["tracks"][0]["keys"][0].update(scaleMultiplier=[1, 1, 2])),
        ):
            document = copy.deepcopy(source); mutate(document); cases.append((name, document, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = [re.search(r"(?ms)^function " + name + r" \{.*?^\}", publisher).group(0)
                       for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument")]
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_v3_object_publisher_accepts_source_and_rejects_invalid_resources(self) -> None:
        """Exercise the actual publisher function on a preserved source plus one model state."""
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        # Keep the real placed curtain state to cover compatibility without scanning every roulette key per mutant.
        curtain = next(row for row in source["instances"] if row["instanceId"] == "world.sequence.instance.curtain_drop")
        source["instances"] = [curtain]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == curtain["templateId"]]
        source["objectResources"] = [row for row in source.get("objectResources", [])
                                     if row.get("sequenceInstanceId") == curtain["instanceId"]]
        source["formatVersion"] = 3
        source.setdefault("objectResources", []).append({
            "objectId": "test.world.object", "displayName": "Test object", "modelAssetId": "Map/Test/test.wmodel",
            "diffuseTextureAssetId": "", "modelPreScale": 0.01, "animated": False,
            "scale": [1, 1, 1], "sequenceInstanceId": "",
        })
        source["templates"].append({
            "sequenceId": "test.world.template", "displayName": "Test state", "category": "Object",
            "durationMs": 1000, "interpolation": "LINEAR", "animationTracks": [],
            "objectMotion": {"velocity": [0, 1, 0], "acceleration": [0, -2, 0], "angularVelocityDegrees": [0, 180, 0],
                             "revolutionDegreesPerSecond": [0, 0, 0], "revolutionOffset": [0, 0, 0],
                             "count": 2, "intervalMs": 100, "spreadDegrees": 30, "seed": 7},
            "tracks": [{"slotId": "object", "keys": [
                {"timeMs": time, "positionOffset": [0, 0, 0], "rotationQuaternion": [0, 0, 0, 1],
                 "scaleMultiplier": [1, 1, 1], "visible": True} for time in (0, 1000)]}],
        })
        source["instances"].append({
            "instanceId": "test.world.instance", "templateId": "test.world.template", "enabled": True,
            "startDelayMs": 0, "playbackSpeed": 1, "anchorKind": "PLAYER", "position": [1, 2, 3],
            "bindings": [{"slotId": "object", "targetKind": "OBJECT_RESOURCE", "targetId": "test.world.object"}],
        })
        cases = [("valid", source, True)]
        with_default = copy.deepcopy(source)
        with_default["objectResources"][-1]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_valid", with_default, True))
        grouped = copy.deepcopy(source)
        grouped["instances"][-1]["anchorKind"] = "WORLD"
        grouped["objectResources"].append({
            "objectId": "test.world.group", "displayName": "Combined motions", "modelAssetId": "",
            "modelPreScale": .01, "animated": False, "scale": [1, 1, 1], "anchorKind": "WORLD",
            "motionInstanceIds": ["test.world.instance"],
        })
        cases.append(("object_group_valid", grouped, True))
        empty_materials = copy.deepcopy(grouped)
        empty_materials["objectResources"][-1]["mapMaterialBindings"] = []
        cases.append(("object_group_empty_material_bindings", empty_materials, True))
        for name, mutate in (
            ("missing", lambda d: d["objectResources"][-1].update(motionInstanceIds=["missing.motion"])),
            ("duplicate", lambda d: d["objectResources"][-1].update(motionInstanceIds=["test.world.instance"] * 2)),
            ("empty", lambda d: d["objectResources"][-1].update(motionInstanceIds=[])),
            ("model", lambda d: d["objectResources"][-1].update(modelAssetId="Map/Test/test.wmodel")),
            ("default", lambda d: d["objectResources"][-1].update(defaultMotionInstanceId="test.world.instance")),
            ("loop", lambda d: d["instances"][-1].update(motionEnd="LOOP")),
            ("player", lambda d: d["instances"][-1].update(anchorKind="PLAYER")),
            ("nested", lambda d: d["instances"][-1]["bindings"][0].update(targetId="test.world.group")),
        ):
            candidate = copy.deepcopy(grouped)
            mutate(candidate)
            cases.append(("object_group_invalid_" + name, candidate, False))
        with_effect = copy.deepcopy(source)
        with_effect["templates"][-1]["effectTracks"] = [{
            "effectTrackId": "effect.smoke", "slotId": "object", "resourceKind": "GROUP",
            "resourceId": "boss.kouku.ball.smoke", "timing": "MOTION_END", "startMs": 0,
            "durationMs": 2000, "positionOffset": [0, -2.3, -1.35],
            "rotationDegrees": [0, 0, 0], "scale": [1, 1, 1],
        }]
        cases.append(("effect_motion_end", with_effect, True))
        ten_balls = copy.deepcopy(with_effect)
        ten_balls["templates"][-1]["objectMotion"].update(count=10, intervalMs=300, spreadDegrees=360)
        cases.append(("effect_ten_complete_emissions", ten_balls, True))
        at_time = copy.deepcopy(with_effect)
        at_time["templates"][-1]["effectTracks"][0].update(timing="TIME", startMs=1000)
        cases.append(("effect_time_includes_model_end", at_time, True))
        for name, fields in (
            ("unknown_timing", {"timing": "FINISH"}), ("end_offset", {"startMs": 1}),
            ("missing_slot", {"slotId": "absent"}), ("unknown_kind", {"resourceKind": "WORLD"}),
            ("path_identity", {"resourceId": "../smoke"}), ("zero_window", {"durationMs": 0}),
            ("boolean_time", {"startMs": True}), ("time_past_end", {"timing": "TIME", "startMs": 1001}),
            ("negative_scale", {"scale": [-1, 1, 1]}), ("excessive_tail", {"durationMs": 600000}),
        ):
            invalid = copy.deepcopy(with_effect)
            invalid["templates"][-1]["effectTracks"][0].update(fields)
            cases.append(("effect_invalid_" + name, invalid, False))
        duplicate_effect = copy.deepcopy(with_effect)
        duplicate_effect["templates"][-1]["effectTracks"] *= 2
        cases.append(("effect_duplicate_track_id", duplicate_effect, False))
        for name, default_id in (("missing", "missing.instance"), ("foreign", curtain["instanceId"]),
                                 ("number", 4), ("null", None), ("empty", "")):
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["defaultMotionInstanceId"] = default_id
            cases.append(("default_motion_" + name, candidate, name == "empty"))
        disabled_default = copy.deepcopy(with_default)
        disabled_default["instances"][-1]["enabled"] = False
        cases.append(("default_motion_disabled", disabled_default, False))
        alias_default = copy.deepcopy(source)
        alias_default["objectResources"][0]["defaultMotionInstanceId"] = curtain["instanceId"]
        cases.append(("default_motion_alias_valid", alias_default, True))
        alias_default_other = copy.deepcopy(alias_default)
        alias_default_other["objectResources"][0]["defaultMotionInstanceId"] = "test.world.instance"
        cases.append(("default_motion_alias_foreign", alias_default_other, False))
        for end in ("STOP", "HOLD", "LOOP"):
            completed = copy.deepcopy(source)
            completed["instances"][-1].update(motionEnd=end, nextMotionId="")
            cases.append(("motion_end_" + end, completed, True))
        for name, fields in (
            ("unknown", {"motionEnd": "FINISH"}), ("lowercase", {"motionEnd": "hold"}),
            ("number", {"motionEnd": 1}), ("null", {"motionEnd": None}),
            ("boolean", {"motionEnd": True}), ("next_missing", {"motionEnd": "NEXT"}),
            ("next_empty", {"motionEnd": "NEXT", "nextMotionId": ""}),
            ("stop_target", {"motionEnd": "STOP", "nextMotionId": "test.world.instance"}),
            ("implicit_stop_target", {"nextMotionId": "test.world.instance"}),
            ("target_number", {"nextMotionId": 7}), ("target_null", {"nextMotionId": None}),
        ):
            invalid = copy.deepcopy(source)
            invalid["instances"][-1].update(fields)
            cases.append(("motion_end_invalid_" + name, invalid, False))
        placed_completion = copy.deepcopy(source)
        placed_completion["instances"][0]["motionEnd"] = "HOLD"
        cases.append(("motion_end_placed_target", placed_completion, False))
        next_source = copy.deepcopy(source)
        next_source["templates"][-1]["objectMotion"]["count"] = 1
        next_state = copy.deepcopy(next_source["instances"][-1])
        next_state.update(instanceId="test.world.next", motionEnd="HOLD")
        next_source["instances"][-1].update(motionEnd="NEXT", nextMotionId=next_state["instanceId"])
        next_source["instances"].append(next_state)
        cases.append(("motion_next_valid", next_source, True))
        for name, mutate in (
            ("unknown", lambda d: d["instances"][-2].update(nextMotionId="missing.motion")),
            ("case_mismatch", lambda d: d["instances"][-2].update(nextMotionId="TEST.WORLD.NEXT")),
            ("disabled", lambda d: d["instances"][-1].update(enabled=False)),
            ("self", lambda d: d["instances"][-2].update(nextMotionId="test.world.instance")),
            ("cycle", lambda d: d["instances"][-1].update(motionEnd="NEXT", nextMotionId="test.world.instance")),
            ("multiple_emissions", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
        ):
            invalid = copy.deepcopy(next_source)
            mutate(invalid)
            cases.append(("motion_next_invalid_" + name, invalid, False))
        other_resource = copy.deepcopy(next_source)
        resource_copy = copy.deepcopy(other_resource["objectResources"][-1])
        resource_copy["objectId"] = "test.other.object"
        other_resource["objectResources"].append(resource_copy)
        other_resource["instances"][-1]["bindings"][0]["targetId"] = resource_copy["objectId"]
        cases.append(("motion_next_different_resource", other_resource, False))
        other_slot = copy.deepcopy(next_source)
        template_copy = copy.deepcopy(other_slot["templates"][-1])
        template_copy["sequenceId"] = "test.other.slot.template"
        template_copy["tracks"][0]["slotId"] = "another.slot"
        other_slot["templates"].append(template_copy)
        other_slot["instances"][-1].update(templateId=template_copy["sequenceId"])
        other_slot["instances"][-1]["bindings"][0]["slotId"] = "another.slot"
        cases.append(("motion_next_different_slot", other_slot, False))
        for depth in (32, 33):
            chain = copy.deepcopy(next_source)
            prototype = chain["instances"][-1]
            chain["instances"] = [chain["instances"][0]]
            for index in range(depth + 1):
                state = copy.deepcopy(prototype)
                state.update(instanceId=f"test.motion.{index}", motionEnd="NEXT" if index < depth else "STOP")
                if index < depth:
                    state["nextMotionId"] = f"test.motion.{index + 1}"
                chain["instances"].append(state)
            cases.append((f"motion_next_depth_{depth}", chain, depth == 32))
        animation_source = copy.deepcopy(source)
        animation_source["objectResources"][-1]["animated"] = True
        animation_source["templates"][-1]["animationTracks"] = [{
            "slotId": "object", "clipName": "mn_rhoc_00_sk.ao_idle_normal_1",
            "playbackRate": 1, "loop": False, "holdLastFrame": True,
        }]
        cases.append(("animation_label_absent", animation_source, True))
        for name, label, valid in (
            ("empty", "", True), ("korean", "카드_등장", True),
            ("maximum", "a" * 128, True), ("bytes_over", "한" * 43, False),
            ("long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
            ("control", "card\nlabel", False),
        ):
            labeled = copy.deepcopy(animation_source)
            labeled["templates"][-1]["animationTracks"][0]["displayName"] = label
            cases.append(("animation_label_" + name, labeled, valid))
        for anchor in ("WORLD", "PLAYER"):
            anchored = copy.deepcopy(source)
            anchored["objectResources"][-1]["anchorKind"] = anchor
            cases.append(("resource_anchor_" + anchor, anchored, True))
        boss_source = copy.deepcopy(source)
        boss_source["objectResources"][-1].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="bip001_R_Hand")
        boss_source["instances"][-1]["anchorKind"] = "BOSS"
        cases.append(("boss_hand_anchor", boss_source, True))
        for name, bone, valid in (
            ("root", "", True), ("maximum", "a" * 128, True), ("korean", "오른손", True),
            ("bytes_over", "한" * 43, False), ("too_long", "a" * 129, False),
            ("control", "hand\n", False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBone"] = bone
            cases.append(("boss_bone_" + name, candidate, valid))
        boss_root = copy.deepcopy(boss_source)
        del boss_root["objectResources"][-1]["anchorBone"]
        cases.append(("boss_bone_absent_root", boss_root, True))
        for name, boss_id, valid in (
            ("maximum", "a" * 128, True), ("empty", "", False),
            ("space", "boss kouku", False), ("path", "../kouku", False),
            ("too_long", "a" * 129, False), ("number", 7, False),
            ("boolean", True, False), ("null", None, False),
        ):
            candidate = copy.deepcopy(boss_source)
            candidate["objectResources"][-1]["anchorBossArchetypeId"] = boss_id
            cases.append(("boss_archetype_" + name, candidate, valid))
        missing_boss = copy.deepcopy(boss_source)
        del missing_boss["objectResources"][-1]["anchorBossArchetypeId"]
        cases.append(("boss_archetype_absent", missing_boss, False))
        for anchor in ("WORLD", "PLAYER"):
            empty_fields = copy.deepcopy(source)
            empty_fields["objectResources"][-1].update(
                anchorKind=anchor, anchorBossArchetypeId="", anchorBone="")
            cases.append(("nonboss_empty_fields_" + anchor, empty_fields, True))
            for field, value in (("anchorBossArchetypeId", "BOSS_KOUKU"), ("anchorBone", "hand"),
                                 ("anchorBossArchetypeId", None), ("anchorBone", None)):
                candidate = copy.deepcopy(empty_fields)
                candidate["objectResources"][-1][field] = value
                cases.append((f"nonboss_{anchor}_{field}_{value}", candidate, False))
            candidate = copy.deepcopy(boss_source)
            candidate["instances"][-1]["anchorKind"] = anchor
            cases.append(("boss_resource_instance_mismatch_" + anchor, candidate, False))
            candidate = copy.deepcopy(source)
            candidate["objectResources"][-1]["anchorKind"] = anchor
            candidate["instances"][-1]["anchorKind"] = "BOSS"
            cases.append(("boss_instance_resource_mismatch_" + anchor, candidate, False))
        implicit_instance = copy.deepcopy(boss_source)
        del implicit_instance["instances"][-1]["anchorKind"]
        cases.append(("boss_resource_implicit_world_instance", implicit_instance, False))
        multiple_boss_bindings = copy.deepcopy(boss_source)
        second_object = copy.deepcopy(multiple_boss_bindings["objectResources"][-1])
        second_object["objectId"] = "test.second.boss.object"
        multiple_boss_bindings["objectResources"].append(second_object)
        second_track = copy.deepcopy(multiple_boss_bindings["templates"][-1]["tracks"][0])
        second_track["slotId"] = "second.object"
        multiple_boss_bindings["templates"][-1]["tracks"].append(second_track)
        multiple_boss_bindings["instances"][-1]["bindings"].append({
            "slotId": "second.object", "targetKind": "OBJECT_RESOURCE", "targetId": second_object["objectId"]})
        cases.append(("boss_anchor_multiple_resources", multiple_boss_bindings, False))
        placed_boss = copy.deepcopy(source)
        placed_boss["instances"][0]["anchorKind"] = "BOSS"
        cases.append(("boss_anchor_placed_instance", placed_boss, False))
        alias_boss = copy.deepcopy(source)
        alias_boss["objectResources"][0].update(
            anchorKind="BOSS", anchorBossArchetypeId="BOSS_KOUKU", anchorBone="")
        cases.append(("boss_anchor_placed_alias", alias_boss, False))
        for name, mutate in (
            ("bad_count", lambda d: d["templates"][-1]["objectMotion"].update(count=0)),
            ("spawn_after_lifetime", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=1000)),
            ("unknown_resource", lambda d: d["instances"][-1]["bindings"][0].update(targetId="missing.object")),
            ("path_escape", lambda d: d["objectResources"][-1].update(modelAssetId="Map/../test.wmodel")),
            ("unknown_property", lambda d: d["objectResources"][-1].update(velocty=1)),
            ("unknown_alias", lambda d: d["objectResources"][-1].update(modelAssetId="", sequenceInstanceId="missing.instance")),
            ("invalid_anchor", lambda d: d["instances"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor", lambda d: d["objectResources"][-1].update(anchorKind="UNKNOWN")),
            ("invalid_resource_anchor_type", lambda d: d["objectResources"][-1].update(anchorKind=0)),
            ("placed_alias_character_anchor", lambda d: d["objectResources"][0].update(anchorKind="PLAYER")),
        ):
            invalid = copy.deepcopy(source)
            mutate(invalid)
            cases.append((name, invalid, False))
        # Authored emission rows replace the seeded emitter: count follows the rows, interval/spread stay 0,
        # every delay stays inside the lifetime, and NEXT keeps requiring a single emission.
        emitted = copy.deepcopy(source)
        emitted["templates"][-1]["objectMotion"].update(count=3, intervalMs=0, spreadDegrees=0, emissions=[
            {"positionOffset": [0, 0, 0], "yawDegrees": 0, "startDelayMs": 0},
            {"positionOffset": [4, 0, 0], "yawDegrees": 90, "startDelayMs": 200},
            {"positionOffset": [-4, 0, 0], "yawDegrees": -90, "startDelayMs": 400}])
        cases.append(("authored_emissions_valid", emitted, True))
        for name, mutate in (
            ("count_mismatch", lambda d: d["templates"][-1]["objectMotion"].update(count=2)),
            ("interval_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(intervalMs=100)),
            ("spread_with_rows", lambda d: d["templates"][-1]["objectMotion"].update(spreadDegrees=30)),
            ("delay_after_lifetime", lambda d: d["templates"][-1]["objectMotion"]["emissions"][-1].update(startDelayMs=1000)),
            ("unknown_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].update(phase=1)),
            ("missing_row_field", lambda d: d["templates"][-1]["objectMotion"]["emissions"][0].pop("yawDegrees")),
            ("fractional_delay", lambda d: d["templates"][-1]["objectMotion"]["emissions"][1].update(startDelayMs=200.5)),
            ("empty_rows", lambda d: d["templates"][-1]["objectMotion"].update(count=1, emissions=[])),
        ):
            invalid = copy.deepcopy(emitted)
            mutate(invalid)
            cases.append(("authored_emissions_invalid_" + name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-WorldSequenceDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match)
            definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            malformed_utf8 = json.dumps(animation_source).replace(
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1"',
                '"clipName": "mn_rhoc_00_sk.ao_idle_normal_1", "displayName": "LABEL_BYTE"',
            ).encode("utf-8").replace(b"LABEL_BYTE", b"\xc0\x80")
            (folder / "animation_label_invalid_utf8.json").write_bytes(malformed_utf8)
            cases.append(("animation_label_invalid_utf8", animation_source, False))
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-WorldSequenceDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"
            script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)],
                                    capture_output=True, text=True, timeout=45)
            self.assertEqual(0, result.returncode, result.stderr)
            actual = {row["name"]: row["valid"] for row in json.loads(result.stdout)}
            self.assertEqual({name: valid for name, _, valid in cases}, actual)

    def test_world_sequences_only_scope_preserves_other_layers_and_rolls_back(self):
        area = "LV_LUT_MIDNIGHTC_ED"
        source = json.loads(read(f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"))
        card = next(row for row in source["instances"] if row["instanceId"] == "world.object.instance.kouku.card")
        source["instances"] = [card]
        source["templates"] = [row for row in source["templates"] if row["sequenceId"] == card["templateId"]]
        source["objectResources"] = [row for row in source["objectResources"] if row["objectId"] == "world.object.kouku.card"]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            catalog = root / "Data/Maps/MapCatalog.json"
            catalog.parent.mkdir(parents=True)
            # Deliberately incomplete unrelated declarations must not block this scope.
            catalog.write_text(json.dumps({"areas": [{"id": area, "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json", "sourceLights": "unrelated.invalid"}]}), encoding="utf-8")
            authoring = root / f"Data/Maps/Authoring/{area}/{area}.worldsequences.json"
            authoring.parent.mkdir(parents=True)
            authoring.write_text(json.dumps(source), encoding="utf-8")
            runtime = root / "Client/Bin/DataFiles/Map"
            runtime.mkdir(parents=True)
            preserved = {}
            for suffix in ("maplights.json", "mapplacements", "mapmaterials.json", "camerashots.json"):
                path = runtime / f"{area}.{suffix}"; path.write_bytes(b"preserved unrelated runtime\r\n"); preserved[path] = path.read_bytes()
            output = runtime / f"{area}.worldsequences.json"
            output.write_bytes(b"previous sequence runtime")
            def run(mode, fail=0):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", "WorldSequences", "-Mode", mode, "-FailureAfterPromote", str(fail)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, run("Validate").returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            failed = run("Publish", 1)
            self.assertNotEqual(0, failed.returncode)
            self.assertEqual(b"previous sequence runtime", output.read_bytes())
            applied = run("Publish")
            self.assertEqual(0, applied.returncode, applied.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("Check").returncode)
            self.assertTrue(all(path.read_bytes() == before for path, before in preserved.items()))
            prior = output.read_bytes()
            source["objectResources"][0]["defaultMotionInstanceId"] = "missing.motion"
            authoring.write_text(json.dumps(source), encoding="utf-8")
            self.assertNotEqual(0, run("Publish").returncode)
            self.assertEqual(prior, output.read_bytes())

    def test_sequence_publish_joins_installed_or_staged_map_and_deploy_targets(self):
        area = "TEST_SEQUENCE_JOIN"
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"))
        instance = next(row for row in source["instances"] if len(row["bindings"]) == 1 and row["bindings"][0]["targetKind"] == "MAP_PLACEMENT")
        template = next(row for row in source["templates"] if row["sequenceId"] == instance["templateId"])
        instance.pop("walkableSurface", None)
        instance["bindings"][0]["targetId"] = "2"
        source.update(areaId=area, instances=[instance], templates=[template], objectResources=[])
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            authoring = root / f"Data/Maps/Authoring/{area}"
            imported = root / f"Data/Maps/Imported/{area}"
            runtime = root / "Client/Bin/DataFiles/Map"
            for directory in (authoring, imported, runtime):
                directory.mkdir(parents=True)
            catalog = root / "Data/Maps/MapCatalog.json"
            entry = {"id": area, "catalogType": "single", "sourceSequences": f"Data/Maps/Authoring/{area}/{area}.worldsequences.json",
                     "sequences": f"Client/Bin/DataFiles/Map/{area}.worldsequences.json"}
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            sequence = authoring / f"{area}.worldsequences.json"
            sequence.write_text(json.dumps(source), encoding="utf-8")
            output = runtime / sequence.name
            output.write_bytes(b"previous sequence runtime")
            asset = lambda name: f'"{name}" "Fixture" "Map/{name}.wmodel" "Prototype_{name}" 1 1 1 Origin'
            placement = lambda number, name: f'{number} "editor.{number}" "fixture" "editor" "{name}" 0 0 0 0 0 0 1 1 1 1 1'
            def write_pair(folder, names, ids):
                (folder / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 1 "{area}" {len(names)}\n' + "\n".join(map(asset, names)) + "\n", encoding="utf-8")
                (folder / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" {len(ids)}\n' + "\n".join(placement(number, name) for number, name in ids) + "\n", encoding="utf-8")
            write_pair(runtime, ["OLD"], [(1, "OLD")])
            write_pair(imported, ["OLD", "NEW"], [(1, "OLD"), (2, "NEW")])
            (authoring / f"{area}.mapplacements").write_bytes((imported / f"{area}.mapplacements").read_bytes())
            def run(scope, mode="Publish"):
                return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"), "-ProjectRoot", str(root), "-AreaId", area,
                    "-Scope", scope, "-Mode", mode], capture_output=True, text=True, timeout=30)
            snapshot = lambda: {path.name: path.read_bytes() for path in runtime.iterdir() if path.is_file()}
            def reject(expected):
                before = snapshot()
                result = run("WorldSequences")
                self.assertNotEqual(0, result.returncode)
                self.assertIn(expected, result.stderr)
                self.assertEqual(before, snapshot(), "A rejected join must preserve every installed file")
            reject("Invalid Map binding")
            before = snapshot()
            result = run("Area", "Validate")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(before, snapshot())
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(source, json.loads(output.read_bytes()))
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            write_pair(runtime, ["OLD"], [(1, "OLD"), (2, "NEW")])
            reject("Invalid world sequence Map target placement/asset")
            # Sky is specifically excluded by native Collect_Placements.
            sky = asset("NEW") + ' "group" "Group" "evidence" Sky Back 1 1 0 0 1 0 1 50 1 1 1 1'
            (runtime / f"{area}.mapassets").write_text(f'LOSTARK_MAP_ASSET_CATALOG 3 "{area}" 1\n{sky}\n', encoding="utf-8")
            (runtime / f"{area}.mapplacements").write_text(f'LOSTARK_MAP_PLACEMENTS 2 "{area}" 1\n{placement(2, "NEW")}\n', encoding="utf-8")
            reject("Invalid Map binding")
            self.assertEqual(0, run("Area").returncode)
            # Single-scope joins must also consume the installed shard set.
            (runtime / f"{area}.mapset").write_text(f'LOSTARK_MAP_SHARD_SET 1 "{area}" 1\n"one" "{area}.mapassets" "{area}.mapplacements" 2 2\n', encoding="utf-8")
            entry["catalogType"] = "shard-set"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            self.assertEqual(0, run("WorldSequences", "Validate").returncode)
            entry["catalogType"] = "single"
            catalog.write_text(json.dumps({"areas": [entry]}), encoding="utf-8")
            # Deploy roles are not the complete native clip list: an arbitrary
            # named animation remains valid here and is checked by CModel later.
            instance["bindings"][0].update(targetKind="DEPLOY_PLACEMENT", targetId="11")
            template["tracks"] = []
            template["animationTracks"] = [{"slotId": instance["bindings"][0]["slotId"], "clipName": "additional.native.clip",
                "playbackRate": 1, "loop": False, "holdLastFrame": True}]
            sequence.write_text(json.dumps(source), encoding="utf-8")
            deploy_asset = '"ANIMATED" ANIM "Fixture" "Map/animated.wmodel" "Prototype_Animated" "" "" 1 0 "fixture" "on" "off"'
            deploy_catalog = f'LOSTARK_DEPLOY_PROP_CATALOG 3 "{area}" 1\n{deploy_asset}\n'
            (imported / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog, encoding="utf-8")
            deploy_row = '11 0 0 "editor.11" "ANIMATED" 0 0 0 0 0 0 1 1 0 0 0 PROJECT_AUTHORED'
            deploy_source = f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 1\n{deploy_row}\n'
            (authoring / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployplacements").write_text(f'LOSTARK_DEPLOY_PROP_PLACEMENTS 2 "{area}" 0\n', encoding="utf-8")
            reject("Invalid animated Deploy binding")
            result = run("Area")
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual(0, run("WorldSequences", "Check").returncode)
            (runtime / f"{area}.deployplacements").write_text(deploy_source.replace('"ANIMATED"', '"ABSENT"'), encoding="utf-8")
            reject("Deploy placement identity is invalid")
            (runtime / f"{area}.deployplacements").write_text(deploy_source, encoding="utf-8")
            (runtime / f"{area}.deployassets").write_text(deploy_catalog.replace(' ANIM ', ' STATIC ').replace('"on" "off"', '"" ""'), encoding="utf-8")
            reject("Invalid animated Deploy binding")

    def test_camera_pattern_authoring_fields_validate_without_changing_legacy_shots(self):
        source = json.loads(read("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json"))
        source["shots"] = [source["shots"][0]]
        cases = [("legacy", copy.deepcopy(source), True)]
        shot = source["shots"][0]
        shot.update(displayName="패턴 카메라", defaultHoldMs=3000, transitionEasing="LINEAR", activation="PATTERN_ONLY")
        cases.append(("pattern", copy.deepcopy(source), True))
        for name, fields in (
            ("name_number", {"displayName": 7}), ("name_empty", {"displayName": ""}),
            ("name_too_long", {"displayName": "가" * 43}), ("name_null", {"displayName": None}),
            ("bad_hold", {"defaultHoldMs": -1}), ("fraction_hold", {"defaultHoldMs": 1.5}),
            ("long_entry_hold", {"defaultHoldMs": 600000, "blendInMs": 1}),
            ("zero_entry_hold", {"defaultHoldMs": 0, "blendInMs": 0}),
            ("bad_easing", {"transitionEasing": "HOLD"}), ("easing_type", {"transitionEasing": True}),
            ("bad_activation", {"activation": "pattern_only"}), ("activation_null", {"activation": None}),
        ):
            invalid = copy.deepcopy(source); invalid["shots"][0].update(fields)
            cases.append((name, invalid, False))
        publisher = read("Tools/MapPipeline/Publish-MapAuthoring.ps1")
        definitions = []
        for name in ("Test-JsonNumber", "Assert-ExactJsonProperties", "Read-CameraShotDocument"):
            match = re.search(r"(?m)^function " + re.escape(name) + r" \{.*?^\}", publisher, re.DOTALL)
            self.assertIsNotNone(match); definitions.append(match.group(0))
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            for name, document, _ in cases:
                (folder / (name + ".json")).write_text(json.dumps(document), encoding="utf-8")
            script = "$ErrorActionPreference='Stop'\n$AreaId='LV_LUT_MIDNIGHTC_ED'\n" + "\n".join(definitions)
            script += "\n$results=@(); foreach($file in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.json') { try { [void](Read-CameraShotDocument $file.FullName); $ok=$true } catch { $ok=$false }; $results += [pscustomobject]@{name=$file.BaseName;valid=$ok} }; ConvertTo-Json -InputObject @($results) -Compress"
            script_path = folder / "validate.ps1"; script_path.write_text(script, encoding="utf-8-sig")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script_path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stderr)
            self.assertEqual({name: valid for name, _, valid in cases}, {r["name"]: r["valid"] for r in json.loads(result.stdout)})

    def test_client_project_registers_each_source_once(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        includes = [
            item.attrib.get("Include", "")
            for kind in ("ClInclude", "ClCompile")
            for item in project.findall(f".//msb:{kind}", namespace)
        ]
        expected = (
            r"..\Public\WorldSequenceDocument.h",
            r"..\Public\WorldSequenceToolPanel.h",
            r"..\Private\WorldSequenceDocument.cpp",
            r"..\Private\WorldSequenceToolPanel.cpp",
        )
        for path in expected:
            self.assertEqual(1, includes.count(path), path)
        panel_compile = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\WorldSequenceToolPanel.cpp']",
            namespace,
        )
        self.assertIsNotNone(panel_compile)
        options = " ".join(
            value.text or ""
            for value in panel_compile.findall("msb:AdditionalOptions", namespace)
        )
        self.assertIn("/utf-8", options)

    def test_document_is_strict_versioned_and_transactional(self) -> None:
        self.assertIn('SCHEMA = "lostark.world-sequences"', self.document_cpp)
        self.assertIn("FORMAT_VERSION = 1", self.document_cpp)
        self.assertIn("Is_ExactObject", self.document_cpp)
        self.assertIn("parse -> exact schema validation", read(
            ".md/GB/08-31/2026-08-31_WORLD_SEQUENCE_MAP_TOOL_PLAN.md"
        ))
        self.assertIn("ReplaceFileW", self.document_cpp)
        self.assertIn("MoveFileExW", self.document_cpp)
        self.assertIn("writeSucceeded = writeSucceeded && !output.fail()", self.document_cpp)

    def test_document_reads_are_bounded_and_display_text_is_safe_utf8(self) -> None:
        self.assertIn("MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u", self.document_cpp)
        self.assertIn("std::filesystem::file_size", self.document_cpp)
        self.assertIn("World sequence document exceeds the 16 MiB parse limit", self.document_cpp)
        self.assertIn("catch (const std::bad_alloc&)", self.document_cpp)
        self.assertIn("input.gcount()", self.document_cpp)
        self.assertIn("input.peek()", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.displayName)", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.category)", self.document_cpp)

    def test_document_rejects_invalid_refs_enums_and_runtime_scales(self) -> None:
        self.assertIn("availablePlacements.find(targetId)", self.document_cpp)
        self.assertIn("availableDeployPlacements.find(targetId)", self.document_cpp)
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind",
            self.document_cpp,
        )
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind",
            self.document_cpp,
        )
        self.assertIn("animationTargetSupported", self.document_cpp)
        self.assertIn("WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation", self.document_cpp)
        self.assertIn("MIN_RUNTIME_SCALE_DETERMINANT", self.document_cpp)
        self.assertIn("std::isfinite(composedX)", self.document_cpp)
        self.assertIn("Sequence scale would create a singular map transform", self.document_cpp)
        self.assertIn("boundTargets.insert(uniqueTarget).second", self.document_cpp)
        self.assertIn("sequenceTargetSupported", self.document_cpp)
        self.assertIn("MAP_ASSET_RENDER_MODE::BACKGROUND", self.panel_cpp)

    def test_preview_uses_runtime_baseline_and_never_edits_map_records(self) -> None:
        self.assertIn("Try_GetRuntimeVisible", self.panel_cpp)
        self.assertIn("baseline->runtimeVisible || !baseline->baseline.visible", self.panel_cpp)
        self.assertIn("restoredRecord.visible = target.runtimeVisible", self.panel_cpp)
        self.assertIn("Stop_AndRestore", self.panel_cpp)
        self.assertNotRegex(self.panel_cpp, r"entry\.record\s*=")
        runtime_h = read("Client/Public/MapPlacementRuntime.h")
        batch_h = read("Client/Public/MapStaticBatchObject.h")
        self.assertIn("Try_GetRuntimeVisible", runtime_h)
        self.assertIn("Try_GetInstanceVisible", batch_h)

    def test_paused_live_edit_is_validated_then_resampled(self) -> None:
        update = re.search(
            r"void Client::CWorldSequenceToolPanel::Update\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(update)
        body = update.group(0)
        self.assertIn("m_bPreviewNeedsRefresh", body)
        self.assertIn("Validate(catalog, placements, deployRuntime, validation)", body)
        self.assertIn("Apply_Preview", body)
        self.assertLess(
            body.index("Validate(catalog, placements, deployRuntime, validation)"),
            body.index("Apply_Preview"),
        )
        self.assertIn("if (m_bPreviewActive)\n\t\tm_bPreviewNeedsRefresh = true", self.panel_cpp)

    def test_selection_cannot_orphan_an_active_preview(self) -> None:
        template_list = re.search(
            r"void Client::CWorldSequenceToolPanel::Render_TemplateList\(\).*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(template_list)
        self.assertIn("ImGui::BeginDisabled(m_bPreviewActive)", template_list.group(0))
        self.assertIn("Is_PreviewActive", self.panel_h)
        self.assertIn("sequenceOwnsPreviewTargets", self.map_tool_cpp)
        self.assertIn("연출 미리보기를 Stop / Restore한 뒤", self.map_tool_cpp)

    def test_map_and_sequence_save_share_validation_and_rollback(self) -> None:
        self.assertIn("Save_PlacementsAndWorldSequences", self.map_tool_h)
        self.assertIn("PrepareAuthoringBackup", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("WriteAuthoringTransactionMarker", self.map_tool_cpp)
        self.assertIn("RecoverAuthoringTransaction", self.map_tool_cpp)
        self.assertIn("Linked save verification failed", self.map_tool_cpp)
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("m_pWorldSequenceToolPanel->Validate", body)
        self.assertIn("Save_PlacementsAndWorldSequences", body)
        toolbar = re.search(
            r"void Client::CMapTool::Render_Toolbar\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(toolbar)
        self.assertIn('ImGui::Button("Save")', toolbar.group(0))
        self.assertIn("Save_AllAuthoring()", toolbar.group(0))

    def test_linked_save_uses_area_lock_and_stale_source_cas(self) -> None:
        self.assertIn("SCOPED_AUTHORING_SAVE_LOCK", self.map_tool_cpp)
        self.assertIn(".linked-save.lock", self.map_tool_cpp)
        self.assertIn("FILE_FLAG_DELETE_ON_CLOSE", self.map_tool_cpp)
        self.assertIn("GetLastError()", self.map_tool_cpp)
        self.assertIn("ERROR_SHARING_VIOLATION", self.map_tool_cpp)
        self.assertIn("Could not open Area authoring lock", self.map_tool_cpp)
        self.assertIn("Matches_LinkedSourceBaseline", self.panel_h)
        self.assertIn("m_PlacementBaselineBytes", self.panel_h)
        self.assertIn("m_SequenceBaselineBytes", self.panel_h)
        self.assertIn("Save conflict: linked map/sequence source changed after Reload", self.panel_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("Matches_LinkedSourceBaseline"))
        self.assertLess(body.index("Matches_LinkedSourceBaseline"),
                        body.index("PrepareAuthoringBackup"))

        map_save = re.search(
            r"bool_t Client::CMapTool::Save_Placements\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(map_save)
        map_body = map_save.group(0)
        self.assertIn("return Save_PlacementsAndWorldSequences()", map_body)
        self.assertLess(map_body.index("return Save_PlacementsAndWorldSequences()"),
                        map_body.index("authoringLock.Acquire"))
        self.assertNotIn("Refresh_LinkedSourceBaseline", map_body)

    def test_linked_save_verifies_exact_intended_content(self) -> None:
        self.assertIn("AreExactlySamePlacementRecords", self.map_tool_cpp)
        self.assertIn("Has_SameDocument", self.panel_h)
        self.assertIn("Is_Equivalent", self.document_h)
        self.assertIn("Linked save verification found different Map Placement content", self.map_tool_cpp)
        self.assertIn("Linked save verification found different World Sequence content", self.map_tool_cpp)
        self.assertIn("stableVerifiedPlacements", self.map_tool_cpp)
        self.assertIn("return left == right", self.map_tool_cpp)
        self.assertIn("return left == right", self.document_cpp)
        self.assertIn("outStoredRecords", read("Client/Public/MapPlacementDocument.h"))
        placement_document = read("Client/Private/MapPlacementDocument.cpp")
        self.assertIn("UNIT_QUATERNION_TOLERANCE", placement_document)
        self.assertIn("std::abs(length - 1.f) <=", placement_document)
        self.assertIn("Linked save verification source changed during final read", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("MAX_TRANSACTION_MARKER_BYTES = 512u", self.map_tool_cpp)
        self.assertIn("transaction marker exceeds its bounded read limit", self.map_tool_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertIn("Adopt_VerifiedLinkedSourceBaseline", body)
        self.assertLess(body.rindex("ClearAuthoringTransactionMarker"),
                        body.index("Adopt_VerifiedLinkedSourceBaseline"))
        self.assertIn("editor baseline stayed unchanged", body)

    def test_reload_stages_placements_and_sequences_before_commit(self) -> None:
        reload_body = re.search(
            r"bool_t Client::CMapTool::Load_Placements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(reload_body)
        body = reload_body.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedDocument", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)
        self.assertLess(body.index("stagedWorldSequencePanel->Load_Area"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertLess(body.index("Stage_PlacementRuntime"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertIn("Is_PreviewActive", body)
        render = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(render)
        self.assertIn("Consume_ReloadAllRequest", render.group(0))
        self.assertIn("Load_Placements()", render.group(0))
        self.assertLess(render.group(0).index("Load_Placements()"),
                        render.group(0).index("return;", render.group(0).index("Load_Placements()")))

    def test_initial_area_load_recovers_an_interrupted_linked_save(self) -> None:
        switch = re.search(
            r"bool_t Client::CMapTool::Switch_EditorArea\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(switch)
        body = switch.group(0)
        self.assertIn("worldSequencePath", body)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedRecords", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)

    def test_ui_rejects_duplicate_and_background_bindings(self) -> None:
        self.assertIn("A map object can be bound to only one target slot", self.panel_cpp)
        self.assertIn("IsSequenceTargetSupported", self.panel_cpp)
        self.assertIn("[Background - unavailable]", self.panel_cpp)
        self.assertIn("sequenceTargetSupported", self.document_h)

    def test_loaded_selection_keeps_template_and_instance_coherent(self) -> None:
        load = re.search(
            r"bool_t Client::CWorldSequenceToolPanel::Load_Area\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(load)
        body = load.group(0)
        self.assertIn("m_SelectedInstanceId", body)
        self.assertIn("m_Document.Get_Instances().front().templateId", body)
        self.assertIn("placementBefore", body)
        self.assertIn("placementAfter", body)
        self.assertIn("Linked map/sequence source changed while the Area was loading", body)

    def test_visible_labels_are_english_and_help_is_korean(self) -> None:
        for label in (
            "New Sequence",
            "Sequence List",
            "Placed Instances",
            "Map Objects",
            "Add Target Track",
            "Add Key at Preview Time",
            "Stop / Restore",
        ):
            self.assertIn(label, self.panel_cpp)
        self.assertIn("ShowKoreanHelp", self.panel_cpp)
        self.assertIn("새 재사용 연출 템플릿을 만듭니다", self.panel_cpp)
        self.assertIn("미리보기는 원본 맵 배치를 수정하지 않으며", self.panel_cpp)

    def test_product_runtime_boundary_is_explicit(self) -> None:
        self.assertIn("Authoring preview only; product runtime publish is separate", self.panel_cpp)
        self.assertIn("서버 상호작용·제품 재생·길 개방은 아직 연결되지 않았습니다", self.panel_cpp)


class AnimatedPropAuthoringContractTests(unittest.TestCase):
    """MapTool owns creating the Deploy ANIM placements a sequence binds to."""

    def setUp(self) -> None:
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")
        self.catalog_h = read("Client/Public/DeployPropCatalog.h")
        self.catalog_cpp = read("Client/Private/DeployPropCatalog.cpp")

    def test_every_map_tool_declaration_has_a_definition(self) -> None:
        declared = set()
        for match in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?;",
            self.map_tool_h,
            flags=re.DOTALL,
        ):
            declared.add(match.group(1))
        defined = set(re.findall(r"CMapTool::([A-Za-z_]\w*)", self.map_tool_cpp))
        ignored = {"ETOI", "ETOUI", "float2_t", "float3_t", "float4_t"}
        missing = sorted(declared - defined - ignored)
        self.assertEqual([], missing)

    def test_animated_prop_authoring_is_reachable_from_the_sequence_mode(self) -> None:
        panel = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(panel)
        self.assertIn("Render_AnimatedPropsAuthoring();", panel.group(0))
        for label in (
            "Animated Props (Deploy ANIM)",
            "Place In Viewport",
            "Apply Transform",
            "Remove Placement",
            "Save Animated Props",
        ):
            self.assertIn(label, self.map_tool_cpp)

    def test_armed_viewport_click_places_only_in_sequence_mode(self) -> None:
        interaction = re.search(
            r"void Client::CMapTool::Update_WorldInteraction\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(interaction)
        body = interaction.group(0)
        self.assertIn(
            "if (m_bAnimatedPropPlacementArmed && mousePressed)\n"
            "\t\t\t(void)Try_PlaceSelectedDeploy();",
            body,
        )
        self.assertIn("m_bAnimatedPropPlacementArmed = false;", body)
        consumes = re.search(
            r"bool_t Client::CMapTool::ConsumesWorldLeftMouse\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(consumes)
        self.assertIn("m_bAnimatedPropPlacementArmed", consumes.group(0))

    def test_project_authored_placement_ids_stay_in_the_editor_domain(self) -> None:
        allocate = re.search(
            r"uint64_t Client::CMapTool::Allocate_AnimatedPropPlacementId\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(allocate)
        body = allocate.group(0)
        self.assertEqual(
            2, body.count("CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID")
        )
        self.assertIn("0u : candidate", body)
        # A project row may never carry extractor-only evidence fields.
        self.assertIn("0u == row.deployActorId && 0u == row.propDefinitionId", self.catalog_cpp)
        self.assertIn("PROJECT_AUTHORED", self.catalog_h)

    def test_deploy_placement_save_is_atomic_and_read_back_verified(self) -> None:
        self.assertIn("MoveFileExW", self.catalog_cpp)
        self.assertIn("MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", self.catalog_cpp)
        save = re.search(
            r"bool_t Client::CMapTool::Save_DeployPlacements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save)
        body = save.group(0)
        self.assertIn("CDeployPropCatalog verification;", body)
        self.assertIn("DeployProp placement save verification found different content", body)
        self.assertLess(body.index("verification.Load"), body.index("m_bDeployDirty = false"))

    def test_unsaved_deploy_authoring_blocks_and_saves_before_sequences(self) -> None:
        unsaved = re.search(
            r"bool_t Client::CMapTool::Has_UnsavedAuthoring\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(unsaved)
        self.assertIn("m_bDeployDirty", unsaved.group(0))
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\treturn true;\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("if (m_bDeployDirty && !Save_DeployPlacements())", body)
        self.assertLess(
            body.index("Save_DeployPlacements()"),
            body.index("Save_PlacementsAndWorldSequences()"),
        )

    def test_removing_a_bound_animated_prop_rolls_the_runtime_back(self) -> None:
        remove = re.search(
            r"bool_t Client::CMapTool::Remove_SelectedAnimatedProp\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(remove)
        body = remove.group(0)
        self.assertIn("CDeployPropCatalog restore = m_DeployRuntime.Get_Catalog();", body)
        self.assertIn("m_pWorldSequenceToolPanel->Validate(", body)
        self.assertIn("Commit_DeployCatalog(std::move(restore)", body)
        self.assertLess(body.index("Validate("), body.index("std::move(restore)"))
        self.assertIn("Source-extracted Deploy placements cannot be removed", body)

    def test_commit_releases_preview_seams_before_rebuilding_the_runtime(self) -> None:
        commit = re.search(
            r"bool_t Client::CMapTool::Commit_DeployCatalog\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(commit)
        body = commit.group(0)
        self.assertIn("Stop_AndRestore(", body)
        self.assertIn("Restore_DestructionPreview();", body)
        self.assertIn("m_pDestructionSimulationController->Clear();", body)
        self.assertLess(body.index("Stop_AndRestore("), body.index("stagedRuntime.Load("))
        self.assertLess(
            body.index("stagedRuntime.Load("),
            body.index("m_DeployRuntime = std::move(stagedRuntime);"),
        )

    def test_korean_help_remains_valid_escaped_utf8(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        map_tool = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\MapTool.cpp']", namespace
        )
        self.assertIsNotNone(map_tool)
        # MapTool now explicitly compiles as UTF-8; escaped help remains valid.

        blocks = re.findall(
            r"static const char_t\* const (ANIMATED_PROP_HELP_\w+)\s*=\s*"
            r"((?:\s*\"(?:[^\"\\]|\\.)*\")+)\s*;",
            self.map_tool_cpp,
        )
        self.assertEqual(5, len(blocks))
        for name, body in blocks:
            data = bytearray()
            for part in re.findall(r"\"((?:[^\"\\]|\\.)*)\"", body):
                index = 0
                while index < len(part):
                    if part[index] == "\\" and part[index + 1] == "x":
                        data.append(int(part[index + 2:index + 4], 16))
                        index += 4
                    elif part[index] == "\\":
                        data.append(ord(part[index + 1]))
                        index += 2
                    else:
                        data.append(ord(part[index]))
                        index += 1
                # An \xHH escape followed by another hex digit would swallow it.
                self.assertIsNone(
                    re.search(r"\\x[0-9A-Fa-f]{2}[0-9A-Fa-f]", part), name
                )
            text = data.decode("utf-8")
            self.assertTrue(any("\uac00" <= ch <= "\ud7a3" for ch in text), name)



if __name__ == "__main__":
    unittest.main(verbosity=2)

```
