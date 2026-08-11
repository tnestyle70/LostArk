# 2026-08-11 Artist 31470 F Reconstructed Render Resource Authority Result

## Verdict

The Product-false immutable resource sidecar is implemented and locally frozen
for independent review. The validator joins the 72 frozen Material bindings to
48 byte-exact canonical DDS files, records actual compressed SRV descriptors,
and preserves the previously approved 27 recipe, 57 renderer, and 46 D3D state
decisions.

This is an offline prerequisite, not an in-game completion claim. It keeps
`sourceExact=false`, `requiresAutomatedWARPProbe=true`,
`requiresManualEyeValidation=true`, `runtimeExecutionAdmission=false`, and
`product=false`. No C++ consumer, shader, Resources, Product, PlayerSkills, or
animation-event file changed.

## Frozen receipt

- path:
  `Data/Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority.receipt.json`
- schema:
  `lostark.artist-31470-reconstructed-render-resource-authority-receipt`
- authority ID: `ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1`
- serialized bytes: 746,788
- raw SHA-256:
  `bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff`
- receipt self/full projection SHA-256:
  `bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a`
- independent decision projection SHA-256:
  `4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412`
- CR count: 0
- UTF-8 BOM: false
- serialized absolute path count: 0

## Derived current resource profile

The following counts are computed from current DDS headers and approved
per-binding sampler color policies; they are not copied as builder inputs.

| Actual resource classification | Unique DDS files |
|---|---:|
| BC1 sRGB | 35 |
| BC3 sRGB | 8 |
| BC5 linear | 4 |
| BC3 linear | 1 |
| total | 48 |

| Actual per-binding SRV | Bindings |
|---|---:|
| DXGI 72 `BC1_UNORM_SRGB` | 58 |
| DXGI 78 `BC3_UNORM_SRGB` | 9 |
| DXGI 83 `BC5_UNORM` | 4 |
| DXGI 77 `BC3_UNORM` | 1 |
| total | 72 |

The 72 binding policies split into 67 sRGB and five linear bindings. Every
unique asset has one consistent color policy. All 48 files have valid legacy
DDS headers, supported DXT1/DXT5/ATI2 FourCC values, one effective mip level,
and compressed payload byte counts that exactly match their dimensions and
block format.

## Material and renderer decisions preserved

| Section | Count |
|---|---:|
| neutral providers | 4 |
| recipe texture0/texture1/neutral decisions | 27 |
| renderer slot-to-Material-input decisions | 57 |
| explicit ambiguous renderer decisions | 3 |
| blend descriptors | 27 |
| two-sided raster descriptors | 18 |
| disable-depth descriptors | 1 |
| total D3D state descriptors | 46 |

The three ambiguous renderer selections remain exactly those frozen by the
integrated approval. This lane does not recompute them from a basename, slot,
or parameter role. Recipe neutral choices and all render-state descriptor rows
are likewise copied with their existing row seals and validated against the
current approved receipt.

## Actual DDS versus prior policy fixture

Each binding now has two intentionally separate fields:

- `priorPolicySrvFixture`: the prior 1x1 RGBA8 policy oracle, DXGI 28 or 29;
- `actualDdsSrvDescriptor`: the real compressed DDS Texture2D SRV, DXGI
  72/77/78/83 with the effective mip count.

The validator requires the old oracle's expected/actual pair to match exactly,
but never treats it as the DDS resource descriptor. The actual descriptor is
derived from the byte-parsed FourCC plus the approved sampler color policy.

## Publisher source-evidence corrective

Independent review found that the first freeze did not bind the format-3
publisher/runtime-catalog source evidence into this sidecar. The corrective now
re-reads and validates:

- integrated publisher commit `932d648f95bc7f2d9c7209fad55aee8a857c94d7`
  and original equivalent commit
  `74c692755791e592d6f808f3c50b3321c60af181`, both tree
  `f4da1e28a13875ae162eec4aa273c75647caa3a9`;
- runtime catalog blob `ca360e952dd110f0246a5e0f1374baf77b7ebc0c`,
  current checkout 26,255,931 bytes, raw
  `bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29`;
- format 3, 555 components, 102 effects, with Artist 31470 at effect index 0;
- exact outer/link/receipt/tool counts `10/16/25/3`;
- link SHA `74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2`;
- receipt self SHA
  `5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3`;
- outer publish-receipt SHA
  `92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94`.

Every sidecar validation reads the current runtime-catalog bytes and invokes
the existing public `validate_reconstructed_runtime_entry` validator. That
validator re-reads the three current publisher tool dependencies. The extracted
entry then strict-equals the entry reconstructed from the frozen candidate and
current tools. The compact publisher authority is part of the independent
decision projection, so a coordinated source-evidence/root reseal cannot bypass
the independent pin.

