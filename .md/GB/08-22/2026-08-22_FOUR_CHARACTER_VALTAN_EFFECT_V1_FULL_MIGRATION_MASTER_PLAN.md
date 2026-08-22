# 2026-08-22 4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획

기준 branch: `origin/main`

V0 fixture origin commit: `61930d4ab7018b9df04dea7c297681505160c34d`

첫 carrier cohort 통합 base: `origin/main@48c53f20`

첫 carrier cohort 구현 merge: PR `#152`, `main@dc436ae8`

V0 상태: `COMPLETE_BASELINE`

V1 상태: `BINDING_1_ACTUAL_SPRITE_AUTOMATED / USER_REVIEW_PENDING`

최종 화면 판정자: 사용자

연결 정본:

- [`Effect Family Runtime ABI 복원 가이드`](../../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)
- [`Character Source-Exact Effect Conquest 기존 결함·기능 계획`](../08-21/2026-08-21_CHARACTER_SOURCE_EXACT_EFFECT_CONQUEST_MASTER_IMPLEMENTATION_PLAN.md)
- [`캐릭터 Effect authored → sealed runtime 통합 결과`](2026-08-22_CHARACTER_EFFECT_RUNTIME_PUBLISH_INTEGRATION_RESULT.md)
- [`Glasshole02 Missing Family ABI 복원 결과`](2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_RESULT.md)
- [`Valtan Core-Three Missing Family ABI 복원 결과`](2026-08-22_VALTAN_FRONT_BACK_FRONT_MISSING_FAMILY_RESTORATION_RESULT.md)
- [`Client frontend 광역 하네스 삭제 결과`](2026-08-22_CLIENT_FRONTEND_HARNESS_REMOVAL_RESULT.md)
- [`도화가 S Typed Rect V1 첫 확장 결과`](2026-08-22_ARTIST_S_TYPED_RECT_V1_EXPANSION_RESULT.md)
- [`Effect Tuple Cohort Inventory 결과`](../08-23/2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_RESULT.md)
- [`Artist F horizontal Sprite canary 결과`](../08-23/2026-08-23_EFFECT_V1_HORIZONTAL_SPRITE_CANARY_IMPLEMENTATION_RESULT.md)
- [`4캐릭터·Valtan 대표 canary 전수 조사 결과`](../08-23/2026-08-23_EFFECT_REPRESENTATIVE_CANARY_SURVEY_RESULT.md)
- [`4캐릭터 Effect V1 수평 application ledger 결과`](../08-23/2026-08-23_FOUR_CHARACTER_EFFECT_V1_HORIZONTAL_APPLICATION_LEDGER_RESULT.md)

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

### 0.5 완료 이름을 두 개로 고정한다 (2026-08-22 사용자 결정)

이 계획의 완료 판정은 `V1_COMPLETE` 하나만 사용한다.

| 이름 | 구성 | 이 계획에서의 위치 |
|---|---|---|
| `V1_COMPLETE` | 올바른 carrier + family별 RT0 Base HLSL + texture/channel/scalar/DynamicParameter 배선 + blend/depth + attachment/timing + Effect Tool 편집·저장 + 사용자 육안 승인 | 완료 조건 |
| `NATIVE_PARITY` | native VF/BasePass, 원본 ShaderMap permutation, exact child cooked variant, MRT 0/2/3/4/5, Distortion 외 scene feedback, hardware sampler 전수 parity | 별도 backlog. 완료 조건 아님 |

도화가 F도 typed RT0 semantic replay golden이며 native VF/pass 전체를 닫은 사례가 아니다.
그 수준이 `V1_GOLDEN_CONTROL`이고 동시에 `V1_COMPLETE`의 기준선이다. 따라서 G03~G08의 각 family는
`NATIVE_PARITY` 항목이 비어 있어도 RT0 Base와 배선, carrier, timing, Tool 저장, 사용자 승인이
닫히면 `V1_TYPED_PRODUCT`로 완료 처리한다. fidelity는 계속 별도 축으로 기록한다.

RT0 Base 재구성의 1차 evidence는 `Data/Effects/Contracts/effect-family-manifest.v1.json`의 parent
material evidence다. 이 문서는 family별 `blendMode`, `twoSided`, `isMasked`, texture parameter의
이름·group·기본 texture, scalar parameter의 이름·group·기본값을 소유한다. DXBC가 없는 family는 이
증거로 `BOUNDED_TRANSLATED` RT0 Base를 닫고, 증거로 의미가 닫히지 않는 scalar는 구현하지 않고
`NATIVE_PARITY` backlog에 남긴다.

