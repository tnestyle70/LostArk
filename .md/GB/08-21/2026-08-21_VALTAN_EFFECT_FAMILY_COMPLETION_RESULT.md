# 2026-08-21 Valtan Effect Family 연결 구현 결과

## 0. 결과 요약

발탄 제품 presentation은 캐릭터와 같은 다음 경로로 연결했다.

```text
Encounter Pattern
  -> Semantic Stage
  -> ordered Clip Occurrence
  -> Product Cue Occurrence 또는 Server Combat-Object Visual
  -> EffectCatalog DIRECT_AUTHORED_DOCUMENT_V13
  -> Unified Effect document
  -> 기존 EffectDocumentRenderer / material profile / shader
  -> EffectPresentationService
```

발탄 전용 renderer, 고정 5-slot, 패턴별 HLSL은 추가하지 않았다. All Effects의 Valtan 트리도
`Pattern -> Stage -> Clip -> Cue -> Unified Effect -> Family`로 같은 document를 열고 저장한다.

현재 제품 저작 그래프는 다음과 같다.

| 항목 | 결과 |
|---|---:|
| Encounter patterns / semantic stages | 33 / 130 |
| animation action bindings / ordered clips | 130 / 137 |
| Boss-root Product cues / cue stages / unique cue clips | 106 / 98 / 100 |
| Server combat-object visuals | 2 |
| 물리 Valtan authored docs / elements | 113 / 3,734 |
| 제품 owned effects / editable docs / elements | 108 / 108 / 3,683 |
| family-linked / project-authored / bounded tuned / legacy generic | 555 / 27 / 9 / 3,092 |
| completion core-linked / reviewed denominator | 542 / 628 |
| family: Mesh / Sprite / Mesh Particle / Sprite Particle / Local Decal / Trail·Ribbon | 1 / 0 / 2,088 / 1,552 / 21 / 20 |
| six-family 트리 밖의 기존 Light | 1 |
| missing resource / unsafe path / duplicate slot | 0 / 0 / 0 |

`555 family-linked`와 `33 patterns 전체 source-exact 완료`는 같은 뜻이 아니다. 3,683개
제품 element는 아직 native source material equation이 아닌 공용 family renderer를 소비하므로
material fidelity는 `FAMILY_LITE`로 표기한다. 엄격한 completion core가
연결된 pattern은 22/33이고, bounded Trail까지 포함한 source-linked pattern은 23/33이다. branch가
유일하지 않거나 source timing이 현재 animation cue 앞에 있는 occurrence는 추측하지 않고 명시적으로
보류했다. 기존 generic row 3,092개도 사용자 손튜닝을 덮지 않기 위해 삭제하지 않았다.

화면 fidelity는 에이전트가 PASS로 판정하지 않았다. 자동 검증은 document parse, resource prepare,
nonzero draw, cue/catalog/doc join, rollback과 멱등성을 증명하고, 최종 색·크기·속도·위치는 사용자가
All Effects에서 판정한다.

## 1. 캐릭터와 동일한 All Effects 저작 계약

발탄 cue는 하나의 stage 이름에 Effect를 뭉개지 않는다. stable `clipOccurrenceId`와
`occurrenceId`를 사용해 같은 stage 안의 여러 animation/effect beat를 구분한다.

```text
Valtan
  -> Phase
    -> Pattern
      -> Semantic Stage
        -> Ordered Clip Occurrence
          -> Product Cue Occurrence (boss snapshot/follow)
          -> Combat Object Visual (Server world-root)
            -> Unified Effect
              -> Mesh
              -> Sprite
              -> Mesh Particle
              -> Sprite Particle
              -> Local Decal
              -> Trail / Ribbon
```

All Effects에서 Product cue와 combat-object visual은 각각 exact authored document의 `authoringPath`를
연다. High Jump sky axe와 Red Blade projectile은 boss-root cue를 중복 스폰하지 않고
Server가 전송한 world-root transform을 소비한다. Open for Editing,
Play Full Effect, Save와 same-revision Hot Reload는 캐릭터 Product cue와 같은 경로를 사용한다. 저장 성공은
선택된 document와 renderer/prewarm cache만 transactionally 교체하고, 이미 재생 중인 occurrence는 기존
shared resource를 유지하며 다음 spawn부터 새 document를 사용한다. 실패하면 이전 document/cache를
보존한다.

Light와 ScreenPost는 이번 공용 6-family 손튜닝 트리의 신규 admission 범위가 아니다. 기존 Whirlwind
Light 한 개는 runtime canary로 유지하지만 신규 standalone Light와 명백한 generic Dust는 source
inventory의 deferred disposition으로 남겼다.

