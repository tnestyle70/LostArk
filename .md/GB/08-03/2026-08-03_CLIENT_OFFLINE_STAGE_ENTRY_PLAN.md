# Client Local / Multiplayer Stage Entry Plan

## 1. 현재 체크포인트와 이번 단계의 완료 조건

현재 Client는 Lobby까지 단독 실행할 수 있다. 그러나 Bern, Valtan, Training의 실제
`CCharacter` 생성은 Server의 `S2C_PLAYER_SPAWNED`에만 연결되어 있으므로 Server 없이
레벨만 전환하면 `Layer_Player`가 비어 있다. 기존 offline smoke도 target level 일치만
확인해 이 상태를 잘못 PASS 처리한다.

또한 Lobby 진입은 `localhost:7777` 연결 실패를 곧바로 local preview로 해석한다. 이
방식은 사용자가 Local을 원한 것인지, Multiplayer 서버 주소가 잘못됐거나 서버가 장애
상태인지 구분하지 못한다.

이번 수직 슬라이스의 완료 조건은 다음과 같다.

- Lobby ImGui에서 `Local Preview`와 `Multiplayer`를 명시적으로 선택한다.
- Local Preview는 socket을 열지 않고 canonical `playerSpawn` 한 곳에 선택 클래스의
  presentation-only `CCharacter` 하나를 생성한다.
- Multiplayer는 입력한 `host:port`에 연결한 뒤 기존
  `C2S_ENTER_WORLD -> S2C_ENTER_ACCEPTED -> S2C_PLAYER_SPAWNED -> snapshot`만 사용한다.
- Server는 기본 loopback bind를 유지하고, 팀 LAN 테스트에서만 명시적인
  `--bind-address 0.0.0.0` 또는 특정 사설 IPv4를 허용한다.
- 연결 실패, enter 거부, timeout, disconnect를 Local Preview로 자동 강등하지 않는다.
- Local Preview 캐릭터는 서버 `PLAYER_ID`, `NET_ENTITY_ID`, packet을 위조하지 않고
  `CNetObjectRegistry`에도 등록하지 않는다.
- Local Preview에서는 gameplay command sink, skill/damage/boss authority를 만들지 않는다.
- offline smoke는 레벨뿐 아니라 선택 클래스 캐릭터와 camera follow까지 확인한다.
- network endpoint smoke는 실제 LAN IPv4 접속, disconnect 후 Lobby 복귀, TCP 연결 뒤
  enter 승인을 보내지 않는 endpoint의 5초 bounded timeout을 확인한다.
- Debug/Release Client build, offline/online smoke, ProjectAudit이 통과한다.

## 2. 모드 판정과 전체 수직 흐름

모드는 연결 결과로 추측하지 않고 Lobby의 사용자 선택으로 결정한다.

```text
Lobby ImGui
-> character class 선택
-> entry mode 선택
   |
   +-> Local Preview
   |   -> socket 연결 없음
   |   -> world ID를 catalog level/scenario/areaId로 해석
   |   -> SceneTransitionService -> Loading
   |   -> Gameplay.world.json의 첫 enabled playerSpawn
   |   -> presentation-only local CCharacter -> camera follow
   |
   +-> Multiplayer
       -> host 입력 + port 입력
       -> endpoint 형식 검증
       -> TCP connect
       -> C2S_ENTER_WORLD
       -> S2C_ENTER_ACCEPTED 이후에만 SceneTransitionService
       -> S2C_PLAYER_SPAWNED -> CNetObjectRegistry -> snapshot
```

기본 endpoint는 개발 편의를 위해 `127.0.0.1:7777`로 표시한다. 같은 LAN의 다른 PC는
서버 PC의 사설 IPv4 주소를 직접 입력한다. 인터넷 외부 접속은 서버 방화벽과 공유기 포트
포워딩 또는 별도 중계가 필요하다.

이번 단계는 IP 직접 입력 연결까지만 소유한다. 자동 LAN IP 탐색은 Server 광고와 Client
발견을 위한 별도 UDP discovery protocol, timeout, 중복 서버 표시, 보안 규칙이 필요하므로
TCP gameplay 연결에 암묵적으로 섞지 않는다.

## 3. 파일별 존재 이유와 책임 지도

### `ClientLaunchOptions`

파일이 존재하는 이유: Lobby에서 선택한 실행 계약을 Loading과 target level까지 전달한다.

- 소유 상태: selected character class, entry mode, multiplayer host/port, scenario.
- 소유하지 않는 상태: socket, network approval, Character GameObject, server authority.
- 불변식: `Local Preview`와 `Multiplayer`는 동시에 활성화되지 않는다.
- 실패 전달: 잘못된 class/mode/endpoint는 Lobby status로 반환한다.

