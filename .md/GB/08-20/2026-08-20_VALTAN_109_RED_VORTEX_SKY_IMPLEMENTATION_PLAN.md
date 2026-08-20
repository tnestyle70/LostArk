# 발탄 109줄 붉은 소용돌이 하늘 구현 계획

## G01. Server 권위 109 stage에 하늘 데이터 연결

- 목표: `VALTAN_ARENA_BREAK_109`의 TAKEOFF부터 RECOVERY까지 붉은 구름과 검은 aperture를 같은 권위 시간축으로 표시한다.
- 종료 증거: camera/sky JSON의 6개 stage가 `ValtanEncounter.json`의 stage 순서·시간·landing anchor와 일치하고 publisher Validate가 통과한다.
- 데이터 정본: `Data/Encounters/Valtan/ValtanCinematicCamera.json`.
- Server·Shared 변경은 없다. 기존 snapshot의 pattern ID, stage index/action ID, pattern sequence, server tick, action start tick만 소비한다.

## G02. 하늘 cue parse와 시간 보간

- `CValtanCinematicCameraDocument`는 formatVersion 3, exact field, stable seed asset ID, 곡선 범위를 parse → validate → stage → commit한다. 실패하면 기존 ready 문서를 보존한다.
- `CValtanCinematicCameraController::Resolve_SkyState`는 새 snapshot에서 권위 action age로 seek하고 동일 snapshot 사이에서는 최대 0.1초만 전진한다.
- cue가 바뀌기 전 local 시간이 끝을 넘으면 마지막 fade/scale 값을 hold한다. 이전 stage duration×회전 속도를 합쳐 stage 경계와 late join의 회전 위상을 보존한다.
- camera cue가 없다는 이유로 sky timeline을 reset하지 않고, sky cue가 없다는 이유로 camera override를 reset하지 않는다.

## G03. 기존 ValtanPhase 6개 레이어를 실제 렌더에 연결

- `CLevel_ValtanArena::Ready_ValtanSkyPresentation`은 두 seed placement의 sourceLevel을 찾고 각 그룹이 정확히 아래 3개의 non-batched `CMapAssetObject`인지 전부 검증한 뒤 한 번에 commit한다.
- 붉은 구름 그룹: `VALTAN_PHASE_CHAOS_CLOUD/ELECTRIC/RING`.
- 검은 aperture 그룹: `VALTAN_PHASE_SPACEHOLE_CLOUD/CORE/STREAK`.
- 기존 SpaceHole 3장은 모두 Additive이므로 첨부 화면의 검은 중심을 만들 수 없다. `SPACEHOLE_CORE`는 centered dark Alpha aperture로, `CHAOS_RING`은 centered broken Additive annulus로만 presentation override한다. 나머지 맵 에셋은 원래 shader/profile을 그대로 쓴다.
- raw UV로 원형 중심을 고정하고 authored UV pan은 내부 cloud breakup에만 사용한다. `CShader` clone이 공유하는 FX11 Effect state는 매 draw 성공·실패 뒤 `NONE/0`으로 복구해 캐릭터·무기·일반 맵으로 누수되지 않게 한다.
- `CMapAssetObject`는 authored opacity를 보존하는 presentation multiplier도 함께 받는다. hidden object는 이미 render queue에 들어갔더라도 draw 직전에 다시 거부한다.
- Level은 기존 transform을 보존하고 cue opacity/scale/회전을 적용한다. 종료·중단·disconnect·level exit에는 base transform 복구 후 숨긴다.
- 연결 종료 복귀 요청이 거부된 프레임도 즉시 return하여 stale snapshot으로 sky를 다시 적용하지 않는다.

## G04. 카메라 구도와 검증

- 최초 구현 재검토에서 TAKEOFF 시작은 하늘 중심과 약 80~90도 어긋나고 DROP 끝에는 SpaceHole 중심이 세로 FOV 밖으로 벗어나는 것을 확인했다. 단순 `lookAt.y > eye.y` 검사는 화면 구도를 보장하지 못한다.
- TAKEOFF 첫 프레임부터 상공을 보고, terminal TAKEOFF pose `eye [151.03,32.5,-117.06] / lookAt [156.03,70.5,-122.06]`를 DROP 끝까지 유지한다. IMPACT 첫 프레임도 같은 pose/FOV로 시작한 뒤 400ms 동안 landing anchor로 복귀한다. X/Z anchor 계약은 유지한다.
- `ClientFrontendHarness --valtan-camera-fast`는 기존 시간축·rollback 검증에 더해 TAKEOFF/DROP의 모든 30Hz 권위 tick에서 실제 SpaceHole/Chaos 중심이 카메라 앞쪽이면서 화면 중앙 세로 40% 안에 함께 있는지 검사한다. DROP 마지막 pose와 IMPACT 첫 pose도 완전히 같아야 한다.
- 새 C++ 파일이 없으므로 `.vcxproj`와 `.filters` 등록 변경은 없다.
- Client/UI는 에이전트가 실행하지 않는다. 실제 색·크기·회전 속도·화면 점유율은 사용자가 Lobby → Valtan → F1 → `Reset + Play 109 Only`로 판정한다.

## 변경 파일

- `Client/Public/ValtanCinematicCameraDocument.h`
- `Client/Private/ValtanCinematicCameraDocument.cpp`
- `Client/Public/ValtanCinematicCameraController.h`
- `Client/Private/ValtanCinematicCameraController.cpp`
- `Client/Public/MapAssetObject.h`
- `Client/Private/MapAssetObject.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- `Data/Encounters/Valtan/ValtanCinematicCamera.json`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- 대응 RESULT 문서

## 1차 구현 전체 코드와 재검토 교정

아래 H/CPP와 JSON은 1차 구현 커밋 `cfceb379`의 전체 코드다. 사용자의 재확인 요청으로 발견한 카메라 구도와 Additive aperture 결함은 위 G03/G04 계약과 대응 RESULT의 후속 교정 항목이 우선하며, 현재 실행 정본은 저장소의 실제 소스다. 대형 실행형 하네스는 적용 가능한 unified diff를 싣는다.

### Client/Public/ValtanCinematicCameraDocument.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CEncounterPatternReference;

struct VALTAN_CINEMATIC_CAMERA_KEYFRAME final
{
    uint32_t iTimeMs = 0u;
    float3_t vEye = {};
    float3_t vLookAt = {};
    f32_t fFovYDegrees = 60.f;
};

/* Linear alone reads as a dolly running at constant speed. Smoothstep eases
   the ends so a stage can settle, and hold keeps a framing locked while the
   arena collapses in front of it. */
enum class VALTAN_CINEMATIC_CAMERA_EASING
{
    LINEAR,
    SMOOTHSTEP,
    HOLD,
    END
};

struct VALTAN_CINEMATIC_CAMERA_CUE final
{
    std::string strCueId;
    std::string strPatternId;
    std::string strStageId;
    std::string strStageActionId;
    uint32_t iStageIndex = 0u;
    uint32_t iDurationMs = 0u;
    VALTAN_CINEMATIC_CAMERA_EASING eEasing =
        VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
    /* Deterministic landing jolt. Amplitude is in world units and decays to
       nothing across its own duration, so the shake can never outlive the cue
       or diverge between clients. */
    f32_t fShakeAmplitude = 0.f;
    uint32_t iShakeDurationMs = 0u;
    std::vector<VALTAN_CINEMATIC_CAMERA_KEYFRAME> Keyframes;
};

/* The 109 sky is a timed presentation layer, never a skybox swap and never a
   gameplay state: it turns on and off across the six authoritative transition
   stages and touches no collision or navigation. The two stable asset IDs seed
   the red-cloud and black-aperture presentation composites. */
struct VALTAN_CINEMATIC_SKY_CUE final
{
    std::string strCueId;
    std::string strPatternId;
    std::string strStageId;
    std::string strStageActionId;
    uint32_t iStageIndex = 0u;
    uint32_t iStageLocalStartMs = 0u;
    uint32_t iStageLocalEndMs = 0u;
    std::string strRedCloudAssetId;
    std::string strBlackApertureAssetId;
    f32_t fCloudOpacityStart = 0.f;
    f32_t fCloudOpacityEnd = 0.f;
    f32_t fApertureScaleStart = 0.f;
    f32_t fApertureScaleEnd = 0.f;
    f32_t fCloudRotationDegreesPerSecond = 0.f;
};

class CValtanCinematicCameraDocument final
{
public:
    bool_t Load(
        const std::filesystem::path& path,
        const CEncounterPatternReference& encounter,
        std::string& outStatus);
    static bool_t Parse_Text(
        std::string_view text,
        const CEncounterPatternReference& encounter,
        CValtanCinematicCameraDocument& outDocument,
        std::string& outStatus);
    void Clear();

    bool_t Is_Ready() const { return m_isReady; }
    const std::string& Get_EncounterId() const { return m_strEncounterId; }
    const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& Get_Cues() const
    {
        return m_Cues;
    }
    const VALTAN_CINEMATIC_CAMERA_CUE* Find_Cue(
        std::string_view patternId,
        uint32_t stageIndex,
        std::string_view stageActionId) const;
    const std::vector<VALTAN_CINEMATIC_SKY_CUE>& Get_SkyCues() const
    {
        return m_SkyCues;
    }
    const VALTAN_CINEMATIC_SKY_CUE* Find_SkyCue(
        std::string_view patternId,
        uint32_t stageIndex,
        std::string_view stageActionId) const;
    /* The clear shot has no pattern to key on, so it is looked up by the boss
       death action instead. Null when the encounter authors none. */
    const VALTAN_CINEMATIC_CAMERA_CUE* Find_DeathCue() const
    {
        return m_hasDeathCue ? &m_DeathCue : nullptr;
    }

private:
    std::string m_strEncounterId;
    std::vector<VALTAN_CINEMATIC_CAMERA_CUE> m_Cues;
    std::vector<VALTAN_CINEMATIC_SKY_CUE> m_SkyCues;
    VALTAN_CINEMATIC_CAMERA_CUE m_DeathCue;
    bool_t m_hasDeathCue = false;
    bool_t m_isReady = false;
};

NS_END
```

### Client/Private/ValtanCinematicCameraDocument.cpp 전체 코드

