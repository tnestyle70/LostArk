# 카드미로 바닥 navigation 결과

## G00. 반영

카드미로 입장/망원경의 Y=-0.01m와 실제 표시 바닥 placement
`10296705976280178153`의800개 삼각형을 대조했다. 기존 native CNavGridBaker로
동일116×114/0.5m 격자를 다시 굽고 bounds Y를 스폰 중심0.1m로 저장했다.
기존 source의 카드 윗면1.84m·장식2.67m·아래 배경-15.522m를 제거했다.

7,569개 실제 바닥 셀 중 기존 벽 차단2,447개를 유지했다. 실제 바닥 밖의
중복 BLOCKED253행만 없앴고 해당 셀은 계속 NO_SURFACE로 이동 불가다.
이동 가능5,122개 셀은 전부 스폰 연결 성분에 속한다.

최신 원본 hash 확인·백업·원자 교체 뒤 공식 Navigation publisher로 Client/Server를
게시했다. 변경된 runtime은 CardMiro navgrid와 Server navsurface뿐이다.
Client/Server navgrid bytes는 같다. 다른 region과 navregions는 바뀌지 않았다.

## G01. 실행 검증

- source geometry 측정:185개 배치/12개 mesh 조사, decode 오류0.
- native baker 재컴파일·실행 성공. source 높이 범위[-0.0100000007,-0.00999999885]m.
- Navigation Publish 성공.5,122셀 연결, maximum step 약1.86e-9m.
- 기존 native ServerNavigation consumer로 스폰+네 방향5개 바닥 표본 및4개 경로 PASS.
  경로 waypoint의 실제 traversal step도 모두 허용된다.
- 동일 조건을 기존 `ServerGameplayContractTests_CardMaze.cpp`에 추가했다.
  새 제품 Debug/Release EXE의 `--card-maze-contract-test`가 모두 exit 0, failures 0이다.

증거는 `out/RaidRelease20260924/nav/`의 baseline, geometry-summary,
paint-pruning, grid-validation, runtime-consumer 및 publish.log다.
Client 입력은 기존 depth hit 또는 현재 플레이어 Y의 ray/plane 교차를 typed 이동 명령으로
제출하며 Server가 이 navigation으로 판정한다. 실제 Debug/Release 화면 클릭은 사용자 확인 대상이다.