### `LobbyCommandService`

파일이 존재하는 이유: ImGui가 packet/socket을 직접 다루지 않고 enter 의도를 제출한다.

- command 입력: character slot, world ID, nickname, entry mode, host, port.
- command 출력: `Level_Lobby::Update`가 한 번 소비하는 immutable 값.
- 금지: UI 함수 안에서 connect, packet 작성, `Change_Level` 호출.

### `Level_Lobby`

파일이 존재하는 이유: Local과 Multiplayer의 서로 다른 입장 상태 머신을 실행한다.

- Local: endpoint를 무시하고 offline mode를 명시한 뒤 scene transition을 제출한다.
- Multiplayer: endpoint 연결과 enter packet을 수행하고 acceptance 이후에만 전환한다.
- 실패: 오류 원인과 endpoint를 status로 표시하고 Lobby에 남는다.
- 금지: Multiplayer 실패를 Local Preview로 우회, 임의 server state 생성.

### `NetworkManager`

파일이 존재하는 이유: 선택된 endpoint에 TCP 연결하고 Shared packet 경계를 소유한다.

- 입력: 검증된 host 문자열과 port.
- 출력: connected 상태 또는 보존된 Winsock/resolve 오류.
- 이번 단계: IPv4 literal과 `localhost`를 지원한다.
- 연결 종료: receive socket을 먼저 `shutdown`/`closesocket`해 blocking receive를 깨운 뒤
  receive thread를 join한다. join 뒤 socket을 닫는 순서로 되돌리지 않는다.
- 금지: Lobby mode 결정, scene transition, Character 생성.

### `ServerApp` / `TcpListener`

파일이 존재하는 이유: Server process가 어느 network interface에서 gameplay 연결을 받을지
명시적으로 결정한다.

- 기본 bind: `127.0.0.1`로 같은 PC 개발만 허용한다.
- LAN bind: `Server.exe --bind-address 0.0.0.0` 또는 특정 사설 IPv4.
- 출력: 실제 bind address와 `7777`을 Server console에 표시한다.
- 금지: 테스트 하네스가 모르는 상태에서 기본값을 `INADDR_ANY`로 바꾸기.
- 외부 조건: Windows Firewall inbound 허용은 사용자가 Server PC에서 수행한다.

### `OfflinePlayerPreview`

파일이 존재하는 이유: server packet을 위조하지 않고 canonical world spawn에서 로컬 표현
캐릭터를 한 번 생성한다.

- 입력 출처: `LevelCatalog.mapAreaId`, selected class.
- spawn 정본: `Data/Worlds/<AreaId>/Gameplay.world.json`.
- 로드: `CProjectDataRoot -> CWorldGameplayDocument`.
- 선택 규칙: area ID 일치, enabled `playerSpawn`, stable placement ID, finite transform을
  검증하고 `placementId` ordinal 정렬 후 첫 항목을 사용한다.
- 출력: `CClientReplication`의 offline 전용 local handle이 가리키는 `CCharacter`.
- 금지: Loader worker에서 GameObject 생성, Server bootstrap을 spawn 정본으로 사용.

### `ClientReplication`

파일이 존재하는 이유: online과 local preview가 동일한 기존 `CCharacter` 생성 경로를
사용하되 권위 ID 저장 영역을 분리한다.

- 공통 private 생성기: CharacterCatalog 조회, Prototype clone, `Layer_Player` 추가,
  Transform 적용을 한 단계로 소유한다.
- online: 기존 server player/net entity ID로 `CNetObjectRegistry`에 등록한다.
- offline: 별도 `LOCAL_PREVIEW_PLAYER_DESC`와 weak handle만 사용한다.
- 실패: 생성 중인 Layer 객체와 handle을 제거하고 부분 commit을 남기지 않는다.

### `Loader`

파일이 존재하는 이유: target scenario에 필요한 선택 클래스 asset을 admit한다.

- online: Server가 승인한 class만 사용한다. 없거나 잘못됐으면 실패한다.
- offline: 명시적인 Local Preview일 때만 Lobby selected class를 사용한다.
- 금지: 모르는 class를 기본 class로 조용히 치환.

### Bern / Valtan / Training level

- replication 초기화 후 Local Preview인 경우에만 `OfflinePlayerPreview`를 호출한다.
- Multiplayer인 경우 기존 network replication만 초기화한다.
- Local Preview에서는 `CNetworkPlayerCommandSink`를 설치하지 않는다.
- spawn 실패 시 target level을 정상 실행하지 않고 Loading/Lobby 복구 경로로 실패를 전달한다.

