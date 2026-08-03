# Character Select와 LEVEL 단일 전환 구조 계획

작성일
2026-08-03

작업 모드
STRUCTURE_FIRST

이 문서는 사용자가 직접 코드를 작성하기 위한 설계 정본이다.
전체 C++ 코드를 대신 작성하지 않는다.
파일이 왜 존재하는지, 어디에 추가하는지, 어떤 헤더가 필요한지, 누가 함수를 호출하는지, 어떤 상태를 바꾸는지, 어떤 불변식을 지켜야 하는지를 먼저 설명한다.

## 1. Level_CharacterSelect가 정말 필요한가

먼저 결론

현재 목표를 기준으로는 `CLevel_CharacterSelect`가 필요하다.

현재 목표는 단순히 ImGui에서 직업 enum 하나를 고르는 것이 아니다.

캐릭터 네 직업 미리보기
→ 선택 캐릭터 전환
→ 전용 카메라와 조명
→ 대기 애니메이션
→ 장비와 색상 변경
→ 외형 커스터마이징
→ 뼈와 socket 확인
→ Effect 연결
→ secondary motion과 solver 확인
→ PBR, SSAO, outline 결과 확인
→ 선택 결과 저장
→ Lobby 복귀
→ 실제 Server 승인 입장에 같은 직업 사용

이 범위는 Lobby의 버튼 패널 하나로 끝나지 않는다.
전용 리소스 묶음, 전용 카메라, 전용 조명, 전용 Layer, 전용 ImGui 편집 상태, 독립적인 진입과 종료 수명이 필요하다.
그래서 별도 Level로 분리하는 것이 맞다.

반대로 다음 범위라면 `CLevel_CharacterSelect`를 만들면 안 된다.

ImGui에 직업 버튼 네 개만 표시
→ enum 하나 저장
→ 바로 Lobby에서 Server 입장 요청

이 정도라면 Lobby 내부 패널이나 Lobby sub-state면 충분하다.
별도 Level을 만들면 Loader, Camera, Light, Layer, 전환 코드가 불필요하게 늘어난다.

이번 계획의 판정 기준

현재 사용자가 원하는 최종 범위에는 3D 미리보기와 제작 툴 연결이 포함된다.
따라서 `LEVEL::CHARACTER_SELECT`와 `CLevel_CharacterSelect`를 유지한다.
다만 첫 구현에서는 직업 선택과 3D 미리보기까지만 닫고, 장비·색상·물리·Effect 편집은 같은 Level 위에 후속 수직 슬라이스로 올린다.
미래 기능을 위해 빈 Manager나 placeholder 파일을 미리 만들지는 않는다.

## 2. 이번 변경이 해결하는 문제

현재 구조의 문제

`CLIENT_SCENARIO`가 Level, 개발 도구, 실행 옵션, smoke 시나리오를 한 enum 안에서 함께 표현한다.
`ClientLaunchOptions`가 시작 Level, Local Preview, Multiplayer, 선택 직업, 서버 주소, Effect 자동 실행, smoke 종료까지 함께 소유한다.
`LevelCatalog.json`이 실행 시나리오와 Level을 다시 연결한다.
`CMainApp`이 실제 게임 루프뿐 아니라 smoke harness, offline overlay, 도구 자동 실행, scene parsing을 함께 담당한다.
Lobby가 캐릭터 선택, Local Preview, Multiplayer 연결, 월드 선택을 동시에 담당한다.
제품 Level에서도 Local Preview와 Server replicated path가 함께 존재한다.

이 상태가 계속되면 기능 하나를 추가할 때 다음 경계를 모두 건드리게 된다.

명령행 파서
→ scenario enum
→ catalog JSON
→ registry
→ loader
→ MainApp smoke
→ Lobby
→ 실제 Level

사용자가 지적한 Winters Engine의 문제도 같은 종류다.
도구와 검증을 계속 MainApp과 런타임 분기에 붙이면 하나의 클래스가 모든 예외를 아는 괴물이 된다.

수정 후 목표

Client는 항상 Lobby에서 시작한다.
Lobby에는 Test, Character Select, Valtan, Bern 네 진입 명령만 있다.
Character Select는 네트워크 없이 전용 Level로 이동한다.
Test, Valtan, Bern은 실제 Server 접속과 입장 승인을 통과한 뒤 해당 Level을 로드한다.
Level 전환 요청은 `LEVEL`만 전달한다.
Loader는 해당 Level에 필요한 리소스만 준비한다.
MainApp은 한 프레임의 안전한 지점에서만 실제 `Change_Level`을 호출한다.
Client 내부 smoke harness와 Local Preview 경로는 삭제한다.
검증은 Visual Studio의 Server + Client 동시 실행, 기존 NetworkProtocolHarness, Server contract test, ProjectAudit으로 수행한다.

## 3. 수정 후 전체 호출 흐름

Client 시작

`WinMain`
→ `CMainApp::Initialize`
→ 공용 Prototype과 ImGui 초기화
→ `CMainApp::Start_Level(LEVEL::LOBBY)`
→ `CLevel_Loading` 생성
→ `CLoader::Ready_For_Lobby`
→ `CLevel_Lobby` 생성
→ MainApp이 안전한 프레임 경계에서 Lobby 활성화

Character Select 진입

Lobby의 Character Select 버튼
→ `CLobbyCommandService::Request(LOBBY_STAGE::CHARACTER_SELECT)`
→ `CLevel_Lobby::Update`가 command 소비
→ `CLevelTransitionService::Request_Load(LEVEL::CHARACTER_SELECT)`
→ `CMainApp::Update`가 Engine update 반환 후 request 소비
→ `CMainApp::Start_Level(LEVEL::CHARACTER_SELECT)`
→ Loading Level
→ `CLoader::Ready_For_CharacterSelect`
→ `CLevel_CharacterSelect` 생성
→ MainApp이 activation request 적용

직업 선택

Character Select ImGui 버튼
→ 선택 index 변경
→ 선택한 직업의 preview를 먼저 staging
→ staging 성공
→ 기존 preview 제거
→ 새 preview를 active로 교체
→ 카메라 target 갱신

확정

Confirm 버튼
→ `CCharacterSelectionState::Select`
→ 선택 직업 유효성 확인
→ 선택 상태 commit
→ `CLevelTransitionService::Request_Load(LEVEL::LOBBY)`
→ Loading
→ Lobby 복귀

Test 진입

Lobby의 Test 버튼
→ 선택 직업 존재 확인
→ `CNetworkManager::Connect("127.0.0.1", 7777)`
→ `C2S_ENTER_WORLD`에 `WORLD_ID::TRAINING_GROUND`와 선택 직업 전송
→ Lobby가 최대 5초 동안 승인 대기
→ 일치하는 `S2C_ENTER_ACCEPTED` 수신
→ `CLevelTransitionService::Request_Load(LEVEL::DEVELOPMENT)`
→ Loader가 `dev.training.ground` 범위 준비
→ Development 활성화

Valtan과 Bern 진입

Test와 같은 흐름을 사용한다.
차이는 Lobby stage가 고정된 `WORLD_ID`와 `LEVEL`로 변환되는 부분뿐이다.
승인 전에는 절대로 제품 Level을 먼저 열지 않는다.

제품 Level 이탈

Server disconnect 감지
→ replicated object와 pending state 정리
→ `CLevelTransitionService::Request_Load(LEVEL::LOBBY)`
→ MainApp이 프레임 경계에서 Loading으로 전환
→ Local Preview로 자동 우회하지 않음

## 4. 전체 파일 변경 지도

유지하면서 수정할 파일

