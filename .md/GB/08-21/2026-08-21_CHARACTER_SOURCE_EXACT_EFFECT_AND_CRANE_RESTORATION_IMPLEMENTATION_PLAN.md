# 2026-08-21 Character Source-Exact Effect 및 두루미 복원 구현 계획

> **최종 runtime 계약 정정**
> Debug Effect Tool, All Effects, selected Save Hot Reload와 Product cue approval/admission은
> `2026-08-21_EFFECT_RUNTIME_SIMPLIFICATION_AND_BA_REGRESSION_IMPLEMENTATION_PLAN.md`에서
> 제거됐다. 이 문서의 source carrier/material family/anchor 복원 범위는 계속 유효하지만,
> 제품 반영은 전체 authored 저장이 끝난 뒤 4-field catalog를 만드는 별도 Full Effect Publish와
> Client 재시작으로 수행한다.

## 0. 목표

이번 대상은 다음 exact `(class, inputSlot, skillId)`다.

| class | slot | skillId | name |
|---|---:|---:|---|
| DimensionMaster | W | 2050120 | 세로 검격 carrier 복원 대상 |
| DimensionMaster | R | 2050180 | 너머 베기 |
| DimensionMaster | S | 2050220 | 일점 관통 |
| DimensionMaster | D | 2050240 | 경계 돌파 |
| DimensionMaster | F | 2050230 | 사각 card 원인과 제한형 material profile 진단 대상 |
| DimensionMaster | T | 2050500 | 업의 경계 |
| LanceMaster | V | 34610 | 적룡질풍격 |
| Warlord | T | 17240 | 풀배럴 캐넌 |
| Artist | E | 31480 | 묵법 : 두루미나래 |

LanceMaster Z나 Warlord Z로 바꾸지 않는다. 현재 balance 정본에서 Lance 대상은 V이고,
Warlord Z는 방어 태세 전환이다.

사용자가 외부 JSON 저작 흐름에서 네 캐릭터를 튜닝하고 있으므로 기존 authored JSON의 미커밋 수치를
덮어쓰지 않는다. 이번 구현은 source carrier/material/shader/model identity를 닫고, 제품 반영은 모든
authored 저장이 끝난 뒤 full publisher가 만드는 4-field direct catalog와 Client 재시작으로 검증한다.
formatVersion 3 VisualProgram sidecar는 marker와 무관하게 항상 parse/validate/stage한다.

사용자 결정에 따라 새 전용 harness는 만들지 않는다.

### 0.1 2026-08-21 구현 중간 기준선

이 계획을 작성한 뒤 실제 selective restore가 진행됐으므로, 이후 구현은 아래 기준선을 보존한다.

| 대상 | 현재 기준선 | 다음 종료 조건 |
|---|---|---|
| DimensionMaster S | 기존 15행을 보존하고 source sprite 2행만 복원해 17행 | runtime row 1/Release build PASS, 사용자 육안 판정 |
| DimensionMaster W | 기존 10행을 보존하고 세로 mesh carrier 4행만 복원해 14행 | runtime row 1/Release build PASS, 사용자 육안 판정 |
| DimensionMaster R | `fm_h_swing_05` 3행을 0.5/0.7/1.05초에 복원 | runtime row 1/Release build PASS, 세 검격 수동 판정 |
| DimensionMaster D | 현재 사용자 튜닝 중인 `fm_d_cone_007` 한 행을 그대로 보존 | runtime row 1, 후속 튜닝과 육안 판정 |
| DimensionMaster F | 두 cloud sprite에만 bounded profile 34 구현, 최종 Particle HLSL 재컴파일 PASS | runtime row 1/Release build PASS; parent noise와 육안 판정 미완료 |
| Artist E crane | exact model/animation/projectile cue를 저작 문서와 runtime model cue에 연결 | runtime row 1/Release build PASS, 사용자 육안 판정 |
| Artist E glow stroke | profile 35, source ribbon 3행, exact 색/alpha/dynamic curve와 붓 끝 anchor 연결. 사용자 저장 뒤 현재 문서는 7행+crane | 최종 4-field Publish 뒤 사용자 육안 판정 |
| DimensionMaster T | authored는 사용자 저장본 보존, model cue MASKED pass 3/TRANSLUCENT pass 4 유지 | focused isolation 및 현재 shader/TU compile PASS, 사용자 육안 판정 |
| LanceMaster V | MakeFlow/WaterTrail/ParticleMaster/SpriteWave 공용 admission 확장 | missing mesh 8 occurrence와 TrailGhost 2는 fail-close |
| Warlord T | 공용 family admission과 BA1 RGB-noise screenPost 1행 연결 | unresolved Light/film/zoom은 fail-close, 사용자 육안 판정 |

