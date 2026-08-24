$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$projector = Join-Path $PSScriptRoot 'Project-ValtanPatternMaster.ps1'
$formatPreservingHelper = Join-Path $PSScriptRoot 'Format-PreservingJsonArray.ps1'
. $formatPreservingHelper
$fixtureRoot = Join-Path ([IO.Path]::GetTempPath()) `
    ('LostArk-ValtanPatternMaster-' + [Guid]::NewGuid().ToString('N'))
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$relativeFiles = @(
    'Data/Valtan/Valtan.pattern.json',
    'Data/Encounters/Valtan/ValtanEncounter.json',
    'Data/Animation/Authored/Valtan/Valtan.patternbindings.json',
    'Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json',
    'Data/Encounters/Valtan/ValtanCombatObjects.json',
    'Data/Actors/BossCatalog.json',
    'Data/Effects/EffectCatalog.json',
    'Data/Balance/DamageProfiles.json',
    'Data/Encounters/Valtan/ValtanCinematicCamera.json',
    'Data/Encounters/Valtan/ValtanWorldEvents.json',
    'Data/Encounters/Valtan/ValtanPatternRotations.json',
    'Data/Animation/Reference/Valtan/Valtan.clipseq'
)

function Copy-FixtureFile([string]$RelativePath) {
    $source = Join-Path $repositoryRoot $RelativePath
    $destination = Join-Path $fixtureRoot $RelativePath
    [IO.Directory]::CreateDirectory((Split-Path -Parent $destination)) | Out-Null
    [IO.File]::Copy($source, $destination, $false)
}

function Read-Json([string]$Path) {
    return Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Write-Json([string]$Path, [object]$Document) {
    [IO.File]::WriteAllText(
        $Path,
        (($Document | ConvertTo-Json -Depth 100) + "`n"),
        $utf8NoBom)
}

function Invoke-ExpectedFailure([scriptblock]$Action, [string]$ExpectedText) {
    $failed = $false
    try {
        & $Action
    }
    catch {
        $failed = $true
        if ($_.Exception.Message -notlike "*$ExpectedText*") {
            throw "Unexpected failure. expected='$ExpectedText' actual='$($_.Exception.Message)'"
        }
    }
    if (-not $failed) { throw "Expected failure did not occur: $ExpectedText" }
}

function Get-ArrayByteSnapshot(
    [string]$Path,
    [string]$ArrayProperty,
    [string]$KeyProperty,
    [string]$Key) {
    $text = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    $layout = Get-FormatPreservingJsonArrayLayout $text $ArrayProperty $KeyProperty
    $rows = @($layout.Elements | Where-Object { [string]$_.Key -ceq $Key })
    if ($rows.Count -ne 1) { throw "Sentinel row is missing: $ArrayProperty/$Key" }
    return [pscustomobject]@{
        Prefix = $text.Substring(0, $layout.OpenOffset + 1)
        Row = [string]$rows[0].Raw
        Suffix = $text.Substring($layout.CloseOffset)
    }
}

