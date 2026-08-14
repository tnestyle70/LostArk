# Bern Stair Navigation RESULT

작성일: 2026-08-14

상태: Server 권위 경로 연결과 자동 검증 완료. 사용자 실제 계단 보행 확인 대기.
남은 authoring 결함은 6절에 분리해 기록한다.

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

### A*에 전역 step gate를 넣지 않은 이유

`CServerNavigation::Find_Path`의 이웃 확장은 `m_Walkable[next]`만 검사하고 높이를 보지 않는다.
여기에 전역 step gate를 넣으면 Valtan이 함께 끊긴다(0.6 초과 5827쌍). 또한 Bern에 0.6 gate를
적용하면 spawn에서 도달 가능한 범위가 7545셀 중 3517셀, 높이 40.74~47.46으로 축소되어
**계단 위(57.33)에 아예 도달하지 못한다.** 따라서 이번 변경에 step gate를 넣지 않았다.

## 4. 반영 내용

- `Data/Maps/MapCatalog.json` — Bern에 `navigationRuntime` 선언 추가
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` — Bern grid 등록. 위 3절 근거를
  주석으로 남기고 두 안전장치는 선언하지 않는다
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` — publisher의 spawn 높이 검사(0.25 이내)를
  통과하도록 `player_2/3/4`의 Y를 baked cell 높이로 스냅. `player_3`은 0.253으로 초과했다
- `Server/Private/GameRoom.cpp` — navigation 필수 world에 `WORLD_ID::BERN` 추가.
  grid 누락·손상은 조용히 평면 이동으로 퇴화하지 않고 room admission 실패로 처리한다
- `Server/Private/ServerGameplayContractTests.cpp` — Bern navigation 계약 3건 추가

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

## 6. 남은 authoring 결함 — 아치 상단이 walkable로 칠해져 있다

계단 자체는 정상적으로 bake돼 있다. `x=24~28` 열에서 `z=130 -> 118`이 47.15에서 49.74까지
셀당 약 0.2씩 매끄럽게 올라간다.

문제는 마당과 계단 사이 `z=131` 한 줄이다.

```text
z=131  x=22..30   height 49.33~53.82   surface=1  bakedWalkable=0  paint=WALKABLE
z=130            height 47.15         (계단 시작)
z=132            height 47.14         (마당)
```

baker는 이 셀들을 walkable=0으로 올바르게 판정했는데 `.navpaint`가 `WALKABLE`로 강제
지정했다. 같은 줄에서 실제 바닥 높이를 가진 `x=20`(47.45)과 `x=33`(47.19)은 반대로
`BLOCKED`로 칠해져 있다. bake ray가 통로 바닥이 아니라 아치 상단을 샘플링한 것으로 보인다.

같은 형태의 ridge가 경로상에 3곳 있다.

```text
z=217 -> 216 -> 215 -> 214   +5.40 / -4.93
z=157 -> 156 -> 155 -> 154   +6.55 / -5.93
z=132 -> 131 -> 130          +5.81 / -5.80   (계단 직전 아치)
```

전체로는 이런 ridge 셀이 26개이고 그중 11개가 `WALKABLE`로 강제 지정돼 있다.

검증한 사실: 이 강제 지정을 제거하면 **계단이 spawn에서 도달 불가능해진다**(step gate 없이도
stairTop 도달 실패). 즉 현재 문서에서 마당과 계단을 잇는 통로는 이 아치 상단뿐이며,
바닥 높이의 정상 통로는 존재하지 않는다.

`.navpaint`는 `BLOCKED`/`WALKABLE`만 저장하고 높이 override가 없으므로 이 결함은 paint 수정으로
해결할 수 없다. MapTool에서 해당 구간을 다시 bake해야 한다.

### 그래서 이번 변경 뒤 화면에서 보일 것

spawn에서 계단 위까지 Server A* 경로는 189셀이며 대부분 매끄럽다. 다만 위 3개 ridge에서
캐릭터가 약 5~6.5 단위 솟았다가 즉시 내려온다. 계단 구간(47.15 -> 49.74 -> 이상)은 정상적으로
올라간다. 이 튀는 구간은 6절 재bake 전까지 남는다.

## 7. 알아둘 동작 변화

Bern grid는 `X 123.99~148.99`, `Z -175.94~-9.44`의 좁은 통로만 덮는다. 이제 이 범위 밖을
우클릭하면 `Find_Path`가 실패해 **이동 명령이 무시된다.** 이전에는 평면 위 어디로든 갔다.

gameplay placement 검사 결과 player spawn 4개, trigger, collision box는 모두 범위 안이다.
`npc.bern.beda.guide`만 `x=149.00`으로 0.01 벗어나지만 NPC는 navigation 투영 대상이 아니라
(투영은 boss placement 전용) spawn에 영향이 없다.