`Client/Public/Client_Defines.h`
`Client/Public/MainApp.h`
`Client/Private/MainApp.cpp`
`Client/Public/LevelRegistry.h`
`Client/Private/LevelRegistry.cpp`
`Client/Public/Loader.h`
`Client/Private/Loader.cpp`
`Client/Public/Level_Lobby.h`
`Client/Private/Level_Lobby.cpp`
`Client/Public/Level_Loading.h`
`Client/Private/Level_Loading.cpp`
`Client/Public/LobbyCommandService.h`
`Client/Private/LobbyCommandService.cpp`
`Client/Public/ClientReplication.h`
`Client/Private/ClientReplication.cpp`
`Client/Public/Level_Bern.h`
`Client/Private/Level_Bern.cpp`
`Client/Public/Level_ValtanArena.h`
`Client/Private/Level_ValtanArena.cpp`
`Client/Public/Level_Development.h`
`Client/Private/Level_Development.cpp`
`Client/Public/Effect_Tool.h`
`Client/Private/Effect_Tool.cpp`
`Client/Default/Client.vcxproj`
`Client/Default/Client.vcxproj.filters`
`Tools/Build/Invoke-BuildAndRegression.ps1`
`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

기존 파일을 이름 변경하며 수정할 파일

`Client/Public/SceneTransitionService.h`
→ `Client/Public/LevelTransitionService.h`

`Client/Private/SceneTransitionService.cpp`
→ `Client/Private/LevelTransitionService.cpp`

이것은 두 번째 전환 시스템을 추가하는 작업이 아니다.
기존 service에서 `CLIENT_SCENARIO`를 제거하고 Level 수명 전환만 남긴 뒤 역할에 맞게 이름을 바꾸는 작업이다.

새로 추가할 C++ 파일

`Client/Public/Level_CharacterSelect.h`
`Client/Private/Level_CharacterSelect.cpp`
`Client/Public/CharacterSelectionState.h`
`Client/Private/CharacterSelectionState.cpp`

현재 worktree에는 `Level_CharacterSelect.h/.cpp` 경로와 프로젝트 항목이 이미 생겨 있다.
그러나 내용이 비어 있거나 잘못된 문자가 있다면 신규 설계 기준으로 다시 작성해야 한다.
이 계획서는 해당 파일을 이미 완성된 것으로 간주하지 않는다.

새로 추가할 실행 설정 파일

`Framework.slnLaunch`

삭제할 파일

`Client/Public/ClientLaunchOptions.h`
`Client/Private/ClientLaunchOptions.cpp`
`Client/Public/LevelCatalog.h`
`Client/Private/LevelCatalog.cpp`
`Client/Public/OfflinePlayerPreview.h`
`Client/Private/OfflinePlayerPreview.cpp`
`Data/Levels/LevelCatalog.json`
`Tools/Build/Invoke-OfflineClientSmoke.ps1`
`Tools/Build/Invoke-NetworkEndpointSmoke.ps1`

삭제할 MainApp 함수

`CMainApp::RenderOfflinePreviewOverlay`
`CMainApp::UpdateSmokeHarness`
`CMainApp::CompleteSmokeHarness`

삭제할 개념

`CLIENT_SCENARIO`
`CLIENT_ENTRY_MODE`
`LOCAL_PREVIEW`
Client 실행 인자의 `--smoke`
Client 실행 인자의 `--scenario`
Client 실행 인자의 Effect 자동 실행과 자동 종료
Client에서 제품 Stage를 offline Character로 대신 검증하는 경로

## 5. Client_Defines.h

수정 위치

`Client/Public/Client_Defines.h`

파일이 존재하는 이유

Client 전체가 공유하는 고정 enum과 기본 타입 경계를 제공한다.
Level 숫자는 Engine Level resource index와 연결되므로 각 Level이 임의의 정수를 사용하면 안 된다.

LEVEL에 필요한 값

`STATIC`
`LOADING`
`LOBBY`
`CHARACTER_SELECT`
`BERN`
`VALTAN_ARENA`
`DEVELOPMENT`
`END`

중요한 불변식

`CHARACTER_SELECT`는 실행 scenario가 아니라 실제 Level resource index다.
Test를 위해 새 `LEVEL::TEST`나 `LEVEL::TRAINING`을 추가하지 않는다.
Test 버튼은 기존 `LEVEL::DEVELOPMENT`와 `WORLD_ID::TRAINING_GROUND`를 사용한다.
모르는 값은 `LOBBY`로 fallback하지 않고 실패한다.

왜 `FRONT_CHARACTER_SELECT`가 필요하지 않은가

`LEVEL::CHARACTER_SELECT`가 이미 실제 화면과 resource lifetime을 표현한다.
`FRONT_CHARACTER_SELECT` 같은 두 번째 enum은 같은 사실을 다른 축에서 중복 표현한다.
중복 enum은 registry와 catalog의 호환성 검사를 다시 요구하고 잘못된 조합을 만들 수 있다.
따라서 `CLIENT_SCENARIO::FRONT_CHARACTER_SELECT`는 추가하지 않고 `CLIENT_SCENARIO` 자체를 폐기한다.

## 6. CharacterSelectionState.h

정확한 추가 위치

헤더
`Client/Public/CharacterSelectionState.h`

구현
`Client/Private/CharacterSelectionState.cpp`

프로젝트
`Client/Default/Client.vcxproj`

헤더 항목
`ClInclude Include="..\Public\CharacterSelectionState.h"`

CPP 항목
`ClCompile Include="..\Private\CharacterSelectionState.cpp"`

필터
헤더와 CPP 모두 `01.Levels\01. CharacterSelect`

추가 include directory
없음

추가 project reference
없음

추가 library
없음

파일이 존재하는 이유

Character Select에서 확정한 직업은 Character Select Level이 파괴된 뒤에도 살아 있어야 한다.
Lobby가 Server 입장 요청을 만들 때 같은 값을 읽어야 한다.
제품 Level의 Loader가 선택 직업 Prototype을 준비할 때도 같은 값을 읽어야 한다.

이 상태를 Level 멤버에 두면 Level 전환 순간 사라진다.
NetworkManager에 두면 접속 전 선택이라는 의미와 맞지 않고 `Close()` 시 함께 초기화될 수 있다.
CharacterCatalog에 두면 읽기 전용 캐릭터 정의와 현재 사용자의 가변 선택을 섞게 된다.
LobbyCommandService에 두면 일회성 command queue와 지속 상태를 섞게 된다.

현재 구조에서 가장 작은 경계는 Client process 범위의 선택 상태 하나다.
나중에 계정 캐릭터 슬롯과 서버 저장이 생기면 이 클래스는 server-provided selection model로 교체될 수 있다.

필요한 헤더

`Network/PacketType.h`
선택 직업 타입인 `LostArk::Shared::CHARACTER_CLASS_ID`가 선언돼 있다.

`Engine_Defines.h`
프로젝트에서 사용하는 `bool_t`와 공용 기본 타입이 필요할 때 사용한다.
표준 `bool`만 쓸 수 있다면 이 include를 줄일 수 있는지 실제 정의를 확인한다.

외부에 공개할 함수

`Select(CHARACTER_CLASS_ID characterClass)`

존재 이유
검증되지 않은 enum 값을 상태에 직접 대입하지 않도록 commit 입구를 하나로 만든다.

호출자
`CLevel_CharacterSelect::Confirm_Selection`

입력
사용자가 확정한 직업 ID

내부 흐름
지원 직업인지 검사
→ 유효하면 선택 값 저장
→ 선택 존재 상태 true
→ 성공 반환

상태 변화
현재 선택 직업과 선택 존재 여부를 함께 변경한다.

실패
지원하지 않는 ID면 기존 선택을 보존하고 false를 반환한다.

`Has_Selection()`

존재 이유
Lobby가 Server 입장을 요청하기 전에 선택이 존재하는지 확인한다.

호출자
`CLevel_Lobby::Begin_StageRequest`
제품 Level Loader의 직업 Prototype 준비 함수

상태 변화
없음

`Get_SelectedClass()`

존재 이유
확정된 직업 ID를 읽는다.

선행 조건
`Has_Selection()`이 true여야 한다.

실패 정책
선택이 없을 때 임의의 LANCE_MASTER를 반환하면 안 된다.
함수 계약을 optional 반환으로 만들거나, 호출자가 먼저 `Has_Selection()`을 확인하는 불변식을 명확히 고정한다.

내부 자료구조

선택 직업 ID 하나
선택 존재 여부 하나

자료구조가 작은 이유

이 클래스는 외형 전체를 저장하는 장소가 아니다.
첫 수직 슬라이스에서는 Server 입장에 필요한 stable class ID만 소유한다.
장비, 염색, 얼굴 값이 실제 server 계약에 추가될 때 별도 immutable selection payload로 확장한다.

핵심 불변식

선택 없음과 기본 직업은 같은 상태가 아니다.
지원하지 않는 enum은 저장하지 않는다.
Level 전환이 선택을 지우지 않는다.
Network disconnect가 선택을 지우지 않는다.

## 7. Level_CharacterSelect.h

정확한 추가 위치

헤더
`Client/Public/Level_CharacterSelect.h`

구현
`Client/Private/Level_CharacterSelect.cpp`

프로젝트
`Client/Default/Client.vcxproj`

헤더 항목
현재 등록된 `ClInclude Include="..\Public\Level_CharacterSelect.h"`를 사용한다.

CPP 항목
현재 등록된 `ClCompile Include="..\Private\Level_CharacterSelect.cpp"`를 사용한다.

필터
헤더와 CPP 모두 현재 존재하는 `01.Levels\01. CharacterSelect`

추가 include directory
없음

추가 project reference
없음

추가 library
없음

파일이 존재하는 이유

캐릭터 선택 제작 화면의 수명을 Lobby와 분리한다.
선택 화면 전용 Camera, Light, Character preview, ImGui panel을 소유한다.
이 Level이 닫히면 선택 화면에만 필요한 GameObject와 Level resource가 함께 정리돼야 한다.

이 클래스가 소유하지 않는 것

Server socket
입장 승인
월드 Player entity
Network snapshot
게임play authority
Character 원본 catalog
Effect asset 원본
Physics solver 구현 자체

필요한 헤더

`Client_Defines.h`
`LEVEL::CHARACTER_SELECT`와 Client 기본 타입을 사용한다.

`Level.h`
`Engine::CLevel`을 상속하고 override 함수를 선언한다.

`Network/PacketType.h`
멤버로 `CHARACTER_CLASS_ID` 값을 직접 보관한다면 완전한 enum 정의가 필요하다.

`<array>`
지원 직업 네 개가 컴파일 시점에 고정되어 있으므로 `std::array`를 사용한다.

전방 선언 가능한 타입

`CCamera_Free`
`CCharacter`

헤더에서는 `shared_ptr<CCamera_Free>`와 `shared_ptr<CCharacter>`만 보관한다.
실제 함수 호출과 `dynamic_pointer_cast`가 있는 CPP에서 각각의 헤더를 include한다.

왜 vector가 아니라 array인가

현재 지원 직업은 LANCE_MASTER, GUNSLINGER, SLAYER, ARTIST 정확히 네 개다.
런타임에 개수가 늘거나 줄지 않는다.
고정 크기라는 불변식을 타입으로 표현하고 heap allocation을 피하기 위해 array가 맞다.
향후 Server 캐릭터 슬롯 목록이 가변적으로 내려오면 그때 vector 기반 slot view model로 바꾼다.

외부에 공개할 함수

`Initialize()`

존재 이유
Loader가 준비한 Prototype을 사용해 Character Select 화면의 실제 객체를 만든다.

호출자
`CLevel_CharacterSelect::Create`

내부 흐름
부모 Level 초기화
→ Light 준비
→ Camera 준비
→ 초기 preview 준비
→ 하나라도 실패하면 초기화 실패 반환

중요한 경계
파일을 읽거나 Model을 디코드하지 않는다.
무거운 리소스 준비는 Loader에서 끝나 있어야 한다.

`Update(float deltaSeconds)`

존재 이유
Character Select의 매 프레임 입력과 UI command를 처리한다.

호출자
Engine Level update

내부 흐름
선택 panel 입력 반영
→ preview 회전이나 camera update
→ Confirm 또는 Back 요청 처리
→ Level 전환은 service에 요청만 제출

상태 변화
현재 preview index
UI status
preview transform

금지
`Change_Level` 직접 호출
Network connect
Server packet 전송
매 프레임 asset file 읽기

`Render()`

존재 이유
Character Select 배경, preview, ImGui panel을 한 화면으로 보여준다.

호출자
Engine Level render

내부 흐름
Level name 또는 기본 UI 렌더
→ 선택 직업 버튼
→ 확정과 뒤로 버튼
→ 현재 선택과 오류 상태 표시

`Create(device, context)`

존재 이유
객체 생성과 Initialize 성공 여부를 한 지점에서 닫는다.

내부 흐름
객체 생성
→ Initialize
→ 실패하면 정리하고 nullptr
→ 성공하면 unique_ptr 반환

Character Select 내부 함수

`Ready_Lights()`

존재 이유
PBR Character preview가 Lobby나 제품 맵 조명에 의존하지 않도록 전용 조명을 만든다.

상태 변화
Engine Light registry에 Character Select Level용 light 추가

실패
조명 추가 실패면 Level 초기화를 중단한다.

`Ready_Camera()`

존재 이유
Character preview를 보는 전용 카메라를 만든다.

내부 흐름
Loader가 등록한 Camera Prototype clone
→ `LAYER_CAMERA`에 추가
→ `CCamera_Free`로 cast
→ 성공한 shared pointer 저장

불변식
Initialize 성공 후 camera pointer는 null이 아니다.
카메라가 preview를 강하게 소유하지 않도록 기존 camera target 계약을 확인한다.

`Ready_Preview(characterClass)`

존재 이유
선택한 직업의 3D Character를 화면에 표시한다.

내부 흐름
직업 ID 검증
→ CharacterCatalog에서 해당 `CHARACTER_SPEC` 조회
→ 필요한 Prototype tag 확인
→ 새 Character를 임시 local 변수로 clone
→ transform과 animation 초기화
→ 모든 준비 성공
→ 기존 preview 제거
→ 새 preview를 active pointer로 교체
→ camera follow target 갱신

왜 staging이 필요한가

기존 preview를 먼저 지운 뒤 새 Character 생성에 실패하면 화면이 비어 버린다.
새 preview가 완전히 준비된 뒤 교체하면 실패 시 기존 화면을 보존할 수 있다.

`Select_Preview(index)`

존재 이유
ImGui index를 stable class ID로 변환하고 preview 교체를 요청한다.

입력 검증
index가 array 범위 안인지 확인한다.

실패
범위를 벗어나면 기존 index와 preview를 유지한다.
새 preview 생성 실패면 기존 index를 유지하고 status에 원인을 남긴다.

`Confirm_Selection()`

존재 이유
화면에서 보고 있는 직업을 process selection state에 확정한다.

내부 흐름
active preview 존재 확인
→ current class 유효성 확인
→ `CCharacterSelectionState::Select`
→ 성공하면 Lobby load request 제출

실패
state commit 실패면 Character Select에 남고 기존 확정 선택을 보존한다.
transition request가 이미 pending이면 중복 요청을 만들지 않는다.

`Request_Back()`

존재 이유
이번 화면의 임시 선택을 확정하지 않고 Lobby로 돌아간다.

불변식
기존에 확정된 `CCharacterSelectionState`는 바꾸지 않는다.

Character Select 멤버 자료구조

지원 직업 목록
`std::array<CHARACTER_CLASS_ID, 4>`

현재 preview index
array의 현재 위치

현재 preview Character
`shared_ptr<CCharacter>`

전용 Camera
`shared_ptr<CCamera_Free>`

상태 문자열
사용자가 실패 원인을 볼 수 있는 문자열

중요한 불변식

현재 index가 유효하면 array 범위 안이다.
현재 preview가 존재하면 preview의 class ID는 array의 현재 값과 같다.
Preview 전환 실패 시 기존 preview와 index가 보존된다.
Confirm 전에는 전역 선택 상태를 바꾸지 않는다.
Character Select는 Server authority를 만들지 않는다.

## 8. LevelRegistry.h

수정 위치

`Client/Public/LevelRegistry.h`
`Client/Private/LevelRegistry.cpp`

파일이 존재하는 이유

`LEVEL` 하나를 실제 Level 생성 함수와 Loader 함수에 연결하기 위해 존재한다.
Level마다 MainApp switch와 Loader switch를 따로 만들면 같은 매핑이 여러 파일에 복제된다.
Registry는 enum을 실행 함수로 변환하는 단일 dispatch table이다.

Registry가 Catalog와 다른 이유

Catalog는 외부 JSON의 stable scenario ID, kind, tool, asset domain을 파싱했다.
Registry는 컴파일된 `LEVEL`과 함수 포인터의 연결만 가진다.
사용자가 선택할 실행 시나리오를 저장하지 않는다.
파일을 읽지 않는다.
Prototype tag나 placement를 저장하지 않는다.

필요한 헤더

`Client_Defines.h`
`LEVEL` 정의

`Engine_Defines.h`
`HRESULT`와 Engine 기본 타입

`MapLoadScope.h`
제품 map descriptor가 `MAP_LOAD_SCOPE`를 값으로 가진다면 필요하다.

`<memory>`
Level 생성 함수가 `unique_ptr<CLevel>`을 반환한다.

전방 선언

`Engine::CLevel`
`CLoader`

헤더에서 pointer와 함수 시그니처만 필요하므로 완전한 클래스 정의는 CPP로 미룬다.

수정할 descriptor

Level ID
제품 또는 Development 구분
디버그 표시용 stable name
Level 생성 함수 포인터
Loader 준비 함수 포인터
필요한 경우 map area ID
필요한 경우 map load scope

stable name은 저장 ID가 아니다.
로그와 도구 표시를 위한 읽기 전용 이름이다.

외부에 공개할 함수

`Find(LEVEL level)`

존재 이유
Level 하나의 descriptor를 찾는다.

내부 흐름
고정 array 순회
→ 같은 LEVEL 검색
→ 발견하면 주소 반환
→ 없으면 nullptr

`Create_Level(LEVEL level, device, context)`

존재 이유
MainApp이나 Loading이 구체 Level 헤더를 모두 알지 않고 생성하게 한다.

내부 흐름
Find
→ descriptor 검증
→ create function 호출
→ 생성 결과 반환

`Execute_Load(LEVEL level, CLoader& loader)`

존재 이유
Loader가 Level switch를 직접 소유하지 않게 한다.

내부 흐름
Find
→ load function 검증
→ 해당 `Ready_For_*` 호출

등록해야 하는 Level

Lobby
Character Select
Bern
Valtan Arena
Development

Loading은 Loading 자신을 준비하기 위한 대상이 아니므로 일반 registry target에서 제외할 수 있다.

자료구조

`std::array<CLIENT_LEVEL_DESCRIPTOR, 5>`

array인 이유
제품에서 지원하는 Level 집합이 컴파일 시점에 고정되어 있다.
동적 plugin registry가 아니므로 map allocation이 필요하지 않다.

핵심 불변식

각 LEVEL은 정확히 한 번만 등록된다.
create function과 load function은 null이 아니다.
모르는 LEVEL은 Lobby로 fallback하지 않는다.
Character Select create와 load 함수가 같은 descriptor에 함께 등록된다.
Loader와 runtime map scope가 같은 descriptor를 소비한다.

## 9. Loader.h

수정 위치

`Client/Public/Loader.h`
`Client/Private/Loader.cpp`

파일이 존재하는 이유

Level 진입 전에 shader, model, camera prototype, map placement와 navigation 같은 무거운 리소스를 준비하기 위해 존재한다.
Level의 `Initialize` 안에서 이 작업을 전부 하면 화면 thread가 멈추고 중간 실패 rollback이 어려워진다.

Loader는 Level을 선택하지 않는다.
이미 결정된 `LEVEL`의 리소스를 준비한다.

필요한 헤더

현재 Loader thread, atomic state, D3D device/context, Level resource rollback에 필요한 기존 헤더를 유지한다.
`ClientLaunchOptions.h`와 `LevelCatalog.h` include는 제거한다.
선택 직업을 읽는 구현부에서 `CharacterSelectionState.h`를 include한다.
Registry를 통한 dispatch에 필요한 선언은 기존 `LevelRegistry` 경계를 사용한다.

외부에 공개할 핵심 함수

`Initialize(LEVEL nextLevel)`
대상 Level 저장
→ state RUNNING 준비
→ worker thread 생성

`Start_Loading()`
Registry에서 대상 Level load function 실행
→ 성공이면 SUCCEEDED
→ 실패면 FAILED와 HRESULT 보존

`Finished()`
worker가 종료 상태인지 조회한다.

`Failed()`
실패 종료인지 조회한다.

`Get_Result()`
원래 HRESULT를 반환한다.
실패 이유를 `E_FAIL` 하나로 뭉개지 않는다.

추가 또는 완성할 함수

`Ready_For_CharacterSelect()`

존재 이유
Character Select 전용 Level resource bundle을 준비한다.

처리 순서
Character Select Level rollback scope 시작
→ Camera Prototype 등록
→ animated mesh shader 등록
→ Character 공용 부품 Prototype 등록
→ 네 지원 직업의 Character rendering Prototype 등록
→ 전부 성공하면 commit

Character Select에서 준비하지 않는 것

Bern map
Valtan map
Navigation
Server world entity
Boss Prototype

`Ready_For_Development()` 수정

Test는 더 이상 scenario를 읽지 않는다.
항상 `dev.training.ground`와 `WORLD_ID::TRAINING_GROUND` 계약에 필요한 리소스를 준비한다.
선택 직업이 없으면 실패한다.

`Ready_Character_Rendering(levelIndex, characterClass)` 형태 검토

현재 함수가 `ClientLaunchOptions`를 내부에서 읽는다면 책임이 숨겨져 있다.
필요한 직업을 인자로 받도록 바꾸는 편이 낫다.

호출 흐름
Character Select Loader는 네 직업 각각에 대해 호출
제품 Level Loader는 확정된 직업 하나에 대해 호출

장점
함수 입력만 보면 무엇을 준비하는지 알 수 있다.
명령행 전역 상태 의존성이 사라진다.
테스트할 때 잘못된 class ID를 직접 전달할 수 있다.

Loader 자료구조

대상 LEVEL
worker HANDLE
atomic state
atomic HRESULT
cancellation flag
status text와 mutex

핵심 불변식

한 Loader instance는 한 대상 LEVEL만 준비한다.
실패한 Level resource는 rollback scope가 제거한다.
완료 전에 Level을 생성하지 않는다.
취소 시 worker를 강제 종료하지 않는다.
ClientLaunchOptions나 scenario를 읽지 않는다.

## 10. LevelTransitionService.h

정확한 위치

기존
`Client/Public/SceneTransitionService.h`
`Client/Private/SceneTransitionService.cpp`

변경 후
`Client/Public/LevelTransitionService.h`
`Client/Private/LevelTransitionService.cpp`

프로젝트
`Client/Default/Client.vcxproj`

필터
헤더와 CPP 모두 기존 `01.Levels`

작업 성격
신규 service 추가가 아니라 기존 service 이름 변경과 책임 축소다.
프로젝트에서 옛 두 파일 항목을 제거하고 새 두 경로를 등록한다.

파일이 존재하는 이유

Level 자신의 `Update()` 안에서 `Change_Level`을 직접 호출하지 않게 하는 수명 경계다.

현재 `CLevel_Manager::Change_Level`은 기존 Level resource를 지우고 현재 Level pointer를 reset한다.
어떤 Level이 자신의 `Update()` 도중 직접 이 함수를 호출하면 실행 중인 `this`가 파괴될 수 있다.
함수가 아직 return하지 않았는데 객체 수명이 끝나는 undefined behavior다.

그래서 Level은 요청만 제출한다.
실제 `Change_Level`은 `Update_Engine()`이 완전히 반환된 뒤 MainApp이 호출한다.

필요한 헤더

`Client_Defines.h`
대상 `LEVEL`

`Engine_Defines.h`
`bool_t`, `HRESULT`

`<string>`
요청 source와 status 보존

`<optional>`은 구현 파일에서 pending request를 보관할 때 사용한다.

요청 자료구조

요청 단계
LOAD 또는 ACTIVATE

대상 LEVEL

요청 source
어느 객체가 요청했는지 디버깅 문자열

왜 단계가 필요한가

일반 Level에서 다른 Level로 갈 때는 먼저 Loading을 열어야 한다.
Loading worker가 끝난 뒤에는 같은 target을 다시 load하는 것이 아니라 이미 준비된 실제 Level을 활성화해야 한다.
둘 다 target은 LEVEL 하나지만 lifecycle 동작이 다르다.

외부에 공개할 함수

`Request_Load(LEVEL target, const char* source)`

호출자
Lobby
Character Select
Bern
Valtan
Development
Loading failure recovery

내부 흐름
target 유효성 검사
→ LOADING과 STATIC 같은 금지 target 검사
→ mutex 잠금
→ pending request 존재 검사
→ LOAD request 저장
→ status 갱신

`Request_Activation(LEVEL target, const char* source)`

호출자
`CLevel_Loading::Update`

존재 이유
Loader가 성공한 target을 실제 Level로 생성해 활성화해 달라고 MainApp에 요청한다.

`Try_Consume(LEVEL_TRANSITION_REQUEST& outRequest)`

호출자
`CMainApp::Apply_LevelRequest`

내부 흐름
mutex 잠금
→ pending 없음이면 false
→ outRequest로 이동
→ pending 제거
→ true

`Is_Pending()`
ImGui 버튼 중복 입력 차단과 상태 표시용이다.

`Get_Status()`
최근 요청이나 거부 원인을 사용자에게 보여준다.

`Report_LoadFailure(HRESULT result)`와 failure consume
Loading 실패 이유를 Lobby 또는 MainApp UI에 보존해야 한다면 유지한다.
단순 bool로 실패를 숨기지 않는다.

핵심 불변식

동시에 pending request는 하나뿐이다.
요청은 `LEVEL`만 식별한다.
scenario, tool, local/multiplayer 모드를 포함하지 않는다.
service는 `Change_Level`을 호출하지 않는다.
MainApp만 request를 소비한다.

## 11. MainApp.h

수정 위치

`Client/Public/MainApp.h`
`Client/Private/MainApp.cpp`

파일이 존재하는 이유

Client process 전체의 시작과 종료를 조율한다.
Engine frame, Network update, ImGui frame, debug tool lifetime, 안전한 Level 교체를 연결한다.
게임 Stage의 세부 상태나 자동 검증 시나리오를 소유하지 않는다.

삭제할 헤더 의존성

`ClientLaunchOptions.h`
`LevelCatalog.h`
`OfflinePlayerPreview.h`
smoke report file 작성에만 쓰던 헤더

삭제할 함수

`RenderOfflinePreviewOverlay()`
Local Preview 개념이 사라진다.

`UpdateSmokeHarness()`
Client runtime이 자동 scenario를 실행하지 않는다.

`CompleteSmokeHarness(bool_t succeeded, const char_t* reason)`
Client가 report를 쓰고 자동 종료하지 않는다.

`Apply_PendingSceneTransition()`
이 이름과 scenario parsing 구현은 삭제한다.
대신 최소 함수 `Apply_LevelRequest()`를 둔다.

이 둘의 차이

옛 함수는 scenario 선택, catalog compatibility, tool activation까지 해석했다.
새 함수는 service에서 받은 LEVEL lifecycle request만 적용한다.

삭제할 멤버

smoke 시작 시간
smoke scenario
smoke dispatch 여부
network stage 관찰 여부
pending enter 관찰 여부
smoke 완료 여부
offline overlay 상태
scenario 기반 transition status

유지할 멤버

ImGui layer
MapTool
EffectTool
AnimationTool
HUDLayoutTool
Developer Tools 표시 상태
Profiler 상태

수정할 public 함수

`Initialize()`

처리 순서
Engine 초기화
→ Network/공용 서비스 초기화
→ Font와 Static Prototype 준비
→ ImGui와 debug tool 기반 준비
→ 무조건 `Start_Level(LEVEL::LOBBY)`

명령행 scenario를 읽지 않는다.

`Update(deltaSeconds)`

권장 순서
ImGui BeginFrame
→ UI가 입력을 소비하는지 계산
→ NetworkManager Update
→ Engine `Update_Engine`
→ debug tool update
→ `Apply_LevelRequest`

가장 중요한 이유
`Apply_LevelRequest`는 반드시 현재 Level의 Update가 반환된 뒤 실행해야 한다.

`Render()`

Engine render
→ combat HUD
→ Developer Tools
→ ImGui EndFrame

Offline preview overlay와 smoke overlay는 호출하지 않는다.

MainApp private 함수

`Start_Level(LEVEL target)`

존재 이유
target resource를 준비할 Loading Level을 연다.

내부 흐름
target descriptor 존재 확인
→ `CLevel_Loading::Create(target)`
→ `Change_Level(LEVEL::LOADING, loading)`

`Apply_LevelRequest()`

존재 이유
실제 Level 파괴와 교체를 안전한 한 지점에서만 수행한다.

내부 흐름
request 하나 consume
→ LOAD면 `Start_Level(target)`
→ ACTIVATE면 Registry로 실제 Level 생성
→ `Change_Level(target, newLevel)`
→ 실패면 HRESULT 보존과 Lobby recovery 요청

불변식

Client에서 실제 `Change_Level`을 호출하는 곳은 MainApp뿐이다.
Loading도 직접 호출하지 않는다.
MainApp은 Server 승인 정책을 판단하지 않는다.
MainApp은 Character class를 선택하지 않는다.
MainApp은 Client smoke를 실행하지 않는다.

## 12. LobbyCommandService.h

수정 위치

`Client/Public/LobbyCommandService.h`
`Client/Private/LobbyCommandService.cpp`

파일이 존재하는 이유

ImGui나 다른 UI가 Lobby 객체를 직접 참조하지 않고 한 번의 Stage 선택 명령을 전달하게 한다.
UI는 명령을 제출하고 Lobby Level이 실제 Network와 Level 전환을 판단한다.

삭제할 command 필드

character slot index
entry mode
nickname
server host
server port

이번 단계에서 endpoint는 제품 실행 설정 `127.0.0.1:7777`로 고정한다.
닉네임과 실제 계정 캐릭터 슬롯이 필요해지면 Server 계약과 함께 별도 vertical slice로 추가한다.

새 command 자료구조

`LOBBY_STAGE`
TEST
CHARACTER_SELECT
VALTAN
BERN
END

`LOBBY_COMMAND`
선택한 `LOBBY_STAGE` 하나

외부에 공개할 함수

`Request(LOBBY_STAGE stage)`

내부 흐름
stage 유효성 검사
→ mutex 잠금
→ pending command 존재 검사
→ command 저장
→ status 갱신

`Try_Consume(LOBBY_COMMAND& outCommand)`

호출자
`CLevel_Lobby::Update`

내부 흐름
mutex 잠금
→ pending 없음이면 false
→ command 이동
→ pending 제거

`Get_Status()`
최근 명령 거부 원인을 ImGui에 보여준다.

자료구조

mutex 하나
optional pending command 하나
status string 하나

핵심 불변식

동시에 pending command는 하나뿐이다.
command는 실제 Level 전환을 수행하지 않는다.
command는 socket을 열지 않는다.
Local과 Multiplayer 선택을 포함하지 않는다.

## 13. Level_Lobby.h

수정 위치

`Client/Public/Level_Lobby.h`
`Client/Private/Level_Lobby.cpp`

파일이 존재하는 이유

Client의 시작 화면이며 네 Stage 진입 의도를 받는다.
Character Select에는 네트워크 없이 이동한다.
Test, Valtan, Bern에는 Server 승인을 받은 뒤 이동한다.

Lobby에서 삭제할 책임

Character 네 개를 직접 생성하는 preview slot
preview camera follow target 선택
Local Preview
Multiplayer mode selector
사용자 입력 server host와 port
scenario 변환
LevelCatalog 조회

필요한 헤더

`Client_Defines.h`
Level 기본 타입

`Level.h`
`CLevel` 상속

`LobbyCommandService.h`
command 구조를 멤버나 함수 시그니처에서 직접 사용한다면 필요하다.

`Network/PacketType.h`
`WORLD_ID`, enter accepted 패킷 타입, 선택 직업 ID

`<chrono>`
5초 승인 deadline

`CCamera_Free`와 `CCharacter` 전방 선언은 preview 삭제 후 더 이상 필요하지 않다.

외부에 공개할 함수

`Initialize()`
Lobby Light와 Camera 또는 배경 UI 준비
→ network pending state 초기화

`Update(deltaSeconds)`

처리 순서
Lobby command consume
→ Stage 종류에 따라 분기
→ Character Select면 Level load request
→ 제품 Stage면 Server 입장 시작
→ accepted packet consume
→ world ID 일치 확인
→ 성공하면 대응 Level load request
→ connection failure와 timeout 확인

`Render()`

Test 버튼
Character Select 버튼
Valtan 버튼
Bern 버튼
현재 확정 직업
network/transition status

Lobby 내부 함수

`Begin_StageRequest(LOBBY_STAGE stage)`

Character Select 흐름
pending network 상태가 없는지 확인
→ `Request_Load(LEVEL::CHARACTER_SELECT)`

Test, Valtan, Bern 흐름
확정 직업 존재 확인
→ stage를 `WORLD_ID`와 target `LEVEL`로 변환
→ 기존 연결 정리 여부 확인
→ `127.0.0.1:7777` 연결
→ `C2S_ENTER_WORLD` 전송
→ pending world와 target level 저장
→ deadline 현재 시각 + 5초
→ WAITING_FOR_APPROVAL

`Resolve_Stage(LOBBY_STAGE stage, WORLD_ID&, LEVEL&)`

매핑
TEST → TRAINING_GROUND → DEVELOPMENT
VALTAN → VALTAN_ARENA → VALTAN_ARENA
BERN → BERN → BERN

Character Select는 network world가 아니므로 이 함수에 넣지 않아도 된다.

`Consume_EnterAccepted()`

처리 순서
accepted 존재 확인
→ 현재 WAITING인지 확인
→ accepted world가 pending world와 같은지 확인
→ NetworkManager가 player/entity ID를 정상 등록했는지 확인
→ target Level load request

실패
예상하지 않은 accepted면 연결을 닫고 Lobby에 남는다.
world가 다르면 연결을 닫고 pending state를 초기화한다.

`Cancel_PendingEntry(reason)`

존재 이유
연결 실패, packet 거부, timeout을 한 순서로 정리한다.

처리 순서
NetworkManager close
→ pending world 초기화
→ pending target 초기화
→ state IDLE
→ deadline 초기화
→ reason 보존

Lobby 상태 자료구조

진입 상태
IDLE
WAITING_FOR_APPROVAL

pending WORLD_ID
pending LEVEL
approval deadline
status string

왜 bool 두 개보다 enum state가 나은가

`m_isEnterRequested`와 `m_isAwaitingEnterAcceptance` 두 bool은 둘 다 true이거나 둘 다 false인 조합의 의미가 불분명하다.
enum 하나는 허용 상태를 제한하고 switch에서 누락을 찾기 쉽다.

핵심 불변식

WAITING이면 pending world와 level이 모두 유효하다.
IDLE이면 pending world와 level은 END다.
승인 전에는 제품 Level load request가 없다.
실패 시 Lobby에 남는다.
실패 시 Local Preview로 우회하지 않는다.
Character Select 진입은 socket을 열지 않는다.

## 14. Level_Loading.h

수정 위치

`Client/Public/Level_Loading.h`
`Client/Private/Level_Loading.cpp`

파일이 존재하는 이유

대상 Level resource가 준비되는 동안 현재 화면을 유지하고 Loader의 성공 또는 실패를 관찰한다.
Level 생성과 resource load를 한 프레임에 섞지 않는다.

수정할 함수

`Update(deltaSeconds)`

현재 문제
Loader 성공 후 Loading 자신의 Update 안에서 `Change_Level`을 직접 호출한다.
이것도 실행 중인 Loading 객체를 파괴할 수 있는 같은 수명 문제다.

변경 흐름
Loader 미완료면 반환
→ 실패면 `Recover_FromFailure(result)`
→ 성공이면 target descriptor 확인
→ `CLevelTransitionService::Request_Activation(target)`
→ activation request가 받아들여지면 중복 요청 차단 상태 기록

`Recover_FromFailure(HRESULT result)`

처리 순서
원래 HRESULT 보존
→ 제품 Stage 연결이 열려 있으면 NetworkManager close
→ 대상 Level partial resource rollback 확인
→ Lobby가 target이 아니면 `Request_Load(LEVEL::LOBBY)`
→ Lobby load 자체 실패면 사용자에게 오류 화면과 retry 제공

`Retry_LobbyLoad()`

직접 `Change_Level`하지 않는다.
`Request_Load(LEVEL::LOBBY)`만 제출한다.

멤버 상태

대상 LEVEL
Loader unique_ptr
activation request 제출 여부
failure 보고 여부
retry 요청 여부

핵심 불변식

Loader 성공 전 ACTIVATE 요청을 보내지 않는다.
ACTIVATE 요청은 한 번만 보낸다.
Loading은 실제 Level을 직접 교체하지 않는다.
실패 HRESULT를 보존한다.

## 15. Bern, Valtan, Development Level

수정 위치

`Client/Public/Level_Bern.h`
`Client/Private/Level_Bern.cpp`
`Client/Public/Level_ValtanArena.h`
`Client/Private/Level_ValtanArena.cpp`
`Client/Public/Level_Development.h`
`Client/Private/Level_Development.cpp`

파일이 존재하는 이유

각 실제 Stage의 runtime object, camera, map placement, replication presentation을 소유한다.
Server가 승인하고 보낸 상태를 화면에 표시한다.

삭제할 의존성

`ClientLaunchOptions`
`LevelCatalog`
`OfflinePlayerPreview`
Local Preview 분기
scenario 기반 tool 분기

공통 Initialize 흐름

Registry descriptor에서 map area와 load scope 확인
→ map placement 생성
→ gameplay layer 준비
→ `CClientReplication` 초기화
→ network command sink 연결
→ Server snapshot presentation 시작

중요한 불변식

제품 Level의 local Character는 Server의 Spawn packet을 통해서만 생긴다.
선택 직업을 근거로 Client가 임의 Player ID나 NetEntityId를 만들지 않는다.
disconnect 시 replicated state를 먼저 정리한 뒤 Lobby 전환을 요청한다.

Development의 의미

Lobby의 Test 버튼이 진입하는 최소 수련장이다.
`dev.training.ground -> LEVEL::DEVELOPMENT -> LV_DEV_TRAINING_GROUND -> WORLD_ID::TRAINING_GROUND`를 유지한다.
Map, Character, Effect, UI, HDR scenario를 하나의 enum으로 선택하지 않는다.
F1 Developer Tools에서 필요한 도구를 명시적으로 열어 테스트한다.

## 16. ClientReplication.h

수정 위치

`Client/Public/ClientReplication.h`
`Client/Private/ClientReplication.cpp`

파일이 존재하는 이유

Server의 player/entity spawn, despawn, snapshot을 Client GameObject presentation으로 변환한다.
Server entity ID와 Client object handle의 대응을 소유한다.

삭제할 구조와 함수

`LOCAL_PREVIEW_PLAYER_DESC`
`Spawn_LocalPreview`
`Matches_LocalPreviewCharacter`
`Get_LocalPreviewPlacementId`
`m_LocalPreviewCharacter`
`m_strLocalPreviewPlacementId`

수정할 `Get_LocalCharacter()`

Network registry의 local character handle만 resolve한다.
없으면 nullptr을 반환한다.
offline preview pointer로 fallback하지 않는다.

수정할 `Reset_World()`

Network registry object 제거
→ world entity 제거
→ local handle 초기화
→ server tick 초기화

Local Preview 별도 제거 단계는 사라진다.

핵심 불변식

NetEntityId가 없는 Character를 replicated player로 등록하지 않는다.
local player 판정은 Server가 승인한 local NetEntityId를 기준으로 한다.
한 NetEntityId는 한 registry record만 가진다.
disconnect 후 이전 object handle은 resolve되지 않는다.

## 17. Debug Tool과 Effect Tool

수정 위치

`Client/Public/MainApp.h`
`Client/Private/MainApp.cpp`
`Client/Public/Effect_Tool.h`
`Client/Private/Effect_Tool.cpp`

Debug Tool이 존재하는 이유

Map, Animation, Effect, UI, profiler를 제작자가 실제 runtime 데이터 위에서 검사하기 위해 존재한다.
Level을 고르는 두 번째 메뉴가 아니다.

수정 방향

F1은 Developer Tools 창을 열고 닫는다.
Developer Tools는 Test 즉 Development에서만 편집 도구를 활성화한다.
도구 버튼은 `EnsureDebugTool(DEBUG_TOOL tool)`에 명시적 tool enum을 전달한다.
`CLIENT_SCENARIO`나 LevelCatalog의 `Tools` 문자열을 읽지 않는다.

금지

F2부터 F5 또는 F7부터 F12로 Level 전환
Developer Tools에서 Lobby, Bern, Valtan을 직접 전환
도구를 열기 위해 Client를 특정 scenario로 재실행

Effect Tool 수정

명령행 Effect asset 자동 open 제거
HDR profile 자동 시작 제거
자동 종료와 smoke report 제거

HDR 검증 기능 자체가 유용하다면 Effect Tool 안의 명시적 버튼으로 남긴다.
사용자가 눌렀을 때만 capture하고 결과를 UI에 표시한다.

Character Select와 툴의 후속 연결

첫 단계에서는 Character Select가 직업 preview만 소유한다.
Animation, Effect, physics 편집을 붙일 때 MainApp의 범용 도구가 선택 Character를 찾기 위해 Level tag나 vector index를 추측하면 안 된다.
`CAnimationTargetService`처럼 이미 존재하는 public target 계약으로 active preview를 등록한다.
후속 수직 슬라이스마다 저장 ID, 적용 대상, 실패 rollback을 따로 닫는다.

## 18. Visual Studio Server와 Client 동시 실행

추가 위치

`Framework.slnLaunch`

파일이 존재하는 이유

Client 내부 smoke 대신 실제 Server와 Client를 같은 디버깅 세션에서 실행하기 위해 존재한다.
개발자가 한 번의 실행으로 Server listener와 Lobby Client를 함께 띄운다.

프로젝트 순서

Server 먼저 Start
Client 다음 Start

Server 인자
기본 listener가 `127.0.0.1`이면 추가 인자 없음
명시해야 한다면 `--bind-address 127.0.0.1`

Client 인자
없음

Client 작업 디렉터리
`Client/Default`

중요한 경계

개인 LAN IP를 파일에 저장하지 않는다.
Client가 Server 준비를 sleep으로 추측하지 않는다.
연결 실패 시 Lobby에 원인을 보여주고 다시 시도할 수 있어야 한다.
Visual Studio 공유 multi-project launch profile은 사용하는 VS 버전 지원 여부를 실제 환경에서 확인한다.

## 19. BuildAndRegression과 ProjectAudit

수정 위치

`Tools/Build/Invoke-BuildAndRegression.ps1`
`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

