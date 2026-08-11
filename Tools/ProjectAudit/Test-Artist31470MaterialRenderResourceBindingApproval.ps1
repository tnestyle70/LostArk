[CmdletBinding()]
param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$generator = 'Tools/LevelPlacementExtractor/build_artist_31470_material_render_resource_binding_approval.py'
$test = 'Tools/LevelPlacementExtractor/test_build_artist_31470_material_render_resource_binding_approval.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.material-render-resource-binding-approved-v1.receipt.json'
$expectedReceiptByteCount = 376183
$expectedReceiptRawSha256 = '68ae71bd70260270404d4a7b6c296e41f74d0031d27899b56a4376c1b11f4931'
$expectedReceiptSha256 = 'd643c9bf1bc2f10a887c805534b28e4322646cea426656de61b894e5b6284644'
$expectedDecisionProjectionSha256 = '4731ed9c2882c948373ec54f56087803145447851f3fc793fb8e9fa9d96cc957'

function Invoke-AutocrlfIsolatedCheckoutRegression {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Root
    )

    $tempBase = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    $isolatedRoot = [IO.Path]::GetFullPath((Join-Path $tempBase (
        'artist31470-material-render-autocrlf-' + [Guid]::NewGuid().ToString('N')
    )))
    if (-not $isolatedRoot.StartsWith($tempBase, [StringComparison]::OrdinalIgnoreCase) -or
        [IO.Path]::GetFileName($isolatedRoot) -notlike
            'artist31470-material-render-autocrlf-*') {
        throw "Unsafe Artist F autocrlf regression path: $isolatedRoot"
    }

    $temporaryIndex = Join-Path $isolatedRoot 'temporary.index'
    $overlayPaths = @(
        '.gitattributes',
        $receiptPath,
        'Tools/LevelPlacementExtractor/artist_31470_material_render_resource_binding_approval.py',
        $generator,
        $test
    )
    $checkoutPaths = @(
        '.gitattributes',
        'Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli',
        'Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli',
        'Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-runtime-oracle.receipt.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-reconstructed-approved-v1.receipt.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json',
        $receiptPath,
        'Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py',
        'Tools/LevelPlacementExtractor/effect_source_contract_io.py',
        'Tools/LevelPlacementExtractor/artist_31470_material_render_resource_binding_approval.py',
        $generator,
        $test
    )
    $hadIndexEnvironment = Test-Path Env:GIT_INDEX_FILE
    $previousIndexEnvironment = $env:GIT_INDEX_FILE
    $pushed = $false

    New-Item -ItemType Directory -Path $isolatedRoot | Out-Null
    try {
        try {
            $env:GIT_INDEX_FILE = $temporaryIndex
            & git read-tree HEAD
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F autocrlf regression temporary read-tree failed.'
            }
            & git -c core.autocrlf=false -c core.safecrlf=false add -- $overlayPaths
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F autocrlf regression temporary-index overlay failed.'
            }
            $checkoutPrefix = $isolatedRoot.Replace('\', '/') + '/'
            & git -c core.autocrlf=true checkout-index --force `
                "--prefix=$checkoutPrefix" -- $checkoutPaths
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F autocrlf regression checkout-index failed.'
            }
        }
        finally {
            if ($hadIndexEnvironment) {
                $env:GIT_INDEX_FILE = $previousIndexEnvironment
            }
            else {
                Remove-Item Env:GIT_INDEX_FILE -ErrorAction SilentlyContinue
            }
        }

        $commonGitDirectory = (& git rev-parse --git-common-dir).Trim()
        if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($commonGitDirectory)) {
            throw 'Artist F autocrlf regression cannot resolve the common Git directory.'
        }
        if (-not [IO.Path]::IsPathRooted($commonGitDirectory)) {
            $commonGitDirectory = Join-Path $Root $commonGitDirectory
        }
        $commonGitDirectory = [IO.Path]::GetFullPath($commonGitDirectory).Replace('\', '/')
        [IO.File]::WriteAllText(
            (Join-Path $isolatedRoot '.git'),
            "gitdir: $commonGitDirectory`n",
            [Text.UTF8Encoding]::new($false)
        )
        if (Test-Path -LiteralPath $temporaryIndex -PathType Leaf) {
            Remove-Item -LiteralPath $temporaryIndex -Force
        }

        Push-Location $isolatedRoot
        $pushed = $true
        $attributeOutput = (& git -c core.autocrlf=true check-attr eol -- $receiptPath |
            Out-String).Trim()
        if ($LASTEXITCODE -ne 0 -or $attributeOutput -notmatch ': eol: lf$') {
            throw "Artist F approval receipt exact-path LF attribute is missing: $attributeOutput"
        }

        $isolatedReceiptPath = Join-Path $isolatedRoot $receiptPath
        $receiptBytes = [IO.File]::ReadAllBytes($isolatedReceiptPath)
        $hasBom = $receiptBytes.Length -ge 3 -and
            $receiptBytes[0] -eq 0xef -and
            $receiptBytes[1] -eq 0xbb -and
            $receiptBytes[2] -eq 0xbf
        $crCount = 0
        foreach ($byte in $receiptBytes) {
            if ($byte -eq 13) {
                ++$crCount
            }
        }
        $rawSha256 = (Get-FileHash -LiteralPath $isolatedReceiptPath -Algorithm SHA256).Hash.ToLowerInvariant()
        $isolatedReceipt = Get-Content -LiteralPath $isolatedReceiptPath -Raw -Encoding UTF8 |
            ConvertFrom-Json
        if ($receiptBytes.Length -ne $expectedReceiptByteCount -or
            $rawSha256 -cne $expectedReceiptRawSha256 -or
            $hasBom -or
            $crCount -ne 0 -or
            $isolatedReceipt.receiptSha256 -cne $expectedReceiptSha256) {
            throw (
                'Artist F approval receipt autocrlf checkout identity changed: ' +
                "bytes=$($receiptBytes.Length) raw=$rawSha256 CR=$crCount BOM=$hasBom " +
                "self=$($isolatedReceipt.receiptSha256)"
            )
        }

        $decisionCommand = (
            "import json,sys;sys.path.insert(0,r'Tools/LevelPlacementExtractor');" +
            'import artist_31470_material_render_resource_binding_approval as a;' +
            "print(a.decision_projection_sha256(json.load(open(r'$receiptPath',encoding='utf-8'))))"
        )
        $decisionProjection = (& python -B -c $decisionCommand 2>&1 | Out-String).Trim()
        if ($LASTEXITCODE -ne 0 -or
            $decisionProjection -cne $expectedDecisionProjectionSha256) {
            throw "Artist F approval decision projection changed after autocrlf checkout: $decisionProjection"
        }

        $savedErrorActionPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $isolatedUnitOutput = (& python -B $test 2>&1 | Out-String).Trim()
            $isolatedUnitExitCode = $LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $savedErrorActionPreference
        }
        if ($isolatedUnitExitCode -ne 0 -or
            $isolatedUnitOutput -notmatch 'Ran 20 tests' -or
            $isolatedUnitOutput -notmatch '(?m)^OK$') {
            throw (
                "Artist F approval autocrlf unit regression failed: " +
                "$isolatedUnitExitCode $isolatedUnitOutput"
            )
        }

        $isolatedCheckOutput = (& python -B $generator '--output' $receiptPath '--check' `
            2>&1 | Out-String).Trim()
        $isolatedCheckExitCode = $LASTEXITCODE
        if ($isolatedCheckExitCode -ne 0 -or
            $isolatedCheckOutput -notmatch
                'PASS Artist 31470 Material render-resource approval recipes=27 renderer=57 descriptors=46') {
            throw (
                "Artist F approval autocrlf generator check failed: " +
                "$isolatedCheckExitCode $isolatedCheckOutput"
            )
        }

        return [PSCustomObject]@{
            ByteCount = $receiptBytes.Length
            RawSha256 = $rawSha256
            ReceiptSha256 = [string]$isolatedReceipt.receiptSha256
            DecisionProjectionSha256 = $decisionProjection
            CarriageReturnCount = $crCount
            HasBom = $hasBom
            UnitTestCount = 20
            GeneratorCheckPassed = $true
        }
    }
    finally {
        if ($pushed) {
            Pop-Location
        }
        if (Test-Path -LiteralPath $isolatedRoot) {
            $resolvedCleanup = [IO.Path]::GetFullPath($isolatedRoot)
            if (-not $resolvedCleanup.StartsWith($tempBase, [StringComparison]::OrdinalIgnoreCase) -or
                [IO.Path]::GetFileName($resolvedCleanup) -notlike
                    'artist31470-material-render-autocrlf-*') {
                throw "Refusing unsafe Artist F autocrlf cleanup: $resolvedCleanup"
            }
            Remove-Item -LiteralPath $resolvedCleanup -Recurse -Force
        }
    }
}

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
        throw "Artist F Material render-resource approval unit tests failed: $unitExitCode $unitOutput"
    }

    & python -B $generator '--output' $receiptPath '--check'
    if ($LASTEXITCODE -ne 0) {
        throw "Artist F Material render-resource approval deterministic check failed: $LASTEXITCODE"
    }

    $autocrlfRegression = Invoke-AutocrlfIsolatedCheckoutRegression `
        -Root $RepositoryRoot

    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $recipes = @($receipt.recipeTextureBindings)
    $renderers = @($receipt.rendererSlotBindings)
    $states = @($receipt.renderStateDescriptors)
    $ambiguous = @($renderers | Where-Object { [int]$_.candidateCount -eq 2 })
    $blend = @($states | Where-Object { $_.descriptorKind -ceq 'D3D11_BLEND_DESC' })
    $raster = @($states | Where-Object { $_.descriptorKind -ceq 'D3D11_RASTERIZER_DESC' })
    $depth = @($states | Where-Object { $_.descriptorKind -ceq 'D3D11_DEPTH_STENCIL_DESC' })

    if ($receipt.schema -cne 'lostark.artist-31470-material-render-resource-binding-approved-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.approvalId -cne 'ARTIST_31470_MATERIAL_RENDER_RESOURCE_BINDING_APPROVED_V1' -or
        $receipt.characterClass -cne 'ARTIST' -or
        [int]$receipt.skillId -ne 31470 -or
        $receipt.inputSlot -cne 'F' -or
        $recipes.Count -ne 27 -or
        $renderers.Count -ne 57 -or
        $ambiguous.Count -ne 3 -or
        $states.Count -ne 46 -or
        $blend.Count -ne 27 -or
        $raster.Count -ne 18 -or
        $depth.Count -ne 1 -or
        [bool]$receipt.approvalContract.runtimeNameOrRoleHeuristicsAllowed -or
        [bool]$receipt.approvalContract.actionTimeIoAllowed -or
        $receipt.approvalContract.transactionPolicy -cne 'PARSE_VALIDATE_STAGE_COMMIT_OR_ROLLBACK' -or
        [bool]$receipt.admission.sourceExact -or
        -not [bool]$receipt.admission.requiresAutomatedWARPProbe -or
        -not [bool]$receipt.admission.requiresManualEyeValidation -or
        [bool]$receipt.admission.runtimeExecutionAdmission -or
        [bool]$receipt.admission.product -or
        $receipt.blockerProjection.bindingFailureBehavior -cne 'ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET' -or
        [bool]$receipt.blockerProjection.partialCommitAllowed) {
        throw 'Artist F Material render-resource approval denominator or fail-closed boundary changed.'
    }

    $expectedAmbiguous = @{
        'fx_pc_sdm_07.par_v_smd_onestroke_swing_01::action-31470/stage-000/notify-018::FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_15::renderer-texture:base' = 'material-input-e51237e20a813da8'
        'fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_16::renderer-texture:base' = 'material-input-787a89b9e8277bec'
        'fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_10::renderer-texture:base' = 'material-input-7aed8cfe5ba9669b'
    }
    foreach ($row in $ambiguous) {
        if (-not $expectedAmbiguous.ContainsKey([string]$row.textureResourceId) -or
            $expectedAmbiguous[[string]$row.textureResourceId] -cne
                [string]$row.selectedMaterialInputFieldId -or
            $row.decisionBasis -cne 'EXPLICIT_INDEPENDENT_AMBIGUITY_APPROVAL') {
            throw "Artist F ambiguous renderer decision changed: $($row.textureResourceId)"
        }
    }

    foreach ($row in @($receipt.neutralProviders) + $recipes + $renderers + $states) {
        if ([bool]$row.sourceExact -or
            -not [bool]$row.requiresAutomatedWARPProbe -or
            -not [bool]$row.requiresManualEyeValidation -or
            [bool]$row.runtimeExecutionAdmission -or
            [bool]$row.product -or
            [string]::IsNullOrWhiteSpace([string]$row.rowSha256)) {
            throw 'Artist F Material render-resource row admission changed.'
        }
    }
    foreach ($row in $recipes) {
        if ($null -eq $row.texture0Provider -or
            $null -eq $row.texture1Provider -or
            $null -eq $row.neutralFallbackDecision -or
            $row.neutralFallbackDecision.materialBindingFailurePolicy -cne
                'FAIL_CLOSED_TRANSACTION_ROLLBACK') {
            throw "Artist F recipe texture/fallback decision is incomplete: $($row.recipeId)"
        }
    }

    Write-Output (
        'PASS: Artist F 31470 Material render-resource approval ' +
        'recipes=27 renderer=57 ambiguous=3 descriptors=27+18+1/46 ' +
        "autocrlf=$($autocrlfRegression.UnitTestCount)/20+check " +
        "bytes=$($autocrlfRegression.ByteCount) CR=$($autocrlfRegression.CarriageReturnCount) " +
        "BOM=$($autocrlfRegression.HasBom.ToString().ToLowerInvariant()) " +
        'warpRequired=true eyeRequired=true sourceExact=false runtime=false product=false'
    )
}
finally {
    Pop-Location
}