## 2. 적용한 source family

### 2.1 Reviewed source family

24개 reviewed source sequence에서 product clip/cue 시간 안에 안전하게 들어오는 source carrier를
missing-only로 적용했다.

- reviewed source documents: 36
- source elements: 279
- 실제 renderer drawable sweep: 279/279
- 두 번째 apply: changed 0
- source rebase / deletion: 0 / 0

Portal Rush의 portal, rush, recovery 3문서 24개도 이 집합에 포함된다. 포탈은 임의의 원형 mesh를 만든
것이 아니라 source `Portal02_01/02`와 `Dash01_1`의 실제 mesh/sprite carrier를 사용한다.

### 2.2 FRONT_BACK_FRONT 3연격

Animation 담당 정본은 active `mesh_att_battle_19_01` 단일 clip occurrence다. 이를 임의의 animation
3개로 바꾸지 않고, 한 clip 안의 source visual timing을 네 개 cue로 분리했다.

| wave | source start | elements | 의미 |
|---|---:|---:|---|
| 01 | 1,169ms | 25 | 첫 검격 visual |
| 02 | 2,253ms | 25 | 둘째 검격 visual |
| 03 | 3,224ms | 25 | 셋째 검격 visual |
| auxiliary | 4,220ms | 25 | damage hit로 세지 않는 후속 visual |

네 document 100개 element 모두 actual drawable sweep을 통과했다. 기존 aggregate cue/doc는 삭제하지 않고
source wave 네 개를 additive cue로 붙였다.

사용자가 지정한 `fx_h_wave_04`, `fx_b_decal_001`, `fx_i_shockwave_02_ycl` 기반 검격·바닥 impact는
source 420637 direct join이라고 위장하지 않고 `PROJECT_TUNED` supplemental 6개로 분리했다.

### 2.3 안전한 누락 clip과 Trail/Ribbon

기존 cue가 없어 source core가 제품에 도달하지 못하던 네 구간을 별도 cue/document로 닫았다.

| 구간 | core | adapter | 합계 |
|---|---:|---:|---:|
| SWING active clip 02 | 141 | 0 | 141 |
| FOUR_SLASH active clip 02 | 19 | CascadeRibbon 1 | 20 |
| BACKSTEP windup trails | 0 | AnimationTrail 3 | 3 |
| JUMP_SPIN spin trails | 0 | AnimationTrail 3 | 3 |
| 합계 | 160 | 7 | 167 |

167/167 element가 actual prepare/nonzero draw를 통과했고 두 번째 apply는 changed 0이다. 이 배치의
FourSlash 20개에 별도로 증명한 weapon-bone Trail 1개를 missing-only로 더해 현재 canonical
FourSlash document는 21개이다. 기존 20개 row의 정본 identity는 바꾸지 않았다. Backstep과
JumpSpin의 six AnimationTrail은 source notify/target/material identity는 보존하지만 Whirlwind의 409-sample
baked history를 재사용하는 `BOUNDED_RECONSTRUCTION`이다. source-exact geometry라고 과장하지 않는다.
history/target이 없는 다른 TrailGhost/AnimationTrail은 unresolved로 남겼다.

### 2.4 Weapon-bone bounded Trail 보강

cue/binding/sequence를 바꾸지 않고 공식 `b_wp_r_01` anchor를 따르는 3-emitter Trail family를
다음 세 구간에 missing-only로 추가했다.

| 구간 | 시작 / 지속 | Trail |
|---|---:|---:|
| WHIRLWIND recovery | 0 / 0.196547s | 3 |
| GROUND_WAVE_SMASH windup | 0.394663 / 0.476573s | 3 |
| JUMP_SPIN recovery | 0 / 0.196547s | 3 |

총 9개는 `PROJECT_TUNED + BOUNDED_RECONSTRUCTION`이다. stationary root에서 moving bone을 샘플해
9개 모두 Trail point·draw가 생성되고, stationary anchor는 9개 모두 제출을 억제했으며,
missing anchor 3건은 playback transaction을 rollback했다. 원본 도끼 궤적을 native material
equation까지 완전 복원했다는 주장은 하지 않는다.

## 3. 사용자 우선 장판·도끼 연출

다음 연출은 source 증거와 project-authored presentation을 receipt에서 분리했다. 프로젝트 선택 texture를
사용했다고 해서 source-exact로 재분류하지 않았다.