`완료`는 데이터 행이 존재한다는 뜻과 제품 화면이 맞다는 뜻을 섞지 않는다. 자동 검증과 사용자 전용
visual fidelity 판정은 대응 RESULT에서 분리한다.

## 1. 이전 DimensionMaster 4-family 실패의 원자 분석

### G00-1. texture를 final equation으로 오인

첫 실패는 material graph의 입력 texture를 찾은 뒤, family 이름과 filename으로 final pixel
equation을 추정한 것이다. texture identity는 필요조건이지만 다음을 증명하지 않는다.

- scalar/vector/static-switch의 실제 packing
- sampler state와 t/s register
- DynamicParameter lane 의미
- particle/mesh vertex factory와 interpolator
- render pass, depth/blend, MRT output

그 결과 DDS는 맞아도 UV, alpha, dissolve, emissive, distortion이 원본과 달랐다.

### G00-2. recovered DXBC와 product execution을 혼동

ShaderCache에서 Glasshole, Crackhole, FluidNinja, CustomParticle, SpriteWave 다섯 DXBC를 회수하고
WARP로 일부 RT 식을 확인한 작업 자체는 성공했다. 실패는 이것을 제품 복원 완료처럼 취급한
경계였다.

당시 product admission은 다음이 0/5였다.

```text
native scalar lane/padding closure
native sampler closure
actual vertex factory/pass identity
engine-owned CB row/resource supply
product MRT compatibility
occurrence selection
```

특히 Glasshole/Crackhole/FluidNinja는 Target0,2,3,4,5를 쓰는 UE3 BasePass shader다. 현재 Effect
pass는 color+distortion 2RT 계약이므로 blob을 직접 연결할 수 없다.

### G00-3. 공용 opcode를 만들고 consumer를 연결하지 않음

SpriteWave opcode 15 같은 family evaluator 일부는 HLSL/C++에 존재하지만, 이번 목표 authored
corpus에서 그 opcode를 선택하는 production packet이 없었다. converter가 목표 document에서
예상 occurrence를 찾지 못했는데도 “공용 ABI 구현” 자체를 진척으로 계산했다.

즉 실패의 단위는 shader 수식 하나가 아니라 다음 chain의 단절이다.

```text
source occurrence
-> material + effective static set
-> carrier/VF/pass
-> exact resources and parameter values
-> runtime packet selection
-> prepared shader/resource packet
-> actual draw
```

## 2. Raw DXBC 공용 확장 판정

### G01-1. 한 번 공용화할 수 있는 것

다음 descriptor/validation framework는 class-neutral하게 한 번 만들 수 있다.

- full variant key: base material + effective static set + platform + VF + pass + shader ID
- bytecode/hash/reflected signature
- arbitrary CB slot/register table
- sparse SRV/sampler register table
- engine-owned resource role
- `(cbSlot,row,lane,semantic)` draw patch table
- output target mask, blend/depth contract
- immutable prewarm packet과 batch rollback

### G01-2. variant마다 다시 닫아야 하는 것

다음은 공용 framework가 자동으로 알아낼 수 없다.

- native scalar group order/padding
- exact material uniform -> t/s mapping과 sampler state
- engine-owned CB/SRV 의미
- 실제 VF interpolator와 pass
- MRT format/slot/blend 호환
- occurrence별 DynamicParameter/particle input 의미

