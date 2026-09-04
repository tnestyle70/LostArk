# 2026-09-04 발탄 Q/W/E/R 카운터와 유령 최종전 런타임 RESULT

브랜치 `GB/KoukuSaydon-Main-Pattern`. 직접 대응 계획은
`2026-09-04_VALTAN_COUNTER_AND_GHOST_FINALE_RUNTIME_IMPLEMENTATION_PLAN.md`다. 같은 working tree에서 먼저
반영된 누락 collider·피자·즉시 침묵은
`2026-09-04_VALTAN_PATTERN_HIT_COLLIDER_AUTHORING_IMPLEMENTATION_PLAN.md`, 잡기 C3 제거와 마력구 오라는
`2026-09-04_VALTAN_GRAB_SERVER_ANCHOR_AND_STAGGER_AURA_STAGE_END_IMPLEMENTATION_PLAN.md`가 범위를 소유한다.

이 문서는 위 세 범위가 현재 Product에 함께 들어간 통합 상태와 실제로 전달받은 실행 증거만 기록한다.
현재 상태는 **구현·projection·Debug Product·Debug Core·Debug FullDiagnostic PASS,
신규 화면·소리·조작 검증 NOT RUN**이다.

---

## 1. 범위와 완료 상태

| 범위 | 구현 상태 | 자동 검증 | 사용자 검증 |
|---|---|---|---|
| 마력구 core+aura `STAGE_END` | 완료 | V2 focused와 Debug Product/Core PASS | NOT RUN |
| 피자 회전 sector와 마지막 위치 decal | 완료 | Valtan canonical/target-anchor, model-view, Product/Core PASS | NOT RUN |
| 잡힌 플레이어 C3 hand-bone overwrite 제거 | 완료 | grip contract, Valtan harness, Product/Core PASS | NOT RUN |
| 여섯 class Q/W/E/R 실제 적중 counter | 완료 | Gameplay Validate/Publish, focused Python, Server/Core PASS | NOT RUN |
| `VALTAN_SILENCE_SLOT` 진입 즉시 침묵 | 완료 | status contract, Gameplay publish, Server/Core PASS | NOT RUN |
| 부활한 본체 유령의 6-pattern 순차 loop | 완료 | phase3 focused contract, Server/Core PASS | NOT RUN |
| walkable random 보조 유령의 1-skill loop | 완료 | phase3 focused contract, Server/Core PASS | NOT RUN |
| 외접반지름 7m 동시 정삼각 포탈 loop | 완료 | V2/model-view/phase3, Server/Core PASS | NOT RUN |
| Debug FullDiagnostic | 완료 | **PASS** | 해당 없음 |

기존 일반 Warp rush의 V2 포탈과 1.9초 cadence는 사용자가 별도 RESULT에서 육안 승인했다. 이번에 추가한
유령 최종전의 **동시 3변 정삼각 포탈 occurrence**는 다른 런타임 경로이므로 그 과거 승인을 재사용하지 않고
NOT RUN으로 둔다.

## 2. 기존 계획 통합 결과

### 2.1 마력구 core와 aura의 stage 종료

`VALTAN_STAGGER_SLOT / CHANNEL / valtan.authoring.stagger-slot.channel`은
`boss.valtan.magicball`과 `boss.valtan.magicball.aura` 두 GROUP을 같은 `b_effectroot`, `STAGE` 0ms,
`ONCE`, `FOLLOW_SLOT`, `TARGET_YAW`로 재생한다. 두 binding의 `stopPolicy`는 모두 `STAGE_END`다.

- aura 두 Mesh leaf의 finite lifetime은 CHANNEL과 같은 12초다.
- 누적 response 1000으로 `VALTAN_GROGGY_FOLLOWUP`에 조기 진입하거나 12초 TIMEOUT으로
  `FINAL_ATTACK`에 진입하면 두 group은 같은 stage edge에서 끝난다.
