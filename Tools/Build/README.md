# Build and regression harness

Run the default Core profile from the repository root, or use an absolute script path from another directory:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

Profiles are cumulative by intent, but broad diagnostics are never part of a normal
Visual Studio Solution Build:

- `-Profile Product`: Engine, UpdateLib, Shared, Server, Client and product CSO closure.
- `-Profile Core` (default): Product plus publisher validation, NetworkProtocol, and
  one real-Server Character Select `Core` isolation scenario.
- `-Profile FullDiagnostic`: Core plus Character Select `Party2`/`Party4` transfer,
  presentation, map, point-light, physics, WModel and broad Server diagnostics for
  affected domains.

The script does not
launch Client or perform Lobby/Bern/Valtan/Development visual smoke. Those checks
belong to the user, through `Framework.slnLaunch` (Server + Client), with the
Client working directory set to `Client/Default`.

`-SkipBuild` repeats the selected profile against existing binaries; it does not prove
that those binaries contain later source changes. Missing executables selected by
that profile remain an error. Run Debug and Release separately. Runtime Resources are managed
directly by the team lead and are not an immutable-pack/hash gate.

When the team already shares local Effect resources and does not want them added
to Git, explicitly pass `-AllowLocalEffectResources`. This forwards
`-AllowLocalResources` to `Tools/EffectPipeline/Validate-EffectSources.ps1`.
Paths, actual DDS/WModel files, and tracked-file Git/LFS identity are still
validated. The report lists untracked local resources separately; this mode is
not proof of Git-only asset delivery. Without the flag, the Git closure check is
unchanged. Neither mode stages or uploads resources.

Effect and resource admission is performed by the domain publishers, focused
checks, and the Client Debug/Release builds. The deleted Imported Artist 31470
corpus is not restored merely to satisfy the old broad EffectRender executable;
that executable remains outside active profiles until its unique assertions use
current Product fixtures. No profile builds or executes a second Client frontend.

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
