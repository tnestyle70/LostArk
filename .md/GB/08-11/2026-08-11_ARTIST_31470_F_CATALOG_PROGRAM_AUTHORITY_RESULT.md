# Artist 31470 F Catalog Program Authority Result

Date: 2026-08-11

Branch: `codex/artist-f-catalog-program-authority-v1`

Base commit: `eacb58bda2315e858c562677bbf38c17d5d3e785`

Publisher integration commit in this branch: `0f7faa3c`, the exact content of
reviewed publisher commit `74c692755791e592d6f808f3c50b3321c60af181`
and tree `f4da1e28a13875ae162eec4aa273c75647caa3a9`.

Status: R3 Catalog authority, opaque preparation transport, and automated
Debug/Release verification PASS. Runtime execution, Product admission,
PlayerSkills/animevents integration, and manual visual validation remain
intentionally false/not performed.

## Frozen authority consumed

The independently reviewed publisher produced the actual format-3 runtime
catalog with 555 components and 102 effects:

- catalog bytes `26,255,931`;
- catalog raw SHA-256
  `bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29`;
- candidate bytes `15,072,141`, raw SHA-256
  `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849`;
- program SHA-256
  `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b`;
- reconstructed link SHA-256
  `74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2`;
- receipt self SHA-256
  `5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3`;
- complete publish receipt SHA-256
  `92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94`;
- candidate-builder tool SHA-256
  `5c207e04952971adb553249540e336ba3ad065719e438a9892c6850d2c989c4e`;
- catalog-validator tool SHA-256
  `5407c3d0983c3aaf4bf085904ef8d7b5f3e9119ae448703ff7e8f612a1c144fb`;
- publisher tool SHA-256
  `ee4a12cf5cbd63bc9af6b0af18ca37da7631a4b0b6ed1465c95bf99fb9be8825`.

The publisher's independent corrective review closed source/runtime asset-ID
substitution, ordinary and escaped-equivalent duplicate JSON keys, generic
fallback for the reserved ID, coordinated reseals of each of the three tool
rows, and failure rollback of source/runtime/temp/backup files before the
commit was integrated here.

## Implemented Catalog contract

`CEffectCatalog` now recognizes a distinct exact
`IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM` payload. It does not call
`Parse_DerivedEntry`, synthesize a generic receipt, read an external candidate,
normalize embedded line endings, or construct a raw drawable document. The
existing generic 17-key compiled path is unchanged and rejects a reconstructed
extension field.

The reconstructed loader validates exact ordered outer-10, link-16,
receipt-25, and three exact tool rows. It pins and recomputes the link, receipt
self, and full receipt digests, then parses the exact embedded UTF-8 string with
`Parse_ReconstructedRuntimeProgram`. It independently checks the fixed
35/7/399/629 denominators and renderer counts Mesh 13, Sprite 16, Decal 3,
Ribbon 1, Light 1, and Post 1.

One privately constructed `EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY` owns the
Catalog identity and `shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>`.
The reconstructed map participates in load staging, snapshots, restore, clear,
revision changes, and public const-pointer queries. Failure restores prior
global status and never replaces any committed map or pointer.

The stable Artist 31470 ID is reserved before payload dispatch. Legacy and
generic payloads reject, raw document/assembly/generic-authority lookup returns
no fallback, and Product spawn denies the ID even after a failed reload or
stale legacy state.

## Opaque production preparation seam

The Catalog builds one immutable
`EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION`. It preserves five exact typed
AnchorRequests with their owner emitter IDs, source order, unique request IDs,
and reverse-owner identity.

`CEffectReconstructedRuntimeBoundary` is the shared non-D3D production seam.
The real Presentation prepare/Product-spawn, Object stage/update/late-update/
submit/render, Playback stage/reset/update/seek, and Renderer stage/render
paths delegate to it before side effects. The Harness executes these same pure
production functions and proves identical Preparation, Entry, and Program
`get()` pointers for Object, Playback, and Renderer seams. Public raw-program
staging is absent.

Inspection and preparation are permitted. Product spawn, execution, submit,
and render all fail closed. The Playback evaluated frame stays empty after
attempted update/seek, while legacy paths continue to clear the reconstructed
boundary and behave as before.

## Correctives applied during this lane

The first pointer harness attempted `CEffectObject::Initialize` with null D3D
device/context. Renderer initialization failed before typed staging, so that
was not accepted as pointer-transport evidence. The proof was replaced with
the shared non-D3D production boundary described above; the Harness now reaches
the exact Object/Playback/Renderer target stages, and static audit pins every
real heavy call site to the same gate. Full Client builds verify those call
sites in both configurations.

The first actual-catalog run failed with:

