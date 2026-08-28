# Build and regression harness

Run from the repository root, or use an absolute script path from another directory:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

The script builds Engine, refreshes EngineSDK with UpdateLib, and builds Shared,
Server, Client and the registered console harnesses. It checks the compiled shader
closure, publisher contracts, numerical tests and headless harnesses. It does not
launch Client or perform Lobby/Bern/Valtan/Development visual smoke. Those checks
belong to the user, through `Framework.slnLaunch` (Server + Client), with the
Client working directory set to `Client/Default`.

`-SkipBuild` repeats the checks against existing binaries; it does not prove that
those binaries contain later source changes. Missing harness executables remain
an error. Run Debug and Release separately. Runtime Resources are managed
directly by the team lead and are not an immutable-pack/hash gate.

When the team already shares local Effect resources and does not want them added
to Git, explicitly pass `-AllowLocalEffectResources`. This forwards
`-AllowLocalResources` to `Tools/EffectPipeline/Validate-EffectSources.ps1`.
Paths, actual DDS/WModel files, and tracked-file Git/LFS identity are still
validated. The report lists untracked local resources separately; this mode is
not proof of Git-only asset delivery. Without the flag, the Git closure check is
unchanged. Neither mode stages or uploads resources.

Effect and resource admission is performed by the domain publishers, focused
Python/WARP checks, and the Client Debug/Release builds. The regression command
does not build or execute a second Client frontend.

## Map frustum and surface diagnostics

`MapFrustumContractHarness` compiles the production
`Client/Private/MapAssetRenderUtils.cpp` and exercises its explicit camera builder
and sphere predicate. It checks the Bern cancellation regression, all six clip
planes, tangency, small camera changes, invalid-input rollback and rejection
grace. It does not initialize Client, exercise instance-buffer uploads, or judge
rendered pixels. The normal regression command builds and runs it; an isolated
repeat after building that configuration is:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapFrustumContractHarness/Run-MapFrustumContractHarness.ps1 -Configuration Debug
```

The regression also runs the synthetic tests in
`Tools/MapPipeline/test_map_surface_depth_contract.py`. Actual Character Select
surface inspection is read-only and requires the local map resources:

```powershell
python -B Tools/MapPipeline/test_map_surface_depth_contract.py --resource-root Client/Bin/Resources --area-id LV_LOBBY_CLASSSELECT_SL00 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:405=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:427=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:436=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:458=0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:471=-0.002
```

These arguments describe the five approved edge corrections relative to Imported;
the two existing central corrections are checked separately. They do not modify
placements. Without a camera log it reports `cameraDepthStatus="not_requested"`; geometry
overlap is not a visual or GPU depth-test PASS. No new F1 capture panel is part of
the initial frustum fix.
