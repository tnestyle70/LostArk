# Release 노란 FPS 표시 구현 계획

## G00. 현재 경계와 목표

사용자가 Release 프레임 드랍을 확인할 수 있도록 기존 좌측 상단 노란 `FPS N`을 표시한다.
기준 HEAD는 `b5589fe6809cba47dc2a87b7cb28f6ba63e2e769`이며 시작 작업 트리는 clean이다.
`CMainApp::RenderFpsText`의 본문 전체가 `_DEBUG`로 제한되어 Release에서는 비어 있다.
호출, FPS 평활화와 폰트는 이미 두 구성에서 사용하는 경로다.

## G01. MainApp.cpp의 표시 조건

`Client/Private/MainApp.cpp`의 `RenderFpsText`에서 `_DEBUG`를 사용자 표시 모드 조건에만
적용한다. Release는 기존 폰트·색·위치·평활화로 항상 표시하고 Debug의 표시 모드는 유지한다.
기존 호출자의 cinematic suppression은 두 구성에서 그대로 적용한다.

현재 `combobox_fps`는 UI에 네트워크 지연 표시로 나타나며 저장값과 기본값은 1이다.
이 값으로 Release 표시를 제한하면 비전투 중 숨겨지므로 Release는 해당 조건을 사용하지 않는다.
개인 UserSettings와 렌더링 품질·프레임 제한·F7/Profiler·저작 도구는 변경하지 않는다.
새 H/CPP, 멤버, 인터페이스, 프로젝트/filter 등록과 JSON 변경은 없다.

## G02. 계약과 검증

`AGENTS.md`, `CLAUDE.md`, 팀 인터페이스 사용서의 Release FPS 비활성 설명을 현재 계약으로
고친다. 기존 날짜별 결과는 당시 기록으로 보존한다.
정본 Product runner의 Release 증분 Build로 컴파일·링크와 배포를 확인하고
`git diff --check`, 기존 UTF-8 BOM 없음/CRLF 보존을 확인한다.
Client/UI는 실행하지 않는다. 실제 노란 표시와 프레임 드랍의 원인·수치는 사용자 확인 영역이며
이번 변경을 성능 개선으로 기록하지 않는다.
