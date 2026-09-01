# 발탄 패턴 마무리 구현 계획서

## 0. 목표와 기준점

- 기준 branch는 `origin/main`의 `d719ceca199a47bc19f8bd93e275a8b1955af434`에서 분기한
  `codex/valtan-pattern-finale-runtime`이며, 통합 전 최신 `origin/main`의 Kakul #292를
  merge해 같은 Product 기준으로 검증한다.
- 피자 패턴과 이미 검증된 회전/후속 decal 계약은 이번 변경에서 수정하지 않는다.
- 발탄 패턴 정본은 `Data/Valtan/Valtan.gameplay.json`과
  `Data/Valtan/Valtan.presentation.json`이다. Product encounter, animation binding,
  Server bootstrap은 기존 Valtan projection/publisher를 통해서만 갱신한다.
- 패턴 성공과 실패는 한 패턴 안에 서로 배타적인 분기 stage를 무한히 늘리지 않는다.
  입력을 받는 본 패턴은 성공 outcome만 발행하고, 공용 `GROGGY`는 별도 후속 패턴으로 전환한다.
- phase 3 유령 발탄은 별도 HP를 가진 가짜 actor를 만들지 않는다. 기존 primary Valtan의
  NetEntityId, HP, damage authority를 유지하고 presentation/visibility/position만 Server snapshot으로 바꾼다.
- Effect V1/V2를 한 authoring 진입점에서 조회하기 위한 얇은 공용 facade
  `EffectResourceCatalog.h/.cpp`를 추가하고 `.vcxproj`와 `.vcxproj.filters`에 등록한다.
  V1/V2의 물리 저장 형식과 runtime renderer는 유지하되, 도구 소비자는 공용 stable resource
  identity와 backend kind를 통해 조회한다.
- Client 실행과 화면 판정은 사용자가 직접 한다. 자동 검증은 데이터 계약, Server runtime,
  packet, Product/Core build까지 수행한다.

## 1. 현재 실측

### 이미 연결된 계약

- `VALTAN_SILENCE_SLOT`은 Server silence deadline과 Client HUD까지 연결돼 있다.
- `VALTAN_BIND_SLOT`은 랜덤 생존자 잠금, 공중 위치, Server 이동/스킬 차단과 원위치 복구가 연결돼 있다.
- `VALTAN_TRIPLE_COUNTER`는 세 counter window와 세 번째 실패 wipe를 이미 가진다.
- phase 3 revive 뒤 primary Valtan은 ghost presentation으로 바뀌며 저장된 여섯 패턴을 순환한다.
- 네 방향 portal combat object는 최초 1회와 이후 5초 간격으로 무한 반복된다.

### 요구와 다른 부분

- Silence는 사자후와 동시에 시작해 2633ms만 유지되고, R 하나가 아니라 모든 skill slot을 붉게 덮는다.
- Bind는 8533ms 동안 Y +10m 상태를 유지해 정확한 5초가 아니며 Client 입력 capture가 없어
  Server가 버릴 입력을 계속 보낸다.
- `VALTAN_STAGGER_SLOT`은 5초 stagger gauge 100 placeholder이며 Y +5m, 실제 HP damage 1000,
  마지막 attack/wipe 분기를 가지지 않는다.
- Triple counter 성공은 groggy 전환이 아니라 다음 counter stage로 계속 진행하며 판정 범위도
  전방 180도가 아닌 작은 원이다.
- 유령 발탄은 여섯 패턴 사이에 사라지거나 랜덤 위치로 이동하지 않는다.
- portal 발탄은 각 꼭짓점에서 다음 꼭짓점이 아니라 중심을 지나 반대 꼭짓점으로 이동한다.
- 최신 main의 Debug Core는 별도 Effect V2 validator에서 명시적 resource root보다
  `LOSTARK_RESOURCE_ROOT` 환경 변수를 우선하는 회귀 때문에 2개 test가 실패한다.

## G00. 데이터와 분기 정본 고정

### 수정 파일

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`

### 변경 계약

- 성공 연출은 target이 없는 `AUDITION_ONLY` 공용 후속 패턴
  `VALTAN_GROGGY_FOLLOWUP`으로 분리한다.
- 본 패턴의 성공 branch는 `nextPatternId`로 후속 패턴을 요청한다. 실패 branch는 본 패턴의
  마지막 공격 stage까지 진행한 뒤 그 contact frame에서 wipe를 한 번만 발생시킨다.
- 새 parser 필드는 unknown kind, 0/음수 threshold, non-finite angle, dangling pattern ID,
  자기 참조와 cycle을 admission에서 거부한다.
- authoring source를 먼저 바꾸고 projection 결과를 생성한다. 생성된 Product JSON과 bootstrap을
  직접 손으로 서로 다른 값으로 편집하지 않는다.

### 데이터 흐름

```text
Data/Valtan authoring
  -> valtan_tuning_pipeline projection/validation
  -> Data/Encounters/Valtan + Data/Animation/Authored/Valtan
  -> Publish-GameplayBalance
  -> Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap
  -> CGameplayCatalog parse/validate/stage/commit
  -> CValtanBrain pinned pattern revision
