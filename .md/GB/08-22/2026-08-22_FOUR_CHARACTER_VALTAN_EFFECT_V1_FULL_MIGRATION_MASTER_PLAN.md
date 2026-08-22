# 2026-08-22 4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획

기준 branch: `origin/main`

기준 commit: `61930d4ab7018b9df04dea7c297681505160c34d`

V0 상태: `COMPLETE_BASELINE`

V1 상태: `PLANNED`

최종 화면 판정자: 사용자

연결 정본:

- [`Effect Family Runtime ABI 복원 가이드`](../../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)
- [`Character Source-Exact Effect Conquest 기존 결함·기능 계획`](../08-21/2026-08-21_CHARACTER_SOURCE_EXACT_EFFECT_CONQUEST_MASTER_IMPLEMENTATION_PLAN.md)
- [`캐릭터 Effect authored → sealed runtime 통합 결과`](2026-08-22_CHARACTER_EFFECT_RUNTIME_PUBLISH_INTEGRATION_RESULT.md)
- [`Glasshole02 Missing Family ABI 복원 결과`](2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_RESULT.md)
- [`Valtan Core-Three Missing Family ABI 복원 결과`](2026-08-22_VALTAN_FRONT_BACK_FRONT_MISSING_FAMILY_RESTORATION_RESULT.md)
- [`Client frontend 광역 하네스 삭제 결과`](2026-08-22_CLIENT_FRONTEND_HARNESS_REMOVAL_RESULT.md)

이 문서는 도화가, 워로드, 창술사, 차원술사와 Valtan의 **현재 Product Effect 전체**를
FAMILY_LITE 가시화 기반에서 typed/source-derived runtime ABI로 승격하는 living master다.
개별 스킬 몇 개를 고치는 목록이 아니라 전체 분모, family/adapter 구현 순서, Product admission과
완료 증거를 소유한다. 각 G를 시작할 때 이 문서의 해당 G에 실제 파일 delta와 최종 계약을 추가하고,
실행 결과는 같은 날짜의 RESULT에만 기록한다.

범위에는 현재 live 행뿐 아니라 V0 손튜닝 중 잘못된 카드/coverage/UV 때문에 삭제한 원본 source
occurrence의 복구도 포함한다. 현재 손튜닝 결과는 보존 기준선이고, 삭제 행은 V1 family가 닫힌 뒤
Tool-only 후보로 되살려 사용자 승인 후에만 Product composition에 다시 넣는다.

## 0. 이번 결정

### 0.1 V0는 완료로 고정한다

V0 완료의 뜻은 다음과 같다.

- 현재 live `205 assets/5,294 elements`의 cue 또는 combat-object visual, EffectCatalog, Authored
  document, sealed runtime 제품 join이 존재한다.
- Sprite/Mesh/Particle/Decal/Trail/Light와 typed presentation을 생성·저장·publish할 기반이 있다.
- generic/grouped translucent와 기존 typed opcode로 현재 live 스킬과 pattern/combat-object visual을
  화면에 표시할 수 있다.
- 4캐릭터의 기존 손튜닝 transform, timing, source recipe와 Valtan pattern composition을 기준선으로
  보존할 수 있다.
- Debug/Release build, publisher, focused harness와 rollback 경계가 있다.

V0 완료는 원작 색·coverage·UV·굴절·MRT가 정확하다는 뜻이 아니다. V0의 목적은 조립된 Effect를
현재 live graph에서 제품으로 보이게 하고 손튜닝 가능한 기준 화면을 확보하는 것이었다. V0 손튜닝
중 빠진 source occurrence나 현재 cue에 미연결된 direct document까지 복구했다는 뜻도 아니다. 이 목적은
`COMPLETE_BASELINE`으로 닫고, V1 작업 중 V0 전체를 다시 설계하지 않는다.

### 0.2 V1 architecture와 fidelity는 서로 다른 축이다

V1 architecture closure는 frozen current row와 generated final Product/presentation row가 다음의 명시적
상태를 갖는다는 뜻이다. frozen `205/5,294`는 입력 기준선이지 최종 완료 분모가 아니며, final에서
비활성인 행도 terminal disposition을 가져야 한다.

```text
runtime architecture
  V0_COMPATIBILITY
  V1_TYPED_CANDIDATE
  V1_TYPED_PRODUCT
  V1_TYPED_PRESENTATION_PRODUCT

disposition
  LIVE_PRODUCT
  AUDITION_ONLY
  LEGACY
  RETIRED
  DISABLED_WITH_RECEIPT

fidelity evidence
  SOURCE_EXACT
  BOUNDED_TRANSLATED
  PROJECT_RECONSTRUCTED
  NOT_APPLICABLE

runtime admission
  NOT_ADMITTED
  TOOL_ONLY
  PRODUCT_ADMITTED

visual review
  NOT_REQUIRED
  USER_REVIEW_PENDING
  USER_APPROVED
  USER_WAIVED
```

진행 중 architecture인 `V0_COMPATIBILITY`, live evidence blocker, `TOOL_ONLY`와 visual-required
`USER_REVIEW_PENDING`은 최종 완료 상태가 아니다. Evidence blocker는 위 enum을 섞지 않고 별도
blocker field가 소유한다. 원본 evidence가 없는 element를 억지로 source-exact라고 만들지 않는다.
그 경우 원본 부재를 receipt로 봉인하고 `V1_TYPED_PRODUCT + PROJECT_RECONSTRUCTED`로 승격한다.
Light/ModelCue처럼 material HLSL이 불필요한 대상은
`V1_TYPED_PRESENTATION_PRODUCT + NOT_APPLICABLE`로 닫는다.

전체 V1 완료는 lineage 이름만 붙이는 것으로 끝나지 않는다. G00이 생성한 domain/state별 final
분모에서 architecture, disposition, fidelity, runtime admission과 visual review가 모두 G11 조건을
만족해야 한다.

### 0.3 V1은 element마다 HLSL 하나를 만드는 계획이 아니다

V1의 identity와 재사용 단위는 다음처럼 분리한다.

```text
program
  parent Material GUID + effective static permutation + stage equation

layout / adapter
  carrier + VF/pass + scene input + RT/MRT topology

descriptor
  child MIC values + texture/sampler/CB wire + render state

occurrence
  stable element selector + composition/attachment/timing
```

같은 program/layout을 쓰는 element는 HLSL과 adapter를 재사용하고 occurrence별 descriptor를 가진다.
texture/scalar/render state만 다르면 descriptor만 추가한다. equation이 다르면 HLSL program을 추가하고,
VF/pass/scene/RT가 다르면 adapter/layout을 추가한다. skill ID나 class 이름으로 C++ renderer를
복제하지 않는다.

### 0.4 도화가 F는 V1 golden control로 고정한다

이 계획에서 도화가 F는 `V1_GOLDEN_CONTROL`이다. 즉 stable occurrence, typed equation/packet,
resource role, carrier/attachment/timeline과 사용자 손튜닝을 한 묶음으로 닫아 높은 화면 품질을 만든
방법론과 품질의 기준이다. 다만 과거 legacy evidence 35행 전체가 native source-exact였다는 뜻은 아니다.
V1 architecture와 `SOURCE_EXACT` fidelity는 계속 별도 축으로 기록한다.

