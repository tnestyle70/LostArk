# 2026-08-10 Artist 31470 F Material Runtime Oracle Plan

## 목표

도화가 F의 `23 arithmetic family / 27 material recipe / 34 rendered occurrence`에 대해
source-revision ShaderCache 취득 가능성을 끝까지 검사하고, 취득할 수 없으면 surviving expression과
exact parameter evidence에서 만든 versioned reconstruction evaluator를 CPU와 실제 HLSL 실행으로
독립 검증한다. 이 단계는 graph를 `SOURCE_EXACT`로 승격하지 않으며 runtime handler와 renderer가
아직 소비하지 않으므로 Product admission도 열지 않는다.

## 입력과 판정 경계

- Material evidence contract: 23 family, 27 recipe, 34 occurrence, exact input lineage 정본
- ShaderCache oracle v2: installed cache 271 shader object / 25 map / 534 reference,
  Artist join `0/23`, MIC static set join `0/24`
- local source archive: 1,813 UPK를 raw-byte SHA-256으로 중복 제거한 624 package
- canonical physical runtime root는 이 slice에서 변경하지 않으며 texture provisioning은 Product gate로 남긴다.

source archive scan은 package 전체 graph를 추측하지 않고 summary와 NameTable이 포함된 압축 chunk만
AES/LZ4 해독한다. `ShaderCache` 또는 `sc_lv_*` name 후보가 있어야 full package decode 대상으로
승격한다. 후보 0이면 `SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE`를 보존한다.

## 구현

### `build_artist_31470_material_runtime_oracle.py`

1. 두 checked receipt의 root identity, self hash, denominator와 fail-closed decision을 검증한다.
2. raw graph expression parameter name을 semantic feature로 분류한다.
3. family마다 stable evaluator ID/version, feature mask와 graph evidence digest를 만든다.
4. 27 recipe의 729 scalar/vector/texture field와 94 static switch를 source lineage에 다시 결합한다.
5. 162 render-state field를 typed status로 운반하되 unresolved 89개는 실행값으로 만들지 않는다.
6. 34 occurrence가 exact recipe binding digest를 소비하는지 검증한다.
7. 네 개의 고정 family input과 실제 recipe binding에서 만든 108개 input을 CPU float evaluator로 계산한다.
8. source archive deep mode에서는 624 package NameTable inventory와 aggregate raw identity를 다시 계산한다.

Reconstruction opcode는 UV transform, panner, two-texture combine, color, desaturation, power, dissolve,
fresnel, distortion과 alpha를 고정 순서로 사용한다. feature는 surviving parameter/texture 이름에서만
활성화한다. 이것은 cooked에서 사라진 edge의 역사적 복원이 아니라 명시적인 재구성 evaluator다.

### `Shader_Artist31470MaterialOracle.hlsl`

CPU와 별도 구현인 HLSL compute evaluator다. receipt의 고정 input/feature mask를 StructuredBuffer로
받아 WARP D3D11에서 실제 실행하고 float4 output을 생성한다. 제품 shader가 아니며 G09가 공통
MaterialBinding extension point에 evaluator를 이식하기 전까지 validation-only다.

### `verify_artist_31470_material_runtime_oracle_hlsl.py`

pinned Windows SDK `d3dcompiler_47.dll`로 compute shader를 compile하고 D3D11 WARP device에서 실행한다.
CPU golden과 family/recipe sample 200개의 identity 순서를 tolerance 안에서 비교한다. compile만 성공한 것을 numeric
oracle PASS로 세지 않는다.

### receipt, tests, ProjectAudit

- `skill.31470.material-runtime-oracle.receipt.json`
- `test_build_artist_31470_material_runtime_oracle.py`
- `Test-Artist31470MaterialRuntimeOracle.ps1`

mutation은 family identity, feature mask, opcode version, recipe field ownership, occurrence recipe link,
CPU sample output, HLSL hash, source archive candidate count와 Product bit를 각각 공격한다.

## 종료 조건

- local source-revision ShaderCache acquisition: exact scanned denominator와 후보 수 기록
- evaluator `implemented/CPU verified/HLSL verified = 23/23/23`
- recipe/occurrence binding `27/27`, `34/34`, typed field `729 + 94`
- render-state typed status `162/162`, explicit 73, unresolved 89
- exact input field ownership loss 0, unknown semantic role 0
- graph fidelity 23/23 `RECONSTRUCTED_GRAPH`
- evaluator fidelity 23/23 `RECONSTRUCTED_NUMERICALLY_VERIFIED`
- source-exact graph/evaluator 0
- common Material runtime handler/renderer consumption false, Product false
- focused unit, shallow/deep audit, HLSL WARP execution, JSON parse, `git diff --check` PASS
- 이미지·육안 검증은 수행하지 않음
