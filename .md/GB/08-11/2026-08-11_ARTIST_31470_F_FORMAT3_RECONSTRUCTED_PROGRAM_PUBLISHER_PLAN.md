# Artist F 31470 Format-3 Reconstructed Program Publisher 계획

날짜: 2026-08-11

브랜치: `codex/artist-f-format3-publisher-v1`

기준 commit: `eacb58bda2315e858c562677bbf38c17d5d3e785`

## 목표와 완료 경계

R2에서 동결한 Artist 31470 reconstructed runtime candidate를 Effect runtime catalog format 3에
직접 내장한다. 제품 런타임은 외부 candidate 파일을 다시 읽지 않고 catalog 안의
`candidateUtf8Json`만 소비할 수 있어야 한다.

이 publish는 복원 프로그램의 검사·준비 경로만 연다. candidate가 가진 72개 blocker와
`sourceExact=false`, `runtimeExecution=false`, `product=false` 판정을 그대로 보존한다. 기존 generic
`IMMUTABLE_COMPILED_IR`의 execution authority로 위장하지 않으며, Authored/Assembly/Compiled artifact나
generic compiler receipt를 합성하지 않는다. PlayerSkills `effectId`, animevent, Playback, Presentation,
Renderer와 C++는 이번 브랜치에서 수정하지 않는다.

완료 결과는 source catalog 102행, runtime catalog format 3의 Effect 102행/Component 555행이며,
신규 한 행만 `IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM`이다.

## 정본과 고정 identity

| 항목 | 값 |
|---|---|
| effectAssetId | `effect.artist.skill.31470` |
| candidate builder commit | `a85b8b41afb2f2a51bceafa55d06bf0937b1a245` |
| candidate builder tree | `384ed35ca808ab9a71a4edb703ca4d9121b48c18` |
| candidate Git blob | `345ab15bbb76648a650eaa854f18c4cd63cb1556` |
| candidate raw SHA-256 | `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849` |
| candidate decoded byte count | `15,072,141` |
| resourceBindingHash | `df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c` |
| inputArtifactCount | `13` |
| inputArtifacts ordered SHA-256 | `938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa` |
| programId | `effect.artist.skill.31470.reconstructed-approved-v1` |
| programVersion | integer `1` |
| programSha256 | `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b` |

candidate 한 파일에만 `.gitattributes`의 `text eol=lf`를 적용한다. `git check-attr`의 exact rule과
`core.autocrlf=true` 별도 clean checkout bytes가 모두 UTF-8/no-BOM/LF-only 15,072,141 bytes와
frozen raw SHA를 만족해야 한다. 다른 JSON에 broad EOL 규칙을 추가하지 않는다.

## runtime entry 계약

신규 entry의 exact ordered 10 keys는 다음과 같다.

```text
payloadKind,effectAssetId,artifactRevision,compilerRevision,sourceExact,
runtimeExecutionAdmission,productAdmission,publishReceiptSha256,publishReceipt,
reconstructedRuntimeProgram
```

고정값은 `payloadKind=IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM`, `artifactRevision=1`,
`compilerRevision=artist31470.reconstructed-runtime-program-link-v1`이고 세 admission gate는 모두
JSON boolean `false`다.

`reconstructedRuntimeProgram`의 exact ordered 16 keys는 다음과 같다.

```text
schema,formatVersion,encoding,effectAssetId,candidateBuilderCommitId,
candidateBuilderTreeId,candidateBlobId,resourceBindingHash,inputArtifactCount,
inputArtifactsOrderedSha256,programId,programVersion,programSha256,
candidateRawSha256,candidateByteCount,candidateUtf8Json
```

schema는 `lostark.effect-reconstructed-runtime-program-link`, formatVersion은 integer `1`, encoding은
`UTF8_JSON_EXACT`다. `candidateUtf8Json`을 UTF-8로 다시 encode한 bytes가 frozen byte count/raw SHA를
만족하고, 파싱한 candidate의 program identity와 false admission도 다시 일치해야 한다. runtime
catalog에는 candidate의 외부 경로를 기록하지 않는다.

## publication provenance receipt

`publishReceipt`는 execution authority가 아니라 publication provenance다. exact ordered 25 keys는
다음과 같다.

```text
schema,formatVersion,receiptRole,payloadKind,effectAssetId,artifactRevision,
compilerRevision,sourceExact,runtimeExecutionAdmission,productAdmission,
candidateBuilderCommitId,candidateBuilderTreeId,candidateBlobId,resourceBindingHash,
inputArtifactCount,inputArtifactsOrderedSha256,programId,programVersion,programSha256,
candidateRawSha256,candidateByteCount,reconstructedRuntimeProgramSha256,
toolDependencies,receiptSha256Domain,receiptSha256
```