### 3.1 돌진

- 전방 붉은 경로: `fx_c_line_003_xcl.dds`
- 보조 pulse: `fx_d_atypical_042_ycl.dds`
- disposition: `PROJECT_TUNED`
- actual draw: PASS

`line_003`의 exact source 근거는 Dash가 아니라 Floor Wipe의 `Atk09_02`다. Dash source의
halfsphere/hemisphere shield system은 존재하지만 현재 420604 product split과 source timing을 안전하게
join하지 못해 exact import하지 않았다. 따라서 붉은 경로는 손튜닝 가능한 presentation이고, 방패 source
family는 branch/timing review 전까지 보류다.

### 3.2 도넛

- inner/outer boundary: `fx_c_ring_002.dds` + `fx_c_ring_004_cl.dds`
- 작은 원 timeline scale growth
- inner/outer impact: `fx_d_atypical_009.dds` 조합
- 네 project element actual draw: PASS

`ring_002`는 Magic Choice outer-end `Atk03_03`에서 exact evidence가 있다. `ring_004`는 Dash shield
material 문맥의 evidence이므로 도넛 두 경계 전체는 project-tuned로 기록했다. scale, color, UV speed는
Unified Effect detail에서 손튜닝한다.

### 3.3 6방향·원형 전멸 장판

기존 Floor Wipe 6축 Local Decal은 보존했다. 한 축의 양방향이 합쳐져 여섯 방향을 만든다.

- center guide: `fx_d_atypical_032.dds`
- center impact: `fx_d_atypical_009.dds`, `032`, `028`, `011`
- 기존 6축 preserve: 6
- 신규 project element: 2
- actual draw: PASS

`fx_c_line_003_xcl`과 exact six-direction `Atk09_02`는 source clip `15_03`에 있으나 Animation 담당 제품
chain에는 없다. animation sequence를 임의 삽입하지 않고 source-only unresolved로 유지했다.

### 3.4 HIGH_JUMP 도끼

AIRBORNE boss-root cue는 중복 시각 소유권이었으므로 폐기했다. Server는 AIRBORNE 진입 시
살아 있는 player별로 `combatobject.valtan.high-jump.target-axe`를 생성하고, BossCatalog의
`combatobject.visual.valtan.high-jump.target-axe.v1` 행이 `effect.valtan.sky-axe.active`를 world-root에
재생한다. 하나의 combat object는 다음 3-family를 소유한다.

- target Local Decal 1: 0..1.2s
- 공식 `Character/Valtan/ValtanWeapon.wmodel` 낙하 Mesh 1: y=15 -> 0, 1.2s
- ground impact Sprite Particle 1: 1.2..1.85s

player가 3명이면 Server가 combat object를 3개 생성하므로, 과거처럼 boss local offset 3개를
고정해 두지 않는다. actual WARP proof에서 fixed world root 1, submitted bounds 3,
mesh/decal/impact 전환 0/1.19/1.2/1.5/1.85s와 exact lifetime-end 억제를 확인했다.
AIRBORNE gameplay hit shape와 damage는 NONE이고 LAND의 기존 1-hit만 유지했다.

## 4. Whirlwind 회귀 canary

기존 Whirlwind active는 변경하지 않았다.

```text
base particle       5
baked AnimationTrail 3
first-edge Light     1
total                9/9
history samples      409
playback clamp       1.2000000477s
```

formatVersion 2 pattern binding의 `valtan.attack.whirlwind.active.clip.01`을 exact join하도록 canary를
갱신했다. legacy CRLF authoring/history raw byte identity와 기존 Whirlwind runtime program SHA는 보존했다.

VisualProgram 결과:

- corpus: rows 135 / schedules 35 / supplemental 15
- runtime: programs 16 / rows 135
- Whirlwind supplemental 4개 불변
- safe-gap AnimationTrail supplemental 6개 추가
- FourSlash CascadeRibbon은 ordinary sourceRecipe이며 sidecar에 중복하지 않음

## 5. 자동 검증 상태

### 5.1 PASS

