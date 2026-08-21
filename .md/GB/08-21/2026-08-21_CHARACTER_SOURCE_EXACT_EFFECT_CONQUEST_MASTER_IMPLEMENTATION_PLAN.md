# 2026-08-21 Character Source-Exact Effect Conquest Master 구현 계획

기준일: 2026-08-21
코드·데이터 기준선: `812d23a4` (`origin/main` pull 반영)
구현 branch/worktree: `codex/effect-family-conquest` / `C:/Users/user/Desktop/LostArk-effect-family-conquest`
문서 성격: 계속 갱신하는 구현 계획서
최종 화면 판정자: 사용자

이 문서는 이미 끝난 작업의 RESULT를 다시 쓰지 않는다. 도화가 F에서 증명한 source/DXBC 기반
복원 공정을 도화가, 워로드, 창술사, 차원술사의 남은 스킬과 공통 material family에 확장하는 다음
구현 정본이다. 실제 구현 상태와 실행한 검증은 같은 이름의 RESULT에서만 완료로 올린다.

연결 문서:

- `2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_RESULT.md`
- `2026-08-17_EFFECT_DIMENSIONMASTER_SHADERCACHE_JOIN_RESULT.md`
- `2026-08-21_CHARACTER_SOURCE_EXACT_EFFECT_AND_CRANE_RESTORATION_IMPLEMENTATION_PLAN.md`
- `2026-08-21_CHARACTER_SOURCE_EXACT_EFFECT_AND_CRANE_RESTORATION_RESULT.md`
- `2026-08-21_DIMENSIONMASTER_BA_COMMAND_BUFFER_IMPLEMENTATION_PLAN.md`
- `2026-08-21_DIMENSIONMASTER_T_SUMMON_AND_WARLORD_FULL_BARREL_FAMILY_IMPLEMENTATION_PLAN.md`
- `2026-08-21_FOUR_CLASS_SKILL_CUE_AND_BA_REGRESSION_IMPLEMENTATION_RESULT.md`

범위는 본 문서 2장의 도화가·워로드·창술사·차원술사 확정 결함과, 사용자가 후속으로
추가한 family-first 렌더링·action-facing anchor·glass/dragon/attractor/screen presentation으로
고정한다. Valtan 데이터·cue·candidate, merge 안전성, 일반 Effect Tool 회귀 조사는 이 세션의
구현 목표나 완료 조건이 아니다. 다만 이 계획의 실제 저장·publish·runtime admission을 닫는 데 필요한
도메인 검증만 수행한다.

## 0. 목표

목표는 현재 범용 translucent renderer로 비슷한 색을 내는 것이 아니다. 원본 action cue, Cascade
occurrence, carrier, material/MIC static set, DDS channel, scalar/vector/dynamic parameter, sampler,
vertex factory, pass, blend, post process를 가능한 끝까지 연결해 제품 스킬을 복원한다.

범용 renderer가 원본 의미를 보존할 수 있으면 그대로 사용한다. 의미가 손실되면 다음 순서로 실제
기능을 뚫는다.

```text
기존 typed family가 exact key를 수용
-> class-neutral family variant 확장
-> 공용 carrier/presentation primitive 추가
-> 정말 한 occurrence에만 존재하면 전용 evaluator/HLSL 추가
-> native ABI가 모두 닫힌 경우에만 raw DXBC 직접 실행 검토
```

전용 shader를 추가하는 비용은 복원 실패의 이유가 아니다. 다만 `skillId` 하나를 검사하는 거대한
class shader를 만드는 대신 다음 variant key로 같은 원본 family를 사용하는 다른 스킬이 재사용한다.

```text
base parent material
+ effective child/static parameter set
+ carrier and source vertex-factory role
+ pass/shader type
+ recovered pixel-shader identity
```

### 0.1 복원 완료의 의미

한 스킬은 다음 곱이 모두 닫혀야 완료다.

```text
source occurrence identity
× carrier/simulation/attachment
× material equation and resources
× render state and scene composition
× animation/product/runtime join
× automatic contract/build verification
× user visual approval
```

DDS가 존재하거나 authored row가 생성됐다는 이유만으로 완료하지 않는다. HLSL이 컴파일됐거나
Client가 링크됐다는 사실도 원본과 같은 화면을 증명하지 않는다.

### 0.2 복원 상태의 직교 축

source 근거, 실행기, 제품 admission과 사용자 승인을 하나의 `fidelity` 값으로 합치지 않는다. 모든
occurrence는 다음 축을 따로 기록한다.

| 축 | 허용 값 | 의미 |
|---|---|---|
| `provenance` | `SOURCE_EXACT`, `DONOR_TRANSPLANT`, `PROJECT_TUNED` | 형태·타이밍·리소스 결정의 소유 근거 |
| `evidence` | `SEALED`, `PARTIAL`, `MISSING` | source object, shader, resource, pass 증거의 폐쇄 정도 |
| `runtimeExecutor` | `NATIVE_DXBC`, `TYPED_HLSL_SEMANTIC_REPLAY`, `TYPED_SOURCE_RECONSTRUCTION`, `TYPED_PRESENTATION`, `NONE` | 실제 Product가 실행하는 코드 경로 |
| `runtimeAdmission` | `ADMITTED`, `AUTHORING_ONLY`, `FAIL_CLOSED` | 현재 제품 draw 허용 여부 |
| `userReview` | `PENDING`, `APPROVED`, `REJECTED` | 사용자의 최종 화면 판정 |

`FAIL_CLOSED`는 fidelity가 아니라 해당 occurrence 하나의 실패 처리다. optional row 하나가 실패해도
검증된 sibling은 유지한다. `exact`라는 단어도 raw DXBC bytes의 존재만 뜻하지 않는다. shader,
resource, binding, carrier, pass, output 중 무엇이 exact인지 따로 기록한다.

### 0.3 사용자 전용 화면 판정

에이전트는 Client/UI를 자율 실행하거나 화면을 캡처하지 않는다. 자동화는 source identity, packet,
shader output, resource residency, build와 runtime 준비까지 닫는다. 실제 스킬의 형태, 색, 타이밍,
밀도, 궤적, 화면 피로도와 도화가 F 수준의 최종 품질은 사용자가 직접 실행하고 서면으로 승인한다.

### 0.4 family-first 제품 구조

한 occurrence 자체를 family라고 부르지 않는다. Product execution closure는 다음 네 variant의 곱이다.

```text
CarrierVariant
× MaterialVariant (presentation-only carrier에서는 null 가능)
× RenderVariant
× CompositionVariant
```

| family 층 | 소유하는 의미 |
|---|---|
| Carrier | mesh/sprite/decal/trail/ribbon/light/post/model topology, spawn simulation, local/world space, attachment |
| Material | parent/child/static set, RGBA named lane, channel swizzle, color space, sampler, scalar/vector/dynamic semantic, 식 |
| Render | vertex factory, shader/pass, blend/depth/raster/stencil, SceneColor/Depth, RT/MRT, HDR/tone-map 위치 |
| Composition | occurrence cohort와 순서, timing owner, anchor/target, position follow/snapshot, orientation authority, stop/cancel, Product join |

Light, ScreenPost, CameraImpulse는 `materialVariantId=null`과 빈 texture lane이 합법이다. LocalDecal,
DecalParticle, AnimationTrail, CascadeRibbon, screen overlay는 material과 render closure가 모두 필요하다.
같은 parent material이라도 child/static set, carrier, named lane, pass 중 하나가 다르면 별도 variant다.

실제 admission key는 네 variant ID를 canonical JSON으로 직렬화한 SHA-256이다. 기존
`effect-family-manifest.v1.json`의 parent-material `familyId`는 후보 corpus 그룹일 뿐 Product admission
key가 아니다. family 하나가 열렸다는 이유로 consumer 전부를 자동 승인하지 않고 exact tuple이 같은
occurrence allowlist만 확대한다.

Carrier taxonomy는 최소한 다음을 구분한다.

```text
standalone mesh / legacy sprite
mesh particle / sprite particle / source decal particle
authored history trail / AnimationTrail / CascadeRibbon / spline segmented ribbon
static LocalDecal / animated ModelCue
PointLight / ScreenPost / ScreenOverlay / CameraImpulse
```

