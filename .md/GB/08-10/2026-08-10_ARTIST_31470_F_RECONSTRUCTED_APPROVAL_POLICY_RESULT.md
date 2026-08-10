# 도화가 31470 F RECONSTRUCTED_APPROVED_V1 정책 구현 결과

## 1. 판정

`RECONSTRUCTED_APPROVED_V1`의 구현·검증 route를 immutable policy receipt로 만들었다. 네 frozen lane의
Git commit/tree/blob와 upstream receipt self-hash를 실제 object database에서 다시 검증하고,
Source 29행·Material 255행·arithmetic 23 family·Geometry 7 carrier를 row identity와 versioned family에
결합했다.

현재 판정은 의도적으로 다음과 같다.

```text
policyRouteApproved     = true
evidenceJoinIntegrity  = true
sourceExactAdmission   = false
executionAdmission     = false
productAdmission       = false
manual eye validation = 0/35 NOT_STARTED
```

이 결과는 복원 실행 완료나 Product admission이 아니다. R2-R7을 진행해도 Source fidelity blocker는
남으며, 최종 실행 fidelity는 `RECONSTRUCTED_APPROVED`로만 표현한다.

## 2. 실제 생성물

| 파일 | 역할 |
|---|---|
| `Data/Effects/Policies/Artist/artist.31470.f.reconstructed-approved-v1.policy-source.json` | reviewed frozen input과 family/rule 정본 |
| `Tools/EffectPipeline/Schemas/lostark.effect-reconstruction-approval-policy.schema.json` | closed JSON Schema |
| `Tools/EffectPipeline/build_reconstructed_effect_approval_policy.py` | Git blob/receipt 검증과 deterministic generator |
| `Data/Effects/Policies/Artist/artist.31470.f.reconstructed-approved-v1.policy.receipt.json` | generated immutable policy receipt |
| `Tools/EffectPipeline/test_build_reconstructed_effect_approval_policy.py` | builder/validator mutation harness |
| `Tools/ProjectAudit/Test-Artist31470ReconstructedApprovalPolicy.ps1` | focused audit |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | permanent audit registration |

기존 Source/Material/Geometry schema·generator와 Runtime C++/shader는 수정하지 않았다.

## 3. Frozen identity 결합 결과

| lane | commit | tree | 결합 결과 |
|---|---|---|---|
| Source | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | `e5687551ac558abf63966c84f8cb5b33cf873188` | receipt schema/version/blob/self-hash PASS |
| Material | `cde8f3bddea2f9415f682b387d2705fd25794075` | `7b8f1d60870fd5c5410a6ef3e8060842bfd09cfd` | acquisition/runtime receipt 2개 PASS |
| Geometry | `0aca792819fdda3f541bb7cec7451c5ed93c6467` | `ef01fb07c1381d2852f5b2c1f58a86b693a55786` | resource binding receipt PASS |
| Runtime foundation | `38ebe7cf7dceb5054bde93812907173cc0f98c67` | `1baecdfc51000380c525cb8041b7b5c3fc505a62` | authority/catalog/audit blob 5개 PASS |

R2 integration commit은 evidence authority에 포함하지 않았다.

## 4. 분모와 family 결과

### 4.1 Source

```text
row        29
family      7
sourceExact 0
execution   0
Product     0
```

Family occurrence 합은 `11 + 5 + 2 + 3 + 1 + 4 + 3 = 29`다. 각 행은 upstream occurrence ID와
required mutated output을 upstream identity로 고정한다. 별도 `policyBindingSha256`는 policy source/schema
semantic SHA, 선택 family, closure basis, fidelity와 oracle set을 결합하므로 의미/family가 바뀌면 row ID도
바뀐다. 기존 provider blocker는 그대로 보존한다.

### 4.2 Material

```text
render state       89
static permutation 94
strict sampler     72
total             255
arithmetic family  23
```

Material route 실제 분포는 다음과 같다.

| family | 행 |
|---|---:|
| current CDO render state | 25 |
| Artist render policy | 64 |
| static explicit | 23 |
| static nonoverride | 43 |
| parent default | 28 |
| Texture2D evidence | 69 |
| role sampler policy | 3 |

이전 `SOURCE_EXACT_SAMPLER` 4건은 72행에 다시 포함됐다.

```text
SOURCE_EXACT_TEXTURE_BINDING_PARTIAL_SAMPLER_TAGS       1
SOURCE_EXACT_TEXTURE_BINDING_SAMPLER_DEFAULT_UNPROVEN   3
fullDescriptorSourceExact                              0/4
samplerFullSourceExactCount                            0/72
forbiddenFullFidelityLabelCount                        0
```

