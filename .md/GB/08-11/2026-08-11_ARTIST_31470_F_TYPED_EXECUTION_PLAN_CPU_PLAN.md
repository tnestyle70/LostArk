# Artist 31470 F Typed Execution Plan CPU Plan

Date: 2026-08-11

Branch: `codex/artist-f-typed-execution-plan-cpu-v1`

Exact base commit: `8352a3718bcf0a3f9386e913a8a90807e70f8bd0`

## Goal

Compile the frozen Artist skill 31470 `F`
`EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM` into one immutable, non-Product CPU
execution plan. The plan must be staged only from the existing Catalog-owned
`EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION`, preserve exact stable identities
and reverse joins, and expose a deterministic inspection-only fixed-step
simulation for automated verification.

This slice does not grant `runtimeExecution`, Product admission, renderer
submission, or visual completion. `CEffectPlayback` may own and expose the
immutable plan after Catalog preparation, but its existing production gate
continues to return an empty evaluated frame. The CPU simulator is compiled
only into `ClientFrontendHarness` until the later sidecar/resource/render
authority is independently frozen.

## Frozen authority

```text
effectAssetId                  effect.artist.skill.31470
programId                      effect.artist.skill.31470.reconstructed-approved-v1
builder commit                 a85b8b41afb2f2a51bceafa55d06bf0937b1a245
builder tree                   384ed35ca808ab9a71a4edb703ca4d9121b48c18
candidate bytes                15,072,141 LF UTF-8, CR 0
candidate raw SHA-256          72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849
program SHA-256                618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b
schedule/emitter/module/dist   7/35/399/629
schedule distribution         4/1/15/12/1/1/1
renderer families              Sprite16/Mesh13/Decal3/Ribbon1/Post1/Light1
distribution variants          612/8/5/1/3
distribution samples           1852 = evaluator1839 + parameter13
bursts                          31
lifetime modules               ordinary34 + seeded1
operational caps               sum1291 / maximum594 / Ribbon500
seed policies                  14
runtimeExecution/Product       false/false
```

## Immutable plan contract

`EFFECT_RECONSTRUCTED_EXECUTION_PLAN` privately owns:

- plan version 1, occurrence RNG version 1, and fixed-step rate 60 Hz;
- the Catalog revision, exact effect/program/candidate identities, and a
  semantic projection SHA-256;
- strong `shared_ptr<const ...>` ownership of the Preparation and Program;
- ordered stable-ID lists plus stable-ID keyed maps for all 7 schedules,
  35 emitters, 399 modules, 629 distributions, and 14 seed policies;
- exact handler/evaluator registry, implementation version/SHA, source class,
  module role, schedule/source identity, timing, burst, cap, and seed joins.
- one frozen distribution ID/module/property/target projection and one frozen
  owner projection for all 629 rows. The owner projection seals each owning
  Property ID/module/path plus its exact-path or bracket-descendant
  classification (`509/120`), and each Property's ordered
  `SemanticDistributionIds` must equal the actual reverse distribution vector;
- one immutable exact seeded-lifetime authority tuple spanning its
  emitter/module/property/distribution/seed, module/property/evaluator
  handlers, row hashes, and capability implementation/family/schema/binding
  identities.

Vector positions remain iteration order only. Persisted occurrence identity is
`<emitter stable ID>::occurrence:<spawn serial>`; no pointer, Prototype tag, or
vector index becomes identity.

The compiler rejects unknown enum values, missing or duplicate stable IDs,
wrong owner/order/membership, handler/evaluator mismatches, malformed numeric
shapes, non-finite values, numeric oracle mismatches, unsupported seeded
lifetime policies, and any attempt to use deferred EF multiply authority as a
CPU timing evaluator. Coordinated ordinary/seeded authority swaps,
bracket-qualified distribution target changes, Property owner-path
reclassification, and extra/duplicate/reordered Property reverse membership
reject at their dedicated semantic gates. Failure never replaces the caller's
prior plan pointer.

## CPU inspection semantics

The Harness-only inspector reconstructs state from zero and advances exact
1/60-second steps. It applies, in stable schedule/emitter order:

1. schedule global-time and emitter delay gates;
2. finite positive emitter duration and exact loop count;
3. lifetime expiry before the current step's cap calculation;
4. ordered burst ranges with the versioned per-emitter occurrence stream;
5. typed spawn rate multiplied by typed rate scale;
6. ordinary lifetime from the occurrence value or the one seeded lifetime's
   independently reset fixed seed policy;