### `MainApp` smoke와 ProjectAudit

- static audit는 파일 등록과 금지 문자열을 보조 확인한다.
- runtime smoke가 실제 Character, class, local control, placement, camera, command sink live count를 완료 증거로 삼는다.
- online smoke의 server approval/snapshot 조건은 완화하지 않는다.
- disconnect smoke는 target level의 server Character를 먼저 관찰한 뒤 Server 종료와 Lobby 복귀를 순서대로 증명한다.
- acceptance timeout smoke는 TCP connect만 받아들이고 응답하지 않는 endpoint를 사용해
  wall-clock 기준 5초 안팎에 Lobby에 남고 socket과 command sink가 정리되는지 증명한다.

## 4. H 파일 자연어 계약

### Entry mode

```text
타입: CLIENT_ENTRY_MODE
값: LOCAL_PREVIEW, MULTIPLAYER
owner: ClientLaunchOptions와 한 번 소비되는 Lobby command
초기값: MULTIPLAYER가 아니라 Lobby UI에서 명시적으로 보이는 Local Preview
불변식: 알 수 없는 값은 진입 거부
```

### Multiplayer endpoint

```text
타입: host string + uint16 port
기본 표시값: 127.0.0.1 + 7777
검증: 빈 host, 0 port, 지나치게 긴 host 거부
저장 범위: 현재 process의 Lobby 선택 상태
금지: Data JSON에 개인 IP를 정본으로 commit
```

### `LOCAL_PREVIEW_PLAYER_DESC`

```text
입력: selected class, nickname, position, yaw, placement ID
출력: 기존 CCharacter runtime 하나
포함하지 않는 값: PLAYER_ID, NET_ENTITY_ID, packet, server snapshot
성공 조건: Layer_Player 등록과 offline local handle commit 완료
실패 조건: spec/prototype/spawn/transform/handle 단계 중 하나라도 실패
```

## 5. CPP 내부 흐름 설계

### Local Preview enter

```text
Lobby command 소비
-> class/world/mode 검증
-> offline flag와 selected class 저장
-> SceneTransitionService 요청
-> Loader가 selected class asset admit
-> target level에서 area gameplay document load/validate
-> playerSpawn 후보 정렬/선택
-> 공통 Character 생성기 호출
-> offline weak handle commit
-> camera가 local Character Transform follow
-> offline overlay 표시
```

중간 실패 시 생성 객체와 handle을 제거한다. level 초기화 실패는 기존 Loading 복구 계약으로
Lobby에 돌아가며 오류 이유를 보존한다.

### Multiplayer enter

```text
Lobby command 소비
-> class/world/mode/endpoint 검증
-> connect(host, port)
-> 실패: status 갱신, Lobby 유지
-> 성공: C2S_ENTER_WORLD
-> S2C_ENTER_ACCEPTED world ID 검증
-> SceneTransitionService 요청
-> Loader는 server-approved class만 admit
-> S2C_PLAYER_SPAWNED가 online Character 생성/registry 등록
-> snapshot이 transform/action/HP/resource/cooldown 갱신
```

enter 거부, acceptance timeout, 진입 후 disconnect는 Local Preview로 전환하지 않는다. replicated
state를 정리하고 기존 Lobby recovery 정책을 따른다. 다시 접속하려면 Lobby에서 새 Multiplayer
command를 제출한다.

## 6. 의존성과 결합 규칙

```text
호출 흐름(Local):
ImGui -> LobbyCommandService -> Level_Lobby -> SceneTransitionService
-> target Level -> OfflinePlayerPreview -> ClientReplication -> CCharacter

호출 흐름(Multiplayer):
ImGui -> LobbyCommandService -> Level_Lobby -> NetworkManager
-> Shared packet -> Server -> Shared packet -> ClientReplication -> CCharacter

데이터 흐름(Local spawn):
LevelCatalog.mapAreaId -> Data/Worlds Gameplay.world.json
-> CWorldGameplayDocument -> LOCAL_PREVIEW_PLAYER_DESC -> Layer_Player

데이터 흐름(Multiplayer spawn):
Server World bootstrap -> S2C_PLAYER_SPAWNED
-> ClientReplication -> CNetObjectRegistry + Layer_Player

Server listen:
Server CLI bind address -> TcpListener -> OS interface:7777
-> Lobby의 Multiplayer endpoint가 TCP connect
```

두 흐름은 Character 생성 helper만 공유한다. Local Preview 데이터가 Server message나 registry로
흘러가면 안 되고, Multiplayer가 Lobby selected class로 server approval을 대신해서도 안 된다.

## 7. 비평 반영 금지 규칙

