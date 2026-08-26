# Bern NPC 및 플레이어 입장 복구 결과

## 완료 상태

Bern 입장 중 NPC spawn 직렬화 실패가 전체 spawn queue를 지우던 문제를 복구했다. Server는 Bern의
enabled NPC와 로컬 플레이어를 모두 queue하며, Client는 플레이어 snapshot 대기 중에도 authored
playerSpawn 근처를 보여준다. Map Tool 저장은 유효한 미적용 행동 draft를 자동 적용한다.

## 실제 변경

- `Server/Private/GameRoom.cpp`, `Server/Public/GameRoom.h`
  - town NPC wire collision radius를 0으로 교정했다.
  - world spawn payload 생성 함수를 분리하고 Bern Join 전에 전체 preflight한다.
- `Server/Private/ServerGameplayContractTests.cpp`
  - Bern 모든 spawn 직렬화, NPC radius 계약, fake-session Join과 로컬 플레이어 spawn 회귀를 추가했다.
  - 현재 Bern revision 142의 16 placements, NPC 10개, disabled legacy trigger 계약으로 갱신했다.
- `Client/Private/Level_Bern.cpp`, `Client/Public/Level_Bern.h`
  - local snapshot이 없을 때 enabled authored playerSpawn을 camera 기준으로 사용한다.
- `Client/Private/ClientReplication.cpp`
  - level loading 중 이미 끊긴 연결을 첫 update에서 감지하도록 초기 상태를 교정했다.
- `Client/Private/MapTool.cpp`
  - `Save Gameplay`와 Save All이 유효한 NPC 행동 draft를 transactional auto-apply한다.
- `2026-08-26_NPC_BEHAVIOR_MAP_TOOL_USER_GUIDE.md`
  - Apply가 선택 단계이고 Save가 유효한 draft를 자동 적용한다는 현재 사용법으로 갱신했다.

## 데이터 상태

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`은 format v6, revision 142이며 placements 16개,
enabled NPC 10개를 포함한다. 다만 현재 디스크 원본의 NPC 10개는 모두 `behavior: null`,
`idleClip: null`이다. 이전 화면에서 편집한 loop draft는 저장 파일에 들어오지 않았으므로 원하는 행동은
Map Tool에서 한 번 다시 설정하고 `Save Gameplay`한 뒤 publish 및 Server 재시작이 필요하다.

## 자동 검증

- Debug Server build: PASS
- Debug Server contract test: `failures : 0`
- Debug Client build/link: PASS
- Debug NetworkProtocolHarness: `failures : 0`
- Release Server build: PASS
- Release Server contract test: `failures : 0`
- Release Client build/link: PASS
- Release NetworkProtocolHarness: `failures : 0`
- World gameplay Validate/Publish: BERN 16, VALTAN_ARENA 154, TRAINING_GROUND 4,
  CHARACTER_SELECT_ARENA 5 PASS

Release/Debug Client의 C4819 및 DirectXTK PDB 경고는 기존 경고이며 이번 빌드에는 compile/link error가
없었다.

## 수동 확인

자동 검증은 완료했다. Client 화면은 팀 규칙에 따라 실행하지 않았으므로 다음은 사용자 확인 대상이다.

1. `Server + Client` Debug profile을 `Ctrl+F5`로 실행한다.
2. Lobby → Character Select → Create Character → Bern으로 들어간다.
3. 캐릭터가 authored spawn 근처에 생성되고 배치 NPC 10개가 보이는지 확인한다.
4. NPC 행동을 다시 저작했다면 Save → Publish/Server build → Server 재시작 후 움직임을 확인한다.

기존 사용자 변경은 보존했고 별도 stage/commit은 하지 않았다.
