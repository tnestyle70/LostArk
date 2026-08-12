[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep,
    [string]$SourcePackageRoot = '',
    [string]$ResourceRoot = '',
    [string]$ExtractionRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/extract_artist_31470_native_base_textures.py'
$test = 'Tools/LevelPlacementExtractor/test_extract_artist_31470_native_base_textures.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.native-base-texture-extraction.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B $test
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F native base texture unit tests failed: $LASTEXITCODE"
    }

    if ($Deep) {
        if ([string]::IsNullOrWhiteSpace($SourcePackageRoot) -or
            -not (Test-Path -LiteralPath $SourcePackageRoot -PathType Container) -or
            [string]::IsNullOrWhiteSpace($ResourceRoot) -or
            -not (Test-Path -LiteralPath $ResourceRoot -PathType Container) -or
            [string]::IsNullOrWhiteSpace($ExtractionRoot) -or
            -not (Test-Path -LiteralPath $ExtractionRoot -PathType Container)) {
            throw 'Deep Artist F native base texture audit requires source package, resource, and extraction roots.'
        }
        & python -B $generator `
            '--source-package-root' $SourcePackageRoot `
            '--resource-root' $ResourceRoot `
            '--extraction-root' $ExtractionRoot `
            '--output' $receiptPath `
            '--check'
    }
    else {
        & python -B $generator '--output' $receiptPath '--validate-only'
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F native base texture receipt check failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json
    $summary = $receipt.summary
    $decision = $receipt.decision
    if ($receipt.schema -cne 'lostark.artist-31470-native-base-texture-extraction-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.status -cne 'SOURCE_EXTRACTED_BASE_DEFAULT_ONLY' -or
        @($receipt.assets).Count -ne 4 -or
        [int]$summary.sourcePackageCount -ne 2 -or
        [int]$summary.sourceExactTexture2DCount -ne 4 -or
        [int]$summary.freshDdsVerifiedCount -ne 4 -or
        [int]$summary.baseDefaultReplacedByActiveMicCount -ne 4 -or
        [int]$summary.runtimeDeploymentAdmittedCount -ne 0 -or
        [int]$summary.runtimeBindingAdmittedCount -ne 0 -or
        [int]$summary.productAdmittedCount -ne 0 -or
        -not [bool]$decision.sourceExactBaseDefaultTextureEvidence -or
        [bool]$decision.activeArtist31470BaseDefaultEffective -or
        [bool]$decision.runtimeDeploymentAdmission -or
        [bool]$decision.runtimeBindingAdmission -or
        [bool]$decision.shaderGraphAdmission -or
        [bool]$decision.productAdmission) {
        throw 'Artist F native base texture denominator or admission boundary changed.'
    }
    foreach ($asset in @($receipt.assets)) {
        if ($null -ne $asset.runtimeAssetId -or
            [bool]$asset.runtimeBindingAdmission -or
            [bool]$asset.productAdmission -or
            $asset.baseLookupEvidence.status -cne 'SOURCE_EXACT_BASE_DEFAULT_REPLACED_BY_ACTIVE_MIC') {
            throw "Artist F native base texture runtime boundary changed: $($asset.logicalTexturePath)"
        }
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 native base textures mode=$mode packages=2 assets=4 fresh-dds=4 active-effective=0 runtime-binding=0 product=0"
}
finally {
    Pop-Location
}
