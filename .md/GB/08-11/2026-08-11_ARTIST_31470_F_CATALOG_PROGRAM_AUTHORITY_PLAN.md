# Artist 31470 F Catalog Program Authority Plan

Date: 2026-08-11

Branch: `codex/artist-f-catalog-program-authority-v1`

Base commit: `eacb58bda2315e858c562677bbf38c17d5d3e785`

Required publisher commit:
`74c692755791e592d6f808f3c50b3321c60af181`, tree
`f4da1e28a13875ae162eec4aa273c75647caa3a9`

## Goal

Make the frozen Artist 31470 reconstructed runtime program a first-class,
immutable `CEffectCatalog` authority without promoting it to runtime execution
or Product admission. The Catalog must load the exact embedded UTF-8 program
from the tracked format-3 runtime catalog, parse it into the already frozen
typed program, commit one Catalog-owned immutable entry, and carry one opaque
entry/preparation identity through the Presentation, Object, Playback, and
Renderer preparation seams.

The slice is complete only when the actual tracked 102-effect/555-component
catalog loads through production `CEffectCatalog::Load`, all 35 emitters are
available as typed immutable rows, every execution and drawing gate remains
closed, failed reloads preserve every prior map/status/revision/pointer, and the
same entry/program pointers survive reload, snapshot, restore, clear, and each
downstream preparation seam.

## Frozen input authority

The Catalog accepts one reconstructed program identity:

- runtime catalog asset ID `effect.artist.skill.31470`;
- program ID `effect.artist.skill.31470.reconstructed-approved-v1`;
- builder commit `a85b8b41afb2f2a51bceafa55d06bf0937b1a245`;
- builder tree `384ed35ca808ab9a71a4edb703ca4d9121b48c18`;
- candidate blob `345ab15bbb76648a650eaa854f18c4cd63cb1556`;
- candidate bytes `15,072,141` and raw SHA-256
  `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849`;
- program SHA-256
  `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b`;
- resource binding SHA-256
  `df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c`;
- 13-input ordered SHA-256
  `938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa`.

The actual tracked catalog is format 3 with 555 components and 102 effects.
Its exact runtime bytes and publisher receipt are pinned only after the
publisher source/runtime-ID, duplicate-key, tool-identity, rollback, and LF
gates receive independent review.

## Catalog payload contract

Keep the existing exact 17-key `IMMUTABLE_COMPILED_IR` payload unchanged. Add a
distinct `IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM` payload so reconstructed
authority never borrows or synthesizes a generic compiled-artifact receipt.

The reconstructed entry has exact ordered outer keys:

```text
payloadKind, effectAssetId, artifactRevision, compilerRevision,
sourceExact, runtimeExecutionAdmission, productAdmission,
publishReceiptSha256, publishReceipt, reconstructedRuntimeProgram
```

The embedded program link has exact ordered keys:

```text
schema, formatVersion, encoding, effectAssetId,
candidateBuilderCommitId, candidateBuilderTreeId, candidateBlobId,
resourceBindingHash, inputArtifactCount, inputArtifactsOrderedSha256,
programId, programVersion, programSha256, candidateRawSha256,
candidateByteCount, candidateUtf8Json
```

The publication receipt has exact ordered 25 keys and exactly three ordered
tool dependencies. It grants publication provenance only. The Catalog pins and
recomputes the link digest, receipt self digest, and complete receipt digest.
It never normalizes `candidateUtf8Json`, reads a candidate path, or synthesizes
raw authoring/Detail/SourceRecipe fallback.

## Immutable ownership and transaction

`EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY` is privately constructed by
`CEffectCatalog` and owns the exact Catalog identity plus a
`shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>`. Public queries return
const shared pointers; no raw JSON document is returned.

`Load` follows parse, validate, stage, commit. The staged maps include legacy
documents, assemblies, components, generic compiled authorities, and
reconstructed entries. A failed load preserves all committed maps, runtime
revision, global status, entry pointer, and program pointer. Runtime snapshots
own the reconstructed map so generation A remains alive while generation B is
queried, and external pointers remain valid across clear/restore.