BuildAndRegression이 존재하는 이유

정본 build 순서와 기존 독립 실행형 contract 검증을 한 명령으로 반복한다.
Client runtime 안에 테스트 분기를 심지 않는다.

유지할 검증

Engine Debug/Release
UpdateLib Debug/Release
Shared와 NetworkProtocolHarness Debug/Release
NetworkProtocolHarness 실행
Server Debug/Release
`Server.exe --contract-test`
Client Debug/Release
ProjectAudit

삭제할 검증

Client `--smoke`
Client `--scenario`
offline Client smoke script
network endpoint Client smoke script
MainApp 자동 종료 report 검사

ProjectAudit에 추가할 규칙

`ClientLaunchOptions.h/.cpp`가 존재하지 않는다.
`LevelCatalog.h/.cpp`와 `Data/Levels/LevelCatalog.json`이 존재하지 않는다.
`OfflinePlayerPreview.h/.cpp`가 존재하지 않는다.
Client source에 `CLIENT_SCENARIO`가 없다.
Client source에 `LOCAL_PREVIEW`가 없다.
Client source에 `UpdateSmokeHarness`가 없다.
Client source에 `CompleteSmokeHarness`가 없다.
Client source에 `RenderOfflinePreviewOverlay`가 없다.
`Change_Level` 호출은 MainApp의 허용 위치에만 있다.
Registry에 Character Select가 정확히 한 번 등록된다.
Character Select H와 CPP가 project와 filter에 등록돼 있다.

