# 2026-09-04 발탄 누락 hit collider · damage · effect · sound 저작 RESULT

브랜치 `GB/KoukuSaydon-Main-Pattern`. 대응 계획은
`2026-09-04_VALTAN_PATTERN_HIT_COLLIDER_AUTHORING_IMPLEMENTATION_PLAN.md`가 소유한다.
이 문서는 현재 working tree의 실제 구현과 지금까지 받은 실행 증거만 기록한다.

현재 상태는 **G00~G11 구현 반영, Debug Product와 Server contract PASS, FullDiagnostic 대기**다.
FullDiagnostic는 실행을 시도했지만 사용자가 눈 검증을 위해 시작한 Client PID 31408과 Server PID 53428이 Debug output을
점유해 admission에서 안전하게 중단됐다. 프로세스를 강제 종료하지 않았고 FullDiagnostic는 PASS로 기록하지 않는다.

## 1. 범위와 완료 상태

| 범위 | 구현 상태 | 최종 검증 상태 |
|---|---|---|
| G00 combat object hit Debug wire | Client Debug mirror와 F1 토글 반영 | source-focused test PASS, 사용자 화면 판정 대기 |
| G01 도넛 크기 | 기존 RING 중심선과 effect world 반지름이 이미 일치해 수치 유지 | 사용자 wire 겹침 판정 대기 |
| G02/G03/G05/G07 누락 stage hit | THREE, twohand, 사자후, rush, whirlwind, CROSS, STRUGGLING, GROUND_ROAR, 피자 타점 반영 | focused validator + Server contract PASS |
| G04 돌 폭발·엄폐 | 4 archetype TIMED hit, same-hitId Sound, active wave 시계, navigation projection과 cover 반영 | focused rock 계약 + Server contract PASS |
| G08 누락 Sound·STAGGER 종료 공격 | 기존 hit semantic Sound, 잡기 6 scope, STAGGER FINAL_ATTACK effect/Sound 반영 | Composition publish와 focused Sound 계약 PASS |
| G09/G11 정렬 gate | effect role/hit alignment와 clip template parity validator 반영 | focused validator PASS |
| G10 즉시 침묵 | STEP_01 ENTER 7633ms가 authoring·projection·bootstrap에 반영 | Server contract PASS, 사용자 runtime 판정 대기 |
| G12 잡기 attachment 위치 진단 | 이번 완료 gate 아님 | 후속 범위 |

## 2. 판정과 연출의 적용 원칙

- 실제 damage 권위는 Server의 stage `hit`, combat object `hits[]`,
  `DAMAGE_GRABBED_PLAYERS` / `EXECUTE_GRABBED_PLAYERS`뿐이다. Client effect가 damage를 만들지 않는다.
- 원본 animation HIT/contact 또는 기존 Server 판정으로 공격 의도를 먼저 확정한다. 그 뒤 이펙트를
  `ATTACK`, `TELEGRAPH`, `STATE`로 분류하고 ATTACK만 같은 contact에 hit·impact Sound를 요구한다.
- 피자 패턴의 회전하는 빨강/노랑 sector는 공격 위치를 예고하는 TELEGRAPH라 회전 중 collider를 만들지 않는다.
  발탄이 찍는 contact의 impact/wave는 ATTACK이고, 그 순간의 원형 stage hit와 impact Sound가 damage를 만든다.
- `validate_valtan_hit_presentation_alignment.py`는 ATTACK binding과 Server hit/Sound의 ±1 tick 정렬을 검사하고,
  `validate_valtan_clip_template_parity.py`는 같은 animation clip occurrence의 hit/effect/Sound parity를 검사한다.
  검토된 연속 hit track, 잘린 clip, 패턴 고유 shape와 기존 추가 pulse만 stable ID waiver로 허용한다.

## 3. 실제 구현

### 3.1 stage hit · effect · sound

- `VALTAN_THREE`는 STEP_01 1617ms, STEP_02 963ms를 추가했다. STEP_03은 기존 첫 damage pulse 500ms를
  보존하고 두 번째 contact만 1350ms에서 공통 twohand 1300ms로 정렬했다.
  - 500ms: occurrence 전용 `boss.valtan.impact` + `G_Voltan2_Attack02_Shot2`.
  - 1300ms: 공통 B3 `boss.valtan.twohand` + semantic impact Sound.
  - clip template에는 `EXTRA_HIT`, `extraHitOffsetsMs [500]` exact waiver를 두어 500ms가 다른 B3 occurrence로
    전파되지 않게 했다.
