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

Character Select의 `Create Character`는 선택 class와 공통 validator를 통과한 1~32-byte UTF-8 nickname을 `CCharacterSelectionState`의 pending identity로 stage한다. Lobby가 그 exact identity로 Bern entry를 승인받고 loading resource, rendering profile, 실제 `Change_Level(BERN)`까지 성공한 뒤에만 created identity로 commit한다. 중간 실패는 pending만 취소하고 기존 created identity는 유지한다. created identity가 없는 direct Bern, Character Select, Training, Valtan, KoukuSaydon entry는 process-local `Test-<process-id>` audition nickname을 사용한다. Bern은 pending 생성이 있으면 이를 우선하며, Lobby/F1의 직접 audition 입장은 생성 commit을 만들지 않는다. Server의 `SERVER_PLAYER::strNickName`과 world transfer가 session lifetime 동안 exact nickname을 보존하고 `S2C_PLAYER_SPAWNED`로 복제한다. nickname은 display text이며 player lookup, Party member ID, 고유성 검사 또는 Client 재실행 뒤 영구 저장에 사용하지 않는다. Bern과 Valtan은 `CClientReplication::Collect_PlayerViews`의 Server-replicated nickname과 weak character presentation을 `CWorldPlayerNameplateView`에 전달한다. projection, UTF-8 변환, font draw 실패는 gameplay와 replication을 건드리지 않고 해당 nameplate만 생략한다.

2026-09-30 23:59 KST까지 공유 LAN Server는 같은 팀 LAN의 `192.168.0.22:7777`이다. Server PC는 현재 `Wi-Fi 2`에서 `192.168.0.22/24`를 소유한다. Server는 `0.0.0.0:7777`에 수신하고 Server PC와 다른 PC의 Client는 모두 concrete endpoint `192.168.0.22:7777`을 사용한다. `Tools/Network/TeamLanEndpoint.json`이 endpoint와 만료일 정본이다. 각 에이전트는 pull 후 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행하고 출력된 역할에 맞는 target을 안내하며, 실제 `Ctrl+F5` 시작과 UI 조작은 사용자가 수행한다.

쿠크 아레나의 광기와 네 HUD는 `CCombatHUDViewModel::Get_KoukuGimmick()`을 읽는다. v63
`PLAYER_SNAPSHOT`의 madness, `eKoukuHudMode`, cooldown 종료 tick과 카드 문양·색이 실제 상태를 소유한다.
광기 100% 및 F1 `Clown > Change to Clown`은 `POLYMORPH`(표시 이름 Clown)를 사용하고,
Mario/Dance/Card Maze는 각각 `MARIO/DANCE/MAZE`를 사용한다. 모든 interaction 스킬 쿨타임은 3초다.
`CPlayerController -> IPlayerCommandSink -> Server -> snapshot -> CCharacter`가 입력·판정·애니메이션을
연결하며 이 모드에서는 기존 class quick-slot과 평타를 보내지 않는다. QWER 춤 순서는 양팔 모으기,
슈퍼맨, 양팔 벌리기, 한 다리 올리기다. 첫 오답은 fail, 유효 입력 없이 창 종료는 timeout이다.
F1의 mode 선택은 typed Debug 명령으로 실제 모델·HUD·스킬을 바꾸고 Return to Player는 원래
class로 복귀한다. 별도 `Kouku UI Preview`는 표시 전용 override이며 해제하면 실제 snapshot으로 돌아간다.
F1 `Kouku UI Preview -> Madness gauge position`에서 위치를 조절한다. `screenOffsetX/Y`는 1280×720 기준 픽셀(+Y 아래), `headOffsetMeters`는 월드 높이다. Save는 `Data/UI/KoukuSaydon/KoukuHudModes.json`의 변경한 위치 필드만 최신 저장본에 병합하고 충돌 시 기존 저장본과 preview를 보존한다. 카드 미로에서는 본인과 동료의 광기 게이지를 모두 숨긴다.
Mario1의 `Mario1_go`/`Mario1_Trigger_5`는 이동 도착 시에만 mode를 전환한다. movePlayer event의
optional `koukuHudMode`는 `MARIO/MAZE/NONE`이고 Server가 이동 성공 후 적용한다. Card Maze Debug gate는
`(0.09,-0.01,1351.48)`에 플레이어만 이동시키고 보스를 생성하지 않는다. Mario2~4 진입점은 미등록이다.
Clown 본체는 `Character/KoukuSaton/MN_RPCZ_00-1/MN_RPCZ_00-1.wmodel`이며 admission scale은
`0.017 × 0.709`다. 별도 프라이팬 `IT_GSTFP_00` 장착 part를 제외했고 본체와 망치 몸동작은 유지한다.
`Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json`이 mode/index별 clip을 소유한다.
Interaction Effect는 동일 Server action의 준비가 끝날 때까지 현재 action age로 재시도하되 실제 clip/playRate, Server action lock, Effect 수명 안에서 한 번만 제출한다. 선택적인 `EFFECT_SPAWN_DESC::PendingAdmission`은 producer가 소유하는 typed lifetime token의 weak 참조다. 기본 nullopt는 기존 호출 동작을 유지하며 Effect service는 queue admission과 commit에서 만료된 요청만 폐기한다. 새 action·취소·사망·class/form 교체는 기존 Character의 token을 해제한다. 이미 active인 Effect의 자연 꼬리는 이 token으로 중단하지 않는다. 저장 JSON과 network wire는 바뀌지 않는다.
G1 전투 진입 snapshot 전에 Server가 참가자 1~4명의 문양을 중복 없이 배정하고 색 RED/BLACK은 독립으로 선택한다. 유효한 기존 배정은 유지하며 G2 진입 연출 시작과 퇴장에 NONE으로 정리한다. Client는 8개 `boss.kouku.card.*` V2 group을 머리 위에 표시하고 카드 상태를 생성하지 않는다.

카드미로의 여섯 class는 `Data/Animation/Authored/<Class>/<Class>.interactionbindings.json`의 원본 손 본과 clip을 사용한다. LMB는 오른쪽에서 왼쪽으로 휘두르는 clip, Q는 점프 내려찍기이며 기존 typed interaction command와 Server hit 시각을 따른다. 기본 무기를 숨기고 별도 카드미로 망치를 표시하며 퇴장 시 기본 무기로 복귀한다. 일반 광기 광대는 망치를 숨기고 Mario 광대만 표시한다.
쿠크의 사망 화면은 기존 DeadScene UI와 typed revive 명령을 사용한다. Server는 사망 XZ의
walkable 지면으로 부활시키며, 더 이상 유효하지 않은 지점은 기존 navigation projection으로 보정한다.
캐릭터 정보창과 아바타 도감은 아레나의 현재 복제 캐릭터를 읽고, 같은 class의 광대 교체도 목록 갱신 경계로 본다.

### 1.1 서로 다른 장소에서 Server와 Client 연결

빠른 endpoint 교체와 실제 4인 LAN, loopback 격리 테스트의 실행 체크리스트는
[네트워크연결가이드.md](네트워크연결가이드.md)를 따른다.

Server와 Client가 같은 PC, 같은 LAN, 서로 다른 네트워크 중 어디에 있는지 먼저 구분한다.