따라서 `RuntimeMaterialV2` 배열을 UE3 native CB라고 간주하는 만능 ABI는 만들지 않는다.

### G01-3. 이번 대상에 대한 결론

현재 보존된 exact DXBC 5종과 이번 목표 occurrence의 exact join은 0개다. 그러므로 Raw DXBC
descriptor 구현을 Whirlwind나 여섯 캐릭터 skill의 선행조건으로 두지 않는다.

- Target0-only exact variant가 실제 목표 MIC와 일치할 때 native descriptor canary를 연다.
- 6RT BasePass family는 full product pass가 닫히기 전까지 recovered RT0 equation을 현 Effect ABI의
  class-neutral HLSL로 번역한다.
- raw blob은 증거/oracle이고, family 이름만 같다는 이유로 다른 MIC에 재사용하지 않는다.

## 3. 공용 particle recipe 확장

이번 목표 문서군은 sourceRecipe 358개, module 4,038개를 가지며 class normalization 기준 기존
playback family가 4,038/4,038을 인식한다. 따라서 즉시 재사용성이 가장 큰 축은 particle recipe다.

### G02. recipe atom 계약

공용 atom은 다음을 보존한다.

```text
ParticleSystem / emitter / selected LOD identity
Required / Spawn / Burst
Size / SizeByLife
Color / ColorOverLife
Velocity / Acceleration / Orbit
Rotation / RotationRate
DynamicParameter
module reference order and duplicate occurrence
distribution lookup table, time scale, start time, random lock
```

TypeData는 명시적 carrier adapter로 나눈다.

- sprite
- mesh
- decal
- AnimationTrail
- Cascade Ribbon
- point light
- model/projectile cue

recipe는 material shader identity를 대신하지 않는다. recipe와 material profile을 별도 join한 뒤
하나의 prepared document로 commit한다.

## 4. 대상별 구현

### G03. DimensionMaster W/R/S/D/F/T

각 skillbinding의 exact clip sequence와 source event occurrence를 유지한다. 기존 4-family 이름을
skill-wide shader로 적용하지 않고 occurrence별 material parent/static set/carrier를 다시 join한다.
사용자가 튜닝 중인 transform/color/timing 값은 보존하고, 빠진 source-derived row만 최소 단위로
되살린다.

#### G03-1. S 2050220

S 검격은 standalone mesh가 아니라 sprite particle occurrence다. 현재 문서에는 기존 15행을 그대로
둔 채 다음 두 행만 source identity 그대로 추가했다.

```text
authored.source-particle.c3dbce9e43ebe73960d43c29
  carrier: sprite, source emitter 38
  material: fx_r_pa_spritewave_24_02_tr
  base/dissolve/emissive/noise:
    fx_m_trail_004_cl / fx_e_ring_039 / fx_d_atypical_028 / fx_m_noise_008

authored.source-particle.7b61af226536a9545acd0c89
  carrier: sprite, source emitter 45 symbol overlay
  material: fx_r_pa_spritewave_30_01_tr
  base/dissolve/noise:
    fx_m_trail_004_cl / fx_r_symbol_swp_01_cl / fx_d_noise_002
```

`fx_f_symbol_042.dds`는 S의 검격 근거가 아니며 현재 source join에서는 skill 2050550 쪽에만 나타난다.
따라서 파일명 유사성으로 S에 추가하지 않는다.

#### G03-2. W 2050120 clip 3

세로 검격은 `fm_m_helix_011.wmodel`을 carrier로 쓰는 네 mesh particle occurrence다. 기존 10행은
건드리지 않고 다음 네 stable row만 추가했다.

```text
authored.source-particle.0d483237146fa5d618d39a7f  start 0.25s
authored.source-particle.da2a381f72e08dbbe43e42f3  start 0.35s
authored.source-particle.0b38a1bdf89659a65e1ed571  start 0.45s
authored.source-particle.798b11b002a959023824d209  start 0.55s
```