### 0.6 첫 확장 실험: 도화가 A 대조군과 S Rect family

전체 스킬을 동시에 승격하지 않는다. 첫 실험은 다음 세 행으로 고정한다.

| 역할 | stable occurrence | 현재 상태 | 실험에서 증명할 것 |
|---|---|---|---|
| 무코드 대조군 | 도화가 A `authored.source-particle.cb346af...` | SpriteWave profile 20과 RT0 식이 이미 연결됨 | 기존 typed family의 spawn/draw/pixel 경로가 최신 Product에서 유지되는지 |
| 구현 canary | 도화가 S `sprite.artist.31420.grass-coverage.v1` | standalone Sprite Rect + generic material | Rect carrier가 typed RuntimeMaterialV2 packet과 RT0 family 함수를 소비하는지 |
| 데이터 확장 | 도화가 S `sprite.artist.31420.grass-tip-emissive.v1` | canary와 같은 carrier, 다른 mask/dissolve와 손튜닝값 | 두 번째 occurrence가 C++/HLSL 추가 없이 descriptor만으로 같은 family를 재사용하는지 |

S의 원본 36행을 다시 넣지 않는다. 현재 Product의 source 1행과 사용자가 승인한 project sprite 2행만
유지하며, 두 project 행의 stable ID, transform, color, emissive, UV, timing, attachment 값은 변경하지
않는다. 이번 변경은 다음 계약만 추가한다.

1. standalone Sprite Rect를 RuntimeMaterialV2 typed carrier로 admission한다.
2. `base RGBA + coverage R + emissive RGB + dissolve R` 네 lane을 갖는 class-neutral RT0 family opcode를
   한 번 구현한다.
3. body와 tip은 같은 opcode와 packet layout을 사용하고 texture asset과 기존 Detail 값만 다르게 둔다.
4. shader는 `base * color + emissive * intensity`, `base alpha * coverage * lifetime`, dissolve threshold,
   RT0 SceneColor와 zero distortion만 계산한다.
5. Debug/Release shader compile, document codec/publisher, focused materializer/harness와 rollback을 자동
   검증하고, 실제 화면 품질은 사용자가 Effect Tool에서 승인한다.

이 실험의 `V1_COMPLETE` 조건은 도화가 F의 실제 성공선과 같다. 올바른 carrier, typed RT0 equation,
texture/channel/sampler packet, render profile, 기존 composition과 사용자 승인을 요구한다. exact cooked
child 실행, native ShaderMap/VF/BasePass, distortion 및 MRT2~5는 `NATIVE_PARITY` 후속 연구이며 이 실험의
완료 조건이 아니다.

### 0.7 창술사 D·F typed-family lane

창술사 D(34110 반월섬)와 F(34150 맹룡열파)는 이 계획의 첫 `V1_COMPLETE` 확장 실험 대상이다.

| 단계 | 상태 |
|---|---|
| Product admission (catalog + animevents cue) | 완료. [RESULT](2026-08-22_LANCEMASTER_D_F_PRODUCT_EFFECT_RESTORATION_RESULT.md) |
| grouped 242행의 family 분해 | 완료. 23개 parent material |
| typed family 확장 | 진행 중. [RESULT](2026-08-22_LANCEMASTER_D_F_V1_FAMILY_EXPANSION_RESULT.md) |
| 사용자 육안 승인·손튜닝 | `USER_REVIEW_PENDING` |

### 0.8 V0 보존과 carrier 승격의 네 가지 terminal 판정

V0 손튜닝 Product는 삭제 대상이 아니라 화면 비교 기준선과 rollback 대상이다. 그러나 V0 행이 원본
carrier를 증명한다는 뜻은 아니다. V1은 source occurrence마다 emitter kind, geometry/model, attachment,
material family가 요구하는 adapter를 확인한 뒤 아래 네 terminal 판정 중 하나만 부여한다.

