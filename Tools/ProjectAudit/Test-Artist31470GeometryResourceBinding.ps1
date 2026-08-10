[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$DeepResourceAudit,
    [string]$SourceMeshRoot = '',
    [string]$LegacyMeshRoot = '',
    [string]$SourceExportReceipt = '',
    [string]$LegacyCookReceipt = '',
    [string]$SourcePackageRoot = '',
    [string]$LegacyConverter = '',
    [string]$PhysicalMeshRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$binding = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Geometry\skill.31470.geometry-binding.json'
$receipt = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Geometry\skill.31470.geometry-resource-binding.receipt.json'
$expectedSemantics = Join-Path $RepositoryRoot `
    'Tools\WModelGeometryContractHarness\Fixtures\artist_31470_v11_expected.json'
$sourceManifest = Join-Path $RepositoryRoot `
    'Data\Effects\Imported\Artist\Artist.resource-source-manifest.json'
$builder = Join-Path $RepositoryRoot `
    'Tools\ModelAssetConverter\build_artist_31470_geometry_resource_binding.py'

Push-Location $RepositoryRoot
try {
    Push-Location 'Tools\ModelAssetConverter'
    try {
        $previousErrorActionPreference = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        try {
            $unitOutput = @(& python -B -m unittest -q `
                'test_build_artist_31470_geometry_resource_binding' 2>&1)
            $unitExitCode = $LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $previousErrorActionPreference
        }
        if ($unitExitCode -ne 0) {
            $unitOutput | ForEach-Object { Write-Output $_.ToString() }
            throw "Geometry resource binding unit tests failed: $unitExitCode"
        }
    }
    finally {
        Pop-Location
    }

    if ($DeepResourceAudit) {
        $decoderHarness = Join-Path $RepositoryRoot `
            "Tools\WModelGeometryContractHarness\Bin\$Configuration\WModelGeometryContractHarness.exe"
        $required = @(
            $SourceMeshRoot,
            $LegacyMeshRoot,
            $SourceExportReceipt,
            $LegacyCookReceipt,
            $SourcePackageRoot,
            $LegacyConverter,
            $PhysicalMeshRoot,
            $decoderHarness,
            $binding,
            $receipt,
            $expectedSemantics,
            $sourceManifest
        )
        if (@($required | Where-Object {
            [string]::IsNullOrWhiteSpace($_) -or
            -not (Test-Path -LiteralPath $_)
        }).Count -ne 0) {
            throw 'Deep geometry resource audit requires seven explicit existing input roots/files and the built decoder harness.'
        }

        $temporaryBase = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
        $temporaryRoot = [IO.Path]::GetFullPath((Join-Path $temporaryBase `
            "artist-31470-geometry-resource-$PID-$Configuration"))
        if (-not $temporaryRoot.StartsWith(
                $temporaryBase, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Temporary geometry resource root escaped TEMP: $temporaryRoot"
        }
        if (Test-Path -LiteralPath $temporaryRoot) {
            Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
        }
        $stage = Join-Path $temporaryRoot 'stage'
        $negativePhysical = Join-Path $temporaryRoot 'negative-physical'
        [IO.Directory]::CreateDirectory($stage) | Out-Null
        [IO.Directory]::CreateDirectory($negativePhysical) | Out-Null

        $runtimeDirectories = @(
            (Join-Path $RepositoryRoot "Engine\Bin\$Configuration"),
            (Join-Path $RepositoryRoot 'Engine\ThirdPartyLib\FMOD\Bin'),
            (Join-Path $RepositoryRoot "Engine\ThirdPartyLib\Assimp\Bin\$Configuration"),
            (Join-Path $RepositoryRoot "Engine\ThirdPartyLib\PhysX\Bin\$Configuration")
        )
        $previousPath = $env:PATH
        try {
            $env:PATH = ($runtimeDirectories -join ';') + ';' + $previousPath
            & python -B $builder build `
                --source-root $SourceMeshRoot `
                --legacy-mesh-root $LegacyMeshRoot `
                --source-manifest $sourceManifest `
                --source-export-receipt $SourceExportReceipt `
                --legacy-cook-receipt $LegacyCookReceipt `
                --source-package-root $SourcePackageRoot `
                --legacy-converter $LegacyConverter `
                --expected-semantics $expectedSemantics `
                --staging-root $stage `
                --binding-output $binding `
                --receipt-output $receipt `
                --decoder-harness $decoderHarness `
                --check
            if ($LASTEXITCODE -ne 0) {
                throw "Geometry resource binding deep rebuild failed: $LASTEXITCODE"
            }

            & python -B $builder verify-deployed `
                --binding $binding `
                --receipt $receipt `
                --expected-semantics $expectedSemantics `
                --physical-mesh-root $PhysicalMeshRoot `
                --decoder-harness $decoderHarness
            if ($LASTEXITCODE -ne 0) {
                throw "Physical geometry resource verification failed: $LASTEXITCODE"
            }

            $targetNames = @(
                'fm_v_wp_wsdm_base_01.wmodel',
                'fm_m_trail_002.wmodel',
                'fm_h_swing_03.wmodel',
                'fm_h_swing_05.wmodel',
                'fm_h_swing_01.wmodel',
                'fm_o_swing_02.wmodel',
                'fm_a_stone_001.wmodel'
            )
            foreach ($name in $targetNames) {
                [IO.File]::Copy(
                    (Join-Path $PhysicalMeshRoot $name),
                    (Join-Path $negativePhysical $name))
            }
            $corrupt = Join-Path $negativePhysical $targetNames[0]
            $corruptBytes = [IO.File]::ReadAllBytes($corrupt)
            $corruptBytes[0] = $corruptBytes[0] -bxor 0x01
            [IO.File]::WriteAllBytes($corrupt, $corruptBytes)
            $previousErrorActionPreference = $ErrorActionPreference
            $ErrorActionPreference = 'Continue'
            try {
                $corruptOutput = (& python -B $builder verify-deployed `
                    --binding $binding `
                    --receipt $receipt `
                    --expected-semantics $expectedSemantics `
                    --physical-mesh-root $negativePhysical `
                    --decoder-harness $decoderHarness 2>&1 | Out-String)
            }
            finally {
                $ErrorActionPreference = $previousErrorActionPreference
            }
            if ($LASTEXITCODE -eq 0) {
                throw 'Corrupt physical WModel passed GeometryBinding verification.'
            }
            [IO.File]::Copy(
                (Join-Path $PhysicalMeshRoot $targetNames[0]),
                $corrupt,
                $true)
            [IO.File]::Delete((Join-Path $negativePhysical $targetNames[1]))
            $ErrorActionPreference = 'Continue'
            try {
                $missingOutput = (& python -B $builder verify-deployed `
                    --binding $binding `
                    --receipt $receipt `
                    --expected-semantics $expectedSemantics `
                    --physical-mesh-root $negativePhysical `
                    --decoder-harness $decoderHarness 2>&1 | Out-String)
            }
            finally {
                $ErrorActionPreference = $previousErrorActionPreference
            }
            if ($LASTEXITCODE -eq 0) {
                throw 'Missing physical WModel passed GeometryBinding verification.'
            }
        }
        finally {
            $env:PATH = $previousPath
            if (Test-Path -LiteralPath $temporaryRoot) {
                Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
            }
        }
    }

    Write-Output (
        'PASS: Artist F 31470 GeometryBinding ' +
        "mode=$(if ($DeepResourceAudit) { 'deep' } else { 'shallow' }) " +
        'carriers=7 expectedTuple=7 preScale=0.01 product=false')
}
finally {
    Pop-Location
}
