# Open World Level Tool과 MAP Effect 배치 구현 계획

## G00. 현재 경로와 목표

`kouku-pattern3-sequence`의 기존 미커밋 복원 작업을 유지한다. 실행 중인 Client/Server와 미저장 저작 문서는 종료하거나 외부에서 덮어쓰지 않는다.

Box Detail의 MAP Effect는 `PositionOffset`을 월드 미터 단위의 고정 원점으로 사용한다. Effect Tool의 독립 미리보기 원점과 같지 않다. 저장된 포탈 박스는 14603 ms에 시작하며 centered Effect 내부 첫 emitter는 0초부터 시작한다. 현재 명시적 Effect Preview가 geometry overlay 경로를 사용해 현재 커서에서 paused Pattern을 열므로 앞 공백이 남는다.

## G01. Box Detail 배치와 즉시 미리보기

`KoukuSaydonActionWorkbench`는 선택한 Pattern/Occurrence의 stable ID와 편집 세대를 담은 PICK/FOCUS 요청만 제출한다. `MainApp_WorldLevel.cpp`가 현재 레벨 카메라와 기존 `CGameInstance::Picking`을 사용한다. 클릭은 보이는 mesh의 실제 world-position target을 읽는다. 표면이 없으면 기존 위치를 유지한다. 요청 버튼의 클릭은 배치에 재사용하지 않으며 Esc, 우클릭, 선택·레벨·도구 변경으로 취소한다. 클릭을 gameplay에 전달하지 않는다.

완료 위치는 현재 Box Detail과 기존 staged geometry overlay에 반영한다. 저장은 기존 Apply/Save 계약을 유지한다. 선택된 고정 원점은 Effect가 비활성인 시간에도 XYZ 표식으로 표시하고 Focus/F로 카메라를 이동한다. F는 도구의 해당 패널에 포커스가 있고 텍스트를 편집하지 않을 때만 사용한다.

명시적 Preview는 박스 시작 시각에서 재생한다. 일반 TRS 편집은 기존 preview clock과 paused 상태를 보존한다. 전체 Pattern의 시작 시각과 내부 emitter 간 상대 시간은 변경하지 않는다.

## G02. Open World Level Tool

신규 `WorldLevelTool.h/.cpp`는 기존 Area catalog, map/deploy placement, world sequence, Composition의 저장 문서와 열린 owner draft를 조회하는 통합 목록이다. Area·이름·종류로 선택하고 위치와 시작/길이를 확인한다. 생성·편집 요청은 MapTool, WorldObjectTool, Action/Sequence Workbench로 전달한다. 별도 저장 포맷이나 두 번째 런타임을 만들지 않는다.

`MainApp`은 F1의 도구 열기, input owner, Area 일치 검사, stable ID deep link, 비동기 MapTool Area 준비를 연결한다. 기존 MapTool의 미저장 문서 보호와 명시적인 Test 편집 workspace 경계를 유지한다. 다른 Area를 목록에서 보는 것만으로 현재 레벨을 전환하지 않는다. Bern 카메라는 현재 Level owner의 읽기 전용 typed 접근자로 노출한다.

## G03. 파일과 검증

신규 WorldLevelTool 및 MainApp_WorldLevel CPP는 `Client.vcxproj`와 `.filters`에 기존 폴더 구조로 등록한다. 기존 CPP/H의 인코딩을 유지한다. MainApp, Workbench, MapTool과 typed camera getter를 소비하는 CPP를 실제 Product 컴파일 명령의 `/c` 경로로 검증하며 산출물은 `out`에 격리한다. stale selection/취소/좌표 검증, 명시 Preview 시작 clock과 geometry preview clock 보존을 확인한다. JSON/XML parse 및 `git diff --check`를 수행한다.

현재 실행 중인 프로그램에는 소스 변경이 적용되지 않는다. 전체 Product Build와 EXE 교체는 사용자가 미저장 편집을 저장하고 종료한 뒤 수행한다. 화면 표시와 최종 위치는 사용자가 직접 조작해 판정한다.
