# 2026-08-23 4캐릭터·Valtan Effect 대표 canary 전수 조사 결과

branch: `codex/four-character-effect-v1-horizontal`

base: `origin/main@d6b27084e5853fb8919685c8f3853c5fc6cadbe7` (`PR #172` Track A merge)

상위 계획:

- [`4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획`](../08-22/2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)
- [`Effect Tuple Cohort Inventory 결과`](2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_RESULT.md)
- [`Artist F horizontal Sprite canary 결과`](2026-08-23_EFFECT_V1_HORIZONTAL_SPRITE_CANARY_IMPLEMENTATION_RESULT.md)

이번 변경은 조사와 다음 수직 슬라이스의 선택 결과만 기록한다. Product binding, authored occurrence,
Client runtime 또는 화면을 바꾸지 않는다. 최종 화면 판정자는 사용자다.

2026-08-23 사용자 후속 결정에 따라 이 문서의 20-slot matrix는 결함 진단과 사용자 검증 표본으로만
사용한다. representative Solo 승인을 수평 batch의 선행 gate로 사용하지 않는다. 공용 runtime spine은
다른 세션이 소유하고, 이 branch는 4캐릭터 Product 전수 ledger와 domain application을 소유한다. Valtan
669행은 다른 세션이 소유하며, 양쪽 정적 evidence나 actual proof를 서로 소급 전파하지 않는다.

## 1. 결론

사용자가 제안한 방향은 맞다. 현재 Product에서 Sprite와 Mesh가 실제 주력이다.

```text
current Product 2,554 occurrence
  Sprite 1,797
  Mesh     667
  ----------------
  합계   2,464 = 96.5%
```

그러나 `사각형으로 보인다`는 현상만으로 한 renderer family를 만들 수는 없다. 사각형은 다음 서로 다른
원인의 공통 증상이다.

- 원래 quad인 Sprite의 coverage/alpha equation이 사라짐
- texture lane은 있으나 register, sampler, channel 또는 color space가 틀림
- scalar/vector/DynamicParameter/time 입력이 family-lite 식에서 소비되지 않음
- Mesh의 vertex factory 또는 WPO vertex program이 실행되지 않음
- Decal이 projector/depth reconstruction pass가 아니라 일반 translucent quad로 처리됨
- blend/depth/cull/stencil/pass/MRT가 source와 다름
- source carrier가 현재 V0에서 다른 carrier로 치환됐거나 occurrence 자체가 빠짐

따라서 공용화 단위는 스킬이나 `Sprite/Mesh` 두 이름만이 아니라 다음 tuple이다.

```text
stable occurrence
  -> composition / carrier / geometry-VF
  -> Program equation
  -> Layout ABI
  -> Descriptor values/resources/dynamics
  -> compiled Adapter carrier/VF/pass/render-state/MRT/draw
```

Program/Layout/Adapter ID를 임의 조합으로 바꿔 보면서 원본을 찾는 방식도 아니다. source evidence로 exact
identity를 먼저 resolve하고 자동 packet·shader·draw 검증을 통과시킨 뒤, 사용자가 대표 occurrence와 전체
composition을 눈으로 판정한다.

## 2. 현재 전수 분모

Track B merge snapshot의 authored evidence 분모는 `416 documents / 7,566 occurrences`다. 그중
animevents, Valtan cue와 boss visual을 통해 snapshot의 runtime Product consumer가 실제로 닿는 분모는
`145 assets / 2,554 occurrences`다. Track A merge 뒤 builder `--check`는 stale이므로 아래 수치는 현행
taxonomy 재생성의 비교 baseline이지 latest-main sealed artifact라고 부르지 않는다.

| domain | Product assets | total | Sprite | Mesh | Decal | Ribbon | Presentation |
|---|---:|---:|---:|---:|---:|---:|---:|
| Artist | 19 | 377 | 309 | 44 | 19 | 5 | 0 |
| Warlord | 22 | 463 | 311 | 140 | 9 | 3 | 0 |
| LanceMaster | 43 | 775 | 515 | 245 | 11 | 4 | 0 |
| DimensionMaster | 15 | 270 | 204 | 64 | 2 | 0 | 0 |
| Valtan | 46 | 669 | 458 | 174 | 33 | 3 | 1 |
| **합계** | **145** | **2,554** | **1,797** | **667** | **74** | **15** | **1** |

