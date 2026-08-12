# NPC placement별 idle 클립 저장 — RESULT

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool`

PLAN: `2026-08-12_NPC_PLACEMENT_IDLECLIP_PLAN.md`

## 1. 구현 완료

- Shared v16: `S2C_WORLD_ENTITY_SPAWNED` += `strPlacementId`(빈 값 허용, stable ID 검증).
  `NETWORK_PROTOCOL_VERSION` 15→16.
- Server: `Send_WorldEntitySpawned`가 entity의 placementId를 복사(클립은 모름).
- Authoring v5: `Gameplay.world.json` npc placement에 `idleClip`(null 허용, ≤64).
  4개 Area 문서 이행(베른 beda entry `idleClip: null`). `CWorldGameplayDocument`
  Load/Save v5, `WORLD_GAMEPLAY_PLACEMENT.npcIdleClip`.
- Publisher: npc kind 8필드 검증 + 클립 이름 패턴(`^[A-Za-z0-9_~.-]{1,64}$`),
  `Client/Bin/DataFiles/World/<WORLD>.npcpresentation.json` 4종을 같은 트랜잭션으로
  생성(빈 entries 허용). Server bootstrap 형식 불변. `FailureAfterPromote` 상한 12.
- Client 신규 `CNpcPlacementPresentationService`(vcxproj/filters 등록):
  placementId→idleClip. Bern/Valtan/CharacterSelect/Training 로더에서 로드,
  문서 누락/손상은 격리(스폰 안 막음, 상태만 debug 출력).
- `CClientReplication` NPC 분기: snapshot placementId로 override 조회, 없으면
  catalog idleClip.
- MapTool: NPC 인스펙터 `Use As Placement Idle`/`Clear Placement Idle`.
  미리보기 스테이지는 placement 클립 우선.

## 2. 자동 검증 (실행함)

- `NetworkProtocolHarness` failures 0 (placementId roundtrip/거부/빈 값 허용 포함).
- `Server.exe --contract-test` 통과. Shared/Server/Client Debug 빌드 오류 0.
- `Publish-WorldGameplay.ps1 -Mode Publish` 성공 — bootstrap 회귀 없음 +
  npcpresentation 4종 생성 확인(BERN entries [] — 아직 저장된 클립 없음).
- `git diff --check` 통과.

## 3. 수동 검증 (사용자 확인 필요)

1. F1 Map Tool → Bern → NPC 배치 → Preview Clip 선택 → `Use As Placement Idle` →
   Save Gameplay → JSON에 `idleClip` 저장 확인.
2. `Publish-WorldGameplay.ps1 -Mode Publish` 재실행 → BERN.npcpresentation.json에
   entry 생성 확인 → Server 재시작 → Bern 진입 → 해당 NPC가 저장 클립 재생.

## 4. 남은 것 / 주의

- Server pre-build가 이 publisher를 실행하므로 팀원 빌드 시 Client DataFiles/World가
  자동 생성된다. 머지 후 protocol v16이라 구버전 Server/Client 혼용 불가.
- MapTool은 클립 존재를 보장하는 목록에서만 선택하므로 authoring은 안전. 손으로 JSON에
  없는 클립을 쓰면 CNpc가 첫 클립으로 격리 재생한다.
