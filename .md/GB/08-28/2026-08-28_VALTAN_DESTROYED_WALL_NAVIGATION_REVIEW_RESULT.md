# 발탄 아레나 파괴된 벽돌 구간의 이동·Navigation 검토 결과

검토일: 2026-08-28. 상태: **검토 완료 / 제품 수정 없음 / 사용자 화면 검증 미실행**.

기준은 `codex/team-endpoint-10-207-18-103`, HEAD
`d77d7e021edefc435119b23f9aab53315e3f870a`의 현재 working tree다.
다른 작업의 미커밋 변경은 보존했다. 이 검토는 게임 코드, authoring 데이터,
runtime 생성물을 수정하거나 publish하지 않았다. 이 RESULT와 Git 제외 `_work` 진단만 추가했다.

## 검토 결론

**파괴 후 nav를 여는 연결은 이미 존재한다. 이번 현상은 단순히 nav 활성화 기능이 없는 문제가 아니다.**

- 첫 등장 입구 벽 A/B에는 파괴 뒤에도 남는 collision receiver가 각각 하나씩 있다.
  실제 Server collision 소스와 현재 bootstrap으로 플레이어 이동·발탄 돌진 차단을 재현했다.
- 초반 돌진 벽과 내부 벽의 nav region은 서로 겹친다. 해당 벽을 파괴해도 다른 INTACT 벽이
  같은 셀을 소유하면 계속 차단된다. 일부 region은 자기 벽의 실제 위치와도 맞지 않는다.
- 파괴 그룹이 참조하는 103개 nav region의 셀은 모두 base-walkable이다.
  따라서 해당 영역 전체를 다시 WALKABLE로 칠하는 조치는 두 원인을 해결하지 않는다.
- 84/30줄에 실제 바닥이 사라지는 구간은 반대로 막혀야 한다. 벽 제거와 바닥 붕괴를
  하나의 “파괴된 곳은 모두 통행 가능” 규칙으로 합치면 안 된다.

사용자가 말한 벽돌의 exact 위치를 지정한 이미지나 좌표는 제공되지 않았다.
아래에서는 첫 등장 입구 벽, 초반 돌진 벽, 내부 일반 벽을 나눠 검토했다.
입구 receiver 결함 하나로 발탄 일반 추적까지 모두 설명하지는 않는다.

## 현재 데이터와 대상 범위

| 실제 데이터 | 실측 |
|---|---:|
| Area | `LV_LUT_HEARTRB_ED` |
| nav dimensions / cell size | 392 × 312 / 0.5 m |
| nav origin X/Z | -6 / -165 |
| 전체 base-walkable 셀 | 21,524 |
| nav blocker region | 103 |
| WorldEvents group / mutation / enabled binding | 105 / 105 / 157 |
| 벽 group / 바닥 붕괴 group | 99 / 6 |
| 초기 상태 | 105개 모두 `INTACT` |
| gameplay collisionBox | 141 |

Client와 Server의 `.navgrid` bytes는 동일하다. SHA-256은
`563985b86eb6ed167f9eb9d07b4edda0cfe8e766fb858a756cd1571ca06f29b0`이다.
`.navblockers`는 authoring과 생성물의 행 정렬 차이가 있지만 region ID, condition ID,
polarity, 셀 집합을 파싱한 결과는 세 파일 모두 동일하다. Client/Server `.navpolicy`도 동일하다.
발탄의 최대 이동 단차 제한은 `0`으로 비활성이므로 단차 guard를 이번 직접 원인으로 지목하지 않는다.

| 벽·바닥 종류 | group | nav region | 현재 활성 파괴 경로 | nav polarity |
|---|---:|---:|---|---|
| 첫 등장 입구 `entrance.frontwallA/B` | 2 | 2 | 등장 회전 SWEEP collision, 일반 contact | 온전할 때 막음 |
| 초반 돌진 `wall159` | 10 | 10 | opening charge, dash charge, 일반 contact | 온전할 때 막음 |
| 내부 일반 `wall` | 57 | 55 | 일반 contact | 온전할 때 막음 |
| 외곽 `outerwall109` | 30 | 30 | 109 IMPACT, dash charge | 온전할 때 막음 |
| 바닥 `floor84` | 3 | 3 | 3시 지형 파괴 IMPACT | 붕괴한 뒤 막음 |
| 바닥 `floor30` | 3 | 3 | 9시 지형 파괴 IMPACT | 붕괴한 뒤 막음 |