위 표의 `Sprite`는 coarse carrier라 Artist standalone Sprite Rect 2행을 포함한다. 이 branch의 정확한
4캐릭터 horizontal fine 분모는 Product `1,885`행 중 `SpriteParticle 1,337 / MeshParticle 493 /
DecalParticle 41 = 1,871`행이다. standalone Sprite 2행과 Ribbon 12행은 `FEATURE_DEFERRED`로 분리한다.

마스터 계획의 frozen V0 `5,294`와 이전 successor snapshot `5,568`은 migration ledger 기준선이다.
현재 runtime reachability `2,554`와 단순 가감하지 않는다. source/current/Product projection을 잇는 G00
full-outer-join이 두 분모의 lineage를 설명해야 한다.

## 3. Product 2,554행의 배타적 현재 위치

아래 표는 같은 행을 두 번 세지 않는다. Decal, Ribbon, Presentation을 먼저 분리하고 Sprite/Mesh만
Program 상태로 나눈 결과다.

| 현재 위치 | occurrence | 의미 |
|---|---:|---|
| family equation + packet open | 1,374 | family/bounded evidence는 있으나 exact typed packet과 actual draw가 없음 |
| exact Program + packet/Adapter open | 799 | exact translated PS는 있으나 packet, VF/pass/state/MRT actual draw가 없음 |
| Program evidence 없음 | 243 | Sprite/Mesh carrier는 있으나 실행 equation 근거가 없음 |
| LocalDecal projector open | 74 | Decal carrier는 있으나 공용 projector/material ABI가 없음 |
| typed packet static-only | 44 | typed P/L/D는 선언됐지만 Track A actual runtime proof가 아님 |
| Ribbon topology open | 15 | history geometry와 material을 별도 복원해야 함 |
| exact DXBC translation open | 4 | exact blob은 있으나 실행 HLSL materialization이 없음 |
| Presentation separate | 1 | Light이며 particle material이 아님 |
| **합계** | **2,554** |  |

Track B 정적 cohort는 Product `790`행을 묶지만 `runtimeVerified=true`인 cohort는 0이다. 가장 큰 정적
cohort 20개를 모두 합쳐도 `421/2,554`행이다. 이 숫자는 우선순위 상한이지 Product runtime coverage가
아니다. 현재 actual compiled binding/draw가 닫힌 것은 Artist F 한 Sprite occurrence뿐이다.

### 3.1 Render state와 output topology는 1급 축이다

현재 authored `renderProfile`로만 보아도 Product에는 다음 carrier/state 조합이 있다.

| carrier | authored state | occurrence |
|---|---|---:|
| Sprite | alpha / two-sided / depth-read | 601 |
| Sprite | alpha / one-sided / depth-read | 422 |
| Sprite | additive / one-sided / depth-read | 501 |
| Sprite | additive / two-sided / depth-read | 273 |
| Mesh | alpha / two-sided / depth-read | 481 |
| Mesh | alpha / one-sided / depth-read | 83 |
| Mesh | additive / one-sided / depth-read | 76 |
| Mesh | additive / two-sided / depth-read | 26 |
| Mesh | opaque / back-cull / depth-write | 1 |
| Decal | alpha / two-sided / depth-read | 72 |
| Decal | alpha / one-sided / depth-read | 2 |
| Ribbon | alpha profiles | 12 |
| Ribbon | additive profiles | 3 |

이 표도 source truth가 아니라 현재 authored 상태다. Valtan Product의 occurrence-exact Program 139행을
source parent render state와 다시 비교하면 138행이 비교 가능하고, 그중 `129`행이 다르며 `9`행만
일치한다. 나머지 1행은 source state가 미확정이다. 현재 Valtan exact 행은 138개가
`alpha_two_sided_depth_read`, 1개가 `alpha_one_sided_depth_read`로 정규화돼 있지만 source에는 additive
one-sided 60, translucent one-sided 51, masked one-sided 19, translucent two-sided 8이 있다.

따라서 Artist F Adapter ID는 일반 Sprite dispatcher가 아니라 다음 한 compiled receipt다.

```text
SpriteParticle + selected VF/layout + pass 1 + MRT_SceneHDR
+ SceneColor RT0 + zero Distortion RT1
+ RS_Cull_None + DSS_ReadOnly + BS_EffectAlpha
```