원본 grayscale DDS도 이 구조에서는 단순 `assetId`가 아니다. Material lane은 `role`, `sourceChannel`,
`textureRegister`, `samplerRegister`, `colorSpace`, filter/address/mip와 combine equation을 함께 봉인한다.
같은 회색 DDS라도 mask R, alpha coverage, emissive luminance, flow RG처럼 읽는 방식이 다르면 다른
MaterialVariant다. `coveragePolicy`, `emissiveSourcePolicy`, `missingLanePolicy`도 variant hash에 포함한다.
DXT1/no-alpha carrier를 무조건 `base.a`로 자르거나, 없는 mask/emissive에 기본 SRV를 끼우거나,
emissive texture가 없는데 intensity만 올리는 implicit fallback은 금지한다. 명시적 constant/luminance 식이
source로 증명되지 않으면 해당 occurrence 하나를 fail-close한다.

## 1. 도화가 F가 남긴 구현 정본

도화가 F `31470 필법 : 한획긋기`는 이 계획의 golden control이다. 현재 제품 정본은 legacy
`effect.artist.skill.31470` reconstructed program이 아니라 clip cue가 직접 가리키는
`effect.artist.skill.31470.unified` direct-authored asset이다.

현재 golden 문서는 다음 계약을 가진다.

```text
17 elements
  particle 14
  decal 2
  trail 1

material execution
  runtimeMaterialV2 8
  artistVisualV4 7
  localDecal 2

portable source recipe
  14 elements
  source modules 165
```

현재 legacy evidence registry의 35행 분모는 다음과 같다.

```text
DXBC semantic replay 7
bounded explicit 22
unresolved fail-closed 4
non-core forbidden 2
draw admitted 29
native selection admitted 0
```

과거 RESULT의 33-core 또는 draw 27/28 수치는 현재 user-tuned 17-element direct Product의 정본
분모가 아니다. 구현 중에는 현재 문서의 stable ID, kind, resource, backend, opcode와 content-addressed
runtime row를 golden fixture로 고정한다.

### 1.1 F에서 재사용할 방법론

F의 성공은 원본 pixel DXBC를 제품 pass에 무조건 직접 연결한 결과가 아니다.

```text
official RefShaderCache와 MIC/static set join
-> packed original DXBC 추출 및 hash 봉인
-> signature-compatible carrier로 D3D11 WARP 재생
-> CB register, texture register/channel, output 식 backward slice
-> CPU closed-form oracle와 WARP output 수치 대조
-> occurrence별 typed evaluator/HLSL로 식 번역
-> strict registry가 occurrence/resource/backend/opcode/pass를 승인
-> portable particle recipe와 direct-authored Product에 연결
```

재사용할 현재 구현과 증거는 다음이다.

- `Client/Public/Effect_Artist31470ShaderRegistry.h`
- `Client/Private/Effect_Artist31470ShaderRegistry.cpp`
- `Client/Bin/ShaderFiles/Shader_Artist31470RuntimeMaterial.hlsli`
- `Client/Bin/ShaderFiles/Shader_Artist31470Active003RibbonMaterial.hlsli`
- `Client/Bin/ShaderFiles/Shader_Artist31470Active011OuterMaterial.hlsli`
- `Client/Bin/ShaderFiles/Shader_Artist31470Active022DecalMaterial.hlsli`
- `Client/Bin/ShaderFiles/Shader_Artist31470Diagnostic.hlsli`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`
- `Client/Bin/ShaderFiles/Shader_EffectRuntimeMaterialPacket.hlsli`
- `Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli`
- `Tools/LevelPlacementExtractor/replay_artist_31470_main_original_dxbc.py`
- `Tools/EffectPipeline/replay_ue3_material_pixel_shaders.py`
- `Tools/EffectPipeline/evaluate_ue3_material_uniform_expressions.py`
- `Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py`
- `Tools/EffectPipeline/materialize_ue3_exact_cooked_shader_variants.py`
- `Data/Effects/Imported/Artist/Materials/skill.31470.main-original-dxbc-replay.receipt.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.main-ref-shader-cache*.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.main-runtime-source-replay*.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority*.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding*.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-runtime-deployment*.json`

F 전용 registry와 frozen reconstructed manager를 다른 스킬에 복사하지 않는다. 새로운 공통 family는
class-neutral variant registry로 만들고, F 자체를 이 registry로 옮기는 리팩터링은 golden parity가
먼저 생긴 뒤 별도 변경으로만 수행한다.

현재 기준선에서 Product sealed path의 content-address prefix `307d1ce0...`, raw authoring file byte SHA
`b503dc4f...`, canonical parsed-document SHA `b1cc0de1...`는 서로 다른 hash domain이다. byte equality를
요구하지 않고 각 domain의 identity를 따로 고정한다.
legacy `effect.artist.skill.31470` reconstructed runtime은 non-Product이므로 새 family executor의 기반으로
복사하지 않는다.

### 1.2 이미 확보된 차원술사 cooked shader oracle

다음 다섯 original pixel DXBC는 content-addressed blob과 contract가 있다.

| family | SHA-256 |
|---|---|
| Glasshole | `e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b` |
| Crackhole | `a04dc3d21a95d5f2299d6bc8d07f65bd7cde640be52ed0df362b4a71b0c680e6` |
| FluidNinja | `9c4706c3b6a36cb5a5eac40d8e871f2b67351265cf211f624614c842f0334b8e` |
| CustomParticle | `340169d8b9146fd280bbc1b509aa312e6afa71573dfe12cf4e89cb0136431ae3` |
| SpriteWave | `7917664ff360df0f6305bf8d6c41a8c506872a62e09d26beb2b8fd8d7164d76c` |

이 다섯 variant는 현재 source-value uniform과 texture binding evidence까지 열렸지만 source-exact sampler,
scalar-group padding, actual VF/pass와 Product admission은 닫히지 않았다. 기존 authoring canary를 제품
exact라고 승격하지 않고, 이번 plan의 material-family G에서 각 blocker를 명시적으로 닫거나 현재
engine ABI로 semantic replay한다.

## 2. 요청 스킬 전체 범위와 현재 문제

### 2.1 도화가

| slot | skill | 현재 실측 | 복원 목표 |
|---|---:|---|---|
| A | `31460 묵법 : 호접몽` | 18행. 검격 8행은 `fx_o_pa_spritewave_01_27_tr`, noise `fx_a_cloud_022`. empty resource override는 codec이 거부하고 Clear는 source reset | 검격 8행에 `uv_noise_velue=0` scalar override만 적용해 exact SpriteWave UV warp/distortion 기여를 0으로 만든다. `PROJECT_TUNED`; base/dissolve/sourceRecipe 불변 |
| S | `31420 묵법 : 난치기` | imported 32 particle+4 LineBlur, current Product 1 particle. LineBlur는 disabled/unresolved이며 RGB/Zoom/Film과 다른 family | `fx_o_grass_03/04` shape/dissolve와 emissive tip/fade를 `PROJECT_TUNED`로 구현. PointLight/MotionBlur는 source executor가 닫힌 뒤 별도 admission |
| R | `31210 필법 : 콩콩이` | BA1 4 particle, BA4 68 particle. `fx_o_symbol_14`가 BA1 particle의 base/noise에 저장됐고 decal 0 | 잘못된 override 원복, source-proven 타격 stage에 `PROJECT_TUNED` true LocalDecal 추가, 기존 self revolution으로 중심 회전과 scale/alpha dissolve |
| D | `31490 묵법 : 범가르기` | authored 56행은 존재하지만 animevent EFFECTREF와 catalog/runtime row가 없음. projectile JSON은 debug hit trajectory일 뿐 visual tiger 증거가 아님 | join 누락 복구 후 별도 호랑이 model/material/animation lineage를 닫고 carrier와 비행 궤적 연결 |
| E | `31480 묵법 : 두루미나래` | 7행+modelCue, crane WModel/`rcc_sk_flyinheaven`, bone update와 typed FlowRibbon 3개가 이미 존재 | animated ModelCue와 FlowRibbon 두 carrier family의 canary. selected runtime, clip time, bone matrices, local velocity를 진단하고 evidence가 틀릴 때만 recook |
| F | `31470 필법 : 한획긋기` | 사용자 승인 golden 17행 direct Product | 변경 금지 golden regression. 공통 renderer/HLSL 변경마다 자동 identity와 사용자 회귀 확인 |
| T | `31950 묵법 : 미르 새김` | imported 22P+1D+1Light+1Post, current 22P+1D. helix 3개는 visible flag만 true이고 execution은 fail-closed. TypeDataRibbon 하나가 sprite fallback-blocked | 먼저 source TypeDataRibbon→typed CascadeRibbon projection/admission과 Animation/CModel owner를 진단. 그 뒤 exact model 또는 별도 spline carrier로 분기 |
| V | `31910 절기 : 몽유도원` | imported 68P+7D+3Light, current 38P+3D. source Light도 disabled/zero. distortion MRT, light, camera SHAKE는 서로 다른 층 | 노란 도깨비불의 회전·중앙 집결, source timing의 distortion material, typed presentation light, animation-cue camera envelope를 각각 복원 |

도화가 원작 역할 참고 화면은
`.md/GB/08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/2026-08-16_ARTIST_VISUAL_EVIDENCE_MANIFEST.md`
를 사용한다. 이 이미지는 source occurrence를 좁히는 기준이지 자동 visual PASS가 아니다.

### 2.2 워로드

| slot | skill | 현재 실측 | 복원 목표 |
|---|---:|---|---|
| W | `17060 파이어 불릿` | root/follow cue, `fm_a_hemisphere_012` mesh particle 한 occurrence, start `0.3284s`, source burst 1. `maxParticles=3`은 capacity이지 방패 세 개의 spawn count가 아니다. owner live root가 곱해져도 presentation yaw smoothing 때문에 action edge 방향과 어긋날 수 있음 | 공용 `ACTION_FACING` orientation으로 Server-approved cast yaw를 고정하고 root/source basis/geometry pre-rotation을 exactly-once 검증. WModel 자체가 세 판 composite인지 먼저 증명하고, 아니면 좌/중/우 stable cohort와 element-level local +Z 이동을 `PROJECT_TUNED`로 추가 |
| A | `17090 갈고리 사슬` | chain06 8 + chain07 4 + helix 2. ordinary authored save에도 exact 12 cardinality를 강제해 삭제 후 atomic roundtrip이 실패 | immutable source evidence의 12행은 보존 검증하되 Product authored는 합법 source subset 저장 허용. 화면의 네 방향 완성 사슬을 기준으로 body/tip composite 구성 |
| T Start | `17240 풀배럴 캐넌` | BA1의 `source-event-007` RGBNoise screenPost가 화면 noise의 실제 소유자 | 이 한 row를 제거/비활성화하고 다른 ScreenPost 기능은 유지 |
| T Release | `17240 clip07/ba3` | 현재 8 particle, decal 0. 손튜닝 pruning에서 exact decal 4개도 같이 제거 | 현재 8행을 보존하고 source emitter 55~58 exact decal 네 장만 선택 복원 |

풀배럴 네 decal의 base는 추측한 cloud texture가 아니라 `fx_k_turtlediff_02.dds`다. noise/mask/emissive/
dissolve lane과 start `0.12s`, life `2s`, size `4.5`, depth `0.25`를 receipt에서 함께 복원한다.

### 2.3 창술사

| slot | skill | 현재 실측 | 복원 목표 |
|---|---:|---|---|
| Q | `34040 이연격` | clip1 mesh 1, clip2 mesh 4가 이미 존재 | BA mesh를 먼저 복사하지 않고 기존 mesh의 admission/material/revision/transform 실패를 고친다. 두 타격에 source-proven slash가 실제 draw된 뒤 부족한 carrier만 추가 |
| A | `34140 선풍참혼` | 2-stage COMBO. 한 대상 row disk rotation은 이미 `[0,-90,0]` | stable element/clip, Apply, atomic save, next-spawn hot reload를 진단하고 transform 합성 순서를 고친다 |
| W donor | `34550 사두룡격` | `fm_d_cone_005` cone occurrence 6개 | E 복사 원본으로 쓰되 cross-document pointer가 아니라 exact resource/material/source row를 새 stable ID로 transplant |
| E | `34560 굉열파` | clip2/clip3 Product, clip3에 기존 trail mesh | source hit 시점에 W cone을 연결. 기본 admission은 impact cone 1개이며 원본 비교가 연속 cone을 요구하면 occurrence cohort로 확대 |
| 용 cohort | `34610 V`, `34630 ALT_V`, `34650 T` | 사용자가 부른 Z는 실제 stance swap. 용은 세 스킬의 mesh particle에 존재 | 세 스킬을 모두 source cohort로 조사해 자연스러운 UV가 깨지는 정확한 대상과 material variant를 연결 |

창술 용 resource는 두 계열이다.

```text
V 34610
  fm_n_flm_ydr_00_sm.wmodel
  fx_k_me_flamesurface_01_01_ma -> fx_j_me_flamesurface_01_ma

