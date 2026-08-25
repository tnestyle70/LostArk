# Bern Runtime Height Guard RESULT

작성일: 2026-08-25

상태: 코드 구현, Server 빌드와 focused 실행형 검증 완료. 사용자 런타임 육안 확인 대기.

## 직접 원인

기존 Bern 1m 정책은 A*의 인접 셀과 이동 직전의 `player Y -> proposed cell Y`를 검사했다.
하지만 `Smooth_MovePath`가 여러 셀을 먼 waypoint 하나로 줄인 뒤 normal walk가 waypoint Y까지
선형 보간했다. 현재 XZ가 아직 낮은 셀 안에 있어도 player Y가 먼저 조금씩 올라갔고, 높은 셀
경계에 도착할 때는 player Y도 이미 높아져 기존 실시간 검사를 통과할 수 있었다.

따라서 재베이크 여부와 무관하게 캐릭터와 follow camera가 다른 층으로 올라갈 수 있는 코드
우회가 남아 있었다.

## 적용 내용

- `CServerNavigation::Resolve_TraversalStep`
  - 현재 XZ와 다음 XZ의 실제 navigation ground를 함께 샘플링한다.
  - Area별 최대 높이 차이를 넘는 이동 단계를 거부한다.
  - 성공 시 waypoint 보간값이 아닌 destination cell의 ground Y를 반환한다.
- `CGameRoom::Update_Players`
  - normal walk의 authoritative Y를 매 tick destination ground Y로 고정한다.
  - 실패하면 좌표를 commit하지 않고 move goal/path를 폐기한다.
  - body collision slide가 바꾼 최종 XZ도 commit 전에 다시 검사하고 ground Y로 고정한다.
- `CPlayerSkillSystem::Clamp_StepToWalkable`
  - root motion과 knockback의 sub-cell march 및 경계 이분 탐색에도 같은 높이 정책을 적용한다.
- focused navigation contract
  - ground Y 고정
  - 실제 Bern runtime grid의 17.03m 인접 deck jump 거부
  - skill/knockback upper-deck 진입 전 clamp를 재현한다.

## 자동 검증

```text
Publish-ServerNavigation.ps1 -Mode ContractTest: PASS
Publish-ServerNavigation.ps1 -Mode Validate: PASS (4 Area)
Server x64 Debug build: PASS, warnings 0, errors 0
Server.exe --navigation-contract-test: PASS, navigation failures 0
  ground lock
  17m live walk rejection
  skill/knockback clamp
  stair route
  repaired corridor ridges
  invalid policy rollback
git diff --check (관련 파일): PASS
```

## 수동 확인

새 `Server/Bin/Debug/Server.exe`를 반드시 다시 시작한 뒤 Lobby -> Character Select -> Bern으로
들어가 이전 경로를 반복 이동한다. 큰 높이 차이 셀을 향한 이동은 마지막 정상 위치에서 멈춰야
하며, 캐릭터와 카메라가 지붕·다리·건물 내부 높이로 올라가면 안 된다. 최종 visual PASS는
사용자 관찰 전까지 기록하지 않는다.