왜 Client harness를 새로 만들지 않는가

이번 목표는 MainApp과 실제 runtime에서 테스트 경계를 제거하는 것이다.
삭제한 smoke를 이름만 바꿔 다른 Client harness로 되살리면 같은 문제가 반복된다.
Protocol serialization처럼 process 밖에서 독립적으로 검증할 수 있는 기존 harness는 유지한다.
실제 Level 진입은 Server와 Client를 함께 실행해 검증한다.

## 20. 프로젝트와 filters 수정

수정 위치

`Client/Default/Client.vcxproj`
`Client/Default/Client.vcxproj.filters`

추가 확인

`Level_CharacterSelect.h/.cpp` 항목이 정확히 한 번 존재하는지 확인한다.
필터는 `01.Levels\01. CharacterSelect`를 유지한다.
`CharacterSelectionState.h/.cpp`를 같은 필터에 추가한다.
`LevelTransitionService.h/.cpp`를 `01.Levels`에 등록한다.

삭제할 항목

`ClientLaunchOptions.h/.cpp`
`LevelCatalog.h/.cpp`
`OfflinePlayerPreview.h/.cpp`
옛 `SceneTransitionService.h/.cpp`
`Data/Levels/LevelCatalog.json`의 `None` 항목

주의

기존 필터를 재배치하지 않는다.
관련 없는 파일의 filter를 정리하지 않는다.
새 include directory나 project reference를 추가하지 않는다.

