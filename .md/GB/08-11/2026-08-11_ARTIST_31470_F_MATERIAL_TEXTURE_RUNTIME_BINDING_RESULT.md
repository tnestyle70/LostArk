# 2026-08-11 Artist 31470 F Material Texture Runtime Binding Result

## 최종 판정

Material sampler policy 72행의 logical texture와 현재 Artist runtime Resources 결합을 별도 receipt로
닫았다. 현재 frozen runtime-cook/export receipt가 실제 runtime asset ID를 증명하는 범위는 68/72행,
고유 texture 기준 44/48개다. 나머지 4행은 exact-DDS 근거가 있지만 실제 Resources 배포/검증 receipt가
없으므로 `UNRESOLVED_RUNTIME_ASSET`으로 유지했다.

이번 변경은 runtime C++, Material program builder, renderer, 실제 `Client/Bin/Resources`를 수정하지
않는다. 따라서 complete runtime binding, renderer consumer, Product admission은 계속 false다.

## 생성된 정본

- receipt:
  `Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json`
- schema: `lostark.artist-31470-material-texture-runtime-binding-receipt`
- formatVersion: exact JSON integer 1
- generated receipt bytes: `314322`
- generated receipt raw SHA-256:
  `3a097a174df6b940989c7ce6c7b4e3b7798256d200cf73e25e694dc827e4346e`
- receipt self digest:
  `39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0`
- independent approval projection:
  `a88c1e11fc5d91807f989e29d5b41f098b6e03e1f8d96362683bd3bc37a51f30`

정확한 분모는 다음과 같다.

| 항목 | 결과 |
|---|---:|
| sampler policy binding | 72 |
| unique logical texture | 48 |
| resolved binding | 68 |
| unresolved binding | 4 |
| resolved unique texture | 44 |
| unresolved unique texture | 4 |
| material occurrence link | 83 |
| provisioning proposal | 4 |
| candidate exact asset observation | 57행 / 36개 unique resource |
| Product-ready binding | 0 |

각 binding은 `samplerPolicyRowId`, `recipeId`, `materialInputFieldId`, 전체
`logicalTexturePath`, ordered `materialOccurrenceIds`에서 identity를 다시 만들며 typed Material
contract의 recipe/input owner, section/index, occurrence reverse coverage와 대조한다. Runtime cook과 export도
basename이 아니라 전체 logical path 및 exported/cooked DDS의 size/SHA로 결합한다.

## 미해결 4개와 배포 제안 경계

미해결 logical texture는 정확히 다음 네 개다.

- `fx_tex_00.fx_a_decal_014`
- `fx_tex_00.fx_a_noise_011`
- `fx_tex_01.fx_c_atypical_016`
- `fx_tex_03.fx_e_ring_001_cl`

exact-DDS receipt가 소유한 기존 nested `fixtureAssetId`만
`RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1` 제안으로 기록했다.

- `Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds`
- `Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds`
- `Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds`
- `Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds`

이 경로들은 새 flat path 추측이 아니며 exact-DDS receipt의 기존 identity다. 그러나 파일을 배포하지
않았고 transactional deploy/verification receipt도 없으므로 runtimeAssetId는 null,
runtimeAssetAdmission/sourceExact/Product는 모두 false다.

## source evidence의 별도 경계

Pure validator는 아래 세 기본 외부 파일을 매 호출마다 strict parse하고 실제 byte count/raw SHA와
canonical JSON SHA를 승인 상수에 대조한다.

| 외부 authority | bytes | raw SHA-256 | canonical JSON SHA-256 |
|---|---:|---|---|
| runtime cook | 138276 | `2e3e5db345e3e845298c6e4dd3c65d931df86d2cde10f72be4ab236265b7e04e` | `a0f7c98bab2ea81bb4a337167d096085e0793edfde71bb4f385db7f4e0af4891` |
| resource export | 159864 | `9fe4ea42e7e3f60bdf8c1697211131331d8b270cddb974e11fa35d30d4279204` | `bc918f8cec1e2e0e10eba800563a3df4b14a38844d356a1531208f44c9e51032` |
| source-pack manifest | 270014 | `8ddce11f3cdd36efc4098b127da860b3e77e0f6916263412f1089cce3967d62d` | `ae0c344f2788bbf5185d25182ee4d29d4cae17200d0e61d0cfd22b1463521a42` |

