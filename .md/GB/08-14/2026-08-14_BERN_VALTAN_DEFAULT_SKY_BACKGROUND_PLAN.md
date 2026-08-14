# Bern / Valtan 기본 하늘 배경 연결 계획

## 목표

- 기존 `CMapAssetObject -> CModel -> CMaterial` 배경 렌더 경로만 사용한다.
- Bern의 `MAP_EDDEDF2CF6A1_SKY_MIRROR_SM` 한 건을 Valtan과 같은 `Sky/None`으로 분류한다.
- placement `12120733164161297609`의 ID, transform, scale, visibility는 변경하지 않는다.
- Bern shard 재생성 시 render profile이 `Opaque/Back`으로 돌아가지 않게 stable assetId sidecar를 builder 입력으로 만든다.
- Valtan은 기존 자산, placement, load scope, resource admission을 검증하며 복제하지 않는다.

## 변경 단위

- `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json`
- `Tools/LevelPlacementExtractor/build_bern_castle_shards.py`
- `Tools/LevelPlacementExtractor/test_build_bern_castle_shards.py`
- builder가 재생성한 Bern BASE catalog와 receipt
- `Publish-MapAuthoring.ps1`이 갱신한 Bern runtime BASE catalog
- Client 프로젝트의 `96.DataFiles/Maps` sidecar 등록

## 완료 조건

- Bern/Valtan source와 runtime에서 기본 `Sky` 행이 각 1개다.
- Bern placement 행은 source/imported/runtime가 완전히 같다.
- 두 Area의 WModel과 기본 하늘 texture가 존재하고 converter info가 성공한다.
- shard fixture, 잘못된 assetId rollback, publisher failure rollback이 통과한다.
- Client Debug/Release가 빌드된다.
- 최종 화면 판정은 사용자가 수행하며 그 전에는 visual PASS로 기록하지 않는다.
