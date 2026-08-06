# 차원술사 2050240 A 경계 돌파 상세 코드 계획

## 1. 변경 경계

기존 Effect runtime을 확장한다. 별도 Particle/Material runtime을 만들지 않는다.

```text
Imported Source Evidence
  -> Effect v11 document
  -> CEffectDocumentCodec validate/stage
  -> CEffectDocumentRenderer
  -> Shader_VtxEffectParticle
```

A의 46 Particle은 pending source material 14 occurrence와 generic standard 32 occurrence로 구성된다.
Material identity 26종을 parent graph 기준 21 profile group으로 자동 연결한다. Light 2와 Screen Post
3은 다음 typed presentation 단계로 유지한다.

2026-08-07 실행기 교정은 Material profile 튜닝 전에 수행한다. 실제 A 캡처에서 발견된 cooked
distribution header 오독과 Color/SubUV 이중 적용을 먼저 제거하지 않으면 profile 수치를 조정해도
잘못된 geometry/UV/color를 기준으로 튜닝하게 된다.

## G40. 구조·성능 baseline

### 대상 파일

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`

현재 구현된 `m_bActiveDocumentMatchesRuntime` cache를 유지한다. Render 함수는 bool만 읽고
`CEffectDocumentCodec::Serialize`를 호출하지 않는다. load/save/promote/commit/discard에서만 cache를
갱신 또는 무효화한다.

하네스는 A의 다음 값을 고정한다.

```text
enabled/disabled notify       13/13
disabled execution             0
active/inactive partition     42/35
Elements                      51
Particle/Light/Post           46/2/3
```

## G41. Material source evidence decoder

### 대상 파일

- `Tools/LevelPlacementExtractor/extract_ue3_placements.py`
- `Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py`
- `Tools/LevelPlacementExtractor/test_effect_extraction_tools.py`
- `Tools/LevelPlacementExtractor/test_extract_umodel_material_dependencies.py`
- `Tools/LevelPlacementExtractor/build_skill_effect_source_receipt.py`

`decode_property_value()`의 nested tagged struct 대상에 UE3 Material input 구조를 추가한다.

```text
ExpressionInput
ColorMaterialInput
ScalarMaterialInput
VectorMaterialInput
Vector2MaterialInput
MaterialInput
```

구조가 tagged property로 끝까지 해석되지 않으면 raw size/hex/decodeError를 보존한다. object/export
reference 0을 정상 node로 만들지 않는다.

`extract_umodel_material_dependencies.py`는 MI props만 읽는 현재 경계를 다음으로 확장한다.

- 같은 physical package 안의 local parent Material export resolve
- recursive parent chain과 cycle 검증
- BlendMode/LightingModel/TwoSided/UseDistortion/depth policy
- parent default와 child override texture/scalar/vector/static switch
- ReferencedTextures/CollectedTextureParameters
- expression non-null/null occurrence와 output link
- package version, native-tail raw hash
- versioned `FMaterialResource / UniformExpressionSet` decode status

native tail hash나 정수 reference scan은 provenance로만 기록한다. versioned decoder가 해석하지 못한
field를 semantic dependency로 승격하지 않는다.

### 산출물

- `Data/Effects/Imported/DimensionMaster/ActionSource/DimensionMaster.A.source-material-contract.json`
- `Data/Effects/Imported/DimensionMaster/ActionSource/DimensionMaster.A.source-material-contract.receipt.json`

contract는 Particle material identity 26/26, parent/profile group 21/21을 소유한다. graph topology가
stripped된 profile은 `RECONSTRUCTED_PROFILE`로 고정한다.

## G42. Effect v11 문서 계약

### 대상 파일

- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Private/Effect_Tool.cpp`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`

추가 타입은 역할을 분리한다.

```cpp
enum class EFFECT_SOURCE_MATERIAL_STATUS : uint8_t
{
    SOURCE_EXACT,
    RUNTIME_EXACT,
    RECONSTRUCTED_PROFILE,
    UNSUPPORTED,
    MISSING_RESOURCE,
    END
};

struct EFFECT_NAMED_FLOAT_DESC
{
    std::string strName;
    float fValue;
};

struct EFFECT_NAMED_FLOAT4_DESC
{
    std::string strName;
    float4_t vValue;
};

struct EFFECT_NAMED_BOOL_DESC
{
    std::string strName;
    bool_t bValue;
};

