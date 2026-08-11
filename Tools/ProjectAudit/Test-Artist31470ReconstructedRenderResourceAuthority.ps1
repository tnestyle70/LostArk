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

$generator = 'Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_render_resource_authority.py'
$module = 'Tools/LevelPlacementExtractor/artist_31470_reconstructed_render_resource_authority.py'
$test = 'Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_render_resource_authority.py'
$receiptPath = 'Data/Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority.receipt.json'
$expectedReceiptByteCount = 746788
$expectedReceiptRawSha256 = 'bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff'
$expectedReceiptSha256 = 'bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a'
$expectedDecisionProjectionSha256 = '4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412'

function Invoke-PythonUnit {
    param(
        [Parameter(Mandatory = $true)]
        [string]$TestPath
    )

    $savedErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = (& python -B $TestPath 2>&1 | Out-String).Trim()
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $savedErrorActionPreference
    }
    if ($exitCode -ne 0 -or
        $output -notmatch 'Ran 31 tests' -or
        $output -notmatch '(?m)^OK$') {
        throw "Artist F render-resource authority unit regression failed: $exitCode $output"
    }
    return $output
}

function Invoke-AutocrlfIsolatedCheckoutRegression {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Root
    )

    $tempBase = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    $isolatedRoot = [IO.Path]::GetFullPath((Join-Path $tempBase (
        'a31470-rra-' + [Guid]::NewGuid().ToString('N')
    )))
    if (-not $isolatedRoot.StartsWith($tempBase, [StringComparison]::OrdinalIgnoreCase) -or
        [IO.Path]::GetFileName($isolatedRoot) -notlike
            'a31470-rra-*') {
        throw "Unsafe Artist F authority autocrlf path: $isolatedRoot"
    }

    $temporaryIndex = Join-Path $isolatedRoot 'temporary.index'
    $historicalRevision = '7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2'
    $overlayPaths = @(
        '.gitattributes',
        $receiptPath,
        $module,
        $generator,
        $test
    )
    $checkoutPaths = @(
        '.gitattributes',
        'Client/Public/Effect_RuntimeAuthority.h',
        'Client/Private/Effect_RuntimeAuthority.cpp',
        'Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli',
        'Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli',
        'Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-runtime-oracle.receipt.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-reconstructed-approved-v1.receipt.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-texture-runtime-binding.receipt.json',
        'Data/Effects/Imported/Artist/Materials/skill.31470.material-render-resource-binding-approved-v1.receipt.json',
        'Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json',
        $receiptPath,
        'Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py',
        'Tools/EffectPipeline/build_effect_derived_artifact.py',
        'Tools/EffectPipeline/Publish-Effects.ps1',
        'Tools/LevelPlacementExtractor/effect_source_contract_io.py',
        'Tools/LevelPlacementExtractor/artist_31470_material_render_resource_binding_approval.py',
        'Tools/LevelPlacementExtractor/build_artist_31470_material_render_resource_binding_approval.py',
        $module,
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
            & git read-tree $historicalRevision
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F authority frozen historical read-tree failed.'
            }
            & git -c core.autocrlf=false -c core.safecrlf=false add -- $overlayPaths
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F authority temporary-index overlay failed.'
            }
            $checkoutPrefix = $isolatedRoot.Replace('\', '/') + '/'
            & git -c core.autocrlf=true checkout-index --force `
                "--prefix=$checkoutPrefix" -- $checkoutPaths
            if ($LASTEXITCODE -ne 0) {
                throw 'Artist F authority autocrlf checkout-index failed.'
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
            throw 'Artist F authority cannot resolve the common Git directory.'
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
            throw "Artist F authority exact-path LF attribute is missing: $attributeOutput"
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
            $isolatedReceipt.receiptSha256 -cne $expectedReceiptSha256 -or
            $isolatedReceipt.decisionProjectionSha256 -cne
                $expectedDecisionProjectionSha256) {
            throw (
                'Artist F authority autocrlf identity changed: ' +
                "bytes=$($receiptBytes.Length) raw=$rawSha256 CR=$crCount BOM=$hasBom " +
                "self=$($isolatedReceipt.receiptSha256) " +
                "decision=$($isolatedReceipt.decisionProjectionSha256)"
            )
        }

        $null = Invoke-PythonUnit -TestPath $test
        $checkOutput = (& python -B $generator '--output' $receiptPath '--check' `
            2>&1 | Out-String).Trim()
        if ($LASTEXITCODE -ne 0 -or
            $checkOutput -notmatch
                'PASS Artist 31470 reconstructed render-resource authority resources=48 bindings=72 recipes=27 renderer=57') {
            throw "Artist F authority autocrlf generator check failed: $checkOutput"
        }

        return [PSCustomObject]@{
            ByteCount = $receiptBytes.Length
            RawSha256 = $rawSha256
            ReceiptSha256 = [string]$isolatedReceipt.receiptSha256
            DecisionProjectionSha256 = [string]$isolatedReceipt.decisionProjectionSha256
            CarriageReturnCount = $crCount
            HasBom = $hasBom
            UnitTestCount = 31
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
                    'a31470-rra-*') {
                throw "Refusing unsafe Artist F authority cleanup: $resolvedCleanup"
            }
            Remove-Item -LiteralPath $resolvedCleanup -Recurse -Force
        }
    }
}

