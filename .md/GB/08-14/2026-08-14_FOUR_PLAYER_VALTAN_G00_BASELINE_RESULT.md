# 2026-08-14 4인 발탄 G00 기준선 결과

## 0. 결론

G00의 코드 구현은 `codex/valtan-four-player-baseline` 분리 worktree에 반영됐다.
기존 Effect dirty worktree는 수정하지 않았다.

현재 blocking transport 위에서 다음 기준선이 실행형으로 닫혔다.

- Valtan enabled player spawn 정확히 네 개
- 네 Client 동시 accepted 및 각 Client의 4-player snapshot 관측
- 다섯 번째 Client의 typed `S2C_ENTER_REJECTED / ROOM_FULL`
- 한 Client disconnect 후 replacement accepted 및 4-player 재수렴
- 전원 disconnect 뒤 두 번째 세대 네 Client accepted
- 마지막 player 퇴장 시 trigger/spawn group/boss/monster/tick damage reset
- reset 중 server tick, PlayerId/NetEntityId allocator, pending transfer 보존
- Valtan 상단 boss name/phase/bar-count 제거, 한 개 HP bar와 `current / max`만 유지

이 결과는 IOCP 완료가 아니다. 느린 reader backpressure, bounded send queue, completion worker와 bounded drain은
전체 구현 계획의 G07에 남아 있다.

## 1. 실제 변경

### Shared

- protocol version `17 -> 18`
- `PACKET_TYPE::S2C_ENTER_REJECTED`
- `ENTER_WORLD_REJECTION_REASON::{ROOM_FULL, END}`
- `S2C_ENTER_REJECTED` transactional codec

### Server

- valid request와 full admission을 분리
- full일 때 map/allocator commit 전에 rejection send 후 close
- `Reset_CharacterSelectArenaWhenEmpty -> Reset_ReplayableArenaWhenEmpty`
- Character Select와 Valtan empty reset에서 trigger와 spawn runtime을 재초기화
- Valtan contract fixture로 full/slot reuse/pristine reset/ID 보존 검증

### Client

- NetworkManager의 connection-scoped pending rejection
- Lobby의 rejection-first 소비와 `Valtan raid is full (4/4)` 상태
- ImGui frame 안의 단일 boss HP bar
- Font Manager의 `current / max` 숫자

### Tooling

- Valtan enabled spawn exactly-four publisher gate
- protocol rejection 정상/오류/불변성 harness
- 새 `ValtanFourPlayerHarness` C++ project와 solution 등록
- owned Server process와 TCP 7777 cleanup을 관리하는 PowerShell runner
- 정본 build regression과 ProjectAudit contract 갱신

## 2. 자동 검증

### PASS

- `Publish-WorldGameplay.ps1 -Mode Validate`
  - BERN 7, VALTAN_ARENA 12, TRAINING_GROUND 4, CHARACTER_SELECT_ARENA 5
  - Valtan spawn groups 3, Character Select spawn groups 2
- Shared x64 Debug/Release build
- NetworkProtocolHarness x64 Debug/Release build 및 실행, failures 0
- Server x64 Debug/Release build
- `Server.exe --contract-test` Debug/Release, failures 0
- Client x64 Debug/Release build 및 link
- ValtanFourPlayerHarness x64 Debug/Release build
- live harness Debug/Release
  - initial four accepted and snapshot count 4
  - fifth `ROOM_FULL`
  - disconnect/replacement count 4
  - second-generation count 4
  - 종료 뒤 TCP 7777 listener 0
- 새 PowerShell script parser와 vcxproj/filters XML parse
- 관련 `git diff --check`

Release Client clean build에는 기존 파일의 C4819/C4244/C4267 및 DirectXTK PDB warning이 있었지만
exit code 0으로 `Client.exe` link와 runtime DLL deploy가 완료됐다.

### aggregate ProjectAudit

전체 123 checks를 완료했으며 aggregate 결과는 `passed=false`, 30 failures다. G00에서 갱신한
`world.monster-spawn-group-contract`와 `ui.combat-font-hud-contract`는 PASS했고, protocol 18,
typed rejection, Lobby consumer의 개별 정규식도 모두 true였다.

실패는 기존 Effect receipt/hash/denominator 불일치, Valtan debris resource 누락,
Data project visibility, rendering 계약 등 G00 밖의 저장소 상태가 대부분이다. 또한 Effect authority
lane이 현재 G00의 `Client/Private/Level_Lobby.cpp` 변경을 out-of-scope path로 보고하는 혼합-scope 실패가
포함됐다. `levels.character-select-contract`도 실패했지만 이 복합 check는 G00 전 기준부터 source의
protocol 17과 audit의 hard-coded protocol 15가 불일치하던 stale check이며, 현재 G00 typed rejection의
추가 조건은 전부 true다.

따라서 aggregate ProjectAudit 전체 PASS는 주장하지 않는다. G00 관련 실행형 protocol/Server/live
harness와 Debug/Release build 결과는 위 PASS 목록대로 독립적으로 확정한다.

## 3. 수동 검증

다음 항목은 실행하지 않았고 자동 PASS로 승격하지 않는다.

- Client 화면 실행
- Valtan 상단 HP bar 위치, 크기, 색, 숫자 가독성
- 네 실제 Client의 LAN 화면/조작 smoke
- slow reader, 10분 soak, 강제 disconnect/rejoin 반복

사용자는 `Server + Client` profile에서 Valtan을 열고 boss activation 뒤 상단에 한 개 bar와
`current / max` 숫자만 보이는지 직접 판정한다.

## 4. 남은 경계

- G01: Server-authoritative Character Select 세션 격리 audition
- G02: Create Character/nickname/Bern/direct Valtan/nameplate
- G03~G05: party authority, hover/RMB UI, ROOM/PARTY chat
- G06: animation HIT/effect impact/Server collider 공통 timeline
- G07: IOCP와 bounded backpressure
- G08: camera/sequence와 4인 통합 raid closure

## 5. Git 상태

- base: `origin/main` `9d379b5fbd2347ce864c905d1926ad1852f44dee`
- branch: `codex/valtan-four-player-baseline`
- stage/commit/push: 실행하지 않음
- 원본 Effect worktree: 수정하지 않음
