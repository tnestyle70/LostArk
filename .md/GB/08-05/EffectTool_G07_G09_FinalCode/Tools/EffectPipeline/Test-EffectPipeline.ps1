$ErrorActionPreference = 'Stop'
$publisher = Join-Path $PSScriptRoot 'Publish-Effects.ps1'
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('LostArkEffectPipeline-' + [Guid]::NewGuid().ToString('N'))
$dataRoot = Join-Path $fixture 'Data'
$resourceRoot = Join-Path $fixture 'Resources'
$output = Join-Path $fixture 'Runtime\EffectCatalog.runtime.json'
$authored = Join-Path $dataRoot 'Effects\Authored'
$effectResource = Join-Path $resourceRoot 'Effect\Test'
$utf8NoBom = [Text.UTF8Encoding]::new($false)

function Write-Utf8([string]$Path, [string]$Text) {
    [IO.Directory]::CreateDirectory((Split-Path -Parent $Path)) | Out-Null
    [IO.File]::WriteAllText($Path, $Text, $utf8NoBom)
}

function Write-Fixture([object]$Document, [object]$Catalog) {
    Write-Utf8 (Join-Path $authored "$effectId.effect.json") `
        (($Document | ConvertTo-Json -Depth 30) + "`n")
    Write-Utf8 (Join-Path $dataRoot 'Effects\EffectCatalog.json') `
        (($Catalog | ConvertTo-Json -Depth 10) + "`n")
}

function Assert-PublishRejected([string]$Name, [byte[]]$Committed) {
    $failed = $false
    try {
        & $publisher -Mode Publish -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output
    }
    catch { $failed = $true }
    if (-not $failed) { throw "$Name was accepted." }
    if ([Convert]::ToBase64String($Committed) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw "$Name changed the committed runtime catalog."
    }
}

function Assert-RuntimeResourceHashes(
    [string]$RuntimePath,
    [string]$ResourcesPath) {
    $runtime = Get-Content -LiteralPath $RuntimePath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    foreach ($entry in @($runtime.effects)) {
        foreach ($dependency in @($entry.dependencies)) {
            $path = Join-Path $ResourcesPath ([string]$dependency.assetId)
            if (-not [IO.File]::Exists($path)) {
                throw "Runtime dependency is missing: $($dependency.assetId)"
            }
            $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
            if ($actual -cne ([string]$dependency.sha256).ToUpperInvariant()) {
                throw "Runtime dependency hash mismatch: $($dependency.assetId)"
            }
        }
    }
}

