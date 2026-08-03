# Build and regression harness

Run from any working directory:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

The harness fixes the Client working directory to `Client/Default`, verifies the runtime shader/resource layout, builds in the repository contract order, runs the protocol harness, then exercises Lobby, Bern, Valtan, and every registered Development scenario through the same scene-transition boundary used by the product. The full-map Development scenario has an explicit 120-second ceiling; product scenes keep the shorter configured ceiling. The harness only terminates processes that it started. Each run clears only its own `.codex_tmp/regression/<Configuration>` output first, so a skipped Release Development scenario cannot inherit a stale PASS report from an older run.

Use `-DeepAssetHash` before integration or asset-pack publication. `-SkipBuild` is for a local repeat against already-built binaries; it does not weaken the smoke or audit checks.
