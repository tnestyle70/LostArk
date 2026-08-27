param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $PSScriptRoot `
    "Bin\$Configuration\EffectRenderContractHarness.exe"
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "EffectRenderContractHarness was not built: $executable"
}

$runtimeDirectories = @(
    (Join-Path $repoRoot "Engine\Bin\$Configuration"),
    (Join-Path $repoRoot 'Engine\ThirdPartyLib\FMOD\Bin'),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\Assimp\Bin\$Configuration"),
    (Join-Path $repoRoot "Engine\ThirdPartyLib\PhysX\Bin\$Configuration")
)
foreach ($directory in $runtimeDirectories) {
    if (-not (Test-Path -LiteralPath $directory -PathType Container)) {
        throw "Effect render harness runtime directory is missing: $directory"
    }
}

$previousPath = $env:PATH
try {
    $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath

    & $executable $repoRoot
    if ($global:LASTEXITCODE -ne 0) {
        throw "EffectRenderContractHarness failed with exit code $global:LASTEXITCODE"
    }
}
finally {
    $env:PATH = $previousPath
}
