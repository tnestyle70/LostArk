# Character Select와 LEVEL 단일 전환 구조 구현 결과

작성일
2026-08-03

대응 계획서
`.md/GB/08-03/2026-08-03_CHARACTER_SELECT_LEVEL_VERTICAL_SLICE_PLAN.md`

현재 정본 주의: 아래 1~10절은 2026-08-03 최초 4-class/Confirm 구현 기록이다. 2026-08-04의 5-class map/Enter-to-Test 확장은 맨 아래 11절이 현재 상태를 대체한다.

## 1. 결과

Character Select를 별도 `LEVEL`로 추가하고 Lobby에서 실제로 진입할 수 있게 연결했다.

Client는 항상 Lobby에서 시작한다.
Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 버튼만 제공한다.
Character Select는 socket 없이 네 playable class의 3D preview, 전용 카메라, 조명, Confirm과 Back을 소유한다.
Test, Valtan, Bern은 확정된 class로 실제 Server에 입장을 요청하고 `S2C_ENTER_ACCEPTED`를 받은 뒤에만 목표 Level을 로드한다.

Client 실행 인자, LevelCatalog, Local Preview, MainApp 내부 smoke harness는 삭제했다.
`Change_Level`은 현재 Level의 update가 끝난 뒤 `CMainApp`만 호출한다.

## 2. 새 파일이 존재하는 이유

### `Client/Public/CharacterSelectionState.h`
### `Client/Private/CharacterSelectionState.cpp`

Lobby와 Character Select보다 오래 살아야 하지만 NetworkManager에는 속하지 않는 선택 class 하나를 보존한다.

자료구조는 `std::optional<CHARACTER_CLASS_ID>`다.
선택 전 상태와 선택 후 상태를 구분해야 하므로 임의의 기본 class나 `END` sentinel을 정상값처럼 저장하지 않는다.
접근은 mutex로 보호하고 playable class 검증을 통과한 값만 commit한다.

### `Client/Public/LevelTransitionService.h`
### `Client/Private/LevelTransitionService.cpp`

실행 중인 Level이 자신의 `Update` 안에서 곧바로 자신을 파괴하지 않게 전환 명령을 MainApp frame 경계까지 보존한다.

자료구조는 `std::optional<LEVEL_TRANSITION_REQUEST>` 하나다.
동시에 두 Level 전환을 진행하지 않는 불변식을 자료구조 자체로 표현한다.
요청은 `LOAD`와 `ACTIVATE` 두 phase만 가진다.
시나리오, 툴 이름, Client 실행 옵션은 소유하지 않는다.

### `Client/Public/Level_CharacterSelect.h`
### `Client/Private/Level_CharacterSelect.cpp`

Character Select만의 카메라, 조명, preview Character, ImGui 입력과 Level 수명을 소유한다.
Lance Master, Gunslinger, Slayer, Artist 네 class를 고정 배열로 순회한다.

새 preview는 먼저 staging clone을 만들고 Transform과 idle animation까지 확인한 뒤 active preview로 교체한다.
중간 생성이 실패하면 staging object만 제거하고 기존 preview를 유지한다.
Confirm은 `CCharacterSelectionState`에 class를 확정한 뒤 Lobby load를 요청한다.
Back은 선택값을 바꾸지 않고 Lobby load만 요청한다.

### `Framework.slnLaunch`

Client 내부 검증 분기 대신 Visual Studio에서 실제 Server와 Client를 함께 실행하기 위해 존재한다.
`Server + Client` profile은 Server project를 먼저, Client project를 다음 순서로 `Start`한다.

## 3. 수정된 책임 경계

### `CLevelRegistry`

Lobby, Character Select, Bern, Valtan, Development 다섯 실제 목적지를 Level 생성 함수와 Loader 함수에 연결한다.
제품 맵의 area ID와 `MAP_LOAD_SCOPE`도 같은 descriptor에서 Loader와 Level이 함께 읽는다.

### `CLoader`

Character Select 진입에서는 전용 카메라와 네 playable class의 rendering prototype을 준비한다.
Bern, Valtan, Development에서는 Server가 승인한 local character class를 사용한다.
Lobby는 별도 preview나 camera resource를 준비하지 않는다.

