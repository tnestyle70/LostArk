[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$mapFrustumExecutable = Join-Path $PSScriptRoot "Bin\$Configuration\MapFrustumContractHarness.exe"
$mapFrustumBinaryRoot = Split-Path -Parent $mapFrustumExecutable
$mapFrustumProcess = $null
$mapFrustumExitCode = 2
$mapFrustumStarted = $false

try {
    if (-not (Test-Path -LiteralPath $mapFrustumExecutable -PathType Leaf)) {
        throw "MapFrustumContractHarness was not built: $mapFrustumExecutable"
    }
    $mapFrustumAssimp = if ($Configuration -eq 'Debug') {
        'assimp-vc143-mtd.dll'
    }
    else {
        'assimp-vc143-mt.dll'
    }
    foreach ($mapFrustumDependency in @(
        'Engine.dll', 'fmod.dll', $mapFrustumAssimp,
        'PhysX_64.dll', 'PhysXCommon_64.dll', 'PhysXFoundation_64.dll')) {
        $mapFrustumDependencyPath = Join-Path $mapFrustumBinaryRoot $mapFrustumDependency
        if (-not (Test-Path -LiteralPath $mapFrustumDependencyPath -PathType Leaf)) {
            throw "MapFrustum harness runtime dependency is missing: $mapFrustumDependencyPath"
        }
    }

    $mapFrustumStartInfo = [Diagnostics.ProcessStartInfo]::new()
    $mapFrustumStartInfo.FileName = $mapFrustumExecutable
    $mapFrustumStartInfo.WorkingDirectory = $mapFrustumBinaryRoot
    $mapFrustumStartInfo.UseShellExecute = $false
    $mapFrustumStartInfo.CreateNoWindow = $true
    $mapFrustumStartInfo.RedirectStandardOutput = $true
    $mapFrustumStartInfo.RedirectStandardError = $true
    $mapFrustumProcess = [Diagnostics.Process]::new()
    $mapFrustumProcess.StartInfo = $mapFrustumStartInfo
    $mapFrustumStarted = $mapFrustumProcess.Start()
    if (-not $mapFrustumStarted) {
        throw 'MapFrustumContractHarness failed to start.'
    }
    $mapFrustumStdoutTask = $mapFrustumProcess.StandardOutput.ReadToEndAsync()
    $mapFrustumStderrTask = $mapFrustumProcess.StandardError.ReadToEndAsync()
    $mapFrustumTimedOut = -not $mapFrustumProcess.WaitForExit(60000)
    if ($mapFrustumTimedOut) {
        # This owned console creates no windows or child processes.
        $mapFrustumProcess.Kill()
        if (-not $mapFrustumProcess.WaitForExit(5000)) {
            throw 'MapFrustumContractHarness did not stop after timeout.'
        }
    }
    $mapFrustumExitCode = $mapFrustumProcess.ExitCode
    if (-not $mapFrustumStdoutTask.Wait(5000) -or
        -not $mapFrustumStderrTask.Wait(5000)) {
        throw 'MapFrustumContractHarness output did not close after exit.'
    }
    [Console]::Out.Write($mapFrustumStdoutTask.GetAwaiter().GetResult())
    [Console]::Error.Write($mapFrustumStderrTask.GetAwaiter().GetResult())
    if ($mapFrustumTimedOut) {
        [Console]::Error.WriteLine('MapFrustumContractHarness exceeded its 60-second timeout.')
        $mapFrustumExitCode = 124
    }
}
catch {
    [Console]::Error.WriteLine($_.Exception.Message)
    # Setup/stream failures use 2; normal native completion retains its exact code.
    $mapFrustumExitCode = 2
}
finally {
    if ($null -ne $mapFrustumProcess) {
        if ($mapFrustumStarted -and -not $mapFrustumProcess.HasExited) {
            $mapFrustumProcess.Kill()
            $null = $mapFrustumProcess.WaitForExit(5000)
        }
        $mapFrustumProcess.Dispose()
    }
}
exit $mapFrustumExitCode
