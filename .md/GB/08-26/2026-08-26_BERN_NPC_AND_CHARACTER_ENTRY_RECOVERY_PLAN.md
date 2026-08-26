# Bern NPC 및 플레이어 입장 복구 계획

## 목표

Lobby에서 Character Select를 거쳐 Bern에 들어갈 때 다음 계약을 한 변경 단위로 복구한다.

1. Server가 Bern의 enabled NPC 10개와 로컬 플레이어 spawn을 모두 전송한다.
2. Client는 로컬 플레이어 snapshot이 도착하기 전에도 authored playerSpawn을 기준으로 카메라를 둔다.
3. 입장 중 연결이 이미 끊긴 경우 Bern 화면에 고립되지 않고 기존 Lobby 복구 경로를 사용한다.
4. Map Tool의 유효한 NPC 행동 draft는 `Save Gameplay`에서 자동 적용되어 파일에 남는다.

## 조사된 원인

`CGameRoom::Build_WorldEntity`가 town NPC의 network `fCollisionRadius`를 플레이어 반경으로 채웠다.
Shared protocol v36은 town NPC의 combat collision radius가 정확히 0이어야 한다. 따라서 Bern 입장 시
`S2C_ENTER_ACCEPTED` 뒤 첫 NPC spawn 직렬화가 실패했고, session hard-close가 뒤의 NPC 및 로컬
플레이어 spawn frame까지 제거했다. Client는 이미 Bern 전환을 승인받았으므로 플레이어 없이 map 전체
bounds를 framing하여 행성처럼 멀리 보이는 카메라가 됐다.

## 구현 계약

### Server

- town NPC wire radius는 0으로 보낸다.
- NPC 물리 차단은 기존 `Refresh_PlayerBlockingBodies`가 만드는 별도 Server blocking body가 소유한다.
- Bern Join은 모든 기존 world entity spawn payload를 먼저 직렬화·검증한다.
- preflight가 모두 성공한 뒤에만 player/session 상태를 commit하고 `S2C_ENTER_ACCEPTED`를 전송한다.
- 실패 시 Client가 Bern으로 전환된 뒤 끊기는 부분 성공 상태를 만들지 않는다.

### Client

- Bern camera는 authoritative local spawn, authored enabled playerSpawn, map bounds 순서로 기준을 고른다.
- replication 초기화 시 이미 끊긴 socket 상태도 connection-loss pending으로 기록한다.

### Map Tool

- `Save Gameplay`는 batch ghost가 없고 NPC 행동 draft가 유효하면 기존 transactional apply 함수를 먼저
  실행하고 Gameplay 문서를 저장한다.
- invalid draft는 상세 오류를 유지하고 파일을 변경하지 않는다.
- 명시적 Apply/Revert와 Reload 보호 계약은 유지한다.

## 검증 계획

- `Publish-WorldGameplay.ps1 -Mode Validate`와 `-Mode Publish`
- Debug/Release Server build 및 `Server.exe --contract-test`
- Debug/Release Client build
- Debug/Release NetworkProtocolHarness build 및 실행
- Bern 16 placements, 10 NPC, disabled legacy trigger 계약 검사
- 모든 Bern world spawn preflight와 실제 fake-session Join 검사
- JSON parse, 수정 파일 hash 비교, scoped `git diff --check`
- 제품 화면은 사용자가 `Server + Client`로 Lobby → Character Select → Bern을 직접 확인한다.

## 작업 경계

Client를 에이전트가 직접 실행하거나 육안 PASS로 판정하지 않는다. 기존 dirty worktree의 무관한 변경은
되돌리거나 stage/commit하지 않는다.
