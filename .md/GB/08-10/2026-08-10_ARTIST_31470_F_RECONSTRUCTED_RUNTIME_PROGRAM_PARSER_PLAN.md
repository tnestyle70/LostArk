# Artist 31470 F Reconstructed Runtime Program Parser Plan

Date: 2026-08-10

Branch: `codex/artist-f-reconstructed-runtime-program-parser-v1`

Base commit: `d705a9dc633af102456553ed17fdb43eda520883`

## Frozen input authority

The independently reviewed final 13-input builder authority is commit
`a85b8b41afb2f2a51bceafa55d06bf0937b1a245`, tree
`384ed35ca808ab9a71a4edb703ca4d9121b48c18`. Its one accepted candidate is
15,072,141 bytes, 269,613 parsed JSON values, and depth 8, with raw SHA-256
`72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849`
and program SHA-256
`618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b`.
The parser accepts this profile only. It has no provisional 68/4 texture
profile, identity range, or fallback schema.

The final profile contains 72 resolved texture bindings: 68 exact runtime-cook
receipts and four reconstructed exact DDS deployment receipts. Blocker ownership
pins 6,108 owned fields, 6,670 token occurrences, and projection SHA-256
`d47b33c183d863761470585348dc4da2cc48e69a12b3594515444d20e5c04650`.

The private parser uses strict UTF-8/no-BOM parsing, duplicate-key rejection
through `CDataJson`, a 20 MiB/64-depth/1,000,000-value ceiling, exact
insertion-order key checks, JSON integer-versus-float checks, normalized
relative-path checks, and canonical row/section/self-digest primitives. It does
not expose a fallback program or admit runtime/Product execution.

## Goal and completion evidence

Implement the closed C++ parser and immutable in-memory representation for the
`RECONSTRUCTED_APPROVED_V1` Artist F runtime program. The parser consumes the
offline-generated candidate only after `CDataJson` has parsed it, preserves all
executable semantics in typed immutable rows, independently rederives every row,
section, blocker, admission, reverse-owner graph, and root identity, and commits
a new shared pointer only after the complete document is valid.

The lane is complete only when the actual generated candidate passes the
Debug and Release `ClientFrontendHarness` parser mode and all of the following
mutations reject without replacing the caller's previously committed pointer:

- unexpected or missing root/nested key;
- floating-point integer token, non-finite numeric value, or unsupported enum;
- duplicate/global ID collision, invalid owner, invalid order, or valid-handler
  reassignment to a different row;
- row, section, blocker-union, or root SHA mismatch, including a fully resealed
  payload whose ownership or admission predicate is false;
- A/B expected program identity mismatch;
- texture cook/deployment receipt identity or source projection mismatch;
- timing, random-seed, renderer, distribution-sample, material descriptor, or
  geometry-carrier projection mismatch after coordinated canonical resealing;
- Product promotion or runtime execution promotion before downstream consumers
  exist.

## Owned files

- `Client/Public/Effect_RuntimeAuthority.h`
- `Client/Private/Effect_RuntimeAuthority.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- this PLAN and the matching RESULT
- a focused ProjectAudit script and registration only if the generated-candidate
  gate cannot be expressed by the harness mode alone

The lane does not modify the Python builder, generated candidate, Source,
Material, Geometry, Playback, Renderer, Catalog, publisher, or Resources.

## Public contract

`Effect_RuntimeAuthority.h` gains closed enums and row structs for the generated
program. The public document owns execution data only; it does not own a mutable
authoring document, Playback state, GPU resources, or raw `SourceRecipe`
pointers. Stable string IDs are used for every cross-row link.

The parser entry accepts:

1. the raw UTF-8 candidate bytes, so BOM, malformed UTF-8, duplicate keys, byte
   size, depth, and value-count bounds are enforced before a JSON tree exists;
2. an expected `{builder commit, builder tree, candidate raw SHA, programId,
   programVersion, programSha256}` identity supplied by the caller;
3. a reference to the caller's current immutable program pointer;
4. an error string.

The parsed `DATA_JSON_VALUE` is private staging state only. No public API returns
it or exposes a `SourceRecipe`, Effect Detail object, authoring pointer, or
catalog wrapper. The catalog `{assetId, version, programSha256}` association is
attached outside this document by the later catalog lane.

It parses and validates into local storage, then replaces the output pointer in
one final assignment. Failure preserves pointer identity and all prior data.
The parsed document keeps `bRuntimeExecutionAdmission=false` and
`bProductAdmission=false`; this lane has no execution consumer.

Canonically resealed negative fixtures use a Harness-target-only semantic
validator behind `LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS`. That symbol
is absent from the public header and production Client project, owns only a
discarded local staged pointer, and cannot replace a caller-visible program.

## Exact validation sequence

1. Require the exact root key set and exact JSON integer token for versions and
   orders.
2. Require closed enum domains and finite numeric fields.
3. Parse every section into typed local rows while enforcing section-local and
   global stable-ID uniqueness.
4. Validate ordered parent/owner links and reject forward or cross-owner
   laundering where the schema forbids it.
5. Recompute the seven action-schedule IDs and the frozen aggregate projection
   over cue, occurrence, system, receipt event index, global time, duration, and
   source cue row SHA.
6. Recompute each `rowSha256` from canonical JSON with only that field removed.
7. Recompute each ordered section digest from the canonical ordered row array.
8. Recompute the exact blocker union and all admission predicates rather than
   trusting serialized booleans or counts.
9. Recompute `programSha256` from the canonical root projection defined by the
   frozen builder schema.
10. Compare the rederived identity with the caller-provided expected identity.
11. Commit the staged shared pointer once.

The exact nested key order, row types, enum literals, and digest projections are
copied from the frozen builder schema and actual generated candidate. They are
not inferred from the rejected `4ffe1102` payload or from free-form runtime
bags.

## Fixed denominators

```text
emitter/module/property/leaf/distribution  35/399/1434/1572/629
seed/default/PointLight                    14/14/8
material family/recipe/input               23/27/729
material static/render/occurrence/policy   94/162/34/255
texture binding/resolved/cook/deployment  72/72/68/4
renderer texture resources                57
geometry carrier/use                       7/13
unknown/ownerless/silent row               0
runtime execution admission                false
Product admission                          false
```

## Build and regression

After the candidate and schema are frozen and the serial build lease is owned:

1. parse the candidate with explicit `CDataJson` limits appropriate to its
   measured byte/value/depth counts;
2. build Engine x64 Debug and run `UpdateLib.bat Debug`;
3. build `ClientFrontendHarness` x64 Debug and run the dedicated parser mode;
4. build Engine x64 Release and run `UpdateLib.bat Release`;
5. build `ClientFrontendHarness` x64 Release and run the same mode;
6. rerun the existing `--effect-runtime-authority` mode in both configurations;
7. parse changed project/filter XML;
8. run the focused audit if added, `git diff --check`, and conflict-marker scan;
9. record only the commands actually executed in RESULT.