네 행은 모두 child `fx_k_me_spritewave_01_45_ad`, parent `fx_m_pa_spritewave_01_ad`, base
`fx_m_spark_001`, dissolve `fx_h_noise_001`, noise `fx_a_noise_008_n`의 exact source packet을
유지한다. 같은 asset을 쓴다는 이유로 한 행으로 합치지 않는다.

#### G03-3. R 2050180

`pc_sp_m_00_sk_sk_foldcut`의 source cue를 한 mesh의 축 회전으로 합치지 않는다. 다음
`fm_h_swing_05.wmodel` 세 행을 0.5/0.7/1.05초에 각각 보존한다.

```text
authored.source-particle.cc4d20091ad0ed409617f51f
authored.source-particle.333341329ff3992ea8c7f0a1
authored.source-particle.f2e42e062ca87d93f0a477e3
```

세 행의 revolution은 0이다. 시작부 `fm_d_helix_015_1`은 source에 mesh rotation-rate가 있는 별도
carrier이므로 세 검격의 대체물로 쓰지 않는다. 사용자 저장 중 빠진 다른 row를 함께
복원하지 않는다.

#### G03-4. D 2050240 clip 2

현재 검격 carrier `authored.source-particle.feb86706b66f889097dcf610`은 이미 존재한다.

```text
mesh: Effect/DimensionMaster/Meshes/fm_d_cone_007.wmodel
material: fx_o_me_master_01_04_ds_ad
parent: fx_d_pa_master_01_ad
base: fx_a_atypical_048.dds
noise: fx_i_shockwave_02_ycl.dds
```

이 행의 transform은 사용자가 같은 worktree에서 계속 튜닝 중인 값이다. 이번 selective restore는
그 값을 덮어쓰지 않는다. noise는 UV 왜곡의 중요한 입력이지만, base/mask/dissolve/alpha equation과
carrier가 함께 닫혀야 하므로 모든 왜곡을 noise 하나로 환원하지 않는다.

#### G03-5. F 2050230 사각 card

cloud 두 행은 sprite carrier이며 child `fx_w_pa_fd_01_3_tr`, parent `fx_mm_fluid_01_tr`를 쓴다.
base `fx_d_cloud_035.dds`와 emissive `fx_o_glass_01.dds`는 불투명 alpha를 가지므로 generic grouped
경로에서 texture alpha를 coverage로 쓰면 quad 경계가 그대로 보인다. source parent가 소유한
transition/noise/opacity 식을 실행하지 않은 것이 핵심 원인이다.

이번 단계는 두 exact child/resource 조합에만 profile 34를 배정한다. profile 34는 source dynamic
parameter의 transition, alpha power, fresnel alpha를 읽고 transition luminance와 radial envelope로
coverage를 제한한다. 이는 native `fx_mm_fluid_01` graph의 exact 복원이 아니라 사각 card를 막기 위한
bounded reconstruction이다. parent noise closure와 native pass ABI는 계속 미완료다.

별도 ground card `authored.source-particle.46bdb5dbec23f48e6c3f1a87`는
`fx_s_atypical_006.dds`와 glasshole child를 쓴다. source의 crack normal과 radial graph가 완전히
닫히지 않았으므로 cloud용 profile 34의 완료 범위에 포함하지 않는다.

#### G03-6. T 2050500

authored clip sequence 전체의 source recipe와 material profile cardinality를 재검증하되 사용자가 저장한
현재 문서는 읽기 전용으로 보존한다. summon model cue는 기존 `MASKED` 계약과 animated model exact
pass 3의 alpha cutoff를 사용하고, translucent cue는 pass 4를 사용한다. 일반 particle/decal과 model cue를
한 occurrence에서 중복 submit하는 별도 경로는 추가하지 않는다. 제품 draw fidelity는 사용자 gate다.

### G04. LanceMaster V / Warlord T

- LanceMaster V 34610: MakeFlow/WaterTrail/Ring 계열을 class-neutral parent/static variant로 연결
- Warlord T 17240: Full Barrel 3 BA의 mesh/sprite/decal carrier와 Simple/Particle/MakeFlow profile을
  exact occurrence별로 연결

