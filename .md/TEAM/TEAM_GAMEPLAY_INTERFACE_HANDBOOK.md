# LostArk 팀 게임플레이 인터페이스 사용서

작성일: 2026-08-03
정본 브랜치: `codex/baren-player-replication`

## 0. 세션 시작과 사용자 전용 화면 검증

모든 세션은 `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, 있으면
`.md/GB/gotchas.local.md`, `.md/TEAM/README.md`, 대응 PLAN/RESULT를 먼저 읽는다.
Artist F, Effect Tool, Character Select와 Client의 시각 결과는 사용자만 직접 조작하고 최종 visual fidelity를 판정한다.
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

2026-08-20 23:59 KST까지 팀 LAN 검증은 Server PC에서 `Framework.slnLaunch`의 `Server + Client` profile로 Server를 `0.0.0.0:7777`에 열고, 다른 PC는 Client project만 시작해 `192.168.200.103:7777`에 연결한다. `Tools/Network/TeamLanEndpoint.json`이 endpoint와 만료일 정본이다. 각 에이전트는 세션 시작 시 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행하고 출력된 `server-host` 또는 `client` 역할에 맞는 target을 안내하며, 실제 `Ctrl+F5` 시작과 UI 조작은 사용자가 수행한다.

### 1.1 서로 다른 장소에서 Server와 Client 연결

Server와 Client가 같은 PC, 같은 LAN, 서로 다른 네트워크 중 어디에 있는지 먼저 구분한다.

| 실행 위치 | Server `--bind-address` | Client `LOSTARK_SERVER_HOST` |
|---|---|---|
| 현재 팀 LAN 검증 | `0.0.0.0` | `192.168.200.103` |
| 같은 PC 격리 검증 | `127.0.0.1` 명시 | `127.0.0.1` 명시 |
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

스크립트는 현재 endpoint IPv4를 실제로 가진 PC를 `server-host`, 나머지를 `client`로 자동 판정한다.
`server-host`는 Server+Client profile을 시작하고, `client`는 불필요한 로컬 Server를 띄우지 않고
Client project만 시작한다. 자동 판정이 예상과 다르면 IP 어댑터 상태와 endpoint 정본을 먼저
교정하며 `-Role Server`로 주소 소유 검사를 우회할 수 없다.

현재 공유 기본값과 다른 endpoint를 검증할 때만 `TeamLanEndpoint.json`, Server/Client 기본값,
공유 debugger 설정, 문서와 audit를 같은 변경 단위로 교체한다. 한 PC의 `.vcxproj.user`만 바꿔
팀 계약을 갈라놓지 않는다.

```xml
<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.200.103</LocalDebuggerEnvironment>
```

`0.0.0.0`은 Server의 수신 주소일 뿐 Client 접속 주소로 사용하지 않는다. 현재 같은 LAN의 모든 Client와 Server PC의 Client는 `192.168.200.103`을 사용한다. 주소를 바꾸면 `Tools/Network/TeamLanEndpoint.json`, Server/Client 코드 기본값, 공유 debugger 설정과 이 사용서를 같은 변경 단위에서 갱신하고 `Sync-TeamLanEndpoint.ps1`, NetworkProtocolHarness, Server contract test로 검증한다.

현재 Server PC 주소가 실제 어댑터에 있는지는 다음 명령으로 확인한다.

```powershell
Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object { $_.AddressState -eq 'Preferred' -and $_.IPAddress -notlike '127.*' } |
    Select-Object InterfaceAlias, IPAddress
```

설정 후 Visual Studio는 `.vcxproj.user`를 메모리에 캐시할 수 있으므로 Server와 Client project를 Reload하거나 Visual Studio를 재시작한다. 그다음 Server PC와 Client PC에서 각각 확인한다.

`Sync-TeamLanEndpoint.ps1`은 `2026-08-20 23:59 KST`가 지나면 실패한다. 2026-08-21 이후 첫
세션은 `-AllowExpired`로 계속 쓰지 않고 새 endpoint 또는 loopback 복귀 계약을 먼저 정한다.

```powershell
# Server PC
Server\Bin\Debug\Server.exe --bind-address 0.0.0.0 --smoke-timeout-ms 500