```cpp
#include "ValtanCinematicCameraDocument.h"

#include "DataJson.h"
#include "EncounterPatternReference.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
    using namespace Client;

    constexpr const char_t* SCHEMA = "lostark.encounter-cinematic-camera";
    constexpr uint32_t FORMAT_VERSION = 3u;
    constexpr f32_t MAX_SHAKE_AMPLITUDE = 2.f;
    constexpr uint32_t MAX_SHAKE_DURATION_MS = 1000u;
    constexpr uint32_t MAX_CUE_COUNT = 32u;
    constexpr uint32_t MAX_KEYFRAME_COUNT = 64u;
    constexpr f32_t MAX_WORLD_COORDINATE = 100000.f;
    constexpr size_t MAX_SKY_ASSET_ID_LENGTH = 128u;

    bool_t Is_StablePresentationAssetId(const std::string_view value)
    {
        return !value.empty() && value.size() <= MAX_SKY_ASSET_ID_LENGTH &&
            std::all_of(value.begin(), value.end(), [](const char_t character)
            {
                return (character >= 'a' && character <= 'z') ||
                    (character >= 'A' && character <= 'Z') ||
                    (character >= '0' && character <= '9') ||
                    '_' == character || '-' == character || '.' == character;
            });
    }

    bool_t Is_ExactObject(
        const DATA_JSON_VALUE& value,
        const std::initializer_list<const char_t*> keys)
    {
        if (!value.Is_Object() || value.Get_Object().size() != keys.size())
            return false;
        return std::all_of(keys.begin(), keys.end(),
            [&value](const char_t* key) { return nullptr != value.Find(key); });
    }

    bool_t Read_String(
        const DATA_JSON_VALUE& parent,
        const char_t* key,
        std::string& outValue)
    {
        const DATA_JSON_VALUE* value = parent.Find(key);
        if (nullptr == value || !value->Is_String() || value->Get_String().empty())
            return false;
        outValue = value->Get_String();
        return true;
    }

    bool_t Read_Unsigned(
        const DATA_JSON_VALUE& parent,
        const char_t* key,
        const uint32_t maximum,
        uint32_t& outValue)
    {
        const DATA_JSON_VALUE* value = parent.Find(key);
        if (nullptr == value || !value->Is_Number())
            return false;
        const double number = value->Get_Number();
        if (!std::isfinite(number) || number < 0.0 ||
            number > static_cast<double>(maximum) || std::floor(number) != number)
        {
            return false;
        }
        outValue = static_cast<uint32_t>(number);
        return true;
    }

    bool_t Read_Easing(
        const DATA_JSON_VALUE& parent,
        VALTAN_CINEMATIC_CAMERA_EASING& outValue)
    {
        std::string text;
        if (!Read_String(parent, "easing", text))
            return false;
        if ("LINEAR" == text)
            outValue = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
        else if ("SMOOTHSTEP" == text)
            outValue = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
        else if ("HOLD" == text)
            outValue = VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
        else
            return false;
        return true;
    }

    /* Amplitude and duration are read together because a shake with only one
       of them authored is an authoring mistake, not a silent no-op. */
    bool_t Read_Shake(
        const DATA_JSON_VALUE& parent,
        VALTAN_CINEMATIC_CAMERA_CUE& outCue)
    {
        const DATA_JSON_VALUE* amplitude = parent.Find("shakeAmplitude");
        if (nullptr == amplitude || !amplitude->Is_Number())
            return false;
        const double value = amplitude->Get_Number();
        if (!std::isfinite(value) || value < 0.0 ||
            value > static_cast<double>(MAX_SHAKE_AMPLITUDE))
        {
            return false;
        }
        outCue.fShakeAmplitude = static_cast<f32_t>(value);
        if (!Read_Unsigned(parent, "shakeDurationMs", MAX_SHAKE_DURATION_MS,
            outCue.iShakeDurationMs))
        {
            return false;
        }
        const bool_t hasAmplitude = outCue.fShakeAmplitude > 0.f;
        const bool_t hasDuration = 0u != outCue.iShakeDurationMs;
        return hasAmplitude == hasDuration &&
            outCue.iShakeDurationMs <= outCue.iDurationMs;
    }

    bool_t Read_Float3(
        const DATA_JSON_VALUE& parent,
        const char_t* key,
        float3_t& outValue)
    {
        const DATA_JSON_VALUE* value = parent.Find(key);
        if (nullptr == value || !value->Is_Array() ||
            3u != value->Get_Array().size())
        {
            return false;
        }
        f32_t components[3]{};
        for (size_t index = 0u; index < 3u; ++index)
        {
            const DATA_JSON_VALUE& component = value->Get_Array()[index];
            if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
                std::abs(component.Get_Number()) > MAX_WORLD_COORDINATE)
            {
                return false;
            }
            components[index] = static_cast<f32_t>(component.Get_Number());
        }
        outValue = float3_t(components[0], components[1], components[2]);
        return true;
    }

    bool_t Read_Fov(
        const DATA_JSON_VALUE& parent,
        f32_t& outValue)
    {
        const DATA_JSON_VALUE* value = parent.Find("fovYDegrees");
        if (nullptr == value || !value->Is_Number() ||
            !std::isfinite(value->Get_Number()) ||
            value->Get_Number() < 10.0 || value->Get_Number() > 120.0)
        {
            return false;
        }
        outValue = static_cast<f32_t>(value->Get_Number());
        return true;
    }
}

bool_t Client::CValtanCinematicCameraDocument::Load(
    const std::filesystem::path& path,
    const CEncounterPatternReference& encounter,
    std::string& outStatus)
{
    std::ifstream input(path, std::ios::binary);
    if (path.empty() || !input.is_open())
    {
        outStatus = "Cinematic camera document is unreadable: " + path.string();
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (input.bad())
    {
        outStatus = "Cinematic camera document read failed: " + path.string();
        return false;
    }
    return Parse_Text(buffer.str(), encounter, *this, outStatus);
}

bool_t Client::CValtanCinematicCameraDocument::Parse_Text(
    const std::string_view text,
    const CEncounterPatternReference& encounter,
    CValtanCinematicCameraDocument& outDocument,
    std::string& outStatus)
{
    if (!encounter.Is_Ready())
    {
        outStatus = "Cinematic camera requires a validated encounter reference";
        return false;
    }
    DATA_JSON_VALUE root;
    std::string parseError;
    DATA_JSON_PARSE_LIMITS limits{};
    limits.iMaximumBytes = 512u * 1024u;
    limits.iMaximumDepth = 16u;
    limits.iMaximumValues = 4096u;
    if (!CDataJson::Parse(text, root, parseError, limits))
    {
        outStatus = "Cinematic camera parse failed: " + parseError;
        return false;
    }
    if (!Is_ExactObject(root,
        { "schema", "formatVersion", "encounterId", "provenance", "cues",
            "skyCues", "deathCue" }))
    {
        outStatus = "Cinematic camera root has unexpected properties";
        return false;
    }

    std::string schema;
    std::string encounterId;
    std::string provenance;
    uint32_t formatVersion = 0u;
    if (!Read_String(root, "schema", schema) || SCHEMA != schema ||
        !Read_Unsigned(root, "formatVersion", FORMAT_VERSION, formatVersion) ||
        FORMAT_VERSION != formatVersion ||
        !Read_String(root, "encounterId", encounterId) ||
        encounterId != encounter.Get_EncounterId() ||
        !Read_String(root, "provenance", provenance) ||
        "PROJECT_AUTHORED" != provenance)
    {
        outStatus = "Cinematic camera header is invalid";
        return false;
    }

    const DATA_JSON_VALUE* cues = root.Find("cues");
    if (nullptr == cues || !cues->Is_Array() || cues->Get_Array().empty() ||
        cues->Get_Array().size() > MAX_CUE_COUNT)
    {
        outStatus = "Cinematic camera cue array is invalid";
        return false;
    }

    std::vector<VALTAN_CINEMATIC_CAMERA_CUE> staged;
    std::unordered_set<std::string> cueIds;
    std::unordered_set<std::string> tuples;
    for (const DATA_JSON_VALUE& cueValue : cues->Get_Array())
    {
        if (!Is_ExactObject(cueValue,
            { "cueId", "patternId", "stageId", "durationMs", "easing",
                "shakeAmplitude", "shakeDurationMs", "keyframes" }))
        {
            outStatus = "Cinematic camera cue has unexpected properties";
            return false;
        }
        VALTAN_CINEMATIC_CAMERA_CUE cue;
        if (!Read_String(cueValue, "cueId", cue.strCueId) ||
            !Read_String(cueValue, "patternId", cue.strPatternId) ||
            !Read_String(cueValue, "stageId", cue.strStageId) ||
            !Read_Unsigned(cueValue, "durationMs",
                CEncounterPatternReference::MAX_STAGE_DURATION_MS, cue.iDurationMs) ||
            0u == cue.iDurationMs || !cueIds.insert(cue.strCueId).second ||
            !Read_Easing(cueValue, cue.eEasing) ||
            !Read_Shake(cueValue, cue))
        {
            outStatus = "Cinematic camera cue identity is invalid";
            return false;
        }

        const ENCOUNTER_PATTERN_REFERENCE* pattern =
            encounter.Find_Pattern(cue.strPatternId);
        if (nullptr == pattern)
        {
            outStatus = "Cinematic camera pattern is unknown: " + cue.strPatternId;
            return false;
        }
        const auto stage = std::find_if(pattern->stages.begin(), pattern->stages.end(),
            [&cue](const ENCOUNTER_STAGE_REFERENCE& value)
            { return value.stageId == cue.strStageId; });
        if (pattern->stages.end() == stage || cue.iDurationMs > stage->iDurationMs)
        {
            outStatus = "Cinematic camera stage is invalid: " + cue.strCueId;
            return false;
        }
        cue.iStageIndex = static_cast<uint32_t>(stage - pattern->stages.begin());
        cue.strStageActionId = stage->actionId;
        const std::string tuple = cue.strPatternId + "\n" +
            std::to_string(cue.iStageIndex) + "\n" + cue.strStageActionId;
        if (!tuples.insert(tuple).second)
        {
            outStatus = "Duplicate cinematic camera encounter tuple";
            return false;
        }

        const DATA_JSON_VALUE* keyframes = cueValue.Find("keyframes");
        if (nullptr == keyframes || !keyframes->Is_Array() ||
            keyframes->Get_Array().size() < 2u ||
            keyframes->Get_Array().size() > MAX_KEYFRAME_COUNT)
        {
            outStatus = "Cinematic camera keyframe array is invalid: " + cue.strCueId;
            return false;
        }
        uint32_t previousTime = 0u;
        for (size_t index = 0u; index < keyframes->Get_Array().size(); ++index)
        {
            const DATA_JSON_VALUE& keyframeValue = keyframes->Get_Array()[index];
            if (!Is_ExactObject(keyframeValue,
                { "timeMs", "eye", "lookAt", "fovYDegrees" }))
            {
                outStatus = "Cinematic camera keyframe has unexpected properties";
                return false;
            }
            VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
            if (!Read_Unsigned(keyframeValue, "timeMs", cue.iDurationMs,
                    keyframe.iTimeMs) ||
                !Read_Float3(keyframeValue, "eye", keyframe.vEye) ||
                !Read_Float3(keyframeValue, "lookAt", keyframe.vLookAt) ||
                !Read_Fov(keyframeValue, keyframe.fFovYDegrees) ||
                (index == 0u && 0u != keyframe.iTimeMs) ||
                (index > 0u && keyframe.iTimeMs <= previousTime))
            {
                outStatus = "Cinematic camera keyframe is invalid: " + cue.strCueId;
                return false;
            }
            const vector_t eye = XMLoadFloat3(&keyframe.vEye);
            const vector_t lookAt = XMLoadFloat3(&keyframe.vLookAt);
            if (XMVectorGetX(XMVector3LengthSq(lookAt - eye)) <= 0.000001f)
            {
                outStatus = "Cinematic camera eye and lookAt must differ";
                return false;
            }
            previousTime = keyframe.iTimeMs;
            cue.Keyframes.push_back(keyframe);
        }
        if (previousTime != cue.iDurationMs)
        {
            outStatus = "Cinematic camera final keyframe must match cue duration";
            return false;
        }
        staged.push_back(std::move(cue));
    }

    const DATA_JSON_VALUE* skyCues = root.Find("skyCues");
    if (nullptr == skyCues || !skyCues->Is_Array() ||
        skyCues->Get_Array().size() > MAX_CUE_COUNT)
    {
        outStatus = "Cinematic sky cue array is invalid";
        return false;
    }
    std::vector<VALTAN_CINEMATIC_SKY_CUE> stagedSky;
    std::unordered_set<std::string> skyCueIds;
    std::unordered_set<std::string> skyTuples;
    for (const DATA_JSON_VALUE& cueValue : skyCues->Get_Array())
    {
        if (!Is_ExactObject(cueValue,
            { "cueId", "patternId", "stageId", "stageLocalStartMs",
                "stageLocalEndMs", "redCloudAssetId", "blackApertureAssetId",
                "cloudOpacityStart", "cloudOpacityEnd", "apertureScaleStart",
                "apertureScaleEnd", "cloudRotationDegreesPerSecond" }))
        {
            outStatus = "Cinematic sky cue has unexpected properties";
            return false;
        }
        VALTAN_CINEMATIC_SKY_CUE cue;
        if (!Read_String(cueValue, "cueId", cue.strCueId) ||
            !Read_String(cueValue, "patternId", cue.strPatternId) ||
            !Read_String(cueValue, "stageId", cue.strStageId) ||
            !skyCueIds.insert(cue.strCueId).second)
        {
            outStatus = "Cinematic sky cue identity is invalid";
            return false;
        }
        const ENCOUNTER_PATTERN_REFERENCE* pattern =
            encounter.Find_Pattern(cue.strPatternId);
        if (nullptr == pattern)
        {
            outStatus = "Cinematic sky pattern is unknown: " + cue.strPatternId;
            return false;
        }
        const auto stage = std::find_if(
            pattern->stages.begin(), pattern->stages.end(),
            [&cue](const ENCOUNTER_STAGE_REFERENCE& value)
            { return value.stageId == cue.strStageId; });
        if (pattern->stages.end() == stage)
        {
            outStatus = "Cinematic sky stage is unknown: " + cue.strCueId;
            return false;
        }
        cue.iStageIndex = static_cast<uint32_t>(stage - pattern->stages.begin());
        cue.strStageActionId = stage->actionId;
        if (!Read_Unsigned(cueValue, "stageLocalStartMs", stage->iDurationMs,
                cue.iStageLocalStartMs) ||
            !Read_Unsigned(cueValue, "stageLocalEndMs", stage->iDurationMs,
                cue.iStageLocalEndMs) ||
            cue.iStageLocalEndMs <= cue.iStageLocalStartMs)
        {
            outStatus = "Cinematic sky window is invalid: " + cue.strCueId;
            return false;
        }
        const std::string tuple = cue.strPatternId + "\n" +
            std::to_string(cue.iStageIndex) + "\n" + cue.strStageActionId;
        if (!skyTuples.insert(tuple).second)
        {
            outStatus = "Duplicate cinematic sky encounter tuple";
            return false;
        }
        if (!Read_String(cueValue, "redCloudAssetId",
                cue.strRedCloudAssetId) ||
            !Read_String(cueValue, "blackApertureAssetId",
                cue.strBlackApertureAssetId) ||
            !Is_StablePresentationAssetId(cue.strRedCloudAssetId) ||
            !Is_StablePresentationAssetId(cue.strBlackApertureAssetId))
        {
            outStatus = "Cinematic sky asset IDs must be stable non-path IDs";
            return false;
        }

        const auto readUnitScalar = [&cueValue](
            const char_t* key, const f32_t maximum, f32_t& outValue)
        {
            const DATA_JSON_VALUE* value = cueValue.Find(key);
            if (nullptr == value || !value->Is_Number() ||
                !std::isfinite(value->Get_Number()) ||
                value->Get_Number() < 0.0 ||
                value->Get_Number() > static_cast<double>(maximum))
            {
                return false;
            }
            outValue = static_cast<f32_t>(value->Get_Number());
            return true;
        };
        const DATA_JSON_VALUE* rotation =
            cueValue.Find("cloudRotationDegreesPerSecond");
        if (!readUnitScalar("cloudOpacityStart", 1.f, cue.fCloudOpacityStart) ||
            !readUnitScalar("cloudOpacityEnd", 1.f, cue.fCloudOpacityEnd) ||
            !readUnitScalar("apertureScaleStart", 8.f, cue.fApertureScaleStart) ||
            !readUnitScalar("apertureScaleEnd", 8.f, cue.fApertureScaleEnd) ||
            nullptr == rotation || !rotation->Is_Number() ||
            !std::isfinite(rotation->Get_Number()) ||
            std::abs(rotation->Get_Number()) > 360.0)
        {
            outStatus = "Cinematic sky curve is invalid: " + cue.strCueId;
            return false;
        }
        cue.fCloudRotationDegreesPerSecond =
            static_cast<f32_t>(rotation->Get_Number());
        stagedSky.push_back(std::move(cue));
    }

    /* The clear shot is optional: an empty object means the encounter authors no
       death camera and the ordinary follow view simply stays. */
    VALTAN_CINEMATIC_CAMERA_CUE deathCue;
    bool_t hasDeathCue = false;
    const DATA_JSON_VALUE* deathCueValue = root.Find("deathCue");
    if (nullptr == deathCueValue || !deathCueValue->Is_Object())
    {
        outStatus = "Cinematic death cue must be an object";
        return false;
    }
    if (!deathCueValue->Get_Object().empty())
    {
        if (!Is_ExactObject(*deathCueValue,
            { "cueId", "durationMs", "easing", "shakeAmplitude",
                "shakeDurationMs", "keyframes" }))
        {
            outStatus = "Cinematic death cue has unexpected properties";
            return false;
        }
        if (!Read_String(*deathCueValue, "cueId", deathCue.strCueId) ||
            !Read_Unsigned(*deathCueValue, "durationMs",
                CEncounterPatternReference::MAX_STAGE_DURATION_MS,
                deathCue.iDurationMs) ||
            0u == deathCue.iDurationMs ||
            !Read_Easing(*deathCueValue, deathCue.eEasing) ||
            !Read_Shake(*deathCueValue, deathCue))
        {
            outStatus = "Cinematic death cue identity is invalid";
            return false;
        }
        const DATA_JSON_VALUE* deathKeyframes = deathCueValue->Find("keyframes");
        if (nullptr == deathKeyframes || !deathKeyframes->Is_Array() ||
            deathKeyframes->Get_Array().size() < 2u ||
            deathKeyframes->Get_Array().size() > MAX_KEYFRAME_COUNT)
        {
            outStatus = "Cinematic death cue keyframe array is invalid";
            return false;
        }
        uint32_t previousDeathTime = 0u;
        for (size_t index = 0u; index < deathKeyframes->Get_Array().size();
            ++index)
        {
            const DATA_JSON_VALUE& keyframeValue =
                deathKeyframes->Get_Array()[index];
            if (!Is_ExactObject(keyframeValue,
                { "timeMs", "eye", "lookAt", "fovYDegrees" }))
            {
                outStatus = "Cinematic death keyframe has unexpected properties";
                return false;
            }
            VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
            if (!Read_Unsigned(keyframeValue, "timeMs", deathCue.iDurationMs,
                    keyframe.iTimeMs) ||
                !Read_Float3(keyframeValue, "eye", keyframe.vEye) ||
                !Read_Float3(keyframeValue, "lookAt", keyframe.vLookAt) ||
                !Read_Fov(keyframeValue, keyframe.fFovYDegrees) ||
                (index == 0u && 0u != keyframe.iTimeMs) ||
                (index > 0u && keyframe.iTimeMs <= previousDeathTime))
            {
                outStatus = "Cinematic death keyframe is invalid";
                return false;
            }
            previousDeathTime = keyframe.iTimeMs;
            deathCue.Keyframes.push_back(keyframe);
        }
        if (previousDeathTime != deathCue.iDurationMs)
        {
            outStatus = "Cinematic death cue final keyframe must match duration";
            return false;
        }
        hasDeathCue = true;
    }

    CValtanCinematicCameraDocument committed;
    committed.m_strEncounterId = std::move(encounterId);
    committed.m_Cues = std::move(staged);
    committed.m_SkyCues = std::move(stagedSky);
    committed.m_DeathCue = std::move(deathCue);
    committed.m_hasDeathCue = hasDeathCue;
    committed.m_isReady = true;
    outDocument = std::move(committed);
    outStatus = "Loaded Valtan cinematic camera cues: " +
        std::to_string(outDocument.m_Cues.size()) + " and sky cues: " +
        std::to_string(outDocument.m_SkyCues.size());
    return true;
}

const Client::VALTAN_CINEMATIC_SKY_CUE*
Client::CValtanCinematicCameraDocument::Find_SkyCue(
    const std::string_view patternId,
    const uint32_t stageIndex,
    const std::string_view stageActionId) const
{
    const auto iter = std::find_if(m_SkyCues.begin(), m_SkyCues.end(),
        [patternId, stageIndex, stageActionId](
            const VALTAN_CINEMATIC_SKY_CUE& cue)
        {
            return cue.strPatternId == patternId &&
                cue.iStageIndex == stageIndex &&
                cue.strStageActionId == stageActionId;
        });
    return m_SkyCues.end() == iter ? nullptr : &(*iter);
}

void Client::CValtanCinematicCameraDocument::Clear()
{
    m_strEncounterId.clear();
    m_Cues.clear();
    m_SkyCues.clear();
    m_DeathCue = VALTAN_CINEMATIC_CAMERA_CUE{};
    m_hasDeathCue = false;
    m_isReady = false;
}

const Client::VALTAN_CINEMATIC_CAMERA_CUE*
Client::CValtanCinematicCameraDocument::Find_Cue(
    const std::string_view patternId,
    const uint32_t stageIndex,
    const std::string_view stageActionId) const
{
    const auto iter = std::find_if(m_Cues.begin(), m_Cues.end(),
        [patternId, stageIndex, stageActionId](
            const VALTAN_CINEMATIC_CAMERA_CUE& cue)
        {
            return cue.strPatternId == patternId &&
                cue.iStageIndex == stageIndex &&
                cue.strStageActionId == stageActionId;
        });
    return m_Cues.end() == iter ? nullptr : &(*iter);
}
```

