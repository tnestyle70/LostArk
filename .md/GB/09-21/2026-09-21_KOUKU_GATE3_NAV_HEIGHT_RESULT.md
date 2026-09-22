# 2026-09-21 쿠크세이돈 3관문 시작 지점 navigation 조사 결과

## G00. 결론

Gate3Fine의 상부 navigation은 잘못된 overhead surface 전체가 아니라 실제 3관문
입장 전 대기 테라스다. 따라서 상부 셀을 대량 `BLOCKED` 처리하거나 낮은 전투장으로
평탄화하는 데이터 변경은 적용하지 않았다. 조사 중 생성한 실험 paint와 runtime은
모두 HEAD 원본으로 복원했고, 추가 publisher는 실행하지 않았다.

## G01. 실측과 authored 근거

- `Gate3Fine.navsource`: 98×96, cell 0.5m, origin (-32, 926).
- walkable high 셀(Y≥25)은 1,162개다. authored 테라스 범위
  x[-30,-5], z[947,972] 안에 1,158개가 있고, 주 높이는 Y=25.6/25.669다.
  바깥의 4개 edge 셀은 테라스 경계에 붙은 끝면이다.
- Y>20 walkable 1,185개 전체에는 경사/장식 셀 27개가 섞이지만, 기존 paint가
  Gate3Fine 의자 좌면 26칸과 바닥 밖 장식 1칸을 이미 `BLOCKED`로 보정한다.
- `Shared/Public/Gameplay/KoukuArenaReadyAreas.h`의
  `Is_KoukuGate3EntryTerrace`는 x[-30,-5], y[23.5,28], z[947,972]를 같은
  authored 공간으로 선언한다. 기존 fine-nav RESULT도 이 테라스와 중앙 전투장의
  바닥 footprint가 겹치지 않으며 일반 보행 연결을 만들지 않는다고 명시한다.
- 월드 player spawn/boss/Mario 위치는 Y≈1.3176의 낮은 전투 바닥이다. 입장 전
  테라스에서 전투장으로 가는 것은 일반 A* 경로가 아니라 Server의 관문 진입/명시적
  이동 계약이 담당한다.

## G02. Server 소비 측정

일반 우클릭 명령은 `GameRoom_PlayerCommands.cpp::Commit_MoveGoal`에서
`CServerNavigation::Find_Path`를 호출하고, `GameRoom_PlayerSimulation.cpp`가
`Resolve_TraversalStep`으로 매 tick 목적지 바닥을 확정한다. `ServerNavigation.cpp`
의 `Select_Region`은 Gate3Fine XZ 범위의 시작점을 detail grid로 보내며 base grid로
fallback하지 않는다. 이는 authored 두 공간을 같은 XZ AABB에 담은 현재 detail-region
계약과 일치한다.

custom A* 측정 결과, 테라스 내부 7개 목적지(반경·경계 샘플)는 모두 경로가 있고,
테라스에서 낮은 전투 바닥의 7개 목적지는 모두 경로가 없다. 이는 상부 바닥의
내부 이동 불가가 아니라 두 stage를 일반 보행으로 연결하지 않은 결과다.

## G03. 파일·검증 상태

- 변경된 navigation authoring/runtime 파일: 없음. `Gate3Fine.navpaint`, Server/Client
  `Gate3Fine.navgrid`, Server `Gate3Fine.navsurface`는 HEAD hash로 복원했다.
- 대량 BLOCKED 후보의 Validate/Publish와 ContractTest는 조사 과정에서 한 번 실행했지만
  결과 산출물은 폐기했고, 최종 디스크에는 게시하지 않았다.
- `git diff --check`는 확인했다. Client/UI 실행과 화면 확인은 하지 않았다.

## G04. 남은 경계

사용자가 말한 “시작 지점”이 대기 테라스에서 낮은 전투장으로의 일반 우클릭 이동을
뜻한다면, navigation source를 지우는 것이 아니라 Server typed gate-entry/teleport
트리거의 실제 호출·도착 증거를 추가 조사해야 한다. 테라스의 authored floor를
차단하면 입장 대기 자체가 망가진다.
