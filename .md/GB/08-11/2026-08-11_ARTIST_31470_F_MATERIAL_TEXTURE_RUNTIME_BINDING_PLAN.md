# 2026-08-11 Artist 31470 F Material Texture Runtime Binding Plan

## 목표와 현재 경계

Material reconstructed policy의 sampler 72행을 실제 Resources 상대 runtime asset ID에 전부
결합한다. 기존 68행은 frozen runtime-cook/export receipt의 full logical path 결합을 그대로
사용한다. 나머지 4행은 독립 PASS를 받은 exact-DDS deployment receipt와 실제 배포 파일을
함께 검증한 뒤 reconstructed deployment 결합으로만 승인한다.

이번 변경이 닫는 범위는 다음과 같다.

- sampler policy 72/72와 unique logical texture 48/48의 runtime asset 결합
- runtime-cook 기반 68행, 44개 unique texture의 기존 증거 보존
- exact-DDS deployment 기반 4행, 4개 unique texture의 별도 typed 증거
- material occurrence link 83개의 owner/path/descriptor/SRV 역결합
- 배포 proposal 4개의 완료·배포 lineage 보존

이번 변경은 Material source fidelity, renderer SRV 소비, R4 또는 Product를 승인하지 않는다.
네 행의 fidelity는 `RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT`이며
`SOURCE_EXACT`로 승격할 수 없다. 전체 receipt의 `sourceExact`, renderer admission, Product도
계속 false다.

## 고정 deployment 권위

Material validator는 아래 artifact를 기본 tracked 경로에서 직접 읽는다. 호출자가 넘긴 JSON
object는 이 파일을 대체하지 못하며, 실제 parse 결과와 strict-equal일 때만 보조 입력으로
허용한다.

| 항목 | 고정값 |
|---|---|
| original authority commit | `01b8b8a8bace09a3576f116771daf4859aa485a3` |
| authority tree | `60a229b3460e2738ef8cf4f9199a1beb8cdeb6d4` |
| receipt path | `Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-runtime-deployment.receipt.json` |
| receipt Git blob | `5d1e3766e90dc3d816b9948d85991ff57a207dca` |
| tracked-text SHA-256 | `7fff1e38bce7a10b67d2b5a89c1f76f798409003b2802051f10dc177e745be18` |
| receipt self SHA-256 | `de52ea2770129d254aa007a5a547ac5027977c809ace951bf6a87e8342b8c466` |
| independent approval projection | `419f6e2c403b0a39b49e64b2cb46b73ae48a2ed420fa0e355983da73a913d3dc` |
| implementation projection | `0eda2556731377fbee03aad19ccb44c90c2092bb61eedf0ff7178540f7f9c78b` |

deployment receipt의 네 행은 logical path, proposal ID, texture resource ID, runtime asset ID,
exact-DDS export/payload, deployed file size/SHA, exact path case, regular-file/symlink-free,
post-verify 결과를 모두 결합한다. Material validator는 receipt 검증과 별도로
`Client/Bin/Resources` 아래 실제 네 DDS를 다시 열어 path case, alias/collision, symlink,
byte count와 raw SHA-256을 검사한다.

Material policy, typed contract, runtime oracle, source-value acquisition, resource manifest,
exact-DDS receipt, native-v14 candidate의 일곱 tracked semantic 입력도 caller object를 권위로
사용하지 않는다. validator는 항상 기본 tracked bytes를 strict parse하고 approval의 tracked/self
pin을 검증한다. supplied object가 있으면 기본 parse 결과와 strict-equal이어야 하며, candidate
observation은 pinned candidate에서만 계산한다.

## v2 receipt 계약

`formatVersion`을 exact JSON integer 2로 올린다. 기존 의미와 구분하기 위해 contract ID도
`ARTIST_31470_MATERIAL_TEXTURE_RUNTIME_BINDING_V2`로 올린다.

| 구분 | 행 | unique texture | status/basis |
|---|---:|---:|---|
| runtime-cook 결합 | 68 | 44 | `RESOLVED_EXACT_RUNTIME_COOK_RECEIPT` |
| exact-DDS deployment 결합 | 4 | 4 | `RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT` |
| 전체 | 72 | 48 | runtime asset binding ready |