### Client/Public/ValtanCinematicCameraController.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "ValtanCinematicCameraDocument.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

struct VALTAN_CINEMATIC_CAMERA_INPUT final
{
    bool_t isValid = false;
    uint64_t iNetEntityId = 0u;
    uint32_t iServerTick = 0u;
    std::string strPatternId;
    std::string strStageId;
    std::string strStageActionId;
    uint32_t iPatternSequence = 0u;
    uint32_t iStageIndex = 0u;
    uint32_t iActionStartTick = 0u;
    /* Death has no pattern to key on, so the clear shot is selected by this flag
       and still runs off the authoritative action start tick. */
    bool_t isBossDead = false;
};

struct VALTAN_CINEMATIC_CAMERA_POSE final
{
    float3_t vEye = {};
    float3_t vLookAt = {};
    f32_t fFovYDegrees = 60.f;
};

/* Presentation-only sky state resolved from the same authoritative tuple as the
   camera, so there is one time axis. It never touches collision or navigation,
   and it is inactive whenever no authored window covers the current action age. */
struct VALTAN_CINEMATIC_SKY_STATE final
{
    bool_t isActive = false;
    std::string strCueId;
    std::string strRedCloudAssetId;
    std::string strBlackApertureAssetId;
    f32_t fCloudOpacity = 0.f;
    f32_t fApertureScale = 0.f;
    f32_t fCloudRotationDegrees = 0.f;
};

class CValtanCinematicCameraController final
{
public:
    bool_t Initialize(
        const CValtanCinematicCameraDocument* document,
        uint32_t fixedTickHz);
    bool_t Update(
        const VALTAN_CINEMATIC_CAMERA_INPUT& input,
        f32_t timeDelta,
        VALTAN_CINEMATIC_CAMERA_POSE& outPose);
    void Reset();

    bool_t Is_Active() const { return nullptr != m_pActiveCue; }
    f32_t Get_ElapsedSeconds() const { return m_fElapsedSeconds; }
    /* Resolved from the same input the camera consumes. A new Server tick seeks
       its authoritative age, while duplicate snapshots advance by a bounded
       local delta so rotation and fades remain smooth between fixed ticks. */
    VALTAN_CINEMATIC_SKY_STATE Resolve_SkyState(
        const VALTAN_CINEMATIC_CAMERA_INPUT& input,
        f32_t timeDelta);
    const VALTAN_CINEMATIC_SKY_STATE& Get_LastSkyState() const
    {
        return m_LastSkyState;
    }

private:
    bool_t Sample_ActiveCue(
        f32_t elapsedSeconds,
        VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
    void Apply_ImpactShake(
        f32_t elapsedSeconds,
        VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
    void Reset_SkyTimeline();

private:
    const CValtanCinematicCameraDocument* m_pDocument = nullptr;
    const VALTAN_CINEMATIC_CAMERA_CUE* m_pActiveCue = nullptr;
    uint32_t m_iFixedTickHz = 0u;
    uint64_t m_iNetEntityId = 0u;
    uint32_t m_iPatternSequence = 0u;
    uint32_t m_iStageIndex = 0u;
    uint32_t m_iActionStartTick = 0u;
    uint32_t m_iLastServerTick = 0u;
    f32_t m_fElapsedSeconds = 0.f;
    std::string m_strCueId;
    bool_t m_hasCueKey = false;
    bool_t m_isCueFinished = false;
    uint64_t m_iSkyNetEntityId = 0u;
    uint32_t m_iSkyPatternSequence = 0u;
    uint32_t m_iSkyStageIndex = 0u;
    uint32_t m_iSkyActionStartTick = 0u;
    uint32_t m_iSkyLastServerTick = 0u;
    f32_t m_fSkyElapsedSeconds = 0.f;
    std::string m_strSkyCueId;
    bool_t m_hasSkyCueKey = false;
    VALTAN_CINEMATIC_SKY_STATE m_LastSkyState;
};

NS_END
```

### Client/Private/ValtanCinematicCameraController.cpp 전체 코드

```cpp
#include "ValtanCinematicCameraController.h"

#include "ActionPresentationTimeline.h"

#include <algorithm>
#include <cmath>

bool_t Client::CValtanCinematicCameraController::Initialize(
    const CValtanCinematicCameraDocument* document,
    const uint32_t fixedTickHz)
{
    Reset();
    if (nullptr == document || !document->Is_Ready() || 0u == fixedTickHz)
        return false;
    m_pDocument = document;
    m_iFixedTickHz = fixedTickHz;
    return true;
}

Client::VALTAN_CINEMATIC_SKY_STATE
Client::CValtanCinematicCameraController::Resolve_SkyState(
    const VALTAN_CINEMATIC_CAMERA_INPUT& input,
    const f32_t timeDelta)
{
    VALTAN_CINEMATIC_SKY_STATE state;
    m_LastSkyState = state;
    if (nullptr == m_pDocument || !input.isValid || 0u == m_iFixedTickHz ||
        0u == input.iNetEntityId || 0u == input.iServerTick ||
        0u == input.iPatternSequence || 0u == input.iActionStartTick ||
        !std::isfinite(timeDelta) || timeDelta < 0.f)
    {
        Reset_SkyTimeline();
        return state;
    }
    const VALTAN_CINEMATIC_SKY_CUE* cue = m_pDocument->Find_SkyCue(
        input.strPatternId, input.iStageIndex, input.strStageActionId);
    if (nullptr == cue || (!input.strStageId.empty() &&
        cue->strStageId != input.strStageId))
    {
        Reset_SkyTimeline();
        return state;
    }

    const bool_t cueChanged = !m_hasSkyCueKey ||
        m_iSkyNetEntityId != input.iNetEntityId ||
        m_iSkyPatternSequence != input.iPatternSequence ||
        m_iSkyStageIndex != input.iStageIndex ||
        m_iSkyActionStartTick != input.iActionStartTick ||
        m_strSkyCueId != cue->strCueId;
    const bool_t serverAdvanced = cueChanged ||
        CActionPresentationTimeline::Is_ForwardTick(
            input.iServerTick, m_iSkyLastServerTick);
    if (serverAdvanced)
    {
        f32_t authoritativeAge = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            input.iServerTick, input.iActionStartTick,
            static_cast<f32_t>(m_iFixedTickHz),
            authoritativeAge))
        {
            Reset_SkyTimeline();
            return state;
        }
        m_fSkyElapsedSeconds = authoritativeAge;
        m_iSkyLastServerTick = input.iServerTick;
    }
    else if (input.iServerTick == m_iSkyLastServerTick)
    {
        m_fSkyElapsedSeconds += (std::min)(timeDelta, 0.1f);
    }

    m_iSkyNetEntityId = input.iNetEntityId;
    m_iSkyPatternSequence = input.iPatternSequence;
    m_iSkyStageIndex = input.iStageIndex;
    m_iSkyActionStartTick = input.iActionStartTick;
    m_strSkyCueId = cue->strCueId;
    m_hasSkyCueKey = true;