ALT_V 34630 / T 34650
  fm_x_flm_gdr_01.wmodel
  fm_x_flm_gdr_01_dragon.wmodel
  fx_t_me_master_01_ph_01_msk / _ph_02_msk
  parent fx_d_me_master_01_ph_msk
```

후자의 source는 base/diffuse, normal, emissive, dissolve/noise, distortion에 서로 다른 panning/scale을
가진다. 현재 하나의 Detail UV speed로 합치면 자연스러운 용 비늘 흐름이 나올 수 없다.

### 2.4 차원술사

| slot | skill | 현재 실측 | 복원 목표 |
|---|---:|---|---|
| LMB | `2050010 기본 공격` | current cue는 BA1/2/4=`ba1.unified`, BA3=`ba3.unified`; 실제 mouse edge당 command 하나 | 구버전 혼용을 먼저 배제하고, source combo window 안의 재클릭만 다음 단계로 buffer. hold 자동 진행 금지 |
| A | `2050210 분광` | source MakeFlow slash는 `.25/.60/.90/1.30s` 네 occurrence인데 current Product에 `fm_h_swing_05` 한 행만 남음. 0ms presentation snapshot은 새 authoritative yaw를 부드럽게 추종 중인 root를 동결 | outer 위치 anchor는 follow, 방향은 Server-approved `ACTION_FACING`을 actionStartTick에 snapshot. 네 source local pose를 selective merge하고 inner occurrence snapshot/독립 transform 유지; gameplay damage는 단일 hit 유지 |
| T | `2050500 업의 경계` | T 누름 즉시 skill command. Server는 direction/distance만 소유하고 승인 target XZ를 snapshot에 보내지 않음 | T targeting mode, 11m range/nav 검증, LMB confirm/RMB cancel, Server-approved target XZ, target-root damage/effect를 한 vertical slice로 구현 |
| F | `2050230 시간 분쇄` | raw source 69행(64 particle+2 light+3 post), current 8행(2 mesh+6 sprite), typed execution 0 | 창 타격, 반구 확장, world shard, RGB/zoom post와 화면 유리 파편을 source role별로 복원 |
| W | `2050120 분절` | Glasshole/Crackhole/LocalCrack/SpriteWave가 반복되고 exact cooked shader evidence가 가장 많이 확보됨 | F에서 여는 glass/crack family를 W의 exact occurrence cohort에 즉시 rollout |

차원 LMB window는 고정 추측 `0.3s` 대신 source evidence를 우선한다. 현재 full-stage window와 과거
source window가 다르므로 G09에서 `BA1 100~510ms`, `BA2 0~410ms`, `BA3 200~1067ms`를 자동 fixture로
만들고 사용자 조작감 승인 뒤 balance 정본을 확정한다. A의 네 slash는 presentation occurrence이며
`PlayerSkills`의 단일 `hitTimeMs=100`과 DamageProfile을 multi-hit로 바꾸지 않는다.

## 3. 공통 capability와 소비자 행렬

### CAP-01. Source shader variant와 DXBC oracle

소유 책임:

```text
source occurrence
-> effective MIC/static set
-> source cache map and shader identity
-> uniform/texture/sampler evidence
-> carrier/VF/pass/output contract
-> runtime backend/opcode/pass
```

F 전용 registry를 복제하지 않고 새 class-neutral source material variant registry가 이 key를 소유한다.
registry는 skill ID로 shader를 고르지 않으며 동일 variant를 사용하는 모든 occurrence를 explicit consumer
목록으로 검증한다.

Raw DXBC 직접 실행은 다음이 전부 닫힌 target에만 허용한다.

```text
input signature and actual vertex factory
constant-buffer row/lane/padding
texture register and source DDS
sampler/filter/address/color space
render target count/format/output role
blend/depth/rasterizer/pass
engine-owned SceneColor/SceneDepth/fog inputs
```

하나라도 열려 있으면 raw blob은 oracle로 유지하고, WARP output과 같은 식을 현재 effect HLSL에
번역한다. 이때 negative control로 사용되지 않는 channel, scalar, dynamic parameter가 실제 결과를
바꾸지 않는 것도 검증한다.

### CAP-02. Glass, crack, refraction world material

이 capability는 하나의 “유리 셰이더”가 아니라 variant 묶음이다.

현재 Product 반복 집계:

| family | Product 소비자 | 현재 행 |
|---|---|---:|
| `fx_j_pa_glasshole_02_tr` | 차원 W `2050120`, A `2050210`, D `2050240` | 3 |
| `fx_mm_fluid_01_tr` | 차원 F 2, 도화가 V 1, 창술 ALT_V 1, 워로드 `17250` 2 | 6 |
| LocalCrack | 차원 LMB/Q/W/R/A/S/F/D/T/후속 corpus | 다수 |
| Crackhole/CrackholeV2 | 차원 W와 후속 corpus | 다수 |

차원 source corpus에서 확인한 최소 family 분모는 다음과 같다.

```text
Glasshole: 2050100, 2050120, 2050210, 2050230, 2050240, 2050510, 2050550
LocalCrack: 2050010, 2050100, 2050110, 2050120, 2050150, 2050180,
            2050190, 2050200, 2050210, 2050220, 2050230, 2050500,
            2050510, 2050540, 2050550