class 전용 monolithic shader를 만들지 않는다. 기존 AnimationTrail carrier가 있는 occurrence는
Valtan canary와 같은 typed packet builder를 사용한다.

공용 resolver/HLSL은 Simple01/02, MakeFlow02/03 mesh/sprite, WaterTrail과 기존 ParticleMaster/
SpriteWave를 class-neutral parent/static/carrier 계약으로 확장했다. LanceMaster V의 missing physical mesh
8 occurrence와 TrailGhost 2개는 근거가 닫히지 않아 승격하지 않았다. Warlord T는 BA1 RGB-noise
screenPost 한 행만 실제 consumer로 연결했고, decoded 값이 없는 Light/film/zoom은 fail-close했다.
문서가 존재하거나 일부 family가 입장했다는 사실을 전체 스킬 visual 완료로 기록하지 않는다.

### G05. Artist E와 두루미

Artist E 31480의 source casting streak는 bone-history AnimationTrail이나 단일 sprite가 아니라
TypeDataRibbon emitter 1/11/17 세 행이다. 이를 AnimationTrail 또는 sprite로 바꾸지 않고 기존
CascadeRibbon carrier가 보존할 수 있는 source recipe/material/resource 범위만 bounded하게 복원한다.

두루미는 shader texture가 아니라 projectile/model carrier다.

```text
skillId: 31480
projectile IDs: 314800 / 314840 family
source mesh: SK_SDM_RCC_00.Mesh.SK_SDM_RCC_00_SK_FX_01
```

원본 skeletal/static mesh 성격, skeleton/animation requirement, material slots를 조사한 뒤 기존
`CModel -> CMaterial` cook/runtime과 `modelCues` 또는 typed projectile presentation으로 연결한다.
별도 legacy model runtime을 만들지 않는다. model cook 또는 material slot이 닫히지 않으면 흰
placeholder로 그리지 않고 해당 crane carrier만 fail-close한다.

#### G05-1. 두루미 구현 기준선

두루미는 다음 exact evidence로 현재 runtime model cue까지 연결됐다.

```text
source package SHA-256:
  504c8594e528e7c687be8b9e8a2f2cecd042df84f3ae60789fbaa8329f68da86
runtime WModel SHA-256:
  cb96358dddf1bf749da1bc555973c8977ceaf21cf61a2bb4cd06ae5ba3ea6936
runtime asset:
  Effect/Artist/Meshes/SK_SDM_RCC_00_SK_FX_01/SK_SDM_RCC_00_SK_FX_01.wmodel
runtime clip: rcc_sk_flyinheaven
spawn/duration/velocity: 1.0s / 1.11111116s / local +Z 9m/s
```

model cue는 translucent alpha, source material의 opacity/color reconstruction, 마지막 frame hold를
사용한다. source mesh/animation/projectile timing identity는 exact지만 material 식과 최종 visual
fidelity는 exact라고 부르지 않는다.

#### G05-2. glow stroke 구현 기준선

원본 SHA가 일치하는 `YINYANGSHI.loa`에서 stage 0 `SK_FlyinHeaven` cue는 0.3006150126초에 시작하고
0.899999976초 지속하며 `B_Body_03`를 따른다. visible carrier는 TypeDataRibbon emitter 1/11/17 세
행이다. source-disabled `_01_02/_01_03` cue는 복원하지 않는다.

```text
emitter 1/11:
  child fx_k_flowrib_01_03_tr
  parent fx_k_flowrib_01_tr
  resources fx_i_atypical_03_ycl / fx_l_environment_001 / fx_d_noise_002

emitter 17:
  child fx_k_flowrib_01_01_tr
  parent fx_k_flowrib_01_tr
  resources fx_k_auraline_02 / fx_d_noise_002
```

