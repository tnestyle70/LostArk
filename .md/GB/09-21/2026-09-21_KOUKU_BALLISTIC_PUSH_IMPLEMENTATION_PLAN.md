# 쿠크 접촉 재타격과 대형 세이튼 포물선 넉백 구현 계획

## G00. 범위와 현재 실측

레이저 ENTER_AREA는 이미 접촉 창별 반복 정책을 갖지만 현재 repeatAfterKnockback의
242ms 대기보다 일곱 창의 222~231ms 수명이 짧다. 기존 rearmOnExit를 이용해 실제 재진입을
다시 판정하도록 데이터 담당이 연결한다. 공용 collider 런타임을 바꾸지 않는다.
대형 세이튼 P86/P87의 공용 logic97 결과는 실제 접촉 장판의 중심을 전달받아야 한다.

## G01. 결과부터 서버 이동까지

Result에 기본 false인 pushBallistic와 AWAY_FROM_CONTACT 방향을 추가한다.
서버 bootstrap PATTERNLOGICPUSH는 기존 8/9/11/12개 필드를 유지하고 열 13에 bool을
추가한다. 포물선은 MAX_HP_PERCENT_DAMAGE, 양의 거리, 100~5000ms, 외곽 이탈 허용을
요구한다. 거리는 포물선만 최대 100m, 기존 지면 밀림은 20m 제한을 유지한다.
접촉 방향은 실제 region을 가진 ENTER_AREA/AREA_OVERLAP의 성공·실패 결과에 한정한다.

LogicRuntime은 판정한 region의 현재 세계 중심을 기존 결과 함수에 전달하고, 기존
WorldToPlayerHit와 PlayerSkillSystem이 방향·속도·수직 초기속도를 저장한다.
서버 Room의 기존 Advance_PlayerKnockback이 XZ와 Y를 함께 적분한다.
중력은 기존 낙하와 같은 9.8m/s²이며 초기 수직속도는 중력×기간÷2이다.
비행 중 보행 네비와 충돌 밀어내기를 건너뛰고 하강 중 실제 바닥에 닿으면 착지한다.
비행 끝에 바닥이 없으면 기존 FALLING과 사망 경로로 수직속도를 이어준다.
기존 KNOCKDOWN 상태와 snapshot XYZ를 사용하므로 Shared packet은 추가하지 않는다.

## G02. 바닥과 보행 가능 여부

현재 navsource v2에는 surface와 walkable이 별도지만 runtime navgrid는 surface를
버린다. 바닥이 있는 BLOCKED 셀과 바닥이 없는 셀을 height 0 같은 추측으로 구분하지
않는다. 기존 navgrid 바이트를 보존하고 version·grid hash·surface mask를 가진
navsurface sidecar를 같은 publisher transaction으로 출력한다. ServerNavigation은
이를 검증하고 비행 착지 전용 Sample_SurfacePosition에서 소비한다.
기존 runtime obstacle은 바닥을 지우지 않고 명시 void condition만 바닥을 지운다.

## G03. 검증과 반영 경계

변경 전 파일별 백업과 인코딩을 out/KoukuBallistic20260921에 기록했다.
접촉 중심 방향, 기존 보스 방향, force 재접촉, 지면 밀림, 포물선 정점·거리·착지·낙사,
막힌 바닥과 바닥 부재, runtime void, malformed surface sidecar를 native 코드로 검증한다.
부모 작업이 데이터 병합·domain publish·최종 빌드를 소유한다. 제품 Client/Server 실행과
UI 조작은 하지 않는다. 신규 제품 C++ 파일과 프로젝트 등록은 없다.
