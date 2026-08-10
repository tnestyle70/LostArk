[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$DeepGeometryAudit,
    [string]$SourceMeshRoot = '',
    [string]$RuntimeMeshRoot = '',
    [string]$SourceExportReceipt = '',
    [string]$LegacyCookReceipt = '',
    [string]$SourcePackageRoot = '',
    [string]$LegacyConverter = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

Push-Location $RepositoryRoot
try {
    Push-Location 'Tools\ModelAssetConverter'
    try {
        & python -B -m unittest -q 'test_cook_wmodel_geometry_contract'
        if ($LASTEXITCODE -ne 0) {
            throw "WModel geometry cooker tests failed: $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }

    Push-Location 'Tools\LevelPlacementExtractor'
    try {
        & python -B -m unittest -q `
            'test_build_artist_31470_wmodel_geometry_parity'
        if ($LASTEXITCODE -ne 0) {
            throw "WModel geometry EOL/hash tests failed: $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }

    & 'Tools\WModelGeometryContractHarness\Run-WModelGeometryContractHarness.ps1' `
        -Configuration $Configuration

    if ($DeepGeometryAudit) {
        $required = @(
            $SourceMeshRoot,
            $RuntimeMeshRoot,
            $SourceExportReceipt,
            $LegacyCookReceipt,
            $SourcePackageRoot,
            $LegacyConverter
        )
        if (@($required | Where-Object {
            [string]::IsNullOrWhiteSpace($_) -or
            -not (Test-Path -LiteralPath $_)
        }).Count -ne 0) {
            throw 'Deep geometry audit requires six explicit existing source paths.'
        }

        & python -B `
            'Tools\ModelAssetConverter\verify_artist_31470_wmodel_geometry_contract.py' `
            '--source-root' $SourceMeshRoot `
            '--runtime-mesh-root' $RuntimeMeshRoot `
            '--source-manifest' `
                'Data\Effects\Imported\Artist\Artist.resource-source-manifest.json' `
            '--source-export-receipt' $SourceExportReceipt `
            '--legacy-cook-receipt' $LegacyCookReceipt `
            '--source-package-root' $SourcePackageRoot `
            '--legacy-converter' $LegacyConverter
        if ($LASTEXITCODE -ne 0) {
            throw "Artist 31470 seven-carrier geometry oracle failed: $LASTEXITCODE"
        }
    }

    Write-Host (
        "Artist 31470 WModel geometry contract audit PASS " +
        "configuration=$Configuration deep=$($DeepGeometryAudit.IsPresent) " +
        'preScaleConsumed=false product=false')
}
finally {
    Pop-Location
}
