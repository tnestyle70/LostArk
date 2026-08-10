# 2026-08-10 Artist 31470 F Reconstructed Source Capability Result

## 결론

`RECONSTRUCTED_APPROVED_V1` Source capability evidence checkpoint를 생성했다. c927 Source execution
receipt가 차단한 29개 module occurrence는 누락 없이 7개 policy family에 배치되며, 각 행은 fixed
seed와 `0.0 / 0.25 / 1.0` 시간의 finite numeric sample 세 개를 가진다.

이 결과는 Source exact 복원 또는 Runtime 실행 합격이 아니다. 현재 revision 근거는 전부
`CURRENT_REVISION_CROSS_REVISION_EVIDENCE`로 남고 `sourceExact=false`다. capability evidence가
준비되었어도 독립 검토와 Runtime handler 소비가 끝나지 않았으므로 execution과 Product는 false다.

## 입력 정본

| artifact | tracked LF/no-BOM canonical SHA-256 | self SHA-256 |
|---|---|---|
| Source execution semantics | `de15843dfa2f151371c1e26c472f2d42a0bfd7c7f8c8a41a3cdd2da08eaccb9a` | `7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c` |
| Custom handler oracle | `c436e69e40e7ad13079940e9ed88d1522942c10a135e19c1d19b020e86b797ac` | `0da627b3ed5b100014f2a2ac1fa3591d861c6a241befee65ca856b406dedaadc` |
| Source oracle acquisition | `05d8c90b2000fbc6aa699f63805784b2013a700aa94ee5e85c4d4a30459cafbc` | `c49f4dfcabc09c765b2127c620c5dbc8676d18819de008eb813e583b3f07e98d` |

Integration branch HEAD는 Source authority로 사용하지 않았다.

## 분모

| family | occurrence | sample |
|---|---:|---:|
| seeded | 11 | 33 |
| cylinder-spin | 5 | 15 |
| ground | 2 | 6 |
| decal | 3 | 9 |
| light | 1 | 3 |
| velocity | 4 | 12 |
| ef-vector-multiply | 3 | 9 |
| 합계 | 29 | 87 |

추가 gate 결과:

- policy family 7/7
- implementation ID/version/contract SHA 7/7
- unknown row 0
- ownerless row 0
- numeric sample 없는 READY 0
- generic fallback 0
- source property 148 = reconstructed input consumed 117 + upstream oracle irrelevant 31
- unconsumed property 0
- distribution binding 65 = source-era pinned 0 + unpinned 65
- source-era identity field missing 60은 전부 explicit false
- global duplicate/foreign ID 0
- sourceExact 0
- current evidence → source exact 승격 0
- Runtime execution admission 0
- Product admission 0

## 보존한 차단 근거

- `CURRENT_REVISION_CROSS_REVISION_EVIDENCE`
- `CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN`
- `EXACT_NATIVE_PARTICLE_OUTPUT_ORACLE_REQUIRED`
- `EXACT_NATIVE_DISTRIBUTION_OUTPUT_ORACLE_REQUIRED`
- `SOURCE_ERA_NATIVE_HANDLER_IDENTITY_UNPINNED`
- `SOURCE_ERA_DISTRIBUTION_EVALUATOR_IDENTITY_UNPINNED`
- `SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED`
- `SOURCE_29_EXECUTION_READINESS_BLOCKED`
- `SOURCE_EXACT_NOT_CLAIMED`
- `INDEPENDENT_REVIEW_PENDING`
- `RUNTIME_HANDLER_CONSUMPTION_PENDING`
- `FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED`

## 구현 파일