| 판정 | 적용 조건 | Product 처리 | 손튜닝값 처리 |
|---|---|---|---|
| `KEEP` | 현재 V0 행의 carrier와 semantic role이 source evidence와 맞고 화면에 의미가 있음 | 같은 stable 행에서 typed material/ABI만 승격하며 중복 행을 만들지 않음 | 기존 transform, size, timing, color를 그대로 유지 |
| `REPLACE` | 현재 V0 행이 역할은 맞지만 Sprite/Mesh/Decal/Trail carrier 또는 attachment가 틀림 | 정확한 carrier 후보를 Tool-only로 만들고 사용자 승인과 동시에 predecessor를 원자적으로 교체 | predecessor의 손튜닝값을 대응 가능한 occurrence 필드에 이식하고 재튜닝 |
| `ADD` | V0 pruning에서 빠졌지만 기존 행과 겹치지 않는 의미 있는 원본 역할임 | 기본 OFF Tool-only candidate로 시작하고 사용자 승인 뒤에만 Product에 추가 | source transform/timeline을 시작점으로 사용하고 별도 손튜닝 |
| `RETIRE` | 중복, 잘못된 carrier로만 표현 가능, 리소스/근거 부재, RT0에서 의미 없음 | Product와 audition 대상에서 제외하고 이유와 source identity를 receipt에 보존 | 이식하지 않음 |

조사 중인 행은 terminal 판정을 받은 것이 아니다. `PENDING_EVIDENCE` 또는 `USER_REVIEW_PENDING`으로
별도 기록하며, 이를 `KEEP`이나 `ADD`로 간주하지 않는다. 특히 다음은 금지한다.

- 원본 source element를 스킬 단위로 bulk append하거나 bulk regenerate하지 않는다.
- Sprite에 decal texture를 연결해 Decal로 간주하거나 generic plane을 정확한 Mesh carrier로 간주하지 않는다.
- `REPLACE` 승인 전에 V0 predecessor를 삭제하거나 stable Product composition을 바꾸지 않는다.
- family HLSL이 보인다는 이유만으로 carrier/attachment/timeline 증거가 없는 행을 Product admission하지 않는다.
- 한 canary가 통과했다는 이유로 같은 스킬의 다른 occurrence transform/timing을 자동 승인하지 않는다.

승격 순서는 항상 다음과 같다.

```text
frozen V0 Product
  -> one source carrier candidate (Tool-only/default OFF)
  -> typed family packet + RT0 program
  -> Solo draw/build/publish evidence
  -> user A/B
  -> KEEP / REPLACE / ADD / RETIRE
  -> approved Product atomic publish
```

첫 carrier cohort는 다음 네 계약으로 고정한다.

| 순서 | 대상 | canary 역할 | 자동 gate | Product gate |
|---:|---|---|---|---|
| 1 | 창술사 D source Mesh | 이미 연결된 source carrier의 no-code control | exact stable row, mesh/resource/family join과 draw contract | 사용자 control 승인 |
| 2 | 워로드 F WPO SinWave Mesh | 잘못 평탄화된 핵심 Mesh material의 family+carrier 승격 | 두 occurrence 중 1 canary + 1 data-only reuse, strict packet | 사용자 전기 표면 승인 뒤 `KEEP` 또는 `REPLACE` |
| 3 | 차원술사 F Fluid01 Sprite | 기존 typed equation을 실제 Sprite carrier first pixel로 연결 | canary + 같은 family 확장, alpha/time/resource closure | 사용자 화면 승인 뒤 `KEEP`/`ADD` 판정 |
| 4 | 창술사 F MakeFlow Mesh | existing profile/HLSL에 effective parent 5-lane과 dynamic semantics 연결 | source carrier/parent/lane/dynamic exact join | 사용자 승인 뒤 `KEEP`/`REPLACE` 판정 |

이 cohort의 자동 성공은 네 스킬 전체 `V1_COMPLETE`가 아니다. canary별 carrier와 typed RT0 실행이
제품 후보가 되었음을 뜻하며, terminal 판정과 Product composition 완료는 사용자 화면 승인 뒤 닫는다.

### 0.9 첫 carrier cohort 구현 checkpoint

`2026-08-22_V1_CARRIER_COHORT_INTEGRATION_RESULT.md` 기준 자동 구현과 Debug/Release는 완료됐다.

- 창술사 D와 F carrier는 `KEEP`이며 F만 profile36 material action을 Product에 publish했다.
- 차원술사 F Fluid01 두 Sprite carrier는 `KEEP`이고 first-pixel witness를 봉인했다.
- 워로드 F는 Product를 동결하고 Tool-only 두 행 후보까지만 만들었다. carrier terminal 판정은
  아직 미부여이며 별도 review state만 `USER_REVIEW_PENDING`이다.
- `REPLACE/ADD/RETIRE`를 Product에 실행한 대상은 아직 없다.
- 다음 gate는 사용자의 네 대상 Solo/전체 composition 판정이며, 그 전에는 `V1_COMPLETE`로 올리지 않는다.