- aura는 `STATE / NONE` 표현이다. aura 자체는 Server hit 또는 damage collider를 만들지 않는다.
- 현재 Mesh stop은 즉시 종료이며 별도 fade-out tail은 이번 범위가 아니다.

### 2.2 피자 sector 회전과 마지막 decal 위치

피자 composite cue는 `arena.center.target-follow`, `follow` 계약을 유지한다. 중심 translation은 저작된
arena center에 고정하고, yaw만 Server가 잠근 대상의 현재 pose를 따라 갱신한다. composite 안의 빨강·노랑
sector particle은 local-space로 같은 root 갱신을 받으므로 패턴 진행 중 중앙에서 대상 방향으로 회전한다.

마지막 착지/사자후 표현에는
`requested.20260827.six-pizza.sector.red-roar-overlay` decal을 composite 19.5초에 배치했다. lifetime은
`0.779999971s`, local yaw는 `-182.75°`로 고정해 마지막으로 확정된 피자 sector 방향에서 재생한다.
이 decal은 별도 action attachment나 transform inheritance를 만들지 않고 같은 composite root를 사용한다.

회전 sector는 TELEGRAPH다. 회전 중 Client effect가 damage를 만들지 않으며, 실제 damage는 기존 계획에서
저작한 STEP_03/04/05/07/11의 Server stage hit과 combat-object hit만 만든다. 즉 sector 회전 복구가 collider
범위나 판정 시계를 다시 바꾸지 않는다.

### 2.3 잡기 C3 제거

Client frame의 마지막에 잡힌 Character transform을 `bip001-l-hand` 행렬로 다시 덮던 C3 presentation 경로를
삭제했다. `CClientReplication`의 attachment presentation staging/update/map과 `CValtan`의 grip runtime cache도
함께 제거했으며, 행렬 합성 전용 harness 등록도 정리했다.

이제 Character body, nameplate와 Debug 표현은 모두 Server가 capture 순간 저장한 boss-local attachment pose를
매 fixed tick 갱신해 보낸 snapshot을 소비한다. `gripLocalOffset`은 authoring/Product의 typed validation
metadata로 남지만 Client hand-bone transform writer는 아니다. 따라서 버러지 잡기와 뒤돌아잡기는 같은 단일
Server snapshot 경로를 쓴다. 실제 animated 왼손 bone을 따라가게 하는 기능은 포함하지 않는다.

### 2.4 즉시 침묵

`VALTAN_SILENCE_SLOT`은 STEP_01 ENTER에서 `SET_PLAYER_SILENCE`를 7633ms 적용한다. 이전
`SILENCE_APPLY` 100ms stage는 topology 호환을 위해 남아 있지만 action은 비어 있다. 침묵은 Client effect나
로컬 입력 추측이 아니라 Server status deadline과 snapshot으로 적용된다.

## 3. Q/W/E/R 실제 적중 카운터

`Data/Balance/PlayerSkills.json`의 여섯 class Q/W/E/R damaging skill 28행에 기존 공용 필드
`counterPower=1`을 저작했다. skill kind와 input slot, damage shape/projectile 경로는 바꾸지 않았다.

```text
Player inputSlot
→ Server-approved skillId
→ 실제 direct/projectile/combat-object overlap
→ CServerCombatHitRuntime::Apply_PlayerToWorld
→ CBossCombatRuntime::Apply_PlayerHit
→ COUNTERABLE stage의 COUNTER_HIT one-shot outcome
→ authored branch / VALTAN_GROGGY_FOLLOWUP
```

- 키를 눌렀다는 사실만으로 counter가 발생하지 않는다. Server overlap이 실제로 성립해야 한다.
- 현재 stage가 `COUNTERABLE`이어야 하고 Triple Counter 전방 proxy, Trash 계열 local-circle proxy도 그대로
  통과해야 한다.
- 허공, 판정 밖, counter window 밖의 Q/W/E/R은 counter outcome을 만들지 않는다.
- `counterPower`는 현 runtime에서 누적 수치가 아니라 0/비0 자격이다. 따라서 임의의 새 threshold나
  Valtan pattern-ID 예외를 추가하지 않았다.
