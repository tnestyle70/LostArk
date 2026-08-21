# 2026-08-21 Valtan Effect Family 연결 구현 결과

## 0. 결과 요약

발탄 제품 presentation은 캐릭터와 같은 다음 경로로 연결했다.

```text
Encounter Pattern
  -> Semantic Stage
  -> ordered Clip Occurrence
  -> Product Cue Occurrence
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
| Encounter patterns / semantic stages | 33 / 127 |
| animation action bindings / ordered clips | 124 / 128 |
| Product cues / cue가 있는 unique clips | 108 / 102 |
| Valtan Catalog rows / editable authored docs | 108 / 108 |
| canonical elements | 3,679 |
| source-linked / project-authored·tuned / legacy generic | 555 / 32 / 3,092 |
| completion core-linked / reviewed denominator | 542 / 628 |
| element kind: particle / trail·ribbon / decal / mesh / light | 3,642 / 10 / 23 / 3 / 1 |
| missing resource / unsafe path / duplicate slot | 0 / 0 / 0 |

`555 source-linked`와 `33 patterns 전체 source-exact 완료`는 같은 뜻이 아니다. 엄격한 completion core가
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
          -> Product Cue Occurrence
            -> Unified Effect
              -> Mesh
              -> Sprite
              -> Mesh Particle
              -> Sprite Particle
              -> Local Decal
              -> Trail / Ribbon
```

All Effects에서 선택한 cue는 exact authored document의 `authoringPath`를 연다. Open for Editing,
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

167/167 element가 actual prepare/nonzero draw를 통과했고 두 번째 apply는 changed 0이다. Backstep과
JumpSpin의 six AnimationTrail은 source notify/target/material identity는 보존하지만 Whirlwind의 409-sample
baked history를 재사용하는 `BOUNDED_RECONSTRUCTION`이다. source-exact geometry라고 과장하지 않는다.
history/target이 없는 다른 TrailGhost/AnimationTrail은 unresolved로 남겼다.

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

AIRBORNE cue/document를 새로 만들고 다음 아홉 element를 연결했다.

- target Local Decal 3
- 공식 `Character/Valtan/ValtanWeapon.wmodel` 낙하 Mesh 3
- ground impact Sprite Particle 3

도끼는 boss가 손에 든 socket actor를 복제하지 않고 snapshot root 아래 독립 presentation Mesh로 둔다.
`useModelMaterial=true`, `modelPreScale=1.0`이며 같은 official WModel cache를 사용한다. geometry와 base
material identity는 공식 asset 재사용이고, 낙하 위치·회전·timing은 `PROJECT_AUTHORED`다.

AIRBORNE gameplay hit shape와 damage는 그대로 NONE이고 LAND의 기존 1-hit만 유지했다. 세 도끼는
presentation-only이며 Client가 damage actor를 만들지 않는다. HIGH_JUMP 9/9와 전체 project batch
24/24가 actual drawable sweep을 통과했다.

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
- Project priority drawable proof: 9 / 24
- Safe reviewed gaps drawable proof: 4 / 167
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

### 5.2 통합 Publish 완료

캐릭터 손튜닝 저장이 끝난 뒤 source hash가 안정된 것을 확인하고 `Publish-Effects.ps1 -Mode Validate`,
`Test-EffectPipeline.ps1`, `Publish-Effects.ps1 -Mode Publish` 순서로 실행했다. Publisher는 direct-authored
Effect 204개와 VisualProgram sidecar를 제품 runtime에 transactionally 게시했다.

게시 뒤 authoring/runtime의 Valtan Catalog는 108/108로 일치하고 누락 asset은 0개다. VisualProgram도
authoring/runtime 모두 16 programs / 135 rows / 15 supplemental로 일치한다. Debug incremental prewarm은
108개 Valtan target 전체를 포함해 10/10 PASS했고, Effect Tool preview 17/17 및 4캐릭터 cue parity 4/4도
Debug/Release에서 PASS했다. Release incremental prewarm의 Debug 전용 replacement 한 항목은
`#if !defined(_DEBUG)` 계약에 따라 비적용이며, 나머지 9/9는 PASS했다.

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

현재 cue 없는 semantic stage는 27개, binding은 있으나 cue가 없는 clip은 26개다. 그중 source core
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
3. HIGH_JUMP target 3회 -> official axe 3회 -> impact -> Valtan landing
4. DASH_CHARGE red line/pulse의 방향·길이·색
5. MAGIC_CHOICE inner 성장/outer boundary/impact UV
6. FLOOR_WIPE 6축, center guide와 center impact
7. PORTAL_RUSH portal/rush/recovery source carrier
8. SWING clip02, FOUR_SLASH clip02, Backstep/JumpSpin Trail

자동 draw PASS는 최종 visual fidelity PASS가 아니다. 사용자가 색, scale, UV 속도, lifetime, local transform을
손튜닝한 뒤 Save/Hot Reload로 다음 cue spawn을 확인한다.