### 0.10 Valtan All Effects 저작 화면 계약

Valtan의 All Effects 화면은 phase 탐색기가 아니라 캐릭터 skill 목록과 같은 pattern 저작 화면이다.
pattern 하나를 열었을 때 순서는 다음으로 고정한다.

```text
Pattern
  -> Saved Unified Effects
       -> Open Saved Effect
       -> Play Saved Effect
  -> Animations / Semantic Stages
       -> ordered clip occurrence
       -> Product cue timing/attachment diagnostics
```

Saved 목록은 pattern의 `Stage.ClipOccurrences[].ProductCues`, `Stage.Effects`,
`Stage.CombatObjectEffects`를 semantic stage 순서로 합치고 `effectAssetId`로 중복 제거한다. 현재 main
정본의 raw link는 Product cue 44, stage-authored reference 61, combat-object visual 2로 107개다.
Whirlwind SPIN의 한 Effect가 Product cue와 explicit reference에 함께 기록돼 있으므로 pattern-local
`effectAssetId` 중복 제거 뒤 실제 Saved 행은 106개다.
`VALTAN_DASH_CHARGE`처럼 Product cue가 아직 없는 pattern도 기존 stage-authored 문서 4개를 Saved 목록에
표시해야 한다. Product cue 행의 Open/Play는 exact clip occurrence와 cue를 사용한다. stage-authored
reference와 combat-object visual은 소유 semantic stage의 ordered clip sequence 전체를 재생한다.
combat-object attachment는 계속 Server world-root 소유이며 Tool의 boss animation 동기화가 그 소유권을
바꾸지 않는다. 동일 `effectAssetId`의 Product/reference provenance는 한 행에 모두 보존하고, 저장 경로는
EffectCatalog의 direct-authored 경로를 reference fallback보다 우선한다. 목록이나 아래 animation tree를
그리는 것만으로 문서를 parse하지 않으며 실제 Open/Play 입력에서 identity와 drawable 상태를 검증한다.

`Repeat rotation in every phase`, `Only stages with an Effect`, phase combo는 이 저작 계약에서 제거한다.
같은 rotation pattern을 phase마다 복제하면 saved Effect가 여러 개인 것처럼 보이고, effect 없는 stage를
숨기면 animation composition 전체를 확인할 수 없기 때문이다. phase/gate/health-bar 정보는 encounter
정본에 계속 남지만 All Effects의 저작 계층을 바꾸지 않는다.

### 0.11 Track A/B 이후 대표 canary checkpoint

Track B merge snapshot은 4캐릭터·Valtan authored `416/7,566`과 Product `145/2,554`를
Program/Layout/Adapter/Descriptor/Composition 축으로 전수 분류했다. 이 inventory의 cohort는 전부 정적
재사용 후보이며 runtime-verified cohort는 0이다. `origin/main@d6b27084`의 Track A registry input 뒤
builder `--check`는 stale이므로 current taxonomy 작업에서 재생성하되 Track A proof를 cohort 전체로
소급 전파하지 않는다.

Track A는 `effect.artist.skill.31470.unified / sprite.2b3dc6842507e910` 한 occurrence에만 Binding을
추가해 registry-inline bit-exact packet과 actual SpriteParticle compiled draw를 자동으로 닫았다.
사용자 A/B 전에는 visual PASS가 아니다. Track B 정적 evidence를 이 actual proof로 소급 승격하지 않는다.

다음 수직 슬라이스는 대표 canary 조사 결과의 taxonomy 기반 20-slot audition matrix를 따른다. 스킬과
pattern은 사용자 UI의 예시 라벨일 뿐 selection key가 아니다. 우선순위는 actual MeshParticle control,
source/current render-state split, cross-domain simple Sprite, LocalDecal, UV-flow, WPO, Glass/MRT,
Ribbon/Presentation, family별 V0 삭제분 default-off 복원 순서다. C++ renderer는 skill/class/material 이름을
switch하지 않고 stable occurrence admission과 일반 `Program × Layout × Adapter` tuple만 소비한다.

### 0.12 4캐릭터 Product 수평 batch 결정 (2026-08-23 사용자 결정)

