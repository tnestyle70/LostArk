# 차원술사 2050550 S 원본 계약 100% 복원 상세 코드 계획

## 1. 구현 경계

이 문서는
`2026-08-06_DIMENSIONMASTER_2050550_S_RECONSTRUCTION_IMPLEMENTATION_PLAN.md`의 G00~G08을 실제
파일, 타입, 함수, 호출 순서와 검증 단위로 고정한다. 아직 구현 결과를 기록하는 RESULT가 아니다.

핵심 원칙은 다음과 같다.

```text
Source Recipe는 원본 의미를 보존한다.
Authored Override는 stable source ID에 대한 delta만 보존한다.
Publisher는 둘을 병합해 기존 Effect runtime 문서를 만든다.
CEffectPlayback과 CEffectDocumentRenderer를 확장하며 두 번째 runtime은 만들지 않는다.
EXACT가 아닌 mapping은 publish 완료로 인정하지 않는다.
```

## 2. 의존 순서

```text
G00 Source animation time
  ↓
G01 Lossless distribution/module/document
  ↓
G02 Emitter scheduler + particle update
  ↓
G03 Mesh-backed particle
  ↓
G04 Material Instance + shader
  ↓
G05 Light/Post/Shake/Sound
  ↓
G06 Effect Tool source hierarchy
  ↓
G07 Publisher exact gate
  ↓
G08 S regenerate + runtime verification + PNG A/B
```

G01 codec가 고정되기 전에 S 문서를 수동 생성하지 않는다. G04 resource closure 전에는 procedural material에
fallback texture를 넣지 않는다. G07 exact gate 전에는 PNG를 근거로 source parameter를 수정하지 않는다.

## 3. G00 Animation source rate와 gameplay timing

### G00-1. `Tools/CharacterAnimationIntake/build_actorx_fbx.py`

현재 `PsaImportOptions(fps_source="SEQUENCE")`는 source fps를 읽지만 Blender scene이 기본 24fps인 상태를
막지 않는다. 아래 helper와 report field를 추가한다.

```python
@dataclass(frozen=True)
class SequenceTiming:
    name: str
    frame_count: int
    source_fps: float
    duration_seconds: float


def sequence_timing(sequence: Any) -> SequenceTiming:
    fps = float(sequence.fps)
    frames = int(sequence.frame_count)
    if not math.isfinite(fps) or fps <= 0.0:
        raise RuntimeError(f"Invalid PSA sequence fps: {sequence.name}={fps}")
    if frames < 1:
        raise RuntimeError(f"Invalid PSA sequence frame count: {sequence.name}={frames}")
    return SequenceTiming(
        name=str(sequence.name),
        frame_count=frames,
        source_fps=fps,
        duration_seconds=max(0, frames - 1) / fps,
    )


def configure_export_clock(export_fps: int) -> None:
    if export_fps <= 0:
        raise RuntimeError("Export fps must be positive.")
    bpy.context.scene.render.fps = export_fps
    bpy.context.scene.render.fps_base = 1.0


def retime_action_to_seconds(
    action: bpy.types.Action,
    source_fps: float,
    export_fps: float,
) -> None:
    scale = export_fps / source_fps
    for curve in action.fcurves:
        for point in curve.keyframe_points:
            point.co.x *= scale
            point.handle_left.x *= scale
            point.handle_right.x *= scale
        curve.update()
```

DimensionMaster combined FBX의 export clock은 30fps로 고정한다. 각 Action key time은 source seconds가 보존되도록
`export_fps / source_fps`로 변환한다. 따라서 S의 151 frames/30fps는 30fps WModel에서 5.0초이고,
28.xfps source sequence도 임의로 길어지지 않는다. report에는 다음을 추가한다.

```json
{
  "sourceFps": 30.0,
  "exportFps": 30.0,
  "sourceDurationSeconds": 5.0,
  "exportDurationSeconds": 5.0,
  "durationDriftSeconds": 0.0
}
```

FBX export 전 모든 Action을 검사하고, WModel cook 후 별도 inspector가 animation name별
`durationTicks/ticksPerSecond`를 같은 report와 대조한다. 허용 오차는 1ms다. S가 정확해도 다른 153 clip의
drift가 1ms를 넘으면 runtime WModel을 교체하지 않는다.

### G00-2. `Tools/CharacterAnimationIntake/test_build_actorx_fbx.py` 신규

Blender API 없이 검증할 수 있는 timing helper를 분리해 다음을 검사한다.

- 151/30 → 5.0s
- 151/24로 잘못 읽은 6.25s를 통과시키지 않음
- 28.181818fps action을 30fps export clock에 retime해 source seconds 보존
- fps 0, NaN, frame 0 거부
- report JSON round-trip

Python test 파일은 프로젝트 등록 대상이 아니다.

### G00-3. `Client/Private/Character.cpp`

`Calculate_ActiveChainPlaybackRate`는 잘못 쿠킹된 WModel을 balance action duration으로 보정하는 함수로 사용하지
않는다. source-correct WModel을 전제로 ACTIVE clip rate 기본값을 1.0으로 둔다. gameplay가 명시적으로 clip
배속을 소유해야 하는 미래 계약은 animation binding의 typed `playbackRate`로 추가하며
`actionDurationMs`에서 역산하지 않는다.

삭제 대상 호출 흐름:

```text
WModel raw duration / PlayerSkills.actionDurationMs
→ CLIP_CHAIN.fPlaybackRate
→ CModel::Set_AnimationSpeed
```

유지 호출 흐름:

```text
source-correct WModel
→ binding clip 선택
→ CModel::Set_AnimationSpeed(1.0f)
→ model cursor milliseconds로 Effect cue 평가
```

`Set_Animation(const char_t*, bool_t)`와 locomotion 복귀도 1.0을 유지한다. 같은 계산이 있는
`Client/Private/Effect_Tool.cpp`의 `Calculate_SynchronizedAnimationPlaybackRate` 계열도 제거하고 runtime과 Tool이
동일 WModel cursor를 사용한다.

### G00-4. balance와 animation data

`Data/Balance/PlayerSkills.json`의 2050550 행은 다음을 만족해야 한다.

```json
{
  "skillId": 2050550,
  "inputSlot": "S",
  "actionDurationMs": 5000,
  "hitTimeMs": 1730,
  "effectId": "effect.dimensionmaster.skill.2050550"
}
```