`wall159`와 내부 일반 벽의 109 cleanup binding 67개는 현재 `enabled=false`다.
109 IMPACT의 제품 stage binding은 외곽 30개만 활성이다. Debug audition의 전체 벽 제거와
자연 진행의 109줄 상태를 같은 것으로 취급하면 안 된다.

## 파괴와 이동의 기존 연결

실제 연결은 `GameRoom.cpp`의 `Build_WorldDestructionStateChanges`와
`Commit_WorldDestructionTransaction`에 있다.

```text
Server가 파괴 조건을 승인
  -> BREAKING: 충돌 차단 유지, 해당 impact receiver 비활성
  -> 최종 DESPAWNED/FRACTURED
     -> collision 변경과 navigation condition을 함께 stage
     -> destruction/collision/navigation commit
     -> 플레이어 목적지 재탐색, 발탄 등 entity의 경로 무효화
     -> Shared delta로 Client의 모델 상태 갱신
```

현재 벽의 `breakingDurationMs=250`은 30 Hz에서 8 tick으로 발행된다.
파편 표현이 먼저 원본을 숨기고 최종 commit까지 약 0.27초 동안 이동을 막는 차이는 가능하다.
계속 막히는 현상을 이 짧은 전이만으로 설명할 수는 없다.

`CServerNavigation::Is_CellWalkable`의 판정은 다음과 같다.

```text
base walkable == 1 && 활성 blocker 수 == 0
```

파괴 condition은 blocker 수를 줄인다. base height나 base walkable을 다시 베이크하지 않는다.
현재 103개 region 안에는 base-nonwalkable 셀이 0개이므로, 해당 region은 기본 바닥 재활성화보다
잔존 blocker의 소유권과 별도 collision을 먼저 확인해야 한다.

근거 위치:

- `Server/Private/GameRoom.cpp:148`, `:8494`, `:8529`
- `Server/Private/ServerNavigation.cpp:275`, `:770`
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1:136` — blocker가 base-walkable 셀만 참조하도록 검증

## 입구 앞벽 A/B — 파괴 후 receiver가 남는 확정 결함

첫 등장 파괴 대상은 `frontwallA`의 placement `17404480870746281119`와
`frontwallB`의 placement `12252498194881956917`이다.

| 대상 | collision 중심 X / Y / Z |
|---|---|
| frontwallA | 145.119236 / 23.918500 / -115.034299 |
| frontwallB | 143.800830 / 24.823741 / -114.608120 |

각 벽의 source collision과 receiver는 위치·회전·크기가 동일한 OBB 두 개다.
그런데 최종 mutation의 해제 대상과 receiver ID는 다음처럼 어긋난다.

```text
A의 mutation 해제 대상
  collision.valtan.wallgroup.frontwallA.17404480870746281119
A의 남는 receiver
  collision.valtan.wallgroup.frontwallA.receiver

B의 mutation 해제 대상
  collision.valtan.wallgroup.frontwallB.12252498194881956917
B의 남는 receiver
  collision.valtan.wallgroup.frontwallB.receiver