| 실행 위치 | Server `--bind-address` | Client `LOSTARK_SERVER_HOST` |
|---|---|---|
| 현재 팀 LAN 공유 Server | `0.0.0.0` | `192.168.0.22` |
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
<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.0.22</LocalDebuggerEnvironment>
```

`0.0.0.0`은 Server의 수신 주소일 뿐 Client 접속 주소로 사용하지 않는다. 현재 Client 기본값은 `192.168.0.22`이다. 주소를 바꾸면 `Tools/Network/TeamLanEndpoint.json`, Server/Client 코드 기본값, 공유 debugger 설정과 이 사용서를 같은 변경 단위에서 갱신하고 `Sync-TeamLanEndpoint.ps1`, NetworkProtocolHarness, Server contract test로 검증한다.

#### pull 후 공유 Server에 들어가는 순서

Server PC와 Client PC는 먼저 같은 commit과 생성 데이터를 맞춘다. 기능 브랜치를 검증할 때도 양쪽이 같은 변경을 사용해야 한다. `pull`만 하고 예전 실행 파일을 쓰면 현재 protocol v110 또는 Debug gameplay revision이 달라 Server가 연결을 종료할 수 있다. Server/Client/Shared는 항상 같은 protocol version으로 다시 빌드한다.

```powershell
git switch main
git pull --ff-only origin main
git lfs pull
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git rev-parse HEAD
```

두 PC의 `git rev-parse HEAD`가 같아야 한다. `Client/Bin/Resources`의 `Fonts, Character, Deploy, Effect, Map, Sound, UI` 일곱 물리 폴더도 팀장이 전달한 같은 runtime 입력이어야 하며, Git에 없는 UI/Character/Map/Sound 리소스는 별도 전달을 먼저 완료한다. Debug configuration으로 공유할 때는 두 PC 모두 Debug 정본 빌드를 실행하고, Server를 중지한 상태에서 Server PC가 `Server/Bin/Debug/Server.exe --reset-valtan-runtime-to-packaged`를 한 번 통과시킨다. cross-PC Debug Hot Reload candidate 공유는 지원하지 않는다.

동기화 뒤 Visual Studio project를 Reload하거나 IDE를 재시작한다. Server PC에서 `Machine role: server-host`를 확인하고 `Server + Client` profile을 시작한다. 다른 PC는 `Machine role: client`를 확인하고 `Client Only (Server Already Running)` profile을 시작한다. 직접 EXE를 실행할 때 shell의 오래된 `LOSTARK_SERVER_HOST`가 새 기본값보다 우선하므로 값이 `127.0.0.1`이면 제거하거나 `192.168.0.22`로 맞춘다.

Server가 `Listening on 0.0.0.0:7777`을 출력한 뒤 다른 PC에서 아래 probe가 성공해야 한다. Windows 네트워크가 `공용`이어도 repository sync가 검증하는 firewall rule은 `Profile Any`, `RemoteAddress LocalSubnet`이므로 직접 IPv4 접속에는 문제가 없다.

```powershell
Test-NetConnection 192.168.0.22 -Port 7777
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
Test-NetConnection 192.168.0.22 -Port 7777
```

`Failed to open TCP listener ... Error=10049`는 `--bind-address`에 적은 주소가 현재 Server PC의 어느 어댑터에도 없다는 뜻이다. Client의 주소나 이전 Wi-Fi 주소를 Server bind 값으로 복사하지 말고 Server는 `0.0.0.0`, Client만 도달 가능한 endpoint를 사용한다.

### 1.2 같은 방 파티와 Bern → Valtan/KoukuSaydon 이동

Bern/Valtan에서 다른 플레이어를 우클릭해 초대하고 상대가 수락하면 최대 4인의
Server-owned 파티가 된다. `IPlayerCommandSink`가 typed invite/respond를 제출하고
`S2C_PARTY_ROSTER`의 배열 첫 member가 leader다. nickname은 표시용이며 초대·roster는
현재 방의 NetEntityId로 식별한다. 옛 초대 응답은 교체된 새 초대를 소비하지 않는다.

Bern의 군단장 레이드 UI는 선택한 Valtan/KoukuSaydon target으로 입장 투표를 요청한다.
파티가 있으면 leader가 요청하고 전원의 수락 뒤 이동하며, 솔로도 같은 target을 소비한다.
Client는 서버가 보낸 vote target에 맞는 레이드 이름을 수락창에 표시한다. Server는 모든
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

Client의 gameplay 키보드·마우스와 raw 입력은 실제 Client 창이 foreground일 때만 유효하다.
포커스 상실·장치 읽기 실패는 입력을 비우며, 복귀 시 이미 누른 키·버튼은 놓고 다시 누른
입력부터 받는다. Controller의 조준·hold 예약도 기존 취소 경로로 정리한다.
사운드는 foreground 창이 같은 프로세스일 때만 FMOD master 출력을 열어 여러 Client EXE의
소리가 겹치지 않게 한다. 같은 프로세스의 분리 도구 창은 소리를 유지하며, mute는 채널별
볼륨·명시 pause·시퀀스 재생 시각을 바꾸지 않는다.

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
| H | `vehicleId` | 탈것 탑승/하차 토글. `C2S_SET_VEHICLE_RIDING`을 제출하고 결과는 `PLAYER_SNAPSHOT.iVehicleId`로 반영 |

`CPlayerController`는 edge input, quick slot, sequence, aim만 만든다. `(class, inputSlot) -> skill ID`는 `CPlayerSkillCatalog`가 `Data/Balance/PlayerSkills.json`에서 해석한다. `IPlayerCommandSink`가 전송 구현을 숨기므로 Controller에서 `CNetworkManager`를 include하지 않는다. Controller는 Transform을 직접 변경하거나 `Play_Skill`을 호출하지 않는다. 일반 이동 명령의 typed sink 송신이 성공하면 `CCharacter::Predict_NetworkMoveGoal(sequence, goal)`로 자기 캐릭터의 표시 예측을 요청한다.

우클릭 이동의 hold 재전송은 유지하고 `effect.world.mouse_click`은 최초 물리 press에서 송신이 성공한 경우에만 생성한다. `Request_MoveToPoint(goal, playClickEffect = true)`의 일반 이동 caller는 raw press edge를 전달하고, Bern NPC 접근의 명시 클릭 caller는 기본값을 사용한다. UI·잡힘·타기팅 종료로 표식을 다시 생성하지 않는다.

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

발탄 F1 `Valtan Arena`의 Start Position / Before Entrance / Arena Start는 기존 typed player teleport를 사용한다. `Despawn Valtan Boss`는 Debug/Release Server에서 ENCOUNTER_VALTAN primary와 owner 종속체만 제거하고 일반 NPC/웨이브 몬스터를 보존한다. 이후 Boss Play Pattern은 disabled placement `boss.valtan.center`를 Server에 준비 요청하고 replicated primary 도착 후 기존 revision/sound/presentation admission을 다시 통과해야 실행된다. spawn 대기는 local boss 생성으로 우회하지 않는다.

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

현재 Shared protocol 110의 Server/Client를 함께 빌드·재시작한다. 새 기능을 이전 실행 파일로 확인하지 않는다.

F1 Sequence Viewer는 모든 Debug Level에서 쿠크/발탄 목록을 읽고, 아레나 실행은
`IPlayerCommandSink -> C2S_DEBUG_WORLD_PLAYBACK -> Room command -> ServerTriggerSystem`
경계를 사용한다. Play/Replay/Stop sequence는 기존 `S2C_WORLD_SEQUENCE_PLAY`의 operation으로
같은 room에 전달한다. Release Server는 요청을 거절하고, 다른 world·없는/비활성 target·사망
player·오래된 request sequence는 실행하지 않는다. 표시 이름은 실행 ID가 아니다.
사용법과 저작/배포 경계는 `AREA_DATA_LAYER_GUIDE.md`의 F1 Sequence Viewer 항목을 따른다.

F1의 카메라 편집 패널을 제거하고 같은 위치에 `Open Balance Test`를 배치했다.
버튼은 기존 공용 `Balance Test` 독립 창을 열며 Debug/Release에서 함께 사용한다.
F6 follow/free 전환과 맵별 카메라 profile 소비는 유지한다.

정본은 `Data/Camera/{CharacterSelect,Bern,Valtan,KoukuSaydon}.camera.json`이며 publisher 없이
직접 읽는다. `CArenaCameraProfile`의 schema/version/areaId·유한 범위 검증을 통과한 profile만
Level이 생성·class 변경·follow 복귀에 소비한다. JSON과 DirectX 카메라는 수직 `fovYDegrees`를
사용하고 수평각은 16:9 기준으로 환산한다. Position offset은 플레이어 기준 월드 XYZ(m),
Rotation은 Pitch/Yaw/Roll(deg), Pitch +는 아래, Yaw 0은 +Z다. 응답0은 즉시 follow다.

optional `characterSizeMultiplier`는 catalog presentation scale에 곱하는 0.25~4배 표현 크기이며
생략하면 1이다. 몸·장비·본 부착이 같은 root를 소비한다. Server Transform과 충돌·공격 반경은
이 값의 소비자가 아니다. optional `classSizeMultipliers`는 GuardianKnight를 포함한 일곱 stable class
이름을 사용한다. 이전 여섯 키 문서는 기존 값을 보존하고 생략된 GuardianKnight를 1로 읽는다.
기본값은 Artist 0.7, DimensionMaster 1.0, 나머지 1이며 현재 catalog scale에 곱한다.
Development/Training/Maharaka는 Character Select profile을 소비한다. `clownSizeMultiplier` 기본0.7과
`marioSizeMultiplier` 기본1은 변신별 추가 배율이다. 로컬·원격 캐릭터가 같은 map profile을 소비한다.
카드미로 플레이어 망치의 `mazeHammerPositionCm`, `mazeHammerRotationDegrees`, `mazeHammerScale`은
손 기준 cm/degree/축별 배율이며 쿠크 휠윈드 Object Transform과 별도 필드다.

source 카메라 기준은 공통50도/16m, 발탄55도/18m다. 쿠크는 optional
`useSourceCameraRegions=true`에서 원본 entrance volume 내부만19m이며 나머지는16m다.
1관문 전장은 이 volume 밖이다. 필드 없는 기존 JSON은 manual pose를 유지하며 지역 이동은
저장 profile을 덮지 않는다. Bern 저장값55도는 사용자 비교값이며 source50도와 구분한다.
원본 package/CDO·구역 연결 복구와 원작 최종 화면 일치 확인은 별도다.

저장본 변경은 재입장 때 확인한다. Valtan/Kouku 연출 종료도 같은 profile로 돌아오며,
마리오·카드미로·컷신의 개별 카메라와 기존 presentation priority는 유지한다.
Character Select의 `Move Player`는 계속 비활성이다. 새 Server command와 Resources 전달물은 없다.

### 4.1.1 Debug 웨이브 몬스터 버튼 (Kouku Book1/Book2, Valtan Stage_1/Stage_2)

Kouku의 `Book1_Monsters`/`Book2_Monsters`와 Valtan의 `Stage_1`/`Stage_2` 트리거 상자는 각각
`spawn.kouku.book1`/`spawn.kouku.book2`, `spawn.valtan.stage01`/`spawn.valtan.stage03` 그룹을 시작한다.
Release Server는 예전처럼 플레이어가 밟으면 그룹을 시작한다. Debug Server는 이 네 상자를 밟아도 G를 눌러도
시작하지 않고 안내도 보내지 않는다. 대신 F1에서 버튼으로 다시 소환한다.

- 쿠크: F1 `KoukuSaydon Arena` → `Bingo Board` 아래 `Normal Monster 1`(=Book1_Monsters, max alive 22),
  `Normal Monster 2`(=Book2_Monsters, max alive 15).
- 발탄: 발탄 아레나 안에서만 보이는 `Valtan Arena` 헤더의 `Normal Monster 1`(=Stage_1, max alive 10),
  `Normal Monster 2`(=Stage_2, max alive 10). Stage_2는 `spawn.valtan.stage03`을 시작하며 G로 움직이는
  `Stage_3` 이동 상자와 다른 기능이다.
- 버튼에 마우스를 올리면 매핑된 trigger 이름, spawn group ID, max alive가 툴팁으로 나온다.

경계는 `CPlayerController -> IPlayerCommandSink -> C2S_DEBUG_RESUMMON_WAVE_MONSTERS(protocol 99) -> CGameRoom`이다.
요청은 group ID를 싣지 않고 (world, button)만 싣는다. Server는 `CServerTriggerSystem::Find_WaveMonsterButton`의 고정 표로만
그룹을 고른다. 요청이 이 방의 world와 다르거나, 그 world에 그 버튼이 없거나, 이 session에 player가 없거나, 발탄
패턴 audition이 진행 중이면 아무것도 바꾸지 않는다. 받아들이면 그 그룹의 살아 있는 monster(죽어서 남은 것 포함)를
제거하고 그룹을 초기화해 첫 wave부터 다시 시작한다. monster는 플레이어 위치와 무관하게 그룹의 저작 anchor에 나타난다.
결과 메시지는 없고 monster는 world snapshot으로 온다. 거절 사유는 Server 콘솔의 `[WaveMonsters]` 줄에 남는다.

Release Server는 이 명령을 무시한다. `Stage_MiniBoss_Spawn`, `Stage_3`, `Stage_Boss`, 다른 월드의 트리거는 Debug에서도
예전처럼 동작한다. 다른 protocol의 Server/Client를 섞어 실행하지 않는다. 현재 wire 정본은 `PacketType.h`의 `NETWORK_PROTOCOL_VERSION` 110이다. 위 protocol 99 표기는 해당 명령이 도입된 버전이며 현재 실행 파일의 호환 버전으로 사용하지 않는다.

### 4.2 마리오 변신·방향키 조작·Debug 점프

Server가 기존 `Mario1_Intro`~`Mario4_Intro` OBB 진입을 검사해 `PLAYER_SNAPSHOT.iMarioStage`를
0(일반)/1~4로 보낸다. 패턴 entry와 Intro 모두 이미 `eMadnessForm=CLOWN`인 플레이어만 입장하며,
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
마리오 안의 뛰어내리기·올라가기·건너가기와 마지막 출구는 밟아서 발동하지 않고 G(Debug 빌드는 ↑도)로 발동한다. G 안내를 화면에 그리는 표시는 다른 팀원이 작업 중이라 이 변경에서는 추가하지 않았고, 쿠크 레벨에 원래 있던 표시(마리오는 `[ Up ]`, 일반 아레나는 `[ G ]`, `requiresInteract` 상자 위 G 키캡)만 있다. G도 밟아서 발동하는 경우와 같은 방 소유 진입 핸들러(`Begin_MarioTriggerMove`)를 거친다.

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
Server와 Client를 protocol 82로 함께 빌드/재시작하고 신규 리소스는 대응 배치 RESULT를 참조한다.

## 5. Character와 Animation

캐릭터의 외형 배율은 `Data/Actors/CharacterCatalog.json` format 4의 optional
`presentationScale`이 소유한다(미지정 1, finite 양수, 최대 100).
`CCharacter::Try_Get_PresentationRootMatrix`의 `Scale * gameplayWorld`를 body/장비/무기와
Animation Target의 현재·과거 pose 및 bone Effect anchor가 소비한다. asset import scale,
Server pose/이동/충돌은 이 값과 분리한다. Character의 `root` Effect cue와 `skill_target`,
명시 world root는 기존 world 크기를 유지한다. class가 없는 쿠크 변신 avatar는 착용자의
class 배율을 상속하지 않는다. catalog는 process 최초 초기화에서 읽으므로 변경 뒤 Client를 재시작하고
character를 다시 생성해야 한다. runtime hot reload는 아니다.

같은 format 4의 per-character `modelMaterialOverrides`는 해당 기본 body/equipment/weapon을,
optional root `modelMaterialOverrides`는 `Character/<Class>/Equipment/`의 선택 장비를 소유한다.
행의 stable key는 `modelAssetId + materialName`이며 기본 모델과 root 모델의 ownership 중복을
거부한다. per-character 최대128행, root 최대4096행이고 root는 source character family만 받는다.
`EquipmentPresentationCatalog`의 visualSet은 교체할 부위를 지정하며 재질 descriptor를 복제하지 않는다.
`CEquipmentPresentationService`는 `CActorCatalog::Build_ModelLoadDescription`으로 descriptor를
받아 기존 `CModel -> CMaterial`과 master palette/socket을 소비한다. source family의 필수 texture,
parameter 또는 실제 mesh UV가 누락되면 착용 transaction을 거절하고 기존 외형을 보존한다.

### 5.1 Source Effect attachment basis

Effect Tool의 개별 Element와 Group 회전은 같은 pivot 계산을 사용한다. 기본은 Anchor origin이며 Group center, Custom point, Element origin을 선택할 수 있다. 선택한 pivot은 편집 세션 상태이고 결과 위치·회전·속도·선형 이동 끝점은 기존 Element 필드로 저장한다. 명시적인 회전 편집은 source fixed-axis sprite에 기존 `followEmitterAxisRotation`을 켜 실제 면에도 emitter basis를 전달한다. camera/velocity billboard의 방향 정책은 유지한다. 서로 다른 ParticleSystem 부모 공간을 쓰는 그룹의 개별 편집에서는 Group center를 사용할 수 없고, source/master가 별도 소유하는 애니메이션 경로는 해당 소유자를 편집한다. 현재 cursor의 preview 재생성으로 world-space 입자 birth history도 갱신하며, 실행 중 제품 Effect를 소급 회전하는 계약은 아니다.

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
쿠크의 Stage 합계는 다음 stage/pattern으로 진행하는 시각이다. Effect/Sound/World/Logic/Summon row는 자신의 시작·종료 시간을 유지한다. `Full lifetime ms`는 전체 row 수명만 바꾸고 마지막 Stage를 늘리지 않는다. 게시 encounter의 optional `timelineDurationMs`와 bootstrap `PATTERNTIMELINE`은 row 수명을, presentation의 `durationMs`/optional `stageDurationMs`는 전체 수명/Stage 합계를 소유한다. 생략한 문서는 기존 Stage 합계를 사용한다. 자연 완료는 원래 시작 tick·occurrence·catalog revision과 남은 핸들을 보존하고, 명시 Stop·수동 새 실행·사망·관문 전환은 잔여 row도 정리한다. 자동 RaidFlow 다음 Entry는 같은 epoch와 immutable catalog revision을 유지하며 이전 row를 넘긴다. Client는 Server 진행을 기다리게 하거나 row 수명으로 다음 Pattern을 결정하지 않는다.
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
target 하나만 같은 revision에서 격리한다. Character Select Loading은 worker 시작과 함께 class target을
priority queue에 놓아 map/model loading과 준비를 겹친다. Debug는 승인된 선택 class를 준비하고,
Release는 선택 화면과 같은 전체 roster의 model/part/shader와 skill Effect를 입장에서 준비한다.
Release class 변경 시 첫 model/Effect 준비 대기를 줄이는 대신 입장 시간과 상주 메모리가 늘어난다.
activation은 이 진입에서 등록한 target의 현재 revision 상태만 확인하고 unrelated background pending을
기다리지 않는다. prepared Product attach는 catalog revision/document identity와 shared immutable
document를 재사용한다.

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
재계산하지 않는다. F1 Balance Test는 이 경계로 최근 16개 event를 표시한다.

보호막이 실제로 소비한 양은 protocol111의 `DAMAGE_HIT_FLAG::ABSORB` 이벤트로 전달한다.
UI는 이를 파란 `흡수`로 표시하고 HP 피해·DPS·stagger에 합산하지 않는다. 부분 흡수는
흡수 이벤트와 남은 실제 HP 피해 이벤트를 각각 전달하며, UI가 shield 수치 차이로 추정하지 않는다.

Gameplay bootstrap의 공통 용량은 `Shared/Public/GameplayDataRevision.h`의 최대131,072행·64MiB다. publisher·Server·Client admission과 Python parser가 같은 유한 상한을 소비하며 format37과 wire protocol111은 유지한다. Server는 파일 읽기 전과 bytes admission에서 크기를 확인하고, 전체 행 수·개별 track4096keys·정규화 geometry·후행 행 거부를 계속 검사한다. 실제 본 회전의 전체 수명 궤적을 임의로 줄여 용량 검사를 통과시키지 않는다. Debug draft 전송의16MiB 상한은 별도 계약이다.

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

Debug/Release F1의 `Open Balance Test`는 공용 수치 편집용 독립 창을 연다. `Players / Skills /
Damage / Bosses / Madness`에서 실제 소비하는 수치를 편집하며 일반 수치 panel은 Valtan pattern source를
로드하지 않는다. `Retail.balanceprofile.json`이 덮는 field는 그 실효값을 읽고 같은 Retail row에
저장하며, 덮지 않는 movement/timing 등은 base JSON에 저장한다. 별도 Valtan authoring backend는 유지한다.

`Save + Validate`는 stable ID와 field 이전값으로 최신 저장본에 병합하고, 변경 field를
`PROJECT_TUNED`로 동기화한 후보를 검증한 뒤 원자 교체한다. 실패하면 기존 저장본을 보존한다.
`Publish Server Data`는 Gameplay/World/Items를 같은 Retail profile의 runtime set으로 게시한다.
파일 저장·게시와 실행 반영은 별도이며 Server와 Client를 재시작한 뒤 Server snapshot/damage event로
확인한다. 일반 수치 panel은 runtime Hot Reload를 수행하지 않는다. 세부 작업법은
`BALANCE_TOOL_OWNER_HANDOFF.md`가 정본이다.

Madness는 Retail `madness[policyId=KOUKUSAYDON]`의 피해·공·인형 배율과 접촉 주기/영역을
소유한다. 피해는 보호막·방어·무적을 처리한 실제 HP 감소 비율에서 계산하고 소수 잔여값을 누적한다.
공/인형은 Server WORLD cue의 생성·파괴 수명에서만 충전하며 이미 CLOWN/마리오인 플레이어는 제외한다.
원본에서 확인한 최대 100·hold 15초·NPC aura +10/초/2m와 인형 공격 4m/30도는 근거로 표시한다.
피해 변환 계수·인형 충전량과 요청에 따른 특수 배율 2배는 PROJECT_TUNED이며 원작 공식으로 표기하지 않는다.

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
| runtime state | `CCombatHUDViewModel`을 제품 HUD가 소비 |
| 최종 image widget 생성 | `CUILayoutRuntime -> CUI_Sprite` |
| 제품 UI picking | `CUIInputRouter`의 reference 좌표 hit test와 typed consumer |

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

- reference resolution은 각 layout JSON이 소유한다(기본1280×720). Sprite는 reference rect를
  보존하고 현재 physical viewport의 X/Y 비율로 geometry/ortho를 갱신한다. 클릭은 그 역변환을
  사용한다. 현재 자동 letterbox/anchor 재배치는 없고16:9끼리의 변경은 모양 비율을 유지한다.
- monitor DPI를 layout rect에 곱해 저장하지 않는다. Client의 PerMonitorV2가 OS bitmap 확대를
  방지하며 글자는 실제 픽셀 크기에 맞는 atlas를 사용한다. ESC에서 고른 physical 해상도는
  `CUserSettings::DISPLAY_APPLY -> CClientWindowDisplay -> CGameInstance::Resize_Viewport`로
  연결한다. UI view에서 swapchain/Win32 style을 직접 바꾸지 않는다.
- ESC 적용/확인의 개인 정본은 `%LOCALAPPDATA%/LostArk/UserSettings.json`이다. `display`는
  width/height/mode, `values`는 기존 stable row별 값이며 source Data/UI를 덮지 않는다.
  저장 전 stage·freshness 재확인·backup·atomic replace와 실패 display 복구를 유지한다.
  전체 창 모드는 monitor native size, 일반 창/전체 화면은 선택 크기를 사용한다.
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
Kouku Composition과 독립 Sequencer의 Save는 로드 기준본·draft·현재 저장본을 비교한다.
서로 다른 stable ID/필드의 변경은 함께 저장하고, 동일 필드의 다른 변경·삭제와 수정·상충하는
행 순서는 경로를 알려 거절하며 draft와 디스크를 보존한다. 좌표 같은 비-ID 배열은 통째로
한 필드다. 외부 writer는 revision 증가와 writer lock을 유지하며, 저장 성공 후 편집기는 병합된
LastGood을 소비한다. 구버전 실행 파일의 충돌을 Reload나 파일 전체 덮어쓰기로 우회하지 않는다.
외부 데이터 반영의 최종 저장본 승인·기승인 유지·편집 보존 정책은 [AGENTS.md](../../AGENTS.md)를
따른다. Client 종료는 데이터 파일 반영의 조건이 아니며, 최신 저장본의 필드 병합과 hash/freshness
검사를 유지한다. 파일 교체가 열린 draft나 캐시의 자동 reload를 뜻하지는 않는다. 실제 점유된
EXE/DLL을 링크·교체하기 위한 종료는 [CLAUDE.md](../../CLAUDE.md)의 빌드 절차로 구분한다.
Encounter/patternbindings와 Gameplay 게시 중 실패하면 domain owner가 이전 생성물·receipt를 복구한다.
배포 성공 후 F1 목록을 갱신하며 Complete Play는 최신 Product를 재조회한다. Workbench의 미저장
변경·배포 진행·source/Product revision 불일치를 거절하고, Server 활성 revision 검사는 유지한다.
새 runtime 데이터의 Server 적용에는 재시작이 필요하다. Product source revision 거절 메시지는
요청 번호와 Server 활성 번호를 함께 표시한다. publish 성공 뒤에는 Server를 재시작하고 Client를 재접속한다.
Summon 박스의 optional `patternSpawns`는 stable spawnId·patternId와 positionOffset·yawOffsetDegrees를
소유한다. optional `anchorKind` 생략 또는 `BOSS`는 본체 상대 좌표(±1000m), `MAP`은 맵 절대 위치
(±100000m)와 절대 yaw다. 같은 Summon 이름의 다른 박스에는 전파하지 않는다. 같은 Gate·actor·boss
target의 leaf MECHANIC Pattern을 최대 16개 지원하며 Animation, BOSS 기준 Effect/Sound 및 배우 자신의
ALBION_AIRBORNE JUMP/SLAM을 재생한다. 자식의 재귀 Summon·Parent·World·Scene·본체 reset과
플레이어 선택/이동·outcome 분기는 거부한다. 각 분신은 Server에서 독립 NetEntity로 생성하고 기존 dependent
spawn/snapshot과 CNpc/CModel 경로로 표시한다. 부모 run 종료·교체·사망 또는 박스 수명 종료 시 정리한다.
MAP spawn은 exact walkable과 authored Y의 navigation 지면 오차 1m 이내를 요구하며, 한 행의 실패도
전체 생성 전에 거부한다. Play Preview도 각 분신의 독립 animation/root/Effect 시계를 사용한다.
Box Detail의 `Create two rows: 5 left + 5 right`는 빈 목록에 중심·좌우 거리·행 간격·layout yaw로
10개 MAP 배치를 만든다. 생성 뒤 각 위치와 방향을 조정할 수 있고 child 전체 길이에 맞춰 Summon과
부모 수명을 늘린다. Publisher는 `SUMMON_PATTERNS` trigger와 `PATTERNSUMMONSPAWN` 행으로 연결한다.
네 방향 중 본체를 고르는 Parent는 Summon occurrence 하나를 사용한다. 재사용 Summon definition에 `summonKind=CROSS_DIRECTION_CLONES`, 전방/후방/왼쪽/오른쪽 순서의 `directionPatternIds` 네 개와 `cloneEndStageId`를 설정한다. occurrence의 start/duration이 유일한 실행 시계이고 별도 Logic이나 Pattern row는 필요 없다. 기분나빠·십자 화염폭발·3갈래 불뿜기는 서로 다른 패턴 세트를 같은 정책으로 재사용한다. 이름만 있는 기존 Summon은 여전히 동작을 추측하지 않는다. typed Summon과 독립 patternSpawns 또는 같은 Summon을 참조하는 CROSS Logic을 함께 사용하면 거부한다. 이전 저작 데이터의 DURATION `CROSS_DIRECTION_CLONES` 연결도 호환한다. `directionPatternIds`는 전방/후방/왼쪽/오른쪽 순서의 네 animation+Effect leaf Pattern, `cloneEndStageId`는 분신이 끝낼 Stage, `summonOccurrenceId`는 같은 Pattern의 이름 있는 Summon 박스를 참조한다. Summon과 Logic 시작은 같고 Summon 수명은 Logic 전체를 포함하며 독립 patternSpawns를 겹치지 않는다. 네 child의 explicit duration은 기존 fixedTimeline으로 게시되고 Logic 창 안에 끝나야 한다. 서버는 현재 본체 위치·yaw와 cutoff까지의 실제 root motion으로 네 목적지를 계산하고 원래 arena boss spawn XZ에 가장 가까운 방향을 한 번 선택한다. 동률은 배열 순서다.

Parent의 identity/clock/뒤쪽 Logic은 유지한다. 선택한 하나만 같은 실제 boss entity의 child animation/root motion/Effect를 재생하고 나머지 세 개는 기존 dependent Summon으로 생성하여 cutoff에서 종료한다. Parent의 Animation/Pattern/이동 Logic과 창이 겹치면 거절한다. Shared snapshot의 optional presentation pattern/action 및 시작 tick/stage는 이 child를 나타내며 parent gameplay snapshot을 대체하지 않는다. Client와 Server는 동일 protocol로 함께 빌드·재시작한다. 일반 Play는 기존 Preview 배우로 같은 네 방향 구성을 보여 주고, Complete Play는 Server의 확정 결과를 사용한다. 일반 Summon의 explicit patternSpawns도 Play에서 독립 배우를 재생한다.

일반 Play는 연결된 `BOSS_TELEPORT_XZ`의 목적지와 시각을 Preview 배우에 적용하고 animation 높이·yaw를 유지한다. `BOSS_TRACK_TARGET`과 `SHOWTIME_PLAYER_TARGETS`의 본체 회전은 살아 있는 복제 플레이어 위치를 읽어 재생하며 기록된 입력으로 되감기를 재현한다. 이름만 등록되고 실행 kind가 없는 Logic은 이름으로 동작을 추측하지 않는다. 회전 외 Showtime 반복 발사·추적 투사체·랜덤 투사는 기존 Server combatobject 계약을 따른다.

일반 Play의 `ALBION_BLUE_CIRCLE`은 countPerPlayer=1, radiusM=0이며 randomPlayerOnly와 arenaRandomCount가 꺼진 플레이어 중심 배치를 지원한다. Trigger 시점마다 살아 있는 플레이어별 발생 위치를 고정하고 기존 BossCatalog combatvisual의 Effect를 수명 동안 재생한다. 원 예고가 생긴 뒤 플레이어를 따라 위치를 옮기지 않으며 Stop/새 Play에서 이전 발생 위치를 정리한다. 전투 판정이나 Server entity를 Preview에서 생성하지 않는다.

패턴 상세 Sequencer는 Stage/Animation/Logic/Summon/World/Scene Profile 및 모든 presentation lane을
드래그 또는 Ctrl+click으로 함께 선택한다. 드래그는 박스의 시간폭 전체를 감싸며 Ctrl+drag는 선택을 추가한다.
Duplicate 또는 Ctrl+D는 선택한 구간과 Collider↔Logic·World↔동반 Effect 연결을 새 stable ID로 복제한다.
Stage가 포함되면 뒤 구간을 밀고 삽입점을 가로지르는 기존 master/World의 수명을 연장한다.
외부 카드와 검색 master 참조는 유지한다. 긴 master/World까지 직접 선택하면 그 전체 구간이 복제되므로
한 타격만 복제할 때는 선택 표시를 확인한다. lane만 복제하면 선택 끝에 배치하고 필요한 끝 시간을 늘린다.
Delete도 선택 전체를 한 번에 처리한다. Earlier/Later는 Stage/Animation만 선택했을 때 사용한다.
쿠크 Composition과 독립 Sequence에서 Ctrl+C는 모든 lane의 선택과 그 소유 연결(hold·summon·group·
region·companion·WORLD owner)을 세션 안에 복사하고, 다른 Pattern을 선택한 뒤 Ctrl+V는 대상 Pattern 끝에
추가하며 빈 대상에는 0ms부터 배치한다. 혼합 선택의 상대 시각·clip 구간·재생 속도·Effect 수명·배치·
그룹을 유지하고 새 stable ID를 발급하며 Ctrl+D와 같은 복제 엔진을 사용한다. 함께 선택한 두 clip의
Animation Blend와 필요한 World owner도 연결한다. 복사 뒤 삭제된 Effect resource·World·Logic·Summon·
Scene Profile 정의는 복원하고, 변경된 정의는 `changed; copy again`으로 전체 붙여넣기를 거절한다.
Pattern row는 같은 Gate·actor·대상 boss의 Parent에만 붙여넣는다. 다른 actor의 animation과 누락된
의존 정의는 이유를 표시하고 전체 붙여넣기를 거절한다. 텍스트 입력·드래그 중에는
단축키를 소비하지 않는다. 기존 Save로 저장하며 운영체제 clipboard나 다른 편집 세션으로 전송하지 않는다.
복제·삭제 후 Save → Publish All Patterns → Server 재시작을 거쳐 Complete Play로 확인한다.
Pattern/Parent/Bundle/Logic/Resource의 Rename은 현재 표시 이름을 열어 Apply하고 기존 Save로 저장한다.
stable ID와 참조는 바뀌지 않는다. Stage 삽입·삭제는 겹치는 BossMotion 시간도 함께 늘리거나 압축하며
이동 위치와 yaw를 보존한다. 이동 구간 전체를 제거할 때는 Move boss를 먼저 해제한다.
Stage Duration/ Fit Stage to Animation은 Effect와 모든 비애니메이션 lane의 시작·길이·fade·배치·그룹을 보존한다.
선택 animation의 playback window만 Stage 안으로 제한하고, 늦게 끝나는 행은 explicit Pattern lifetime을 늘려 끝까지 재생한다.
leaf의 추가 tail은 마지막 source pose를 유지하며 fixed Animation Blend 충돌은 시간 변경 없이 전체 편집을 거절한다.
Animation의 optional blendInMs는 직전 clip 끝점에서 현재 clip으로 보간한다. Product의 짧은 playMs는
남은 Stage 동안 종료 자세를 유지하고 LOOP_TO_WINDOW는 지정한 시간창에서 반복한다.
Preview, Product의 body/weapon 및 서버 bone-contact bake가 같은 샘플링 계약을 사용한다.
F1은 관문별 `Saved Pattern Flow`와 `All Patterns`를 구분한다. Boss Tool의 `Pattern Flow`에서
Pattern 또는 Bundle stable ID를 추가하고 순서·대기 시간을 편집하여 `Save Pattern Flow` →
`Publish Saved Patterns`로 게시한다. 정본은 기존 KoukuSaydon Composition의 optional `patternFlows`다.
Action Workbench는 모델 선택과 무관하게 전체 Gate→Parent→Bundle→Pattern 트리를 표시한다.
단독 Pattern/Bundle Complete Play는 대상 관문이 준비되지 않았으면 기존 Gate 활성화를 먼저 요청한다.
보스 spawn·플레이어 이동 승인과 같은 world session·저장 source revision을 확인한 뒤 재생을 제출한다.
이미 준비된 관문은 위치를 초기화하지 않는다. 준비 중 Stop·실패·관문/연결 변경은 지연 재생을 취소한다.
`Complete Play - Sequences + Pattern Flow`는 typed Raid START로 선택 관문부터의 전체 진행을 Server에 요청한다.
Server가 저장 Action·Sequence revision과 Flow를 검증하고 실행 epoch·시작 참가자·공통 tick을 고정한다.
입장 Sequence 종료 전에는 보스를 생성하지 않는다. 1·2관문은 종료 뒤 기존 audition으로 저장 Flow를 실행하며, 3관문 최초 입장은 아래 `WAIT_ENTRY` 승인을 기다린다. BINGO는 별도 입장 Sequence 없이 같은 준비 승인 후 저장 Flow를 시작한다.
Stop은 실행 owner와 epoch가 일치할 때만 처리한다. `Play Saved Pattern Flow`는 전투 순서만 실행하고 `Composition Play All`은 선택 관문의
게시 목록에서 Bundle을 하나의 동시 실행 항목으로 유지하며 해당 child의 중복 단독 재생을 제외한다.
Flow는 정확한 Server COMPLETED를 받은 뒤 대기 시간을 거쳐 다음 typed Pattern/Bundle 요청을 보낸다.
`patternFlows.entryGroups`는 기존 entry의 stable `startEntryId`/`endEntryId`로 순서가 겹치지 않는 구간을 묶는다.
`groupId`와 `displayName`은 묶음의 저장 식별자와 표시 이름이고, 기존 동시 실행 Bundle과 별개다.
optional `repeatUntilHealthBars`와 `transitionAt`(`PATTERN_END`/`GROUP_END`)는 Server가 실제 보스 HP로 판정하는 반복 조건이다.
Server는 현재 패턴과 카운터 성공 후속의 완료를 기다린 뒤, 임계 미도달이면 구간 끝에서 처음으로 돌아가고
도달하면 지정 완료 경계에서 구간 다음으로 이동한다. 이미 넘긴 임계의 아직 시작하지 않은 일반 구간은
건너뛰고 다음 기믹에서 멈추므로, 여러 임계를 한 번에 넘겨도 기믹은 순서대로 한 번씩 실행한다. 0줄 구간은 보스 사망 전까지 반복한다.
HP 그룹은 `loopStartEntryId`와 혼용하지 않으며, 누락 참조·겹침·역전·범위를 벗어난 임계는 저장/게시/입장에서 거절한다.
Boss Tool의 Flow Groups에서 구간·HP 조건을 편집하고 F1 Saved Pattern Flow에서 묶음별로 표시한다.
HP 반복이 있는 `Play Saved Pattern Flow`는 typed Server Complete Play로 전달하며 Client가 HP 또는 다음 기믹을 결정하지 않는다.
게시된 Pattern/Bundle과 Complete Play의 typed 명령은 Debug/Release에서 같은 scope·revision·owner 검증을 사용한다.
memory draft와 Mario test override는 계속 Debug 전용이다. 실제 HP 구간별 패턴 순서는 Composition authoring이 소유한다.

`patternFlows.loopStartEntryId`는 같은 Flow의 stable entry ID를 가리키는 optional 반복 기점이다.
지정하면 최초 시작부터 마지막 항목까지 한 번 실행한 뒤, 마지막 wait를 지키고 해당 entry부터 끝까지 반복한다.
순서 변경에도 ID로 기점을 유지하며, 비어 있지 않은 잘못된 ID·누락된 참조는 저장/게시에서 거부한다. Tool에서 기점
entry를 삭제하면 반복 설정도 해제한다. `Repeat From Selected`와 `Clear Repeat`로 편집한다.
미지정 또는 빈 문자열이면 기존 동작을 유지하고 저장 시 이 필드를 생략한다. 전투 전용 Saved Pattern Flow는 GATE1만 처음으로 순환하고, Sequence 포함
Server RaidFlow는 GATE1과 BINGO가 처음으로 순환한다. GATE2·GATE3는 마지막 항목에서 종료한다.
지정한 기점은 두 소비자에 동일하게 적용한다. Server RaidFlow는 첫 entry로 돌아오는 경계에서도 같은 실행
epoch와 게시 revision을 유지하며 잔여 row를 넘긴다. Stop·중단·관문 완료·연결 또는 revision 변경은 기존 종료 경로를 따른다.
거부·취소·연결 종료는 남은 순서를 취소한다. Server는 묶음의 모든 대상을
검증한 뒤 하나의 run epoch와 공통 시작 tick을 확정한다. offset은 30Hz tick으로 올림하며 Stop/Restart는
원래 run epoch를 명시한다. 같은 방의 Client와 늦게 입장한 Client는 복제된 묶음 상태와 시작 tick을 소비한다.
공통 Camera/Scene Profile은 묶음 시계에서 한 번 실행하며 다른 소유자의 겹치는 전역 연출은 게시 단계에서
거부한다. Preview는 연출 확인이고 조건부 gameplay 결과는 Server Complete Play에서 확인한다.
Sequencer의 Reset 오른쪽 `Play Pattern`은 현재 Pattern/Parent/Bundle을 기존 Server audition으로
요청한다. 현재 Apply된 메모리 draft를 고정해 검증하므로 Save와 Publish가 선행 조건은 아니다.
검증 실패·미지원 대상은 기존 실행을 보존하고 이유를 표시한다. 자동 저장·게시나 local collider 판정은 하지 않는다.
명시적 Play Pattern은 일반 패턴도 준비 진행·실패와 Server 승인·거절 상태를 같은 Workbench에 표시한다.
버튼의 준비 안내는 서버 실행 완료를 의미하지 않으며, 실제 재생은 리소스 준비와 Server admission 뒤 시작한다.
버튼 tooltip의 대상 이름과 stable ID가 실제 실행 단위다. Resources에서 고른 Pattern은 Append할
원본이며 현재 타임라인을 바꾸지 않는다. 기존 local 재생은 `Play Preview`로 구분하고 별도
Sequence workspace는 원래 전투 입장 흐름을 유지한다. 후속 패턴을 가진 Logic도 Server에서
그대로 실행되므로 1페이즈 이름의 Pattern이 완료 이후 2페이즈까지 이어질 수 있다.
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
Server/Shared/Client는 같은 protocol 84로 함께 빌드·재시작한다. FEAR snapshot 상태와
빙고·마리오·갈고리 attachment wire, 마리오 원본 공의 `iMarioPoppedBallMask`(u16)·
`iMarioCurseReleasedMask`(u8)와 카드미로 ENTRY_HIDDEN을 함께 포함한다. 두 기능이 별도 branch에서
각각 79를 사용했으므로 두 종류의 v79 및 이전73/77/78 실행 파일과 혼용하지 않는다.
마리오 진행 횟수는 Server 방 상태가 소유한다. 시작·초기화는1이며, `ENTER_AREA`의 단일
`MARIO_ENTER` 결과가 실제 입장 commit에 성공했을 때만1..4단계를 소비한다. UI 이름이나
Client collider가 횟수를 증가시키지 않는다. `MARIO_ENTER`는 Gate 3 Pattern의 ENTER_AREA box
(Collider region 필요)의 sole Success이며 completion chain은 선택이다. optional `marioStage` 0..4는
0=live counter, 1..4=저작 단계(요청 test stage 우선)이고 0이 아닐 때만 문서·projection·
`PATTERNLOGICOUTCOME` 11번째 field로 실려 기존 행과 bootstrap은 byte 동일하다. chain 없는 입장은
`startMs+durationMs`와 패턴 완료에서 portal을 닫고 Client hold를 게시하지 않는다. protocol 85는
그대로지만 bootstrap 행과 문서 key가 바뀌므로 Server/Client를 함께 빌드·재시작한다.
`PATTERN_COMPLETION_COUNT` duration은 같은
관문의1..16개 패턴 pool과 완료 개수를 저장하고, Server가 중복 없이 선택한 실제 패턴의
`PATTERN_COMPLETED`만 센다. Success는 비워 두거나 FOLLOWUP_PATTERN 하나를 연결한다.
비어 있으면 마지막 완료로 해당 chain을 종료하고, 연결했으면 기존 후속 패턴을 실행한다.
Timeout·취소·실패를 성공으로 바꾸지 않는다. 명시적인 lifetime을 가진 Parent Summon은 기존
확장 Stage를 사용해 후보로 검증하며, 원본의 빈 Stage 배열만으로 배제하지 않는다.
Sequencer Play와 Play Preview는 completion-count Logic 또는 Mario 입장 패턴도 로컬 표현으로 확인한다.
실제 chain·입장·Collider 판정은 Play Pattern으로 현재 draft를 검증한 뒤 기존 typed Server 경로에서 확인한다.
해당 요청의 Server 패턴 ID·현재 시작 tick으로 선택과 커서를 갱신하고, 편집 입력이 시작되면
이번 실행의 자동 선택을 멈춰 미적용 입력을 보존한다. Stop Pattern은 같은 Server service로 제출한다.
로컬 Play/Pause/Resume/Stop/Reset과 ruler scrub는 별도 preview clock만 조작하며 서버 준비·재생 중에는
로컬 Play/Resume/scrub를 거절한다. 서버 패턴을 Stop Pattern으로 종료한 뒤 로컬 미리보기를 재생한다.
마리오 시작 root의 entry collider·anchor·시계는 child 패턴이 바뀌는 동안 Server가 유지하고,
기존 Bundle member state를 통해 Client의 retained entry presentation에 전달한다. 늦은 입장도
같은 root 시계를 소비하며, 입장 소비·chain 종료·취소에는 해당 owner의 상태를 정리한다.
Success 후속 패턴이 연결된 솔로 마리오 회차는 Server가 참가자 identity와 실제 복귀 완료를
보관한다. 지정한 랜덤 패턴을 모두 마쳐도 진입자가 남아 있으면 2페이즈 Success를 보류한다.
미진입 또는 복귀 완료 상태에서는 지정 완료 개수를 채운 직후 이어지며, 사망·퇴장은 성공으로
대신 처리하지 않는다. 현재 P33은 감금 판정이 없다.
숫자열 0키는 마리오 조작 중 typed MARIO_RETURN 요청을 보낸다. Server가 해당 회차의 마지막
movePlayer 트리거와 3관문 stage.kakul.sl05 목적지의 navigation/collision을 검증하고 기존
스크립트 이동을 실행한다. 마지막 출구와 0키 모두 실제 착지 뒤 복귀를 완료한다. Client는
직접 좌표를 보내거나 이동하지 않으며 UI·free camera·텍스트 입력 중에는 요청하지 않는다.
F1 단독 Test의 Mario stage0은 현재 Server 횟수,1..4는 해당 요청 한 번의 재현 시작값이다.
seed를 함께 지정하면 같은 후보 순서를 재현한다. Saved Pattern Flow는 테스트 강제값을 사용하지
않고 실제 Server 진행 횟수를 소비한다. 입력은 기존 typed audition request이며 별도 local 실행을 만들지 않는다.

optional `resetBossToSpawn`은 패턴 시작 때 Server가 실제 보스를 spawn에 복구한다.
함께 지정하는 optional `resetBossYawDegrees`는 유한한 -360~360도의 절대 yaw로, 매 재생 같은 방향을 snapshot에 반영한다.
누락하면 기존 yaw를 유지한다. 이 필드를 배포할 때는 확장된 PATTERNSPAWNRESET을 읽는 Server도 함께 빌드·재시작한다.
optional `bossMotion { startMs, endMs, startPosition, endPosition, yawDegrees }`는 패턴 내부 구간의
절대 월드 XZ 이동을 저장한다. 두 위치의 base Y는 같으며 시작 전에는 시작점, 종료 후에는 도착점을 유지한다.
spawn reset 및 REAL_GAZE_TELEPORT와 동시 사용은 거부한다. Server는 audition stage 전에 경로 navigation을
검증하고 30Hz fixed tick에서 보간한 위치·yaw를 기존 snapshot/늦은 입장의 spawn으로 전달한다.
Client는 기존 root 수평 억제를 유지하고 원본 animation의 수직 pose를 재생한다. 별도 높이 arc는 더하지 않는다.
`bossMotion.keys`를 명시하면 2~512개의 `{timeMs, position:[x,y,z]}`를 순서대로 보간하는
절대 컷씬 경로를 사용한다. 첫·마지막 key는 구간과 양 끝 위치에 일치해야 하고 중복 시각·비유한 좌표는
거부한다. 이 명시 경로는 무대 밖 연출을 포함할 수 있어 일반 이동 navigation에 투영하지 않는다.
원본 root 변위를 경로에 합친 연출은 `animationRootVerticalScale=0`으로 수직 변위를 중복 적용하지 않는다.
key를 생략한 기존 선형 XZ 경로의 높이·navigation 검증은 유지한다. 부모 시간 이동은 key 시각도 함께 옮긴다.

`CARD_MAZE_STAGE_PLAYERS`는 같은 Pattern의 고정 MAP EFFECT occurrence ID 1~4개를
`playerEntryEffectOccurrenceIds`에 순서대로 저장한다. publisher가 현재 효과 위치를
`playerEntryPositions`로 해석하고 Server가 roster와 목적지를 검증한 뒤 일괄 이동한다.
이후 `CARD_MAZE_HIDE_NEXT`는 같은 roster 순서로 소멸 표시를 진행한다. 숨김은 사망이 아니며
실제 카드미로 이동은 기존 `CARD_MAZE_ENTER`가 소유한다. 저작 UI가 플레이어 Transform을 직접 바꾸지 않는다.

BossCatalog의 `animationSetId`가 `bodyModel`과 다르면 기존 CModel에 동일 골격의 animation set을
추가한다. 원래 body clip과 이름이 겹치거나 골격이 다르면 admission에 실패한다. 서버 판정용 bone bake도
해당 clip의 실제 donor 파일을 읽으며 별도 컷씬 배우를 생성해 기존 보스 재사용을 대신하지 않는다.
Stage의 optional `retargetOnEnter`는 strict boolean, 기본false다. Workbench Stage Detail과
Composition parse/validate/save가 같은 값을 소유하고 기존 `RETARGET_RANDOM_ALIVE` ENTER action으로
투영한다. Server는 Stage 진입 때 살아 있는 player의 현재 위치와 yaw를 한 번 확정해 다음 지정
Stage까지 유지한다. fixed-yaw `bossMotion`과의 동시 사용, 잘못된 타입/중복 action은 거부한다.
`retargetTarget`은 retargetOnEnter가 true일 때만 지정하는 optional 값이며 기본
`RANDOM_ALIVE`, 다른 허용값은 `NEAREST_ALIVE`다. Client Stage parse/validate/save가 값을
보존하고 publisher는 기존 ENTER action의 targetId를 `boss.target.pattern` 또는
`boss.target.nearest`로 지정한다. 최근접 선택은 살아 있는 player의 XZ 거리를 사용하며,
일반 Preview도 같은 선택 정책과 기존 Stage별 표본/되감기 경로를 소비한다.
Preview는 Stage별 표본을 저장해 되감기에 재사용하며, 처음 방문한 미래 Stage는 현재 player를
표본으로 삼는다. 명시 Model Reference는 이 retarget을 사용하지 않는다.
Pattern의 optional `animationRootVerticalScale`은 0~1, 기본 1이며 원본 root의 기준 pose 대비 수직
변위만 조절한다. mesh 크기·clip 속도·Server base Y는 바꾸지 않는다. Product action binding과
Preview가 같은 값을 소비하고, bone Collider 투영도 같은 배율을 사용한다. 다음 action/idle은
자기 Pattern 값 또는 기본 1을 적용하므로 이전 패턴의 높이 설정을 이어받지 않는다.
Play Bundle과 실제 보스 actor를 소유한 단일 Pattern Play는 같은 actor/weapon preview 경로를 사용한다. Model Reference는 제자리 비교다.
Local Animation Play도 실제 CModel의 suppression 전 root를 source crop/rate/loop와 Pattern 절대 시각으로
샘플해 preview actor를 이동한다. 수동 bossMotion/charge/teleport가 이동을 소유하면 자동 root 이동을
중복 적용하지 않는다. 종료와 실패에서 이전 model suppression 상태를 복원한다.
TRIGGER Logic의 optional `triggerKind=ANIMATION_BLEND`는 Logic occurrence 구간으로 인접한 두
animation pose를 혼합한다. 기존 occurrence ID·crop·playRate·endPolicy가 정본이며 실행용
`animationBlendWindows`는 파생 값이다. Preview, Product action pose와 source-bone bake는 같은
Pattern 절대 시각을 사용한다. 해당 Product Pattern은 기존 fixedTimeline 경로를 사용하며
새 Server combat trigger로 투영하지 않는다. 로컬 Save는 Publish를 자동 실행하지 않는다.
이동하는 자식 Pattern은 정확히 하나의 전체 수명·비반복 occurrence에 한해 부모로 확장한다.
부모의 독립 animation/이동/reset/retarget와의 동시 소유를 거부하고 child BossMotion의 시각만
부모 offset만큼 옮긴다. 다른 문서에서 가져온 Pattern은 편집 가능한 복사본이며 자동 동기화 참조가 아니다.
BOSS_SPAWN World Object는 Product의 `worldEmissionAnchors`와 bossMotion을 사용해 각 emission 시각의 생성점을
고정한 뒤 개별 objectMotion을 재생한다. 객체가 이동하는 보스를 매 프레임 따라가는 정책은 아니다.

F1 Action Workbench 바로 아래 `Open Sequencer Benchmark`는 동일한 Timeline/Resources/Box Detail ImGui와
기본 배치를 사용하는 독립 Sequence 세션이다. Patterns 목록 창 이름은 `Composition Sequencer`로 표시한다.
저장 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, compositionId는
`boss.composition.kakulsaydon.sequencer`이며 기존 Action 문서와 교차 Save/Reload를 거부한다.
같은 Preview backend의 재생 소유자를 입력 focus와 분리하여 다른 창을 열거나 닫아도 활성 미리보기를
덮지 않는다. 연출 Save는 Action/Server Product를 변경하지 않는다.
Sequence 전용 `TRIGGER / ROOM_PLAYER_ARRIVAL`은 Logic occurrence의 optional
`roomPlayerArrival { playerSlot: 0..3, position: [x,y,z] }`를 사용한다. Box Detail에서는 슬롯을
1..4로 표시하며 시간과 목적지를 각각 편집한다. 일반 Play Sequence에서는 `CPlayerController -> IPlayerCommandSink`
의 Debug World Playback `PLACE_ROOM_PLAYER`가 run epoch, root Pattern ID, occurrence ID와
목적지를 제출한다. Server는 첫 도착 요청에서 같은 방의 접속자를 PlayerId 순으로 고정하고,
해당 슬롯의 이동 가능 상태·navigation·높이·충돌을 검증한 뒤 기존 teleport reset과 snapshot을
사용한다. 빈 슬롯과 중간 퇴장자는 건너뛰며 새 입장자로 그 슬롯을 다시 채우지 않는다.
다른 World와 Release Server는 실행하지 않는다. Pause·scrub은 이동 요청을 만들지 않고,
같은 재생의 occurrence는 한 번만 보낸다. 거절·5초 응답 부재는 전투 진입을 막는다.
Sequence의 `Complete Play`와 `Complete Play - Sequences + Pattern Flow`는 같은 Server Raid START를 사용한다.
Sequence의 combat handoff는 BINGO 앵콜까지 허용하며, 관문 intro 선택은 게시된 Server raid plan이 소유한다. 직접 BINGO Complete Play는 전투 Loop이고 컷씬 단독은 앵콜/최종엔딩 행의 일반 Play다. SOUND의 MAP anchor는 비위치 음향이며 followBoss·bone·World occurrence·emission index를 사용하지 않는다.
ROOM_PLAYER_ARRIVAL은 게시된 Sequence의 슬롯·위치·시각을 Server가 실행하며 Client가 목적지를 다시 제출하지 않는다.
Client는 `S2C_KOUKUSAYDON_RAID_STATE`의 공통 tick으로 기존 presentation player를 샘플링하고,
같은 composition ID·Sequence revision의 저장 문서를 실행 동안 유지한다. 로컬 Pause·scrub·Preview 전환은
Server 시계를 바꾸지 않는다. owner의 Stop은 해당 실행 epoch를 명시한 Raid STOP이다. 명시적 Reset이나 새 로컬 Preview 요청도 활성 Raid가 있으면 기존 typed STOP의 종료 확인을 기다린 뒤 최신 draft의 로컬 재생을 소비한다. 단순 선택·Reload로 내부 Stop을 호출하는 경우에는 Raid를 중단하지 않으며, Save가 실행 중 pin한 revision을 바꾸지 않는다.
F6 free camera에서는 camera track override를 해제하고 현재 pose/FOV를 유지한다. follow로 복귀하면
현재 연출 시각의 camera track을 다시 적용한다. 정상 종료에서는 플레이어 follow를 켠다.
`1관문_통합_시퀀스` 종료 시 Server가 전투 배우와 Flow를 시작하며, Client는 복제된 관문 상태로
맵·조명·HUD를 표시한다. 일반 `Play Sequence`는 기존 연출 Preview이며 자동 전투를 시작하지 않는다.

Kouku `Publish All Patterns`는 Product, 쿠크 범위 World, Gameplay balance를 같은 게시 작업으로 처리한다.
F1 Boss Tuning의 Save가 기록한 `Gameplay.world.json` 위치·방향도 Server worldbootstrap에 포함한다.
기존 World publisher의 `-WorldId KAKULSAYDON_ARENA` 범위를 쓰며 다른 World 출력은 변경하지 않는다.
중간 실패는 이전 세 domain 출력과 receipt를 함께 복구한다. 실행 중 Server의 world 상태는 바뀌지 않으므로
게시 후 Server를 재시작해야 Complete Play가 새 위치를 사용한다.

Composition Effect Resources는 V2 GROUP 목록이 기본이며 V1 탭에서 기존 authored Effect와 stable Element를
선택한다. `resourceKind=V1_EFFECT/V1_ELEMENT`와 optional `elementId`는 원본 V1 문서를 참조한다.
Effect occurrence는 BODY/WEAPON named bone, BOSS/WORLD anchor와 local offset을 사용하고,
named bone은 기본적으로 위치만 따르고 회전 basis는 boss facing을 쓴다. optional
`boneRotation`이 이 basis를 고른다. 기본 `TARGET_YAW`는 기존 동작이고 `BONE`은 본 자체의
자세를 써서 Effect가 그 본과 함께 회전한다. `BONE`은 resource kind `EFFECT`, `anchorKind`
`BOSS`, 비어 있지 않은 `bone`에서만 허용하며 occurrence의 Rotation은 본 frame에서 적용된다.
Collider의 서버용 bone 궤적은 publish에서 구워지므로 Collider에는 허용하지 않는다.
기본값이면 문서에 key를 쓰지 않아 기존 행의 Product bytes가 그대로다.
MAP을 선택하면 고정 월드 위치·회전·크기를 사용한다. MAP은 follow/bone/world 참조를 함께 저장하지
않으며 `Use Player Position`은 현재 플레이어 위치를 occurrence에 복사한다. 같은 규칙은 독립
Sequencer Benchmark에도 적용된다. Effect Tool의 Kouku `Play All`은 현재 플레이어의 위치·방향을
임시 기준으로 사용한다. Append한 occurrence의 앵커와 저장된 Sequence는 이 임시 재생으로 바뀌지 않는다.
이 scene player 등록은 `CClientReplication`이 local spawn/class 교체를 commit한 뒤
`CAnimationTargetService`에 Bind하고 local despawn/reset/destructor에서 자기 캐릭터만 Unbind한다.
원격 player나 실패한 교체는 기존 target을 보존하며, 카메라·입력 연결로 이 등록을 대신하지 않는다.
destructor가 이미 제거된 Layer를 다시 조작하지 않는 종료 계약도 유지한다.
관측한 anchor 기록으로 외부 시계 재생·seek를 처리하며 과거 기록이 없는 구간을 임의 포즈로 보충하지 않는다.

Kouku FEAR Result는 durationMs와 optional sceneProfileId/lightResourceId/effectResourceId/soundResourceId/effectDelayMs를
소유한다. 서버의 FEAR action, 시작 tick, 종료 tick, Result logicId가 상태의 정본이며 공포 중 이동·스킬을
차단한다. Client는 각 class 공포 animation과 로컬 플레이어의 Scene/캐릭터 Light/화면 Effect를 그 시간에
맞춰 재생하고 종료하면 기존 표현을 복원한다. 화면 Effect는 등록된 V1 원본 또는 V2 ScreenPost 경로를 사용한다.
soundResourceId는 SOUND 리소스만 참조하며 얼굴의 effectDelayMs 시점에 같은 FEAR session에서 한 번 재생한다.
같은 snapshot과 반복 얼굴이 보이스를 중첩 시작하지 않고 FEAR 종료·사망·연결 정리 시 기존 SoundCue handle을 해제한다.
GAZE_REAL_BOSS의 Fail에 FEAR를 연결하면 시야 밖 보스에 대한 공포가 되고, ENTER_AREA의 Success에
연결하면 따라가는 Collider 접촉 공포가 된다. OBJECT_CONTACT는 World Object 접촉에 사용한다.
COUNTER_WINDOW는 실제 Server counter hit를 소비하며 Success의 FOLLOWUP_PATTERN과
endsPatternOnSuccess로 그로기 후속 재생을 연결한다.
ENTER_AREA의 optional bossChargeDistanceM은 Trigger 시작 시 살아 있는 target의 방향을 한 번 확정해
해당 occurrence duration 동안 지정 거리를 이동한다. navigation/collision이 막으면 경계에서 멈추고,
이동한 Server pose로 같은 tick의 접촉을 검사한다. 절대 bossMotion과의 중복 소유는 거부한다.

Kouku의 `ALBION_BLUE_CIRCLE` Trigger는 시작 시 살아 있는 플레이어의 위치에 고정 장판을 만든다.
Logic의 `countPerPlayer`는 1..8, `radiusM`은 1개일 때 0, 여러 개일 때 (0,20]m이며,
`effectLifetimeMs`는 1..600000ms다. 등록된 `알비온_플레이어장판`은 1개·0m·7000ms를 사용한다.
여러 개는 플레이어 주변 원주에 등간격으로 놓고 Server가 모든 생성점의 navigation을 검사한 뒤
기존 CombatObjectRuntime transaction으로 함께 생성한다. 하나라도 실패하면 기존 객체를 보존한다.
optional `randomPlayerOnly=true`는 준비된 생존 플레이어 중 한 명만 Server가 선택하며
마리오 참가자·사망·낙하는 제외한다. 이때 `countPerPlayer=1`, `radiusM=0`이어야 한다.
optional `arenaRandomCount`(0..32)는 플레이어 수와 별도의 nav 랜덤 장판 수다. 0 또는 생략이면
기존 생성 의미를 유지한다. 양수이면 `arenaRandomRadiusM`((0,100]m),
`arenaHeightToleranceM`((0,10]m), `arenaMinimumSpacingM`((0,20]m)를 함께 지정한다.
비활성 상태의 세 값은 0이다. 발탄의 공용 nav 후보 선택을 사용해 boss spawn 주변의 같은
지면과 랜덤 지점 사이 간격을 검증한다. 한 명 선택 시 대상이 없거나 전체 위치를 확보하지 못하면
기존 객체·ID를 보존한다. 위치는 생성 순간 고정하며 지속 이동 추적은 하지 않는다.
`combatobject.kouku.albion.bluecircle` / `combatvisual.kouku.albion.bluecircle`을 BossCatalog의
`effect.kouku.albion.bluecircle.warning.impact.runtime`에 연결한다. 이 문서가 예고 2초 뒤 폭발과
잔상을 소유하며 Trigger는 피해를 추가하지 않는다. Logic은 원하는 Pattern 시점에 Append하고
Save → Publish All Patterns를 거친다. 단순 Effect/독립 Sequencer 재생은 플레이어별 Server 생성을 실행하지 않는다.
Kouku Arena Loader는 BossCatalog의 해당 family V1 combat-object visuals를 중복 제거해 기존
Product 준비 큐와 level activation probe에 넣는다. 첫 spawn이 prewarm을 생략하거나 Effect
문서를 동기 로드하지 않는다. Client는 자연 완료의 잔상을 유지하고 Server의 Stop/사망/despawn은
level-owned Effect handle까지 정리한다.

Kouku `SHOWTIME_PLAYER_TARGETS` Duration은 같은 Pattern의 고정 Effect 그룹과 추적 occurrence를
각각 선택한다. 고정 그룹은 하나 이상의 Effect occurrence를 받으며 하나의 compound Effect도
사용할 수 있다. 같은 Pattern의 SHOWTIME Duration이 fixedSelectionGroupId로 소유한 그룹은
MAP·followBoss=false SOUND를 함께 포함할 수 있고 최소 하나의 MAP EFFECT를 요구한다.
각 SOUND는 Server가 확정한 동일 CombatObject 위치·birth clock의 기존 SoundCue 경로에서
재생된다. 일반 선택 그룹·tracking에 SOUND를 허용하거나 Collider와 혼합하지 않는다.
Collider 그룹의 최소 두 행 계약은 유지한다. Publisher의 `PATTERNSHOWTIMETARGETS` 11필드는 encounter/pattern/occurrence,
start/duration, fixed/tracking visual ID, fixed lifetime, interval, follow speed scale이다.
비활성 ID는 `-`이며 최소 하나는 필요하다. fixed lifetime은 비활성일 때 0, 활성일 때
1..600000ms이고 interval은 1..600000ms, speed scale은 0.01..10이다. 요청 기본값인 2000ms와
0.5는 사용자 튜닝값이다. Server는 duration 시작과 interval마다 준비된 생존 player 1..4명의
현재 XZ·server ground Y에 고정 group instance를 만든다. 추적은 window별 player당 하나이며
실효 이동속도에 scale을 곱해 매 fixed tick 접근한다. death/leave/window end는 해당 tracking을
despawn하고 기존 fixed group은 자체 저작 수명까지 남는다. join/revive는 새 tracker를 만들고,
pattern abort/room reset은 전부 정리한다. 저작 run owner 퇴장은 기존 전체 abort 계약을 유지한다.
기존 CombatObject spawn/snapshot/despawn의 object ID, spawn tick과 pinned revision을 사용하며
archetype는 `combatobject.kouku.showtime.fixed` / `.tracking`이다. Client의 group clock과 상대
TRS는 visual mapping이 소유한다. 새 damage나 Client player pose 입력은 없다.

추적 occurrence가 있는 duration 동안 보스 몸 방향은 기존 Server pattern target의 현재 위치를 향한다.
고정 Effect만 생성하는 duration은 기존 target/yaw와 BossMotion 방향을 유지한다. 일반 Play와
되감기의 Client preview도 같은 조건을 사용하며 별도 방향 flag를 만들지 않는다.
대상이 죽거나 퇴장하면 기존 random-alive 선택 정책을 재사용하며 yaw는 snapshot으로만
복제한다. TRIGGER `BOSS_TELEPORT_XZ`는 기존 `teleportPosition`의 XZ만 적용하고 Y는
저작 참고값으로 보존한다. Server가 목적지 navigation과 실제 보스 크기의 overlap을 검증한
뒤 현재 높이·animation clock을 유지하며, 캡처된 stage root XZ/지면 기준도 함께 옮긴다.
절대 `BossMotion`과는 병용할 수 없고 animation root motion의 수직 진행은 유지한다.

TRIGGER `ALBION_AIRBORNE`는 `airbornePhase`, `airborneHeightM`,
`airborneDurationMs`, `teleportPosition`을 함께 저장한다. phase는
`SELECT_PLAYER/JUMP/APPEAR_PLAYER/DISAPPEAR/CENTER/SLAM`이다. JUMP는 양수 높이와
0~600000 ms 상승 시간을 사용하며, 0 ms는 첫 tick부터 해당 높이로 시작한다. 양수 시간의
기존 상승 보간은 유지한다. JUMP는 이후 그 높이를 유지하므로 시작 높이에서 clip의 원본 하강으로
착지하려면 같은 clock에 SLAM box를 함께 두고, `_start` Stage가 반복되면 각 Stage 시작에
SLAM을 하나씩 둔다(쿠크 훌라후프 P84: `쿠크_훌라후프_상단시작` JUMP 0 ms +
`쿠크_훌라후프_하강착지` SLAM). APPEAR_PLAYER는 양수 등장 높이를 사용한다. 나머지 높이·시간은0이며
CENTER만 절대 목적지 XYZ를 가진다. 기존25열 mechanic 부모 뒤에 같은 occurrence의7열
`PATTERNALBIONAIRBORNE`를 게시한다. Server가 살아 있는 플레이어 ID를 선택하고 등장할
때 그 플레이어의 현재 XZ와 navigation·body collision을 검증한다. 같은 시각이면 선택부터
처리한다. 기존 stage root 이동 경로가 phase의 높이를 소비하고 순간이동 때 root 원점도
갱신한다. SLAM은 시작 시점의 XZ를 고정하고 trigger 이후 원본 하강의 누적 최저값으로 착지하며 원본의 작은 반등으로
다시 뜨지 않는다. 이 착지 보정은 source Stage에 한정하며 다음 Stage부터 기존 native root 이동을
재개한다. 따라서 후속 상승 clip을 지면에 고정하지 않는다. 실패 시 기존 pose를 유지한다. 새 packet이나 Client gameplay 이동은 없다.
일반 Play는 복제된 플레이어 ID와 등장 시점의 위치를 preview 실행에 고정해 seek에서도
같은 결과를 사용한다. `Complete Play (Server)`가 실제 Server 권위 재생이며,
`resetBossToSpawn`이 참이면 기존 Gate별 spawn에서 시작한다.

SELECT_PLAYER의 optional `airborneTargetPositionPolicy`는 기본 `APPEAR`이며 위 기존 동작을 유지한다.
`SELECT`이면 선택 시점 플레이어의 navigation ground XYZ를 한 번 저장하고 이후 이동·사망·퇴장에도
APPEAR_PLAYER가 같은 좌표를 사용한다. 목적지 navigation·body 검증은 유지하며 실패 시 기존 pose를 보존한다.
optional `selectedEffectGroupId`는 이 SELECT 정책에만 허용하며 같은 Pattern의 두 개 이상 MAP/nonfollow
Effect occurrence를 참조한다. 첫 멤버의 MAP XYZ를 공통 원점으로 빼고 각 시작 시각에서 SELECT 시각을 빼서
기존 `combatobject.kouku.showtime.fixed` visual 하나에 투영한다. 원래 일반 Effect lane에서는 해당 멤버를 제외한다.
Server는 ground capture와 presentation-only combat object를 한 transaction으로 확정한다. 새 Shared packet은 없다.
선택 정책이 있는 supplement만 기존7열 뒤에 `SELECT`, visual ID, lifetime ms를 붙인10열을 사용한다.
visual/lifetime 쌍은 optional이며 capture만 있으면 빈 ID/0이다. visual이 있으면 lifetime1~600000ms이고
SELECT 시작 시각+수명이 Pattern 끝을 넘을 수 없다. Client 일반 Play의0ms SELECT는 비동기 Effect 준비보다 먼저
실제 player ground를 고정하며 동일 rewind에서는 같은 점을 재사용한다. 기존 APPEAR 정책은 변경하지 않는다.

DURATION `BOSS_TRACK_TARGET`은 추가 field 없이 지정된 Server pattern target을 향해 몸만
회전한다. occurrence의 start/duration이 회전 구간이며, 현재 yaw에서 목표 yaw까지 최단 각도를
남은 시간 비율로 보간해 마지막 유효 tick에 도달한다. 사망/이탈 시 기존 target 재선택을
사용한다. Collider·결과·Hold·Effect template은 없고 시각 객체를 만들지 않는다. 게시 시 기존
`mechanicTriggers`/25-field `PATTERNMECHANICTRIGGER`를 사용하고 다른 기믹 값은0 또는 `-`다.
SHOWTIME_PLAYER_TARGETS의 장판/추적 visual과 독립이며 새 Client yaw 판정이나 packet은 없다.
optional `followSpeedScale > 0`이면 대상의 실효 이동속도에 배율을 곱해 몸 정면으로 전진한다.
이 추적 이동의 회전은 수명과 독립적인 최단각 최대180도/초이며, 기존 이동 없는 회전 창은
남은 시간 보간을 유지한다. Saydon의 설치 BODY 모델 정면은 +X이므로 목표 body yaw는 방향각-90도,
전진 방향은 body yaw+90도다. Preview와 Server가 같은 tick 순서로 회전 후 이동한다.


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

Composition의 Camera/Sound는 World Object 참조 없이 전역 재생할 수 있다. 기존 Camera의
`anchorKind=WORLD, worldId=""`는 shot asset의 좌표를 사용하는 유효한 저장값이다.
WORLD의 worldId 필수 검사는 실제 Object transform을 소비하는 Effect/Light/Collider에만
적용한다. 공용 codec 변경 시 gameplay Composition과 Sequence Composition 양쪽의 실제 저장
행을 검증하며 Camera 값을 MAP으로 자동 변경하지 않는다.

Kouku All Effects와 Composition Resources의 저장 Effect 목록은 EffectAuthoringResourceTree의 동일한
표시명·검색·정렬과 `1관문 / 2관문 / 3관문 / 공통` 네 상위 분류를 사용한다. metadata를 우선하고
빙고는 3관문에 포함하며 관문 근거가 없는 ID는 공통에 보존한다. 분류 결과는 resource refresh 때
캐시하며 렌더 프레임마다 전체 경로를 재구성하거나 정렬하지 않는다. 각 화면의 Open/Play/Append는 유지한다.

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

시각용 model-less World 그룹은 `motionInstanceIds`의 enabled member들을 하나의 Composition
World box로 재생할 수 있다. 이때 `objectResourceId`와 `sequenceInstanceId`는 같은 group
objectId이며, 목록과 Append는 member의 전체 presentation tail을 포함한 길이를 사용한다.
Server는 그룹을 여러 cue로 분해하지 않고 기존 한 owned WORLD cue의 ID·clock·placement를
전달한다. Client는 같은 전용 `CWorldSequencePlayer`에 member들을 준비하고 생성 원점과
재생·탐색·중지를 공유한다. 모든 member가 꺼져 있거나 참조가 잘못되면 거절한다. 이 시각용
그룹의 member collider/combatBody/walkable은 지원하지 않으며 publisher와 Client admission이
거부한다. 그룹 생성으로 피해·충돌 판정을 Client에 추가하지 않는다.
칼날처럼 시각적으로 X축 자전하는 WORLD의 중심 CIRCLE은 local offset=0, bone 없음과
uniform XYZ scale 조건에서 모델 회전과 분리된 수평 원형 판정을 사용한다. 이동은 기존
Transform keys의 translation/visibility를 사용하며 물리 velocity·가속·공전·무작위 분산을
서버 경로로 추측 변환하지 않는다. 움직이는 중심 원은 STOP/HOLD와 Parent 반복창을 사용한다.
Server ENTER_AREA는 일정 반경 원의 이전 tick부터 현재 tick까지 이동 구간도 검사한다.
생성 지연·숨김 구간·Logic 창·Lifetime을 경계로 잘라 검사하며 숨은 구간을 가로질러 판정하지 않는다.
피해·즉사는 기존 Logic Result로 설정한다. 이 계약에 맵 끝이나 아이언메이든 접촉 시
오브젝트를 자동 소멸시키는 동작은 포함되지 않는다.
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

발탄 일반 피격의 강제 변위는 보행용 walkable clamp 대신 Server 벽/몸체 sweep 뒤 물리 지면 지지를 검사한다. 붕괴 void·지지면 부재·1m를 넘는 하강은 기존 FALLING으로 전환하고, 큰 상승은 이동을 끝낸다. 뒤잡기 ARENA_EJECTION의 별도 강제 발사 계약은 유지한다. 발탄 navpolicy의 보행 최대 단차는 1m이며 추적 이동은 매 tick 지면 높이를 검증한다. 보행 불가 지지면에서의 복귀와 대각선 모서리 등 남은 경계는 `.md/GB/09-23/2026-09-23_VALTAN_FORCED_MOVEMENT_SUPPORT_RESULT.md`를 따른다.

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

Rendering quality의 optional `colorAdjustment`는 `bloomTint:[R,G,B,1]`(RGB0..1)과
`desaturation:0..1`을 가진다. 기본값은 white/0이며 이전 profile 출력이 유지된다.
`environmentRegions`의 optional `priority`는 큰 값이 우선하고 같은 값이면 작은 convex AABB를 선택한다.
optional `postProcess`는 bloomThreshold(0..64), bloomIntensity(0..16), bloomTint, desaturation
네 필드를 모두 요구한다. optional `qualityOverride`는 같은 전체 quality 구조를 받아 독립 영역의
품질 기준을 보존한다. 활성 profile multiplier와 사용자 video 설정을 적용한 뒤 region postProcess를
덮어쓰며, 이탈하면 활성 profile의 실효 품질로 돌아간다. exposure/gamma/whitePoint와 기존 후처리
입력은 region blend 시간을 사용하고 기술 on/off 설정은 선택된 영역 값을 쓴다. optional
`specularColor:[R,G,B,A]`는 RGB 0~64/A 0~1이며 없으면 활성 profile specular를 상속한다.
추가 색상은 diffuse/ambient와 함께 보간한다. parser·publisher·Workbench serializer가 같은 optional
계약을 사용하며 잘못된 영역 품질·LUT·색상은 전체 catalog 교체 전에 거절한다.
Bern/Character Select의 source-rendering과 before-restoration profile은 Benchmark의 session 비교용이다.
자동 Save/Publish하지 않으며 native tone/LUT/DOF 복원과 현재 Hable adapter를 구분한다.

플레이어 profile의 defense는 발탄 incoming damage에 실제로 사용된다. 원작 Server 공식이 client
payload에 없으므로 `raw * 100 / (100 + defense)`는 `PROJECT_TUNED` 중앙 계약이며
`CGameplayCatalog::Apply_Defense` 한 곳에서만 계산한다. boss defense/outgoing 감산은 아직 없다.

수업용 `CMonster`와 `astar/Monster`는 제거 대상 레거시다. 제품 일반 몬스터는 Valtan archetype을 `MonsterCatalog.json`과 `MonsterProfiles.json`에 등록하고, Area `SpawnGroups.world.json` → publisher → Server `CSpawnGroupRuntime/CMonsterBrain` → Shared world entity spawn/snapshot/despawn → Client catalog presentation 경로를 사용한다. `MonsterProfiles.json` formatVersion 2의 `targetReleaseRange`, `turnSpeedDegreesPerSecond`, `acceleration`, `deceleration`, `arrivalSlowRadius`가 Server 권위 추적 유지·회전·가감속 정본이며 publisher는 이를 spawn-group bootstrap v4로 투영한다. Brain은 살아 있는 현재 타깃을 release range 안에서 유지하고 WINDUP부터 RECOVERY까지 타깃과 공격 방향을 고정하며, Server navigation 경로 단축과 기존 원형 body sweep/slide를 이동·knockback에 함께 사용한다. 레거시 클래스를 이 계약에 다시 연결하지 않는다.

Client 일반 몬스터 표현은 root-motion 억제와 network-transform 보간을 독립 정책으로 사용한다. Server 권위 몬스터는 기존 2-tick `CNpcNetworkTransformInterpolator`로 표시하고, `WINDUP` occurrence에서 공격 clip을 0초부터 한 번 시작한 뒤 `ACTIVE/RECOVERY` 동안 같은 clip을 이어서 재생한다. 다음 `WINDUP`은 같은 clip이어도 새 occurrence로 다시 시작한다. `MonsterCatalog.json` formatVersion 2의 presentation-only `attackPresentations[]`는 실제 clip과 playback rate를 소유한다. optional `endEffectAssetId`는 해당 공격 clip이 끝날 때 한 번 재생하는 Effect ID이며, 공격이 중단되면 미재생 cue를 취소한다. 마리오 광대의 망치 마지막 효과가 이 계약을 사용한다. Client는 Server entity ID와 occurrence tick으로 pool을 결정적으로 선택하므로 Server timing과 Client 간 일치성을 바꾸지 않는다. IDLE/CHASE 중 damage event는 catalog hit clip을 짧게 재생한 뒤 이전 locomotion으로 복귀하고 ATTACK/DEAD는 덮어쓰지 않는다. Valtan Loader는 spawn 전에 지원 archetype prototype을 미리 준비하고, 누락 모델/clip은 해당 archetype/action 표현만 격리한다.

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

모델·텍스처·사운드 등의 Resources payload는 팀장이 `Client/Bin/Resources` 물리 폴더로 관리하고 Git에는 추적하지 않는다. AssetPacks lock, immutable Resource manifest, Resource ZIP hash, Snapshot/Publish/Hydrate/Verify를 팀 완료 조건으로 사용하지 않는다. 코드와 데이터에는 Resources 상대 asset ID만 저장하고 팀원별 절대 경로 하드코딩은 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ 팀장이 전달한 Resources 물리 폴더 확인
→ 같은 commit의 게시된 Client/Server DataFiles 확인
→ C++/HLSL 변경 시 정상 증분 Debug Product Build
→ 담당 public interface에서 작업 시작
```

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, Data 정본, publisher/schema와 대응 Client/Server DataFiles 출력, 필요한 project/filter 등록과 검증, RESULT를 같은 변경 단위로 묶는다. 게시 snapshot은 해당 publisher로 생성하고 일반 `git add`로 전달한다. 받는 PC는 저작 변경이 없으면 전체 publish/navigation bake를 반복하지 않는다. 컴파일·링크 산출물, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, `Client/Bin/Resources` payload는 stage하지 않는다.

