# LostArk 팀 게임플레이 인터페이스 사용서

작성일: 2026-08-03
정본 브랜치: `codex/baren-player-replication`

## 0. 세션 시작과 사용자 전용 화면 검증

모든 세션은 `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, 있으면
`.md/GB/gotchas.local.md`, `.md/TEAM/README.md`, 대응 PLAN/RESULT를 먼저 읽는다.
Artist F, Character Select와 Client Effect의 시각 결과는 사용자만 직접 조작하고 최종 visual fidelity를 판정한다.
에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고 화면 캡처·스크린샷 생성을 하지 않으며 visual fidelity를 대신 판정하지 않는다.
사용자가 대화에 첨부한 스크린샷이나 이미지 분석을 요청하면 에이전트는 반드시 열람·분석해 관찰 결과와 가능한 occurrence 진단을 보고한다.
빌드와 구조화된 진단, 실행 준비 후 사용자가 누를 경로를 전달하고 멈추며, 사용자 서면 판정 전에는
first pixel, eye smoke, visual PASS 또는 occurrence 승인을 완료로 기록하지 않는다.

## 1. 한 줄 계약

제품 게임플레이는 항상 다음 한 방향으로 흐른다.

```text
Input/UI intent
-> Client command service 또는 IPlayerCommandSink
-> Shared C2S message
-> Server GameRoom 30 Hz authority
-> Shared S2C snapshot
-> ClientReplication
-> Character/Valtan presentation 또는 CombatHUDViewModel
```

Client는 입력을 빠르게 제출하지만 위치, damage, cooldown, HP, boss phase를 확정하지 않는다. Server가 확정한 snapshot만 제품 화면의 정답이다.

Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 명령만 제공한다. Character Select는 Lobby가 `WORLD_ID::CHARACTER_SELECT_ARENA` 승인 payload를 검증한 뒤 기존 socket을 one-shot handoff하여 같은 visual map을 여는 Server 전용 Level이다. offline Preview와 `Preview / Server Play` mode 선택은 없다. Level은 직접 connect/send하지 않고 `CClientReplication`, `CNetworkPlayerCommandSink`, `CPlayerController`로 HUD·우클릭 이동·quick-slot 스킬을 Server snapshot에 연결한다. class thumbnail 선택은 target asset admission 뒤 즉시 typed class-change command를 제출하며, Server 승인 snapshot이 같은 entity presentation과 skill catalog class를 교체한다. 살아 있는 위치와 identity는 유지하고 전투 상태는 새 profile로 초기화하며, 사망 중 변경은 원래 projected spawn에서 부활한다. 연결 실패·거부·5초 timeout은 Lobby에 남고 disconnect는 Lobby로 복귀하며 자동 local gameplay fallback은 없다. Debug ImGui의 일반 몬스터, `MINIBOSS_LUGARU`, Valtan 선택은 `IWorldEntityCommandSink`를 통해 stable SpawnGroup/placement ID만 제출한다. Server는 Character Select Area 문서, navigation, profile을 검증해 실제 entity 생성이 성공한 뒤 활성화 결과를 회신하고 기존 monster brain 또는 Valtan brain으로 broadcast하며 Client local spawn은 없다. 마지막 플레이어가 퇴장하면 동적 audition entity와 SpawnGroup 상태를 초기화해 다음 입장을 새 세대로 시작한다. Bern/Valtan map 진입도 마지막 Server 승인 class로 Lobby Server 승인이 필수다.

`CHARACTER_SELECT_ARENA`의 gameplay authority는 Server에 남지만 simulation ownership은 session-private이다. 각 session은 자기 `CGameRoom`의 player, audition entity, HP와 damage event만 snapshot으로 받고 다른 Character Select session과 만나지 않는다. 퇴장한 session의 private room은 queued `LEAVE`와 reset을 처리한 뒤 폐기한다. Bern, Valtan, Training Ground는 기존처럼 world별 shared room이므로 그 안에서는 여러 player가 같은 authoritative 상태를 본다.

Character Select의 `Create Character`는 선택 class와 공통 validator를 통과한 1~32-byte UTF-8 nickname을 `CCharacterSelectionState`의 pending identity로 stage한다. Lobby가 그 exact identity로 Bern entry를 승인받고 loading resource, rendering profile, 실제 `Change_Level(BERN)`까지 성공한 뒤에만 created identity로 commit한다. 중간 실패는 pending만 취소하고 기존 created identity는 유지한다. created identity가 없는 direct Character Select, Training, Valtan entry는 process-local `Test-<process-id>` audition nickname을 사용한다. Server의 `SERVER_PLAYER::strNickName`과 world transfer가 session lifetime 동안 exact nickname을 보존하고 `S2C_PLAYER_SPAWNED`로 복제한다. nickname은 display text이며 player lookup, Party member ID, 고유성 검사 또는 Client 재실행 뒤 영구 저장에 사용하지 않는다. Bern과 Valtan은 `CClientReplication::Collect_PlayerViews`의 Server-replicated nickname과 weak character presentation을 `CWorldPlayerNameplateView`에 전달한다. projection, UTF-8 변환, font draw 실패는 gameplay와 replication을 건드리지 않고 해당 nameplate만 생략한다.

2026-09-30 23:59 KST까지 공유 LAN Server는 같은 팀 LAN의 `10.207.18.103:7777`이다. Server는 `0.0.0.0:7777`에 수신하고 Server PC와 다른 PC의 Client는 모두 concrete endpoint `10.207.18.103:7777`을 사용한다. `Tools/Network/TeamLanEndpoint.json`이 endpoint와 만료일 정본이다. 각 에이전트는 pull 후 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행하고 출력된 역할에 맞는 target을 안내하며, 실제 `Ctrl+F5` 시작과 UI 조작은 사용자가 수행한다.

### 1.1 서로 다른 장소에서 Server와 Client 연결

Server와 Client가 같은 PC, 같은 LAN, 서로 다른 네트워크 중 어디에 있는지 먼저 구분한다.

| 실행 위치 | Server `--bind-address` | Client `LOSTARK_SERVER_HOST` |
|---|---|---|
| 현재 팀 LAN 공유 Server | `0.0.0.0` | `10.207.18.103` |
| 같은 PC 격리 harness | 명시적 `127.0.0.1` | 명시적 `127.0.0.1` |
| 서로 다른 장소/네트워크 | `0.0.0.0` | Server PC의 VPN IPv4(권장) 또는 TCP 7777이 포트포워딩된 공인 endpoint |

`192.168.x.x`, `10.x.x.x`, `172.16.x.x`~`172.31.x.x`는 사설 주소다. 서로 다른 장소의 Client는 Server PC의 Wi-Fi 사설 주소로 직접 접속할 수 없다. 팀 테스트는 두 PC를 같은 사설망처럼 연결하는 VPN 주소를 우선 사용한다. 공인 인터넷에 직접 노출해야 한다면 Server PC로 TCP `7777`을 포트포워딩하고 Windows Firewall의 인바운드 범위를 승인된 원격 주소로 제한한다.

Server PC의 Git 제외 로컬 파일 `Server/Default/Server.vcxproj.user`에는 다음처럼 모든 현재 어댑터를 수신하도록 지정한다. DHCP로 Wi-Fi 주소가 바뀌어도 존재하지 않는 옛 주소에 bind하지 않으므로 `WSAEADDRNOTAVAIL (10049)`를 피할 수 있다.

```xml
<LocalDebuggerCommandArguments>--bind-address 0.0.0.0</LocalDebuggerCommandArguments>
```

각 PC의 에이전트는 세션 시작 시 아래 명령으로 Server/Client의 Git 제외 로컬 설정을 정본과 동기화한다. Server가 아직 꺼져 있으면 `Endpoint status now: not-listening`이 나올 수 있지만 설정 실패는 아니다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

스크립트는 현재 endpoint IPv4가 `Connected + Preferred` 상태인 PC를 `server-host`, 나머지를 `client`로 자동 판정한다.
`server-host`는 Server+Client profile을 시작하고, `client`는 불필요한 로컬 Server를 띄우지 않고
Client project만 시작한다. 자동 판정이 예상과 다르면 IP 어댑터 상태와 endpoint 정본을 먼저
교정하며 `-Role Server`로 주소 소유 검사를 우회할 수 없다.

현재 공유 기본값과 다른 endpoint를 검증할 때만 `TeamLanEndpoint.json`, Server/Client 기본값,
공유 debugger 설정, 문서와 audit를 같은 변경 단위로 교체한다. 한 PC의 `.vcxproj.user`만 바꿔
팀 계약을 갈라놓지 않는다.

```xml
<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=10.207.18.103</LocalDebuggerEnvironment>
```

`0.0.0.0`은 Server의 수신 주소일 뿐 Client 접속 주소로 사용하지 않는다. 현재 Client 기본값은 `10.207.18.103`이다. 주소를 바꾸면 `Tools/Network/TeamLanEndpoint.json`, Server/Client 코드 기본값, 공유 debugger 설정과 이 사용서를 같은 변경 단위에서 갱신하고 `Sync-TeamLanEndpoint.ps1`, NetworkProtocolHarness, Server contract test로 검증한다.

#### pull 후 공유 Server에 들어가는 순서

Server PC와 Client PC는 먼저 같은 `origin/main` commit과 생성 데이터를 맞춘다. `pull`만 하고 예전 실행 파일을 쓰면 protocol v40 또는 Debug gameplay revision이 달라 Server가 연결을 종료할 수 있다.

```powershell
git switch main
git pull --ff-only origin main
git lfs pull
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git rev-parse HEAD
```

두 PC의 `git rev-parse HEAD`가 같아야 한다. `Client/Bin/Resources`의 `Fonts, Character, Deploy, Effect, Map, UI` 여섯 물리 폴더도 팀장이 전달한 같은 runtime 입력이어야 하며, Git에 없는 UI/Character/Map 리소스는 별도 전달을 먼저 완료한다. Debug configuration으로 공유할 때는 두 PC 모두 Debug 정본 빌드를 실행하고, Server를 중지한 상태에서 Server PC가 `Server/Bin/Debug/Server.exe --reset-valtan-runtime-to-packaged`를 한 번 통과시킨다. cross-PC Debug Hot Reload candidate 공유는 지원하지 않는다.

동기화 뒤 Visual Studio project를 Reload하거나 IDE를 재시작한다. Server PC에서 `Machine role: server-host`를 확인하고 `Server + Client` profile을 시작한다. 다른 PC는 `Machine role: client`를 확인하고 `Client Only (Server Already Running)` profile을 시작한다. 직접 EXE를 실행할 때 shell의 오래된 `LOSTARK_SERVER_HOST`가 새 기본값보다 우선하므로 값이 `127.0.0.1`이면 제거하거나 `10.207.18.103`으로 맞춘다.

Server가 `Listening on 0.0.0.0:7777`을 출력한 뒤 다른 PC에서 아래 probe가 성공해야 한다. Windows 네트워크가 `공용`이어도 repository sync가 검증하는 firewall rule은 `Profile Any`, `RemoteAddress LocalSubnet`이므로 직접 IPv4 접속에는 문제가 없다.

```powershell
Test-NetConnection 10.207.18.103 -Port 7777
```

`TcpTestSucceeded: False`면 Server listener, endpoint 어댑터, TCP 7777 firewall, 공유기의 AP/client isolation 순서로 본다. `True`인데 Lobby에서 거부되거나 끊기면 서로 다른 commit/binary/protocol/gameplay bootstrap을 먼저 확인한다. 승인 뒤 `Stage loading failed`로 Lobby에 남으면 네트워크가 아니라 Client runtime Resources 또는 Loader 문제다. Bern과 Valtan은 player spawn이 네 개라 Server PC의 Client도 입장하면 다른 PC 세 대까지 같은 room에 들어갈 수 있다. Character Select는 session-private이므로 여러 PC가 같은 Server를 써도 서로 보이지 않으며, 동시 플레이 확인은 Bern 또는 Valtan에서 한다.

#### Lobby fallback 진단과 4인 대조

Lobby로 복귀하면 `Last Lobby recovery diagnostic`에서 최초 reason과 source/detail, remote/local endpoint,
protocol, world/player/entity, packet/WSA/HRESULT, terminal UTC, 마지막 Server tick, raw/event queue
current/high와 capture 경로를 확인한다. 운영 실패를 runtime assertion으로 process 종료하지 않으며,
reason schema와 실패 재현은 NetworkProtocolHarness와 Server contract test가 assertion으로 고정한다.

Client capture는 실행 파일 옆 `Diagnostics/client-session-<pid>.jsonl`, Server capture는 실행 파일 옆
`Diagnostics/server-session-<pid>.jsonl`이다. 기본 Debug 실행이면 각각
`Client/Bin/Debug/Diagnostics`, `Server/Bin/Debug/Diagnostics` 아래에 생긴다. gameplay payload와 nickname은
기록하지 않는다. Client의 `localEndpoint`는 실제 IP와 ephemeral port이고 direct LAN에서는 Server line의
`peerAddress:peerPort`와 정확히 일치한다. protocol v39에서 이미 끊어진 TCP는 Server-only 원인을 Lobby로
되돌려 보낼 수 없으므로 이 endpoint와 terminal UTC, player/entity를 함께 대조한다.

`ROOM_FULL`이면 Server line의 context에서 `candidateSessionId`,
`registeredSessionsIncludingCandidate`, `activeRoster`를 본다. registered 수에는 거부된 candidate도
포함된다. active roster의 네 `peer`를 네 Client Lobby의 local endpoint와 대조해 실제 사용자에게 없는
endpoint가 있을 때만 stale/ghost 후보로 판정한다. `lastInboundAgeMs`는 heartbeat가 없는 계약에서 건강한
idle Client도 커질 수 있으므로 단독 ghost 증거가 아니다. 네 endpoint가 모두 실제 접속자와 일치하면
Server-host Client를 포함한 실제 fifth entry인지 확인한다.

- `CLIENT_LOAD_*`, `CLIENT_ACTIVATION_*`, presentation/revision reason은 평균 Wi-Fi 속도가 아니라 해당
  Client의 Data/Resources/Loader/Level 적용 경로를 먼저 조사한다.
- `SERVER_SEND_ERROR_OR_TIMEOUT`과 큰 outbound high-watermark는 평균 RTT가 아니라 해당 Client가 socket을
  drain하지 못한 blocked-send 증거다.
- `SERVER_ROOM_INGRESS_OVERFLOW` 또는 reliable overflow는 Server queue pressure다.
- `CLIENT_EXPECTED_ROOM_FULL`은 정상 capacity rejection이며 active roster 대조 전에는 ghost로 단정하지 않는다.

현재 Server PC 주소가 실제 어댑터에 있는지는 다음 명령으로 확인한다.

```powershell
Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object { $_.AddressState -eq 'Preferred' -and $_.IPAddress -notlike '127.*' } |
    Select-Object InterfaceAlias, IPAddress