Crackspace/WindowCrack/IceCrack: 2050190, 2050510, 2050550
```

모든 corpus skill을 즉시 Product roster에 승격하지 않는다. F와 W에서 family variant를 닫은 뒤 현재
playable skill occurrence를 우선 rollout하고, 나머지는 source inventory의 verified consumer로 유지한다.

차원 F의 현 profile 34는 exact 두 sprite만 admit하는 bounded square-card 방지식이다. 이것을 native
`Fluid01` 복원으로 간주하지 않는다. parent noise, sampler/register/pass를 닫고 carrier별 sprite/mesh
variant로 확장한다.

### CAP-03. Screen presentation과 textured shard overlay

기존 ScreenPost는 RGBNoise, ZoomBlur, FilmNoise 세 scalar profile만 지원한다. 화면을 가로지르는 실제
유리 파편은 world Glasshole/Fluid material과 다른 기능이다.

새 screen overlay channel은 다음을 소유한다.

```text
Resources-relative overlay texture and optional normal/noise/mask lanes
screen-space position/scale/rotation/angular velocity
per-shard deterministic seed and depth order
spawn/life/color/alpha/dissolve curves
SceneColor refraction sample and tint
HDR before/after tone-map placement proven by source pass
global ScreenPost enable switch and cancel-safe clear
```

Engine은 generic screen overlay composite primitive와 ping-pong target만 소유한다. Client Effect 문서는
LostArk profile ID, DDS binding, curves와 occurrence를 소유한다. 한 overlay의 resource/pass 실패는 해당
overlay만 격리하고 기존 scene과 다른 presentation channel을 유지한다.

차원 F raw source가 가진 `fx_d_fragment_005` world shard와 RGBNoise 2 + ZoomBlur 1은 먼저 exact
occurrence로 복원한다. 화면 textured shard가 raw source occurrence에 없다면 사용자 원본 화면을 근거로
`PROJECT_TUNED` presentation layer로 별도 표기하고 world source와 섞어 exact라고 부르지 않는다.

이 capability가 닫히면 차원 W/A/D와 `2050550` 등 Glasshole/WindowCrack 소비자를 inventory 순서로
확대한다.

### CAP-04. Dragon flow와 arc-length UV

Artist T 전용 Manager나 Lance skill switch를 만들지 않는다. 먼저 source TypeDataRibbon을 기존
`CASCADE_RIBBON` packet/validator와 typed trail renderer에 정확히 projection한다. 이 source ribbon과
새 dragon spine은 같은 carrier가 아니다. 기존 ribbon으로 source shape가 닫히지 않을 때만 별도
`SPLINE_SEGMENTED_RIBBON` carrier를 추가한다.

공용 dragon-flow carrier는 다음을 갖는다.

```text
source WModel 또는 sampled spine control points
head/body/tail segment roles
cumulative arc length U
cross-section V
head/tail width and taper curve
carrier travel, bend and orientation-to-tangent
base/normal/emissive/dissolve/distortion independent UV panners
deterministic seed and fixed-step simulation
root/bone/world target attachment
```

창술 `fm_x_flm_gdr_01` body와 `fm_x_flm_gdr_01_dragon`은 source mesh UV를 보존하고 material의 named
lane panning을 분리한다. Artist T는 source action의 combined mesh owner가 Effect element가 아니라
Animation/CModel로 기록돼 있으므로 다음 순서로 처리한다.

```text
source object/combined-mesh lineage 재감사
-> 독립 skeletal/static asset이면 CModel/CMaterial cook + model cue
-> 독립 asset이 아니면 recovered dragon material + spline/segmented carrier로 source silhouette 재현
```

단순 helix 세 행을 크게 늘려 용처럼 보이게 만드는 것은 종료 조건이 아니다.

### CAP-05. Attractor, vortex와 camera presentation

도화가 V의 source에는 velocity, acceleration, orbit, cylinder-spin 계열 module이 있으나 현재 제품
generic 초기속도만으로 중앙 집결을 완성할 수 없다. 먼저 exact yellow-wisp occurrence의 source module
curve를 재구성한다. source가 target seek를 직접 요구할 때만 공용 attractor atom을 추가한다.

공용 atom의 계약:

```text
world/root/bone target
active normalized interval
radial and tangential acceleration
maximum speed and convergence radius
optional arrival damping
deterministic seed/fixed-step behavior
stop/cancel/death/level-change clear
```

distortion은 material/render MRT, PointLight와 ScreenPost는 presentation carrier, SHAKE/FOV는 composition
camera channel로 각각 분리한다. FOV/shake는 exclusive cinematic camera override를 직접 점유하지 않는다.
Animation cue가 source timing을 소유한 SHAKE를 typed additive camera impulse/FOV envelope로 materialize하고,
여러 effect의 source order, clamp, 종료 복귀를 검증한다.

### CAP-06. Animated model/projectile carrier

현재 ModelCue는 WModel clip, pre-transform, local velocity, alpha pass와 bone matrices를 실행할 수 있다.
따라서 도화가 E는 새 renderer가 아니라 telemetry와 runtime identity 진단 대상이다.

D 호랑이와 T 용은 다음 조건으로 ModelCue를 사용한다.

- source mesh가 실제 skeletal animation을 소유한다.
- WModel cook receipt가 source package, skeleton, clip, material slot hash를 봉인한다.
- direction은 CAP-11의 orientation authority/temporal policy와 root coordinate contract로 변환한다.
- renderer는 velocity tangent 또는 source orientation을 명시적으로 선택한다.
- model cue 실패가 particle/decal 나머지를 제거하지 않는다.

static mesh flow라면 ModelCue에 가짜 animation 이름을 넣지 않고 CAP-04 carrier를 사용한다.

### CAP-07. Local Decal projector와 authored motion

도화가 R과 워로드 T는 texture override가 아니라 실제 Local Decal carrier를 사용한다. projector는
height/diffuse/dissolve/normal/specular/emissive named lane, channel, color space, sampler, depth, cull/pass를
보존한다.

현재 `revolutionDegreesPerSecond`는 self angular velocity로 실행되므로 중심에 놓인 R symbol 회전에
충분하다. orbital revolution은 source occurrence가 실제 공전을 요구할 때만 별도 schema로 추가한다.
scale/alpha/dissolve 종료가 decal slab 전체를 갑자기 끄지 않도록 life curve를 적용한다.

### CAP-08. Effect Tool source-aware authoring

현재 Tool이 지원하는 여섯 family는 Mesh, Sprite, MeshParticle, SpriteParticle, LocalDecal, Trail이다.
이 분류는 AnimationTrail과 CascadeRibbon을 합치고, source DecalParticle, ModelCue, Light, ScreenPost를
표현하지 못한다. Light와 ScreenPost는 문서/runtime에 존재하지만 tree에서 `END`로 떨어져 영구 편집할
수 없다.

이번 공통 수정:

- authored history trail / AnimationTrail / CascadeRibbon / spline ribbon을 subtype으로 분리
- static LocalDecal과 source DecalParticle을 분리
- `MODEL_CUE`, `PRESENTATION_LIGHT`, `PRESENTATION_SCREEN_POST`, 이후 ScreenOverlay/CameraImpulse family 추가
- tree/count/solo/mute/isolation/Apply/Revert/Save roundtrip 연결
- Light의 enabled/range/intensity/color/ambient/falloff 편집
- Post의 profile/intensity/secondary/frequency/tint/seed 편집
- source-owned material lane에 `inherit`, `explicit binding`, `disabled/influence=0` 세 상태 제공하되,
  기존 scalar 0으로 exact 식을 제어할 수 있는 family에는 새 schema를 만들지 않음
- compiler source reset과 실제 disabled를 UI와 JSON에서 구분
- ModelCue clip/trajectory는 새 typed transaction으로만 편집
- 외부 disk revision 충돌은 기존 문서를 유지하고 Reload Saved 안내

전역 Preview ScreenPost checkbox는 비영속 A/B 필터로 유지하며 문서 값을 수정하지 않는다.

### CAP-09. Server-authoritative ground target

차원 T는 presentation-only cursor가 아니다. Client preview와 Server 결과가 다른 위치에 생기지 않도록
다음 vertical slice를 한 변경 단위로 닫는다.

```text
PlayerController targeting state
-> screen cursor world/nav projection
-> local range/validity preview
-> LMB confirm consumes basic attack input
-> IPlayerCommandSink / C2S_USE_SKILL target intent
-> Server finite/range/navigation validation
-> SERVER_PLAYER accepted target XZ ownership
-> PLAYER_SNAPSHOT target XZ replication
-> ClientReplication / Character presentation
-> target-root effect and Server damage shape
```

T를 처음 누를 때 command, cooldown, resource를 소비하지 않는다. LMB confirm에서만 intent를 보내며
RMB/Esc/두 번째 T, 다른 skill, death, class/level change는 preview를 transactionally 취소한다. range 밖
cursor는 11m에 clamp하고 nav-invalid 위치는 빨간 표시와 confirm 금지로 처리한다.

Remote Client도 같은 target을 봐야 하므로 local remembered cursor를 제품 정답으로 쓰지 않는다.

### CAP-10. Combo와 presentation timeline occurrence

차원 LMB의 combo stage/window는 Server가 소유하고 Client는 snapshot을 표현한다. 차원 A의 네 slash는
Effect-local presentation timeline이며 Server 단일 damage timing과 분리한다.

- physical LMB up->down 하나당 command 하나
- held LMB 자동 BA 진행 금지
- stage window 안의 명시적 다음 click 한 개만 buffer
- BA3만 `ba3.unified`, BA1/2/4는 `ba1.unified`
- stage와 effect occurrence timing을 vector index가 아니라 stable clip/effect ID로 검증
- A의 네 visual slash는 source cadence 또는 명시적 PROJECT_TUNED cadence를 Effect document가 소유

### CAP-11. Action-facing local anchor

위치 anchor의 시간 정책과 방향 authority를 하나의 `follow` boolean으로 합치지 않는다.

```text
local TRS
× source-basis yaw
× orientation authority
    ANCHOR | ACTION_FACING