# Client PC: VPN/LAN endpoint가 실제로 열렸는지 확인
Test-NetConnection <SERVER_REACHABLE_IPV4> -Port 7777
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
| 좌클릭 | `34010` | 평타 콤보. 누르고 있으면 100ms마다 재전송한다 |
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

현재 서버 충돌 계약은 walkable nav cell 경계다. 동적 capsule-vs-capsule, projectile, knockback obstacle collision은 아직 public 계약이 아니므로 Character collider에서 임의로 서버 판정을 대신하지 않는다. 추가할 때는 Server collision owner, shape ID, broad/narrow phase, snapshot correction, harness를 한 변경 단위로 닫는다.

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
- codec/publisher/runtime은 attachment mode, basis 값, transform composition order를 함께 validate하고
  unknown/non-finite 값은 stage 전에 거부한다.
- 위치뿐 아니라 orientation covariance, fixed-step snapshot 불변성, invalid follow fail-closed를 harness로
  검증한다.
- stable occurrence/material program의 근거가 부족하면 해당 occurrence만 draw 전 fail-closed하며,
  family 전체 fallback이나 white/opaque texture 대체로 열지 않는다.

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
| `Data/Balance/PlayerSkills.json` | slot, 이름, `skillKind`, cooldown, action/hit time, cost, 이동 거리, range, damage 참조, `effectId`, `comboStages` | Server skill, UI definition, Effect presentation |
| `Data/Balance/DamageProfiles.json` | attack power에 곱하는 damage rate percent | Server 판정, UI 예상 표시 |
| `Data/Balance/BossProfiles.json` | boss HP, engage range, speed, phase threshold | Server boss, UI 이름 |
| `Data/Encounters/Valtan/ValtanEncounter.json` | state/action/pattern timing/range/damage 참조 | Server Valtan brain |

UI 담당자는 JSON을 매 프레임 읽지 않는다. `CCombatHUDViewModel::Initialize_Definitions()`가 정의를 준비하고 `CClientReplication`이 snapshot마다 runtime 상태를 적용한다. UI 코드에서 packet, socket, Character, boss GameObject를 직접 조회하지 않는다.

Debug F1 `Balance Tool`은 다섯 class와 발탄을 선택해 stats/movement/skill/combo/pattern을 편집하고,
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

```text
CGameRoom::Tick
-> CValtanBrain::Update
-> nearest living player acquire
-> IDLE / CHASE / PATTERN_WINDUP / PATTERN_ACTIVE / PATTERN_RECOVERY / DEAD
-> damage exactly once in active window
-> S2C_WORLD_SNAPSHOT
-> CValtan presentation + CCombatHUDViewModel
```

`BossProfiles.json`은 boss 기본 수치, `ValtanEncounter.json`은 pattern timeline, `DamageProfiles.json`은 피해량을 소유한다. Client `CValtan`의 로컬 AI는 Development preview 외 제품 정답이 아니다.

플레이어 profile의 defense는 발탄 incoming damage에 실제로 사용된다. 원작 Server 공식이 client
payload에 없으므로 `raw * 100 / (100 + defense)`는 `PROJECT_TUNED` 중앙 계약이며
`CGameplayCatalog::Apply_Defense` 한 곳에서만 계산한다. boss defense/outgoing 감산은 아직 없다.

수업용 `CMonster`와 `astar/Monster`는 제거 대상 레거시다. 제품 일반 몬스터는 실제 4개 Valtan archetype을 `MonsterCatalog.json`과 `MonsterProfiles.json`에 등록하고, Area `SpawnGroups.world.json` → publisher → Server `CSpawnGroupRuntime/CMonsterBrain` → Shared world entity spawn/snapshot/despawn → Client catalog presentation 경로를 사용한다. 레거시 클래스를 이 계약에 다시 연결하지 않는다.

