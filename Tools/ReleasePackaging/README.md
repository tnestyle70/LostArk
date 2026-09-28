# Portable Release packaging

Run from the repository root after the final Release Product build, publishers and required raid checks.
Resources stay external; the package contains Client/Server executables, app-local VC DLLs, compiled shaders,
current Data/DataFiles closure and the launcher. No Client or Server is started by packaging or these tests.

```powershell
python Tools/GameplayPipeline/generate_numeric_source_bindings.py
python Tools/ReleasePackaging/test_package_tools.py
python Tools/ReleasePackaging/build_portable.py --plan-only
python Tools/ReleasePackaging/build_portable.py --stage out/ReleasePackaging/20260929-final --output-zip C:\Users\user\Desktop\LostArk-Release-20260929.zip --build-receipt out/BuildPipeline/runs/<successful-release-product>.json
```

Use a fresh stage directory under `out`. Supply the real successful Product receipt; a skipped build or
invalid runtime-input receipt is rejected. Source metadata must be generated after the final data publish and
line-ending normalization. `--crt-root` optionally selects the official VS app-local CRT directory.
The build machine needs Python 3.11+, Visual Studio C++ redistributable files and .NET Framework's C# compiler;
players need neither Python nor Visual Studio.

Compiler files, fixtures, plans and delivery receipts remain under `out/ReleasePackaging`.
The builder checks current source references, numeric source hashes, Kouku action/sequence revisions,
ZIP CRC and every manifest SHA256, and runs `LostArk.exe --check` without launching the game.
A previous destination ZIP is backed up before atomic replacement; a separately named old ZIP is untouched.
The launcher accepts later native numeric saves only through the exact source allowlist and matching
`BalanceNumeric.save.receipt.json`/`NumericBalance.active.json` hashes. All other package files stay pinned.
`README_실행방법.md` is the user guide copied into the ZIP. GUI, audio and real four-Client LAN checks remain user-run.