× anchor temporal policy
    SNAPSHOT | FOLLOW
```

`ANCHOR`는 기존 문서와 동일하게 sampled owner/model root의 방향을 사용한다. `ACTION_FACING`은 같은
`PLAYER_SNAPSHOT`이 전달한 finite yaw와 `actionStartTick`을 Character가 한 쌍으로 캡처해 effect spawn에
전달하고, 위치의 follow/snapshot과 독립적으로 cast 방향만 고정한다. unknown token과 non-finite yaw는
정상값으로 fallback하지 않고 cue 또는 occurrence를 격리한다. 기존 cue는 orientation token이 없으면
`ANCHOR`로 해석해 하위 호환한다. 첫 admission은 `anchor="root"`로 제한한다. bone/socket anchor는 source
local basis를 대체하는 의미가 닫히기 전까지 `ACTION_FACING`을 거부한다. root의 translation/scale은 sampled
anchor에서 유지하고 rotational basis만 action yaw로 교체한다.

Server는 skill press의 aim으로 yaw를 즉시 확정하지만 Client character는 그 yaw를 presentation turn
rate로 부드럽게 추종한다. 따라서 0ms에 보이는 root만 snapshot하거나 짧은 effect가 live root를 따라가게
두면 본체 smoothing 동안 effect가 이전/중간 방향으로 생성된다. 이 결함을 Detail rotation이나 추측
`-90` 보정으로 숨기지 않는다.

첫 소비자 계약:

```text
Dimension A: outer position FOLLOW + orientation ACTION_FACING
             inner source occurrence SNAPSHOT
Warlord W:   outer position FOLLOW + orientation ACTION_FACING
             inner attachment disabled/live position
```

차원 A의 각 `.25/.60/.90/1.30s` occurrence는 독립 local pose를 유지하되 네 행 모두 같은 cast-facing
basis를 사용한다. 워로드 W는 caster 위치를 계속 추종하더라도 cast 방향은 흔들리지 않는다. 이후 source가
명시적으로 live steering을 요구하는 skill만 `ANCHOR+FOLLOW`를 유지한다.

두 스킬 모두 local occurrence 위치와 element-level velocity는 action basis에 정확히 한 번만 곱한다.
source basis yaw, Detail rotation, source mesh rotation/type-data geometry pre-rotation은 서로 다른 소유자이며
중복 적용하지 않는다. yaw 0/90/180/-90도, 직전 yaw가 반대인 action edge, mid-action owner turn으로
position, forward vector, position follow와 orientation snapshot의 독립성을 검증한다.

## 4. G별 구현 순서

### G00. 기준선과 machine-readable restoration inventory

목표: 문서의 대상 목록과 실제 source/product/runtime 분모가 다시 어긋나지 않게 한다.

추가할 산출물:

```text
Data/Effects/Contracts/character-effect-restoration-targets.v1.json
Tools/EffectPipeline/build_character_effect_restoration_inventory.py
Tools/EffectPipeline/test_build_character_effect_restoration_inventory.py
```

contract top-level은 target 행의 평면 목록이 아니라 다음 registry다.

```text
carrierVariants[]
materialVariants[]
renderVariants[]
compositionVariants[]
cohorts[]
occurrences[]
targets[]
```

각 occurrence는 `authoredNode.kind=ELEMENT|MODEL_CUE|ANIMATION_CUE`, stable ID, provenance/evidence의
field-level receipt, 네 variant ID, canonical `executionClosureId`, shader evidence, runtime executor/admission,
Product join, user review, blocker와 `failureScope=THIS_OCCURRENCE`를 소유한다. CompositionVariant는
position anchor/follow와 `ANCHOR|ACTION_FACING` orientation을 별도 field로 봉인한다. class/slot/skill/clip은
target과 composition이 소유하며 shader selector에 들어가지 않는다.

MaterialVariant는 parent/child/static-set SHA뿐 아니라 named lane마다 다음을 봉인한다.

```text
laneId / semantic role / assetId / sourceChannel
textureRegister / samplerRegister / colorSpace
filter / addressU,V,W / mip
scalar-vector-static-dynamic consumed and suppressed masks
particleColorPolicy / recovered equation ID
coveragePolicy / emissiveSourcePolicy / missingLanePolicy
```

문서에는 사람이 읽는 목적과 판단만 두고 exact 행 목록은 contract가 소유한다. first canary에는 Artist F
17행, 차원 A MakeFlow, 워로드 W hemisphere, Glasshole02, Fluid01 sprite/mesh, Lance dragon body/dragon,
Artist E ModelCue/FlowRibbon과 T TypeDataRibbon을 포함한다.

validator는 다음을 실패 처리한다.

- playable clip에 effectref, catalog 또는 runtime direct row가 없음
- stable occurrence가 중복되거나 다른 source node를 가리킴. 단, explicit project-retime substitution receipt가
  두 stable ID, 원본/저작 timing과 target cardinality를 모두 봉인한 차원 A canary만 좁게 허용
- resource path가 없거나 Resources root를 벗어남
- family tuple/closure hash와 carrier/pass가 불일치
- DXT1/no-alpha texture를 명시적 coverage 식 없이 `base.a`로 admission
- missing mask/emissive lane에 implicit default SRV를 연결하거나 source 없이 intensity/luminance fallback
- `materialVariantId=null`인데 Light/Post/Camera가 아닌 carrier
- TypeDataRibbon을 sprite로 silent projection
- sourceNode occurrence time과 authored timing이 다르지만 `PROJECT_TUNED` field receipt가 없음
- 선언한 MRT output 일부가 검증되지 않았는데 RT0만으로 admission
- `PROJECT_TUNED` occurrence를 source exact로 승격
- FAIL_CLOSED optional row가 sibling visible/draw/완료 분모를 제거
- golden Artist F 17행 identity/backend/opcode가 변함

첫 정상 회귀는 도화가 D의 missing effectref+catalog, 차원 A의 source occurrence/cardinality와 혼합 timing
provenance를 의도적으로 검출해야 한다.

### G01. authored subset save와 Presentation Tool 경계

목표: 손튜닝을 저장할 수 있고 Light/Post를 Tool에서 실제로 제어할 수 있게 한다.

워로드 A의 `Apply_Warlord17090SourceProjection` 책임을 다음처럼 분리한다.

```text
immutable source evidence validator
  source exact 12 = chain06 8 + chain07 4를 검증