## 21. 구현 순서

1단계
`Client_Defines.h`에서 Level 집합을 확정한다.
`CLIENT_SCENARIO`를 소비하는 위치를 `rg`로 전부 찾는다.
아직 파일을 삭제하지 않고 소비자 목록을 만든다.

2단계
`CharacterSelectionState.h/.cpp`를 작성한다.
지원 class 검증과 선택 없음 불변식을 먼저 닫는다.

3단계
SceneTransitionService를 LevelTransitionService로 바꾼다.
request에서 scenario를 제거한다.
LOAD와 ACTIVATE 단계만 남긴다.

4단계
LevelRegistry에 Character Select를 등록한다.
Catalog가 제공하던 제품 map의 고정 정보 중 실제 필요한 것만 descriptor로 옮긴다.

5단계
Loader에서 launch option과 catalog 의존성을 제거한다.
Character Select bundle과 선택 직업 인자 전달을 완성한다.

6단계
`Level_CharacterSelect.h/.cpp`를 작성한다.
Camera, Light, 네 직업 preview, Confirm, Back만 먼저 닫는다.
외형·physics·Effect 편집은 아직 넣지 않는다.

7단계
Lobby에서 preview와 Local/Multiplayer UI를 제거한다.
네 Stage 버튼과 Server 승인 상태 머신만 남긴다.