A second independent review reproduced a split-read TOCTOU in the first
publisher corrective: the builder hashed one `read_bytes()` result and then
`load_json(path)` reopened the catalog. A different, still structurally valid
second object could therefore be parsed while the receipt claimed the first
raw identity. The final corrective removes that reopen. Cold validation
strict-parses the exact first byte buffer, passes that same object to the public
catalog validator, and caches only that object. Cached validation still reads
and authenticates the path exactly once and revalidates the exact cached entry
plus all current tools. Split-read A/B fixtures prove both paths perform one
catalog read and never consume the forged second stream.

The `932d648f`/`bf0807...` catalog tuple is historical/base
program-publication evidence only. It is not a final-catalog self-pin. If a
later publisher transports this unchanged sidecar into a new final catalog,
that publication must add a separate non-circular receipt or new outer
extension that binds the final catalog bytes, then re-run this strict
validation across the transport bridge. This lane makes no circular
runtime-authority claim.

## Validation evidence

- Python strict/mutation suite: 31/31 PASS.
- deterministic generator `--check`: PASS.
- actual DDS byte/header/payload deep verification: PASS, 48/48.
- coordinated row/root mutation matrix: PASS; DDS header/raw/SRV/color,
  binding owner/path, recipe choice, renderer ambiguity, D3D state, order,
  external DDS, approval, tool, root, A/B, type, and independent pin mutations
  all rejected.
- publisher corrective matrix: PASS; coordinated outer, link, receipt,
  tool-dependency, and full-catalog A/B reseals all rejected, and read tracing
  proves the current catalog plus all three current tools are read. Cold/cache
  split-read fixtures additionally prove catalog read-count `1`, strict
  duplicate/non-finite rejection, parsed/public-validator object identity, and
  cached-object identity.
- receipt LF/raw/self/decision identity: PASS locally.
- `core.autocrlf=true` isolated checkout after publisher corrective: PASS;
  receipt `746788` bytes, raw/self/decision identities unchanged, CR `0`, BOM
  absent, Python suite 31/31 PASS, and deterministic generator `--check` PASS.
- focused ProjectAudit after publisher corrective: PASS; resources/bindings
  `48/72`, publisher outer/link/receipt/tool tuple `10/16/25/3`, and the
  isolated checkout contract all passed.
- full ProjectAudit after the single-read publisher corrective: registered
  sidecar check PASS; the aggregate command completed in 450.7 seconds and
  exited 1 with the same
  15 unrelated repository/environment baselines. The render-resource authority
  check is absent from the failure list.
- `git diff --check`, Python compile/JSON parse, PowerShell parse, and conflict
  marker scan: PASS on the final corrective dirty scope.

## Transaction and admission boundary

The builder reads canonical Resources only during offline build/validation.
The receipt explicitly sets `actionTimeIoAllowed=false`; a future C++ consumer
must preload and stage the resource set before any action. Its required failure
behavior is `ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET`, with no partial commit.

This receipt is render-resource proof only and carries no Execute, Submit,
runtime, or Product admission. The separate CPU-plan identity currently
abbreviated `e05c...` cannot discharge any row in this receipt. A later seam
proof must have its own stable authority ID, revision, full self identity, and
exact denominators. It must join the CPU `7/35/399/629` schedule/emitter/module/
distribution authority to this sidecar's `48/72/27/57/46` resource/binding/
recipe/renderer/explicit-state rows, plus the still-missing default-state
`9/26` denominators. Missing, extra, reordered, stale, or independently
advanced inputs must reject before Submit.

The required authority graph is acyclic: historical publisher/candidate feeds
the separate CPU proof and this immutable resource sidecar; those feed the
WARP/C++ seam proof; that proof feeds a separate publication bridge; only a
later explicit gate may admit Execute, Submit, runtime, or Product. No CPU hash
may stand in for resource evidence, and no final catalog hash may feed backward
into and reseal this unchanged sidecar.

Remaining mandatory gates are:

1. independent review of this exact uncommitted receipt and validator;
2. a separate non-circular publication receipt/outer extension and transport
   revalidation before any final-catalog embedding;
3. automated D3D11 WARP creation/reflection of sampler, SRV, blend, raster, and
   depth descriptors, including the still-missing explicit `RS_Default` nine
   and `DSS_Default` 26 decisions; this sidecar is not a complete/renderable
   state claim;
4. a C++ transactional resource-set and renderer-family consumer that uses one
   immutable byte buffer through `CreateDDSTextureFromMemoryEx` and queried
   Texture2D/SRV descriptors, exact entry/revision/program/sidecar/device
   identity at stage and commit recheck, and no device-context identity;
5. exact neutral and sampler packet consumption: mathematical `0.5` rather
   than R8 value 128, exact unique U/V/W modes including clamp U/V with
   W=`WRAP`, and no real-binding-to-neutral fallback;
6. manual Artist F in-game eye validation;
7. explicit later runtime and Product admission.

No stage, commit, or push is performed by this implementation lane before the
independent freeze verdict.