mutable authored subset validator/projector
  retained row가 allowlisted source occurrence인지 검증
  mesh/material/rotation/preScale/dynamic identity 검증
  duplicate/unknown/malformed row fail-close
  exact 12 cardinality는 요구하지 않음
```

ordinary authored Load와 atomic Save roundtrip은 mutable subset만 사용한다. source importer/receipt harness만
immutable exact cardinality를 요구한다. temp save 재로드가 실패하면 기존 disk/document/preview를 유지한다.

같은 G에서 CAP-08 Light/Post tree를 연결하고 워로드 BA1 RGBNoise row를 Tool에서 선택할 수 있음을
harness로 검증한다.

### G02. product join closure와 direct runtime admission

목표: source document가 있어도 실제 skill에서 생성되지 않는 join 누락을 자동 차단한다.

```text
PlayerSkills inputSlot/skillId
-> skillbindings clip
-> clip-local animevent effectref=asset
-> source/direct EffectCatalog row
-> published 4-field runtime row
-> CEffectPresentationService prewarm/spawn
```

도화가 D `31490`을 첫 fixture로 effectref, catalog, runtime publish를 복구한다. E와 F가 같은 변경에서
중복되거나 다른 asset으로 바뀌지 않는 negative regression을 둔다.

### G03. class-neutral shader variant execution

목표: F 방법론을 반복 가능한 family pipeline으로 만들고 skill hack을 차단한다.

새 common registry는 CAP-01 key, evidence state, backend, opcode, pass/output role, expected texture mask와
consumer occurrence set을 소유한다. Artist F registry는 그대로 둔다.

가족 하나의 작업 순서:

1. exact occurrence와 MIC hierarchy를 inventory에 봉인한다.
2. effective static set으로 shader map 하나를 선택한다.
3. original DXBC/hash와 native object binding을 추출한다.
4. WARP synthetic fixture와 source-value fixture를 각각 실행한다.
5. sampler/VF/pass/MRT blocker를 명시한다.
6. native exact 또는 semantic HLSL backend를 선택한다.
7. resource/backend/opcode/pass negative tests를 만든다.
8. Product direct-authored occurrence에만 admission한다.

첫 cohorts는 `Glasshole02`, `Fluid01 sprite`, `Fluid01 mesh`, Lance dragon masked body/dragon이다.

### G04. 저위험 content 복원

공통 infrastructure 뒤 다음을 먼저 닫는다.

- 도화가 A 검격 8행 noise influence 0
- 도화가 S `PROJECT_TUNED` grass shape/dissolve와 emissive tip
- 도화가 R true Local Decal symbol 회전·소멸
- 워로드 T BA1 RGBNoise 제거
- 워로드 T BA3 exact decal 4개 selective restore
- 창술 Q existing mesh draw/admission 진단
- 창술 A transform Apply/Save/next-spawn 회귀
- 창술 E W cone exact transplant
- 차원 A current 9행 손튜닝 보존, MakeFlow 네 occurrence selective merge와 position FOLLOW,
  `ACTION_FACING`, inner SNAPSHOT 연결
- 워로드 W position FOLLOW와 `ACTION_FACING`, source basis, Detail/source mesh rotation exactly-once 진단
- 워로드 WModel 자체의 3-shield composite 여부를 resource/model receipt로 판정하고, 아니면 좌/중/우 stable
  cohort와 element-level local +Z travel을 `PROJECT_TUNED`로 추가

데이터 splice는 기존 user-tuned 행을 deep-equality로 보존하고 target stable ID만 추가/교체한다. 차원 A의
과거 34/63행 runtime 파일은 history 증거일 뿐 current Product로 rollback하지 않는다. retained event030
행의 sourceNode/local pose와 `.25s` PROJECT_TUNED retime을 field provenance로 분리한다. 이 visible row를
첫 타격으로 보존하고 `.60/.90/1.30s` source occurrence 세 행만 append해 Product visual cardinality를 정확히
네 번으로 만든다(`9 -> 12`). immutable base `.25s` source 행은 evidence-only로 보존하되 current retimed
row의 명시적 substitution receipt 없이는 Product에 중복 admission하지 않는다. 같은 sourceNode의 retimed
project row와 exact row가 공존하는 예외도 두 stable ID, field provenance와 target cardinality를 receipt가
봉인할 때만 허용한다.

### G05. animated animal carrier

도화가 E의 기존 crane와 FlowRibbon 세 행을 animated ModelCue/typed ribbon telemetry canary로 사용한다.
clip position, animation duration, bone matrix update, model pass, local velocity, ribbon carrier subtype과 selected
runtime asset ID를 focused harness에서 확인한다.

그 뒤 D 호랑이와 T 용 source lineage를 조사해 CAP-06 또는 CAP-04로 분기한다. 새 WModel이 필요하면
source package/mesh/skeleton/animation/material receipt, ModelAssetConverter, `CModel -> CMaterial`, project
registration과 runtime resource existence를 한 변경 단위로 닫는다.

### G06. glass/crack family와 차원 F/W rollout

작업 순서:

1. 차원 W의 existing Glasshole/Crackhole cooked oracle를 현재 source values로 재검증한다.
2. Glasshole02 sprite variant를 W/A/D occurrence에 admission한다.
3. F의 Fluid01 두 sprite를 bounded profile 34에서 full typed sprite variant로 승격한다.
4. Fluid01 mesh variant를 도화가 V, 창술 ALT_V, 워로드 17250에 별도로 admission한다.
5. LocalCrack/Crackhole mesh/sprite variants를 carrier별로 분리한다.
6. F raw 69행에서 role-selected sphere/hemisphere/world shard/light/post를 복원한다.
7. CAP-03 screen overlay를 연결하고 F 화면 파편을 구현한다.
8. W와 다른 playable 차원 occurrence에 family cohort rollout한다.

같은 parent 이름이어도 child/static set/carrier/role lane이 다르면 서로 다른 variant다.

### G07. dragon flow와 궁극기 rollout

Artist T의 fallback-blocked TypeDataRibbon을 먼저 typed CascadeRibbon으로 projection해 existing carrier
closure를 검증한다. 창술 V/ALT_V/T의 dragon rows를 첫 material/geometry cohort로 삼아 independent UV
lanes와 motion을 검증한다. Artist T는 source asset lineage 결과에 따라 exact model cue, CascadeRibbon,
또는 별도 spline carrier를 사용한다.

같은 MakeFlow/WaterTrail/SpriteWave/Fluid parent를 쓰는 도화가 V, 워로드 `17250/17820`, 창술
`34610/34630/34650` occurrence를 inventory가 소비자로 제시하게 한다. 한 스킬이 통과했다고 family
전체를 자동 admission하지 않고 exact variant key가 같은 occurrence만 확대한다.

### G08. 도화가 V attractor와 screen presentation

source timeline의 early distortion/FOV/light와 late `3.0/3.2/3.4s` burst를 occurrence별로 복원한다.
노란 wisp의 spawn cylinder/orbit/velocity curve를 먼저 재생하고, 부족한 target-seek 동작만 CAP-05로
구현한다.

camera/post/light/overlay submission은 source order가 deterministic해야 하며 effect stop, death,
disconnect, class/level change 뒤 한 frame 안에 제거된다. 다른 effect의 presentation을 clear하지 않는다.

### G09. 차원 LMB/A/T gameplay vertical slice

LMB:

- current one-edge-one-command contract 회귀
- source combo windows publisher/bootstrap 반영
- Server buffer acceptance/rejection boundary tests
- BA stage snapshot과 effectref mapping 회귀

A presentation:

- current user-tuned subset과 source visual occurrence 네 개를 별도 receipt로 봉인
- source cadence `.25/.60/.90/1.30s`와 equal-quarter 후보를 별도 provenance로 출력
- CAP-11 position FOLLOW/action-facing orientation/occurrence SNAPSHOT을 적용하고 0/90/180/-90 matrix 검증
- 승인된 visual timing만 authored/runtime에 commit
- Server single `hitTimeMs`와 DamageProfile은 이 G에서 변경하지 않음

T:

- CAP-09 전체 Client/Shared/Server/Client presentation 구현
- invalid finite/range/nav, duplicate sequence, cancel, cooldown/resource rollback tests
- approved target XZ에서 summon effect와 damage shape가 같은 root를 쓰는 contract test

### G10. 전체 Product publish와 class rollout

각 capability가 자동 PASS된 뒤 대상 direct-authored 문서를 selective update한다. 모든 authoring 저장이
끝난 한 revision에서만 full `Publish-Effects.ps1 -Mode Publish`를 실행하고 Client를 재시작한다.

Product admission 순서:

```text
solo occurrence
-> material family cohort
-> 한 clip Product cue
-> 전체 skill
-> animation anchor/targeting
-> class 전체
-> cross-class shared family consumers
```

### G11. 사용자 visual iteration과 golden 승격

각 스킬은 사용자의 첫 관찰을 RESULT에 다음 네 축으로 기록한다.

```text
형태/coverage
색/HDR/bloom
timing/density/trajectory
화면 post/camera 피로도와 종료 복귀
```

문제가 있으면 숫자를 무작정 만지지 않고 occurrence, carrier, material equation, render pass 중 어느
층인지 먼저 분류한다. 사용자가 승인한 스킬은 stable occurrence/resource/backend/opcode와 user-tuned
transform을 golden regression으로 추가한다.

## 5. 변경 파일 스택

### 5.1 Data와 publisher

```text
Data/Balance/PlayerSkills.json
Data/Animation/Authored/<Class>/<Class>.skillbindings.json
Data/Animation/Authored/<Class>/<Class>.animevents
Data/Effects/Authored/effect.<class>.skill.*.effect.json
Data/Effects/Imported/<Class>/...
Data/Effects/Contracts/...
Data/Effects/CookedShaders/<sha256>.dxbc
Data/Effects/EffectCatalog.json
Tools/EffectPipeline/Publish-Effects.ps1
Tools/EffectPipeline/extract_ue3_material_shader_maps.py
Tools/EffectPipeline/replay_ue3_material_pixel_shaders.py
Tools/EffectPipeline/evaluate_ue3_material_uniform_expressions.py
Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py
Tools/EffectPipeline/materialize_ue3_exact_cooked_shader_variants.py
Tools/EffectPipeline/build_character_effect_restoration_inventory.py
Tools/EffectPipeline/test_build_character_effect_restoration_inventory.py
```

generated `Client/Bin/DataFiles/Effect` 문서를 직접 고치지 않는다.

### 5.2 Client Effect runtime와 Tool

```text
Client/Public/Effect_AuthoringDocument.h
Client/Public/Effect_MaterialTemplate.h
Client/Public/Effect_Tool.h
Client/Public/Effect_Playback.h
Client/Public/Effect_DocumentRenderer.h
Client/Public/AnimationEffectCueDocument.h
Client/Public/Effect_PresentationService.h
Client/Public/Character.h
Client/Private/AnimationEffectCueDocument.cpp
Client/Private/Animation_Tool.cpp
Client/Private/Character.cpp
Client/Private/ClientReplication.cpp
Client/Private/Effect_DocumentCodec.cpp
Client/Private/Effect_Tool.cpp
Client/Private/Effect_Playback.cpp
Client/Private/Effect_DocumentRenderer.cpp
Client/Private/Effect_Object.cpp
Client/Private/Effect_Catalog.cpp
Client/Private/Effect_PresentationService.cpp
```

공통 source variant registry를 새 파일로 만들면 `Client.vcxproj`와 `.filters`, ClientFrontendHarness와
같은 G에서 등록한다. F 전용 registry는 golden fixture로 유지한다.

### 5.3 Shader와 rendering

```text
Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli
Client/Bin/ShaderFiles/Shader_EffectRuntimeMaterialPacket.hlsli
Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli
Client/Bin/ShaderFiles/Shader_EffectLocalDecalAdapter.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl
Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl
Client/Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl
Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl
Engine/Public/PresentationProvider.h
Engine/Public/Presentation_Manager.h
Engine/Private/Presentation_Manager.cpp
Engine/Private/Renderer.cpp
```

새 glass/dragon/screen overlay HLSL은 실제 first consumer와 같은 G에서 추가하고 project/filter 등록,
standalone FXC entry compile, runtime pass를 함께 닫는다. 미래용 빈 shader 파일은 만들지 않는다.

### 5.4 차원 T network/gameplay

```text
Client/Private/PlayerController.cpp
Client/Public/PlayerCommandSink.h
Client/Public/NetworkPlayerCommandSink.h
Client/Private/NetworkPlayerCommandSink.cpp
Shared/Public/Network/PacketMessages.h
Shared/Private/Network/PacketMessages.cpp
Server/Public/ServerPlayer.h
Server/Public/PlayerSkillSystem.h
Server/Private/PlayerSkillSystem.cpp
Server/Private/ServerGameplayContractTests.cpp
Client/Private/ClientReplication.cpp
```

Shared packet을 바꾸면 writer/reader, NetworkProtocolHarness, Server contract, Client snapshot consumer를
같은 변경 단위에서 수정한다.

## 6. 자동 검증 계획

### 6.1 Data, source와 join

- 모든 JSON parse와 schema/version 검사
- requested skill/clip/action/effectref cardinality
- source occurrence stable ID와 resource hash
- immutable source exact vs mutable Product subset 분리
- missing/duplicate/unknown ID와 path traversal rejection
- 4-field runtime catalog row와 direct-authored path existence
- Artist F 17행 golden deep identity

### 6.2 Shader oracle와 typed runtime

- original DXBC SHA/signature/variant-key check
- truncated/unknown DXBC와 ambiguous static-set negative cases
- WARP fixed-input output와 CPU closed-form oracle tolerance
- source scalar/vector/dynamic parameter sensitivity
- texture channel negative controls
- sampler/color-space/pass/output-role contract
- backend/opcode/pass/resource mask admission과 mismatch fail-close
- one-vs-multi render-target RT0 parity where source contract permits
- 차원 A MakeFlow는 carrier/mesh/module identity와 reconstructed material executor 상태를 따로 판정;
  unresolved parent diff/flow lanes가 남으면 pixel exact로 승격 금지

### 6.3 Carrier와 presentation

- spline degenerate/zero-length path rejection
- arc-length U invariance under segment subdivision
- deterministic seed and fixed-step replay
- attractor convergence/overshoot/cancel tests
- model animation clip/bone matrices and orientation-to-velocity
- root yaw 0/90/180/-90과 직전 반대 presentation yaw에서 authoritative action edge
- 차원 A position-follow/action-facing/inner-snapshot occurrence별 불변
- 워로드 W live-position follow와 action-facing 방향 snapshot의 독립 갱신
- source-basis yaw/Detail rotation/source mesh rotation exactly-once와 네 local occurrence 독립 matrix
- Local Decal projector/self-spin/orbit/fade distinction
- Light/Post/Overlay ordering, odd/even ping-pong, max count, invalid resource isolation
- stop/death/disconnect/level transition clear

### 6.4 Tool과 save

- source exact 12와 Product subset roundtrip 각각 검증
- delete 12→legal composite subset Save→temp Load→hot reload
- duplicate stable ID, unknown model/material, malformed retained row rollback
- Light/Post tree cardinality와 solo/mute
- Apply/Revert/Save/Reload persistence
- global preview A/B가 문서를 변경하지 않음
- disk revision conflict 시 기존 document 보존

### 6.5 Gameplay

- LMB physical edge 한 번당 USE_SKILL 하나
- combo window 안/밖, stage 끝, held input, unrelated move/skill pending 정책
- T preview 상태에서 LMB/RMB/Esc/T/다른 skill input consumption
- finite/range/nav/sequence/cooldown/resource validation
- Server-approved target XZ snapshot roundtrip
- remote Client target presentation
- effect root와 damage root equality

## 7. build와 regression 종료 조건

각 G의 focused harness 뒤 최종 통합에서는 AGENTS 정본 순서를 지킨다.

```powershell
powershell -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate

