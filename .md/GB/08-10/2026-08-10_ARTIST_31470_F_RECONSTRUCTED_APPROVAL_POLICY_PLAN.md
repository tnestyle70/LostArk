# 도화가 31470 F RECONSTRUCTED_APPROVED_V1 정책 구현 계획

## 1. 목적

Source-era actual-output provider 탐색이 끝난 뒤에도 도화가 F 복원을 계속할 수 있도록,
원본 충실도와 재구성 실행 승인을 분리한 immutable 정책 계약을 만든다.

이 정책은 값을 추정해 실행하거나 Product를 여는 문서가 아니다. 독립 검토가 끝난 네 frozen lane을
정확한 Git commit/tree/blob/receipt identity로 결합하고, Source 29행과 Material 255행이 앞으로 어떤
versioned reconstruction family와 oracle로만 닫힐 수 있는지를 고정한다.

```text
evidence fidelity       = 기존 blocker와 label을 그대로 보존
reconstruction route    = RECONSTRUCTED_APPROVED_V1
sourceExactAdmission    = false
executionAdmission      = false
productAdmission        = false
```

## 2. 소유 범위

이 lane이 소유하는 것은 새 정책 source, JSON Schema, deterministic generator, generated receipt,
mutation harness, focused ProjectAudit, PLAN/RESULT뿐이다.

다음 기존 계약은 수정하지 않는다.

- Source evidence/schema/generator
- Material evidence/schema/generator
- Geometry contract/decoder/cooker
- Runtime foundation C++와 shader
- Authored Effect, Assembly, runtime catalog와 Resources

정책 receipt는 runtime semantic authority가 아니다. R2 이후 typed materializer가 이 정책과 frozen
evidence를 함께 소비하되, 정책 자체를 실행 payload로 읽어서는 안 된다.

## 3. Frozen 입력

| lane | commit | tree | 정책이 다시 검증하는 artifact |
|---|---|---|---|
| Source | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | `e5687551ac558abf63966c84f8cb5b33cf873188` | Source oracle acquisition receipt |
| Material | `cde8f3bddea2f9415f682b387d2705fd25794075` | `7b8f1d60870fd5c5410a6ef3e8060842bfd09cfd` | source-value acquisition + runtime oracle receipt |
| Geometry | `0aca792819fdda3f541bb7cec7451c5ed93c6467` | `ef01fb07c1381d2852f5b2c1f58a86b693a55786` | geometry resource binding receipt |
| Runtime foundation | `38ebe7cf7dceb5054bde93812907173cc0f98c67` | `1baecdfc51000380c525cb8041b7b5c3fc505a62` | runtime authority/catalog public implementation blob set |

R2 integration commit은 evidence authority로 사용하지 않는다. 위 네 commit에서 `git cat-file`로 실제
blob bytes를 다시 읽고, receipt self-hash와 schema/version을 검증한 뒤에만 정책 receipt를 생성한다.

## 4. 고정 분모

| 영역 | 분모 |
|---|---:|
| Source execution blocker row | 29 |
| Material render-state row | 89 |
| Material static-permutation row | 94 |
| Material strict sampler row | 72 |
| Material execution row 합계 | 255 |
| Material arithmetic family | 23 |
| Geometry carrier | 7 |
| 최종 runtime/manual occurrence | 35 |

Sampler 72에는 이전 `SOURCE_EXACT_SAMPLER` 4건을 반드시 포함한다. 네 건의 full descriptor는 Source-era
Texture2D CDO/config가 없으므로 exact가 아니다.

- explicit address tag가 있는 1건:
  `SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS`
- default provenance가 없는 3건:
  `SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN`

두 label 모두 `fullDescriptorSourceExact=false`, `sourceExact=false`다.

## 5. Source reconstruction family

| policy family | 행 | 허용 근거 | 필수 oracle 핵심 |
|---|---:|---|---|
| `RECONSTRUCTED_APPROVED_CURRENT_NATIVE_SEEDED_WRAPPER_V1` | 11 | current binary semantics + public UE3 behavior | same/different seed, base/current state |
| `RECONSTRUCTED_APPROVED_EF_CYLINDER_SPIN_V1` | 5 | native shape + independent implementation | seeded/unseeded basis, height/radius/velocity/rotation |
| `RECONSTRUCTED_APPROVED_GROUND_QUERY_V1` | 2 | tagged input + fixed scene query | skip 0/1/no-hit, local/world, rollback |
| `RECONSTRUCTED_APPROVED_DECAL_TYPEDATA_V1` | 3 | tagged input + typed render packet | projection/rotation/size/blend, invalid rollback |
| `RECONSTRUCTED_APPROVED_LIGHT_TYPEDATA_V1` | 1 | tagged/CDO evidence + typed light packet | brightness/radius/color/falloff/lifetime |
| `RECONSTRUCTED_APPROVED_VELOCITY_OVER_LIFE_V1` | 4 | tagged input + independent vector math | normalized time, local/world, owner transform |
| `RECONSTRUCTED_APPROVED_EF_VECTOR_MULTIPLY_V1` | 3 | native shape + independent parameter math | present/absent, Direct/Normal/Abs, four ranges |

각 Source occurrence는 exact class, occurrence ID, upstream cluster, required mutated output의 upstream hash와
정책 source/schema semantic SHA, 선택 family, closure basis, fidelity, oracle set의 policy binding hash를
함께 소유한다.
family나 정책 의미가 바뀌면 같은 upstream row라도 `policyRowId`가 바뀐다. 승인 정책은
`BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER`를 제거하지 않는다.

## 6. Material reconstruction family

