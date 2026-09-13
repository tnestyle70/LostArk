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

Client는 입력을 빠르게 제출하지만 위치, damage, cooldown, HP, boss phase를 확정하지 않는다. Server가 확정한 snapshot이 권위 상태다. 일반 클릭 이동의 자기 캐릭터 표시는 입력 직후 예측할 수 있으며 Server snapshot으로 보정한다.

Lobby는 `Test`, `Character Select`, `Valtan`, `KoukuSaydon`, `Bern` 다섯 명령만 제공한다. Character Select는 Lobby가 `WORLD_ID::CHARACTER_SELECT_ARENA` 승인 payload를 검증한 뒤 기존 socket을 one-shot handoff하여 같은 visual map을 여는 Server 전용 Level이다. offline Preview와 `Preview / Server Play` mode 선택은 없다. Level은 직접 connect/send하지 않고 `CClientReplication`, `CNetworkPlayerCommandSink`, `CPlayerController`로 HUD·우클릭 이동·quick-slot 스킬을 Server snapshot에 연결한다. class thumbnail 선택은 target asset admission 뒤 즉시 typed class-change command를 제출하며, Server 승인 snapshot이 같은 entity presentation과 skill catalog class를 교체한다. 살아 있는 위치와 identity는 유지하고 전투 상태는 새 profile로 초기화하며, 사망 중 변경은 원래 projected spawn에서 부활한다. 연결 실패·거부·5초 timeout은 Lobby에 남고 disconnect는 Lobby로 복귀하며 자동 local gameplay fallback은 없다. Debug ImGui의 일반 몬스터, `MINIBOSS_LUGARU`, Valtan 선택은 `IWorldEntityCommandSink`를 통해 stable SpawnGroup/placement ID만 제출한다. Server는 Character Select Area 문서, navigation, profile을 검증해 실제 entity 생성이 성공한 뒤 활성화 결과를 회신하고 기존 monster brain 또는 Valtan brain으로 broadcast하며 Client local spawn은 없다. 마지막 플레이어가 퇴장하면 동적 audition entity와 SpawnGroup 상태를 초기화해 다음 입장을 새 세대로 시작한다. Bern/Valtan map 진입도 마지막 Server 승인 class로 Lobby Server 승인이 필수다.

Lobby의 `KoukuSaydon` 버튼은 기존 `CLobbyCommandService -> C2S_ENTER_WORLD -> S2C_ENTER_ACCEPTED` 경로로 입장하며, 선택 class와 created/audition nickname을 발탄과 같은 규칙으로 사용한다. Character Select ImGui의 `Enter KoukuSaydon Arena`는 현재 session의 기존 typed world transfer를 사용한다.

`CHARACTER_SELECT_ARENA`의 gameplay authority는 Server에 남지만 simulation ownership은 session-private이다. 각 session은 자기 `CGameRoom`의 player, audition entity, HP와 damage event만 snapshot으로 받고 다른 Character Select session과 만나지 않는다. 퇴장한 session의 private room은 queued `LEAVE`와 reset을 처리한 뒤 폐기한다. Bern, Valtan, Training Ground는 기존처럼 world별 shared room이므로 그 안에서는 여러 player가 같은 authoritative 상태를 본다.

Character Select의 `Create Character`는 선택 class와 공통 validator를 통과한 1~32-byte UTF-8 nickname을 `CCharacterSelectionState`의 pending identity로 stage한다. Lobby가 그 exact identity로 Bern entry를 승인받고 loading resource, rendering profile, 실제 `Change_Level(BERN)`까지 성공한 뒤에만 created identity로 commit한다. 중간 실패는 pending만 취소하고 기존 created identity는 유지한다. created identity가 없는 direct Character Select, Training, Valtan entry는 process-local `Test-<process-id>` audition nickname을 사용한다. Server의 `SERVER_PLAYER::strNickName`과 world transfer가 session lifetime 동안 exact nickname을 보존하고 `S2C_PLAYER_SPAWNED`로 복제한다. nickname은 display text이며 player lookup, Party member ID, 고유성 검사 또는 Client 재실행 뒤 영구 저장에 사용하지 않는다. Bern과 Valtan은 `CClientReplication::Collect_PlayerViews`의 Server-replicated nickname과 weak character presentation을 `CWorldPlayerNameplateView`에 전달한다. projection, UTF-8 변환, font draw 실패는 gameplay와 replication을 건드리지 않고 해당 nameplate만 생략한다.

2026-09-30 23:59 KST까지 공유 LAN Server는 같은 팀 LAN의 `192.168.0.14:7777`이다. Server PC는 현재 `Wi-Fi 2`에서 `192.168.0.14/24`를 소유한다. Server는 `0.0.0.0:7777`에 수신하고 Server PC와 다른 PC의 Client는 모두 concrete endpoint `192.168.0.14:7777`을 사용한다. `Tools/Network/TeamLanEndpoint.json`이 endpoint와 만료일 정본이다. 각 에이전트는 pull 후 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행하고 출력된 역할에 맞는 target을 안내하며, 실제 `Ctrl+F5` 시작과 UI 조작은 사용자가 수행한다.

쿠크 아레나의 광기와 네 HUD는 `CCombatHUDViewModel::Get_KoukuGimmick()`을 읽는다. v63
`PLAYER_SNAPSHOT`의 madness, `eKoukuHudMode`, cooldown 종료 tick과 카드 문양·색이 실제 상태를 소유한다.
광기 100% 및 F1 `Clown > Change to Clown`은 `POLYMORPH`(표시 이름 Clown)를 사용하고,
Mario/Dance/Card Maze는 각각 `MARIO/DANCE/MAZE`를 사용한다. 모든 interaction 스킬 쿨타임은 3초다.
`CPlayerController -> IPlayerCommandSink -> Server -> snapshot -> CCharacter`가 입력·판정·애니메이션을
연결하며 이 모드에서는 기존 class quick-slot과 평타를 보내지 않는다. QWER 춤 순서는 양팔 모으기,
슈퍼맨, 양팔 벌리기, 한 다리 올리기다. 첫 오답은 fail, 유효 입력 없이 창 종료는 timeout이다.
F1의 mode 선택은 typed Debug 명령으로 실제 모델·HUD·스킬을 바꾸고 Return to Player는 원래
class로 복귀한다. 별도 `Kouku UI Preview`는 표시 전용 override이며 해제하면 실제 snapshot으로 돌아간다.
Mario1의 `Mario1_go`/`Mario1_Trigger_5`는 이동 도착 시에만 mode를 전환한다. movePlayer event의
optional `koukuHudMode`는 `MARIO/MAZE/NONE`이고 Server가 이동 성공 후 적용한다. Card Maze Debug gate는
`(0.09,-0.01,1351.48)`에 플레이어만 이동시키고 보스를 생성하지 않는다. Mario2~4 진입점은 미등록이다.
Clown 본체는 `Character/KoukuSaton/MN_RPCZ_00-1/MN_RPCZ_00-1.wmodel`이며 admission scale은
`0.017 × 0.709`다. 별도 프라이팬 `IT_GSTFP_00` 장착 part를 제외했고 본체와 망치 몸동작은 유지한다.
`Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json`이 mode/index별 clip을 소유한다.
G1 Saydon 활성 동안 Server가 카드 문양 4종 × RED/BLACK 중 하나를 배정해 복제하며 G1 종료/퇴장에
NONE으로 정리한다. Client는 8개 `boss.kouku.card.*` V2 group을 머리 위에 표시하고 카드 상태를 생성하지 않는다.
쿠크의 사망 화면은 기존 DeadScene UI와 typed revive 명령을 사용한다. Server는 사망 XZ의
walkable 지면으로 부활시키며, 더 이상 유효하지 않은 지점은 기존 navigation projection으로 보정한다.
캐릭터 정보창과 아바타 도감은 아레나의 현재 복제 캐릭터를 읽고, 같은 class의 광대 교체도 목록 갱신 경계로 본다.

### 1.1 서로 다른 장소에서 Server와 Client 연결

빠른 endpoint 교체와 실제 4인 LAN, loopback 격리 테스트의 실행 체크리스트는
[네트워크연결가이드.md](네트워크연결가이드.md)를 따른다.

Server와 Client가 같은 PC, 같은 LAN, 서로 다른 네트워크 중 어디에 있는지 먼저 구분한다.

| 실행 위치 | Server `--bind-address` | Client `LOSTARK_SERVER_HOST` |
|---|---|---|
| 현재 팀 LAN 공유 Server | `0.0.0.0` | `192.168.0.14` |
| 같은 PC 격리 harness | 명시적 `127.0.0.1` | 명시적 `127.0.0.1` |
| 서로 다른 장소/VPN | `0.0.0.0` | Server PC가 실제 소유한 VPN IPv4와 별도 source CIDR 방화벽 규칙 |
| 일반 NAT 공인 IPv4 | 현 단일 endpoint sync 미지원 | advertised endpoint와 Server owner address 분리 구현을 먼저 추가 |

`192.168.x.x`, `10.x.x.x`, `172.16.x.x`~`172.31.x.x`는 사설 주소다. 서로 다른 장소의 Client는 Server PC의 Wi-Fi 사설 주소로 직접 접속할 수 없다. 팀 테스트는 Server PC가 실제 소유한 VPN IPv4를 우선 사용하되, sync가 생성하는 `RemoteAddress LocalSubnet` 규칙을 `Any`로 넓히지 않고 승인된 VPN source CIDR을 별도 규칙으로 허용한다. 일반 NAT의 공인 IPv4는 공유기가 소유하므로 현재 `Sync-TeamLanEndpoint.ps1`의 Server 소유권 검사를 통과할 수 없다. 공인 노출은 공인 advertised endpoint와 Server owner address를 분리하는 구현, TCP `7777` 포트포워딩과 승인된 exact remote IPv4/CIDR 방화벽 계약을 별도 작업으로 먼저 추가한다.

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
<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.0.14</LocalDebuggerEnvironment>
```

`0.0.0.0`은 Server의 수신 주소일 뿐 Client 접속 주소로 사용하지 않는다. 현재 Client 기본값은 `192.168.0.14`이다. 주소를 바꾸면 `Tools/Network/TeamLanEndpoint.json`, Server/Client 코드 기본값, 공유 debugger 설정과 이 사용서를 같은 변경 단위에서 갱신하고 `Sync-TeamLanEndpoint.ps1`, NetworkProtocolHarness, Server contract test로 검증한다.

#### pull 후 공유 Server에 들어가는 순서

Server PC와 Client PC는 먼저 같은 commit과 생성 데이터를 맞춘다. 기능 브랜치를 검증할 때도 양쪽이 같은 변경을 사용해야 한다. `pull`만 하고 예전 실행 파일을 쓰면 현재 protocol v81 또는 Debug gameplay revision이 달라 Server가 연결을 종료할 수 있다. Server/Client/Shared는 항상 같은 protocol version으로 다시 빌드한다.

```powershell
git switch main
git pull --ff-only origin main
git lfs pull
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git rev-parse HEAD
```

두 PC의 `git rev-parse HEAD`가 같아야 한다. `Client/Bin/Resources`의 `Fonts, Character, Deploy, Effect, Map, Sound, UI` 일곱 물리 폴더도 팀장이 전달한 같은 runtime 입력이어야 하며, Git에 없는 UI/Character/Map/Sound 리소스는 별도 전달을 먼저 완료한다. Debug configuration으로 공유할 때는 두 PC 모두 Debug 정본 빌드를 실행하고, Server를 중지한 상태에서 Server PC가 `Server/Bin/Debug/Server.exe --reset-valtan-runtime-to-packaged`를 한 번 통과시킨다. cross-PC Debug Hot Reload candidate 공유는 지원하지 않는다.

동기화 뒤 Visual Studio project를 Reload하거나 IDE를 재시작한다. Server PC에서 `Machine role: server-host`를 확인하고 `Server + Client` profile을 시작한다. 다른 PC는 `Machine role: client`를 확인하고 `Client Only (Server Already Running)` profile을 시작한다. 직접 EXE를 실행할 때 shell의 오래된 `LOSTARK_SERVER_HOST`가 새 기본값보다 우선하므로 값이 `127.0.0.1`이면 제거하거나 `192.168.0.14`로 맞춘다.

Server가 `Listening on 0.0.0.0:7777`을 출력한 뒤 다른 PC에서 아래 probe가 성공해야 한다. Windows 네트워크가 `공용`이어도 repository sync가 검증하는 firewall rule은 `Profile Any`, `RemoteAddress LocalSubnet`이므로 직접 IPv4 접속에는 문제가 없다.

```powershell
Test-NetConnection 192.168.0.14 -Port 7777
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
`peerAddress:peerPort`와 정확히 일치한다. 이미 끊어진 TCP는 Server-only 원인을 Lobby로
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
$connectedInterfaceIndexes = @(
    Get-NetIPInterface -AddressFamily IPv4 |
        Where-Object { $_.ConnectionState -eq 'Connected' } |
        ForEach-Object { $_.InterfaceIndex }
)
Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object {
        $_.AddressState -eq 'Preferred' -and
        $connectedInterfaceIndexes -contains $_.InterfaceIndex -and
        $_.IPAddress -notlike '127.*'
    } |
    Select-Object InterfaceAlias, IPAddress