## 11. 완료 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

기본 Product는 Engine → Shared → Server → Client의 컴파일·링크와 SDK/shader/runtime DLL 배포를 수행한다. Client 프로젝트가 SDK 배포를 소유하므로 `UpdateLib.bat`을 별도 자동 단계로 반복하지 않는다. publisher와 광역 하네스는 명시 owner/Core/FullDiagnostic 경로에서 실행하며 매 pull/build의 필수 단계가 아니다. 데이터만 변경한 경우 해당 domain 게시·실제 소비 검증을 수행하고, C++/HLSL을 변경했으면 정상 증분 Product Build를 추가한다.

아래는 네트워크·레벨 전환 등 전체 흐름을 변경하거나 명시적으로 광역 통합 검증을 수행할 때의 확인 항목이다. 일반 변경 완료 조건은 `AGENTS.md`의 기능별 최소 검증을 따른다. 실제 Client/UI 실행과 화면 판정은 사용자가 수행한다.

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
- Character Select `Create Character`의 process-session pending/created nickname transaction, Server-approved Bern commit, created/audition direct Bern/Valtan/KoukuSaydon identity, Bern/Valtan Server-replicated nameplate

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

MAZE 진입 → 30tick 뒤 중앙의 Server 삐에로 상자(`MONSTER_KOUKU_CLOWN_BOX`)를 망치로 파괴 → 망원경 가격 → MAZE 망치 타격 → Server의 자기 문양 한 방 처치 → 1개 처치 개인 출구 → 암전 중앙 이동 → 생존 참가자 전원 집결 후 2관문 복귀를 사용한다. 문양별 목표는 동시에 1마리이며 목표 처치 전 세토 접촉 등으로 진행이 초기화되면 랜덤 통로로 보충한다. 중앙 반경 5m를 제외한 세토 접촉은 본인 스택·출구를 취소한다. `cardmiro.march.instance.from{3,6,9,12}.lane{1..9}` 36개 경로는 WorldSequence 정본의 선형 키를 WorldGameplay publisher가 worldbootstrap v10에 투영한다. Server 판정과 Client 표현은 protocol 81의 `PLAYER_SNAPSHOT::CardMaze` 행진 시계를 함께 소비한다.

