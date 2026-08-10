# 2026-08-10 Artist 31470 F Geometry Resource Binding Plan

## 목표

G02에서 독립 승인된 WModel 1.1 후보 7개를 도화가 F의 범용 typed
`GeometryBinding`으로 고정하고, 팀장이 관리하는 실제 Resources의 정확한 7개 carrier만
transactional deployment한다.

이 G는 geometry 실행이나 Product admission을 열지 않는다. 출력은 G08이 소비할
`{assetId, payloadSha256, provenanceSha256, geometryPreScale, channels, bounds,
cacheIdentity}` expected tuple과 그 생성 근거다.

## 소유 범위

- `Tools/ModelAssetConverter/build_artist_31470_geometry_resource_binding.py`
- 전용 Python mutation/rollback test
- `Data/Effects/Imported/Artist/Geometry`의 typed binding과 evidence receipt
- 전용 ProjectAudit와 공통 ProjectAudit 등록
- 이 PLAN/RESULT
- 사용자 관리 물리 root의 명시된 WModel 7개와 외부 legacy backup

Source/Material 계약과 Effect runtime C++은 수정하지 않는다. 이미지·스크린샷·육안 비교는
검증에 사용하지 않는다.

## 입력과 승인 경계

- 승인된 G02 commit: `2b3d7a6c410f963b2e47aa7999504c422fff7c32`
- immutable semantic golden:
  `Tools/WModelGeometryContractHarness/Fixtures/artist_31470_v11_expected.json`
- legacy runtime WModel, source glTF/bin, export/cook receipt, installed source package,
  기존 `ModelAssetConverter.exe`
- 물리 배포 root:
  `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`

G02 golden과 candidate 전체 SHA, metadata payload/provenance SHA, decoded channels와 bounds가
하나라도 다르면 binding을 만들지 않는다.

## 출력 계약

`lostark.effect-geometry-binding`은 runtime/compiler가 읽을 수 있는 범용 schema다. 각 row는
stable binding ID, Resources-relative carrier asset ID, payload/provenance SHA, float32 0.01,
channel/evidence mask, submesh별 channel digest와 bounds, 모든 필드를 포함한 cache identity를
소유한다.

Artist 전용 evidence receipt는 source/legacy/candidate identity, G02/tool/receipt identity,
typed binding hash와 denominator를 소유한다. tracked JSON은 물리 Resources의 현재 상태를
Product 근거로 과장하지 않는다.

## 배포 transaction

1. 물리 7개가 receipt-pinned legacy SHA인지 모두 preflight한다.
2. 외부의 비어 있는 backup root에 7개 원본과 manifest를 먼저 기록하고 다시 hash한다.
3. 같은 directory의 임시 파일을 flush/hash한 뒤 `os.replace`로 정확한 7개만 교체한다.
4. 7개 모두 Python parser와 C++ decoder로 expected tuple을 재검증한다.
5. write 또는 post-validation 실패 시 7개 전체를 원본 bytes로 복원하고 residue 0을 확인한다.

missing target, changed legacy hash, duplicate target/cache identity, 중간 교체 실패,
post-validation 실패를 자동 test한다.

## 완료 조건

- typed binding 7/7, cache identity collision 0
- staged candidate와 G02 expected tuple 7/7
- 물리 WModel 1.1 배포와 Debug/Release C++ decode 7/7
- legacy WModel 7개 persistent backup과 rollback test PASS
- shallow/deep ProjectAudit, JSON parse, `git diff --check` PASS
- runtime preScale consumer 0, Product admission false 유지