`DimensionMaster.skillbindings.json`은
`pc_sp_m_00_sk_sk_super_instance`를 연결하고, `DimensionMaster.animevents`의 Effect cue는 0ms에서 동일 Effect
asset을 시작한다. Effect Element의 start time은 0.010/0.950/1.500/2.178/3.000/3.700/3.750초를 그대로
소유하므로 cue와 element에 같은 offset을 중복 적용하지 않는다.

## 4. G01 Lossless distribution/module/document

### G01-1. `Client/Public/Effect_Distribution.h` 신규

새 파일은 source distribution 타입과 evaluator 공개 계약만 소유한다. 파일 인코딩은 UTF-8 BOM 없음이다.

```cpp
#pragma once

#include "Client_Defines.h"

#include <string>
#include <vector>

namespace Client
{
enum class EFFECT_INTERPOLATION_MODE : uint8_t
{
    CONSTANT,
    LINEAR,
    CUBIC,
    END
};

enum class EFFECT_DISTRIBUTION_KIND : uint8_t
{
    CONSTANT,
    UNIFORM,
    CONSTANT_CURVE,
    UNIFORM_CURVE,
    END
};

struct EFFECT_FLOAT_CURVE_KEY_DESC final
{
    f32_t fTime = 0.f;
    f32_t fValue = 0.f;
    f32_t fArriveTangent = 0.f;
    f32_t fLeaveTangent = 0.f;
    EFFECT_INTERPOLATION_MODE eInterpolation = EFFECT_INTERPOLATION_MODE::LINEAR;
};

struct EFFECT_VECTOR_CURVE_KEY_DESC final
{
    f32_t fTime = 0.f;
    float3_t vValue{};
    float3_t vArriveTangent{};
    float3_t vLeaveTangent{};
    EFFECT_INTERPOLATION_MODE eInterpolation = EFFECT_INTERPOLATION_MODE::LINEAR;
};

struct EFFECT_FLOAT_DISTRIBUTION_DESC final
{
    EFFECT_DISTRIBUTION_KIND eKind = EFFECT_DISTRIBUTION_KIND::CONSTANT;
    f32_t fConstant = 0.f;
    f32_t fUniformMin = 0.f;
    f32_t fUniformMax = 0.f;
    std::vector<EFFECT_FLOAT_CURVE_KEY_DESC> MinKeys;
    std::vector<EFFECT_FLOAT_CURVE_KEY_DESC> MaxKeys;
};

struct EFFECT_VECTOR_DISTRIBUTION_DESC final
{
    EFFECT_DISTRIBUTION_KIND eKind = EFFECT_DISTRIBUTION_KIND::CONSTANT;
    float3_t vConstant{};
    float3_t vUniformMin{};
    float3_t vUniformMax{};
    std::vector<EFFECT_VECTOR_CURVE_KEY_DESC> MinKeys;
    std::vector<EFFECT_VECTOR_CURVE_KEY_DESC> MaxKeys;
};

struct EFFECT_RANDOM_STREAM final
{
    uint32_t iInitialSeed = 0u;
    uint32_t iState = 0u;
};

class CEffectDistribution final
{
public:
    static bool_t Validate(
        const EFFECT_FLOAT_DISTRIBUTION_DESC& Distribution,
        std::string& strError);
    static bool_t Validate(
        const EFFECT_VECTOR_DISTRIBUTION_DESC& Distribution,
        std::string& strError);
    static f32_t Evaluate(
        const EFFECT_FLOAT_DISTRIBUTION_DESC& Distribution,
        f32_t fTime,
        EFFECT_RANDOM_STREAM& Random);
    static float3_t Evaluate(
        const EFFECT_VECTOR_DISTRIBUTION_DESC& Distribution,
        f32_t fTime,
        EFFECT_RANDOM_STREAM& Random);
};
}
```

### G01-2. `Client/Private/Effect_Distribution.cpp` 신규

구현 규칙은 다음으로 고정한다.

- key time은 오름차순이며 duplicate time은 원본 occurrence 순서로 거부한다.
- `CONSTANT`는 random stream을 소비하지 않는다.
- `UNIFORM`은 축마다 원본 분포 정책에 맞는 random draw를 소비한다.
- curve는 key 밖에서 첫/마지막 값을 clamp한다.
- `CONSTANT`, `LINEAR`, `CUBIC`을 구분한다.
- UniformCurve는 min/max curve를 각각 평가한 뒤 하나의 alpha draw로 보간한다.
- 모든 반환값은 finite여야 하며 NaN을 0으로 치환하지 않는다.
- random generator는 platform-independent integer recurrence를 사용하고 seed 0도 명시된 상태로 처리한다.

### G01-3. `Client/Public/Effect_AuthoringDocument.h`

포맷을 9로 올리고 v3~v8 호환 입력은 유지한다.

```cpp
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 9u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;

enum class EFFECT_MAPPING_STATUS : uint8_t
{
    EXACT,
    APPROXIMATION,
    UNSUPPORTED,
    MISSING_RESOURCE,
    DEFAULT_NEEDS_TUNING,
    END
};

enum class EFFECT_MODULE_PHASE : uint8_t
{
    EMITTER,
    SPAWN,
    UPDATE,
    RENDER,
    END
};

enum class EFFECT_MODULE_KIND : uint8_t
{
    REQUIRED,
    SPAWN,
    LIFETIME,
    LOCATION,
    LOCATION_DIRECT,
    LOCATION_EMITTER,
    PRIMITIVE_SPHERE,
    PRIMITIVE_CYLINDER,
    CIRCLE_SURFACE,
    VELOCITY,
    VELOCITY_OVER_LIFE,
    ACCELERATION,
    ORBIT,
    VORTEX,
    LOCAL_VECTOR_FIELD,
    SIZE,
    SIZE_OVER_LIFE,
    ROTATION,
    ROTATION_RATE,
    ROTATION_RATE_OVER_LIFE,
    AXIS_LOCK,
    CAMERA_OFFSET,
    COLOR,
    COLOR_OVER_LIFE,
    SUB_UV,
    MESH_TYPE_DATA,
    MESH_MATERIAL,
    DYNAMIC_PARAMETER,
    EVENT_GENERATOR,
    EVENT_RECEIVER,
    LIGHT_TYPE_DATA,
    SCREEN_POST_TYPE_DATA,
    END
};

struct EFFECT_SOURCE_REF_DESC final
{
    std::string strStableId;
    std::string strPackagePath;
    std::string strObjectPath;
    uint32_t iOccurrence = 0u;
};

struct EFFECT_FIELD_MAPPING_DESC final
{
    std::string strSourceProperty;
    std::string strRuntimeConsumer;
    EFFECT_MAPPING_STATUS eStatus = EFFECT_MAPPING_STATUS::UNSUPPORTED;
    std::string strReason;
};
```

