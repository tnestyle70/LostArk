[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$guardedScripts = @(
    'Tools/Build/Invoke-BuildAndRegression.ps1',
    'Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1',
    'Tools/Network/Run-CharacterSelectIsolationHarness.ps1',
    'Tools/ValtanPipeline/Test-ValtanPatternMaster.ps1',
    'Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1'
)
foreach ($relativePath in $guardedScripts) {
    $parseErrors = $null
    $tokens = $null
    $ast = [Management.Automation.Language.Parser]::ParseFile(
        (Join-Path $repoRoot $relativePath), [ref]$tokens, [ref]$parseErrors)
    if ($parseErrors.Count -ne 0) {
        throw "PowerShell parse failed: $relativePath"
    }
    $scopedExitReferences = @($ast.FindAll({
        param($node)
        $node -is [Management.Automation.Language.VariableExpressionAst] -and
            $node.VariablePath.UserPath -ieq 'LASTEXITCODE'
    }, $true))
    if ($scopedExitReferences.Count -ne 0) {
        throw "Native exit status can be shadowed in $relativePath"
    }
    $nonPortableProcessKills = @($ast.FindAll({
        param($node)
        $node -is [Management.Automation.Language.InvokeMemberExpressionAst] -and
            $node.Expression.Extent.Text -eq '$harnessProcess' -and
            $node.Member.Value -eq 'Kill' -and $node.Arguments.Count -ne 0
    }, $true))
    if ($nonPortableProcessKills.Count -ne 0) {
        throw "Owned harness cleanup uses a Process.Kill overload unavailable in Windows PowerShell: $relativePath"
    }
}

function Test-OwnedNetworkProcessExitCodes {
    param([string]$ProbeExecutable, [string]$FixtureRoot)

    # Exercise the production owned-harness try/catch/finally statements only.
    # Server startup, sockets and real product executables are outside this test.
    function Get-HarnessPortListeners { return @() }
    function Write-CapturedLog { param([string]$Path, [switch]$AsError) }
    foreach ($networkName in @('CharacterSelectIsolation')) {
        $relativeScript = "Tools/Network/Run-${networkName}Harness.ps1"
        $parseErrors = $null
        $tokens = $null
        $ast = [Management.Automation.Language.Parser]::ParseFile(
            (Join-Path $repoRoot $relativeScript), [ref]$tokens, [ref]$parseErrors)
        $nativeTry = $ast.EndBlock.Statements | Where-Object {
            $_ -is [Management.Automation.Language.TryStatementAst] -and
            @($_.Body.Statements | Where-Object {
                $_ -is [Management.Automation.Language.AssignmentStatementAst] -and
                    $_.Left.Extent.Text -eq '$harnessStartInfo'
            }).Count -eq 1
        } | Select-Object -First 1
        if ($parseErrors.Count -ne 0 -or $null -eq $nativeTry) {
            throw "Cannot locate the production owned-harness process block: $relativeScript"
        }
        $statements = @($nativeTry.Body.Statements)
        $first = 0
        while ($statements[$first].Extent.Text -notmatch '^\$harnessStartInfo\s*=') { ++$first }
        $body = ($statements[$first..($statements.Count - 1)] | ForEach-Object { $_.Extent.Text }) -join "`n"
        $catches = ($nativeTry.CatchClauses | ForEach-Object { $_.Extent.Text }) -join "`n"
        # Observe the owned PID before the unmodified production finally disposes
        # its handle, so a failed regression can still clean up only this fixture.
        $finallyBody = ($nativeTry.Finally.Statements | ForEach-Object { $_.Extent.Text }) -join "`n"
        $probeBody = [scriptblock]::Create("try {`n$body`n}`n$catches`nfinally {`n" +
            'if ($null -ne $harnessProcess) { $fixtureHarnessProcessId = $harnessProcess.Id }' +
            "`n$finallyBody`n}")
        foreach ($caseName in @('success', 'failure', 'timeout')) {
            $caseRoot = Join-Path $FixtureRoot "$networkName-$caseName"
            New-Item -ItemType Directory -Path $caseRoot -Force | Out-Null
            $harnessExe = $ProbeExecutable
            $harnessStdout = Join-Path $caseRoot 'harness.stdout.log'
            $harnessStderr = Join-Path $caseRoot 'harness.stderr.log'
            $serverStdout = Join-Path $caseRoot 'unused-server.stdout.log'
            $serverStderr = Join-Path $caseRoot 'unused-server.stderr.log'
            $HarnessPort = 1
            $Scenario = 'Core'
            $G02IdentityFast = $false
            # Only the extracted process block receives a short forced deadline;
            # the production runner's public CLI limits remain unchanged.
            $HarnessTimeoutMilliseconds = if ($caseName -eq 'timeout') { -4000 } else { 1000 }
            $harnessExternalTimeoutMilliseconds = if ($caseName -eq 'timeout') { 1000 } else { 5000 }
            $expectedExit = if ($caseName -eq 'success') { 0 } else { 37 }
            [Environment]::SetEnvironmentVariable('LOSTARK_NATIVE_HARNESS_EXIT_PROBE', $expectedExit.ToString(), 'Process')
            [Environment]::SetEnvironmentVariable('LOSTARK_NATIVE_HARNESS_PROBE_DELAY_MS',
                $(if ($caseName -eq 'timeout') { '30000' } else { '0' }), 'Process')
            $serverProcess = [Diagnostics.Process]::new()
            $serverProcess.StartInfo.FileName = $ProbeExecutable
            $serverProcess.StartInfo.UseShellExecute = $false
            $serverProcess.StartInfo.CreateNoWindow = $true
            $serverProcess.StartInfo.RedirectStandardOutput = $true
            $serverProcess.StartInfo.RedirectStandardError = $true
            $serverProcess.StartInfo.EnvironmentVariables['LOSTARK_NATIVE_HARNESS_PROBE_DELAY_MS'] = '30000'
            $harnessProcess = $null
            $fixtureHarnessProcessId = 0
            $harnessExitCode = -1
            $failure = $null
            $elapsed = [Diagnostics.Stopwatch]::StartNew()
            try {
                if (-not $serverProcess.Start()) { throw 'Owned cleanup fixture failed to start.' }
                $fixtureStdout = $serverProcess.StandardOutput.ReadToEndAsync()
                $fixtureStderr = $serverProcess.StandardError.ReadToEndAsync()
                try { . $probeBody *> $null }
                catch { $failure = $_ }
                $serverProcess.Refresh()
                $stdout = if (Test-Path -LiteralPath $harnessStdout) { [IO.File]::ReadAllText($harnessStdout) } else { '' }
                $stderr = if (Test-Path -LiteralPath $harnessStderr) { [IO.File]::ReadAllText($harnessStderr) } else { '' }
                $displayName = 'Character Select isolation harness'
                $expectedFailure = if ($caseName -eq 'timeout') {
                    "$displayName exceeded its external timeout: Core"
                } else { "$displayName failed with code 37." }
                $fixtureResidue = if ($fixtureHarnessProcessId -ne 0) {
                    Get-Process -Id $fixtureHarnessProcessId -ErrorAction SilentlyContinue
                } else { $null }
                if (($caseName -eq 'success' -and $null -ne $failure) -or
                    ($caseName -ne 'success' -and
                        ($null -eq $failure -or $failure.Exception.Message -ne $expectedFailure)) -or
                    ($caseName -ne 'timeout' -and $harnessExitCode -ne $expectedExit) -or
                    $stdout -notmatch "native-exit-probe stdout exit=$expectedExit" -or
                    $stderr -notmatch "native-exit-probe stderr exit=$expectedExit" -or
                    $fixtureHarnessProcessId -eq 0 -or $null -ne $fixtureResidue -or
                    -not $serverProcess.HasExited -or $elapsed.ElapsedMilliseconds -ge 7000) {
                    throw "Owned process contract failed: $networkName case=$caseName exit=$harnessExitCode error=$failure stdout=$stdout stderr=$stderr"
                }
                Write-Output "PASS $networkName owned-harness $caseName, diagnostics and bounded cleanup"
            }
            finally {
                # PID alone is not cleanup authority: a regression may leave only
                # this newly created fixture executable alive at the recorded PID.
                if ($fixtureHarnessProcessId -ne 0) {
                    $residue = Get-Process -Id $fixtureHarnessProcessId -ErrorAction SilentlyContinue
                    if ($null -ne $residue) {
                        if ($residue.Path -ne $ProbeExecutable) {
                            throw "Refusing to stop a non-fixture process at PID $fixtureHarnessProcessId"
                        }
                        $residue.Kill()
                        $null = $residue.WaitForExit(5000)
                        $residue.Dispose()
                    }
                }
                $serverProcess.Refresh()
                if (-not $serverProcess.HasExited) {
                    $serverProcess.Kill()
                    $null = $serverProcess.WaitForExit(5000)
                }
                $serverProcess.Dispose()
            }
        }
    }
}

function Test-WrapperStderrAndExitCodes {
    # Run unmodified copies of the production wrappers around a deterministic
    # console fixture. Never replace or stop an existing repository executable.
    $generatedRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot '.codex_tmp'))
    $fixtureRoot = Join-Path $generatedRoot ('native-exit-wrapper-' + [Guid]::NewGuid().ToString('N'))
    $probeVariable = 'LOSTARK_NATIVE_HARNESS_EXIT_PROBE'
    $previousProbe = [Environment]::GetEnvironmentVariable($probeVariable, 'Process')
    $delayVariable = 'LOSTARK_NATIVE_HARNESS_PROBE_DELAY_MS'
    $previousDelay = [Environment]::GetEnvironmentVariable($delayVariable, 'Process')
    $wrapperNames = @(
        'PointLightFalloffContractHarness'
    )
    try {
        New-Item -ItemType Directory -Path $fixtureRoot -Force | Out-Null
        $sourcePath = Join-Path $fixtureRoot 'NativeExitProbe.cs'
        $probeExecutable = Join-Path $fixtureRoot 'NativeExitProbe.exe'
        @'
using System;
internal static class NativeExitProbe
{
    private static int Main()
    {
        int code;
        if (!int.TryParse(Environment.GetEnvironmentVariable(
            "LOSTARK_NATIVE_HARNESS_EXIT_PROBE"), out code)) return 113;
        Console.Out.WriteLine("native-exit-probe stdout exit={0}", code);
        Console.Error.WriteLine("native-exit-probe stderr exit={0}", code);
        int delay;
        if (int.TryParse(Environment.GetEnvironmentVariable(
            "LOSTARK_NATIVE_HARNESS_PROBE_DELAY_MS"), out delay) && delay > 0)
            System.Threading.Thread.Sleep(delay);
        return code;
    }
}
'@ | Set-Content -LiteralPath $sourcePath -Encoding UTF8
        $compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
        if (-not (Test-Path -LiteralPath $compiler -PathType Leaf)) {
            throw "Native exit fixture requires the .NET Framework compiler: $compiler"
        }
        $previousNativeErrorAction = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $global:LASTEXITCODE = -1
            $compilerOutput = @(& $compiler /nologo /target:exe "/out:$probeExecutable" $sourcePath 2>&1)
            $compilerExitCode = $global:LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $previousNativeErrorAction
        }
        if ($compilerExitCode -ne 0) {
            throw "Native exit fixture compiler failed with code ${compilerExitCode}: $($compilerOutput -join ' ')"
        }
        foreach ($relativeDirectory in @(
            "Engine/Bin/$Configuration",
            'Engine/ThirdPartyLib/FMOD/Bin',
            "Engine/ThirdPartyLib/Assimp/Bin/$Configuration",
            "Engine/ThirdPartyLib/PhysX/Bin/$Configuration")) {
            New-Item -ItemType Directory -Path (Join-Path $fixtureRoot $relativeDirectory) -Force | Out-Null
        }
        foreach ($wrapperName in $wrapperNames) {
            $relativeScript = "Tools/$wrapperName/Run-$wrapperName.ps1"
            $fixtureScript = Join-Path $fixtureRoot $relativeScript
            $fixtureBinaryDirectory = Join-Path (Split-Path -Parent $fixtureScript) "Bin/$Configuration"
            New-Item -ItemType Directory -Path $fixtureBinaryDirectory -Force | Out-Null
            Copy-Item -LiteralPath (Join-Path $repoRoot $relativeScript) -Destination $fixtureScript
            Copy-Item -LiteralPath $probeExecutable -Destination (Join-Path $fixtureBinaryDirectory "$wrapperName.exe")
            foreach ($expectedExitCode in @(0, 37)) {
                [Environment]::SetEnvironmentVariable($probeVariable, $expectedExitCode.ToString(), 'Process')
                [Environment]::SetEnvironmentVariable($delayVariable, '0', 'Process')
                # This shadow must stay harmless even when the native process
                # writes stderr and returns a nonzero code through a pipeline.
                $LASTEXITCODE = 0
                $failure = $null
                $lines = [Collections.Generic.List[string]]::new()
                $pathBefore = $env:PATH
                $locationBefore = (Get-Location).Path
                try {
                    & $fixtureScript -Configuration $Configuration 2>&1 |
                        ForEach-Object { $lines.Add($_.ToString()) }
                }
                catch {
                    $failure = $_
                }
                $nativeExitCode = $global:LASTEXITCODE
                $output = $lines -join [Environment]::NewLine
                $expectedFailure = "$wrapperName failed with exit code $expectedExitCode"
                if ($nativeExitCode -ne $expectedExitCode -or $LASTEXITCODE -ne 0 -or
                    ($expectedExitCode -eq 0 -and $null -ne $failure) -or
                    ($expectedExitCode -ne 0 -and
                        ($null -eq $failure -or $failure.Exception.Message -ne $expectedFailure)) -or
                    $output -notmatch "native-exit-probe stdout exit=$expectedExitCode" -or
                    $output -notmatch "native-exit-probe stderr exit=$expectedExitCode" -or
                    $env:PATH -cne $pathBefore -or (Get-Location).Path -cne $locationBefore -or
                    $ErrorActionPreference -ne 'Stop') {
                    throw "Wrapper exit/stderr contract failed: $wrapperName expected=$expectedExitCode actual=$nativeExitCode error=$failure output=$output"
                }
                Write-Output "PASS $wrapperName stderr and native exit $expectedExitCode"
            }
        }
        Test-OwnedNetworkProcessExitCodes -ProbeExecutable $probeExecutable -FixtureRoot $fixtureRoot
    }
    finally {
        $restoreProbe = if ($null -eq $previousProbe) {
            [Management.Automation.Language.NullString]::Value
        }
        else { $previousProbe }
        [Environment]::SetEnvironmentVariable($probeVariable, $restoreProbe, 'Process')
        $restoreDelay = if ($null -eq $previousDelay) {
            [Management.Automation.Language.NullString]::Value
        }
        else { $previousDelay }
        [Environment]::SetEnvironmentVariable($delayVariable, $restoreDelay, 'Process')
        if (Test-Path -LiteralPath $fixtureRoot -PathType Container) {
            $resolvedFixtureRoot = (Resolve-Path -LiteralPath $fixtureRoot).Path
            if (-not $resolvedFixtureRoot.StartsWith(
                $generatedRoot.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
                throw "Native exit fixture cleanup escaped generated root: $resolvedFixtureRoot"
            }
            Remove-Item -LiteralPath $resolvedFixtureRoot -Recurse -Force
        }
    }
}

Test-WrapperStderrAndExitCodes
$global:LASTEXITCODE = 0
Write-Output "Native harness exit propagation: PASS ($Configuration, $($guardedScripts.Count) script guards, two wrapper cases and three owned-process cases)"
