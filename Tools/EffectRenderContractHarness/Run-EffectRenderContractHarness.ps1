param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',

    [Parameter(Mandatory = $true)]
    [ValidateRange(0, 65536)]
    [int]$ExpectedBindingCount
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$productCatalog = Join-Path $repoRoot `
    'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
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

foreach ($inputFile in @($productCatalog)) {
    if (-not (Test-Path -LiteralPath $inputFile -PathType Leaf)) {
        throw "Effect render harness input is missing: $inputFile"
    }
}

$previousPath = $env:PATH
try {
    $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath

    & $executable $repoRoot $ExpectedBindingCount $productCatalog
    if ($LASTEXITCODE -ne 0) {
        throw "EffectRenderContractHarness failed with exit code $LASTEXITCODE"
    }
}
finally {
    $env:PATH = $previousPath
}