```

`CServerCollisionSystem::Prepare_StateChanges`는 exact ID와 그 ID 아래 `.` 자식만 변경한다.
위 receiver는 해제 대상의 자식이 아니라 형제이므로 포함되지 않는다.

publisher는 단일 source의 leaf ID를 collisionStateId로 고르면서, 입구 receiver는 별도의
`group.receiver` 이름을 요구한다. 이 두 규칙이 현재 서로 맞지 않는다.
source는 꺼지고 nav도 열릴 수 있지만, 같은 자리에 receiver collision은 남는다.

### 실제 함수 재현

현재 `ServerCollisionSystem.cpp`를 MSVC x64/C++20으로 직접 컴파일한 console 진단에서
실제 runtime의 collision 141개와 벽 mutation 99개의 최종 해제 대상을 읽었다.
`Prepare_StateChanges -> Commit_StateChanges` 후 실제 이동·sweep 함수를 호출했다.
부모 검토에서도 진단 소스를 읽고 같은 실행 파일을 재실행해 동일 결과를 확인했다.

| 검사 | 결과 |
|---|---|
| 파괴 전 active collision | 141 |
| 모든 벽의 최종 해제 적용 후 | **2** |
| 남은 ID | `frontwallA.receiver`, `frontwallB.receiver` |
| 원본 source collision | 두 벽 모두 해제됨 |
| 각 receiver의 player blocking / impact enabled | 계속 활성 |
| 각 receiver만 남긴 사본에서 `Resolve_PlayerMove` | 차단 재현 |
| 같은 사본의 boss wall / receiver sweep | 해당 receiver 충돌 재현 |
| 메모리에서만 두 receiver를 추가 해제한 대조군 | active=0, 동일 이동·sweep 통과 |

이것은 수정 완료나 화면 검증이 아니라 **현재 결함의 실행 재현**이다.
진단의 boss sweep 반경은 1 m이며, 실제 전투 전체 시뮬레이션이나 일반 CHASE 재현은 아니다.

근거 위치:

- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json:2505`
- `Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap:107`
- `Server/Private/ServerCollisionSystem.cpp:861`
- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1:903`, `:925`
- `_work/review_valtan_nav_20260828_server_collision/frontwall_collision_probe.cpp`
- `_work/review_valtan_nav_20260828_server_collision/probe_result.txt`

## 초반 돌진 벽과 내부 벽 — nav 중첩·위치 정합성

아래 수치는 모든 벽이 INTACT이고 바닥은 붕괴하지 않은 기준 상태에서,
표에 적힌 group의 condition만 파괴 완료로 바꿨을 때의 **nav 셀 집합 계산**이다.
실제 플레이 중의 특정 순간을 캡처한 값이나, collision까지 통과한다는 뜻은 아니다.

| 파괴 완료로 가정한 벽 | 해당 region의 고유 셀 | nav가 열리는 셀 | 다른 벽 때문에 남는 셀 |
|---|---:|---:|---:|
| 입구 A/B 둘 | 30 | 27 | 3 |
| `wall159` 열 벽 | 186 | 148 | 38 |
| 입구 A/B + `wall159` | 216 | 175 | 41 |
| 내부·입구·초반 돌진 69개 전체 | 1,399 | 1,394 | 5 |
| 외곽까지 포함한 99개 전체 | 1,580 | 1,580 | 0 |

초기 벽 blocker의 고유 셀 1,580개 중 **239개는 둘 이상의 region이 겹친다**.
한 셀의 최대 owner 수는 3이다. 겹침 자체는 오류가 아니다. 실제로 벽 둘이 같은 공간을
차지하면 둘 다 사라질 때까지 막혀 있어야 한다. 다만 현재 region이 실제 벽 위치를
정확히 나타내는지는 별도로 검토해야 한다.

### 실제로 nav가 하나도 열리지 않는 단독 파괴 사례

`wall159.11047903315509031966`은 region 9셀 전부가
`wall.9335938568718910930`과 겹친다. 앞의 벽만 파괴하면 nav가 열리는 셀은 **0개**다.
이는 두 캐릭터의 공통 A* 경로 차단으로 이어질 수 있다.

다음 세 `wall159`는 셀 `(342,70)` 하나를 함께 소유한다.

```text
10619331645164562008
10758879797263276529
15221142224278623810
```

셋 중 하나만 파괴하면 이 셀은 열리지 않는다.

### 벽 위치와 다른 곳을 막는 사례

위 셀 `(342,70)`의 월드 중심은 `(165.25, -129.75)`다.
하지만 `15221142224278623810` source collision 중심은
`(166.366824, -121.549857)`로, 두 점의 XZ 거리는 **약 8.28 m**다.
이 벽 OBB의 XZ half extents는 `(0.99, 3.8935)`이므로 그 단일 셀은 자기 collision
footprint 밖에 있다. `10758879797263276529`도 같은 셀에서 중심까지 약 4.95 m 떨어져 있다.

`Split-ValtanIndependentWallGroups.ps1`은 기존 큰 region을 가장 가까운 벽 중심 기준으로
분배한다. 셀을 하나도 받지 못한 벽에는 기존 region의 가장 가까운 셀 하나를 공유시킨다.
현재는 이 결과가 실제 벽의 바닥 footprint와 맞지 않는 사례가 남아 있다.

따라서 “어떤 벽을 부쉈을 때 그 자리의 길이 열려야 하는가”를 맞추려면,
각 stable wall ID의 실제 footprint와 nav region을 다시 대조해야 한다.
다른 살아 있는 벽의 차단을 강제로 지우거나 모든 region을 한 번에 해제하는 것은 해결책이 아니다.

근거 위치:

- `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers:266`, `:268`, `:470`, `:472`, `:474`
- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json:1016`, `:1048`
- `Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1:107`, `:142`

