# Level 네비게이션 표시와 1관문 세이튼 생성 위치

## G00. 현재 소비자와 수정 범위

사용자는 세이튼이 1관문 중앙 대신 시퀀스 시작 위치에 생성되고 가장자리 이동 가능 범위도
확인하기 어렵다고 보고했다. F1의 기존 `Level Navigation`은 레벨 이동 명령이다. 이를
변경하지 않고 `Arena Camera / Player` 바로 아래에 별도 `Show Navigation` 표시 항목을 둔다.
새 전역 기능키는 추가하지 않고 F1 안의 토글로 켜고 끈다. Client/UI 실행과 화면 판정은
사용자가 하며 에이전트는 코드·수치·컴파일과 필요한 데이터 게시를 담당한다.

## G01. CLevelNavigationDebug와 MainApp 연결

`Client/Public/LevelNavigationDebug.h`, `Client/Private/LevelNavigationDebug.cpp`는 현재
Level의 게시된 네비게이션을 읽어 표시하는 Debug 전용 뷰다. `CLevelRegistry`의 실제 Area와
`CMapNavigationContract`로 경로를 결정하며 `Engine::CNavGrid`의 기존 parser를 소비한다.
카메라·플레이어의 이동 판정이나 navgrid 저장·베이크는 소유하지 않는다.

`Sync_Level`, `Render_Controls`, `Render_Overlay`를 MainApp의 Debug render와 F1 화면에
연결한다. 표시 기본값은 꺼짐이고 Level 변경 시 이전 Area의 데이터와 표시 상태를 해제한다.
F1 창을 닫아도 켜 둔 오버레이는 유지한다. 같은 Level에서 명시 Reload가 실패하면 기존
표시 데이터는 보존하고 실패 사유를 표시한다. 일반 제품 빌드에는 Debug 표시가 포함되지 않는다.
Development의 Map Editor 모드에서는 고정 수련장 Area를 표시하지 않고 기존 MapTool의
Navigation 표시를 사용한다. 제품 Level의 Area와 편집기에서 선택한 Area를 혼용하지 않는다.

이동 가능 셀과 막힌 셀은 실제 게시 grid를 기준으로 그린다. 베이크 미검출 분류는 기존
`CNavGridPaintDocument`의 원본/paint가 게시 grid와 대응할 때만 사용한다. 현재 disk 파일과
실행 중 Server 메모리 상태를 동일하다고 설명하지 않는다. 미게시 authoring, detail region,
grid 바깥 영역을 실제 runtime 이동 가능 영역으로 오인하지 않게 범위와 상태를 표시한다.
오버레이는 카메라 주변과 화면 영역으로 작업량을 제한하며 범위·표시 수를 UI에서 확인할 수 있다.

MainApp 기존 파일의 인코딩과 다른 세션의 변경을 보존한다. 두 새 파일은 UTF-8 BOM 없이
추가하고 `Client.vcxproj`와 `.filters`의 기존 Map 도구 분류에 정확히 한 번 등록한다.

## G02. 중앙 spawn과 시퀀스 시작점의 분리

Server의 생성 경로는 typed placement ID → `Find_Placement` → `Build_WorldEntity` →
broadcast다. Client가 보스의 생성 좌표를 덮어쓰지 않는다. 현재 G1 placement
`boss.kakulsaydon.g1.saydon`은 `[6.43, 1.3, 730]`이며 이전 연출 시작점 변경의 결과다.
기존 G1 중앙 `[-0.07, 1.32, 737.53]`과 P8/P36의 BossMotion 시작·종료 위치를 대조한 뒤
world placement의 position만 중앙으로 고친다. 시퀀스의 이동 시작점은 BossMotion에 남긴다.

사용자가 이미 승인한 현재 디스크 기준으로 최신 stable ID와 해당 field를 다시 읽고 백업·
hash 확인·원자 교체한다. 무관한 world row, Composition과 사용자의 시퀀스는 보존한다.
변경한 world domain의 publisher를 실행하고 생성 bootstrap에서 Server 소비 좌표를 확인한다.
실행 중인 Server가 자동 갱신됐다고 기록하지 않는다.

## G03. 종료 증거

새 표시 뷰와 MainApp을 현재 헤더로 최소 컴파일한다. 실제 navgrid와 source/paint의
descriptor·셀 상태·원본 불일치 처리를 확인하고 같은 Level Reload 실패 시 이전 데이터
보존, Level 전환 시 초기화와 표시 범위 계산을 검사한다. 창·오류 대화상자를 열지 않는
검사만 사용한다. 변경 JSON/XML parse와 `git diff --check`, 게시 결과와 실제 프로세스
상태를 기록한다. 육안으로 확인한 것처럼 결과를 기록하지 않는다.
