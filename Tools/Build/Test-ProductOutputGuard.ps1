[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$guardModulePath = Join-Path $PSScriptRoot 'ProductOutputGuard.psm1'
$runnerPath = Join-Path $PSScriptRoot 'Invoke-BuildAndRegression.ps1'
$fixtureParent = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\', '/')
$fixtureRoot = Join-Path $fixtureParent `
    ("LostArk-ProductOutputGuard-{0}" -f [Guid]::NewGuid().ToString('N'))
$ownedProcesses = [Collections.Generic.List[Diagnostics.Process]]::new()
$checks = 0

function Assert-FixtureCondition {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )
    if (-not $Condition) { throw "Product output guard fixture failed: $Message" }
    $script:checks++
}

function Start-OwnedFixtureProcess {
    param([Parameter(Mandatory = $true)][string]$ExecutablePath)

    $process = Start-Process -FilePath $ExecutablePath -PassThru `
        -WindowStyle Hidden -ArgumentList @('127.0.0.1', '-n', '120', '-w', '1000')
    $ownedProcesses.Add($process) | Out-Null
    for ($attempt = 0; $attempt -lt 50; ++$attempt) {
        $process.Refresh()
        if (-not $process.HasExited) { return $process }
        Start-Sleep -Milliseconds 20
    }
    throw "Fixture executable exited before inspection: $ExecutablePath"
}

function Write-FakeBuildDomainModule {
    param([Parameter(Mandatory = $true)][string]$Path)

    $source = @'
function Read-BuildDomainManifest {
    [pscustomobject]@{ domains = @([pscustomobject]@{ id = 'fixture.publisher' }) }
}
function Enter-BuildExclusiveLock { [IO.MemoryStream]::new() }
function Get-BuildDomainsForProfile { param($Manifest, $Profile); @($Manifest.domains) }
function Invoke-BuildDomain {
    param($RepositoryRoot, $Domain, $ResourceRoot, $ReceiptRoot)
    [IO.Directory]::CreateDirectory((Join-Path $RepositoryRoot 'Data')) | Out-Null
    [IO.File]::WriteAllText(
        (Join-Path $RepositoryRoot 'Data\publisher-was-invoked.marker'), 'mutated')
    [pscustomobject]@{
        domainId = 'fixture.publisher'
        reused = $false
        fingerprintSha256 = 'fixture'
        receiptSha256 = 'fixture'
    }
}
'@
    [IO.File]::WriteAllText($Path, $source, [Text.UTF8Encoding]::new($false))
}

