# 2026-08-11 Artist 31470 F Reconstructed Render Resource Authority Plan

## Goal and boundary

Create one immutable, offline render-resource authority sidecar for the frozen
Artist 31470 F reconstructed program. The sidecar must bind the 72 approved
Material texture bindings to the actual 48 DDS files in the canonical main
`Client/Bin/Resources` tree and preserve the already approved recipe, renderer,
and D3D state decisions.

This lane owns Python validation, one generated JSON receipt, focused audit,
and PLAN/RESULT only. It does not change Engine or Client C++, shader source,
`Client/Bin/Resources`, Product data, `PlayerSkills`, or animation events. It
does not open runtime execution. The result remains `sourceExact=false`,
`runtimeExecutionAdmission=false`, and `product=false`.

## Frozen inputs

| Authority | Frozen identity |
|---|---|
| integration base | commit `846394cb39306c3d5f781c3eae83adb59324ace3`, tree `68be53aa25a3c7ed697e5c13485dba68ea79ebe6` |
| reconstructed candidate builder | commit `a85b8b41afb2f2a51bceafa55d06bf0937b1a245`, tree `384ed35ca808ab9a71a4edb703ca4d9121b48c18` |
| candidate raw | 15,072,141 bytes, `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849` |
| program | `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b`, input artifacts 13, ordered input digest `938dbd9573ca3a5784675ba9d412b9dc3c12a7431a06c70e37d8c9bf2e614eaa` |
| typed parser integration | commit `eacb58bda2315e858c562677bbf38c17d5d3e785`, tree `8a2828fe2b3deb9c1270143b78a6edd6211d4801` |
| Material texture authority | commit `1a0b1a6834d562dac02db4f57dda54644d75695b`, tracked SHA `87a28be564308117ac666206382c94ce5ee2bf37a47111cbef717994a0266077`, self `3e722cf02085497c63083fbf51161ff5fd6670be91607737863b9c4019e55b48` |
| Material render-resource approval | commit `d053522f6c993730d1ee7a8eb156861f63a02b6d`, 376,183 bytes, raw `68ae71bd70260270404d4a7b6c296e41f74d0031d27899b56a4376c1b11f4931`, self `d643c9bf1bc2f10a887c805534b28e4322646cea426656de61b894e5b6284644`, decision `4731ed9c2882c948373ec54f56087803145447851f3fc793fb8e9fa9d96cc957` |
| format-3 publisher | integrated commit `932d648f95bc7f2d9c7209fad55aee8a857c94d7`, original commit `74c692755791e592d6f808f3c50b3321c60af181`, tree `f4da1e28a13875ae162eec4aa273c75647caa3a9` |
| historical/base runtime catalog checkout | blob `ca360e952dd110f0246a5e0f1374baf77b7ebc0c`, 26,255,931 bytes, raw `bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29` |
| Artist 31470 publisher tuple | outer/link/receipt/tool `10/16/25/3`, link `74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2`, receipt self `5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3`, outer receipt `92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94` |

The validator must parse the default candidate and approval from their actual
current files. Supplied objects are never reused as expected authority; they
must independently validate and recursively strict-equal the default objects.

The same rule applies to the tracked runtime catalog. Every validation reads
its current checkout path exactly once, requires the frozen Git blob and raw
checkout identity, and strict-parses duplicate/non-finite-free JSON directly
from that same hashed byte buffer. It never reopens the path for parsing. The
public runtime-catalog validator and the extracted Artist 31470 entry validator
must receive that exact parsed object; the latter re-reads the current
candidate-builder, catalog-validator, and publisher tools. The extracted
outer-10/link-16/receipt-25/tool-3 entry must recursively strict-equal the entry
reconstructed from the frozen candidate and current tools. A caller-supplied
catalog or entry is comparison input only.

This `932d648f`/`bf0807...` tuple is historical/base program-publication
source evidence for this freeze. It is not, and cannot be used as, a self-pin
for a future final catalog that embeds this sidecar. Future transport must keep
the sidecar unchanged, bind the resulting final catalog through a separate
non-circular publication receipt or new outer extension, and re-run strict
validation across that bridge.

## Resource identity contract

The canonical resource root is derived from `git rev-parse --git-common-dir`
and is exactly `<common-worktree>/Client/Bin/Resources`. This allows a linked
clean worktree to validate the team-managed main Resources bytes without
copying them into the feature worktree. A caller-supplied alternate root is
rejected.

Each `runtimeAssetId` must be a normalized, case-exact Resources-relative DDS
path with no drive prefix, backslash, empty component, `.`/`..`, symlink, or
root escape. The output records only the relative asset ID; no absolute path is
serialized.

For each of the 48 unique files, the builder records and validates:

- byte count and raw SHA-256;
- `DDS ` magic and the complete legacy 124-byte header fields;
- width, height, raw/effective mip count, pixel format, FourCC, caps, data
  offset, and payload byte count;
- exact block-compressed payload size across all effective mip levels;
- DXT1/BC1, DXT5/BC3, or ATI2/BC5 compression family;
- the approved color-space policy shared by every binding of that asset;
- the actual compressed DXGI SRV format and full Texture2D SRV descriptor.

