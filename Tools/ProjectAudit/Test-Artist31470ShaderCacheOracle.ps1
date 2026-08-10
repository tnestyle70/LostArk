[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepShaderCacheAudit,
    [string]$SourcePackageRoot = '',
    [string]$MaterialInventoryRoot = '',
    [string]$GlobalMaterialPackage = '',
    [string]$ShaderCachePackage = '',
    [string]$D3DCompiler = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/extract_artist_31470_shader_cache_oracle.py'
$test = 'Tools/LevelPlacementExtractor/test_extract_artist_31470_shader_cache_oracle.py'
$materialContract = 'Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.shader-cache-oracle.receipt.json'

Push-Location $RepositoryRoot
try {
    $savedErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $unitOutput = (& python -B $test 2>&1 | Out-String).Trim()
        $unitExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $savedErrorActionPreference
    }
    if ($unitExitCode -ne 0) {
        throw "Artist F ShaderCache unit tests failed: $unitExitCode $unitOutput"
    }

    if ($DeepShaderCacheAudit) {
        foreach ($path in @(
            $SourcePackageRoot,
            $MaterialInventoryRoot,
            $GlobalMaterialPackage,
            $ShaderCachePackage,
            $D3DCompiler
        )) {
            if ([string]::IsNullOrWhiteSpace($path) -or
                -not (Test-Path -LiteralPath $path)) {
                throw 'Deep Artist F ShaderCache audit requires all five explicit external paths.'
            }
        }
        & python -B $generator `
            '--material-contract' $materialContract `
            '--source-package-root' $SourcePackageRoot `
            '--material-inventory-root' $MaterialInventoryRoot `
            '--global-material-package' $GlobalMaterialPackage `
            '--shader-cache-package' $ShaderCachePackage `
            '--d3dcompiler' $D3DCompiler `
            '--output' $receiptPath `
            '--check'
    }
    else {
        & python -B $generator `
            '--material-contract' $materialContract `
            '--output' $receiptPath `
            '--validate-only'
    }
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F ShaderCache receipt check failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $decision = $receipt.joinDecision
    $probe = $receipt.micTailShaderObjectIdProbe
    if ($receipt.schema -cne 'lostark.artist-31470-shader-cache-oracle-receipt' -or
        [int]$receipt.formatVersion -ne 2 -or
        @($receipt.materialNativeKeys).Count -ne 23 -or
        @($receipt.recipeNativeKeys).Count -ne 27 -or
        @($receipt.materialTopologyCompleteness).Count -ne 23 -or
        @($receipt.shaderCacheCandidates).Count -ne 11 -or
        @($receipt.primaryShaderCache.codeRecords).Count -ne 271 -or
        [int]$summary.primaryLz4DecodedCodeRecordCount -ne 271 -or
        [int]$summary.primaryDxbcTotalSizeValidatedCount -ne 271 -or
        [int]$summary.primaryD3dDisassemblyValidatedCount -ne 271 -or
        [int]$summary.primaryUniqueDxbcCount -ne 240 -or
        [int]$summary.primaryDecodedShaderObjectCount -ne 271 -or
        [int]$summary.primaryDecodedMaterialShaderMapCount -ne 25 -or
        [int]$summary.primaryDecodedShaderReferenceCount -ne 534 -or
        [int]$summary.materialInstanceRecipeCount -ne 25 -or
        [int]$summary.staticPermutationMicCount -ne 24 -or
        [int]$summary.boundedInventoryReportCount -ne 30 -or
        [int]$summary.boundedInventoryFamilyCoverageCount -ne 21 -or
        [int]$summary.boundedInventoryAlternateObjectCandidateCount -ne 1 -or
        [int]$summary.strictCrossRevisionTopologyImprovementCandidateCount -ne 0 -or
        [int]$probe.alignedNonzero16ByteWindowCount -ne 4816 -or
        [int]$probe.descriptorShaderIdCandidateCount -ne 271 -or
        [int]$probe.directShaderIdMatchCount -ne 0 -or
        [string]$probe.status -cne 'MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID' -or
        [int]$decision.exactMaterialShaderMapJoinCount -ne 0 -or
        [int]$decision.exactMicStaticParameterSetJoinCount -ne 0 -or
        [int]$decision.reconstructedNumericallyVerifiedFamilyCount -ne 0 -or
        [bool]$receipt.admission.executionAdmission -or
        [bool]$receipt.admission.productAdmission) {
        throw 'Artist F ShaderCache denominator or fail-closed admission changed.'
    }

    $mode = if ($DeepShaderCacheAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 ShaderCache mode=$mode material=23 recipe=27 mic=25/24 windows=4816 cache=1596 objects/maps/refs=271/25/534 dxbc=271/271/271 unique=240 joins=0/23,0/24 product=false"
}
finally {
    Pop-Location
}
