# Object 목록의 Arena 진입 후 로드 복구 계획

## G00. 실제 증상과 원인

첨부 화면은 KoukuSaydon 아레나 진입을 완료했지만 Composition Actions의 Object 목록이
`Enter KoukuSaydon, then Reload Source`를 표시한다. `CWorldObjectTool::Open`은
Load 실패에도 m_Open=true를 유지하고, Begin_WorkbenchFrame은 최초 Open만 호출한다.
아레나 진입 전 실패 뒤 실제 진입해도 다시 로드하지 않는다. 수동 Reload는 비선택
Action Workbench 도킹 탭에만 있어 Object 목록에서는 찾기 어렵다.

## G01. 코드 변경

기존 `Client/Public/WorldObjectTool.h`에 최초 source load를 Arena 활성화까지 미룬 상태
`m_SourceLoadDeferred`를 추가한다. 저장 문서의 dirty 상태나 preview 준비와 구분한다.

기존 `Client/Private/WorldObjectTool.cpp`의 Load_Source는 실제 CurrentLevelID와
Get_Active를 함께 확인한다. Arena 객체 생성만으로 아직 구성 중인 validation target을 읽지 않는다.
미로드 상태에서 Arena가 없으면 재시도를 예약하고, 실제 읽기 시작 전 예약을 해제한다.
따라서 파일 누락·검증 실패는 매 frame I/O를 반복하지 않는다.

Begin_WorkbenchFrame은 미로드·미편집·지연 상태이며 실제 Kouku Level이 활성화되면
기존 Load_Source를 한 번 재시도한다. 이미 로드한 문서와 미저장 초안은 자동 교체하지 않는다.
PATTERNS pane의 미로드 분기에 Reload Source를 표시해 다른 도킹 탭을 열지 않고 복구한다.
수동 Reload와 dirty 확인에는 기존 Render_Toolbar 경로를 재사용한다.

새 C++ 파일·project/filter 등록은 없다. Map/WorldSequence 데이터와 Publish, Resources,
렌더링 설정, 이전 네트워크 변경은 수정하지 않는다.

## G02. 검증·실행 경계

최초 Lobby/Loading 프레임, Arena 객체만 생성된 상태, 실제 활성 후 1회 로드, 파일 실패 후
반복 프레임, 수동 재시도, 이미 로드한 dirty draft 보존을 확인한다.
변경 TU 최소 컴파일과 관련 기존 계약 검사, diff/인코딩 검증을 수행한다.
Client가 실행 중이면 먼저 out 전용 OBJ/PDB로 컴파일해 실행파일을 보존한다.
정상 Product 링크는 실행 중 Client 출력물 점유가 풀린 뒤에만 진행한다.
사용자의 Client/UI를 종료하거나 조작하지 않는다. 기존 실행에서 즉시 시도할 복구 경로는
오른쪽 위 Action Workbench 탭 → Reload Source다. 최종 목록·화면 검증은 사용자가 한다.