- Encounter -> binding -> cue -> Catalog -> authored product join: dangling/duplicate 0
- Catalog 밖 authored-only legacy 문서 4개는 제품 join 분모와 분리해 보존
- resource references 12,159: missing/unsafe/duplicate slot 0
- Reviewed drawable proof: 36 documents / 279 elements
- FRONT_BACK_FRONT drawable proof: 4 / 100
- Project priority drawable proof: 9 documents / 17 appended elements
- Safe reviewed gaps drawable proof: 4 / 167
- FourSlash weapon-bone Trail proof/application: 1 draw / changed 0
- Combat-object drawable proof: sky-axe 3 families + Red Blade source 5
- bounded weapon Trail proof/application: 3 documents / 9 elements / changed 0
- 모든 applicator post-apply check: changed 0
- Whirlwind canary: 9/9, 409 samples, 1.2초 clamp
- VisualProgram corpus/runtime check: 135 rows, 16 programs, 15 supplemental
- focused Python pipeline tests 및 runtime tests PASS
- `Test-EffectPipeline.ps1` contract tests 40/40 PASS
- boss pattern Effect mapping validator PASS
- Character WModel element 경계/rollback harness PASS
- null CDO ParticleModuleSize fallback harness PASS
- Debug/Release Whirlwind/Backstep/JumpSpin VisualProgram Playback -> shared Ribbon renderer WARP 제출 PASS
- Valtan binding/cue/occurrence scheduling focused harness PASS
- Client x64 Debug/Release full build/link PASS
- ClientFrontendHarness x64 Debug/Release build/link PASS

### 5.2 통합 Publish 현재 상태

최신 `origin/main@0f884326`과 PR #139 build repair를 통합하고 Effect Data project registration을
동기화했다. 이어서 다음 publish gate를 순서대로 완료했다.

1. `Publish-Effects.ps1 -Mode Validate`: 204 catalog entries PASS
2. `Test-EffectPipeline.ps1`: 42 tests PASS
3. `Publish-Effects.ps1 -Mode Publish`: 204 Effects, VisualProgram 16개/135 rows 게시 PASS
4. 게시 직후 `Publish-Effects.ps1 -Mode Validate`: PASS

게시된 runtime Valtan 집합은 108개다. `effect.valtan.sky-axe.active`와
`effect.valtan.red-blade-wave.active`가 combat-object owner로 존재하고, retired
`effect.valtan.high-jump.airborne`은 없다. 따라서 authoring의 106 boss-root cue + 2 combat-object
visual partition과 제품 runtime identity가 일치한다. Debug/Release 실행형 회귀는 별도 최종
matrix로 기록하며 Client/UI visual PASS는 사용자가 직접 판정한다.

### 5.3 최종 자동 회귀

- Valtan focused Python: 218 tests PASS, optional schema dependency 1 skip
- FBF composition 음성 경계: overlay/plan 자체 재해시와 application receipt 위조 2건 모두 fail-closed
- ClientFrontendHarness Debug: 9/9 관련 mode PASS
- ClientFrontendHarness Release: 7/9 mode PASS; incremental/loading prewarm의 Valtan
  105 player / 106 boss / 2 combat ownership과 108개 product coverage assertion은 모두 PASS했다.
  두 mode의 유일한 실패는 Release에서 의도적으로 비활성인 Debug-authoring same-revision
  replacement smoke 1건이며 Release 제품 Valtan 재생 경로의 실패가 아니다.
- Engine Release -> `UpdateLib.bat Release` -> Harness Release link: PASS
- Client full incremental Debug/Release build/link: PASS
- Gameplay Validate: 33 patterns / 130 stages / 2 combat objects PASS
- Effect Validate: 204 catalog entries, VisualProgram 16개/135 rows PASS
- Effect Data project registration: 1,821 files / 205 filters PASS
- JSON/XML parse와 `git diff --check`: PASS

## 6. 명시적으로 남긴 경계

다음 여덟 pattern은 source branch 또는 visual signature가 유일하지 않아 source-exact라고 추측하지 않았다.

- DASH_CHARGE
- STOMP
- BIND_CHARGE_SMASH
- FOUR_PILLARS_105
- ENTRANCE_WHIRLWIND
- MAGIC_ORB_STAGGER_76
- CENTER_GRAB_COUNTER_64
- ARENA_BREAK_33

EARTHQUAKE_SMASH는 reviewed branch지만 68개 core occurrence가 현재 cue 시작보다 앞서 있어
`NEGATIVE_BEFORE_CUE_START`다. cue 시간을 소급해 다른 presentation을 깨지 않고 보류했다.

`VALTAN_ARENA_BREAK_84`는 세 stage 모두 animation binding/cue/damage가 없는 environment/world-destruction
mechanic이다. boss Effect cue completion 분모에 넣지 않았다.