```

## G01. Silence와 Bind

### Silence

- 기존 `STEP_01` 2633ms에서는 사자후 animation만 재생한다.
- 다음 `SILENCE_HOLD` stage를 정확히 5000ms로 추가한다.
- `SILENCE_HOLD` ENTER/EXIT가 `SET_PLAYER_SILENCE`를 소유한다. 사자후 도중에는 silence가
  시작되지 않고, 사자후가 끝난 다음 tick부터 150 server tick 동안 유지된다.
- Server skill denial은 유지한다. HUD는 기존 R slot에 현재 resolve된 icon texture를 유지한 채
  R 한 칸에만 silence tint/cooldown sweep을 표시한다. 모든 slot의 synthetic cooldown 변조는 제거한다.
- 별도 binary image asset과 새 wire field는 추가하지 않는다.

### Bind

- `STEP_01`을 5000ms hold와 3533ms boss recovery로 나눈다.
- 랜덤 생존자 선택과 Y +10m는 기존 Server authority를 유지한다. 5초 stage ENTER에서 저장하고
  EXIT에서 navigation-valid 원위치로 복구한다.
- 실제 `GRABBED/BOSS_LEFT_HAND` attachment는 world Y hold와 서로 다른 transform owner라 겹치지 않는다.
  대신 Trash grab과 동일한 입력 불가 결과를 `isPatternBound` capture gate에 연결한다.
- move, skill press/release, aim, Esther command가 모두 같은 bound 상태를 거부한다.
- boss animation은 recovery stage로 이어져 기존 8533ms sequence의 tail을 보존한다.

### 수정 파일

- `Client/Private/CombatHUDViewModel.cpp`
- `Client/Private/MainApp.cpp`
- `Client/Public/PlayerController.h`
- `Client/Private/PlayerController.cpp`
- `Server/Private/GameRoom.cpp`
- authoring/projection/published data와 root-motion 산출물
- status/coverage Python test, Valtan canonical graph test, Server gameplay contract test

## G02. 마력구 damage 분기와 공용 Groggy

### 입력과 상태

- `VALTAN_STAGGER_SLOT`을 magic-orb channel 본 패턴으로 승격한다.
- pattern 시작 시 boss의 base Y를 한 번 저장하고 Y +5.0m를 적용한다.
- channel 동안 `CBossCombatRuntime::Apply_PlayerHit`이 방어, 무적, shield 처리를 끝낸 뒤 확정한
  `BOSS_HIT_RESULT.iHealthDamage`만 occurrence 누적값에 더한다.
- 999 이하에서는 성공하지 않는다. 누적값이 1000 이상이 되는 최초 1회에만
  `HEALTH_DAMAGE_THRESHOLD_REACHED` outcome을 발행한다.
- raw damage, stagger damage, shield가 흡수한 damage와 invulnerable hit는 누적하지 않는다.

### 성공과 실패

- 성공 outcome은 `VALTAN_GROGGY_FOLLOWUP`으로 전환한다.
- success, timeout, abort, phase reset 어느 종료 경로에서도 boss Y를 저장한 base로 정확히 복구한다.
- 실패하면 channel animation을 중간에 끊지 않고 final attack stage로 진행한다.
- final attack animation의 실제 contact frame에서 radius 100 wipe hit pulse를 한 번만 생성한다.
- 성공 outcome과 deadline이 같은 tick이면 기존 outcome-first 규칙을 유지해 성공을 우선한다.

### 수정 파일

- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ServerWorldEntity.h`
- `Server/Public/BossCombatRuntime.h`
- `Server/Private/BossCombatRuntime.cpp`
- `Server/Public/ValtanBrain.h`
- `Server/Private/ValtanBrain.cpp`
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- gameplay/presentation authoring과 generated projection
- publisher validator와 Server contract harness

## G03. 3연속 counter 전체 창

- `COUNTER_1`, `FAIL_ATTACK_1`, `COUNTER_2`, `FAIL_ATTACK_2`, `COUNTER_3`의 전체 sequence에서
  counter 가능한 공격을 받도록 stage clock을 구성한다.
- 기존 `iCounterPower > 0` 조건은 유지한다. 일반 damage를 counter 성공으로 승격하지 않는다.
- 이 패턴의 proxy만 `BOSS_FORWARD_ARC` kind를 사용한다.
- boss forward와 source-to-boss XZ vector의 dot이 0 이상인 ±90도 경계를 포함하고,
  후면과 non-finite 입력은 거부한다.
