[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepExactDdsDeploymentAudit,
    [string]$SourceDdsRoot = '',
    [string]$RuntimeResources = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}
if ([string]::IsNullOrWhiteSpace($SourceDdsRoot)) {
    $SourceDdsRoot = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot '.codex_tmp\artist-f-dds-recovery\source-export'))
}
if ([string]::IsNullOrWhiteSpace($RuntimeResources)) {
    $RuntimeResources = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot 'Client\Bin\Resources'))
}

$deployer = 'Tools/LevelPlacementExtractor/deploy_artist_31470_exact_dds_runtime.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.exact-dds-runtime-deployment.receipt.json'

Push-Location $RepositoryRoot
try {
    $arguments = @(
        $deployer,
        '--output', $receiptPath,
        '--check'
    )
    if ($DeepExactDdsDeploymentAudit) {
        foreach ($path in @($SourceDdsRoot, $RuntimeResources)) {
            if ([string]::IsNullOrWhiteSpace($path) -or -not (Test-Path -LiteralPath $path)) {
                throw "Deep Artist F exact DDS deployment audit input is missing: $path"
            }
        }
        $arguments += @(
            '--source-dds-root', $SourceDdsRoot,
            '--runtime-resource-root', $RuntimeResources
        )
    }

    & python -B @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F exact DDS deployment validation failed: $LASTEXITCODE"
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $assets = @($receipt.assets)
    $expectedAssets = @(
        'Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds',
        'Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds',
        'Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds',
        'Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds'
    )
    $actualAssets = @($assets.runtimeAssetId)
    if ($receipt.schema -cne 'lostark.artist-31470-exact-dds-runtime-deployment-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        $assets.Count -ne 4 -or
        ($actualAssets -join "`n") -cne ($expectedAssets -join "`n") -or
        [int]$receipt.summary.deployedAssetCount -ne 4 -or
        [int]$receipt.summary.postVerifiedAssetCount -ne 4 -or
        [int]$receipt.summary.runtimeAssetDeploymentAdmittedCount -ne 4 -or
        [int]$receipt.summary.recoveryBackupPayloadFileCount -ne 0 -or
        [int]$receipt.summary.recoveryAbsentTargetMarkerCount -ne 4 -or
        [int]$receipt.summary.sourceExactMaterialClaimCount -ne 0 -or
        [int]$receipt.summary.rendererConsumerReadyCount -ne 0 -or
        [int]$receipt.summary.productReadyCount -ne 0 -or
        -not [bool]$receipt.admission.transactionCommitted -or
        -not [bool]$receipt.admission.allFourRuntimeAssetsPostVerified -or
        [bool]$receipt.admission.sourceExactMaterialClaim -or
        [bool]$receipt.admission.rendererTextureSrvConsumerComplete -or
        [bool]$receipt.admission.r4Complete -or
        [bool]$receipt.admission.productReady -or
        $receipt.recoveryBackup.anchor -cne 'RUNTIME_RESOURCE_ROOT_PARENT' -or
        $receipt.recoveryBackup.manifestCanonicalSelfSha256 -cne '9058170aab950ce5ceff1cfffa96331d1cdea97ef24b0902cb9d5be0815c4c9a' -or
        $receipt.recoveryBackup.manifestRawSha256 -cne '382c845589627926a2d04fa34a263c9314f9f96f70ff305b84086835c1de2f33' -or
        -not [bool]$receipt.recoveryBackup.preservedAfterCommit) {
        throw 'Artist F exact DDS deployment denominator or fail-closed admission changed.'
    }

    foreach ($asset in $assets) {
        if ($asset.policy -cne 'RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1' -or
            $asset.deploymentStatus -cne 'COMMITTED_POST_VERIFIED' -or
            [bool]$asset.sourceExactMaterialClaim -or
            -not [bool]$asset.runtimeAssetDeploymentAdmission -or
            [bool]$asset.rendererConsumerAdmission -or
            [bool]$asset.productAdmission -or
            @($asset.blockers).Count -ne 1 -or
            @($asset.blockers)[0] -cne 'R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE') {
            throw "Artist F exact DDS deployment row boundary changed: $($asset.deploymentRowId)"
        }
    }

    $mode = if ($DeepExactDdsDeploymentAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 exact DDS runtime deployment mode=$mode assets=4/4 sourceExactMaterial=false r4=false product=false"
}
finally {
    Pop-Location
}