- damage-less counter 전용 guard와 일반 damaging hit 경로는 합치지 않았다.

Gameplay publisher는 damaging ACTIVE/COMBO skill의 명시적 `counterPower`를 허용하되, damage-less guard의
COUNTER-kind 경계는 유지한다. 공식 provenance receipt와 Server bootstrap도 28행의 결과값을 함께 갱신했다.

## 4. 유령 최종전 세 독립 loop

### 4.1 본체 유령의 순차 공격

부활 완료 뒤 primary `BOSS_VALTAN` identity가 다음 여섯 pattern을 ordered loop로 반복한다.

```text
VALTAN_WHIRLWIND
→ VALTAN_FOUR_SLASH
→ VALTAN_SEQUENCE_FOUR
→ VALTAN_CROSS
→ VALTAN_CHARGE
→ VALTAN_CHARGE_2
→ 처음으로 반복
```

같은 본체, HP와 HUD identity를 유지한다. 각 스킬 종료 뒤 본체를 random 위치로 옮기던 product relocation 호출은
제거했으며, 위치 무작위화는 아래 보조 유령 lane만 소유한다. 각 pattern의 기존 Server hit와 Client
animation/effect/sound binding을 재사용한다.

### 4.2 보조 유령의 random 1-skill occurrence

보조 lane은 동시에 최대 한 entity만 유지한다. owner net entity ID와 auxiliary occurrence sequence에 서로 다른
domain salt를 섞어 spawn 위치와 위 여섯 pattern 중 하나를 결정한다. wall clock, 다음 entity ID와 primary
pattern cursor는 random seed가 아니다.

- 후보 XZ는 finale spawn half-extents 안에서 고르고 live navigation exact point와 같은 deck 높이를 검사한다.
- 유령 body 반경 전체를 navigation half-cell 이하 간격으로 검사해 footprint가 walkable한 후보만 commit한다.
- 128개 후보가 모두 실패하면 occurrence sequence와 entity ID를 소비하지 않고 다음 fixed tick에 같은
  occurrence를 재시도한다.
- spawn과 wire payload admission이 끝난 뒤에만 occurrence를 commit한다.
- child sequence는 선택한 pattern ID 하나만 가진다. 그 pattern/follow-up이 끝나면 child source의 combat
  object를 취소하고 lifecycle/despawn을 전송한 뒤 entity를 지운다.
- despawn과 다음 spawn은 서로 다른 fixed-tick edge다. 보조 lane 실패나 대기는 primary loop와 portal clock을
  멈추지 않는다.

### 4.3 외접반지름 7m 동시 정삼각 포탈

포탈 scheduler는 primary/auxiliary action과 독립된 occurrence clock을 쓴다. immutable arena center와 world yaw
0을 기준으로 start angle 30°, step 120°의 세 vertex를 만든다.

```text
circumradius = 7.0m
edge = 7 * sqrt(3) = 12.12435565298m
three routes = V0→V1, V1→V2, V2→V0
speed = edge / 1.3s = 9.32642742537m/s
```

포탈은 `VALTAN_WARP`와 같은 authored world-transform 이동이며 navigation을 전혀 소비하지 않는다. boss spawn
XYZ를 그대로 기준으로 만든 세 vertex의 finite 값과 정확한 반지름·변 길이를 검사하되 navigation load/sample,
exact-walkable, same-deck, height-transition, line-of-sight 검사나 projection은 하지 않는다. 한 vertex/edge라도
finite·geometry 검증에 실패하면 일부만 spawn하지 않고 occurrence 전체를 거부하며, 검증 뒤 하나의 radial volley
transaction으로 combat object 세 개를 동시에 만든다. full-footprint walkable-nav 검증은 보조 유령 spawn에만
남아 있다. 사용자 육안 검증에서는 R7 세 변이 축소 아레나의 nav gap을 의도대로 살짝 가로지르는지도 확인한다.