8단계
Loading과 MainApp의 모든 `Change_Level`을 안전한 MainApp 경계로 모은다.
이 단계가 끝나면 `rg "Change_Level" Client`로 호출 위치를 확인한다.

9단계
Bern, Valtan, Development와 ClientReplication에서 Local Preview를 제거한다.
실제 Server spawn만 남긴다.

10단계
Debug Tool과 Effect Tool에서 scenario 자동 실행을 제거한다.

11단계
삭제 대상 파일과 project/filter 항목을 제거한다.
삭제 전 `rg`로 include와 symbol 소비자가 0인지 확인한다.

12단계
`Framework.slnLaunch`, BuildAndRegression, ProjectAudit을 수정한다.

13단계
빌드와 실제 Server + Client 수동 검증을 수행한다.
검증 결과를 대응 RESULT 문서에만 기록한다.

## 22. 대표 실패 상황과 원인 찾기

Character Select 버튼을 눌러도 이동하지 않는다.

확인 순서
Lobby button이 command를 제출했는가
→ Lobby Update가 command를 consume했는가
→ LevelTransitionService에 LOAD request가 생겼는가
→ MainApp이 Update_Engine 뒤 request를 consume했는가
→ Start_Level이 Registry descriptor를 찾았는가

Character Select Loading이 실패한다.