carrier가 같아도 source blend/sidedness/depth/pass/MRT가 다르면 이 Adapter를 재사용하지 않는다. 현재
registry 구현은 rasterizer/depth/blend/stencil/pass/MRT를 compiled Adapter receipt 안에 봉인한다. 추후
state ID를 별도 축으로 factor하더라도 Binding의 exact tuple에서 사라지게 해서는 안 된다.

## 4. taxonomy 기반 20-slot audition matrix

20개는 스킬 목록이 아니다. 다음 축을 deterministic set-cover해 뽑는 taxonomy cell이다.

```text
fine carrier/geometry
× source/current blend-sidedness-depth
× Program/Layout/Descriptor evidence
× VF/pass/scene/MRT dependency
× Product/live-hidden/source-missing scope
```

캐릭터 스킬과 Valtan pattern은 사용자가 UI에서 찾기 위한 예시 라벨일 뿐 renderer selection key가 아니다.
`정적`은 Track B evidence이며 actual runtime proof나 사용자 visual PASS가 아니다.

| # | taxonomy cell | 현재 대표 occurrence 예시 | 이 칸이 닫는 질문 |
|---:|---|---|---|
| 1 | actual Sprite golden control | Artist F `sprite.2b3dc6842507e910` | registry/inline no-change와 actual draw spine |
| 2 | SpriteParticle alpha two-sided | Track A shadow 또는 exact Product row | 같은 carrier에서 descriptor만 다른가 |
| 3 | SpriteParticle alpha one-sided | Dimension F Fluid01 `1ae3416a...` | cull/state가 다른 Sprite actual adapter |
| 4 | SpriteParticle additive one-sided | Dimension R Basic01 `6280dcd7...` | alpha Adapter 재사용을 막고 additive state 검증 |
| 5 | SpriteParticle additive two-sided | Artist R BA4 `a3aeca8c...` | sidedness와 additive 조합 분리 |
| 6 | MeshParticle alpha two-sided | Artist Q typed Mesh `8b8cad3a...` | 첫 actual Mesh carrier/VF/pass/state/MRT |
| 7 | MeshParticle alpha one-sided | Artist V Mesh `cf524680...` | one-sided Mesh state와 exact Program |
| 8 | MeshParticle additive one-sided | Warlord F WPO source `8c0d6ab0...` | additive Mesh와 WPO를 단계적으로 분리 |
| 9 | MeshParticle additive two-sided | Dimension A Mesh `b9ba4503...` | two-sided Mesh state 변형 |
| 10 | opaque/back-cull/depth-write Mesh | Artist F `mesh.cc04feee8a36940b` | translucent particle pass와 다른 depth-write path |
| 11 | LocalDecal alpha two-sided | Artist R symbol14 또는 Warlord T emitter55 | projector/depth reconstruction과 material equation |
| 12 | LocalDecal alpha one-sided | Artist F `decal.6f78bff02c657a14` | Decal cull/state variant |
| 13 | Ribbon alpha | Valtan Whirlwind emitter5258 | history geometry와 alpha material |
| 14 | Ribbon additive | Valtan Whirlwind emitter5260 | 같은 topology의 additive state reuse 여부 |
| 15 | standalone Rect coverage | Artist S grass body/tip | particle billboard와 다른 Rect carrier, coverage/dissolve |
| 16 | source/current state mismatch | Valtan simple Sprite `source.8f7fea...` | current alpha-two-sided와 source additive-one-sided 교정 |
| 17 | UV flow / DynamicParameter | Lance F FlowTrail, Lance V helix | texture lane, sampler, time origin, dynamic semantics |
| 18 | vertex WPO | Warlord F default-off WPO candidate | RT0 pixel 식과 vertex displacement 분리 |
| 19 | scene feedback / MRT | Dimension W Glasshole02 | scene texture, depth, refraction, auxiliary MRT |
| 20 | mixed composition + source-missing | Valtan FBF current successor + WaterTrail/stone candidate | live composition 보존과 uncatalogued restoration Solo |

각 cell의 대표 occurrence는 evidence가 좋아지면 바뀔 수 있다. 변하지 않는 것은 taxonomy key와 admission
gate다. 예를 들어 Artist Q를 첫 Mesh로 쓰는 이유는 skill이 특별해서가 아니라 Product inline
RuntimeMaterialV2 packet이 이미 typed-closed여서 compiled Mesh Adapter만 독립 검증하기 가장 작기 때문이다.
Warlord R stone은 좋은 후속 Product-origin exact cohort지만 packet/sampler/packing materialization이 먼저
필요하다.

