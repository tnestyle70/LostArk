# NPC placement별 idle 클립 저장 — PLAN

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool`

목표: MapTool에서 NPC placement마다 idle 클립을 골라 저장하면, 에디터 미리보기와
제품 Bern(Server snapshot 경로) 양쪽에서 그 클립이 재생된다. Server는 클립을 모른다 —
placement 식별자만 snapshot에 실어 나르고, 클립 매핑은 Client 전용 published 문서다.

## 0. 실측 (2026-08-12)

- `S2C_WORLD_ENTITY_SPAWNED`(PacketMessages.h:139)에 placementId 없음.
  Writer/Reader는 PacketMessages.cpp:450/487, `Is_Valid_StableId(id, allowEmpty)` 사용.
- `NETWORK_PROTOCOL_VERSION = 15` (PacketType.h:8).
- Server `SERVER_WORLD_ENTITY.strPlacementId` 이미 존재(ServerWorldEntity.h:28).
  전송 지점 `CGameRoom::Send_WorldEntitySpawned`(GameRoom.cpp:918-937) — 복사만 추가.
- harness roundtrip: NetworkProtocolHarness.cpp `Test_WorldEntitySpawnedRoundTrip`(545).
- publisher: actor placement(=playerSpawn/npc/boss)는 `Assert-ExactProperties` 7필드
  고정(Publish-WorldGameplay.ps1:532-534). bootstrap row는 commonFields만(572-573).
  출력 트랜잭션은 608~ staging→promote→rollback 구조, 대상은
  `Server/Bin/DataFiles/World/<WORLD>.worldbootstrap`(+spawngroups).
- Client/Bin/DataFiles에는 World 폴더 없음(Effect/Map/Navigation/Rendering).
- 클라이언트 스폰 소비: ClientReplication.cpp:409~ NPC 분기, `desc.pIdleClip =
  actor->idleClip.c_str()`.
- CNpc: 모르는 클립 이름은 첫 클립 fallback (presentation 격리, Npc.h 주석 계약).

## 1. 계약 설계

1. **Shared v16**: `S2C_WORLD_ENTITY_SPAWNED` += `std::string strPlacementId`
   (`Is_Valid_StableId(..., true)` — 동적 스폰은 빈 값 허용). strEncounterId 다음에
   기록. `NETWORK_PROTOCOL_VERSION` 15→16. harness roundtrip에 필드 추가 +
   `"../npc_1"` 거부 케이스.
2. **Server**: `Send_WorldEntitySpawned`에 `message.strPlacementId = entity.strPlacementId;`.
3. **Authoring v5**: `Gameplay.world.json` formatVersion 4→5. npc placement만
   `idleClip`(null 또는 비어있지 않은 string ≤64) 8번째 필드. 다른 kind는 이 필드 금지.
   4개 Area 문서 모두 version bump(Bern만 npc 보유).
   - `CWorldGameplayDocument`: `WORLD_GAMEPLAY_PLACEMENT` += `std::string npcIdleClip;`
     Load/Save/Is_Valid를 v5로.
   - publisher: npc kind는 8필드 Assert, 나머지는 7필드. bootstrap row 불변(클립 미출력).
4. **Client published 문서(신규)**: publisher가 world마다
   `Client/Bin/DataFiles/World/<WORLD>.npcpresentation.json` 생성(트랜잭션 promotions에
   포함, 빈 entries 허용):
   `{"schema":"lostark.npc-placement-presentation","formatVersion":1,"worldId":"BERN",
     "entries":[{"placementId":"...","idleClip":"..."}]}`
   idleClip이 null인 npc는 entries에서 제외.
5. **Client 서비스(신규 파일)**: `CNpcPlacementPresentationService`
   (Client/Public+Private, vcxproj/filters 등록 필요) —
   `Begin_LevelLoad(level)`, `Load(level, worldId)` (parse→validate→stage→commit,
   실패 시 빈 상태 유지 + 상태 문자열), `Find_IdleClip(level, placementId)` →
   `const std::string*`. Loader의 Bern/Valtan/CharacterSelect/Training ready 경로에서 Load.
6. **ClientReplication**: NPC 분기에서
   `pOverride = CNpcPlacementPresentationService::Find_IdleClip(level, spawned.strPlacementId)`
   → 있으면 desc.pIdleClip 교체.
7. **MapTool**: NPC 인스펙터 Preview Clip 아래 `Use As Placement Idle` 버튼 —
   staged.npcIdleClip = 현재 미리보기 클립, edited 커밋 경로 재사용. `Clear` 버튼으로
   catalog 기본 복귀. `Stage_WorldNpcPreviews`는 placement.npcIdleClip 우선.

## 2. 검증

1. Shared+harness 빌드, `NetworkProtocolHarness` failures 0.
2. Server 빌드, `Server.exe --contract-test` failures 0.
3. publisher 실행 → 4개 npcpresentation.json 생성 + bootstrap 회귀 없음.
4. Client 빌드. MapTool에서 슈미트 배치 → 클립 저장 → Save → JSON 확인 → Reload 후
   미리보기가 저장 클립으로 뜨는지.
5. publish 후 Server 재시작 + 로컬 루프백 Bern 진입 → 저장 클립 재생 확인(사용자).

## 3. 함정 메모

- protocol version은 C2S_HELLO/S2C_ENTER_ACCEPTED가 검사 — 양쪽 재빌드 필수.
- harness의 despawn 페이로드 byte-exact 테스트는 spawn과 무관(변경 없음).
- 새 Client 파일 2개는 vcxproj `ClCompile/ClInclude`와 filters 등록까지가 완료 조건.
- Bern 외 world의 npcpresentation은 빈 entries — 파일이 없으면 Load 실패로 fail-closed,
  publisher가 항상 4개 모두 생성하므로 정상 경로에선 존재.