powershell -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Test-EffectPipeline.ps1

powershell -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug

powershell -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release

git diff --check
```

Engine public/presentation 변경이 포함된 G는 다음을 생략하지 않는다.

```text
Engine x64 Debug/Release
UpdateLib.bat Debug/Release
Shared + NetworkProtocolHarness Debug/Release와 실행
Server Debug/Release + Server.exe --contract-test
ClientFrontendHarness Debug/Release와 focused effect flags
Client x64 Debug/Release
```

새 focused harness가 추가할 최소 실행 flag:

```text
--effect-character-restoration-inventory-fast
--effect-authored-subset-save-fast
--effect-source-variant-fast
--effect-glass-family-fast
--effect-dragon-flow-fast
--effect-screen-overlay-fast
--effect-presentation-clear-fast
--dimensionmaster-ground-target-fast
```

실제 구현 전에는 위 새 flag를 PASS로 기록하지 않는다. 기존 F registry/WARP/material/draw harness도
같이 실행해 golden regression을 확인한다.

## 8. 사용자 수동 검증 순서

사용자는 빌드·publish·재시작 준비가 끝난 뒤 `Client/Default` working directory에서 직접 확인한다.

### 도화가

```text
A  검격 edge가 noise에 묻히지 않고 나비/decal은 유지
S  난초 길이와 풀끝 light가 함께 fade
R  실제 바닥 symbol, 중심 회전, scale/alpha dissolve
D  호랑이 carrier와 비행 방향/animation
E  두루미 날갯짓과 이동, 붓 ribbon
F  golden 화면 무회귀
T  용 head/body/tail silhouette와 자연스러운 UV flow
V  노란 wisp 중앙 집결, 화면 소용돌이, 종료 시 camera/post 복귀
```

### 워로드

```text
W  동/서/남/북 및 180도 반전 cast에서 전면 hemisphere/방패가 승인된 cast 방향으로 생성되고,
   세 shield formation과 전진이 caster 회전 뒤에도 흔들리지 않음
