# Artist 31470 F Reconstructed Runtime Program Parser Result

Date: 2026-08-11

Branch: `codex/artist-f-reconstructed-runtime-program-parser-v1`

Base commit: `d705a9dc633af102456553ed17fdb43eda520883`

Status: implementation and automated parser verification PASS; runtime execution,
Product admission, and manual visual validation remain intentionally blocked.

## Frozen authority consumed

The parser accepts one final R2 authority only:

- builder commit `a85b8b41afb2f2a51bceafa55d06bf0937b1a245`;
- builder tree `384ed35ca808ab9a71a4edb703ca4d9121b48c18`;
- candidate bytes `15,072,141`;
- candidate raw SHA-256
  `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849`;
- program SHA-256
  `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b`;
- blocker ownership `6,108` fields, `6,670` token occurrences, projection
  `d47b33c183d863761470585348dc4da2cc48e69a12b3594515444d20e5c04650`;
- 72 resolved texture bindings: 68 exact runtime-cook receipts and four
  reconstructed exact DDS deployment receipts.

`build_artist_31470_reconstructed_runtime_program.py --check` reproduced
`bytes=15072141`, the frozen program SHA, `runtime=false`, and `product=false`.
There is no accepted provisional 68/4 profile, identity range, or schema
fallback.

## Implemented contract

`EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM` now owns immutable typed rows for the
entire generated execution payload. Emitters preserve transforms, timing,
bursts, deterministic random policy, attachment and anchor requests, renderer
configuration, adapters, and action-cue inputs. The document also owns typed
distribution tables, curves, samples and parameter branches; seed/default and
PointLight values; material evaluator families, recipes, policy values,
sampler/D3D/SRV descriptors and texture receipts; and geometry carrier,
submesh, pre-scale, bounds, metadata, and use records. Consumers can resolve
stable IDs without reparsing raw JSON. No public `SourceRecipe`, Effect Detail,
or raw JSON pointer is retained.

The parser enforces bounded UTF-8 with no BOM, duplicate-key rejection,
20 MiB/64-depth/1,000,000-value limits, exact root/row/nested key order,
closed enum and nullability domains, finite numbers, integer-versus-float token
domains, normalized Resources-relative paths, and canonical row/section/root
seals. It stages locally and changes the caller's immutable shared pointer only
after every contract succeeds.

Reverse validation covers global ID uniqueness, per-owner order, handler
kind/class/variant, module/property/distribution ownership, emitter timing and
renderer projections, attachment anchors, geometry use and TypeDataMesh
ownership, material occurrence/input/policy/texture joins, and texture source
A/B identities. Blocker ownership, blocker union, summary, and admission are
rederived rather than trusted.

All seven action schedules are pinned by an aggregate SHA-256 over exact order,
cue, occurrence, system, source receipt event index, global time, duration, and
source cue row SHA. Each schedule ID is rederived from its executable
projection, closing the prior gap where time and duration were merely parsed.

## Test-only semantic boundary

The prior public semantic-test entry was removed. Production exposes only the
frozen-identity parser, and the production Client project does not define
`LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS`.

The Harness target alone defines that macro and receives a private validator
that stages into a discarded local pointer. It cannot replace a caller-visible
program. Its resealed fixtures preserve builder insertion order while
recomputing canonical row, section, root, raw, and program identities, so each
negative reaches the asserted parser stage rather than being intercepted by a
raw-SHA or key-order gate.

The canonical negative matrix covers every action-schedule executable field,
integer/float token swaps, nested extra/missing keys, enum and boolean domain
attacks, duplicate ID/order, handler and owner reassignment, timing, random,
renderer, distribution, D3D material descriptor, texture A/B and deployment
identity, geometry carrier, artifact identity, blocker removal, admission
promotion, and builder A/B identity swap. Each rejection preserves the prior
pointer and program hash. The runner stops constructing further 15 MiB fixtures
after the first mismatch.

Two earliest-gate results are intentional and asserted:

- a geometry-use emitter reassignment is rejected first by the shared
  renderer/texture reverse-owner gate; the independent geometry-carrier byte
  size attack reaches the frozen geometry projection gate;
- `runtimeExecution=true` is rejected by the root contract/seal gate because
  the frozen schema itself requires runtime and Product admission to remain
  false.

## Automated verification

The exact final source snapshot produced the following results:

- `ClientFrontendHarness.vcxproj` x64 Debug `/m:1`: compile/link PASS;
- Debug `--effect-reconstructed-runtime-program <final-candidate>`: 4/4 PASS,
  `failures : 0`;
- Debug `--effect-runtime-authority`: 7/7 PASS, `failures : 0`;
- `ClientFrontendHarness.vcxproj` x64 Release `/m:1`: compile/link PASS;
- Release `--effect-reconstructed-runtime-program <final-candidate>`: 4/4
  PASS, `failures : 0`;
- Release `--effect-runtime-authority`: 7/7 PASS, `failures : 0`;
- focused `Test-EffectRuntimeAuthority.ps1` with the Release harness and final
  candidate: PASS;
- changed Client/Harness project and filter XML: parse PASS;
- `git diff --check`: PASS;
- conflict-marker scan: PASS;
- MSBuild, compiler, linker, and Harness process count after verification: 0.

Both configurations emitted the repository's existing `EngineSDK` C4828
encoding warnings. The Harness post-build command also reported that `pwsh.exe`
was unavailable, but MSBuild, compiler, and linker exited successfully; the
same audit was therefore run explicitly with Windows PowerShell and passed.

The full `Invoke-ProjectAudit.ps1` was executed and exited 1 with 12 unrelated
repository/environment baselines. They include map/runtime-root and project
data visibility mismatches, missing G09/current cross-document runtime paths,
an unbuilt WModel geometry harness, four-class authored rollout failure at
`Artist/31210`, and missing runtime actor resources. Its Artist F source
capability unit set and other reported unit suites passed, and the full audit
did not add files to this lane's diff. None of the 12 failures is introduced by
the parser files or changes the focused parser PASS.

## Remaining boundary

This result materializes and validates data; it does not execute particles or
draw them. Admission remains `sourceExact=false`, `runtimeExecution=false`, and
`product=false`. R3 must consume these typed vectors through stable IDs, then R4
through R7 must bind geometry/materials, implement renderer families, run all
35 occurrences, perform human-eye validation, and freeze the product path.
No manual in-game or visual PASS is claimed here.

The worktree remains an uncommitted review snapshot. No files were staged,
committed, or pushed by this lane.
