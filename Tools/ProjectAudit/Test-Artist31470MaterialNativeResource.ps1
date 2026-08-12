[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$Deep,
    [string]$SourcePackageRoot = '',
    [string]$GlobalMaterialPackage = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/extract_artist_31470_material_native_resource.py'
$test = 'Tools/LevelPlacementExtractor/test_extract_artist_31470_material_native_resource.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.material-native-resource.receipt.json'

Push-Location $RepositoryRoot
try {
    & python -B $test
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material native resource unit tests failed: $LASTEXITCODE"
    }

    if ($Deep) {
        if ([string]::IsNullOrWhiteSpace($SourcePackageRoot) -or
            -not (Test-Path -LiteralPath $SourcePackageRoot -PathType Container) -or
            [string]::IsNullOrWhiteSpace($GlobalMaterialPackage) -or
            -not (Test-Path -LiteralPath $GlobalMaterialPackage -PathType Leaf)) {
            throw 'Deep Artist F Material native resource audit requires source package root and global Material package.'
        }
        & python -B $generator `
            '--source-package-root' $SourcePackageRoot `
            '--global-material-package' $GlobalMaterialPackage `
            '--output' $receiptPath `
            '--check'
    }
    else {
        & python -B $generator '--output' $receiptPath '--validate-only'
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material native resource receipt check failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $decision = $receipt.decision
    if ($receipt.schema -cne 'lostark.artist-31470-material-native-resource-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        @($receipt.families).Count -ne 23 -or
        [int]$summary.sourceParsedTailCount -ne 23 -or
        [int]$summary.currentGlobalParsedTailCount -ne 23 -or
        [int]$summary.sourceGlobalSemanticParityCount -ne 23 -or
        [int]$summary.referencedTextureCount -ne 64 -or
        [int]$summary.legacyTextureLookupFamilyCount -ne 4 -or
        [int]$summary.legacyTextureLookupCount -ne 7 -or
        [int]$summary.sourceUnparsedByteCount -ne 0 -or
        [int]$summary.currentGlobalUnparsedByteCount -ne 0 -or
        [int]$summary.inlineShaderPayloadFamilyCount -ne 0 -or
        [int]$summary.uniqueReferencedTextureCount -ne 55 -or
        [int]$summary.artistRuntimeBoundReferencedTextureCount -ne 17 -or
        [int]$summary.artistRuntimeUnboundReferencedTextureCount -ne 38 -or
        [int]$summary.artistDomainAvailableReferencedTextureCount -ne 34 -or
        [int]$summary.artistDomainMissingReferencedTextureCount -ne 21 -or
        [int]$summary.artistMissingButOtherEffectDomainAvailableCount -ne 11 -or
        [int]$summary.allEffectDomainsMissingReferencedTextureCount -ne 10 -or
        [int]$summary.lookupBackedUniqueTextureCount -ne 7 -or
        [int]$summary.unboundLookupBackedUniqueTextureCount -ne 4 -or
        [int]$summary.overriddenLookupBackedUniqueTextureCount -ne 5 -or
        [int]$summary.differentReplacementLookupBackedUniqueTextureCount -ne 4 -or
        [int]$summary.identityOverrideLookupBackedUniqueTextureCount -ne 1 -or
        [int]$summary.effectiveLookupBackedUniqueTextureCount -ne 3 -or
        [int]$summary.effectiveLookupMissingRuntimeBindingCount -ne 0 -or
        [int]$summary.overriddenLookupEffectiveOverrideMissingRuntimeBindingCount -ne 0 -or
        [int]$summary.mainDissolvePriorityFamilyCount -ne 2 -or
        [int]$summary.mainDissolveLegacyTextureLookupCount -ne 0 -or
        [int]$summary.mainDissolveMetadataEffectiveTextureMembershipDeltaCount -ne 0 -or
        @($receipt.mainDissolvePriority.rows).Count -ne 2 -or
        [int]$receipt.mainDissolvePriority.summary.effectiveBindingCount -ne 9 -or
        [int]$receipt.mainDissolvePriority.summary.nativeEffectiveTextureOverlapCount -ne 1 -or
        $receipt.mainDissolvePriority.nextEvidenceOwner -cne 'MATCHING_REVISION_FMATERIALSHADERMAP_OR_SHADER_MAP_ID_DERIVATION' -or
        -not [bool]$decision.sourceExactLegacyTextureMetadata -or
        [bool]$decision.shaderUvUniformAdmission -or
        [bool]$decision.arithmeticGraphAdmission -or
        [bool]$decision.inlineShaderPayloadAvailable -or
        [bool]$decision.productAdmission) {
        throw 'Artist F Material native resource denominator or fidelity boundary changed.'
    }

    $mode = if ($Deep) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 Material native resource mode=$mode families=23 textures=64/55 exact-bound=17 artist-dds=34/55 lookups=4/7 effective=3 replaced=4 identity-override=1 main-dissolve=2 lookup=0 delta=0 effective-missing=0 parity=23 remainder=0 inline-shader=0 product=false"
}
finally {
    Pop-Location
}
