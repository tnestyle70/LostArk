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

## G05. 실제 F1 화면과 G1 데이터 후속

사용자 화면에서 G1이 기존 28개 평면 목록임을 확인했다. 사용자 최종 선택에 따라 Balance Test는 버튼으로 별도 창을 여는 방식을 유지하고, F1에서 Player Follow Camera가 있던 자리에 실행 버튼을 배치한다. 저장·게시 job은 F1 표시 여부와 무관하게 수거한다. Player Follow Camera 패널의 함수·선언·전용 draft는 제거한다. 제품 카메라와 F6 동작은 기존 소비자를 유지한다.

G1은 쓰리투원투하를 포함한 일반 패턴과 기존 추적 연결을 7개 반복 구간으로 배치하고, 130/110/85/60/50/30줄 기믹을 각 한 번 실행한다. 실행 중 패턴과 카운터 후속이 끝난 뒤 전환한다. 실제 authoring과 publisher 출력의 변경은 G1 HP Flow PLAN/RESULT에 기록한다. F1 목록은 반복 청크와 일회 기믹을 구분해 표시하고 Complete Play는 그 저장된 flow를 소비한다.

현재 실행 중인 프로그램은 조작하지 않는다. 소스 컴파일과 데이터 검증, 게시 파일, 재실행 후 화면 판정을 각각 구분한다.

## G06. Complete Play 준비 오류와 Character Select Movie

실제 실행 로그의 준비 실패는 `resources.final_revision`에서 고정된 Action 2237과 저장된 Action 2238이 달라진 경우다. 이때 성공한 Boss Tool Reload 안내가 오류 이유로 표시되는 결함을 수정한다. Server admission도 게시 잠금·로드 실패·Action/Sequence revision 불일치를 구분하며 기존 검증과 실패 시 기존 generation 보존을 유지한다.

F1 공통 `Character Select Movie`는 `CLevel_CharacterSelect::Render_ClassSelectMovieControls()`를 호출한다. Guardian Knight를 기본으로 선택하고 기존 ClassSelectionPresentation의 Play/Stop과 준비 여부를 사용한다. 다른 level에서는 Character Select 진입 안내를 표시하며 Server의 실제 선택 class를 바꾸지 않는다. 기존 파일과 설치된 영화 리소스를 재사용한다.

## G07. Debug 첫 프레임의 이전 CMainApp 객체 파일 복구

13:06~13:07의 실제 실행 세 번은 ClientStartup에서 모든 초기화와 Lobby 진입에 성공한 뒤 Client.exe의 string assign 또는 shared_ptr 해제에서 접근 위반으로 종료됐다. `MainApp.h`에서 camera draft를 제거한 08:33 이후에도 `MainApp_RenderingLighting.obj`와 일부 직접 소비자가 07:30 객체로 남았다. 해당 MSBuild read tracking에는 소스와 PCH만 있고 `MainApp.h`가 빠져 있다. 첫 프레임 `UpdateLightingPreview`가 옛 멤버 위치를 사용하는 경로를 조사했다.

Debug/Release의 직접 소비자·객체 시각·헤더 추적 기록을 대조하고, 오래된 구조가 확인된 Client 객체만 격리한 뒤 같은 VS18 Insiders/toolset의 정상 Product Build로 재컴파일한다. 소스 timestamp나 의존성 기록을 조작하지 않고, PCH/셰이더/Engine 전체 Clean 또는 Rebuild는 하지 않는다. 정상 재컴파일 뒤 각 소비자의 헤더 추적 복구와 실제 compile/link 범위를 확인한다. 빌드 설정 변경이 필요하면 해당 원인을 좁혀 적용하며 최초 실패 EXE의 WER·심벌·초기화 로그는 별도 증거로 보존한다. Client 자율 실행 없이 사용자가 같은 Debug EXE로 로비 화면을 확인한다.
