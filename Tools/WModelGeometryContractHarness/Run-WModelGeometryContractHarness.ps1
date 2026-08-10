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
$goldenHex = Join-Path $PSScriptRoot `
    'Fixtures\wmodel_v11_writer_independent_golden.hex'
$goldenExpected = Join-Path $PSScriptRoot `
    'Fixtures\wmodel_v11_writer_independent_golden.expected.json'
foreach ($fixture in @($goldenHex, $goldenExpected)) {
    if (-not (Test-Path -LiteralPath $fixture -PathType Leaf)) {
        throw "WModel writer-independent golden fixture is missing: $fixture"
    }
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
    & $executable '--writer-independent-golden' $goldenHex $goldenExpected
    if ($LASTEXITCODE -ne 0) {
        throw "WModel writer-independent immutable golden failed: $LASTEXITCODE"
    }
    $utf8NoBom = New-Object Text.UTF8Encoding($false)
    $mutatedHexPath = Join-Path $suiteRoot 'mutated-independent-golden.hex'
    $goldenHexText = [IO.File]::ReadAllText($goldenHex)
    if (-not $goldenHexText.StartsWith('5', [StringComparison]::Ordinal)) {
        throw 'WModel writer-independent golden hex mutation fixture changed.'
    }
    [IO.File]::WriteAllText(
        $mutatedHexPath,
        '0' + $goldenHexText.Substring(1),
        $utf8NoBom)
    & $executable '--writer-independent-golden' $mutatedHexPath $goldenExpected
    if ($LASTEXITCODE -eq 0) {
        throw 'Mutated WModel writer-independent golden bytes were accepted.'
    }

    $mutatedManifestPath = Join-Path $suiteRoot `
        'mutated-independent-golden.expected.json'
    $goldenManifestText = [IO.File]::ReadAllText($goldenExpected)
    $mutatedManifestText = $goldenManifestText.Replace(
        '"decodedByteCount": 850',
        '"decodedByteCount": 851')
    if ($mutatedManifestText -eq $goldenManifestText) {
        throw 'WModel writer-independent golden manifest mutation fixture changed.'
    }
    [IO.File]::WriteAllText(
        $mutatedManifestPath,
        $mutatedManifestText,
        $utf8NoBom)
    & $executable '--writer-independent-golden' $goldenHex $mutatedManifestPath
    if ($LASTEXITCODE -eq 0) {
        throw 'Mutated WModel writer-independent golden manifest was accepted.'
    }
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
