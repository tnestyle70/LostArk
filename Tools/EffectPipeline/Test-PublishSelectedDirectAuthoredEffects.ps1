$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$publisher = Join-Path $PSScriptRoot `
    'Publish-SelectedDirectAuthoredEffects.ps1'
$runtimeValidator = Join-Path $PSScriptRoot `
    'validate_direct_authored_effect_runtime.py'
$canonicalRuntimeCatalog = Join-Path $repoRoot `
    'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$artistQ = 'effect.artist.skill.31200.unified'
$artistS = 'effect.artist.skill.31420.unified'
$artistA = 'effect.artist.skill.31460.linear-reveal.unified'
$valtanSkyAxe = 'effect.valtan.sky-axe.active'
$selectedTargets = @(
    'effect.artist.skill.31000.ba1.unified',
    'effect.artist.skill.31000.ba2.unified',
    'effect.artist.skill.31000.ba3.unified',
    'effect.artist.skill.31000.ba4.unified',
    'effect.artist.skill.31050.clip2.unified',
    $artistQ,
    'effect.artist.skill.31210.ba1.unified',
    'effect.artist.skill.31210.ba4.unified',
    $artistS,
    $artistA,
    'effect.dimensionmaster.skill.2050010.ba2.unified',
    'effect.dimensionmaster.skill.2050010.ba3.unified',
    'effect.dimensionmaster.skill.2050100.unified',
    'effect.dimensionmaster.skill.2050180.unified',
    'effect.dimensionmaster.skill.2050210.a1.unified',
    'effect.dimensionmaster.skill.2050220.unified',
    'effect.dimensionmaster.skill.2050240.clip2.unified',
    'effect.dimensionmaster.skill.2050500.unified',
    'effect.lancemaster.skill.34010.ba1.unified',
    'effect.lancemaster.skill.34010.ba2.unified',
    'effect.lancemaster.skill.34010.ba3.unified',
    'effect.lancemaster.skill.34010.ba4.unified',
    'effect.lancemaster.skill.34040.clip1.unified',
    'effect.lancemaster.skill.34040.clip2.unified',
    'effect.lancemaster.skill.34090.unified',
    'effect.lancemaster.skill.34100.clip1.unified',
    'effect.lancemaster.skill.34100.clip2.unified',
    'effect.lancemaster.skill.34100.clip3.unified',
    'effect.lancemaster.skill.34110.unified',
    'effect.lancemaster.skill.34110.v1.unified',
    'effect.lancemaster.skill.34120.clip1.unified',
    'effect.lancemaster.skill.34120.clip2.unified',
    'effect.lancemaster.skill.34120.clip3.unified',
    'effect.lancemaster.skill.34140.ba1.clip1.unified',
    'effect.lancemaster.skill.34140.ba2.clip2.unified',
    'effect.lancemaster.skill.34150.unified',
    'effect.lancemaster.skill.34160.ba1.unified',
    'effect.lancemaster.skill.34610.clip1.unified',
    'effect.lancemaster.skill.34610.clip2.unified',
    'effect.lancemaster.skill.34610.clip3.unified',
    'effect.lancemaster.skill.34650.clip1.unified',
    'effect.warlord.skill.17100.unified',
    'effect.warlord.skill.17110.clip2.unified',
    'effect.warlord.skill.17110.clip3.unified',
    'effect.warlord.skill.17140.unified',
    'effect.warlord.skill.17170.clip1.unified',
    'effect.warlord.skill.17170.clip2.unified',
    'effect.warlord.skill.17170.clip3.unified',
    'effect.warlord.skill.17240.ba1.unified',
    'effect.warlord.skill.17240.ba2.unified',
    'effect.warlord.skill.17240.ba3.unified',
    'effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01',
    'effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01',
    'effect.valtan.carrier-v1.attack.high-jump.land.clip-01',
    $valtanSkyAxe)
$protectedArtistA = 'effect.artist.skill.31460.unified'
$untouchedId = 'effect.fixture.keep'
$testRoot = Join-Path ([IO.Path]::GetTempPath()) `
    ('LostArkSelectedEffectPublisher-' + [Guid]::NewGuid().ToString('N'))