- `VALTAN_GROUND_ROAR` STEP_01은 기존 stage topology를 유지했다. 한 stage가 hit track 하나만 가질 수 있어
  발구르기 두 번과 뒤 사자후를 `CIRCLE 12m`, `damage.valtan.ledge-roar`, offsets
  `[600, 1300, 2700]`으로 합쳤다. impact는 600/1300ms, 사자후 burst는 뒤 clip 733ms에 재생된다.
  첫 두 contact도 12m/ledge-roar response를 쓰는 의도적 절충은 occurrence의 `HIT_SHAPE`·`HIT_RESPONSE`
  waiver와 사유로 고정했다.
- 피자 패턴은 중앙 착지 STEP_03 267ms, 쿵 STEP_04 2100ms, 사자후 STEP_05 1300ms,
  도약 착지 STEP_07 250ms, 모아치기 STEP_11 150/700/1150ms에 Server hit와 대응 V2/Sound를 배치했다.
  STEP_07은 중앙 기준 CIRCLE 25m이며 살아 있고 아직 폭발 전인 피자 돌의 `coverRadiusM 1.5`를 엄폐로 쓴다.
- 이 밖에 계획서의 twohand, ROAR_CHARGE, SEQUENCE_RUSH, SEQUENCE_WHIRLWIND, CROSS, STRUGGLING,
  SEQUENCE_FOUR와 기존 hit의 impact Sound 누락을 authoring 데이터에 반영했다.

### 3.2 잡기 6 scope와 STAGGER FINAL_ATTACK

- `VALTAN_TRASH`, `VALTAN_TRASH_CATCH_IF`, `VALTAN_TRASH_CATCH_SUCCESS` 각각의 `CATCH_SLAM`과
  `EXECUTE_TAIL`, 총 6 scope에 `G_Voltan2_Attack13_Shot1` semantic cue를 추가했다.
- 이 clip들은 원본 1500ms부터 잘라 재생하므로 cue `startMs 1500`과 clip `sourceStartMs 1500`이 같고,
  stage wall에서는 ENTER 0ms다. 각각 `DAMAGE_GRABBED_PLAYERS`와 `EXECUTE_GRABBED_PLAYERS` ENTER action에 붙는다.
- `VALTAN_STAGGER_SLOT` `FINAL_ATTACK`의 기존 CIRCLE 100m 전멸 hit 2900ms에
  `boss.valtan.six.sonic` V2 group과 `G_Voltan2_Attack25_Shot2` semantic impact Sound를 2900ms로 맞췄다.
  six.sonic은 `b_effectroot` snapshot binding이고 EffectRoles에서 ATTACK/BINDING_START로 검사한다.
- `mesh_att_battle_17_end`의 clip-native Attack15/16 계열 5 event(원본 6행)는 Character Sound catalog와
  Resources payload가 없어 Composition publish를 실패시켰다. builder는 이 exact unavailable 목록만 명시적으로
  건너뛰며, 가용한 `Attack25_Shot2`가 실제 wipe impact Sound를 담당한다.

### 3.3 돌 폭발과 Sound 시계

| combat object | spawn 기준 | hit age | lifetime | active telegraph | primary wave / Sound | secondary wave |
|---|---:|---:|---:|---:|---:|---:|
| ground-roar rock | object 0ms | 5000ms | 6200ms | 4.0s | 5.0s | 5.2s |
| struggling rock | object 0ms | 5000ms | 6200ms | 4.0s | 5.0s | 5.2s |
| part-break rock | object 0ms | 5000ms | 6200ms | 4.0s | 5.0s | 5.2s |
| six-pizza rock | pattern 1000ms | 19500ms | 20700ms | 18.5s | 19.5s | 19.7s |

- 네 archetype은 TIMED CIRCLE 3m, `damage.valtan.stomp`, `coverRadiusM 1.5`를 사용하고
  `presentationEvents`는 비워 중복 폭발을 막았다. part-break는 ground-roar active/explode 문서를 재사용한다.
- 피자 돌 네 root의 placement는 아레나 중앙 반경 10m, 시작각 45°라 XZ offset이 각각 약 `±7.071m`다.
  이 10m는 돌의 배치 반경이고 각 돌이 주는 폭발 collider 반지름 3m와 다른 수치다.
- 폭발 Sound는 별도 5초 timer가 아니다. Server가 각 object-local `hit.trigger.atMs`에 보낸
  `HIT_PULSE(hitId)`를 Client가 받아 같은 `hitId`의 `G_Voltan2_Attack09_ProjExp1`을 즉시 재생한다.
  active effect의 primary `donut.impact.wave.black` 시작도 같은 hit age다. secondary wave만 200ms 뒤의 잔여 연출이다.