    const f32_t startSeconds =
        static_cast<f32_t>(cue->iStageLocalStartMs) * 0.001f;
    const f32_t endSeconds = static_cast<f32_t>(cue->iStageLocalEndMs) * 0.001f;
    if (m_fSkyElapsedSeconds < startSeconds)
    {
        return state;
    }
    const f32_t span = endSeconds - startSeconds;
    const f32_t ratio = span <= 0.f ? 1.f :
        std::clamp((m_fSkyElapsedSeconds - startSeconds) / span, 0.f, 1.f);
    state.isActive = true;
    state.strCueId = cue->strCueId;
    state.strRedCloudAssetId = cue->strRedCloudAssetId;
    state.strBlackApertureAssetId = cue->strBlackApertureAssetId;
    state.fCloudOpacity = cue->fCloudOpacityStart +
        (cue->fCloudOpacityEnd - cue->fCloudOpacityStart) * ratio;
    state.fApertureScale = cue->fApertureScaleStart +
        (cue->fApertureScaleEnd - cue->fApertureScaleStart) * ratio;
    f32_t rotationPhaseOffset = 0.f;
    for (const VALTAN_CINEMATIC_SKY_CUE& prior :
        m_pDocument->Get_SkyCues())
    {
        if (prior.strPatternId != cue->strPatternId ||
            prior.iStageIndex >= cue->iStageIndex)
        {
            continue;
        }
        rotationPhaseOffset += prior.fCloudRotationDegreesPerSecond *
            static_cast<f32_t>(
                prior.iStageLocalEndMs - prior.iStageLocalStartMs) * 0.001f;
    }
    state.fCloudRotationDegrees =
        rotationPhaseOffset + cue->fCloudRotationDegreesPerSecond *
        (m_fSkyElapsedSeconds - startSeconds);
    m_LastSkyState = state;
    return state;
}

bool_t Client::CValtanCinematicCameraController::Update(
    const VALTAN_CINEMATIC_CAMERA_INPUT& input,
    const f32_t timeDelta,
    VALTAN_CINEMATIC_CAMERA_POSE& outPose)
{
    /* A dead boss carries no pattern sequence, so the clear shot is the one cue
       that does not require one. Everything else still does. */
    if (nullptr == m_pDocument || !input.isValid || 0u == input.iNetEntityId ||
        0u == input.iServerTick ||
        (0u == input.iPatternSequence && !input.isBossDead) ||
        0u == input.iActionStartTick || !std::isfinite(timeDelta) ||
        timeDelta < 0.f)
    {
        m_pActiveCue = nullptr;
        m_strCueId.clear();
        m_hasCueKey = false;
        m_isCueFinished = false;
        return false;
    }

    const VALTAN_CINEMATIC_CAMERA_CUE* cue = input.isBossDead ?
        m_pDocument->Find_DeathCue() :
        m_pDocument->Find_Cue(
            input.strPatternId, input.iStageIndex, input.strStageActionId);
    if (nullptr == cue || (!input.isBossDead && !input.strStageId.empty() &&
        cue->strStageId != input.strStageId))
    {
        m_pActiveCue = nullptr;
        m_strCueId.clear();
        m_hasCueKey = false;
        m_isCueFinished = false;
        return false;
    }

    const bool_t cueChanged = !m_hasCueKey ||
        m_iNetEntityId != input.iNetEntityId ||
        m_iPatternSequence != input.iPatternSequence ||
        m_iStageIndex != input.iStageIndex ||
        m_iActionStartTick != input.iActionStartTick ||
        m_strCueId != cue->strCueId;
    if (!cueChanged && m_isCueFinished)
        return false;

    const bool_t serverAdvanced = cueChanged ||
        Client::CActionPresentationTimeline::Is_ForwardTick(
            input.iServerTick, m_iLastServerTick);
    if (serverAdvanced)
    {
        f32_t authoritativeAge = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            input.iServerTick, input.iActionStartTick,
            static_cast<f32_t>(m_iFixedTickHz), authoritativeAge))
        {
            m_pActiveCue = nullptr;
            m_strCueId.clear();
            m_hasCueKey = false;
            m_isCueFinished = false;
            return false;
        }
        m_fElapsedSeconds = authoritativeAge;
        m_iLastServerTick = input.iServerTick;
    }
    else
    {
        m_fElapsedSeconds += (std::min)(timeDelta, 0.1f);
    }

    m_pActiveCue = cue;
    m_iNetEntityId = input.iNetEntityId;
    m_iPatternSequence = input.iPatternSequence;
    m_iStageIndex = input.iStageIndex;
    m_iActionStartTick = input.iActionStartTick;
    m_strCueId = cue->strCueId;
    m_hasCueKey = true;
    m_isCueFinished = false;
    if (m_fElapsedSeconds * 1000.f >= static_cast<f32_t>(cue->iDurationMs))
    {
        m_pActiveCue = nullptr;
        m_isCueFinished = true;
        return false;
    }
    return Sample_ActiveCue(m_fElapsedSeconds, outPose);
}

void Client::CValtanCinematicCameraController::Reset()
{
    m_pDocument = nullptr;
    m_pActiveCue = nullptr;
    m_iFixedTickHz = 0u;
    m_iNetEntityId = 0u;
    m_iPatternSequence = 0u;
    m_iStageIndex = 0u;
    m_iActionStartTick = 0u;
    m_iLastServerTick = 0u;
    m_fElapsedSeconds = 0.f;
    m_strCueId.clear();
    m_hasCueKey = false;
    m_isCueFinished = false;
    Reset_SkyTimeline();
}

void Client::CValtanCinematicCameraController::Reset_SkyTimeline()
{
    m_iSkyNetEntityId = 0u;
    m_iSkyPatternSequence = 0u;
    m_iSkyStageIndex = 0u;
    m_iSkyActionStartTick = 0u;
    m_iSkyLastServerTick = 0u;
    m_fSkyElapsedSeconds = 0.f;
    m_strSkyCueId.clear();
    m_hasSkyCueKey = false;
    m_LastSkyState = VALTAN_CINEMATIC_SKY_STATE{};
}

bool_t Client::CValtanCinematicCameraController::Sample_ActiveCue(
    const f32_t elapsedSeconds,
    VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
    if (nullptr == m_pActiveCue || m_pActiveCue->Keyframes.size() < 2u)
        return false;
    const f32_t elapsedMs = elapsedSeconds * 1000.f;
    const auto upper = std::upper_bound(
        m_pActiveCue->Keyframes.begin(), m_pActiveCue->Keyframes.end(), elapsedMs,
        [](const f32_t value, const VALTAN_CINEMATIC_CAMERA_KEYFRAME& frame)
        { return value < static_cast<f32_t>(frame.iTimeMs); });
    if (m_pActiveCue->Keyframes.begin() == upper)
    {
        const auto& first = m_pActiveCue->Keyframes.front();
        outPose = { first.vEye, first.vLookAt, first.fFovYDegrees };
        return true;
    }
    const auto& left = *(upper - 1);
    if (m_pActiveCue->Keyframes.end() == upper)
    {
        outPose = { left.vEye, left.vLookAt, left.fFovYDegrees };
        return true;
    }
    const auto& right = *upper;
    const f32_t span = static_cast<f32_t>(right.iTimeMs - left.iTimeMs);
    const f32_t rawAlpha = span <= 0.f ? 0.f : (std::clamp)(
        (elapsedMs - static_cast<f32_t>(left.iTimeMs)) / span, 0.f, 1.f);
    f32_t alpha = rawAlpha;
    switch (m_pActiveCue->eEasing)
    {
    case VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP:
        alpha = rawAlpha * rawAlpha * (3.f - 2.f * rawAlpha);
        break;
    case VALTAN_CINEMATIC_CAMERA_EASING::HOLD:
        alpha = 0.f;
        break;
    default:
        break;
    }
    XMStoreFloat3(&outPose.vEye, XMVectorLerp(
        XMLoadFloat3(&left.vEye), XMLoadFloat3(&right.vEye), alpha));
    XMStoreFloat3(&outPose.vLookAt, XMVectorLerp(
        XMLoadFloat3(&left.vLookAt), XMLoadFloat3(&right.vLookAt), alpha));
    outPose.fFovYDegrees = left.fFovYDegrees +
        (right.fFovYDegrees - left.fFovYDegrees) * alpha;
    Apply_ImpactShake(elapsedSeconds, outPose);
    return true;
}

void Client::CValtanCinematicCameraController::Apply_ImpactShake(
    const f32_t elapsedSeconds,
    VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
    if (nullptr == m_pActiveCue || m_pActiveCue->fShakeAmplitude <= 0.f ||
        0u == m_pActiveCue->iShakeDurationMs || elapsedSeconds < 0.f)
    {
        return;
    }
    const f32_t duration =
        static_cast<f32_t>(m_pActiveCue->iShakeDurationMs) * 0.001f;
    if (elapsedSeconds >= duration)
        return;
    /* A fixed frequency against the cue clock, decaying linearly to zero. No
       RNG and no frame-time dependence, so every client shakes identically and
       the jolt cannot outlive the stage. */
    constexpr f32_t SHAKE_RADIANS_PER_SECOND = 84.f;
    const f32_t decay = 1.f - elapsedSeconds / duration;
    const f32_t phase = elapsedSeconds * SHAKE_RADIANS_PER_SECOND;
    const f32_t amplitude = m_pActiveCue->fShakeAmplitude * decay * decay;
    outPose.vEye.x += std::sin(phase) * amplitude;
    outPose.vEye.y += std::sin(phase * 1.7f) * amplitude * 0.6f;
    outPose.vEye.z += std::cos(phase * 1.3f) * amplitude;
}
```

### Client/Public/MapAssetObject.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final : public CGameObject
{
public:
    struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        uint32_t prototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
        uint64_t placementId = {};
        std::string assetId;
        std::wstring modelPrototypeTag;
        float3_t position = {};
        float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
        float3_t signedScale = float3_t(1.f, 1.f, 1.f);
        bool_t applyBottomCenter = false;
        bool_t visible = true;
        MAP_ASSET_RENDER_PROFILE renderProfile;
    };

private:
    CMapAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
    virtual ~CMapAssetObject();

    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual void Update(f32_t fTimeDelta) override;
    virtual void Late_Update(f32_t fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT Render_Shadow() override;

    uint64_t Get_PlacementId() const { return m_iPlacementId; }
    const std::string& Get_AssetId() const { return m_AssetId; }
    const float3_t& Get_Position() const { return m_vPlacementPosition; }
    const float4_t& Get_RotationQuaternion() const { return m_vRotationQuaternion; }
    const float3_t& Get_SignedScale() const { return m_vSignedScale; }
    bool_t Is_Visible() const { return m_bVisible; }
    bool_t Is_Mirrored() const { return m_bMirrored; }
    void Set_PlacementTransform(const float3_t& position,
        const float4_t& rotationQuaternion, const float3_t& signedScale);
    void Set_Visible(bool_t visible) { m_bVisible = visible; }
    void Set_PresentationOpacityMultiplier(f32_t multiplier);

private:
    uint64_t m_iPlacementId = {};
    std::string m_AssetId;
    float3_t m_vPlacementPosition = {};
    float4_t m_vRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
    float3_t m_vSignedScale = float3_t(1.f, 1.f, 1.f);

    bool_t m_bApplyBottomCenter = false;
    bool_t m_bVisible = true;
    bool_t m_bMirrored = false;
    //Frustum Culling을 위한 멤버 변수 추가
    bool_t m_bHasLocalCullBounds = false;
    bool_t m_bHasWorldCullBounds = false;
    float3_t m_vLocalCullCenter = {};
    f32_t m_fLocalCullRadius = {};
    float3_t m_vWorldCullCenter = {};
    f32_t m_fWorldCullRadius = {};

    MAP_ASSET_RENDER_PROFILE m_RenderProfile;
    /* Runtime presentation may fade a placement without mutating the authored
       catalog profile shared by every occurrence of the asset. */
    f32_t m_fPresentationOpacityMultiplier = 1.f;
    f32_t m_fElapsedTime = {};

    shared_ptr<CShader> m_pShaderCom = { nullptr };
    shared_ptr<CModel> m_pModelCom = { nullptr };

private:
    HRESULT Ready_Components(uint32_t prototypeLevelIndex,
        const std::wstring& modelPrototypeTag);
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShadowShaderResources();
    //Frustum Culling
    void Ready_CullBounds();
    void Update_WorldCullBounds();

    float3_t Compute_WorldOrigin(const float3_t& placementPosition,
        const float4_t& rotationQuaternion, const float3_t& signedScale) const;
    uint32_t Select_ShaderPass() const;

public:
    static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
    virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/MapAssetObject.cpp 전체 코드

```cpp
#include "MapAssetObject.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Profiler.h"

#include "MapAssetRenderUtils.h"

#include <algorithm>
#include <cmath>

CMapAssetObject::CMapAssetObject(ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
    : CGameObject { pDevice, pContext }
{
}

CMapAssetObject::~CMapAssetObject()
{
}

HRESULT CMapAssetObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapAssetObject::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    //매개 변수로 받은 Arg를 MAP_ASSET_DESC*로 캐스팅해서 초기화 데이터 채워넣기
    const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);

    const vector_t quaternion = XMLoadFloat4(&desc.rotationQuaternion);
    const float quaternionLength = XMVectorGetX(XMVector4Length(quaternion));

    if (0 == desc.placementId || desc.assetId.empty() ||
        desc.modelPrototypeTag.empty() ||
        !std::isfinite(quaternionLength) || quaternionLength < 0.000001f ||
        std::abs(desc.signedScale.x) < 0.000001f ||
        std::abs(desc.signedScale.y) < 0.000001f ||
        std::abs(desc.signedScale.z) < 0.000001f)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)) ||
        FAILED(Ready_Components(
            desc.prototypeLevelIndex, desc.modelPrototypeTag)))
        return E_FAIL;

    m_iPlacementId = desc.placementId;
    m_AssetId = desc.assetId;
    m_bApplyBottomCenter = desc.applyBottomCenter;
    m_bVisible = desc.visible;
    m_RenderProfile = desc.renderProfile;
    m_fPresentationOpacityMultiplier = 1.f;

    /*CModel이 로드하며 만든 local AABB를 한 번만 bounding sphere로 변환한다.*/
    Ready_CullBounds();

    Set_PlacementTransform(
        desc.position, desc.rotationQuaternion, desc.signedScale);
    return S_OK;
}

