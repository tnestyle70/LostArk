# 2026-08-11 Artist 31470 F Exact DDS Runtime Deployment Plan

## 현재 실제 반영 상태와 이번 경계

Material texture runtime binding receipt `39c91577c09b853fa55a8fd5531c1cddc4fef928d77a6caa7f67c472a56159e0`는
sampler policy 72행 중 68행을 현재 runtime cook asset에 결합했고, 다음 네 logical texture만 exact-DDS
provisioning proposal로 남겼다.

- `fx_tex_00.fx_a_decal_014`
- `fx_tex_00.fx_a_noise_011`
- `fx_tex_01.fx_c_atypical_016`
- `fx_tex_03.fx_e_ring_001_cl`

이번 변경은 proposal이 소유한 nested `fixtureAssetId`와 exact-DDS receipt가 증명한 UModel-exported DDS
bytes만 사용해 네 파일을 한 transaction으로 runtime Resources에 배포한다. basename 검색, 다른 candidate
slot, 이미지 비교, 재인코딩은 입력이 아니다. C++, renderer, runtime Material consumer, Product admission은
수정하지 않는다.

배포 대상 asset ID는 정확히 다음 네 개다.

- `Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds`
- `Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds`
- `Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds`
- `Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds`

## 추가·수정할 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Tools/LevelPlacementExtractor/deploy_artist_31470_exact_dds_runtime.py` | 두 frozen receipt와 DDS source bytes를 strict 검증하고 네 target을 all-or-nothing으로 배포한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Tools/LevelPlacementExtractor/artist_31470_exact_dds_runtime_deployment_approval.py` | authority commit/tree/path/blob, binding self digest와 최종 deployment projection을 독립 pin한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Tools/LevelPlacementExtractor/test_deploy_artist_31470_exact_dds_runtime.py` | path/casefold/symlink/partial copy/commit move/post-verify/receipt failure에서 전부 rollback되는지 검증한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-runtime-deployment.receipt.json` | 실제 four-file pre-state, source exact DDS identity, deployed byte count/SHA와 post-verify 결과를 기록한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Tools/ProjectAudit/Test-Artist31470ExactDdsRuntimeDeployment.ps1` | shallow receipt 검증과 explicit source/runtime root를 받는 deep byte/path 검증을 실행한다. |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | focused shallow deployment contract를 permanent audit에 등록한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-exact-dds-deploy/LostArk/.md/GB/08-11/2026-08-11_ARTIST_31470_F_EXACT_DDS_RUNTIME_DEPLOYMENT_RESULT.md` | 구현·자동 검증·physical deployment·남은 R4 경계를 분리 기록한다. |

이번 변경은 Python/JSON/PowerShell만 추가하므로 `.vcxproj`, `.vcxproj.filters`, C++ build는 해당하지
않고 build lease를 사용하지 않는다.

## 배포 함수의 책임과 실제 흐름

`load_and_validate_contract`는 duplicate key/BOM/non-finite/type를 거부하고 authority
`fda3b5637847f9205915ad25ff02215424024b88`의 binding/exact-DDS receipt identity를 검증한다. 네 proposal은
logical path, proposal ID, texture resource ID, policy, nested fixture ID, exact export/serial/DDS identity까지
exact-DDS asset row와 1:1이어야 한다.

`inspect_source_and_targets`는 source receipt의 전체 `sourceExtractedDdsRelativePath`와 proposal의 전체
runtime asset ID만 해석한다. 절대/drive/backslash/`..`, casefold sibling collision, symlink/junction/reparse
segment를 거부하고 source와 기존 target의 size/SHA/path case를 기록한다.

`deploy_transaction`의 순서는 고정한다.

1. receipt 출력 경로의 absolute Windows case-insensitive identity가 네 runtime target 및
   runtime-side backup/stage/temp namespace와 겹치지 않는지 어떤 write보다 먼저 검증한다.
2. 네 source DDS를 transaction temp에 byte-for-byte copy하고 모두 size/SHA/header를 재검증한다.
3. 네 target의 기존 상태를 모두 backup한다. 부재는 explicit absent marker로 보존한다.
4. `Resources` 밖의 `Client/Bin/Artist31470ExactDdsRecovery/<backupId>`에 rollback manifest와 기존
   payload backup을 atomic commit한다. 이 durable backup은 성공 뒤에도 제거하지 않는다.
5. staged file을 네 target에 하나씩 atomic replace한다.
6. 네 target 전체의 exact path case, regular-file, no-reparse, size/SHA를 post-verify한다.
7. deployment receipt를 atomic replace한다.
8. receipt commit 뒤 네 target 전체와 durable recovery manifest/payload를 다시 검증한 뒤에만 성공한다.
9. 성공한 뒤 transaction temp만 제거하고 durable recovery backup은 보존한다.

copy, backup, directory creation, move, post-verify, receipt replace 중 어느 단계든 실패하면 네 target을
transaction 시작 전 상태로 모두 복원하고 새로 만든 빈 directory를 제거한다. 기존 target이 정확한 bytes여도
pre-state와 backup을 기록한다. 기존 target이 다른 bytes면 팀장 관리 입력을 덮어쓰지 않고 transaction 시작
전에 fail-closed한다.

## receipt와 admission 불변식

- schema는 `lostark.artist-31470-exact-dds-runtime-deployment-receipt`, `formatVersion`은 exact integer 1이다.
- policy는 `RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1` 하나뿐이다.
- deployed row는 4개이며 ordered logical/runtime asset projection이 frozen proposal과 정확히 같다.
- 각 row는 physical UPK identity, export index/serial identity, UModel DDS identity, target pre-state,
  deployed file size/SHA를 소유한다.
- receipt는 `RUNTIME_RESOURCE_ROOT_PARENT` 기준 durable recovery directory와 manifest의 canonical
  self digest 및 실제 직렬화 파일 raw SHA-256을 서로 다른 필드로 기록한다.
- runtime asset bytes admission은 true지만 `sourceExactMaterialClaim`, renderer consumer, R4, Product는 false다.
- blocker는 `R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE`를 유지한다.
- receipt root/row digest를 함께 다시 봉인해도 independent approval projection이 다르면 거부한다.
- shallow audit은 tracked receipt만 검증하고, deep audit은 explicit source/runtime root의 실제 네 파일을 다시
  case/size/SHA 검증한다.

## 검증

1. focused Python unit/failure-injection suite
2. clean temporary runtime root에 실제 deployment 후 receipt validate/check
3. source DDS와 deployed DDS 4/4 raw size/SHA deep audit
4. focused ProjectAudit shallow/deep
5. strict JSON parse, `git diff --check`, 전체 ProjectAudit baseline 분리 기록

눈 검증과 renderer 검증은 이번 변경의 성공 조건이 아니다. 이 배포 receipt를 Material binding corrective가
소비해 72/72 runtime asset binding으로 승격한 뒤 R4 texture SRV consumer에서 별도로 검증한다.
