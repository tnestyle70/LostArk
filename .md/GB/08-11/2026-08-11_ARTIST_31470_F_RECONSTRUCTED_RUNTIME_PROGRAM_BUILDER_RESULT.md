# Artist F 31470 Reconstructed Runtime Program Builder 결과

날짜: 2026-08-11

브랜치: `codex/artist-f-reconstructed-runtime-program-builder-v1`

## 판정

R2 Python offline typed materializer와 immutable candidate는 focused 자동 mutation 검증을 통과했다.
현재 SHA의 독립 frozen-tree 재감사는 대기 중이며 그 전에는 최종 PASS로 승격하지 않는다.
이 결과는 R3 C++ parser/executor 또는 runtime/Product 승인이 아니다. candidate admission은
`sourceExact=false`, `runtimeExecution=false`, `product=false`를 유지한다.

Material texture runtime binding 72/72의 독립 PASS 원본 commit
`1a0b1a6834d562dac02db4f57dda54644d75695b`을 13번째 input authority로 직접 결합했다.
이 receipt는 exact-DDS deployment 원본 `01b8b8a8bace09a3576f116771daf4859aa485a3`을
transitively 결합한다. 통합 복사 commit은 evidence authority로 사용하지 않았다.

정확한 13번째 authority는 다음과 같다.

| 항목 | 값 |
|---|---|
| Material72 commit | `1a0b1a6834d562dac02db4f57dda54644d75695b` |
| tree | `84cc7ef8cde7a7cf5194b0ed2ccf56a45a927b57` |
| receipt blob | `1a917e44d5605e322e2c554db21573d856b05874` |
| tracked/raw SHA-256 | `87a28be564308117ac666206382c94ce5ee2bf37a47111cbef717994a0266077` |
| canonical JSON SHA-256 | `6b83552e90b18500fec831939087c57e368225878982c0cd71de431e03a1d27b` |
| receipt self SHA-256 | `3e722cf02085497c63083fbf51161ff5fd6670be91607737863b9c4019e55b48` |
| independent approval projection | `be605d29418bb44e7d86c3ecd61a1b9c6807dded201875075532cf961133a876` |

## 구현 결과

- builder: `Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py`
- candidate: `Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json`
- unit: `Tools/EffectPipeline/test_build_artist_31470_reconstructed_runtime_program.py`
- focused audit: `Tools/ProjectAudit/Test-Artist31470ReconstructedRuntimeProgram.ps1`
- ProjectAudit 등록: `effect.artist-31470-reconstructed-runtime-program`

review-ready corrective checkpoint:

```text
bytes                 15,072,141
candidate raw SHA-256 72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849
program SHA-256       618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b
input artifacts       13
emitters/schedules    35/7
modules               399 = source handler 370 + reconstructed handler 29
properties/leaves     1,434/1,572
literals              1,590
distributions         629 = 612/8/5/1/3
seed/default/light    14/14/8
Material              family23/recipe27/occurrence34/policy255
Material texture      resolved72/unresolved0 = 68 cook + 4 deployment, preview slot57
Geometry              carrier7/use13
sourceExact           0
runtime/Product       false/false
```

## 닫힌 계약

- 13개 입력 전체를 exact Git commit/tree/blob, schema, tracked LF text, canonical JSON/self SHA의
  ordered authority table로 재도출해 고정했다.
- 35 occurrence를 candidate element ID가 아니라 exact `sourceNode`로 결합했다.
- action-cue recipe의 full precision schedule 7행을 35 emitter에 4/1/15/12/1/1/1로 역결합했다.
- Required/Spawn/Lifetime typed rows에서 delay/duration/loop/lifetime과 burst 31행을 재도출했다.
- standard Lifetime 34행과 seeded Lifetime 1행을 분리해 owner/seed를 검증했다.
- reconstructed Source 29행은 implementation tuple, property consumption, literal/distribution/seed/
  ActionCue projection과 numeric sample 87개를 직접 소유한다.
- reconstructed Source 29행을 frozen capability receipt의 정확한 행과 역결합해 family/implementation/
  source row/nested projection의 coordinated reseal을 거부한다.
- distribution field provenance의 upstream empty digest 908개는 program-owned typed value binding SHA로
  다시 묶었다.
- Material family sample 92개, recipe sample 108개, D3D descriptor 107개, SRV 72개를 program 안에
  typed payload로 보존했다.
