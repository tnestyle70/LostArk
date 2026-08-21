$ErrorActionPreference = 'Stop'
$publisher = Join-Path $PSScriptRoot 'Publish-Effects.ps1'
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('LostArkEffectPipeline-' + [Guid]::NewGuid().ToString('N'))
$dataRoot = Join-Path $fixture 'Data'
$resourceRoot = Join-Path $fixture 'Resources'
$output = Join-Path $fixture 'Runtime\EffectCatalog.runtime.json'
$authored = Join-Path $dataRoot 'Effects\Authored'
$assemblies = Join-Path $dataRoot 'Effects\Assemblies\Fixture'
$components = Join-Path $dataRoot 'Effects\Components\Fixture'
$effectResource = Join-Path $resourceRoot 'Effect\Test'
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$utf8NoBomStrict = [Text.UTF8Encoding]::new($false, $true)

function Write-Utf8([string]$Path, [string]$Text) {
    [IO.Directory]::CreateDirectory((Split-Path -Parent $Path)) | Out-Null
    [IO.File]::WriteAllText($Path, $Text, $utf8NoBom)
}

function Get-CanonicalTrackedTextSha256([string]$Path) {
    $payload = [IO.File]::ReadAllBytes($Path)
    if ($payload.Length -ge 3 -and $payload[0] -eq 0xEF -and
        $payload[1] -eq 0xBB -and $payload[2] -eq 0xBF) {
        throw "Tracked JSON must be UTF-8 without BOM: $Path"
    }
    try {
        $text = $utf8NoBomStrict.GetString($payload)
    }
    catch {
        throw "Tracked JSON must be valid UTF-8: $Path"
    }
    $normalized = $text.Replace("`r`n", "`n").Replace("`r", "`n")
    $canonicalBytes = $utf8NoBom.GetBytes($normalized)
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString(
            $sha.ComputeHash($canonicalBytes))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha.Dispose()
    }
}