void CMapAssetObject::Update(f32_t fTimeDelta)
{
    m_fElapsedTime += fTimeDelta;
    if (m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::BACKGROUND)
    {
        const float4_t* cameraPosition = CGameInstance::Get().Get_CamPosition();
        if (nullptr != cameraPosition)
            m_pTransformCom->Set_State(
                STATE::POSITION, XMLoadFloat4(cameraPosition));
    }
}

void CMapAssetObject::Late_Update(f32_t fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    Engine::CProfiler* pProfiler =
        CGameInstance::Get().Get_Profiler();

    //Visible 여부와 Frustum 통과 여부와 무관한 전체 placement 개수
    //MapAssetObject가 Update를 돌면서 Profiler의 Counter 증가 시키기 O(N)
    if (nullptr != pProfiler)
    {
        pProfiler->Add_Counter(
            Engine::EProfilerCounter::MapPlacements);

        pProfiler->Add_Counter(
            Engine::EProfilerCounter::MapFallbackObjects);
    }

    if (!m_bVisible)
        return;
    if (MAP_ASSET_RENDER_MODE::DEFERRED ==
        m_RenderProfile.renderMode &&
        CGameInstance::Get().Is_ShadowLightEnabled())
    {
        /* Shadow visibility follows authored placement visibility, not the
        camera frustum, so an off-screen caster cannot pop its shadow. */
        CGameInstance::Get().Add_RenderObject(
            RENDERGROUP::SHADOW,
            static_pointer_cast<CGameObject>(shared_from_this()));
    }
    //해당 Asset이 Background인지 여부 체크, sky background는 카메라를 따라 움직이기 때문에,
    //Frustum Culling에서 제외
    const bool_t bBackground =
        m_RenderProfile.renderMode ==
        MAP_ASSET_RENDER_MODE::BACKGROUND;

    //Sky/BackGround는 카메라를 따라 움직이므로 일반적인 world-space Frustum 대상에서 제외
    //bounds가 없는 모델은 fail-open으로 렌더
    if (!bBackground && m_bHasWorldCullBounds &&
        !CGameInstance::Get().isIn_Frustum_InWorldSpace(
            XMLoadFloat3(&m_vWorldCullCenter), m_fWorldCullRadius))
        return;

    //Profiler에 Visible Instance 추가
    if (nullptr != pProfiler)
    {
        pProfiler->Add_Counter(
            Engine::EProfilerCounter::MapVisibleInstances);
    }

    RENDERGROUP renderGroup = RENDERGROUP::NONBLEND;

    if (m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::TRANSLUCENT ||
        m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::ADDITIVE)
        renderGroup = RENDERGROUP::BLEND;
    else if (m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::BACKGROUND)
        renderGroup = RENDERGROUP::PRIORITY;

    CGameInstance::Get().Add_RenderObject(
        renderGroup,
        static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapAssetObject::Render()
{
    /* Late_Update may already have queued this object when a presentation cue
       hides it. Re-check at draw time so the previous frame cannot leak through. */
    if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f)
        return S_OK;
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const uint32_t passIndex =
        Select_ShaderPass();
    MAP_ASSET_RENDER_PROFILE presentationProfile = m_RenderProfile;
    presentationProfile.opacity *= m_fPresentationOpacityMultiplier;

    for (uint32_t meshIndex = 0; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
    {
        if (FAILED(
            CMapAssetRenderUtils::Bind_Material(
                m_pModelCom, m_pShaderCom, meshIndex,
                presentationProfile, m_fElapsedTime)) ||

            FAILED(m_pShaderCom->Begin(passIndex)) ||

            FAILED(m_pModelCom->Render(meshIndex)))
        {
            return E_FAIL;
        }
    }
    return S_OK;
}

HRESULT CMapAssetObject::Render_Shadow()
{
    constexpr uint32_t STATIC_SHADOW_PASS_BASE = 12u;
    if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f)
        return S_OK;
    if (MAP_ASSET_RENDER_MODE::DEFERRED != m_RenderProfile.renderMode)
        return S_OK;
    if (FAILED(Bind_ShadowShaderResources()))
        return E_FAIL;

    const uint32_t iCullPass = Select_ShaderPass();
    if (iCullPass > 2u)
        return E_UNEXPECTED;
    MAP_ASSET_RENDER_PROFILE presentationProfile = m_RenderProfile;
    presentationProfile.opacity *= m_fPresentationOpacityMultiplier;

    for (uint32_t iMesh = 0;
        iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
    {
        if (FAILED(CMapAssetRenderUtils::Bind_Material(
                m_pModelCom, m_pShaderCom, iMesh,
                presentationProfile, m_fElapsedTime)) ||
            FAILED(m_pShaderCom->Begin(
                STATIC_SHADOW_PASS_BASE + iCullPass)) ||
            FAILED(m_pModelCom->Render(iMesh)))
        {
            return E_FAIL;
        }
    }
    return S_OK;
}

void CMapAssetObject::Set_PresentationOpacityMultiplier(
    const f32_t multiplier)
{
    /* A corrupt presentation value must fail closed instead of reaching the
       shader as NaN. Valid callers may only attenuate authored opacity. */
    m_fPresentationOpacityMultiplier = std::isfinite(multiplier) ?
        (std::clamp)(multiplier, 0.f, 1.f) : 0.f;
}

uint32_t CMapAssetObject::Select_ShaderPass() const
{
    return CMapAssetRenderUtils::Select_Pass(
        m_RenderProfile, m_bMirrored);
}

void CMapAssetObject::Set_PlacementTransform(const float3_t& position,
    const float4_t& rotationQuaternion, const float3_t& signedScale)
{
    vector_t quaternion = XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion));

    if (XMVectorGetW(quaternion) < 0.f)
        quaternion = XMVectorNegate(quaternion);

    XMStoreFloat4(&m_vRotationQuaternion, quaternion);

    m_vPlacementPosition = position;
    m_vSignedScale = signedScale;
    m_bMirrored = signedScale.x * signedScale.y * signedScale.z < 0.f;

    const float3_t worldOrigin = Compute_WorldOrigin(
        position, m_vRotationQuaternion, signedScale);

    const matrix_t world = XMMatrixScaling(
        signedScale.x, signedScale.y, signedScale.z) *
        XMMatrixRotationQuaternion(quaternion);

    m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
    m_pTransformCom->Set_State(STATE::UP, world.r[1]);
    m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
    m_pTransformCom->Set_State(
        STATE::POSITION, XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));

    //bottom-center 보정을 포함한 최종 world 행렬을 사용해야 실제 렌더 위치와 bounds가 일치한다.
    Update_WorldCullBounds();
}

