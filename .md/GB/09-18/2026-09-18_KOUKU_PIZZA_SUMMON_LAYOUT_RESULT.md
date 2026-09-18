# 쿠크 피자 Summon의 테이블 좌우 배치 결과

## G01. 소스 구현 완료

기존 `Summon occurrence.patternSpawns → SUMMON_PATTERNS → dependent boss clone` 경로를 확장했다.
World Object 또는 Parent timeline으로 패턴을 복제하지 않는다.

- occurrence당 최대 16개. 동일 패턴을 참조하는 좌우 5명씩 10개 배치를 지원한다.
- `anchorKind` 생략/`BOSS`는 기존 본체 상대 위치·yaw, `MAP`은 맵 고정 XYZ·yaw다.
  BOSS offset은 ±1000m, MAP 좌표는 ±100000m이며 finite·unique ID 검사는 유지한다.
- 같은 Gate·actor·boss target의 leaf MECHANIC에서 Animation, BOSS 기준 Effect/Sound,
  ALBION_AIRBORNE JUMP/SLAM을 허용한다. 재귀 Summon, 전역 World/Scene/Camera,
  플레이어 선택, outcome 분기와 본체 reset은 계속 거부한다.
- Preview는 clone별 actor/session/root-motion/airborne sampler를 준비한다.
  Server도 각 clone에서 기존 JUMP/SLAM을 실행한다. 0ms trigger는 spawn 표시 전에 반영한다.
- Server는 10개를 모두 stage한 뒤 commit한다. MAP 좌표는 exact walkable과 navigation Y 오차
  1m 이내를 요구한다. 실패하면 부분 생성하지 않는다. 부모 run 종료·교체·사망과 lifetime 정리는 유지한다.
- 새 Shared packet, C++ 파일, project/filter 등록은 없다. 기존 CNpc/CModel과 spawn/snapshot을 사용한다.

## G02. 저작 UI

`쿠크_피자 → 쿠크_피자_훌라후프쿠크소환 Summon 박스 → Box Detail → Spawned Patterns`에서
빈 목록의 `Create two rows: 5 left + 5 right`를 연다. Formation Pattern에 `쿠크_훌라후프_레드`를 선택한다.

실제 2관문 FLOOR15 테이블과 현재 게시 navigation을 기준으로 측정한 시작값은 다음과 같다.

| 입력 | 값 |
|---|---|
| Formation center XYZ (m) | 6.43052775, 10.5600004, 321.25947198 |
| Center to each side (m) | 4 |
| Spacing along each row (m) | 2 |
| Layout yaw (degrees) | -45 |

`Create 10 spawned Patterns`를 누른 뒤 각 행의 MAP World position/yaw를 개별 조정한다.
`Use Player Position`도 사용할 수 있다. 모든 배우는 기본적으로 반대쪽 줄을 바라본다.
Save 후 Play Preview, Publish 후 Server Play로 확인한다. 새 C++/Server 코드이므로 사용자가
전체 빌드한 새 Client와 Server를 실행해야 한다.

테이블 자체 yaw는 -135도지만 UI의 행은 local Z 방향이므로 Layout yaw는 **-45도**다.
좌우는 테이블 pivot에서 안쪽 local +X를 보는 기준이다. 원하는 카메라 기준 좌우 또는 세부 위치는
사용자가 Preview에서 조정한다. 표의 값은 수치상 바닥 안에 놓이는 후보이며 최종 화면 승인값은 아니다.

현재 저장 데이터에서 P84 길이는 17333ms이고 기존 P25 Summon은 1461ms에 시작해 16586ms 유지한다.
새 배치 UI는 child가 잘리지 않게 Summon을 최소17333ms, 부모를 최소18794ms로 자동 확장한다.
기존 Animation/Effect 행의 시작·길이는 변경하지 않는다.

## G03. 실행한 검증

| 검증 | 결과 |
|---|---|
| Client 변경 3 translation unit 격리 Debug 컴파일 | PASS |
| Server 변경 3 translation unit 최종 격리 Debug 컴파일(MAP height guard 포함) | PASS |
| 실제 Composition codec probe | 45 checks / 0 failures |
| 새 Summon 배치 Python 검사 3개 | PASS: 10/16개, 초과17개 거부, 좌표/identity/window/child scope |
| 기존 Cross Summon 5개 + airborne projection 1개 | PASS |
| 현재 P25/P84 closure에 10개 MAP 행을 메모리에서 연결 후 validate_document/validate_publishable/projected_outputs | PASS, 실제 정본 미변경 |
| 10개 배치 중심의 실제 테이블 삼각형·published nav | 10/10 walkable, 지면10.56~10.595m |
| 반경1m 몸체가 교차하는 nav cell·바닥 둘레128표본 | 전부 같은 테이블 높이·walkable |
| PowerShell publisher parse | 오류0 |
| 변경 파일 git diff --check | PASS |

격리 TU 컴파일은 Product 링크·배포 성공의 대체 증거가 아니다. 사용자 요청에 따라 전체 Product Build는
사용자가 수행한다. Client/UI 자율 실행과 화면 캡처는 수행하지 않았고 Preview/Server 화면은 미검증이다.

## G04. 반영 범위와 인계

**C++/publisher/UI 코드는 반영했다. 편집 중인 Composition 정본과 runtime 게시물은 교체하지 않았다.**
10개 배치는 위 UI에서 생성·Save해야 한다. 새 코드가 생겼다는 이유로 실행 중 Client의 draft가 갱신되거나
기존 이름뿐인 summon.8이 자동으로 10명을 생성하지 않는다. 기존 사용자 편집을 보존한다.

다른 기능의 미커밋 변경이 있는 작업 폴더이므로 자동 stage/commit하지 않았다.
공용 계약은 `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 Summon 항목을 갱신했다.

검증 산출물은 `out/KoukuPizzaSummon20260918/`에 있다.

- `client-compile.log`, `codec-run.log`
- `server/compile.log`
- `layout.json`, `measure_layout.py` — 좌표 후보와 실제 mesh/nav 근거. 정본으로 직접 복사하는 파일은 아니다.