4캐릭터 Product는 class나 skill 단위 canary 승인을 다음 batch의 선행 조건으로 두지 않는다. 최신 Product
분모는 `1,885`행이고, 실제 particle material 수평 적용 범위는 `SpriteParticle 1,337 + MeshParticle 493
+ DecalParticle 41 = 1,871`행이다. Artist standalone Sprite Rect 2행과 Ribbon 12행은 같은 전수 ledger에
남기되 S/M/D 공용 Adapter 적용 분모에는 섞지 않는다.

`전체 배선`은 다음 두 상태를 구별한다.

```text
모든 1,885행
  -> source/current composition·carrier·state와 P/L/D/A evidence를 전수 판정
  -> 자동 증거가 닫힌 모든 S/M/D행은 같은 batch에서 Binding 생성
  -> 한 축이라도 열려 있는 행은 Binding 없이 blocker를 봉인하고 기존 V0 draw 유지
```

이 결정을 구현한 4캐릭터 application ledger의 current partition은 다음과 같다. 이 상태들은 서로
배타적이며 합계가 항상 `1,885`여야 한다.

| application state | occurrence | 현재 의미 |
|---|---:|---|
| `CURRENT_BOUND_INLINE_EXACT` | 1 | Track A actual Sprite Binding |
| `INLINE_MIRROR_CANDIDATE` | 43 | typed inline packet은 닫혔으나 공용 registry batch 대기 |
| `SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING` | 64 | exact Program·기본 state/layout evidence, packet/actual tuple 대기 |
| `SOURCE_EXACT_PACKET_PENDING` | 596 | occurrence-exact Program이 있으나 ABI/scene/MRT/WPO/state 중 후속 작업 필요 |
| `PROJECT_RECONSTRUCTION_PENDING` | 711 | admitted reconstructed profile을 typed packet/Program으로 구현해야 함 |
| `FEATURE_DEFERRED` | 14 | standalone Sprite Rect 2 + Ribbon 12 |
| `EVIDENCE_BLOCKED` | 456 | source identity/state/Program/Descriptor 등 필수 근거 미폐쇄 |

committed FourClassCombat source contract와 exact material path로 join되는 행은 `1,512`, 미가입은 `373`이다.
원본 render state는 `1,204`행만 strict resolve되며 `681`행은 unresolved다. 비교 가능한 축에서는 현재 V0와
`1,125`행이 일치하고 `79`행이 다르다. mismatch 축은 중복 집계로 blend 54, two-sided 34, depth-test
20이다. source contract가 depth-write를 캡처하지 않았으므로 masked 행을 opaque/depth-write로 추정하지
않는다.

사용자 occurrence Solo는 자동으로 닫힌 cohort를 Product에 적용하기 위한 입장권이 아니다. 전체 batch의
publisher·exact packet·prepared snapshot·actual draw·Debug/Release 검증을 먼저 끝내고, 사용자는 그 뒤 대표
4스킬과 영향받은 composition을 화면에서 판정한다. 사용자 판정 전에도 자동 Binding은 존재할 수 있지만
그 행의 visual 상태는 계속 `USER_REVIEW_PENDING`이며 `V1_COMPLETE` 또는 원작 fidelity PASS로 기록하지 않는다.

이 결정은 현재 Product에 살아 있는 occurrence에만 적용한다. V0에서 빠진 source occurrence는 composition
merge 결정이 별도이므로 `restoration-candidate`, Product cue 미연결, default-off 경계를 유지한다. 자동
material 증거가 닫혔다는 이유로 삭제분을 Product에 일괄 삽입하지 않는다.

공용화 단위는 세 carrier capability와 그 아래의 typed tuple이다.

```text
S = SpriteParticle Standard RT0
M = MeshParticle CModel RT0
D = LocalDecal Projector RT0

Binding = stable occurrence
        + Program equation
        + Layout ABI
        + Descriptor values/resources/dynamics
        + compiled Adapter carrier/VF/pass/state/MRT
```

Adapter 구현은 동일 tuple에 대해 도메인 무관해야 한다. Artist, DimensionMaster, LanceMaster, Warlord,
Valtan 이름과 skill ID, source material 이름을 C++ renderer switch에 넣지 않는다. source state를 확인할 수
없는 행은 현재 `renderProfile`을 원본이라고 추정하지 않고 evidence blocker로 남긴다.

병렬 작업 소유권은 다음으로 고정한다.