module payload는 이름 없는 `ValueA/ValueB`를 쓰지 않고 의미별 struct를 사용한다. 공통 패턴은 다음이다.

```cpp
struct EFFECT_SPAWN_MODULE_DESC final
{
    EFFECT_FLOAT_DISTRIBUTION_DESC RatePerSecond;
    std::vector<EFFECT_BURST_DESC> Bursts;
};

struct EFFECT_LIFETIME_MODULE_DESC final
{
    EFFECT_FLOAT_DISTRIBUTION_DESC LifeTimeSeconds;
};

struct EFFECT_LOCATION_MODULE_DESC final
{
    EFFECT_VECTOR_DISTRIBUTION_DESC StartLocation;
    bool_t bDistributeOverNPoints = false;
    f32_t fDistributeThreshold = 1.f;
};

struct EFFECT_SIZE_MODULE_DESC final
{
    EFFECT_VECTOR_DISTRIBUTION_DESC Size;
    bool_t bMultiplyX = true;
    bool_t bMultiplyY = true;
    bool_t bMultiplyZ = true;
};

struct EFFECT_ROTATION_MODULE_DESC final
{
    EFFECT_VECTOR_DISTRIBUTION_DESC RotationTurns;
};

struct EFFECT_COLOR_MODULE_DESC final
{
    EFFECT_VECTOR_DISTRIBUTION_DESC RGB;
    EFFECT_FLOAT_DISTRIBUTION_DESC Alpha;
};

enum class EFFECT_AXIS_LOCK : uint8_t
{
    NONE,
    X,
    Y,
    Z,
    NEGATIVE_X,
    NEGATIVE_Y,
    NEGATIVE_Z,
    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z,
    END
};

struct EFFECT_BURST_DESC final
{
    f32_t fEmitterTime = 0.f;
    uint32_t iCountMin = 0u;
    uint32_t iCountMax = 0u;
};

struct EFFECT_MESH_TYPE_DATA_DESC final
{
    std::string strMeshAssetId;
    bool_t bOverrideMaterial = false;
    bool_t bCameraFacing = false;
    EFFECT_AXIS_LOCK eAxisLock = EFFECT_AXIS_LOCK::NONE;
};
```

나머지 payload의 필드 소유권도 아래처럼 고정한다.

| 타입 | 필드 |
|---|---|
| `EFFECT_REQUIRED_MODULE_DESC` | screen alignment, local space, material source ID, sort mode |
| `EFFECT_PRIMITIVE_LOCATION_MODULE_DESC` | shape, center distribution, radius/height distribution, surface-only, velocity-from-position |
| `EFFECT_VELOCITY_MODULE_DESC` | start velocity, radial velocity, world-space, owner-scale, over-life policy |
| `EFFECT_ACCELERATION_MODULE_DESC` | acceleration distribution, owner-scale |
| `EFFECT_ORBIT_MODULE_DESC` | offset, rotation, rotation rate distributions, chain mode |
| `EFFECT_SUB_UV_MODULE_DESC` | horizontal/vertical image count, interpolation mode, sub-image index distribution |
| `EFFECT_MESH_MATERIAL_DESC` | source material ID, override mesh section index |
| `EFFECT_DYNAMIC_PARAMETER_MODULE_DESC` | 네 channel의 name, value distribution, spawn-time-only, velocity mapping policy |
| `EFFECT_PARTICLE_EVENT_MODULE_DESC` | event kind, event name, frequency, receiver source ID, spawn count distribution |
| `EFFECT_LIGHT_MODULE_DESC` | color, brightness, radius, falloff distributions, translucency policy |
| `EFFECT_SCREEN_POST_MODULE_DESC` | RGB noise/zoom subtype, intensity/time distributions, viewport policy |

`EFFECT_SOURCE_OCCURRENCE_DESC`는 source ref, group ID, start/duration, source notify kind와 element stable ID 목록을
소유한다. `EFFECT_PRESENTATION_PAYLOAD_DESC`는 Light/Post/Shake/Sound/Assetless/Model payload의 variant다.
`EFFECT_RENDER_STATE_DESC`는 blend mode, cull, two-sided, depth test/write와 soft-particle policy를 소유한다.
`EFFECT_SHADER_TEMPLATE`와 `EFFECT_MATERIAL_CONSTANT_BUFFER`는 G04에서 S의 root expression 조합을 compile한
stable template 및 그 template의 고정 GPU 상수 레이아웃이다.

모든 payload는 `std::variant`로 한 번만 소유한다.

```cpp
using EFFECT_MODULE_PAYLOAD_DESC = std::variant<
    EFFECT_REQUIRED_MODULE_DESC,
    EFFECT_SPAWN_MODULE_DESC,
    EFFECT_LIFETIME_MODULE_DESC,
    EFFECT_LOCATION_MODULE_DESC,
    EFFECT_PRIMITIVE_LOCATION_MODULE_DESC,
    EFFECT_VELOCITY_MODULE_DESC,
    EFFECT_ACCELERATION_MODULE_DESC,
    EFFECT_ORBIT_MODULE_DESC,
    EFFECT_SIZE_MODULE_DESC,
    EFFECT_ROTATION_MODULE_DESC,
    EFFECT_COLOR_MODULE_DESC,
    EFFECT_SUB_UV_MODULE_DESC,
    EFFECT_MESH_TYPE_DATA_DESC,
    EFFECT_MESH_MATERIAL_DESC,
    EFFECT_DYNAMIC_PARAMETER_MODULE_DESC,
    EFFECT_PARTICLE_EVENT_MODULE_DESC,
    EFFECT_LIGHT_MODULE_DESC,
    EFFECT_SCREEN_POST_MODULE_DESC>;

struct EFFECT_MODULE_RECIPE_DESC final
{
    EFFECT_SOURCE_REF_DESC Source;
    EFFECT_MODULE_KIND eKind = EFFECT_MODULE_KIND::REQUIRED;
    EFFECT_MODULE_PHASE ePhase = EFFECT_MODULE_PHASE::EMITTER;
    bool_t bEnabled = true;
    bool_t bSeeded = false;
    uint32_t iSeed = 0u;
    EFFECT_MODULE_PAYLOAD_DESC Payload;
    std::vector<EFFECT_FIELD_MAPPING_DESC> FieldMappings;
};

struct EFFECT_EMITTER_RECIPE_DESC final
{
    EFFECT_SOURCE_REF_DESC Source;
    f32_t fDelaySeconds = 0.f;
    f32_t fDurationSeconds = 0.f;
    uint32_t iLoopCount = 1u;
    bool_t bLocalSpace = false;
    uint32_t iMaxParticleCount = 0u;
    std::vector<EFFECT_MODULE_RECIPE_DESC> Modules;
};
```