- 승인 Source29/Material255/Arithmetic23/Geometry7 projection을 frozen approval receipt와 exact join했다.
- Geometry 7 carrier의 candidate bytes, payload/provenance, pre-scale, submesh/channel/bounds tuple과
  13 Mesh use를 결합했다.
- renderer Detail은 diagnostic-only로 격리했다. ScreenPost1, Light1, Decal3, Ribbon1은 별도 typed
  adapter를 소유한다.
- Ribbon point cap은 임의 64 clamp를 제거하고 typed TypeData max 500을 사용한다. emitter particle
  pool peak 594는 별도 값으로 유지한다. source에 없는 orientation은
  `CAMERA_FACING_SINGLE_SHEET_RECONSTRUCTED_V1` 정책과 R5 probe blocker로 명시했다.
- Decal yaw CDO와 capability output은 모두 true로 닫혀 이전 conflict blocker를 제거하고 R5 runtime
  probe blocker만 유지했다.
- Material texture 72행은 모두 runtime asset resolved다. frozen receipt의 source status는
  `RESOLVED_EXACT_RUNTIME_COOK_RECEIPT` 68행과
  `RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT` 4행으로 보존했다. 후자에는 proposal와
  deployment row ID/SHA를 함께 보존하고, 전체 72행 projection SHA
  `df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c`로 고정했다.
  안전한 경로인 배포 4행 runtime asset A/B 교환도 거부한다.
- blocker union/admission blockers는 모든 owned row와 nested adapter/policy blocker 및 필수 global gate에서
  재도출한다. 해소된 `MATERIAL_TEXTURE_RUNTIME_ASSET_UNRESOLVED`와
  `EXACT_DDS_TRANSACTIONAL_DEPLOYMENT_PENDING`만 제거했다. R4 texture consumer와 기존 Material sampler/
  source-fidelity blocker는 유지한다. blocker 소유 field 6,108개/token occurrence 6,670개의 exact ordered
  owner/path/value projection을 SHA `d47b33c183d863761470585348dc4da2cc48e69a12b3594515444d20e5c04650`로
  hard-pin해 owner 목록과 union을 함께 세탁할 수 없게 했다. emitter Source/compiler
  identity 35행도 frozen Source evidence에서 재도출한다.

## 검증 결과

실행 완료:

```text
python -B Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py --check
PASS bytes=15,072,141 sha=618d5684... runtime=false product=false

python -B Tools/EffectPipeline/test_build_artist_31470_reconstructed_runtime_program.py
Ran 12 tests / OK

powershell -ExecutionPolicy Bypass \
  -File Tools/ProjectAudit/Test-Artist31470ReconstructedRuntimeProgram.ps1
PASS

git diff --check
PASS
```

전체 `Invoke-ProjectAudit.ps1`은 이 R2 focused check가 아닌 기존 저장소 baseline 12건으로 exit 1이었다.
대표 외부 항목은 map roots/project DataFiles 불일치, G09 미완료, 미빌드 Geometry harness,
four-class Artist/31210 stage count 불일치, 로컬 Resources character model 누락이다. 실행 뒤 R2 범위 밖
tracked/untracked 파일 증가는 없었다.

영구 mutation tests는 bool/int/string laundering, unsafe path, approval row coordinated reseal, schedule
rounding, default duration zero, burst sentinel, seeded Lifetime, capability receipt family/source-row drift,
D3D/SRV, geometry tuple/cache, texture owner/배포 4행 A-B swap, status/basis/proposal/deployment identity
위조·누락, 구 68+4 receipt A/B, 13 input authority field drift, blocker 공동 제거,
emitter Source/LOD identity와 Ribbon cap/orientation을 포함한다. accepted mutation은 0건이다.

## 남은 경계

1. exact review-ready candidate를 독립 frozen-tree audit한 뒤 검증 commit으로 고정한다.
2. final candidate SHA로 C++ parser lane을 동결한다.
3. R3 executor에서 72 runtime asset과 72 sampler/SRV descriptor의 exact reverse join을 소비한다.
4. R4 Material/Geometry resource consumer, R5 six renderer probes, R6 35 occurrence manual eye
   validation, R7 transaction/build regression을 순서대로 수행한다.

자동 이미지 비교는 사용하지 않았다. 실제 눈 검증은 R6 runtime stage가 연결된 뒤 사람이 수행한다.