struct EFFECT_SOURCE_MATERIAL_DESC
{
    std::string strProfileId;
    std::string strParentMaterialPath;
    EFFECT_SOURCE_MATERIAL_STATUS eStatus;
    std::vector<EFFECT_NAMED_FLOAT_DESC> Scalars;
    std::vector<EFFECT_NAMED_FLOAT4_DESC> Vectors;
    std::vector<EFFECT_NAMED_BOOL_DESC> StaticSwitches;
    std::array<int8_t, 4> iDynamicParameterSemantic;
    std::string strSubUVMode;
};
```

`EFFECT_MATERIAL_DESC`는 위 optional payload를 소유한다. source texture는 기존 stable resource binding
형식을 재사용하되 source profile template의 가변 slot registry로 검증한다. format version은 11로
올리고 v3~v10 reader 호환을 유지한다.

Codec validation:

- 빈/미등록 profile ID 거부
- duplicate named parameter 거부
- NaN/Inf 거부
- 안전한 Resources-relative asset ID만 허용
- required slot/resource 미존재 거부
- `UNSUPPORTED/MISSING_RESOURCE` 실행 문서 stage 거부
- 중간 실패 시 기존 document/preview 유지

Effect Tool은 parent, source status, graph coverage, named parameter, DynamicParameter channel,
SubUV policy와 runtime profile을 선택 항목에 따라 동적으로 표시한다.

## G43. Profile registry와 materialization

### 대상 파일

- `Client/Public/Effect_MaterialTemplate.h`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/cook_effect_runtime_resources.py`
- `Tools/LevelPlacementExtractor/export_effect_resources.py`
- 관련 Python unit test

21 profile graph는 stable parent graph ID를 사용한다. instance 26종은 named override payload만 가진다.
기존 72건 `PARAMETER_NAME_HEURISTIC`을 명시적 source parameter binding으로 교체한다.

pending 5종의 직접 확인 dependency:

```text
ring         fx_bg_waterspray_01, fx_d_noise_009
distortion   fx_a_noise_002
aura         fx_a_glow_009, fx_a_cloud_026
circle/dot   tagged referenced texture 없음
```

현재 누락된 aura texture 2종은 원본 FX_TEX package에서 DimensionMaster runtime 폴더로 cook한다.
다른 class resource나 0-byte 파일을 대체 입력으로 사용하지 않는다.

unknown profile은 Renderer가 `S_FALSE`로 숨기지 않는다. publisher/codec stage에서 실패하고 source
material path와 profile ID를 오류로 반환한다.

## G44. Particle shader profile 실행

### 대상 파일

- `Engine/Public/VIBuffer_ParticleRect.h`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`

Renderer는 profile ID를 compact runtime index로 변환하고 profile texture, scalar/vector, blend/depth
pass를 bind한다. shader는 standard 공식을 무조건 적용하지 않고 profile별 typed 계산을 선택한다.

Particle DynamicParameter 33개는 profile registry에 명시된 채널 의미로만 PS에서 소비한다.
pending profile 5개뿐 아니라 기존 standard profile 28개도 coverage에 포함한다. 채널 의미를 source
evidence로 결정할 수 없는 profile은 `RECONSTRUCTED_PROFILE`을 유지한다.

SubUV 2개는 instance payload를 다음처럼 확장한다.

```text
frame0 UV scale/offset
frame1 UV scale/offset
frame blend alpha
8x4 atlas
psuvim_linear_blend
RandomImageTime=1
AllowImageFlipping=true
SquareImageFlipping=true
```

PS에서 두 atlas frame을 sample하고 blend한다. frame selection, random time, square flipping seed는
source recipe의 deterministic seed를 사용한다.

CameraOffset은 Particle 27개를 camera-forward snapshot으로 회귀 검증한다. deferred ScreenPost 3개는
이번 수치에 섞지 않는다.

## G45. Particle module coverage 재산정

### 대상 파일

- `Client/Public/Effect_Playback.h`
- `Client/Private/Effect_Playback.cpp`
- `Tools/LevelPlacementExtractor/build_dimensionmaster_base_effects.py`
- `Tools/LevelPlacementExtractor/materialize_dimensionmaster_base_effects.py`
- 관련 receipt/harness

stale receipt의 전체 560 기준 partial/unsupported 값을 폐기한다. 기본 A Particle scope의 source module
518 occurrence/28 class를 현재 executor capability table에서 재산정한다.

- source order와 duplicate occurrence 유지
- emitter delay와 notify start 분리
- position integration 한 번
- local/world, bone/socket, seeded distribution, vector field, event, orbit/vortex
- sprite/mesh size/rotation, color/alpha, camera offset, dynamic parameter, SubUV

단순히 문서에 존재하거나 executor `switch` 분기를 통과한 것을 실행으로 세지 않는다. receipt는
다음을 별도로 집계한다.

```text
accounted module occurrence
executed module occurrence
unsupported execution
silent ignored module
RUNTIME_EXACT occurrence
RECONSTRUCTED_PROFILE occurrence
```

accounted/executed가 각각 518/518이 아니거나 unsupported/silent ignored가 0이 아니면 미지원
class와 occurrence를 receipt에 그대로 남기고 완료로 표시하지 않는다.

## G46. 검증 순서

1. Python parser/material/profile unit test
2. A 13 active/13 inactive, 42 active partition, 51 Elements test
3. material identity 26/26, profile graph 21/21, pending occurrence 14/14 test
4. parameter-name heuristic 0, silent fallback/skip 0 test
5. DynamicParameter Particle 33, CameraOffset Particle 27, SubUV 2 snapshot
6. active Particle module 518/518 capability receipt
7. Effect publish/materialization과 save/reload identity
8. targeted x64 Debug Client build와 shader compile
9. All Effects A load, Data Files Component/WFX audition
10. Dimension Master 동일 scene profiler recapture
11. 고정 카메라 A00~A04 수동 A/B
12. 관련 `git diff --check`

profiler gate는 per-frame Serialize 0과 기존 `EffectTool.ModelViewWindow` 약 287ms 대비 90% 이상 감소다.
GPU A/B는 시각 회귀이며 stripped graph profile을 `RUNTIME_EXACT`로 승격시키지 않는다.

## G47. 이번 범위 밖

- Light 2 typed cue
- Screen Post 3 typed cue
- Camera/Sound typed presentation
- 원본 uncooked graph 또는 compiled shader가 없는 profile의 `RUNTIME_EXACT` 승격

위 항목은 Particle quad/fallback으로 가장하지 않고 별도 presentation/material exact gate로 남긴다.

## G48. FRawDistribution cooked table decoder

### 대상 파일

- `Client/Private/Effect_Distribution.cpp`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

`LookupTable`은 다음 layout으로 평가한다.

```text
[ range minimum, range maximum,
  entry 0 minimum components..., entry 0 maximum components...?,
  entry 1 minimum components..., entry 1 maximum components...? ... ]
