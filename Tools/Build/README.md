# Build and regression harness

Run the default Product profile from the repository root, or use an absolute script path from another directory:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

Profiles are cumulative by intent, but broad diagnostics are never part of a normal
Visual Studio Solution Build:

- `-Profile Product` (default): Engine, Shared, Server and Client compilation and
  normal MSBuild deployment. Publishers, source/resource fingerprints and broad
  diagnostics do not run. `Client.vcxproj` is the single owner of EngineSDK,
  compiled-shader and Client runtime dependency deployment.
- `-Profile Core`: Product plus publisher validation, NetworkProtocol, and
  one real-Server Character Select `Core` isolation scenario.
- `-Profile FullDiagnostic`: Core plus Character Select `Party2`/`Party4` transfer,
  presentation, map, point-light, physics, WModel and broad Server diagnostics for
  affected domains.

The script does not
launch Client or perform Lobby/Bern/Valtan/Development visual smoke. Those checks
belong to the user, through `Framework.slnLaunch` (Server + Client), with the
Client working directory set to `Client/Default`.

The diagnostic domain graph is declared in `BuildDomains.json`. Each validation or publisher
action writes a content receipt under `out/BuildPipeline/receipts`, so an unchanged
input/output fingerprint is reused instead of publishing the same data again from
explicit domain invocation and Core validation. A successful Core/FullDiagnostic run writes HEAD plus dirty
identity, source/input hash, EXE/PDB hashes, and step timings under
`out/BuildPipeline/runs`.

`-SkipBuild` is supported only for Product. It checks the existing runtime layout
without compiling and is not evidence that current source has been built. Run
Debug and Release separately. Runtime Resources are managed directly by the team
lead and are not an immutable-pack/hash gate.

Product writes a compact result to `out/BuildPipeline/runs/*-product.json`.
Each project records elapsed time, toolchain and tracking state before/after,
and counts of OBJ, PCH, CSO and binary outputs whose size or modification time
changed. Those counts measure output writes, including deployed copies; they
do not count failed compiler attempts. Missing diagnostics are reported as
unavailable rather than as zero work. No binary content or source hash is read
for this measurement.

The runner invokes the ordered projects with one MSBuild node per project.
The shared project settings control C++ `/MP` workers for both the IDE and CLI;
`-MaxCompilerProcesses 4` explicitly overrides `CL_MPCount` for one runner call.
Zero (the default) leaves that shared setting and explicit environment choices
intact. This does not enable `UseMultiToolTask` or change the shader worker limit.

Keep the same Visual Studio installation between IDE and command-line builds.
`-MSBuildPath` chooses an explicit executable; otherwise the runner respects
`MSBUILD_EXE_PATH`, `VSINSTALLDIR`, then PATH before discovering the latest
complete C++ installation (including Preview). The selected path/version is
recorded. Toolset, SDK or command-line changes may correctly invalidate existing
tracking state even when source files did not change.

For a rebuild investigation, add `-BuildLogDirectory out/BuildDiagnostics`.
This preserves a per-project binlog and diagnostic text log, and includes up to
20 dependency/command-change message samples in the result. Message wording is
toolset/localization dependent; an empty sample list is not proof that no
recompilation occurred. Ordinary builds do not create these large logs.

Product Effect runtime resources are Drive-owned and excluded from Git. Domain
validation accepts those local files by default while still checking safe asset IDs,
physical DDS/WModel bytes, and referenced entries that are present in the Git index.
The legacy `-AllowLocalEffectResources` switch remains accepted for command
compatibility. The report lists untracked local resources separately; validation is
not proof of Git-only asset delivery and never stages or uploads resources.

Effect and resource admission is performed by the domain publishers, focused
checks, and the Client Debug/Release builds. The deleted Imported Artist 31470
corpus is not restored merely to satisfy the retired broad EffectRender executable.
V2 stage/commit and resource-root checks now live in EffectToolV2 focused tests;
the explicit compiled-shader closure builds one temporary RenderingPipeline probe and verifies
actual V1/V2 WARP draw/readback. No profile builds a second Client frontend or a
persistent EffectRender project/cache.

## C++ edit and merge boundaries

