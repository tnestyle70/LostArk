[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepMaterialAudit,
    [string]$SourcePackageRoot = '',
    [string]$ExactDdsRoot = '',
    [string]$SourcePackManifest = '',
    [string]$InstalledReleaseRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK',
    [string]$UModelExe = 'C:\Users\user\Desktop\Resource_LostArk\06_Tools\UEViewerLostArk_runtime\umodel_lostark_v7.exe'
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
$oracleReceipt = "$materialRoot/skill.31470.material-oracle-acquisition.receipt.json"

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
            '--source-pack-manifest' $SourcePackManifest `
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

    $oracleArguments = @(
        'Tools/LevelPlacementExtractor/build_artist_31470_material_oracle_acquisition.py',
        '--typed-contract', $contractPath,
        '--raw-render-receipt', $renderReceipt,
        '--material-closure', $materialClosure,
        '--output', $oracleReceipt,
        '--check'
    )
    if ($DeepMaterialAudit) {
        if ([string]::IsNullOrWhiteSpace($InstalledReleaseRoot) -or
            -not (Test-Path -LiteralPath $InstalledReleaseRoot -PathType Container) -or
            [string]::IsNullOrWhiteSpace($UModelExe) -or
            -not (Test-Path -LiteralPath $UModelExe -PathType Leaf)) {
            throw 'Deep Artist F Material audit requires the installed LOSTARK release root and UModel v7.'
        }
        $oracleArguments += @(
            '--installed-release-root', $InstalledReleaseRoot,
            '--umodel-exe', $UModelExe
        )
    }
    & python @oracleArguments
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material oracle acquisition check failed: $LASTEXITCODE"
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
                'test_build_artist_31470_material_oracle_acquisition' `
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
    $oracle = Get-Content -LiteralPath $oracleReceipt -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if ($rawReceipt.schema -cne 'lostark.artist-31470-material-render-state-evidence-receipt' -or
        [int]$rawReceipt.formatVersion -ne 3 -or
        @($rawReceipt.graphExpressions).Count -ne 925 -or
        @($rawReceipt.textureSamplerExports).Count -ne 4 -or
        [int]$rawReceipt.summary.rawPackageCount -ne 19 -or
        [int]$rawReceipt.summary.uniqueBaseMaterialGraphCount -ne 23) {
        throw 'Artist F raw Material evidence denominator changed.'
    }
    $summary = $contract.summary
    $admission = $contract.admission
    if ($contract.schema -cne 'lostark.artist-31470-typed-material-evidence-contract' -or
        [int]$contract.formatVersion -ne 4 -or
        @($contract.materialRecipes).Count -ne 27 -or
        @($contract.occurrences).Count -ne 34 -or
        @($contract.graphFamilies).Count -ne 23 -or
        @($contract.exactSamplerBindings).Count -ne 0 -or
        @($contract.rejectedSamplerBindings).Count -ne 4 -or
        [int]$summary.scalarOverrideCount -ne 342 -or
        [int]$summary.vectorOverrideCount -ne 19 -or
        [int]$summary.directTextureOverrideCount -ne 71 -or
        [int]$summary.directTextureExactSamplerCount -ne 0 -or
        [int]$summary.directTextureUnprovenSamplerCount -ne 71 -or
        [int]$summary.parentDefaultExactSamplerCount -ne 0 -or
        [int]$summary.exactSamplerBindingCount -ne 0 -or
        [int]$summary.previouslyAdmittedExactSamplerBindingCount -ne 4 -or
        [int]$summary.rejectedSamplerBindingCount -ne 4 -or
        [int]$summary.strictSamplerExecutionRowCount -ne 72 -or
        [string]$summary.rejectedSamplerBindingSetSha256 -cne 'dfc923cab4dd2155385c2c066f261cea689a863c8e8179b48d2677556a849d4c' -or
        [int]$summary.usedMaterialRecipeCount -ne 27 -or
        [int]$summary.unusedMaterialRecipeCount -ne 0 -or
        [int]$summary.unexpectedOccurrenceMaterialCount -ne 0 -or
        [string]$summary.occurrenceMaterialJoinSha256 -cne '1c56ff7bf67dc94a61129372a0e71f57a74171ee47ddf57702cd88b95606b296' -or
        [string]$summary.occurrenceIdentitySha256 -cne '8110cc2e44b885b82093b029d763a44301ba3787dee56ab0d90bb37e135d8c11' -or
        [string]$summary.recipeIdentitySha256 -cne 'f4cb7f8d5fae3699d55eb56a1a0442c6d55a0e5f062766a23fb654d665d24e22' -or
        [string]$summary.recipeFamilyJoinSha256 -cne '9877829577c550acb7452e8ff279c7819f17151513cee53994bee116620838f2' -or
        [string]$summary.renderFieldEvidenceSha256 -cne '6d5d70af3215c36509e86340af00aafceb94182f5c93b903c4ca951283a8d5b9' -or
        [string]$summary.exactInputLineageSha256 -cne '52c73db22b2e8bbe35b655719f082c73bd0df6dce5bce6a5243b1dc5e17be2ec' -or
        [string]$summary.recipeCompositionSha256 -cne '39b73297b4e96e26dd2d5b79aacf85f32bb083407823b571c80c4d8dce05c9f6' -or
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
    foreach ($binding in @($contract.rejectedSamplerBindings)) {
        $origin = [string]$binding.bindingOrigin
        $originCounts[$origin] = 1 + [int]$originCounts[$origin]
        $rawFields = $binding.sourceTextureEvidence.rawSamplerFields
        if ([bool]$binding.sourceSpecificAdmission -or
            [bool]$binding.legacyDdsProjection.sourceSpecificAdmission -or
            [string]$binding.fidelity -cne 'UNRESOLVED_SAMPLER_PROVENANCE' -or
            [string]$binding.decision -cne 'BLOCKED_INCOMPLETE_SOURCE_SPECIFIC_SAMPLER_DESCRIPTOR' -or
            $null -eq $rawFields.addressx -or
            $null -eq $rawFields.addressy -or
            $null -eq $rawFields.srgb -or
            $null -eq $rawFields.filter -or
            $null -eq $rawFields.lodgroup) {
            throw "Artist F rejected sampler provenance changed: $($binding.rejectionId)"
        }
    }
    if ([int]$originCounts['INSTANCE_OVERRIDE'] -ne 3 -or
        [int]$originCounts['PARENT_DEFAULT'] -ne 1) {
        throw 'Artist F rejected legacy-exact sampler origin denominator changed.'
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
    if ($oracle.schema -cne 'lostark.artist-31470-material-oracle-acquisition-receipt' -or
        [int]$oracle.formatVersion -ne 1 -or
        @($oracle.families).Count -ne 23 -or
        [int]$oracle.summary.materialRecipeCount -ne 27 -or
        [int]$oracle.summary.survivingExpressionCount -ne 925 -or
        [int]$oracle.summary.cookedNullExpressionCount -ne 1803 -or
        [int]$oracle.summary.resolvedInputEdgeCount -ne 125 -or
        [int]$oracle.summary.unresolvedInputEdgeCount -ne 502 -or
        [int]$oracle.summary.oracleAvailableFamilyCount -ne 0 -or
        [int]$oracle.summary.implementedEvaluatorCount -ne 0 -or
        [int]$oracle.summary.productFamilyCount -ne 0 -or
        [int]$oracle.summary.installedMaterialLeafFamilyCount -ne 23 -or
        [int]$oracle.summary.shaderCacheExportCount -ne 1596 -or
        [int]$oracle.summary.selectedShaderCacheCandidateCount -ne 11 -or
        [bool]$oracle.admission.executionReady -or
        [bool]$oracle.admission.product) {
        throw 'Artist F Material oracle acquisition boundary changed.'
    }
    foreach ($family in @($oracle.families)) {
        if ([string]$family.oracleStatus -cne 'SHADERCACHE_PRESENT_DECODER_PENDING' -or
            [string]$family.graphProvenance -cne 'RECONSTRUCTED_GRAPH' -or
            [bool]$family.sourceExactGraph -or
            [bool]$family.evaluatorImplemented -or
            [bool]$family.executable -or
            [bool]$family.product -or
            [bool]$family.minimumIndependentNumericOracle.imageValidationAllowed) {
            throw "Artist F Material oracle laundering detected: $($family.familyId)"
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
    Write-Output "PASS: Artist F 31470 Material evidence mode=$mode recipes=27 occurrences=34 inputs=342/19/71 samplers=0/72 rejectedLegacy=3+1 graphs=23 stripped=1803/502 shadercache=1596/11 oracle=0 runtime=false product=false"
}
finally {
    Pop-Location
}
