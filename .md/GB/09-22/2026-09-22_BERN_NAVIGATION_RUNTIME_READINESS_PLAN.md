# Bern 영역 런타임 전달 검사 PLAN

## G01 현재 상태와 변경 경계

Bern3 원본은 667886ffb에 추가됐고 4b7fdaf25의 양쪽 runtime navregions는 3개 영역을 선언하지만 Bern3 격자와 sidecar는 Git에 없다. 현재 Product는 기본 grid 존재만 확인한다. 이 작업은 기존 Product runtimeDataChecks에 게시된 navregions의 참조와 grid byte 길이·필수 sidecar header 검사를 추가한다. 게시·C++/HLSL·Server 시작·Client 실행은 변경하지 않는다.

새 작업 브랜치는 codex/bern-navigation-runtime-fix이며 최신 origin/main 4044a7893에서 작업한다. 0ebd23cd1 이후 Navigation 원본·게시 파일·publisher 변경은 없다.

## G01 파일과 호출 계약

- 수정: `Tools/Build/Invoke-BuildAndRegression.ps1`의 `Test-ProductPublishedCatalogs` 바로 앞에 아래 함수를 추가하고 Product runtimeDataChecks 수집에 연결한다. `$grids`는 required 기본 grid와 양쪽 게시 목록이 요구하는 세부 grid의 상대경로, optional manifest step을 소유한다. 검사 실패는 path/result/details를 남기며 다른 grid를 계속 검사한다.
- 수정: 기존 `Tools/Build/test_build_profile_contract.py`에 정상·세부 누락·잘림·sidecar 누락 사례를 추가한다. 실제 runner 함수만 AST로 읽어 임시 데이터에 호출하는 기존 테스트 방식을 사용한다.
- 문서: Tools/Build/README.md와 CLAUDE.md의 Product 준비 검사 범위, gotchas의 반복 방지 항목을 좁게 갱신한다. C++ 파일이나 project/filter 등록은 없다.

## G01 교체 가능한 코드

### Invoke-BuildAndRegression.ps1: Test-ProductPublishedCatalogs 바로 앞