HRESULT CMapAssetObject::Ready_Components(
    uint32_t prototypeLevelIndex,
    const std::wstring& modelPrototypeTag)
{
    if (FAILED(__super::Add_Component(
        prototypeLevelIndex,
        TEXT("Prototype_Component_Shader_VtxMeshBinary"),
        TEXT("Com_Shader"), m_pShaderCom)) ||
        FAILED(__super::Add_Component(
            prototypeLevelIndex, modelPrototypeTag,
            TEXT("Com_Model"), m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapAssetObject::Bind_ShaderResources()
{
    matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
    world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
    const matrix_t inverseTranspose =
        XMMatrixTranspose(XMMatrixInverse(nullptr, world));
    float4x4_t storedInverseTranspose{};
    XMStoreFloat4x4(&storedInverseTranspose, inverseTranspose);
    if (FAILED(m_pTransformCom->Bind_ShaderResource(
        m_pShaderCom, "g_WorldMatrix")) ||
        FAILED(m_pShaderCom->Bind_Matrix(
            "g_WorldInvTransposeMatrix", &storedInverseTranspose)) ||
        FAILED(CGameInstance::Get().Bind_Transform(
            m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_Transform(
            m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapAssetObject::Bind_ShadowShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(
        m_pShaderCom, "g_WorldMatrix")) ||
        FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
            m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
            m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
    {
        return E_FAIL;
    }
    return S_OK;
}

void CMapAssetObject::Ready_CullBounds()
{
    m_bHasLocalCullBounds = false;
    m_bHasWorldCullBounds = false;
    m_vLocalCullCenter = {};
    m_fLocalCullRadius = 0.f;
    m_vWorldCullCenter = {};
    m_fWorldCullRadius = 0.f;

    if (nullptr == m_pModelCom ||
        !m_pModelCom->Has_LocalBounds())
        return;

    const float3_t& minimum =
        m_pModelCom->Get_LocalBoundsMin();
    const float3_t& maximum =
        m_pModelCom->Get_LocalBoundsMax();
    //예외처리 <- 근데 무슨 경우?
    if (!std::isfinite(minimum.x) ||
        !std::isfinite(minimum.y) ||
        !std::isfinite(minimum.z) ||
        !std::isfinite(maximum.x) ||
        !std::isfinite(maximum.y) ||
        !std::isfinite(maximum.z) ||
        maximum.x < minimum.x ||
        maximum.y < minimum.y ||
        maximum.z < minimum.z)
    {
        return;
    }

    m_vLocalCullCenter = float3_t(
        (minimum.x + maximum.x) * 0.5f,
        (minimum.y + maximum.y) * 0.5f,
        (minimum.z + maximum.z) * 0.5f);

    const f32_t extentX =
        (maximum.x - minimum.x) * 0.5f;
    const f32_t extentY =
        (maximum.y - minimum.y) * 0.5f;
    const f32_t extentZ =
        (maximum.z - minimum.z) * 0.5f;

    const f32_t radius = std::sqrt(
        extentX * extentX +
        extentY * extentY +
        extentZ * extentZ);

    if (!std::isfinite(radius))
        return;

    //점이나 얇은 mesh가 쉽게 잘리지 않도록 최소 반지름
    m_fLocalCullRadius =
        radius > 0.05f ? radius : 0.05f;

    m_bHasLocalCullBounds = true;
}

void CMapAssetObject::Update_WorldCullBounds()
{
    m_bHasWorldCullBounds = false;

    if (!m_bHasLocalCullBounds ||
        nullptr == m_pTransformCom)
        return;

    const matrix_t world =
        XMLoadFloat4x4(
            m_pTransformCom->Get_WorldMatrixPtr());

    const vector_t worldCenter =
        XMVector3TransformCoord(
            XMLoadFloat3(&m_vLocalCullCenter),
            world);

    float3_t storedWorldCenter{};
    XMStoreFloat3(
        &storedWorldCenter,
        worldCenter);
    //vector의 길이 가지고 오기. row 0 1 2의 x y z의 길이 구하기
    const f32_t scaleX =
        XMVectorGetX(
            XMVector3Length(world.r[0]));
    const f32_t scaleY =
        XMVectorGetX(
            XMVector3Length(world.r[1]));
    const f32_t scaleZ =
        XMVectorGetX(
            XMVector3Length(world.r[2]));
    //가장 큰 스케일 기준으로 컬링
    f32_t maximumScale = scaleX;
    if (scaleY > maximumScale)
        maximumScale = scaleY;
    if (scaleZ > maximumScale)
        maximumScale = scaleZ;
    //worldRadius scale 보정
    const f32_t worldRadius =
        m_fLocalCullRadius *
        maximumScale *
        1.02f + 0.05f;

    if (!std::isfinite(storedWorldCenter.x) ||
        !std::isfinite(storedWorldCenter.y) ||
        !std::isfinite(storedWorldCenter.z) ||
        !std::isfinite(worldRadius) ||
        maximumScale < 0.000001f)
    {
        return;
    }

    m_vWorldCullCenter = storedWorldCenter;
    m_fWorldCullRadius = worldRadius;
    m_bHasWorldCullBounds = true;
}

float3_t CMapAssetObject::Compute_WorldOrigin(const float3_t& placementPosition,
    const float4_t& rotationQuaternion, const float3_t& signedScale) const
{
    float3_t worldOrigin = placementPosition;
    if (m_bApplyBottomCenter && m_pModelCom->Has_LocalBounds())
    {
        const float3_t& minimum = m_pModelCom->Get_LocalBoundsMin();
        const float3_t& maximum = m_pModelCom->Get_LocalBoundsMax();
        const vector_t localAnchor = XMVectorSet(
            (minimum.x + maximum.x) * 0.5f,
            minimum.y,
            (minimum.z + maximum.z) * 0.5f,
            1.f);
        const matrix_t transform = XMMatrixScaling(
            signedScale.x, signedScale.y, signedScale.z) *
            XMMatrixRotationQuaternion(
                XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion)));
        float3_t anchorOffset{};
        XMStoreFloat3(&anchorOffset,
            XMVector3TransformCoord(localAnchor, transform));
        worldOrigin.x -= anchorOffset.x;
        worldOrigin.y -= anchorOffset.y;
        worldOrigin.z -= anchorOffset.z;
    }

    return worldOrigin;
}

unique_ptr<CMapAssetObject> CMapAssetObject::Create(
    ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance = unique_ptr<CMapAssetObject>(
        new CMapAssetObject(pDevice, pContext));
    if (FAILED(pInstance->Initialize_Prototype()))
        return nullptr;
    return pInstance;
}

shared_ptr<CPrototype> CMapAssetObject::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CMapAssetObject>(new CMapAssetObject(*this));
    if (FAILED(pInstance->Initialize(pArg)))
        return nullptr;
    return pInstance;
}
```

### Client/Public/Level_ValtanArena.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "EncounterPatternReference.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "ValtanCinematicCameraController.h"
#include "ValtanCinematicCameraDocument.h"
#include "WorldDestructionDebrisPresentationDocument.h"
#include "WorldDestructionDebrisPresentationRuntime.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldPlayerNameplateView.h"

#include <array>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;
class CMapAssetObject;

class CLevel_ValtanArena final : public CLevel
{
private:
    CLevel_ValtanArena(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

public:
    virtual ~CLevel_ValtanArena();

    virtual HRESULT Initialize() override;
    virtual void Update(f32_t fTimeDelta) override;
    virtual HRESULT Render() override;

private:
    HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
    bool_t Ready_CinematicCamera();
    bool_t Ready_ValtanSkyPresentation(std::string& outStatus);
    bool_t Bind_CameraToLocalCharacter();
    void Update_CinematicCamera(f32_t fTimeDelta);
    void Apply_ValtanSkyPresentation(
        const VALTAN_CINEMATIC_SKY_STATE& state);
    void Reset_ValtanSkyPresentation();
    void Clear_ValtanSkyPresentation();
    void End_CinematicCameraOverride();
    void End_CinematicCamera();
    void Update_WorldDestructionPresentation(f32_t fTimeDelta);
    bool_t Apply_EncounterPropPresentation();
#ifdef _DEBUG
    /* Debug audition of an authored health-bar pattern. The panel only submits
    typed requests and reports what the Server answered; it never starts a
    pattern, moves the camera or breaks a wall on its own. */
    void Render_AuditionPanel();
    void Update_AuditionTransaction();
    bool_t Submit_Audition(
        LostArk::Shared::VALTAN_AUDITION_OPERATION operation);
    void Request_OrderedAuditionStop(bool_t restartAfterStop);
    struct AUDITION_PENDING_REQUEST final
    {
        uint32_t iSequence = 0u;
        LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
            LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
        uint32_t iTargetHealthBar = 0u;
        uint64_t iLastSentAtMilliseconds = 0u;
        uint32_t iRetryCount = 0u;

        [[nodiscard]] bool_t Is_Active() const
        {
            return 0u != iSequence;
        }
    };
    /* One ordered chapter run over the existing audition operations. It resets
    once at the entrance and then only crosses the authored bars, so the
    environment stays cumulative the way the recording shows it. */
    struct ENVIRONMENT_TIMELINE_STEP final
    {
        LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
            LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
        uint32_t iTargetHealthBar = 0u;
        bool_t waitForPattern = false;
    };
    void Start_EnvironmentTimeline();
    void Advance_EnvironmentTimeline(bool_t isBossPatternRunning);
#endif

private:
    CMapPlacementRuntime m_MapRuntime;
    CDeployPropRuntime m_DeployRuntime;
    shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
    bool_t m_bMapLightSubmissionFailureReported = false;
    shared_ptr<CCamera_Free> m_pCamera = { nullptr };
    weak_ptr<CCharacter> m_pCameraTarget;
    weak_ptr<CTransform> m_pCinematicRestoreTarget;
    bool_t m_bCinematicRestoreFollowRequested = false;
    bool_t m_bCinematicCameraApplied = false;
    uint64_t m_iCinematicCameraOwnerId = 0u;
    CEncounterPatternReference m_ValtanEncounterReference;
    CValtanCinematicCameraDocument m_ValtanCinematicCameraDocument;
    CValtanCinematicCameraController m_ValtanCinematicCameraController;
    /* The Server-owned pattern clock only selects this presentation. The six
       cached map objects never participate in collision or navigation. */
    VALTAN_CINEMATIC_SKY_STATE m_ValtanSkyState;
    struct VALTAN_SKY_PRESENTATION_LAYER final
    {
        shared_ptr<CMapAssetObject> pObject;
        uint64_t iPlacementId = 0u;
        float3_t vBasePosition = {};
        float4_t vBaseRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
        float3_t vBaseSignedScale = float3_t(1.f, 1.f, 1.f);
        f32_t fRotationMultiplier = 1.f;
    };
    static constexpr size_t VALTAN_SKY_LAYER_COUNT = 3u;
    std::array<VALTAN_SKY_PRESENTATION_LAYER, VALTAN_SKY_LAYER_COUNT>
        m_ValtanRedCloudLayers{};
    std::array<VALTAN_SKY_PRESENTATION_LAYER, VALTAN_SKY_LAYER_COUNT>
        m_ValtanBlackApertureLayers{};
    std::string m_strValtanRedCloudSeedAssetId;
    std::string m_strValtanBlackApertureSeedAssetId;
    bool_t m_bValtanSkyPresentationApplied = false;
    CWorldDestructionProjectionDocument m_WorldDestructionProjectionDocument;
    CWorldDestructionDebrisPresentationDocument
        m_WorldDestructionDebrisPresentationDocument;
    CWorldDestructionDebrisPresentationRuntime
        m_WorldDestructionDebrisPresentationRuntime;
    uint64_t m_iObservedWorldDestructionPresentationGeneration = 0u;
    uint32_t m_iObservedEncounterPropEpoch = 0u;
    uint32_t m_iObservedEncounterPropServerTick = 0u;
    CClientReplication m_Replication;
    CWorldPlayerNameplateView m_PlayerNameplateView;
    std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
    shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
    CPlayerController m_PlayerController;
#ifdef _DEBUG
    size_t m_iSelectedAuditionBarIndex = 0u;
    uint32_t m_iNextAuditionRequestSequence = 1u;
    AUDITION_PENDING_REQUEST m_PendingAuditionRequest;
    bool_t m_bOrderedAuditionActive = false;
    bool_t m_bStopAuditionQueued = false;
    bool_t m_bRestartOrderedAfterStop = false;
    std::string m_strAuditionStatus;
    std::vector<ENVIRONMENT_TIMELINE_STEP> m_EnvironmentTimeline;
    size_t m_iEnvironmentTimelineStep = 0u;
    bool_t m_bEnvironmentTimelineWaiting = false;
    bool_t m_bEnvironmentTimelinePatternStarted = false;
#endif

public:
    static unique_ptr<CLevel_ValtanArena> Create(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### Client/Private/Level_ValtanArena.cpp 전체 코드

```cpp
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

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <limits>
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
        return;
    }
    if (!Apply_EncounterPropPresentation())
    {
        CLevelTransitionService::Report_LoadFailure(
            E_FAIL,
            "[Level_ValtanArena][EncounterPropSync] " +
                m_DeployRuntime.Get_Status());
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
    constexpr uint64_t AUDITION_RETRY_INTERVAL_MILLISECONDS = 750u;
    constexpr uint32_t AUDITION_MAX_RETRY_COUNT = 3u;

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
    const std::vector<uint32_t> bars =
        Collect_AuditionHealthBars(m_ValtanEncounterReference);
    if (!isBarless && m_iSelectedAuditionBarIndex >= bars.size())
    {
        m_strAuditionStatus = "No authored health-bar pattern is selected.";
        return false;
    }

    const uint32_t sequence = 0u == m_iNextAuditionRequestSequence ?
        1u : m_iNextAuditionRequestSequence;
    const uint32_t targetHealthBar =
        isBarless ? 0u : bars[m_iSelectedAuditionBarIndex];
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
        m_EnvironmentTimeline.clear();
        m_iEnvironmentTimelineStep = 0u;
        m_strAuditionStatus =
            "Full environment timeline finished. Pillars stay raised: the shatter has no product trigger yet.";
        return;
    }

    const ENVIRONMENT_TIMELINE_STEP& step =
        m_EnvironmentTimeline[m_iEnvironmentTimelineStep];
    if (0u != step.iTargetHealthBar)
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
    reacted to the 109 collapse by mistake. */
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
        "Floor Stage A (84): INTACT %zu | BREAKING %zu | GONE %zu   (expected 2)",
        floorStageAIntact,
        floorStageABreaking,
        floorStageAGone);
    ImGui::Text(
        "Floor Stage B (30): INTACT %zu | BREAKING %zu | GONE %zu   (expected 4)",
        floorStageBIntact,
        floorStageBBreaking,
        floorStageBGone);
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
        "The first Debug frame with a replicated local character starts this once automatically.");
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
        states.emplace_back(
            VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index], deployState);
    }
    if (!m_DeployRuntime.Set_States(states))
        return false;
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
            layer.iPlacementId = entry.record.placementId;
            layer.vBasePosition = entry.record.position;
            layer.vBaseRotationQuaternion = entry.record.rotationQuaternion;
            layer.vBaseSignedScale = entry.record.signedScale;
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

    constexpr std::array<f32_t, VALTAN_SKY_LAYER_COUNT>
        RED_ROTATION_MULTIPLIERS = { 1.f, -0.72f, 1.35f };
    constexpr std::array<f32_t, VALTAN_SKY_LAYER_COUNT>
        APERTURE_ROTATION_MULTIPLIERS = { -0.45f, 0.68f, -1.1f };
    for (size_t index = 0u; index < VALTAN_SKY_LAYER_COUNT; ++index)
    {
        stagedRedCloudLayers[index].fRotationMultiplier =
            RED_ROTATION_MULTIPLIERS[index];
        stagedBlackApertureLayers[index].fRotationMultiplier =
            APERTURE_ROTATION_MULTIPLIERS[index];
    }

    /* Resolve every dependency before changing the currently committed set. */
    Clear_ValtanSkyPresentation();
    m_ValtanRedCloudLayers = std::move(stagedRedCloudLayers);
    m_ValtanBlackApertureLayers = std::move(stagedBlackApertureLayers);
    m_strValtanRedCloudSeedAssetId = redCloudSeed;
    m_strValtanBlackApertureSeedAssetId = blackApertureSeed;
    m_bValtanSkyPresentationApplied = true;
    Reset_ValtanSkyPresentation();
    outStatus = "Cinematic sky presentation cached six map layers";
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
        (std::clamp)(state.fApertureScale, 0.f, 8.f);
    const f32_t apertureOpacity =
        (std::clamp)(apertureScale, 0.f, 1.f);
    constexpr f32_t VISIBLE_EPSILON = 0.001f;
    const bool_t showRedCloud = cloudOpacity > VISIBLE_EPSILON;
    const bool_t showBlackAperture =
        apertureScale > VISIBLE_EPSILON &&
        apertureOpacity > VISIBLE_EPSILON;

    auto applyGroup = [rotationDegrees = state.fCloudRotationDegrees](
        auto& layers,
        const bool_t visible,
        const f32_t opacity,
        const f32_t scaleMultiplier)
    {
        for (VALTAN_SKY_PRESENTATION_LAYER& layer : layers)
        {
            if (nullptr == layer.pObject)
                continue;
            if (!visible)
            {
                layer.pObject->Set_Visible(false);
                layer.pObject->Set_PresentationOpacityMultiplier(1.f);
                layer.pObject->Set_PlacementTransform(
                    layer.vBasePosition,
                    layer.vBaseRotationQuaternion,
                    layer.vBaseSignedScale);
                continue;
            }

            const vector_t baseQuaternion =
                XMLoadFloat4(&layer.vBaseRotationQuaternion);
            const vector_t yawQuaternion = XMQuaternionRotationRollPitchYaw(
                0.f,
                XMConvertToRadians(
                    rotationDegrees * layer.fRotationMultiplier),
                0.f);
            float4_t presentationQuaternion{};
            /* DirectXMath defines Multiply(Q1, Q2) as Q2*Q1: this applies
               the authored base first and the presentation world-Y spin second. */
            XMStoreFloat4(&presentationQuaternion,
                XMQuaternionMultiply(baseQuaternion, yawQuaternion));
            const float3_t presentationScale(
                layer.vBaseSignedScale.x * scaleMultiplier,
                layer.vBaseSignedScale.y * scaleMultiplier,
                layer.vBaseSignedScale.z * scaleMultiplier);
            layer.pObject->Set_PlacementTransform(
                layer.vBasePosition,
                presentationQuaternion,
                presentationScale);
            layer.pObject->Set_PresentationOpacityMultiplier(opacity);
            layer.pObject->Set_Visible(true);
        }
    };

    applyGroup(
        m_ValtanRedCloudLayers, showRedCloud, cloudOpacity, 1.f);
    applyGroup(
        m_ValtanBlackApertureLayers, showBlackAperture,
        apertureOpacity, showBlackAperture ? apertureScale : 1.f);
    m_bValtanSkyPresentationApplied = showRedCloud || showBlackAperture;
}

void CLevel_ValtanArena::Reset_ValtanSkyPresentation()
{
    if (!m_bValtanSkyPresentationApplied)
        return;
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
            layer.pObject->Set_PlacementTransform(
                layer.vBasePosition,
                layer.vBaseRotationQuaternion,
                layer.vBaseSignedScale);
        }
    };
    resetGroup(m_ValtanRedCloudLayers);
    resetGroup(m_ValtanBlackApertureLayers);
    m_bValtanSkyPresentationApplied = false;
}