확인 순서
Registry의 load function이 `Ready_For_CharacterSelect`인가
→ Camera Prototype 등록 성공 여부
→ animated shader 등록 성공 여부
→ 네 직업 Character spec 존재 여부
→ 중복 Prototype tag 여부
→ rollback 후 해당 Level resource가 남아 있지 않은가

직업을 바꾸면 화면이 비어 버린다.

원인 후보
기존 preview를 먼저 제거했다.
새 preview clone 실패 이유를 버렸다.
Layer에서 기존 객체를 제거한 뒤 rollback하지 않았다.

해결 기준
새 preview를 staging하고 성공 후 교체한다.

Confirm 후 Lobby에서 선택이 없다.

확인 순서
Confirm이 임시 index만 바꾸고 state commit을 호출하지 않았는가
→ state가 Level 멤버로 들어가 파괴됐는가
→ Back과 Confirm이 같은 함수를 호출하는가
→ unsupported enum 검증에서 거부됐는가

Bern 버튼을 누르자 Server 없이 Level이 열린다.

원인
Lobby가 승인 전에 transition request를 제출했다.
옛 Local Preview 분기가 남았다.
MainApp smoke가 자동 transition을 제출했다.

Audit 기준
승인 packet을 소비한 분기에서만 제품 target LOAD가 발생해야 한다.

