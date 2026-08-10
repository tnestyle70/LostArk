# 2026-08-11 Artist 31470 F Material Texture Runtime Binding Plan

## 현재 실제 반영 상태와 이번 경계

Material reconstructed policy `97597531215fa9c9873fe1be3ba8cd23db60031d`는 sampler descriptor
72/72를 선택했지만, 각 `logicalTexturePath`를 실제 Resources asset ID에 결합하는 계약은 없다.
이번 변경은 그 72행만 다룬다. C++, renderer, runtime program builder, Product admission은 수정하지 않는다.

실측 denominator는 다음과 같다.

- sampler policy: 72행, unique logical texture 48개
- frozen Artist runtime-cook full-path join: 44/48 texture, policy 68/72행
- cook 결합된 현재 Resources 파일: 44/44 size/SHA 일치
- exact-DDS만 있고 current runtime-cook mapping이 없는 texture: 4개, policy 4행
- native-v14 candidate texture slot: 57행, unique asset ID 36개; Material field authority로 사용하지 않음
- pinned source-pack manifest package identity: unique texture 45/48. `wp_mn_lrcn_01`의 3개 texture는
  resource manifest와 runtime cook/export에는 있지만 source-pack package row가 없어 별도 evidence
  blocker로 보존함

4개 미결합 texture는 다음과 같다.

- `fx_tex_00.fx_a_decal_014`
- `fx_tex_00.fx_a_noise_011`
- `fx_tex_01.fx_c_atypical_016`
- `fx_tex_03.fx_e_ring_001_cl`

이 네 행은 basename으로 추측하지 않는다. exact-DDS receipt의 authenticated DDS/export identity와 다음
exact-DDS receipt가 이미 소유한 nested `fixtureAssetId`를 provisioning proposal로만 기록한다.

- `Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds`
- `Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds`
- `Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds`
- `Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds`

이 제안은 `RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1`이며 source exact 승격이 아니다.
transactional deploy/verification receipt가 생기기 전에는 `UNRESOLVED_RUNTIME_ASSET`과 runtime admission
false를 유지한다.

## 추가·수정할 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_material_texture_runtime_binding.py` | frozen Material/cook/export/UPK/DDS/candidate evidence를 72행 binding receipt로 결합하고 검증한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Tools/LevelPlacementExtractor/artist_31470_material_texture_runtime_binding_approval.py` | 72행 결정 projection과 frozen external identity를 독립 pin한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Tools/LevelPlacementExtractor/test_build_artist_31470_material_texture_runtime_binding.py` | owner/path/status/asset/reseal/path-safety/duplicate-key mutation을 거부한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json` | 68 resolved + 4 unresolved runtime binding과 네 provisioning proposal의 generated 정본이다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Tools/ProjectAudit/Test-Artist31470MaterialTextureRuntimeBinding.ps1` | shallow checked receipt와 optional external deep file/hash 검증을 실행한다. |
| 수정 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | focused non-deep check를 permanent audit에 등록한다. |
| 추가 | `C:/Users/user/.codex/worktrees/artist-f-v2-material-texture-binding/LostArk/.md/GB/08-11/2026-08-11_ARTIST_31470_F_MATERIAL_TEXTURE_RUNTIME_BINDING_RESULT.md` | 실제 검증 결과와 미완료 4행을 분리 기록한다. |

## receipt 계약과 생성 흐름

각 binding은 `samplerPolicyRowId`, `recipeId`, `materialInputFieldId`, `logicalTexturePath`, ordered
`materialOccurrenceIds` 전체에서 stable `bindingId`를 다시 만든다. Policy 72행과 typed contract의 recipe
input owner/occurrence reverse join이 정확히 하나씩 존재해야 한다.

resolved 행은 logical path 전체 문자열로 runtime-cook row와 source export row를 각각 하나만 찾는다.
Cook `sourceFile`은 export output relative path와 같고, exported DDS와 runtime copied DDS의 size/SHA가
같아야 한다. Runtime asset ID는 `Effect/Artist/Textures/*.dds`의 안전한 flat path만 허용한다.

각 logical path는 tracked Artist source resource manifest의 physical UPK row와 external source-pack
manifest의 raw size/SHA에 결합한다. Deep mode는 실제 UPK와 runtime DDS bytes를 다시 해시한다.

unresolved 네 행은 cook/export mapping을 만들지 않는다. exact-DDS receipt row와 authenticated Texture2D
export/DDS identity를 복사하고 `RUNTIME_COOK_OR_TRANSACTIONAL_DEPLOY_RECEIPT_REQUIRED` blocker를 유지한다.
Provisioning proposal은 exact-DDS receipt의 기존 `fixtureAssetId`와 exact source DDS identity만 결합하며
새 flat path를 만들거나 deploy 완료를 주장하지 않는다.

## 불변식과 실패 처리

- JSON은 UTF-8 BOM, duplicate key, non-finite 숫자를 거부한다.
- `formatVersion`은 exact JSON integer다.
- policy 72, unique logical 48, resolved 68, unresolved 4, provisioning proposal 4를 다시 유도한다.
- policy row, recipe/input owner, occurrence, descriptor/SRV identity의 제거·교환·재봉인을 거부한다.
- source logical path와 runtime asset ID는 casefold collision과 many-to-one mapping을 거부한다.
- 절대 경로, drive path, backslash, `..`, wrong prefix/suffix를 runtime asset ID로 허용하지 않는다.
- candidate slot은 exact asset ID 관찰값일 뿐 authority/admission이 아니다.
- 네 proposal은 target file이나 transactional deployment receipt가 없으므로 resolved로 승격할 수 없다.
- 실패하면 output 파일을 부분 교체하지 않는다. `--check`는 파일을 쓰지 않는다.

## 검증

1. focused Python unit mutation suite
2. generator `--check` deterministic rebuild
3. focused ProjectAudit shallow
4. external cook/export/source-pack/UPK/Resources를 지정한 deep audit
5. strict JSON parse, `git diff --check`, 전체 ProjectAudit 결과 분리 기록

이번 변경은 Python/JSON/PowerShell만 추가하므로 `.vcxproj`와 `.vcxproj.filters` 등록 및 C++ build는
해당하지 않는다.