| 소유자 | 단독 소유 | 소유하지 않음 |
|---|---|---|
| 공용 runtime spine PR | registry fragment merge, compiled S/M/D Adapter, exact dual-resolve, actual draw, focused harness | 4캐릭터/Valtan occurrence 정책과 대량 authored mutation |
| 4캐릭터 horizontal PR | 1,885행 ledger, four-character fragment, evidence-closed authored packet/Binding, 대표 4스킬 composition regression | 공용 registry/C++/HLSL의 두 번째 구현, Valtan 669행 |
| Valtan horizontal PR | Valtan 669행 ledger와 Valtan fragment/authored packet | 4캐릭터 1,885행과 공용 spine 재구현 |
| 최종 integration owner | 두 domain fragment 병합 뒤 단일 runtime registry/catalog publish와 Debug/Release | domain별 근거를 다른 domain의 runtime proof로 소급 승격 |

4캐릭터 대표 acceptance composition은 차원술사 R `2050180`, 도화가 A `31460`, 창술사 D `34110`,
워로드 R `17110` clip2/clip3의 5개 문서, S/M/D `131`행이다. 이 스킬 이름은 renderer selection key가
아니며, 전체 batch 뒤 사용자가 실제 composition을 찾기 위한 고정 검증 표본이다.

공용 S6/M3/D14와 current enabled inline 44행의 exact 교집합은 Artist F 5행이다. 공용 fragment가 그중
Sprite/Mesh/Decal golden 3행을 소유하므로 4캐릭터 첫 fragment는 `sprite.c65181324417a1a8`과
`decal.6f78bff02c657a14` 두 Binding만 추가한다. 예상 상태는 공용 merge 뒤 bound 3/candidate 41,
four-character fragment 뒤 bound 5/candidate 39다. 대표 131행은 세 공용 Program과 교집합이 0이므로 이
첫 fragment만으로 대표 스킬 화면은 바뀌지 않는다. 대표 화면을 여는 첫 후속 capability는 차원술사 R의
exact `fx_mm_basic_01_ad` Sprite tuple이고, 이후 창술사 D lensflare/noise tuple 순서다.

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

현재 4캐릭터 composition 분모는 57 distinct numeric skill IDs, 105 `effectref=asset` cue rows,
99 unique assets다.
같은 asset을 가리키는 반복 cue도 attachment/timing composition variant로 ledger에 유지한다. 4캐릭터
live 1,885행은 visible 1,842, hidden 43이고 typed seed 48은 모두 visible이다. 따라서 현재 visible
V0/untyped는 1,794행이다. 위 `205/5,294/46` 표는 V0 frozen fixture이고, 현재 Product successor
분모는 Valtan을 합쳐 `207 assets / 5,568 elements / 48 typed`로 별도 유지한다.

### 1.3 direct-authored migration corpus

4캐릭터의 현재 `DIRECT_AUTHORED_DOCUMENT_V13` 후보 corpus는 109 assets/1,983 elements이고 visible
element는 1,940이다. live cue 도달 범위는 99/1,885이므로 10문서/98행이 catalog에는 있으나 현재
animevent에 join되지 않는다.

```text
DimensionMaster BA2, BA4, overslash clip2, clip3
Warlord BA1, BA2, battlefield clip3, clip4, clip8
Warlord F WPO SinWave Tool-only candidate
```

이 10문서를 강제로 cue에 연결하지 않는다. G00에서 `JOIN_REQUIRED`, `NON_PRODUCT_EVIDENCE`,
`SUPERSEDED` 중 하나를 source/skillbinding 증거로 확정한다. Hidden 43행도 수를 줄이기 위해 삭제하지
않고 `DISABLED_WITH_RECEIPT` 또는 실제 V1 target으로 분류한다.

Valtan canonical V0 corpus 108/3,683은 모두 Product다. boss-root pattern cue가 106/3,665를,
BossCatalog combat-object visual이 나머지 2/18을 소유한다. 따라서 현재 direct/canonical migration
corpus는 217 assets/5,666 elements, live Product는 207/5,568이며 미결 join/disposition은
4캐릭터 10 assets/98 elements다. frozen V0 fixture `214/5,390 -> 205/5,294 + 9/96`도 회귀 비교용으로
계속 보존한다.

### 1.4 catalog-addressable 상한과 live 분모를 섞지 않는다

현재 5개 domain의 catalog-addressable authored 상한은 318문서, 7,826 elements이고 Artist legacy
immutable program 한 행이 별도로 있다.

| domain | catalog entries | canonical authored docs | authored elements |
|---|---:|---:|---:|
| Artist | 34 | 33 | 560 |
| Warlord | 55 | 55 | 1,095 |
| LanceMaster | 84 | 84 | 1,827 |
| DimensionMaster | 38 | 38 | 661 |
| Valtan | 108 | 108 | 3,683 |
| **합계** | **319** | **318** | **7,826** |

