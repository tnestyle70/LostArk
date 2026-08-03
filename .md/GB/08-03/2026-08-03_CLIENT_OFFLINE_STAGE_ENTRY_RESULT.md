# Client Local / Multiplayer Stage Entry Result

> **SUPERSEDED — 2026-08-03**
> 이 문서는 당시 완료 상태의 기록이다. Local Preview, ClientLaunchOptions, LevelCatalog,
> Client smoke는 다음 변경에서 삭제할 대상이며 현재 목표 계약이 아니다. 새 정본은
> `2026-08-03_CHARACTER_SELECT_LEVEL_VERTICAL_SLICE_PLAN.md`다.

## 1. 완료 상태

Lobby에서 Bern, Valtan, Training 진입 모드를 `Local Preview`와 `Multiplayer`로 명시적으로
분리했다. 기본 선택은 Local이며, Client만 실행해도 선택 class의 실제 `CCharacter`가 canonical
world spawn에 생성되고 camera follow가 연결된다. Multiplayer는 입력한 IPv4/`localhost`와
port에만 연결하며 Server 승인을 받은 뒤 기존 server-authoritative replication으로 진입한다.

자동 LAN 서버 탐색은 구현하지 않았다. 현재 팀 공동 플레이 계약은 Server PC가 명시적으로
`0.0.0.0` 또는 사설 IPv4에 bind하고, Client가 Server PC의 사설 IPv4를 직접 입력하는 방식이다.

## 2. 구현된 계약

### Local Preview

- `ClientLaunchOptions`와 Lobby command에 entry mode, selected class, endpoint를 한 번 전달하는
  상태로 추가했다.
- Lobby ImGui에서 Local/Multiplayer와 Server IP/port를 선택한다.
- Local은 socket을 열지 않고 `Data/Worlds/<AreaId>/Gameplay.world.json`을
  `CWorldGameplayDocument`로 검증한다.
- enabled `playerSpawn`을 placement ID ordinal 순으로 정렬해 첫 spawn을 사용한다.
- online과 동일한 private Character 생성 helper를 사용하되 server player/net entity ID,
  packet, `CNetObjectRegistry`는 만들지 않는다.
- `CNetworkPlayerCommandSink`를 설치하지 않고 gameplay input, skill/damage/boss authority를
  활성화하지 않는다.
- 화면에는 `LOCAL PREVIEW — Server gameplay is disabled`와 spawn placement ID를 표시한다.

### Multiplayer와 LAN

- Client는 bounded nonblocking connect로 입력한 IPv4 또는 `localhost`와 port에 연결한다.
- Server 기본 bind는 `127.0.0.1`이며 `Server.exe --bind-address 0.0.0.0` 또는 특정 사설
  IPv4를 명시해야 다른 PC의 연결을 받는다.
- 연결 실패·enter 거부·timeout은 Lobby에 남고 Local로 자동 전환하지 않는다.
- 스테이지 진입 후 Server 연결이 끊기면 replication world를 정리하고
  `CSceneTransitionService`를 통해 Lobby로 복귀한다.
- TCP connect 뒤 `S2C_ENTER_ACCEPTED`가 오지 않으면 프레임 delta가 아닌 `steady_clock`
  절대 deadline으로 5초 뒤 연결을 닫고 Lobby에 남는다.
- 연결 종료는 receive socket을 먼저 `shutdown`/`closesocket`한 뒤 receive thread를 join한다.
  응답 없는 endpoint에서도 종료 join이 무한 대기하지 않는다.

### 실행 하네스

- offline smoke report version 2가 network 연결, command sink live count, class ID, placement ID,
  camera follow 상태를 기록한다.
- `Invoke-OfflineClientSmoke.ps1`은 7777 listener가 없는 상태에서 Bern/Valtan/Training을
  Debug/Release로 검증한다.
- `Invoke-NetworkEndpointSmoke.ps1`은 Server PID가 정확히 `0.0.0.0:7777`을 소유하는지,
  현재 LAN IPv4로 실제 접속되는지, Server 종료 후 Lobby 복귀와 sink 정리가 되는지,
  응답 없는 TCP endpoint에서 enter 승인 timeout과 연결 정리가 되는지 검증한다.
- ProjectAudit은 명시적 mode, canonical spawn, fake network ID 부재, project/filter 등록,
  LAN bind와 disconnect recovery 하네스를 고정한다.

## 3. 자동 검증 결과

### Debug

- canonical build/regression: Engine, UpdateLib, Shared, protocol harness, Server contract,
  Client build 성공.
- online smoke: Lobby, Bern, Valtan, 네 class Training 성공.
- development smoke: map, character, HDR, effect, UI 성공.
- offline smoke: Bern 17490 ms, Valtan 17420 ms, Training 13330 ms.
- LAN endpoint: `192.168.1.52:7777`, 17609 ms.
- disconnect recovery: 29439 ms, Lobby 복귀와 command sink 0개 확인.
- enter approval timeout: 5101 ms, Lobby 유지와 연결·command sink 정리 확인.

### Release

- canonical build/regression과 protocol/server contract 성공.
- online smoke: Lobby, Bern, Valtan, 네 class Training 성공.
- Release Development ImGui smoke는 제품 구성에서 의도적으로 제외되어 실행 PASS로 기록하지
  않았다.
- offline smoke: Bern 3701 ms, Valtan 3519 ms, Training 2714 ms.
- LAN endpoint: `192.168.1.52:7777`, 3718 ms.
- disconnect recovery: 29403 ms, Lobby 복귀와 command sink 0개 확인.
- enter approval timeout: 5100 ms, Lobby 유지와 연결·command sink 정리 확인.

공통으로 ProjectAudit 59개가 통과했다. 빌드에는 기존 C4819 코드페이지 경고와 DirectXTK
LNK4099 PDB 경고가 남아 있으나 새 오류는 없다.

## 4. 수정 중 발견해 닫은 회귀

기본 entry mode를 Local로 통일한 직후 smoke가 `dev.map.active` 같은 비-player Development
시나리오까지 offline Character 검증 대상으로 분류해 timeout이 발생했다. 검사 대상을 Bern,
Valtan, Training으로 제한하고 Debug development 전체 smoke를 다시 통과시켰다.

응답 없는 TCP endpoint의 approval timeout을 처음 추가했을 때 Client 종료가 receive thread join에서
멈췄다. member socket을 무효화한 뒤 local socket을 `shutdown`/`closesocket`하고 join하는 순서로
수정했다. 이어진 비평에서 main thread의 member socket 변경과 receive thread의 동시 읽기가 data
race라는 지적을 재현해, receive worker에는 연결 시점 socket을 값으로 전달하고 immutable local
handle만 `recv`하도록 보강했다. Debug/Release에서 약 5.1초의 bounded recovery를 다시 확인했다.

최종 비평 재검토에서는 worker의 member socket read가 제거된 것과 위 회귀 결과를 확인했고,
추가 P0/P1 없이 `PR APPROVE` 판정을 받았다.

## 5. 남은 경계

- 자동 LAN discovery, 인터넷 NAT traversal, relay, server browser는 이번 계약 밖이다.
- Windows Firewall inbound 허용은 Server PC 운영 설정이다.
- Local Preview는 표현 확인용이며 이동·스킬·피해·보스 판정을 추가하려면 별도의 local authority
  설계가 필요하다. 현재 server-authoritative gameplay를 우회하지 않는다.