- `Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py`
- `Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_source_capability.py`
- `Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-source-capability.receipt.json`
- `Tools/ProjectAudit/Test-Artist31470ReconstructedSourceCapability.ps1`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`의 focused check 등록

Runtime C++, Material, Geometry, Resources 파일은 수정하지 않았다.

## 독립 감사 corrective

첫 snapshot의 독립 감사 BLOCK 6건을 재현했다. corrective는 15 exact variant별 recursive closed schema,
148 property consumption receipt, 65 distribution의 explicit unpinned fidelity, exact owner/global ID gate,
tracked JSON canonical hash, 5개 execution dependency와 family semantic digest를 추가했다. Public validator에서는
`compare_rebuilt=False`를 제거하고 build 내부 private validation에서만 사용한다. Reviewer가 지목한
`alphascaleoverlife`, cylinder `surfaceonly/positive_y/negative_z/positive_z` mutation은 모두 output을 변경한다.

## 실행한 검증

### Python unit 및 mutation

`python -B Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_source_capability.py`

- 25/25 PASS
- 7 family 각각의 실제 source property/distribution/default mutation이 numeric output을 변경함
- reviewer field/schema/identity/ownership/EOL/dependency/default provenance 반례 PASS
- strict duplicate key, bool/float version, raw/self/tool/implementation hash mutation 거부
- unknown class, duplicate occurrence, missing/non-finite sample, blocker 제거, execution/Product 승격 거부
- input과 output을 함께 재봉인한 stored row mutation도 frozen source projection과 불일치하여 거부

### Focused ProjectAudit

- shallow PASS
- deep PASS
- deep는 기존 Source acquisition과 Custom handler의 설치 binary/package identity 검사까지 재실행함

최종 deep 출력:

```text
PASS: Artist F 31470 reconstructed Source capability mode=deep families=7 occurrences=29 properties=148 distributions=65 samples=87 unknown=0 ownerless=0 genericFallback=0 sourceExact=0 execution=false product=false
```

### 정적 검사

- generator `--check` PASS
- JSON parse PASS
- `git diff --check` PASS
- Python cache 산출물 없음

## 미검증과 다음 단계

- 독립 reviewer의 frozen diff 판정은 아직 받지 않았다.
- Runtime typed materializer가 7 variant를 실제 opcode/typed payload로 소비하는지는 아직 미구현이다.
- 399 module/629 distribution 전체 compile, 6 renderer, 35/35 Runtime admission은 이 checkpoint 범위가 아니다.
- Product admission과 이미지/육안 검증은 수행하지 않았다.

따라서 현재 상태는 `CAPABILITY_EVIDENCE_READY_FOR_INDEPENDENT_REVIEW`, Runtime execution은 BLOCK,
Product는 false다.

## 2026-08-11 Decal current-CDO corrective

### 재현과 수정 결과

기존 generator는 세 `EF_DECAL_DESCRIPTOR` occurrence의 Source semantics가 모두 다음 값을 보존해도
`decal.yawOnly=false`를 독립적으로 하드코딩했다.

- exact source literal: `nearPlane=-300.0` (3/3)
- current EFGAME CDO와 implicit default: `farPlane=300.0`, `defaultSize=50x50`,
  `blendRange=100x100`, `bonlycalcrotationyaw=true`, `bsupported3ddrawmode=true` (3/3)

Corrective generator는 root current-default evidence, exact CDO object/class/export/record/property set,
세 occurrence의 unique implicit-default owner/provenance/value set을 compositional하게 대조한다. Decal
implementation은 v2로 올렸고 closed typed input/output schema와 9개 numeric sample에는
`yawOnly=true`, `supports3dDrawMode=true`를 추가했다. output frustum은 세 행 모두
`[-300, 300, 50, 50, 100, 100]`이다. source fidelity와 blocker는 바뀌지 않았고 runtime/Product는 false다.

### 일곱 family explicit-default 충돌 전수 조사

| capability family | blocked occurrence | Source semantics implicitDefaults overlap | 판정 |
|---|---:|---|---|
| seeded | 11 | 없음; fixed oracle context와 seed fallback만 사용 | 충돌 없음 |
| cylinder-spin | 5 | 없음; fixed oracle context와 explicit literal fallback만 사용 | 충돌 없음 |
| ground | 2 | 없음; fixed ground query threshold만 사용 | 충돌 없음 |
| decal | 3 | Decal current EFGAME CDO 3행 | 기존 yaw false 충돌 수정, 3/3 exact join |
| light | 1 | Light current default chain 1행 | `light.*` explicit default 없음; typed pointLightAdapter field 직접 소비 |
| velocity | 4 | 없음; fixed input velocity context만 사용 | 충돌 없음 |
| ef-vector-multiply | 3 | 없음; fixed base vector context만 사용 | 충돌 없음 |

### corrective 검증 상태

- Python unit/mutation: 29/29 PASS
- 기존 denominator 유지: family 7, occurrence 29, property 148, distribution 65, sample 87
- Decal exact: occurrence 3, sample 9, near/CDO/implicit-default/schema/output 3/3 PASS
- coordinated resealed yaw/default/sample mutation REJECT
- sourceExact 0, runtime execution 0, Product 0
- generator `--check` PASS
- focused ProjectAudit shallow PASS
- focused ProjectAudit deep PASS; 설치 binary/package identity prerequisite 재검증 포함
- receipt/PLAN/RESULT JSON·UTF-8 parse PASS
- `git diff --check` PASS
- Python cache 산출물 없음

Corrective receipt self SHA는 `5d1b827cb3bbd9ac4ddbf3c3dd976a584d9251017e9979b5aae771e8e9a1ae1f`,
Decal implementation SHA는 `bfef25b543f0385fff3df382ce5aa187da07a72de27065414786cf1ea9c58d35`,
explicit defaults SHA는 `1ac952d51761a1db3b4d62578fc72c3d7f1ecad45beba18fdee87e090a47491f`다.
독립 frozen review 전에는 integration authority로 승격하지 않는다.
