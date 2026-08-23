# 2026-08-23 대표 4스킬 V1 StandardColor 수평 슬라이스 계획

기준 branch: `codex/representative-four-v1-material-slice`

최종 재배치 기준 main: `79b5ba042d9351198e517ff27c464fb1a1538190`

최종 화면 판정자: 사용자

## 0. 결론

기존 V0 effect 문서는 수정하지 않는다. 대표 4스킬의 원래 composition, carrier, timing,
stable occurrence와 authoring 값을 복제한 병렬 V1 문서 5개를 만든다.

```text
DimensionMaster R 2050180       10행
Artist A 31460                  18행
LanceMaster D 34110             88행
Warlord R 17110 clip2            3행
Warlord R 17110 clip3           12행
                                -----
Sprite 98 + Mesh 25 + Decal 8 = 131행
```

워로드 R clip2/clip3를 하나로 합치면 원래 cue timing과 composition이 바뀌므로 물리 문서는
5개다. Effect Tool에서는 V0와 V1을 별도 Saved Unified Effect로 열고 비교한다. 사용자 승인 전에는
Product animevent/cue를 V1으로 교체하지 않는다.

## 1. 판정 라벨

### `SOURCE_EXACT`

원본 occurrence의 Program, ordered ABI, texture/channel/sampler, scalar/vector/dynamic parameter,
blend/depth/cull/pass/MRT와 compiled draw가 모두 닫힌 경우에만 사용한다.

현재 대표 131행의 실행 가능한 `SOURCE_EXACT` admission은 `0`이다. occurrence-level HLSL 근거가
있는 행을 runtime Program이 닫힌 것처럼 승격하지 않는다.

### `PROJECT_TUNED_APPROX`

V0의 composition/carrier/timing/resource evidence를 보존하고, 프로젝트 공용 typed
`StandardColorV1` RT0 식으로 base radiance, coverage, optional dissolve, lifetime alpha를
결정적으로 연결한다. 원본 shader 함수의 exact 재현이라고 표기하지 않는다.

이번 V1 131행은 모두 이 라벨로 생성한다. lane을 닫지 못하는 행은 흰 texture나 generic fallback을
조용히 사용하지 않고 publisher에서 실패시킨다. textureless source family에만 도메인별로 명시한
PROJECT_TUNED fallback DDS를 사용하고 receipt에 기록한다.

`standardColorV1` backend 자체는 이번 단계에서 PROJECT_TUNED 전용이다. 따라서 Effect Tool은 enabled
packet을 내부 exact mirror라는 이유로 `EXACT`라고 표시하지 않고 `APPROXIMATE`로 표시하며 Animation
Tool Product cue 전송도 막는다. 나중에 StandardColor를 `SOURCE_EXACT`로 쓰려면 source graph 동치성을
검증하는 별도 evidence-bearing fidelity 계약을 먼저 추가한다.

## 2. 공용 Material 식

```text
baseRadiance = Sample(base lane, selected R/G/B/RGB channel)
coverage     = Sample(coverage lane, selected R/G/B/A channel)
coverage    *= carrier alpha

optional dissolve:
  coverage *= smoothstep(threshold - softness,
                         threshold + softness,
                         Sample(dissolve lane, selected channel))

RT0.rgb = baseRadiance * carrier color
RT0.a   = coverage
RT1.xy  = 0
```

기존 `Shader_EffectStandardColorV1.hlsli`를 두 번째 renderer 없이 사용한다. SpriteParticle,
MeshParticle CModel, LocalDecal projector의 기존 draw에 typed packet을 바인딩한다. Mesh shader에만
동일 include/dispatch가 빠져 있으므로 기존 Mesh draw에 추가한다. MeshParticle의 source SubUV도 같은
particle frame resolver가 계산한 current/next atlas transform과 blend를 기존 Mesh draw에 전달한다.

lane은 전부 `R/linear/wrap`으로 평탄화하지 않는다. base/emissive radiance는 RGB, scalar 계열은 R을
사용하고, coverage/dissolve는 DDS alpha가 실제로 변할 때 A를 사용한다. base color-space와 U/V address는
동일 asset의 유일한 `sourceProfile.textures[]` 근거를 상속한다. 근거가 없으면 명시적 PROJECT_TUNED
`linear/wrap/wrap` 정책을 쓰며, filter=linear와 Texture2D W=wrap도 receipt에 approximation으로 남긴다.

## 3. compiled Adapter 범위

Adapter는 skill/class가 아니라 `carrier × render profile × shader/layout × MRT`의 compiled
allowlist다.

| carrier | profile | pass |
|---|---|---:|
| SpriteParticle | alpha two-sided depth-read | 1 |
| SpriteParticle | additive two-sided depth-read | 2 |
| SpriteParticle | alpha one-sided depth-read | 3 |
| SpriteParticle | additive one-sided depth-read | 4 |
| MeshParticle CModel | alpha two-sided depth-read | 1 |
| MeshParticle CModel | alpha one-sided depth-read | 3 |
| LocalDecal projector | alpha two-sided projector depth-sample (`DSS_ZNone`) | 1 |
| LocalDecal projector | alpha one-sided depth-read | 3 |

모든 Adapter는 `MRT_SceneHDR`, scene color `SV_TARGET0`, deterministic zero distortion
`SV_TARGET1`을 고정한다. JSON은 shader path, pass, raster/depth/blend/MRT를 작성할 수 없다.

## 4. Registry와 dual-resolve

Registry v1에 공용 Program `standardColorV1/opcode 1`을 허용한다. Layout은 2개 또는 3개의 contiguous
lane과 역할/channel/color-space ABI 조합별 stable ID를 선언하고 Descriptor는 texture/sampler만 공급한다. StandardColor
typed packet은 Layout의 역할에서 결정적으로 합성한다.

