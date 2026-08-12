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
$expectedReceiptByteCount = 774127
$expectedReceiptRawSha256 = '1567c622876f74018ac9a21a4ba9e04dd8a3fd08f0bfe934698a65b8185d2660'
$expectedReceiptSha256 = '6f4ed12c7c5b6499ece7cf520436f747e4877a4a89a1584ba57de7324adf8ac4'
$expectedDecisionProjectionSha256 = 'fcef9bb95c5412f1d25f206e207b6eccd8198a26a8994a6ee5ac179498b001de'

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
            $historicalRuntimeBlob = (& git rev-parse (
                $historicalRevision +
                ':Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json'
            )).Trim()
            if ($LASTEXITCODE -ne 0 -or
                $historicalRuntimeBlob -cne
                    'ca360e952dd110f0246a5e0f1374baf77b7ebc0c') {
                throw (
                    'Artist F frozen historical runtime blob changed: ' +
                    $historicalRuntimeBlob
                )
            }
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

        # The current exact-path attribute deliberately checks out the frozen
        # 7d3 catalog as LF.  Its immutable sidecar predates that transport
        # rule and records the single trailing newline as CRLF, so prove both
        # byte identities before recreating the historical generator input.
        $isolatedRuntimePath = Join-Path $isolatedRoot `
            'Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json'
        $lfRuntimeBytes = [IO.File]::ReadAllBytes($isolatedRuntimePath)
        $lfRuntimeSha = (Get-FileHash -LiteralPath $isolatedRuntimePath `
            -Algorithm SHA256).Hash.ToLowerInvariant()
        $lfRuntimeCrIndex = [Array]::IndexOf($lfRuntimeBytes, [byte]13)
        if ($lfRuntimeBytes.Length -ne 26255930 -or
            $lfRuntimeSha -cne
                'b5086d14940ecb35d3c577024902a080e57f571112f0e79a4f8c8f0aa875509f' -or
            $lfRuntimeCrIndex -ne -1 -or
            $lfRuntimeBytes[$lfRuntimeBytes.Length - 1] -ne 10) {
            throw (
                'Artist F frozen runtime LF checkout identity changed: ' +
                "bytes=$($lfRuntimeBytes.Length) raw=$lfRuntimeSha " +
                "firstCR=$lfRuntimeCrIndex"
            )
        }
        $historicalRuntimeBytes = [byte[]]::new($lfRuntimeBytes.Length + 1)
        [Array]::Copy(
            $lfRuntimeBytes, 0, $historicalRuntimeBytes, 0,
            $lfRuntimeBytes.Length - 1)
        $historicalRuntimeBytes[$historicalRuntimeBytes.Length - 2] = 13
        $historicalRuntimeBytes[$historicalRuntimeBytes.Length - 1] = 10
        [IO.File]::WriteAllBytes($isolatedRuntimePath, $historicalRuntimeBytes)
        $historicalRuntimeSha = (Get-FileHash -LiteralPath $isolatedRuntimePath `
            -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($historicalRuntimeBytes.Length -ne 26255931 -or
            $historicalRuntimeSha -cne
                'bf0807ec1b4d975c988ed7e8bb204c6b1713218968be76ea6accb6340e714d29') {
            throw (
                'Artist F historical runtime CRLF identity changed: ' +
                "bytes=$($historicalRuntimeBytes.Length) " +
                "raw=$historicalRuntimeSha"
            )
        }

        $null = Invoke-PythonUnit -TestPath $test
        $checkOutput = (& python -B $generator '--output' $receiptPath '--check' `
            2>&1 | Out-String).Trim()
        if ($LASTEXITCODE -ne 0 -or
            $checkOutput -notmatch
                'PASS Artist 31470 reconstructed render-resource authority resources=52 bindings=77 recipes=27 renderer=57') {
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
        $resources.Count -ne 52 -or
        $bindings.Count -ne 77 -or
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
        $publisherAuthority.authorityScope -cne
            'BASE_RUNTIME_ENTRY_PROJECTION_BEFORE_RENDER_RESOURCE_SIDECAR' -or
        [bool]$publisherAuthority.runtimeCatalogBytesRead -or
        [bool]$publisherAuthority.completedRuntimeEntryRead -or
        [bool]$publisherAuthority.renderResourceSidecarRead -or
        -not [bool]$publisherAuthority.selfReferenceExcluded -or
        [int]$publisherAuthority.projectionKeyCount -ne 17 -or
        $publisherAuthority.projectionCanonicalSha256 -cne
            'e7a630b9d94dfb177b3f678561865bd9e7bad2dd3f1eb082656b79e5c3af3190' -or
        $publisherAuthority.baseProjection.programSha256 -cne
            '8e618a53242fb2fee9b13528d9696182038ded977454d98ff49ff500570ebeb8' -or
        $publisherAuthority.baseProjection.candidateRawSha256 -cne
            'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6' -or
        [int]$publisherAuthority.baseProjection.candidateByteCount -ne 15117436 -or
        [bool]$publisherAuthority.baseProjection.sourceExact -or
        [bool]$publisherAuthority.baseProjection.runtimeExecutionAdmission -or
        [bool]$publisherAuthority.baseProjection.productAdmission) {
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
    if ($resourceFormats['BC1_SRGB'] -ne 39 -or
        $resourceFormats['BC3_SRGB'] -ne 8 -or
        $resourceFormats['BC5_LINEAR'] -ne 4 -or
        $resourceFormats['BC3_LINEAR'] -ne 1 -or
        $srvFormats[72] -ne 63 -or
        $srvFormats[78] -ne 9 -or
        $srvFormats[83] -ne 4 -or
        $srvFormats[77] -ne 1 -or
        $colors['SRGB'] -ne 72 -or
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
        'resources=52 bindings=77 formats=39+8+4+1 srv=63+9+4+1 colors=72+5 ' +
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