function Invoke-FakeRepositoryRunner {
    param([Parameter(Mandatory = $true)][string]$FakeRunnerPath)

    $powershell = (Get-Command powershell.exe -ErrorAction Stop).Source
    $startInfo = [Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $powershell
    $startInfo.Arguments = ('-NoProfile -ExecutionPolicy Bypass -File "{0}" ' +
        '-Configuration Debug -Profile Product -SkipBuild') -f $FakeRunnerPath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    if (-not $process.Start()) { throw 'Failed to start the fake repository runner.' }
    $ownedProcesses.Add($process) | Out-Null
    $stdout = $process.StandardOutput.ReadToEnd()
    $stderr = $process.StandardError.ReadToEnd()
    if (-not $process.WaitForExit(15000)) {
        $process.Kill()
        throw 'Fake repository runner did not reject the locked output within 15 seconds.'
    }
    return [pscustomobject]@{
        ExitCode = $process.ExitCode
        Output = $stdout + [Environment]::NewLine + $stderr
    }
}

try {
    [IO.Directory]::CreateDirectory($fixtureRoot) | Out-Null
    Import-Module $guardModulePath -Force

    $standardOutputs = @(Get-StandardProductOutputPaths `
        -RepositoryRoot $fixtureRoot)
    Assert-FixtureCondition ($standardOutputs.Count -eq 4) `
        'Debug/Release Client/Server must produce exactly four standard targets.'
    Assert-FixtureCondition (
        @(Get-RunningStandardProductOutputs -RepositoryRoot $fixtureRoot).Count -eq 0) `
        'an empty fake repository must pass.'
    Assert-StandardProductOutputsNotRunning -RepositoryRoot $fixtureRoot
    $checks++

    $systemPing = Join-Path $env:SystemRoot 'System32\ping.exe'
    Assert-FixtureCondition ([IO.File]::Exists($systemPing)) `
        "fixture executable is missing: $systemPing"

    $otherDirectory = Join-Path $fixtureRoot 'Other'
    [IO.Directory]::CreateDirectory($otherDirectory) | Out-Null
    $otherClient = Join-Path $otherDirectory 'Client.exe'
    Copy-Item -LiteralPath $systemPing -Destination $otherClient
    [void](Start-OwnedFixtureProcess $otherClient)
    Assert-FixtureCondition (
        @(Get-RunningStandardProductOutputs -RepositoryRoot $fixtureRoot).Count -eq 0) `
        'a same-named executable outside standard output paths must not match.'
    Assert-StandardProductOutputsNotRunning -RepositoryRoot $fixtureRoot
    $checks++

    $releaseServer = Join-Path $fixtureRoot 'Server\Bin\Release\Server.exe'
    [IO.Directory]::CreateDirectory(
        [IO.Path]::GetDirectoryName($releaseServer)) | Out-Null
    Copy-Item -LiteralPath $systemPing -Destination $releaseServer
    $releaseProcess = Start-OwnedFixtureProcess $releaseServer
    $running = @(Get-RunningStandardProductOutputs -RepositoryRoot $fixtureRoot)
    Assert-FixtureCondition ($running.Count -eq 1) `
        'the Release Server fixture must be discovered once.'
    Assert-FixtureCondition (
        $running[0].Configuration -ceq 'Release' -and
        $running[0].Role -ceq 'Server' -and
        $running[0].ProcessId -eq $releaseProcess.Id -and
        [string]::Equals(
            $running[0].Path, [IO.Path]::GetFullPath($releaseServer),
            [StringComparison]::OrdinalIgnoreCase)) `
        'the discovered process must preserve exact configuration, role, PID, and path.'

    $directMessage = ''
    try {
        Assert-StandardProductOutputsNotRunning -RepositoryRoot $fixtureRoot
    }
    catch {
        $directMessage = $_.Exception.Message
    }
    Assert-FixtureCondition (
        $directMessage.Contains("PID $($releaseProcess.Id)") -and
        $directMessage.Contains([IO.Path]::GetFullPath($releaseServer))) `
        'direct rejection must report the exact fixture PID and path.'

    $fakeBuildDirectory = Join-Path $fixtureRoot 'Tools\Build'
    [IO.Directory]::CreateDirectory($fakeBuildDirectory) | Out-Null
    Copy-Item -LiteralPath $runnerPath -Destination `
        (Join-Path $fakeBuildDirectory 'Invoke-BuildAndRegression.ps1')
    Copy-Item -LiteralPath $guardModulePath -Destination `
        (Join-Path $fakeBuildDirectory 'ProductOutputGuard.psm1')
    Write-FakeBuildDomainModule `
        (Join-Path $fakeBuildDirectory 'BuildDomainPipeline.psm1')
    [IO.File]::WriteAllText(
        (Join-Path $fakeBuildDirectory 'BuildDomains.json'), '{}',
        [Text.UTF8Encoding]::new($false))

    foreach ($gate in @(
            'test_release_client_surface_contract.py',
            'test_build_profile_contract.py',
            'test_build_domain_pipeline_receipts.py')) {
        [IO.File]::WriteAllText(
            (Join-Path $fakeBuildDirectory $gate), 'raise SystemExit(0)',
            [Text.UTF8Encoding]::new($false))
    }

    $fakeRunner = Join-Path $fakeBuildDirectory 'Invoke-BuildAndRegression.ps1'
    $integration = Invoke-FakeRepositoryRunner $fakeRunner
    $mutationMarker = Join-Path $fixtureRoot `
        'Data\publisher-was-invoked.marker'
    $compactIntegrationOutput = [Text.RegularExpressions.Regex]::Replace(
        $integration.Output, '\s+', '')
    $compactReleasePath = [Text.RegularExpressions.Regex]::Replace(
        [IO.Path]::GetFullPath($releaseServer), '\s+', '')
    Assert-FixtureCondition ($integration.ExitCode -ne 0) `
        '-SkipBuild must reject a Release standard process while Debug is selected.'
    Assert-FixtureCondition (
        $compactIntegrationOutput.Contains("PID$($releaseProcess.Id)") -and
        $compactIntegrationOutput.Contains($compactReleasePath)) `
        ('the Invoke runner must surface the exact cross-configuration PID and path. ' +
        "Child output: $($integration.Output)")
    Assert-FixtureCondition (-not [IO.File]::Exists($mutationMarker)) `
        'the fake publisher must not mutate shared Data before output rejection.'

    Write-Host "ProductOutputGuard fixture: $checks/$checks passed"
}
finally {
    foreach ($process in @($ownedProcesses)) {
        try {
            $process.Refresh()
            if (-not $process.HasExited) {
                Stop-Process -Id $process.Id -Force -ErrorAction Stop
                $process.WaitForExit(5000) | Out-Null
            }
        }
        catch {
            Write-Warning "Failed to stop owned fixture PID $($process.Id): $($_.Exception.Message)"
        }
        finally {
            $process.Dispose()
        }
    }

    $resolvedFixture = [IO.Path]::GetFullPath($fixtureRoot)
    $requiredPrefix = $fixtureParent + [IO.Path]::DirectorySeparatorChar
    $safeFixture = $resolvedFixture.StartsWith(
            $requiredPrefix, [StringComparison]::OrdinalIgnoreCase) -and
        [IO.Path]::GetFileName($resolvedFixture).StartsWith(
            'LostArk-ProductOutputGuard-', [StringComparison]::Ordinal)
    if ([IO.Directory]::Exists($resolvedFixture)) {
        if (-not $safeFixture) {
            throw "Refusing to remove an unsafe fixture path: $resolvedFixture"
        }
        Remove-Item -LiteralPath $resolvedFixture -Recurse -Force
    }
}
