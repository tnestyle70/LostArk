[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$executable = Join-Path $PSScriptRoot `
    "Bin\$Configuration\WModelGeometryContractHarness.exe"
if (-not (Test-Path -LiteralPath $executable)) {
    throw "WModelGeometryContractHarness was not built: $executable"
}

$temporaryRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$suiteRoot = [IO.Path]::GetFullPath((Join-Path $temporaryRoot `
    "lostark-wmodel-geometry-$PID-$Configuration"))
if (-not $suiteRoot.StartsWith($temporaryRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Geometry harness suite escaped the temporary root: $suiteRoot"
}

$runtimeDirectories = @(
    (Join-Path $repoRoot "Engine\Bin\$Configuration"),
    (Join-Path $repoRoot 'Engine\ThirdPartyLib\FMOD\Bin'),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\Assimp\Bin\$Configuration"),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\PhysX\Bin\$Configuration")
)
foreach ($directory in $runtimeDirectories) {
    if (-not (Test-Path -LiteralPath $directory)) {
        throw "Geometry harness runtime directory is missing: $directory"
    }
}

$previousPath = $env:PATH
try {
    if (Test-Path -LiteralPath $suiteRoot) {
        Remove-Item -LiteralPath $suiteRoot -Recurse -Force
    }
    & python -B `
        (Join-Path $repoRoot `
            'Tools\ModelAssetConverter\test_cook_wmodel_geometry_contract.py') `
        '--write-harness-suite' $suiteRoot
    if ($LASTEXITCODE -ne 0) {
        throw "WModel geometry harness fixture generation failed: $LASTEXITCODE"
    }

    $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath
    & $executable $suiteRoot
    if ($LASTEXITCODE -ne 0) {
        throw "WModelGeometryContractHarness failed: $LASTEXITCODE"
    }
}
finally {
    $env:PATH = $previousPath
    if (Test-Path -LiteralPath $suiteRoot) {
        Remove-Item -LiteralPath $suiteRoot -Recurse -Force
    }
}
