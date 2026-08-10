[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepMaterialRuntimeAudit,
    [string]$SourceArchiveRoot = '',
    [string]$D3DCompiler = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll'
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/build_artist_31470_material_runtime_oracle.py'
$test = 'Tools/LevelPlacementExtractor/test_build_artist_31470_material_runtime_oracle.py'
$hlslVerifier = 'Tools/LevelPlacementExtractor/verify_artist_31470_material_runtime_oracle_hlsl.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.material-runtime-oracle.receipt.json'

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
        throw "Artist F Material runtime oracle unit tests failed: $unitExitCode $unitOutput"
    }

    & python -B $generator '--output' $receiptPath '--shallow-check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material runtime oracle shallow check failed: $LASTEXITCODE"
    }

    & python -B $hlslVerifier `
        '--receipt' $receiptPath `
        '--d3dcompiler' $D3DCompiler
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material WARP replay failed: $LASTEXITCODE"
    }

    if ($DeepMaterialRuntimeAudit) {
        if ([string]::IsNullOrWhiteSpace($SourceArchiveRoot) -or
            -not (Test-Path -LiteralPath $SourceArchiveRoot -PathType Container)) {
            throw 'Deep Artist F Material runtime audit requires SourceArchiveRoot.'
        }
        if (-not (Test-Path -LiteralPath $D3DCompiler -PathType Leaf)) {
            throw 'Deep Artist F Material runtime audit requires the pinned D3D compiler.'
        }
        & python -B $generator `
            '--source-archive-root' $SourceArchiveRoot `
            '--d3dcompiler' $D3DCompiler `
            '--output' $receiptPath `
            '--run-hlsl' `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F Material runtime oracle deep check failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $summary = $receipt.summary
    $archive = $receipt.sourceRevisionShaderCacheAcquisition
    $hlsl = $receipt.hlslVerification
    $stateProvider = $receipt.warpStateProviderVerification
    $matrices = $receipt.materialFeasibilityMatrices
    $feasibility = $receipt.materialFeasibilityMatrices.summary
    if ($receipt.schema -cne 'lostark.artist-31470-material-runtime-oracle-receipt' -or
        [int]$receipt.formatVersion -ne 3 -or
        [int]$summary.materialFamilyCount -ne 23 -or
        [int]$summary.implementedEvaluatorCount -ne 23 -or
        [int]$summary.cpuVerifiedEvaluatorCount -ne 23 -or
        [int]$summary.hlslVerifiedEvaluatorCount -ne 23 -or
        [int]$summary.sourceExactEvaluatorCount -ne 0 -or
        [int]$summary.materialRecipeBindingCount -ne 27 -or
        [int]$summary.materialOccurrenceBindingCount -ne 34 -or
        [int]$summary.inputBindingCount -ne 729 -or
        [int]$summary.staticSwitchBindingCount -ne 94 -or
        [int]$summary.totalTypedFieldBindingCount -ne 823 -or
        [int]$summary.renderStateBindingCount -ne 162 -or
        [int]$summary.resolvedRenderStateBindingCount -ne 73 -or
        [int]$summary.unresolvedRenderStateBindingCount -ne 89 -or
        [int]$summary.familyNumericSampleCount -ne 92 -or
        [int]$summary.recipeNumericSampleCount -ne 108 -or
        [int]$summary.hlslSampleCount -ne 200 -or
        [int]$summary.materialFeasibilityRowCount -ne 255 -or
        [int]$summary.materialFeasibilityReadyCount -ne 0 -or
        [int]$summary.materialFeasibilityBlockedCount -ne 255 -or
        [int]$feasibility.renderStateRowCount -ne 89 -or
        [int]$feasibility.renderStateReadinessCount -ne 0 -or
        [int]$feasibility.staticPermutationRowCount -ne 94 -or
        [int]$feasibility.staticPermutationReadinessCount -ne 0 -or
        [int]$feasibility.staticExactGuidJoinCount -ne 66 -or
        [int]$feasibility.staticOverrideTrueSourceValueAcquiredCount -ne 23 -or
        [int]$feasibility.staticNonoverrideSemanticsUnverifiedCount -ne 43 -or
        [int]$feasibility.staticNoExactGuidEntryCount -ne 28 -or
        [int]$feasibility.strictSamplerRowCount -ne 72 -or
        [int]$feasibility.strictSamplerReadinessCount -ne 0 -or
        [int]$feasibility.strictSamplerRejectedLegacyExactRowCount -ne 4 -or
        [int]$feasibility.strictSamplerSourceTextureEvidenceRowCount -ne 72 -or
        [int]$feasibility.totalRowCount -ne 255 -or
        [int]$feasibility.blockedRowCount -ne 255 -or
        [int]$feasibility.ownerlessRowCount -ne 0 -or
        [int]$feasibility.unknownDecisionRowCount -ne 0 -or
        -not [bool]$feasibility.evidenceIntegrity -or
        [bool]$feasibility.executionReadiness -or
        [int]$archive.fileCount -ne 1813 -or
        [int]$archive.uniquePackageContentCount -ne 624 -or
        [int]$archive.shaderCacheNameCandidateCount -ne 0 -or
        [string]$archive.decision -cne 'SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE' -or
        -not [bool]$hlsl.verified -or
        [int]$hlsl.sampleCount -ne 200 -or
        -not [bool]$stateProvider.verified -or
        [string]$stateProvider.backend -cne 'D3D11_WARP_STATE_OBJECTS' -or
        [int]$stateProvider.pilotCount -ne 4 -or
        -not [bool]$receipt.admission.evidenceIntegrityAdmission -or
        [bool]$receipt.admission.executionReadinessAdmission -or
        [bool]$receipt.admission.materialRuntimeHandlerConsumptionAdmission -or
        [bool]$receipt.admission.rendererConsumptionAdmission -or
        [bool]$receipt.admission.productAdmission) {
        throw 'Artist F Material runtime denominator or fail-closed admission changed.'
    }

    foreach ($row in @($matrices.staticPermutationRows)) {
        $parent = $row.parentIdentity
        $selection = $row.instanceRecordIdentity.micNativeSelection
        if ([string]::IsNullOrWhiteSpace([string]$parent.expressionGuidHex) -or
            [string]::IsNullOrWhiteSpace([string]$parent.defaultValuePropertyRecordSha256) -or
            [string]::IsNullOrWhiteSpace([string]$parent.expressionGuidPropertyRecordSha256)) {
            throw "Artist F static parent GUID/value provenance missing: $($row.matrixRowId)"
        }
        if ([string]$selection.selectionOutcome -cne 'NO_EXACT_GUID_NATIVE_ENTRY') {
            if ([int]$selection.exactNameAndGuidMatchCount -ne 1 -or
                [string]$selection.entry.expressionGuidHex -cne [string]$parent.expressionGuidHex -or
                [string]::IsNullOrWhiteSpace([string]$selection.nativeTail.nativeTailSha256) -or
                $null -eq $selection.entry.value -or
                $null -eq $selection.entry.bOverride) {
                throw "Artist F static native GUID/value/bOverride provenance changed: $($row.matrixRowId)"
            }
        }
    }

    foreach ($row in @($matrices.strictSamplerRows)) {
        $fields = $row.sourceTextureEvidence.fields
        if ($null -eq $fields.addressx -or
            $null -eq $fields.addressy -or
            $null -eq $fields.srgb -or
            $null -eq $fields.filter -or
            $null -eq $fields.lodgroup -or
            [string]::IsNullOrWhiteSpace([string]$row.sourceValueAcquisitionEvidence.receiptSha256) -or
            [string]$row.executionDecision -cne 'BLOCKED') {
            throw "Artist F strict sampler source provenance changed: $($row.matrixRowId)"
        }
    }

    $mode = if ($DeepMaterialRuntimeAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 Material runtime mode=$mode family=23 recipe=27 occurrence=34 fields=729+94 render=73/162 feasibility=0/89+0/94+0/72 static=23/43/28 oracle=92+108/200 statePilots=4 sourceExact=0 shaderCache=0/624 readiness=false product=false"
}
finally {
    Pop-Location
}
