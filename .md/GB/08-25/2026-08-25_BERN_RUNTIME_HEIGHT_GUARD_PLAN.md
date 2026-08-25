# Bern Runtime Height Guard PLAN

작성일: 2026-08-25

## 목표

베른 내비게이션에 잘못된 높은 셀이 남아 있어도 캐릭터의 Server 권위 Y와 이를 따라가는
카메라가 다른 층으로 이동하지 못하게 한다. 이번 변경은 재베이크 결과에 의존하지 않는다.

## 확인된 우회

`Find_Path`는 Bern의 1m 인접 높이 정책을 검사하지만, `Smooth_MovePath`가 여러 셀을 하나의
먼 waypoint로 줄인 뒤 `CGameRoom::Update_Players`가 현재 셀의 높이가 아니라 waypoint Y까지
선형 보간한다. 기존 실시간 검사는 현재 XZ의 navigation 높이만 확인하고 실제로 적용할
`proposedY`를 확인하거나 navigation 높이로 교체하지 않아, 같은 낮은 셀 안에서도 Y가 먼저
상승할 수 있다. 그 결과 높은 셀 경계에 도착했을 때는 이미 player Y가 높아져 기존 검사를
통과할 수 있다.

## 수정 계약

1. `CServerNavigation::Resolve_TraversalStep`이 현재 XZ 셀과 다음 XZ 셀의 실제 navigation
   높이를 함께 샘플링한다.
2. 두 셀의 높이 차이가 Area policy를 넘으면 이동 단계를 거부한다.
3. 허용된 단계는 waypoint 보간 Y가 아니라 다음 XZ 셀의 실제 ground Y를 반환한다.
4. `CGameRoom::Update_Players`는 normal walk의 `proposedY`를 이 반환값으로만 정한다.
5. body collision slide의 최종 XZ도 commit 전에 같은 방식으로 다시 검사한다.
6. 스킬 root motion과 넉백이 사용하는 `Clamp_StepToWalkable`도 모든 sub-cell 단계에서
   같은 높이 정책을 적용한다.
7. 실패하면 마지막 정상 좌표를 유지하고 목표와 path를 폐기한다.

## 검증

- Bern runtime grid의 실제 17.03m 인접 셀 이동 단계 거부
- 정상 지상 셀 이동은 destination ground Y로 고정
- `Server.exe --navigation-contract-test`
- Server x64 Debug build
- `git diff --check`
- 최종 Lobby -> Character Select -> Bern 육안 확인은 사용자가 수행
