# 2026-08-10 Artist 31470 F Derived Artifact Publisher Plan

기준 commit: `ccb60a519d1c9e2fb955652e862561b9b670438e`

branch: `codex/artist-f-derived-publisher-v2`

상위 정본: `2026-08-10_ARTIST_31470_F_COMPLETE_RESTORATION_IMPLEMENTATION_PLAN.md`의
`G05-P1. Derived Artifact and Publisher Schema Code`

## 목표와 금지 경계

이 lane은 실제 Artist F v13 authoring, Assembly, compiled artifact를 생성하지 않는다. Source semantic,
Geometry, Material, ResourceBinding, CompilerInput의 최종 hash도 요구하지 않는다. synthetic
zero-blocker fixture만 사용해 다음 generic 계약을 구현한다.

- read-only/non-drawable v14 Source Contract를 가리키는 `lostark.effect-authoring` v13 identity carrier
- 같은 six-hash identity를 보존하는 `lostark.effect-assembly` v2 identity carrier
- immutable compiled IR wrapper와 carrier/Assembly/artifact bytes를 결합하는 receipt
- compiled artifact만 runtime semantic authority로 인정하는 runtime catalog format 3
- verified v13 carrier + Assembly + compiled artifact + receipt pair가 아니면 publish하지 않는 publisher
- blocker, corrupt/missing/hash swap/revision mismatch에서 기존 Product output을 byte-for-byte 보존하는 transaction

v13 carrier와 Assembly의 raw element/component 내용은 runtime semantics 권위가 아니다. G06 runtime
compiler/executor 파일은 이 lane에서 수정하지 않는다. 이미지 또는 육안 검증은 하지 않는다.

## G05-P1A. Contract schema와 deterministic builder

새 `Tools/EffectPipeline/build_effect_derived_artifact.py`가 다음 schema를 manual fail-closed validator로
구현한다.

```text
lostark.effect-derived-build-request / formatVersion 1
lostark.effect-derived-identity / formatVersion 1
lostark.effect-authoring / version 13 / DERIVED_IDENTITY_CARRIER
lostark.effect-assembly / version 2 / DERIVED_IDENTITY_CARRIER
lostark.effect-compiled-artifact / formatVersion 1
lostark.effect-compiled-artifact-receipt / formatVersion 1
lostark.effect-runtime-catalog / formatVersion 3
```

`derivedIdentity`는 아래 여섯 lower-case SHA-256을 exact name으로 보존한다.

```text
sourceContractHash
sourceSemanticClosureHash
geometryContractHash
materialContractHash
resourceBindingHash
compilerInputHash
```

build request는 여섯 input path와 expected raw/canonical SHA, compiled IR path/raw/canonical hash,
compiler revision,
artifact revision과 artifact-binding/execution blocker set을 가진다. builder는 path를 명시된 input root
안으로 제한하고 raw bytes와 canonical JSON semantics를 다시 hash한다. six-hash identity에는 canonical
hash만 사용하고 raw hash는 입력 파일 binding에 사용한다. `executionBlockers` 또는 `artifactBindingBlockers`가 하나라도
있으면 output staging 전에 거부한다. `productAdmission=true`는 네 blocker set이 모두 비고
`executionAdmission=true`일 때만 허용한다.

builder는 모든 payload를 메모리에서 생성·상호검증한 뒤 private staging directory에 쓰고, 전체
round-trip이 성공한 경우에만 네 target을 replace한다. 실패하면 staging을 삭제하고 기존 target bytes를
변경하지 않는다.

compiled receipt는 builder, JSON schema, `Publish-Effects.ps1`의 repository-relative path와 raw SHA,
canonical SHA, hash domain을 ordered dependency row로 보존한다. tracked Python/PowerShell은
UTF-8/no-BOM + LF canonical text, schema는 duplicate-key를 거부한 canonical JSON을 사용한다. raw SHA는
관찰값이고 checkout EOL이 달라도 canonical dependency가 같아야 한다.

## G05-P1B. Publisher와 runtime catalog format 3

`Tools/EffectPipeline/Publish-Effects.ps1`의 legacy v5-v12/Assembly v1 경로는 보존한다. catalog entry의
authoring document가 v13이면 다음을 추가 요구한다.

```text
compiledArtifactPath  Effects/Compiled/<effectAssetId>.compiled-effect.json
compiledReceiptPath   Effects/Compiled/<effectAssetId>.compiled-effect.receipt.json
```

publisher는 authoring carrier, scanned Assembly v2, compiled artifact, receipt를 새 validator에 함께
전달한다. validator가 source v14 read-only/non-drawable, six-hash equality, exact carrier/Assembly/artifact
raw SHA, compiler/artifact revision, compiled IR hash와 sole authority를 확인한 뒤에만 derived runtime
entry를 만든다. v14 문서를 catalog input으로 직접 지정하면 항상 거부한다.

derived entry가 하나라도 있으면 runtime root는 format 3이다. 기존 v5-v12 entry는
`LEGACY_ASSEMBLY_V1`, derived entry는 `IMMUTABLE_COMPILED_IR` payload kind를 사용한다. format 3 derived
entry는 compiled artifact와 receipt를 embed하며 v13 elements 또는 Assembly component cues를 runtime
semantics로 복사하지 않는다. actual tracked catalog에는 이번 lane에서 v13 entry를 추가하지 않으므로
현재 product output은 format 2로 유지된다.

publish는 unique temporary file에서 round-trip과 format validator를 끝낸 뒤 destination을 atomic
replace한다. validation, missing pair, hash/revision mismatch 또는 move failure 시 기존 destination을
복원하고 temporary/backup partial output을 남기지 않는다.

## G05-P1C. Synthetic fixture와 검증

`Tools/EffectPipeline/test_build_effect_derived_artifact.py`는 repository Data/Resources를 쓰지 않고 temp
root만 사용한다. 다음을 actual builder/publisher CLI로 검증한다.

- zero-blocker carrier/Assembly/artifact/receipt build와 stable hash round-trip
- six-hash field 하나씩 swap, compiled IR mutation, carrier/Assembly/artifact mutation 거부
- LF/CRLF canonical equality와 raw inequality, BOM/duplicate-key/semantic mutation 거부
- builder/schema/publisher dependency canonical hash mutation 거부
- compiler revision, artifact revision, effect identity mismatch 거부
- v14 direct publish, missing artifact/receipt, wrong path와 partial pair 거부
- execution blocker에서 output 0, product blocker와 admission 모순 거부
- failed build/publish가 pre-existing targets와 runtime catalog bytes를 보존
- format 3 validator가 raw v13 semantic payload를 runtime authority로 받지 않음
- production script와 schema에 Artist ID 및 7/35/399/629 count hardcode 0

focused PowerShell audit는 Python unit과 synthetic publisher CLI를 실행한다. `git diff --check`, JSON
schema parse, 현재 tracked catalog `-Mode Validate`가 완료 조건이다. Engine/Client full build는 public C++
변경이 없으므로 요청하지 않는다.