```text
base_radiance                     -> baseRadianceLaneId/channel
coverage                          -> coverageLaneId/channel
dissolve                          -> optional dissolve lane/channel
```

V1 문서에는 Registry가 생성한 packet과 같은 inline `material.execution` mirror를 둔다.
Python publisher와 C++ loader 모두 모든 ordered field, lane, sampler, StandardColor enum/string,
float32 bit pattern까지 비교한다. Binding이 있는 occurrence의 packet/carrier/adapter/draw mismatch는
fail-closed이며 inline fallback을 사용하지 않는다. Binding 0인 기존 V0 문서는 기존 draw를 유지한다.

## 5. V1 문서 identity

```text
effect.dimensionmaster.skill.2050180.v1.unified
effect.artist.skill.31460.v1.unified
effect.lancemaster.skill.34110.v1.unified
effect.warlord.skill.17110.clip2.v1.unified
effect.warlord.skill.17110.clip3.v1.unified
```

각 문서는 기존 V0의 element ID를 그대로 보존한다. effectAssetId와 displayName만 V1 identity로
분리한다. EffectCatalog에는 `REGISTRY_BOUND_AUDITION_ONLY`, `PROJECT_TUNED_APPROX`, V0 stable ID와 raw
SHA-256 seal을 함께 등록한다. Publisher는 marker 일부 누락, source drift, StandardColor Product cue 편입을
거부한다.

## 6. 실제 호출 흐름

```text
V0 authored document
-> deterministic V1 materializer
-> V1 authored document + provenance receipt + registry fragment
-> Publish-Effects validation
-> EffectCatalog immutable revision/generation
-> Program/Layout/Descriptor/Adapter/Binding resolve
-> inline/registry bit-exact compare
-> prewarm target
-> prepared document snapshot
-> existing Render_Particles / Render_Mesh / Render_Decal
-> actual pass/state/MRT validation
-> user Effect Tool V0/V1 visual A/B
```

## 7. 변경 파일

### 생성 도구와 데이터

- `Tools/EffectPipeline/materialize_representative_four_v1_standard_color.py`
- `Tools/EffectPipeline/test_materialize_representative_four_v1_standard_color.py`
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
- `Data/Effects/AuthoredCorrections/Generated/RepresentativeFour/representative-four-v1-standard-color.receipt.json`
- `Data/Effects/MaterialPrograms/Fragments/representative-four-v1-standard-color.material-program-fragment.v1.json`
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050180.v1.unified.effect.json`
- `Data/Effects/Authored/effect.artist.skill.31460.v1.unified.effect.json`
- `Data/Effects/Authored/effect.lancemaster.skill.34110.v1.unified.effect.json`
- `Data/Effects/Authored/effect.warlord.skill.17110.clip2.v1.unified.effect.json`
- `Data/Effects/Authored/effect.warlord.skill.17110.clip3.v1.unified.effect.json`
- `Data/Effects/EffectCatalog.json`

### Registry와 runtime

- `Tools/EffectPipeline/build_effect_material_program_registry.py`
- `Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json`
- `Tools/EffectPipeline/test_build_effect_material_program_registry.py`
- `Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`
- `Client/Public/Effect_MaterialProgramRegistry.h`
- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Private/Effect_MaterialProgramRegistry.cpp`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Private/Effect_Tool.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl`

### 프로젝트와 증거

- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
- `.md/GB/08-23/2026-08-23_REPRESENTATIVE_FOUR_V1_STANDARD_COLOR_HORIZONTAL_SLICE_RESULT.md`
- `.gitattributes`

새 C++ 파일은 만들지 않는다. 새 Git 관리 Data/도구 파일은 실제 project/filter convention에 따라
필요한 항목만 등록한다.

## 8. 자동 검증

- V1 5문서, 131행, S98/M25/D8 identity와 V0 composition/timing hash 보존
- 모든 행 `PROJECT_TUNED_APPROX`, `SOURCE_EXACT` 허위 admission 0
- 131 enabled inline packet과 131 Registry Binding의 field/float-bit exact compare
- unknown/missing lane, wrong channel, adapter/profile/carrier mismatch, inline drift 거부
- audition marker/source SHA drift, Product cue 승격, 허위 `SOURCE_EXACT` 거부
- immutable catalog revision과 registry generation이 prewarm/prepared snapshot까지 유지
- Sprite/Mesh/Decal의 compiled adapter가 actual shader/pass/state/MRT draw에서 검증
- MeshParticle source SubUV current/next atlas transform이 실제 Mesh shader까지 전달
- Binding 0 V0 baseline 통계 불변
- focused Python tests, publisher Check, EffectRenderContractHarness Debug/Release
- Engine/UpdateLib/Client Debug/Release
- JSON/XML parse, `git diff --check`

## 9. 사용자 visual gate

에이전트는 Client나 Effect Tool을 실행하거나 화면 PASS를 선언하지 않는다. 빌드가 끝나면 정본 폴더의
`Framework.sln`, 구성/profile, V0/V1 exact asset identity, baseline/candidate commit, Client EXE와 runtime
catalog SHA-256을 보고한다. 사용자가 직접 Effect Tool의 All Effects에서 같은 스킬의 V0/V1 Saved
Unified Effect를 선택해 Solo 재생하고 최종 방향을 결정한다.

## 10. 의도적으로 제외

- Product animevent/cue의 V1 자동 교체
- Artist F 31470 Full35 변경
- Valtan 669 ledger 또는 Track B artifact의 runtime proof 승격
- Mesh WPO/native vertex parity, Glass/distortion, Trail, Presentation
- exact로 닫히지 않은 원본 shader 함수를 SOURCE_EXACT로 표시하는 것
- skill-ID/class-ID renderer switch