| policy family | 행 | fidelity 경계 |
|---|---:|---|
| current CDO render state | 25 | current revision explicit, Source-era 아님 |
| Artist render policy | 64 | Source/current default value 없음 |
| static explicit selection | 23 | exact override value만 보존, selection semantics reconstructed |
| static nonoverride selection | 43 | exact nonoverride row, inheritance semantics reconstructed |
| parent-default selection | 28 | exact parent value, native selection reconstructed |
| current/archive Texture2D evidence | 69 | texture binding/partial tag와 full descriptor를 분리 |
| role sampler policy | 3 | source texture export가 없는 행만 명시적 role policy |
| arithmetic evaluator | 23 family | CPU/WARP numeric proof 보존, graph는 `RECONSTRUCTED_GRAPH` |

모든 255행은 recipe, occurrence set, field ID/kind, binding origin, upstream decision의 upstream digest와
정책 source/schema semantic SHA, 선택 family/basis/fidelity/oracle의 policy binding digest를 함께 소유한다. 정책 family를
바꾸거나 recipe/occurrence를 교환해 receipt를 다시 seal해도 row ID가 유지될 수 없고 frozen rebuild와
달라져 실패해야 한다.

## 7. 정책 Schema와 generated receipt

### 7.1 Source 문서

`lostark.effect-reconstruction-approval-policy-source` format 1은 다음을 소유한다.

- target와 policy ID/version
- reconstruction route 승인 범위
- 네 frozen lane identity
- 분모
- Source/Material family와 row routing rule
- forbidden fidelity claim
- R2-R7 global gate
- rollback condition

### 7.2 Receipt 문서

`lostark.effect-reconstruction-approval-policy` format 1은 다음을 생성한다.

- policy source/schema canonical SHA-256
- verified Git commit/tree/blob와 raw/canonical/receipt SHA-256
- Source 29 row와 family 7개
- Material 255 row와 arithmetic 23 family
- Geometry 7 expected tuple projection
- fidelity/admission/manual-validation/rollback 계약
- receipt self-hash

JSON Schema는 root와 모든 object `$def`에서 `additionalProperties=false`,
`required == properties`를 강제한다. 허용하지 않은 schema keyword, reference sibling, nested definition과
object closure 완화를 거부하고 admission과 Source-exact 관련 값은 `const false`로 닫는다.

## 8. 현재 admission과 미래 predicate

V1 정책 route 승인과 evidence join integrity는 true일 수 있다. 실행과 Product는 다음 이유로 false다.

- R2 typed materialization 미완료
- R3 typed executor 미완료
- R4 geometry/material runtime binding 미완료
- R5 여섯 renderer family 미완료
- R6 35 occurrence numeric/runtime/manual 검증 미완료
- R7 freeze/build/transaction regression 미완료

R2-R7이 끝나더라도 Source exact는 열리지 않는다. 최종 35/35가 가능해지는 label은
`RECONSTRUCTED_APPROVED`이며 `SOURCE_EXACT`가 아니다.

## 9. 수동 눈 검증 계약

수동 검증은 35 occurrence 모두의 human-eye runtime checklist다. 자동 screenshot/image 비교를 oracle로
사용하지 않고 capture artifact도 완료 조건으로 요구하지 않는다. numeric/state/identity gate가 먼저
통과한 뒤 같은 prepared revision을 사람이 확인한다.

현재 값은 다음으로 고정한다.

```text
requiredOccurrenceCount = 35
completedOccurrenceCount = 0
status = NOT_STARTED
```

## 10. Mutation과 회귀 검증

필수 반례는 다음과 같다.

- JSON version의 `true`, `1.0`, `"1"`
- target alias, occurrence denominator `35 -> 1`, gate/rollback trivialization
- frozen commit/tree/blob/receipt hash 변경
- 실제 descendant commit/tree/blob로 coordinated 교체
- Source 29 또는 Material 89/94/72/255 축소
- family rule gap/overlap과 row family 재배정
- Source family/oracle와 render rule/fidelity coordinated swap
- Source/Material evidence blocker 삭제
- recipe/occurrence/field ownership 교환
- 구 sampler 4건을 `SOURCE_EXACT_SAMPLER`로 재승격
- Source/Material/Geometry/arithmetic의 `sourceExact=true`
- row/global execution 또는 Product admission true
- Geometry preScale 변경
- manual 35/35 조작
- self-hash만 다시 seal한 semantic mutation
- tracked LF/CRLF 비교와 duplicate JSON key
- nested JSON Schema `additionalProperties`, required/property, unsupported keyword 완화

Focused audit는 unit mutation suite, generator `--check`, JSON parse, fixed summary를 한 번에 실행한다.

## 11. Rollback

다음 중 하나라도 발생하면 정책 생성·R2 소비·Product stage를 중단하고 이전 catalog/prepared state를
유지한다.

- frozen Git/receipt identity 불일치
- denominator/row identity 불일치
- evidence blocker 또는 fidelity label 소실
- unknown/unversioned family
- numeric/state/mutation oracle 실패
- Geometry/Material/resource/prepared identity 불일치
- runtime attach revision/hash/pointer 불일치
- manual occurrence 실패
- partial/stale Product stage

## 12. 완료 조건

이 lane은 다음까지만 완료한다.

- policy source/schema/generator/receipt deterministic
- 29/255/72/23/7 분모와 1+3 sampler 재분류 고정
- 모든 Source exact/execution/Product admission false
- mutation suite와 focused ProjectAudit PASS
- frozen checkpoint 독립 리뷰 후 단일 commit/push

R2 executor, Material/Geometry consumer, renderer, manual runtime 검증과 Product publish는 이 lane의 완료
주장이 아니다.