void CLevel_ValtanArena::Clear_ValtanSkyPresentation()
{
    Reset_ValtanSkyPresentation();
    m_ValtanRedCloudLayers = {};
    m_ValtanBlackApertureLayers = {};
    m_strValtanRedCloudSeedAssetId.clear();
    m_strValtanBlackApertureSeedAssetId.clear();
    m_bValtanSkyPresentationApplied = false;
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
```

### Data/Encounters/Valtan/ValtanCinematicCamera.json 전체 코드

```json
{
    "schema":  "lostark.encounter-cinematic-camera",
    "formatVersion":  3,
    "encounterId":  "ENCOUNTER_VALTAN",
    "provenance":  "PROJECT_AUTHORED",
    "cues":  [
                 {
                     "cueId":  "camera.valtan.arena-break-109.takeoff",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "TAKEOFF",
                     "durationMs":  900,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       149.53,
                                                       26,
                                                       -116.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          25,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  42
                                       },
                                       {
                                           "timeMs":  900,
                                           "eye":  [
                                                       151.03,
                                                       32.5,
                                                       -117.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          78,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  38
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-109.drop",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "DROP",
                     "durationMs":  700,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       151.03,
                                                       32.5,
                                                       -117.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          78,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  38
                                       },
                                       {
                                           "timeMs":  430,
                                           "eye":  [
                                                       153.03,
                                                       39,
                                                       -108.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          110,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  46
                                       },
                                       {
                                           "timeMs":  700,
                                           "eye":  [
                                                       156.03,
                                                       57,
                                                       -92.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          92,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  58
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-109.impact",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "IMPACT",
                     "durationMs":  400,
                     "easing":  "LINEAR",
                     "shakeAmplitude":  0.45,
                     "shakeDurationMs":  220,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       156.03,
                                                       57,
                                                       -92.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          92,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  58
                                       },
                                       {
                                           "timeMs":  400,
                                           "eye":  [
                                                       156.03,
                                                       58.5,
                                                       -90.56
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  59
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-109.impact-hold",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "IMPACT_HOLD",
                     "durationMs":  1100,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       156.03,
                                                       58.5,
                                                       -90.56
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  59
                                       },
                                       {
                                           "timeMs":  1100,
                                           "eye":  [
                                                       156.03,
                                                       61,
                                                       -88.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  60
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-109.wide-reveal",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "WIDE_REVEAL",
                     "durationMs":  2300,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       156.03,
                                                       61,
                                                       -88.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  60
                                       },
                                       {
                                           "timeMs":  2300,
                                           "eye":  [
                                                       156.03,
                                                       65,
                                                       -82.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  62
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-109.recovery",
                     "patternId":  "VALTAN_ARENA_BREAK_109",
                     "stageId":  "RECOVERY",
                     "durationMs":  870,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       156.03,
                                                       65,
                                                       -82.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  62
                                       },
                                       {
                                           "timeMs":  870,
                                           "eye":  [
                                                       156.03,
                                                       67,
                                                       -80.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          23,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  62
                                       }
                                   ]
                 },
                 {
                     "cueId":  "camera.valtan.arena-break-33.cutscene",
                     "patternId":  "VALTAN_ARENA_BREAK_33",
                     "stageId":  "CUTSCENE",
                     "durationMs":  2500,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0.00,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       178.00,
                                                       43.00,
                                                       -91.00
                                                   ],
                                           "lookAt":  [
                                                          151.25,
                                                          24.50,
                                                          -121.75
                                                      ],
                                           "fovYDegrees":  51.0
                                       },
                                       {
                                           "timeMs":  1250,
                                           "eye":  [
                                                       171.00,
                                                       38.00,
                                                       -99.00
                                                   ],
                                           "lookAt":  [
                                                          151.25,
                                                          24.00,
                                                          -121.75
                                                      ],
                                           "fovYDegrees":  47.0
                                       },
                                       {
                                           "timeMs":  2500,
                                           "eye":  [
                                                       165.00,
                                                       33.00,
                                                       -106.00
                                                   ],
                                           "lookAt":  [
                                                          151.25,
                                                          23.50,
                                                          -121.75
                                                      ],
                                           "fovYDegrees":  43.0
                                       }
                                   ]
                 }
             ],
    "skyCues":  [
                    {
                        "cueId":  "sky.valtan.arena-break-109.takeoff",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "TAKEOFF",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  900,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  0,
                        "cloudOpacityEnd":  0.72,
                        "apertureScaleStart":  0.15,
                        "apertureScaleEnd":  0.55,
                        "cloudRotationDegreesPerSecond":  8
                    },
                    {
                        "cueId":  "sky.valtan.arena-break-109.drop",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "DROP",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  700,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  0.72,
                        "cloudOpacityEnd":  1,
                        "apertureScaleStart":  0.55,
                        "apertureScaleEnd":  1,
                        "cloudRotationDegreesPerSecond":  12
                    },
                    {
                        "cueId":  "sky.valtan.arena-break-109.impact",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "IMPACT",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  400,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  1,
                        "cloudOpacityEnd":  1,
                        "apertureScaleStart":  1,
                        "apertureScaleEnd":  1.1,
                        "cloudRotationDegreesPerSecond":  18
                    },
                    {
                        "cueId":  "sky.valtan.arena-break-109.impact-hold",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "IMPACT_HOLD",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  1100,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  1,
                        "cloudOpacityEnd":  1,
                        "apertureScaleStart":  1.1,
                        "apertureScaleEnd":  1,
                        "cloudRotationDegreesPerSecond":  14
                    },
                    {
                        "cueId":  "sky.valtan.arena-break-109.wide-reveal",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "WIDE_REVEAL",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  2300,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  1,
                        "cloudOpacityEnd":  0.82,
                        "apertureScaleStart":  1,
                        "apertureScaleEnd":  0.92,
                        "cloudRotationDegreesPerSecond":  10
                    },
                    {
                        "cueId":  "sky.valtan.arena-break-109.recovery",
                        "patternId":  "VALTAN_ARENA_BREAK_109",
                        "stageId":  "RECOVERY",
                        "stageLocalStartMs":  0,
                        "stageLocalEndMs":  870,
                        "redCloudAssetId":  "VALTAN_PHASE_CHAOS_CLOUD",
                        "blackApertureAssetId":  "VALTAN_PHASE_SPACEHOLE_CORE",
                        "cloudOpacityStart":  0.82,
                        "cloudOpacityEnd":  0,
                        "apertureScaleStart":  0.92,
                        "apertureScaleEnd":  0,
                        "cloudRotationDegreesPerSecond":  8
                    }
                ],
    "deathCue":  {
                     "cueId":  "camera.valtan.clear.wide",
                     "durationMs":  2090,
                     "easing":  "SMOOTHSTEP",
                     "shakeAmplitude":  0,
                     "shakeDurationMs":  0,
                     "keyframes":  [
                                       {
                                           "timeMs":  0,
                                           "eye":  [
                                                       134.03,
                                                       29.5,
                                                       -102.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          24,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  52
                                       },
                                       {
                                           "timeMs":  1090,
                                           "eye":  [
                                                       142.03,
                                                       34,
                                                       -107.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          24,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  55
                                       },
                                       {
                                           "timeMs":  2090,
                                           "eye":  [
                                                       156.03,
                                                       53,
                                                       -96.06
                                                   ],
                                           "lookAt":  [
                                                          156.03,
                                                          22.99751,
                                                          -122.06
                                                      ],
                                           "fovYDegrees":  58
                                       }
                                   ]
                 }
}
```

### Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp 교체 패치

기준점은 `Test_ValtanCinematicCamera()`의 109 camera/sky fixture다. 아래 patch는 obsolete version, stable asset ID, 연속 회전, bounded interpolation, stage-end hold, recovery reset 검증을 포함한다.

```diff
diff --git a/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp b/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
index 9fdc9e3b..95470722 100644
--- a/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
+++ b/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp
@@ -5384,7 +5384,7 @@ namespace
     }

     std::string Make_ValtanCameraFixture(
-        const uint32_t formatVersion = 2u,
+        const uint32_t formatVersion = 3u,
         const bool_t includeUnknownRootKey = false,
         const bool_t duplicateTuple = false)
     {
@@ -5422,6 +5422,31 @@ namespace
         return text.str();
     }

+    std::string Make_ValtanSkyFixture(
+        const std::string_view redCloudAssetId,
+        const std::string_view blackApertureAssetId)
+    {
+        std::string text = Make_ValtanCameraFixture();
+        const std::string emptyArray = "\"skyCues\":[]";
+        const size_t offset = text.find(emptyArray);
+        if (std::string::npos == offset)
+            return {};
+        std::ostringstream sky;
+        sky << "\"skyCues\":[{";
+        sky << "\"cueId\":\"fixture.sky.valtan.109\",";
+        sky << "\"patternId\":\"VALTAN_ARENA_BREAK_109\",";
+        sky << "\"stageId\":\"WIDE_REVEAL\",";
+        sky << "\"stageLocalStartMs\":0,\"stageLocalEndMs\":2000,";
+        sky << "\"redCloudAssetId\":\"" << redCloudAssetId << "\",";
+        sky << "\"blackApertureAssetId\":\"" <<
+            blackApertureAssetId << "\",";
+        sky << "\"cloudOpacityStart\":0.2,\"cloudOpacityEnd\":1,";
+        sky << "\"apertureScaleStart\":0.4,\"apertureScaleEnd\":1,";
+        sky << "\"cloudRotationDegreesPerSecond\":12}]";
+        text.replace(offset, emptyArray.size(), sky.str());
+        return text;
+    }
+
     std::string Make_ValtanCameraEasingFixture(
         const char_t* easing,
         const f32_t shakeAmplitude,
@@ -5429,7 +5454,7 @@ namespace
     {
         std::ostringstream text;
         text << "{\"schema\":\"lostark.encounter-cinematic-camera\",";
-        text << "\"formatVersion\":2,";
+        text << "\"formatVersion\":3,";
         text << "\"encounterId\":\"ENCOUNTER_VALTAN\",";
         text << "\"provenance\":\"PROJECT_AUTHORED\",";
         text << "\"cues\":[{\"cueId\":\"fixture.valtan.easing\",";
@@ -5462,14 +5487,42 @@ namespace
                     "Encounters/Valtan/ValtanCinematicCamera.json"),
                 encounter, error);
         runner.Require(
-            documentLoaded && !document.Get_SkyCues().empty(),
-            "Valtan Sky Cues Load From The Camera Document");
+            documentLoaded && 6u == document.Get_SkyCues().size(),
+            "Valtan 109 Sky Loads One Cue For Every Transition Stage");
+
+        const auto hasExactSkyCue = [&document](
+            const uint32_t stageIndex,
+            const char_t* stageActionId)
+        {
+            const VALTAN_CINEMATIC_SKY_CUE* cue = document.Find_SkyCue(
+                "VALTAN_ARENA_BREAK_109", stageIndex, stageActionId);
+            return nullptr != cue &&
+                cue->strRedCloudAssetId == "VALTAN_PHASE_CHAOS_CLOUD" &&
+                cue->strBlackApertureAssetId ==
+                    "VALTAN_PHASE_SPACEHOLE_CORE";
+        };
+        runner.Require(
+            documentLoaded &&
+            hasExactSkyCue(0u,
+                "valtan.mechanic.arena-break-109.takeoff") &&
+            hasExactSkyCue(1u,
+                "valtan.mechanic.arena-break-109.drop") &&
+            hasExactSkyCue(2u,
+                "valtan.mechanic.arena-break-109.impact") &&
+            hasExactSkyCue(3u,
+                "valtan.mechanic.arena-break-109.impact-hold") &&
+            hasExactSkyCue(4u,
+                "valtan.mechanic.arena-break-109.wide-reveal") &&
+            hasExactSkyCue(5u,
+                "valtan.mechanic.arena-break-109.recovery"),
+            "Valtan 109 Sky Resolves Stable Cloud And Aperture Seed IDs");

         CValtanCinematicCameraController controller;
         const bool_t initialized = documentLoaded &&
             controller.Initialize(&document, encounter.Get_FixedTickHz());

-        const auto sampleSky = [&](
+        const auto makeInput = [](
+            const char_t* stageId,
             const char_t* stageActionId,
             const uint32_t stageIndex,
             const f32_t seconds)
@@ -5477,49 +5530,162 @@ namespace
             VALTAN_CINEMATIC_CAMERA_INPUT input{};
             input.isValid = true;
             input.iNetEntityId = 91u;
-            input.strPatternId = "VALTAN_FOUR_PILLARS_105";
+            input.strPatternId = "VALTAN_ARENA_BREAK_109";
+            input.strStageId = stageId;
             input.strStageActionId = stageActionId;
-            input.iPatternSequence = 1u;
+            input.iPatternSequence = 7u;
             input.iStageIndex = stageIndex;
             input.iActionStartTick = 100u;
             input.iServerTick =
                 100u + static_cast<uint32_t>(seconds * 30.f + 0.5f);
-            return controller.Resolve_SkyState(input);
+            return input;
         };

-        /* Before its authored window the layer is simply not on. */
-        const VALTAN_CINEMATIC_SKY_STATE early = sampleSky(
-            "valtan.mechanic.four-pillars-105.takeoff", 0u, 0.1f);
-        const VALTAN_CINEMATIC_SKY_STATE rising = sampleSky(
-            "valtan.mechanic.four-pillars-105.takeoff", 0u, 1.5f);
+        VALTAN_CINEMATIC_CAMERA_INPUT takeoffInput = makeInput(
+            "TAKEOFF", "valtan.mechanic.arena-break-109.takeoff", 0u, 0.f);
+        const VALTAN_CINEMATIC_SKY_STATE takeoff =
+            controller.Resolve_SkyState(takeoffInput, 0.f);
         runner.Require(
-            initialized && !early.isActive && rising.isActive &&
-            rising.fCloudOpacity > 0.5f,
-            "Valtan Sky Cloud Rises Only Inside Its Authored Stage Window");
-
-        const VALTAN_CINEMATIC_SKY_STATE aperture = sampleSky(
-            "valtan.mechanic.four-pillars-105.yellow-zone", 1u, 0.85f);
+            initialized && takeoff.isActive &&
+            takeoff.strRedCloudAssetId == "VALTAN_PHASE_CHAOS_CLOUD" &&
+            takeoff.strBlackApertureAssetId ==
+                "VALTAN_PHASE_SPACEHOLE_CORE" &&
+            std::abs(takeoff.fCloudOpacity) < 0.000001f &&
+            takeoff.fApertureScale > 0.f,
+            "Valtan 109 Sky Begins With A Dark Aperture And Cloud Fade In");
+
+        VALTAN_CINEMATIC_CAMERA_INPUT takeoffEndInput = makeInput(
+            "TAKEOFF", "valtan.mechanic.arena-break-109.takeoff", 0u, 0.9f);
+        const VALTAN_CINEMATIC_SKY_STATE takeoffEnd =
+            controller.Resolve_SkyState(takeoffEndInput, 0.f);
+        VALTAN_CINEMATIC_CAMERA_INPUT dropStartInput = makeInput(
+            "DROP", "valtan.mechanic.arena-break-109.drop", 1u, 0.f);
+        const VALTAN_CINEMATIC_SKY_STATE dropStart =
+            controller.Resolve_SkyState(dropStartInput, 0.f);
         runner.Require(
-            aperture.isActive && aperture.fApertureScale > 0.8f &&
-            aperture.fCloudRotationDegrees > 0.f,
-            "Valtan Sky Aperture Opens On The Authored Stage");
-
-        const VALTAN_CINEMATIC_SKY_STATE cleared = sampleSky(
-            "valtan.mechanic.four-pillars-105.recovery", 3u, 0.99f);
+            takeoffEnd.isActive && dropStart.isActive &&
+            std::abs(takeoffEnd.fCloudRotationDegrees -
+                dropStart.fCloudRotationDegrees) < 0.000001f,
+            "Valtan 109 Sky Rotation Is Continuous Across Stage Cues");
+
+        VALTAN_CINEMATIC_CAMERA_INPUT delayedTakeoffInput = takeoffEndInput;
+        const VALTAN_CINEMATIC_SKY_STATE delayedTakeoff =
+            controller.Resolve_SkyState(delayedTakeoffInput, 0.1f);
+        runner.Require(
+            delayedTakeoff.isActive &&
+            std::abs(delayedTakeoff.fCloudOpacity -
+                takeoffEnd.fCloudOpacity) < 0.000001f &&
+            std::abs(delayedTakeoff.fApertureScale -
+                takeoffEnd.fApertureScale) < 0.000001f,
+            "Valtan 109 Sky Holds The Cue End Until The Next Authoritative Stage");
+
+        VALTAN_CINEMATIC_CAMERA_INPUT dropInput = makeInput(
+            "DROP", "valtan.mechanic.arena-break-109.drop", 1u, 0.5f);
+        const VALTAN_CINEMATIC_SKY_STATE lateDrop =
+            controller.Resolve_SkyState(dropInput, 0.f);
+        const VALTAN_CINEMATIC_SKY_STATE duplicateDrop =
+            controller.Resolve_SkyState(dropInput, 0.05f);
+        const VALTAN_CINEMATIC_SKY_STATE boundedDrop =
+            controller.Resolve_SkyState(dropInput, 1.f);
+        runner.Require(
+            lateDrop.isActive && lateDrop.fCloudOpacity > 0.9f &&
+            duplicateDrop.fCloudOpacity > lateDrop.fCloudOpacity &&
+            duplicateDrop.fCloudRotationDegrees >
+                lateDrop.fCloudRotationDegrees &&
+            boundedDrop.fCloudOpacity > duplicateDrop.fCloudOpacity &&
+            boundedDrop.fCloudOpacity < 0.999f,
+            "Valtan 109 Sky Duplicate Snapshot Uses Bounded Local Interpolation");
+
+        /* A newer snapshot is always authoritative, even when bounded local
+           interpolation had already drawn slightly farther ahead. */
+        dropInput.iServerTick = 116u;
+        const VALTAN_CINEMATIC_SKY_STATE authoritativeDrop =
+            controller.Resolve_SkyState(dropInput, 0.f);
+        runner.Require(
+            authoritativeDrop.isActive &&
+            authoritativeDrop.fCloudOpacity < boundedDrop.fCloudOpacity &&
+            authoritativeDrop.fCloudOpacity > lateDrop.fCloudOpacity,
+            "Valtan 109 Sky New Snapshot Seeks Authoritative Stage Age");
+
+        VALTAN_CINEMATIC_CAMERA_INPUT noCameraInput = dropInput;
+        noCameraInput.strPatternId = "VALTAN_FOUR_PILLARS_105";
+        noCameraInput.strStageId = "TAKEOFF";
+        noCameraInput.strStageActionId =
+            "valtan.mechanic.four-pillars-105.takeoff";
+        noCameraInput.iStageIndex = 0u;
+        VALTAN_CINEMATIC_CAMERA_POSE noCameraPose{};
+        runner.Require(
+            !controller.Update(noCameraInput, 0.f, noCameraPose) &&
+            controller.Get_LastSkyState().isActive &&
+            controller.Get_LastSkyState().strCueId ==
+                authoritativeDrop.strCueId,
+            "Valtan Missing Camera Cue Does Not Reset Active Sky State");
+
+        VALTAN_CINEMATIC_CAMERA_INPUT recoveryInput = makeInput(
+            "RECOVERY", "valtan.mechanic.arena-break-109.recovery", 5u,
+            0.833333f);
+        const VALTAN_CINEMATIC_SKY_STATE cleared =
+            controller.Resolve_SkyState(recoveryInput, 0.f);
         runner.Require(
             cleared.isActive && cleared.fCloudOpacity < 0.1f &&
             cleared.fApertureScale < 0.1f,
-            "Valtan Sky Clears Back To The Storm Sky");
+            "Valtan 109 Sky Clears Back To The Arena Sky During Recovery");