카메라는 MapTool Camera의 `cardmaze.follow`/`cardmaze.telescope`에서 조정하고 MapAuthoring을 publish한다. 관전은 역할 이름이 아니라 플레이어별 관전 flag로 켜진다. 최초 담당과 탈출자는 중앙 상자를 망치로 다시 가격하여 각각 토글한다. 이동 암전은 서버 시작 tick 기준 36tick, 위치 commit은 18tick이다. 최종 복귀는 World Gameplay의 disabled `cardmaze.return` movePlayer 목적지를 읽으며 기본은 기존 2관문 (3.38, 10.56, 323.92)이다. 이 행을 활성화하면 밟기 트리거로도 동작하므로 설정 전용으로 disabled를 유지한다. WorldGameplay publish와 서버 재시작이 필요하다.

문양 플레이어·병사 발밑은 `cardmaze.mark.heart/spade/club/diamond`, 개인 출구는 `cardmaze.exit.heart/spade/club/diamond` Effect GROUP을 사용한다. 두 그룹은 같은 `cardmaze.symbol.*` Decal leaf와 기존 `Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47{,_1,_2,_3}.dds`를 사용한다(하트/스페이드/클럽/다이아몬드 순서). 기존 춤 연출은 수정하지 않는다. 플레이어/병사 표시는 해당 객체의 이동 위치만 따라가고 출구는 서버 출구 좌표에 고정한다. Client는 이동·처치 판정 없이 snapshot과 복제 객체로 표시·정리만 한다. 다인 플레이와 Release의 망원경 담당은 문양과 목표를 받지 않는다. Debug Server에서 방에 정확히 한 명이면 최초 상자 타격자가 HUNTER 문양과 망원경 owner identity를 함께 받는다. 이 1인 테스트만 관전 중 이동과 중앙 밖 관전 유지가 허용되며 세토 접촉·처치·출구는 기존 규칙을 사용한다. 다른 참가자가 죽어 혼자 생존한 상황은 이 예외를 켜지 않는다. Protocol 79 Client/Server를 함께 사용한다. 화면 크기·색상·실제 4인 입력·접촉·암전은 사용자 런타임 확인 대상이다.