현재 live successor 분모 5,568과의 차이 2,258행은 legacy, duplicate, audition 또는 현재 cue 비도달
문서다. frozen V0의 `7,550 - 5,294 = 2,256`도 별도 fixture로 유지한다.
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

HLSL은 program equation을 소유한다. Layout은 CB/SRV register와 semantic wire를, Descriptor는 occurrence의
values/resources/sampler/channel/color space를 소유한다. compiled Adapter는 geometry/VF/pass/scene input,
RT/MRT topology, blend/cull/depth/stencil state와 draw transaction을 함께 봉인한다. Authored document는
stable element, composition과 carrier를 소유하고 registry Binding이 그 occurrence에 네 ID를 admission한다.

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

파일 안에서는 `programs`, `layouts`, `descriptors`, `adapters`, `bindings` 다섯 collection을 분리한다.
Program은 equation identity, Layout은 GPU 전달 계약, Descriptor는 occurrence의 effective
values/resources/samplers, Adapter는 compiled carrier/VF/pass/state/MRT allowlist, Binding은 stable
occurrence가 네 ID를 함께 소비할 자격을 소유한다.

`ue3-material-family-registry.v1.json`은 계속 discovery intake이고, 새 registry가 Product admission
정본이다. occurrence 선택을 두 번째 파일이나 C++ skill switch에 중복하지 않고 registry Binding의
`effectAssetId + elementId`가 stable occurrence를 선택한다. `CEffect_Catalog`이 parse→validate→stage→commit하고
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

- frozen V0 live 205/5,294와 현재 successor live 207/5,568을 각각 deterministic하게 재생성한다.
- frozen direct/canonical `214/5,390 -> 9/96 non-live`와 현재
  `217/5,666 -> 10/98 non-live`의 join/disposition을 각각 확정한다.
- frozen catalog 316/7,550과 현재 catalog 319/7,826을 섞지 않고 live 차이를 분류한다.
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
- frozen V0 fixture가 `205/5,294/46 typed seed/5,248 rows without execution`을 재현해야 한다.
- 현재 successor fixture가 `207/5,568/48 typed seed`를 재현해야 한다.
- 4캐릭터 frozen fixture `direct 106/1,707, live 97/1,611, unjoined 9/96`과 현재 fixture
  `direct 109/1,983, live 99/1,885, unjoined 10/98`을 둘 다 재현해야 한다.
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

## 6. 각 occurrence와 수평 batch의 admission gate

모든 G03~G10 cohort는 적용 가능한 다음 순서를 생략하지 않는다.

1. Product cue → effect asset → stable element identity
2. source emitter/carrier/mesh/texture/timeline join
3. child/parent/effective static set/ShaderMap 선택
4. cooked material이 있으면 DXBC와 translated HLSL equation parity. source가 없으면 reconstruction
   receipt와 deterministic sensitivity, non-pixel presentation이면 `NOT_APPLICABLE`
5. CB lane, SRV wire, sampler, channel/color space, dynamic parameter
6. actual VF/pass, scene input, RT/MRT/coverage와 render state
7. 현재 Product occurrence는 위 자동 증거가 닫힌 행을 domain batch로 모두 선택한다. 사용자 Solo나
   대표 canary 승인을 batch admission의 선행 조건으로 두지 않는다. Product에서 빠진 source row만
   default-off `restoration-candidate` 경계를 사용한다.
8. 구조적으로 같은 Program/Layout/Descriptor/Adapter closure가 증명된 행만 registry fragment를 생성하고
   fidelity는 occurrence별 evidence로 유지한다.
9. Debug/Release compile, exact dual-resolve, first draw, state/rollback proof
10. Product staged publish/restart admission. evidence-open 행은 Binding 없이 기존 V0 draw를 유지한다.
11. 전체 적용 뒤 사용자 A/B와 composition 검증. 이 단계는 이미 자동 admission된 Binding을 여는 gate가
    아니라 visual 상태를 `USER_REVIEW_PENDING`에서 닫는 gate다.

한 gate의 PASS를 다음 gate로 대신하지 않는다. 특히 texture parity, DXBC 확보, HLSL 번역, first draw,
사용자 visual과 Product admission은 서로 다른 상태다. 사용자 visual 전 수평 Binding을 허용한다는 결정은
근거가 없는 tuple을 허용하거나 visual PASS를 자동 선언한다는 뜻이 아니다.