`EFFECT_ELEMENT_DESC`에는 v9 source-exact 실행 recipe를 optional로 추가한다. v9에 recipe가 있으면 flat v8
`Detail.Particle`을 실행 정본으로 사용하지 않는다.

```cpp
std::optional<EFFECT_EMITTER_RECIPE_DESC> SourceEmitter;
```

`EFFECT_DOCUMENT_DESC`에는 source revision, occurrence와 presentation cue를 추가한다.

```cpp
std::string strSourceRevision;
std::vector<EFFECT_SOURCE_OCCURRENCE_DESC> SourceOccurrences;
std::vector<EFFECT_PRESENTATION_CUE_DESC> PresentationCues;
std::vector<EFFECT_ELEMENT_DESC> Elements;
```

### G01-4. `Client/Private/Effect_DocumentCodec.cpp`

추가 private parser/serializer는 타입별로 분리한다.

```cpp
bool_t Parse_FloatDistribution(
    const DATA_JSON_VALUE& Value,
    EFFECT_FLOAT_DISTRIBUTION_DESC& OutDistribution,
    std::string& strOutError);
bool_t Parse_VectorDistribution(
    const DATA_JSON_VALUE& Value,
    EFFECT_VECTOR_DISTRIBUTION_DESC& OutDistribution,
    std::string& strOutError);
bool_t Parse_ModuleRecipe(
    const DATA_JSON_VALUE& Value,
    EFFECT_MODULE_RECIPE_DESC& OutModule,
    std::string& strOutError);
bool_t Parse_EmitterRecipe(
    const DATA_JSON_VALUE& Value,
    EFFECT_EMITTER_RECIPE_DESC& OutEmitter,
    std::string& strOutError);
bool_t Parse_SourceOccurrence(
    const DATA_JSON_VALUE& Value,
    EFFECT_SOURCE_OCCURRENCE_DESC& OutOccurrence,
    std::string& strOutError);
bool_t Parse_PresentationCue(
    const DATA_JSON_VALUE& Value,
    EFFECT_PRESENTATION_CUE_DESC& OutCue,
    std::string& strOutError);
void Serialize_FloatDistribution(
    std::ostringstream& Stream,
    const EFFECT_FLOAT_DISTRIBUTION_DESC& Distribution);
void Serialize_VectorDistribution(
    std::ostringstream& Stream,
    const EFFECT_VECTOR_DISTRIBUTION_DESC& Distribution);
void Serialize_ModuleRecipe(
    std::ostringstream& Stream,
    const EFFECT_MODULE_RECIPE_DESC& Module);
void Serialize_EmitterRecipe(
    std::ostringstream& Stream,
    const EFFECT_EMITTER_RECIPE_DESC& Emitter);
```

`Parse_Value`는 임시 `EFFECT_DOCUMENT_DESC Staged`를 완성한 뒤 `Validate`가 성공할 때만 출력에 대입한다.
v9 source-exact document는 다음을 거부한다.

- empty/duplicate stable ID
- non-finite distribution/key/tangent
- key time 역전 또는 duplicate
- kind와 variant payload 불일치
- module phase 불일치
- source occurrence가 가리키지 않는 emitter
- `EXACT` mapping인데 runtime consumer가 비어 있음
- Resources 밖 asset ID 또는 절대 경로

Serialize → Parse → Serialize 결과는 byte-identical canonical JSON이어야 한다.

## 5. G02 Emitter scheduler와 particle simulator

### G02-1. `Client/Public/Effect_Playback.h`

기존 state를 다음으로 확장한다.

```cpp
struct PARTICLE_STATE final
{
    float3_t vPosition{};
    float3_t vVelocity{};
    float3_t vAcceleration{};
    float3_t vBaseSize{ 1.f, 1.f, 1.f };
    float3_t vSize{ 1.f, 1.f, 1.f };
    float3_t vRotationRadians{};
    float3_t vRotationRateRadiansPerSecond{};
    float3_t vOrbitOffset{};
    float4_t vColor{ 1.f, 1.f, 1.f, 1.f };
    float4_t vDynamicParameter{};
    f32_t fCameraOffset = 0.f;
    f32_t fAgeSeconds = 0.f;
    f32_t fLifeTimeSeconds = 1.f;
    uint32_t iSubImage = 0u;
    EFFECT_RANDOM_STREAM Random{};
    float4x4_t SpawnRootWorld{};
};

struct ELEMENT_STATE final
{
    f32_t fEmitterAgeSeconds = 0.f;
    f32_t fLoopAgeSeconds = 0.f;
    uint32_t iCompletedLoops = 0u;
    uint32_t iNextBurst = 0u;
    f32_t fSpawnAccumulator = 0.f;
    EFFECT_RANDOM_STREAM Random{};
    std::vector<PARTICLE_STATE> Particles;
};
```

추가 private 함수:

```cpp
void Update_Emitter(
    const EFFECT_ELEMENT_DESC& Element,
    ELEMENT_STATE& State,
    f32_t fFixedDelta,
    const float4x4_t& RootWorld);
void Spawn_FromRecipe(
    const EFFECT_EMITTER_RECIPE_DESC& Recipe,
    ELEMENT_STATE& State,
    uint32_t iCount,
    const float4x4_t& RootWorld);
void Apply_SpawnModules(
    const EFFECT_EMITTER_RECIPE_DESC& Recipe,
    ELEMENT_STATE& State,
    PARTICLE_STATE& Particle);
void Apply_UpdateModules(
    const EFFECT_EMITTER_RECIPE_DESC& Recipe,
    PARTICLE_STATE& Particle,
    f32_t fFixedDelta);
void Emit_ParticleEvents(
    const EFFECT_EMITTER_RECIPE_DESC& Recipe,
    const PARTICLE_STATE& Particle,
    EFFECT_PARTICLE_EVENT eEvent);
```