combat body와 공격 footprint는 `Shared/Public/Gameplay/CombatCollisionContract.h`의 pure XZ primitive로 평가한다. 플레이어 스킬은 target body radius, 일반 몬스터와 Lugaru는 player footprint, Valtan은 circle/ring/cone/forward-box/cross와 player footprint의 교차를 Server fixed tick에서 판정한다. HP·damage·counter·death는 기존 Data와 Server 순서를 유지한다. Client `CCollider`는 spawn packet의 Server radius를 표시하는 Debug mirror일 뿐 damage 판정이나 PhysX 권위가 아니다.

## 8. MapTool과 gameplay 저장

정적 visual 배치와 gameplay 배치는 분리한다.

- visual import definition: `Data/Maps/Imported/<AreaId>/`
- visual authoring: `Data/Maps/Authoring/<AreaId>/`
- gameplay authoring: `Data/Worlds/<AreaId>/Gameplay.world.json`
- monster wave authoring: `Data/Worlds/<AreaId>/SpawnGroups.world.json`
- navigation authoring: `Data/Navigation/<AreaId>.navsource/.navpaint/.navblockers` 또는 uniform `<AreaId>.navgrid.json`
- Client map/navigation 생성물: `Client/Bin/DataFiles/Map`, `Client/Bin/DataFiles/Navigation`
- Server world 생성물: `Server/Bin/DataFiles/World/*.worldbootstrap`, `*.spawngroupsbootstrap`

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

gameplay authoring은 formatVersion 4다. 제품 publisher/runtime는 현재 `playerSpawn`, `npc`, `boss`,
단일 `movePlayer`, `changeLevel`, `activateSpawnGroup`, `activateEncounter` action을 가진 `triggerBox`, 정적 `collisionBox`를 admission한다.
placement에는 stable placement ID, kind, encounter ID, position, yaw, enabled를 저장한다.
NPC/boss는 stable archetype ID를 소유하지만 `playerSpawn`의 `archetypeId`와 `encounterId`는 `null`이며 실제 class는
session/player selection이 소유한다. NetEntityId, pointer, Prototype tag, vector index, runtime HP/phase를
저장하지 않는다.

v4 `CWorldGameplayDocument`에서 `triggerBox`는 half extents, once 정책, typed event를 소유하고
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
`NPC_BEDA`는 `NpcCatalog.json` → Server world entity → Client replication → `CNpc` 경로로 표시한다.

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
않고, live event만 one-shot mesh debris/effect cue를 만든다. Effect Tool private type과 active document를
MapTool 또는 제품 runtime에 의존시키지 않는다. 상세 작업법은 `MAP_DESTRUCTION_PHYSX_HANDOFF.md`를
따른다.

Map/Encounter 담당자가 좌표를 수정하면 navigation publish가 활성 playerSpawn/boss 좌표의 walkable cell과 높이 오차를 검사한다. 생성된 Server bootstrap/navgrid를 직접 편집하지 않는다.

Area별 optional layer와 현재 Bern/Valtan/Training 데이터 보유 현황은 `AREA_DATA_LAYER_GUIDE.md`를 정본으로 사용한다.

