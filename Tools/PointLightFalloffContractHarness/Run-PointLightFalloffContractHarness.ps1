param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $PSScriptRoot `
    "Bin\$Configuration\PointLightFalloffContractHarness.exe"

if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "PointLightFalloffContractHarness was not built: $executable"
}

$runtimeDirectories = @(
    (Join-Path $repoRoot "Engine\Bin\$Configuration"),
    (Join-Path $repoRoot 'Engine\ThirdPartyLib\FMOD\Bin'),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\Assimp\Bin\$Configuration"),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\PhysX\Bin\$Configuration")
)
foreach ($directory in $runtimeDirectories) {
    if (-not (Test-Path -LiteralPath $directory -PathType Container)) {
        throw "PointLight harness runtime directory is missing: $directory"
    }
}

$previousPath = $env:PATH
try {
    $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath
    & $executable $repoRoot
    if ($LASTEXITCODE -ne 0) {
        throw "PointLightFalloffContractHarness failed with exit code $LASTEXITCODE"
    }
}
finally {
    $env:PATH = $previousPath
}