나머지 4캐릭터 Effect를 “도화가 F처럼 V1으로 올린다”는 목표는 각 effect를 스킬 전용 renderer로
복제하는 것이 아니다. 필요한 distinct HLSL program과 shared adapter를 family 단위로 닫고, 현재 V0
손튜닝 composition 및 복구 승인된 source occurrence를 typed Product에 연결한 뒤 사용자가 effect
composition을 승인하는 것이다.

## 1. V0 기준선 실측

### 1.1 병합 기준

V1은 다음 병합 결과 위에서 시작한다.

| PR | 현재 의미 |
|---|---|
| `#140` | Valtan V0 pattern Effect graph와 family-lite publish 기반 |
| `#142` | Glasshole 제외 4캐릭터 Effect composition, typed seed와 gameplay 연결 |
| `#143`, `#144` | Glasshole02 translated Tool canary와 receipt 안정화 |
| `#145` | Valtan Ground/Masked/Crack 3 family·9 occurrence bounded RT0 Tool canary |
| `#146` | monolithic ClientFrontendHarness 삭제, focused 검증으로 전환 |

`#145`는 main에 병합됐지만 V1 Product 진척으로 세지 않는다. 해당 canary는 기본 OFF이고
Authored 문서를 바꾸지 않는 Tool projection이며 Product, actual VF/pass, exact sampler, scene CB,
MRT/coverage와 visual admission이 false다.

### 1.2 live Product 분모

다음 분모는 수동 추측이 아니다. frozen V0 fixture의 정확한 입력은 다음과 같다.

