# Bern Stair Navigation RESULT

작성일: 2026-08-14

상태: Server 권위 경로, 다층 높이 보정, Area별 step guard와 자동 검증 완료.
사용자 실제 보행 화면 확인만 대기한다.

## 1. 증상

Bern 입장 후 우클릭 이동에서 캐릭터가 성채 계단을 밟지 않고 통과해 지나간다.
(사용자 관찰, `Screenshots/5.png`, 2026-08-14 02:25)

## 2. 직접 원인 — Bern은 navigation이 아예 연결된 적이 없다

세 지점이 모두 Bern을 제외하고 있었다.

| 지점 | 상태 |
|---|---|
| `Data/Maps/MapCatalog.json` | Bern만 `navigationRuntime` 미선언 |
| `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | `$grids` 목록에 Bern 없음 |
| `Server/Private/GameRoom.cpp` | navigation 필수 world에 `BERN` 없음 |

결과적으로 `Client/Bin/DataFiles/Navigation/`과 `Server/Bin/DataFiles/Navigation/`에
`LV_BER_BERNCASTLE.navgrid`가 존재하지 않았고, Bern room에서
`m_ServerNavigation.Is_Loaded()`가 항상 false였다.

이동 tick의 Y는 오직 navigation path point에서만 온다.

```cpp
// Server/Private/GameRoom.cpp
float targetY = player.fPositionY;              // path가 없으면 현재 Y 유지
if (player.iMovePathIndex < player.MovePath.size())
    targetY = pathPoint.y;                      // 높이가 바뀌는 유일한 경로
```

`MovePath`는 `Find_Path`로만 채워지고 그 호출은 `Is_Loaded()` 안에 있다. 따라서 Bern에서는
플레이어가 **spawn Y(약 42.25)를 세션 내내 유지한 채 평면을 미끄러진다.** 이것이 계단 통과의
직접 원인이다. Client 표현 문제도 collision 문제도 아니다.

`Data/Navigation/LV_BER_BERNCASTLE.navsource`와 `.navpaint`는 2026-08-08부터 존재했다.
저작 데이터는 있었고 소비자가 없었다.

## 3. 왜 단순히 켜지지 못했나

`Convert-NavigationAuthoringGrid`의 두 안전장치가 Bern grid를 거부한다. 실측값은 다음과 같다.

```text
LV_BER_BERNCASTLE  50x333  cellSize=0.5  walkable=7545
  max adjacent step        = 17.03   (ClassSelect 선언 한계 0.6)
  0.6 초과 인접 쌍         = 1170     (그중 5.0 초과 277)
  walkable connected 성분  = 107      (최대 6517, 50셀 이상은 5개)
