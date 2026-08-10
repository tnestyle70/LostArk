# 2026-08-11 Artist 31470 F Exact DDS Runtime Deployment Result

## 최종 판정

Material texture binding corrective authority가 제안한 exact-DDS 네 파일을 basename 추측이나 재인코딩 없이
실제 `C:/Users/user/Desktop/LostArk/Client/Bin/Resources`에 한 transaction으로 배포했다. 네 target은
transaction 직전 모두 부재했고, commit 뒤 exact path case, regular-file, DDS header, byte count와 raw
SHA-256을 4/4 다시 검증했다.

이 변경이 승인하는 것은 네 runtime asset의 물리 bytes뿐이다. Material source-exact claim, renderer texture
SRV consumer, R4, Product는 모두 false이며 blocker `R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE`를 유지한다.

## frozen 입력 authority

- original authority commit: `fda3b5637847f9205915ad25ff02215424024b88`
- authority tree: `2f00f00851ee93f498dd6c13d6a3055209d4d8c3`
- Material texture binding tracked-text SHA-256:
  `3a097a174df6b940989c7ce6c7b4e3b7798256d200cf73e25e694dc827e4346e`
- Material texture binding receipt self:
  `39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0`
- Material texture binding independent approval:
  `a88c1e11fc5d91807f989e29d5b41f098b6e03e1f8d96362683bd3bc37a51f30`
- exact-DDS receipt tracked-text SHA-256:
  `3b21d1ce5d9fa6575e0d289967584e8198aec42c99aa634929bf01e2b7a97824`

Corrective commit은 이 branch에 `-x` cherry-pick되어 local commit `da0b9d04`가 됐다. Receipt의
authority는 local cherry-pick SHA가 아니라 위 original commit/tree를 기록한다.

## 생성된 deployment receipt

- path:
  `Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-runtime-deployment.receipt.json`
- schema: `lostark.artist-31470-exact-dds-runtime-deployment-receipt`
- formatVersion: exact integer 1
- receipt self SHA-256:
  `de52ea2770129d254aa007a5a547ac5027977c809ace951bf6a87e8342b8c466`
- tracked/raw receipt SHA-256:
  `7fff1e38bce7a10b67d2b5a89c1f76f798409003b2802051f10dc177e745be18`
- independent approval projection:
  `419f6e2c403b0a39b49e64b2cb46b73ae48a2ed420fa0e355983da73a913d3dc`
- recovery manifest canonical self digest:
  `9058170aab950ce5ceff1cfffa96331d1cdea97ef24b0902cb9d5be0815c4c9a`
- recovery `rollback-manifest.json` raw file SHA-256:
  `382c845589627926a2d04fa34a263c9314f9f96f70ff305b84086835c1de2f33`

두 개의 서로 다른 temporary runtime root에 실제 transaction을 실행해 receipt bytes가 byte-for-byte 같고
각 root의 source/runtime/recovery deep 검증이 모두 PASS함을 먼저 확인했다. 그 뒤 같은 frozen tool로 실제
team-managed Resources transaction을 실행했으며 최종 receipt self도 isolated 결과와 같다.

독립 리뷰 corrective에서는 네 runtime asset 또는 recovery manifest를 다시 쓰지 않았다. 기존 receipt의
historical pre-state와 durable recovery identity를 검증한 뒤 receipt만 atomic refresh했고, refresh 전후 네 DDS의
size/raw SHA-256과 `rollback-manifest.json`의 2,010 bytes/raw SHA-256이 모두 동일함을 확인했다.

## 실제 배포 파일

| runtime asset ID | byte count | raw SHA-256 |
|---|---:|---|
| `Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds` | 65,664 | `c2ead8a025cea1f70c8b03f9a488748ebd616828695199539d4e5010e5dc24ef` |
| `Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds` | 16,512 | `c8d94d0750517d5654416e71bcd8666d79630c478c757d196e37ba056f87cc53` |
| `Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds` | 32,896 | `bbd2dc3ba79d24a5806af63d00dee302a04fb5c6e8be343275a63a9353981f43` |
| `Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds` | 65,664 | `3c8987c8bc4bda1d3fd0f4840124e4fc1ba2eb3899307df8fe5852c4a760738e` |

## recoverable backup

Durable recovery backup은 target `Resources` 밖에 보존했다.

```text
C:/Users/user/Desktop/LostArk/Client/Bin/Artist31470ExactDdsRecovery/
  artist-31470-exact-dds-70a55d1f943fd9f0d3d5/
    rollback-manifest.json
    files/
```

배포 전 네 target이 모두 부재했으므로 payload backup file은 0개이고 absent marker는 4개다. Recovery action은
네 deployed file을 제거하는 것이다. Manifest는 target asset ID와 pre-state, recovery action, row/root digest를
보존한다. 기존 target이 exact-equal인 재실행 사례에서는 네 파일 모두 이 외부 backup에 복사한 뒤 commit하며,
다른 bytes인 기존 target은 transaction 시작 전에 fail-closed한다.

## 자동 검증

- Python failure/mutation suite: 33/33 PASS
  - absent/exact-existing/mismatched pre-state
  - source/runtime path exact case와 source/runtime/backup casefold sibling collision
  - receipt output과 runtime target exact/case-only, recovery manifest, transaction stage namespace collision pre-write 거부
  - source/runtime symlink·junction·reparse 거부
  - corrupt source, partial stage copy, partial directory creation, backup copy 실패
  - second/third atomic move, post-verify, receipt move, post-receipt target/recovery mutation 실패 all-four rollback
  - canonical recovery manifest self와 raw serialized file SHA 분리, legacy receipt verify-only refresh
  - receipt runtime asset/source serial/authority/unknown-key coordinated reseal 거부
  - binding authority coordinated reseal, BOM, duplicate key 거부
- deterministic isolated deployment: 2회 receipt byte-for-byte 동일, source/runtime/recovery deep PASS
- actual deployment CLI: PASS, assets 4/4
- actual corrective receipt refresh: PASS; runtime/recovery verify-only, physical bytes unchanged
- deployment receipt shallow check: PASS
- deployment receipt deep source/runtime/recovery check: PASS
- focused ProjectAudit shallow/deep: PASS
- upstream Material texture binding focused shallow/deep: PASS
- PowerShell parse: PASS

이 lane은 Python/JSON/PowerShell과 physical runtime resource transaction만 포함하므로 C++ build와 build
lease를 사용하지 않았다. 전체 ProjectAudit은 99개 중 이 focused exact-DDS 항목을 포함한 91개가 PASS했고,
이 lane 밖 기존 baseline 8개가 FAIL했다: data-source visibility, G09 authoring/runtime 및 cross-document 계약,
Artist source contract, 미빌드 WModel geometry harness, WFX assembly, representative authored readiness,
four-class authored rollout의 Artist/31210 stage-count mismatch다. Client runtime 눈 검증은 이 receipt의 독립
review 뒤 상위 integration lane에서 실행한다.

## 남은 경계

1. 독립 reviewer가 frozen uncommitted snapshot의 validator, rollback, receipt와 실제 physical state를 검증한다.
2. reviewer PASS 뒤에만 commit/push한다.
3. Material binding corrective/runtime program이 이 deployment receipt를 exact input authority로 pin하고
   sampler 72/72 runtime asset binding으로 승격한다.
4. R4 texture SRV consumer와 renderer가 72/72를 소비한 뒤 실제 Artist F 눈 검증으로 진행한다.