```

설정 후 Visual Studio는 `.vcxproj.user`를 메모리에 캐시할 수 있으므로 Server와 Client project를 Reload하거나 Visual Studio를 재시작한다. 그다음 Server PC와 Client PC에서 각각 확인한다.

`Sync-TeamLanEndpoint.ps1`은 `2026-09-30 23:59 KST`가 지나면 실패한다. 2026-10-01 이후 첫
세션은 `-AllowExpired`로 계속 쓰지 않고 새 endpoint 또는 loopback 복귀 계약을 먼저 정한다.

```powershell
# Server PC
Server\Bin\Debug\Server.exe --bind-address 0.0.0.0 --smoke-timeout-ms 500

# Client PC: VPN/LAN endpoint가 실제로 열렸는지 확인
Test-NetConnection 10.207.18.103 -Port 7777
```

`Failed to open TCP listener ... Error=10049`는 `--bind-address`에 적은 주소가 현재 Server PC의 어느 어댑터에도 없다는 뜻이다. Client의 주소나 이전 Wi-Fi 주소를 Server bind 값으로 복사하지 말고 Server는 `0.0.0.0`, Client만 도달 가능한 endpoint를 사용한다.

## 2. 팀원이 먼저 읽을 파일

| 담당 | 시작 파일 | 데이터 정본 |
|---|---|---|
| UI | `Client/Public/CombatHUDViewModel.h`, `LobbyCommandService.h`, `LevelTransitionService.h` | `Data/UI`, `Data/Balance` |
| Player/Input | `Client/Public/PlayerController.h`, `PlayerCommandSink.h` | `Data/Balance/PlayerSkills.json` |
| Character/Animation | `Client/Public/Character.h`, `CharacterSpec.h`, `AnimationTargetService.h` | `Data/Actors`, `Data/Animation` |
| Server/Player | `Server/Public/GameRoom.h`, `PlayerSkillSystem.h`, `ServerNavigation.h` | `Data/Balance`, `Data/Navigation`, `Data/Worlds` |
| Boss | `Server/Public/ValtanBrain.h`, `ServerWorldEntity.h` | `Data/Balance/BossProfiles.json`, `Data/Encounters` |
| Map/Encounter | `Client/Public/MapTool.h`, `WorldGameplayDocument.h` | `Data/Maps/Authoring`, `Data/Worlds`, `Data/Navigation` |
| 통합/검증 | `AGENTS.md`, `CLAUDE.md` | `Tools/Build`, 각 domain publisher, 실행형 harness |

### 2.1 기능 담당자의 수직 슬라이스 책임

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

기능 담당자는 수직 슬라이스의 구현 책임자다. 위 표는 어디서 시작하고 어떤 정본을 읽는지
알려주는 표이지, 해당 행 밖의 파일을 수정하지 못하게 하는 권한표가 아니다.

```text
요청 기능
-> authoring/balance JSON과 stable ID
-> 필요한 Shared command/snapshot
-> Server validation·authority·room state
-> Client replication·presentation·ViewModel
-> 실제 UI/Character/Map 소비자
-> protocol/server/client harness와 실패 경로
```

- Player/Input 기능이 Server 판정을 요구하면 그 작업자가 Shared message와 `GameRoom`/skill
  system까지 함께 구현한다.
- Character/Animation 기능이 새 action을 요구하면 balance/action ID, Server 승인 상태,
  snapshot과 presentation mapping을 함께 닫는다.
- UI 기능에 새 runtime 값이 필요하면 UI에서 socket을 읽지 않고 Server snapshot과
  `CCombatHUDViewModel`까지 확장한다.
- Map/Encounter 기능이 spawn이나 encounter truth를 바꾸면 world authoring, publisher,
  Server bootstrap/runtime과 Client presentation을 함께 검증한다.
- 다른 담당자의 미커밋 줄을 덮어쓰거나 public interface를 우회하는 것은 계속 금지한다.
  필요한 교차 영역 수정 자체는 금지하지 않는다.

## 3. 플레이어 입력과 스킬

현재 제품 입력은 다음과 같다.

| 입력 | stable ID | 결과 |
|---|---:|---|
| 우클릭 | move sequence | `C2S_MOVE`로 목표 X/Z 제출 |
| 좌클릭 | `34010` | 평타 콤보. 물리 LMB 한 번당 성공한 command를 최대 한 번 제출한다 |
| Q | `34120` | 연환섬 사용 의도 제출 |
| W | `34080` | 일섬각 사용 의도 제출 |
| E | `34070` | 회선창 사용 의도 제출 |
| R | `34150` | 맹룡열파 사용 의도 제출 |
| A | `34110` | 반월섬 사용 의도 제출 |
| S | `34090` | 철량추 사용 의도 제출 |
| T | `34640` | 맹룡난무 사용 의도 제출 |
| V | `34600` | 은하유성탄 사용 의도 제출 |
| Alt+V | `34620` | 은하비섬창 사용 의도 제출 |

`CPlayerController`는 edge input, quick slot, sequence, aim만 만든다. `(class, inputSlot) -> skill ID`는 `CPlayerSkillCatalog`가 `Data/Balance/PlayerSkills.json`에서 해석한다. `IPlayerCommandSink`가 전송 구현을 숨기므로 Controller에서 `CNetworkManager`를 include하지 않는다. Character를 직접 이동하거나 `Play_Skill`을 호출하지 않는다.

two-step ground target의 optional 정본은 `Data/Balance/PlayerSkillTargeting.json`이다. T/2050500은 첫 키 입력에서 packet·sequence·resource·cooldown을 소비하지 않고 class-neutral targeting state와 두 preview만 연다. valid navigation sample의 LMB confirm만 기존 `C2S_USE_SKILL`에 typed `GROUND_POINT` intent를 실으며 RMB cancel은 packet을 만들지 않는다. 성공한 confirm LMB는 물리 release 전까지 BA로 다시 해석하지 않는다. Client의 11m clamp와 red invalid 표시는 preview이고, Server가 finite/range/current navigation을 다시 검증해 승인한 target XYZ만 `PLAYER_SNAPSHOT`으로 복제한다. Character의 `skill_target` pseudo anchor와 Server damage shape는 이 승인 XYZ를 함께 사용한다. 사거리 링 asset identity는 `SOURCE_EXTRACTED`, cursor marker identity는 `RUNTIME_RESOURCE`지만 두 texture의 preview scale/tint/usage는 모두 `PROJECT_TUNED`다.

LMB COMBO는 `comboStages[].hitTimeMs`, `comboAdvanceMs`, `actionDurationMs`를 구분한다. `hitTimeMs`는 damage 발생 시점이다. non-final stage의 input window가 non-zero인 manual COMBO에서 `comboAdvanceMs`는 필수 caster hit/projectile spawn이 끝난 뒤 buffered BA가 다음 stage로 갈 수 있는 가장 이른 시점이며, release는 재생 중인 stage를 자르지 않고 아직 commit되지 않은 continuation만 취소한다. non-final stage가 `inputOpenMs/inputCloseMs == 0/0`이고 `comboAdvanceMs == actionDurationMs`이면 automatic COMBO다. 이 경우 Server는 추가 LMB 없이 full-motion 경계마다 다음 stage로 전환하고, pending MOVE/SKILL도 chain을 끊지 않은 채 마지막 stage 종료 뒤 commit한다. 차원술사 `2050010`은 `_01(3000ms source/2x) -> _03 -> _04`의 automatic 3-stage 계약이다. manual COMBO 중 pending command는 현재 stage의 `actionDurationMs`까지 유지한 뒤 commit한다. Client는 성공한 LMB command를 같은 물리 press에서 재제출하지 않고, 명시 command가 수락된 뒤에도 실제 LMB release 전까지 BA를 억제한다.

스킬 서버 흐름은 다음과 같다.

```text
CPlayerController::Update
-> IPlayerCommandSink::Request_UseSkill
-> CNetworkManager::Send_UseSkill
-> C2S_USE_SKILL { clientSequence, skillId, aimX, aimZ }
-> CServerApp::On_SessionFrame
-> CGameRoom::Handle_UseSkill
-> CPlayerSkillSystem::Try_Start / Update
-> S2C_WORLD_SNAPSHOT
```

Client payload에는 PlayerId와 NetEntityId가 없다. Server가 SessionId로 player를 찾고 sequence, class, 생존, 현재 action, cooldown, resource를 검사한다. 승인하면 이동 목표를 취소하고 action tick, skill ID, cooldown end tick을 Server 상태에 기록한다.

## 4. 이동, Navigation, collider 경계

우클릭 피킹은 입력 목표를 얻기 위한 Client 표현 계층이다. 제품 위치의 정답은 Server Navigation이다.

- 일반 이동: Server가 navgrid에서 시작/목표를 projection하고 8방향 A* path를 만든다.
- 높이: 각 Server nav point의 Y를 사용한다.
- 스킬 이동: `movementDistance`를 action duration에 분배하고 매 tick 다음 위치를 navgrid로 projection한다.
- 보스 이동: `CValtanBrain`이 같은 Server Navigation으로 target까지 path를 계산한다.
- Client `CNavigation`, mesh picking, animation root motion은 Server 위치를 확정하지 않는다.

walkable nav cell 경계와 별개로, 투사체·지연 장판·보스 이동 공격은 room-owned `CCombatObjectRuntime`의 pure XZ pose/swept primitive가 Server fixed tick에서 판정한다. 플레이어 투사체와 발탄 전투 객체는 spawn adapter만 다르고 같은 live set과 hit resolver를 사용한다. Shared combat-object lifecycle/full snapshot과 Client world-root Effect는 위치 표현만 담당하며 Client collider가 피해를 판정하지 않는다. 동적 capsule-vs-capsule와 knockback obstacle collision은 아직 public 계약이 아니므로 추가할 때 Server collision owner, shape ID, broad/narrow phase, snapshot correction, harness를 한 변경 단위로 닫는다.

## 5. Character와 Animation

### 5.1 Source Effect attachment basis

Imported/Product Effect의 attachment는 `FOLLOW_NAMED_ANCHORS`와 `SNAPSHOT_ROOT`를 같은 transform
경로로 취급하지 않는다. named-bone follow는 admitted model/bone world가 소유한 import basis를
상속한다. root snapshot은 source cue local transform 뒤, occurrence 시작 시 캡처한 actor root 앞에
character-specific `snapshotRootSourceBasisYawDegrees`를 정확히 한 번 합성한다.

- 이 값은 화면을 보고 맞춘 global particle yaw가 아니며 character import evidence가 소유한다.
- 다른 character에 Artist의 `-90°`를 복사하지 않는다. 근거가 없으면 identity `0°`다.
- follow occurrence에 snapshot basis를 다시 적용하거나, Mesh geometry pitch/scale로 방향 오류를 숨기지
  않는다.
- codec/source validator/runtime은 attachment mode, basis 값, transform composition order를 함께 validate하고
  unknown/non-finite 값은 stage 전에 거부한다.
- 위치뿐 아니라 orientation covariance, fixed-step snapshot 불변성, invalid follow fail-closed를 harness로
  검증한다.
- stable occurrence/material program의 근거가 부족하면 해당 occurrence만 draw 전 fail-closed하며,
  family 전체 fallback이나 white/opaque texture 대체로 열지 않는다.

animevent v6은 position follow policy와 orientation authority를 다른 필드로 저장한다.
`orientation=action_facing`은 root anchor의 ACTIVE 스킬만 Server snapshot에서 actionStartTick과 함께
캡힌 yaw를 사용한다. 위치는 outer `follow` 계약대로 살아 있을 수 있고, 각 occurrence는
시작 시점에 그 root를 snapshot할 수 있다. world-root, HOLD, mirrored/non-finite basis는 현재
action-facing admission 대상이 아니며 해당 cue만 격리한다. v5의 누락값은 `anchor`다.

Character는 Server action을 시각화한다.

```text
CClientReplication::Apply_WorldSnapshot
-> CCharacter::Apply_NetworkState
-> CCharacter::Apply_NetworkAction
-> approved skill: Play_Skill
-> locomotion: RUN / IDLE
```

`Set_Locomotion()`만으로 스킬 계약 전체가 닫히는 것은 아니다. 이 함수는 이동 중 RUN, 정지 시 IDLE을 고르는 표현 함수다. 스킬 중에는 locomotion animation 전환을 보류한다. 실제 skill 시작은 snapshot의 `action`, `skillId`, `actionStartTick`이 바뀌었을 때만 `Apply_NetworkAction()`이 수행한다.

Character/Animation 담당자는 clip mapping, part, notify, blend와 재생 결과를 소유한다. damage, cooldown, resource, hit 여부, 위치 정답은 수정하지 않는다. `Logic_*`에서 DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다.

현재 roster는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster, Warlord 여섯 class다. 여섯 class의 quick slot ACTIVE 스킬과 LMB COMBO 평타는 모두 Server 승인과 snapshot presentation을 사용한다. `PlayerSkills.json`이 `(characterClass, inputSlot) -> skillId`, kind, timing, comboStages를 소유하고, `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`이 같은 skillId에 연결할 실제 model clip 순서를 소유한다. ACTIVE는 하나 이상의 순차 clip을 재생하고 마지막 pose를 Server `NONE`까지 유지한다. COMBO는 Server `iComboStage`가 지정한 BA 단계로 직접 이동한다. 누락되거나 잘못된 presentation 문서는 spawn/replication을 중단하지 않고 해당 action의 표현만 격리한다.

Animation Tool은 Scene Character의 현재 model에 실제 존재하는 clip만 저장할 수 있다. 작업자는 key/skill row에서 ACTIVE의 순차 clip 또는 COMBO의 BA1/BA2/BA3/BA4 clip을 지정하고 atomic Save한다. `inputSlot`, `skillId`, `skillKind`, timing, damage와 combo 단계 수는 Tool에서 바꾸지 않는다. `.skilltiming/.clipmap/.animnotify/.clipseq` 및 `Data/Animation/Reference`는 read-only 참고 자료다.

제품 Effect 선택 정본은 `PlayerSkills.inputSlot -> skillId -> skillbindings clip`에서
`clip-local animevent effectref=asset -> Effect catalog/prewarm`으로 이어지는 경로다. `PlayerSkills.effectId`를 복원 결과로 바꾸거나
다른 class/skill 문서를 fallback으로 복사하지 않는다. Product membership은 source
`Data/Effects/EffectCatalog.json`의 direct row와 clip-local `effectref=asset` cue가 결정한다. Catalog row가
가리키는 `Data/Effects/Authored/<EffectAssetId>.effect.json`이 제품 element, Transform, lifetime과 v15 typed
runtime carrier의 단일 정본이다. authoring-only 문서 중 Catalog row가 없는 ID는 Character Select 준비 대상이
아니다. generated runtime catalog, hash seal, VisualPrograms sidecar와 Effect publish 단계는 사용하지 않는다.

Debug Effect Tool의 Save는 해당 source 문서를 원자 저장하고 새 Product target을 parse → validate → GPU stage →
commit한다. 성공 뒤 다음 cue spawn은 새 document를, 이미 재생 중인 occurrence는 기존 immutable document를
사용한다. 중간 실패는 저장 파일을 compare-and-swap으로 이전 bytes로 복원하고 기존 prepared target을 유지한다.
다음 Client 실행도 같은 source 문서를 읽으므로 별도 publish나 재시작 적용 절차가 없다. 전체 source 검증은
`Tools/EffectPipeline/Validate-EffectSources.ps1`로 수행한다.
V1 Product source가 실제 참조하는 DDS/WModel dependency closure는 같은 Resources-relative 경로로 Git/LFS에
선별 추적한다. 팀원은 pull 뒤 `git lfs pull`을 수행하며, 전체 Resources pack과 미참조 파일은 포함하지 않는다.
Valtan actor Product가 직접 참조하는 body, Parts1/Parts2, AnimSet과 weapon 다섯 WModel, 그리고 그 material
table이 참조하는 body/parts TGA 12개와 weapon DDS 8개도 같은 pull-only closure에 포함한다. 따라서 새 clone은
발탄 본체를 위해 별도 추출 WModel이나 material texture를 수동 복사하지 않는다.

Character는 cue/anchor/HIT metadata를 먼저 commit하고 Product ID만 revision별 queue에 등록한다.
등록 frame에는 resource 작업을 하지 않으며 다음 frame부터 main thread가 target 하나씩 parse,
drawable validation, budget 산정과 GPU 준비를 수행한다. 성공한 target만 prepared로 commit하고 실패한
target 하나만 같은 revision에서 격리한다. Character Select Loading은 worker 시작과 함께 선택 class target을
priority queue에 놓아 map/model loading과 준비를 겹친다. activation은 선택 target의 현재 revision 상태만
확인하고 unrelated background pending을 기다리지 않는다. prepared Product attach는 catalog
revision/document identity와 shared immutable document를 재사용한다.

Source Trim `rotation`은 source module이 만든 initial sprite/source-mesh rotation과 source rotation-rate에 정확히 한 번 적용한다. authoring/codec/source validator 범위는 finite `[-360, 360]`이며 authored billboard roll lane은 별도 값으로 유지한다.

Character Select 내부 class 변경은 Server snapshot의 stable entity/class generation을 stage하고 새 class
Product target이 settle된 뒤에만 presentation을 교체한다. 준비 중 입력은 차단되고 기존
character는 유지되며, replacement transaction 실패는 Character Select 입력 정지 대신 Lobby로 복귀한다.

## 6. UI와 밸런스 데이터

UI가 바로 사용할 읽기 경계는 `CCombatHUDViewModel`이다.

`Get_Player()`가 제공하는 값:

- 최신 server tick
- current/max HP
- current/max resource
- current/max identity — class identity 게이지다. max가 0이면 그 class는 게이지가 없고 UI는 아무것도
  그리지 않는다. 현재 Warlord만 보유하며 방어 태세를 유지하는 동안 소모되고 0에서 기본 stance로
  자동 복귀한다. UI가 소모량이나 복귀를 자체 판정하지 않는다.
- server action
- skill ID, input slot, 표시 이름, action ID
- cooldown duration tick, cooldown end tick, 표시 damage

`Get_Boss()`가 제공하는 값:

- archetype ID와 표시 이름
- current/max HP
- phase
- server action과 action ID

`Get_DamageEvents()`는 최근 128개 Server `DAMAGE_EVENT`를 server tick과 함께 보관한다. 실제 적용
damage, target NetEntityId, world anchor, incoming/outgoing을 제공하며 UI가 HP 차이로 damage를
재계산하지 않는다. F1 Balance Tool은 이 경계로 최근 16개 event를 표시한다.

쿨타임 남은 tick은 `max(0, cooldownEndTick - serverTick)`이며 UI가 별도 timer를 정답으로 만들지 않는다. 표시 damage는 데이터 정의를 읽은 값이고 실제 피해 적용은 Server만 한다.

밸런스 정본:

| 파일 | 수정하는 값 | 주 소비자 |
|---|---|---|
| `Data/Balance/PlayerProfiles.json` | class별 max HP/resource/move speed, 기본 stance가 아닌 stance의 이동 배율, identity 게이지 최대치와 충전·소모 속도 | Server spawn, HUD snapshot |
| `Data/Balance/PlayerSkills.json` | slot, 이름, `skillKind`, cooldown, action/hit/`comboAdvanceMs`, cost, 이동 거리, range, damage 참조, `effectId`, `comboStages` | Server skill, UI definition, Effect presentation |
| `Data/Balance/DamageProfiles.json` | attack power에 곱하는 damage rate percent | Server 판정, UI 예상 표시 |
| `Data/Balance/BossProfiles.json` | boss HP, engage range, speed, phase threshold | Server boss, UI 이름 |
| `Data/Encounters/Valtan/ValtanEncounter.json` | state/action/pattern timing/range/damage 참조 | Server Valtan brain |
| `Data/Encounters/Valtan/ValtanCombatObjects.json` | pattern stage가 생성하는 지연/이동 객체의 stable ID, motion, life, hit | Server room combat-object runtime |
| `Data/Actors/BossCatalog.json`의 `combatObjectVisuals` | gameplay object ID + visual ID를 Product Effect ID에 연결 | Client replication/effect prewarm |

UI 담당자는 JSON을 매 프레임 읽지 않는다. `CCombatHUDViewModel::Initialize_Definitions()`가 정의를 준비하고 `CClientReplication`이 snapshot마다 runtime 상태를 적용한다. UI 코드에서 packet, socket, Character, boss GameObject를 직접 조회하지 않는다.

Debug F1 `Balance Tool`은 여섯 class와 발탄을 선택해 stats/movement/skill/combo/pattern을 편집하고,
field provenance와 Server snapshot/damage event를 같은 화면에서 검증한다. Save는 authoring JSON을
staging한 뒤 변경 field를 `PROJECT_TUNED`로 동기화하고 publisher Validate를 수행한다. Publish 후
Server 재시작이 필요하며 runtime Hot Reload 버튼은 없다. 세부 작업법은
`BALANCE_TOOL_OWNER_HANDOFF.md`가 정본이다.

### 6.1 ImGui authoring에서 제품 이미지 UI로 전환

ImGui는 최종 제품 UI가 아니라 layout authoring과 debug command를 위한 작업면이다. UI 담당자가
ImGui로 만든 창이나 버튼을 스크린샷으로 떠서 교체하는 것이 아니다. `CHUDLayoutTool`에서 실제
이미지 asset을 slot/layer에 연결하고, 저장된 JSON을 제품 런타임이 읽어 image widget을 만드는
방식으로 전환한다.

현재 정본과 구현 상태:

| 항목 | 현재 상태 |
|---|---|
| Combat HUD layout | `Data/UI/HUD/HUD_Layout.json`, asset domain `UI/HUD/` |
| Screen UI layout | `Data/UI/ScreenUI/ScreenUI.json`, asset domain `UI/ScreenUI/` |
| ImGui authoring | asset palette, thumbnail, drag/drop, rect/rotation, layer order, hover preview, save/load 구현 |
| runtime state | `CCombatHUDViewModel`과 임시 runtime HUD overlay 구현 |
| 최종 image widget 생성 | layout JSON을 `CUIObject` 계열로 만드는 factory는 미구현 |
| 제품 UI picking | screen-space input router와 command binding schema는 미구현 |

작성에서 실행까지의 목표 흐름은 하나다.

```text
Resources/UI image asset
-> CHUDLayoutTool (ImGui authoring)
-> Data/UI/*.json (stable slot.id + geometry + draw order + image asset ID)
-> runtime layout loader (parse -> validate -> stage -> commit)
-> CUIObject image widget tree
-> screen-space UI hit test
-> stable UI command
-> CLobbyCommandService / CLevelTransitionService / IPlayerCommandSink
-> Server snapshot when authority is required
-> CCombatHUDViewModel
-> widget presentation
```

저장과 asset 규칙:

- reference resolution은 현재 1280×720이며 viewport scale/letterbox 보정 뒤 같은 좌표계로
  draw와 hit test를 수행한다.
- `slot.id`가 stable widget identity다. pointer, vector index, ImGui label, 보이는 문자열을
  저장 ID로 사용하지 않는다.
- image는 `Client/Bin/Resources/UI/<Domain>/...`에 두고 JSON에는 `UI/...` 상대 asset ID만
  저장한다. 절대 경로, drive path, `..`, `Resources/LostArk` wrapper는 거부한다.
- render order와 picking order는 같은 계약을 사용한다. 뒤에서 앞으로 그리고, 겹친 widget은
  앞에서 뒤로 검사해 최상위 하나만 pointer를 소비한다.
- display-only HUD는 기본적으로 hit test하지 않는다. interactive widget은 향후 schema version
  갱신과 함께 `enabled/visible`, hit-test shape, stable command ID, pointer capture 정책을
  명시해야 한다. 현재 format version 1에는 runtime command binding이 없으므로 임의 문자열
  필드를 끼워 넣지 않는다.
- normal/hover/pressed/disabled 표현은 widget 상태로 선택하며 hover image 존재 여부가 command
  권한을 뜻하지 않는다. alpha-mask picking은 성능과 판정 harness가 있는 별도 옵션으로만
  추가하고 기본은 transformed rectangle hit test다.

UI picking은 3D 월드 ray를 쏘는 `CPicking`과 다른 기능이다. mouse viewport 좌표를 reference
resolution으로 변환하고 회전까지 반영한 slot rect를 검사한다. visible/enabled 상태가 아니거나
크기가 0인 widget은 hit 대상이 아니다. UI가 click을 소비한 프레임에는
`CGameInstance::SetInputBlocked()` 또는 동등한 단일 input arbitration 경계로 ground move와
gameplay click을 보내지 않는다.

interaction을 구현할 때 JSON의 stable command ID는 함수 이름이나 packet opcode가 아니다.
런타임 registry가 이를 typed UI command로 해석하고 Lobby 선택은 `CLobbyCommandService`, scene
이동은 `CLevelTransitionService`, gameplay action은 `IPlayerCommandSink`로 제출한다. UI가
packet을 조립하거나 socket을 호출하고, click callback에서 `Change_Level`이나 Character 상태를
직접 변경하는 것은 금지한다.

이 수직 슬라이스의 완료 검증에는 duplicate/unknown slot ID, unsafe/missing asset, 잘못된 rect와
rotation, unknown command, 겹침 시 topmost 선택, resolution/letterbox 보정, hidden/disabled 제외,
중간 load 실패 시 기존 UI 유지, UI 소비 click의 gameplay 차단이 포함되어야 한다. runtime
factory와 router가 생기기 전까지는 authoring tool의 save 성공만으로 제품 UI 전환 완료를
선언하지 않는다.

Git 관리 대상 데이터는 Visual Studio Client 프로젝트의 `96.DataFiles` 필터에서 원본을 바로 연다. 이 항목들은 `None` 링크이며 복사본이나 runtime 배포본이 아니다. 수치 튜닝 절차와 무중단 reload를 아직 활성화하지 않은 이유는 `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.

## 7. Valtan Boss

제품 발탄의 transform, target, action, phase, HP, damage는 Server authority다.

실행 중 boss pattern이 한 플레이어를 잠그면 Server의 stable `NetEntityId`가
`WORLD_ENTITY_SNAPSHOT::iPatternTargetNetEntityId`로 복제된다. 일반 NPC와 monster는 이
필드를 반드시 invalid로 둔다. Client는 대상을 다시 고르지 않으며, 62줄 잡기 표현은 이 ID로
찾은 Character만 `bip001-l-hand` animated socket에 붙였다가 success/recovery 또는 패턴 종료 시
Server snapshot transform으로 돌려놓는다.

```text
CGameRoom::Tick
-> CValtanBrain::Update
-> nearest living player acquire
-> IDLE / CHASE / PATTERN_WINDUP / PATTERN_ACTIVE / PATTERN_RECOVERY / DEAD
-> stage inline hit 또는 CCombatObjectRuntime spawn/update
-> Server hit resolver가 damage/status를 확정
-> reliable combat-object spawn/despawn + S2C_WORLD_SNAPSHOT full live set
-> CValtan presentation + world-root Effect + CCombatHUDViewModel
```

발탄 저작 정본은 `Data/Valtan`의 다음 다섯 split source다.

| source | 소유 내용 |
|---|---|
| `Valtan.gameplay.json` | Server pattern graph, decision, stage, action, hit, motion, volley |
| `Valtan.presentation.json` | animation occurrence, Effect invocation, camera invocation, cue scale policy |
| `Valtan.combatobjects.json` | combat-object origin, movement, hit geometry와 damage profile reference |
| `Valtan.worldeventsets.json` | stable world-event set membership |
| `Valtan.legacy-compatibility.json` | 아직 승격하지 않은 Product closure와 migration identity |

`Data/Valtan/Valtan.pattern.json`은 migration fixture이며 새 값을 저장하지 않는다. publisher가 다섯 source를 stable
ID로 strict join해 `ValtanEncounter.json`, rotations, combat objects, world events, pattern bindings/cues와 Server
bootstrap을 생성한다. 이 generated Product는 read-only이고 Server와 Arena가 split source를 두 번째 런타임으로
직접 읽지 않는다.

`Data/Actors/BossCatalog.json` format v4의 Valtan `presentationScale: 0.75`는 replicated Arena와 Character/Boss
Preview가 함께 소비하는 Client actor scale이다. `BossProfiles.json`의 Server collision radius와 hit geometry는
바꾸지 않는다. managed Effect cue 15개는 `OWNER_RELATIVE` 4개, `GAMEPLAY_FOOTPRINT` 9개,
`ARENA_ABSOLUTE` 2개로 분류한다. owner-relative만 actor scale을 상속하고, 나머지 두 policy는 owner scale을 제거한
뒤 authored `worldScale`을 사용한다. 이 정책은 호출 transform 계약이며 `Data/Effects/Authored`의 element geometry를
고치거나 sky-axe에 `0.75`를 강제하지 않는다. Client `CValtan`의 로컬 AI는 Development preview 외 제품 정답이
아니다. 세부 필드와 publish 절차는 `발탄인수인계서.md`를 따른다.

Effect 시각 기준의 global bloom scatter 정본은 `Data/Rendering/Authored/RenderingProfiles.json`의 exact
`bloomScatter: 1.0`이다. Rendering publisher는 float32 경계 검증 뒤 같은 값을 runtime JSON에 투영하며 Editor
Save 뒤 별도 Effect publish나 사용자 sidecar로 이 값을 다시 선택하지 않는다.

플레이어 profile의 defense는 발탄 incoming damage에 실제로 사용된다. 원작 Server 공식이 client
payload에 없으므로 `raw * 100 / (100 + defense)`는 `PROJECT_TUNED` 중앙 계약이며
`CGameplayCatalog::Apply_Defense` 한 곳에서만 계산한다. boss defense/outgoing 감산은 아직 없다.

수업용 `CMonster`와 `astar/Monster`는 제거 대상 레거시다. 제품 일반 몬스터는 Valtan archetype을 `MonsterCatalog.json`과 `MonsterProfiles.json`에 등록하고, Area `SpawnGroups.world.json` → publisher → Server `CSpawnGroupRuntime/CMonsterBrain` → Shared world entity spawn/snapshot/despawn → Client catalog presentation 경로를 사용한다. `MonsterProfiles.json` formatVersion 2의 `targetReleaseRange`, `turnSpeedDegreesPerSecond`, `acceleration`, `deceleration`, `arrivalSlowRadius`가 Server 권위 추적 유지·회전·가감속 정본이며 publisher는 이를 spawn-group bootstrap v4로 투영한다. Brain은 살아 있는 현재 타깃을 release range 안에서 유지하고 WINDUP부터 RECOVERY까지 타깃과 공격 방향을 고정하며, Server navigation 경로 단축과 기존 원형 body sweep/slide를 이동·knockback에 함께 사용한다. 레거시 클래스를 이 계약에 다시 연결하지 않는다.

Client 일반 몬스터 표현은 root-motion 억제와 network-transform 보간을 독립 정책으로 사용한다. Server 권위 몬스터는 기존 2-tick `CNpcNetworkTransformInterpolator`로 표시하고, `WINDUP` occurrence에서 공격 clip을 0초부터 한 번 시작한 뒤 `ACTIVE/RECOVERY` 동안 같은 clip을 이어서 재생한다. 다음 `WINDUP`은 같은 clip이어도 새 occurrence로 다시 시작한다. `MonsterCatalog.json` formatVersion 2의 presentation-only `attackPresentations[]`는 실제 clip과 playback rate를 소유한다. Client는 Server entity ID와 occurrence tick으로 pool을 결정적으로 선택하므로 Server timing과 Client 간 일치성을 바꾸지 않는다. IDLE/CHASE 중 damage event는 catalog hit clip을 짧게 재생한 뒤 이전 locomotion으로 복귀하고 ATTACK/DEAD는 덮어쓰지 않는다. Valtan Loader는 spawn 전에 지원 archetype prototype을 미리 준비하고, 누락 모델/clip은 해당 archetype/action 표현만 격리한다.

combat body와 공격 footprint는 `Shared/Public/Gameplay/CombatCollisionContract.h`와 Server combat-object geometry의 pure XZ primitive로 평가한다. 플레이어 스킬은 target body radius, 일반 몬스터와 Lugaru는 player footprint, Valtan inline stage는 circle/ring/cone/forward-box/cross, 이동 검기는 swept circle과 player footprint의 교차를 Server fixed tick에서 판정한다. HP·damage·counter·death는 같은 typed Server hit resolver 순서를 유지한다. Client `CCollider`와 world-root Effect는 Debug/visual projection일 뿐 damage 판정이나 PhysX 권위가 아니다.

## 8. MapTool과 gameplay 저장

정적 visual 배치와 gameplay 배치는 분리한다.

- visual import definition: `Data/Maps/Imported/<AreaId>/`
- visual authoring: `Data/Maps/Authoring/<AreaId>/`
- gameplay authoring: `Data/Worlds/<AreaId>/Gameplay.world.json`
- monster wave authoring: `Data/Worlds/<AreaId>/SpawnGroups.world.json`
- navigation authoring: `Data/Navigation/<AreaId>.navsource/.navpaint/.navblockers` 또는 uniform `<AreaId>.navgrid.json`
- Client map/navigation 생성물: `Client/Bin/DataFiles/Map`, `Client/Bin/DataFiles/Navigation`
- Server world 생성물: `Server/Bin/DataFiles/World/*.worldbootstrap`, `*.spawngroupsbootstrap`
- Client NPC 표현 생성물: `Client/Bin/DataFiles/World/*.npcpresentation.json`

`Data`만 사람이 편집하거나 재추출 결과를 반영하는 정본이다. Client/Server `Bin/DataFiles`는
publisher 출력이며 source로 다시 읽어 authoring을 갱신하지 않는다. Visual Studio의
`96.DataFiles` 필터도 `Data` 원본만 연결한다.

Debug Map Editor 사용 절차는 다음과 같다.

1. Debug Lobby에서 `Test`를 누르고 기존 Server 승인을 기다린다.
2. `LEVEL::DEVELOPMENT`의 격리된 editor shell 로드가 끝날 때까지 기다린다.
3. F1로 공통 Developer Tools를 열고 `Map Tool`을 선택한다.
4. Area selector에서 Character Select, Bern, Valtan, Training Map 중 하나를 고른다.
5. visual placement 또는 정책이 허용한 gameplay/navigation만 편집한다.
6. Area 전환·reload·Lobby 복귀 전에 dirty gate에서 Save/Discard/Cancel을 결정한다.
7. 저장 뒤 필요한 publisher를 별도로 Validate/Publish한다.

제품 `CLevelRegistry`는 제품 Level의 `MAP_LOAD_SCOPE`만 소유한다. Map Editor의 대상과
authoring policy는 별도 descriptor이며 제품 Level에서 MapTool authoring을 활성화하지 않는다.
Debug Lobby Test의 승인 소비자가 editor intent를 세운다. F1/Map Tool은 전환 권한이 없고
도구 표시와 선택만 담당한다. Release Test에는 Debug editor intent가 없으므로 기존 training
map, 서버 승인 character, replication, controller 경로를 유지한다.

Editor Area 정책은 `AREA_DATA_LAYER_GUIDE.md` 4절이 정본이다. 특히 Character Select와
원본 Training Map은 gameplay 문서를 만들지 않는다. Bern은 명시된 source/paint 경로에서
Nav Bounds bootstrap만 허용하며 실제 bake 검증 전에는 Server 제품 navigation으로 취급하지
않는다. 원본 Training Map은 navigation 문서를 추측 생성하지 않는다. Valtan DeployProp과 World
Destruction은 Debug authoring/preview로 편집할 수 있지만 제품 destroyable publisher gate와는 분리한다.

gameplay authoring은 formatVersion 6이다. 제품 publisher/runtime는 현재 `playerSpawn`, `npc`, `boss`,
단일 `movePlayer`, `changeLevel`, `activateSpawnGroup`, `activateEncounter` action을 가진 `triggerBox`, 정적 `collisionBox`를 admission한다.
placement에는 stable placement ID, kind, encounter ID, position, yaw, enabled를 저장한다.
NPC/boss는 stable archetype ID를 소유하지만 `playerSpawn`의 `archetypeId`와 `encounterId`는 `null`이며 실제 class는
session/player selection이 소유한다. NetEntityId, pointer, Prototype tag, vector index, runtime HP/phase를
저장하지 않는다.

v6 `CWorldGameplayDocument`에서 NPC는 optional `behavior`로 stationary/patrol/wander,
waypoint, 속도, timing과 semantic action을 저작한다. `behavior: null`은 기존 정적 idle과 같다.
publisher는 같은 transaction에서 Server `worldbootstrap` v7에는 이동·행동 의미만,
Client `npcpresentation` v2에는 placement별 idle/walk/action clip만 생성한다. Server가 30 Hz
transform/action을 확정하고 Client는 `(actionId, actionStartTick)` edge와 snapshot 보간으로 표현한다.
Map Tool 변경은 `Apply NPC Behavior -> Save Gameplay -> Publish-WorldGameplay -> Server restart`
전에는 실행 중 제품 room에 반영되지 않는다.

같은 v6 문서에서 `triggerBox`는 half extents, once 정책, typed event를 소유하고
`collisionBox`는 transform, half extents, enabled만 소유한다.
제품 event는 `movePlayer`, `changeLevel`, `activateSpawnGroup`, `activateEncounter`다. movePlayer는 `targetPosition`, `durationSeconds`, `arcHeight`를 저장한다. `activateSpawnGroup`은 `SpawnGroups.world.json`의 stable group ID, `activateEncounter`는 같은 gameplay 문서의 disabled boss placement ID만 저장한다.
Server는 yaw OBB enter edge를 판정하고 일반 이동/스킬을 중단한 뒤 30 Hz 직선·포물선 이동을 확정한다.
일반 보행은 player OBB를 반영한 swept collision으로 정적 collisionBox 앞에서 정지한다.
Shared `TRIGGER_MOVE` action과 player transform snapshot이 Client 표현의 유일한 입력이다. Client는 별도
jump clip 계약이 생기기 전까지 RUN locomotion으로 이동을 표현한다. `destroyable`과 나머지 trigger event는
여전히 publisher가 fail-closed로 거부하며, parser 존재만으로 제품 runtime 지원 완료라고 판단하지 않는다.

`changeLevel`은 Bern과 Valtan Arena 사이 target world만 저장한다. Server가 source room leave와 target room
enter를 처리하고 새 `S2C_ENTER_ACCEPTED`를 보낸 뒤에만 Client가 `CLevelTransitionService`로 전환한다.

Debug Development MapTool은 action이 아직 없는 `triggerBox`를 disabled draft로 배치하고 position,
yaw, half extents, once 정책을 편집한다. 선택한 box에 지원 action 하나를 선택하고,
movePlayer는 목적지를 맵에서 pick한 뒤 duration/arc를 정해야 enabled로 저장할 수 있다. wire box는 저작용 presentation이며 overlap/action 권위는
Server에만 있다. Save 뒤 publisher와 Server 재시작 전에는 제품 월드에 적용된 것이 아니다.

MapTool `Spawn Groups` panel은 anchor, group, prerequisite, maxAlive, wave, entry의 archetype/count/delay를 별도 dirty 상태로 편집한다. Trigger Box는 이 정의를 복제하지 않고 group ID만 참조한다. Valtan Stage 1 → Lugaru → Stage 3은 prerequisite 완료 뒤에만 다음 group activation이 성공하며, 마지막 boss trigger는 disabled `boss.valtan.center`를 활성화한다.

같은 panel의 `Collision Box` option은 표면 pick, position, yaw, half extents, enabled, 목록 선택과
delete를 제공한다. 파란 wire OBB는 저작 표시일 뿐이며 실제 차단은 Server bootstrap을 읽은 뒤 적용된다.
`NpcCatalog.json`의 supported archetype은 현재 75종이다. Bern authoring은 전체 placement 16개 중
NPC 10개를 보존하며 초기 v6 migration에서는 모두 `behavior: null`이다. 특정 Aylara/Beda
placement를 pre-authored 행동 샘플로 전제하지 않는다. enabled NPC는
`NpcCatalog.json -> Server world entity -> Client replication -> CNpc` 경로로 표시한다.

Valtan `Destruction Model View`의 Debug 경계는 다음과 같다.

```text
destructionsimulation.json
  -> CDestructionSimulationController
  -> CDestructionSimulationRuntime
  -> CDeployPropObject의 source/proxy presentation
  -> CRigidBody -> CPhysics_Manager -> PhysX
```

source placement 하나가 Wall Mesh Emitter 하나이고 runtime이 stable fragment ID 12개를 파생한다.
MapTool은 All Fragments/Solo Emitter/Solo Fragment와 60 Hz deterministic seek를 제공한다. 이 결과는
Server truth가 아니며 Client PhysX pose를 Server로 보내지 않는다. preview ground도 editor support일 뿐
`Gameplay.world.json collisionBox`가 아니다. persistent FRACTURED full sync는 과거 debris를 재생하지
않고, live event만 one-shot mesh debris/effect cue를 만든다. 삭제된 Effect editor private type이나
active document를 MapTool 또는 제품 runtime에 의존시키지 않는다. 상세 작업법은 `MAP_DESTRUCTION_PHYSX_HANDOFF.md`를
따른다.

Map/Encounter 담당자가 좌표를 수정하면 navigation publish가 활성 playerSpawn/boss 좌표의 walkable cell과 높이 오차를 검사한다. 생성된 Server bootstrap/navgrid를 직접 편집하지 않는다.

Area별 optional layer와 현재 Bern/Valtan/Training 데이터 보유 현황은 `AREA_DATA_LAYER_GUIDE.md`를 정본으로 사용한다.

## 9. 데이터 변경 절차

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
```

네 publisher는 parse → validate → stage → commit을 따른다. unknown field, schema/version 오류, 중복 stable ID, 잘못된 참조, non-finite 위치, navigation 밖 spawn을 정상값으로 보정해 숨기지 않고 실패시킨다.

새 스킬을 추가할 때는 다음을 함께 변경한다.

1. `PlayerSkills.json`과 참조 `DamageProfiles.json`. 비어 있지 않은 `effectId`는 Client의
   canonical source/진단 참조이므로 stable ID여야 하고 Server bootstrap에는 싣지 않는다.
   실제 clip별 Product Effect는 `skillbindings`가 소유한 clip의 `effectref=asset` animevent가
   가리키는 Authored Effect다. `effectId`가 비어 있다는 이유만으로 실제 Product cue를 추측하거나
   반대로 source/imported 문서를 제품 재생 대상으로 승격하지 않는다.
   다중 clip stage도 첫 clip의 재생률로 하나의 Effect 문서를 진행하지 않는다. 시각 요소가 있는
   각 clip이 clip-local Product cue를 소유하고 기존 Character의 `playMs`, `playRate`, loop와
   authoritative late-catch-up을 소비한다. Character gameplay 준비는 이 source-admitted cue target
   집합을 metadata-only로 등록하고 첫 화면 frame을 양보한 뒤 target 하나/frame으로 준비한다. 성공한
   target만 catalog revision 단위 prepared set에 들어가며, 전투 Update의 Product Spawn은 cache-only
   document lookup과 prepared bundle만 붙인다. prepared miss에서 JSON/shader/model/DDS/vector-field
   load 또는 synchronous document stage를 수행하지 않는다. 준비 전 발생해 fail-closed한 occurrence는
   준비 완료 뒤 소급 재생하지 않는다.
2. Character presentation의 stable action/skill mapping
3. 필요한 Shared message/snapshot 확장
4. Server validation/action/damage 처리
5. `CCombatHUDViewModel` 소비 확인
6. protocol harness와 `Server.exe --contract-test`

## 10. Asset과 Git

`Client/Bin/Resources`는 `Fonts, Character, Deploy, Effect, Map, UI` 여섯 root만 허용한다. asset ID는 Resources 상대 경로이며 절대 경로, drive-qualified 경로, `..` 탈출을 금지한다.

runtime payload는 기본적으로 팀장이 `Client/Bin/Resources` 물리 폴더로 관리한다. pull-only 재현이 명시된 feature는 Product가 실제 참조하는 최소 dependency closure만 Git/LFS에 포함할 수 있다. AssetPacks lock, immutable manifest, ZIP hash, Snapshot/Publish/Hydrate/Verify를 팀 완료 조건으로 사용하지 않는다. 코드와 데이터에는 Resources 상대 asset ID만 저장하고 팀원별 절대 경로 하드코딩은 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ 팀장이 전달한 Resources 물리 폴더 확인
→ Debug 전체 회귀
→ 담당 public interface에서 작업 시작
```

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, 소비 데이터, project/filter 등록, harness, RESULT를 같은 검증 단위로 묶는다. build output, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, 그리고 승인된 최소 dependency closure 밖의 Resources payload를 stage하지 않는다.

## 11. 완료 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

자동화 순서는 Engine → UpdateLib → Shared/Protocol Harness → Server build/contract test → Client build → balance/world/navigation/rendering publisher validate + Effect source validate → 변경 domain의 실행형 harness이다. 실제 Level 흐름은 `Framework.slnLaunch`로 Server와 Client를 함께 실행해 검증한다.

최소 성공 증거:

- Protocol Harness `failures : 0`
- Server gameplay contract `failures : 0`
- Debug/Release Client와 Server 빌드 성공
- 실제 Server+Client에서 Lobby → Character Select 승인 진입 → class 연속 변경/스킬 → Lobby → Bern/Valtan 진입 확인
- 연결 실패 시 Lobby 유지와 제품 Level disconnect 후 Lobby 복귀 확인
- 변경 domain의 publisher 또는 source validator와 실행형 harness 성공
- `git diff --check` 성공
- 잔류 Client/Server/7777 listener 없음

## 12. 현재 완료와 다음 경계

완료:

- 서버 권위 우클릭 이동과 Navigation path
- 여섯 class 전체 quick slot과 LMB COMBO의 command, server approval, action/damage/cooldown/resource
- snapshot 기반 Character skill/locomotion 표현
- Animation Tool의 data-driven key/skill → ordered clip/BA stage authoring, atomic Save, safe action-boundary reload
- HUD용 player/boss runtime ViewModel
- Valtan 추적, pattern, damage, phase, death
- player projectile와 발탄 하늘 도끼·붉은 검기의 단일 room-owned combat-object runtime, reliable lifecycle/full snapshot, world-root Effect projection
- world gameplay와 navigation 배치 정합성 검사
- Valtan Debug MapTool의 12-piece Mesh Emitter PhysX audition과 All/Emitter/Fragment Solo
- `dev.training.ground` 최소 Area, class-neutral player spawn, RCArena 10종 admission, 서버 navigation
- Lobby의 Lance Master/Gunslinger/Slayer/Artist/DimensionMaster/Warlord 여섯 선택 slot, Lobby 승인 Character Select visual map, Server-authoritative class 변경, 여섯 class Loader/Server profile과 runtime HUD. DimensionMaster는 combined body와 L/S/P/E 네 정적 기본 무기 파츠를 사용하며 runtime payload는 팀장 관리 Resources 물리 폴더를 사용한다.
- Character Select `Create Character`의 process-session pending/created nickname transaction, Server-approved Bern commit, created/audition direct Valtan identity, Bern/Valtan Server-replicated nameplate

별도 수직 슬라이스:

- 이동기·스탠스 전환과 skill별 weapon visibility/reattach
- `Data/UI` layout에서 `CUIObject` image widget을 생성하는 runtime factory
- 1280×720 reference 좌표 보정, draw-order 기반 2D UI picking과 input arbitration
- stable UI command binding과 Lobby/Scene/Gameplay typed command service 연결
- 추가 스킬
- party/raid admission과 roster
- 동적 capsule obstacle collision, prop 차폐·개별 파괴와 knockback obstacle 판정
- 잡몹 및 추가 boss pattern
- Valtan destroyable publisher, Server 상태/동적 collision·navigation, Shared replication과 제품 debris/effect cue

이 항목들은 현재 인터페이스를 우회해 임시 구현하지 않는다.