try {
    $rootScannerText =
        '{"note":"\"patterns\": [","nested":{"patterns":[]},"patterns":[{"patternId":"A","value":1}]}'
    $rootScannerDocument = $rootScannerText | ConvertFrom-Json
    $rootScannerResult = Update-FormatPreservingJsonArrayRows `
        $rootScannerText 'patterns' 'patternId' @($rootScannerDocument.patterns)
    if ($rootScannerResult -cne $rootScannerText) {
        throw 'Root array scanner changed a semantic no-op document.'
    }
    $caseSensitiveText = '{"patterns":[{"patternId":"A","value":1}]}'
    $caseSensitiveDesired = '{"patternId":"a","value":2}' | ConvertFrom-Json
    $caseSensitiveResult = Update-FormatPreservingJsonArrayRows `
        $caseSensitiveText 'patterns' 'patternId' @($caseSensitiveDesired)
    $caseSensitiveDocument = $caseSensitiveResult | ConvertFrom-Json
    if (@($caseSensitiveDocument.patterns).Count -ne 2 -or
        (@($caseSensitiveDocument.patterns.patternId) -join ',') -cne 'A,a') {
        throw 'Formatting-preserving row keys are not ordinal case-sensitive.'
    }
    Invoke-ExpectedFailure {
        $null = Get-FormatPreservingJsonArrayLayout `
            '{"patterns":[{"patternId":1}]}' 'patterns' 'patternId'
    } "row is missing 'patternId'"
    Invoke-ExpectedFailure {
        $null = Get-FormatPreservingJsonArrayLayout `
            '{"patterns":[{"PatternId":"A"}]}' 'patterns' 'patternId'
    } "missing exact property 'patternId'"

    [IO.Directory]::CreateDirectory($fixtureRoot) | Out-Null
    foreach ($relativeFile in $relativeFiles) { Copy-FixtureFile $relativeFile }
    $masterPath = Join-Path $fixtureRoot 'Data\Valtan\Valtan.pattern.json'
    $receiptPath = Join-Path $fixtureRoot 'Audit\Valtan.pattern.projection.json'
    $combatObjectPath = Join-Path $fixtureRoot `
        'Data\Encounters\Valtan\ValtanCombatObjects.json'
    $rotationPath = Join-Path $fixtureRoot `
        'Data\Encounters\Valtan\ValtanPatternRotations.json'

    $staleCombatObjects = Read-Json $combatObjectPath
    (@($staleCombatObjects.objects | Where-Object {
        $_.combatObjectArchetypeId -eq 'combatobject.valtan.high-jump.target-axe'
    }))[0].lifeMs = 5999
    Write-Json $combatObjectPath $staleCombatObjects

    $staleRotations = Read-Json $rotationPath
    (@($staleRotations.rotations | Where-Object {
        $_.rotationId -eq 'rotation.valtan.160.130'
    }))[0].selectionMode = 'ORDERED_INTRO_THEN_WEIGHTED'
    Write-Json $rotationPath $staleRotations

    $bindingPath = Join-Path $fixtureRoot `
        'Data\Animation\Authored\Valtan\Valtan.patternbindings.json'
    $bindingText = [IO.File]::ReadAllText($bindingPath, [Text.Encoding]::UTF8)
    $sentinelPattern = [regex]::new(
        '"actionId"\s*:\s*"valtan\.attack\.swing\.windup"')
    if ($sentinelPattern.Matches($bindingText).Count -ne 1) {
        throw 'Binding sentinel setup did not resolve exactly one unmanaged row.'
    }
    $bindingText = $sentinelPattern.Replace(
        $bindingText,
        '"actionId"  :   "valtan.attack.swing.windup"',
        1)
    [IO.File]::WriteAllText($bindingPath, $bindingText, $utf8NoBom)

    $sentinelSpecs = @(
        @('Data\Encounters\Valtan\ValtanEncounter.json',
            'patterns','patternId','VALTAN_SWING'),
        @('Data\Animation\Authored\Valtan\Valtan.patternbindings.json',
            'bindings','actionId','valtan.attack.swing.windup'),
        @('Data\Animation\Authored\Valtan\Valtan.patterneffectcues.json',
            'cues','bindingId','cue.valtan.carrier-v1.attack.backstep.windup.clip-01'),
        @('Data\Encounters\Valtan\ValtanCombatObjects.json',
            'objects','combatObjectArchetypeId',
            'combatobject.valtan.red-blade-wave.projectile'),
        @('Data\Encounters\Valtan\ValtanPatternRotations.json',
            'rotations','rotationId','rotation.valtan.109.100')
    )
    $sentinelSnapshots = @{}
    foreach ($sentinelSpec in $sentinelSpecs) {
        $sentinelPath = Join-Path $fixtureRoot $sentinelSpec[0]
        $sentinelSnapshots[$sentinelSpec[0]] = Get-ArrayByteSnapshot `
            $sentinelPath $sentinelSpec[1] $sentinelSpec[2] $sentinelSpec[3]
    }

    & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
        -MasterPath $masterPath -SkipProductDriftCheck
    & $projector -Mode Publish -RepositoryRoot $fixtureRoot `
        -MasterPath $masterPath -ReceiptPath $receiptPath
    & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
        -MasterPath $masterPath

    foreach ($sentinelSpec in $sentinelSpecs) {
        $sentinelPath = Join-Path $fixtureRoot $sentinelSpec[0]
        $before = $sentinelSnapshots[$sentinelSpec[0]]
        $after = Get-ArrayByteSnapshot $sentinelPath $sentinelSpec[1] `
            $sentinelSpec[2] $sentinelSpec[3]
        if ([string]$before.Prefix -cne [string]$after.Prefix -or
            [string]$before.Row -cne [string]$after.Row -or
            [string]$before.Suffix -cne [string]$after.Suffix) {
            throw "Formatting-preserving Publish changed unrelated bytes: $($sentinelSpec[0])"
        }
    }

    $master = Read-Json $masterPath
    $encounter = Read-Json (Join-Path $fixtureRoot `
        'Data\Encounters\Valtan\ValtanEncounter.json')
    $bindings = Read-Json (Join-Path $fixtureRoot `
        'Data\Animation\Authored\Valtan\Valtan.patternbindings.json')
    $cues = Read-Json (Join-Path $fixtureRoot `
        'Data\Animation\Authored\Valtan\Valtan.patterneffectcues.json')
    $combatObjects = Read-Json $combatObjectPath
    $rotations = Read-Json $rotationPath
    $receipt = Read-Json $receiptPath

    if (@($master.patterns).Count -ne 7 -or
        @($master.independentEffects).Count -ne 2 -or
        @($master.counterReactionLayers).Count -ne 4) {
        throw 'Master Phase-1 inventory count is invalid.'
    }
    $weightedRotationIds = @($master.normalSelection.ranges.rotationId)
    $weightedRotations = @($rotations.rotations | Where-Object {
        $_.rotationId -cin $weightedRotationIds
    })
    if ($weightedRotations.Count -ne 2 -or
        @($weightedRotations | Where-Object {
            [string]$_.selectionMode -cne 'WEIGHTED_POOL' -or
            (@($_.patternIds) -join ',') -cne
                'VALTAN_WHIRLWIND,VALTAN_DASH_CHARGE,VALTAN_FOUR_SLASH,VALTAN_FIST_IN_OUT,VALTAN_HIGH_JUMP'
        }).Count -ne 0) {
        throw 'Phase-1 weighted normal-selection projection is invalid.'
    }
    if ((@($master.counterReactionLayers.ownerPatternId | Sort-Object -Unique) -join ',') -cne
        'VALTAN_CENTER_GRAB_COUNTER_64,VALTAN_TRIPLE_COUNTER') {
        throw 'Counter reaction layer does not reference the exact legacy Product owners.'
    }
    if (($master | ConvertTo-Json -Depth 100 -Compress) -match '"loop"\s*:') {
        throw 'Master must not overload the product loop boolean.'
    }
    $allowedEndPolicies = @('EXACT','HOLD_LAST_POSE','LOOP_TO_STAGE_END')
    $masterEndPolicies = @($master.patterns.stages.animation.endPolicy)
    if ($masterEndPolicies.Count -ne 31 -or
        @($masterEndPolicies | Where-Object {
            $_ -cnotin $allowedEndPolicies
        }).Count -ne 0) {
        throw 'Every master stage must declare an animation endPolicy.'
    }
    $allowedMappingBases = @(
        'CURRENT_PRODUCT_BASELINE','PATTERN_PR_REFERENCE','ANIMATION_PR_127',
        'SOURCE_REVIEWED_DELTA','PROJECT_AUTHORED','LEGACY_V1_MIGRATION'
    )
    $masterMappingBases = @(
        $master.patterns.stages.animation.occurrences.mappingBasis
        $master.patterns.stages.effectRefs.cueProjection.mappingBasis
        $master.independentEffects.cueProjection.mappingBasis
    ) | Where-Object { $null -ne $_ } | Sort-Object -Unique
    if (@($masterMappingBases | Where-Object { $_ -cnotin $allowedMappingBases }).Count -ne 0) {
        throw 'Master mappingBasis is outside the public vocabulary.'
    }
    $dash = @($master.patterns | Where-Object patternId -eq 'VALTAN_DASH_CHARGE')[0]
    $dashWindup = @($dash.stages | Where-Object stageId -eq 'WINDUP')[0]
    $dashCharge = @($dash.stages | Where-Object stageId -eq 'CHARGE')[0]
    $dashGroggy = @($dash.stages | Where-Object stageId -eq 'GROGGY')[0]
    if ([int]$dashWindup.animation.repeatCount -ne 3 -or
        [string]$dashWindup.animation.endPolicy -cne 'EXACT' -or
        @($dashWindup.animation.occurrences).Count -ne 3 -or
        @($dashCharge.branches).Count -ne 2 -or
        @($dashGroggy.actions).Count -ne 2 -or
        @($dashGroggy.branches).Count -ne 2) {
        throw 'Dash repeat/branch/groggy master contract is invalid.'
    }
    $publishedDashBinding = @($bindings.bindings | Where-Object {
        $_.actionId -eq 'valtan.attack.dash-charge.windup'
    })[0]
    if (@($publishedDashBinding.clips).Count -ne 3 -or
        @($publishedDashBinding.clips | Where-Object { [bool]$_.loop }).Count -ne 0) {
        throw 'Dash repeatCount was not projected to three explicit product occurrences.'
    }
    if (@($encounter.patterns | Where-Object {
        $_.patternId -in @('VALTAN_TRIPLE_SLASH','VALTAN_ROTATION_SLASH')
    }).Count -ne 0 -or
        @($encounter.patterns | Where-Object patternId -eq 'VALTAN_FOUR_SLASH').Count -ne 1) {
        throw 'Four-slash consolidation projection is invalid.'
    }
    $fourSlash = @($encounter.patterns | Where-Object patternId -eq 'VALTAN_FOUR_SLASH')[0]
    $slashContacts = @($fourSlash.stages | Where-Object stageId -eq 'SLASHES')[0]
    $spinContact = @($fourSlash.stages | Where-Object stageId -eq 'SPIN')[0]
    if ((@($slashContacts.hitOffsetsMs) -join ',') -cne '1790,2560,3330' -or
        (@($spinContact.hitOffsetsMs) -join ',') -cne '600' -or
        ([int]$slashContacts.hitCount + [int]$spinContact.hitCount) -ne 4) {
        throw 'Four-slash contact timing projection is invalid.'
    }
    $masterFourSlash = @($master.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_FOUR_SLASH'
    })[0]
    if ([string](@($masterFourSlash.stages | Where-Object {
            $_.stageId -eq 'SLASHES'
        })[0].animation.endPolicy) -cne 'HOLD_LAST_POSE') {
        throw 'Four-slash natural clip hold policy is not explicit.'
    }
    $whirlwind = @($master.patterns | Where-Object patternId -eq 'VALTAN_WHIRLWIND')[0]
    $whirlwindSpin = @($whirlwind.stages | Where-Object stageId -eq 'SPIN')[0]
    $whirlwindContacts = @()
    for ($contactIndex = 0; $contactIndex -lt [int]$whirlwindSpin.hitCount; ++$contactIndex) {
        $whirlwindContacts += [int]$whirlwindSpin.hitDelayMs +
            ($contactIndex * [int]$whirlwindSpin.hitIntervalMs)
    }
    if ([math]::Abs([double]$whirlwindSpin.animation.occurrences[0].playRate -
            0.888888889) -gt 0.000000001 -or
        [string]$whirlwindSpin.animation.endPolicy -cne 'LOOP_TO_STAGE_END' -or
        ($whirlwindContacts -join ',') -cne '0,350,700,1050') {
        throw 'Whirlwind two-cycle presentation rate is invalid.'
    }
    $floorWipe = @($master.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_FLOOR_WIPE_130'
    })[0]
    if ((@($floorWipe.stages.sequenceRole) -join ',') -cne
            'WINDUP,SECOND_SMASH,WINDUP,SECOND_SMASH,RECOVERY' -or
        @($floorWipe.stages.animation.occurrences | Where-Object {
            [double]$_.playRate -ne 1.0 -or -not [bool]$_.repeatUntilStageEnd
        }).Count -ne 0) {
        throw 'Floor-wipe intended sequence roles or baseline clip policy is invalid.'
    }
    $highJump = @($master.patterns | Where-Object patternId -eq 'VALTAN_HIGH_JUMP')[0]
    $arenaBreak = @($master.patterns | Where-Object patternId -eq 'VALTAN_ARENA_BREAK_109')[0]
    if ([string]$highJump.serverMotion.travelStageId -cne 'LAND' -or
        [string]$arenaBreak.serverMotion.travelStageId -cne 'DROP') {
        throw 'Leap travelStageId master contract is invalid.'
    }
    $fist = @($master.patterns | Where-Object patternId -eq 'VALTAN_FIST_IN_OUT')[0]
    $fistOuter = @($fist.stages | Where-Object stageId -eq 'OUTER')[0]
    $fistRecovery = @($fist.stages | Where-Object stageId -eq 'RECOVERY')[0]
    $highRecovery = @($highJump.stages | Where-Object stageId -eq 'RECOVERY')[0]
    if ([string]$fistOuter.animation.occurrences[0].clip -cne 'mesh_att_battle_1_01' -or
        [int]$fistOuter.animation.occurrences[0].sourceStartMs -ne 0 -or
        [int]$fistOuter.animation.occurrences[0].playMs -ne 800 -or
        [int]$fistRecovery.animation.occurrences[0].sourceStartMs -ne 800 -or
        [int]$fistRecovery.animation.occurrences[0].playMs -ne 800 -or
        [string]$highRecovery.animation.occurrences[0].clip -cne
            'mesh_att_battle_8_01_loop' -or
        [string]$highRecovery.animation.occurrences[0].mappingBasis -cne
            'PROJECT_AUTHORED') {
        throw 'Composite source sequence tail projection is invalid.'
    }
    $arenaTakeoff = @($arenaBreak.stages | Where-Object stageId -eq 'TAKEOFF')[0]
    $arenaDrop = @($arenaBreak.stages | Where-Object stageId -eq 'DROP')[0]
    $arenaImpact = @($arenaBreak.stages | Where-Object stageId -eq 'IMPACT')[0]
    $arenaImpactHold = @($arenaBreak.stages | Where-Object stageId -eq 'IMPACT_HOLD')[0]
    $arenaWide = @($arenaBreak.stages | Where-Object stageId -eq 'WIDE_REVEAL')[0]
    $arenaRecovery = @($arenaBreak.stages | Where-Object stageId -eq 'RECOVERY')[0]
    if (@($arenaBreak.presentationSources).Count -ne 2 -or
        (@($arenaBreak.presentationSources.sourceActionId) -join ',') -cne '420629,420616' -or
        @($arenaDrop.animation.occurrences).Count -ne 2 -or
        @($arenaImpactHold.animation.occurrences).Count -ne 2 -or
        ([int]$arenaTakeoff.animation.occurrences[0].sourceStartMs -ne 0) -or
        ([int]$arenaTakeoff.animation.occurrences[0].playMs -ne 900) -or
        (@($arenaDrop.animation.occurrences.sourceStartMs) -join ',') -cne '900,0' -or
        (@($arenaDrop.animation.occurrences.playMs) -join ',') -cne '300,400' -or
        ([string]$arenaImpact.animation.occurrences[0].clip -cne 'mesh_att_battle_12_02') -or
        ([int]$arenaImpact.animation.occurrences[0].sourceStartMs -ne 400) -or
        (@($arenaImpactHold.animation.occurrences.sourceStartMs) -join ',') -cne '800,0' -or
        (@($arenaImpactHold.animation.occurrences.playMs) -join ',') -cne '200,900' -or
        ([int]$arenaWide.animation.occurrences[0].sourceStartMs -ne 900) -or
        ([int]$arenaWide.animation.occurrences[0].playMs -ne 300) -or
        ([string]$arenaWide.animation.endPolicy -cne 'HOLD_LAST_POSE') -or
        ([int]$arenaRecovery.animation.occurrences[0].playMs -ne 0) -or
        ([double]$arenaRecovery.animation.occurrences[0].playRate -ne 1.0) -or
        (-not [bool]$arenaRecovery.animation.occurrences[0].repeatUntilStageEnd)) {
        throw 'Arena-break continuous presentation window contract is invalid.'
    }
    $publishedDrop = @($bindings.bindings | Where-Object {
        $_.actionId -eq 'valtan.mechanic.arena-break-109.drop'
    })[0]
    $publishedImpactHold = @($bindings.bindings | Where-Object {
        $_.actionId -eq 'valtan.mechanic.arena-break-109.impact-hold'
    })[0]
    if (@($publishedDrop.clips).Count -ne 2 -or
        @($publishedImpactHold.clips).Count -ne 2) {
        throw 'Arena-break ordered source windows were not projected to product bindings.'
    }
    $roarCue = @($cues.cues | Where-Object {
        $_.bindingId -eq
            'cue.valtan.carrier-v1.mechanic.arena-break-109.roar-recovery.clip-01'
    })
    $impactCue = @($cues.cues | Where-Object {
        $_.bindingId -eq
            'cue.valtan.carrier-v1.mechanic.arena-break-109.impact.clip-01'
    })
    if ($roarCue.Count -ne 1 -or [string]$roarCue[0].stageId -cne 'RECOVERY' -or
        $impactCue.Count -ne 1 -or [int]$impactCue[0].sourceStartMs -ne 400 -or
        [string]$impactCue[0].clipOccurrenceId -cne
            'valtan.mechanic.arena-break-109.impact.clip.01') {
        throw 'Arena-break roar recovery cue projection is invalid.'
    }
    $managedStageCueRefs = @($master.patterns.stages.effectRefs | Where-Object {
        $_.refType -eq 'CUE_BINDING'
    })
    $donutIndependent = @($master.independentEffects | Where-Object {
        $_.independentEffectId -eq 'valtan.independent-effect.donut-in-out'
    })[0]
    $donutReferences = @(
        foreach ($pattern in @($master.patterns)) {
            foreach ($stage in @($pattern.stages)) {
                foreach ($effectRef in @($stage.effectRefs)) {
                    if ($effectRef.refType -eq 'INDEPENDENT_EFFECT' -and
                        $effectRef.refId -eq $donutIndependent.independentEffectId) {
                        [pscustomobject]@{
                            patternId = [string]$pattern.patternId
                            stageId = [string]$stage.stageId
                        }
                    }
                }
            }
        }
    )
    if ($managedStageCueRefs.Count -ne 14 -or
        @($managedStageCueRefs | Where-Object { $null -eq $_.cueProjection }).Count -ne 0 -or
        $null -eq $donutIndependent.cueProjection -or
        $donutReferences.Count -ne 1 -or
        [string]$donutReferences[0].patternId -cne [string]$donutIndependent.ownerPatternId -or
        [string]$donutReferences[0].stageId -cne [string]$donutIndependent.ownerStageId) {
        throw 'Managed effect cue timing is not fully owned by the master.'
    }
    $axeObject = @($combatObjects.objects | Where-Object {
        $_.combatObjectArchetypeId -eq 'combatobject.valtan.high-jump.target-axe'
    })[0]
    $airborne = @($highJump.stages | Where-Object stageId -eq 'AIRBORNE')[0]
    if ([int]$axeObject.lifeMs -ne [int]$airborne.durationMs) {
        throw 'Axe combat-object lifeMs was not projected from AIRBORNE durationMs.'
    }
    if ([string]$receipt.runtimeAuthority -cne 'EXISTING_PRODUCT_DOCUMENTS') {
        throw 'Projection receipt incorrectly declares a second runtime authority.'
    }

    $baselineMasterText = [IO.File]::ReadAllText($masterPath, [Text.Encoding]::UTF8)
    $invalidRepeat = $baselineMasterText | ConvertFrom-Json
    (@($invalidRepeat.patterns | Where-Object patternId -eq 'VALTAN_DASH_CHARGE')[0].stages |
        Where-Object stageId -eq 'WINDUP')[0].animation.repeatCount = 2
    Write-Json $masterPath $invalidRepeat
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'finite repeatCount must equal the number of explicit same-clip occurrences'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidIndependentOwner = $baselineMasterText | ConvertFrom-Json
    $invalidDonut = @($invalidIndependentOwner.independentEffects | Where-Object {
        $_.independentEffectId -eq 'valtan.independent-effect.donut-in-out'
    })[0]
    $invalidDonutPattern = @($invalidIndependentOwner.patterns | Where-Object {
        $_.patternId -eq $invalidDonut.ownerPatternId
    })[0]
    $invalidOuter = @($invalidDonutPattern.stages | Where-Object {
        $_.stageId -eq 'OUTER'
    })[0]
    $invalidOuter.effectRefs = @($invalidOuter.effectRefs) + [pscustomobject]@{
        refType = 'INDEPENDENT_EFFECT'
        refId = [string]$invalidDonut.independentEffectId
    }
    Write-Json $masterPath $invalidIndependentOwner
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'must have exactly one reference on its declared owner stage'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidCrossPatternOwner = $baselineMasterText | ConvertFrom-Json
    $crossPatternDonut = @($invalidCrossPatternOwner.independentEffects | Where-Object {
        $_.independentEffectId -eq 'valtan.independent-effect.donut-in-out'
    })[0]
    $crossPatternWindup = @(@($invalidCrossPatternOwner.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_WHIRLWIND'
    })[0].stages | Where-Object { $_.stageId -eq 'WINDUP' })[0]
    $crossPatternWindup.effectRefs = @($crossPatternWindup.effectRefs) + [pscustomobject]@{
        refType = 'INDEPENDENT_EFFECT'
        refId = [string]$crossPatternDonut.independentEffectId
    }
    Write-Json $masterPath $invalidCrossPatternOwner
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'must have exactly one reference on its declared owner stage'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidExactBudget = $baselineMasterText | ConvertFrom-Json
    (@($invalidExactBudget.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_DASH_CHARGE'
    })[0].stages | Where-Object stageId -eq 'WINDUP')[0].animation.occurrences[0].playMs = 590
    Write-Json $masterPath $invalidExactBudget
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'EXACT animation must fill its Server stage within 2ms'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidLoopPolicy = $baselineMasterText | ConvertFrom-Json
    (@($invalidLoopPolicy.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_WHIRLWIND'
    })[0].stages | Where-Object stageId -eq 'SPIN')[0].animation.endPolicy = 'EXACT'
    Write-Json $masterPath $invalidLoopPolicy
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'EXACT animation must fill its Server stage within 2ms'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidSource = $baselineMasterText | ConvertFrom-Json
    (@($invalidSource.patterns | Where-Object {
        $_.patternId -eq 'VALTAN_ARENA_BREAK_109'
    }))[0].presentationSources = @(
        (@($invalidSource.patterns | Where-Object {
            $_.patternId -eq 'VALTAN_ARENA_BREAK_109'
        }))[0].presentationSources | Where-Object role -ne 'ROAR_RECOVERY'
    )
    Write-Json $masterPath $invalidSource
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'clip is absent from declared presentationSources'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidDamage = $baselineMasterText | ConvertFrom-Json
    (@($invalidDamage.patterns | Where-Object patternId -eq 'VALTAN_WHIRLWIND')[0].stages |
        Where-Object stageId -eq 'SPIN')[0].serverDamageProfileId = 123
    Write-Json $masterPath $invalidDamage
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'serverDamageProfileId must be a JSON string'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidWorldRef = $baselineMasterText | ConvertFrom-Json
    (@($invalidWorldRef.patterns | Where-Object patternId -eq 'VALTAN_ARENA_BREAK_109')[0].worldEventTriggerRefs)[0].stageId = 'MISSING'
    Write-Json $masterPath $invalidWorldRef
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'world event trigger reference has invalid owner'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidWeightedPool = $baselineMasterText | ConvertFrom-Json
    $invalidWeightedPool.normalSelection.patternIds[0] = 'VALTAN_EARTHQUAKE_SMASH'
    Write-Json $masterPath $invalidWeightedPool
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'weighted pool must contain exactly the five managed normal patterns'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidCounterReaction = $baselineMasterText | ConvertFrom-Json
    $invalidCounterReaction.counterReactionLayers[0].successActionId =
        'valtan.reactive.triple-counter.recovery'
    Write-Json $masterPath $invalidCounterReaction
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath -SkipProductDriftCheck
    } 'counter reaction branch/flag contract changed'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    $invalidCombatObjects = Read-Json $combatObjectPath
    (@($invalidCombatObjects.objects | Where-Object {
        $_.combatObjectArchetypeId -eq 'combatobject.valtan.high-jump.target-axe'
    }))[0].lifeMs = 5999
    Write-Json $combatObjectPath $invalidCombatObjects
    Invoke-ExpectedFailure {
        & $projector -Mode Validate -RepositoryRoot $fixtureRoot `
            -MasterPath $masterPath
    } 'Combat-object lifetime has drifted from the owner stage'

    [IO.File]::WriteAllText($masterPath, $baselineMasterText, $utf8NoBom)
    Write-Host 'Valtan pattern master focused harness PASS'
}
finally {
    $resolvedTempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    $resolvedFixture = [IO.Path]::GetFullPath($fixtureRoot)
    if ($resolvedFixture.StartsWith($resolvedTempRoot, [StringComparison]::OrdinalIgnoreCase) -and
        [IO.Directory]::Exists($resolvedFixture)) {
        Remove-Item -LiteralPath $resolvedFixture -Recurse -Force
    }
}