```text
Effect runtime Component rejected: Effect source Material texture is invalid:
dissolve_tex_01
(Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_001.dds).
```

The dedicated worktree does not carry team-managed, non-Git
`Client/Bin/Resources`. No production validation was weakened. The final
commands explicitly set
`LOSTARK_RESOURCE_ROOT=C:\Users\user\Desktop\LostArk\Client\Bin\Resources`;
all 555 legacy components then resolved their physical resources and the exact
102-effect catalog passed. Focused ProjectAudit now requires an existing
Resources root and a known catalog-referenced DDS before invoking the Harness.

## Automated verification

All commands below were executed against the final R3 source and publisher
catalog unless explicitly identified as an environment baseline.

- LAN endpoint sync: local configuration PASS; server-host endpoint retained;
  listener absence was non-blocking.
- Engine x64 Debug and Release: PASS.
- `UpdateLib.bat` Debug and Release: PASS.
- `ClientFrontendHarness.vcxproj` x64 Debug `/m:1`: PASS.
- Debug `--effect-runtime-authority <candidate> <actual-catalog>` with the
  canonical Resources root: 38/38 PASS, `failures : 0`, 91.9 seconds.
- `ClientFrontendHarness.vcxproj` x64 Release `/m:1`: PASS.
- Release same actual-catalog mode: 38/38 PASS, `failures : 0`, 9.0 seconds.
- Client x64 Debug `/m:1`: PASS after final boundary/pin changes.
- Client x64 Release `/m:1`: PASS after final boundary/pin changes.
- focused `Test-EffectRuntimeAuthority.ps1` with the Debug Harness, frozen
  candidate, actual catalog, and canonical Resources root: PASS, 578.1 seconds.
- focused same audit with the Release Harness: PASS, 48.6 seconds.
- Release `--effect-reconstructed-runtime-program`: 4/4 PASS in this lane.
- `git diff --check`: PASS before documentation; rerun in the final scope audit.

The focused 38-test catalog matrix includes actual format3/102/555 loading,
exact immutable identity and family counts, zero Artist raw fallback, one opaque
pointer across all seams, five AnchorRequest owners, execution/submit/render/
Product denial, reload A/B and clear lifetime, status/revision/all-map/pointer
rollback, outer/link/receipt missing-extra-reorder-type attacks, embedded CRLF,
program/candidate/tool identity mutations, coordinated reseal, generic
laundering, and stale-state spawn denial.

The full `Invoke-ProjectAudit.ps1` ran for 243.5 seconds and exited 1 with 16
repository/environment categories. Reported categories were map extracted-root
and Character Select contracts, project Data visibility, G09 runtime/cross-
document baselines, Artist source/material/DDS/geometry aggregate checks,
missing Debug WModel geometry harness, derived publisher, reconstructed program,
WFX assembly, representative authored readiness, four-class exact rollout at
`Artist/31210`, actor assets, and DimensionMaster animation resources.

Two full-audit failures are specifically a checkout-timing baseline in this
old worktree: it was created before the publisher added the candidate
`text eol=lf` rule, so Git reports `i/lf w/crlf attr/text eol=lf` and the
working bytes remain `15,370,273`/`c8dbd33a...` although staged and unstaged
candidate diffs are empty and the index/HEAD blob is the exact
`345ab15b...` LF blob. Guarded exact-path `git restore` and authorized
`git checkout-index --force` both left the clean-equivalent CRLF worktree bytes
unchanged; no delete/recreate or candidate content change was performed.
Consequently the full audit's derived-publisher and reconstructed-program
checks report stale/noncanonical working bytes. The clean integration checkout
at `932d648f` independently passed LF bytes `15,072,141`, raw SHA `72e417...`,
CR0, BOM false, builder `--check`, Debug/Release parser 4/4, legacy authority
7/7, focused audit, diff check, and clean status. R3 itself uses the exact
embedded catalog bytes and its test-only fixture canonicalizes only the external
attack input; production performs no EOL normalization.

## Runtime and manual status

No runtime execution or Product promotion was made. No PlayerSkills or
animevents binding was changed. No particles, meshes, decals, ribbons, lights,
or post effects were rendered by this lane, and no in-game or human-eye PASS is
claimed.

R4 and later work must implement the typed executor, geometry/material binding,
Playback and six renderer-family consumers, Product admission, all-occurrence
runtime capture, and final Artist F visual validation. This R3 result proves
that those consumers can receive one exact immutable program without raw
fallback or pointer/identity laundering.

The publisher prerequisite is committed as `0f7faa3c` in this branch. The R3
Catalog/transport/harness/audit/document changes remain unstaged and uncommitted
as a review-ready snapshot. No R3 commit or push was performed.