An asset requested by both linear and sRGB policies is a hard blocker. BC5
requested as sRGB is also a hard blocker because the frozen D3D11 mapping has no
BC5 sRGB format.

## Binding and policy contract

All 72 Material texture bindings stay in candidate order. Each sidecar row
binds the candidate binding/row SHA, recipe, Material input, sampler policy/row
SHA, occurrences, source receipt identities, exact runtime asset, deduplicated
resource row, sampler descriptor, color policy, actual DDS SRV descriptor, and
actual resource byte identity.

The earlier `d3dSrvOracle` is retained under
`priorPolicySrvFixture` with the explicit role
`MATERIAL_POLICY_1X1_RGBA8_SRV_ORACLE_NOT_ACTUAL_DDS_DESCRIPTOR`. Its RGBA8
DXGI 28/29 value is never substituted for the compressed DDS descriptor. The
actual descriptor uses DXGI 72/77/78/83 as derived from DDS FourCC plus the
approved sampler color policy.

The sidecar copies the independently approved, row-sealed sections without
reclassification or runtime heuristics:

- four neutral providers;
- 27 recipe texture0/texture1/neutral decisions;
- 57 renderer slot-to-Material-input decisions, including the three explicit
  ambiguity decisions;
- 46 render-state decisions: blend 27, two-sided raster 18, disable-depth 1.

No basename, slot-role, or parameter-name heuristic is evaluated by this lane.

## Strict validation and sealing

The root has an exact ordered schema. Every resource, binding, neutral,
recipe, renderer, and render-state row has a canonical SHA-256 seal. The root
contains both an independently frozen decision projection SHA and a full
receipt self SHA. Recursive types are exact; booleans cannot be substituted by
integers, non-finite numbers are rejected, duplicate JSON keys are rejected,
and missing/extra/reordered keys fail.

Validation always rebuilds the expected receipt from the frozen default
program/approval, current tracked format-3 runtime catalog, current publisher
tools, and current canonical Resources bytes. Optional supplied DDS, approval,
program, publisher catalog/entry, tool, or root objects are comparison inputs
only. The historical/base publisher source-evidence tuple is included in the
independent decision projection as well as the full receipt projection. A
failure does not produce
a staged resource set and the future consumer contract remains
`PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK` with
`ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET`.

The generated receipt is raw-byte frozen. `.gitattributes` receives one
exact-path `text eol=lf` rule; no broad JSON rule is introduced.

## Owned files

| File | Role |
|---|---|
| `Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_render_resource_authority.py` | actual DDS parser, deterministic builder, strict current-authority validator |
| `Tools/LevelPlacementExtractor/artist_31470_reconstructed_render_resource_authority.py` | independent decision/full-receipt projection pins |
| `Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_render_resource_authority.py` | strict positive and coordinated mutation regressions |
| `Data/Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority.receipt.json` | immutable Product-false resource sidecar |
| `Tools/ProjectAudit/Test-Artist31470ReconstructedRenderResourceAuthority.ps1` | focused counts, identity, clean-checkout, and scope audit |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | focused audit registration |
| `.gitattributes` | exact receipt LF checkout rule |
| this PLAN and matching RESULT | contract and executed evidence |

There are no C++ files, so no `.vcxproj` or `.vcxproj.filters` changes and no
C++ build are required for this prerequisite lane.

## Negative matrix

The unit and focused suites must reject:

- coordinated DDS header, byte identity, compressed SRV, or color mutation;
- binding recipe/owner/path mutation and resource/binding reorder;
- recipe texture provider, ambiguous renderer selection, or D3D descriptor
  mutation after row/root resealing;
- supplied actual DDS bytes that differ from canonical Resources;
- coordinated supplied approval reseal, candidate A/B mismatch, or current tool
  mutation;
- coordinated publisher outer, link, receipt, tool-dependency, or full-catalog
  A/B mutation after every dependent publisher digest is re-sealed;
- any validator path that does not re-read the current runtime catalog and all
  three current publisher tool dependencies;
- cold or cached validation that reopens the catalog path after hashing, parses
  a second A/B byte stream, or validates an object other than the one produced
  from the first exact byte buffer;
- alternate Resources root, path traversal, bool-to-integer substitution, or
  blocker/admission promotion;
- independent decision/full receipt projection mismatch;
- BOM, CRLF, or output staleness.

## Acceptance gates

1. Python mutation suite passes.
2. Deterministic generator `--check` passes while re-reading all 48 DDS files.
3. `core.autocrlf=true` isolated checkout preserves receipt bytes/raw/self/
   decision, CR0/BOM0, and repeats unit plus generator checks.
4. Focused ProjectAudit passes and is registered in full ProjectAudit.
5. JSON/Python/PowerShell syntax, `git diff --check`, and exact dirty scope pass.
6. Worktree remains unstaged and uncommitted for independent review.

WARP reflection, the C++ transactional resource consumer, shader binding,
manual in-game eye validation, and Product admission remain later lanes.