try {
    [IO.Directory]::CreateDirectory($authored) | Out-Null
    [IO.Directory]::CreateDirectory($effectResource) | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'base.dds'), [byte[]](1,2,3,4))
    $referenceDocumentPath = Join-Path $PSScriptRoot `
        '..\..\Data\Effects\Authored\effect.dimensionmaster.skill.2050110.effect.json'
    $referenceDocument = [IO.File]::ReadAllText(
        [IO.Path]::GetFullPath($referenceDocumentPath), [Text.Encoding]::UTF8) |
        ConvertFrom-Json
    $completeDetail = $referenceDocument.elements[0].detail
    $effectId = 'effect.pipeline.fixture'
    $document = [ordered]@{
        schema = 'lostark.effect-authoring'
        version = 5
        effectAssetId = $effectId
        displayName = 'Pipeline Fixture'
        elements = @([ordered]@{
            id = 'sprite'
            kind = 'sprite'
            resources = @([ordered]@{ slot = 'base'; assetId = 'Effect/Test/base.dds' })
            material = [ordered]@{ renderProfile = 'alpha_two_sided_depth_read' }
            detail = $completeDetail
        })
    }
    $catalog = [ordered]@{
        formatVersion = 1
        effects = @([ordered]@{
            effectAssetId = $effectId
            authoringPath = "Effects/Authored/$effectId.effect.json"
        })
    }
    Write-Fixture $document $catalog

    & $publisher -Mode Publish -DataRoot $dataRoot -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Baseline publish failed.' }
    $baseline = [IO.File]::ReadAllBytes($output)

    $catalog.formatVersion = 2
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong catalog version' $baseline
    $catalog.formatVersion = 1

    $document.version = 4
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong document version' $baseline
    $document.version = 5

    $catalog.effects[0].authoringPath = 'C:/escape.effect.json'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Drive-qualified authoring path' $baseline
    $catalog.effects[0].authoringPath = "Effects/Authored/$effectId.effect.json"

    $document.elements[0].resources[0].assetId = 'C:/escape.dds'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Drive-qualified resource path' $baseline
    $document.elements[0].resources[0].assetId = 'Effect/Test/base.dds'

    $document.elements[0].kind = 'unknown'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unknown Element kind' $baseline
    $document.elements[0].kind = 'sprite'

    $catalog.effects = @($catalog.effects[0], $catalog.effects[0])
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Duplicate Effect ID' $baseline
    $catalog.effects = @($catalog.effects[0])

    $firstElement = $document.elements[0]
    $duplicateElement = ($firstElement | ConvertTo-Json -Depth 30) |
        ConvertFrom-Json
    $document.elements = @($firstElement, $duplicateElement)
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Duplicate Element ID' $baseline
    $document.elements = @($firstElement)

    $document.elements[0].resources[0].assetId =
        'Effect/Test/missing.dds'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Missing resource payload' $baseline
    $document.elements[0].resources[0].assetId = 'Effect/Test/base.dds'

    Write-Fixture $document $catalog
    Assert-RuntimeResourceHashes $output $resourceRoot
    [IO.File]::WriteAllBytes(
        (Join-Path $effectResource 'base.dds'), [byte[]](9,8,7,6))
    $hashRejected = $false
    try { Assert-RuntimeResourceHashes $output $resourceRoot }
    catch { $hashRejected = $true }
    if (-not $hashRejected) { throw 'Runtime dependency hash mismatch was accepted.' }
    [IO.File]::WriteAllBytes(
        (Join-Path $effectResource 'base.dds'), [byte[]](1,2,3,4))

    Write-Fixture $document $catalog
    $outputLock = [IO.File]::Open(
        $output, [IO.FileMode]::Open, [IO.FileAccess]::Read,
        [IO.FileShare]::Read)
    try {
        Assert-PublishRejected 'Atomic promote failure' $baseline
    }
    finally {
        $outputLock.Dispose()
    }

    $document.elements[0].resources[0].assetId = '../escape.dds'
    Write-Utf8 (Join-Path $authored "$effectId.effect.json") (($document | ConvertTo-Json -Depth 30) + "`n")
    $failed = $false
    try {
        & $publisher -Mode Publish -DataRoot $dataRoot -ResourceRoot $resourceRoot -OutputPath $output
    }
    catch { $failed = $true }
    if (-not $failed) { throw 'Unsafe resource path was accepted.' }
    $afterFailure = [IO.File]::ReadAllBytes($output)
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String($afterFailure)) {
        throw 'Failed publish changed the committed runtime catalog.'
    }

    $document.elements[0].resources[0].assetId = 'Effect/Test/base.dds'
    $document.elements[0].detail.particle.maxParticles = 4096
    Write-Utf8 (Join-Path $authored "$effectId.effect.json") (($document | ConvertTo-Json -Depth 30) + "`n")
    $failed = $false
    try {
        & $publisher -Mode Publish -DataRoot $dataRoot -ResourceRoot $resourceRoot -OutputPath $output
    }
    catch { $failed = $true }
    if (-not $failed) { throw 'Out-of-budget Effect detail was accepted.' }
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'Detail validation failure changed the committed runtime catalog.'
    }

    $document.elements[0].detail.particle.maxParticles = 256
    $document.elements[0].resources = @()
    Write-Utf8 (Join-Path $authored "$effectId.effect.json") (($document | ConvertTo-Json -Depth 30) + "`n")
    $failed = $false
    try {
        & $publisher -Mode Publish -DataRoot $dataRoot -ResourceRoot $resourceRoot -OutputPath $output
    }
    catch { $failed = $true }
    if (-not $failed) { throw 'Missing required Base binding was accepted.' }
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'Required binding failure changed the committed runtime catalog.'
    }
    Write-Host 'PASS: Effect pipeline publish, version/path/kind/duplicate/resource/hash/budget/binding/promote rejection, and rollback.'
}
finally {
    if (Test-Path -LiteralPath $fixture) {
        Remove-Item -LiteralPath $fixture -Recurse -Force
    }
}