현재 ownership-aware visual이 없는 semantic stage는 30개, binding은 있으나 boss cue나
combat-object visual이 없는 clip은 35개다. 그중 source core
누락은 0이고 남은 reviewed core 86개는 모두 negative timing이다. 나머지는 branch 미확정, source mapping
부재, environment ownership 또는 visual이 없는 pacing stage다.

## 7. 사용자 눈검증·손튜닝 경로

사용자는 새 Client 빌드에서 다음 순서로 확인한다.

```text
F1
  -> Effect Tool
  -> All Effects
  -> Valtan
  -> Phase
  -> Pattern
  -> Semantic Stage
  -> Ordered Clip Occurrence
  -> Product Cue Occurrence
  -> Open for Editing
```

우선 확인 순서:

1. WHIRLWIND active 9/9 회귀
2. FRONT_BACK_FRONT source wave 01/02/03와 auxiliary timing
3. HIGH_JUMP player별 combat-object target -> official axe descent -> impact -> Valtan landing
4. DASH_CHARGE red line/pulse의 방향·길이·색
5. MAGIC_CHOICE inner 성장/outer boundary/impact UV
6. FLOOR_WIPE 6축, center guide와 center impact
7. PORTAL_RUSH portal/rush/recovery source carrier
8. SWING clip02, FOUR_SLASH clip02, Backstep/JumpSpin Trail
9. WHIRLWIND recovery, GROUND_WAVE windup, JUMP_SPIN recovery weapon-bone Trail

자동 draw PASS는 최종 visual fidelity PASS가 아니다. 사용자가 색, scale, UV 속도, lifetime, local transform을
손튜닝한 뒤 Save/Hot Reload로 다음 cue spawn을 확인한다.

## 8. 2026-08-22 latest sequence·family-rendering 후속 결과

### 8.1 sequence와 effect ownership

최신 pattern 작업자의 sequence를 행 단위로 통합했다. 현재 정본은 33 patterns,
130 semantic stages, 130 action bindings, 137 ordered clips이다. 기존 병합 회귀였던
duplicate `BeginPattern`과 hit damage 이중 소유를 제거하고 Server contract-test failures 0을 확인했다.

Boss-root cue는 106개이고, High Jump sky axe와 Red Blade projectile 2개는 Server combat-object
visual이다. publisher는 combat-object owner action에 boss-root cue가 하나라도 다시 생기면
effect ID가 달라도 fail-closed한다.

### 8.2 완료한 추가 draw proof

- X/Z Local Decal footprint 교정 후 project priority 9-document / 17-element actual sweep PASS
- FourSlash `b_wp_r_01` Trail: moving anchor draw 1, stationary suppress 1, missing-anchor rollback PASS
- High Jump sky-axe: fixed world-root mesh/decal/sprite transition PASS
- Red Blade source family 5개: moving world-root late-seek draw PASS
- Whirlwind recovery, Ground Wave windup, Jump Spin recovery Trail 9개: moving draw 9,
  stationary suppress 9, missing-anchor rollback 3 PASS

모든 applicator는 missing-only이고 두 번째 apply/check에서 canonical source row를 재작성하지
않는다. 이 proof는 draw 제출과 transaction boundary를 증명하지 사용자 visual fidelity를
대신 판정하지 않는다.

### 8.3 공용 color pipeline 의존성

발탄 전용 shader나 filename switch는 추가하지 않았다. 현재 generic `base.a` / `mask.r`
고정 해석으로는 alpha coverage를 가진 `fx_c_ring_002.dds`와 alpha가 항상 1인
`fx_c_line_003_xcl.dds`, `fx_d_atypical_032.dds`의 의미를 완전히 닫지 못한다. lane별
role/channel/color space/sampler, explicit coverage owner, base-radiance emissive, scene-linear HDR 계약은
별도 class-neutral color pipeline이 main에 통합된 뒤 소비한다.

Magic Choice Sprite Particle companion은 이 ABI를 요구하므로 candidate-only로 보존했고
canonical에 적용하지 않았다. 이것은 누락을 숨기는 것이 아니라 기존 캐릭터·발탄
family 색 경로를 함께 수정하기 위한 회귀 차단이다.

### 8.4 현재 판정

- latest sequence / combat ownership: PASS
- family candidate actual prepare/draw/finite/rollback: PASS
- common color ABI integration: PENDING
- final runtime Publish after latest main merge: PASS (204 Effects, Valtan 108, VisualProgram 16/135)
- Debug/Release Client build and Valtan executable regression: PASS (Release Debug-authoring smoke 제외 경계 명시)
- user All Effects eye check / visual PASS: NOT RUN / NOT GRANTED