$dataRoot = Join-Path $testRoot 'Data'
$resourceRoot = Join-Path $testRoot 'Resources'
$runtimeRoot = Join-Path $testRoot 'Runtime'
$sourceAuthoredRoot = Join-Path $dataRoot 'Effects\Authored'
$runtimeAuthoredRoot = Join-Path $runtimeRoot 'Authored'
$sourceCatalogPath = Join-Path $dataRoot 'Effects\EffectCatalog.json'
$runtimeCatalogPath = Join-Path $runtimeRoot 'EffectCatalog.runtime.json'
$visualProgramPath = Join-Path $runtimeRoot `
    'EffectVisualPrograms.runtime.json'
$fixtureResource = Join-Path $resourceRoot `
    'Effect\Fixture\selected.dds'
$fixtureMeshResource = Join-Path $resourceRoot `
    'Character\Fixture\selected.wmodel'

function Write-Utf8Json([string]$Path, [object]$Value) {
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path)) |
        Out-Null
    [IO.File]::WriteAllText(
        $Path,
        ($Value | ConvertTo-Json -Depth 100 -Compress) + "`n",
        $utf8NoBom)
}

function Get-Sha256([byte[]]$Payload) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString(
            $sha.ComputeHash($Payload))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function New-AuthoredDocument(
    [string]$EffectId,
    [string]$DisplayName) {
    return [ordered]@{
        schema = 'lostark.effect-authoring'
        version = 13
        effectAssetId = $EffectId
        displayName = $DisplayName
        particleSystem = [ordered]@{}
        modelCues = @()
        elements = @([ordered]@{
            id = 'fixture.sprite'
            visible = $true
            kind = 'particle'
            resources = @([ordered]@{
                slotId = 'base'
                assetId = 'Effect/Fixture/selected.dds'
            })
        })
    }
}

function New-MeshAuthoredDocument(
    [string]$EffectId,
    [string]$DisplayName) {
    $document = New-AuthoredDocument $EffectId $DisplayName
    $document.elements[0].id = 'fixture.mesh'
    $document.elements[0].kind = 'mesh'
    $document.elements[0].resources = @([ordered]@{
        slotId = 'meshModel'
        assetId = 'Character/Fixture/selected.wmodel'
    })
    return $document
}

