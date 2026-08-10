[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [switch]$DeepMaterialTextureBindingAudit,
    [string]$RuntimeCookReceipt = 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\ARTIST\all_bound_skills\runtime-cook-receipt.json',
    [string]$ResourceExportReceipt = 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\ARTIST\all_bound_skills\resource-export-receipt.json',
    [string]$SourcePackManifest = 'C:\Users\user\Desktop\Resource_LostArk\00_SourcePackages\Effect_DIMENSIONMASTER_20260803_v3\source_pack_manifest.json',
    [string]$RuntimeResources = 'C:\Users\user\Desktop\LostArk\Client\Bin\Resources'
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/build_artist_31470_material_texture_runtime_binding.py'
$test = 'Tools/LevelPlacementExtractor/test_build_artist_31470_material_texture_runtime_binding.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json'

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
        throw "Artist F Material texture runtime binding unit tests failed: $unitExitCode $unitOutput"
    }

    & python -B $generator '--output' $receiptPath '--validate-only'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material texture runtime binding shallow validation failed: $LASTEXITCODE"
    }

    if ($DeepMaterialTextureBindingAudit) {
        foreach ($path in @(
            $RuntimeCookReceipt,
            $ResourceExportReceipt,
            $SourcePackManifest,
            $RuntimeResources
        )) {
            if ([string]::IsNullOrWhiteSpace($path) -or -not (Test-Path -LiteralPath $path)) {
                throw "Deep Artist F Material texture binding audit input is missing: $path"
            }
        }
        & python -B $generator `
            '--output' $receiptPath `
            '--runtime-cook' $RuntimeCookReceipt `
            '--resource-export' $ResourceExportReceipt `
            '--source-pack' $SourcePackManifest `
            '--runtime-resources' $RuntimeResources `
            '--check' `
            '--deep-verify'
        if ($LASTEXITCODE -ne 0) {
            throw "Artist F Material texture runtime binding deep validation failed: $LASTEXITCODE"
        }
    }

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $bindings = @($receipt.materialTextureBindings)
    $resources = @($receipt.textureResources)
    $proposals = @($receipt.provisioningProposals)
    $resolvedBindings = @($bindings | Where-Object {
        $_.status -ceq 'RESOLVED_EXACT_RUNTIME_COOK_RECEIPT'
    })
    $unresolvedBindings = @($bindings | Where-Object {
        $_.status -ceq 'UNRESOLVED_RUNTIME_ASSET'
    })
    $resolvedResources = @($resources | Where-Object {
        $_.status -ceq 'RESOLVED_EXACT_RUNTIME_COOK_RECEIPT'
    })
    $unresolvedResources = @($resources | Where-Object {
        $_.status -ceq 'UNRESOLVED_RUNTIME_ASSET'
    })
    if ($receipt.schema -cne 'lostark.artist-31470-material-texture-runtime-binding-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        $bindings.Count -ne 72 -or
        $resources.Count -ne 48 -or
        $resolvedBindings.Count -ne 68 -or
        $unresolvedBindings.Count -ne 4 -or
        $resolvedResources.Count -ne 44 -or
        $unresolvedResources.Count -ne 4 -or
        $proposals.Count -ne 4 -or
        [int]$receipt.summary.materialOccurrenceLinkCount -ne 83 -or
        [int]$receipt.summary.sourcePackageBoundUniqueTextureCount -ne 45 -or
        [int]$receipt.summary.sourcePackageUnboundUniqueTextureCount -ne 3 -or
        [bool]$receipt.admission.completeRuntimeBinding.ready -or
        [bool]$receipt.admission.rendererConsumer.ready -or
        [bool]$receipt.admission.product) {
        throw 'Artist F Material texture runtime binding denominator or fail-closed boundary changed.'
    }

    $expectedProposalAssets = @(
        'Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds',
        'Effect/Artist/Textures/FX_TEX_00/fx_a_noise_011.dds',
        'Effect/Artist/Textures/FX_TEX_01/fx_c_atypical_016.dds',
        'Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds'
    )
    $actualProposalAssets = @($proposals.proposedRuntimeAssetId | Sort-Object -CaseSensitive)
    $expectedProposalAssets = @($expectedProposalAssets | Sort-Object -CaseSensitive)
    if (($actualProposalAssets -join "`n") -cne ($expectedProposalAssets -join "`n")) {
        throw 'Artist F Material texture provisioning proposal asset IDs changed.'
    }
    foreach ($proposal in $proposals) {
        if ($proposal.policy -cne 'RECONSTRUCTED_RUNTIME_DEPLOYMENT_FROM_EXACT_DDS_FIXTURE_V1' -or
            $proposal.deploymentStatus -cne 'PROPOSED_TRANSACTIONAL_DEPLOYMENT_NOT_VERIFIED' -or
            [bool]$proposal.sourceExact -or
            [bool]$proposal.runtimeAssetAdmission -or
            [bool]$proposal.productAdmission) {
            throw "Artist F Material texture proposal was promoted: $($proposal.proposalId)"
        }
    }

    $mode = if ($DeepMaterialTextureBindingAudit) { 'deep' } else { 'shallow' }
    Write-Output "PASS: Artist F 31470 Material texture runtime binding mode=$mode rows=68+4/72 unique=44+4/48 proposals=4 product=false"
}
finally {
    Pop-Location
}