The four Product projects import `CppCompilation.props`. x64 Debug and Release
build one project-local PCH from `CppStandardPch.h`; `/FI` supplies it before the
source. The PCH contains stable standard-library headers only. Do not add
`Engine_Defines.h`, gameplay types, authoring documents, or shader registries to
it: changing such input would invalidate every PCH consumer. Optimized units
with different compiler settings and third-party implementations opt out.
`CL_MPCount` defaults to the smaller of 8 and the machine's logical processor
count, unless explicitly set. The shader worker count remains independent.

Include the actual contract a file uses; PCH is not the declaration owner.
`Engine_Defines.h` no longer imports every vendor library or `Engine_Struct.h`.

| Required declaration | Direct header |
| --- | --- |
| Engine initialization | `Engine_InitTypes.h` |
| Lighting, shadows, fog, rendering quality | `Engine_RenderTypes.h` |
| Only forward declarations of rendering API types | `Engine_RenderFwd.h` |
| Vertex layouts and input elements | `Engine_VertexTypes.h` |
| Animation keyframes | `Engine_AnimationTypes.h` |
| Assimp scene/importer, DDS/WIC loaders, FX11 | The specific vendor header in the implementation that uses it |

`Engine_Struct.h` remains an explicit compatibility aggregate; avoid it in new
shared headers. Keep pointer/reference-only implementation types forward-declared.
`CGameInstance` exposes rendering declarations without importing their definitions.
A CPP that consumes a returned rendering value includes `Engine_RenderTypes.h`.

Effect Tool editing, resources, document IO and playback are separate CPP files;
Effect renderer preparation, material binding and geometry submission likewise
compile separately. Add a new responsibility to the matching unit. An `.inl`
included by a large CPP does not create a separate compilation unit. Register new
CPP files in both vcxproj and filters, preserving existing physical organization.

Use Build after pulling or merging. Missing outputs, changed headers, toolchain
changes and compiler-option changes legitimately trigger compilation; no-change
Build should write no OBJ/PCH/CSO outputs. Do not delete tracking state or run
Clean/Rebuild merely because several branches were merged. Runtime ZIPs supply
executables and CSOs, not portable compiler intermediate state.

For a focused include audit, `/p:LostArkUsePch=false` disables the Product PCH.
Use a separate intermediate directory for that comparison: toggling the option
in the normal directory correctly invalidates compiler command tracking.

Source-contract checks use `cpp_source_domains.py` to read a split implementation.
`read_cpp_domain(ROOT / "Client", "Effect_Tool")` joins the core and same-prefix
CPP files actually registered in the product vcxproj, plus the registered private
`Effect_Tool_Internal.h`; it excludes the distinct `Effect_Tool_V2` owner.
Register each new domain CPP in vcxproj/filters so the compiler and source checks
see the same implementation. Keep private shared declarations in that owner's
registered `_Internal.h`; do not add unrelated headers to the inspection scope.

`read_source_text(path, ...)` expands only the known split owners listed in the
helper. When splitting another owner, connect its actual source-inspection callers
explicitly or add it to that list after reviewing those callers. Keep physical
file identity, encoding and project metadata checks as physical reads. Inspect
functions with an exact signature and `cpp_function_body` or
`cpp_function_definition`, rather than the position of the next function.

## Map surface diagnostics

`-Profile FullDiagnostic` keeps the map-water binding/pass assertion with the
MapPipeline that owns map presentation validation:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Test-MapWaterRenderContract.ps1
```

The profile also runs the synthetic tests in
`Tools/MapPipeline/test_map_surface_depth_contract.py`. Actual Character Select
surface inspection is read-only and requires the local map resources:

```powershell
python -B Tools/MapPipeline/test_map_surface_depth_contract.py --resource-root Client/Bin/Resources --area-id LV_LOBBY_CLASSSELECT_SL00 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:405=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:427=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:436=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:444=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:458=0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:471=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:474=-0.004
```

These arguments describe the seven approved edge corrections relative to Imported;
the two existing central corrections are checked separately. They do not modify
placements. The report covers all six pair combinations in the 442/444/458/474
bridge K4 and separates whole-polygon near-coplanar counts from signed bevel/slope
crossings. Without a camera log it reports `cameraDepthStatus="not_requested"`; geometry
overlap is not a visual or GPU depth-test PASS. No new F1 capture panel is part of
the initial frustum fix.
