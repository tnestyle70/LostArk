# NPC 배치 툴 뼈대 — RESULT

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool` (미커밋)

PLAN: `2026-08-12_NPC_PLACEMENT_TOOL_PLAN.md`

## 1. 구현 완료

- `Data/Actors/NpcCatalog.json` — `NPC_BEDA / NPC_AYLARA / NPC_FORMAN / NPC_SCHMIDT` 4종
  supported 등록.
- `CActorCatalog::Get_Npcs()` — supported NPC 전체 열람 API 추가.
- `CNpcPresentationAssetService` — beda 하드코딩 제거, archetype별 모델 프로토타입 태그를
  catalog `modelAssetId` stem에서 파생(`Prototype_Component_Model_<stem>`, Beda 기존 태그와
  동일해 회귀 없음). ready 상태를 (level, archetype)로 관리, `Prototype_GameObject_Npc`는
  레벨당 1회만 등록. `Get_ModelPrototypeTag()` 공개.
- `CClientReplication` NPC 분기 — presentation id 하드코딩 제거, 태그를 서비스에서 조회.
- `CLoader::Ready_For_Development` Map Editor 진입 시 `Begin_LevelLoad(DEVELOPMENT)` 리셋 추가.
- `CMapTool` — NPC kind가 catalog 콤보로 4종 선택. NPC placement가 에디터에서 실제 모델로
  미리보기(`Layer_NpcPreviews`, per-placement 격리·실패 시 해당 건만 스킵). 배치/편집/삭제/
  Reload/Area 전환 시 동기화. 선택 NPC placement 인스펙터에 Preview Clip 콤보(모델 클립 열거,
  선택 즉시 미리보기 재생, 저장 계약은 catalog `idleClip` 유지).

새 파일 없음 → vcxproj/filters 변경 없음. Engine/Shared/Server 미변경.

## 2. 자동 검증 (실행함)

- Client x64 Debug 빌드 오류 0 (C4819 경고는 기존 파일들의 기존 경고).
- `NpcCatalog.json` JSON parse OK, 4 entry.
- `Publish-WorldGameplay.ps1` 전체 Area 검증 통과 (BERN 7 / VALTAN 12 / TRAINING 4 /
  CHARACTER_SELECT 5 placements).
- `git diff --check` 통과.

## 3. 수동 검증 (미실행 — 사용자 확인 필요)

1. Lobby `Test` → F1 Map Tool → Bern Area → World Gameplay 모드.
2. NPC kind 콤보에서 4종 확인, 각각 Arm → 클릭 배치 → 즉시 모델 표시.
3. placement 선택 → Preview Clip 변경 → 재생 확인.
4. 삭제/Reload/Area 전환 시 미리보기 정리 확인. Save 후 JSON 확인.
5. publish + Server 재시작 + 로컬 루프백으로 Bern 진입 → 배치 NPC가 제품 snapshot 경로로 표시.

## 4. 다음 단계 (이번 범위 제외)

- per-placement 애니메이션 저장(placement schema 확장 + snapshot→placement 매핑, Server는 클립 몰라야 함).
- 머리/몸통 분리 커스텀(cook 파이프라인 + CNpc 파츠 지원).
- 미리보기 표시/숨김 토글, NpcCatalog `idleClip`을 툴에서 저장하는 writer.
- NPC 추가 추출 범위 결정(2026-08-02 RESULT 6장 그대로).