```powershell
function Test-ProductNavigationInputs {
    param($NavigationDomain)

    # Inspect published dependencies only. Baking and world admission stay with
    # the navigation publisher and Server; this must not mutate either root.
    $grids = [ordered]@{}
    foreach ($relative in $NavigationDomain.requiredOutputPatterns) {
        if ($relative -match '^(Server|Client)/Bin/DataFiles/Navigation/[^/]+\.navgrid$') {
            $grids[[string]$relative] = $null
        }
    }
    foreach ($side in @('Server', 'Client')) {
        $relativeRoot = "$side/Bin/DataFiles/Navigation"
        foreach ($manifest in @(Get-ChildItem -LiteralPath (Join-Path $repoRoot $relativeRoot) `
            -Filter '*.navregions' -File -ErrorAction SilentlyContinue)) {
            $timer = [Diagnostics.Stopwatch]::StartNew()
            $relative = "$relativeRoot/$($manifest.Name)"
            try {
                $lines = @([IO.File]::ReadAllLines($manifest.FullName) | Where-Object { $_.Trim() })
                $area = $manifest.BaseName
                if ($area -notmatch '^[A-Za-z0-9_-]+$' -or $lines.Count -eq 0 -or
                    $lines[0] -cnotmatch ('^LOSTARK_NAVGRID_REGIONS 1 "' + [regex]::Escape($area) + '" ([0-9]+)$')) {
                    throw 'Invalid navigation region manifest header'
                }
                $count = [uint64]$Matches[1]
                if ($count -gt 64 -or $lines.Count -ne $count + 1) { throw 'Truncated or invalid navigation region manifest' }
                $grids["$relativeRoot/$area.navgrid"] = $null
                $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($line in @($lines | Select-Object -Skip 1)) {
                    if ($line -cnotmatch '^REGION "([A-Za-z0-9_-]{1,32})" (\S+)$') { throw 'Invalid navigation region row' }
                    $region = $Matches[1]
                    $step = [single]::Parse($Matches[2], [Globalization.CultureInfo]::InvariantCulture)
                    if (-not $seen.Add($region) -or [double]::IsNaN($step) -or [double]::IsInfinity($step) -or $step -lt 0) {
                        throw 'Duplicate region or invalid navigation step policy'
                    }
                    $grids["$relativeRoot/$area.$region.navgrid"] = $step
                }
            }
            catch {
                [pscustomobject]@{ path=$relative; publisher='Tools/NavigationPipeline/Publish-ServerNavigation.ps1';
                    result='FAIL'; elapsedMs=$timer.ElapsedMilliseconds; details=$_.Exception.Message }
            }
            finally { $timer.Stop() }
        }
    }
    foreach ($entry in $grids.GetEnumerator()) {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        $relative = [string]$entry.Key
        $result = 'FAIL'
        $details = ''
        try {
            $path = Join-Path $repoRoot $relative
            $stream = [IO.File]::OpenRead($path)
            $reader = [IO.BinaryReader]::new($stream)
            try {
                if ($stream.Length -lt 20) { throw 'Truncated navigation grid header (expected 20 bytes)' }
                $width = $reader.ReadUInt32(); $height = $reader.ReadUInt32()
                $cellSize = $reader.ReadSingle(); $originX = $reader.ReadSingle(); $originZ = $reader.ReadSingle()
                $cells = [uint64]$width * [uint64]$height
                if ($width -eq 0 -or $height -eq 0 -or $cells -gt 1000000 -or $cellSize -le 0 -or
                    [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or
                    [single]::IsNaN($originX) -or [single]::IsInfinity($originX) -or
                    [single]::IsNaN($originZ) -or [single]::IsInfinity($originZ)) { throw 'Invalid navigation grid header' }
                if ($stream.Length -ne 20 + 5 * $cells) { throw 'Truncated navigation grid payload or unexpected trailing bytes' }
            }
            finally { $reader.Dispose() }
            $stem = [IO.Path]::GetFileNameWithoutExtension($path)
            foreach ($extension in @('navpolicy', 'navblockers')) {
                $relative = [IO.Path]::ChangeExtension([string]$entry.Key, $extension).Replace('\', '/')
                $sidecar = Join-Path $repoRoot $relative
                if (-not (Test-Path -LiteralPath $sidecar -PathType Leaf) -or (Get-Item -LiteralPath $sidecar).Length -eq 0) {
                    throw 'Missing or empty navigation sidecar'
                }
                $header = [IO.File]::ReadLines($sidecar) | Select-Object -First 1
                $tokens = [regex]::Matches($header, '"[^"]*"|\S+') | ForEach-Object { $_.Value.Trim('"') }
                if ($tokens.Count -lt 3 -or $tokens[1] -cne '1' -or $tokens[2] -cne $stem) { throw 'Invalid navigation sidecar identity' }
                if ($extension -eq 'navpolicy') {
                    if ($tokens.Count -ne 4 -or $tokens[0] -cne 'LOSTARK_NAVIGATION_POLICY') { throw 'Invalid navigation policy header' }
                    $step = [single]::Parse($tokens[3], [Globalization.CultureInfo]::InvariantCulture)
                    if ([double]::IsNaN($step) -or [double]::IsInfinity($step) -or $step -lt 0 -or
                        ($null -ne $entry.Value -and [Math]::Abs($step - $entry.Value) -gt 0.000001)) { throw 'Navigation policy differs from region manifest' }
                }
                else {
                    if ($tokens.Count -ne 9 -or $tokens[0] -cne 'LOSTARK_NAVGRID_BLOCKERS' -or
                        [uint32]$tokens[3] -ne $width -or [uint32]$tokens[4] -ne $height -or [uint32]$tokens[8] -gt 256) {
                        throw 'Invalid navigation blocker header'
                    }
                    for ($index = 0; $index -lt 3; ++$index) {
                        $value = [single]::Parse($tokens[5 + $index], [Globalization.CultureInfo]::InvariantCulture)
                        if ([double]::IsNaN($value) -or [double]::IsInfinity($value) -or
                            [Math]::Abs($value - @($cellSize, $originX, $originZ)[$index]) -gt 0.000001) { throw 'Navigation blocker grid differs from payload' }
                    }
                    if ($null -ne $entry.Value -and [uint32]$tokens[8] -ne 0) { throw 'Detail navigation region declares runtime blockers' }
                }
            }
            $relative = [string]$entry.Key
            $result = 'PASS'
            $details = 'Published grid byte length and required sidecar headers are complete'
        }
        catch { $details = $_.Exception.Message }
        finally { $timer.Stop() }
        [pscustomobject]@{ path=$relative; publisher='Tools/NavigationPipeline/Publish-ServerNavigation.ps1';
            result=$result; elapsedMs=$timer.ElapsedMilliseconds; details=$details }
    }
}

```

### Product runtimeDataChecks 대입 교체

```powershell
        $script:runtimeDataChecks = @(
            Test-ProductPublishedCatalogs
            Test-ProductNavigationInputs (Get-BuildDomainById $runtimeInputManifest 'navigation')
        )
```

## G01 검증

기존 build-profile 테스트에서 임시 정상2x2 grid와 Bern3 region을 사용해 정상, missing region grid, header/payload truncation, policy/blocker missing, manifest duplicate/invalid ID를 판정한다. 현재 정상 게시 Bern 파일 사본에도 같은 실제 함수를 실행하고 원본 hash 불변을 확인한다. PS AST parse와 git diff --check를 실행한다. 셀별 높이 유효성·층별 겹침·world spawn admission은 기존 publisher/Server 범위로 남기며 이 준비 검사의 PASS로 대체하지 않는다.

## 기존 build-profile 테스트 삽입 정본

파일 상단 import subprocess 바로 다음에 import struct를 추가한다. BuildProfileContractTests의 test_product_catalog_check_preserves_failure_without_publishing 바로 앞에 아래 메서드를 추가한다.

```python
    def test_product_navigation_region_dependencies_are_checked_without_writes(self) -> None:
        with tempfile.TemporaryDirectory(prefix="lostark-nav-readiness-") as directory:
            fixture = Path(directory)
            nav = fixture / "Server/Bin/DataFiles/Navigation"
            nav.mkdir(parents=True)
            area = "LV_BER_BERNCASTLE"
            originals = {}
            for stem in (area, area + ".Bern3"):
                originals[nav / (stem + ".navgrid")] = struct.pack("<IIfff", 2, 2, .5, 1000.1239, -250.9876) + bytes([1] * 4) + struct.pack("<ffff", 0, 0, 0, 0)
                originals[nav / (stem + ".navpolicy")] = f'LOSTARK_NAVIGATION_POLICY 1 "{stem}" 1\n'.encode()
                originals[nav / (stem + ".navblockers")] = f'LOSTARK_NAVGRID_BLOCKERS 1 "{stem}" 2 2 0.5 1000.1239 -250.9876 0\n'.encode()
            manifest = nav / (area + ".navregions")
            originals[manifest] = f'LOSTARK_NAVGRID_REGIONS 1 "{area}" 1\nREGION "Bern3" 1\n'.encode()
            script = fixture / "check.ps1"
            script.write_text(
                "param([string]$RunnerPath, [string]$FixtureRoot)\n"
                "$ErrorActionPreference = 'Stop'\n"
                "$ast = [Management.Automation.Language.Parser]::ParseFile($RunnerPath, [ref]$null, [ref]$null)\n"
                "$function = $ast.Find({ param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] "
                "-and $node.Name -eq 'Test-ProductNavigationInputs' }, $false)\n"
                "Invoke-Expression $function.Extent.Text\n"
                "$repoRoot = $FixtureRoot\n"
                "$domain = @{requiredOutputPatterns=@('Server/Bin/DataFiles/Navigation/LV_BER_BERNCASTLE.navgrid')}\n"
                "@(Test-ProductNavigationInputs $domain) | ConvertTo-Json -Depth 4\n", encoding="utf-8")
            region_grid = nav / (area + ".Bern3.navgrid")
            scenarios = (
                ("normal", None, None, None),
                ("missing region", region_grid, None, ".Bern3.navgrid"),
                ("truncated header", region_grid, originals[region_grid][:12], ".Bern3.navgrid"),
                ("truncated payload", region_grid, originals[region_grid][:-1], ".Bern3.navgrid"),
                ("missing base", nav / (area + ".navgrid"), None, area + ".navgrid"),
                ("missing policy", nav / (area + ".Bern3.navpolicy"), None, ".Bern3.navpolicy"),
                ("missing blockers", nav / (area + ".Bern3.navblockers"), None, ".Bern3.navblockers"),
                ("policy mismatch", nav / (area + ".Bern3.navpolicy"), f'LOSTARK_NAVIGATION_POLICY 1 "{area}.Bern3" 2\n'.encode(), ".Bern3.navpolicy"),
                ("unsafe region", manifest, f'LOSTARK_NAVGRID_REGIONS 1 "{area}" 1\nREGION "../Bern3" 1\n'.encode(), ".navregions"),
                ("manifest truncated", manifest, f'LOSTARK_NAVGRID_REGIONS 1 "{area}" 2\nREGION "Bern3" 1\n'.encode(), ".navregions"),
            )
            for label, changed, replacement, failure_path in scenarios:
                with self.subTest(label=label):
                    for path, data in originals.items():
                        path.write_bytes(data)
                    if changed is not None:
                        if replacement is None:
                            changed.unlink()
                        else:
                            changed.write_bytes(replacement)
                    before = {p.name: p.read_bytes() for p in nav.iterdir()}
                    result = subprocess.run(
                        ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script),
                         "-RunnerPath", str(ROOT / "Tools/Build/Invoke-BuildAndRegression.ps1"),
                         "-FixtureRoot", str(fixture)], text=True, capture_output=True, timeout=20, check=False)
                    self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                    rows = json.loads(result.stdout)
                    if isinstance(rows, dict):
                        rows = [rows]
                    failed = [row for row in rows if row["result"] != "PASS"]
                    if failure_path is None:
                        self.assertEqual(2, len(rows))
                        self.assertEqual([], failed)
                    else:
                        self.assertTrue(any(row["path"].endswith(failure_path) for row in failed), result.stdout)
                    self.assertEqual(before, {p.name: p.read_bytes() for p in nav.iterdir()})
        runner = read("Tools/Build/Invoke-BuildAndRegression.ps1")
        self.assertIn("Test-ProductNavigationInputs (Get-BuildDomainById $runtimeInputManifest 'navigation')", runner)

```
