# 2026-08-23 발탄 진입로 네비게이션 원인 규명과 A* 수직 비용 결과

## 1. 판정

발탄 맵 스폰에서 첫 트리거로 갈 때 "앞으로 갔다가 아래로 갔다가 위로 갔다가" 도착하는
증상의 원인을 실측으로 확정하고 Server A*를 고쳤다.

증상은 두 가지가 겹쳐 있었고 성격이 서로 다르다.

- **지그재그는 결함이다.** Server A*의 간선 비용이 순수 2D라 높이를 전혀 보지 않았고,
  진입로 바닥 위에 얹힌 구조물 상판(약 +5.6m)을 공짜로 타고 넘었다. 그 결과 경로의
  높이가 들쭉날쭉해져 `Has_LineOfSight`의 1m 높이 허용치가 string-pull 스무딩을
  거부했고, raw A* 계단이 그대로 이동 명령에 남았다. **이번에 고쳤다.**
- **우회 자체는 정상이다.** 스폰과 첫 트리거 사이에 x 47–52, z 280–291에 걸친 27셀
  벽이 실제로 있고 개구부는 북쪽 끝 z=292 하나뿐이다. 의심스러운 높이 894셀을 전부
  이웃 중앙값으로 내려도 경로는 24.390m / 2 waypoint로 **한 자리도 바뀌지 않는다.**
  구조물을 끼고 도는 것은 지형이지 버그가 아니다.

조사 중에 같은 함수 계통에서 **두 번째 독립 결함**을 찾아 함께 고쳤다.

- **스무딩이 아직 안 부서진 벽을 통과했다.** `Has_LineOfSight`가 쓰는
  `Sample_Position`은 `m_Walkable`만 보고 런타임 블로커(`m_BlockCounts`)를 보지 않는
  반면 `Find_Path`가 쓰는 `Is_CellWalkable`은 둘 다 본다. 그래서 A*는 서 있는 벽을
  피해 돌아가는데 string pull이 그 벽을 가로질러 당겼다. 기존 계약 테스트가
  "`Find_Path`가 거부한다"고 못박은 바로 그 쌍
  `(160.25,-130.75) -> (162.25,-135.75)`에서 `Has_LineOfSight`가 **`true`를 반환**했다.
  **이번에 고쳤다.**

`.md/GB/08-22` 세션이 navpaint를 손봤다가 "전혀 바뀐 게 없어" 판정으로 전부 되돌린
이유도 여기서 설명된다. navpaint는 WALKABLE/BLOCKED만 덮어쓸 수 있고 높이는
navsource만 소유한다(`Publish-ServerNavigation.ps1:225`). 높이가 원인인 문제를
navpaint로 고칠 수 있는 경로는 처음부터 없었다.

Client/UI는 에이전트가 실행하지 않았다. 실제 화면에서 캐릭터가 직선에 가깝게 걷는지는
사용자가 최종 판정해야 한다.

## 2. 실측으로 확정한 사실

측정 대상은 `Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`
(611,540 B = 헤더 20 + walkable 122,304 + heights 489,216)와 같은 폴더의
`.navblockers`이며, `ServerNavigation.cpp`의 `Resolve_Cell / Sample_Position /
Has_LineOfSight / Find_Path`와 `GameRoom.cpp:259 Smooth_MovePath`를 그대로 포팅해
재현했다.

| 항목 | 값 |
|---|---|
| 그리드 | 392 × 312, cell 0.5m, origin (-6, -165) |
| baked walkable | 21,381 셀 |
| player_1 스폰 | world (9.769, 9.771, -21.405), cell (31, 287) |
| 첫 movePlayer 트리거 `player_Move.1` | world (30.679, 10.040, -28.017), cell (73, 273) |
| 직선 XZ 거리 | 21.93 m |
| 스폰 연결 성분 | 2,407 셀 (블로커 on/off 동일) |

블로커는 무관하다. 런타임 블로커를 전부 끈 flood fill도 2,407셀로 **완전히 같다.**
`Stage_1`·`Stage_MiniBoss`·보스가 스폰에서 도달 불가인 것은 결함이 아니라 설계다.
`player.spawn.editor`와 `player_Move.1`이 각각 z=-35.94, z=-37.15로 순간이동시켜
그 간극을 건넌다.

진입로 높이장은 같은 XZ 구간에 두 층이 겹쳐 있다.

```text
navsource 원본 행 (cellX cellZ surface walkable height)
    46 280 1 1 10.6388578      <- 바닥
    47 280 1 0 14.7426834      <- 상판
    47 279 1 1 15.8114986      <- 상판인데 walkable=1
    48 279 1 1 15.4502401
    51 279 1 1 14.4840374
    52 280 1 1 11.2138348
```

`Publish-ServerNavigation.ps1`이 발탄을 publish할 때 스스로 찍는 값이 이를 뒷받침한다.

```text
LV_LUT_HEARTRB_ED         392x312 cellSize=0.5 walkable=21381 maxStep=19.8920774459839
LV_BER_BERNCASTLE          50x333 cellSize=0.5 walkable=7545  maxStep=17.0317230224609
LV_LOBBY_CLASSSELECT_SL00   62x62 cellSize=0.5 walkable=2176  maxStep=0.474029541015625
```