비평 에이전트의 P0/P1 지적을 실제 코드와 대조해 다음 실행 계약으로 고정한다.

- 가짜 `S2C_PLAYER_SPAWNED`, `PLAYER_ID`, `NET_ENTITY_ID` 생성 금지.
- offline Character의 `CNetObjectRegistry` 등록 금지.
- online class 오류를 Lobby selected class로 fallback 금지.
- offline command sink, local skill/damage/boss authority 생성 금지.
- 접속 성공 후 enter 거부·timeout·disconnect를 offline으로 우회 금지.
- Loader worker에서 GameObject 생성 금지.
- runtime map placement나 Server bootstrap을 offline spawn 정본으로 사용 금지.
- 연결 실패를 entry mode 판정으로 사용 금지.
- 개인 IP나 탐색 결과를 Git 정본 데이터로 저장 금지.
- Server의 기본 bind를 무조건 전체 interface로 노출 금지.

## 8. 프로젝트 등록

새 `OfflinePlayerPreview` H/CPP는 다음을 한 변경 단위로 등록한다.

- 물리 파일: `Client/Public`, `Client/Private`
- `Client.vcxproj`: `ClInclude`, `ClCompile`
- `Client.vcxproj.filters`: presentation spawn 책임과 일치하는 `01. Levels` 필터
- 새 Engine 또는 Shared project reference는 만들지 않는다.

## 9. 완료 smoke와 회귀 검증

### Offline Bern / Valtan / Training

시작 전에 7777 listener가 없음을 harness가 강제한다. 다음 조건을 모두 만족해야 PASS다.

- entry mode가 `LOCAL_PREVIEW`다.
- `NetworkManager.Is_Connected() == false`다.
- target level이 일치한다.
- `Layer_Player[0]`가 존재한다.
- Character가 locally controlled이고 selected class와 일치한다.
- replication offline local handle이 같은 객체를 가리킨다.
- 기록된 placement ID가 canonical enabled `playerSpawn`이다.
- camera follow가 활성화되고 동일 Character Transform을 가리킨다.
- network command sink가 없고 Valtan boss/network action이 생성되지 않는다.
- report에는 class ID, placement ID, camera follow, network 연결 여부, command sink live count가 기록된다.

### Online Bern / Valtan / Training

- 실제 Server PID가 7777 listener를 소유한다.
- `C2S_ENTER_WORLD -> S2C_ENTER_ACCEPTED -> S2C_PLAYER_SPAWNED`가 확인된다.
- player ID와 net entity ID가 유효하다.
- snapshot과 기존 skill/cooldown, Valtan action 회귀 조건이 유지된다.
- 잘못된 IP 또는 닫힌 port는 Lobby 오류로 남고 Local Preview로 진입하지 않는다.
- `--bind-address 0.0.0.0` Server는 실제 `0.0.0.0:7777` listener를 만들고 종료 시 정리한다.
- Server가 스테이지 진입 후 종료되면 replicated state와 online command sink를 정리하고 Lobby로 복귀한다.
- TCP 연결 후 `S2C_ENTER_ACCEPTED`가 오지 않으면 `steady_clock` 절대 deadline으로 5초 뒤
  연결을 닫고 Lobby에 남는다. 프레임 delta 누적을 timeout 정본으로 사용하지 않는다.

### 정적·빌드 검증

1. 관련 JSON/XML parse와 `git diff --check`
2. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
3. Client x64 Debug/Release build
4. 별도 offline smoke report 세 시나리오
5. LAN endpoint, disconnect recovery, enter approval timeout smoke
6. 기존 online build/regression 세 시나리오
7. 변경 diff와 Git 상태 확인 후 단일 계약 commit/push

```powershell
Tools/Build/Invoke-OfflineClientSmoke.ps1 -Configuration Debug
Tools/Build/Invoke-OfflineClientSmoke.ps1 -Configuration Release
Tools/Build/Invoke-NetworkEndpointSmoke.ps1 -Configuration Debug
Tools/Build/Invoke-NetworkEndpointSmoke.ps1 -Configuration Release
```

## 10. 비평 결론과 승인 게이트

초기 계획은 target level만 확인해 캐릭터 부재를 통과시키므로 승인 불가였다. 위 P0/P1
계약을 구현하고 실제 runtime smoke로 증명하기 전에는 완료 또는 PR ready로 기록하지 않는다.

자동 LAN discovery는 이번 PR의 완료 조건이 아니다. 별도 구현 시 Server advertisement,
Client discovery, 서버 목록 stable ID, timeout/중복/보안 harness를 먼저 계획하고 현재 TCP
connect 계약 위에 선택적으로 추가한다.