### G02-2. `Client/Private/Effect_Playback.cpp`

`Step`의 v9 호출 순서는 고정한다.

```text
advance document clock
→ resolve occurrence local time
→ advance emitter delay/duration/loop
→ continuous spawn accumulation
→ ordered burst processing
→ source module order spawn evaluation
→ source module order update evaluation
→ particle event delivery
→ remove expired particle
→ Rebuild_Frame
```

fixed substep은 기존 playback step을 사용하되 최대 step 수 초과를 조용히 버리지 않는다. Seek는 Reset 후
동일 fixed steps를 재실행해 restart와 같은 state를 만든다. module별 random stream을 임의로 새로 만들지 않고
source seeded module은 자신의 seed, unseeded module은 emitter stream을 순서대로 소비한다.

원본 단위 변환은 아래 상수 하나로만 적용한다.

```cpp
inline constexpr f32_t UE3_CENTIMETERS_TO_WORLD_METERS = 0.01f;
```

Position, velocity, acceleration, primitive radius/height에 차원에 맞게 적용하고 normalized color, turns,
seconds에는 적용하지 않는다. Size는 renderer/model source unit 계약에 따라 한 번만 적용하며 test에서 mesh와 sprite를
분리한다.

## 6. G03 Mesh-backed particle

### G03-1. evaluated state

`EFFECT_EVALUATED_PARTICLE`에 다음을 추가한다.

```cpp
std::string strMeshAssetId;
float3_t vSize{ 1.f, 1.f, 1.f };
float3_t vRotationRadians{};
float4_t vColor{ 1.f, 1.f, 1.f, 1.f };
float4_t vDynamicParameter{};
uint32_t iSubImage = 0u;
bool_t bMeshParticle = false;
```

`Rebuild_Frame`은 particle world를 다음 순서로 만든다.

```text
source mesh pre-scale/pre-rotation
→ per-particle 3D size
→ per-particle rotation
→ orbit/camera offset/position
→ local-space spawn root 또는 current root
→ document particle-system multiplier
```

### G03-2. `Client/Public/Effect_DocumentRenderer.h`

element 하나당 단일 optional mesh가 아니라 asset ID별 staged resource cache를 둔다.

```cpp
struct MESH_PARTICLE_RESOURCE final
{
    shared_ptr<CModel> pModel;
    std::string strMaterialInstanceId;
};

HRESULT Stage_MeshParticleResource(
    const EFFECT_ELEMENT_DESC& Element,
    const EFFECT_MESH_TYPE_DATA_DESC& MeshType,
    MESH_PARTICLE_RESOURCE& OutResource,
    std::string& strError);
HRESULT Render_MeshParticle(
    const EFFECT_EVALUATED_PARTICLE& Particle,
    const MESH_PARTICLE_RESOURCE& Resource);
```

resource stage는 모든 mesh/material을 임시 cache에 로드한 뒤 성공 시 교체한다. 하나라도 실패하면 현재 renderer
cache를 유지한다.

### G03-3. `Client/Private/Effect_DocumentRenderer.cpp`

`Render_Particles`는 `bMeshParticle`로 분기하되 기존 `CModel`의 mesh/submesh 순회와 `CMaterial` bind를
재사용한다. S 검 mesh는 `Effect/DimensionMaster/Meshes/fm_s_swp_superweapon_01.wmodel`이며 source material
override ID로 G04 material instance를 선택한다. model material fallback을 exact 경로에서 허용하지 않는다.

## 7. G04 Material Instance closure와 shader

### G04-1. `Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py`

기존 MaterialInstanceConstant props의 parent/scalar/vector/texture 파싱을 재귀 closure로 확장한다.

```python
@dataclass(frozen=True)
class MaterialParameterKey:
    association: str
    name: str
    index: int


def resolve_material_instance(
    object_path: str,
    source_index: SourceObjectIndex,
    resolving: set[str],
) -> ResolvedMaterialInstance:
    if object_path in resolving:
        raise MaterialClosureError(f"Material parent cycle: {object_path}")
    source = source_index.require(object_path)
    parent = resolve_parent(source, source_index, resolving)
    return merge_parent_and_overrides(parent, parse_instance_overrides(source))
```

`merge_parent_and_overrides`는 parameter name만 보지 않고 association/name/index key를 사용한다. texture,
scalar, vector, static switch의 override flag가 false면 parent 값을 유지한다. root Material expression graph를 찾지
못하면 `resolved=true`로 만들지 않는다.

receipt의 material 한 행은 다음을 포함한다.

```json
{
  "sourceMaterialPath": "sk_swp_dmc_01.mat.sk_swp_dmc_01_mi_dead",
  "parentChain": ["child", "parent", "root"],
  "textures": [],
  "scalars": [],
  "vectors": [],
  "staticSwitches": [],
  "renderState": {
    "blendMode": "translucent-additive",
    "twoSided": false,
    "depthTest": true,
    "depthWrite": false
  },
  "rootExpressionGraphStatus": "EXACT"
}
```

### G04-2. authoring material descriptor

`EFFECT_MATERIAL_DESC`를 다섯 고정 texture slot의 의미 추측에서 named parameter contract로 확장한다.

```cpp
struct EFFECT_TEXTURE_PARAMETER_DESC final
{
    std::string strName;
    std::string strAssetId;
};

struct EFFECT_SCALAR_PARAMETER_DESC final
{
    std::string strName;
    f32_t fValue = 0.f;
};

struct EFFECT_VECTOR_PARAMETER_DESC final
{
    std::string strName;
    float4_t vValue{};
};

struct EFFECT_STATIC_SWITCH_PARAMETER_DESC final
{
    std::string strName;
    bool_t bValue = false;
};

struct EFFECT_MATERIAL_INSTANCE_DESC final
{
    std::string strSourceMaterialId;
    std::string strShaderTemplateId;
    std::vector<EFFECT_TEXTURE_PARAMETER_DESC> Textures;
    std::vector<EFFECT_SCALAR_PARAMETER_DESC> Scalars;
    std::vector<EFFECT_VECTOR_PARAMETER_DESC> Vectors;
    std::vector<EFFECT_STATIC_SWITCH_PARAMETER_DESC> StaticSwitches;
    EFFECT_RENDER_STATE_DESC RenderState;
};
```

### G04-3. renderer와 HLSL

`CEffectDocumentRenderer::Bind_MaterialInputs`는 이름 기반 recipe를 shader template의 stable binding table로
compile한 결과만 사용한다. 매 frame string lookup을 하지 않는다. stage 시 다음을 만든다.

