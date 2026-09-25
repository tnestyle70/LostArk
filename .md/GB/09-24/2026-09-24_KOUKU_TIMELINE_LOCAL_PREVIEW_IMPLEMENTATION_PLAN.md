# 쿠크 타임라인 미리보기와 서버 패턴 재생 분리

## G00. 현재 상태와 목표

Workbench의 Play, Play Preview, Play Bundle과 Play Pattern이 모두
`Request_SelectedServerPlay`로 연결돼 있다. 타임라인을 앞뒤로 조작하는 기존
`Request_PatternPreview` / `Request_BundlePreview`와 MainApp의 local presentation
소비자는 남아 있다. 사용자는 Collider의 실제 Server 판정은 유지하면서 Play를
기존 타임라인 미리보기로 되돌리도록 요청했다.

## G01. 버튼과 시계의 역할

`Client/Private/KoukuSaydonActionWorkbench.cpp`에서 Play 계열 버튼은 현재 cursor의
local preview를 요청한다. Pause/Resume/Stop/Reset과 ruler scrub는 기존 preview
transport를 사용한다. Play Pattern은 현재 applied draft의 immutable snapshot,
검증, 리소스 준비, Server admission과 Collider/Logic 실행 경로를 유지한다.
별도 Stop Pattern은 준비 취소 또는 기존 audition Stop을 요청한다.

서버 준비·재생 중에는 같은 장면의 local preview 시작·scrub를 거절하여 표현과 판정의
시계를 혼합하지 않는다. 서버가 보낸 시간의 읽기 전용 표시와 타임라인 데이터 편집은
기존 계약을 유지한다. Sequence의 명시적 Complete Play는 변경하지 않는다.

## G02. 저장과 구현 경계

현재 사용자가 편집 중인 Composition JSON과 미저장 draft를 보존한다. 이 변경은
소스 JSON, publisher, Shared protocol과 Server 실행 데이터의 변경을 요구하지 않는다.
기존 CPP의 UTF-8 무BOM/CRLF를 유지하며 새 제품 C++ 파일과 프로젝트 등록은 없다.
CLAUDE와 팀 사용서의 두 Play 역할 설명을 실제 동작으로 갱신한다.

## G03. 검증

기존 Workbench 검증 경로에서 cursor 시작·pause·seek·bundle 및 Server 준비/재생 중
상호배제를 확인한다. 해당 CPP의 Debug/Release 컴파일과 가능한 Product Build,
프로젝트 XML parse 및 변경 범위 diff-check를 수행한다. 실행 중 EXE의 링크 점유는
실제 경로로 판단하고 Client를 자동 종료하지 않는다. 화면 조작과 최종 재생 판정은
사용자가 수행하며 실행한 검증과 미실행 항목을 RESULT에 구분한다.