## 5. 실제 관찰과 구조적 위험을 분리한다

- Artist Q/R/V의 square/black-purple/white-ray와 Warlord A chain missing은 기존 사용자 관찰을 가진다.
- Artist S, Warlord F, Dimension F의 typed focused 결과는 자동 구현 증거이며 사용자 화면 승인은 아직 없다.
- Lance V square와 Valtan square/invisible은 현재 사용자 기억 또는 구조적 위험이다. occurrence별 visual
  receipt가 생기기 전 `USER_OBSERVED_DEFECT`로 승격하지 않는다.
- Track A Artist F도 자동 bit-exact/actual draw는 닫혔지만 사용자 A/B 전에는 visual PASS가 아니다.

같은 이유로 “잘 나오는 particle”도 V1 complete 대조군이라고 자동 판정하지 않는다. V0 family-lite 결과가
우연히 비슷할 수 있으므로 exact packet/adapter와 사용자 대조가 모두 필요하다.

## 6. V0 삭제분을 복원하는 방법

현재 Product에 살아 있는 대표 행은 새 element를 만들지 않는다. 같은 stable occurrence에 default-off
Binding을 붙여 ordinary/typed A/B를 하고, Binding resolve 또는 canary identity가 다르면 fail-closed한다.

Product에서 빠진 source occurrence는 다음 세 입력을 full outer join한다.

```text
original source occurrence
+ current V0 user-tuned document
+ verified V1 Program/Layout/Descriptor/Adapter
= Tool-only default-off restoration candidate
```

단순 `source row count - Product row count`는 삭제 수가 아니다. split, merge, substitution, current-only
project authored 행이 섞여 있기 때문이다. `SOURCE_MISSING_IN_V0`만 deterministic candidate로 만들고,
`MERGED_OR_SUBSTITUTED_IN_V0`와 `AMBIGUOUS_SOURCE_JOIN`은 사용자 판단 전 Product에 넣지 않는다.

저장소의 `build_source_restoration_candidates.py`와 기존 `*.restoration-candidate.effect.json`이 이 경계의
선행 구현이다. 별도 uncatalogued v13 문서로 열고 Product cue에는 연결하지 않으며 source carrier,
renderer shape, mesh/resource와 deterministic stable ID를 보존한다. 현재 builder의 대상은 일부 character
skill로 제한돼 있으므로 20-slot audition에는 같은 transaction을 일반화하되 Product auto-insert 기능을
추가하지 않는다.

대표 복원 대상은 다음과 같다.

- Artist R hidden MissileTrail `authored.source-particle.4687f6fe02bab525583b1959`: 한 행만 Solo하고 같은
  hidden cohort 6행을 일괄 enable하지 않는다.
- Lance V source-missing rows: MakeFlow family가 닫힌 뒤 full-outer-join 결과만 복원한다. 과거 134행
  candidate와 현재 39행을 단순 감산하지 않는다.
- Valtan FBF WaterTrail `par_n_rpbf_atk_01_02.em15`: exact carrier는 sphere Mesh, scale `0.01`이며 현재
  audition 문서는 evidence shell이다. preserved source identity에서 candidate를 다시 생성한다.
- Valtan FBF Masked stone `par_n_rpbf_atk_01_02.em07`: 현재 V0 plane과 source stone carrier가 달라
  material만 켜지 않고 carrier/admission까지 함께 Solo한다.

## 7. 수평 구현 순서와 병렬 소유권

각 단계는 스킬 이름 switch가 아니라 stable occurrence와 일반 `Program × Layout × Adapter` capability로
구현한다. 대표 occurrence는 자동 batch를 여는 승인 gate가 아니라 actual draw fixture와 사후 진단 표본이다.

| tranche | 적용 단위 | 현재 4캐릭터 분모 | 다음 자동 gate |
|---:|---|---:|---|
| 0 | 공용 S/M/D registry·actual draw spine | domain 공통 | Layout ABI 호환, inline exact, carrier/pass/state/MRT, Binding0 회귀, Debug/Release |
| 1 | current inline packet registry mirror | bound 1 + 후보 43 | 동일 packet 전 field/float-bit exact와 compiled tuple 존재 |
| 2 | source-exact simple RT0 packet materialization | 64 | source identity/state, texture/sampler/lane packing, actual Adapter closure |
| 3 | 나머지 source-exact packet materialization | 596 | scene/MRT/WPO/time/dynamic blocker를 capability별 해소 |
| 4 | admitted project reconstruction | 711 | reconstruction equation·sensitivity receipt와 typed packet |
| 5 | evidence blocked | 456 | 빠진 source identity/state/Program/Descriptor 근거를 추가하기 전 Binding 금지 |
| 6 | standalone Sprite/Ribbon | 14 | S/M/D와 분리된 carrier/topology lane |