### 입장 Sequence와 열린 아레나 전투 연결

Kouku Sequence Composition의 optional `enterCombatOnFinish`는 GATE1/GATE2/GATE3별 입장 Pattern 하나를 식별한다. publisher는 Action의 저장 `patternFlows`와 Sequence의 입장·클리어·도착 슬롯을 `RAIDGATE`, `RAIDFLOWSTEP`, `RAIDARRIVAL` 행으로 투영한다. `C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST` START는 Server-active gameplay revision, 저장 Action·Sequence revision, 시작 관문을 전달한다. Server는 같은 비쿠크 gameplay를 유지하는 최신 게시 Product만 승인하고 실행 동안 immutable catalog를 유지한다. Debug owner의 STOP은 실행 epoch가 일치해야 하며 재전송은 같은 요청 결과를 반환한다. Release는 published `RAIDGATE.entrySequenceInstanceId`에 해당하는 입장 collider 접촉이 같은 준비 helper를 호출한다. 기존 단독 WorldSequence 재생을 중복 실행하지 않으며, 준비 실패 뒤에는 해당 one-shot activation만 복원해 재접촉을 허용한다. 기존 GateProgress packet ID를 유지한 뒤 Raid packet을 추가한다.

Server는 시작 시 1~4명의 PlayerId 순서를 고정하고 `PREPARING` 상태에 roster와 준비 기한(Release 10초, Debug의 지연 리소스 준비는 20분)을 전송한다. 각 참가자는 저장 Action·Sequence의 정확한 revision을 검사하고 Sequence 문서를 immutable copy로 미리 읽은 뒤 같은 epoch/revision으로 `READY` 또는 이유가 있는 `FAILED`를 회신한다. 준비 단계에서는 재생·teleport·boss despawn을 하지 않는다. 모든 시작 참가자의 READY 뒤 다음 fixed tick에서 공통 시작 시각을 확정한다. FAILED·timeout·owner STOP·참가자 퇴장은 기존 actor·위치·미니게임 상태를 보존한 채 준비를 중단한다. 중복 ACK는 멱등이며 이전 epoch·다른 revision·late join의 ACK는 시작 roster를 바꾸지 못한다. 입장 중 이동·스킬을 차단하며 저작 도착 시각에 각 슬롯을 이동한다. 1·2관문 Sequence가 끝나면 기존 audition 경로로 Flow의 Pattern/Bundle을 순서대로 실행한다. Client는 READY 전에 관문 오브젝트와 조명을 준비하고 같은 Server 실행의 stage를 전투 전환에서 commit한다. 카드미로가 활성 상태이거나 참가자의 역할이 남아 있으면 다음 Flow 항목을 기다린다. 게시된 primary 보스의 실제 HP 0/사망은 기존 GateProgress clear mask를 갱신하고 `WAIT_GATE`로 전환한다. Gate 2의 보조 배우는 이 완료 조건을 지연하거나 대신 충족하지 않는다. 자동 10초 전환은 없으며 기존 클리어·MVP·던전입장 UI를 거친 고정 roster 전원의 승인만 다음 관문 Sequence를 시작한다. 거절·투표 timeout은 WAIT_GATE를 유지한다. 재시작 투표 승인도 같은 관문의 입장 Sequence와 첫 Flow 항목으로 돌아간다. 2관문 클리어 후 승인하면 3관문 입장 Sequence만 재생한다. 3관문 최초 Sequence 종료는 `WAIT_ENTRY`로 전환하며 HUD를 표시하되 이동·스킬 입력을 차단한다. 마지막 네 Effect의 도착 슬롯 위치와 전투 시작 위치는 별개다. 기존 leader·전원 승인 정책의 `3관문 입장` 버튼이 명시 `ENTER_GATE3` 투표를 제출하고, Server가 모든 참가자의 session·profile·목적지 navigation/높이/충돌과 기존 audition evaluator의 첫 Flow admission을 staged actor로 검증한 뒤 전원을 원래 전투 위치로 이동하고 첫 Flow를 시작한다. 검증 실패는 위치와 대기 상태를 유지한다. 한 번 입장한 뒤 버튼은 `재시작`이며 재시작 Sequence 뒤에는 자동으로 전투를 시작한다. 3관문 클리어 후 별도 `빙고 입장` UI·투표를 표시하지 않는다. Server의 5초 false-clear 뒤 Encore intro Sequence를 자동 재생하고 준비·목적지 검증을 거쳐 BINGO 전투를 시작한다. 단독으로 소환한 보스의 Kill Boss 이후 ADVANCE/RESTART도 기존 raid 준비 owner로 들어가 전원 READY 뒤 다음/같은 관문 intro를 먼저 재생한다. 준비 실패는 이전 actor와 위치를 보존하고, 성공 뒤 이전 audition·Bingo·미로를 정리하고 Mario 진입 단계를 초기화한다. UI에는 기존 세 관문을 완료 상태로 표시한다. 빙고 클리어는 WAIT_GATE와 clear mask를 유지하여 기존 EXIT·재시작 UI를 받는다. 단순 Flow 종료나 보스 제거는 관문 클리어로 취급하지 않는다.