Loading 전환 중 crash가 난다.

원인 후보
Level Update 안에서 `Change_Level`을 직접 호출했다.
Loading Update 안에서 자신을 파괴했다.
ACTIVATE request를 매 프레임 중복 제출했다.

확인
`rg -n "Change_Level" Client`
MainApp 허용 위치 외 호출이 있으면 실패다.

접속 실패 후 예전 Character가 남는다.

원인
disconnect recovery가 registry와 world entity를 reset하지 않았다.
Local Preview fallback pointer가 남았다.

해결 기준
`CClientReplication::Reset_World` 후 local handle resolve가 nullptr여야 한다.

## 23. 디버거에서 확인할 호출 순서

Character Select 진입 breakpoint

`CLobbyCommandService::Request`
→ `CLobbyCommandService::Try_Consume`
→ `CLevel_Lobby::Begin_StageRequest`
→ `CLevelTransitionService::Request_Load`
→ `CMainApp::Apply_LevelRequest`
→ `CMainApp::Start_Level`
→ `CLoader::Ready_For_CharacterSelect`
→ `CLevelTransitionService::Request_Activation`
→ `CMainApp::Apply_LevelRequest`
→ `CLevelRegistry::Create_Level`
→ `CLevel_CharacterSelect::Initialize`

직업 확정 breakpoint

`CLevel_CharacterSelect::Confirm_Selection`
→ `CCharacterSelectionState::Select`
→ `CLevelTransitionService::Request_Load`

Bern 진입 breakpoint

`CLevel_Lobby::Begin_StageRequest`
→ `CNetworkManager::Connect`
→ enter packet send
→ `Try_Consume_EnterAccepted`
→ world ID 검증
→ `Request_Load(LEVEL::BERN)`

각 breakpoint에서 볼 값

target LEVEL
pending WORLD_ID
selected CHARACTER_CLASS_ID
transition phase
Loader HRESULT
current Level ID
Network connected 여부
local NetEntityId

## 24. 작은 요구사항 변경 연습

Lobby 버튼 순서를 바꾸고 싶다.

수정 대상
`CLevel_Lobby::Render`

수정하지 않을 대상
Registry
Loader
Network protocol

이유
화면 순서만 바뀌고 stage 의미는 그대로다.

승인 timeout을 5초에서 8초로 바꾸고 싶다.

수정 대상
Lobby의 timeout 상수 또는 duration

확인할 불변식
deadline은 request를 보낸 시점에 한 번 계산한다.
매 프레임 현재 시각에 8초를 다시 더하면 timeout이 영원히 오지 않는다.

지원 직업을 하나 추가하고 싶다.

수정 대상
Shared class ID와 protocol 지원 여부
CharacterCatalog spec
Character asset과 Prototype
Character Select 지원 목록
Loader 준비 목록
Server enter validation

주의
array 크기만 늘리고 Server validation을 빼면 Client에서 보이지만 입장은 거부된다.

Character preview를 마우스로 회전하고 싶다.

수정 대상
Character Select의 preview transform 입력 처리

수정하지 않을 대상
CharacterSelectionState
NetworkManager
Server packet

이유
회전은 편집 화면 presentation 상태이지 선택 저장 계약이 아니다.

외형 색상을 저장하고 싶다.

먼저 결정할 것
로컬 preview 전용인가
실제 계정 캐릭터 데이터인가

로컬 preview 전용이면 Character Select의 임시 appearance state와 render parameter만 바꾼다.
실제 게임에 반영한다면 stable appearance payload, Server validation, 저장, spawn snapshot, Client presentation을 한 수직 슬라이스로 추가한다.

## 25. 주석 작성 규칙

주석은 한글로 작성한다.
문자 인코딩은 기존 C++ 파일의 인코딩을 감지해 유지한다.
새 C++ 파일은 UTF-8 BOM 없음이다.

좋은 주석

Level Update 안에서 현재 Level을 교체하면 실행 중인 객체가 파괴되므로 실제 교체는 MainApp 프레임 경계에서 수행한다.

새 preview 생성이 실패해도 기존 화면을 유지하기 위해 성공한 객체만 active preview로 교체한다.

승인받지 않은 world로 전환하지 않도록 pending world와 accepted world가 같은 경우에만 Level load를 요청한다.

피할 주석

캐릭터를 선택한다.
함수 이름을 반복한다.

일단 임시로 처리한다.
이유와 제거 조건이 없다.

서버 승인 메시지를 받음.
어떤 검증 뒤 어떤 상태가 변하는지 설명하지 않는다.

## 26. 완료 조건

구현 완료

Client가 항상 Lobby에서 시작한다.
Lobby에 Test, Character Select, Valtan, Bern 네 버튼이 있다.
Character Select가 별도 Level과 Loader bundle으로 열린다.
네 직업 preview와 Confirm, Back이 동작한다.
확정한 class ID가 Lobby 복귀 후 유지된다.
Test, Valtan, Bern은 Server 승인 후에만 열린다.
제품 Level에 Local Preview 경로가 없다.
ClientLaunchOptions와 LevelCatalog가 삭제됐다.
MainApp smoke harness와 offline overlay가 삭제됐다.
MainApp만 실제 `Change_Level`을 호출한다.
F1 Developer Tools는 Development에서 명시적으로 도구를 연다.
Server와 Client 동시 실행 profile이 있다.

자동 검증 완료

JSON parse 성공
VCXPROJ와 filters XML parse 성공
Engine Debug/Release 성공
UpdateLib Debug/Release 성공
Shared와 NetworkProtocolHarness Debug/Release 성공
NetworkProtocolHarness 실행 성공
Server Debug/Release 성공
Server contract test 성공
Client Debug/Release 성공
ProjectAudit 성공
`git diff --check` 성공

수동 검증 완료

Server와 Client 동시 실행
Lobby 표시
Character Select 왕복
네 직업 preview 교체
Confirm 선택 유지
Server 없는 제품 Stage 진입 거부
Server 실행 후 Test 진입
Server 실행 후 Bern 진입
Server 실행 후 Valtan 진입
disconnect 시 Lobby 복귀와 replicated state 정리
F1 도구 열기와 각 도구 명시적 활성화

## 27. 이번 계획에 포함하지 않는 것

외형 커스터마이징의 실제 Server 저장
장비 inventory 계약
PhysX bone branch와 spherical joint 구현
cloth solver 구현
Effect socket authoring 전체 구현
PBR, SSAO, outline, fog render target 재구성
frustum, occlusion, distance culling과 LOD 구현
AI Behavior Tree editor 구현
MapTool 전체 개편

이 기능들은 Character Select Level이 실제 preview와 선택 상태까지 닫힌 뒤 각각 별도 vertical slice로 계획한다.
같은 Level을 작업 무대로 사용할 수는 있지만, 아직 소비자가 없는 interface와 placeholder Manager를 먼저 만들지 않는다.

## 28. 구현 전에 스스로 답할 문장

`CLevel_CharacterSelect`가 필요한 이유를 Lobby panel과 비교해 설명할 수 있다.
`CharacterSelectionState`가 Catalog나 NetworkManager가 아닌 이유를 설명할 수 있다.
입력부터 실제 Level 활성화까지 호출 흐름을 그릴 수 있다.
Loader와 Level Initialize의 책임 차이를 설명할 수 있다.
MainApp만 `Change_Level`을 호출해야 하는 수명 이유를 설명할 수 있다.
Lobby의 WAITING 상태에서 유효해야 하는 변수와 불변식을 설명할 수 있다.
preview clone 실패와 Server 승인 timeout을 재현하고 원인을 찾을 수 있다.
버튼 순서, timeout, 직업 추가 같은 작은 변경의 수정 범위를 스스로 고를 수 있다.