```cpp
struct COMPILED_EFFECT_MATERIAL final
{
    EFFECT_SHADER_TEMPLATE eTemplate = EFFECT_SHADER_TEMPLATE::STANDARD;
    EFFECT_RENDER_STATE_DESC RenderState;
    std::vector<ComPtr<ID3D11ShaderResourceView>> TextureViews;
    EFFECT_MATERIAL_CONSTANT_BUFFER Constants{};
};
```

`Shader_EffectCommon.hlsli`와 pass shader는 S source graph가 요구하는 Panner, Rotator, Fresnel, distortion,
dissolve, emissive, dynamic parameter를 명시적 template로 구현한다. `effect.standard` 하나에 boolean을 계속
추가하지 않고 expression 조합별 stable template ID를 publisher가 선택한다. 미등록 조합은
`UNSUPPORTED`이며 generic white material로 그리지 않는다.

## 8. G05 Particle 밖 presentation channel

### G05-1. document type

```cpp
enum class EFFECT_PRESENTATION_CUE_KIND : uint8_t
{
    LIGHT,
    RGB_NOISE,
    ZOOM_BLUR,
    VIEW_SHAKE,
    SOUND,
    ASSETLESS_EFFECT,
    MODEL,
    END
};

struct EFFECT_PRESENTATION_CUE_DESC final
{
    EFFECT_SOURCE_REF_DESC Source;
    EFFECT_PRESENTATION_CUE_KIND eKind = EFFECT_PRESENTATION_CUE_KIND::LIGHT;
    f32_t fStartSeconds = 0.f;
    f32_t fDurationSeconds = 0.f;
    EFFECT_PRESENTATION_PAYLOAD_DESC Payload;
    std::vector<EFFECT_FIELD_MAPPING_DESC> FieldMappings;
};
```

Light payload는 position/radius/color/intensity/falloff curve, Post payload는 intensity/time curve와 screen policy,
Shake payload는 amplitude/frequency/duration, Sound payload는 stable cue ID를 소유한다.

### G05-2. runtime 연결

기존 Effect presentation service에 staged cue runner를 추가한다. `CEffectPlayback`은 timeline에서 cue start/stop
event를 evaluated frame에 기록하고, presentation service가 Light/Post/Shake/Sound consumer에 typed command를
제출한다. gameplay damage/hit 판정은 이 경로를 소비하지 않는다.

Seek 시 presentation state를 Reset한 뒤 target time까지 deterministic state를 재구성한다. one-shot sound는 Tool
scrub에서 mute 정책을 적용할 수 있지만 product forward playback에서는 원본 occurrence대로 한 번 실행한다.

## 9. G06 Effect Tool source hierarchy와 override

### G06-1. `Client/Public/Effect_Tool.h`

selection을 다음처럼 확장한다.

```cpp
enum class EFFECT_DETAIL_SELECTION : uint8_t
{
    NONE,
    DOCUMENT,
    OCCURRENCE_GROUP,
    PARTICLE_SYSTEM,
    EMITTER,
    MODULE,
    ELEMENT,
    PRESENTATION_CUE,
    END
};

struct EFFECT_SOURCE_SELECTION final
{
    EFFECT_DETAIL_SELECTION eKind = EFFECT_DETAIL_SELECTION::NONE;
    std::string strEffectAssetId;
    std::string strOccurrenceId;
    std::string strEmitterId;
    std::string strModuleId;
    std::string strElementId;
};
```

추가 함수:

```cpp
void Render_SourceHierarchy();
void Render_SourceCoverage();
void Render_SourceModuleDetail();
void Render_AuthoredOverrideDetail();
bool_t Try_SelectSource(const EFFECT_SOURCE_SELECTION& Selection);
bool_t Try_SeekSelectionStart();
bool_t Try_SoloSelection();
bool_t Stage_OverrideDraftPreview();
bool_t Apply_OverrideDraft(EFFECT_AUTHORED_OVERRIDE_DOCUMENT& Document) const;
bool_t Validate_OverrideTarget(std::string& strError) const;
```

### G06-2. 화면 계약

All Effects의 S 한 행은 완성 Effect 하나다. 확장 트리는 다음 순서다.

```text
S | 찰나 | Exact 0/200
├─ Body Animation | 5.000s @ source 30fps
├─ Clock / Initial Field | 0.010s
│  └─ System → Emitter → Module
├─ Background / Time Pause | 0.950s
├─ Weapon | 1.500s
├─ Hand | 2.178s
├─ Hole / Crack | 3.000s
├─ Final Impact | 3.700s / 3.750s
├─ Light / Camera / Post / Sound
└─ Coverage
```

Detail 상단은 선택 종류와 stable source ID를 항상 표시한다. 원본 값은 read-only, runtime sample은 read-only,
override만 editable이다. Apply는 memory override document만 dirty로 만들고 Save가 파일을 교체한다. 다른 문서
선택은 기존 pending load modal의 Save/Discard/Cancel을 사용한다.

`Seek to Source Start`는 선택 occurrence의 Notify start 또는 emitter/module start를 사용한다. `Solo`는 선택
group의 다른 source occurrence를 숨기되 timeline clock과 body animation을 임의로 0으로 재정의하지 않는다.

## 10. G07 Converter, override merge, publisher

### G07-1. `build_imported_effect_documents.py`

현재 `emitter_detail`에서 acceleration/size/color curve를 상수 또는 start/end로 축약하는 코드를 v9 source recipe
builder로 교체한다.

