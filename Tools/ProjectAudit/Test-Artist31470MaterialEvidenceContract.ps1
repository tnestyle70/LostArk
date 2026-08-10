[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepMaterialAudit,
    [string]$SourcePackageRoot = '',
    [string]$ExactDdsRoot = '',
    [string]$SourcePackManifest = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$materialRoot = 'Data/Effects/Imported/Artist/Materials'
$activeInventory = 'Data/Effects/Imported/Artist/skill.31470.source-active-effect-inventory.receipt.json'
$materialClosure = "$materialRoot/skill.31470.active-material-closure.json"
$ddsReceipt = "$materialRoot/skill.31470.exact-dds-recovery.receipt.json"
$renderReceipt = "$materialRoot/skill.31470.material-render-state-evidence.receipt.json"
$contractPath = "$materialRoot/skill.31470.typed-material-evidence-contract.json"

Push-Location $RepositoryRoot
try {
    if ($DeepMaterialAudit) {
        foreach ($path in @($SourcePackageRoot, $ExactDdsRoot, $SourcePackManifest)) {
            if ([string]::IsNullOrWhiteSpace($path) -or
                -not (Test-Path -LiteralPath $path)) {
                throw 'Deep Artist F Material audit requires explicit existing UPK root, DDS root, and source-pack manifest.'
            }
        }
        & python 'Tools/LevelPlacementExtractor/extract_artist_31470_material_render_state.py' `
            '--material-closure' $materialClosure `
            '--exact-dds-receipt' $ddsReceipt `
            '--source-package-root' $SourcePackageRoot `
            '--output' $renderReceipt `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F raw Material render-state check failed: $LASTEXITCODE"
        }
    }

    $generatorArguments = @(
        'Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py',
        '--active-inventory', $activeInventory,
        '--material-closure', $materialClosure,
        '--exact-dds-receipt', $ddsReceipt,
        '--render-state-receipt', $renderReceipt,
        '--output', $contractPath,
        '--check'
    )
    if ($DeepMaterialAudit) {
        $generatorArguments += @(
            '--source-package-root', $SourcePackageRoot,
            '--exact-dds-root', $ExactDdsRoot,
            '--source-pack-manifest', $SourcePackManifest
        )
    }
    & python @generatorArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F typed Material evidence check failed: $LASTEXITCODE"
    }

    Push-Location 'Tools/LevelPlacementExtractor'
    try {
        $savedSourcePackageRoot = $env:ARTIST_F_MATERIAL_SOURCE_PACKAGE_ROOT
        $savedExactDdsRoot = $env:ARTIST_F_MATERIAL_EXACT_DDS_ROOT
        $savedSourcePackManifest = $env:ARTIST_F_MATERIAL_SOURCE_PACK_MANIFEST
        if ($DeepMaterialAudit) {
            $env:ARTIST_F_MATERIAL_SOURCE_PACKAGE_ROOT = $SourcePackageRoot
            $env:ARTIST_F_MATERIAL_EXACT_DDS_ROOT = $ExactDdsRoot
            $env:ARTIST_F_MATERIAL_SOURCE_PACK_MANIFEST = $SourcePackManifest
        }
        $savedErrorActionPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $unitTestOutput = (& python -m unittest -q `
                'test_build_artist_31470_material_evidence_contract' `
                2>&1 | Out-String).Trim()
            $unitTestExitCode = $LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $savedErrorActionPreference
            $env:ARTIST_F_MATERIAL_SOURCE_PACKAGE_ROOT = $savedSourcePackageRoot
            $env:ARTIST_F_MATERIAL_EXACT_DDS_ROOT = $savedExactDdsRoot
            $env:ARTIST_F_MATERIAL_SOURCE_PACK_MANIFEST = $savedSourcePackManifest
        }
        if ($unitTestExitCode -ne 0) {
            throw "Artist F typed Material unit tests failed: $unitTestExitCode $unitTestOutput"
        }
    }
    finally {
        Pop-Location
    }

    $contract = Get-Content -LiteralPath $contractPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $rawReceipt = Get-Content -LiteralPath $renderReceipt -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if ($rawReceipt.schema -cne 'lostark.artist-31470-material-render-state-evidence-receipt' -or
        [int]$rawReceipt.formatVersion -ne 2 -or
        @($rawReceipt.graphExpressions).Count -ne 925 -or
        @($rawReceipt.textureSamplerExports).Count -ne 4 -or
        [int]$rawReceipt.summary.rawPackageCount -ne 19 -or
        [int]$rawReceipt.summary.uniqueBaseMaterialGraphCount -ne 23) {
        throw 'Artist F raw Material evidence denominator changed.'
    }
    $summary = $contract.summary
    $admission = $contract.admission
    if ($contract.schema -cne 'lostark.artist-31470-typed-material-evidence-contract' -or
        [int]$contract.formatVersion -ne 1 -or
        @($contract.materialRecipes).Count -ne 27 -or
        @($contract.occurrences).Count -ne 34 -or
        @($contract.graphFamilies).Count -ne 23 -or
        @($contract.exactSamplerBindings).Count -ne 4 -or
        [int]$summary.scalarOverrideCount -ne 342 -or
        [int]$summary.vectorOverrideCount -ne 19 -or
        [int]$summary.directTextureOverrideCount -ne 71 -or
        [int]$summary.directTextureExactSamplerCount -ne 3 -or
        [int]$summary.directTextureUnprovenSamplerCount -ne 68 -or
        [int]$summary.parentDefaultExactSamplerCount -ne 1 -or
        [int]$summary.usedMaterialRecipeCount -ne 27 -or
        [int]$summary.unusedMaterialRecipeCount -ne 0 -or
        [int]$summary.unexpectedOccurrenceMaterialCount -ne 0 -or
        [string]$summary.occurrenceMaterialJoinSha256 -cne '1c56ff7bf67dc94a61129372a0e71f57a74171ee47ddf57702cd88b95606b296' -or
        [int]$summary.cookedStrippedNullExpressionCount -ne 1803 -or
        [int]$summary.unresolvedGraphEdgeCount -ne 502 -or
        [int]$summary.sourceExactGraphFamilyCount -ne 0 -or
        [int]$summary.sourceExactInputRecipeCount -ne 27 -or
        [int]$summary.sourceExactStaticPermutationRecipeCount -ne 0 -or
        [int]$summary.sourceExactFullRenderStateRecipeCount -ne 0 -or
        [int]$summary.sourceExactPartialCullRecipeCount -ne 18 -or
        [int]$summary.implementedArithmeticEvaluatorCount -ne 0 -or
        [int]$admission.executableRecipeCount -ne 0 -or
        [int]$admission.productRecipeCount -ne 0 -or
        [int]$admission.executableOccurrenceCount -ne 0 -or
        [int]$admission.productOccurrenceCount -ne 0) {
        throw 'Artist F typed Material evidence denominator or admission boundary changed.'
    }

    $originCounts = @{}
    foreach ($binding in @($contract.exactSamplerBindings)) {
        $origin = [string]$binding.bindingOrigin
        $originCounts[$origin] = 1 + [int]$originCounts[$origin]
    }
    if ([int]$originCounts['INSTANCE_OVERRIDE'] -ne 3 -or
        [int]$originCounts['PARENT_DEFAULT'] -ne 1) {
        throw 'Artist F exact sampler origin denominator changed.'
    }
    foreach ($family in @($contract.graphFamilies)) {
        if ([string]$family.graphProvenance -cne 'RECONSTRUCTED_GRAPH' -or
            [bool]$family.sourceExactGraph -or
            [string]$family.evaluator.fidelity -cne 'RECONSTRUCTED_ARITHMETIC_FAMILY' -or
            [bool]$family.evaluator.sourceExact -or
            [bool]$family.evaluator.implemented) {
            throw "Artist F graph fidelity laundering detected: $($family.familyId)"
        }
    }
    foreach ($recipe in @($contract.materialRecipes)) {
        if ([bool]$recipe.staticPermutation.sourceExact -or
            @($recipe.staticPermutation.selectedParameters).Count -ne 0 -or
            [bool]$recipe.renderState.fullCullModeExact -or
            [bool]$recipe.renderState.fullRenderStateExact -or
            [bool]$recipe.admission.executable -or
            [bool]$recipe.admission.product) {
            throw "Artist F Material recipe opened an unproven boundary: $($recipe.recipeId)"
        }
    }

    $mode = if ($DeepMaterialAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 Material evidence mode=$mode recipes=27 occurrences=34 inputs=342/19/71 samplers=3+1 graphs=23 stripped=1803/502 runtime=false product=false"
}
finally {
    Pop-Location
}