Arithmetic 23 family의 기존 CPU/WARP numeric proof는 보존했지만 graph provenance는 계속
`RECONSTRUCTED_GRAPH`, Source exact는 0, runtime execution은 0이다.

### 4.3 Geometry

7 carrier의 asset ID, candidate SHA, payload SHA, metadata identity와 `geometryPreScale=0.01`을 결합했다.
기존 artifact binding integrity는 보존했지만 runtime preScale/bounds/cache/shader consumption이 아직
없으므로 execution/Product는 0이다.

## 5. Receipt identity

현재 generated receipt의 self-hash는 다음과 같다.

```text
receiptSha256             da922add6d90287c88eed1d97efd8bd29ef35cd724c11f87ccfbb0bf9c3b7957
policySourceCanonicalSha  d559485809068968180d3b913e6e72121fcb219d977a3b596eff0c80ff6e83d7
policySchemaCanonicalSha  d441711c2a88e58121d7ed3f335bdfa0dc38ed72c522232614ffbda1e8bcce81
```

Generator는 receipt를 쓰기 전에 네 Git commit/tree/blob와 upstream self-hash를 검증한다. `--check`는
같은 frozen input에서 다시 생성한 canonical document와 tracked LF/CRLF-normalized 비교를 수행한다.

## 6. 실행한 검증

### 6.1 Unit/mutation harness

`python -B -m unittest -v Tools.EffectPipeline.test_build_reconstructed_effect_approval_policy`

- 41/41 PASS
- version bool/float/string 거부
- commit/tree/blob/receipt identity mutation 거부
- real descendant commit/tree/blob coordinated replacement 거부
- 29/89/94/72/255 denominator 축소 거부
- effect occurrence `35 -> 1`, target alias, global gate trivialization 거부
- family rule gap/overlap 거부
- Source family/oracle와 render family/fidelity coordinated swap 거부
- evidence blocker, owner, family reassignment 거부
- sampler exact laundering 거부
- Source exact/execution/Product/manual 조작 거부
- Geometry preScale, arithmetic fidelity, receipt self-hash mutation 거부
- LF/CRLF stable comparison과 duplicate key 거부
- nested Schema closure 완화와 unsupported keyword 거부

### 6.2 Generator

`python -B Tools/EffectPipeline/build_reconstructed_effect_approval_policy.py --check`

```text
PASS: reconstructed approval policy source=29 material=255 sampler=72 geometry=7 sourceExact=false execution=false product=false
```

### 6.3 Focused ProjectAudit

`powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470ReconstructedApprovalPolicy.ps1`

```text
PASS: Artist F 31470 reconstructed approval policy tests=41 source=29 material=255 sampler=72 arithmetic=23 geometry=7 sourceExact=false execution=false product=false
```

### 6.4 Full ProjectAudit

`powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1 -ReportPath .codex_tmp/Artist31470ReconstructedPolicy.ProjectAudit.json`

- 전체 exit: 1
- 전체 failure: 13
- 신규 `effect.artist-31470-reconstructed-approval-policy`: PASS
- 실패 범위: map/runtime root 2건, project data visibility, 기존 Effect Tool/runtime boundary,
  미빌드 Source/Geometry harness, component/authored rollout, 물리 Character resource 등 정책 lane 밖 항목

이 실행에서 13건의 baseline delta 동일성은 비교하지 않았으므로 full audit를 PASS로 기록하지 않는다.
정책 자체의 permanent check가 전체 audit 안에서 PASS한 사실만 기록한다.

Debug/Release runtime build는 실행하지 않았다. 이 변경은 Python/JSON/ProjectAudit 정책 계약만 추가하고
Runtime C++를 수정하지 않는다. 이 RESULT는 R2-R7 runtime 검증을 PASS로 기록하지 않는다.

## 7. 남은 경계

다음 작업은 이 receipt를 실행값으로 읽는 것이 아니라 frozen evidence와 함께 typed materializer input
gate로 소비하는 R2다.

1. Source 29행 versioned handler와 independent mutated-output oracle
2. Material 255행 value/state materialization과 mutation oracle
3. arithmetic 23 family의 common shader binding
4. Geometry preScale/bounds/cache/shader의 정확히 한 번 소비
5. typed executor와 six renderer family
6. fixed seed/time 35 occurrence numeric runtime oracle
7. 사람 눈 35/35 checklist
8. Debug/Release, transaction rollback, freeze 후 Product atomic publish

위 항목이 끝나기 전에는 execution/Product를 열 수 없다. 끝난 뒤에도 Source exact는 열지 않는다.
