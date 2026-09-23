# Release F1 레이드 테스트 구현 계획

## G00. 현재 경계와 목표

PR454를 d9361f98f로 통합했다. Retail Gameplay 게시가 통과했으며 기존 F1 도구는 MainApp의 `_DEBUG` 조건 때문에 Release에서 접근할 수 없다. UI를 여는 기능과 서버 판정 경계를 구분해, 같은 typed 명령과 기존 도구를 두 빌드에서 소비한다.

## G01. MainApp 공용 진입점

`Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`의 F1 상태, Balance Test, Profiler, 두 레이드 Boss Tool과 Complete Play 진입점을 공통으로 연결한다. 기존 저작 도구와 Map/Sequence 편집은 Debug 경계를 유지한다. Release는 창을 닫은 상태로 시작하고 패턴 그래프를 명시적인 Load 버튼에서만 읽는다. F6 카메라와 F7 Profiler 단축키를 유지한다. FPS 표시도 유지한다.

F1 → 기존 Boss Tool → dependency preparation → typed command sink → Server admission → snapshot/presentation의 기존 경계를 사용한다. 임의 Client boss HP, 위치 또는 clear 상태를 만들지 않는다. Profiler Capture는 창을 열 때 자동 시작하지 않는다.

## G02. Arena 준비와 상태 소비

Kouku와 Valtan의 기존 gate 활성화 및 Complete Play 준비 상태, 취소와 응답 소비를 Release에서도 활성화한다. 월드나 세션이 바뀌면 기존 요청 수명 검증으로 정리한다. Map authoring과 local preview는 이 변경의 공통 실행 경로가 아니다.

## G03. 서버 수치와 HUD

별도 Retail/cooldown 구현 계획의 typed room 정책과 snapshot duration을 HUD 쿨타임 비율의 분모로 사용한다. Kill Boss는 현재 관문 보스의 HP를 0으로 만들고 기존 사망 처리와 레이드 전환을 통과한다.

## G04. 검증

기존 파일만 수정하므로 프로젝트 항목 추가는 없다. Debug/Release 제품 빌드, 바뀐 command 및 protocol/native 회귀 검사, JSON parse와 `git diff --check`를 수행한다. Client 화면, 4인 LAN 체감, 패턴의 최종 시각 판정은 사용자가 확인한다. 실제 실행 증거와 남은 사항은 대응 RESULT에 기록한다.
