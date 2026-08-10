# 2026-08-10 Artist 31470 F Geometry Resource Binding Result

## 결과

도화가 F의 WModel 1.1 carrier 7개를 범용 typed `GeometryBinding`으로 고정하고 팀장 관리
물리 Resources의 정확한 7개 파일에 transactional deployment했다.

- typed binding: 7 carrier, 7 submesh, 12,686 vertex, 18,936 index
- `COLOR_0` carrier: 2
- staged G02 expected tuple match: 7/7
- 물리 deployed candidate hash match: 7/7
- Debug/Release C++ decoded semantic match: 7/7
- cache identity collision: 0
- runtime `geometryPreScale` consumed: 0/7
- Product admission: false

이미지, 스크린샷, 육안 비교는 수행하지 않았다.

## tracked artifact

- `Data/Effects/Imported/Artist/Geometry/skill.31470.geometry-binding.json`
  - schema `lostark.effect-geometry-binding`, format 1
  - binding SHA-256
    `1dff0059706bdff9ed1433654035220da8cc8916fe94c0d4bc24b5330f57a004`
- `Data/Effects/Imported/Artist/Geometry/skill.31470.geometry-resource-binding.receipt.json`
  - schema `lostark.artist-31470-geometry-resource-binding-receipt`, format 1
  - receipt SHA-256
    `5b4331c33dd2ead13f69c9e3c58a392665c5ce71ce96835ac568d841a09e9156`

각 typed row의 cache identity는 타입 검증을 통과한 실제 asset ID, payload/provenance SHA,
JSON float preScale, exact integer channel/evidence mask, submesh channel digest와 bounds 전체에서
재도출한다. caller가 hash만 다시 봉인해도 G02 immutable golden과 다르면 validator가 거부한다.

G02 승인 commit은 이 branch의 graph ancestor가 아니다. 승인 commit
`2b3d7a6c410f963b2e47aa7999504c422fff7c32`과 tree-equivalent base
`513a2dde5ae317cab8fee18777397d887075e5c5`의 full tree SHA가 모두
`1b217af4a159e69c95daa4b71f4de86b2b8ded18`인지 확인하고, source manifest, cooker,
verifier, semantic golden 4개 blob도 현재 tree와 canonical byte equivalence를 확인한다.

외부 authoritative pin이 없는 export receipt, cook receipt, legacy converter 3개와 설치 source
package 7개는 총 10개의 `OBSERVED_UNVERIFIED` evidence로 남겼다. 이 값은 source-exact로
승격하지 않는다. 다만 receipt의 manifest/export/cook 행과 실제 glTF/buffer/legacy bytes를
row별로 다시 join하며 source glTF와 legacy WModel SHA는 G02 golden이 별도로 고정한다.

## 물리 배포와 backup

배포 root:
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`

| asset | deployed bytes | deployed SHA-256 |
|---|---:|---|
| `fm_v_wp_wsdm_base_01.wmodel` | 642,904 | `804a5bcfb2dd9abc445db1a514e249d1ca9916010937a013831e9d3a3cbbae18` |
| `fm_m_trail_002.wmodel` | 13,704 | `db1c9412d588174ad5f6330b8daf403c0ab615251bf0448a1b2b37bf4a30683d` |
| `fm_h_swing_03.wmodel` | 33,300 | `e2b9f3337904fceca31c84650724788f20b1eaf68e0420615692c87028a7434c` |
| `fm_h_swing_05.wmodel` | 27,264 | `9586b5d299b5e9dc7e5973cf8b4b7f31058413c334dc3f7ff2d8410ab7b0a8fd` |
| `fm_h_swing_01.wmodel` | 18,660 | `d30f3fdc0f6c365d7686b6bf442b65806d1ce2ddac7061fb74751e24e48b0ab4` |
| `fm_o_swing_02.wmodel` | 18,504 | `18ee0bd0315c3464c7ef1b64e67d5b172eb187fbb7909ec7c619669aa7e5d78e` |
| `fm_a_stone_001.wmodel` | 11,808 | `eb08b11e4631938f93b896d9ebf9e7f25d22492094dcf69de443080d5c111c54` |

legacy v1.0 backup은 다음 외부 root에 보존했다.

`C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/ARTIST/artist_31470_geometry_resource_backup_g02_2b3d7a6c`

- backup manifest raw SHA-256:
  `009b49188d776f40f8c8eb21e79b8d009f221ef1fc22d6a244d9ee6e3201f07d`
- legacy 7개 bytes/SHA는 기존 runtime cook receipt와 모두 일치
- deployment result raw SHA-256:
  `428d8742255d23bcece791a6e903111e3bd6ae8f57e74b3a10817732c86b6be9`

## 자동 검증

- Python GeometryBinding/transaction unit: shallow 17 PASS + external-root 3 skip,
  deep 20/20 PASS
  - exact JSON integer와 duplicate/BOM 거부
  - preScale JSON float-only, mask exact integer와 payload/provenance coordinated re-seal 거부
  - cache identity collision 거부
  - missing/changed/duplicate physical target와 case-only/case-fold alias preflight 거부
  - 실제 glTF+export/cook receipt coordinated reseal, legacy bytes+cook row reseal 거부
  - changed legacy physical revision과 checked receipt 동시 reseal dry-run 거부
  - `OBSERVED_UNVERIFIED` external identity의 `SOURCE_EXACT` laundering 거부
  - replace 3번째 fault와 post-validation fault에서 7/7 rollback, residue 0
- Engine + WModelGeometryContractHarness x64 Debug/Release build: PASS
- GeometryBinding deep audit Debug/Release: PASS
  - source+legacy에서 deterministic candidate 재생성
  - checked-in binding/receipt `--check`
  - physical candidate C++ decode 7/7
  - corrupt physical byte와 missing physical target 거부
- shallow ProjectAudit registration: PASS
- JSON parse와 `git diff --check`: PASS

전체 `Invoke-ProjectAudit.ps1`에서는 이 작업의
`effect.artist-31470-geometry-resource-binding` 검사가 PASS하여 실패 목록에 없었다. 전체
exit code는 이 브랜치의 기존 범위 밖 검사 12건 때문에 1이었으며, 해당 실패를 이 G에서
완료로 기록하지 않는다.

기존 build가 출력한 FXC deprecation, legacy encoding/conversion, missing third-party PDB warning은
남아 있으나 compile/link exit는 Debug/Release 모두 0이었다.

## 남은 경계

이 G는 payload provision과 expected tuple만 닫았다. G08이 이 binding을 `CModel` load/cache
identity와 vertex/bounds staging에 연결하여 `geometryPreScale=0.01`을 정확히 한 번 소비해야 한다.
그 전까지 Mesh StartSize의 기존 숨은 `x0.01` 상쇄는 제거되지 않았고 Product admission은 false다.