각 leg의 시간 계약은 다음과 같다.

| 시각 | 상태 |
|---:|---|
| 0ms | 세 vertex의 black+cyan 포탈과 damage proxy 3개 동시 생성 |
| 0~300ms | 포탈 유지, proxy 정지 |
| 300~1600ms | 세 proxy가 각 다음 vertex까지 동시 이동 |
| 1600~1900ms | 도착 뒤 포탈 dissolve tail, proxy lifetime 유지 |
| 1900~2200ms | 포탈이 없는 300ms gap |
| 2200ms | 다음 정삼각 occurrence 동시 시작 |

combat object의 일반화된 `movement.startDelayMs=300`, `expireOnDistanceEnd=false`가 이동 종료와 lifetime 종료를
분리한다. 다른 object는 기본값 `0/true`로 기존 의미를 유지한다. Server hit/damage는 이동하는 세 combat-object
proxy가 소유하며 Client-only damage나 local prediction은 없다.

Client는 기존 `boss.valtan.portal` V2 group을 재사용한다. 각 object root에는 local origin의 black/cyan leaf
한 쌍만 있고 group/leaf lifetime은 1900ms, dissolve 시작은 정규화 `0.84210526`이다. 폐회로에서는 각 leg의
destination이 다음 leg의 start와 같으므로 세 root만으로 세 꼭짓점과 각 edge의 양 끝이 모두 표현된다. endpoint
leaf를 중복 배치해 같은 vertex를 두 번 그리는 overdraw는 만들지 않았다. visual root는 spawn pose에 고정되고
이동 damage proxy snapshot에 끌려가지 않는다.

### 4.4 시작·중단·reset 소유권

phase3 activation이 primary sequence, auxiliary occurrence/due tick, portal occurrence/last-spawn tick을 함께
초기화한다. primary owner가 죽거나 reset/teardown되어 finale owner 조건을 잃으면 dependent child를 despawn하고
각 source의 combat object lifecycle도 취소한다. 세 lane의 cursor는 서로 공유하지 않는다.

## 5. 자동 검증 증거

### 5.1 authoring·projection·focused contract

| 검증 | 결과 |
|---|---|
| Valtan RootMotion `--check` | PASS: 51 patterns / 126 stages / 8064 samples |
| RootMotion unit | 5/5 PASS |
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS: clip parity 13 templates / 34 occurrences / 30 hits / 28 effects / 30 sounds; alignment 18 roles / 44 ATTACK bindings / 472 hit points / 9 combat-object hits |
| `Publish-GameplayBalance.ps1 -Mode Validate`와 `-Mode Publish` | PASS: 65 patterns / 280 stages / 9 combat objects |
| V2 focused + grip/status/portal 묶음 | 66/66 PASS |
| Effect model-view | 16/16 PASS |
| Action Presentation Workbench 묶음 | 49/49 PASS |

첫 FullDiagnostic 시도는 오래된 Valtan master mutation fixture 네 건에서 중단됐다. 이를 현행 계약으로
국소 교정했고 대상 test `2 + 1 + 1`이 모두 PASS했다. 다음 실행에서 구 phase3 Server fixture와 portal의
navigation 결합을 발견해, 본체·보조 유령 계약을 현행화하고 portal을 WARP와 같은 nav-independent transform
경로로 고쳤다. Server 전체 계약 `failures : 0` 확인 후 최종 FullDiagnostic 전체를 다시 실행해 PASS했다.

### 5.2 Debug Product와 Core

| 검증 | 결과 |
|---|---|
| `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` | PASS. Engine / Shared / Server / Client compile·link와 product CSO closure PASS; WARP readback V1=1352, V2=1352 pixels |
| Product evidence | `out/BuildPipeline/runs/20260904T092236649Z-debug-product-02a1ac7c.json` |
| `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core` | PASS, exit code 0 |
| Core Valtan suites | harness 30/30, flow 13/13, tuning 11/11, canonical 7/7 PASS |
| Core network/session | NetworkProtocol build/run과 Character Select isolation live scenario PASS |
| Core evidence | `out/BuildPipeline/runs/20260904T094506332Z-debug-core-b2b37399.json` |