### nav region이 없는 내부 두 벽

`wall.15675921917269125843`, `wall.13221745590928612509`는 nav region이 없다.
다만 현재 collision 중심의 base nav는 둘 다 walkable이고 활성 wall region도 덮지 않는다.
따라서 이 두 벽은 “파괴 후 nav가 영구적으로 남는다”의 직접 사례가 아니다.
파괴 전 A*와 collision의 일치 여부를 정리할 때 함께 볼 대상이다.

## 플레이어·발탄·파편을 구분한 영향

| 경로 | 현재 판정과 이번 해석 |
|---|---|
| 플레이어 우클릭 | Server A* 후 실제 이동에서 collision sweep도 소비. nav 중첩과 잔존 receiver 둘 다 영향 가능 |
| 발탄 일반 CHASE | 동일 Server A*를 소비. 잔존 receiver만으로 일반 추적 불가를 설명하면 안 됨 |
| 발탄 stage/root motion | live walkable 샘플로 진행 제한. 잔존 nav blocker 영향 가능 |
| 발탄 돌진 | wall/receiver sweep을 추가 소비. 잔존 입구 receiver 영향 재현 |
| Client 우클릭 목표 | 플레이어 Y 평면 ray 교차로 X/Z를 Server에 제출. Client nav의 파괴 반영 누락이 일반 우클릭의 직접 원인은 아님 |
| 날아가는 벽 파편 | 제품 PhysX actor `iCollisionMask=0`. 표현 전용이라 Server 이동 권위를 차단하지 않음 |

추가로 `Sample_Position` 및 이를 사용하는 smoothing의 `Has_LineOfSight`,
`Resolve_TraversalStep`는 base walkable만 검사한다. A*와 모든 이동 단계가 동일한 overlay
판정을 하는 구조는 아니다. 수정 검증에는 경로 단축, 스킬 이동, 발탄 추적·돌진을 함께 넣어야 한다.

근거 위치:

- `Server/Private/GameRoom.cpp:2096`, `:8167`, `:9391`, `:9829`
- `Server/Private/ValtanBrain.cpp:2324`
- `Server/Private/ServerNavigation.cpp:453`, `:495`
- `Client/Private/PlayerController.cpp:263`, `:763`
- `Client/Private/WorldDestructionDebrisPresentationRuntime.cpp:601`

## 활성화 작업을 한다면 필요한 범위

구현은 이번 검토에 포함하지 않았다. 현재 구조에 맞는 순서는 다음과 같다.

1. **입구 source와 receiver의 파괴 소유권부터 맞춘다.**
   기존 collision state transaction을 유지하고 A/B 각각의 source+receiver가 함께
   해제되는 ID 범위로 publisher와 해당 검증을 맞춘다. runtime 생성물을 직접 수정하지 않는다.
2. **초반·내부 벽의 nav region을 실제 footprint와 대조한다.**
   벽 아래 남는 바닥은 base walkable로 유지하고, 온전한 해당 벽만 blocker를 소유하게 한다.
   현재 멀리 떨어진 한 셀 공유와 잔존 overlap이 실제 형상에 필요한지 구분한다.
3. **base nav 보정은 바닥이 실제 존재하는 누락 셀에만 한정한다.**
   전체 nav 활성화, 파편 collision 추가, 새 navigation runtime은 필요하지 않다.
   실제 빈 공간과 84/30줄 붕괴 sector는 계속 차단한다.
4. **기존 테스트에 통행 결과를 추가한다.**
   source+receiver 해제, BREAKING 동안 차단 유지, 이웃 INTACT 보존,
   단독 파괴 후 목표 위치 도달, 전체 벽 제거 뒤 collision 잔존 0,
   reset·실패 rollback·바닥 붕괴를 같은 경로로 검증한다.

현재 `ServerGameplayContractTests.cpp:5480`은 선택한 159벽의 한 점만 열리고 이웃 벽이
전체 관통 경로를 계속 막는 것을 기대한다. `:13579`의 전체 벽 제거 테스트도 벽 99개
DESPAWNED와 바닥 6개 INTACT를 검사할 뿐, active collision=0이나 실제 통행을 검사하지 않는다.
기존 테스트가 통과해도 이번 receiver 결함은 놓칠 수 있다. 새 테스트 체계보다 이 블록 확장이 적절하다.