### `CLevel_Lobby`

네 stage 명령과 Server 입장 승인 대기만 소유한다.
Character Select는 network 없이 Level load를 요청한다.
Test는 `WORLD_ID::TRAINING_GROUND`와 `LEVEL::DEVELOPMENT`로 연결한다.
Valtan과 Bern은 각 world ID와 Level로 연결한다.

제품 stage는 선택 class가 없으면 거부한다.
기본 endpoint `127.0.0.1:7777`에 연결하고 `C2S_ENTER_WORLD`를 보낸다.
5초 안에 승인이 없거나 다른 world를 승인하면 연결을 닫고 Lobby에 남는다.

### `CLevel_Loading`

Loader의 성공과 실패를 관찰한다.
성공하면 `ACTIVATE` 요청만 제출한다.
실패하면 target Level의 부분 resource를 정리하고 network session을 닫은 뒤 Lobby load를 요청한다.
Lobby 자체 로드가 실패하면 기존 상태를 숨기지 않고 Retry UI를 표시한다.

### `CMainApp`

게임 loop, ImGui frame, network update와 Level 전환 frame 경계만 소유한다.
Client launch option parsing, offline overlay, smoke update와 자동 종료 코드는 제거했다.

`CLevelTransitionService`의 `LOAD`를 소비하면 Loading Level을 만든다.
`ACTIVATE`를 소비하면 Registry의 생성 함수로 목표 Level을 만든 뒤 `Change_Level`을 호출한다.
현재 Client source의 `Change_Level` 두 호출은 모두 `CMainApp`에만 있다.

### Bern, Valtan, Development와 ClientReplication

Local Preview 분기를 삭제했다.
제품 Level은 network command sink와 replicated local Character만 사용한다.
연결이 끊기면 replicated state를 정리하고 Lobby load를 요청한다.

## 4. 삭제한 레거시

다음 파일을 삭제했다.

- `Client/Public/ClientLaunchOptions.h`
- `Client/Private/ClientLaunchOptions.cpp`
- `Client/Public/LevelCatalog.h`
- `Client/Private/LevelCatalog.cpp`
- `Client/Public/OfflinePlayerPreview.h`
- `Client/Private/OfflinePlayerPreview.cpp`
- `Client/Public/SceneTransitionService.h`
- `Client/Private/SceneTransitionService.cpp`
- `Data/Levels/LevelCatalog.json`
- `Tools/Build/Invoke-OfflineClientSmoke.ps1`
- `Tools/Build/Invoke-NetworkEndpointSmoke.ps1`

`CMainApp::RenderOfflinePreviewOverlay`, `Apply_PendingSceneTransition`, `UpdateSmokeHarness`, `CompleteSmokeHarness`와 관련 상태도 제거했다.

## 5. 호출 흐름

```text
Client 시작
-> CMainApp::Initialize
-> CMainApp::Start_Level(LOBBY)
-> CLevel_Loading
-> CLoader::Execute_Load(LOBBY)
-> CLevelTransitionService::Request_Activation(LOBBY)
-> CMainApp::Apply_LevelRequest
-> CLevelRegistry::Create_Level(LOBBY)
-> CGameInstance::Change_Level
```

```text
Lobby Character Select 버튼
-> CLobbyCommandService::Request(CHARACTER_SELECT)
-> CLevel_Lobby::Consume_Command
-> CLevelTransitionService::Request_Load(CHARACTER_SELECT)
-> CMainApp::Apply_LevelRequest
-> CLevel_Loading
-> CLoader::Ready_For_CharacterSelect
-> 네 class rendering prototype 준비
-> CLevelTransitionService::Request_Activation
-> CMainApp::Apply_LevelRequest
-> CLevel_CharacterSelect 생성
```

```text
Character Select Confirm
-> CCharacterSelectionState::Select
-> CLevelTransitionService::Request_Load(LOBBY)
-> Lobby 복귀
-> Test / Valtan / Bern 버튼
-> CNetworkManager::Connect_To_Server(127.0.0.1, 7777)
-> C2S_ENTER_WORLD(selected class)
-> S2C_ENTER_ACCEPTED
-> 목표 Level load
-> replicated Character와 network command sink 사용
```