### 5.3 Debug FullDiagnostic

| 검증 | 상태 |
|---|---|
| `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic` | **PASS, exit code 0** |
| FullDiagnostic evidence | `out/BuildPipeline/runs/20260904T103406014Z-debug-fulldiagnostic-da9fe4da.json` |

최종 실행에서 Valtan master 70/70, Server gameplay `failures : 0`, Character Select Core/Party2/Party4,
PointLight, physics/destruction과 WModel 진단까지 모두 통과했다.

## 6. 사용자 육안·청각·조작 검증

아래는 모두 **NOT RUN**이다. 에이전트는 Client/UI를 실행·조작하거나 visual/audio PASS를 대신 판정하지 않았다.
FullDiagnostic를 통과한 새 Debug Server+Client에서 사용자가 직접 확인한다.

1. `VALTAN_STAGGER_SLOT` CHANNEL 시작에 magicball core와 aura가 함께 나오고, 조기 groggy와 12초 timeout
   각각의 stage edge에서 둘 다 사라지는지 확인한다.
2. 피자 패턴의 빨강·노랑 sector가 arena center에서 대상 방향을 따라 계속 회전하고, 후속 red decal이 마지막
   sector 방향에 생성되는지 확인한다. 회전 중에는 damage가 없고 각 impact contact에만 damage/Sound가 있는지도
   함께 확인한다.
3. 버러지 잡기와 뒤돌아잡기에서 Character body, nameplate와 Debug 표현이 같은 Server snapshot pose를 따르고
   별도의 왼손 C3 궤도로 갈라지지 않는지 확인한다.
4. 모든 class의 Q/W/E/R을 Triple Counter와 Trash counter window 안/밖, proxy 안/밖, 실제 적중/허공으로 나눠
   성공 때만 groggy branch로 가는지 확인한다.
5. `VALTAN_SILENCE_SLOT` 시작 frame부터 skill 사용이 막히는지 확인한다.
6. 부활한 본체가 여섯 pattern을 정해진 순서로 반복하고 공격 사이 임의 teleport가 없는지 확인한다.
7. 보조 유령이 walkable random 위치에 최대 하나만 나타나 여섯 pattern 중 하나만 사용한 뒤 사라지고 다음
   occurrence로 반복하는지 확인한다.
8. 세 포탈이 외접반지름 7m 정삼각형의 꼭짓점에 동시에 생기고, damage wire 세 개가 0.3초 뒤 동시에 출발해
   1.6초에 도착하며, 포탈이 1.9초에 완전히 사라진 뒤 0.3초 gap을 두고 반복하는지 확인한다.
9. 본체 사망, encounter reset 또는 방 종료 뒤 보조 유령과 portal combat object/effect가 남지 않는지 확인한다.

## 7. 남은 경계

- 남은 자동 gate는 없다. 다음 단계는 6절의 사용자 육안·청각·조작 검증이다.
- 6절의 신규 시각·소리·조작 결과는 사용자의 서면 판정 전까지 PASS가 아니다.
- C3 제거는 좌표 writer를 단일화한 결과이며 animated 손바닥 추적 기능은 아니다. 그 기능이 필요하면
  Server-authored socket pose를 별도 wire/runtime 수직 슬라이스로 설계해야 한다.
- 마력구 조기 `STAGE_END`는 즉시 제거다. fade-out이 필요하면 V2 leaf stop envelope/runtime 계약을 별도로
  확장해야 한다.
- 피자 돌/엄폐는 붕괴 전 full-arena occurrence를 전제로 하며 붕괴 이후 pattern 선택 guard는 별도 scheduler
  정책이다.
- 구현은 공유 dirty working tree에 있으며 이 RESULT 작성 과정에서 stage/commit/push하지 않았다.
