# 2026-08-10 Artist 31470 F Material Reconstructed Policy Plan

## Frozen implementation refinements

- `OpacityMaskClipValue` 26 rows use the same explicit float32 value. Twenty-five rows retain the serialized current Material CDO candidate as their policy basis; the one row whose current CDO field is omitted uses the named opacity-threshold role policy. Neither basis is source-era exact evidence.
- D3D11 verification projects every field of the depth-stencil, rasterizer, and sampler descriptors, including sampler `BorderColor`.
- Sampler HLSL receives primitive filter/address/sRGB policy fields rather than the expected output vector, and 72 typed SRGB/LINEAR mappings are verified with `CreateShaderResourceView -> GetDesc` on WARP.
- A separate approval module pins the ordered 255-row policy projection, HLSL source/DXBC/input/output, and 107-row WARP descriptor projection. The generator-derived receipt alone is not the approval authority.

## 목표

frozen Material evidence commit `cde8f3bddea2f9415f682b387d2705fd25794075`의
render-state 89, static permutation 94, strict sampler 72행을 변경하지 않고 소비해
`RECONSTRUCTED_APPROVED_V1` 실행값 정책 255행을 만든다.

이 정책은 source-era exact 복원이 아니다. 모든 행은 `sourceExact=false`와 기존 evidence blocker를
유지한다. 다만 source artifact 부재를 silent fallback으로 처리하지 않고, recipe/field/occurrence에
고정된 명시적 값 또는 descriptor, provider basis, implementation ID/version과 numeric oracle을 가진다.

첫 checkpoint는 policy compiler 입력만 승인한다. Material runtime consumer, renderer consumption과
Product admission은 모두 false로 유지한다.

## 입력과 금지 경계

- 입력
  - `skill.31470.material-runtime-oracle.receipt.json`
  - `skill.31470.material-source-value-acquisition.receipt.json`
  - `skill.31470.typed-material-evidence-contract.json`
- frozen parent: `cde8f3bddea2f9415f682b387d2705fd25794075`
- 금지
  - Source, Geometry, Runtime C++ 수정
  - 기존 Material evidence/receipt의 source fidelity 승격
  - 이전 sampler 4행을 `SOURCE_EXACT_SAMPLER`로 복구
  - owner 없는 default, field-name global fallback, 이미지 기반 검증

## 정책값

### Render-state 89

- `bDisableDepthTest=false`: effect depth-test enabled role policy
- `bUseOneLayerDistortion=false`: omitted opt-in feature disabled role policy
- `OpacityMaskClipValue=0.33329999446868896`: current serialized Material CDO candidate를
  reconstruction value provider로 사용
- `TwoSided=false`: omitted opt-in cull policy, D3D11 back-face cull
- `LightingModel=mlm_unlit`: effect-material unlit role policy

각 행은 upstream recipe/field/occurrence owner, omitted default chain과 blocker를 그대로 보존한다.

### Static permutation 94

- exact MIC `bOverride=true` 23행은 instance value를 유지하되 execution policy fidelity는
  `RECONSTRUCTED_APPROVED_V1`이다.
- `bOverride=false` 43행은 해당 행의 parent expression default를 선택한다.
- exact GUID entry 없음 28행도 해당 행의 parent expression default를 선택한다.

전역 parameter-name fallback은 사용하지 않는다. field ID와 source section/index가 같은 행의
parent/MIC evidence만 사용할 수 있다.

### Strict sampler 72

- source explicit AddressX/Y가 있으면 그 값을 유지하고, omitted이면 effect UV wrap role policy를 쓴다.
- source explicit sRGB가 있으면 유지하고, omitted이면 source LODGroup의 color/normal/specular role로
  SRGB/LINEAR을 결정한다.
- source explicit LODGroup을 유지한다.
- omitted Filter는 current Texture CDO의 `TF_Linear` 후보를 reconstruction policy로 사용한다.
- AddressW, comparison, border, LOD 범위는 versioned D3D11 effect sampler role policy가 소유한다.

72행 모두 full descriptor를 가지지만 `sourceExact=false`이며 기존 sampler provenance blocker를
그대로 운반한다.

## 구현 파일

- `Tools/LevelPlacementExtractor/build_artist_31470_material_reconstructed_policy.py`
  - upstream receipt 세 개를 검증하고 ordered 255행을 pure function으로 재생성한다.
  - strict schema, row/owner/order/value/provider/implementation/oracle digest를 검증한다.
- `Tools/LevelPlacementExtractor/verify_artist_31470_material_reconstructed_policy_hlsl.py`
  - 255행을 D3D11 WARP compute에서 finite float4로 재생한다.
  - depth/cull/sampler descriptor 행은 D3D11 state object `GetDesc`를 zero tolerance로 대조한다.
- `Tools/MaterialEvaluatorHarness/Shader_Artist31470MaterialReconstructedPolicy.hlsl`
  - 128-byte typed input 한 행을 정책 kind별 numeric projection으로 변환한다.
- `Data/Effects/Imported/Artist/Materials/skill.31470.material-reconstructed-approved-v1.receipt.json`
  - generated policy receipt다.
- `Tools/LevelPlacementExtractor/test_build_artist_31470_material_reconstructed_policy.py`
  - actual builder와 pure validator mutation을 검증한다.
- `Tools/ProjectAudit/Test-Artist31470MaterialReconstructedPolicy.ps1`
  - shallow/deep/WARP focused gate다.

## 검증과 종료 조건

- exact denominator/order: `89 + 94 + 72 = 255`
- 모든 행에 recipe, field, nonempty occurrence owner가 있고 upstream과 exact 일치
- 모든 행에 typed selected value/descriptor, provider basis, implementation ID/version 존재
- 모든 행 `sourceExact=false`, 기존 evidence blocker exact inclusion
- static exact override 23행 보존, sampler source-exact 0/72
- WARP compute 255/255 finite exact, D3D state descriptor 107/107 zero-tolerance
- row/owner/order/value/provider-policy swap과 coordinated reseal reject
- NaN/Inf/bool-as-number와 WARP descriptor mutation reject
- policy 255/255, runtime consumer 0, renderer consumer 0, Product 0
- unit, shallow/deep focused audit, JSON parse, `git diff --check` PASS