`S2C_KOUKUSAYDON_RAID_STATE`의 phase·epoch·시작/종료/server tick이 모든 Client의 연출 시계다. Client는 저장 Sequence의 정확한 composition ID와 revision을 별도로 pin하고 기존 presentation player로 표현한다. 늦은 입장에는 현재 상태와 원래 시작 tick에 이어 `S2C_GATE_PROGRESS_STATE`의 현재 관문·클리어 mask·진행 중 투표 상태를 초기 reliable batch로 전송한다. 시작 참가자 슬롯과 투표 권한은 추가하지 않는다. 시작 참가자가 퇴장하면 실행을 중단한다. 시작 참가자가 준비 중 Sequence를 불러오지 못하면 FAILED로 전원 시작을 중단한다. 준비가 끝난 뒤의 표시 실패나 late join의 문서 불일치는 해당 Client에 이유를 표시하며 이미 확정된 Server 실행 시각을 바꾸지 않는다. 단독 F1 Gate/일반 Preview는 별도 기존 시험 경로를 유지한다. 일반 Preview의 도착 Logic 또는 ImGui의 `3관문 입장 전 공간` 이동은 기존 Server 승인 teleport를 사용한다. 제품 Raid가 없는 이 공간에서는 같은 `ENTER_GATE3` 투표를 받아 실제 proposer 위치와 참가자 roster, 전원 목적지 및 boss 생성·정리를 사전 검증한 뒤 기존 3관문을 활성화한다. 거절·timeout·사전 검증 실패는 이전 위치와 보스를 보존한다. Client에서 WAIT_ENTRY를 만들지 않는다. Client의 대기 BGM과 이 버튼 표시는 공유 ready-area 범위에 대한 replicated XYZ를 읽으며 이동 판정 권위를 갖지 않는다. 시작 공간 및 3관문 판자 공간의 BGM은 local Sequence 재생 중과 Server CINEMATIC/COMBAT 중 억제하고 Level 퇴장에서 정리한다.

Raid packet 계약은 protocol 96이며 Server와 Client를 함께 빌드·재시작해야 한다. `GATE_PROGRESS_KIND::ENTER_GATE3`는 현재 관문 재시작과 분리한 3관문 입장 요청이다. `WAIT_ENTRY`는 기존 phase 값 뒤에 추가했고 GATE3의 종료 tick 없는 대기 상태로만 허용한다. START와 상태의 관문 ID는 기존 BINGO를 포함한다. protocol 95 이하 peer는 ENTER_WORLD의 exact version 검사에서 거절한다. 이후 Action·Sequence 게시 데이터만 바뀌면 다음 START가 exact revision으로 재승인하며 진행 중 실행은 기존 catalog와 문서를 유지한다. World placement·navigation·다른 gameplay balance를 적용하는 Server 재시작과, Client의 저장 문서 Reload·화면 확인은 별개다. 게시 또는 파일 설치 성공을 실행 중 도구의 미저장 draft 반영이나 화면 검증 완료로 기록하지 않는다.

Object Tool의 Group Layout은 motion emission의 count·spacing·Delay를 소유한다. Box Detail의 Object/Motion 열기는 stable object ID와 instance ID를 전달하며 자동 preview·저장을 하지 않는다. WORLD Collider/전용 Logic 참조를 함께 바꾸는 저장은 source 변경을 재확인한 뒤 교체하며, 공유 또는 모호한 참조와 미저장 Composition은 이유를 표시하고 기존 문서를 보존한다.

Kouku Animation occurrence의 `sourceStartMs`와 optional `sourceEndMs`는 원본 클립의 선택 구간이며, `sourceEndMs=0` 또는 생략은 원본 끝이다. `startOffsetMs`와 `playMs`는 stage 안의 재생 위치와 길이를 소유한다. `LOOP_TO_WINDOW`는 선택한 source 구간만 반복하며 stage 길이는 별도로 편집한다. Source In/Out은 원본 구간만 바꾸고, 일반 clip 앞 edge는 timeline/source 시작을 함께 자른다. 반복을 켠 뒤 edge 편집은 반복 구간을 유지하며 timeline 길이만 바꾼다. 인접 clip의 Blend In은 이전 clip의 실제 HOLD/LOOP 종료 sample과 보간한다. Preview, 제품 NPC, Server용 bone collider bake는 같은 source 시간 계약을 사용한다. Product는 기존 stage당 animation 하나와 Server action clock을 유지하며 저장 후 기존 publisher를 거쳐 적용한다.

Kouku의 자동 원본 이동은 같은 publisher가 생성한 stage `rootMotionSamples {timeMs, forward, lateral, up}`를 기존 `PATTERNSTAGEROOTMOTION`으로 전달한다. 세 위치 성분은 meter이며 Server의 stage 시작 위치·yaw, navigation·body collision이 실제 XYZ를 확정한다. 기존 세 성분 bootstrap은 up=0으로 호환하고 새 Kouku 출력만 네 성분을 사용한다. snapshot protocol 변경은 없다. 수동 BossMotion·돌진·REAL_GAZE_TELEPORT가 있는 Pattern에는 자동 이동을 겹치지 않는다. 자동 대상 Pattern의 모델 root와 본 궤적에서는 Server에 이전한 이동을 억제한다. 원본 해석·저장·재생 절차와 in-place clip 경계는 `ANIMATION_TOOL_OWNER_HANDOFF.md`의 Kouku 절차를 따른다.


## C++ 공용 선언을 소비할 때의 include 경계

`Engine_Defines.h`에 기능별 헤더를 추가해 모든 담당자에게 전파하지 않는다. 렌더 설정은
`Engine_RenderTypes.h`, 정점은 `Engine_VertexTypes.h`, keyframe은
`Engine_AnimationTypes.h`, 초기화는 `Engine_InitTypes.h`를 실제 소비 파일에서 포함한다.
`GameInstance.h`는 렌더 API의 전방 선언만 제공하므로 반환값을 사용하는 CPP는 해당 정의를
직접 포함한다. Assimp·DirectXTK·FX11·DirectInput도 실제 구현의 의존성으로 선언한다.
표준 라이브러리 PCH·대형 CPP 분리·병합 후 Build 확인은 `../../Tools/Build/README.md`를 따른다.

## Character Action의 독립 Collider / Logic / Result

`Data/Animation/HitShapes/<Asset>.hitshapes.json`의 formatVersion 4는 실제 caster/projectile hit마다
stable `colliderId`, `logic.logicId`, `result.resultId`를 가진다. Logic은 DURATION / AREA_OVERLAP으로
자기 collider와 RESULT를 참조하며 RESULT kind는 DAMAGE / STAGGER / COUNTER다. 종류별 판정은
서로 독립된 시간·shape를 사용하고 수치는 기존 PlayerSkills의 damage profile / staggerDamage /
counterPower를 소비한다. DAMAGE만 기존 HP 예산·part damage·push를 사용한다. trait-only 행이
HP 피해나 카드미로 즉사를 만들지 않는다. 원본 provenance와 나머지 skill/projectile 필드는 보존한다.

Gameplay publisher는 연결 ID·종류·상한을 검증하고 gameplay bootstrap v34의 기존 SKILLHIT/SKILLPROJ에
Result kind를 싣는다. Server `CPlayerSkillSystem`은 DAMAGE 행만 HP 예산을 분배하고 각 trait 행은
기존 `CServerCombatHitRuntime`으로 해당 수치만 전달한다. Client UI는 packet이나 피해 판정을 만들지 않는다.
저장·적용 경로는 Action Workbench의 Save Combat → 명시 Gameplay Publish → 새 Server 재시작이다.
기존 v33 실행 파일/부트스트랩과 혼합하지 않으며 네트워크 packet version을 바꾼 계약은 아니다.

통합 창의 사용법과 각 저장 owner는 `ANIMATION_TOOL_OWNER_HANDOFF.md`를 따른다.


### SHOWTIME_PLAYER_TARGETS의 optional 랜덤 낙하 세트

Composition logic는 기존 `fixedSelectionGroupId` 또는 `trackingPresentationOccurrenceId`를 유지하고 optional `randomVolleyOccurrenceSets`, `randomSpawnIntervalMs`, `randomArenaRadiusM`, `randomArenaHeightToleranceM`을 함께 저장한다. `randomVolleyOccurrenceSets`는 순서 있는 stable occurrence ID 배열1~32개다. 각 배열의 원본 시간 차이·TRS를 유지하며 최소 하나의 MAP 행과 선택적인 BOSS-follow 행을 허용한다. bone/World/Logic 부착은 지원하지 않는다. 랜덤은 서버 지면 위치 선택이며 세트 순서는 저장된 배열 순서다.

Projector는 각 세트의 hash visual ID·lifetime을 `showtimeTargets.randomVolleys`로 게시한다. Gameplay publisher는 기존11field `PATTERNSHOWTIMETARGETS` 뒤에10field `PATTERNSHOWTIMERANDOM`을 추가한다: encounter, pattern, occurrence, ordinal, visual ID, lifetimeMs, intervalMs, radiusM, heightToleranceM. ordinal은같은 duration 안에서0부터 연속하며 같은 콘텐츠의 visual ID가 반복될 수 있다. 서버에는 Effect asset path나 selection group을 전달하지 않는다.

서버는 보스 spawn 중심의 기존 arena sampler로 exact walkable/height를 검증하고 전역 한 세트를 각 간격에 생성한다. 4인이라고 랜덤 세트 수를4배로 만들지 않는다. 기존 플레이어별 fixed/tracking은 독립적으로 유지한다. 종료 뒤에는 새 세트만 멈추며 이미 생성한 finite 세트의 수명은 보존한다. Client는 같은 source entity의 실제 보스 transform/model과 CombatObject 지면 root를 기존 Sample 함수에 따로 전달해 BOSS 총구와 MAP 낙하를 같은 clock으로 표현한다.

### World Object Collider 편집과 저장

Object Sequencer의 Play 왼쪽과 Object Detail의 Edit Parent Object 왼쪽 Save는 같은 원본 저장을 사용한다. 새 Logic 정의만 만들었어도 Pattern 또는 Sequence 문서는 미저장 상태다. 해당 Save 후 Object Save를 사용한다. 이미 실행 중인 Publish만 완료를 기다리며, Object Save를 위해 Publish를 새로 시작할 필요는 없다. 저장 상태와 게시 상태는 별개로 표시한다.

Collider 행은 일반 피해(DAMAGE, 최대 HP 정수 비율), 즉사(INSTANT_DEATH), 갈고리 부착(HOOK_CAPTURE)을 설정한다. 모델 자전과 독립된 수평 rectangle이 Object 이동을 따르며, 갈고리의 본과 미터 offset으로 실제 끝 위치를 지정한다. WorldSequence 데이터 계약은 AREA_DATA_LAYER_GUIDE의 Object Motion Collider 절을 따른다. 제품 결과는 기존 Server ENTER_AREA와 WORLD_HOOK_TIP attachment로 전달한다. 기존 잡힌 캐릭터 pose를 유지하므로 새 매달림 애니메이션을 복원했다는 의미는 아니다.

### HP가 있는 Kouku WORLD cue

Object Tool `combatBody/UNTIL_DESTROYED`는 Kouku projector→`PATTERNWORLDCOMBAT`→Server의 명시적 `WORLD_OBJECT` 피격 대상으로 연결된다. 기존 PlayerSkill/Projectile/ServerCombatHit 경로가 HP를 판정하며, 일반 monster/NPC bootstrap 생성과 Client monster presentation을 사용하지 않는다. 생성 전 cue packet을 검증하고 boss tick 중에는 stage한 뒤 tick 경계에서 entity를 commit한다.

