# 2026-08-10 Artist 31470 F Material Reconstructed Policy Result

## 결과

Material checkpoint `cde8f3bddea2f9415f682b387d2705fd25794075`의 미해결 255개 행을 변경하지 않고, 그 위에 `RECONSTRUCTED_APPROVED_V1` 실행값 정책 receipt를 추가했다.

- render state: 89/89 값 선택
- static permutation: 94/94 값 선택
- sampler descriptor: 72/72 값 선택
- 정책 compiler 입력: 255/255 승인
- typed runtime consumer: 0/255
- renderer consumer: 0/255
- Product admission: false

이 checkpoint는 Material 값을 추측 fallback으로 실행시키는 runtime 변경이 아니다. 각 값을 recipe, field, occurrence owner와 1:1로 고정한 다음, 별도 runtime consumer가 소비할 수 있는 typed 입력을 만든 단계다.

## 고정 입력

| 입력 | 고정 self digest |
|---|---|
| Material runtime oracle | `e128e281753fbd01582e588afbb682847401348836a046ad424a720360003ff6` |
| source-value acquisition | `cf45b6db4290aaffb10410bea346daf1bbbc52d585611a4347326483a7d48f43` |
| typed Material contract | `638fae77c5805a8d33cacb69b5cdd810d40d2f304606dd56f970dd2504c1cfcb` |

세 입력은 self digest를 정상 재봉인해도 위 frozen digest와 다르면 거부된다. mutable integration HEAD는 Material evidence authority로 사용하지 않았다.

## 선택한 정책값

### Render state 89

| field | count | selected value |
|---|---:|---|
| `bDisableDepthTest` | 26 | `false` |
| `bUseOneLayerDistortion` | 26 | `false` |
| `OpacityMaskClipValue` | 26 | float32 `0.33329999446868896` |
| `TwoSided` | 9 | `false` |
| `LightingModel` | 2 | `mlm_unlit` |

Opacity 25행은 current Material CDO의 serialized candidate record를 policy basis로 사용한다. current CDO에서도 해당 field가 생략된 1행은 이름이 고정된 opacity threshold role policy를 사용한다. 둘 다 `admissibleAsSourceEra=false`이며 `sourceExact=false`다.

### Static permutation 94

- exact MIC `bOverride=true`: 23행의 값 유지(14 true, 9 false)
- exact MIC entry의 `bOverride=false`: 43행에서 같은 행의 parent default 선택
- exact GUID entry 없음: 28행에서 같은 행의 parent default 선택
- 최종 값: true 85, false 9

exact override 값도 실행 정책 fidelity는 `RECONSTRUCTED_APPROVED_V1`이다. source evidence fidelity를 `SOURCE_EXACT`로 승격하지 않았다.

### Sampler 72

- Address U/V: wrap 63, clamp 9
- Address W: wrap 72
- Filter: linear 72
- sRGB: true 67, false 5
- LODGroup: source에 serialized된 닫힌 5개 enum domain 유지
- comparison, border, MinLOD/MaxLOD: versioned D3D11 effect sampler role policy

이전 4개 sampler를 포함하여 `SOURCE_EXACT_SAMPLER`는 0/72다. source explicit field는 그 값을 정책 선택의 근거로 보존할 뿐, 완전한 descriptor의 source exact 증거로 세탁하지 않는다.

## 검증

### Numeric/HLSL

- D3D11 WARP compute: 255/255
- input ABI: 행당 128 bytes
- output: 행당 finite float4
- tolerance: 0.0
- HLSL tracked SHA: `2901471f07495ff079c64ea8234ca30cd26300819d56760dbdeabae353c0b718`
- DXBC SHA: `a7a4192b0d1fa70e19be8a14c8b7001d4e4020b95459bf6f88da9d3b09922bd9`
- input SHA: `751c1969342ccf23e608c403e7d3c126b6bc07febd2d978d767347ecb3aaffd1`
- output SHA: `8eb1f9e4a53b681a8c3cf8bdada42718470d278955bc05df97e11922baeb9445`

### D3D11 state descriptors

- depth-stencil: 26/26
- rasterizer/cull: 9/9
- sampler: 72/72
- 합계: 107/107
- `Create*State -> GetDesc` expected/actual 전체 descriptor exact equality
- sampler `BorderColor`를 포함한 모든 필드 projection
- descriptor projection SHA: `5b76e727418c588efd614524e5202b3aaf7040ba4064d0faafe7195ce994c49d`