The stable Artist ID is reserved before payload dispatch. It is rejected from
legacy and generic payloads, hidden from raw document/assembly/generic-authority
queries, and denied by Product spawn even if a stale legacy snapshot exists.

## Shared preparation boundary

The Catalog produces one privately constructed
`EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION` that owns the immutable entry and
five typed `{ownerEmitterId, AnchorRequest}` rows. The exact five owners and
request IDs are validated in source order and globally unique.

`CEffectReconstructedRuntimeBoundary` is a production, non-D3D component linked
by both Client and Harness. Presentation prepare, Product spawn, Object stage
and execution/submit/render, Playback stage/execution, and Renderer
stage/render call this same component before side effects. Object, Playback,
and Renderer carry the opaque preparation; they do not accept a public raw
program pointer.

The boundary permits inspection/preparation only. Product spawn, simulation,
frame submission, and rendering remain false for the reconstructed entry.
Legacy behavior remains unchanged when no reconstructed preparation is staged.

## Fixed denominators

```text
emitter/action schedule/module/distribution  35/7/399/629
Mesh/Sprite/Decal/Ribbon/Light/Post            13/16/3/1/1/1
typed owner AnchorRequests                     5
legacy/raw fallback for Artist 31470           0
runtime execution admission                    false
Product admission                              false
```

## Negative and lifetime matrix

The focused Harness must reject transactionally:

- malformed catalog JSON and reserved-ID legacy/generic laundering;
- missing, extra, reordered, or wrong-type outer-10, link-16, and receipt-25
  fields;
- candidate raw SHA, program SHA, embedded CRLF, link, receipt, or tool
  identity changes;
- coordinated tool/receipt reseals and generic-17 reconstructed extensions;
- invalid restaging after a valid opaque preparation.

Every catalog rejection asserts the intended earliest error stage and preserves
status, revision, all four public ID projections, entry pointer, and program
pointer. The positive path asserts exact pointer `get()` identity across
Catalog, Presentation preparation, Object, Playback, Renderer, reload A/B,
snapshot, restore, and clear lifetimes.

## Owned files

- `Client/Public/Effect_Catalog.h`
- `Client/Private/Effect_Catalog.cpp`
- `Client/Public/Effect_PresentationService.h`
- `Client/Private/Effect_PresentationService.cpp`
- `Client/Public/Effect_Object.h`
- `Client/Private/Effect_Object.cpp`
- `Client/Public/Effect_Playback.h`
- `Client/Private/Effect_Playback.cpp`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Test-EffectRuntimeAuthority.ps1`
- this PLAN and the matching RESULT

The publisher commit owns `.gitattributes`, source/runtime catalog data,
publisher/validator code and tests, the actual runtime catalog, and its own
PLAN/RESULT. It is consumed as a reviewed prerequisite rather than rewritten by
this lane.

## Build and verification

After the publisher commit and exact receipt/tool/catalog identities are
frozen, own the serial build lease and:

1. build `ClientFrontendHarness` x64 Debug and Release with `/m:1`;
2. run `--effect-runtime-authority` in both configurations with the frozen
   candidate, actual tracked runtime catalog, and canonical non-Git Resources
   root;
3. build Client x64 Debug and Release so every real call site compiles;
4. run focused `Test-EffectRuntimeAuthority.ps1` in both configurations with
   explicit candidate, actual catalog, and Resources root;
5. run full `Invoke-ProjectAudit.ps1` and separate unrelated repository or
   checkout baselines from focused R3 results;
6. run `git diff --check`, conflict-marker and process-count checks;
7. leave the R3 changes uncommitted and unstaged for independent review.

Manual in-game or eye validation is not part of this slice. R4 and later lanes
must implement typed execution, renderer families, Product admission, and
visual validation before Artist F can be called visually complete.