7. operational particle caps with explicit dropped-by-cap counts;
8. immutable state/frame projections and stable occurrence packet order.

The frame packet is inspection-only and contains stable occurrence, schedule,
and emitter IDs, renderer family, loop/spawn sequence, occurrence and lifetime
random values, age, and lifetime. It is not a GPU/resource packet and must not
be consumed by a production renderer.

## Forbidden input and Product boundary

`Effect_ReconstructedExecution.cpp` must not read or name `SourceRecipe`,
`DetailTransform`, `RendererRuntimeConfig`, or legacy `EFFECT_DETAIL_DESC`.
The raw poison harness mutates those fields after typed parsing and requires an
identical semantic plan projection.

Only the plan compiler is in the Client build. The CPU inspector implementation
is behind `LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS`, a define owned only by
`ClientFrontendHarness`. `CEffectPlayback::Update`, `Seek`, and `Reset` keep the
existing Catalog boundary denial; no production tick calls the inspector.

## Transaction and lifetime

`CEffectPlayback::Stage_ReconstructedRuntimeProgram` performs:

```text
Catalog Preparation stage
-> typed execution plan compile/validate/stage
-> clear prior legacy document/state
-> commit boundary
-> commit immutable plan
```

If boundary or plan compilation fails, the prior Playback Preparation and plan
pointers remain unchanged. Catalog reload creates an independent generation-B
plan while generation A continues to strongly own its A Preparation and
Program. Both plan generations must have identical semantic projection SHA
and distinct Catalog revisions/pointers.

## Owned files

- new `Client/Public/Effect_ReconstructedExecution.h`
- new `Client/Private/Effect_ReconstructedExecution.cpp`
- modify `Client/Public/Effect_Playback.h`
- modify `Client/Private/Effect_Playback.cpp`
- modify Client project and filter registration
- modify `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- modify Harness project and filter registration
- new `Tools/ProjectAudit/Test-Artist31470TypedExecutionPlan.ps1`
- modify `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- this PLAN and the matching RESULT

The separate renderer/resource preflight owns no files above. Its later
resource compiler may join the frozen execution plan by stable ID. Any future
GPU packet remains a separate Product-disconnected contract until its own
authority exists.

## Harness matrix

The compiled Harness must prove:

- exact 7/35/399/629, schedule distribution, renderer distribution, 31 bursts,
  34+1 lifetime roles, 1291/594/500 caps, 14 seeds, and 612/8/5/1/3 variants;
- all 399 modules and 629 distributions have exact stable owners, orders,
  handler/evaluator identities, and complete reverse membership;
- 60 Hz schedule/delay gates, rate/rate-scale, burst, loop completion,
  lifetime expiry, seeded lifetime, cap, packet order, and deterministic RNG;
- repeated runs produce identical immutable state/frame projections;
- invalid sample, wrong A/B identity, unknown variant, unknown handler, wrong
  seed, and broken schedule joins reject without replacing prior pointers;
- a full-object ordinary/seeded lifetime authority swap and a
  `dynamicparams[0].paramvalue` target mutation reach their intended semantic
  gates and preserve prior plan/status ownership;
- a coordinated `dynamicparams` owner path to
  `dynamicparams[0].paramvalue` reclassification and extra, duplicate, and
  reordered `SemanticDistributionIds` mutations reach the owner-projection or
  exact reverse-vector gates and preserve the prior plan pointer plus exact
  failure status;
- raw poison does not change the execution semantic projection;
- Catalog reload A/B and invalid Playback restage preserve ownership and
  rollback rules;
- Product/runtime/submit/render remain false and evaluated frames remain empty.

## Build and verification

After receiving the serial build lease:

1. build `ClientFrontendHarness` x64 Debug and Release with `/m:1`;
2. run `--effect-reconstructed-execution-plan` in both configurations with
   the exact candidate, tracked runtime catalog, and canonical team-managed
   Resources root;
3. build Client x64 Debug and Release with `/m:1`;
4. run the focused typed execution audit against both Harness binaries;
5. run existing runtime-authority/reconstructed-program regressions;
6. run `Invoke-ProjectAudit.ps1`, separating unrelated repository baselines;
7. run XML parse, `git diff --check`, conflict-marker, process, and scope checks;
8. leave all changes unstaged and uncommitted for independent review. Do not
   push or claim visual/Product completion.