-        /* A pattern that authors no sky window must leave the layer off, and a
-           reset must not keep the last state alive. */
-        const VALTAN_CINEMATIC_SKY_STATE wrongPattern = sampleSky(
-            "valtan.mechanic.arena-break-109.impact", 2u, 0.1f);
+        /* Missing sky authorship resets only the sky clock. It must not tear down
+           an independently active cinematic camera on the same controller. */
+        VALTAN_CINEMATIC_CAMERA_INPUT cameraInput = makeInput(
+            "WIDE_REVEAL",
+            "valtan.mechanic.arena-break-109.wide-reveal", 4u, 0.5f);
+        VALTAN_CINEMATIC_CAMERA_POSE pose{};
+        const bool_t cameraActive = controller.Update(cameraInput, 0.f, pose);
+        VALTAN_CINEMATIC_CAMERA_INPUT wrongPattern = cameraInput;
+        wrongPattern.strPatternId = "VALTAN_ARENA_BREAK_33";
+        wrongPattern.strStageId = "CUTSCENE";
+        wrongPattern.strStageActionId =
+            "valtan.mechanic.arena-break-33.cutscene";
+        wrongPattern.iStageIndex = 0u;
+        const VALTAN_CINEMATIC_SKY_STATE missingSky =
+            controller.Resolve_SkyState(wrongPattern, 0.f);
+        runner.Require(
+            cameraActive && controller.Is_Active() && !missingSky.isActive &&
+            !controller.Get_LastSkyState().isActive,
+            "Valtan Sky Reset Is Isolated From The Cinematic Camera");
         controller.Reset();
         runner.Require(
-            !wrongPattern.isActive &&
+            !controller.Is_Active() &&
             !controller.Get_LastSkyState().isActive,
-            "Valtan Sky Stays Off For Unauthored Stages And After Reset");
+            "Valtan Sky And Camera Reset Clear Their Presentation State");
+
+        CValtanCinematicCameraDocument preserved;
+        const bool_t stableFixtureParsed = encounterLoaded &&
+            CValtanCinematicCameraDocument::Parse_Text(
+                Make_ValtanSkyFixture("VALTAN_PHASE_CHAOS_CLOUD",
+                    "VALTAN_PHASE_SPACEHOLE_CORE"),
+                encounter, preserved, status);
+        const auto rejectsAndPreserves = [&encounter, &preserved](
+            const std::string& candidate)
+        {
+            std::string error;
+            return !CValtanCinematicCameraDocument::Parse_Text(
+                candidate, encounter, preserved, error) && !error.empty() &&
+                preserved.Is_Ready() && preserved.Get_SkyCues().size() == 1u &&
+                preserved.Get_SkyCues().front().strRedCloudAssetId ==
+                    "VALTAN_PHASE_CHAOS_CLOUD" &&
+                preserved.Get_SkyCues().front().strBlackApertureAssetId ==
+                    "VALTAN_PHASE_SPACEHOLE_CORE";
+        };
+        runner.Require(
+            stableFixtureParsed &&
+            rejectsAndPreserves(Make_ValtanSkyFixture(
+                "", "VALTAN_PHASE_SPACEHOLE_CORE")) &&
+            rejectsAndPreserves(Make_ValtanSkyFixture(
+                "Map/ValtanPhase/CHAOS_CLOUD",
+                "VALTAN_PHASE_SPACEHOLE_CORE")) &&
+            rejectsAndPreserves(Make_ValtanSkyFixture(
+                "VALTAN_PHASE_CHAOS_CLOUD", "C:SPACEHOLE")) &&
+            rejectsAndPreserves(Make_ValtanSkyFixture(
+                std::string(129u, 'A'), "VALTAN_PHASE_SPACEHOLE_CORE")),
+            "Valtan Sky Rejects Empty Path Or Oversized Asset IDs Transactionally");
     }

     void Test_ValtanCinematicCameraEasing(TEST_RUNNER& runner)
@@ -5663,6 +5829,45 @@ namespace
                 "valtan.mechanic.arena-break-33.cutscene"),
             "Valtan Camera Loads Every Exact Encounter Tuple");

+        const VALTAN_CINEMATIC_CAMERA_CUE* takeoffCue = actual.Find_Cue(
+            "VALTAN_ARENA_BREAK_109", 0u,
+            "valtan.mechanic.arena-break-109.takeoff");
+        const VALTAN_CINEMATIC_CAMERA_CUE* dropCue = actual.Find_Cue(
+            "VALTAN_ARENA_BREAK_109", 1u,
+            "valtan.mechanic.arena-break-109.drop");
+        const VALTAN_CINEMATIC_CAMERA_CUE* impactCue = actual.Find_Cue(
+            "VALTAN_ARENA_BREAK_109", 2u,
+            "valtan.mechanic.arena-break-109.impact");
+        const auto lookAnchored = [](const VALTAN_CINEMATIC_CAMERA_CUE* cue)
+        {
+            return nullptr != cue && std::all_of(
+                cue->Keyframes.begin(), cue->Keyframes.end(),
+                [](const VALTAN_CINEMATIC_CAMERA_KEYFRAME& frame)
+                {
+                    return std::abs(frame.vLookAt.x - 156.03f) < 0.001f &&
+                        std::abs(frame.vLookAt.z + 122.06f) < 0.001f;
+                });
+        };
+        const bool_t skywardThenImpactReturn = nullptr != takeoffCue &&
+            nullptr != dropCue && nullptr != impactCue &&
+            takeoffCue->Keyframes.size() >= 2u &&
+            dropCue->Keyframes.size() >= 3u &&
+            impactCue->Keyframes.size() >= 2u &&
+            takeoffCue->Keyframes.back().vLookAt.y >
+                takeoffCue->Keyframes.back().vEye.y + 30.f &&
+            dropCue->Keyframes[1u].vLookAt.y >
+                dropCue->Keyframes[1u].vEye.y + 50.f &&
+            dropCue->Keyframes.back().vLookAt.y >
+                dropCue->Keyframes.back().vEye.y + 30.f &&
+            std::abs(impactCue->Keyframes.front().vLookAt.y -
+                dropCue->Keyframes.back().vLookAt.y) < 0.001f &&
+            impactCue->Keyframes.back().vLookAt.y <
+                impactCue->Keyframes.back().vEye.y;
+        runner.Require(
+            skywardThenImpactReturn && lookAnchored(takeoffCue) &&
+            lookAnchored(dropCue) && lookAnchored(impactCue),
+            "Valtan 109 Camera Looks Skyward Then Returns To The Anchor On Impact");
+
         CValtanCinematicCameraDocument preserved;
         const bool_t baseline = encounterLoaded &&
             CValtanCinematicCameraDocument::Parse_Text(
@@ -5679,13 +5884,14 @@ namespace
                 preserved.Get_Cues().front().strCueId == before;
         };
         runner.Require(baseline &&
-            rejectsAndPreserves(Make_ValtanCameraFixture(3u)),
-            "Valtan Camera Rejects Unknown Version Transactionally");
+            rejectsAndPreserves(Make_ValtanCameraFixture(2u)) &&
+            rejectsAndPreserves(Make_ValtanCameraFixture(4u)),
+            "Valtan Camera Rejects Obsolete And Unknown Versions Transactionally");
         runner.Require(baseline && rejectsAndPreserves(
-            Make_ValtanCameraFixture(2u, true)),
+            Make_ValtanCameraFixture(3u, true)),
             "Valtan Camera Rejects Unknown Key Transactionally");
         runner.Require(baseline && rejectsAndPreserves(
-            Make_ValtanCameraFixture(2u, false, true)),
+            Make_ValtanCameraFixture(3u, false, true)),
             "Valtan Camera Rejects Duplicate Encounter Tuple Transactionally");

         CValtanCinematicCameraController controller;
```