profile 35 `FLOWRIBBON01`은 원본 color/alpha/width와 `x_tiling/y_tiling/dissolve/disort` curve를 기존
DynamicTrail carrier에 연결한다. 붓 anchor는 원본 weapon PSK의 `b_root -> b_body_01 -> 02 -> 03`
누적 46.447189331cm를 runtime `b_wp_1 + [0.4644718933105469, 0, 0]`으로 합성한다. native UE3
VF/pass를 직접 실행하는 경로는 아니므로 bounded reconstruction 경계는 유지한다. 사용자 저장 뒤 현재
문서는 7 element + crane cue이고 아래 세 stable row는 각각 정확히 한 번 보존됐다.

```text
authored.source-particle.8467a9b69e22208c6d4ea99e
authored.source-particle.a006dbe9b5650a0b13880aa5
authored.source-particle.c6d4247396f641316d28767c
```

현재 authored JSON의 stable ID/cardinality, source material/resource/module, float32 anchor와 crane cue는
PASS다. 04:09 runtime row는 이전 저장본이므로 현재 7행 문서의 제품 반영 증거로 재사용하지 않는다.
최종 4-field full Publish와 visual fidelity는 별도 통합/사용자 gate다.

## 5. 변경 파일과 소유 경계

### 5.1 현재 구현에 직접 연결된 파일

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050120.clip3.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050180.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050240.clip2.unified.effect.json
Data/Effects/Authored/effect.artist.skill.31480.unified.effect.json
Data/Effects/Imported/Artist/CurrentCombat/skill.31480.crane-model-cook.receipt.json
Data/Effects/Imported/Artist/CurrentCombat/skill.31480.flowribbon-brush-tip-anchor.receipt.json
Client/Public/Effect_AuthoringDocument.h
Client/Public/Effect_MaterialTemplate.h
Client/Private/Effect_DocumentCodec.cpp
Client/Private/Effect_DocumentRenderer.cpp
Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli
Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl
```

공유 worktree의 위 파일에는 다른 캐릭터 튜닝 변경도 함께 있을 수 있다. 이 계획의
완료 단위는 파일 전체가 아니라 위에서 열거한 exact row, model cue, profile 34 분기다.

### 5.2 후속 대상

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050500*.effect.json
Data/Effects/Authored/effect.lancemaster.skill.34610*.effect.json
Data/Effects/Authored/effect.warlord.skill.17240*.effect.json
Data/Effects/EffectCatalog.json
Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json
Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json
```

새 C++ file은 추가하지 않았다. 따라서 이 변경으로 새 `.vcxproj`/`.vcxproj.filters` 등록도 필요하지
않다. 미래용 `NativeDxbcManager`나 캐릭터별 두 번째 renderer를 만들지 않고 기존
`CModel -> CMaterial`, Effect document renderer, VisualProgram 경로를 확장한다.

## 6. 검증

새 harness 없이 다음을 실행한다.

1. target `(class, slot, skillId)`와 skillbinding exact clip cardinality 확인
2. authored JSON/schema와 source occurrence/material/resource identity 검사
3. Effect publisher Validate/Publish
4. VisualProgram corpus/runtime check
5. resource path/hash와 model header/material slot 검사
6. Client x64 Debug/Release build
7. `git diff --check`
8. final 4-field Full Publish와 Client 재시작 뒤 사용자가 실제 스킬을 직접 육안 판정

자동 검증은 draw 준비와 identity를 증명하지만 원본과 같은 visual fidelity를 대신 판정하지 않는다.

현재까지 실행된 명령과 미실행 항목은 대응 RESULT에만 기록한다. 공통 material HLSL 10개 조합과
Catalog/Renderer/MainApp 선택 TU는 현재 리비전에서 오류 0으로 통과했다. 04:09 full publish는 과거
6-field 전환 게시본이며 현재 authored 저장본의 최종 제품 증거가 아니다. direct catalog는 이제 4필드가
정본이고 legacy 6필드는 전환 읽기만 허용하며, VisualProgram sidecar는 marker와 무관하게 항상 로드한다.
모든 authored 저장이 자연스럽게 끝난 뒤 별도 통합 단계에서 full Publish를 한 번 실행한다. Product draw와
화면 fidelity는 사용자의 서면 관찰 전에는 PASS로 쓰지 않는다.