WORLD timeline은 한 번의 spawn을 소유하고 Server HP와 개별 cue identity가 이후 수명을 소유한다. 정상 Pattern/Bundle 완료는 유지하며 HP0, 취소, 새 run, 원래 boss의 제거·사망, owner session 퇴장과 room reset은 exact `STOP_CUE`로 모델·Effect를 함께 정리한다. late join은 살아 있는 cue만 재생하고 Client tombstone은 먼저 도착한 STOP 뒤의 지연 PLAY를 무시한다. wire protocol88의 `bUntilDestroyed`와 `STOP_CUE`를 쓰므로 양쪽 실행 파일을 함께 갱신한다. 데이터 필드·현재 고정 WORLD/count1/LOOP 지원 범위는 [Area 가이드](AREA_DATA_LAYER_GUIDE.md#생존하는-pattern-world-object와-object-tool-반복)를 따른다.


### Kouku occurrence의 반복 Effect 수명과 Pattern 미리보기

V1_EFFECT/V1_ELEMENT presentation occurrence의 optional `loopEffectToDuration`(기본false)은 occurrence 끝까지 원래 속도로 재생한다. native loop0는 bounded 방출을 연장하고, finite source는 prepared 전체 수명마다 handle을 다시 생성한다. 반복의 root/bone/source-anchor 샘플은 cycle 시작 나이를 더한 전체 occurrence 시계를 사용한다. `fitEffectToDuration`과 동시true는 거절한다. sprite/mesh/CASCADE_RIBBON_V1의 실제 source recipe만 지원하며 particle 고유수명·원본 emitter 주기·finite loop·공용 Effect asset을 변경하지 않는다. 기존 occurrence 종료 owner가 정리한다. Workbench의 `Loop Effect through lifetime`와 `Match remaining animation time` 후 Apply/Save로 마지막 animation end에 맞출 수 있다.

Effect Tool에서 Effect를 열면 저장된 SourceModelPreview를 기본 애니메이션으로 사용한다. Use current Pattern animation을 명시적으로 누르면 현재 Composition의 해당 Effect occurrence가 유일하거나 선택됐을 때 animation snapshot·effect start·duration을 고정하며 본 sampling도 같은 값을 소비한다. 이후 Boss Tool 선택 변화는 고정된 snapshot을 바꾸지 않으며 다시 버튼을 누르면 갱신한다. Use saved source animation과 성공한 문서 재오픈은 저장 원본으로 복귀한다. 로드 실패·미저장 취소는 현재 선택을 보존한다. 중복 미선택/미지원 nested·blend·trim은 이유를 표시하고 기존 preview 선택을 유지한다. 공용 Effect asset은 수정하지 않는다. 제품 빌드와 사용자 화면 판정은 별개다.


### Kouku Pattern 삭제

Patterns 창의 Delete Selected Pattern 또는 Pattern 우클릭 Delete는 같은 확인창을 연다. Flow·Bundle·Parent timeline/Pattern box·Logic의 후속/분신/방향/랜덤 후보·Summon 참조가 있으면 owner와 stable ID를 표시하고 삭제를 차단한다. 해당 owner에서 연결을 먼저 제거한다. Delete from Draft는 candidate 검증 후 draft만 바꾸며 Save와 Publish All Patterns가 각각 저장과 제품 반영을 소유한다. 공용 Animation/Effect/Logic 원본은 삭제하지 않는다. 오래된 확인창·외부 수정·게시 중·검증 실패는 기존 draft를 보존한다.

### Kouku 추적·회전 카드의 생성과 접촉

Composition DURATION 또는 TRIGGER의 `PURSUIT_PROJECTILES`는 `visualIds` 1~4개와 `contactVisualId`로 저장된 V1_EFFECT resource를 참조한다. `speedMps`는 .01~100m/s, `contactRadiusM`은 .01~10m, `spawnRadiusM`은 0~100m다. `spawnIntervalMs=0`은 한번 생성, `countPerWave` 생략은 문양 개수만큼 생성한다. 명시 개수는 1~16이다. `maxDistanceM`은 선택 필드이며0~1000m다. 생략/0이면 이동 거리는 기존 수명·접촉으로 제한하고, 양수이면 생성 위치부터 누적 이동 거리를 제한한다. `homing=true`와 한번 생성, 거리 제한0일 때만 `lifetimeMs=0`을 허용한다. 0보다 큰 수명과 발사 간격은 최대600000ms다. Logic occurrence duration은 생성 창이며 무한 객체의 자연 종료 시각이 아니다.

Server가 대상 선택·이동·Shared XZ swept circle 접촉과 객체 종료를 소유한다. 접촉 반경에는 대상 player body radius를 더한다. 무한 객체는 접촉 전 시간으로 만료하지 않으며 명시 Stop·owner/target 소멸에서 정리한다. 유한 직선 객체는 접촉·수명·양수 최대 거리 중 먼저 도달한 종료 조건에서 같은 폭발 event를 한 번 보낸다. 기존18열 bootstrap은 그대로 읽고, 양수 최대 거리는 선택적인19번째 열로 전달한다. 이 Logic은 damage나 RESULT 판정을 임의로 추가하지 않는다.

publisher는 실제 Effect 참조를 `targetedCombatVisuals`로 고정한다. Client는 서버 snapshot의 위치·yaw와 독립된 표시 반복 시계를 사용한다. 접촉 event의 stable ID는 고정한 `contactVisualId`와 일치해야 하며 폭발은 카드 despawn 뒤에도 자기 위치·리소스를 보존한다. 새 socket 호출·Client local 추적 권위·중복 모델 runtime은 없다. 편집은 Logic Detail의 카드·폭발 선택과 속도·최대 거리·반경·수명·간격 입력, Apply → Save → Publish All Patterns로 반영한다.

Kouku Product의 게시 가능한 pattern 개수 상한은 Shared `MAX_VALTAN_PATTERN_FLOW_SLOTS`와 같은255다. projector·Gameplay bootstrap·Boss Tool은 동일 한계를 적용한다. 저장된 미완성 Pattern은 ready inventory에서 별도 unavailable로 남으며 정상 Pattern을 게시하기 위해 원본을 삭제하지 않는다.


### Kouku 연출의 텍스트 자막

Composition presentation resource의 `kind=SUBTITLE`은 `subtitleText`와 `subtitlePosition` (`NORMAL` / `UPPER`)을 소유한다. 텍스트는 1~4096 byte valid UTF-8 plain text이며 줄바꿈 LF를 허용하고 markup/control은 거부한다. `assetId`는 원본 GameMsg stable ID이고 이미지 파일 경로가 아니다. resource/occurrence는 MAP anchor와 `followBoss=false`를 사용하며 기존 occurrence의 `startMs`/`durationMs`로 시간을 편집한다. 기존 Save/Reload/Seek/Stop 경로가 같은 데이터를 소비한다.

SUBTITLE occurrence의 `positionOffset` X/Y는 높이 1080 기준 화면 pixel offset이며 양수 Y가
아래 방향이다. `scale` X는 균일 글자 배율이고 Box Detail의 `Text scale`이 XYZ를 함께
설정한다. `Screen X / Y`와 `Text scale`은 같은 preview clock의 활성 자막에 즉시 반영되며
Preview/Apply/Save와 제품 재생이 같은 값을 소비한다. rotation/Z offset은 자막 배치에
사용하지 않는다. zero offset/unit scale은 기존 NORMAL/UPPER 배치를 유지한다. 화면 폭을
넘는 문구에는 기존 폰트의 폭 맞춤 축소가 적용된다.

Presentation Player의 읽기 전용 활성 자막 목록을 MainApp이 기존 한글 폰트와 UI text layer로 그린다. cinematic HUD 숨김과 자막 숨김은 분리된다. 이 계약은 현재 한국어 문구를 직접 저장하며 언어별 catalog 선택이나 다국어 font fallback 구현을 뜻하지 않는다. 원본 시간/문구가 없는 구간은 임의 대사를 만들지 않는다.


### 빙고 Parent와 서버 주기 기믹

BINGO는 기존 saved Flow의 Pattern/Bundle 경로로 실행하며 새 별도 시퀀서나 보스 런타임을 만들지 않는다. BINGO Parent가 G3 MN_RPCT_05 Pattern을 재사용할 때만 `boss.kakulsaydon.g3.saydon` → `boss.kakulsaydon.bingo.saydon` child 참조를 허용한다. 그 외 관문/owner 불일치는 계속 거부한다. `DURATION / BINGO_BOARD`는 결과·collider를 소유하지 않는 1~600,000ms 서버 주기 기믹이다. 해당 행이 끝나면 신규 발생을 멈추며 기존 폭탄 표식·fuse는 완료한다. 같은 Complete Play run의 반복 Parent는 보드와 다음 bomb/hammer/madness deadline을 유지한다. run restart/exit/아레나 시작점 복귀는 모두 초기화한다.

BINGO 직접 시작 Flow는 Action/Sequence revision을 pin하고 준비와 목적지 admission을 마친 서버 전투를 소비한다. Parent의 선택적 `loopStartPatternOccurrenceId`는 자기 자식 occurrence ID를 참조한다. Server는 자식을 실제 완료 순서로 실행하고 마지막 자식 뒤 해당 기점으로 돌아간다. 최초 구간 24개와 반복 구간 14개가 같은 P96 안에 있으며, BINGO_BOARD의 소유권과 주기 deadline은 Parent를 다시 시작하지 않고 유지한다. Stop·사망·관문 종료는 기존 실행 정리 경로를 사용한다.

Mario Parent의 선택적 `playChildrenSequentially: true`는 원래 입장 애니메이션 뒤 자식을 순서대로 실행한다. 자식의 카운터 후속 그로기와 완료, 플레이어의 Mario 복귀가 끝나야 기존 2페이즈 후속 Pattern을 시작한다. 두 옵션은 Client 문서 codec, projector, `PATTERNPARENTCHILD` 게시 행과 Server catalog/scheduler가 함께 소비한다. 옵션 없는 기존 Parent는 기존 유한 펼치기를 유지하며, 새 옵션의 중첩 자식 Parent와 잘못된 반복 ID는 거부한다.

보드는 처음 일반 해골2칸,5초마다 살아 있는 보드 위 플레이어1명 표식,5초 후 현재 칸 중심에 폭탄 설치,3초 후 중심+상하좌우를 일반해골로 만든다. 기존 일반해골은 빨간해골로 승격하며 빨간해골은 유지한다. 기존 완성 line 승격도 보존한다. 두 해골 모두 초당 광기3이며 최대 도달 시 기존 광대 변신 policy를 소비한다. 10초마다 같은 축의 서로 최소2칸 떨어진 두 무작위 경로를 선택한다. 원본4.69m 망치 머리 폭보다 넓은6.08m 이상 간격으로 실제 머리도 겹치지 않는다. UV 화살표3초 뒤 원본 하강1.4초+이동1.6초를 소비하고 머리의 swept XZ만 서버 즉사 판정한다. 높은 사슬은 판정하지 않는다.

갈고리 attachment는 마지막 authored grip에 도달하면 끝쪽 정지/tail을 기다리지 않고 이동 잠금을 해제한다. 마지막 숨김 key 좌표도 적용한 뒤 해제하며 강제1.5초 knockdown을 추가하지 않는다. 정지 갈고리는 기존 명시 deadline을 유지한다.


### 쿠크 바닥 이동과 동적 발판의 예측

기존 TRIGGER mechanic의 `BOSS_TELEPORT_XZ`는 Y를 보존한다. `BOSS_TELEPORT_GROUNDED`는 navigation·authoring 목적지 높이·body overlap을 먼저 검증하고 성공할 때만 보스와 animation root 기준을 바닥으로 이동한다. 해당 Pattern의 다음 Stage root origin도 바닥에서 시작하되 원본 Up curve는 유지한다. 실패는 위치·root·정책을 보존하고 Pattern 종료/새 실행은 바닥 정책을 초기화한다. 같은25필드 bootstrap 계약의 새 kind이며 Shared packet 형식은 바꾸지 않는다.

Server가 제공한 동적 support 위에서는 기존 snapshot.canPredictMove=false로 Client 정적 nav 예측이 Server Y를 덮지 않게 한다. support 포함 판정은 ServerNavigation의 exact circle을 공유한다. 이 값은 표현 예측 허용이며 이동 입력 잠금이 아니다. typed MoveGoal과 Server simulation은 계속되고 Client는 기존 XYZ snapshot 보간을 사용한다. 발판 이탈·소멸 뒤에는 원래 예측 조건으로 복귀한다.


### 쿠크 안전존 Duration과 상태 문구

Composition `INVULNERABILITY_ZONE`은 같은 시간 창의 고정 MAP Collider 1~64개를 연결하는 DURATION이다. Result 슬롯은 없으며 영역 안 플레이어에게 같은 Pattern 실행의 `INSTANT_DEATH`, `MAX_HP_PERCENT_DAMAGE`, `FEAR` Result가 새로 적용되는 것을 막는다. 이미 걸린 공포를 해제하는 정화나 다른 보스·별도 페널티까지 막는 전역 무적은 아니다. 게시 region의 XZ 플레이어 중심 포함 판정을 소비한다.

Protocol 99의 `SNAPSHOT_PLAYER.iInvulnerabilityZonePulseTick`은 서버가 정한 입장 및 2초 간격의 표시 occurrence이며 밖/종료/비활성에는 0이다. Client는 파란 무적 문구만 표시하고 지역 충돌이나 반복 시간을 판정하지 않는다. 문구 중복 제거는 플레이어와 단어별로 구분해 서로 다른 상태의 tick이 상대 문구를 다시 생성하지 않게 한다. Client와 Server를 함께 빌드·재시작한다.


### 쿠크 Result의 강제 밀림과 아레나 경계 이탈

`MAX_HP_PERCENT_DAMAGE` Result의 양수 `pushRangeM`/`pushMs`는 기존 밀림 시간과 거리를 사용한다. optional `forcePush`와 `pushCanLeaveArena`의 기본값은 false이며, true일 때 양수 거리 또는 상승 높이가 필수다. `pushYawOffsetDegrees`는 기본0, 범위−360~360이고 0이 아닌 값은 `BOSS_FORWARD` 밀림에만 허용한다. 방향은 body local+Z에 yaw offset을 적용하므로 collider의 실제 축이+X라면+90도를 사용한다. 사선 collider는 각자의 저작 축을 소비한다.

`forcePush`는 공포·다운·기상 보호·진행 중 밀림을 새 밀림으로 교체한다. 사망·낙하·잡힘·패턴 부착·맵 이동 등 제외 상태는 유지한다. 안전존이 같은 Pattern의 피해 Result를 막으면 그 Result의 밀림도 발생하지 않는다. `pushCanLeaveArena`는 Kouku 본 아레나의 명시된 밀림에서 외곽 통과가 확인됐을 때 기존 서버 FALLING→DEAD 경로를 사용하며 Mario와 일반 밀림의 navigation 제한은 유지한다. 거리만으로 사망을 예약하거나 Client가 낙사를 판정하지 않는다.

Gameplay bootstrap의 `PATTERNLOGICPUSH`는 기존8/9필드를 계속 읽는다. 새 flag가 필요하면 direction·force·leave의11필드, yaw까지 필요하면12필드, ballistic은13필드, 상승 높이는14필드를 쓴다. Client codec, projector, Gameplay publisher와 Server catalog가 같은 조건을 검증한다.

### 쿠크 포물선 넉백과 크기 프로필 보완

Composition Result의 optional `pushBallistic`은 기존 직선 push 기본값을 유지한다. 활성화 시 `pushRangeM`은0~100m, `pushMs`는100~5000이며, optional `pushHeightM`은0~100m다. 거리0은 양수 높이를 가진 수직 상승에만 허용한다. 높이0은 기존 gravity를 사용하고 양수는 지정 수명 중간에 해당 높이를 지나는 상승·하강 호를 만든다. `pushCanLeaveArena=false`는 비행 중 XZ를 navigation·collision 경계 안으로 제한한다. Gate1은 항상 이 펜스를 적용한다. 다른 관문의 명시 허용 비행만 물리 바닥 이탈 시 FALLING으로 연결한다. AWAY_FROM_CONTACT는 실제 판정 장판 중심 기준이고 BOSS_FORWARD만 yaw offset을 사용한다. Client 편집→projector→publisher→Server parser가 같은 값을 소비한다.

Character Size Save/Reload는 계속 선택 맵별 camera JSON을 소유한다. 카메라 컷신 재생 여부가 크기 적용을 막지 않는다. Test/Training/Maharaka 공용 Development는 CharacterSelect의 저장된 크기만 읽고 기존 카메라 포즈를 유지한다. 현재 맵들의 Artist/DimensionMaster 배율을 동일하게 맞춘 값은 각 Data/Camera 문서가 정본이며 모델 자체 catalog scale은 별개다.



### 쿠크 추적 카드 Trigger와 접촉 Preview

`TRIGGER / PURSUIT_PROJECTILES`는 각 Logic Box의 `startMs`에 한 번 생성하며 `spawnIntervalMs=0`이다. 박스 길이는 이미 생성한 카드의 수명이 아니다. `lifetimeMs=0`, homing과 거리 제한0은 기존 room-owned 추적으로 접촉 전까지 유지하며 명시 Stop·대상 무효·방 정리는 기존 소유권 경로로 종료한다. `DURATION`의 기존 순차 생성은 계속 지원한다. 두 종류 모두 설치된 세이튼 +X 전방을 body yaw+90도로 해석한다. 영구 추적의 전체 수명 CONTACT는 임시 최대시간을 실제 만료로 사용하지 않으며 명시한 짧은 판정 창은 보존한다.

Composition의 `Play Pattern`은 현재 Apply된 메모리 draft를 기존 Server audition으로 실행한다. Parent와 Bundle도 필요한 패턴 연결을 함께 준비한다. 저장·Publish 없이 request sequence와 SHA-256으로 고정한 임시 Kouku rows를 승인하며, Server가 접촉·피해·상승/하강과 비행 경계를 소유한다. Server 선택 실행은 0ms부터 시작한다. `Play`/`Play Preview`/`Play Bundle`은 cursor에서 로컬 표현을 재생하며 Pause/Resume과 스크럽을 지원한다. 정식 Publish generation은 임시 실행으로 바뀌지 않는다. 낙사 허용 비행만 바닥 이탈 시 FALLING/DEAD로 진행하며 Client에는 별도 피해·낙사 판정을 만들지 않는다.

자연 완료된 audition의 영구 추적은 계속 유지한다. 남아 있는 같은 epoch의 `Stop`은 sequencer와 F1에서 제출할 수 있으며 기존 Server 소유권 검증 후 잔여 카드를 정리한다. 자연 완료 자체를 Stop으로 바꾸거나 Client가 카드를 임의 삭제하지 않는다.

본이 지정된 `ENTER_AREA`, `OBJECT_OVERLAP`, `OBJECT_CONTACT` Collider는 같은 installed WModel clip·preScale·socket·occurrence TRS를 bake한 track을 게시한다. 피해 collider의 bone을 읽지 않고 boss root로 대체하지 않는다. MAP 고정 경고 범위와 WEAPON 본 추적 범위는 서로 다른 anchor로 유지한다.


### 탈것 presentation lifetime와 원본 스킬 제어

VehicleCatalog formatVersion 4는 optional ambientEffectCues(MOUNT_END), mountEffectCues(NATURAL), mountSoundEvent, dismountSoundEvent를 Character mount commit에서 소비한다. skill의 optional shakeCues/directionalLightCues/materialVectorCues는 기존 clipIndex와 Server action clock을 사용한다. Client presentation만 소유하며 gameplay 이동·판정은 변경하지 않는다. Light는 로컬 플레이어의 일시 scene-relative 배율이고 매 프레임 원래 RenderingProfile을 복원한 뒤 적용한다. material vector는 실제 clone의 named source parameter를 재구성하며 종료하면 원래 상수로 복원한다. CModel source material mutation은 copy-on-write로 다른 clone·prototype을 보존한다. 새 optional 배열은 generator와 Workbench의 최신 subtree 저장에서 보존한다.


### Effect owner presentation 제어

Effect 저작 문서의 optional ownerControls는 재질·로컬 방향광·일시 가시성을 저장한다. Effect Tool과 component assembly 변환은 같은 배열과 stable control ID를 보존하고, 제어만 있는 문서도 기존 Effect playback clock으로 재생한다. 범용 저장·원복 규칙은 [렌더링·이펙트 복원 V2](../GB/렌더링이펙트복원V2.md)의 OwnerControls 항목을 따른다.

`CEffectObject`는 weak Character owner와 effect occurrence token, 제품 action-start identity를 전달한다. `CCharacter`는 활성 key sample만 적용하며 취소·숨김·실패·owner 변경·종료 때 해당 token을 해제한다. 재질은 제어 전 실제 CModel 값을 복원하고 일시 visibility flag는 기존 stance/장비 상태와 분리한다. 서로 다른 occurrence나 사용자 재질 변경을 전체 Clear로 지우지 않는다. UI는 이 기존 경계로만 제어를 제출하며 Shared/Server gameplay state나 판정 권위를 추가하지 않는다.

### Ctrl 핑 표시 입력

`CPlayerController`는 Ctrl 새 press부터 다음 물리 좌클릭 한 번을 로컬 핑으로 소비한다. Ctrl 해제·다른 키/우클릭·UI/focus/capture 차단은 대기를 취소한다. 대기 과녁은 실제 캐릭터 머리를 따라가고, 핑은 기존 피킹 XZ와 Character navigation 높이에 3초 표시한다. 기존 `CClickMoveEffect -> CEffectPresentationService`의 준비·수명 경계를 재사용한다. 이 표시에는 이동·공격·MAZE command나 party broadcast가 없으며 Ctrl+Z/X/C Esther 명령은 기존 typed sink를 사용한다.


### 쿠크 카드비·관문 음악 소비 계약

SHOWTIME_PLAYER_TARGETS의 random volley는 fixed/tracking template 없이 단독으로 사용할 수 있다. 기존4개 random 필드는 함께 저장하며 optional `randomAnchorKind`는 `BOSS_SPAWN`(기본값) 또는 현재 `BOSS`, `randomScaleMin/Max`는 기본1과 finite[.01,10]의정렬된범위를 사용한다. Action Workbench가 저장·표시하고 projector→Gameplay publisher→Server가 동일 값으로 소비한다. Server가 navigation에서 확정한 occurrence scale은 Shared protocol103의 S2C_COMBAT_OBJECT_SPAWNED에 불변값으로 포함되며 Client 재시도·late join과 타격 primitive에 동일하게 적용한다. UI가 scale·위치를 독립 추첨하지 않는다.

유한 random volley에는 MAP SOUND occurrence를 함께 포함할 수 있다. 최소 한 개의 MAP EFFECT가 위치 기준을 소유하며 SOUND만 있는 세트, BOSS-follow SOUND 및 looping targeted SOUND는 거부한다. Sound도 같은 content-addressed visual ID와 Server birth clock에 속하므로 각 투하에서 한 번 재생하고 늦은 입장에서는 이미 지난 음원 구간을 다시 시작하지 않는다. 기존 SoundCueCatalog의 variant는 해당 CombatObject ID/spawn tick/occurrence로 고정되며, 원본 Wwise avoid-repeat 메모리 전체를 재구현한 계약은 아니다.

`CARD_RAIN_SOLDIERS`는 typed trigger이며 optional `soldierCounts`는 CLUB·HEART·DIAMOND 순서의 정수3개(각0..32, 합계1..64, 기본1/1/1), `spawnRadiusMinM/MaxM`는 정렬된 finite0..100m(기본3..6m)를 받는다. Box Detail에서 편집하며 publisher의 `PATTERNCARDRAINSOLDIERS` supplemental row를 Server가 소비한다. Server는 보스 기준 반경의 navigation 위치를 전부 확보한 뒤 MonsterCatalog의 세 archetype을 Spawn_Monster로 생성하며 maze 진행 상태에는 등록하지 않는다. MonsterProfiles의 전투 수치와 기존 MonsterBrain·navigation으로 플레이어를 추적·공격한다. 같은 profile을 쓰는 미로 target은 생성 직후 maze에 등록하고 generic Brain에서 제외해 미로가 위치와 접촉을 계속 소유한다. 카드비 병정은 패턴 종료와30초 이후에도 유지되며, 병정 사망·owner 사망/제거 때 정리된다.

쿠크 Level의 단일 BGM owner가 Ready Terrace·GATE1/2/3·Mario1~4·Card Maze·Bingo를 승인된 player/raid 상태에서 선택한다. 시퀀스와컷씬/카메라 재생 중에는 BGM을 중지하고 같은 state의 반복 snapshot은 음악을 재시작하지 않는다. 원본 intro/loop 구간은 WAV smpl metadata를 소비한다. cue sound는 기존 pattern presentation 경로를 사용한다.


### 플레이어 피격 표현과 공중 착지

Protocol 110의 `PLAYER_SNAPSHOT.isKnockbackAirborne`는 Server ballistic 피격의 공중 상태이며 KNOCKDOWN 이외에는 false다. 기존 Server knockback integrator가 착지를 확정하면 같은 action occurrence에서 false로 바뀐다. Client는 공중 넘어짐 자세를 유지한 뒤 그 edge에서 착지·누운 자세로 진행하며, 지연 locomotion이나 stance 갱신으로 피격 clip을 덮어쓰지 않는다. 실제 push에는 이동 후 최소 1초 회복 자세를 두고 더 긴 authored downMs를 보존한다. push-only 표현은 기존 Collider 재접촉/반복 타격 저항을 바꾸지 않으며 공중 기상은 거절한다. G 이동·teleport·일반 TRIGGER_MOVE는 이 피격 경로를 사용하지 않는다. 이전 protocol의 Client/Server와는 연결되지 않으므로 양쪽을 함께 빌드한다.

### Guardian 변신·Monster 공격·Gate3 오라 입력

GuardianKnight Z는 identity 충전량에 관계없이 Server에 변신을 요청한다. 실제 stance commit에서 identity와 Ember orb를 채우며 기존 3초 공통 cooldown과 15초 지속을 유지한다. Client가 stance를 직접 교체하지 않는다.

GuardianKnight 일반 S `49220 / 스피닝 플레임`은 `PlayerSkillTargeting.json`의 `GROUND_POINT`로 차원술사 T와 같은 S 조준 → LMB 확정 경로를 사용한다. Server가 4.5m 사거리와 navigation을 다시 검증하며 지점 표시는 현재 피해 반경 1.3m와 같은 지름 2.6m다. HUMAN stance, stable skill ID, 피해·쿨타임·Ember 계약은 유지한다. 사용자가 지정한 원본 `49290` 용머리/원형 이펙트는 `49220.source.49290` Effect ID와 skillbinding의 두 clipOccurrenceId로 provenance를 보존한다. `skill_target` snapshot cue가 승인 지점에 재생되고 내부 원본 model/bone anchor는 그대로 사용한다. 두 body/FX clip의 presentation playRate는 2.888889/1.333333으로 기존 피해 300ms와 action 2000ms에 맞춘 프로젝트 조정값이다. DRAGON S `49230`은 이 교체 대상에 포함하지 않는다.

발탄 일반·루가루, 쿠크 시작 4종·카드미로 병정의 공격은 MonsterProfiles의 attackPushRangeM/attackPushMs/attackKnockdown/attackDownMs=0/0/false/0으로 피해만 적용한다. Valtan/Kouku boss push와 몬스터 자신의 hitKnockbackScale은 유지한다. MonsterBrain은 공용 Apply_WorldToPlayer 한 번만 호출한다.

Gate3 진입 오라는 원본 Prop300010 사각형의 회전과 실측 크기를 사용한다. Client는 replicated player 위치와 Server tick으로 10초 연속 체류를 표시한 뒤 기존 IPlayerCommandSink::Request_GateProgressPropose(ENTER_GATE3)를 한 번 제출한다. 이탈·사망·phase 변경은 취소하고 파티 동의 및 최종 이동은 기존 Server 입장 계약을 유지한다. WAIT_ENTRY의 이동만 준비 데크 안에서 허용하고 스킬은 계속 차단한다.


### 고대의 바다 비행 입력과 presentation

vehicle9523 고대의 바다는 `CPlayerController -> IPlayerCommandSink -> C2S_MOVE`의
typed flight intent와 Server snapshot phase/startTick/duration을 사용한다(Shared protocol105). 동일 protocol에는 F1 에스더 지정 소환 command도 포함되므로 Client/Server를 함께 갱신한다.
E는 이륙/착륙 전환, WASD는 비행 이동, Space/Ctrl hold는 상승/하강이며 서버가 고도와
XZ navigation/collision을 확정한다. WASD 조합을 유지하는 동안 카메라 기준을 고정해
용 회전과 카메라 추종이 서로 입력을 되먹이지 않게 한다. UI/free-camera/focus 전환은
입력을 중지하고 키 해제 뒤 재입력을 요구한다. 좌클릭 drag 공전은 presentation 입력이다.

Valtan/Kouku의 탑승 예외는9523만 허용한다. takeoff/loop/landing 표현 구간은
`Data/Actors/VehicleCatalog.json`의 E skill98523 `flightWindow`가 소유하고 실제 WModel
clip 길이와 함께 Vehicle publisher가 Server phase duration으로 변환한다. Action Workbench
Save Flight Logic은 해당 subtree만 최신 저장본에 병합하며 다른 effect/sound draft를 버리지 않는다.
`Data/Vehicles/VehicleProfiles.json`의 `flight`는 hoverHeight/maximumHeight/speed/verticalSpeed 물리 입력을 소유한다.
기존 Server authority와 CModel animation/IK, follow camera를 사용하며 두 번째 이동 runtime은 없다.


### 쿠크 Collider의 수명·이동·접촉 계약

Collider Box Detail은 `1. Lifetime / 2. Motion / shape / 3. Trigger / 4. Logic` 추가 설정을 선택한다. Start/Lifetime, position/rotation/scale, anchor/bone과 종료 위치·크기는 항상 표시한다.
Pattern 이름·부모 분류는 Collider 선택 시 표시하지 않는다. Shape 변경은 선택 박스의 resource를
분리하므로 다른 박스의 모양을 바꾸지 않는다. `CYLINDER`는 `radiusM`과
`halfExtents[1]`의 반높이를 가진 수직 원통이다. `CIRCLE`의 기존 평면 범위와 구분한다.

occurrence의 `colliderMotion=LINEAR`는 `positionOffset`/`scale`에서
`colliderEndPositionOffset`/`colliderEndScale`까지 `durationMs` 동안 선형 보간한다.
생략 또는 `STATIC`은 기존 정적 동작이다. 원형 LINEAR는 시작·끝 각각 X/Z scale이 같아야 한다.
`Keep bottom fixed while growing`은 높이 증가의 절반만 중심 Y를 이동시킨다. 단순 상승은
시작·끝 크기를 같게 두고 종료 Y를 높인다. BOSS의 `Follow anchor`를 끄면
`BOSS_START`가 Logic 창 시작 위치·방향을 고정한다. 플레이어를 향하는 보스 회전은 기존
BOSS_TRACK_TARGET/회전 Logic으로 먼저 설정하고, Collider는 그 순간의 방향을 보존한다.
본 Collider는 기존 Follow=true 조건을 유지한다. 서버 판정은 원통과 이동 Collider의 수직 범위와
플레이어 몸체 높이를 검사하며, 자체 선형 이동 원통·박스는 고정 tick 사이도 sweep한다.

`ENTER_AREA.repeatIntervalMs`와 `AREA_OVERLAP` Duration의 같은 필드는 플레이어별 접촉 반복 간격이며0은 기존 정책이다. 양수34..600000ms는 접촉한 첫 tick부터 간격마다 Success를 실행한다.
단발, 재진입, 넉백 종료 후 반복, 일정 간격 반복 중 하나를 선택한다. 마지막 정책은
접촉 중 수명 종료 전까지만 Success를 반복한다. 결과는 기존 `MAX_HP_PERCENT_DAMAGE`
등의 Result 슬롯을 사용한다. 저장은 저작 JSON, Publish는 런타임 snapshot, Server 재시작이나
재생 승인에 따른 catalog 갱신은 각각 별도 단계다.

Logic Box Detail은 연결된 Success/Fail/Timeout Result의 typed 수치를 편집한다. 같은 stable Logic ID를 공유하는 창은 같은 값을 소비하며 표시 이름에 피해·넉백 수치를 고정하지 않는다. BOSS/WORLD 본 Collider는 실제 모델의 전체 bone basis에 local XYZ TRS를 먼저 합성한 뒤 최종 XZ 중심·yaw를 얻는다. damageable WORLD의 `ownerWorldOccurrenceId`는 body 사망·취소·만료와 contact ledger의 수명을 묶고, 명시적 전체 수명 광기의 `authoredMadness`만 기존 aura를 대체한다.

빙고의 `BINGO_COMPLETED_LINES` Duration은 매 세 번째 폭탄의 실제 폭발·tile 갱신 사건에서 빨간 가로·세로 완성 줄을 한 번 판정한다. `threshold`는1..10(기본3)이며 대각선은 제외한다. Success의 `PLAYER_INVULNERABILITY` Result는 양수 `durationMs`(1..600000) 동안 생존자를 보호한다. 현재 저작값은30000ms다. 후속 `BINGO_DETONATION`은 성공이면 보스13줄 피해, 실패이면 보호막·개인 무적을 우회하는 encounter wipe다. 보드·폭탄은 encounter가 유지하고, 이동→첫 클립→메두사→블랙홀13초의 작은 Parent는 일반 반복 Flow에 삽입한다. 폭탄의 표식6초+대기2초+fuse4초는 Parent 애니메이션 길이와 독립이다.

빙고 전투 묶음은 `patternFlows`의 일반 entry/loop와 `bingoSpecialPatternId`의 특수 Parent를 함께 저장한다. 이 참조는 제품 BINGO에 필수이며 같은 gate·encounter·boss와 유일한 폭발을 가진 유효 Parent만 게시한다. `RAIDBINGOSPECIAL` supplemental 행을 Server gate definition에 고정하고 이름 검색이나 전체 패턴 추론으로 선택하지 않는다. 빙고 페이즈 진입 때 encounter 시계를 시작하며 매 세 번째 머리 표식에 현재 일반 occurrence를 정리하고 특수 Parent를 실행한다. 완료 뒤 중단했던 일반 entry를 처음부터 재생한다. 보드·폭탄 시계와 raid owner는 유지하며 동시에 두 패턴이 보스를 제어하지 않는다. 기존 다중 actor 동시 재생 Bundle 계약은 유지한다.

같은 Server 접촉 경로의 추가 입력 계약은 다음과 같다.

- Result `FIXED_DAMAGE`는 정수 `damageAmount`(1~1,000,000,000 HP)를 받으며 `percent`와 `durationMs`는0이다. 기존 안전존·피해 및 밀림 경로를 함께 사용한다.
- Attack template의 optional `riseHeightM`/`pushMs`는 생략·0/0이면 기존 반응이다. 양수 높이(최대100m)는100~5000ms 비행 시간과 짝을 이루며 기존 TIMED/CONTACT 피해에 수평0의 상승·하강을 연결한다. `PATTERNATTACKHIT`는 기존25열과 높이·시간을 덧붙인27열을 모두 받는다.
- `PURSUIT_PROJECTILES.cardSymbols`는 `visualIds`와 같은 개수의 HEART/SPADE/CLUB/DIAMOND를 순서대로 지정한다. 각 카드와 같은 문양인 플레이어는 그 카드의 접촉 피해에서 제외하며 색상은 비교하지 않는다. 생략하면 기존 접촉 규칙을 유지한다.
- `BOSS_CURRENT` bone track은 `OBJECT_CONTACT`와 플레이어 `ENTER_AREA`에 허용한다. 두 소비자 모두 정확한 Trigger 시작·수명·양 끝 key와 identity baseline, 고정 회전·크기 검증을 요구한다.
- GATE1·GATE3는 `pushCanLeaveArena`와 관계없이 지지면 경계를 막아 낙사를 금지한다. GATE2·BINGO는 지지면을 이탈한 뒤 최초 지지 높이보다5m 아래로 내려가면 Server가 사망을 확정한다. 공중 재피격은 기준 높이를 바꾸지 않으며, 부활은 현재 관문의 검증된 시작 위치로 복귀한다.

카드 비로 생성한 병정은 생성 Pattern 종료와30초 제한으로 제거하지 않는다. 기존 Server
몬스터 AI가 플레이어 추적·공격을 계속하며 자기 사망·소환자 소멸·방 정리에서 제거한다.
반복 소환의 방 상한은48마리다. 뿅망치는 타격 구간의 Collider를 실제 BODY/WEAPON bone에
연결하고, 플레이어 타격은 ENTER_AREA, 카드 반응은 기존 OBJECT_CONTACT와 Result를 사용한다.

Collider 상세의 Duplicate는 현재 상세값을 Apply한 뒤 같은 시각에 복제한다. 연결 Logic 창과
그 창을 공유하는 Collider도 기존 소유 연결대로 함께 복제하며 복제본만 선택한다. Ctrl/Shift로
동일 BOSS 기준 Collider 2개 이상을 선택하면 Set Group/Ungroup과 동일 시각 그룹 복제를
사용할 수 있다. 시작·종료 geometry와 타이밍은 유지하고 그룹 ID·occurrence 연결은 독립된다.


Collider의 `anchorPresentationOccurrenceId`는 같은 Pattern의 fixed BOSS Effect 시작 프레임을
참조한다. Server는 `captureStartMs`를 30Hz 올림 틱으로 평가해 고정 basis를 저장하고,
Client는 Effect와 Collider에 같은 프레임을 사용한다. 그룹의 Edit member/Back to Group은
group ID를 보존한다. 같은 Logic 창을 공유하는 행은 Once 이력과 Damage/Horizontal 값을 공유한다.

Protocol 106의 Debug draft audition은 최대16MiB를 48KiB chunk로 받고 순서·만료·world·checksum을
검증한 뒤 기존 catalog parser로 현재 non-Kouku 수치와 합친 후보를 검증한다. 승인된 hash와
run epoch에 해당하는 Client 메모리 presentation·animation bindings만 활성화한다. 다른 클라이언트에
임시 presentation JSON을 배포하는 기능은 없으며 해당 실행본을 모르는 클라이언트는 표현을 거부한다.
정식 F1/Complete Play는 기존 게시 Product 계약을 유지한다. Client·Server를 함께 갱신한다.

Effect occurrence의 optional `effectSourceStartMs`는 원본 재생 시작 위치이며 기본값은0이다.
앞 edge trim은 source-in·시작·수명을 함께 변경하고 body 이동은 시작만 바꾼다. 상세의
Source In은 박스 시작을 유지한다. Fit/Loop는 source-in 뒤의 남은 구간을 사용하고
attachment는 Pattern 시계, explicit fade는 박스 시계를 유지한다. Sound는 같은 kind의
박스2개 이상을 영구 그룹으로 저장하며 Effect+Sound의 임시 혼합 선택은 동일 delta로 이동한다.

순수 `ENTER_AREA` Trigger의 optional `colliderDamageContactRole`은 `DAMAGE` 또는
`KNOCKBACK`이다. Collider Apply는 역할·반복 설정이 맞는 정의를 재사용하며, 수평 밀림이나
상승이 있으면 KNOCKBACK을 선택한다. 표시 이름으로 잡기 Trigger를 재사용하지 않는다.
실제 hold·기믹 Result가 연결된 window는 자동 재연결 대상이 아니다.

Retail damage override의 optional `bossHealthBarDamage`는 ACTIVE 스킬 한 cast의
보스 HP 피해 총량을 체력 줄 수로 지정한다. 기본값0은 기존 계산이며 양수는 최대 HP와
최대 줄 수로 계산해 다단 타격에 분배한다. 이 보스 피해에는 공격 배율·편차·치명타·방어력을
재적용하지 않지만 적중·무적·실드 판정은 유지한다. Publish와 Server 재시작 후 적용된다.

### F1 몸통 콜라이더와 일반 이동 목적지

Debug/Release 공통 `Load KoukuSaydon Inventory`는 플레이어와 보스 몸통 표시를 켠다.
`Player / Boss Body Colliders`에서 다시 끌 수 있으며, 기존 process-global visibility가
현재 객체와 이후 Server snapshot으로 생성되는 객체에 적용된다. `CCollider`의 기존
Component → GameInstance → Renderer 경로를 공유하며 Client 표시가 Server 충돌 판정을
변경하지 않는다. Release Engine의 Component/Bounding virtual 및 collider layout도 공통화되므로
Engine·SDK·Client를 같은 변경으로 빌드한다.

일반 우클릭 목적지가 현재 접촉한 동적 몸통 안이면 Server는 그 몸통 경계에서 이동을
완료한다. 목적지가 몸통 너머에 있거나 아직 접촉하지 않았거나 다른 층이면 기존 경로·접선
이동을 유지한다. editor picking, G 이동과 teleport는 이 도착 처리의 대상이 아니다.