```

navgrid는 XZ 셀당 높이를 **하나만** 가진다. Bern 성채는 지상 통로 위에 다리·테라스·아치가
겹치는 다층 구조라 한 셀이 두 층을 표현할 수 없다. 따라서 `MaximumStepHeight`와
`RequireSingleComponent`는 이 Area에 적용할 수 없다.

Valtan도 같은 성격이다. 이미 배포된 `LV_LUT_HEARTRB_ED.navgrid`는 maxStep 19.89,
0.6 초과 5827쌍이며 두 장치를 선언하지 않는다.

### 2026-08-14에 전역 step gate를 넣지 않은 이유

`CServerNavigation::Find_Path`의 이웃 확장은 `m_Walkable[next]`만 검사하고 높이를 보지 않는다.
여기에 전역 step gate를 넣으면 Valtan이 함께 끊긴다(0.6 초과 5827쌍). 또한 Bern에 0.6 gate를
적용하면 spawn에서 도달 가능한 범위가 7545셀 중 3517셀, 높이 40.74~47.46으로 축소되어
**계단 위(57.33)에 아예 도달하지 못했다.** 그래서 당시에는 결함 ridge를 먼저 보정하지 않은 채
전역 gate를 추가하지 않았다. 2026-08-25 후속 수정은 세 ridge의 지상 높이를 먼저 복원하고,
Valtan과 분리된 Area별 runtime policy로 Bern에만 1m gate를 적용한다.

## 4. 반영 내용

- `Data/Maps/MapCatalog.json` — Bern에 `navigationRuntime` 선언 추가
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` — Bern grid 등록. 위 3절 근거를
  주석으로 남기고 두 안전장치는 선언하지 않는다
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` — publisher의 spawn 높이 검사(0.25 이내)를
  통과하도록 `player_2/3/4`의 Y를 baked cell 높이로 스냅. `player_3`은 0.253으로 초과했다
- `Server/Private/GameRoom.cpp` — navigation 필수 world에 `WORLD_ID::BERN` 추가.
  grid 누락·손상은 조용히 평면 이동으로 퇴화하지 않고 room admission 실패로 처리한다
- `Server/Private/ServerGameplayContractTests.cpp` — Bern navigation 계약 3건 추가

2026-08-25 후속 수정:

- `.navpaint` version 3에 resolved cell의 명시적 높이 override를 추가했다. 기존 version 1/2는
  그대로 호환 로드하며 MapTool `Reset`은 walkability와 높이를 함께 bake 결과로 되돌린다.
- 세 corridor ridge(`z=216/215`, `156/155`, `131`)를 실제 지상 높이로 보정했다.
- publisher가 모든 Area에 `.navpolicy`를 생성한다. Bern은 `maximumTraversalStepHeight=1.0`,
  Valtan은 기존 encounter 지형을 보존하도록 `0`(비활성)이다.
- Server A*는 이웃과 diagonal side edge 모두 Area step policy를 검사한다. 이동 적용 직전에도
  같은 정책을 재검사해 stale/smoothed path가 다른 층으로 넘어가면 마지막 정상 위치를 유지하고
  해당 이동 목표를 폐기한다.
- policy 또는 blocker sidecar가 잘못되면 navigation load 전체를 rollback한다.
- `Server.exe --navigation-contract-test`를 추가해 대형 gameplay 계약과 독립적으로 Bern 경로를
  재현할 수 있게 했다.

## 5. 자동 검증

```text
Publish-ServerNavigation.ps1 -Mode Publish
  LV_BER_BERNCASTLE 50x333, cellSize=0.5, walkable=7545, maxStep=17.0317230224609

Server x64 Debug 빌드: 오류 0
Server.exe --contract-test: failures 0 (427행)
  [PASS] Load Bern server navigation
  [PASS] Project all Bern player spawns to baked navigation
  [PASS] Climb the Bern castle stairs along the authoritative path

JSON parse: MapCatalog.json, Gameplay.world.json 정상
git diff --check: 오류 없음
```

`ProjectAudit`은 작업트리 전체 기준 29개 check 실패로 종료했다. 전부
`effect.artist-31470-*`와 진행 중인 Valtan/rendering WIP이며 navigation·map·Bern 관련 실패는
없다. navigation Validate는 Bern 포함 4개 Area 모두 성공했다. 전체 ProjectAudit을 PASS로
기록하지 않는다.

2026-08-25 후속 검증:

```text
Publish-ServerNavigation.ps1 -Mode ContractTest: PASS
Publish-ServerNavigation.ps1 -Mode Validate: PASS (4 Area)
Publish-ServerNavigation.ps1 -Mode Publish: PASS (4 Area)
Server x64 Debug build: PASS
Client x64 Debug build: PASS
Server.exe --navigation-contract-test: navigation failures 0
  [PASS] Bern 1m deck-step guard
  [PASS] spawn-to-stairs path and every path step <= 1m
  [PASS] all three corridor ridges are on the ground deck
  [PASS] invalid runtime policy rejects and rolls back the whole grid