## 6. 프로젝트 등록

`Client/Default/Client.vcxproj`와 `Client.vcxproj.filters`에 다음 파일을 등록했다.

- `CharacterSelectionState.h/.cpp`
- `LevelTransitionService.h/.cpp`
- `Level_CharacterSelect.h/.cpp`

Character Select 관련 파일은 `01.Levels/01. CharacterSelect` filter에 배치했다.
삭제한 legacy source, header와 `LevelCatalog.json` project item도 제거했다.
두 XML은 parse에 성공했다.

## 7. 자동 검증 결과

성공한 검증

- Client Debug x64 build와 link 성공
- Server Debug x64 build 성공
- Engine, UpdateLib, Shared, NetworkProtocolHarness, Server, Client Release x64 build 성공
- NetworkProtocolHarness Debug 실행 `failures : 0`
- NetworkProtocolHarness Release 실행 `failures : 0`
- `Server.exe --contract-test` Debug 실행 `failures : 0`
- `Server.exe --contract-test` Release 실행 `failures : 0`
- Debug 실제 Server와 Client 동시 기동, Server `127.0.0.1:7777` listener와 두 process 생존 확인
- Release 실제 Server와 Client 동시 기동, Server `127.0.0.1:7777` listener와 두 process 생존 확인
- Client source의 legacy runtime symbol 0개 확인
- Client source의 `Change_Level` 호출 두 개가 모두 MainApp에만 존재함을 확인
- VCXPROJ, filters, JSON, PowerShell script parse 성공
- `git diff --check` 성공

기존 환경 경고

- 일부 기존 Client/Shared 파일의 UTF-8과 CP949 혼재로 `C4819`가 출력된다.
- 기존 third-party DirectXTK PDB가 없어 `LNK4099`가 출력된다.
- Visual Studio 전역 vcpkg target이 `pwsh.exe`를 먼저 찾는 문구가 있지만 Windows PowerShell fallback 뒤 build exit code는 0이다.

## 8. ProjectAudit에서 남은 한 건

새 runtime 경계와 harness 경계 검사는 모두 통과했다.
전체 ProjectAudit는 `asset-lock.inventory` 한 건 때문에 실패한다.

Release 검증 종료 시 Resources 실제 상태는 다음과 같다.

```text
manifest files: 7922
actual files: 9180
extra files: 1258
missing manifest files: 0
size-mismatched manifest files: 0
```

extra file은 두 디렉터리에만 있다.

```text
Map/LV_LOBBY_CLASSSELECT_SL00: 215 files
Map/LV_SHS_RCARENA_D: 1043 files
```

두 payload와 대응 mapassets/mapplacements, authoring 파일은 구현 도중 별도 작업에서 생성됐다.
현재 immutable resource manifest에는 포함되지 않는다.
사용자 자산을 임의 삭제하거나 검증을 약화하지 않았고, manifest에도 근거 없이 편입하지 않았다.
이 두 Area를 정식 payload로 채택하려면 runtime 사용 범위와 placement를 검증한 뒤 새 immutable resource pack version, manifest, SHA-256과 lock을 함께 갱신해야 한다.

## 9. 수동 검증 상태

실제 Server와 Client의 동시 시작과 process 생존까지 확인했다.
다음 UI 조작은 자동 클릭이나 Client 내부 harness를 다시 만들지 않았으므로 아직 수동 확인 전이다.

- Lobby 네 버튼 표시
- Character Select 진입
- 네 class preview 교체
- Confirm 후 선택 유지
- Test, Bern, Valtan 실제 진입 화면
- 제품 Level에서 Server 종료 후 Lobby 복귀 화면
- F1 Developer Tools의 각 도구 명시적 생성

## 10. 현재 완료 경계

Character Select Level, ImGui, class preview, Lobby 복귀, 실제 Server 승인 입장, 단일 Level 전환 경계와 legacy 삭제는 코드와 빌드 기준으로 완료됐다.

