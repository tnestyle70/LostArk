[CmdletBinding()]
param(
    [string]$RepoRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
$publisher = Join-Path $RepoRoot `
    'Tools\RenderingPipeline\Publish-RenderingProfiles.ps1'
$authoredPath = Join-Path $RepoRoot `
    'Data\Rendering\Authored\RenderingProfiles.json'

function Invoke-PublisherCase(
    [string]$Mode,
    [string]$Source,
    [string]$Destination,
    [bool]$ShouldPass,
    [string]$Case) {
    $arguments = @(
        '-NoProfile',
        '-ExecutionPolicy', 'Bypass',
        '-File', $publisher,
        '-Mode', $Mode,
        '-SourcePath', $Source)
    if (-not [string]::IsNullOrWhiteSpace($Destination)) {
        $arguments += @('-DestinationPath', $Destination)
    }

    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $output = & powershell.exe @arguments 2>&1
        $passed = $LASTEXITCODE -eq 0
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($passed -ne $ShouldPass) {
        throw "$Case expected pass=$ShouldPass, actual pass=$passed. $($output -join ' ')"
    }
}

function Read-TestJson([string]$Path) {
    return [IO.File]::ReadAllText(
        $Path, [Text.UTF8Encoding]::new($false, $true)) |
        ConvertFrom-Json
}

function Copy-TestJson([object]$Value) {
    return $Value | ConvertTo-Json -Depth 16 | ConvertFrom-Json
}

function Write-TestJson([string]$Path, [object]$Value) {
    $document = ($Value | ConvertTo-Json -Depth 16) + [Environment]::NewLine
    [IO.File]::WriteAllText(
        $Path, $document, [Text.UTF8Encoding]::new($false))
}

if (-not (Test-Path -LiteralPath $publisher -PathType Leaf)) {
    throw "Rendering profile publisher is missing: $publisher"
}
if (-not (Test-Path -LiteralPath $authoredPath -PathType Leaf)) {
    throw "Authored rendering profile is missing: $authoredPath"
}

Invoke-PublisherCase 'Validate' $authoredPath '' $true `
    'canonical authored float32 round-trip'

$temporaryRoot = Join-Path ([IO.Path]::GetTempPath()) `
    ('lostark-rendering-profile-test-' + [guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($temporaryRoot) | Out-Null
try {
    $authored = Read-TestJson $authoredPath
    $publishedPath = Join-Path $temporaryRoot 'published-runtime.json'
    Invoke-PublisherCase 'Publish' $authoredPath $publishedPath $true `
        'canonical authored temporary publish'

    $published = Read-TestJson $publishedPath
    $authoredCanonical = $authored | ConvertTo-Json -Depth 16 -Compress
    $publishedCanonical = $published | ConvertTo-Json -Depth 16 -Compress
    if ($authoredCanonical -cne $publishedCanonical) {
        throw 'Temporary publish changed rendering profile semantics.'
    }
    if ([uint32]$authored.revision -ne [uint32]$published.revision -or
        [single]$authored.globalQuality.bloomScatter -ne
        [single]$published.globalQuality.bloomScatter) {
        throw 'Temporary publish changed revision or Bloom Scatter.'
    }

    $minimumBoundary = Copy-TestJson $authored
    $minimumBoundary.globalQuality.fxaaEdgeThreshold = 0.0311999992
    $minimumBoundary.globalQuality.fxaaEdgeThresholdMin = 0.0155999996
    $minimumBoundaryPath = Join-Path $temporaryRoot 'float32-minimum.json'
    Write-TestJson $minimumBoundaryPath $minimumBoundary
    Invoke-PublisherCase 'Validate' $minimumBoundaryPath '' $true `
        'C++ float32 minimum serialization'

    $maximumBoundary = Copy-TestJson $authored
    $maximumBoundary.globalQuality.fxaaEdgeThreshold = 0.333000004
    $maximumBoundary.globalQuality.fxaaEdgeThresholdMin = 0.0833000019
    $maximumBoundaryPath = Join-Path $temporaryRoot 'float32-maximum.json'
    Write-TestJson $maximumBoundaryPath $maximumBoundary
    Invoke-PublisherCase 'Validate' $maximumBoundaryPath '' $true `
        'C++ float32 maximum serialization'

    $belowMinimum = Copy-TestJson $authored
    $belowMinimum.globalQuality.fxaaEdgeThreshold =
        [double][single]0.0312 - 0.0000000001
    $belowMinimumPath = Join-Path $temporaryRoot 'below-float32-minimum.json'
    Write-TestJson $belowMinimumPath $belowMinimum
    Invoke-PublisherCase 'Validate' $belowMinimumPath '' $false `
        'value below the C++ float32 minimum'

    $aboveMaximum = Copy-TestJson $authored
    $aboveMaximum.globalQuality.fxaaEdgeThreshold =
        [double][single]0.333 + 0.00000001
    $aboveMaximumPath = Join-Path $temporaryRoot 'above-float32-maximum.json'
    Write-TestJson $aboveMaximumPath $aboveMaximum
    Invoke-PublisherCase 'Validate' $aboveMaximumPath '' $false `
        'value above the C++ float32 maximum'

    $preservedDestination = Join-Path $temporaryRoot 'preserved-runtime.json'
    Copy-Item -LiteralPath $publishedPath -Destination $preservedDestination
    $beforeRejectedPublish = (Get-FileHash -LiteralPath $preservedDestination `
        -Algorithm SHA256).Hash
    Invoke-PublisherCase 'Publish' $belowMinimumPath $preservedDestination $false `
        'rejected publish preserves the previous runtime'
    $afterRejectedPublish = (Get-FileHash -LiteralPath $preservedDestination `
        -Algorithm SHA256).Hash
    if ($beforeRejectedPublish -cne $afterRejectedPublish) {
        throw 'Rejected rendering publish changed the previous runtime.'
    }

    $numericString = Copy-TestJson $authored
    $numericString.globalQuality.bloomScatter = '1.0'
    $numericStringPath = Join-Path $temporaryRoot 'numeric-string.json'
    Write-TestJson $numericStringPath $numericString
    Invoke-PublisherCase 'Validate' $numericStringPath '' $false `
        'numeric string rejection'

    $duplicateKeyPath = Join-Path $temporaryRoot 'duplicate-key.json'
    $sourceText = [IO.File]::ReadAllText(
        $authoredPath, [Text.UTF8Encoding]::new($false, $true))
    $duplicateText = [regex]::Replace(
        $sourceText, '^\s*\{', "{`n  `"revision`": 1,", 1)
    if ($duplicateText -ceq $sourceText) {
        throw 'Duplicate-key fixture did not modify the source document.'
    }
    [IO.File]::WriteAllText(
        $duplicateKeyPath, $duplicateText, [Text.UTF8Encoding]::new($false))
    Invoke-PublisherCase 'Validate' $duplicateKeyPath '' $false `
        'duplicate JSON key rejection'
}
finally {
    if (Test-Path -LiteralPath $temporaryRoot) {
        Remove-Item -LiteralPath $temporaryRoot -Recurse -Force
    }
}

Write-Host 'Rendering profile pipeline test PASS'
Write-Host '  canonical authored: validate and temporary publish'
Write-Host '  float32 contract: serialized min/max accepted, outside rejected'
Write-Host '  transaction: rejected publish preserves previous runtime'
Write-Host '  strict JSON: numeric strings and duplicate keys rejected'
$global:LASTEXITCODE = 0