## 9. 데이터 변경 절차

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
```

세 publisher는 parse → validate → stage → commit을 따른다. unknown field, schema/version 오류, 중복 stable ID, 잘못된 참조, non-finite 위치, navigation 밖 spawn을 정상값으로 보정해 숨기지 않고 실패시킨다.

새 스킬을 추가할 때는 다음을 함께 변경한다.

1. `PlayerSkills.json`과 참조 `DamageProfiles.json`. 비어 있지 않은 `effectId`는 Client의
   canonical source/진단 참조이므로 stable ID여야 하고 Server bootstrap에는 싣지 않는다.
   실제 clip별 Product Effect는 `skillbindings`가 소유한 clip의 `effectref=asset` animevent가
   가리키는 Authored Effect다. `effectId`가 비어 있다는 이유만으로 실제 Product cue를 추측하거나
   반대로 source/imported 문서를 제품 재생 대상으로 승격하지 않는다.
   다중 clip stage도 첫 clip의 재생률로 하나의 Effect 문서를 진행하지 않는다. 시각 요소가 있는
   각 clip이 clip-local Product cue를 소유하고 기존 Character의 `playMs`, `playRate`, loop와
   authoritative late-catch-up을 소비한다. Character gameplay 준비는 이 검증된 cue target 집합을
   catalog revision 단위로 transactional prewarm하며, 전투 Update의 Product Spawn은 prepared
   bundle만 붙인다. prepared miss에서 shader/model/DDS/vector-field load 또는 synchronous document
   stage를 수행하지 않는다.
2. Character presentation의 stable action/skill mapping
3. 필요한 Shared message/snapshot 확장
4. Server validation/action/damage 처리
5. `CCombatHUDViewModel` 소비 확인
6. protocol harness와 `Server.exe --contract-test`

## 10. Asset과 Git

`Client/Bin/Resources`는 `Fonts, Character, Deploy, Effect, Map, UI` 여섯 root만 허용한다. asset ID는 Resources 상대 경로이며 절대 경로, drive-qualified 경로, `..` 탈출을 금지한다.

runtime payload는 팀장이 `Client/Bin/Resources` 물리 폴더로 관리한다. AssetPacks lock, immutable manifest, ZIP hash, Snapshot/Publish/Hydrate/Verify를 팀 완료 조건으로 사용하지 않는다. 코드와 데이터에는 Resources 상대 asset ID만 저장하고 팀원별 절대 경로 하드코딩은 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ 팀장이 전달한 Resources 물리 폴더 확인
→ Debug 전체 회귀
→ 담당 public interface에서 작업 시작
```

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, 소비 데이터, project/filter 등록, harness, RESULT를 같은 검증 단위로 묶는다. build output, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, Resources payload를 stage하지 않는다.

## 11. 완료 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

자동화 순서는 Engine → UpdateLib → Shared/Protocol Harness → Server build/contract test → Client build → balance/world/navigation/effect/rendering publisher validate → 변경 domain의 실행형 harness이다. 실제 Level 흐름은 `Framework.slnLaunch`로 Server와 Client를 함께 실행해 검증한다.

최소 성공 증거:

- Protocol Harness `failures : 0`
- Server gameplay contract `failures : 0`
- Debug/Release Client와 Server 빌드 성공
- 실제 Server+Client에서 Lobby → Character Select 승인 진입 → class 연속 변경/스킬 → Lobby → Bern/Valtan 진입 확인
- 연결 실패 시 Lobby 유지와 제품 Level disconnect 후 Lobby 복귀 확인
- 변경 domain의 publisher Validate와 실행형 harness 성공
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
- world gameplay와 navigation 배치 정합성 검사
- Valtan Debug MapTool의 12-piece Mesh Emitter PhysX audition과 All/Emitter/Fragment Solo
- `dev.training.ground` 최소 Area, class-neutral player spawn, RCArena 10종 admission, 서버 navigation
- Lobby의 Lance Master/Gunslinger/Slayer/Artist/DimensionMaster/Warlord 여섯 선택 slot, Lobby 승인 Character Select visual map, Server-authoritative class 변경, 여섯 class Loader/Server profile과 runtime HUD. DimensionMaster는 combined body와 L/S/P/E 네 정적 기본 무기 파츠를 사용하며 runtime payload는 팀장 관리 Resources 물리 폴더를 사용한다.

별도 수직 슬라이스:

- 이동기·스탠스 전환과 skill별 weapon visibility/reattach
- `Data/UI` layout에서 `CUIObject` image widget을 생성하는 runtime factory
- 1280×720 reference 좌표 보정, draw-order 기반 2D UI picking과 input arbitration
- stable UI command binding과 Lobby/Scene/Gameplay typed command service 연결
- 추가 스킬
- party/raid admission과 roster
- 동적 collider, projectile, knockback/피격 판정
- 잡몹 및 추가 boss pattern
- Valtan destroyable publisher, Server 상태/동적 collision·navigation, Shared replication과 제품 debris/effect cue

이 항목들은 현재 인터페이스를 우회해 임시 구현하지 않는다.