function Install-SealedDocument(
    [string]$EffectId,
    [object]$Document) {
    $payload = $utf8NoBom.GetBytes(
        ($Document | ConvertTo-Json -Depth 30 -Compress))
    $sha = Get-Sha256 $payload
    $relative = "Authored/$EffectId.$sha.effect.json"
    $destination = Join-Path $runtimeRoot $relative.Replace('/', '\')
    [IO.File]::WriteAllBytes($destination, $payload)
    return $relative
}

function Assert-True([bool]$Condition, [string]$Message) {
    if (-not $Condition) {
        throw $Message
    }
}

function Assert-ThrowsLike(
    [scriptblock]$Action,
    [string]$ExpectedText,
    [string]$Label) {
    $caught = $null
    try {
        & $Action
    }
    catch {
        $caught = $_
    }
    if ($null -eq $caught -or
        $caught.Exception.Message.IndexOf(
            $ExpectedText, [StringComparison]::OrdinalIgnoreCase) -lt 0) {
        throw "$Label did not fail with '$ExpectedText'."
    }
}

try {
    foreach ($directory in @(
            $sourceAuthoredRoot,
            $runtimeAuthoredRoot,
            [IO.Path]::GetDirectoryName($fixtureResource),
            [IO.Path]::GetDirectoryName($fixtureMeshResource))) {
        [IO.Directory]::CreateDirectory($directory) | Out-Null
    }
    [IO.File]::WriteAllBytes($fixtureResource, [byte[]](1, 2, 3, 4))
    [IO.File]::WriteAllBytes(
        $fixtureMeshResource, [byte[]](5, 6, 7, 8))

    $sourceCatalog = [ordered]@{
        formatVersion = 1
        effects = @($selectedTargets | ForEach-Object {
            [ordered]@{
                effectAssetId = $_
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                authoringPath = "Effects/Authored/$_.effect.json"
            }
        })
    }
    Write-Utf8Json $sourceCatalogPath $sourceCatalog
    foreach ($effectId in $selectedTargets) {
        $document = if ($effectId -ceq $valtanSkyAxe) {
            New-MeshAuthoredDocument $effectId "$effectId new"
        }
        else {
            New-AuthoredDocument $effectId "$effectId new"
        }
        Write-Utf8Json `
            (Join-Path $sourceAuthoredRoot "$effectId.effect.json") `
            $document
    }

    $oldS = Install-SealedDocument $artistS `
        (New-AuthoredDocument $artistS 'Artist S old')
    $protectedArtistASeal = Install-SealedDocument $protectedArtistA `
        (New-AuthoredDocument $protectedArtistA 'Protected Artist A V0')
    $untouchedRelative = Install-SealedDocument $untouchedId `
        (New-AuthoredDocument $untouchedId 'Untouched')

    $canonical = Get-Content -LiteralPath $canonicalRuntimeCatalog `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $materialPrograms = $canonical.materialPrograms
    $materialPrograms.bindings = @()
    $runtimeCatalog = [ordered]@{
        schema = 'lostark.effect-runtime-catalog'
        formatVersion = 4
        materialPrograms = $materialPrograms
        components = @()
        effects = @(
            [ordered]@{
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                effectAssetId = $artistS
                authoringFormatVersion = 13
                authoredDocumentPath = $oldS
            },
            [ordered]@{
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                effectAssetId = $protectedArtistA
                authoringFormatVersion = 13
                authoredDocumentPath = $protectedArtistASeal
            },
            [ordered]@{
                payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
                effectAssetId = $untouchedId
                authoringFormatVersion = 13
                authoredDocumentPath = $untouchedRelative
            })
    }
    Write-Utf8Json $runtimeCatalogPath $runtimeCatalog
    Write-Utf8Json $visualProgramPath ([ordered]@{
        schema = 'lostark.effect-visual-program-runtime'
        formatVersion = 1
        programs = @()
    })

    $python = Get-Command python -ErrorAction Stop
    & $python.Source -B $runtimeValidator --catalog $runtimeCatalogPath
    if ($LASTEXITCODE -ne 0) {
        throw 'Initial selected Effect publisher fixture was invalid.'
    }
    $initialCatalogBytes = [IO.File]::ReadAllBytes($runtimeCatalogPath)
    $initialCatalogSha = Get-Sha256 $initialCatalogBytes
    $initialVisualSha = Get-Sha256 `
        ([IO.File]::ReadAllBytes($visualProgramPath))
    $untouchedPath = Join-Path $runtimeRoot `
        $untouchedRelative.Replace('/', '\')
    $untouchedBytes = [IO.File]::ReadAllBytes($untouchedPath)
    $initialSealNames = @(
        Get-ChildItem -LiteralPath $runtimeAuthoredRoot -File |
            Sort-Object Name | ForEach-Object { $_.Name })

    & $publisher -Mode Validate -EffectAssetId $selectedTargets `
        -DataRoot $dataRoot -ResourceRoot $resourceRoot `
        -RuntimeRoot $runtimeRoot
    Assert-True ((Get-Sha256 ([IO.File]::ReadAllBytes(
        $runtimeCatalogPath))) -ceq $initialCatalogSha) `
        'Validate mode changed the runtime catalog.'
    Assert-True (@(Get-ChildItem -LiteralPath $runtimeAuthoredRoot -File).Count `
        -eq $initialSealNames.Count) `
        'Validate mode installed a runtime seal.'
    Assert-ThrowsLike {
        & $publisher -Mode Validate -EffectAssetId $protectedArtistA `
            -DataRoot $dataRoot -ResourceRoot $resourceRoot `
            -RuntimeRoot $runtimeRoot
    } 'audited direct-authored allowlist' 'Protected Artist A baseline target'

    $boundCatalog = Get-Content -LiteralPath $runtimeCatalogPath `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $boundCatalog.materialPrograms.bindings = @([ordered]@{
        effectAssetId = $artistS
    })
    Write-Utf8Json $runtimeCatalogPath $boundCatalog
    Assert-ThrowsLike {
        & $publisher -Mode Validate -EffectAssetId $artistS `
            -DataRoot $dataRoot -ResourceRoot $resourceRoot `
            -RuntimeRoot $runtimeRoot
    } 'material-program bound target' 'Material-bound selected target'
    [IO.File]::WriteAllBytes($runtimeCatalogPath, $initialCatalogBytes)

    Write-Utf8Json $visualProgramPath ([ordered]@{
        schema = 'lostark.effect-visual-program-runtime'
        formatVersion = 1
        programs = @([ordered]@{ effectAssetId = $artistA })
    })
    Assert-ThrowsLike {
        & $publisher -Mode Validate -EffectAssetId $artistA `
            -DataRoot $dataRoot -ResourceRoot $resourceRoot `
            -RuntimeRoot $runtimeRoot
    } 'visual-program bound target' 'Visual-bound selected target'
    Write-Utf8Json $visualProgramPath ([ordered]@{
        schema = 'lostark.effect-visual-program-runtime'
        formatVersion = 1
        programs = @()
    })

    & $publisher -Mode Publish -EffectAssetId $selectedTargets `
        -DataRoot $dataRoot -ResourceRoot $resourceRoot `
        -RuntimeRoot $runtimeRoot
    $publishedBytes = [IO.File]::ReadAllBytes($runtimeCatalogPath)
    $publishedSha = Get-Sha256 $publishedBytes
    $published = $utf8NoBom.GetString($publishedBytes) | ConvertFrom-Json
    foreach ($effectId in $selectedTargets) {
        $entry = @($published.effects | Where-Object {
            [string]$_.effectAssetId -ceq $effectId
        })[0]
        if ($effectId -ceq $artistS) {
            Assert-True (
                [string]$entry.authoredDocumentPath -cne $oldS) `
                "Published target remained stale: $effectId"
        }
        $sealedPath = Join-Path $runtimeRoot `
            ([string]$entry.authoredDocumentPath).Replace('/', '\')
        Assert-True ([IO.File]::Exists($sealedPath)) `
            "Published target seal is missing: $effectId"
    }
    $untouchedEntry = @($published.effects | Where-Object {
        [string]$_.effectAssetId -ceq $untouchedId
    })[0]
    Assert-True (
        [string]$untouchedEntry.authoredDocumentPath -ceq
            $untouchedRelative) `
        'Non-target runtime entry changed.'
    Assert-True ((Get-Sha256 ([IO.File]::ReadAllBytes(
        $untouchedPath))) -ceq (Get-Sha256 $untouchedBytes)) `
        'Non-target sealed file changed.'
    Assert-True ([IO.File]::Exists((Join-Path $runtimeRoot `
        $oldS.Replace('/', '\')))) 'Old Artist S seal was removed.'
    Assert-True ([IO.File]::Exists((Join-Path $runtimeRoot `
        $protectedArtistASeal.Replace('/', '\')))) `
        'Protected Artist A V0 seal was removed.'
    Assert-True ((Get-Sha256 ([IO.File]::ReadAllBytes(
        $visualProgramPath))) -ceq $initialVisualSha) `
        'Visual-program sidecar changed.'

    & $publisher -Mode Publish -EffectAssetId $selectedTargets `
        -DataRoot $dataRoot -ResourceRoot $resourceRoot `
        -RuntimeRoot $runtimeRoot
    Assert-True ((Get-Sha256 ([IO.File]::ReadAllBytes(
        $runtimeCatalogPath))) -ceq $publishedSha) `
        'No-op selected publish rewrote the runtime catalog.'

    $mutatedS = New-AuthoredDocument $artistS 'Artist S rollback probe'
    Write-Utf8Json (Join-Path $sourceAuthoredRoot "$artistS.effect.json") `
        $mutatedS
    $beforeRollbackSeals = @(
        Get-ChildItem -LiteralPath $runtimeAuthoredRoot -File |
            Sort-Object Name | ForEach-Object { $_.Name })
    Assert-ThrowsLike {
        & $publisher -Mode Publish -EffectAssetId $artistS `
            -DataRoot $dataRoot -ResourceRoot $resourceRoot `
            -RuntimeRoot $runtimeRoot `
            -TestFaultInjection AfterCatalogBackup
    } 'Injected selected Effect publish failure' `
        'Selected catalog rollback fault'
    Assert-True ((Get-Sha256 ([IO.File]::ReadAllBytes(
        $runtimeCatalogPath))) -ceq $publishedSha) `
        'Injected failure did not restore the runtime catalog.'
    $afterRollbackSeals = @(
        Get-ChildItem -LiteralPath $runtimeAuthoredRoot -File |
            Sort-Object Name | ForEach-Object { $_.Name })
    Assert-True (($beforeRollbackSeals -join "`n") -ceq
        ($afterRollbackSeals -join "`n")) `
        'Injected failure did not remove its new seal.'
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $runtimeRoot `
        '.Publish-Effects.transaction.lock'))) `
        'Selected publisher left its transaction lock behind.'

    & $python.Source -B $runtimeValidator --catalog $runtimeCatalogPath
    if ($LASTEXITCODE -ne 0) {
        throw 'Published selected Effect fixture failed final runtime validation.'
    }
    Write-Host 'PASS: selected direct-authored Effect validate/publish/no-op/binding/rollback contract.'
}
finally {
    if (Test-Path -LiteralPath $testRoot) {
        Remove-Item -LiteralPath $testRoot -Recurse -Force
    }
}