schema는 `lostark.effect-reconstructed-runtime-program-publish-receipt`, receiptRole은
`PUBLICATION_PROVENANCE_ONLY_NOT_EXECUTION_AUTHORITY`다. toolDependencies는 candidate builder,
catalog validator, PowerShell publisher 세 파일을 exact ordered row로 기록한다. 각 row의 keys는
`role,path,hashDomain,sha256`, hashDomain은 `TRACKED_SOURCE_EOL_CANONICAL_TEXT`다. UTF-8 BOM은
거부하고 checkout LF/CRLF 차이만 LF로 정규화하여 Git text identity와 결합한다.

`reconstructedRuntimeProgramSha256`은 exact 16-key link의 canonical JSON SHA다.
`receiptSha256`은 마지막 self field를 제외한 첫 24-key canonical JSON SHA이고,
outer `publishReceiptSha256`은 self field를 포함한 full 25-key receipt canonical JSON SHA다.
public reconstructed receipt/entry validator는 언제나 현재 tracked tool 세 파일의 exact canonical SHA를
대조한다. current-tool 대조를 끄는 public/default 경로를 두지 않는다.

## 구현 순서와 rollback

1. Python validator가 candidate bytes, Git commit/tree/blob, input artifact projection, program identity와
   false gate를 모두 검증한다.
2. 같은 validator가 exact link/receipt/entry를 생성한 뒤 독립 strict validator로 round-trip한다.
3. `Publish-Effects.ps1`은 source catalog의 exact 3-key reconstructed row가 예약 ID
   `effect.artist.skill.31470`인지 먼저 확인하고 안전한 Data 경로로 resolve한다. Python prepare 결과의
   `effectAssetId`도 source row와 exact-equal해야 하며 그 뒤에만 private stage에 format-3 catalog를
   만든다. 같은 ID는 source와 runtime catalog의 payload dispatch 전에 전역 예약하므로 legacy 또는
   generic compiled payload로 fallback할 수 없다.
4. PowerShell이 읽는 모든 file-backed authority JSON은 `ConvertFrom-Json` 전에 strict UTF-8/no-BOM
   raw parser를 통과한다. parser는 object/array/value 전체를 순회하고 JSON string escape를 decode한
   key를 ordinal set에 보존하여 같은 key와 escaped-equivalent duplicate key를 모두 거부한다.
5. staged catalog를 legacy/generic/reconstructed payload별로 재검증한 뒤에만 destination을 atomic
   replace한다. 실패하면 이전 output bytes를 보존하고 temp/backup residue를 남기지 않는다.
6. 기존 101 legacy row와 generic 17-key 계약은 변경하지 않는다.

tracked authoring JSON의 `sourceDocumentFileSha256`도 Git checkout EOL 차이에 흔들리지 않도록
UTF-8/no-BOM LF-canonical text hash로 검증한다. binary/resource/raw artifact identity는 계속 exact raw
bytes hash다.

## 검증 계획

1. Python unit에서 exact 10/16/25/4-key order와 integer token, candidate raw/program/resource/asset
   mutation, coordinated reseal, tool identity reseal, candidate missing-after-publish, source row의 31471
   identity substitution, raw duplicate owner key와 escaped-equivalent duplicate key, mixed primitive token을
   지난 deeper array/object duplicate, reserved 31470 legacy/generic payload, valid generic source fallback,
   direct public-validator coordinated tool reseal, 두 atomic fault rollback을 검증한다.
2. legacy Effect pipeline에서 authoring LF/CRLF parity, BOM 거부, assembly를 갱신하지 않은 semantic
   mutation 거부와 기존 publish/rollback 회귀를 검증한다.
3. `git check-attr`와 `core.autocrlf=true` 별도 checkout으로 candidate exact LF identity를 검증한다.
4. 실제 source/runtime catalog를 Validate와 Publish하고 format 3, 102 Effects, 555 Components,
   reconstructed entry 1개, false gates, embedded bytes/hash, external candidate path 부재를 확인한다.
5. Python/PowerShell parse, JSON parse, focused ProjectAudit, `git diff --check`를 실행한다.
6. 전체 ProjectAudit의 기존 외부 baseline 실패는 이번 corrective와 분리해 기록한다. C++ build와
   runtime 눈 검증은 catalog consumer 브랜치가 이 frozen 계약을 연결한 뒤 수행한다.