발탄의 인접 셀 최대 단차가 19.89m다. 발탄은 `-MaximumStepHeight`도
`-RequireSingleComponent`도 없이 publish되므로(같은 스크립트 558–561행) 5m 단차도
473개 성분 분할도 publish 검사를 그냥 통과한다.

## 3. 근본 원인 체인

```text
navgrid는 XZ 셀당 높이를 하나만 저장한다
  -> 바닥 통로 위 구조물 상판이 같은 셀 column을 차지하고 walkable=1로 구워진다
  -> Find_Path의 간선 비용이 (diagonal ? 1.414 : 1.0) 순수 2D라 수직 이동이 0원이다
  -> A*가 상판을 타 넘는 최단 2D 경로를 고른다 (경로 총 등반 8.85m, 최고점 Y=15.81)
  -> Smooth_MovePath가 부르는 Has_LineOfSight가 1m 높이 허용치로 그 구간을 거부한다
  -> string-pull이 한 칸씩만 전진해 raw 계단이 살아남는다
  -> waypoint 10개가 이동 명령에 남고, z가 -25.25에서 -24.75로 되돌아가는 구간이 생긴다
```

마지막 줄이 사용자가 말한 "아래로 갔다가 위로 갔다가"와 정확히 같은 지점이다.

## 4. 변경한 코드

### 4.1 `Server/Private/ServerNavigation.cpp` — `Find_Path` 간선 비용

간선 비용을 2D에서 3D로 바꿨다. 매직 상수는 없고, 수직 성분을 수평과 같은 단위로
세는 것이 전부다.

```cpp
			const float flatCost = diagonal ? 1.41421356f : 1.f;
			const float riseCost = std::abs(
				m_Heights[next] - m_Heights[current]) / m_fCellSize;
			const float candidate =
				costs[current] + std::hypot(flatCost, riseCost);
```

간선을 제거하지 않고 비용만 더하므로 **어떤 Area의 연결성도 줄지 않는다.** 기존 2D
heuristic은 3D 비용의 하한이므로 admissible·consistent가 유지되고 closed-set 조기
종료도 그대로 유효하다.

### 4.2 `Server/Private/ServerNavigation.cpp` — `Has_LineOfSight`가 블로커를 존중

`Sample_Position`은 그대로 뒀다. ground-target 스킬과 root motion clamp가 서 있는 벽
아래 바닥 높이를 계속 필요로 하기 때문이다. 대신 `Has_LineOfSight`가 양 끝과 모든
샘플에서 `Is_PointWalkableExact`(walkable + 블로커 둘 다 검사)를 함께 요구한다.

```cpp
	if (!Sample_Position(startX, startZ, startPoint) ||
		!Sample_Position(endX, endZ, endPoint) ||
		!Is_PointWalkableExact(startX, startZ) ||
		!Is_PointWalkableExact(endX, endZ))
	{
		return false;
	}
```

```cpp
		const float sampleX = startX + deltaX * ratio;
		const float sampleZ = startZ + deltaZ * ratio;
		SERVER_NAV_POINT samplePoint{};
		if (!Sample_Position(sampleX, sampleZ, samplePoint) ||
			!Is_PointWalkableExact(sampleX, sampleZ))
		{
			return false;
		}
```

`Has_LineOfSight`의 소비자는 `GameRoom.cpp:271`과 `:288`의 `Smooth_MovePath` 둘뿐이라
영향 범위가 이동 명령 스무딩으로 한정된다. 더 엄격해질 뿐이므로 최악의 경우 raw A*
경로를 그대로 쓰며 도달 가능성은 변하지 않는다.

효과 측정: 양 끝이 모두 설 수 있는 지점인 벽 횡단 쌍 44건에서

| | 스무딩 결과가 서 있는 벽을 관통 |
|---|---|
| 수정 전 | 42 / 44 (95%), 최악 51개 샘플이 벽 안 |
| 수정 후 | 2 / 44 (5%), 최악 1개 샘플짜리 모서리 스침 |
| raw A*만 (정합성) | 0 / 44 |

Bern과 Lobby는 런타임 블로커 셀이 0개라 부작용이 없다. 발탄 진입로 경로도
2 waypoint / 24.390m로 변하지 않는다. 아레나 내부 경로는 클릭 지점이 서 있는 벽
안이면 정확한 클릭 좌표 대신 가장 가까운 walkable 셀 중심에서 멈춘다.

### 4.3 `Server/Private/ServerGameplayContractTests.cpp` — 회귀 계약 추가

`Find authoritative navigation path` 바로 뒤에 두 항목을 넣었다. 좌표를 파일에 박지
않고 `CWorldBootstrap`에서 첫 enabled `PLAYER_SPAWN`과 가장 가까운 enabled
`MOVE_PLAYER` 트리거를 찾아 쓴다.

