# 2026-08-15 에스더 게이지·실리안 소환 런타임 PLAN

작성자: JS · 2026-08-15 · branch `feature/esther-summon-runtime`

같은 날짜 `ESTHER_SILIAN_SUMMON_EXTRACTION_*`의 후속. 추출·쿠킹된 `NPC_59030`
(실리안, 클립 `npc_idle_battle_1`/`npc_att_battle_7_01`)을 발탄 아레나에서 실제
소환 연출로 실행하는 수직 슬라이스다.

## 1. 계약 (사용자 결정 2026-08-15)

- 에스더 게이지는 **room 단위 공유 상태**다. 0~1000, VALTAN_ARENA에서만 존재.
  테스트용으로 초당 200씩 자동 충전(가득 5초). 파티원 전원이 같은 값을 본다.
- 사용 조건: 게이지 가득일 때만. 원작의 파티장 제한은 테스트라 두지 않는다.
  사용 시 전량 소모.
- 입력: `Ctrl+Z`(슬롯1 실리안) / `Ctrl+X`(슬롯2 웨이) / `Ctrl+C`(슬롯3 바훈투르).
  이번 슬라이스는 슬롯1만 소환을 실행하고 2·3은 Server가 UNSUPPORTED로 거부한다
  (웨이·바훈투르 에셋 추출 후 같은 계약으로 활성화).
- 소환 위치는 시전자 위치, Look은 시전 시점 마우스 월드 방향.
- 실리안 타임라인(Server 소유, 시간은 authored 클립 실측 기준):
  `esther.appear`(idle 0.8s) → `esther.strike`(대검 일격 3.2s ≒ 94f/30fps) →
  `esther.leave`(idle로 Y +3.5/s 상승 1.5s) → despawn. 데미지 판정은 이번 범위 밖.
- Ctrl이 눌린 프레임의 Z/X/C는 에스더 전용이다. Warlord의 Z/X 태세 전환과
  물리 키가 겹치므로 Ctrl 동시 입력 시 일반 슬롯 제출을 막는다.

## 2. 프로토콜·데이터

- `NETWORK_PROTOCOL_VERSION` 19 → 20.
- 신규 `C2S_USE_ESTHER_SKILL { iClientSequence, iSlotIndex(u8 1..3), fAimX, fAimZ }`.
- `S2C_WORLD_SNAPSHOT` += `iEstherGauge(u32)`, `iEstherGaugeMaximum(u32)`.
  maximum 0 = 이 world에 에스더 없음(HUD 미표시). Bern/Training/CS는 0.
- 소환체는 기존 `S2C_WORLD_ENTITY_SPAWNED`(kind NPC, archetype `NPC_59030`,
  fCollisionRadius 0)와 `WORLD_ENTITY_SNAPSHOT`(strActionId가 위 esther.* 단계,
  eAction IDLE/PATTERN_ACTIVE)을 그대로 탄다. 신규 entity 메시지 없음.
- Server는 클립 이름을 모른다. `NpcCatalog.json` entry에 optional
  `"actionClips": { "esther.strike": "npc_att_battle_7_01" }`를 추가하고
  Client `CActorCatalog`가 파싱한다. appear/leave는 idleClip 사용.
- 게이지 수치(최대 1000, 충전 200/s, appear/strike/leave ms)는 테스트 상수로
  Server `CEstherSkillSystem`에 둔다. 원작 충전 메커니즘 확정 시
  `Data/Balance` 정본·publisher 계약으로 승격한다(RESULT에 경계 기록).

## 3. 변경 파일

| 파일 | 내용 |
|---|---|
| Shared/PacketType.h | `C2S_USE_ESTHER_SKILL` enum + Is_Known + version 20 |
| Shared/PacketMessages.h/.cpp | struct + Write/Read, SNAPSHOT 두 필드 직렬화 |
| Server/EstherSkillSystem.h/.cpp (신규) | 게이지 상태·충전·소모 판정, 소환 타임라인 상수, 슬롯 roster |
| Server/RoomCommand.h | `USE_ESTHER_SKILL` + payload |
| Server/ServerApp.cpp | frame dispatch 분기 |
| Server/GameRoom.h/.cpp | `Handle_UseEstherSkill`, tick 충전, 소환 entity 생성·타임라인 갱신·despawn, snapshot에 게이지 기입 |
| Server/ServerGameplayContractTests.cpp | 충전/가득 전 거부/소환+소모/타임라인 despawn/비발탄 거부/슬롯 2·3 거부 |
| Client/PlayerCommandSink.h | `Request_EstherSkill(slot, aimX, aimZ)` |
| Client/NetworkPlayerCommandSink.h/.cpp, NetworkManager.h/.cpp | 전송 구현 |
| Client/PlayerController.h/.cpp | Ctrl+Z/X/C edge → Request_EstherSkill(마우스 aim), Ctrl 프레임의 일반 Z/X/C 차단 |
| Client/ActorCatalog.h/.cpp | `actionClips` optional 파싱 |
| Client/ClientReplication.h/.cpp | NPC entity의 strActionId → actionClips/idleClip 클립 전환 |
| Client/CombatHUDViewModel.h/.cpp, HUDRuntimeView.cpp | 에스더 게이지 표시(max 0이면 숨김) |
| Data/Actors/NpcCatalog.json | NPC_59030 entry에 actionClips 추가 |
| Tools/NetworkProtocolHarness | v20 round-trip + invalid(slot 0/4, non-finite aim, seq 0) |

신규 파일 등록: `Server.vcxproj`/`.filters`에 EstherSkillSystem 2건.

소환 NPC 모델 admission: Valtan Level은 NPC placement가 없어 `NPC_59030`이
선로드되지 않는다. Character Select 동적 monster/Valtan spawn이 쓰는 기존 lazy
admission 경로를 실측해 같은 경로로 태우고, 두 번째 loader를 만들지 않는다.

## 4. 검증

1. Shared+NetworkProtocolHarness 빌드·실행 → 신규 왕복 PASS.
2. Server 빌드 → `Server.exe --contract-test` PASS (신규 계약 포함).
3. Client 빌드 → 로컬 Server+Client로 발탄 진입 → HUD 게이지 충전 표시,
   가득 전 Ctrl+Z 무반응, 가득 후 Ctrl+Z: 시전자 위치에 실리안 등장 →
   일격 → 상승 소멸, 게이지 0 리셋. 파티 2클라에서 게이지 동일 표시(사용자).
4. `git diff --check`.
