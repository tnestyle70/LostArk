# Artist 31470 F Typed Execution Plan CPU Result

Date: 2026-08-11

Branch: `codex/artist-f-typed-execution-plan-cpu-v1`

Exact base commit: `8352a3718bcf0a3f9386e913a8a90807e70f8bd0`

Status: the second independent frozen review corrective is implemented and its
Debug/Release compiled verification passes. Runtime execution, Product
admission, renderer submission, PlayerSkills/animevents changes, manual
in-game validation, commit, and push remain intentionally false/not performed.

## Implemented contract

The new immutable execution plan is compiled only from the Catalog-owned
`EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION` and its exact typed Program. It owns
the Preparation and Program strongly and stores stable-ID keyed schedules,
emitters, modules, distributions, and seed policies plus deterministic ordered
ID projections. Handler and evaluator implementations are joined by exact
registry ID, kind, source class, variant, version, and SHA. Unknown values,
fallback handler identities, partial reverse membership, and invalid numeric
or seed semantics reject transactionally.

Distribution ownership accepts only the frozen shapes: 509 paths equal their
property path and 120 are bracket-qualified descendants such as
`dynamicparams[0].paramvalue`; all 629 have an exact property-ID reverse join
and zero broad-prefix owners. The frozen target projection seals ordered
distribution ID/module/property/target rows. A separate owner projection seals
the joined Property module/path and exact-versus-bracket classification with
SHA-256 `35b097...d88`, and every Property's ordered
`SemanticDistributionIds` must equal the actual filtered distribution vector.
Coordinated target, owner-path reclassification, extra, duplicate, and reorder
mutations therefore reach dedicated semantic gates and roll back. Timing
handler selection is also closed:
required/spawn and 34 ordinary lifetime modules use exact source handlers,
while the one approved seeded lifetime uses its exact reconstructed handler
and capability identity. Its exact emitter/module/property/distribution/seed
IDs, three handler IDs and row hashes, capability implementation/version/SHA,
family/input/output/default SHAs, and binding SHAs are preserved in an
immutable plan authority tuple. A coordinated ordinary/seeded full-object
authority swap reaches this tuple gate and rolls back. No other
selection/class/owner tuple is admitted.

The compiler freezes 7 schedules, 35 emitters, 399 modules, 629 distributions,
31 bursts, 34 ordinary plus one seeded lifetime, cap sum 1291, cap maximum 594,
Ribbon maximum 500, 14 seed policies, and distribution variants 612/8/5/1/3.
It re-evaluates all 1,852 frozen numeric samples and requires the exact
1,839 evaluator plus 13 parameter-branch split.

`CEffectPlayback` now stages the plan behind its existing opaque Catalog
Preparation. Compilation finishes before the Playback boundary and plan are
committed. Invalid restaging preserves both prior pointers. Legacy document
staging clears the plan. Production `Update`, `Seek`, and `Reset` remain behind
the existing false reconstructed execution gate and do not call the simulator.

## Inspection-only CPU simulation

The Harness-only simulator advances deterministic 60 Hz steps in exact stable
schedule/emitter order. It covers schedule time, emitter delay/duration/loop,
ordered bursts, typed rate and rate-scale, lifetime expiry, the fixed seeded
lifetime stream, operational caps, occurrence IDs, and immutable state/frame
projection hashes. The test packet records stable schedule/emitter/occurrence
IDs and does not cross into GPU/resource submission.

The macro `LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS` is present only in the
Harness Debug/Release configurations. The production Client compiles the plan
compiler but has no callable CPU simulator. EF multiply rows remain typed and
counted but are explicitly deferred and rejected if selected as timing
evaluators.

## Raw input and Product boundaries

The new execution source contains no `SourceRecipe`, `DetailTransform`,
`RendererRuntimeConfig`, or legacy detail descriptor consumption. The Harness
has a poison-only copy test for those raw fields and requires the same semantic
plan SHA.

The Program continues to require `runtimeExecution=false` and `Product=false`.
No PlayerSkills, animevents, Product spawn, resource creation, render-family
submission, D3D call, or on-screen effect was added. This result is CPU plan
and automated inspection evidence only, not an in-game Artist F PASS.

## Automated verification

- team LAN sync: configuration PASS; server-host listener absence non-blocking;
- dedicated worktree base/branch exactness: PASS;
- frozen candidate working bytes: 15,072,141, SHA-256 `72e417...9849`, CR 0;
- Client/Harness project and filter XML parse: PASS;
- focused static `Test-Artist31470TypedExecutionPlan.ps1`: PASS;
- pre-corrective ClientFrontendHarness x64 Debug/Release `/m:1` build and
  `--effect-reconstructed-execution-plan`: PASS, `failures : 0`;
- pre-corrective Client x64 Debug/Release full build: PASS;
- pre-corrective Debug dynamic focused audit with the canonical Resource root:
  PASS,
  `schedules=7 emitters=35 modules=399 distributions=629 rng=v1 fixedHz=60
  Product=false`;
- pre-corrective Release dynamic focused audit: same PASS;
- full ProjectAudit ran for 326.8 seconds and exited 1 on 15 repository/clean
  resource baselines outside this slice, including missing extracted map and
  character assets, project Data visibility `1519/1497`, an unbuilt
  `WModelGeometryContractHarness`, and the existing Artist/31210
  manifest/binding stage mismatch. Its Artist 31470 reconstructed runtime
  program check passed `tests=12 emitters=35 schedules=7 modules=399
  distributions=629 runtime=false product=false`; Python 91/21 and the custom
  handler subchecks also passed;
- corrective ClientFrontendHarness x64 Debug/Release `/m:1` builds: PASS;
- corrective `--effect-reconstructed-execution-plan` runs: Debug PASS in
  94.8 seconds and Release PASS in 9.2 seconds, both `failures : 0`; the exact
  seeded full-object authority swap, seeded `{3}` to `{4}`, and
  bracket-qualified dynamicparams target mutations all reached their intended
  semantic gates and preserved prior plan/pointer/status rollback;
- corrective Client x64 Debug/Release `/m:1` builds: PASS in 23.7/29.2 seconds;
- corrective dynamic focused audits with the canonical Resource root: Debug
  PASS in 95.5 seconds and Release PASS in 10.1 seconds, both
  `schedules=7 emitters=35 modules=399 distributions=629 rng=v1 fixedHz=60
  Product=false`;
- final focused static audit, project/filter XML parse, and
  `git diff --check`: PASS.
- second corrective ClientFrontendHarness x64 Debug/Release `/m:1` builds:
  PASS;
- second corrective `--effect-reconstructed-execution-plan` runs: Debug PASS
  in 93.8 seconds and Release PASS in 9.1 seconds, both `failures : 0`; owner
  path exact-versus-bracket reclassification and extra, duplicate, and
  reordered per-Property semantic reverse vectors all reached their dedicated
  status gate and preserved the prior plan pointer;
- second corrective Client x64 Debug/Release `/m:1` builds: PASS in 3.7/9.0
  seconds;
- second corrective dynamic focused audits with the canonical Resource root:
  Debug PASS in 93.6 seconds and Release PASS in 10.0 seconds, both
  `schedules=7 emitters=35 modules=399 distributions=629 rng=v1 fixedHz=60
  Product=false`;
- second corrective final static audit, project/filter XML parse,
  `git diff --check`, untracked text check, process cleanup, and exact scope
  refreeze: PASS.

## Review status

All changes remain unstaged and uncommitted in the dedicated worktree. No
commit or push has been performed. The full ProjectAudit's actual repository
baseline result and both corrective verification rounds are recorded above;
the worktree is refrozen for independent review.