```python
FLOAT_DISTRIBUTION_BUILDERS = {
    "distributionfloatconstant": build_float_constant,
    "distributionfloatuniform": build_float_uniform,
    "distributionfloatconstantcurve": build_float_constant_curve,
    "distributionfloatuniformcurve": build_float_uniform_curve,
}

VECTOR_DISTRIBUTION_BUILDERS = {
    "distributionvectorconstant": build_vector_constant,
    "distributionvectoruniform": build_vector_uniform,
    "distributionvectorconstantcurve": build_vector_constant_curve,
    "distributionvectoruniformcurve": build_vector_uniform_curve,
}

MODULE_BUILDERS = {
    "particlemodulerequired": build_required_module,
    "particlemodulespawn": build_spawn_module,
    "particlemodulelifetime": build_lifetime_module,
    "particlemodulelocation": build_location_module,
    "particlemodulelocation_seeded": build_location_seeded_module,
    "particlemodulelocationdirect": build_location_direct_module,
    "efparticlemodulelocationemitter": build_location_emitter_module,
    "particlemodulelocationprimitivesphere": build_sphere_module,
    "particlemodulelocationprimitivesphere_seeded": build_sphere_seeded_module,
    "efparticlemodulelocationprimitivesphere": build_effect_sphere_module,
    "particlemodulelocationprimitivecylinder": build_cylinder_module,
    "particlemodulelocationprimitivecylinder_seeded": build_cylinder_seeded_module,
    "efparticlemodulelocationcirclesurface": build_circle_surface_module,
    "particlemodulevelocity": build_velocity_module,
    "particlemodulevelocityoverlifetime": build_velocity_over_life_module,
    "efparticlemodulevelocityoverlifetime": build_effect_velocity_over_life_module,
    "particlemoduleacceleration": build_acceleration_module,
    "efparticlemoduleacceleration": build_effect_acceleration_module,
    "particlemoduleorbit": build_orbit_module,
    "efparticlemodulevortex": build_vortex_module,
    "particlemodulelocalvectorfield": build_local_vector_field_module,
    "particlemodulevectorfieldrotationrate": build_vector_field_rotation_rate_module,
    "particlemodulevectorfieldscaleoverlife": build_vector_field_scale_over_life_module,
    "particlemodulesize": build_size_module,
    "particlemodulesize_seeded": build_size_seeded_module,
    "particlemodulesizemultiplylife": build_size_over_life_module,
    "particlemodulerotation": build_rotation_module,
    "particlemodulerotation_seeded": build_rotation_seeded_module,
    "particlemodulerotationrate": build_rotation_rate_module,
    "particlemodulerotationratemultiplylife": build_rotation_rate_over_life_module,
    "particlemodulemeshrotation": build_mesh_rotation_module,
    "particlemodulemeshrotation_seeded": build_mesh_rotation_seeded_module,
    "particlemodulemeshrotationrate": build_mesh_rotation_rate_module,
    "particlemodulemeshrotationratemultiplylife": build_mesh_rotation_rate_over_life_module,
    "particlemoduleorientationaxislock": build_axis_lock_module,
    "particlemodulecameraoffset": build_camera_offset_module,
    "particlemodulecolor": build_color_module,
    "particlemodulecolor_seeded": build_color_seeded_module,
    "particlemodulecoloroverlife": build_color_over_life_module,
    "particlemodulecolorscaleoverlife": build_color_scale_over_life_module,
    "particlemodulesubuv": build_sub_uv_module,
    "particlemoduletypedatamesh": build_mesh_type_data_module,
    "particlemodulemeshmaterial": build_mesh_material_module,
    "particlemoduleparameterdynamic": build_dynamic_parameter_module,
    "particlemoduleeventgenerator": build_event_generator_module,
    "particlemoduleeventreceiverspawn": build_event_receiver_spawn_module,
    "efparticlemoduletypedatalight": build_light_type_data_module,
    "efparticlemoduletypedatadecal": build_decal_type_data_module,
}

def build_distribution_float(source: SourceObject) -> dict[str, Any]:
    builder = FLOAT_DISTRIBUTION_BUILDERS.get(folded(source.class_name))
    if builder is None:
        raise UnsupportedSourceType(source.class_name, source.object_path)
    return validate_lossless_distribution(builder(source))

def build_distribution_vector(source: SourceObject) -> dict[str, Any]:
    builder = VECTOR_DISTRIBUTION_BUILDERS.get(folded(source.class_name))
    if builder is None:
        raise UnsupportedSourceType(source.class_name, source.object_path)
    return validate_lossless_distribution(builder(source))

def build_module_recipe(module: SourceObject, context: BuildContext) -> dict[str, Any]:
    builder = MODULE_BUILDERS.get(folded(module.class_name))
    if builder is None:
        raise UnsupportedSourceType(module.class_name, module.object_path)
    return builder(module, context)

def build_emitter_recipe(
    occurrence: SourceOccurrence,
    partition: EmitterPartition,
    context: BuildContext,
) -> dict[str, Any]:
    modules = [
        build_module_recipe(context.source_index.require(module_id), context)
        for module_id in partition.module_ids
    ]
    return {
        "source": build_source_ref(occurrence, partition),
        "delaySeconds": partition.delay_seconds,
        "durationSeconds": partition.duration_seconds,
        "loopCount": partition.loop_count,
        "localSpace": partition.local_space,
        "maxParticleCount": partition.max_particle_count,
        "modules": modules,
    }
```

위 함수의 실제 구현에서는 placeholder 반환을 허용하지 않는다. 알 수 없는 distribution/module class는 source
class/path/property dump와 함께 `UNSUPPORTED`로 receipt에 남기고 S exact publish를 실패시킨다. source occurrence
단위 stable ID는 다음 조합으로 생성한다.

```text
skillId / notifyIndex / sourceSystemPath / emitterPath / lodIndex / modulePath / moduleOccurrence
```

동일 BGCrack system이 3.000초에 두 번 호출된 경우 notifyIndex가 다르므로 둘 다 보존된다.

### G07-2. Authored override schema

`Data/Effects/Authored`의 S 문서는 generated Element 복사본 대신 다음 stable delta를 소유하도록 migration한다.

```cpp
enum class EFFECT_OVERRIDE_OPERATION : uint8_t
{
    REPLACE,
    ADD,
    MULTIPLY,
    END
};

using EFFECT_OVERRIDE_VALUE = std::variant<
    bool_t,
    int32_t,
    uint32_t,
    f32_t,
    float3_t,
    float4_t,
    std::string>;

struct EFFECT_FIELD_OVERRIDE_DESC final
{
    std::string strTargetStableId;
    std::string strFieldPath;
    EFFECT_OVERRIDE_OPERATION eOperation = EFFECT_OVERRIDE_OPERATION::REPLACE;
    EFFECT_OVERRIDE_VALUE Value;
    std::string strReason;
};

struct EFFECT_AUTHORED_OVERRIDE_DOCUMENT final
{
    uint32_t iSchemaVersion = 1u;
    std::string strEffectAssetId;
    std::string strSourceRevision;
    std::vector<EFFECT_FIELD_OVERRIDE_DESC> Overrides;
};
```