네 deployment resource는 `deploymentEvidence`에 artifact commit/tree/blob, receipt tracked/self,
approval/implementation projection, deployment row ID/SHA, runtime ID, size/SHA와 post-verify flag를
소유한다. 네 provisioning proposal은 삭제하지 않고
`COMPLETED_POST_VERIFIED_EXACT_DDS_DEPLOYMENT` lineage로 보존한다.

admission은 다음처럼 분리한다.

- `bindingReceipt.ready=true`, rowCount 72
- `resolvedRuntimeAssets.ready=true`, rowCount 72
- `completeRuntimeBinding.ready=true`
- `rendererConsumer.ready=false`, blocker `R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE`
- `product=false`

## 변경 파일

| 파일 | 역할 |
|---|---|
| `Tools/LevelPlacementExtractor/build_artist_31470_material_texture_runtime_binding.py` | 68 cook + 4 deployment typed join, 실제 tracked receipt/Resources 검증, v2 receipt 생성·검증 |
| `Tools/LevelPlacementExtractor/artist_31470_material_texture_runtime_binding_approval.py` | deployment artifact pins와 최종 v2 projection 승인 |
| `Tools/LevelPlacementExtractor/test_build_artist_31470_material_texture_runtime_binding.py` | 기존 mutation 27개와 deployment 공격 회귀 |
| `Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json` | 생성된 72/72 v2 정본 |
| `Tools/ProjectAudit/Test-Artist31470MaterialTextureRuntimeBinding.ps1` | shallow/deep 분모와 fail-closed 경계 검사 |
| `.md/GB/08-11/2026-08-11_ARTIST_31470_F_MATERIAL_TEXTURE_RUNTIME_BINDING_RESULT.md` | 실제 검증 결과와 다음 R4 경계 기록 |

exact-DDS deployment commit의 generator/receipt/approval/test/audit/PLAN/RESULT는 `-x` cherry-pick으로
같은 브랜치에 포함하되 그 파일을 이번 Material receipt의 새 evidence authority로 재작성하지 않는다.

## 실패와 rollback

다음 중 하나라도 발생하면 receipt 생성과 validation을 실패시킨다.

- deployment receipt commit/tree/blob/tracked/self/approval/implementation identity 불일치
- deployment 4행 누락, 순서/owner/logical/runtime ID 교환, 가짜 완료 상태
- deployment runtime ID 또는 deployed-file ID가 exact-DDS fixture ID와 exact-case 불일치
- exact-DDS row와 deployment row의 path/size/SHA 불일치
- 실제 Resources 파일 누락, casefold alias, case mismatch, symlink, size/SHA 불일치
- supplied/resealed deployment object가 기본 tracked parse 결과와 불일치
- 일곱 tracked supplied object가 기본 tracked parse 결과와 불일치
- 72 policy, 48 logical, 83 occurrence, 68+4/44+4 분모 불일치
- runtime asset traversal, absolute path, backslash, wrong prefix/suffix, many-to-one alias
- sourceExact, renderer, R4, Product의 무근거 승격

generator는 완성된 candidate를 검증한 뒤에만 atomic replace하며 `--check`는 출력 파일을
수정하지 않는다. 실제 Resources 배포는 이 변경에서 수행하지 않는다.

## 검증

1. 기존 27개를 포함한 Python mutation suite
2. deployment row omit/swap/path/size/SHA/status와 authority commit/tree/blob/self 공격
3. 4행 A/B 교환, coordinated reseal, supplied object 대체 공격
4. tracked deployment receipt bytes와 실제 deployed DDS bytes 변조 공격
5. sourceExact/Product 승격 공격
6. generator deterministic `--check`와 validate-only
7. focused ProjectAudit shallow/deep
8. deep mode에서 source UPK와 runtime DDS 48/48 path/size/SHA 역결합
9. JSON parse와 `git diff --check`

C++ 또는 project 파일 변경이 없으므로 C++ build와 `.vcxproj/.filters` 등록은 이번 검증 범위가
아니다. renderer 소비는 후속 R4 수직 슬라이스에서 별도로 닫는다.
