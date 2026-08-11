[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

function Assert-ExactExitCode {
    param(
        [int]$Actual,
        [int]$Expected,
        [string]$Context
    )
    if ($Actual -ne $Expected) {
        throw "$Context expected exit $Expected, actual $Actual"
    }
}

function Test-WriterIndependentManifestSemantics {
    param([string]$Path)
    try {
        $manifest = Get-Content -LiteralPath $Path -Raw -Encoding UTF8 |
            ConvertFrom-Json
    }
    catch {
        return $false
    }
    try {
        if ($manifest.formatVersion.GetType() -ne [int] -or
            $manifest.decodedByteCount.GetType() -ne [int] -or
            $manifest.expected.materialIndex.GetType() -ne [int] -or
            $manifest.expected.vertexCount.GetType() -ne [int] -or
            $manifest.expected.sourceFidelityFlags.GetType() -ne [int] -or
            $manifest.expected.externalAuthentication.GetType() -ne [bool] -or
            $manifest.expected.productAdmission.GetType() -ne [bool]) {
            return $false
        }
        return $manifest.schema -eq `
                'lostark.wmodel-v11-writer-independent-golden' -and
            $manifest.formatVersion -eq 1 -and
            $manifest.payloadFile -eq `
                'wmodel_v11_writer_independent_golden.hex' -and
            $manifest.payloadEncoding -eq `
                'LOWERCASE_HEX_WITH_ASCII_WHITESPACE_ONLY' -and
            $manifest.decodedByteCount -eq 850 -and
            $manifest.decodedSha256 -eq `
                '6bb409094185d9c41f6cb241d42bdc767b0a3868f7ed981d194e8fe1ccd23627' -and
            $manifest.testConstruction -eq `
                'FROZEN_BYTES_NO_RUNTIME_COOKER_IMPORT' -and
            $manifest.expected.outerVersion -eq '1.1' -and
            $manifest.expected.meshVersion -eq '1.1' -and
            $manifest.expected.meshName -eq 'fixture' -and
            $manifest.expected.materialIndex -eq 0 -and
            $manifest.expected.vertexCount -eq 3 -and
            (@($manifest.expected.indexU32) -join ',') -eq '0,2,1' -and
            $manifest.expected.sourceToWModelScale -eq 100.0 -and
            $manifest.expected.geometryPreScale -eq 0.01 -and
            $manifest.expected.sourceFidelityFlags -eq 0 -and
            -not $manifest.expected.externalAuthentication -and
            -not $manifest.expected.productAdmission
    }
    catch {
        return $false
    }
}

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
    Assert-ExactExitCode $LASTEXITCODE 0 `
        'WModel geometry harness fixture generation'

    $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath
    & $executable '--writer-independent-golden' $goldenHex $goldenExpected
    Assert-ExactExitCode $LASTEXITCODE 0 `
        'WModel writer-independent immutable golden'
    if (-not (Test-WriterIndependentManifestSemantics $goldenExpected)) {
        throw 'Valid writer-independent manifest failed semantic validation.'
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
    Assert-ExactExitCode $LASTEXITCODE 1 `
        'Mutated WModel writer-independent golden bytes'

    $hashMutatedManifestPath = Join-Path $suiteRoot `
        'hash-mutated-independent-golden.expected.json'
    $goldenManifestText = [IO.File]::ReadAllText($goldenExpected)
    [IO.File]::WriteAllText(
        $hashMutatedManifestPath,
        $goldenManifestText + ' ',
        $utf8NoBom)
    if (-not (Test-WriterIndependentManifestSemantics `
            $hashMutatedManifestPath)) {
        throw 'Whitespace-only manifest hash mutation changed semantics.'
    }
    & $executable '--writer-independent-golden' `
        $goldenHex $hashMutatedManifestPath
    Assert-ExactExitCode $LASTEXITCODE 1 `
        'Whitespace-only writer-independent manifest hash mutation'

    $semanticMutatedManifestPath = Join-Path $suiteRoot `
        'semantic-mutated-independent-golden.expected.json'
    $semanticMutatedManifestText = $goldenManifestText.Replace(
        '"decodedByteCount": 850',
        '"decodedByteCount": 851')
    if ($semanticMutatedManifestText -eq $goldenManifestText) {
        throw 'WModel writer-independent golden manifest mutation fixture changed.'
    }
    [IO.File]::WriteAllText(
        $semanticMutatedManifestPath,
        $semanticMutatedManifestText,
        $utf8NoBom)
    if (Test-WriterIndependentManifestSemantics $semanticMutatedManifestPath) {
        throw 'Semantic writer-independent manifest mutation was accepted.'
    }
    & $executable '--writer-independent-golden' `
        $goldenHex $semanticMutatedManifestPath
    Assert-ExactExitCode $LASTEXITCODE 1 `
        'Semantic writer-independent manifest mutation'
    Write-Output ('WModel manifest mutation contract: ' +
        'wholeFileHashRejected=1 semanticFieldRejected=1')

    $missingFixture = Join-Path $suiteRoot 'corrupt_header.wmodel'
    $missingFixtureBytes = [IO.File]::ReadAllBytes($missingFixture)
    [IO.File]::Delete($missingFixture)
    try {
        & $executable $suiteRoot
        Assert-ExactExitCode $LASTEXITCODE 1 `
            'Missing required corrupt fixture'
    }
    finally {
        [IO.File]::WriteAllBytes($missingFixture, $missingFixtureBytes)
    }

    $renamedFixture = Join-Path $suiteRoot 'corrupt_stride.wmodel'
    $renamedFixtureTarget = Join-Path $suiteRoot 'renamed_corrupt_stride.wmodel'
    if (Test-Path -LiteralPath $renamedFixtureTarget) {
        throw "Corrupt fixture rename target already exists: $renamedFixtureTarget"
    }
    [IO.File]::Move($renamedFixture, $renamedFixtureTarget)
    try {
        & $executable $suiteRoot
        Assert-ExactExitCode $LASTEXITCODE 1 `
            'Renamed required corrupt fixture'
    }
    finally {
        [IO.File]::Move($renamedFixtureTarget, $renamedFixture)
    }

    $caseRenamedFixture = Join-Path $suiteRoot 'corrupt_header.wmodel'
    $caseRenamedFixtureTarget = Join-Path $suiteRoot 'corrupt_Header.wmodel'
    $caseRenameStaging = Join-Path $suiteRoot 'case-rename-staging.tmp'
    if (Test-Path -LiteralPath $caseRenameStaging) {
        throw "Corrupt fixture case-rename staging path exists: $caseRenameStaging"
    }
    [IO.File]::Move($caseRenamedFixture, $caseRenameStaging)
    [IO.File]::Move($caseRenameStaging, $caseRenamedFixtureTarget)
    try {
        & $executable $suiteRoot
        Assert-ExactExitCode $LASTEXITCODE 1 `
            'Case-only renamed required corrupt fixture'
    }
    finally {
        [IO.File]::Move($caseRenamedFixtureTarget, $caseRenameStaging)
        [IO.File]::Move($caseRenameStaging, $caseRenamedFixture)
    }

    $emptyFixture = Join-Path $suiteRoot 'corrupt_bounds.wmodel'
    $emptyFixtureBytes = [IO.File]::ReadAllBytes($emptyFixture)
    [IO.File]::WriteAllBytes($emptyFixture, [byte[]]@())
    try {
        & $executable $suiteRoot
        Assert-ExactExitCode $LASTEXITCODE 1 `
            'Empty required corrupt fixture'
    }
    finally {
        [IO.File]::WriteAllBytes($emptyFixture, $emptyFixtureBytes)
    }

    & $executable $suiteRoot
    Assert-ExactExitCode $LASTEXITCODE 0 'Valid WModel geometry contract suite'
}
finally {
    $env:PATH = $previousPath
    if (Test-Path -LiteralPath $suiteRoot) {
        Remove-Item -LiteralPath $suiteRoot -Recurse -Force
    }
}