```

정본 전체 Debug 회귀는 Server/Client link까지 성공했지만 navigation 이후의 기존
`Valtan pattern rotation product header is invalid` gate에서 종료했다. 대형
`Server.exe --contract-test`는 현재 작업트리의 기존 broad fixture가 Windows stack overflow
(`0xC00000FD`)로 시작 전에 종료한다. 둘을 이번 navigation PASS로 기록하지 않았고, Bern은 위의
분리된 publisher/실행형 계약으로 검증했다.

## 6. 해결한 authoring 결함 — 아치 상단 대신 지상 통로를 사용한다

계단 자체는 정상적으로 bake돼 있다. `x=24~28` 열에서 `z=130 -> 118`이 47.15에서 49.74까지
셀당 약 0.2씩 매끄럽게 올라간다.

문제였던 마당과 계단 사이 `z=131` 한 줄은 다음처럼 upper surface를 고르고 있었다.

```text
z=131  x=22..30   height 49.33~53.82   surface=1  bakedWalkable=0  paint=WALKABLE
z=130            height 47.15         (계단 시작)
z=132            height 47.14         (마당)
```

baker는 이 셀들을 walkable=0으로 판정했는데 `.navpaint`가 `WALKABLE`로 강제 지정했다.
같은 XZ에 실제 통로와 아치가 겹치며 단일 높이 source가 upper surface를 선택한 것이 원인이었다.

같은 형태의 ridge가 경로상에 3곳 있다.

```text
z=217 -> 216 -> 215 -> 214   +5.40 / -4.93
z=157 -> 156 -> 155 -> 154   +6.55 / -5.93
z=132 -> 131 -> 130          +5.81 / -5.80   (계단 직전 아치)
```

단순히 `WALKABLE`을 제거하면 통로가 끊기므로, version 3 height override로 세 ridge의
walkability는 유지하고 높이만 인접 지상 표면으로 선형 보간했다. 보정 뒤에도 7545 walkable
cell 수는 유지되며 spawn에서 계단 위까지 경로가 존재한다. A*의 Bern 1m edge gate 때문에
남아 있는 지붕·테라스·다른 층의 5~17m 인접 관계는 경로로 사용할 수 없다.

따라서 이전에 예고했던 5~6.5m 순간 상승은 더 이상 정상 동작으로 남겨두지 않는다. 같은 결함이
다시 publish되더라도 A*와 이동 적용 직전의 두 방어가 캐릭터 Y와 카메라가 다른 층으로 들어가는
것을 차단한다.

### 2026-08-25 두 번째 후속 교정

위의 "이동 적용 직전 방어"에는 smoothed waypoint Y 보간 우회가 남아 있었다. 현재 XZ가 낮은
셀 안에 있는 동안 player Y부터 먼 waypoint 높이로 보간되어, 높은 셀 경계에서는 이미 높아진
player Y가 검사를 통과할 수 있었다. 후속 `BERN_RUNTIME_HEIGHT_GUARD` 변경은 매 normal-walk
tick의 Y를 destination navigation cell의 ground로만 정하고, collision slide와
skill/knockback도 같은 current-cell -> destination-cell 높이 검사를 사용한다. 구체적 구현과
검증은 `../08-25/2026-08-25_BERN_RUNTIME_HEIGHT_GUARD_RESULT.md`가 정본이다.

## 7. 알아둘 동작 변화

Bern grid는 `X 123.99~148.99`, `Z -175.94~-9.44`의 좁은 통로만 덮는다. 이제 이 범위 밖을
우클릭하면 `Find_Path`가 실패해 **이동 명령이 무시된다.** 이전에는 평면 위 어디로든 갔다.

gameplay placement 검사 결과 player spawn 4개, trigger, collision box는 모두 범위 안이다.
`npc.bern.beda.guide`만 `x=149.00`으로 0.01 벗어나지만 NPC는 navigation 투영 대상이 아니라
(투영은 boss placement 전용) spawn에 영향이 없다.