Push-Location $RepositoryRoot
try {
    # The immutable sidecar records the historical bf0807 catalog and its
    # original current-tool tuple.  Re-run that exact contract only inside the
    # frozen 7d3e957 historical checkout, then independently validate the current
    # outer-13 bridge.  The final catalog identity never feeds back into the
    # sidecar receipt projection.
    $autocrlfRegression = Invoke-AutocrlfIsolatedCheckoutRegression -Root $RepositoryRoot
    $bridgeBuilder = Join-Path $RepositoryRoot `
        'Tools\EffectPipeline\build_effect_derived_artifact.py'
    $currentRuntimeCatalog = Join-Path $RepositoryRoot `
        'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
    $bridgeOutput = (& python -B $bridgeBuilder validate-runtime-catalog `
        --catalog $currentRuntimeCatalog 2>&1 | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "Current Artist F render-resource bridge validation failed: $bridgeOutput"
    }

    $receiptBytes = [IO.File]::ReadAllBytes((Join-Path $RepositoryRoot $receiptPath))
    $receiptRawSha = (Get-FileHash -LiteralPath $receiptPath -Algorithm SHA256).Hash.ToLowerInvariant()
    $receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json
    $resources = @($receipt.textureResources)
    $bindings = @($receipt.textureBindings)
    $recipes = @($receipt.recipeTextureBindings)
    $renderers = @($receipt.rendererSlotBindings)
    $states = @($receipt.renderStateDescriptors)
    $ambiguous = @($renderers | Where-Object { [int]$_.candidateCount -eq 2 })
    $publisherAuthority = $receipt.sourceEvidence.publisherRuntimeCatalogAuthority

    if ($receiptBytes.Length -ne $expectedReceiptByteCount -or
        $receiptRawSha -cne $expectedReceiptRawSha256 -or
        $receipt.receiptSha256 -cne $expectedReceiptSha256 -or
        $receipt.decisionProjectionSha256 -cne $expectedDecisionProjectionSha256 -or
        $receipt.schema -cne 'lostark.artist-31470-reconstructed-render-resource-authority-receipt' -or
        [int]$receipt.formatVersion -ne 1 -or
        $receipt.authorityId -cne 'ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1' -or
        $resources.Count -ne 48 -or
        $bindings.Count -ne 72 -or
        $recipes.Count -ne 27 -or
        $renderers.Count -ne 57 -or
        $ambiguous.Count -ne 3 -or
        $states.Count -ne 46 -or
        [bool]$receipt.authorityContract.runtimeNameOrRoleHeuristicsAllowed -or
        [bool]$receipt.authorityContract.absoluteResourcePathsAllowedInReceipt -or
        [bool]$receipt.authorityContract.actionTimeIoAllowed -or
        [bool]$receipt.admission.sourceExact -or
        [bool]$receipt.admission.runtimeExecutionAdmission -or
        [bool]$receipt.admission.product -or
        $receipt.blockerProjection.bindingFailureBehavior -cne
            'ROLLBACK_PRESERVE_PREVIOUS_RESOURCE_SET' -or
        [bool]$receipt.blockerProjection.partialCommitAllowed -or
        $publisherAuthority.publisherIntegrationCommitId -cne
            '932d648f95bc7f2d9c7209fad55aee8a857c94d7' -or
        $publisherAuthority.publisherOriginalCommitId -cne
            '74c692755791e592d6f808f3c50b3321c60af181' -or
        [int]$publisherAuthority.currentCheckoutByteCount -ne 26255931 -or
        $publisherAuthority.currentCheckoutRawSha256 -cne
            'bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29' -or
        [int]$publisherAuthority.outerKeyCount -ne 10 -or
        [int]$publisherAuthority.linkKeyCount -ne 16 -or
        [int]$publisherAuthority.receiptKeyCount -ne 25 -or
        [int]$publisherAuthority.toolDependencyCount -ne 3 -or
        $publisherAuthority.linkCanonicalSha256 -cne
            '74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2' -or
        $publisherAuthority.receiptSelfSha256 -cne
            '5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3' -or
        $publisherAuthority.outerPublishReceiptSha256 -cne
            '92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94' -or
        [bool]$publisherAuthority.sourceExact -or
        [bool]$publisherAuthority.runtimeExecutionAdmission -or
        [bool]$publisherAuthority.productAdmission) {
        throw 'Artist F render-resource authority identity or fail-closed boundary changed.'
    }

    $resourceFormats = @{}
    foreach ($row in @($receipt.summary.resourceFormatCounts)) {
        $resourceFormats[[string]$row.classification] = [int]$row.count
    }
    $srvFormats = @{}
    foreach ($row in @($receipt.summary.bindingSrvDxgiFormatCounts)) {
        $srvFormats[[int]$row.dxgiFormat] = [int]$row.count
    }
    $colors = @{}
    foreach ($row in @($receipt.summary.bindingColorSpaceCounts)) {
        $colors[[string]$row.colorSpacePolicy] = [int]$row.count
    }
    if ($resourceFormats['BC1_SRGB'] -ne 35 -or
        $resourceFormats['BC3_SRGB'] -ne 8 -or
        $resourceFormats['BC5_LINEAR'] -ne 4 -or
        $resourceFormats['BC3_LINEAR'] -ne 1 -or
        $srvFormats[72] -ne 58 -or
        $srvFormats[78] -ne 9 -or
        $srvFormats[83] -ne 4 -or
        $srvFormats[77] -ne 1 -or
        $colors['SRGB'] -ne 67 -or
        $colors['LINEAR'] -ne 5) {
        throw 'Artist F actual DDS/SRV/color derived counts changed.'
    }

    $receiptText = [Text.Encoding]::UTF8.GetString($receiptBytes)
    if ($receiptText -match '(?i)[A-Z]:[\\/]' -or
        $receiptText -match '(?i)Users[\\/]') {
        throw 'Artist F authority receipt leaked an absolute filesystem path.'
    }

    $allowedPaths = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::OrdinalIgnoreCase
    )
    @(
        '.gitattributes',
        'Tools/ProjectAudit/Invoke-ProjectAudit.ps1',
        'Tools/ProjectAudit/Test-Artist31470ReconstructedRenderResourceAuthority.ps1',
        'Tools/ProjectAudit/Test-EffectDerivedArtifactPublisher.ps1',
        'Tools/EffectPipeline/build_effect_derived_artifact.py',
        'Tools/EffectPipeline/test_build_effect_derived_artifact.py',
        'Tools/EffectPipeline/Publish-Effects.ps1',
        'Data/Effects/EffectCatalog.json',
        'Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json',
        $module,
        $generator,
        $test,
        $receiptPath,
        '.md/GB/08-11/2026-08-11_ARTIST_31470_F_RENDER_RESOURCE_PUBLISHER_BRIDGE_PLAN.md',
        '.md/GB/08-11/2026-08-11_ARTIST_31470_F_RENDER_RESOURCE_PUBLISHER_BRIDGE_RESULT.md',
        '.md/GB/08-11/2026-08-11_ARTIST_31470_F_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_PLAN.md',
        '.md/GB/08-11/2026-08-11_ARTIST_31470_F_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_RESULT.md'
    ) | ForEach-Object { $null = $allowedPaths.Add($_) }
    foreach ($line in @(& git status --short --untracked-files=all)) {
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }
        $path = $line.Substring(3).Replace('\', '/')
        if (-not $allowedPaths.Contains($path)) {
            throw "Artist F authority lane touched an out-of-scope path: $path"
        }
    }
    $staged = @(& git diff --cached --name-only)
    if ($staged.Count -ne 0) {
        throw 'Artist F authority lane must remain unstaged for independent review.'
    }
    $savedErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $diffCheckOutput = (& git diff --check 2>&1 | Out-String).Trim()
        $diffCheckExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $savedErrorActionPreference
    }
    if ($diffCheckExitCode -ne 0) {
        throw "Artist F authority git diff --check failed: $diffCheckOutput"
    }

    Write-Output (
        'PASS: Artist F 31470 reconstructed render-resource authority ' +
        'resources=48 bindings=72 formats=35+8+4+1 srv=58+9+4+1 colors=67+5 ' +
        'recipes=27 renderer=57 ambiguous=3 descriptors=27+18+1/46 ' +
        'publisher=10/16/25/tool3 bridge=13/21/26/tool3 ' +
        "autocrlf=$($autocrlfRegression.UnitTestCount)/31+check " +
        "bytes=$($autocrlfRegression.ByteCount) " +
        "CR=$($autocrlfRegression.CarriageReturnCount) " +
        "BOM=$($autocrlfRegression.HasBom.ToString().ToLowerInvariant()) " +
        'sourceExact=false runtime=false product=false'
    )
}
finally {
    Pop-Location
}