function Write-Fixture(
    [object]$Document,
    [object]$Catalog,
    [object[]]$SourceActionCues = @()) {
    $authoringPath = Join-Path $authored "$effectId.effect.json"
    Write-Utf8 $authoringPath (($Document | ConvertTo-Json -Depth 30) + "`n")
    Write-Utf8 (Join-Path $dataRoot 'Effects\EffectCatalog.json') `
        (($Catalog | ConvertTo-Json -Depth 10) + "`n")
    $componentId = 'effect.component.fixture.00'
    $componentDocument = ($Document | ConvertTo-Json -Depth 30) |
        ConvertFrom-Json
    $componentDocument.effectAssetId = $componentId
    $emitters = [Collections.Generic.List[object]]::new()
    for ($index = 0; $index -lt @($componentDocument.elements).Count; ++$index) {
        $element = $componentDocument.elements[$index]
        $renderer = if (@($element.resources | Where-Object {
            $_.slotId -eq 'meshModel'
        }).Count -ne 0) { 'mesh' } else { [string]$element.kind }
        $moduleCount = if ($null -ne $element.sourceRecipe) {
            @($element.sourceRecipe.modules).Count
        } else { 0 }
        $emitters.Add([ordered]@{
            emitterId = [string]$element.id
            elementId = [string]$element.id
            sourceElementIndex = $index
            renderer = $renderer
            visible = [bool]$element.visible
            resourceBindingCount = @($element.resources).Count
            moduleCount = $moduleCount
        })
    }
    $component = [ordered]@{
        schema = 'lostark.effect-component'
        version = 1
        componentAssetId = $componentId
        displayName = 'Fixture Component'
        componentType = 'particleSystem'
        source = [ordered]@{
            effectAssetId = $effectId
            groupId = 'fixture_group'
            sourceNodes = @('Fixture/Node/0')
            sourceElementSha256 = ('0' * 64)
        }
        emitters = @($emitters)
        document = $componentDocument
    }
    $particleSystem = if ($null -ne $Document.particleSystem) {
        $Document.particleSystem
    } else { [ordered]@{} }
    $modelCues = if ($null -ne $Document.modelCues) {
        @($Document.modelCues)
    } else { @() }
    $assembly = [ordered]@{
        schema = 'lostark.effect-assembly'
        version = 1
        effectAssetId = $effectId
        displayName = [string]$Document.displayName
        sourceAuthoringVersion = [int]$Document.version
        particleSystem = $particleSystem
        modelCues = @($modelCues)
        componentCues = @([ordered]@{
            cueId = 'component-00'
            componentAssetId = $componentId
            startDelaySeconds = 0.0
            visible = $true
            anchor = 'root'
            localTransform = [ordered]@{
                position = @(0.0, 0.0, 0.0)
                rotationDegrees = @(0.0, 0.0, 0.0)
                scale = @(1.0, 1.0, 1.0)
            }
        })
        sourceActionCues = @($SourceActionCues)
        sourceDocumentSha256 = ('1' * 64)
        sourceDocumentFileSha256 = Get-CanonicalTrackedTextSha256 $authoringPath
    }
    Write-Utf8 (Join-Path $components 'Fixture_00.particlesystem.wfx.json') `
        (($component | ConvertTo-Json -Depth 40) + "`n")
    Write-Utf8 (Join-Path $assemblies "$effectId.assembly.json") `
        (($assembly | ConvertTo-Json -Depth 40) + "`n")
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

function Assert-ValidateRejected([string]$Name, [byte[]]$Committed) {
    $failed = $false
    try {
        & $publisher -Mode Validate -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output
    }
    catch { $failed = $true }
    if (-not $failed) { throw "$Name was accepted by validation." }
    if ([Convert]::ToBase64String($Committed) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw "$Name changed the committed runtime catalog during validation."
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
    $visualProgramSource = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot `
        '..\..\Data\Effects\VisualPrograms\effect-visual-program-runtime.v1.json'))
    $fixtureVisualProgramSource = Join-Path $dataRoot `
        'Effects\VisualPrograms\effect-visual-program-runtime.v1.json'
    [IO.Directory]::CreateDirectory(
        (Split-Path -Parent $fixtureVisualProgramSource)) | Out-Null
    [IO.File]::WriteAllBytes(
        $fixtureVisualProgramSource,
        [IO.File]::ReadAllBytes($visualProgramSource))
    foreach ($animationAssetId in @(
            'Artist', 'DimensionMaster', 'LanceMaster', 'Warlord')) {
        $eventPath = Join-Path $dataRoot (
            "Animation\Authored\$animationAssetId\$animationAssetId.animevents")
        $eventRows = if ($animationAssetId -ceq 'Artist') {
            @('"fixture_clip" EFFECT startms=0 effectref=asset payload="effect.valtan.pipeline-fixture"')
        }
        else {
            @()
        }
        Write-Utf8 $eventPath ((
            "LOSTARK_ANIM_EVENTS 5 `"$animationAssetId`" $($eventRows.Count)`n" +
            (($eventRows -join "`n") + $(if ($eventRows.Count) { "`n" } else { '' }))))
    }
    Write-Utf8 (Join-Path $dataRoot `
        'Animation\Authored\Valtan\Valtan.patterneffectcues.json') ((
        [ordered]@{
            schema = 'lostark.valtan-pattern-effect-cues'
            formatVersion = 2
            ownerArchetypeId = 'BOSS_VALTAN'
            cues = @([ordered]@{
                bindingId = 'cue.valtan.pipeline-fixture.active'
                occurrenceId = 'cue.valtan.pipeline-fixture.active.occurrence.01'
                patternId = 'VALTAN_PIPELINE_FIXTURE'
                stageId = 'ACTIVE'
                actionId = 'valtan.pipeline-fixture.active'
                clipOccurrenceId = 'valtan.pipeline-fixture.active.clip.01'
                effectAssetId = 'effect.valtan.pipeline-fixture'
                anchorSlotId = 'root'
                followPolicy = 'follow'
                stopPolicy = 'cue_end'
                repeatPolicy = 'once'
                sourceStartMs = 0
                sourceEndMs = 100
                localTransform = [ordered]@{
                    position = @(0.0, 0.0, 0.0)
                    rotationDegrees = @(0.0, 0.0, 0.0)
                    scale = @(1.0, 1.0, 1.0)
                }
            })
        } | ConvertTo-Json -Depth 10) + "`n")
    Write-Utf8 (Join-Path $dataRoot `
        'Animation\Authored\Valtan\Valtan.patternbindings.json') ((
        [ordered]@{
            schema = 'lostark.valtan-pattern-bindings'
            formatVersion = 2
            bossArchetypeId = 'BOSS_VALTAN'
            bindings = @([ordered]@{
                actionId = 'valtan.pipeline-fixture.active'
                clips = @([ordered]@{
                    clipOccurrenceId = 'valtan.pipeline-fixture.active.clip.01'
                    clip = 'fixture_clip'
                    mappingBasis = 'PROJECT_AUTHORED'
                    sourceStartMs = 0
                    playMs = 100
                    playRate = 1.0
                    loop = $false
                })
            })
        } | ConvertTo-Json -Depth 10) + "`n")
    Write-Utf8 (Join-Path $dataRoot `
        'Animation\Reference\Valtan\Valtan.animnotify') ((
        "LOSTARK_ANIM_NOTIFY 1 `"Valtan`" 1`n" +
        "`"fixture_clip`" skill=1 len=0.1000 name=`"Fixture`"`n"))
    Write-Utf8 (Join-Path $dataRoot `
        'Encounters\Valtan\ValtanEncounter.json') ((
        [ordered]@{ patterns = @([ordered]@{
            patternId = 'VALTAN_PIPELINE_FIXTURE'
            stages = @([ordered]@{
                stageId = 'ACTIVE'
                actionId = 'valtan.pipeline-fixture.active'
                durationMs = 100
            })
        }) } | ConvertTo-Json -Depth 10) + "`n")
    [IO.Directory]::CreateDirectory($effectResource) | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'base.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'blankwhite.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'normal.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'reflection.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'dissolve.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'mask.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'emissive.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'noise.dds'), [byte[]](1,2,3,4))
    [IO.File]::WriteAllBytes((Join-Path $effectResource 'mesh.wmodel'), [byte[]](5,6,7,8))
    $modelCueResource = Join-Path $resourceRoot 'Character\Test'
    [IO.Directory]::CreateDirectory($modelCueResource) | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $modelCueResource 'cue.wmodel'),
        [byte[]](9,10,11,12))
    $referenceDocumentPath = Join-Path $PSScriptRoot `
        '..\..\Data\Effects\Authored\effect.dimensionmaster.skill.2050240.effect.json'
    $referenceDocument = [IO.File]::ReadAllText(
        [IO.Path]::GetFullPath($referenceDocumentPath), [Text.Encoding]::UTF8) |
        ConvertFrom-Json
    $completeDetail = $referenceDocument.elements[0].detail
    $effectId = 'effect.valtan.pipeline-fixture'
    $document = [ordered]@{
        schema = 'lostark.effect-authoring'
        version = 6
        effectAssetId = $effectId
        displayName = 'Pipeline Fixture'
        elements = @([ordered]@{
            id = 'sprite'
            displayName = 'Sprite'
            groupId = 'fixture_group'
            sourceNode = 'Fixture/Node/0'
            visible = $true
            kind = 'sprite'
            resources = @([ordered]@{ slotId = 'base'; assetId = 'Effect/Test/base.dds' })
            material = [ordered]@{
                templateId = 'effect.standard'
                renderProfile = 'alpha_two_sided_depth_read'
            }
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
    $baselineRuntime = Get-Content -LiteralPath $output -Raw -Encoding UTF8 |
        ConvertFrom-Json
    if ([int]$baselineRuntime.formatVersion -ne 2 -or
        $null -ne $baselineRuntime.PSObject.Properties['schema'] -or
        $null -ne $baselineRuntime.effects[0].PSObject.Properties['payloadKind']) {
        throw 'Legacy-only publish no longer preserves the format-2 catalog shape.'
    }

    $valtanCueFixturePath = Join-Path $dataRoot `
        'Animation\Authored\Valtan\Valtan.patterneffectcues.json'
    $valtanBindingFixturePath = Join-Path $dataRoot `
        'Animation\Authored\Valtan\Valtan.patternbindings.json'
    $valtanDurationFixturePath = Join-Path $dataRoot `
        'Animation\Reference\Valtan\Valtan.animnotify'
    $validValtanCueText = [IO.File]::ReadAllText(
        $valtanCueFixturePath, $utf8NoBomStrict)
    $validValtanBindingText = [IO.File]::ReadAllText(
        $valtanBindingFixturePath, $utf8NoBomStrict)
    $validValtanDurationText = [IO.File]::ReadAllText(
        $valtanDurationFixturePath, $utf8NoBomStrict)

    $invalidValtanBinding = $validValtanBindingText | ConvertFrom-Json
    $invalidValtanBinding.bindings[0].clips[0].mappingBasis = 'NAME_GUESS'
    Write-Utf8 $valtanBindingFixturePath `
        (($invalidValtanBinding | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected 'Valtan invalid clip mappingBasis' $baseline
    Write-Utf8 $valtanBindingFixturePath $validValtanBindingText

    $invalidValtanBinding = $validValtanBindingText | ConvertFrom-Json
    $firstClip = $invalidValtanBinding.bindings[0].clips[0]
    $secondClip = ($firstClip | ConvertTo-Json -Depth 10) | ConvertFrom-Json
    $firstClip.loop = $true
    $secondClip.clipOccurrenceId = 'valtan.pipeline-fixture.active.clip.02'
    $secondClip.loop = $false
    $invalidValtanBinding.bindings[0].clips = @($firstClip, $secondClip)
    Write-Utf8 $valtanBindingFixturePath `
        (($invalidValtanBinding | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected 'Valtan non-final loop clip' $baseline
    Write-Utf8 $valtanBindingFixturePath $validValtanBindingText

    $invalidValtanCue = $validValtanCueText | ConvertFrom-Json
    $invalidValtanCue.cues[0].sourceStartMs = 100
    $invalidValtanCue.cues[0].sourceEndMs = 101
    Write-Utf8 $valtanCueFixturePath `
        (($invalidValtanCue | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected 'Valtan cue outside clip source segment' $baseline
    Write-Utf8 $valtanCueFixturePath $validValtanCueText

    $invalidValtanCue = $validValtanCueText | ConvertFrom-Json
    $invalidValtanCue.cues[0].repeatPolicy = 'each_loop'
    Write-Utf8 $valtanCueFixturePath `
        (($invalidValtanCue | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected 'Valtan each_loop cue on non-loop clip' $baseline
    Write-Utf8 $valtanCueFixturePath $validValtanCueText

    # Source positions are clip-local coordinates, not direct encounter wall
    # milliseconds. A sourceStart at 2450ms is valid when its effective
    # source segment maps to this 100ms gameplay stage.
    $sourceLocalBinding = $validValtanBindingText | ConvertFrom-Json
    $sourceLocalBinding.bindings[0].clips[0].sourceStartMs = 2450
    $sourceLocalBinding.bindings[0].clips[0].playMs = 100
    $sourceLocalCue = $validValtanCueText | ConvertFrom-Json
    $sourceLocalCue.cues[0].sourceStartMs = 2450
    $sourceLocalCue.cues[0].sourceEndMs = 2550
    Write-Utf8 $valtanDurationFixturePath ((
        "LOSTARK_ANIM_NOTIFY 1 `"Valtan`" 1`n" +
        "`"fixture_clip`" skill=1 len=3.0000 name=`"Fixture`"`n"))
    Write-Utf8 $valtanBindingFixturePath `
        (($sourceLocalBinding | ConvertTo-Json -Depth 20) + "`n")
    Write-Utf8 $valtanCueFixturePath `
        (($sourceLocalCue | ConvertTo-Json -Depth 20) + "`n")
    & $publisher -Mode Validate -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) {
        throw 'Valtan source-local wall mapping fixture was rejected.'
    }

    $sourceEndStartCue = ($sourceLocalCue | ConvertTo-Json -Depth 20) |
        ConvertFrom-Json
    $sourceEndStartCue.cues[0].sourceStartMs = 2550
    $sourceEndStartCue.cues[0].sourceEndMs = $null
    $sourceEndStartCue.cues[0].stopPolicy = 'natural'
    Write-Utf8 $valtanCueFixturePath `
        (($sourceEndStartCue | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected `
        'Valtan cue sourceStart at exclusive clip sourceEnd' $baseline

    $stageEndBinding = ($sourceLocalBinding | ConvertTo-Json -Depth 20) |
        ConvertFrom-Json
    $stageEndBinding.bindings[0].clips[0].playMs = 200
    $stageEndCue = ($sourceLocalCue | ConvertTo-Json -Depth 20) |
        ConvertFrom-Json
    $stageEndCue.cues[0].sourceStartMs = 2550
    $stageEndCue.cues[0].sourceEndMs = $null
    $stageEndCue.cues[0].stopPolicy = 'natural'
    Write-Utf8 $valtanBindingFixturePath `
        (($stageEndBinding | ConvertTo-Json -Depth 20) + "`n")
    Write-Utf8 $valtanCueFixturePath `
        (($stageEndCue | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected `
        'Valtan non-loop cue start at exclusive encounter stage end' $baseline

    $stageEndBinding.bindings[0].clips[0].loop = $true
    Write-Utf8 $valtanBindingFixturePath `
        (($stageEndBinding | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected `
        'Valtan loop cue start at exclusive encounter stage end' $baseline

    $sourceLocalBinding.bindings[0].clips[0].playMs = 200
    $sourceLocalCue.cues[0].sourceStartMs = 2551
    $sourceLocalCue.cues[0].sourceEndMs = 2552
    Write-Utf8 $valtanBindingFixturePath `
        (($sourceLocalBinding | ConvertTo-Json -Depth 20) + "`n")
    Write-Utf8 $valtanCueFixturePath `
        (($sourceLocalCue | ConvertTo-Json -Depth 20) + "`n")
    Assert-ValidateRejected 'Valtan cue outside encounter wall stage' $baseline
    Write-Utf8 $valtanCueFixturePath $validValtanCueText
    Write-Utf8 $valtanBindingFixturePath $validValtanBindingText
    Write-Utf8 $valtanDurationFixturePath $validValtanDurationText

    $authoringPath = Join-Path $authored "$effectId.effect.json"
    $authoringLf = [IO.File]::ReadAllText(
        $authoringPath, $utf8NoBomStrict).Replace("`r`n", "`n").Replace("`r", "`n")
    Write-Utf8 $authoringPath $authoringLf
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'LF authoring checkout changed the published catalog identity.'
    }

    $authoringCrLf = $authoringLf.Replace("`n", "`r`n")
    Write-Utf8 $authoringPath $authoringCrLf
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'CRLF authoring checkout changed the published catalog identity.'
    }

    [byte[]]$bomAuthoring = @([byte]0xEF, [byte]0xBB, [byte]0xBF) +
        $utf8NoBom.GetBytes($authoringCrLf)
    [IO.File]::WriteAllBytes($authoringPath, $bomAuthoring)
    Assert-PublishRejected 'UTF-8 BOM authoring' $baseline

    $semanticAuthoring = $authoringLf | ConvertFrom-Json
    $semanticAuthoring.displayName = 'Semantically Changed Fixture'
    Write-Utf8 $authoringPath `
        (($semanticAuthoring | ConvertTo-Json -Depth 30) + "`n")
    Assert-PublishRejected 'Unresealed authoring semantic mutation' $baseline
    Write-Fixture $document $catalog

    $componentPath = Join-Path $components 'Fixture_00.particlesystem.wfx.json'
    $assemblyPath = Join-Path $assemblies "$effectId.assembly.json"
    $invalidComponent = Get-Content -LiteralPath $componentPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $invalidComponent.displayName = 'x' * 65
    Write-Utf8 $componentPath (($invalidComponent | ConvertTo-Json -Depth 40) + "`n")
    Assert-ValidateRejected 'Overlong Effect Component displayName' $baseline

    Write-Fixture $document $catalog
    $invalidComponent = Get-Content -LiteralPath $componentPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $invalidComponent.document.displayName = 'x' * 65
    Write-Utf8 $componentPath (($invalidComponent | ConvertTo-Json -Depth 40) + "`n")
    Assert-ValidateRejected 'Overlong Effect Component Document displayName' $baseline

    Write-Fixture $document $catalog
    $invalidAssembly = Get-Content -LiteralPath $assemblyPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $invalidAssembly.displayName = 'x' * 65
    Write-Utf8 $assemblyPath (($invalidAssembly | ConvertTo-Json -Depth 40) + "`n")
    Assert-ValidateRejected 'Overlong Effect Assembly displayName' $baseline
    Write-Fixture $document $catalog

    $document.elements[0].detail.particle | Add-Member `
        -NotePropertyName sourceScale -NotePropertyValue ([ordered]@{
            count = 16.0
            size = 16.0
            lifeTime = 16.0
            speed = -16.0
            rotation = 360.0
            alpha = 0.0
            spawnDelay = 0.0
        }) -Force
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Boundary Source Trim publish failed.' }
    $sourceScaleBaseline = [IO.File]::ReadAllBytes($output)

    $document.elements[0].detail.particle.sourceScale.count = 0.0
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim nonpositive count' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.count = 16.0

    $document.elements[0].detail.particle.sourceScale.size = 0.0
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim nonpositive size' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.size = 16.0

    $document.elements[0].detail.particle.sourceScale.lifeTime = 16.01
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim lifetime overflow' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.lifeTime = 16.0

    $document.elements[0].detail.particle.sourceScale.rotation = 360.01
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim rotation overflow' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.rotation = 360.0

    $document.elements[0].detail.particle.sourceScale.speed = -16.01
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim speed overflow' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.speed = -16.0

    $document.elements[0].detail.particle.sourceScale.alpha = -0.01
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim negative alpha' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.alpha = 0.0

    $document.elements[0].detail.particle.sourceScale.spawnDelay = 16.01
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Source Trim delay overflow' $sourceScaleBaseline
    $document.elements[0].detail.particle.sourceScale.spawnDelay = 0.0

    Write-Fixture $document $catalog
    foreach ($path in @($authoringPath, $componentPath)) {
        $text = [IO.File]::ReadAllText($path, $utf8NoBomStrict)
        $nonFinite = [regex]::Replace(
            $text, '"rotation"\s*:\s*360(?:\.0+)?',
            '"rotation": 1e309', 1)
        if ($nonFinite -ceq $text) {
            throw "Source Trim non-finite fixture did not find rotation: $path"
        }
        Write-Utf8 $path $nonFinite
    }
    $nonFiniteAssembly = Get-Content -LiteralPath $assemblyPath `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $nonFiniteAssembly.sourceDocumentFileSha256 =
        Get-CanonicalTrackedTextSha256 $authoringPath
    Write-Utf8 $assemblyPath `
        (($nonFiniteAssembly | ConvertTo-Json -Depth 40) + "`n")
    Assert-PublishRejected 'Source Trim non-finite rotation' $sourceScaleBaseline

    $document.elements[0].detail.particle.PSObject.Properties.Remove(
        'sourceScale')
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Source Trim fixture restore failed.' }
    if ([Convert]::ToBase64String($baseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'Source Trim fixture restore changed the baseline catalog.'
    }

    $document.version = 7
    $document['modelCues'] = @([ordered]@{
        cueId = 'fixture_model'
        modelAssetId = 'Character/Test/cue.wmodel'
        clipName = 'fixture_clip'
        startDelaySeconds = 0.0
        durationSeconds = 1.0
        visible = $true
        localTransform = [ordered]@{
            position = @(0.0, 0.0, 0.0)
            rotationDegrees = @(0.0, 0.0, 0.0)
            scale = @(1.0, 1.0, 1.0)
        }
        assetPreTransform = [ordered]@{
            scale = @(0.01, 0.01, 0.01)
            rotationDegrees = @(0.0, -90.0, 0.0)
        }
    })
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v7 Model Cue publish failed.' }
    Assert-RuntimeResourceHashes $output $resourceRoot

    $document.version = 8
    $document['particleSystem'] = [ordered]@{
        uniformScaleMultiplier = 1.0
        yawOffsetDegrees = 0.0
        directionYawDegrees = 0.0
        initialSpeedMultiplier = 1.0
    }
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v8 Particle System publish failed.' }
    Assert-RuntimeResourceHashes $output $resourceRoot
    $modelCueBaseline = [IO.File]::ReadAllBytes($output)

    $invalidSourceCue = [ordered]@{
        cueId = 'skill-1/clip-000/notify-001'
        localTimeSeconds = 1.0e22
        globalTimeSeconds = 1.0e22
        durationSeconds = -3.0
        executionEnabled = $true
    }
    Write-Fixture $document $catalog @($invalidSourceCue)
    Assert-PublishRejected 'Executable malformed Source Action cue time' `
        $modelCueBaseline
    $invalidSourceCue.executionEnabled = $false
    $invalidSourceCue.sourceExecutionStatus = 'INVALID_SOURCE_TIME_FAIL_CLOSED'
    $invalidSourceCue.executionDisabledReason =
        'SOURCE_ACTION_CUE_TIME_OUTSIDE_FINITE_0_TO_60_SECONDS'
    Write-Fixture $document $catalog @($invalidSourceCue)
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Fail-closed malformed Source Action cue publish failed.' }
    $modelCueBaseline = [IO.File]::ReadAllBytes($output)
    Write-Fixture $document $catalog

    $document.particleSystem.uniformScaleMultiplier = 0.0
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Invalid Particle System scale' $modelCueBaseline
    $document.particleSystem.uniformScaleMultiplier = 1.0

    $version8Element = ($document.elements[0] | ConvertTo-Json -Depth 30) |
        ConvertFrom-Json
    $groupedElement = ($referenceDocument.elements[2] |
        ConvertTo-Json -Depth 100) | ConvertFrom-Json
    $groupedElement.id = 'grouped'
    $groupedElement.displayName = 'Grouped Source Material'
    $groupedElement.groupId = 'fixture_group'
    $groupedElement.sourceNode = 'Fixture/Grouped/0'
    $groupedElement.resources = @([ordered]@{
        slotId = 'base'
        assetId = 'Effect/Test/base.dds'
    })
    $groupedElement.material.templateId = 'effect.source_material'
    $groupedElement.material.sourceMaterialPath =
        'fx_m_mi_00.fx_mi.fx_fixture_grouped_tr'
    $groupedElement.material.sourceProfile.profileId =
        'ue3.material.fixture.grouped'
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.grouped-translucent.v1'
    $groupedElement.material.sourceProfile.parentMaterialPath =
        'fx_m.fx_fixture_grouped_tr'
    $groupedElement.material.sourceProfile.semanticStatus =
        'reconstructed_profile'
    $groupedElement.material.sourceProfile.scalars = @()
    $groupedElement.material.sourceProfile.vectors = @()
    $document.version = 12
    $document.elements = @($groupedElement)
    Write-Fixture $document $catalog
    & $publisher -Mode Validate -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v12 Grouped Source Material validation failed.' }
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v12 Grouped Source Material publish failed.' }

    $originalSourceSubUVMode =
        [string]$groupedElement.material.sourceProfile.subUVMode
    foreach ($sourceSubUVMode in @(
            'none',
            'psuvim_random',
            'psuvim_linear_blend',
            'psuvim_linear_blend_random_flip_square')) {
        $groupedElement.material.sourceProfile.subUVMode = $sourceSubUVMode
        Write-Fixture $document $catalog
        & $publisher -Mode Validate -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output
        if ($LASTEXITCODE) {
            throw "Source Material SubUV mode validation failed: $sourceSubUVMode"
        }
        & $publisher -Mode Publish -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output
        if ($LASTEXITCODE) {
            throw "Source Material SubUV mode publish failed: $sourceSubUVMode"
        }
    }
    $groupedElement.material.sourceProfile.subUVMode = $originalSourceSubUVMode
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Grouped Source Material restore failed.' }
    $groupedBaseline = [IO.File]::ReadAllBytes($output)

    # The two strict typed family profiles remain compiler evidence only here;
    # this focused publisher seam proves their complete texture/resource ABI
    # and rollback without changing product mappings.
    $typedElement = ($groupedElement | ConvertTo-Json -Depth 100) |
        ConvertFrom-Json
    $typedElement.kind = 'particle'
    $typedElement.resources = @(
        [ordered]@{ slotId = 'meshModel'; assetId = 'Effect/Test/mesh.wmodel' },
        [ordered]@{ slotId = 'base'; assetId = 'Effect/Test/base.dds' },
        [ordered]@{ slotId = 'mask'; assetId = 'Effect/Test/mask.dds' },
        [ordered]@{ slotId = 'emissive'; assetId = 'Effect/Test/emissive.dds' },
        [ordered]@{ slotId = 'noise'; assetId = 'Effect/Test/noise.dds' },
        [ordered]@{ slotId = 'dissolve'; assetId = 'Effect/Test/dissolve.dds' })
    $typedElement.material.sourceProfile.runtimeShaderProfileId =
		'effect.ue3.missiletrail-two-emissive.v1'
    $typedElement.material.sourceProfile.dynamicParameterSemantics = @(
        'missile_alpha_pan','missile_noise_strength',
        'missile_noise_pan','missile_dissolve')
    $typedElement.material.sourceProfile | Add-Member `
        -NotePropertyName textures -Force -NotePropertyValue @(
        [ordered]@{ name='alpha_tex'; sourceObjectPath='fx.alpha'; assetId='Effect/Test/mask.dds' },
        [ordered]@{ name='emissive_tex01'; sourceObjectPath='fx.emissive01'; assetId='Effect/Test/base.dds' },
        [ordered]@{ name='emissive_tex02'; sourceObjectPath='fx.emissive02'; assetId='Effect/Test/emissive.dds' },
        [ordered]@{ name='uv_dissolve_tex'; sourceObjectPath='fx.dissolve'; assetId='Effect/Test/dissolve.dds' },
        [ordered]@{ name='uv_noise_tex'; sourceObjectPath='fx.noise'; assetId='Effect/Test/noise.dds' })
    $document.elements = @($typedElement)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Typed MissileTrail publish failed.' }
    $typedMissileBaseline = [IO.File]::ReadAllBytes($output)
    $savedTypedResources = @($typedElement.resources)
    $typedElement.resources = @($typedElement.resources | Where-Object {
        $_.slotId -ne 'emissive' })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Typed MissileTrail missing second emissive lane' `
        $typedMissileBaseline
    $typedElement.resources = $savedTypedResources
    $savedTypedTextures = @($typedElement.material.sourceProfile.textures)
    $typedElement.material.sourceProfile.textures = @(
        $savedTypedTextures | Where-Object { $_.name -ne 'emissive_tex02' })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Typed MissileTrail missing named emissive texture' `
        $typedMissileBaseline

    $typedElement.resources = @(
        [ordered]@{ slotId = 'meshModel'; assetId = 'Effect/Test/mesh.wmodel' },
        [ordered]@{ slotId = 'base'; assetId = 'Effect/Test/base.dds' },
        [ordered]@{ slotId = 'noise'; assetId = 'Effect/Test/noise.dds' })
    $typedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.watertrail-01.v1'
    $typedElement.material.sourceProfile.dynamicParameterSemantics = @(
        'water_alpha_pan','water_noise_pan','water_dissolve',
        'water_noise_strength')
    $typedElement.material.sourceProfile.textures = @(
        [ordered]@{ name='maintex'; sourceObjectPath='fx.main'; assetId='Effect/Test/base.dds' },
        [ordered]@{ name='uv_noise_tex'; sourceObjectPath='fx.noise'; assetId='Effect/Test/noise.dds' })
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Typed WaterTrail publish failed.' }
    $typedWaterBaseline = [IO.File]::ReadAllBytes($output)
    $typedElement.material.sourceProfile.textures = @(
        $typedElement.material.sourceProfile.textures | Where-Object {
            $_.name -ne 'uv_noise_tex' })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Typed WaterTrail missing named noise texture' `
        $typedWaterBaseline

    # Exact UE3 material-family profiles are executable only when every
    # required named texture lane is closed. CustomParticle's unresolved
    # a_noise_01_tex lane is intentionally optional and is not synthesized.
    $ue3FamilyProfileFixtures = @(
        [ordered]@{
            profile = 'effect.ue3.slice.v1'
            roles = @('slice_flow_texture')
        },
        [ordered]@{
            profile = 'effect.ue3.glasshole-02.v1'
            roles = @('aura_texture','cracknormal_tex','in_hole_texture')
        },
        [ordered]@{
            profile = 'effect.ue3.fluidninja-01.v1'
            roles = @(
                'diff_tex','flow_1_tex','flow_2_tex','mask_tex','opacity_tex')
        },
        [ordered]@{
            profile = 'effect.ue3.customparticle-01.v1'
            roles = @('diff_tex')
        },
        [ordered]@{
            profile = 'effect.ue3.crackholev2-01.v1'
            roles = @(
                '01.map_e','06.map_f','06.map','mask_noisemap',
                'mask_tex_l','mask_tex_r')
        }
    )
    foreach ($familyFixture in $ue3FamilyProfileFixtures) {
        $familyElement = ($typedElement | ConvertTo-Json -Depth 100) |
            ConvertFrom-Json
        $familyElement.resources = @([ordered]@{
            slotId = 'base'
            assetId = 'Effect/Test/base.dds'
        })
        $familyElement.material.sourceProfile.runtimeShaderProfileId =
            [string]$familyFixture.profile
        $familyElement.material.sourceProfile.dynamicParameterSemantics = @(
            'unbound','unbound','unbound','unbound')
        $familyElement.material.sourceProfile.textures = @(
            foreach ($role in @($familyFixture.roles)) {
                [ordered]@{
                    name = [string]$role
                    sourceObjectPath = "fx.fixture.$role"
                    assetId = 'Effect/Test/base.dds'
                }
            })
        $document.elements = @($familyElement)
        Write-Fixture $document $catalog
        & $publisher -Mode Publish -DataRoot $dataRoot `
            -ResourceRoot $resourceRoot -OutputPath $output
        if ($LASTEXITCODE) {
            throw "UE3 family profile publish failed: $($familyFixture.profile)"
        }
        $familyBaseline = [IO.File]::ReadAllBytes($output)
        $missingRole = [string]@($familyFixture.roles)[-1]
        $familyElement.material.sourceProfile.textures = @(
            $familyElement.material.sourceProfile.textures | Where-Object {
                $_.name -ne $missingRole })
        Write-Fixture $document $catalog
        Assert-PublishRejected `
            "UE3 family profile missing named texture: $($familyFixture.profile)" `
            $familyBaseline
    }

    $document.elements = @($groupedElement)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE -or
        [Convert]::ToBase64String($groupedBaseline) -ne
        [Convert]::ToBase64String([IO.File]::ReadAllBytes($output))) {
        throw 'Grouped Source Material restore after typed profile fixtures failed.'
    }

    # G3 artist overrides are admissible metadata only when their effective
    # values still agree with compiler-declared targets. They never open an
    # authoring-approximate product gate.
    $document.version = 13
    $legacyCatalogEntry = $catalog.effects[0]
    $catalog.effects = @([ordered]@{
        effectAssetId = $effectId
        payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
        authoringPath = "Effects/Authored/$effectId.effect.json"
    })
    $groupedElement.material.sourceProfile.scalars = @([ordered]@{
        name = 'opacity_strength'
        group = ''
        value = 0.75
    })
    $groupedElement.material.sourceProfile.vectors = @([ordered]@{
        name = 'tint_color'
        group = ''
        value = @(0.25, 0.5, 0.75, 1.0)
    })
    $groupedElement.material.sourceProfile | Add-Member `
        -NotePropertyName sourceBlendClass -NotePropertyValue 'translucent' `
        -Force
    $groupedElement | Add-Member -NotePropertyName authoringOverrides `
        -NotePropertyValue ([ordered]@{
            resources = @([ordered]@{
                slotId = 'base'
                assetId = 'Effect/Test/base.dds'
                compilerAssetId = 'Effect/Test/dissolve.dds'
            })
            scalars = @([ordered]@{
                name = 'opacity_strength'
                value = 0.75
                compilerValue = 0.5
            })
            colors = @([ordered]@{
                name = 'tint_color'
                value = @(0.25, 0.5, 0.75, 1.0)
                compilerValue = @(1.0, 1.0, 1.0, 1.0)
            })
        }) -Force
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Valid G3 authoring overrides publish failed.' }
    $overrideBaseline = [IO.File]::ReadAllBytes($output)

    $directRuntime = Get-Content -LiteralPath $output -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $directRuntimeRow = @($directRuntime.effects | Where-Object {
        [string]$_.effectAssetId -ceq $effectId })
    if ($directRuntimeRow.Count -ne 1 -or
        (@($directRuntimeRow[0].PSObject.Properties.Name) -join '|') -cne
            ('payloadKind|effectAssetId|authoringFormatVersion|' +
                'authoredDocumentPath')) {
        throw 'Direct authored runtime row is not the minimal four-field contract.'
    }
    $directRuntime = $null

    $sourceProfileBackup = ($groupedElement.material.sourceProfile |
        ConvertTo-Json -Depth 30) | ConvertFrom-Json
    $executionPropertyBackup =
        $groupedElement.material.PSObject.Properties['execution']
    $executionBackup = if ($null -eq $executionPropertyBackup) {
        $null
    } else {
        ($executionPropertyBackup.Value | ConvertTo-Json -Depth 30) |
            ConvertFrom-Json
    }
    $templateBackup = [string]$groupedElement.material.templateId
    $groupedElement.material.templateId = 'effect.standard'
    $groupedElement.material.sourceProfile = [ordered]@{ enabled = $false }
    $groupedElement.material | Add-Member -NotePropertyName execution `
        -NotePropertyValue ([ordered]@{
            enabled = $true
            textureLanes = @()
            scalars = @([ordered]@{
                name = 'opacity_strength'
                packedIndex = 0
                value = 0.75
            })
            vectors = @()
            artistParameters = @()
            colors = @([ordered]@{
                name = 'tint_color'
                packedIndex = 0
                value = @(0.25, 0.5, 0.75, 1.0)
            })
        }) -Force
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) {
        throw 'Valid execution-only G3 authoring overrides publish failed.'
    }
    $groupedElement.material.templateId = $templateBackup
    $groupedElement.material.sourceProfile = $sourceProfileBackup
    if ($null -eq $executionPropertyBackup) {
        $groupedElement.material.PSObject.Properties.Remove('execution')
    } else {
        $groupedElement.material.execution = $executionBackup
    }
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Source-profile G3 override restore failed.' }
    $overrideBaseline = [IO.File]::ReadAllBytes($output)

    $groupedElement.authoringOverrides.resources[0].assetId = '../escape.dds'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unsafe G3 resource override' $overrideBaseline
    $groupedElement.authoringOverrides.resources[0].assetId =
        'Effect/Test/base.dds'

    $groupedElement.material.sourceProfile.sourceBlendClass = 'screen'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unknown G3 source blend class' $overrideBaseline
    $groupedElement.material.sourceProfile.sourceBlendClass = 'translucent'

    $groupedElement.authoringOverrides.resources[0].compilerAssetId =
        'Effect/Test/mesh.wmodel'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong-kind G3 resource override' $overrideBaseline
    $groupedElement.authoringOverrides.resources[0].compilerAssetId =
        'Effect/Test/dissolve.dds'

    $resourceOverrideCopy =
        ($groupedElement.authoringOverrides.resources[0] |
            ConvertTo-Json -Depth 10) | ConvertFrom-Json
    $groupedElement.authoringOverrides.resources = @(
        $groupedElement.authoringOverrides.resources[0],
        $resourceOverrideCopy)
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Duplicate G3 resource override' $overrideBaseline
    $groupedElement.authoringOverrides.resources = @(
        $groupedElement.authoringOverrides.resources[0])

    $groupedElement.authoringOverrides.scalars[0].name = 'missing_scalar'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unknown G3 scalar override' $overrideBaseline
    $groupedElement.authoringOverrides.scalars[0].name = 'opacity_strength'

    $groupedElement.authoringOverrides.scalars[0].value = 0.8
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Stale G3 scalar effective value' $overrideBaseline
    $groupedElement.authoringOverrides.scalars[0].value = 0.75

    $groupedElement.authoringOverrides.colors[0].name = 'opacity_strength'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong-type G3 color override' $overrideBaseline
    $groupedElement.authoringOverrides.colors[0].name = 'tint_color'

    $groupedElement.authoringOverrides.colors[0].compilerValue =
        @(0.25, 0.5, 0.75, 1.0)
    Write-Fixture $document $catalog
    Assert-PublishRejected 'No-op G3 color override' $overrideBaseline
    $groupedElement.authoringOverrides.colors[0].compilerValue =
        @(1.0, 1.0, 1.0, 1.0)

    $groupedElement.PSObject.Properties.Remove('authoringOverrides')
    $groupedElement.material.sourceProfile.scalars = @()
    $groupedElement.material.sourceProfile.vectors = @()
    $groupedElement.material.sourceProfile.PSObject.Properties.Remove(
        'sourceBlendClass')
    $document.version = 12
    $catalog.effects = @($legacyCatalogEntry)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Grouped Source Material G3 restore failed.' }
    $groupedBaseline = [IO.File]::ReadAllBytes($output)

    $originalExecutionProperty =
        $groupedElement.material.PSObject.Properties['execution']
    $originalExecution = if ($null -eq $originalExecutionProperty) {
        $null
    } else {
        ($originalExecutionProperty.Value | ConvertTo-Json -Depth 30) |
            ConvertFrom-Json
    }
    $groupedElement.material | Add-Member -NotePropertyName execution `
        -NotePropertyValue ([ordered]@{
            enabled = $false
            failClosed = $true
            authoringApproximate = $true
        }) -Force
    Write-Fixture $document $catalog
    Assert-PublishRejected `
        'Authoring-approximate document transaction' $groupedBaseline
    if ($null -eq $originalExecutionProperty) {
        $groupedElement.material.PSObject.Properties.Remove('execution')
    } else {
        $groupedElement.material.execution = $originalExecution
    }

	$ordinaryEffectId = $effectId
	$ordinaryDocumentEffectId = [string]$document.effectAssetId
	$ordinaryCatalogEffects = @($catalog.effects)
	$ordinarySourceMaterialPath =
		[string]$groupedElement.material.sourceMaterialPath
	$ordinaryVersion = [int]$document.version
	$effectId = 'effect.warlord.skill.17090.unified'
	$document.effectAssetId = $effectId
	$document.version = 13
	$catalog.effects = @([ordered]@{
		effectAssetId = $effectId
		payloadKind = 'DIRECT_AUTHORED_DOCUMENT_V13'
		authoringPath = "Effects/Authored/$effectId.effect.json"
	})
	$groupedElement.material.sourceMaterialPath =
		'fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma'
	Write-Fixture $document $catalog
	Assert-PublishRejected `
		'Warlord chain grouped Full promotion' $groupedBaseline
	$effectId = $ordinaryEffectId
	$document.effectAssetId = $ordinaryDocumentEffectId
	$document.version = $ordinaryVersion
	$catalog.effects = $ordinaryCatalogEffects
	$groupedElement.material.sourceMaterialPath = $ordinarySourceMaterialPath

    $groupedElement.resources = @([ordered]@{
        slotId = 'noise'
        assetId = 'Effect/Test/base.dds'
    })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Grouped profile without an alpha/color carrier' `
        $groupedBaseline

    $groupedElement.resources = @([ordered]@{
        slotId = 'mask'
        assetId = 'Effect/Test/base.dds'
    })
    $groupedElement.material.sourceProfile.scalars = @([ordered]@{
        name = 'emission_strength'
        group = 'emission'
        value = 1.0
    })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Grouped emission profile without emission carrier' `
        $groupedBaseline

    $groupedElement.resources = @([ordered]@{
        slotId = 'emissive'
        assetId = 'Effect/Test/base.dds'
    })
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Grouped emission carrier publish failed.' }
    $groupedElement.material.sourceProfile.scalars = @()

    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.fallback-blocked.v1'
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Fail-closed Source Material publish failed.' }

    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.one-layer-distortion.v1'
    $groupedElement.resources = @([ordered]@{
        slotId = 'noise'
        assetId = 'Effect/Test/base.dds'
    })
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Noise-only finite profile publish failed.' }

    $groupedElement.resources = @()
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.circle.v1'
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Procedural finite profile publish failed.' }

    $groupedElement.resources = @([ordered]@{
        slotId = 'meshModel'
        assetId = 'Effect/Test/mesh.wmodel'
    })
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.local-crack.v1'
    $groupedElement.material.sourceProfile | Add-Member `
        -NotePropertyName textures -NotePropertyValue @() -Force
    $groupedElement.material.sourceProfile.textures = @(
        [ordered]@{
            name = 'normal_tex'
            sourceObjectPath = 'fx_tex_06.fx_j_normal_bc5_09'
            assetId = 'Effect/Test/normal.dds'
            addressU = 'wrap'
            addressV = 'wrap'
            colorSpace = 'linear'
            samplingEvidence = 'ue3_property_or_class_default.v1'
        },
        [ordered]@{
            name = 'refle_tex'
            sourceObjectPath = 'fx_tex_00.fx_b_atypical_004_cube'
            assetId = 'Effect/Test/reflection.dds'
            addressU = 'wrap'
            addressV = 'wrap'
            colorSpace = 'srgb'
            samplingEvidence = 'ue3_property_or_class_default.v1'
        },
        [ordered]@{
            name = 'dissolve_tex'
            sourceObjectPath = 'fx_tex_04.fx_h_atypical_01_1'
            assetId = 'Effect/Test/dissolve.dds'
            addressU = 'wrap'
            addressV = 'wrap'
            colorSpace = 'srgb'
            samplingEvidence = 'ue3_property_or_class_default.v1'
        }
    )
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'LocalCrack named texture publish failed.' }
    $localCrackBaseline = [IO.File]::ReadAllBytes($output)

    $localCrackReflection = $groupedElement.material.sourceProfile.textures[1]
    $groupedElement.material.sourceProfile.textures = @(
        $groupedElement.material.sourceProfile.textures |
            Where-Object { $_.name -ne 'refle_tex' })
    Write-Fixture $document $catalog
    Assert-PublishRejected 'LocalCrack without named reflection texture' `
        $localCrackBaseline
    $groupedElement.material.sourceProfile.textures = @(
        $groupedElement.material.sourceProfile.textures[0],
        $localCrackReflection,
        $groupedElement.material.sourceProfile.textures[1]
    )
    $localCrackReflection.samplingEvidence = 'legacy_default'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'LocalCrack with legacy sampling fallback' `
        $localCrackBaseline
    $localCrackReflection.samplingEvidence =
        'ue3_property_or_class_default.v1'
    $groupedElement.material.sourceProfile.textures = @()

    $finiteBaseline = [IO.File]::ReadAllBytes($output)
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.reconstructed-standard.v1'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Reconstructed standard profile without Base' `
        $finiteBaseline

    $groupedElement.resources = @([ordered]@{
        slotId = 'base'
        assetId = 'Effect/Test/base.dds'
    })
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.grouped-translucent.v1'
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Grouped Source Material restore failed.' }
    $groupedBaseline = [IO.File]::ReadAllBytes($output)

    $groupedElement.resources[0].assetId = 'Effect/Test/blankwhite.dds'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Grouped profile with unsafe blank-white Base only' `
        $groupedBaseline
    $groupedElement.resources[0].assetId = 'Effect/Test/base.dds'

    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.unregistered.v1'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unregistered Source Material shader profile' `
        $groupedBaseline
    $groupedElement.material.sourceProfile.runtimeShaderProfileId =
        'effect.ue3.grouped-translucent.v1'

    $document.version = 8
    $document.elements = @($version8Element)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v8 baseline restore failed.' }
    $modelCueBaseline = [IO.File]::ReadAllBytes($output)

    $duplicateCue = ($document.modelCues[0] | ConvertTo-Json -Depth 10) |
        ConvertFrom-Json
    $document.modelCues = @($document.modelCues[0], $duplicateCue)
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Duplicate Model Cue ID' $modelCueBaseline
    $document.modelCues = @($document.modelCues[0])
    $document.modelCues[0].modelAssetId = '../escape.wmodel'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unsafe Model Cue resource path' $modelCueBaseline
    $document.modelCues[0].modelAssetId = 'Character/Test/cue.wmodel'
    $document.modelCues[0].durationSeconds = 0.0
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Invalid Model Cue duration' $modelCueBaseline
    $document.version = 6
    $document.Remove('modelCues')
    $document.Remove('particleSystem')
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'v6 compatibility restore failed.' }
    $baseline = [IO.File]::ReadAllBytes($output)

    $originalElement = ($document.elements[0] | ConvertTo-Json -Depth 30) |
        ConvertFrom-Json
    $meshParticle = ($originalElement | ConvertTo-Json -Depth 30) |
        ConvertFrom-Json
    $meshParticle.kind = 'particle'
    $meshParticle.resources = @([ordered]@{
        slotId = 'meshModel'
        assetId = 'Effect/Test/mesh.wmodel'
    })
    $meshParticle.detail.mesh.useModelMaterial = $true
    $document.elements = @($meshParticle)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Mesh-backed Particle publish failed.' }
    $meshParticleBaseline = [IO.File]::ReadAllBytes($output)

    $meshParticle.detail.mesh.useModelMaterial = $false
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Mesh particle without Base override' `
        $meshParticleBaseline

    $document.elements = @($originalElement)
    Write-Fixture $document $catalog
    & $publisher -Mode Publish -DataRoot $dataRoot `
        -ResourceRoot $resourceRoot -OutputPath $output
    if ($LASTEXITCODE) { throw 'Baseline restore publish failed.' }
    $baseline = [IO.File]::ReadAllBytes($output)

    $componentPath = Join-Path $components `
        'Fixture_00.particlesystem.wfx.json'
    $mismatchedComponent = Get-Content -LiteralPath $componentPath `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    $mismatchedComponent.document.elements[0].detail.transform.position[0] = 1.0
    Write-Utf8 $componentPath `
        (($mismatchedComponent | ConvertTo-Json -Depth 100) + "`n")
    Assert-ValidateRejected `
        'Same-count Component payload mismatch' $baseline
    Assert-PublishRejected `
        'Same-count Component payload mismatch' $baseline
    Write-Fixture $document $catalog

    $catalog.formatVersion = 2
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong catalog version' $baseline
    $catalog.formatVersion = 1

    $document.version = 4
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Wrong document version' $baseline
    $document.version = 6

    $document.elements[0].material.templateId = 'effect.unproven.custom'
    Write-Fixture $document $catalog
    Assert-PublishRejected 'Unknown Material Template' $baseline
    $document.elements[0].material.templateId = 'effect.standard'

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
    Write-Fixture $document $catalog
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
    Write-Fixture $document $catalog
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
    Write-Fixture $document $catalog
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
    Write-Host 'PASS: Effect pipeline v12 grouped-source/v8 particle-system/v7 model-cue/v6 compatibility/mesh-particle publish, tracked-text LF/CRLF parity with BOM/semantic-mutation rejection, full Assembly identity, shader-profile/version/path/kind/duplicate/resource/hash/budget/binding/promote rejection, and rollback.'
}
finally {
    if (Test-Path -LiteralPath $fixture) {
        Remove-Item -LiteralPath $fixture -Recurse -Force
    }
}

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$pythonContractTests = @(
    'Tools.EffectPipeline.test_build_valtan_stage_effects',
    'Tools.EffectPipeline.test_build_valtan_action_bindings',
    'Tools.EffectPipeline.test_build_valtan_source_occurrence_inventory',
    'Tools.EffectPipeline.test_build_valtan_portal_rush_imported_canary',
    'Tools.EffectPipeline.test_build_valtan_source_timing_delta_proposals',
    'Tools.EffectPipeline.test_migrate_valtan_pattern_occurrences_v2'
)
Push-Location $repositoryRoot
try {
    & python -m unittest @pythonContractTests
    if ($LASTEXITCODE) {
        throw 'EffectPipeline Python contract tests failed.'
    }
}
finally {
    Pop-Location
}