```

앞의 두 range 값은 평가에서 건너뛴다. `Op=None`은 entry당 componentCount개,
`Op=Random/Extreme`은 minimum/maximum 각각 componentCount개를 쓴다. 명시적 chunk가 없을 때
Vector3를 4개로 padding하지 않는다. `(table.size - 2) % chunk != 0`이면 해당 문서를 stage 실패로
처리하고 silent default로 진행하지 않는다.

원본 `LookupTableChunkSize`와 `LookupTableNumElements`는 각각 정본 stride와 `1 또는 2`에 정확히
일치해야 한다. `FRawDistributionVector.Type & 0x07`은 runtime random lock flag로 보존한다. UE와
동일하게 X/Y/Z 난수를 먼저 세 번 소비한 뒤 XY/XZ/YZ/XYZ 축을 잠그며, A의 Type 4 occurrence 13개를
실데이터 계약으로 검증한다.

Python의 `distribution_float()`와 `table_vector_samples()`도 동일하게 range header를 제외한다.
이 함수가 만드는 `Detail` preview와 C++ SourceRecipe 평가값이 서로 달라지지 않게 fixture를 공유한다.

실데이터 회귀값:

```text
A particlespriteemitter_17 StartSize source  = 500, 0, 0
project particle world scale                 = 5, 0, 0
PSA_Square final quad                         = 5 x 5
```

## G49. Source color / authored color override

### 대상 파일

- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`
- `Client/Private/Effect_Playback.cpp` 회귀 확인
- A Imported/Authored 문서와 conversion receipt

SourceRecipe에 color 계열 module이 있으면 생성 결과는 다음을 만족한다.

```text
detail.color.multiply                  [1, 1, 1, 1]
detail.linearLerp.colorMultiply        false
detail.linearLerp.endColorMultiply     [1, 1, 1, 1]
```

emissive, distortion, color offset 등 source color module과 다른 Material/authoring 필드는 유지한다.
Playback은 `Particle.vColor * Detail override`를 최종 색으로 사용한다. 이 식 자체를 제거하면 사용자가
Effect Detail에서 multiplier를 조절할 수 없으므로 생성기의 중복 baseline만 제거한다.

현재 A의 기존 Authored/Imported detail 51개가 전부 동일하다는 precondition을 검사한 뒤에만 A를
재생성한다. 불일치가 하나라도 생기면 자동 overwrite를 중단하고 override migration을 별도 수행한다.

## G50. Source SubUV / authored UV override

### 대상 파일

- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`
- Python/C++ SubUV snapshot

SourceRecipe SubUV가 활성인 element는 생성 시 legacy sequence/tile을 identity로 둔다. instance payload의
`uvTransform/uvTransformNext/blend`가 8x4 frame을 소유하고, 공통 `g_UVOffset`은 panner/start/wave만
더한다. 공통 `g_UVScale`은 이 경우 `(1,1)`이어야 한다. Source SubUV가 없는 legacy element는 기존
Detail sequence 동작을 유지한다.

## G51. 생성·게시·빌드 검증

1. Python distribution/color/SubUV unit test
2. A Authored/Imported detail migration precondition
3. A source document 재생성 및 material profile 재적용
4. Component/Assembly/Runtime Catalog 재생성·publish
5. C++ distribution + actual A emitter harness
6. Effect pipeline 및 final tool harness
7. x64 Debug Client/HLSL build
8. Client startup smoke
9. ProjectAudit와 `git diff --check`
10. Dimension Master A 고정 카메라 수동 재캡처

자동 gate는 실행기 의미와 빌드만 닫는다. 수동 캡처 전에는 A의 시각 결과나 3 FPS 개선을 PASS로
기록하지 않는다. 캡처 후 남는 차이가 geometry/timeline이 아니라 profile별 shading임을 확인한 뒤
21개 Material profile 튜닝으로 진행한다.