외형 appearance payload, 장비 inventory, PhysX bone branch와 joint solver, Effect socket authoring, render target debugger, PBR/SSAO/outline/fog, AI/Map/Balance tool 전체 구현은 이번 변경에 placeholder로 추가하지 않았다.
이 기능들은 Character Select를 실제 소비 Level로 사용하되 각각 Data, runtime, authoring UI, save/load, 실패 rollback과 검증을 닫는 별도 수직 슬라이스로 진행한다.

## 11. 2026-08-04 5-class map/Enter-to-Test 확장

현재 구현은 최초 결과의 네 class, 빈 배경, Confirm 후 Lobby 복귀 계약을 다음으로 대체한다.

```text
Lobby -> Character Select
-> Loader가 LV_LOBBY_CLASSSELECT_SL00 map과 다섯 class prototype stage
-> Level이 같은 scope의 803-placement map을 commit
-> Lance Master / Gunslinger / Slayer / Artist / Dimensionist preview
-> Enter가 선택 class와 tokenized TEST command를 commit
-> Lobby load/activation
-> Lobby가 command를 한 번 소비
-> C2S_ENTER_WORLD(TRAINING_GROUND, selected class)
-> S2C_ENTER_ACCEPTED
-> DEVELOPMENT/Test 진입
```

Character Select 자체는 socket을 열지 않는다. handoff token은 command를 예약한 정확한 load/activation 실패만 취소하므로 이전 실패가 새 command를 지우지 않는다. Character Select를 건너뛴 Test/Bern/Valtan은 Lance Master를 명시적인 기본 선택으로 commit하고 같은 Server 승인 경로를 사용한다.

map 조명은 재진입 때 직접 추가하지 않고 `CMapPlacementRuntime::Ensure_DefaultLight()`를 사용한다. 이 수정으로 Character Select 재진입 때 전역 light가 누적되지 않는다.

스킬 완료 범위는 다섯 class가 아니다. 기존 네 class Q/W만 다음 ID로 Server catalog와 contract test가 닫혔다.

| class | Q | W |
|---|---:|---:|
| Lance Master | 34120 | 34080 |
| Gunslinger | 38020 | 38050 |
| Slayer | 45050 | 45060 |
| Artist | 31210 | 31230 |

Dimensionist는 선택, profile, network spawn, HUD class identity, IDLE/RUN과 Animation Tool 경로까지다. reference 문서가 0-row라서 Q/W를 임의로 만들지 않았다.

현재 변경 기준 자동 검증:

- JSON, VCXPROJ/filters XML, PowerShell parse 성공
- gameplay balance Validate: player profiles 5, skills 15, damage profiles 16, bosses 1
- Debug/Release Engine -> UpdateLib -> Shared -> Server -> Client build 성공
- Debug/Release NetworkProtocolHarness `failures : 0`
- Debug/Release ClientFrontendHarness `failures : 0`
- Debug/Release `Server.exe --contract-test` `failures : 0`
- ProjectAudit의 신규 Character Select, five-class, Dimensionist, Q/W, effect admission check 통과
- Debug/Release ProjectAudit 각 64개 중 63개 통과. 실패는 `asset-lock.inventory` 한 건: lock `.3` 7,922 files, 현재 Resources 10,158 files
- Debug local smoke: Server `127.0.0.1:7777` listener와 Client가 20초 동안 생존·응답함을 확인하고 시작한 PID만 종료

아직 현재 변경 기준으로 수동 확인하지 않은 항목:

- 실제 화면에서 Character Select 803-placement map 표시
- 다섯 preview 교체와 Enter 후 Test 진입
- 각 class의 Character/HUD/이동, 기존 네 class Q/W
- Dimensionist Animation Tool 154 clip 재생과 Effect Tool candidate open
- 새 immutable pack의 Snapshot -> Verify -> Publish -> Hydrate

따라서 코드·Debug/Release 자동 계약은 연결됐지만, 화면 조작 smoke와 팀 배포 pack까지 완료했다고 주장하지 않는다.
