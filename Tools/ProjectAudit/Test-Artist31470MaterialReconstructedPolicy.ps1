[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepMaterialPolicyAudit,
    [string]$D3DCompiler = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\d3dcompiler_47.dll'
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/build_artist_31470_material_reconstructed_policy.py'
$test = 'Tools/LevelPlacementExtractor/test_build_artist_31470_material_reconstructed_policy.py'
$verifier = 'Tools/LevelPlacementExtractor/verify_artist_31470_material_reconstructed_policy_hlsl.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.material-reconstructed-approved-v1.receipt.json'

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
        throw "Artist F reconstructed Material policy unit tests failed: $unitExitCode $unitOutput"
    }

    & python -B $generator '--output' $receiptPath '--shallow-check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed Material policy shallow check failed: $LASTEXITCODE"
    }

    & python -B $verifier `
        '--receipt' $receiptPath `
        '--d3dcompiler' $D3DCompiler
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F reconstructed Material policy WARP replay failed: $LASTEXITCODE"
    }

    if ($DeepMaterialPolicyAudit) {
        if (-not (Test-Path -LiteralPath $D3DCompiler -PathType Leaf)) {
            throw 'Deep Artist F reconstructed Material policy audit requires the pinned D3D compiler.'
        }
        & python -B $generator `
            '--output' $receiptPath `
            '--d3dcompiler' $D3DCompiler `
            '--run-hlsl' `
            '--check'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F reconstructed Material policy deep check failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json
    $render = @($receipt.renderStatePolicies)
    $static = @($receipt.staticPermutationPolicies)
    $sampler = @($receipt.samplerPolicies)
    $rows = @($render) + @($static) + @($sampler)
    $summary = $receipt.summary
    if ($receipt.schema -cne 'lostark.artist-31470-material-reconstructed-policy-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        $receipt.policyContract.policyId -cne 'RECONSTRUCTED_APPROVED_V1' -or
        [bool]$receipt.policyContract.sourceExact -or
        [bool]$receipt.policyContract.runtimeConsumerImplemented -or
        [bool]$receipt.policyContract.rendererConsumerImplemented -or
        [bool]$receipt.policyContract.productAdmission -or
        $render.Count -ne 89 -or
        $static.Count -ne 94 -or
        $sampler.Count -ne 72 -or
        $rows.Count -ne 255 -or
        [int]$summary.staticExactOverrideValueRetainedCount -ne 23 -or
        [int]$summary.sourceExactPolicyRowCount -ne 0 -or
        [int]$summary.d3dDescriptorOracleRowCount -ne 107 -or
        [int]$summary.srvColorSpaceOracleRowCount -ne 72 -or
        [int]$summary.runtimeConsumerReadyRowCount -ne 0 -or
        [int]$summary.rendererConsumerReadyRowCount -ne 0 -or
        [int]$summary.productReadyRowCount -ne 0 -or
        -not [bool]$receipt.admission.policySelection.ready -or
        [int]$receipt.admission.policySelection.rowCount -ne 255 -or
        [bool]$receipt.admission.runtimeConsumer.ready -or
        [bool]$receipt.admission.rendererConsumer.ready -or
        [bool]$receipt.admission.product -or
        -not [bool]$receipt.hlslVerification.verified -or
        [int]$receipt.hlslVerification.sampleCount -ne 255 -or
        [double]$receipt.hlslVerification.numericTolerance -ne 0.0 -or
        -not [bool]$receipt.warpDescriptorVerification.verified -or
        [int]$receipt.warpDescriptorVerification.descriptorRowCount -ne 107 -or
        [int]$receipt.warpDescriptorVerification.srvColorSpaceRowCount -ne 72 -or
        [double]$receipt.warpDescriptorVerification.numericTolerance -ne 0.0) {
        throw 'Artist F reconstructed Material policy denominator or fail-closed boundary changed.'
    }

    for ($index = 0; $index -lt $rows.Count; ++$index) {
        $row = $rows[$index]
        if ([int]$row.policyOrder -ne $index -or
            [string]::IsNullOrWhiteSpace([string]$row.policyRowId) -or
            [string]::IsNullOrWhiteSpace([string]$row.materialRecipeId) -or
            [string]::IsNullOrWhiteSpace([string]$row.fieldId) -or
            @($row.materialOccurrenceIds).Count -eq 0 -or
            [bool]$row.sourceExact -or
            @($row.evidenceBlockers.sourceValueAcquisition).Count -eq 0 -or
            @($row.evidenceBlockers.runtimeOracle).Count -eq 0 -or
            -not [bool]$row.policySelectionAdmission -or
            [bool]$row.runtimeConsumerAdmission -or
            [bool]$row.rendererConsumerAdmission -or
            [bool]$row.productAdmission -or
            [double]$row.numericOracle.numericTolerance -ne 0.0) {
            throw "Artist F reconstructed Material policy row changed: $($row.policyRowId)"
        }
    }

    if (($sampler | ConvertTo-Json -Depth 30 -Compress) -cmatch 'SOURCE_EXACT_SAMPLER') {
        throw 'Artist F reconstructed Material policy reintroduced SOURCE_EXACT_SAMPLER.'
    }

    foreach ($result in @($receipt.hlslVerification.rowResults)) {
        $expected = $result.expectedFloat4 | ConvertTo-Json -Depth 4 -Compress
        $actual = $result.actualFloat4 | ConvertTo-Json -Depth 4 -Compress
        if ($expected -cne $actual -or [string]$result.decision -cne 'PASS') {
            throw "Artist F reconstructed HLSL row changed: $($result.policyRowId)"
        }
    }
    foreach ($result in @($receipt.warpDescriptorVerification.rowResults)) {
        $expected = $result.expectedDescriptor | ConvertTo-Json -Depth 20 -Compress
        $actual = $result.actualDescriptor | ConvertTo-Json -Depth 20 -Compress
        if ($expected -cne $actual -or [string]$result.decision -cne 'PASS') {
            throw "Artist F reconstructed WARP descriptor row changed: $($result.policyRowId)"
        }
    }
    foreach ($result in @($receipt.warpDescriptorVerification.srvRowResults)) {
        $expected = $result.expectedSrv | ConvertTo-Json -Depth 20 -Compress
        $actual = $result.actualSrv | ConvertTo-Json -Depth 20 -Compress
        if ($expected -cne $actual -or [string]$result.decision -cne 'PASS') {
            throw "Artist F reconstructed WARP SRV row changed: $($result.policyRowId)"
        }
    }

    $mode = if ($DeepMaterialPolicyAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 Material reconstructed policy mode=$mode rows=89+94+72/255 staticExact=23 warp=255+107 srv=72 sourceExact=0 runtime=0 product=false"
}
finally {
    Pop-Location
}
