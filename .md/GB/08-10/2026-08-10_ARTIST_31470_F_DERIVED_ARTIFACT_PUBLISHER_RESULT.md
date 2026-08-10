# 2026-08-10 Artist 31470 F Derived Artifact Publisher Result

기준 commit: `ccb60a519d1c9e2fb955652e862561b9b670438e`

branch: `codex/artist-f-derived-publisher-v2`

## 완료 상태

G05-P1 code-only 계약을 구현했다. 실제 Artist F v13 authoring, Assembly, compiled artifact, receipt,
runtime catalog entry는 생성하지 않았고 `Data/Effects` 변경도 없다. synthetic fixture의
`executionAdmission=true`만 검증했으며 모든 compiled artifact/receipt/runtime entry의
`productAdmission`은 `false`, publication state는 `CODE_ONLY_NOT_ADMITTED`다.

구현된 경계는 다음과 같다.

- v14 Source는 `readOnly=true`, `drawable=false`인 source identity로만 v13 carrier에 연결한다.
- v13 carrier는 exact-key identity-only 문서이고 `elements=[]`다. particle/module/material/runtime field를
  넣으면 거부한다.
- Assembly v2도 component cue를 소유하지 않는 identity carrier다.
- `sourceContractHash`, `sourceSemanticClosureHash`, `geometryContractHash`, `materialContractHash`,
  `resourceBindingHash`, `compilerInputHash`의 canonical field order를 carrier, Assembly, artifact,
  receipt, runtime entry에서 다시 검증한다. 각 upstream JSON은 raw SHA와 EOL/whitespace 독립 canonical
  SHA를 함께 검증하고 six-hash identity에는 canonical SHA만 사용한다.
- compiled receipt는 carrier/Assembly/artifact raw SHA, compiled IR canonical SHA, compiler/artifact
  revision, zero artifact/execution blocker count와 execution admission을 결합한다. generator/schema/
  publisher도 repository-relative path, raw observed SHA, canonical required SHA와 hash domain으로 pin한다.
- runtime catalog format 3 derived entry는 `IMMUTABLE_COMPILED_IR` artifact/receipt만 embed한다. v13
  carrier/Assembly raw payload는 entry에 없으며 sole semantic authority field를 변경할 수 없다.
- `Publish-Effects.ps1`은 v13에 exact `Effects/Compiled` artifact/receipt pair를 요구하고 v14 direct
  publish, partial pair, unsafe path, cross-bundle swap을 거부한다.
- derived entry가 없는 기존 publish는 format 2 shape를 그대로 보존한다. derived entry가 있는
  transaction만 mixed legacy/compiled format 3을 stage한다.
- builder 네 파일과 publisher runtime catalog는 parse -> validate -> private stage -> commit 순서를
  사용한다. 실패 시 기존 target/catalog bytes를 보존하고 transaction temp를 제거한다.

production tool과 schema에는 Artist ID, `31470`, `7/35/399/629` 분모 hardcode가 없다.

## 변경 파일

```text
Tools/EffectPipeline/build_effect_derived_artifact.py
Tools/EffectPipeline/test_build_effect_derived_artifact.py
Tools/EffectPipeline/Schemas/lostark.effect-derived-artifact-contract.schema.json
Tools/EffectPipeline/Publish-Effects.ps1
Tools/EffectPipeline/Test-EffectPipeline.ps1
Tools/ProjectAudit/Test-EffectDerivedArtifactPublisher.ps1
Tools/ProjectAudit/Invoke-ProjectAudit.ps1
.md/GB/08-10/2026-08-10_ARTIST_31470_F_DERIVED_ARTIFACT_PUBLISHER_PLAN.md
.md/GB/08-10/2026-08-10_ARTIST_31470_F_DERIVED_ARTIFACT_PUBLISHER_RESULT.md
```

Client/Engine runtime compiler, executor, renderer, catalog C++와 `Client/Bin/Resources`는 수정하지 않았다.

## 실제 검증

```text
python -B Tools/EffectPipeline/test_build_effect_derived_artifact.py -v
  PASS: 14/14
  zero blocker build, canonical identity order, blocker no-write, raw/canonical hash mutation,
  LF/CRLF equivalence, BOM/duplicate-key/path escape, tool dependency mutation, revision/type mutation,
  identity-only exact keys, compiled A/carrier B swap, partial commit rollback,
  format3 mutation, v14 direct reject, missing pair/file, Publisher no-overwrite를 actual CLI로 검증

powershell -NoProfile -ExecutionPolicy Bypass \
  -File Tools/ProjectAudit/Test-EffectDerivedArtifactPublisher.ps1
  PASS: derived Effect artifact publisher schema tests=14 execution=true product=false

powershell -NoProfile -ExecutionPolicy Bypass \
  -File Tools/EffectPipeline/Test-EffectPipeline.ps1
  PASS: legacy format2 shape와 v5-v12 Assembly/resource/hash/budget/rollback 회귀

Python AST 2/2, JSON schema parse, PowerShell parse 2/2
git diff --check PASS
```

전체 ProjectAudit는 94 check 중 새 `effect.derived-artifact-publisher`가 PASS했다. 전체 exit는 기준 tree에
이미 기록된 다음 9개 실패 때문에 1이다.

```text
projects.data-source-visibility
effect.g09-authoring-world-runtime-boundary
effect.g09-cross-document-contract
effect.artist-31470-source-contract
effect.artist-31470-material-evidence-contract
effect.artist-31470-wmodel-geometry-contract
effect.wfx-component-assembly
effect.representative-authored-readiness
effect.four-class-authored-clip-product-exact101
```

현재 tracked catalog에 대한 `Publish-Effects.ps1 -Mode Validate`도 기존
`effect.artist.skill.31000.ba1` Assembly source file hash mismatch에서 fail-closed한다. 이 lane의
synthetic legacy와 derived publisher 회귀는 모두 PASS했으며 기존 Product output을 교체하지 않았다.

## 다음 consumer 경계

G06은 runtime catalog format 3의 `payloadKind=IMMUTABLE_COMPILED_IR` entry에서 embedded
`compiledArtifact`와 `compiledReceipt`만 parse해야 한다. v13 carrier와 Assembly는 identity/hash 확인용이며
Playback/Renderer 입력이 아니다. G05-S/G/M final hash와 actual Artist F output은 모든 final hash가 동결된
Gate 10B에서만 builder request로 공급한다.
