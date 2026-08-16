# 2026-08-14 루트모션 네비게이션 클램프 RESULT

작성자: JS · 2026-08-14 · branch `feature/root-motion-navigation-clamp`

PLAN 없음. 원인이 한 지점으로 좁혀져 조사 후 바로 구현했다.

## 1. 문제

스킬 애니메이션의 루트모션이 navigation의 nonwalkable을 무시하고 클립 델타만큼
그대로 이동했다.

`Server/Private/PlayerSkillSystem.cpp`의 루트모션 적용부가 `Project_Point`를 썼다.
`Project_Point`는 내부 `Resolve_Cell`이 **반경 12셀 안에서 가장 가까운 walkable 셀**을
찾아 true를 돌려준다. 목적지가 nonwalkable이어도 근처에 walkable이 있으면 true다.
그런데 코드는 투영 결과를 버리고 원본 좌표를 대입했다.

```cpp
if (navigation->Project_Point(nextX, nextZ, projected))
{
    player.fPositionX = nextX;        // 원본 그대로
    player.fPositionY = projected.y;  // 높이만 투영값
    player.fPositionZ = nextZ;        // 원본 그대로
}
```

대조군인 일반 이동은 `Sample_Position`(스냅 없이 해당 셀이 nonwalkable이면 false)을
쓰고 `Resolve_PlayerMove` 스윕까지 거친다. 루트모션은 둘 다 건너뛰고 있었다.

## 2. 구현 완료

루트모션 변위를 매 tick 두 관문에 통과시킨다.

- `CPlayerSkillSystem::Clamp_StepToWalkable` 신설. `Sample_Position`으로 step 구간을
  셀 절반씩 훑어 **첫 번째** 막힌 지점을 찾고, 그 bracket만 12회 이분 탐색해 경계에
  밀착시킨다. 훑기 없이 이분 탐색만 하면 긴 대시가 막힌 띠를 넘어 반대편 walkable에
  착지할 때 그것을 도달 가능으로 오판한다.
- 같은 자리에서 `CServerCollisionSystem::Resolve_PlayerMove`를 태운다. 루트모션은
  여태 정적 collisionBox 스윕도 거치지 않았다.
- `CPlayerSkillSystem::Update`에 `const CServerCollisionSystem*` 인자 추가.
  `CGameRoom`이 `&m_ServerCollisionSystem`을 넘긴다.
- `CServerNavigation::Get_CellSize()` 접근자 추가. 훑기 간격 산출에만 쓴다.

액션은 취소하지 않고 tick 단위로만 자른다. 클립이 다음 tick에 옆으로 꺾이면 자연히
다시 움직인다.

시작 위치 자체가 nonwalkable이면 그 tick 이동을 거부한다. 일반 이동의
`Sample_Position` 게이트와 같은 정책이다.

변경 파일 5개. 신규 파일 없음. Shared 프로토콜·Data·Client·Engine 미변경
(UpdateLib 불필요, 프로토콜 버전 그대로 18).

| 파일 | 내용 |
|---|---|
| `Server/Public/PlayerSkillSystem.h` | `Update` 인자 추가, `Clamp_StepToWalkable` 선언 |
| `Server/Private/PlayerSkillSystem.cpp` | 헬퍼 구현, 적용부 교체 |
| `Server/Public/ServerNavigation.h` | `Get_CellSize()` |
| `Server/Private/GameRoom.cpp` | collision 인자 전달 |
| `Server/Private/ServerGameplayContractTests.cpp` | 신규 검증 5건, 기존 호출부 9곳 갱신 |

## 3. 자동 검증 (실행함)

- Server x64 Debug 빌드 exit 0.
- `Server.exe --contract-test` failures 0. 신규 5건:

```
[PASS] Find a Valtan navigation boundary to clamp root motion against
[PASS] Stop root motion against the Valtan non-walkable boundary
[PASS] Preserve root motion that stays on walkable navigation
[PASS] Find walkable navigation beyond the blocked band
[PASS] Refuse root motion that would cross a blocked band in one step
```

- `Run-ValtanFourPlayerHarness.ps1` failures 0.
- `Run-CharacterSelectIsolationHarness.ps1` failures 0.
- `git diff --check` 통과. 편집 파일 전부 CRLF 유지, BOM 없음.
- `Invoke-ProjectAudit.ps1` 123건 중 29건 실패로 변경 전 기준선과 동일하다. 실패
  항목 목록도 같다(effect 24건 + `projects.data-source-visibility`,
  `levels.character-select-contract`, `rendering.quality-workbench-contract`,
  `rendering.point-light-falloff-contract`, `world.authoring-format-v4`). 이번
  변경으로 새로 깨진 검사는 없다. 이 29건은 main 머지 이전부터 실패하던 항목이다.

테스트는 좌표를 하드코딩하지 않는다. 실행 시점에 로드된 발탄 그리드에서 walkable
지점부터 전진해 실제 경계를 찾아 쓴다. 재bake돼도 깨지지 않고, 경계나 막힌 띠를 찾지
못하면 조용히 통과하지 않고 실패한다.

## 4. 수동 검증 (사용자 확인 완료, 2026-08-14)

로컬 루프백(Server `0.0.0.0:7777`, Client `LOSTARK_SERVER_HOST=127.0.0.1`)에서
사용자가 직접 확인. 벽에서 정지 동작이 의도대로 작동.

## 5. 구현 중 잡은 것

- 첫 구현에 "목적지가 walkable이면 그대로 수락"하는 조기 반환이 있었다. 벽 너머
  착지점이 walkable이면 훑기를 건너뛰어, 고치려던 터널링을 그대로 갖고 있었다.
  터널링 계약 테스트가 잡아냈다.
- 테스트 탐색 루프가 `lastWalkableX`를 루프 안에서 갱신하면서 종료 조건에도 써서
  상한이 계속 밀렸다. 시작점을 별도 상수로 분리했다.

## 6. 남은 경계

- Release 구성 미빌드. Debug만 검증했다.
- 훑기 상한은 256 샘플이다. 한 tick step이 128셀을 넘으면 그 이후 구간을 훑지 않는다.
  30Hz에서 현실적인 대시 속도로는 도달하지 않지만, 순간이동성 스킬을 추가하면
  이 상한을 먼저 확인해야 한다.
- 몬스터·보스는 `PLAYER_ROOT_MOTION_SAMPLE`을 소비하지 않는다. 이번 범위 밖이며,
  루트모션 기반 보스 패턴이 생기면 같은 클램프를 태워야 한다.
- 벽을 따라 미끄러지는(slide) 처리는 넣지 않았다. 정지로 합의했다.