공용 runtime spine와 최종 merged registry publish는 다른 세션과 한 integration owner가 맡는다. 이 branch는
4캐릭터 1,885행 ledger, four-character registry fragment, evidence-closed authored packet과 대표 4스킬
composition regression만 맡는다. Valtan branch는 669행 ledger/fragment/authored data만 맡고 공용 C++를
복제하지 않는다.

같은 tranche 안에서는 자동 증거가 닫힌 행을 전부 batch한다. texture register, sampler, channel,
scalar/vector layout, DynamicParameter semantic, VF, state, pass 또는 MRT가 다르면 별도 tuple/capability로
분리하지만 사용자 Solo 결과를 기다리느라 evidence-closed sibling을 보류하지 않는다.

## 8. 사용자가 실제로 눈으로 볼 단위

모든 scalar/vector 조합을 사용자가 손으로 하나씩 바꿀 필요는 없다.

1. 자동: inline↔registry bit-exact packet, float bit pattern, ordered lanes, sampler/channel, state/pass/MRT,
   invalid binding fail-close, parameter sensitivity를 검증한다.
2. 자동 batch 적용 뒤 사용자 Full composition: 대표 4스킬 또는 Valtan pattern에서 timing, attachment,
   겹침, 누락을 본다.
3. 결함이 보일 때만 occurrence Solo로 내려가 first pixel, silhouette/coverage, UV motion, color/alpha,
   lifetime, scale/direction과 해당 tuple을 진단한다.
4. 수정 뒤 영향받은 exact tuple cohort와 composition을 다시 본다.

따라서 visual 작업량은 `2,554행 × 모든 parameter 조합`도 아니고, 수평 적용 전에 모든 occurrence Solo를
승인하는 과정도 아니다. `자동으로 검증된 tuple batch + 대표 실제 composition smoke + 결함 발생 시 해당
occurrence drill-down`이다. 다만 현재 정적 cohort 340개가 그대로 340개 runtime family라는 뜻도 아니며,
actual draw에서 split/merge될 수 있다.

## 9. 아직 남은 변수

- current Product 2,554와 frozen/current migration ledger 분모의 source lineage full crosswalk
- current authored renderProfile과 source parent blend/sidedness/depth의 full crosswalk
- occurrence exact static permutation과 vertex shader/WPO evidence
- parent가 가진 texture/scalar/vector override의 effective descriptor materialization
- native sampler, channel, color space와 DynamicParameter/time origin
- Decal projector/depth reconstruction과 Glass scene input/MRT
- Ribbon history geometry, Light/ScreenPost/ModelCue presentation
- 사용자 A/B와 V0 삭제분 `ADD/REPLACE/RETIRE` 결정

즉 구조는 공용으로 복원 가능하다. 빠른 길도 `Sprite 하나, Mesh 하나`를 끝내는 데서 시작한다. 단 그
두 개를 모든 Sprite/Mesh에 적용하는 것이 아니라, 그 adapter spine 위에 exact Program/Layout/Descriptor
tuple를 하나씩 admission하고 검증된 cohort만 데이터 확장하는 방식이다.

## 10. 조사 검증

이번 조사에는 Client/UI 실행, 화면 캡처, Product mutation과 visual PASS 선언이 없다.

실측 정본:

- `Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json`
- `Data/Effects/Contracts/character-effect-restoration-targets.v1.json`
- `Data/Balance/PlayerSkills.json`
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`

정적 inventory의 모든 `runtimeVerified`와 `runtimeDescriptorExpansionEligible`은 계속 false다. Artist F
Track A actual proof는 별도 Result와 focused harness 증거로만 인정하며 Track B artifact에 소급 기록하지
않는다. latest main에서 `build_effect_tuple_cohort_inventory.py --check`는 `STALE`이므로 다음 taxonomy
inventory 작업이 current registry input을 반영해 다시 생성하되 Track A proof를 정적 cohort 전체에
전파하지 않아야 한다. 기존 source restoration candidate 세 대상의 `--check`는 모두 PASS다.