A  화면상 완성 사슬 네 방향, 저장/재시작 뒤 유지
T  Start RGBNoise 제거
T  Release 바닥 decal 네 장의 검은 경계/붉은 내부와 타이밍
```

### 창술사

```text
Q  두 clip의 검격 mesh 실제 draw와 방향
A  선택 occurrence rotation/position이 다음 spawn에 반영
E  굉열파 hit에서 사두룡격 cone 역할 재현
V/ALT_V/T  대상 용의 궤적, 비늘/빛/normal UV flow와 dissolve
```

### 차원술사

```text
LMB  한 클릭 BA1, window 안 재클릭으로 BA2→BA3→BA4
A    동/서/남/북 및 직전 반대 방향 cast에서 세로 MakeFlow 네 검격이 승인된 action-facing 기준으로 생성,
     local 간격·형태와 animation contact 유지
T    targeting enter, 11m clamp, invalid 표시, LMB confirm, RMB cancel,
     승인 위치의 summon/damage와 정상 gameplay 복귀
F    창 타격, 반구 확장, world shard, 화면 유리 파편과 post
W    F에서 연 family가 기존 vertical slash/유리 균열에 재사용
```

## 9. living plan 갱신 규칙

이 문서는 각 G가 끝날 때 다음 표만 현재 사실로 갱신하고, 실행 세부 로그는 RESULT로 보낸다.

| G | 상태 | 자동 증거 | 사용자 화면 | 다음 blocker |
|---|---|---|---|---|
| G00 inventory | `AUTO_PASS` | 22 occurrence family-first contract, builder check, focused 23 tests | 해당 없음 | first Product family executor |
| G01 Tool/save | `IMPLEMENTED` | Warlord 17090 retained subset Save/Load 7/7, invalid identity/mesh/recipe rollback | 미실행 | Light/Post/Ribbon subtype Tool tree |
| G02 join | `PLANNED` | D 누락 실측 | 미실행 | effectref/catalog 복구 |
| G03 shader variants | `EVIDENCE_PARTIAL` | F/W existing oracle | 미실행 | sampler/VF/pass/Product admission |
| G04 low-risk skills | `IMPLEMENTED` | A/W action-facing Debug/Release 8/8, Dimension A 4 occurrence authored | 미실행 | runtime full publish, Artist/Warlord corrections |
| G05 animated animals | `EVIDENCE_PARTIAL` | E WModel/clip 연결 | 미실행 | D/T asset lineage |
| G06 glass/crack | `EVIDENCE_PARTIAL` | 5 cooked DXBC, Product consumer inventory | 미실행 | typed variants/screen overlay |
| G07 dragon/ultimate | `EVIDENCE_PARTIAL` | Lance mesh/material cohort | 미실행 | independent UV/material execution |
| G08 Artist V | `EVIDENCE_PARTIAL` | source timing/module audit | 미실행 | attractor/camera channel |
| G09 Dimension gameplay | `EVIDENCE_PARTIAL` | current command/snapshot audit | 미실행 | approved target replication |
| G10 publish | `PLANNED` | 미실행 | 미실행 | preceding Gs |
| G11 visual/golden | `PLANNED` | 미실행 | 사용자 대기 | 실제 관찰 |

상태 값은 `PLANNED`, `EVIDENCE_PARTIAL`, `IMPLEMENTED`, `AUTO_PASS`, `USER_REVIEW`, `COMPLETE`,
`FAIL_CLOSED`만 사용한다. `COMPLETE`는 automatic PASS와 사용자의 해당 스킬 visual 승인 둘 다 있을
때만 기록한다.

## 10. 첫 실행 단위

다음 구현 세션은 범위가 가장 명확하고 다른 고난도 G의 안전 기반이 되는 순서로 시작한다.

```text
1. clean baseline과 G00 family-first restoration inventory
2. CAP-11 차원 A/워로드 W action-facing matrix harness와 source occurrence selective merge
3. G01 Warlord A subset save + Light/Post/Ribbon subtype Tool tree
4. G02 Artist D product join closure
5. G04 Warlord T noise/decal와 Artist A/R low-risk corrections
6. G06 Glasshole02 첫 Product family canary
7. G06 Dimension F world composition + CAP-03 screen overlay
8. G07 source CascadeRibbon + Lance dragon typed UV family + Artist T carrier
9. G08 Artist V attractor/screen presentation
10. G09 Dimension LMB/T gameplay, cross-class rollout, full publish/build, 사용자 판정
```

도화가 F는 모든 단계에서 수정 대상이 아니라 이미 승인된 golden identity canary다. 새 family는
F의 17행 Product identity·resource·backend를 수정하지 않는 별도 execution closure로 추가한다.