- 어느 stage에서든 최초 `COUNTER_HIT`은 `VALTAN_GROGGY_FOLLOWUP`으로 즉시 전환한다.
- 끝까지 성공하지 않으면 기존 fail DAG를 보존하고 세 번째 공격 contact frame의 wipe가 실행된다.

### 종료 불변식

- 한 occurrence에서 성공 outcome과 follow-up은 각각 한 번뿐이다.
- counter proxy는 stage exit, occurrence cancel, phase reset에 남지 않는다.
- 기존 circle proxy를 쓰는 다른 패턴의 geometry는 변경하지 않는다.

## G04. phase 3 유령 반복과 사각 portal

### primary ghost 재등장

- 현재 primary `BOSS_VALTAN`의 HP, NetEntityId와 six-pattern cursor를 유지한다.
- 여섯 패턴 중 하나가 끝날 때 `GHOST_HIDDEN`과 `INVULNERABLE`을 한 fixed tick 적용한다.
- 숨김 tick에 brain이 다음 패턴을 선택하지 않도록 gate한다.
- arena spawn center와 half extents 안에서 navigation-valid 후보를 deterministic random으로 찾는다.
  body radius clearance와 deck Y tolerance를 통과한 위치만 commit한다.
- 후보가 없으면 기존 pose, visibility, HP와 cursor를 유지하고 부분 commit하지 않는다.
- 다음 tick에 hidden/invulnerable을 해제하고 다음 저장 패턴을 시작한다.
- Client `CValtan`은 hidden flag 동안 body/equipment render queue 제출만 생략한다. snapshot과
  combat state 수신은 계속하고, flag가 풀리면 같은 primary presentation이 자동 복구된다.

### 네 방향 portal

- combat object direction policy에 `NEXT_RADIAL_SLOT`을 추가한다.
- 한 volley의 네 spawn point를 0/90/180/270 순서로 만들고 각 ordinal의 target을 다음 radial slot으로 둔다.
- 이동은 0->90, 90->180, 180->270, 270->0의 네 변이며 동시에 시작한다.
- 기존 5초 stage와 150 tick scheduler를 유지한다. 속도는 한 변을 5초 안에 완주하도록 하고
  max distance는 arena square 한 변 길이와 일치시킨다.
- portal center는 immutable arena spawn center이며 ghost random reposition으로 움직이지 않는다.

### 수정 파일

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ServerWorldEntity.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/CombatObjectRuntime.cpp`
- `Client/Public/Valtan.h`
- `Client/Private/Valtan.cpp`
- phase 3 authoring/publisher contract와 focused tests

## G05. validator 회귀와 생성물 commit

- `effect_v2_binding_pipeline.validate_binding_document`에 명시적 `resource_root`를 전달한다.
- caller가 준 root가 있으면 환경 변수보다 우선하고, 없을 때만 기존 environment/default 탐색을 사용한다.
- 명시 root와 environment root가 다를 때 명시 root가 이기는 regression test를 추가한다.
- Valtan projection은 `Validate` 뒤 exact artifact staging을 거쳐 `Commit`한다.
- gameplay publisher도 먼저 `Validate`하고 성공한 동일 source로 bootstrap을 생성한다.
- JSON/XML parse, `git diff --check`와 generated drift 검사를 통과시킨다.

## G06. 자동 검증과 수동 확인

### 자동 검증

1. Valtan authoring/presentation/status/coverage Python suites
2. phase 3 primary ghost loop, ghost part swap, square portal contract tests
3. Effect V2 validator tests
4. Gameplay publisher `-Validate`
5. Network protocol harness
6. Valtan canonical graph and Server gameplay contract harness
7. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`
8. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core`
9. `git diff --check`

### 사용자 수동 확인

- Silence: 사자후가 끝난 직후 R slot 하나에만 표시되고 정확히 5초 뒤 해제되는지 확인한다.
- Bind: 랜덤 플레이어가 공중에서 정확히 5초 동안 move/skill/Esther 입력이 막히고 원위치로 복구되는지 확인한다.
- Magic orb: 999 damage 실패, 1000 이상 성공 groggy, 실패 final animation 뒤 wipe를 확인한다.
- Triple counter: 각 구간의 전방 180도 성공과 전체 실패 시 세 번째 wipe를 확인한다.
- Phase 3: 유령이 패턴마다 사라졌다 랜덤 위치에서 재등장하고 여섯 패턴을 반복하며,
  네 portal dash가 5초마다 사각형 네 변을 동시에 그리는지 확인한다.

## 완료 경계

- 구현 완료, 자동 검증 완료, 사용자 육안 검증은 RESULT에서 분리해 기록한다.
- Product/Core가 모두 통과하지 않으면 완료로 기록하지 않는다.
- 물리 Resource pack과 Client 화면 fidelity는 자동 PASS로 기록하지 않는다.