- 피자 돌은 pattern 1000ms에 생성되고 pattern 19450ms의 STEP_07 hit 때 object age 18450ms라 아직 살아 있다.
  object age 19500ms(pattern 20500ms)에 폭발하므로 착지 순간 엄폐와 이후 폭발이 모두 성립한다.
- hit을 가진 돌은 Server navigation의 같은 높이 walkable point에 최대 2m로 투영한 뒤 원자적으로 spawn한다.
  같은 stage의 inline hit + rock volley 조합은 GROUND_ROAR STEP_01과 STRUGGLING STEP_04 두 exact tuple만
  publisher/Server parser가 허용하고 나머지는 계속 fail-closed다.
- 피자 엄폐/폭발 검증은 파괴 전 full arena에서 시작하는 audition occurrence 기준이다. 별도 collapse-state guard는
  추가하지 않았으며, 현재 navigation에서 네 root를 원자적으로 투영할 수 없으면 volley를 commit하지 않는 안전 경계만 유지한다.
  Product scheduler는 붕괴 이후나 deferred occurrence로 이 피자 패턴을 선택하지 않아야 한다.

### 3.4 즉시 침묵

- `VALTAN_SILENCE_SLOT` STEP_01 ENTER가 `SET_PLAYER_SILENCE`, duration 7633ms를 소유한다.
- `SILENCE_APPLY` 100ms stage의 event는 비웠다. stage 자체는 기존 topology 보존을 위해 유지한다.
- `Data/Encounters/Valtan/ValtanEncounter.json`과 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`도
  STEP_01 ENTER 7633ms로 투영·publish됐다. 실제 적용에는 새 Server 실행이 필요하다.

### 3.5 자동 정렬 gate

- `Data/Effects/V2/EffectRoles.json`은 V2 resource/binding의 ATTACK·TELEGRAPH·STATE 역할을 소유한다.
- `Data/Valtan/Valtan.hitalignment-allowlist.json`은 연속 damage track과 외부 Product binding 같은 검토 예외만 소유한다.
- `Data/Valtan/Valtan.cliptemplates.json`과 clip parity validator는 같은 clip의 occurrence를 source clock에서
  stage wall clock으로 변환해 hit/effect/Sound를 비교한다. playback rate와 잘린 clip도 같은 변환을 쓴다.
- 두 validator는 `BuildDomains.json`의 `valtan.product`, `Project-ValtanPatternMaster.ps1 -Mode Validate`,
  `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`에 연결했다.

## 4. 회귀 보호

- read-only topology 비교에서 승인된 Warp cadence 변경을 제외한 기존 pattern의
  `stageId`, `actionId`, `stageKind`, `durationMs`, `defaultNextActionId`, `motion`, `branches`는 유지됐다.
- 감사 중 THREE STEP_03을 `[1300]`만 남기면 기존 500ms damage가 사라지는 회귀를 발견해 `[500,1300]`으로 복구했다.
- GROUND_ROAR의 600/1300ms 발구르기가 8m stomp가 아니라 12m ledge-roar response를 쓰는 것은 단일 hit-track 구조에서
  stage 분할 회귀를 피한 명시적 선택이다. 숨겨진 fallback이 아니라 exact waiver와 Server contract 대상이다.
- 다른 작업자가 편집 중인 `Tools/ValtanPipeline/test_action_composition_workbench_regression_oracles.py`는
  수정하거나 되돌리지 않았다.

## 5. 자동 검증

### 5.1 마지막 변경 뒤 확인된 증거

| 검증 | 결과 |
|---|---|
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS. clip parity `13 templates / 34 occurrences / 30 hits / 28 effects / 30 sounds / 26 waivers`; alignment `17 roles / 44 ATTACK bindings / 472 stage hit points / 101 sound-aligned / 22 continuous-track exceptions / 8 external bindings / 9 object hits` |
| Composition publish + validate | PASS, `sourceManifestId 3a73…`. unavailable clip-native Sound의 exact skip와 semantic cue publish 확인 |
| Sound/alignment/clip/status/portal/V2 binding 6-module unittest | `Ran 60 tests`, PASS |
| rock pillar / hit-effect presentation / part-break focused suite | `8/8 + 19/19 + 4/4` PASS. active primary-wave = object hit clock 포함 |
| combat-object Sound exact identity | `1/1` PASS |
| Effect model-view + CROSS rock focused suite | `16/16 + 7/7` PASS |
| `validate_effect_sources.py` | PASS (`directSourceCount=180`, `generatedArtifactCount=0`; 기존 unbound reference 1건 보고) |
| `Publish-GameplayBalance.ps1 -Mode Publish -SkipValtanSplitProjection` | PASS |
| Server Debug compile + `Server.exe --contract-test` | PASS, `failures : 0` |
| canonical typed transaction / Sound owner focused suites | `24/24 + 20/20` PASS |
| world-destruction optional `hitActivation` schema + contract | PASS, revision `d4c6…` |
| `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` | PASS. receipt `out/BuildPipeline/receipts/product.debug.receipt.json`, evidence `out/BuildPipeline/runs/20260904T081301278Z-debug-product-9ca7e4f5.json` |
| 구현 대상 파일 `git diff --check` | PASS. working-copy 개행 warning만 있음 |

Product와 Server contract까지는 마지막 데이터 기준으로 닫혔다. FullDiagnostic와 사용자 화면 판정은 별도다.

### 5.2 FullDiagnostic 상태

| 검증 | 상태 |
|---|---|
| `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic` | PENDING. 사용자 실행 Client PID 31408 + Server PID 53428의 Debug output lock을 admission이 검출해 빌드 전 안전 중단 |

마지막 rock placement/publisher 교정 전의 broad Server contract 실행은 G04 navigation 1건과 동시 실행 case 6건,
총 7건을 실패했다. 해당 데이터·admission 교정, 재-publish와 재컴파일 뒤 `failures : 0`으로 닫혔다.
현재 남은 자동 gate는 실행 중인 프로세스를 사용자가 종료한 뒤의 FullDiagnostic 재실행이다.

## 6. 수동 검증

이번 collider/effect/Sound 변경은 사용자 화면 검증 전이다. 에이전트는 Client/UI를 실행하거나 visual PASS를 대신 판정하지 않았다.

사용자는 최종 Debug build 뒤 Server를 재시작하고 다음을 확인한다.

1. F1 `Live Combat Geometry`의 `Combat Object Hit` wire가 도넛·돌·피자 착지 이펙트 footprint와 겹치는지 확인한다.
2. 피자 회전 sector 중에는 damage가 없고, 각 찍기/착지 contact에만 wire·damage·impact Sound가 함께 발생하는지 확인한다.
3. 피자 STEP_07에서 돌 뒤 플레이어만 살아남고, 돌 primary wave와 폭발 damage/Sound가 같은 순간인지 확인한다.
4. THREE STEP_03에서 500ms 첫 pulse와 1300ms twohand contact가 모두 남았는지 확인한다.
5. 잡기 슬램/처형 6 scope와 STAGGER FINAL_ATTACK 2900ms에 Sound/effect가 한 번씩 재생되는지 확인한다.
6. SILENCE_SLOT 진입 즉시 스킬이 막히고 7633ms duration snapshot이 적용되는지 확인한다.

Warp 포탈은 사용자가 별도 RESULT에서 이미 "깔끔하고 괜찮다"고 승인한 기준선이며, 그 수동 PASS를 이번 신규
collider/effect/Sound 항목의 눈 검증으로 대체하지 않는다.

## 7. 남은 경계와 후속 범위

- Product와 Server contract는 닫혔다. 사용자가 실행한 Client/Server가 종료된 뒤 5.2의 FullDiagnostic를 마지막
  working tree 기준으로 다시 실행해 결과를 추가해야 한다.
- 신규 이펙트·wire·damage·Sound와 즉시 침묵은 6절 사용자 수동 판정이 남았다.
- G12 왼손 잡기 attachment 좌표 진단은 이 변경의 완료 gate가 아니며 별도 후속이다.
- Q/W/E/R 실제 적중으로 TRIPLE_COUNTER 성공 후 Groggy 전환하는 기능은 별도 Server counter 계약으로 구현한다.
- 마지막 유령 발탄 패턴의 본체 고정 순차 스킬 루프, walkable random 보조 유령 1스킬 루프,
  반지름 6m 삼각 포탈 독립 루프는 별도 구현 계획서로 분리한다. 삼각 포탈은 사용자 승인 Warp cadence
  `0.0s 생성 → 0.3s 돌진 → 1.6s 도착 → 1.9s 소멸`을 재사용한다.
- 공유 dirty worktree이므로 이 문서 작성 시 stage/commit/push하지 않았다.