override codec는 Effect document codec과 분리된 file-level parser이지만, merge 결과는 반드시
`CEffectDocumentCodec::Validate`를 통과한 기존 `EFFECT_DOCUMENT_DESC`다. runtime은 override 파일을 직접 읽지
않는다.

```json
{
  "schema": "lostark.effect-authored-override",
  "schemaVersion": 1,
  "effectAssetId": "effect.dimensionmaster.skill.2050550",
  "sourceRevision": "<sha256>",
  "overrides": [
    {
      "targetStableId": "<source-id>",
      "fieldPath": "modules/<module-id>/size/uniformScale",
      "operation": "multiply",
      "value": 1.0,
      "reason": "renderer calibration"
    }
  ]
}
```

identity `1.0` override는 저장하지 않는다. sourceRevision이 달라졌을 때 target과 field가 동일하게 재해결되지
않으면 자동 이식하지 않고 publish를 실패시킨다.

### G07-3. `Tools/EffectPipeline/Publish-Effects.ps1`

publisher 함수 경계:

```powershell
function Read-EffectImportedRecipe { param([string]$Path) }
function Read-EffectAuthoredOverride { param([string]$Path) }
function Test-EffectSourceCoverage { param($Recipe, [string]$Profile) }
function Merge-EffectOverride { param($Recipe, $Override) }
function Compile-EffectRuntimeDocument { param($MergedRecipe) }
function Write-EffectRuntimeDocumentAtomic { param($Document, [string]$Path) }
```

S는 `-Profile Exact`를 사용한다. 아래 합계가 하나라도 0이 아니면 commit 전에 실패한다.

```text
approximationCount
unsupportedCount
missingResourceCount
defaultNeedsTuningCount
fallbackResourceCount
unusedSourceFieldCount
```

runtime JSON과 receipt는 임시 디렉터리에 완성하고 JSON parse, asset path, shader template, codec harness가 모두
성공한 뒤 `Move-Item`으로 교체한다. 중간 실패 시 기존 runtime 파일을 유지한다.

## 11. G08 S exact closure와 검증

### G08-1. 자동 test 순서

1. `test_build_actorx_fbx.py`: source seconds와 export seconds.
2. `test_extract_ue3_particle_module_closure.py`: 1,647/1,647.
3. `test_build_imported_effect_documents.py`: 28 occurrence, 200 partition, distribution/module round-trip.
4. material dependency test: 140/140 material, 210/210 resource, parent/static switch cycle rejection.
5. `Test-EffectPipeline.ps1`: exact publish와 rollback.
6. `ClientFrontendHarness`: codec, distribution, fixed-step particle, mesh transform, cue timeline.
7. `Test-EffectToolFinal.ps1`: hierarchy/selection/seek/solo/apply/save/reload 계약.
8. Debug/Release build/regression.
9. `Invoke-ProjectAudit.ps1`.
10. `git diff --check`.

### G08-2. ClientFrontendHarness fixture

S 전체 200 emitter를 GPU 없이 완전히 비교하는 fixture와, 각 기능의 최소 fixture를 함께 둔다.

```text
sample 0.000s: no 0.010s occurrence
sample 0.010s: clock occurrence active
sample 0.950s: background occurrence active
sample 1.500s: weapon mesh emitter active
sample 1.730s: gameplay hit timestamp unchanged by visual timeline
sample 2.178s: hand occurrence active
sample 3.000s: hole + duplicate crack occurrences active
sample 3.700s: first final occurrence active
sample 3.750s: final particle/post/light/shake occurrences active
sample 3.800s: cancel window begins
sample 5.000s: body animation complete
```

timeline 비교는 1ms 이내다. particle float state는 정해진 epsilon을 사용하고 asset ID/status/count는 정확히
일치해야 한다.

### G08-3. 수동 PNG A/B

exact gate 통과 후 fixed camera, fixed resolution, fixed exposure로 S00~S06과 같은 landmark를 캡처한다. 차이는
다음 두 종류로만 기록한다.

```text
SOURCE_MAPPING_DEFECT
RENDERER_CALIBRATION_DIFFERENCE
```

PNG에서 보이는 길이를 근거로 timeline, velocity, rotation, lifetime을 수정하지 않는다. source mapping defect는
parser/runtime를 고치고, renderer 차이는 shader/blend/exposure 계약 또는 명시적 authored override로 고친다.

## 12. 프로젝트 등록

신규 C++ 두 파일만 프로젝트에 등록한다.

`Client/Default/Client.vcxproj`:

```xml
<ClInclude Include="..\Public\Effect_Distribution.h" />
<ClCompile Include="..\Private\Effect_Distribution.cpp" />
```

`Client/Default/Client.vcxproj.filters`:

```xml
<ClInclude Include="..\Public\Effect_Distribution.h">
  <Filter>Header Files\Effect</Filter>
</ClInclude>
<ClCompile Include="..\Private\Effect_Distribution.cpp">
  <Filter>Source Files\Effect</Filter>
</ClCompile>
```

실제 기존 filter 이름을 구현 시 다시 확인해 동일한 filter를 사용하며 새 filter로 기존 항목을 재배치하지 않는다.
Python test와 JSON은 vcxproj에 등록하지 않는다. Git 관리 대상 Data 원본은 기존처럼 Client 프로젝트의
`96.DataFiles` 아래 `None` 계약만 따른다.

## 13. G별 완료 체크

| G | 코드 완료 | 자동 검증 | 수동 검증 |
|---|---|---|---|
| G00 | source seconds 보존 cook, 임시 rate 제거, hit 1730 | 154 clip drift ≤1ms | S body 5.0s |
| G01 | v9 codec/distribution/module | canonical round-trip | Tool 원본 값 표시 |
| G02 | S module simulator | fixed-seed state | particle motion |
| G03 | mesh particle transform/material | transform snapshot | 검 방향/크기 |
| G04 | MI closure/shader | 140/140, 210/210 | emissive/blend |
| G05 | typed presentation cue | start/stop/seek | Light/Post/Shake |
| G06 | hierarchy/override UX | tool audit | 선택/solo/apply/save |
| G07 | exact publisher | fail-closed/rollback | 없음 |
| G08 | S exact runtime | exact gate 전체 | PNG A/B |

각 G의 구현 결과는 대응 RESULT에 `구현 완료 / 자동 검증 / 수동 검증 / 남은 경계`로 분리해 기록한다.
S의 `APPROXIMATION 163 + UNSUPPORTED 37`이 모두 `EXACT 200`으로 전환되기 전에는 최종 완료로 표시하지 않는다.