- `Data/Animation/Authored/{Artist,Warlord,LanceMaster,DimensionMaster}/{Class}.animevents`의
  `effectref=asset` payload를 class별 unique 처리한다.
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`의 boss-root `effectAssetId`와
  `Data/Actors/BossCatalog.json`의 `combatObjectVisuals[].effectAssetId`를 각각 unique 처리한다.
- 두 집합을 union한 뒤 `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`의 `effectAssetId`와
  exact join한다.
- catalog가 가리키는 `Client/Bin/DataFiles/Effect/Authored/*.effect.json` canonical document의
  element를 센다.

G00 receipt는 위 입력 파일과 선택된 canonical document의 SHA-256, asset-ID union, exact join 결과를
함께 봉인한다. 이후 generated final 분모가 바뀌어도 이 `205/5,294` fixture는 V0 snapshot으로 유지한다.

| domain | live Effect assets | live elements | 현재 `execution.enabled` typed seed | `execution.enabled=false` rows |
|---|---:|---:|---:|---:|
| 도화가 / Artist | 19 | 377 | 32 | 345 |
| 워로드 / Warlord | 22 | 463 | 0 | 463 |
| 창술사 / LanceMaster | 41 | 501 | 12 | 489 |
| 차원술사 / DimensionMaster | 15 | 270 | 2 | 268 |
| Valtan | 108 | 3,683 | 0 | 3,683 |
| **합계** | **205** | **5,294** | **46** | **5,248** |

현재 46행은 `runtimeMaterialV2 37 + artistVisualV4 7 + localDecal 2`다. 이 수는 typed 실행
seed일 뿐, 46행 전부가 source-exact admission을 통과했다는 뜻이 아니다.

live element의 현재 carrier/kind 분모는 다음과 같다.

| kind | count |
|---|---:|
| particle | 5,201 |
| decal | 57 |
| trail | 32 |
| sprite | 2 |
| light | 1 |
| mesh | 1 |
| **합계** | **5,294** |

Effect element 밖의 Product presentation도 G00 분모에 포함한다. 현재 별도 ModelCue는 최소 2행,
DimensionMaster F screen overlay 문서는 1개다. 이후 builder가 발견하는 screen/light/model
presentation은 별도 typed presentation 분모로 추가하며 material element 수에 중복 합산하지 않는다.

4캐릭터 composition 분모는 55 distinct skill IDs, 103 `effectref=asset` cue rows, 97 unique assets다.
같은 asset을 가리키는 반복 cue도 attachment/timing composition variant로 ledger에 유지한다. 4캐릭터
live 1,611행은 visible 1,568, hidden 43이고 typed seed 46은 모두 visible이다. 따라서 현재 visible
V0/untyped는 1,522행이다.

### 1.3 direct-authored migration corpus

4캐릭터의 `DIRECT_AUTHORED_DOCUMENT_V13` 후보 corpus는 106 assets/1,707 elements이고 현재 visible
element는 1,664다. live cue 도달 범위는 97/1,611이므로 9문서/96행이 catalog에는 있으나 현재
animevent에 join되지 않는다.

```text
DimensionMaster BA2, BA4, overslash clip2, clip3
Warlord BA1, BA2, battlefield clip3, clip4, clip8
```

이 9문서를 강제로 cue에 연결하지 않는다. G00에서 `JOIN_REQUIRED`, `NON_PRODUCT_EVIDENCE`,
`SUPERSEDED` 중 하나를 source/skillbinding 증거로 확정한다. Hidden 43행도 수를 줄이기 위해 삭제하지
않고 `DISABLED_WITH_RECEIPT` 또는 실제 V1 target으로 분류한다.

Valtan canonical V0 corpus 108/3,683은 모두 Product다. boss-root pattern cue가 106/3,665를,
BossCatalog combat-object visual이 나머지 2/18을 소유한다. 따라서 현재 direct/canonical migration
corpus는 214 assets/5,390 elements, live Product는 205/5,294이며 미결 join/disposition은
4캐릭터 9 assets/96 elements다.

### 1.4 catalog-addressable 상한과 live 분모를 섞지 않는다

현재 5개 domain의 catalog-addressable authored 상한은 315문서, 7,550 elements이고 Artist legacy
immutable program 한 행이 별도로 있다.

| domain | catalog entries | canonical authored docs | authored elements |
|---|---:|---:|---:|
| Artist | 34 | 33 | 560 |
| Warlord | 54 | 54 | 1,093 |
| LanceMaster | 82 | 82 | 1,553 |
| DimensionMaster | 38 | 38 | 661 |
| Valtan | 108 | 108 | 3,683 |
| **합계** | **316** | **315** | **7,550** |

frozen live 분모 5,294와의 차이 2,256행은 legacy, duplicate, audition 또는 현재 cue 비도달 문서다.
이를 무조건 Product로 승격하지 않는다. G00에서 각 행을 `LIVE_PRODUCT`, `AUDITION_ONLY`,
`LEGACY`, `RETIRED` 중 하나로 분류한다. “전체 V1”은 frozen 5,294행 각각의 successor/retirement를
설명하고, frozen catalog-addressable non-live 2,256행도 누락 없이 disposition을 가지며, 그 결정과
source restoration/join을 반영해 생성한 최종 Product 분모를 100% 닫는다는 뜻이다.

### 1.5 evidence inventory를 Product 분모로 오해하지 않는다

- `effect-family-manifest.v1.json`의 169 family와 5,105 scan elements는 discovery inventory다.
- 현재 canonical live rows의 non-empty child material path는 665종이다.
- sourceProfile로 join된 parent material path는 현재 145종이다.
- `character-effect-restoration-targets.v1.json`의 22 occurrence는 기존 우선 canary 집합이다.
- Valtan source inventory의 9,434 occurrence candidate, 1,044 carrier, reviewed 628 denominator는
  source evidence pool이다.

이 숫자 중 어느 것도 “필요한 HLSL 수”가 아니다. 실제 program/layout/descriptor 수는 G00/G01의
verified tuple inventory가 계산한다.

4캐릭터 direct-authored 1,707행의 현재 carrier 분모는 SpriteParticle 1,228,
MeshParticle 428, source decal 34, authored/local decal 3, trail 12, standalone sprite 2다.
StandardColorV1 capability는 존재하지만 현재 4캐릭터 Product consumer는 0이다.

## 2. V1 target architecture

### 2.1 한 element가 pixel이 되는 계약

```text
EffectAssetId + stable ElementId
  -> Composition / SourceRecipe
  -> Carrier
  -> child MIC effective override
  -> parent Material + static permutation
  -> programId + layoutId + descriptorId
  -> per-occurrence values/resources/dynamics
  -> shared carrier adapter
  -> Bind -> Begin -> Draw -> state restore
```

HLSL은 program equation을 소유한다. Descriptor는 CB/SRV/sampler/channel/color space와 render
state를 소유한다. Adapter는 geometry/VF/pass/scene input/RT topology와 draw transaction을 소유한다.
Authored document는 stable element, composition, carrier와 admitted program/descriptor 선택을 소유한다.

### 2.2 정본 데이터 계약

G00에서 다음 generated inventory를 추가한다.

```text
Data/Effects/Contracts/effect-v1-migration-inventory.v1.json
Tools/EffectPipeline/Schemas/lostark.effect-v1-migration-inventory.schema.json
Tools/EffectPipeline/build_effect_v1_migration_inventory.py
Tools/EffectPipeline/test_build_effect_v1_migration_inventory.py
```

각 live element 행은 최소 다음 필드를 가진다.

```text
domain / effectAssetId / elementId / cue reachability
carrier kind / source occurrence / model or texture resources
child material / parent material / static set / program hash
current backend / opcode / pass / render profile
programId / layoutId / descriptorId / adapterId
runtime architecture / disposition / fidelity evidence
evidence blockers / Product admission / visual review
source-to-V0 merge disposition / lineage predecessor / replacement / retirement reason
```

G01에서는 discovery용 `ue3-material-family-registry.v1.json`과 별도로 admitted runtime descriptor를
소유할 단일 registry를 추가한다.

```text
Data/Effects/Contracts/effect-material-program-registry.v1.json
Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json
```

이 registry는 다음을 소유한다.

```text
program identity and translated HLSL asset
packet/CB layout and named lane AST
native t/s wire, channel, color space, sampler
VF/pass input signature and dynamic semantic map
scene input and RT/MRT/coverage topology
blend/cull/depth/stencil state
supported carrier adapter
evidence receipt and Product admission status
```

파일 안에서는 `program`, `layout`, `descriptor`, `admission` 네 collection을 분리한다. Program은
equation identity, layout은 GPU 전달 계약, descriptor는 occurrence의 effective values/resources/state,
admission은 stable element가 세 ID를 소비할 자격을 소유한다.

`ue3-material-family-registry.v1.json`은 계속 discovery intake이고, 새 registry가 Product admission
정본이다. occurrence 선택을 두 번째 binding 파일에 중복하지 않고 Authored element의 material
execution이 stable registry ID를 참조한다. `CEffect_Catalog`이 parse→validate→stage→commit하고
`CEffect_PresentationService`가 main-thread prewarm한 뒤 `CEffect_DocumentRenderer`가 소비한다.

### 2.3 migration 중 fallback 정책

- V0 element는 자기 V1 replacement가 Product admission되기 전까지 그대로 유지한다.
- Tool canary는 default OFF, stable occurrence만 replacement하고 실패하면 fail-close한다.
- V1 packet을 소유한 뒤 draw가 실패하면 generic family-lite로 조용히 떨어지지 않는다.
- 한 verified tuple의 Product admission은 다른 element의 V0 상태를 바꾸지 않는다.
- Product rollout은 staged document 전체가 성공한 뒤 generation을 원자적으로 교체한다.
- rollback은 직전 sealed V0/V1 document와 catalog generation을 보존한다.

## 3. family·adapter migration lanes

### Lane A. Standard radiance / coverage / dissolve

대상은 base radiance와 coverage 의미가 명시 가능하지만 grouped heuristic 때문에 사각 카드,
black-card, 잘못된 alpha가 생기는 대다수 Sprite/Mesh Particle이다.

첫 canary와 확장 기준:

- 도화가 A noise influence 0, S grass-tip dissolve
- 창술사 Q/A 검격과 WaterTrail/MissileTrail cohort
- 워로드 사슬/방패/일반 충격 particle
- Valtan generic sprite/mesh의 source-backed coverage cohort

`StandardColorV1`은 explicit lane/channel이 맞는 행에 재사용한다. UV graph, dynamic parameter,
scene input이 다른 family를 StandardColorV1로 억지로 축약하지 않는다.

### Lane B. Local Decal / projected ground

대상:

- 도화가 R symbol revolution과 F ground decal
- 워로드 Full Barrel T의 exact 4 decal
- Valtan Product decal 21행과 Ground04 canary

texture를 바꿔 Sprite를 decal로 만들지 않는다. projector carrier, decal VF/pass, depth reconstruction,
material equation과 state를 분리해 닫는다. Valtan 21행은 현재 source material identity가 비어 있으므로
Ground04 하나를 전체 21행의 equation이라고 가정하지 않는다.

### Lane C. UV flow / masked / dragon / helix

대상:

- 창술사 V screw MakeFlow, T와 용 cohort의 DragonMasked UV
- 도화가 T MakeFlow와 미르세김의 pixel material 부분
- Valtan wave, smoke, dissolve, masked mesh cohort

UV start, time origin, pan/tiling, DynamicParameter, SubUV, wrap/clamp, masked clip을 typed,
fidelity-qualified descriptor로
복원한다. CModel 용 animation owner와 Mesh/Sprite material을 같은 family로 합치지 않는다.

### Lane D. Glass / refraction / scene feedback / MRT

대상:

- 차원술사 W Glasshole, FluidNinja, CustomParticle, CrackHole와 관련 glass cohort
- 차원술사 F 반구, world shard, screen shard와 scene feedback
- 구조적으로 같은 verified program/layout을 공유하는 차원술사 glass skill 확장

Glasshole02 Tool canary를 첫 seed로 쓰되 source sampler, actual VF/pass, scene CB, MRT 0/2/3/4/5,
coverage와 사용자 A/B가 닫히기 전에는 Product로 올리지 않는다. Glass HLSL 하나를 모든 glass 이름에
공유하지 않고 program/layout/descriptor tuple가 구조적으로 같은 cohort만 확장한다.

### Lane E. Trail / Ribbon / history geometry

대상:

- 도화가 CascadeRibbon
- 창술사 weapon trail과 검격 trail
- Valtan trail/ribbon Product 20행

geometry/history sampling proof와 material equation/VF proof를 별도 gate로 둔다. V0에서 trail이
보인다는 사실은 width basis, tangent, UV distance, source material이 exact하다는 뜻이 아니다.

### Lane F. Light / ScreenPost / screen-space presentation

대상:

- 도화가 V attractor와 화면 계층
- 워로드 F/T의 RGBNoise, Light, ScreenPost
- 차원술사 F screen overlay와 glass shard
- Valtan Light 1행과 후속 source presentation

Light와 ScreenPost는 material HLSL 행이 아니라 typed presentation ABI다. scene ping-pong,
Target_Depth/Color, overlay lifetime, cancellation과 state rollback을 별도 adapter로 닫는다.

### Lane G. ModelCue / animation-owned presentation

대상:

- 도화가 D/E/T의 범·두루미·미르세김 CModel animation
- 차원술사 summon ModelCue

ModelCue는 material family가 아니다. model/clip/trajectory, root/bone attachment, runtime catalog join,
animation frame 진행을 typed presentation으로 검증한다. 용의 body animation과 용 표면 UV shader는
서로 다른 lane으로 닫는다.

## 4. 기존 결함·손튜닝의 V1 이관

V1은 V0에서 끝낸 composition/gameplay 수직 슬라이스를 다시 흔들지 않는다.

| domain | V0에서 보존할 기준 | V1이 바꿀 부분 |
|---|---|---|
| 도화가 | A/S/R materialization, D catalog join, E crane path, T/V source composition, F 17-element golden | coverage, decal, flow, dragon surface, screen/presentation ABI. S grass-tip의 emissive+bloom 대 실제 Light는 canary interview로 확정 |
| 워로드 | A chain save invariant 수정, W action-facing, T noise/decal authored 복원 | chain/shield/impact material, LocalDecal, Light/ScreenPost ABI. Full Barrel은 slot 이름이 아니라 stable skill/effect ID로 target 고정 |
| 창술사 | Q exact mesh 유지, A exact row identity 선택·save/publish 경로, E cone transplant, dragon authored cohort | slash coverage, flow/dragon UV, trail/VF/material ABI. A의 다음 cast transform 반영은 사용자 `PENDING` |
| 차원술사 | LMB clock/combo, A action-facing과 4-hit occurrence/timing, T Server target, F overlay binding | A/S/standard coverage, Glass/refraction/MRT, shard와 presentation ABI |
| Valtan | 33 pattern/130 semantic stage, 108 catalog docs/3,683 live V0 graph, hand-tuned composition | 108/3,683 전체의 material/presentation ABI |

대부분의 V1 Product patch는 stable ID, transform, timing, source recipe를 deep-equal로 유지하고
material program/descriptor 참조만 바꾼다. Composition 변경이 필요한 경우 material migration과
같은 PR에 몰아넣지 않고 별도 typed receipt와 사용자 판정을 가진다.

위 표의 V0 기준은 자동 연결·데이터 보존 기준이다. Artist F golden을 제외한 visual fidelity와
사용자 손튜닝 결과의 최종 승인은 `PENDING`을 유지한다.

### 4.1 V0 hand-tuning snapshot을 먼저 봉인한다

V1은 현재 Authored document를 제자리에서 bulk regenerate하지 않는다. G00은 각 canonical document와
element의 semantic hash를 V0 snapshot receipt로 봉인한다.

```text
v0DocumentHash / v0ElementHash
stable element ID / source occurrence ID
transform / timing / attachment / SourceRecipe hash
material/resource hash
visible / disabled state
```

Matched element의 transform, timing, attachment와 사용자가 조정한 composition은 V0 snapshot이
우선한다. V1은 program/layout/descriptor만 selective replacement한다. Candidate 적용 중 unrelated
field가 바뀌거나 expected V0 hash가 다르면 CAS처럼 전체 stage를 거부하고 최신 문서를 유지한다.

### 4.2 V0에서 삭제한 source element를 복구하는 3-way merge

V0 손튜닝 중 사각 카드, 잘못된 alpha/UV, 미지원 carrier 때문에 지운 source occurrence는 V1의 핵심
복구 대상이다. 현재 live 5,294행만 변환하면 이를 영구 누락시키므로 다음 3-way ledger를 사용한다.

```text
원본 source occurrence 전체
+ 현재 V0 user-tuned Product document
+ 새 V1 program/layout/descriptor
= Tool-only V1 restoration candidate
```

ledger는 source와 current V0를 full outer join하며 각 행에 nullable
`sourceOccurrenceId? / currentElementId?`를 둔다. source-side 행은 다음 merge state 중 하나를 가진다.

| merge disposition | 처리 |
|---|---|
| `MATCHED_EXISTING` | V0 composition을 보존하고 V1 material/presentation ABI만 교체 |
| `SOURCE_MISSING_IN_V0` | 원본 carrier/resource/timeline과 V1 ABI로 deterministic candidate를 다시 생성 |
| `MERGED_OR_SUBSTITUTED_IN_V0` | 어느 source row를 되살릴지 사용자 interview 전 fail-close |
| `INTENTIONALLY_DISABLED` | 삭제하지 않고 disabled/retirement receipt와 사용자 결정을 기록 |
| `AMBIGUOUS_SOURCE_JOIN` | Product materialization 금지, evidence blocker 유지 |

source가 없고 current V0에만 있는 행은 merge state가 아니라 current-only lineage를 가진다.

| current-only lineage | 처리 |
|---|---|
| `V0_PROJECT_AUTHORED` | 현재 composition을 baseline Product에 한 번만 유지하고 fidelity를 `PROJECT_RECONSTRUCTED`로 기록 |
| `CURRENT_ORPHAN_BLOCKED` | provenance가 확인될 때까지 V1 Product replacement와 retirement를 모두 금지 |

`SOURCE_MISSING_IN_V0`는 family renderer가 닫히기 전에는 Product Authored 파일에 바로 넣지 않는다.
Imported evidence 아래 candidate와 기존 Tool preview projection으로 Complete/Solo A/B를 제공한다.
사용자가 원본 구성 복구를 승인하면 stable source-derived ID로 같은 Product document에 selective insert하고,
승인하지 않으면 `INTENTIONALLY_DISABLED` receipt로 남긴다.

우선순위는 다음과 같다.

```text
source identity/carrier/material evidence -> 원본 source
matched transform/timing/attachment        -> 현재 V0 손튜닝
pixel equation/runtime ABI                 -> V1 registry
Product 포함 여부와 최종 손튜닝            -> 사용자 승인
```

따라서 도화가 F처럼 빠르게 올린다는 것은 현재 행만 shader로 바꾸는 것이 아니라, family를 하나씩
닫은 직후 그 family 때문에 과거에 지웠던 source cohort를 함께 복구·Solo 검토·Product 승격한다는 뜻이다.

`5,294`는 현재 live V0 기준선이지 최종 V1 Product element 수가 아니다. G00 builder는 다음 식으로
최종 행 분모를 매번 생성한다.

```text
generatedFinalReachableRows
  = baselineCurrentProductRows
  - approvedRetiredBaselineRows
  + approvedRestoredSourceRows
  + approvedJoinRequiredRows
```

matched, accepted substitute와 `V0_PROJECT_AUTHORED`는 baseline에 이미 포함되므로 다시 더하지 않는다.
거부되거나 disabled인 `SOURCE_MISSING_IN_V0`도 baseline에서 빼지 않는다. final asset 분모는 행 수를
더해서 구하지 않고 최종 stable effectAssetId 집합의 union으로 계산한다. 따라서 삭제 occurrence를
되살려 Product 행 수가 증가하는 것은 회귀가 아니며, source lineage, 사용자 승인과 before/after
composition receipt 없는 증가는 회귀로 처리한다.

`generatedFinalReachableRows`는 다시 `Character active Product`, `Valtan active Product`,
`disabled but still reachable`로 partition한다. 전역 `generatedFinalActiveProductRows`는 두 active
domain의 합이며, G09와 G10은 자기 domain 분모만 소비한다. 승인된 retirement는 reachable 식에서
차감된 뒤 별도 `generatedFinalRetiredRows`와 terminal receipt로 센다. screen/Light/ModelCue는 element
행에 중복 합산하지 않고 `generatedFinalPresentationRows`로 유지한다.

속도를 위해 character 전체를 한 번에 bulk regenerate하지 않는다. 한 program/layout family가 닫히는
PR마다 다음 두 묶음을 함께 stage한다.

```text
현재 V0에서 그 family를 쓰는 matched elements의 typed replacement
+ V0에서 삭제됐지만 같은 family 증거로 복구 가능한 source occurrences의 Tool-only candidates
```

이렇게 하면 renderer/HLSL을 한 번 뚫을 때 현재 행과 과거 삭제 행을 동시에 회수하면서도, 사용자가
승인하지 않은 원본 element가 Product에 자동 삽입되는 일을 막을 수 있다.

## 5. G별 구현 순서

### G00. V0 freeze와 전체 migration ledger

목표:

- live 205/5,294를 deterministic하게 재생성한다.
- direct/canonical corpus 214/5,390과 live 사이 4캐릭터 9/96의 join/disposition을 확정한다.
- catalog 316/7,550과 live 차이를 분류한다.
- source occurrence→current element→V1 lineage crosswalk를 만든다.
- V0 document/element snapshot hash와 3-way merge disposition을 봉인한다.
- ModelCue, screen overlay, Light 같은 비-material presentation 분모를 별도로 센다.
- `baselineAssets/Rows`, `generatedFinalReachableAssets/Rows`,
  `generatedFinalCharacterActiveProductAssets/Rows`,
  `generatedFinalValtanActiveProductAssets/Rows`, `generatedFinalActiveProductAssets/Rows`,
  `generatedFinalDisabledRows`, `generatedFinalRetiredRows`, `generatedFinalCatalogNonLiveAssets/Rows`,
  `generatedFinalPresentationRows`를 서로 다른 필드로 산출한다.
- Product-reachable/eligible source와 raw evidence-only source를 분리한다. Valtan 9,434 raw candidate 같은
  evidence pool은 `NON_PRODUCT_EVIDENCE` 범위와 receipt 없이는 final restoration 분모에 들어오지 않는다.

검증:

- missing catalog/document/element ID, duplicate live ID, non-finite transform을 거부한다.
- 같은 입력에서 artifact SHA가 동일해야 한다.
- 현재 main fixture가 `205/5,294/46 typed seed/5,248 rows without execution`을 재현해야 한다.
- 4캐릭터 fixture가 direct 106/1,707, live 97/1,611, unjoined 9/96을 재현해야 한다.
- Valtan fixture가 pattern cue 106/3,665 + combat-object visual 2/18 = live 108/3,683을 재현해야 한다.

종료 증거: 모든 current row는 lineage/disposition을 가진다. 모든 source candidate는 stable inventory
row와 명시적 blocker를 가지며, G00이 9,434 Valtan candidate를 임의 해석 완료로 만들지 않는다.
G00에서는 ambiguity를 억지로 0으로 만들지 않고 eligible source의 merge state 또는 stable blocker
coverage를 100%로 만든다. `AMBIGUOUS_SOURCE_JOIN`, `MERGED_OR_SUBSTITUTED_IN_V0` 해소와
restore/retain/retire terminal decision 100%는 해당 family A/B와 사용자 interview 뒤 G11에서 요구한다.

### G01. Program/layout/descriptor registry

목표:

- current backend/opcode를 stable program/layout/descriptor ID로 감싼다.
- existing RuntimeMaterialV2, ArtistVisualV4, LocalDecal, StandardColorV1을 legacy-compatible seed로
  registry에 넣는다.
- Glasshole와 Valtan hard-coded canary contract를 같은 registry schema로 표현하되 Product flag는
  false로 유지한다.
- `Publish-Effects.ps1`이 authoring registry를 version/hash가 봉인된
  `EffectCatalog.runtime.json.materialPrograms` section으로 생성한다. Client는 Contracts JSON을 직접
  읽지 않고 EffectCatalog generation과 함께 parse→validate→stage→commit한다.
- `programId`는 build-time에 컴파일·등록된 shader/technique만 resolve한다. Product runtime HLSL
  source compile은 금지한다.
- program/layout generation별 shader object와 immutable resource cache는 main-thread prewarm하며,
  하나라도 실패하면 새 catalog generation 전체를 commit하지 않는다.

수정 시작점:

```text
Client/Public/Effect_AuthoringDocument.h
Client/Public/Effect_MaterialTemplate.h
Client/Public/Effect_Catalog.h
Client/Private/Effect_Catalog.cpp
Client/Private/Effect_DocumentCodec.cpp
Client/Private/Effect_PresentationService.cpp
Client/Public/Effect_DocumentRenderer.h
Client/Private/Effect_DocumentRenderer.cpp
Tools/EffectPipeline/Publish-Effects.ps1
```

종료 증거: unknown ID, duplicate ID, incompatible layout/carrier/pass, hash drift와 partial resource stage가
모두 기존 generation을 유지하며 실패한다. 같은 programId의 shader object 중복 생성은 0이어야 한다.

### G02. Shared carrier adapter와 fail-closed packet binder

목표:

- Sprite RT0, Sprite scene/MRT, Mesh RT0, Mesh scene/MRT, LocalDecal, Trail/Ribbon의 공용 scheduling
  skeleton을 descriptor-driven으로 만든다.
- 하나의 mega packet을 만들지 않고 layout별 packet을 선택한다.
- Tool-only canary의 state save/restore와 stable occurrence gate를 공용 transaction으로 승격한다.

종료 증거:

- synthetic packet에서 CB/SRV/sampler/VF/pass/RT/state mismatch가 모두 거부된다.
- draw ownership을 획득한 뒤 실패하면 generic fallback이 발생하지 않는다.
- Glasshole와 Valtan current canary가 byte/equation 결과를 유지한다.

### G02a. 공용 DXBC→HLSL program materialization

목표:

```text
child MIC + parent + effective static set
-> ShaderMap + VF/pass program identity
-> cooked DXBC + native binding wire
-> translated HLSL or reviewed reconstruction
-> WARP/sensitivity receipt
-> program registry admission candidate
```

모든 discovered target은 `REUSED`, `TRANSLATED`, `PROJECT_RECONSTRUCTED`, `NON_PIXEL`, `BLOCKED`
중 하나를 가진다. HLSL 파일 수를 child material 수와 같게 만들지 않고 distinct program identity만
materialize한다. Source가 없는 reconstruction은 DXBC parity 대신 deterministic input sensitivity와
explicit assumption receipt를 요구한다.

종료 증거: program ID, source/equation hash, HLSL compile identity와 fidelity status가 서로 일치하고,
unknown program candidate가 Product admission으로 넘어가지 않는다.

G03~G08의 모든 cohort receipt는 공통으로 다음 네 축을 따로 기록한다.

| 축 | cohort 종료 상태 |
|---|---|
| architecture | `V1_TYPED_CANDIDATE`, `V1_TYPED_PRODUCT` 또는 `V1_TYPED_PRESENTATION_PRODUCT` |
| fidelity | `SOURCE_EXACT`, `BOUNDED_TRANSLATED`, `PROJECT_RECONSTRUCTED`, `NOT_APPLICABLE` 중 known value |
| runtime admission | `TOOL_ONLY` 또는 `PRODUCT_ADMITTED`; Product 종료에는 후자 필요 |
| visual | `USER_REVIEW_PENDING`, `USER_APPROVED`, `USER_WAIVED`, `NOT_REQUIRED` |

`COHORT_AUTOMATED_COMPLETE`는 architecture, fidelity, runtime admission과 build/rollback 증거가 닫힌
상태다. `COHORT_VISUAL_APPROVED`는 사용자가 직접 `USER_APPROVED`를 준 상태다. `USER_WAIVED`는 review
closure일 뿐 fidelity PASS가 아니다. G09/G10은 두 상태를 별도 집계한다.

### G03. Standard color/coverage 대량 cohort

목표:

- Lane A의 role/channel/color-space **후보**를 원본 wire와 sensitivity로 감사한다.
- distinct program/layout별 대표 Sprite RT0와 Mesh RT0를 canary로 닫는다.
- program/layout/descriptor tuple의 구조적 동등성이 증명된 cohort만 registry row로 확장한다.
- 구조적 동등성과 fidelity를 분리한다. 같은 tuple을 재사용해도 source evidence가 없으면
  `PROJECT_RECONSTRUCTED`이며 자동으로 `SOURCE_EXACT`가 되지 않는다.

Review coverage anchor:

```text
도화가 A/S
-> 창술사 Q/A
-> 워로드 standard impact/chain/shield
-> 차원술사 standard slash
-> Valtan reviewed standard cohorts
```

이 목록은 StandardColorV1 소비자를 미리 확정하거나 class 순서로 Product를 bulk rollout하는 목록이
아니다. 도화가 A/S와 창술사 Q/A도 우선 감사 대상으로만 두며, 실제 적용 순서는 verified
program/layout/descriptor cohort가 결정한다.

종료 증거: 대상 cohort의 rectangular/black-card heuristic이 제거되고, lifetime/dissolve와
base radiance/coverage sensitivity harness가 통과하며 사용자 canary A/B가 기록된다. 같은 family로
분류되어 V0에서 삭제한 source occurrence도 Tool-only 복원 후보 또는 명시적 disposition을 가진다.
Product cohort 종료에는 공통 4축 receipt의 `PRODUCT_ADMITTED + USER_APPROVED`가 필요하다.

### G04. LocalDecal projector 전체 승격

목표:

- Ground04 bounded canary를 실제 decal VF/pass와 검증된 sampler/scene CB에 연결한다. source evidence가
  모두 닫히면 `SOURCE_EXACT`, Winters RT0 projector로 경계를 명시하면 `BOUNDED_TRANSLATED`다.
- 도화가 R/F, 워로드 Full Barrel 4행, Valtan 21행을 typed descriptor와 명시적 fidelity별로 분리한다.

종료 증거: ground projection, depth intersection, rotation/revolution, lifetime fade, state rollback이
Debug/Release에서 통과하고 typed descriptor, fidelity receipt와 사용자 visual 승인을 가진 cohort만
Product에 들어간다. 원본 decal이 V0에서 삭제됐다면 같은 gate로 stable source-derived candidate를
복구하고 공통 4축 receipt를 기록한다.

### G05. UV flow/masked/dragon/helix

목표:

- MakeFlow와 DragonMasked의 time origin, dynamic parameter, sampler와 masked coverage를 닫는다.
- 창술사와 도화가가 구조적으로 같은 program/layout/descriptor tuple임을 증명한 경우에만 registry를
  재사용한다. 재사용 자체는 두 occurrence의 fidelity 등급을 같게 만들지 않는다.
- Valtan Dissolve/Smoke/Wave를 같은 layout 위의 별도 program/descriptor로 연결한다.

종료 증거: 0/0.25/0.5/1.0 lifetime과 30/60/120FPS에서 UV phase가 안정적이고, facing/scale 변화와
SubUV 시작 frame이 각 occurrence의 source 또는 reconstruction receipt를 따른다. 용·helix family 때문에
V0에서 삭제한 Sprite/Mesh source rows도 Tool-only 복원 후보로 함께 생성한다. Product cohort 종료에는
known fidelity, `PRODUCT_ADMITTED`와 사용자의 UV/coverage `USER_APPROVED`를 각각 기록한다.

### G06. Glass/refraction/MRT

목표:

- Glasshole02의 source sampler, actual VF/pass, scene CB와 MRT/coverage를 닫는다.
- FluidNinja, CustomParticle, CrackHole와 DimensionMaster W/F의 typed variants를 program registry에
  추가한다.

종료 경로는 둘이다.

- native VF/pass, scene input과 MRT 전체가 닫히면 `SOURCE_EXACT` 후보로 승격한다.
- Winters RT0 projection이 의도한 제품 목표라면 손실 범위와 output mapping을 receipt로 봉인하고
  `BOUNDED_TRANSLATED`로 사용자 승인을 받는다.

어느 경로든 DXBC↔HLSL numeric/spatial A/B, depth/camera/time sensitivity, RT metadata, Tool user A/B와
Product rollback이 모두 닫힌 뒤 W 첫 cohort를 Product에 admission한다. source가 없어 HLSL을 재구성한
variant는 sensitivity receipt와 사용자 승인으로 `PROJECT_RECONSTRUCTED`가 될 수 있으나 source-exact라고
표기하지 않는다. 공통 4축 receipt에서 native/bounded/reconstructed fidelity와 Product/visual 상태를
서로 대신하지 않는다.

### G07. Trail/Ribbon

목표:

- Artist ribbon, Lance weapon trail, Valtan 20행을 source topology/material tuple로 분류한다.
- point history, width/tangent, distance UV와 material packet을 함께 검증한다.

종료 증거: carrier geometry receipt와 material receipt가 모두 있는 verified cohort만 Product에 들어간다.

여기서 verified cohort는 geometry/material tuple의 구조적 동등성을 뜻한다. source evidence가 없는
project ribbon도 typed history geometry, fail-closed packet, 명시적 fidelity와 사용자 승인을 닫으면
Product가 될 수 있다. V0에서 카드나 잘못된 trail로 보여 삭제한 source row도 같은 topology cohort에서
복원 후보로 회수하며 공통 4축 receipt를 기록한다.

### G08a. Light presentation

목표:

- 도화가 S/V, 워로드 F/T와 Valtan Light를 material emissive와 분리한 typed Light ledger로 닫는다.
- lifetime, attenuation, color/intensity, owner cancellation과 level change rollback을 검증한다.

종료 증거: Light 생성 실패, 취소와 level 전환이 다른 draw/scene state를 보존하고 사용자 판정 전
visual PASS로 올라가지 않는다. architecture는 presentation Product, fidelity는 `NOT_APPLICABLE`,
runtime admission과 visual은 공통 4축 receipt로 분리한다.

### G08b. ScreenPost / overlay / shard presentation

목표:

- 워로드 F/T RGBNoise, 도화가 V 화면 계층, 차원술사 F overlay/shard를 별도 screen adapter로 닫는다.
- scene ping-pong, depth/color input, draw order, lifetime, cancel과 resource failure rollback을 검증한다.

종료 증거: screen presentation 하나의 실패가 다른 scene target이나 presentation을 오염시키지 않고,
사용자가 직접 화면 강도·범위·타이밍을 승인한다. screen material program이 있으면 그 program fidelity를
별도로 기록하고 presentation adapter 자체는 공통 4축 receipt를 가진다.

### G08c. ModelCue / animation-owned presentation

목표:

- 도화가 D/E/T와 DimensionMaster summon의 model/clip/trajectory/root·bone attachment를 material
  program과 분리된 typed ModelCue로 닫는다.
- model animation과 표면 pixel material의 독립적인 stage/rollback을 검증한다.

종료 증거: clip 또는 model resource 실패 시 해당 cue만 격리되고, 성공 시 실제 frame 진행과 attachment를
사용자가 직접 판정한다. ModelCue는 `NOT_APPLICABLE` fidelity의 typed presentation으로 공통 4축
receipt를 가진다.

### G09. 4캐릭터 V1 domain closure

목표:

- G03~G08에서 증명된 typed lineage를 4캐릭터 domain closure audit로 집계한다.
- class 순서가 아니라 verified tuple cohort 순서로 rollout한다.
- 각 class의 모든 skill effect를 composition smoke matrix로 닫고, source에서 삭제된 occurrence의
  Product 복구 여부를 사용자와 확정한다.

live 1,611행 중 visible 1,568행은 `V1_TYPED_PRODUCT` 또는 `V1_TYPED_PRESENTATION_PRODUCT` lineage와
명시적 fidelity를 가져야 한다. hidden 43행은 강제로 보이게 하지 않고 `DISABLED_WITH_RECEIPT` 또는
`RETIRED` disposition을 갖는다. 현재 live가 project-reconstructed인 경우에도 typed ABI, fail-closed
Product admission과 사용자 승인을 닫으면 유지할 수 있다.

frozen baseline closure 분모:

| class | live assets | live elements | active rows requiring V1 Product architecture | hidden rows requiring terminal disposition |
|---|---:|---:|---:|---:|
| Artist | 19 | 377 | 338 | 39 |
| Warlord | 22 | 463 | 463 | 0 |
| LanceMaster | 41 | 501 | 501 | 0 |
| DimensionMaster | 15 | 270 | 266 | 4 |
| **합계** | **97** | **1,611** | **1,568** | **43** |

이 1,611은 현재 live closure 분모다. G00이 산출한 `SOURCE_MISSING_IN_V0` 가운데 사용자가 복구를
승인한 행은 별도 generated restoration denominator에 더해진다. 반대로 명시적으로 retirement된 행은
receipt를 가지므로 “누락”으로 세지 않는다. 최종 G09는 frozen 1,611을 그대로 요구하지 않고,
`generatedFinalCharacterActiveProductRows`가 모두 V1 typed+Product-admitted+known fidelity인지, 해당
domain의 disabled/retired 행이 모두 terminal disposition인지 검사한다.

### G10. Valtan V1 domain closure

목표:

- FRONT_BACK_FRONT core-three 9행을 full ABI/Product로 먼저 닫는다.
- LocalDecal 21, MeshParticle 2,088, SpriteParticle 1,552, Trail/Ribbon 20, Mesh 1, Light 1을
  verified tuple cohort로 순차 승격한다.
- boss-root pattern cue 106 assets/3,665 elements와 combat-object visual 2 assets/18 elements, 즉 live
  108/3,683 전체의 lineage를 닫는다.

Valtan Product admission은 family별 구현 후 pattern/effect 또는 combat-object visual 단위로 원자
적용한다. active 행은 typed Product architecture와
`SOURCE_EXACT | BOUNDED_TRANSLATED | PROJECT_RECONSTRUCTED | NOT_APPLICABLE` 중 known fidelity를 가진다.
source inventory의 branch 후보를 element 수를 채우기 위해 강제 materialize하지 않으며, V0에서 삭제된
source occurrence는 4캐릭터와 같은 3-way gate로만 복구한다.

G10 종료 시 모든 canonical/source-restoration row는 `V1 Product lineage` 또는 terminal disposition 중
하나를 가진다. `generatedFinalValtanActiveProductRows`는 모두 V1 typed, `PRODUCT_ADMITTED`, known
fidelity여야 하며 disabled/retired 행을 Product lineage 수를 채우기 위해
강제로 활성화하지 않는다.

### G11. V0 Product fallback 종료와 최종 publish

목표:

- frozen V0 fixture `205 assets/5,294 rows`가 계속 재현되는지 확인한다.
- `generatedFinalActiveProductRows`의 `V0_COMPATIBILITY`와 V1 lineage 누락을 0으로 만든다.
- `generatedFinalCatalogNonLiveRows`의 terminal disposition 누락을 0으로 만든다.
- Product runtime에서 hard-coded skill/class canary selector를 제거하고 stable registry ID만 소비한다.
- V0 shader/path는 explicit legacy/debug compatibility가 필요할 때만 남기고 Product cue에서 제거한다.
- `SOURCE_MISSING_IN_V0`의 승인·retirement·blocker 미결을 0으로 만들고, 승인된 복원 행을 generated
  final Product denominator에 포함한다.

종료 증거:

```text
frozen V0 baseline fixture                 205 assets / 5,294 rows
generated final Product join failures      0
active Product rows without V1 Product architecture  0 / generatedFinalActiveProductRows
active Product rows without known fidelity           0 / generatedFinalActiveProductRows
active Product rows not PRODUCT_ADMITTED              0 / generatedFinalActiveProductRows
generated reachable rows without lineage or terminal disposition  0 / generatedFinalReachableRows
generated disabled rows without terminal disposition              0 / generatedFinalDisabledRows
generated retired rows without terminal disposition               0 / generatedFinalRetiredRows
generated final active Product V0_COMPATIBILITY       0
generated catalog non-live without terminal disposition  0 / generatedFinalCatalogNonLiveRows
eligible source AMBIGUOUS_SOURCE_JOIN                 0
eligible MERGED_OR_SUBSTITUTED_IN_V0 unresolved       0
eligible source restore/retain/retire terminal coverage 100%
raw evidence candidates without NON_PRODUCT_EVIDENCE scope receipt 0
presentation rows without V1 typed Product architecture  0 / generatedFinalPresentationRows
presentation rows without known fidelity incl. NOT_APPLICABLE 0 / generatedFinalPresentationRows
presentation rows not PRODUCT_ADMITTED     0 / generatedFinalPresentationRows
presentation build/prewarm/rollback failures 0
unknown program/layout/descriptor IDs      0
Product Tool-only canary flags             0
Product selector resolving TOOL_ONLY       0
generated active Product/presentation USER_REVIEW_PENDING rows 0
publisher/runtime generation mismatch      0
user-review-required rows falsely PASS     0
generated final Product asset composition review closed  generatedFinalActiveProductAssets / generatedFinalActiveProductAssets
USER_WAIVED composition count              separately reported
program/fidelity ledger unknown            0
Artist legacy no-authored entry disposition 1 / 1
duplicate shader objects per program generation  0
prewarm failures at committed generation   0
unreceipted occurrence/draw cardinality increase 0
```

성능과 transaction 증거에는 migration 전후 submitted draw, failed draw, state switch, shader object와
resource cache 수치를 같이 기록한다. 행 수와 draw 수가 증가했다면 source 복구 receipt로 설명되어야 한다.

최종 상태를 둘로 나눈다.

- `V1_AUTOMATED_MIGRATION_COMPLETE`: lineage/disposition, registry, build/publish/rollback과 위 자동 분모가
  모두 닫힌 상태
- `V1_VISUAL_REVIEW_CLOSED`: baseline과 generated final Product/presentation의 모든 required review가
  `USER_APPROVED` 또는 `USER_WAIVED`인 상태
- `V1_VISUAL_FIDELITY_COMPLETE`: 같은 generated final 분모의 required review가 모두
  `USER_APPROVED`이며 waiver가 0인 상태

전체 `V1_COMPLETE`는 `V1_AUTOMATED_MIGRATION_COMPLETE`와 `V1_VISUAL_FIDELITY_COMPLETE`를 모두
만족할 때만 선언한다. `PROJECT_RECONSTRUCTED` Product도 자동 gate와 `USER_APPROVED`를 모두 요구한다.
`V1_VISUAL_REVIEW_CLOSED`만으로 완벽 복원을 선언하지 않는다.

## 6. 각 canary와 cohort의 admission gate

모든 G03~G10 cohort는 적용 가능한 다음 순서를 생략하지 않는다.

1. Product cue → effect asset → stable element identity
2. source emitter/carrier/mesh/texture/timeline join
3. child/parent/effective static set/ShaderMap 선택
4. cooked material이 있으면 DXBC와 translated HLSL equation parity. source가 없으면 reconstruction
   receipt와 deterministic sensitivity, non-pixel presentation이면 `NOT_APPLICABLE`
5. CB lane, SRV wire, sampler, channel/color space, dynamic parameter
6. actual VF/pass, scene input, RT/MRT/coverage와 render state
7. Tool이 지원되는 pixel family는 default-off stable occurrence canary. Tool에서 표현할 수 없는
   Product presentation은 사용자 조작 가능한 Product audition 경로
8. Debug/Release compile, first draw, state/rollback proof
9. 사용자 Tool A/B와 손튜닝
10. Product staged publish/restart admission
11. 구조적으로 같은 program/layout/descriptor closure가 증명된 cohort만 registry 확장하고 fidelity는
    occurrence별 evidence로 유지

한 gate의 PASS를 다음 gate로 대신하지 않는다. 특히 texture parity, DXBC 확보, HLSL 번역, first draw,
사용자 visual과 Product admission은 서로 다른 상태다.

## 7. PR과 main 통합 단위

V1 전체를 한 PR로 올리지 않는다. 과거 #141의 203 files/46 regression 결합을 반복하지 않도록 다음
한 수직 슬라이스만 한 PR에 담는다.

```text
one stable canary or one proven cohort
+ registry/descriptor data
+ required C++/HLSL adapter
+ focused receipt/harness
+ selective authored Product rows
+ publisher output contract
+ PLAN/RESULT update
```

- 다른 cohort의 authored rows를 bulk regenerate하지 않는다.
- 사용자 승인 전 canary PR은 `productMutation=false`를 유지한다.
- Product PR은 승인된 stable IDs만 selective mutation한다.
- PR merge 뒤 최신 main에서 다음 cohort branch를 만든다.
- 다른 세션의 dirty worktree나 hand-tuned rows를 자동 정리하지 않는다.

## 8. 자동 검증

ClientFrontendHarness는 삭제된 정본이므로 다시 추가하지 않는다. 각 변경은 다음 focused gate를 쓴다.

### 모든 G

- JSON/schema parse
- migration inventory `--check`
- registry/receipt deterministic hash check
- 관련 Python focused tests
- `Publish-Effects.ps1 -Mode Validate`
- `Sync-EffectDataProject.ps1 -Check`
- `git diff --check`

### HLSL/program G

- source DXBC identity와 translation receipt
- WARP numeric/spatial parity
- Debug/Release shader compile
- CB/SRV/sampler/channel sensitivity
- invalid/missing resource fail-close

### C++ adapter/Product G

- Client x64 Debug/Release
- Engine public 변경 시 Engine Debug/Release → `UpdateLib.bat` → Client Debug/Release
- stage-all/commit, failure rollback, state restore focused test
- Effect publisher Validate/Publish와 sealed runtime identity check
- gameplay/public packet을 건드린 경우에만 Shared/Server 정본 build와 해당 contract test

정본 전체 회귀 명령은 필요한 통합 checkpoint에서
`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug|Release`를 사용한다.

## 9. 사용자 interview와 화면 검증

에이전트는 Client/UI를 자율 실행·조작하거나 visual PASS를 대신 선언하지 않는다. 다음 지점에서
사용자 interview를 요청한다.

- 같은 source evidence가 두 ShaderMap/permutation 후보를 남길 때
- source가 없는 live element를 project-authored V1로 유지할지 retirement할지 결정할 때
- 한 source-derived equation의 RT0 결과와 native MRT/scene 결과 중 Product 목표를 고를 때
- canary의 scale, direction, timing, UV phase, coverage/color가 자동 증거로 결정되지 않을 때
- cohort Product rollout 전에 대표 occurrence A/B 승인이 필요할 때

사용자 화면 matrix는 다음 두 층으로 기록한다.

1. verified program/descriptor 대표 canary의 material A/B
2. 모든 live skill/pattern asset의 composition smoke

대표 canary 승인만으로 다른 transform/timing occurrence를 자동 승인하지 않는다.

## 10. living status

| G | 상태 | 자동 분모 | 사용자 화면 | 다음 blocker |
|---|---|---|---|---|
| V0 baseline | `COMPLETE_BASELINE` | live 205 assets / 5,294 elements join 실측 | 기존 손튜닝 기준 | V1 ledger 생성 |
| G00 inventory | `PLANNED` | 5,294 live + 2,256 catalog-addressable non-live + source restoration + presentation | 해당 없음 | builder/schema |
| G01 registry | `PLANNED` | current 46 typed seed + Tool canary descriptors | 해당 없음 | stable IDs/layout schema |
| G02 adapters | `PLANNED` | carrier/layout matrix | 해당 없음 | descriptor-driven binder |
| G02a programs | `PLANNED` | distinct program identity 분모 미산출 | 해당 없음 | DXBC translation/reconstruction materialization |
| G03 coverage | `PLANNED` | verified cohort 미산출 | `PENDING` | first Sprite/Mesh RT0 canary |
| G04 decal | `PLANNED` | live decal 57 | `PENDING` | actual decal VF/pass |
| G05 flow/masked | `PLANNED` | verified cohort 미산출 | `PENDING` | time/dynamic/sampler ABI |
| G06 glass | `TOOL_CANARY_SEED` | Glasshole one occurrence | `PENDING` | sampler/VF/MRT/Product |
| G07 trail | `PLANNED` | live trail 32 | `PENDING` | topology/material split |
| G08a-c presentation | `TYPED_SEEDS_PRESENT` | Light 1 + ModelCue/screen generated inventory pending | `PENDING` | Light/screen/model separate adapters |
| G09 characters | `PLANNED` | live 97/1,611, visible 1,568, hidden 43 + generated restoration | `PENDING` | G03~G08 cohorts |
| G10 Valtan | `TOOL_CANARY_SEED` | live 108 assets / 3,683 elements + generated restoration | `PENDING` | core-three full ABI/Product |
| G11 automated | `PLANNED` | generated final Product/non-live/restoration/presentation gap 0 | 해당 없음 | all prior automated gates |
| G11 visual | `PLANNED` | generated final Product/presentation review 분모 | `PENDING` | 모든 required row `USER_APPROVED` |

이 표는 각 merge 뒤 현재 main 실측으로 갱신한다. 자동 증거와 사용자 화면 상태를 한 칸에 합치지
않으며, 예전 분모가 바뀌면 builder receipt와 이유를 같은 변경에서 갱신한다.