## 7. PR과 main 통합 단위

V1 전체를 한 PR로 올리지 않는다. 과거 #141의 203 files/46 regression 결합을 반복하지 않도록 다음
한 수직 슬라이스만 한 PR에 담는다.

```text
one generic carrier/program capability
+ one deterministic domain application batch
+ registry fragment/descriptor data
+ required C++/HLSL adapter
+ focused receipt/harness
+ evidence-closed authored Product rows
+ publisher output contract
+ PLAN/RESULT update
```

- source evidence가 열려 있는 다른 cohort의 authored rows를 같이 regenerate하지 않는다.
- 사용자 승인 전에도 자동 evidence가 닫힌 current Product occurrence는 Binding을 가질 수 있다. visual
  receipt는 별도 `USER_REVIEW_PENDING`으로 유지한다.
- Product PR은 ledger가 자동 admission한 stable IDs만 builder로 selective mutation한다.
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
| G00 inventory | `FOUR_CHARACTER_HORIZONTAL_APPLICATION_LEDGER_COMPLETE / FULL_OUTER_MIGRATION_LEDGER_PARTIAL` | current 4캐릭터 Product 99 assets/1,885행, fine S/M/D 1,871행의 source/current/P-L-D-A/application state. Valtan 및 삭제분 full outer join은 별도 | 해당 없음 | 공용 spine merge 뒤 binding-aware ledger 재생성 |
| G01 registry | `BINDING_1_AUTOMATED` | Program 1 + Layout 1 + Descriptor 1 + Adapter 1 + Artist F Binding 1 | `USER_REVIEW_PENDING` | representative Mesh/Decal/flow tuple admission |
| G02 adapters | `FIRST_ACTUAL_SPRITE_ADAPTER_AUTOMATED` | Artist F stable occurrence 1의 carrier/pass/state/MRT actual draw | `USER_REVIEW_PENDING` | 일반 actual MeshParticle adapter canary |
| G02a programs | `FIRST_PROGRAM_CONSUMER_AUTOMATED` | Artist F opcode 6 one Program, inline↔registry bit-exact | `USER_REVIEW_PENDING` | exact translated Program을 packet/adapter와 수직 연결 |
| G03 coverage | `FIRST_RECT_CANARY_AUTOMATED` | 도화가 S Sprite Rect 2행 typed packet/publish/build | `USER_REVIEW_PENDING` | body/tip first pixel과 A control 사용자 판정 |
| G04 decal | `PLANNED` | live decal 57 | `PENDING` | actual decal VF/pass |
| G05 flow/masked | `FIRST_CARRIER_COHORT_AUTOMATED` | Fluid01 Sprite 2 + MakeFlow Mesh 1 + WPO Mesh Tool 2 | `USER_REVIEW_PENDING` | 사용자 Solo 뒤 WPO terminal 판정, time/dynamic/sampler 후속 |
| G06 glass | `TOOL_CANARY_SEED` | Glasshole one occurrence | `PENDING` | sampler/VF/MRT/Product |
| G07 trail | `PLANNED` | live trail 32 | `PENDING` | topology/material split |
| G08a-c presentation | `TYPED_SEEDS_PRESENT` | Light 1 + ModelCue/screen generated inventory pending | `PENDING` | Light/screen/model separate adapters |
| G09 characters | `HORIZONTAL_LEDGER_COMPLETE / BINDING_1_AUTOMATED` | Product 1,885 전수: current bound 1, inline mirror 43, source-exact pending 660, reconstruction pending 711, deferred 14, blocked 456 | `USER_REVIEW_PENDING` | 공용 S/M/D spine merge, evidence-closed domain fragment batch, 대표 4스킬 composition A/B |
| G10 Valtan | `TOOL_CANARY_SEED` | live 108 assets / 3,683 elements + generated restoration | `PENDING` | core-three full ABI/Product |
| G11 automated | `PLANNED` | generated final Product/non-live/restoration/presentation gap 0 | 해당 없음 | all prior automated gates |
| G11 visual | `PLANNED` | generated final Product/presentation review 분모 | `PENDING` | 모든 required row `USER_APPROVED` |

이 표는 각 merge 뒤 현재 main 실측으로 갱신한다. 자동 증거와 사용자 화면 상태를 한 칸에 합치지
않으며, 예전 분모가 바뀌면 builder receipt와 이유를 같은 변경에서 갱신한다.