## 실행한 검증과 남은 경계

| 실행 | 결과 |
|---|---|
| Navigation publisher `-Mode Validate` | PASS. Valtan 392×312 / walkable 21,524 |
| Navigation publisher `-Mode ContractTest` | PASS. paint 및 blocker acceptance/rejection |
| WorldGameplay publisher `-Mode Validate` | PASS. Valtan 154 placements / 3 spawn groups / 1 prop set |
| ValtanWorldDestruction publisher `-Mode Validate` | PASS. 105 groups / 157 bindings |
| ValtanWorldDestruction publisher `-Mode ContractTest` | PASS |
| 기존 Debug Server `--navigation-contract-test` | 7 PASS / failures 0. 이 전용 모드는 Bern 회귀이며 Valtan 통행 완료 증거는 아님 |
| 기존 Debug Server `--contract-test` | **843 PASS / 1 FAILURE**, 전체 PASS 아님 |
| 현재 ServerCollisionSystem 소스 독립 compile + headless probe | 결함 재현 및 메모리 대조군 확인, exit 0 |
| JSON 파싱 / Client·Server nav parity / region 셀·collision ownership 대조 | 완료 |
| `git diff --check` | exit 0. 기존 working tree의 LF/CRLF 경고는 별도 |
| 제품 전체 Debug/Release 재빌드 | 미실행. 제품 변경 없는 검토이며 사용자 실행 세션을 보존 |
| 실제 Client 조작·스크린샷·육안 판정 | 미실행, 사용자 전용 |

전체 Server contract의 단일 실패는 다음 항목이다.

```text
Reset a missing or retired candidate to packaged idempotently, reject corrupt durable state
without mutation, and refuse a concurrent Server owner
```

테스트는 Debug runtime activation mutex를 먼저 획득해야 한다.
검토 중 이미 실행 중인 사용자 Debug Server가 같은 mutex를 소유하고 있었고,
`OpenExisting` 후 0 ms 획득 검사에서도 `HeldByAnotherThread=True`를 확인했다.
`ServerGameplayContractTests.cpp:2820`과 `ServerApp.cpp:1764`의 소유권 검사와 일치한다.
사용자 Server를 종료해 재실행하지 않았으며 이 실행을 전체 PASS로 기록하지 않는다.

진단 로그:

```text
_work/review_valtan_destroyed_wall_nav_20260828/server-contract-debug.log
_work/review_valtan_destroyed_wall_nav_20260828/server-navigation-contract-debug.log
_work/review_valtan_nav_20260828_server_collision/probe_result.txt
```

## 사용자가 직접 확인할 범위

현재 실행 중인 Server·Client는 종료하거나 조작하지 않았다. 새 게임 바이너리나 runtime 데이터는 없다.
새로 시작해야 한다면 LAN sync 결과에 따른 Visual Studio `Server + Client` profile이 이 PC의 대상이다.

1. 기존 Client의 Lobby에서 `Valtan`에 진입해 첫 등장 앞벽 A/B의 파괴를 관찰한다.
2. 파괴 표시 후 0.3초 이상 지난 뒤 위 X/Z의 벽 자리와 안쪽 바닥 사이를 우클릭으로 왕복한다.
3. 발탄 일반 추적과 돌진을 나눠 확인한다. 플레이어만 막히는지, 돌진도 막히는지,
   둘 다 같은 빈 바닥을 피해 가는지 구분하면 receiver와 nav 중첩을 분리할 수 있다.
4. 별도로 초반 돌진 벽은 해당 벽 하나가 부서진 경우와 인접 벽까지 사라진 경우를 구분한다.
5. 후반 84/30줄에 사라진 바닥은 통행 가능 기대 범위에 넣지 않는다.

현재 소스의 `Level_ValtanArena.cpp:405`는 기존 중복 audition panel을 더 이상 그리지 않는다.
`Render_AuditionPanel`에 남은 `Reset + Break Every Wall (Keep Floor)`와 nav counter 문자열만 보고
지금 UI에서 그 버튼을 누를 수 있다고 안내하지 않는다. 현재 주 도구는 `F1 -> Boss Tool`이다.

이 RESULT는 원인 검토를 완료한 기록이며, 이동 기능 수정 완료나 사용자 visual PASS 기록이 아니다.