```

설정 후 Visual Studio는 `.vcxproj.user`를 메모리에 캐시할 수 있으므로 Server와 Client project를 Reload하거나 Visual Studio를 재시작한다. 그다음 Server PC와 Client PC에서 각각 확인한다.

`Sync-TeamLanEndpoint.ps1`은 `2026-09-30 23:59 KST`가 지나면 실패한다. 2026-10-01 이후 첫
세션은 `-AllowExpired`로 계속 쓰지 않고 새 endpoint 또는 loopback 복귀 계약을 먼저 정한다.

```powershell
# Server PC의 로컬 bind-only smoke. 500ms 후 종료되므로 remote probe용이 아니다.
Server\Bin\Debug\Server.exe --bind-address 0.0.0.0 --smoke-timeout-ms 500
```

실제 cross-PC probe 전에는 Server PC에서 `--smoke-timeout-ms` 없이 `Server + Client`
profile을 정상 시작해 listener를 계속 유지한다. 그다음 Client PC에서 확인한다.

```powershell
Test-NetConnection 192.168.0.14 -Port 7777
```

`Failed to open TCP listener ... Error=10049`는 `--bind-address`에 적은 주소가 현재 Server PC의 어느 어댑터에도 없다는 뜻이다. Client의 주소나 이전 Wi-Fi 주소를 Server bind 값으로 복사하지 말고 Server는 `0.0.0.0`, Client만 도달 가능한 endpoint를 사용한다.

### 1.2 같은 방 파티와 Bern → Valtan 이동

Bern/Valtan에서 다른 플레이어를 우클릭해 초대하고 상대가 수락하면 최대 4인의
Server-owned 파티가 된다. `IPlayerCommandSink`가 typed invite/respond를 제출하고
`S2C_PARTY_ROSTER`의 배열 첫 member가 leader다. nickname은 표시용이며 초대·roster는
현재 방의 NetEntityId로 식별한다. 옛 초대 응답은 교체된 새 초대를 소비하지 않는다.

Bern의 발탄 안내 NPC entry는 leader만 전체 파티를 요청할 수 있다. Server는 모든
member의 source session/binding, 목적지 자리·profile·navigation 및 초기 reliable
송신 준비를 검증한 뒤 한 batch로 이동한다. 이 준비 중 실패하면 기존 파티와 위치·HP를
유지하고 `S2C_PARTY_TRANSFER_RESULT`로 이유를 알린다. 실패를 Lobby admission 거절로
처리하지 않는다. 성공은 기존 `S2C_ENTER_ACCEPTED`만 사용하며 새 방의 NetEntityId로
roster와 leader를 재구성한다. 실제 commit 뒤 발생하는 연결 종료는 일반 disconnect
정리 경로를 따른다. 전역 영구 Party ID나 재접속 복구 계약은 아니다.

파티 HP는 같은 world snapshot에서 NetEntityId로 연결한 값만 표시한다. 아직 HP를
받지 못했으면 이름은 유지하되 체력을 100%로 꾸미지 않는다. 파티/NPC 메뉴에서 소비한
마우스 버튼은 물리적으로 놓을 때까지 이동·공격 입력으로 다시 해석하지 않는다.

## 2. 팀원이 먼저 읽을 파일

| 담당 | 시작 파일 | 데이터 정본 |
|---|---|---|
| UI | `Client/Public/CombatHUDViewModel.h`, `LobbyCommandService.h`, `LevelTransitionService.h` | `Data/UI`, `Data/Balance` |
| Player/Input | `Client/Public/PlayerController.h`, `PlayerCommandSink.h` | `Data/Balance/PlayerSkills.json` |
| Character/Animation | `Client/Public/Character.h`, `CharacterSpec.h`, `AnimationTargetService.h` | `Data/Actors`, `Data/Animation` |
| Server/Player | `Server/Public/GameRoom.h`, `PlayerSkillSystem.h`, `ServerNavigation.h` | `Data/Balance`, `Data/Navigation`, `Data/Worlds` |
| Boss | `Server/Public/ValtanBrain.h`, `ServerWorldEntity.h` | `Data/Balance/BossProfiles.json`, `Data/Encounters` |
| Boss Composition/Sequencer | `Client/Public/BossCompositionDocument.h`, `Client/Private/SequencerTool.cpp`, `Client/Public/ValtanActionWorkbench.h` | `Data/Compositions`와 manifest가 참조하는 `CValtanActionWorkbench` typed owner |
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

`CPlayerController`는 edge input, quick slot, sequence, aim만 만든다. `(class, inputSlot) -> skill ID`는 `CPlayerSkillCatalog`가 `Data/Balance/PlayerSkills.json`에서 해석한다. `IPlayerCommandSink`가 전송 구현을 숨기므로 Controller에서 `CNetworkManager`를 include하지 않는다. Controller는 Transform을 직접 변경하거나 `Play_Skill`을 호출하지 않는다. 일반 이동 명령의 typed sink 송신이 성공하면 `CCharacter::Predict_NetworkMoveGoal(sequence, goal)`로 자기 캐릭터의 표시 예측을 요청한다.

일반 클릭 이동 예측은 `CCharacter`의 기존 `CNavigation/CNavPathFollower`와 `CLocalMovePrediction`이 소유한다. 송신 직후 RUN을 요청하고 다음 ObjectUpdate부터 위치를 전진시킨다. `ClientReplication`은 protocol 81 `PLAYER_SNAPSHOT`의 `iLastProcessedMoveSequence`, `fMoveSpeed`(태세 배율 포함), `canPredictMove`, `hasMoveGoal`, `fMoveWaypointX/Y/Z`를 전달한다. 처리 sequence는 이동 승인이 아니며, 최신 미처리 클릭은 이전 IDLE 응답으로 취소하지 않는다. 처리된 입력은 Server 위치와 다음 경유점으로 보정한다. 외삽은 150ms와 다음 경유점으로, 무응답 예측은 350ms로 제한한다. 연속 위치 오차는 최소 80ms와 오차/이동속도에 따른 기간으로 줄이며, 큰 불연속·피격·사망·패턴 구속·마리오는 예측을 해제한다. 일반 예측에서 SKILL 보간으로 넘어갈 때만 표시 잔여 offset을 120ms에 줄인다. 두 replication 소비자는 같은 SKILL 상태를 전달한다. 다른 플레이어는 기존 2 tick 보간을 사용하고 스킬·피해·충돌 권위는 Server에 남는다. Client 경로를 Server 정답으로 전송하지 않는다. Client/Server gameplay socket은 TCP_NODELAY를 사용한다.

제품 Loader는 양쪽에 배포된 맵 `.navpolicy`의 최대 인접 높이 차이를 검증해 `CNavigation::Create_NavGrid`의 네 번째 인자로 전달한다. prototype/Clone이 값을 보존하고 Character 예측은 `Get_MaxStepHeight()`로 경로를 요청한다. 기존 명시적 raw/editor 호출은 기본값 0.6을 유지한다. 일반 MOVE 위치는 목표 XZ로 전진하고 얼굴 회전과 분리한다. Client는 현재 발밑 지면을 읽으며 먼 경유점 Y를 미리 보간하지 않는다. `CCharacter::Update_PresentationYaw`가 최단 회전을 소유하고 ACK helper는 각도를 다시 보간하지 않는다.

two-step ground target의 optional 정본은 `Data/Balance/PlayerSkillTargeting.json`이다. T/2050500은 첫 키 입력에서 packet·sequence·resource·cooldown을 소비하지 않고 class-neutral targeting state와 두 preview만 연다. valid navigation sample의 LMB confirm만 기존 `C2S_USE_SKILL`에 typed `GROUND_POINT` intent를 실으며 RMB cancel은 packet을 만들지 않는다. 성공한 confirm LMB는 물리 release 전까지 BA로 다시 해석하지 않는다. Client의 11m clamp와 red invalid 표시는 preview이고, Server가 finite/range/current navigation을 다시 검증해 승인한 target XYZ만 `PLAYER_SNAPSHOT`으로 복제한다. Character의 `skill_target` pseudo anchor와 Server damage shape는 이 승인 XYZ를 함께 사용한다. 사거리 링 asset identity는 `SOURCE_EXTRACTED`, cursor marker identity는 `RUNTIME_RESOURCE`지만 두 texture의 preview scale/tint/usage는 모두 `PROJECT_TUNED`다.

LMB COMBO는 `comboStages[].hitTimeMs`, `comboAdvanceMs`, `actionDurationMs`를 구분한다. `hitTimeMs`는 damage 발생 시점이다. non-final stage의 input window가 non-zero인 manual COMBO에서 `comboAdvanceMs`는 필수 caster hit/projectile spawn이 끝난 뒤 buffered BA가 다음 stage로 갈 수 있는 가장 이른 시점이며, release는 재생 중인 stage를 자르지 않고 아직 commit되지 않은 continuation만 취소한다. non-final stage가 `inputOpenMs/inputCloseMs == 0/0`이고 `comboAdvanceMs == actionDurationMs`이면 automatic COMBO다. 이 경우 Server는 추가 LMB 없이 full-motion 경계마다 다음 stage로 전환하고, pending MOVE/SKILL도 chain을 끊지 않은 채 마지막 stage 종료 뒤 commit한다. 차원술사 `2050010`은 `_01 -> _02 -> _03 -> _04`의 manual 4-stage 계약이며 추가 클릭 또는 hold 입력으로 다음 단계에 진입한다. 각 stage의 동작 길이는 1400/1500/1067/1700ms다. manual COMBO 중 pending command는 현재 stage의 `actionDurationMs`까지 유지한 뒤 commit한다. Client는 성공한 LMB command를 같은 물리 press에서 재제출하지 않고, 명시 command가 수락된 뒤에도 실제 LMB release 전까지 BA를 억제한다.

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

### 4.1 F1 아레나 카메라와 플레이어 위치 작업

발탄·쿠크 아레나에서 F1 `Arena Camera / Player`는 현재 아레나의 자유 카메라 속도를 조절한다.
기본은 모두 20m/s이며 범위는 0.1~400m/s다. Shift는 30배 이동이다. 설정은 아레나별로 이번
프로세스에서 유지되고 같은 아레나에 재입장해도 보존하며 프로그램 종료 후 디스크에 저장하지 않는다.

플레이어 위치를 바꾸려면 F6 자유 카메라 → F1 `Move Player` → UI 밖의 지면을 한 번 클릭한다.
버튼을 누르면 mouse-look이 꺼지고, Esc/우클릭/F6 follow 복귀는 아직 제출하지 않은 선택을 취소한다.
다시 둘러보려면 Tab으로 mouse-look을 켠다. send 성공은 완료가 아니며 F1의 Server 응답을 확인한다.

`CPlayerController -> IPlayerCommandSink -> C2S_DEBUG_TELEPORT_TO_POSITION -> CGameRoom`이
좌표 의도를 전달하고 Server는 같은 session/world, 새 요청 순서, 생존·capture 상태, navigation과
정적/동적 blocker를 검증한다. 피킹 높이와 Server 바닥 높이가 1m보다 다르면 다른 층/소품으로 보고
거절한다. 성공은 해당 player만 이동·action reset 후 snapshot으로 보이고, 거절은 기존 상태를 보존한다.
중복 요청은 이전 응답만 돌려주며 재이동하지 않는다. Release Server는 이 명령을 거절한다.
UI 위 클릭은 ImGui와 제품 UI의 같은 프레임 mouse claim 모두에서 차단한다.

현재 Shared protocol 81의 Server/Client를 함께 빌드·재시작한다. 새 기능을 이전 실행 파일로 확인하지 않는다.

F1 Sequence Viewer는 모든 Debug Level에서 쿠크/발탄 목록을 읽고, 아레나 실행은
`IPlayerCommandSink -> C2S_DEBUG_WORLD_PLAYBACK -> Room command -> ServerTriggerSystem`
경계를 사용한다. Play/Replay/Stop sequence는 기존 `S2C_WORLD_SEQUENCE_PLAY`의 operation으로
같은 room에 전달한다. Release Server는 요청을 거절하고, 다른 world·없는/비활성 target·사망
player·오래된 request sequence는 실행하지 않는다. 표시 이름은 실행 ID가 아니다.
사용법과 저작/배포 경계는 `AREA_DATA_LAYER_GUIDE.md`의 F1 Sequence Viewer 항목을 따른다.

맵별 플레이어 시점은 같은 F1 항목의 `Move Player` 아래 `Player Follow Camera`에서 설정한다.
`Camera map`은 Character Select / KoukuSaydon 두 맵만 선택하며 Valtan profile은 수정하지 않는다.
Position offset은 플레이어 기준 월드 XYZ(m), Rotation은 Pitch/Yaw/Roll(deg), Pitch +는 아래,
Yaw 0은 +Z다. FOV와 응답(0이면 즉시 follow)도 함께 저장한다. `Apply / Follow current map`은
현재 선택한 활성 맵의 카메라만 바꾸고 follow로 복귀하며, 연출 카메라가 사용 중이면 비활성이다.
`Save camera settings`는 선택한 JSON만 저장하고 다음 진입 때 자동 적용한다. `Reload saved`와
`Reset draft`는 편집값만 바꾸므로 현재 화면에 적용하려면 Apply를 누른다.

정본은 `Data/Camera/CharacterSelect.camera.json`, `Data/Camera/KoukuSaydon.camera.json`이다.
`CArenaCameraProfile`은 schema/version/areaId와 유한 범위를 검증하고 실패 시 기존 값·파일을
보존한다. 각 Level이 생성·class 변경·follow 복귀에서 profile을 소비하며 Kouku 연출 종료
위치·주시점·FOV도 같은 값으로 돌아온다. 연출 자체의 roll은 기존 override를 따르고 follow 복귀
후 저장한 roll을 적용한다. Character Select에서는 카메라 튜닝만 추가하며 `Move Player`는
비활성이다. 새 Server command와 Resources 전달물은 없다.

### 4.2 마리오 변신·방향키 조작·Debug 점프

Server가 기존 `Mario1_Intro`~`Mario4_Intro` OBB 진입을 검사해 `PLAYER_SNAPSHOT.iMarioStage`를
0(일반)/1~4로 보낸다. 입장 시 기존 `eMadnessForm=CLOWN` 외형 교체 경로를 사용하고,
기존 퇴장 이동·다른 F1 배치·아레나 컷신 강제 배치 시 전용 상태를 해제하고 입장 전 외형을 복원한다.
사망/낙하·프로필 교체·월드 전환도 상태를 정리한다. 일반 F1 Clown만 켠 상태는 마리오 모드가 아니다.

Client는 마리오에서 `Request_MarioMove`로 `MARIO_DIRECTION { STOP=0, LEFT=1, RIGHT=2 }`만 제출한다.
`C2S_MARIO_MOVE { iClientSequence, eWorldId, eDirection }`는 7바이트이며 자유 XZ/카메라 방향을 받지 않는다.
Server는 session/world/순서/enum/마리오 상태를 검사하고 기존 navigation·collision 이동기에
0.75m 직접 목표를 공급한다. A* 우회 경로는 만들지 않는다. 입력 유지 시 100ms 갱신,
키 해제 시 STOP, 입력이 끊기면 9틱(30Hz에서 300ms) 만료로 정지한다.

Server의 18개 stable entry/exit trigger 연결이 구간별 진행선을 소유한다. 승인된 실제 착지점부터
다음 출구 trigger 중심까지의 XZ 축과 고정 RIGHT 부호를 사용한다. 카메라의 회전/보정은 축을 바꾸지 않는다.
진행 중 위치를 고정 원점/축으로 복원해 왕복·점프·knockback의 깊이 방향 누적 오차를 막고,
Mario body 충돌에서만 옆미끄러짐을 끈다. 기존 navigation의 구멍/높이/충돌을 강제로 개방하지 않는다.
기존 저작 movePlayer 완료 시 출처 placement ID로 다음 구간을 선택한다. 일반 Debug 점프는 축/원점을 바꾸지 않는다.
초기 이동 도중에는 Clown 상태만 적용하고 착지 후 진행선을 확정한다.
M4 Tigger_2의 현재 다음 출구는 Tigger_5이며 착지점과 이 출구로 진행선을 다시 계산한다.
유효한 연결이 없으면 자유 이동으로 대체하지 않는다. 마리오 밖의 회전/이동/충돌 처리는 유지한다.

Debug follow-player에서 ↑의 새 누름은 현재 Server가 제안한 건너가기 trigger가 있으면 기존
`Request_InteractTrigger`를 우선 제출한다. 없으면 최근 ←/→ 방향으로
`Request_DebugMarioJump` → `C2S_DEBUG_MARIO_JUMP { iClientSequence, eWorldId, eDirection }`를 보낸다.
이 요청도 7바이트이며 LEFT/RIGHT만 허용한다. 착지 좌표는 Server가 같은 진행선에서 결정한다.
마리오에서는 ↓/Shift 점프/우클릭 이동/일반 스킬을 막고,
컷신·UI·자유 카메라·공중·사망·잡힘 중 입력은 제출하지 않는다. 자유 카메라 Shift 가속은 유지한다.
몸체 교체는 물리 입력 edge/capture 상태를 보존해 계속 눌린 입력을 새 입력으로 재해석하지 않는다.
상호작용 안내도 마리오에서는 `[ Up ]`, 일반 아레나에서는 `[ G ]`다.

Debug Server는 session의 플레이어만 대상으로 하며 명시적인 stage 1은 같은 기본 grid,
stage 2/3/4는 authored `MarioN_go` 목적지와 같은 상세 navregion인지 검사한다.
최대 4m부터 0.25m씩 줄여 최소 0.75m의 같은 region 착지를 찾으며, 시작은 바닥에서 0.25m 이내,
착지는 시작 바닥과 높이 차이 1m 이내의 exact walkable/collision-clear 지점이어야 한다.
허용된 점프는 기존 `CServerTriggerSystem::Begin_MovePlayer`의 0.6초/arcHeight 1.5m 이동과
`TRIGGER_MOVE` snapshot을 재사용한다. 테스트용 gap 통과이며 새 jump 스킬/스켈레탈 clip은 아니다.
HP·소품 상태·trigger membership은 초기화하지 않는다. 새 요청 순서를 검사하고 중복에는 이전 verdict만 반환한다.
Release Server는 점프에 `REJECTED_DISABLED`를 반환한다. F1 `Mario Controls (Debug Jump)`에 승인/거절 이유를 표시한다.

마리오의 원본 공은 Q 뿅망치로 터진다. `Publish-WorldGameplay.ps1`이 worldbootstrap v11의 `MARIOBALL`
행(stage, layout, slot, color, placementId, x, y, z)으로 싣고 slot은 해당 layout WorldSequence의 바인딩
인덱스와 같은 순서다. Server는 몬스터와 같은 전방 120°·2.4m 원뿔에 공 반지름 0.47m를 더하고 높이 창
1.2m로 판정해 `iMarioPoppedBallMask` 비트를 세우며, 한 색이 그 layout에서 전부 터지면
`iMarioCurseReleasedMask`의 해당 비트(0 빨강, 1 파랑, 2 노랑)를 세운다. 공은 엔티티가 아니므로
damage event를 만들지 않고, 스테이지가 비어 초기화되면 두 마스크도 0으로 돌아간다. Client는 표시만
담당한다(배치 숨김, `boss.kouku.ball.smoke.<색>_1` 1회, 중앙 문구 3초).

### 4.3 마리오 원본 배치 선택

Shared protocol 76의 `PLAYER_SNAPSHOT.iMarioLayoutVariant`는 0=미선택, 1–3=원본 색 공 Case다.
Server가 Mario1–4 진입 시 34/33/33 확률로 선택하고 같은 stage의 살아 있는 참여자는
이미 선택된 Case를 공유한다. 마지막 참여자 퇴장·사망 후 새 진입은 다시 선택한다.
Client는 `world.sequence.instance.marioN.source.layoutC`의 기존 MAP_PLACEMENT 트랙만 재생한다.
클라이언트 독자 난수 선택이나 모든 후보 동시 활성화는 금지한다.

`SpawnGroups.world.json`의 `spawn.marioN.source`는 원본 actor별 anchor를 가진 일반 몬스터 그룹이다.
Server 진입 코드가 활성화하고 해당 stage가 비면 entity despawn 후 `Reset_Group`으로 일정만 초기화한다.
기존 카드미로 그룹이나 다른 stage는 초기화하지 않는다. 원본 XZ를 유지하는 Sample_Position과
원본 높이 대비 0.25m 검사 후 생성한다. 종료 지점은 기존 lane graph의 terminal exit로도 인식하여
퇴장 목적지를 편집해도 stage/layout과 생성 그룹이 남지 않는다.

현재 일반 ZoneLevel 0의 원본 배치이며 ZoneLevel 2 전용 추가 actor는 제외한다.
추가 MonsterCatalog의 `MONSTER_MARIO_REUP/RHKP/CDMD/CLUB/HEART/DIAMOND`는 현재 프로젝트 튜닝
순찰/근접 공격 프로필이다. Server가 같은 stage·같은 높이의 가장 가까운 published lane을
source anchor에 평행 이동한 직선에서 왕복시킨다. 끝·네비 단절·충돌에서는 이동을 멈추고
world Y축으로 선회한다. 접선 미끄러짐으로 통로를 이탈하지 않는다.
같은 stage의 생존·combat-ready player만 2m 안에서 감지하며, 높이차 0.8m 초과와
trigger 이동 중인 player는 제외한다. 600ms 예고/100ms 판정/1300ms 회복이며 해당 공격당
고정한 대상의 전방·거리·높이를 재검사해 기존 ServerCombatHitRuntime으로 한 번만 피해를 준다.
MonsterProfiles의 Mario 여섯 프로필만 HP 1, 방어 0, 이동 1.2m/s, 선회 360deg/s로 설정한다.
실제 양수 피해를 주는 player hit 한 번에 몬스터가 죽으며 player HP는 변경하지 않는다.
Client는 기존 IDLE/CHASE/ATTACK/DEAD snapshot으로 catalog의 원본 clip을 자동 재생한다.
마리오 여섯 종류의 순찰은 `walk_normal_1` 계열을 사용한다. Q/W는 기존 InteractionSlot
명령으로 전달하며 마우스 이동과 class 스킬은 계속 차단한다. Q는 승인 후 12틱에
전방 120도·2.4m(+몬스터 반경), 높이차 0.8m 이내의 같은 stage 몬스터를 한 번 판정한다.
피해/사망은 ServerCombatHitRuntime에서 확정하고 W는 Q 피해를 공유하지 않는다.
Mario modelYawDegrees=-90은 모델 +X 앞축을 Server yaw의 +Z 앞축으로 정렬한다.
이는 원본 AI·표적 집계·폭탄 피해의 복원 완료를 의미하지 않는다.
Server와 Client를 protocol 81으로 함께 빌드/재시작하고 신규 리소스는 대응 배치 RESULT를 참조한다.

## 5. Character와 Animation

캐릭터의 외형 배율은 `Data/Actors/CharacterCatalog.json` format 4의 optional
`presentationScale`이 소유한다(미지정 1, finite 양수, 최대 100).
`CCharacter::Try_Get_PresentationRootMatrix`의 `Scale * gameplayWorld`를 body/장비/무기와
Animation Target의 현재·과거 pose 및 bone Effect anchor가 소비한다. asset import scale,
Server pose/이동/충돌은 이 값과 분리한다. Character의 `root` Effect cue와 `skill_target`,
명시 world root는 기존 world 크기를 유지한다. class가 없는 쿠크 변신 avatar는 착용자의
class 배율을 상속하지 않는다. catalog는 process 최초 초기화에서 읽으므로 변경 뒤 Client를 재시작하고
character를 다시 생성해야 한다. runtime hot reload는 아니다.

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

F1 진입 이름은 `Action Workbench`다. Boss 선택에 따라 동일한 Sequencer/Resources/Patterns/Box Detail 창이
Valtan/KoukuSaydon의 독립된 문서·draft·저장 session을 연결한다. 아래 split-owner 계약은 Valtan session에 적용한다.
Valtan의 Server stage, animation occurrence, Effect, Pattern Sound, Shake/Camera와 world/combat-object를
stable action·occurrence ID로 join한다. Workbench는 새 Product JSON이나 두 번째 runtime을 소유하지 않는다.
stage·release·Counter/Groggy·Collider는 `CBalanceTool`의 joined gameplay draft를, animation slot과 Effect
invocation은 `Data/Valtan/Valtan.presentation.json` source owner를 사용한다.
`Valtan.patternbindings.json`과 `Valtan.patterneffectcues.json`은 projector가 만드는 read-only Product다.
Counter enable은 paired counterable flag, `COUNTER_HIT` branch, same-pattern GROGGY action 또는 typed
cross-pattern GROGGY target과 paired groggy flag를 한 Server-authority 단위로 만들고 Animation notify가
결과를 확정하지 않는다. Save는 split source와
generated Product를 공통 writer generation으로 commit한 뒤 exact canonical reload까지 검사한다. 로컬 저장
뒤에도 같은 immutable revision이 Server-active로 확인되기 전에는 Complete Play와 Restart를 실행하지 않는다.
실제 seek/stop adapter가 연결된 lane만 local `PLAY`다. Effect invocation은 typed Details의
`EDIT/SAVE`를 지원하지만 일반 timeline block drag와 local seek/stop은 지원하지 않는다.
Camera/World lane은 owner file과 stable row를 표시하는 `INSPECT` 상태로 남긴다. Sound는 별도
typed owner에서 `EDIT/SAVE`하되 local seek/stop transport는 `INSPECT`다.
`Animation Sequence Intake`는 review 원본이고 promotion transaction 전에는 Product/Server pattern이 아니다.

Server collider의 mutation UI는 Valtan Action Workbench의 typed `Stage Hit (Boss -> Player)` Details
하나뿐이다. Balance Tool의 broad Valtan pattern panel은 같은 joined 값을 읽기 전용으로 표시한다. Collider
lane `+`는 hit가 없는 non-WAIT `MANUAL_SERVER_AUDITION` Stage에 기본 `BOX`(length 8.0m,
half-width 2.5m)를 만들고, 기존 non-WAIT hit는 canonical/manual 여부와 무관하게 제자리 Tune할 수 있다.
Remove는 non-CAPTURE manual audition hit만 허용한다. canonical no-hit Add, canonical Remove, CAPTURE의
geometry-only Remove는 C++ setter와 최종 `SET_STAGE_HIT` writer가 모두 거부한다. 모든 안정 Stage의
Gameplay/Logic/Collider Details를 열어 보는 것과 Stage topology 또는 Add/Remove 권한을 주는 것은 다른
admission이다. Details를 열기 위해 Pattern을 `manualAuditions`로 승격하지 않는다. selection candidate와
manual audition의 교집합은 publisher/pipeline admission 오류다.

hit timing은 pulse schedule과 `ACTIVE_WINDOW` 중 정확히 하나다. Active Window는 Stage-local 반열린 구간
`[startMs, startMs + lifetimeMs)`이고 target당 한 번만 판정한다. `anchor`와 `activation`은 geometry Tune 때도
lossless round-trip하며, `NONE`은 schedule/activation/anchor/damage/response까지 함께 비운다. Damage profile
rate는 Workbench에서 같은 canonical draft의 `SET_DAMAGE_RATE`로 저장하고, 공유 Stage 수와 방어 전 raw damage를
표시한다. Sequencer Save는 Pattern/Balance, Sound, EffectV2 owner와 pipeline/reload를 계속 all-or-nothing으로
처리하고, 실패 문장은 `[Pattern]`, `[Sound]`, `[EffectV2]`, `[Pipeline]` 중 최초 차단 owner를 표시한다.

Effect invocation Details는 exact clip occurrence와 source start/end, stop/repeat, anchor/follow,
local transform, scale policy를 canonical `Valtan.presentation.json`에 add/update/remove한다. projector가
만드는 `Valtan.patterneffectcues.json`을 직접 저장하지 않는다. 이 편집은 Pattern과 Effect asset의
연결·시간·배치만 소유한다. asset 내부 element/재질/lifetime 편집 deep-link는
Save/Publish/Reload된 exact admitted occurrence와 전체 cue field가 일치할 때만 선택
`effectAssetId`의 `Data/Effects/Authored/*.effect.json`을 Effect Tool에서 연다. draft-only row에
asset-only fallback을 제공하지 않는다.

Pattern Sound Add는 exact clip occurrence와 검증된 Sound event로 deterministic stable row ID를 만들고,
Remove는 exact `bindingId + occurrenceId`만 지운다. Sound owner Save와 local source reload 성공 여부는
canonical gameplay/presentation generation과 별도로 표시한다. Sound Save는 shared read-generation
admission을 통과한 별도 CAS commit이며 Pattern writer와 하나의 atomic Save가 아니다. runtime 적용은
exact Pattern revision이 Server-active가 된 뒤 `Retry Apply`로 수행하고, 적용 전·후 revision CAS와
revision-pinned ready receipt를 검사한다. Sound transport에 seek/stop handle이 없으면 공통 playhead
재생 완료로 표시하지 않는다.

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

Element 목록의 Shift/Ctrl 클릭은 다중 표시 집합을 만든다. Delete와 Duplicate는 표시된 집합을 우선하고,
표시가 없으면 Detail에서 열린 한 Element를 사용한다. Duplicate는 내부 transform inheritance를 새 ID로
연결하고 외부 master와 timing을 보존하며, 새 복제본을 다시 표시한다. 반복 타격 간격은 Start Delay로 편집한다.

직접 저작한 Sprite Particle은 `drag`, `rotationRangeDegrees`, `spinRangeDegreesPerSecond`, `subUVOverLife`,
`initialVelocity.uniformSolidAngle`을 사용할 수 있다. 기본값에서는 기존 재생을 유지한다. 입자별 SubUV는
UV sequence/loop와 혼합하지 않는다. `material.colorTexturesSRGB`는 일반 material의 base/base2/emissive에만
적용하고 mask/noise/dissolve는 linear로 유지한다. `actionCueAttachment.orientation=owner_yaw`는 실제 본 위치와
owner의 unit 방향을 사용한다. Data Files의 `Load Saved Element for Editing`으로 이 native owner-yaw Sprite
Particle을 복사할 때는 본 부착을 초기화하며 새 대상에서 `Attach Selected Element`로 다시 연결한다.
SourceRecipe/FOLLOW 이력과 Trail을 임의로 떼어 복사하는 기능은 아니다. 기존 Data Files는 V2 문서를 직접
읽지 않으며, 모아치기에 옮긴 손 불꽃·연기는 일반 authored Element이므로 같은 저장/복사 경로를 사용한다.

Valtan 연결 Effect의 편집 진입은 `F1 → Valtan Boss Tool → Boss Verification → Pattern →
Stage → Edit Linked Effect`다. `patternId/stageId/cueOccurrenceId/effectAssetId`의
exact tuple을 현재 Product tree와 다시 대조해 문서를 연다. clip-bound cue는 기존
전체 Pattern timeline의 t=0 pause로, `STAGE_CLOCK`은 static Valtan target으로 연결한다.
자동 Play나 Server 명령은 보내지 않는다. 사용자는 Model View Timeline의 `Play` 또는
`Restart + Play`로 재생한다. 미저장 문서는 기존 Save/Discard/Cancel을 거치며 unlink
진행 중에는 이 deep-link도 잠긴다. 연결은 `Valtan.presentation.json`, Effect 내용은
해당 `Authored/*.effect.json`이 각각 소유한다. unlink는 cue 연결만 지우며 사용자 Effect
파일과 catalog를 자동 삭제하지 않는다.

V1 Product source가 실제 참조하는 DDS/WModel dependency closure는 팀장 Drive의 같은
Resources-relative 경로에 둔다. Git은 authored source와 asset ID만 전달한다.
물리 pack은 기본 validator와 전체 회귀가 파일 실재·안전 경로·내용과 local 미추적 개수를 검사한다.
`-AllowLocalResources`와 `-AllowLocalEffectResources`는 과거 명령 호환용이므로 생략할 수 있다. 이 검증을
Git 배포 PASS로 기록하지 않는다.
Valtan actor Product가 직접 참조하는 body, Parts1/Parts2, AnimSet과 weapon 다섯 WModel, 그리고 그 material
table이 참조하는 body/parts TGA 12개와 weapon DDS 8개도 같은 Drive 전달 묶음에 포함한다. 새 clone은
실행 전에 팀장에게서 이 물리 리소스를 같은 상대 경로로 전달받는다.

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
필드를 반드시 invalid로 둔다. Client는 이 target을 다시 고르지 않는다. 별도로 player가 `GRABBED`인 동안
`PLAYER_SNAPSHOT`의 attachment owner/slot과 Server가 매 tick 계산한 world position/yaw가 포획 판정·release·
ejection의 정본이다. Server는 capture 순간의
boss-local 상대 위치와 yaw offset을 저장해 매 tick `boss pos + yaw 회전(offset)`을 플레이어 위치로 복제하고,
이 위치가 판정·release·ejection의 정본이다. Client `CCharacter::Update`는 `Update_NetworkTransform`이 그
값을 보간한 직후, 파츠가 world를 합성하기 전에 owner `CValtan`의 `bip001-l-hand` socket(bone × presentation
root)과 admitted `gripLocalOffset`(boss yaw frame forward/right, world up)으로 발 원점 POSITION만 교체한다.
따라서 본체·장비·collider wire·nameplate가 같은 손 위치를 따르고, release 뒤 0.2초 동안 마지막 손 위치에서
Server 경로로 합류한다. `gripLocalOffset`은 `ValtanEncounter.json`의 모든 CAPTURE hit에서 같아야 하며 다르면
joined presentation admission이 실패한다. Server가 보는 잡힌 플레이어 위치는 계속 capture 지점 기준이므로
다른 플레이어 skill 판정과 던지기 시작점은 손 위치와 다를 수 있다.

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
| `Valtan.gameplay.json` | Server pattern graph, decision, stage, action, hit, motion, volley와 inline `scriptedSequence` 순서·반복·간격 |
| `Valtan.presentation.json` | animation occurrence, Effect invocation, camera invocation, cue scale policy |
| `Valtan.combatobjects.json` | combat-object origin, movement, hit geometry, damage profile reference와 optional 독립 `lifetimeMs` |
| `Valtan.worldeventsets.json` | stable world-event set membership |
| `Valtan.legacy-compatibility.json` | 아직 승격하지 않은 Product closure와 migration identity |

`Data/Valtan/Valtan.pattern.json`은 migration fixture이며 새 값을 저장하지 않는다. publisher는 다섯 split source를
stable ID로 strict join해 `ValtanEncounter.json`, rotations, combat objects, world events, pattern bindings/cues와
Server bootstrap을 생성한다. `Valtan.gameplay.json`의 `decisionModel.scriptedSequence`가 Product 순서의 유일한
저작 정본이며 publisher가 generated `ValtanPatternRotations.json`에 exact 투영한다. 정본이 31 Pattern이면 Product도
31 Pattern이어야 하며 29 Pattern 상태를 병행 허용하지 않는다. 외부 validator는 `Validate` 하나다. canonical Save는
source candidate를 먼저 검증하고 같은 transaction에서 Product를 생성·commit하며, 이전 generated Product parity를
저장 선행조건으로 사용하지 않는다. generated Product는 read-only이고 Server와 Arena가 split source를 두 번째
런타임으로 직접 읽지 않는다.

`Data/Compositions/Bosses/Valtan.bosscomposition.json`은 위 split source와 Effect V1/V2, Sound,
Shake owner의 경로·coverage·Pattern index를 묶는 `SHADOW` source manifest다. 값을 다시 복사해 소유하는
새 gameplay 정본이 아니다. `Publish-Compositions.ps1`은 이를 검증해
`Client/Bin/DataFiles/Compositions`에 resolved read model과 receipt를 생성하지만 현재 Product는
`runtimeEligible=false`이며 Client 또는 Server gameplay가 실행하지 않는다.

현재 화면과 owner 경계는 다음과 같다.

| 화면/런타임 | 현재 역할 |
|---|---|
| Valtan Boss Tool | Server Product Pattern inventory, live state, Next/Restart/Flow command |
| Action Workbench | Boss 선택과 공용 Sequencer/Resources/Patterns/Box Detail; Valtan split owner 또는 Kouku Composition의 편집·Preview·Save |
| Effect Tool V1 | 복원 Effect asset·Current Effect·Effect Detail·V1 Resource·World Object·Model View·Effect Sequencer |
| Effect Tool V2 | V2 leaf/group CPU draft·독립 Resource/Sequencer·target attachment 편집 |
| Server | branch, motion, hit, combat object, phase의 gameplay 권위 |

`Valtan.bosscomposition.json`은 `SHADOW`, `KoukuSaydonGate1.bosscomposition.json`은 `REFERENCE_ONLY`,
`ValtanArena.sequencer.json`과 `KoukuSaydonArena.sequencer.json`은 `SHADOW`다. 공용 Workbench는 이 source
manifest의 admission을 편집 진입 조건으로 사용하지 않는다. generated resolved Product를 timeline runtime으로
읽지 않는다. 이 SHADOW/REFERENCE_ONLY 문서와 별도로
`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`은 쿠크 Stage/Animation 저작 정본이며
Action Workbench의 KoukuSaydon session이 전체 트리를 편집하고 단일 파일 CAS Save한다. K Boss Tool은 명시 publish된
`Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`의 전체 `patternInventory`를 표시하고,
실행 가능한 Product와 Server bootstrap은 기존 typed 재생 경로에서 소비한다.
Composition v3는 `GATE1/GATE2/GATE3/BINGO`, 부모 폴더, 재생 묶음, 대상 보스가 지정된 패턴을 저장한다.
카드미로 Logic의 `CARD_MAZE_HIDE_NEXT`는 첫 occurrence에서 살아 있는 참가자를 월드 X 내림차순,
동률 PlayerId 순으로 확정하고 한 occurrence마다 한 명을 숨긴다. `CARD_MAZE_ENTER`의
`teleportPosition`은 서버 navigation과 collision으로 검증하는 미로 중앙 목적지다. 참가자 전원
검증 뒤 같은 tick에 이동·표시 복구·MAZE HUD를 적용하고 기존 중앙 Q 망원경 진행을 사용한다.
입장 실패나 패턴 Stop은 이 연출이 숨긴 참가자를 다시 표시하며, 실패하면 전원 원래 위치를 유지한다.
Shared protocol 81의 `CARD_MAZE_PRESENTATION.flags` bit16이 표시 상태를 복제한다.
Server/Client를 함께 다시 빌드하고 publish 뒤 Server를 재시작한다.
Effect occurrence의 dissolve 시작·끝은 lifetime 정규화 시간이다. 같은 값은 기존 runtime의 즉시
전환 규칙을 사용하며 1/1은 lifetime 끝까지 dissolve-out을 하지 않는다. 역전된 구간은 거부한다.
Composition Patterns의 Model View는 표시 필터이며 묶음의 실행 대상을 바꾸지 않는다.
Parent는 기존 분류 폴더를 유지하면서 optional `timelinePatternId`로 자기 실행 Pattern을 가질 수 있다.
Parent를 선택해 `Create Parent Timeline`을 누르면 기존 Animation/Logic/Effect/Collider/World 등 행과
새 Pattern 행을 함께 편집한다. `Append Pattern at Cursor`는 같은 관문·모델·대상 보스의 독립 Pattern을
stable ID로 참조하고, 배치별 시작·시간창·반복을 저장한다. 원본 편집은 모든 참조에 반영된다.
부모의 명시 duration과 공통 행은 자식의 종료와 독립이다. 시간창 끝에서는 자식의 미완료 판정을
실패 결과 없이 취소하고 다음 구간으로 진행한다. Preview와 publisher는 이를 기존 단일 Pattern 경로로
전개하며 Server는 Parent의 절대 시간으로 stage 경계를 계산한다. 같은 보스의 자식 애니메이션 겹침,
Parent 중첩·순환, 자식의 전체 패턴 조기 종료·동적 follow-up·강제 BossMotion 등 의미를 보존할 수 없는
조합은 사유와 함께 거부한다. 원본을 수정하면 참조 Parent와 관련 Bundle도 다시 게시해야 한다.
Bundle은 여러 보스의 동시 재생을 위해 stable child pattern ID와 시작 offset을 참조한다. 묶음을 선택하면 자식별 요약 Sequencer와 공통
Camera/Scene Profile 행을, 자식을 선택하면 기존 상세 Sequencer를 편집한다. Create Parent → Create Bundle →
Create Pattern 또는 Link Existing Pattern으로 연결하며 원본 패턴·클립을 복제하지 않는다.
모든 Composition Save는 수정 내용을 저장하고, Patterns 목록의 `Publish All Patterns`가 전체
Parent/Bundle/Pattern을 함께 게시한다. 수동 PRODUCT 선택은 없으며 publisher가 각 대상의 실행
가능 여부를 검사한다. 미완성 항목은 F1의 같은 위치에 사유와 함께 남고 재생만 비활성화된다.
Encounter/patternbindings와 Gameplay 게시 중 실패하면 domain owner가 이전 생성물·receipt를 복구한다.
배포 성공 후 F1 목록을 갱신하며 Complete Play는 최신 Product를 재조회한다. Workbench의 미저장
변경·배포 진행·source/Product revision 불일치를 거절하고, Server 활성 revision 검사는 유지한다.
새 runtime 데이터의 Server 적용에는 재시작이 필요하다. Product source revision 거절 메시지는
요청 번호와 Server 활성 번호를 함께 표시한다. publish 성공 뒤에는 Server를 재시작하고 Client를 재접속한다.
패턴 상세 Sequencer는 Stage/Animation/Logic/Summon/World/Scene Profile 및 모든 presentation lane을
드래그 또는 Ctrl+click으로 함께 선택한다. 드래그는 박스의 시간폭 전체를 감싸며 Ctrl+drag는 선택을 추가한다.
Duplicate 또는 Ctrl+D는 선택한 구간과 Collider↔Logic·World↔동반 Effect 연결을 새 stable ID로 복제한다.
Stage가 포함되면 뒤 구간을 밀고 삽입점을 가로지르는 기존 master/World의 수명을 연장한다.
외부 카드와 검색 master 참조는 유지한다. 긴 master/World까지 직접 선택하면 그 전체 구간이 복제되므로
한 타격만 복제할 때는 선택 표시를 확인한다. lane만 복제하면 선택 끝에 배치하고 필요한 끝 시간을 늘린다.
Delete도 선택 전체를 한 번에 처리한다. Earlier/Later는 Stage/Animation만 선택했을 때 사용한다.
복제·삭제 후 Save → Publish All Patterns → Server 재시작을 거쳐 Complete Play로 확인한다.
Pattern/Parent/Bundle/Logic/Resource의 Rename은 현재 표시 이름을 열어 Apply하고 기존 Save로 저장한다.
stable ID와 참조는 바뀌지 않는다. Stage 삽입·삭제는 겹치는 BossMotion 시간도 함께 늘리거나 압축하며
이동 위치와 yaw를 보존한다. 이동 구간 전체를 제거할 때는 Move boss를 먼저 해제한다.
Animation의 optional blendInMs는 직전 clip 끝점에서 현재 clip으로 보간한다. Product의 짧은 playMs는
남은 Stage 동안 종료 자세를 유지하고 LOOP_TO_WINDOW는 지정한 시간창에서 반복한다.
Preview, Product의 body/weapon 및 서버 bone-contact bake가 같은 샘플링 계약을 사용한다.
F1은 관문별 `Saved Pattern Flow`와 `All Patterns`를 구분한다. Boss Tool의 `Pattern Flow`에서
Pattern 또는 Bundle stable ID를 추가하고 순서·대기 시간을 편집하여 `Save Pattern Flow` →
`Publish Saved Patterns`로 게시한다. 정본은 기존 KoukuSaydon Composition의 optional `patternFlows`다.
Action Workbench는 모델 선택과 무관하게 전체 Gate→Parent→Bundle→Pattern 트리를 표시한다.
`Complete Play - Sequences + Pattern Flow`는 관문의 `enterCombatOnFinish=true` 입장 Sequence 하나를
0ms부터 재생하고 Server Gate 승인 뒤 플레이어 follow camera로 복귀하여 저장 Flow를 시작한다.
정상 완료 이벤트만 연결되며 Stop/실패/관문·world·게시 revision 변경은
전투 시작을 막는다. `Play Saved Pattern Flow`는 전투 순서만 실행하고 `Composition Play All`은 선택 관문의
게시 목록에서 Bundle을 하나의 동시 실행 항목으로 유지하며 해당 child의 중복 단독 재생을 제외한다.
Flow는 정확한 Server COMPLETED를 받은 뒤 대기 시간을 거쳐 다음 typed Pattern/Bundle 요청을 보낸다.
거부·취소·연결 종료는 남은 순서를 취소한다. Server는 묶음의 모든 대상을
검증한 뒤 하나의 run epoch와 공통 시작 tick을 확정한다. offset은 30Hz tick으로 올림하며 Stop/Restart는
원래 run epoch를 명시한다. 같은 방의 Client와 늦게 입장한 Client는 복제된 묶음 상태와 시작 tick을 소비한다.
공통 Camera/Scene Profile은 묶음 시계에서 한 번 실행하며 다른 소유자의 겹치는 전역 연출은 게시 단계에서
거부한다. Preview는 연출 확인이고 조건부 gameplay 결과는 Server Complete Play에서 확인한다.
K Resource는 실제 `MN_RPCZ_00` 모델 clip 전체를 읽고 action reference는 참고 트리로만 사용한다.
reference에 없는 물리 clip은 `sourceActionId=0`, `sourceStageId=RAW`, 빈 `referenceRevision`으로 저장한다.
PRODUCT의 `sourceActionIds`가 비어 있으면 K bootstrap의 PATTERNSOURCE 행을 생략한다.
현재 Resource/Animation family preview는 기존 collision-off 로컬 preview actor에서 실행한다.
Kouku Composition Play는 같은 clock으로 Animation, WORLD와 presentation occurrence를 재생한다.
WORLD 정의의 optional `positionOffset: [x,y,z]`와 speed, WORLD occurrence의 `durationMs`는
projector의 `worldSequences`와 Gameplay bootstrap `PATTERNWORLDSEQUENCE`를 거쳐 Server cue까지
전달된다. protocol 69의 `S2C_WORLD_SEQUENCE_PLAY::iDurationMs`는 요청 구간의 경과시간 제한이며
1..600000ms를 사용한다. 0은 기존 요청의 authored 수명을 사용한다. 구간은 playback speed와
별도로 측정한다. Object Motion은 이 구간 안에서 생성한 객체의 이동과 MOTION_END Effect 수명을
마저 재생한 뒤 정리한다. 명시적 Stop은 원래 배치를 복구하고 생성한 객체를 즉시 정리한다.
Object Motion의 optional `spawnHalfExtents: [x,y,z]`는 기준 위치 주변 생성 범위의 반폭이며
생략하면 `[0,0,0]`으로 기존 위치를 유지한다. X/Z를 독립적으로 지정해 직사각형에서
생성하고 기존 seed와 emitter 시계를 사용한다. World Object Tool 저장·Map publisher·runtime이
같은 값을 소비한다. 생성 간격 `intervalMs`는 이동 수명과 별도이며 마지막 생성의 이동과
MOTION_END tail까지 WORLD box 구간과 함께 확인한다.
WORLD cue는 run epoch·member·cue ID와 시작 tick을 함께 전달한다. Client는 전달 지연만큼 시계를 맞추고,
STOP_OWNER는 취소·실패·restart에 사용하고, 정상 완료의 FINISH_OWNER는 이미 생성한 공과 Effect의
남은 수명을 보존한다. 두 명령 모두 해당 run/member가 만든 객체에만 적용한다.
Server/Shared/Client는 같은 protocol 81으로 함께 빌드·재시작한다. FEAR snapshot 상태와
빙고·마리오·갈고리 attachment wire, 마리오 원본 공의 `iMarioPoppedBallMask`(u16)·
`iMarioCurseReleasedMask`(u8)와 카드미로 ENTRY_HIDDEN을 함께 포함한다. 두 기능이 별도 branch에서
각각 79를 사용했으므로 두 종류의 v79 및 이전73/77/78 실행 파일과 혼용하지 않는다.
optional `resetBossToSpawn`은 패턴 시작 때 Server가 실제 보스를 spawn에 복구한다.
함께 지정하는 optional `resetBossYawDegrees`는 유한한 -360~360도의 절대 yaw로, 매 재생 같은 방향을 snapshot에 반영한다.
누락하면 기존 yaw를 유지한다. 이 필드를 배포할 때는 확장된 PATTERNSPAWNRESET을 읽는 Server도 함께 빌드·재시작한다.
optional `bossMotion { startMs, endMs, startPosition, endPosition, yawDegrees }`는 패턴 내부 구간의
절대 월드 XZ 이동을 저장한다. 두 위치의 base Y는 같으며 시작 전에는 시작점, 종료 후에는 도착점을 유지한다.
spawn reset 및 REAL_GAZE_TELEPORT와 동시 사용은 거부한다. Server는 audition stage 전에 경로 navigation을
검증하고 30Hz fixed tick에서 보간한 위치·yaw를 기존 snapshot/늦은 입장의 spawn으로 전달한다.
Client는 기존 root 수평 억제를 유지하고 원본 animation의 수직 pose를 재생한다. 별도 높이 arc는 더하지 않는다.
Stage의 optional `retargetOnEnter`는 strict boolean, 기본false다. Workbench Stage Detail과
Composition parse/validate/save가 같은 값을 소유하고 기존 `RETARGET_RANDOM_ALIVE` ENTER action으로
투영한다. Server는 Stage 진입 때 살아 있는 player의 현재 위치와 yaw를 한 번 확정해 다음 지정
Stage까지 유지한다. fixed-yaw `bossMotion`과의 동시 사용, 잘못된 타입/중복 action은 거부한다.
Preview는 Stage별 표본을 저장해 되감기에 재사용하며, 처음 방문한 미래 Stage는 현재 player를
표본으로 삼는다. 명시 Model Reference는 이 retarget을 사용하지 않는다.
Pattern의 optional `animationRootVerticalScale`은 0~1, 기본 1이며 원본 root의 기준 pose 대비 수직
변위만 조절한다. mesh 크기·clip 속도·Server base Y는 바꾸지 않는다. Product action binding과
Preview가 같은 값을 소비하고, bone Collider 투영도 같은 배율을 사용한다. 다음 action/idle은
자기 Pattern 값 또는 기본 1을 적용하므로 이전 패턴의 높이 설정을 이어받지 않는다.
Play Bundle과 실제 보스 actor를 소유한 단일 Pattern Play는 같은 actor/weapon preview 경로를 사용한다. Model Reference는 제자리 비교다.
BOSS_SPAWN World Object는 Product의 `worldEmissionAnchors`와 bossMotion을 사용해 각 emission 시각의 생성점을
고정한 뒤 개별 objectMotion을 재생한다. 객체가 이동하는 보스를 매 프레임 따라가는 정책은 아니다.

F1 Action Workbench 바로 아래 `Open Sequencer Benchmark`는 동일한 Timeline/Resources/Box Detail ImGui와
기본 배치를 사용하는 독립 Sequence 세션이다. Patterns 목록 창 이름은 `Composition Sequencer`로 표시한다.
저장 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, compositionId는
`boss.composition.kakulsaydon.sequencer`이며 기존 Action 문서와 교차 Save/Reload를 거부한다.
같은 Preview backend의 재생 소유자를 입력 focus와 분리하여 다른 창을 열거나 닫아도 활성 미리보기를
덮지 않는다. 연출 Save는 Action/Server Product를 변경하지 않으며 파티 생성은 후속 Summon/Logic 작업이다.
Sequence의 `Complete Play`는 선택 Gate에서 `enterCombatOnFinish=true`인 입장 하나를 0ms부터 재생한다.
Pause/Resume은 현재 연출 시간을 유지하고 Stop/Reset·재생 실패·다른 Preview 소유자 전환은 전투 연결을 취소한다.
`1관문_통합_시퀀스`는 흡입·팝업북·피날레를 포함하며, 종료 뒤 기존 F1 Gate command로 열린 전투 아레나에 진입한다.
입장 Pattern의 Play Sequence도 같은 전투 연결을 사용한다. legacy popup/finale·클리어·카드미로의 개별 Preview는
저작 확인으로 유지하며 자동으로 보스를 생성하거나 Pattern Flow를 시작하지 않는다.

Kouku `Publish All Patterns`는 Product, 쿠크 범위 World, Gameplay balance를 같은 게시 작업으로 처리한다.
F1 Boss Tuning의 Save가 기록한 `Gameplay.world.json` 위치·방향도 Server worldbootstrap에 포함한다.
기존 World publisher의 `-WorldId KAKULSAYDON_ARENA` 범위를 쓰며 다른 World 출력은 변경하지 않는다.
중간 실패는 이전 세 domain 출력과 receipt를 함께 복구한다. 실행 중 Server의 world 상태는 바뀌지 않으므로
게시 후 Server를 재시작해야 Complete Play가 새 위치를 사용한다.

Composition Effect Resources는 V2 GROUP 목록이 기본이며 V1 탭에서 기존 authored Effect와 stable Element를
선택한다. `resourceKind=V1_EFFECT/V1_ELEMENT`와 optional `elementId`는 원본 V1 문서를 참조한다.
Effect occurrence는 BODY/WEAPON named bone, BOSS/WORLD anchor와 local offset을 사용하고,
MAP을 선택하면 고정 월드 위치·회전·크기를 사용한다. MAP은 follow/bone/world 참조를 함께 저장하지
않으며 `Use Player Position`은 현재 플레이어 위치를 occurrence에 복사한다. 같은 규칙은 독립
Sequencer Benchmark에도 적용된다. Effect Tool의 Kouku `Play All`은 현재 플레이어의 위치·방향을
임시 기준으로 사용한다. Append한 occurrence의 앵커와 저장된 Sequence는 이 임시 재생으로 바뀌지 않는다.
이 scene player 등록은 `CClientReplication`이 local spawn/class 교체를 commit한 뒤
`CAnimationTargetService`에 Bind하고 local despawn/reset/destructor에서 자기 캐릭터만 Unbind한다.
원격 player나 실패한 교체는 기존 target을 보존하며, 카메라·입력 연결로 이 등록을 대신하지 않는다.
destructor가 이미 제거된 Layer를 다시 조작하지 않는 종료 계약도 유지한다.
관측한 anchor 기록으로 외부 시계 재생·seek를 처리하며 과거 기록이 없는 구간을 임의 포즈로 보충하지 않는다.

Kouku FEAR Result는 durationMs와 optional sceneProfileId/lightResourceId/effectResourceId/effectDelayMs를
소유한다. 서버의 FEAR action, 시작 tick, 종료 tick, Result logicId가 상태의 정본이며 공포 중 이동·스킬을
차단한다. Client는 각 class 공포 animation과 로컬 플레이어의 Scene/캐릭터 Light/화면 Effect를 그 시간에
맞춰 재생하고 종료하면 기존 표현을 복원한다. 전체 화면 이미지는 V2 ScreenPost Effect로 관리한다.
GAZE_REAL_BOSS의 Fail에 FEAR를 연결하면 시야 밖 보스에 대한 공포가 되고, ENTER_AREA의 Success에
연결하면 따라가는 Collider 접촉 공포가 된다. OBJECT_CONTACT는 World Object 접촉에 사용한다.
COUNTER_WINDOW는 실제 Server counter hit를 소비하며 Success의 FOLLOWUP_PATTERN과
endsPatternOnSuccess로 그로기 후속 재생을 연결한다.
ENTER_AREA의 optional bossChargeDistanceM은 Trigger 시작 시 살아 있는 target의 방향을 한 번 확정해
해당 occurrence duration 동안 지정 거리를 이동한다. navigation/collision이 막으면 경계에서 멈추고,
이동한 Server pose로 같은 tick의 접촉을 검사한다. 절대 bossMotion과의 중복 소유는 거부한다.

Kouku의 `ALBION_BLUE_CIRCLE` Trigger는 시작 시 살아 있는 플레이어마다 고정 장판을 만든다.
Logic의 `countPerPlayer`는 1..8, `radiusM`은 1개일 때 0, 여러 개일 때 (0,20]m이며,
`effectLifetimeMs`는 1..600000ms다. 등록된 `알비온_플레이어장판`은 1개·0m·7000ms를 사용한다.
여러 개는 플레이어 주변 원주에 등간격으로 놓고 Server가 모든 생성점의 navigation을 검사한 뒤
기존 CombatObjectRuntime transaction으로 함께 생성한다. 하나라도 실패하면 기존 객체를 보존한다.
`combatobject.kouku.albion.bluecircle` / `combatvisual.kouku.albion.bluecircle`을 BossCatalog의
`effect.kouku.albion.bluecircle.warning.impact.runtime`에 연결한다. 이 문서가 예고 2초 뒤 폭발과
잔상을 소유하며 Trigger는 피해를 추가하지 않는다. Logic은 원하는 Pattern 시점에 Append하고
Save → Publish All Patterns를 거친다. 단순 Effect/독립 Sequencer 재생은 플레이어별 Server 생성을 실행하지 않는다.
Kouku Arena Loader는 BossCatalog의 해당 family V1 combat-object visuals를 중복 제거해 기존
Product 준비 큐와 level activation probe에 넣는다. 첫 spawn이 prewarm을 생략하거나 Effect
문서를 동기 로드하지 않는다. Client는 자연 완료의 잔상을 유지하고 Server의 Stop/사망/despawn은
level-owned Effect handle까지 정리한다.

단순 피해 영역은 Collider의 Box Detail에서 데미지 모드를 선택하고 최대 HP 대비 피해율,
반복 접촉 정책, 밀림 거리/시간/방향을 직접 편집한다. 내부적으로 기존 ENTER_AREA와
MAX_HP_PERCENT_DAMAGE Result를 원자적으로 연결하며, 별도 Client 피해 런타임을 만들지 않는다.
같은 설정의 definition은 재사용하되 설정 변경은 선택한 occurrence의 연결만 바꾼다.
다른 영역과 공유하는 Trigger/Result를 직접 수정하지 않고, 기존 특수 Logic과 outcome은 보존한다.
Result의 optional `pushDirection`은 `AWAY_FROM_BOSS`가 기본이며 `BOSS_FORWARD`는
해당 hit 시점 보스의 yaw를 사용한다. 후자는 양수 pushRangeM/pushMs 쌍이 있어야 한다.
navigation/collision과 snapshot은 기존 Server hit reaction 경로가 계속 소유한다.

Geometry Collider resource의 `SECTOR`와 `REVERSE_SECTOR`는 occurrence Scale X/Z를
독립적으로 사용한다. 각도는 축척 전 원의 각이며, Reverse는 같은 타원 안에서 지정한
safe sector를 제외한 영역이다. reverse halfAngle0은 전체 타원,180은 빈 영역이다.
생성된 region의 optional `radiusXM/radiusZM`은 쌍으로 저장하며 누락 시 기존 radiusM을
양 축에 사용한다. Shared ellipse-sector 교차는 metre 단위 플레이어 원과 타원 arc/방사
선분의 실제 최단거리를 사용한다. Client wire는 같은 독립 반경·각·reverse를 표시한다.
Collider 자체의 X/Z 축척을 허용하는 변경이며 World Track의 기존 균일 X/Z 축척 계약은 유지한다.

잡기는 짧은 ENTER_AREA Trigger의 Success에 CAPTURE_PLAYER를 연결하고, occurrence의
`holdLogicOccurrenceId`로 같은 패턴의 DURATION `ATTACHMENT_HOLD`를 참조한다. Hold는 판정이나
outcome 없이 붙잡힌 상태의 종료 시각만 소유한다. 늦게 잡힌 대상도 원래 Hold 끝 tick에 해제하며,
Collider 종료는 잡기 해제가 아니다. Server는 실제 attachment commit에 성공한 뒤에만 Success를
확정하고 Hold 종료·Stop/Restart·보스/대상 사망·owner sequence 변경 때 기존 Release 경로를 사용한다.
FEAR와 GRABBED는 서로 덮어쓰지 않는다.
CAPTURE_PLAYER의 `attachmentSlot=BOSS_LEFT_HAND`와 `gripLocalOffset {forwardM,upM,rightM}`는
패턴별 `attachmentGrips` 배열로 patternbindings에 투영하며 같은 패턴의 서로 다른 grip은 거부한다.
Client는 같은 snapshot의 owner pattern과 게시 revision으로 grip을 선택한다. CNpc의 실제 BODY
`bip001-l-hand` socket에 owner yaw 기준 forward/right와 world up의 미터 단위 보정을 합성하고,
기존 `IPlayerHandGripSocketSource -> CCharacter::Apply_NetworkAttachment`를 사용한다.
Grip/animation 재로드 실패는 이전 캐시를 보존하며 owner·bone·grip 표현이 없으면 Server 위치를 유지한다.

F1 `Effect Tool V1`과 `Effect Tool V2`는 별도 버튼·창·입력 focus·visibility로 연다. V1은 Current Effect·Effect Detail·Model View·Effect Resources·Effect Sequencer를, V2는 자기 CPU draft·Resources·Sequencer와 기존 target attachment 도구를 소유한다. 한 도구를 닫아도 다른 도구의 창과 draft를 닫지 않으며 각 Sequencer의 창 ID와 기본 저장 ID를 구분한다. 이전 Effect Composition Workbench enum은 V1 호환 진입점이다. 각 Resource 트리는 자기 V1 또는 V2 root만 표시하고 typed resource open은 해당 도구로 전달한다. Parent와 표시 이름은 `Data/Effects/EffectResourceTree.json`의 stable reference metadata로 저장하며 V1/V2 Effect body의 원본 경로·codec을 변경하지 않는다. Tree 조회는 metadata만 읽고 선택한 파일의 Open/Play에서 필요한 항목만 stage한다.

V1 Full Restore의 `Effect Detail → Skill Bloom Intensity`는 선택한 문서 전체의 bloom 강도를 즉시 조절한다. optional JSON root `bloomIntensity`의 기본값은1.3, 범위는유한한0~16이며 `Save Changes`로 저장한다. 같은 스킬의 단계별·clip별·통합 문서도 각각 독립 값이다. 원본 HDR 색과 다른 문서 값은 유지하고 전역 intensity를 추가로 곱하지 않는다. 상세 출력·후처리 경계는 [Effect 문서별 bloom 계약](EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md#문서별-bloom-기여)을 따른다.

Current Effect의 Play All/Family/Element는 미리보기이며 Append만 별도 Effect Sequencer에 occurrence를 추가한다. 캐릭터 skillbinding·Valtan Product·Kouku Pattern/Bundle의 실제 clip sequence는 읽기 전용 모델 참고이며 저장 단위는 `Data/Effects/Sequences/<id>.effectsequence.json`의 stable source reference와 occurrence 시간이다. 해당 Save는 boss Composition이나 skillbinding을 변경하지 않는다. Native V2 leaf Open/Save는 원래 leaf ID/파일을 유지하며 group으로 확장하는 것은 명시적 생성 명령이다. Effect CPU draft 저장에 GPU preview나 타 보스 전체 admission을 선행조건으로 붙이지 않는다.

Kouku의 독립 V1 Effect/Element Resource에 `sourceModelPreview`가 있으면 그 원본 actor·clip·Source In을
기존 CNpc/CModel Preview에서 함께 준비한다. Effect의 마지막 입자까지 마지막 pose를 유지하며
빈 Resource용 Stage에서 본 애니메이션을 조회하지 않는다. 이 독립 Preview 정책은 제품 Pattern의
저장된 animation/history 계약을 변경하지 않는다.

쿠크 금빛 이동 축포 `effect.kouku.gate1.intro.gold-trails.full.restore`는 기존 V1 문서의
4경로·24행, tail 포함 9413ms resource다. Effect Tool의 독립 Play All과 Composition의 명시적
Append에서 사용한다. 원본 `.matinee_0.1/.2` source와 사용자 `authored.portal-arrival.1/.2`는
같은 Move·loop 교정을 소비하며, 현재 P4 `.presentation.20/.21`은 authored 문서를 저장된
시각·MAP 위치로 참조한다. 독립 문서의 0초 시작과 floor 기준을 기존 P4에 덮지 않는다.
Matinee Move는 SourceTransformTrack, 거리 방출은 SpawnPerUnit, 잔광은 기존 world particle·cascadeRibbonV1가
소유한다. 새 gameplay 이동 권위나 별도 렌더 경로를 만들지 않는다. 원본 camera cut 시점의
위치 도약과 사용자 화면 미확인 범위는
[금빛 이동 축포 결과 G07](../GB/09-11/2026-09-11_KOUKU_PLAYER_ANCHOR_RAINBOW_FIREWORKS_IMPLEMENTATION_RESULT.md)을 따른다.

World Object category는 Area 저작 `objectResources`의 model/base texture/pre-scale과 자식 Motion을 읽어 현재 Effect draft에 적용한다. 원본 Object/Motion은 수정하지 않는다. 공의 `objectMotion` velocity/acceleration/count/interval/lifetime은 기존 Mesh Particle로 옮기며 실제 충돌 물리를 추가하지 않는다. 모델 참고는 기존 Kouku preview actor/CModel을 재사용하고 root motion·WORLD gameplay는 실행하지 않는다. 실제 Product 이동 입자는 Server presentation root의 birth 시각 표본을 사용하며 과거 표본이 없으면 해당 Effect 오류를 표시한다.

Kouku Action Workbench Camera는 이름으로 shot 생성 → 현재 view의 eye/lookAt/FOV capture → Box Detail의 blend-in/default hold/blend-out → Save → Append 흐름을 제공한다. `activation: "PATTERN_ONLY"` shot은 명시적으로 배치한 Pattern/Bundle Camera에서 소비하며 저작 Preview는 저장본, Complete Play는 Map publisher 배포본을 읽는다. 진입 pose는 시작 때 취득하고 복귀 목표는 매 프레임 현재 player follow pose를 사용한다. Camera box duration은 진입+유지이며 복귀 tail은 별도로 검사한다. 기존 Valtan 문서 제한을 넓히지 않고 Kouku의 LINEAR/SMOOTHSTEP 전환을 지원한다.

F1 `Cinematic Camera Tool`의 Kouku Area source도 같은 Area `camerashots.json`의 PATTERN_ONLY shot을
Cut List/Capture Pos/Keyframe Editor로 편집한다. 표시 이름은 UTF-8 `displayName`, 참조는 변경하지
않는 stable `shotId`다. Save는 parser 검증과 디스크 baseline CAS를 거치며 AUTO/마리오 shot과
다른 도구의 변경을 보존한다. 복귀는 마지막 key에 플레이어 좌표를 저장하지 않고 `blendOutMs` 동안
현재 follow pose로 보간한다. Kouku Area cameraTrack은0ms의 P1 한 개를 허용하며
이 경우 durationMs 동안 같은 Eye/LookAt/FOV를 유지한다. 둘 이상이면 마지막 key는
durationMs여야 한다. P1 수정은 Fine Adjust 또는 Capture / Replace Selected Scene을 사용하고
Capture Pos는 새 Pos를 추가한다. Valtan 문서는 최소 두 key를 유지한다.
플레이어 복귀를 확인하려면 F6 Follow 상태에서 Play Bundle을 시작한다.

F1 `World Object Tool`의 정의와 이름을 가진 상태는 Area의
`Data/Maps/Authoring/<Area>/<Area>.worldsequences.json` v3에 저장한다. `objectResources`의 모델은
stable `objectId`를 `OBJECT_RESOURCE` binding으로 연결하고, 기존 커튼·룰렛은 stable
`sequenceInstanceId`를 참조하는 별칭으로 재사용한다. 상태는 기존 template의 transform/animation
track과 `objectMotion`, instance의 `anchorKind=WORLD|PLAYER`·`position`을 소비한다. 저장 schema와
생성 개수·간격·수명 제한은 [Area 데이터 레이어 가이드](AREA_DATA_LAYER_GUIDE.md)의 WorldSequence
계약을 따른다. Action Workbench `World` resource의 `Append selected World Object at Cursor`는
저장된 상태를 기존 WORLD 정의와 occurrence에 연결한다. Map publisher가 배포한 상태를
`CWorldSequencePlayer`가 동일하게 재생하므로 Effect나 별도 오브젝트 runtime을 추가하지 않는다.
부모는 공통 Object 설정, 자식 instance ID는 독립 저장 Motion이다. Collider/Logic Result의
`PLAY_WORLD_OBJECT_MOTION`은 `targetWorldInstanceId`와 `motionInstanceId`를 저장한다.
Server가 판정을 확정하면 `S2C_WORLD_SEQUENCE_PLAY`의 `strTargetSequenceInstanceId`에 기존
target을, `strSequenceInstanceId`에 재생할 Motion을 보내고 Client는 같은 객체에서 모션만 교체한다.
target 필드가 비어 있는 기존 메시지는 WORLD 생성 요청이다. Result는 같은 판정 창의 같은
target/Motion 쌍에 한 번만 송신한다. `OBJECT_OVERLAP`은 player 입력과 무관하게 source Collider와
고정 target 원의 겹침을 Server fixed tick에서 확인한다. target 반경은 저작 값이며 모델의 bone
변형을 추적하지 않는다. 기존 player 영역 판정은 계속 별도 Logic 종류로 유지한다.
`OBJECT_CONTACT`는 같은 Pattern의 stable `targetWorldOccurrenceIds`를 대상으로 판정하며
`PLAY_CONTACT_WORLD_OBJECT_MOTION`은 접촉한 occurrence의 기존 객체에만 Motion을 적용한다.
동일 `contactGroupId`의 같은 타격 창은 시간·수명을 일치시키고 서로 다른 `contactPriority`를 사용한다.
큰 priority가 같은 카드의 반응을 우선하며, 적용한 Motion priority를 Pattern ledger에 보존하여
뒤집기 `HOLD` 뒤의 낮은 priority 들썩임이 그 카드를 Idle로 되돌리지 않게 한다. 같은 priority의
다음 타격은 허용하고 새 Pattern/Reset에서는 ledger를 초기화한다. `COMPLETE_LOGIC_WINDOW`의
`contactTargetWorldOccurrenceId`는 지정 카드 접촉만 전체 기한을 성공시킬 수 있다. 짧은 접촉 창의
Miss는 전체기한 Timeout이 아니며, 전체기한 마지막 tick의 접촉 성공을 Timeout보다 먼저 처리한다.
LOGIC occurrence의 optional `enabled:false`는 저작 창·RESULT를 보존하고 gameplay 투영에서 제외한다.
TRIGGER의 `REAL_GAZE_TELEPORT`는 명시한
teleportPosition과 clonePatternId, clockHours를 사용해 Server가 진짜 이동과 clone 3개를 함께 commit한다.
`HUD_ENTER` trigger는 hudMode를 적용한다. 이름만 있는 기존 TRIGGER는 실행 동작을 추측하지 않는다.
각도·거리 설정은 F1 시야 패널에서 Composition 정본에 저장하고 Gameplay publish 및 Server 재시작으로 적용한다.

optional `presentationResources/presentationOccurrences`는 EFFECT group/leaf, SOUND, CAMERA와
COLLIDER, LIGHT를 소유한다. LIGHT는 `LightResources.json` 또는 Area v2 map light의 stable ID를 참조하며
MAP 고정 위치, PLAYER 살아 있는 복제 캐릭터 각각, BOSS 현재 pattern 소유자의 anchor와 box 수명·fade·brightness 배율을 소비한다.
Server protocol/gameplay는 조명 값을 소유하지 않는다. WORLD/SCENE_PROFILE은 기존 정의·배치를 유지한다. 명시 publish한 Client
Product `patterns[]`와 v63 보스 snapshot의 pattern ID/start tick/sequence를
`CKoukuSaydonPresentationPlayer`가 소비한다. Product parse·validate·stage 실패를 authoring 직접 읽기로
우회하지 않는다. Preview Play/seek/pause/Stop과 owner 종료는 자신이 만든 effect/sound/camera handle을
정리하고 Scene Profile을 복원한다. Scene Profile 전환은 현재 즉시 적용이며 blend 시간 보간은 지원하지 않는다.
Effect box의 수명·fade·dissolve·transform은 clone별 override이고 원본 V2 asset은 유지한다.
WORLD 정의의 optional `companionEffectResourceId`는 같은 Composition의 EFFECT resource를,
EFFECT box의 optional `worldOccurrenceId`는 같은 pattern의 WORLD box를 참조한다. World
Preview/Append는 두 항목을 함께 준비하고 제품은 명시적으로 저장된 Effect box만 재생한다.
연결된 Effect box의 시간·속성은 독립 편집할 수 있다. Preview의 source draft generation 변경은
현재 clock에서 다시 stage하며 실패 이유와 정상 기존 항목을 보존한다.
Effect V2 `TexturedOverlay`는 기존 `Add_ScreenOverlay`를 사용한다. 전체화면 커튼
`boss.kouku.curtain_1`의 원본 texture ID는 `Effect/KoukuSaydon/Screen/fx_d_symbol_100_ycl.dds`이며
World mesh나 카메라 위치를 바꿔 화면 효과를 흉내 내지 않는다. 이 profile은 alpha envelope를 사용하고
texture dissolve는 사용하지 않는다. 원본 texture 근거와 조정한 이동 시간, Resources 전달 및
사용자 화면 검증 상태는 대응 RESULT에 기록한다.

Collider Detail에서 새 Logic을 연결하는 `Apply Values`와 `Apply`는 정의값·Logic 시간창·`logicOccurrenceId`
연결을 `Set_ColliderLogicValues`의 한 validated candidate로 commit한다. 같은 정의의 정확한
start/duration만 재사용하며, 다른 타격 구간은 독립 window를 생성한다. 명시 `Shared Logic window`
선택은 그 창의 시간·결과를 유지한다. 이름만 있는 Trigger나 잘못된 target은 오류를 표시하고
기존 draft·ID counter·저장 원문을 보존한다. 실제 source 교체는 기존 `Save`의 CAS 경로로 수행한다.

Collider Box Detail의 position/rotation/scale·치수 편집은 stable Pattern/occurrence의 geometry만
즉시 Preview하며 Apply/Save 전에는 draft나 원본을 바꾸지 않는다. active Pattern/Bundle member의
clock·actor·Effect/SFX session을 유지하고 inactive만 현재 cursor에서 paused Preview를 준비한다.
Collider geometry 편집은 선택 전환 뒤에도 보존하며 Apply 없이 Save 후보에 모아 CAS 저장한다.
Revert geometry는 선택한 box를 적용된 값으로 복구하고 Reset은 임시 요청을 폐기한다.
Save는 저장할 변경이 있을 때 활성화한다. 미적용 시간·Anchor·Logic은 기존 Apply를 사용한다.
Logic이 연결된 지면 gameplay Collider는 Yaw만 편집하며 X/Z 기울기는 Publish All Patterns의 실행 검증에서 거절한다.
이 gameplay Collider의 SECTOR 크기는 Radius로 X/Z를 함께 변경한다. 기존 X/Z 불일치 값은
Detail에 이유를 표시하고 Radius에서 수정한다. 잘못된 geometry의 Save 실패는 편집값과 이전
원본을 보존한다. Publish 성공은 정상 항목의 배포 성공이며 재생 불가 항목은 F1의 사유를 확인한다.

V2 Effect Box Detail의 Position/Rotation/Scale은 같은 Pattern/Bundle의 현재 actor·bone·WORLD와
clock에서 즉시 Preview한다. Box Preview도 그 소유자를 사용하며 독립 Resource Preview는 별도다.
여러 Effect box의 geometry 편집은 선택 전환 뒤에도 보존하고 Apply 없이 Save 후보에 모아 CAS
저장한다. 미적용 시간·Bone·Logic까지 저장하려면 Apply를 사용한다. invalid/CAS 실패는 원본을
보존하며 Revert geometry는 선택한 box만 복구한다. V2는 기존 객체와 시간·다른 cue를 유지하며
해당 Effect의 particle/trail을 같은 age로 다시 계산한다. 기록되지 않은 과거 anchor는 오류로
격리하고 현재 actor pose로 위장하지 않는다.

Collider resource는 `shape`, 크기, `colliderKind=GEOMETRY|ROULETTE_CARD_REGION`을 정의한다.
각 box는 `regionId`, `cardSymbol/cardColor`, `anchorKind=BOSS|WORLD`, `worldId`와
`logicOccurrenceId`를 소유한다. WORLD 앵커는 실제 sequence placement TRS를 사용하고 Product에는
`worldSequenceInstanceId`를 투영한다. `ROULETTE_CARD_MATCH`는 연결된 지역에서 창 종료 tick의
플레이어 XZ와 문양·색을 비교한다. 일치 success/불일치 fail/지역 밖 timeout은 연결된 기존 RESULT를 실행한다.
일반 영역은 `CIRCLE`을 포함하며 DURATION `AREA_OVERLAP`의 `insideOutcome=SUCCESS|FAIL`이
안쪽의 결과 슬롯을 정하고 바깥은 기존 Timeout을 소비한다. TRIGGER `ENTER_AREA`는 최초 진입
Success, 미진입 Timeout과 기존 `MAX_HP_PERCENT_DAMAGE` 최대 HP % RESULT를 재사용한다.
WORLD ENTER_AREA는 같은 pattern의 WORLD box, 단일 MAP_PLACEMENT baseline TRS와
animated key를 투영해 Server tick에서 샘플한다. WORLD 수명 밖의 창과 잘못된 binding/transform은
publisher가 거부한다. `debugRender`(기본 true)는 wire 표시만 바꾸며 판정의 enabled 값이 아니다.
Client `CHitAreaWire`는 debug mirror이며 PhysX collider가 hit·카드 판정 권위를 갖지 않는다.
다중 family 전체를 위한 범용 arena scene runner는 별도 범위다.
기존 Valtan Sequencer의 Sound/Effect/Logic/Collider/Camera 편집 기능은 기존 경로에 남아 있다.

`Data/Actors/BossCatalog.json` format v5의 현재 Valtan `presentationScale: 1.0`은 replicated Arena와 Character/Boss
Preview가 함께 소비하는 Client actor scale이다. 일반 `BOSS_VALTAN.maximumHp`는 `600000`, 종속
`BOSS_VALTAN_GHOST.maximumHp`는 `60000`이며 health bar 수는 `160`을 유지한다. `BossProfiles.json`의 Server body radius는 scale-one 모델의
몸통·다리 실측에 맞춘 `1.4m`를 유지하며 Client Debug collider는 같은 replicated radius를 표시한다. 이 의도적인
시각 배율을 body radius에 다시 곱하지 않는다. 공격 hit geometry와 장판 크기는 별도 저작 값이다.
Effect cue의 scale policy는 `OWNER_RELATIVE`, `GAMEPLAY_FOOTPRINT`,
`ARENA_ABSOLUTE`로 구분한다. owner-relative만 actor scale을 상속하고, 나머지 두 policy는 owner scale을 제거한
뒤 authored `worldScale`을 사용한다. 이 정책은 호출 transform 계약이며 `Data/Effects/Authored`의 element geometry를
고치거나 sky-axe에 actor scale을 강제하지 않는다. Client `CValtan`의 로컬 AI는 Development preview 외 제품 정답이
아니다. 세부 필드와 publish 절차는 `발탄인수인계서.md`를 따른다.

v5의 `bodyModelPreScale/weaponModelPreScale`은 모델 단위를 맞추는 admission 값이다. 본체는
`0.0001/100.0`, `BOSS_VALTAN_GHOST`는 `0.01/1.0`을 사용한다. 종속 유령 entity는 기존 Server BOSS
spawn/snapshot/despawn 경로와 `iOwnerBossNetEntityId`를 계속 사용하지만, phase 3 primary 망령화는
별도 damage entity를 만들지 않는다. Server의 primary archetype/NetEntityId/HP/damage/HUD/reward owner는
`BOSS_VALTAN` 그대로이고 Client만 `BOSS_VALTAN_GHOST` body/weapon part group을 원자 교체한다. 부활 완료
뒤 primary는 Six Pizza/Ground Roar/Stagger/Bind/Silence/Triple Counter exact 6-pattern을 순환하며, 독립
150-tick scheduler가 immutable arena center 기준의 네 radial slot에 portal combat object 네 개를 같은 tick에
생성한다. 각 object는 `NEXT_RADIAL_SLOT`으로 0->1, 1->2, 2->3, 3->0의 사각형 네 변을
동시에 질주한다. authored world angle은 45/135/225/315도이며 네 개는 독립 boss-body가
아니라 portal presentation을 가진 combat object다. primary HUD/BGM/툴 target은 계속 본체만 소유한다.
protocol v44는 이 owner와 typed 사망 despawn reason을
추가하므로 Server/Client를 함께 배포한다. Server는 HP 0에서 전투 entity를 제거하고 Client만 유효한
사망 clip 종료까지 기존 presentation을 보존한다. clip 없음·재생 실패·퇴장은 즉시 정리한다.

Gameplay bootstrap v26은 `PORTAL_CROSS_ARENA`, `RETURN_TO_ARENA_CENTER`, `ARENA_EJECTION`,
`NAVIGATION_BLOCKED`, `GHOST_PORTAL_LOOP`와 중앙 접근 옵션을 소비한다. `arena.center`는 중앙/yaw 0,
`arena.center.facing`은 중앙/해당 occurrence의 서버 확정 yaw를 사용하는 snapshot cue anchor다.
`arena.center.target-follow`은 `LOCK_RANDOM_ALIVE_ON_START`가 고른 동일 player ID를 유지한 채 Server가
매 fixed tick 저작 landing center→현재 target 위치로 계산한 yaw를 사용하는 follow cue anchor다. Client는
복제된 현재 boss yaw로 기존 world-root handle만 갱신하며 target을 다시 고르거나 player yaw를 사용하지 않는다.
도넛은 `SPAWN_COMBAT_OBJECT`로 생성한 2600ms 독립 object이며 foreground INNER는 100ms다.
기존 stage 피해와 cue를 중복 재생하지 않는다. 일반 element의 world birth anchor는 생성 tick root를 사용하고,
Six Pizza의 정적 sector처럼 root를 계속 따라야 하는 particle만 authored local-space를 명시한다. element 자체의
scale curve는 계속 진행한다. 세부 ID와 실패 경계는 `발탄인수인계서.md` 11.9~11.10을 따른다.

Valtan Boss Tool의 Next는 live Product, 같은 owner의 Flow/isolated 또는 idle에서 선택하는 Server 권위 예약 한 칸이다.
현재 패턴의 최종 world/prop/hit commit 뒤 다음 fixed tick에서 시작하며 맵·플레이어·HP·cooldown을 reset하지 않는다.
Flow 중에는 현재 occurrence 뒤 남은 재생만 종료하며 저장 배열을 수정하지 않는다. 공용
`CValtanPatternAuditionService`가 isolated/Next lifecycle을, `CValtanPatternFlowService`가 Ordered Flow lifecycle을
각각 소비한다. UI가 NetworkManager queue를 직접 나눠 읽지 않는다. `Save Flow`는 현재 slot을 inline
`decisionModel.scriptedSequence` draft로 stage하고 gameplay와 generated Product를 한 canonical transaction으로
commit한다. 물리 commit 성공과 editor reopen/candidate apply 상태를 구분하며, 성공 즉시
`Boss Verification -> Current Patterns`가 saved order를 표시한다. 실행 중 Flow는 pinned 이전 revision을 중간
교체하지 않는다. `Load Flow`는 `Valtan.gameplay.json` saved sequence를 다시 읽을 뿐 playback을 바꾸지 않는다.
`Restart Flow (Fresh Arena)`는 최신 saved candidate와 Server-active gameplay revision이 같은지 확인하고,
저장된 scriptedSequence를 다시 읽은 뒤 walls/floors/props/collision/navigation/combat objects를 Server에서
초기화하고 그 동일 revision의 첫 배열 슬롯(화면 01)부터 새 FLOW_START를 제출한다. Boss Verification의
`Restart Saved Pattern (Fresh Arena)`도 saved slot이 정확히 하나일 때 이 transaction을 공유한다. 순서와
중복 Pattern을 보존하고 admission/reset 실패 시 새 playback을 시작하지 않는다.
예약·적용 상태·전멸 대기·취소와 Trash 포획 분기의 상세 계약은 `보스툴.md`와 `발탄인수인계서.md`의 10.4, 11.9를 따른다.

F1의 `Valtan Logic Pattern`은 Valtan Boss Tool과 동급인 독립 대형 창이지만, 소유자는 같은 하나의
`CValtanBossTool`이다. 선택 또는 live Pattern의 Stage role, 실제 clip, authored/cross-pattern branch,
counter proxy, response threshold/progress, status action과 damage profile을 읽기 전용으로 그린다.
연속 Server snapshot의 source/target이 단 하나의 저작 edge와 맞을 때만 observed branch로 표시하며,
여러 outcome이 같은 target을 가리키면 추측하지 않는다. graph, Server 상태와 world/preview를
수정하거나 input owner를 가져가지 않는다.
`Save Flow Product publish 성공 -> Product-synced Current Patterns 갱신 -> 같은 revision으로 Restart Flow -> 01 재생`과
live 강조·시각 가독성은 사용자가 Debug Server와 Client에서 직접 smoke한다. 에이전트는 Client/UI를 자율 실행하거나
visual PASS를 대신 판정하지 않는다.

패턴 선택은 `Build_PlayablePatternInventory`가 strict joined split 정의에서 만든 공통 집합을 사용한다.
Valtan Boss Tool·Play/Repeat·Next·Flow·All Effects와 publisher 사이에 별도 고정 개수나 Core ID 목록을 두지 않는다.
Core/Animator/Derived는 표시 분류이며 등록 총수는 Flow의 1~255슬롯 U8 전송 용량과 별개다. 256슬롯은
저장·Product 투영·Client 전송·Server catalog에서 거부한다. 유령 finale와 도넛도
같은 집합으로 선택한다. 새 패턴의 stable ID·stage/action 연결·소유자 검증은 유지하며 상세 확장 규격은
`보스툴.md`의 Pattern 목록 정본을 따른다.

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
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate
```

위 publisher는 parse → validate → stage → commit을 따른다. unknown field, schema/version 오류, 중복 stable ID, 잘못된 참조, non-finite 위치, navigation 밖 spawn을 정상값으로 보정해 숨기지 않고 실패시킨다.

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

`Client/Bin/Resources`는 `Fonts, Character, Deploy, Effect, Map, Sound, UI` 일곱 root만 허용한다. asset ID는 Resources 상대 경로이며 절대 경로, drive-qualified 경로, `..` 탈출을 금지한다.

runtime payload는 팀장이 `Client/Bin/Resources` 물리 폴더로 관리하고 Git에는 추적하지 않는다. AssetPacks lock, immutable Resource manifest, Resource ZIP hash, Snapshot/Publish/Hydrate/Verify를 팀 완료 조건으로 사용하지 않는다. 코드와 데이터에는 Resources 상대 asset ID만 저장하고 팀원별 절대 경로 하드코딩은 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ 팀장이 전달한 Resources 물리 폴더 확인
→ Debug 전체 회귀
→ 담당 public interface에서 작업 시작
```

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, 소비 데이터, project/filter 등록, harness, RESULT를 같은 검증 단위로 묶는다. build output, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, `Client/Bin/Resources` payload를 stage하지 않는다.

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
- generic Boss Composition writer와 multi-owner Save transaction
- resolved Composition Product의 Client/Server runtime consumer와 공통 Sound/Camera/UI transport
- KoukuSaydon Mario2~4 진입 데이터와 범용 Arena Sequencer scene runner

이 항목들은 현재 인터페이스를 우회해 임시 구현하지 않는다.


WorldSequence v3 instance의 optional `walkableSurface { radiusM, localHeightM }`는 단일 MAP_PLACEMENT의 고정 수평 원판과 WORLD/STOP에 한정한다. Kouku WORLD cue의 활성 구간이 Server effective support height를 소유하며 기본 blocked/NO_SURFACE와 step policy는 유지한다. Motion Detail에서 설정을 저장한 뒤 Map 및 Kouku Product publisher를 소비한다. Publish All Patterns에서 실행 조건을 충족하지 못한 패턴은 전체 트리에 사유와 함께 남지만 실행 데이터에는 포함되지 않는다. 클라이언트의 로컬 Transform으로 서버 보행면을 대신하지 않는다.

### 카드미로 진행·관전 계약

MAZE 망치 타격 → Server의 자기 문양 한 방 처치 → 3스택 개인 출구 → 암전 중앙 이동 → 생존 참가자 전원 집결 후 2관문 복귀를 사용한다. 문양별 목표는 동시에 1마리이며 3스택 전까지 랜덤 통로로 보충한다. 중앙 반경 5m를 제외한 세토 접촉은 본인 스택·출구를 취소한다. `cardmiro.march.instance.from{3,6,9,12}.lane{1..9}` 36개 경로는 WorldSequence 정본의 선형 키를 WorldGameplay publisher가 worldbootstrap v10에 투영한다. Server 판정과 Client 표현은 protocol 81의 `PLAYER_SNAPSHOT::CardMaze` 행진 시계를 함께 소비한다.

카메라는 MapTool Camera의 `cardmaze.follow`/`cardmaze.telescope`에서 조정하고 MapAuthoring을 publish한다. 관전은 역할 이름이 아니라 플레이어별 관전 flag로 켜진다. 최초 담당과 탈출자는 중앙 상자를 망치로 다시 가격하여 각각 토글한다. 이동 암전은 서버 시작 tick 기준 36tick, 위치 commit은 18tick이다. 최종 복귀는 World Gameplay의 disabled `cardmaze.return` movePlayer 목적지를 읽으며 기본은 기존 2관문 (3.38, 10.56, 323.92)이다. 이 행을 활성화하면 밟기 트리거로도 동작하므로 설정 전용으로 disabled를 유지한다. WorldGameplay publish와 서버 재시작이 필요하다.

문양 플레이어·병사 발밑은 `cardmaze.mark.heart/spade/club/diamond`, 개인 출구는 `cardmaze.exit.heart/spade/club/diamond` Effect GROUP을 사용한다. 두 그룹은 같은 `cardmaze.symbol.*` Decal leaf와 기존 `Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47{,_1,_2,_3}.dds`를 사용한다(하트/스페이드/클럽/다이아몬드 순서). 기존 춤 연출은 수정하지 않는다. 플레이어/병사 표시는 해당 객체의 이동 위치만 따라가고 출구는 서버 출구 좌표에 고정한다. Client는 이동·처치 판정 없이 snapshot과 복제 객체로 표시·정리만 한다. 다인 플레이와 Release의 망원경 담당은 문양과 목표를 받지 않는다. Debug Server에서 방에 정확히 한 명이면 최초 상자 타격자가 HUNTER 문양과 망원경 owner identity를 함께 받는다. 이 1인 테스트만 관전 중 이동과 중앙 밖 관전 유지가 허용되며 세토 접촉·처치·출구는 기존 규칙을 사용한다. 다른 참가자가 죽어 혼자 생존한 상황은 이 예외를 켜지 않는다. Protocol 79 Client/Server를 함께 사용한다. 화면 크기·색상·실제 4인 입력·접촉·암전은 사용자 런타임 확인 대상이다.


### 입장 Sequence와 열린 아레나 전투 연결

Kouku Sequence Composition의 Pattern은 optional `enterCombatOnFinish`를 갖는다. GATE1/GATE2/GATE3별로 이 값이 true인 Pattern 정확히 하나를 Complete Play의 입장으로 선택하며, 해당 입장의 Play Sequence도 0ms부터 같은 경로를 사용한다. legacy popup/finale·클리어·카드미로는 자동 순회 대상이 아니다. 데이터와 새 소비 코드가 함께 설치돼야 하며 기존 Client에 새 metadata 문서를 먼저 배포하지 않는다.

MainApp은 입장 중 gameplay 입력과 보스 HUD를 보류하고, 종료 뒤 기존 Debug Gate command를 제출한다. Level은 Server 보스 생성 및 플레이어 이동 승인이 모두 확인된 뒤에만 gate를 확정한다. 성공하면 follow camera/HUD를 복귀하고 입장 시작과 같은 revision의 Pattern Flow를 제출한다. F1의 기존 1관문·3관문은 SL05 열린 전투 아레나를 즉시 시험하는 경로를 유지한다. Client transform이나 cinematic 프록시를 서버 전투 권위로 사용하지 않는다.

Object Tool의 Group Layout은 motion emission의 count·spacing·Delay를 소유한다. Box Detail의 Object/Motion 열기는 stable object ID와 instance ID를 전달하며 자동 preview·저장을 하지 않는다. WORLD Collider/전용 Logic 참조를 함께 바꾸는 저장은 source 변경을 재확인한 뒤 교체하며, 공유 또는 모호한 참조와 미저장 Composition은 이유를 표시하고 기존 문서를 보존한다.

Kouku Animation occurrence의 `sourceStartMs`와 optional `sourceEndMs`는 원본 클립의 선택 구간이며, `sourceEndMs=0` 또는 생략은 원본 끝이다. `startOffsetMs`와 `playMs`는 stage 안의 재생 위치와 길이를 소유한다. `LOOP_TO_WINDOW`는 선택한 source 구간만 반복하며 stage 길이는 별도로 편집한다. Source In/Out은 원본 구간만 바꾸고, 일반 clip 앞 edge는 timeline/source 시작을 함께 자른다. 반복을 켠 뒤 edge 편집은 반복 구간을 유지하며 timeline 길이만 바꾼다. 인접 clip의 Blend In은 이전 clip의 실제 HOLD/LOOP 종료 sample과 보간한다. Preview, 제품 NPC, Server용 bone collider bake는 같은 source 시간 계약을 사용한다. Product는 기존 stage당 animation 하나와 Server action clock을 유지하며 저장 후 기존 publisher를 거쳐 적용한다.


## C++ 공용 선언을 소비할 때의 include 경계

`Engine_Defines.h`에 기능별 헤더를 추가해 모든 담당자에게 전파하지 않는다. 렌더 설정은
`Engine_RenderTypes.h`, 정점은 `Engine_VertexTypes.h`, keyframe은
`Engine_AnimationTypes.h`, 초기화는 `Engine_InitTypes.h`를 실제 소비 파일에서 포함한다.
`GameInstance.h`는 렌더 API의 전방 선언만 제공하므로 반환값을 사용하는 CPP는 해당 정의를
직접 포함한다. Assimp·DirectXTK·FX11·DirectInput도 실제 구현의 의존성으로 선언한다.
표준 라이브러리 PCH·대형 CPP 분리·병합 후 Build 확인은 `../../Tools/Build/README.md`를 따른다.