수정 전에는 원본 runtime-cook 파일 identity를 receipt에 기록하면서 caller가 전달한
`fx_tex_00.fx_a_decal_013` 행의 `runtimeAssetId`만
`Effect/Artist/Textures/forged_resealed.dds`로 바꾸고 resource/binding/root를 재봉인한 receipt가
`require_approval=false` pure validation을 통과했다. 수정 후 외부 실제 파일 parse 결과가 유일한
authority이며 supplied object는 그 결과와 strict-equal이어야 하므로 같은 공격을 거부한다.

48개 고유 texture 중 45개는 pinned source-pack manifest의 logical/physical package와 raw UPK
size/SHA까지 결합된다. `wp_mn_lrcn_01`의 diffuse/normal/specular 3개는 Artist resource manifest와
runtime export/cook receipt에는 존재하지만 해당 source-pack manifest에는 package row가 없다. 이 세
행은 source package hash를 만들지 않고 `SOURCE_PACKAGE_NOT_IN_PINNED_SOURCE_PACK_MANIFEST`로 명시했다.
이는 현재 runtime asset identity를 무효화하지 않지만 source-package deep provenance가 45/48이라는
사실을 보존한다.

## 검증 결과

- Python unit mutation suite: 27/27 PASS
  - owner/field/occurrence/descriptor/SRV mutation reject
  - traversal, drive path, backslash, wrong prefix/suffix reject
  - runtime asset casefold collision 및 many-to-one reject
  - unresolved→resolved 승격과 proposal target/policy/sourceExact 승격 reject
  - row/root digest를 다시 봉인한 source package mutation도 independent approval에서 reject
  - approval을 끈 pure validator도 resource/cook unknown key, forged package SHA, package case mutation reject
  - external cook/export/source-pack coordinated mutation은 raw identity pin에서 reject
  - pure validator가 변조된 기본 runtime-cook 실제 bytes를 읽어 raw identity에서 reject
  - consumed runtime asset, resource/binding/root와 supplied cook object를 함께 재봉인한 공격도
    approved parsed object strict-equality에서 reject
  - duplicate JSON key, BOM, formatVersion `true`/`1.0`/`"1"` reject
  - LF/CRLF tracked-text hash equivalence PASS
- deterministic external rebuild `--check`: PASS
- validate-only shallow audit: PASS
- deep audit:
  - frozen cook/export/source-pack raw receipts PASS
  - source-pack UPK size/SHA PASS
  - 현재 runtime DDS 44/44 path case/size/SHA PASS
  - physical Artist texture casefold collision 0
- focused ProjectAudit shallow/deep: PASS

Focused 출력은 다음과 같다.

```text
PASS: Artist F 31470 Material texture runtime binding mode=shallow rows=68+4/72 unique=44+4/48 proposals=4 product=false
PASS: Artist F 31470 Material texture runtime binding mode=deep rows=68+4/72 unique=44+4/48 proposals=4 product=false
```

## 다음 단계

1. exact-DDS 4개를 위 nested asset ID에 transactionally 배포한다.
2. 배포 전 대상 부재와 배포 후 파일 byte count/SHA/path case를 한 receipt에 기록한다.
3. 그 receipt를 이 binding generator가 소비하도록 새 corrective에서 연결한 뒤 72/72 resolved로 승격한다.
4. runtime program builder/renderer는 72/72 texture asset binding과 sampler descriptor를 함께 소비한 뒤
   별도 runtime 눈 검증으로 진행한다.
