# 2026-08-10 Artist 31470 F Material Runtime Oracle Plan

## 목표

도화가 F의 Material 증거를 `23 arithmetic family / 27 material recipe / 34 rendered occurrence`로
보존하면서 R0 실행 가능성을 행 단위로 판정한다. 기존 receipt의 render-state omitted 89개,
static permutation 94개, direct-unproven sampler 68개를 각각 안정 ID를 가진 251개 행으로 만든다.

이 단계는 두 판정을 분리한다.

- evidence integrity: 입력 정본, 소유권, 순서, evaluator, archive identity와 검증 결과가 서로 일치하는가
- execution readiness: 각 행에 source 값을 정하는 독립 provider와 actual output pilot가 있는가

evidence integrity가 PASS여도 251개 중 하나라도 `BLOCKED`, ownerless 또는 unknown이면 execution
readiness는 BLOCK이다. 이 경우 final materializer, Playback, renderer와 Product admission으로 넘어가지 않는다.

## 입력과 판정 경계

- Material evidence contract: 23 family, 27 recipe, 34 occurrence, exact input lineage 정본
- ShaderCache oracle v2: installed cache 271 shader object / 25 map / 534 reference,
  Artist join `0/23`, MIC static set join `0/24`
- local source archive: 1,813 UPK를 raw-byte SHA-256으로 중복 제거한 624 package
- canonical physical runtime root는 이 slice에서 변경하지 않으며 texture provisioning은 Product gate로 남긴다.

source archive scan은 package 전체 graph를 추측하지 않고 summary와 NameTable이 포함된 압축 chunk만
AES/LZ4 해독한다. `ShaderCache` 또는 `sc_lv_*` name 후보가 있어야 full package decode 대상으로
승격한다. 후보 0이면 `SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE`를 보존한다.

현재 revision의 값, 이름 유사성, parent 이름, evaluator 자체 출력은 source-era Material state의
provider가 아니다. WARP state object 생성은 D3D11 descriptor가 어떤 출력 상태를 만드는지 증명할 뿐,
그 descriptor 값이 원본 도화가 F의 값이라는 증거로 사용하지 않는다.

## 구현

### `build_artist_31470_material_runtime_oracle.py`

1. checked receipt의 root identity, self hash, denominator와 fail-closed decision을 검증한다.
2. raw graph expression에서 feature mask를 독립 재도출하고 stored feature mask와 exact 비교한다.
3. family마다 stable evaluator ID/version, feature mask와 graph evidence digest를 만든다.
4. 27 recipe의 729 scalar/vector/texture field와 94 static switch를 exact
   `sourceSection/sourceSectionIndex/sourceOwnerRecipeId` 순서로 다시 결합한다.
5. occurrence마다 recipe, binding, evaluator owner를 exact mapping으로 검증한다. set membership만으로는
   다른 recipe의 binding/evaluator를 수용하지 않는다.
6. render-state 89, static permutation 94, direct-unproven sampler 68의 feasibility row를 만든다.
7. 각 row에 instance, parent, nested default, CDO, ShaderCache, runtime capture identity와 availability,
   acquisition outcome, provider, input domain, expected output, tolerance, pilot, decision, owner를 기록한다.
8. source archive 1,813/624 projection과 generator/verifier/HLSL/입력 receipt의 실제 canonical hash를
   매 검증에서 다시 계산한다. archive candidate count만 맞춘 coordinated reseal도 거부한다.
9. 모든 canonical JSON parse/write는 `NaN`, `+Inf`, `-Inf`를 거부한다.

Reconstruction opcode는 UV transform, panner, two-texture combine, color, desaturation, power, dissolve,
fresnel, distortion과 alpha를 고정 순서로 사용한다. feature는 surviving parameter/texture 이름에서만
활성화한다. 이것은 cooked에서 사라진 edge의 역사적 복원이 아니라 명시적인 재구성 evaluator다.

### `verify_artist_31470_material_runtime_oracle_hlsl.py`

pinned Windows SDK `d3dcompiler_47.dll`로 compute shader를 compile하고 D3D11 WARP device에서 실행한다.
CPU golden과 family/recipe sample 200개의 identity 순서를 tolerance 안에서 비교한다. compile 성공만으로
numeric oracle PASS를 주지 않으며 actual WARP output의 모든 lane이 finite인지 먼저 검사한다.

별도의 D3D11 WARP state-object provider pilot은 다음 네 descriptor mutation을 실제 생성 후 `GetDesc`로
round-trip한다.

1. blend mode
2. depth enable
3. rasterizer two-sided/cull
4. sampler address wrap/clamp

이 네 pilot은 provider implementation 검증이다. source-specific row value의 provenance가 없으면 해당
feasibility row는 계속 `BLOCKED`다.

### receipt, tests, ProjectAudit

- `skill.31470.material-runtime-oracle.receipt.json` format version 2
- `test_build_artist_31470_material_runtime_oracle.py`
- `Test-Artist31470MaterialRuntimeOracle.ps1`

mutation은 occurrence cross-recipe binding/evaluator, raw expression feature mask, input/static source order,
세 matrix denominator, owner, archive projection, tracked canonical hash, actual WARP `NaN/+Inf/-Inf`, Product
bit와 execution-readiness 세탁을 각각 공격한다. render root identity 또는 ShaderCache denominator를 바꾸고
상·하위 receipt hash를 함께 reseal하는 공격도 shared shallow 인증 경로에서 거부한다.

## 종료 조건

### Evidence-integrity PASS

- family/recipe/occurrence `23/27/34`, input/static `729/94` 분모 보존
- exact recipe-to-binding/evaluator ownership 및 ordered source identity 손실 0
- render-state/static/sampler matrix `89/94/68`, 합계 251행, denominator shrink 0
- ownerless 0, unknown decision 0
- WARP numeric sample 200 및 state-object pilot 4 actual 실행
- non-finite output와 coordinated reseal mutation 전부 거부
- source archive 1,813/624와 tracked canonical hash 재현
- Product false, runtime handler/renderer consumption false

### Execution-readiness PASS

- `renderStateRows 89/89`, `staticPermutationRows 94/94`, `directUnprovenSamplerRows 68/68`가 모두
  `FEASIBLE` 또는 독립 증거가 있는 `VERIFIED_IRRELEVANT`
- source-specific provider, actual output pilot, input domain, expected output, tolerance, final runtime owner가
  모든 행에 존재
- blocked 0, ownerless 0, unknown/unresolved execution 0

Evidence-integrity만 PASS하고 execution-readiness가 BLOCK이면 이 corrective는 정직한 R0 BLOCK으로
동결한다. 이미지·육안 검증은 수행하지 않는다.