Sampler 72행은 typeless texture에 선택된 SRGB/LINEAR format으로 `CreateShaderResourceView`를 호출하고 `GetDesc`를 대조했다. SRV projection은 72/72이며 SHA는 `47d4c6bea3fa30805ae5a085cfe2094c56766a04ae45d4c76120dbefecedb14f`다.

### Mutation suite

33개 test가 다음 공격을 실제 validator/builder 경계에서 거부했다.

- row 제거/순서/ordinal 및 recipe/field/occurrence owner swap
- selected value, static override/parent decision, provider, implementation swap
- blocker 제거와 `sourceExact=true` 승격
- sampler clamp→wrap, explicit sRGB false→true, filter/AddressW/border/LOD 변경
- unknown LODGroup
- bool-as-int, NaN, +Inf, -Inf
- HLSL result row permutation
- WARP expected/actual descriptor 동시 변조 후 재봉인
- WARP descriptor/SRV expected·actual 동시 변조와 stale digest 유지
- descriptor extra/missing field
- 독립 approval에서 누락될 수 있는 fidelity/upstream evidence/row admission/top-level admission 변조
- current Texture CDO `TF_Linear` candidate missing/duplicate/value/record/export/admissibility 변조
- sampler expected-vector echo 대신 primitive HLSL input ABI
- HLSL/WARP top-level tolerance, max error, compiler identity, entry/profile, feature level, unknown key 재봉인
- actual CLI receipt의 root/row/descriptor/oracle duplicate key와 UTF-8 BOM을 shallow/deep 모두 거부
- LF/CRLF receipt shallow/deep 동일 결과
- strict loader, upstream validator, reconstructed verifier, runtime WARP support를 포함한 7개 direct-import canonical hash closure 변조
- actual CLI에서 shadow `PYTHONPATH` strict loader와 runtime WARP support가 로드되는 공격. 두 경우 모두 marker 실행 뒤 실제 `module.__file__` 경로 불일치로 fail-closed
- runtime/renderer/Product admission 승격
- upstream receipt의 coordinated reseal

실행 결과:

```text
python -B Tools/LevelPlacementExtractor/test_build_artist_31470_material_reconstructed_policy.py
Ran 33 tests ... OK

powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MaterialReconstructedPolicy.ps1
PASS: ... mode=shallow rows=89+94+72/255 staticExact=23 warp=255+107 srv=72 sourceExact=0 runtime=0 product=false

powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MaterialReconstructedPolicy.ps1 -DeepMaterialPolicyAudit
PASS: ... mode=deep rows=89+94+72/255 staticExact=23 warp=255+107 srv=72 sourceExact=0 runtime=0 product=false
```

이미지·육안 비교는 실행하지 않았고 완료 조건에도 포함하지 않았다.

## 남은 경계

다음 lane은 이 receipt를 immutable typed IR에 결합하고 255행을 실제 Material compiler/renderer가 소비하게 해야 한다. 그때까지 다음은 의도적으로 닫혀 있다.

- typed runtime consumer 0/255
- renderer consumer 0/255
- Product admission false
- 기존 source evidence blocker 전부 유지

첫 runtime 결합에서는 raw Material 문서나 field-name fallback을 허용하지 않고, receipt의 `policyRowId + recipeId + fieldId + occurrenceIds + implementation version` 전체 identity를 검증해야 한다.

## 독립 판정

최초 checkpoint는 oracle metadata 교정 뒤 `PASS — P1 0, P2 0`을 받았으나, exact commit `3910d402` 후속 감사에서 strict JSON parsing과 direct-import dependency pinning 두 건이 P2로 추가 확인됐다.

첫 corrective `d0816326`은 strict parsing과 canonical dependency byte closure를 추가했다. 후속 독립 감사에서는 실행된 shadow 모듈과 고정 정본 경로에서 다시 계산한 hash가 서로 달라도 통과할 수 있다는 P2가 확인됐다.

두 번째 corrective는 canonical repository root와 실제 loaded `module.__file__`를 일치시킨 뒤 그 실제 파일의 canonical text hash를 receipt에 결합한다. actual CLI shadow-helper 공격 두 건, duplicate/BOM/LF·CRLF fixture, dependency byte mutation, unit 33/33, deep WARP 255 + descriptor 107 + SRV 72를 self-review에서 재현했다. `sourceExact=0`, runtime/renderer/Product 0/false와 255개 selected value/descriptor는 변하지 않았다.

최종 generated receipt self digest는 `10c7cacac0c54bf22060ab54a5596d48785631dd65b12c1c2810a87eb013d1c7`이며, actual-loaded-module direct-import closure projection SHA는 `3622e631e6a06e4b4d0e5cae6a23dfc6da27326f372a41536ec50c2eba28d642`다.