```text
Find the Valtan entrance spawn and its nearest move-player trigger
Walk the Valtan entrance on its floor instead of the walkway above it
```

두 번째 항목은 경로의 어떤 점도 양 끝 높이보다 2m 넘게 높지 않을 것을 요구한다.
상판을 타면 +5.567m, 바닥에 머물면 +0.169m라 2m가 두 결과를 안전하게 가른다.

블로커 결함에는 기존 벽 조건 블록의
`Keep the intact Valtan wall footprint and its cross-wall path dynamically blocked`
바로 뒤에 한 항목을 넣었다.

```text
Refuse line of sight across a Valtan wall that has not fallen
```

`Find_Path`가 거부한다고 이미 못박은 같은 쌍에 `Has_LineOfSight`도 `false`일 것을
요구하고, 아레나 열린 바닥의 짧은 구간이 여전히 `true`인지를 positive control로 함께
검사한다.

## 5. 자동 검증 결과

| 검증 | 결과 |
|---|---|
| `Server.vcxproj` Debug x64 빌드 | 성공, 컴파일 오류 0 |
| `Server.exe --contract-test` (두 수정 반영) | 708 PASS, **failures : 0** |
| A* 3D 비용만 되돌림 | **failures : 1** — 진입로 항목만 FAILURE |
| LOS 블로커 검사만 되돌림 | **failures : 1** — LOS 항목만 FAILURE, 진입로는 PASS |
| `git diff --check` | 이상 없음 |
| 인코딩 | `ServerNavigation.cpp` ASCII/CRLF 유지, 테스트 파일 BOM·비ASCII 6바이트·CRLF 유지 |

두 수정을 각각 따로 되돌렸을 때 대응하는 테스트 하나씩만 실패한다. 즉 두 항목이
서로 독립된 회귀 가드로 동작한다.

경로 영향은 4개 Area 전부를 재측정했다. 길이는 최대 +0.9%이고 waypoint와 헛등반은
전부 줄었으며 `NO PATH`였던 항목은 수정 전에도 `NO PATH`였다.

| 경로 | 수정 전 | 수정 후 |
|---|---|---|
| 발탄 스폰 → `player_Move.1` | wp 10, 등반 8.85m, 최고 Y 15.81 | **wp 2**, 등반 0.97m, 최고 Y 10.35 |
| 발탄 스폰 → `player.spawn.editor` | wp 11, 등반 8.82m | **wp 3**, 등반 1.19m |
| 발탄 보스 → 아레나 +12m | wp 1, 1.000× | wp 1, 1.000× (동일) |
| Bern 스폰 → `npc.bern.aylara` | wp 22, 등반 50.67m | **wp 13**, 등반 18.45m, 길이 +0.5% |
| Bern 스폰 → `valtan` | wp 22, 등반 50.67m | **wp 14**, 등반 18.45m, 길이 +0.9% |
| Lobby 스폰 → 보스 | wp 3, 1.504× | wp 3, 1.504× (동일) |

하드 단차 제한은 검토했다가 **측정 결과로 기각했다.** 1.0m 제한을 걸면 Bern 스폰의
도달 가능 배치가 3/4에서 **0/4**로 떨어지고 발탄 최대 연결 성분이 13,215셀에서
2,464셀로 쪼개져 아레나 보스 추적이 깨진다. 비용 방식은 간선을 지우지 않으므로 이
문제가 없다.

## 6. 사용자 확인이 필요한 것

에이전트는 빌드와 계약 테스트까지만 수행했다. 다음은 사용자가 직접 봐야 한다.

1. 발탄 진입 후 스폰에서 첫 트리거 방향으로 우클릭했을 때 캐릭터가 계단식으로
   꺾지 않고 두 구간으로 걸어가는지
2. 캐릭터가 진입로 철 구조물 위로 떠오르지 않는지
3. 아레나에서 아직 안 부서진 벽 건너편을 우클릭했을 때 벽에 처박혀 멈추지 않고
   벽을 끼고 돌아가는지
4. Bern에서 기존 이동감이 유지되는지 (경로가 평평해진 대신 최대 +0.9% 길어진다)

## 7. 남은 경계

- 진입로 우회 1.112×는 지형이다. 높이 894셀을 전부 보정해도 경로가 동일하므로
  더 줄이려면 x 47–52 / z 280–291 벽 자체를 MapTool에서 다시 저작해야 한다.
- 발탄 navgrid는 여전히 walkable 21,381셀 중 2,215셀(10.4%)이 이웃 중앙값과 1m 이상,
  574셀이 4m 이상 어긋나 있다. 상판이 바닥 셀에 얹히는 구조라 근본 해결은 MapTool
  재베이크이며 맵 담당 영역이다. 이번 변경은 그 데이터를 건드리지 않고 A*가 그런
  데이터 위에서 올바르게 동작하게 만든 것이다.
- 발탄이 `-RequireSingleComponent` 없이 publish되는 것은 movePlayer 설계상 정상이다.
  `-MaximumStepHeight`를 켜는 것은 지금 데이터로는 publish를 깨뜨리므로 재베이크 뒤에
  별도로 판단한다.
