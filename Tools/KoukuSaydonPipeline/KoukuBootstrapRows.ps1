# Shared canonical gameplay row validation/emission. Both publication and
# unsaved Debug audition call this owner; this file performs no file writes.
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
. (Join-Path $PSScriptRoot 'Publish-KoukuRaidRows.ps1')
. (Join-Path $PSScriptRoot 'KoukuParentSequenceContract.ps1')

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    [string[]]$actual = @($Value.PSObject.Properties.Name)
    [string[]]$expectedSorted = @($Expected)
    [Array]::Sort($actual, [StringComparer]::Ordinal)
    [Array]::Sort($expectedSorted, [StringComparer]::Ordinal)
    if (($actual -join "`n") -cne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId([string]$Value, [string]$Context) {
    if ($Value -notmatch $stableIdPattern) { throw "$Context is not a stable ID: '$Value'" }
}

function Test-KoukuActorContactWindows([object]$Child) {
    foreach ($window in @($Child.logicWindows)) {
        if ($window.kind -cnotin @('AREA_OVERLAP','ENTER_AREA')) { return $false }
        foreach ($slot in @('onSuccess','onFail','onTimeout')) {
            foreach ($outcome in @($window.$slot)) {
                if ($outcome.kind -cnotin @('FIXED_DAMAGE','MAX_HP_PERCENT_DAMAGE','MADNESS_GAUGE_ADD_PERCENT')) { return $false }
            }
        }
    }
    return $true
}

function Format-InvariantFloat([double]$Value, [string]$Context) {
    if ([double]::IsNaN($Value) -or [double]::IsInfinity($Value) -or $Value -lt 0.0 -or $Value -gt 100000.0) {
        throw "$Context is invalid: $Value"
    }
    return $Value.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
}

function Format-InvariantSignedFloat([double]$Value, [string]$Context) {
    if ([double]::IsNaN($Value) -or [double]::IsInfinity($Value) -or
        $Value -lt -100000.0 -or $Value -gt 100000.0) {
        throw "$Context is invalid: $Value"
    }
    return $Value.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
}

function Assert-JsonInteger(
    [object]$Value,
    [string]$Context,
    [long]$Minimum = 0,
    [long]$Maximum = [long]::MaxValue) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonNumber([object]$Value, [string]$Context) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64]) -and
        ($Value -isnot [double]) -and ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number)) {
        throw "$Context must be finite."
    }
}

function Format-JsonSignedNumbers([object[]]$Values, [string]$Context) {
    # Collider tracks contain tens of thousands of keys. Keep the exact JSON
    # number and signed-float admission, without two function/pipeline calls
    # per coordinate. The emitted doubles use the same invariant R format.
    $formatted = [string[]]::new($Values.Count)
    for ($index = 0; $index -lt $Values.Count; ++$index) {
        $value = $Values[$index]
        if (($value -isnot [int]) -and ($value -isnot [long]) -and
            ($value -isnot [uint32]) -and ($value -isnot [uint64]) -and
            ($value -isnot [double]) -and ($value -isnot [decimal])) {
            throw "$Context must be a JSON number."
        }
        $number = [double]$value
        if ([double]::IsNaN($number) -or [double]::IsInfinity($number)) {
            throw "$Context must be finite."
        }
        if ($number -lt -100000.0 -or $number -gt 100000.0) {
            throw "$Context is invalid: $number"
        }
        $formatted[$index] = $number.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
    }
    return ,$formatted
}

function Assert-JsonString([object]$Value, [string]$Context) {
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

function Get-KoukuTargetedVisualIndex([object]$Bindings, [uint32]$ExpectedRevision) {
    # Full resource/occurrence parity is checked by the composition projector
    # above. This join pins the Server's IDs/lifetimes to that same Client Product.
    Assert-JsonInteger $Bindings.sourceRevision 'Kouku targeted visual sourceRevision' 1 ([uint32]::MaxValue)
    if ($Bindings.schema -cne 'lostark.kouku-saydon-pattern-bindings' -or
        $Bindings.formatVersion -ne 1 -or $Bindings.sourceRevision -ne $ExpectedRevision -or
        $Bindings.targetedCombatVisuals -isnot [Array] -or
        @($Bindings.targetedCombatVisuals).Count -gt 8192) {
        throw 'Kouku targeted visuals require the same-revision Pattern bindings Product.'
    }
    $index = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($visual in @($Bindings.targetedCombatVisuals)) {
        $isPursuit = $visual.combatObjectArchetypeId -ceq 'combatobject.kouku.pursuit'
        $visualKeys = @('clientVisualId','combatObjectArchetypeId','durationMs','loop','resources','occurrences')
        if ($isPursuit) { $visualKeys += @('contactVisualId','contactEffectAssetId') }
        Assert-ExactProperties $visual $visualKeys 'Kouku targeted visual'
        if ($isPursuit) {
            Assert-StableId $visual.contactVisualId 'Pursuit contact visual'
            Assert-StableId $visual.contactEffectAssetId 'Pursuit contact effect'
        }
        Assert-JsonString $visual.clientVisualId 'Kouku targeted clientVisualId'
        Assert-JsonString $visual.combatObjectArchetypeId 'Kouku targeted archetype'
        Assert-JsonInteger $visual.durationMs 'Kouku targeted durationMs' 1 600000
        $role = if ($visual.combatObjectArchetypeId -ceq 'combatobject.kouku.showtime.fixed') { 'fixed' }
            elseif ($visual.combatObjectArchetypeId -ceq 'combatobject.kouku.showtime.tracking') { 'tracking' } elseif ($isPursuit) { 'pursuit' } else { '' }
        $visualPattern = if ($isPursuit) { '^kouku\.pursuit\.[0-9a-f]{64}$' } else { '^kouku\.showtime\.' + $role + '\.[0-9a-f]{64}$' }
        if (-not $role -or $visual.clientVisualId -cnotmatch $visualPattern -or
            $index.ContainsKey([string]$visual.clientVisualId) -or $visual.loop -isnot [bool] -or
            $visual.loop -ne ($role -cin @('tracking','pursuit')) -or $visual.resources -isnot [Array] -or
            @($visual.resources).Count -lt 1 -or @($visual.resources).Count -gt 1024 -or
            $visual.occurrences -isnot [Array] -or @($visual.occurrences).Count -lt 1 -or @($visual.occurrences).Count -gt 1024) {
            throw 'Kouku targeted visual identity, lifetime or bounded template is invalid.'
        }
        $index.Add([string]$visual.clientVisualId, $visual)
    }
    return ,$index
}

function New-KoukuAttackHitRows($Hits, [string]$EncounterId, [string]$PatternId, [string]$OwnerId,
    [string]$Role, [uint32]$SetIndex, [uint32]$LifetimeMs) {
    if ($null -eq $Hits) { return }
    if ($Hits -isnot [Array] -or $Hits.Count -gt 32) { throw 'Attack hits must be a bounded array.' }
    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $ordinal = 0
    foreach ($hit in $Hits) {
        Assert-ExactProperties $hit (@('hitId','trigger','atMs','endMs','repeatCount','repeatIntervalMs','shape',
            'radiusM','innerRadiusM','lengthM','halfWidthM','angleDegrees','offsetForwardM','offsetRightM','yawOffsetDegrees',
            'damageKind','damagePercent','damageProfileId') + $(if ($null -ne $hit.PSObject.Properties['riseHeightM']) { @('riseHeightM') } else { @() }) + $(if ($null -ne $hit.PSObject.Properties['pushMs']) { @('pushMs') } else { @() })) 'Attack hit'
        Assert-StableId $hit.hitId 'Attack hit ID'
        if (-not $ids.Add($hit.hitId)) { throw 'Attack hit ID repeats within its template.' }
        foreach ($field in @('atMs','endMs','repeatIntervalMs')) { Assert-JsonInteger $hit.$field "Attack $field" 0 600000 }
        Assert-JsonInteger $hit.repeatCount 'Attack repeatCount' 1 64
        Assert-JsonInteger $hit.damagePercent 'Attack damagePercent' 0 100
        foreach ($field in @('radiusM','innerRadiusM','lengthM','halfWidthM','angleDegrees','offsetForwardM','offsetRightM','yawOffsetDegrees')) {
            Assert-JsonNumber $hit.$field "Attack $field"
            if ([math]::Abs($hit.$field) -gt 1000) { throw 'Attack dimension exceeds 1000 m.' }
        }
        if ($hit.radiusM -lt 0 -or $hit.innerRadiusM -lt 0 -or $hit.lengthM -lt 0 -or $hit.halfWidthM -lt 0 -or
            $hit.angleDegrees -lt 0 -or $hit.angleDegrees -gt 360 -or [math]::Abs($hit.yawOffsetDegrees) -gt 360 -or
            ($hit.repeatCount -gt 1 -and $hit.repeatIntervalMs -lt 34)) { throw 'Attack dimensions or tick interval are invalid.' }
        $last = [uint64]$hit.atMs + [uint64]($hit.repeatCount - 1) * [uint64]$hit.repeatIntervalMs
        if (($hit.trigger -ceq 'TIMED' -and ($hit.endMs -ne 0 -or $last -gt $LifetimeMs)) -or
            ($hit.trigger -ceq 'CONTACT' -and ($hit.endMs -le $hit.atMs -or $hit.endMs -gt $LifetimeMs)) -or
            $hit.trigger -cnotin @('TIMED','CONTACT')) { throw 'Attack window exceeds its owning lifetime.' }
        $validShape = switch -CaseSensitive ($hit.shape) {
            'CIRCLE' { $hit.radiusM -gt 0 -and $hit.innerRadiusM -eq 0 }
            'RING' { $hit.innerRadiusM -gt 0 -and $hit.innerRadiusM -lt $hit.radiusM }
            'BOX' { $hit.lengthM -gt 0 -and $hit.halfWidthM -gt 0 }
            'CONE' { $hit.lengthM -gt 0 -and $hit.angleDegrees -gt 0 -and $hit.innerRadiusM -lt $hit.lengthM }
            default { $false }
        }
        if (-not $validShape) { throw 'Attack primitive is invalid.' }
        $profile = '-'
        if ($hit.damageKind -ceq 'PROFILE') {
            Assert-StableId $hit.damageProfileId 'Attack damage profile'
            if ($hit.damagePercent -ne 0 -or -not $damageIds.Contains($hit.damageProfileId)) { throw 'Attack damage profile is missing.' }
            $profile = $hit.damageProfileId
        } elseif ($hit.damageProfileId -cne '' -or
            -not (($hit.damageKind -ceq 'MAX_HP_PERCENT' -and $hit.damagePercent -gt 0) -or
                  ($hit.damageKind -ceq 'INSTANT_DEATH' -and $hit.damagePercent -eq 0))) { throw 'Attack damage policy is invalid.' }
        $rise = 0; $push = 0
        if ($null -ne $hit.PSObject.Properties['riseHeightM']) { $rise = $hit.riseHeightM }
        if ($null -ne $hit.PSObject.Properties['pushMs']) { $push = $hit.pushMs }
        Assert-JsonNumber $rise 'Attack riseHeightM'
        Assert-JsonInteger $push 'Attack pushMs' 0 5000
        if ($rise -lt 0 -or $rise -gt 100 -or ($rise -eq 0 -and $push -ne 0) -or ($rise -gt 0 -and $push -lt 100)) { throw 'Attack rise height and flight time must form a bounded pair.' }
        $flight = @(); if ($rise -gt 0) { $flight = @((Format-InvariantSignedFloat $rise 'Attack rise height'),$push) }
        $numbers = @('radiusM','innerRadiusM','lengthM','halfWidthM','angleDegrees','offsetForwardM','offsetRightM','yawOffsetDegrees') |
            ForEach-Object { Format-InvariantSignedFloat $hit.$_ "Attack $_" }
        (@('PATTERNATTACKHIT',$EncounterId,$PatternId,$OwnerId,$Role,$SetIndex,$ordinal,$hit.hitId,$hit.trigger,
            $hit.atMs,$hit.endMs,$hit.repeatCount,$hit.repeatIntervalMs,$hit.shape) + $numbers +
            @($hit.damageKind,$hit.damagePercent,$profile) + $flight) -join "`t"
        ++$ordinal
    }
}

function New-KoukuPursuitProjectileRows([object]$Pattern, [string]$EncounterId,
    [uint32]$PatternDurationMs, [Collections.Generic.Dictionary[string,object]]$Visuals) {
    if ($Pattern.pursuitProjectiles -isnot [Array] -or @($Pattern.pursuitProjectiles).Count -gt 64) { throw 'Pursuit requires a bounded array.' }
    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($target in @($Pattern.pursuitProjectiles)) {
        $hasDistance = $null -ne $target.PSObject.Properties['maxDistanceM']
        Assert-ExactProperties $target (@('occurrenceId','startMs','durationMs','visualIds','contactVisualId','speedMps',
            'contactRadiusM','spawnRadiusM','lifetimeMs','spawnIntervalMs','homing','countPerWave') + $(if ($hasDistance) { @('maxDistanceM') } else { @() }) + $(if ($null -ne $target.PSObject.Properties['projectileHits']) { @('projectileHits') } else { @() }) + $(if ($null -ne $target.PSObject.Properties['cardSymbols']) { @('cardSymbols') } else { @() })) 'Pursuit projectile'
        $distance = if ($hasDistance) { $target.maxDistanceM } else { 0 }
        Assert-JsonNumber $distance 'Pursuit maxDistanceM'
        Assert-StableId $target.occurrenceId 'Pursuit occurrence'
        Assert-StableId $target.contactVisualId 'Pursuit contact visual'
        Assert-JsonInteger $target.startMs 'Pursuit startMs' 0 600000
        Assert-JsonInteger $target.durationMs 'Pursuit durationMs' 1 600000
        Assert-JsonInteger $target.lifetimeMs 'Pursuit lifetimeMs' 0 600000
        Assert-JsonInteger $target.spawnIntervalMs 'Pursuit spawnIntervalMs' 0 600000
        Assert-JsonInteger $target.countPerWave 'Pursuit countPerWave' 1 16
        foreach ($field in @('speedMps','contactRadiusM','spawnRadiusM')) { Assert-JsonNumber $target.$field "Pursuit $field" }
        if (-not $ids.Add($target.occurrenceId) -or ($target.startMs + $target.durationMs) -gt $PatternDurationMs -or
            @($Pattern.logicWindows | Where-Object { $_.windowId -ceq $target.occurrenceId }).Count -ne 0 -or
            $target.visualIds -isnot [Array] -or @($target.visualIds).Count -lt 1 -or @($target.visualIds).Count -gt 4 -or
            $target.homing -isnot [bool] -or $target.speedMps -lt .01 -or $target.speedMps -gt 100 -or
            $distance -lt 0 -or $distance -gt 1000 -or
            $target.contactRadiusM -lt .01 -or $target.contactRadiusM -gt 10 -or $target.spawnRadiusM -lt 0 -or $target.spawnRadiusM -gt 100 -or
            ($target.lifetimeMs -eq 0 -and (-not $target.homing -or $target.spawnIntervalMs -ne 0 -or $distance -ne 0))) { throw 'Pursuit motion or lifetime is invalid.' }
        $pool = @($target.visualIds)
        foreach ($visualId in $pool) {
            Assert-StableId $visualId 'Pursuit visual'
            if (-not $Visuals.ContainsKey($visualId) -or $Visuals[$visualId].combatObjectArchetypeId -cne 'combatobject.kouku.pursuit' -or
                $Visuals[$visualId].contactVisualId -cne $target.contactVisualId) { throw 'Pursuit visual does not join its pinned Client template.' }
        }
        while ($pool.Count -lt 4) { $pool += '-' }
        $homing = if ($target.homing) { '1' } else { '0' }
        (@('PATTERNPURSUITPROJECTILES',$EncounterId,$Pattern.patternId,$target.occurrenceId,$target.startMs,$target.durationMs) + $pool +
            @($target.contactVisualId,(Format-InvariantFloat $target.speedMps 'Pursuit speed'),
              (Format-InvariantFloat $target.contactRadiusM 'Pursuit contact radius'),(Format-InvariantFloat $target.spawnRadiusM 'Pursuit spawn radius'),
              $target.lifetimeMs,$target.spawnIntervalMs,$homing,$target.countPerWave) +
            $(if ($distance -gt 0) { @((Format-InvariantFloat $distance 'Pursuit maximum distance')) } else { @() })) -join "`t"
        if ($null -ne $target.PSObject.Properties['cardSymbols']) {
            if ($target.cardSymbols -isnot [Array] -or @($target.cardSymbols).Count -ne @($target.visualIds).Count -or
                @($target.cardSymbols | Where-Object { $_ -isnot [string] -or $_ -cnotin @('HEART','SPADE','CLUB','DIAMOND') }).Count -ne 0) {
                throw 'Pursuit cardSymbols requires one valid symbol per visual.'
            }
            @('PATTERNPURSUITCARDS',$EncounterId,$Pattern.patternId,$target.occurrenceId,($target.cardSymbols -join ',')) -join "`t"
        }
        $hitLife = if ($target.lifetimeMs) { $target.lifetimeMs } else { 600000 }
        New-KoukuAttackHitRows $target.projectileHits $EncounterId $Pattern.patternId $target.occurrenceId 'PROJECTILE' 0 $hitLife
    }
}

function New-KoukuShowtimeTargetRows([object]$Pattern, [string]$EncounterId,
    [uint32]$PatternDurationMs, [Collections.Generic.Dictionary[string,object]]$Visuals) {
    if ($Pattern.showtimeTargets -isnot [Array] -or @($Pattern.showtimeTargets).Count -gt 64) {
        throw 'Kouku SHOWTIME targets must be a bounded array.'
    }
    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $rows = [Collections.Generic.List[string]]::new()
    foreach ($target in @($Pattern.showtimeTargets)) {
        $randomKeys = @('randomVolleys','randomSpawnIntervalMs','randomArenaRadiusM','randomArenaHeightToleranceM','randomAnchorKind','randomScaleMin','randomScaleMax')
        $hasRandom = @($randomKeys | Where-Object { $null -ne $target.PSObject.Properties[$_] }).Count
        if ($hasRandom -ne 0 -and $hasRandom -ne $randomKeys.Count) { throw 'Kouku random volleys require their pool, interval and arena bounds together.' }
        Assert-ExactProperties $target (@('occurrenceId','startMs','durationMs','fixedVisualId','trackingVisualId',
            'fixedLifetimeMs','spawnIntervalMs','followSpeedScale') + $(if ($hasRandom) { $randomKeys } else { @() }) + @(@('fixedHits','trackingHits') | Where-Object { $null -ne $target.PSObject.Properties[$_] })) 'Kouku SHOWTIME target'
        foreach ($field in @('occurrenceId','fixedVisualId','trackingVisualId')) {
            Assert-JsonString $target.$field "Kouku SHOWTIME $field"
        }
        Assert-StableId $target.occurrenceId 'Kouku SHOWTIME occurrenceId'
        Assert-JsonInteger $target.startMs 'Kouku SHOWTIME startMs' 0 600000
        Assert-JsonInteger $target.durationMs 'Kouku SHOWTIME durationMs' 1 600000
        Assert-JsonInteger $target.fixedLifetimeMs 'Kouku SHOWTIME fixedLifetimeMs' 0 600000
        Assert-JsonInteger $target.spawnIntervalMs 'Kouku SHOWTIME spawnIntervalMs' 1 600000
        Assert-JsonNumber $target.followSpeedScale 'Kouku SHOWTIME followSpeedScale'
        if (-not $ids.Add([string]$target.occurrenceId) -or
            @($Pattern.logicWindows | Where-Object { $_.windowId -ceq $target.occurrenceId }).Count -ne 0 -or
            ([uint64]$target.startMs + [uint64]$target.durationMs) -gt $PatternDurationMs -or
            $target.followSpeedScale -lt .01 -or $target.followSpeedScale -gt 10 -or
            (-not $target.fixedVisualId -and -not $target.trackingVisualId -and -not $hasRandom) -or
            (-not $target.fixedVisualId -and $target.fixedLifetimeMs -ne 0)) {
            throw 'Kouku SHOWTIME target identity, window, selection or movement is invalid.'
        }
        foreach ($role in @('fixed','tracking')) {
            $visualId = [string]$target.($role + 'VisualId')
            if (-not $visualId) { continue }
            Assert-StableId $visualId "Kouku SHOWTIME $role visual"
            if (-not $Visuals.ContainsKey($visualId) -or
                $Visuals[$visualId].combatObjectArchetypeId -cne ('combatobject.kouku.showtime.' + $role) -or
                ($role -ceq 'fixed' -and $Visuals[$visualId].durationMs -ne $target.fixedLifetimeMs)) {
                throw "Kouku SHOWTIME $role visual does not exact-join its Client template."
            }
        }
        $fixed = if ($target.fixedVisualId) { $target.fixedVisualId } else { '-' }
        $tracking = if ($target.trackingVisualId) { $target.trackingVisualId } else { '-' }
        $speed = Format-InvariantFloat $target.followSpeedScale 'Kouku SHOWTIME follow speed'
        $rows.Add((@('PATTERNSHOWTIMETARGETS', $EncounterId, $Pattern.patternId, $target.occurrenceId,
            $target.startMs, $target.durationMs, $fixed, $tracking, $target.fixedLifetimeMs, $target.spawnIntervalMs, $speed) -join "`t"))
        foreach ($role in @('fixed','tracking')) {
            $life = if ($role -ceq 'fixed') { $target.fixedLifetimeMs } else { $target.durationMs }
            foreach ($hitRow in @(New-KoukuAttackHitRows $target.($role + 'Hits') $EncounterId $Pattern.patternId $target.occurrenceId $role.ToUpperInvariant() 0 $life)) { $rows.Add($hitRow) }
        }
        if ($hasRandom) {
            if ($target.randomVolleys -isnot [Array] -or @($target.randomVolleys).Count -lt 1 -or @($target.randomVolleys).Count -gt 32) {
                throw 'Kouku random volley pool requires 1..32 ordered templates.'
            }
            Assert-JsonInteger $target.randomSpawnIntervalMs 'Kouku random interval' 1 600000
            Assert-JsonNumber $target.randomArenaRadiusM 'Kouku random arena radius'
            Assert-JsonNumber $target.randomArenaHeightToleranceM 'Kouku random arena height tolerance'
            if ($target.randomArenaRadiusM -le 0 -or $target.randomArenaRadiusM -gt 1000 -or
                $target.randomArenaHeightToleranceM -le 0 -or $target.randomArenaHeightToleranceM -gt 10) { throw 'Kouku random arena bounds are invalid.' }
            $radius = Format-InvariantFloat $target.randomArenaRadiusM 'Kouku random arena radius'
            $height = Format-InvariantFloat $target.randomArenaHeightToleranceM 'Kouku random arena height'
            Assert-JsonNumber $target.randomScaleMin 'Kouku minimum scale'
            Assert-JsonNumber $target.randomScaleMax 'Kouku maximum scale'
            if ($target.randomAnchorKind -cnotin @('BOSS_SPAWN','BOSS') -or $target.randomScaleMin -lt .01 -or $target.randomScaleMax -gt 10 -or $target.randomScaleMax -lt $target.randomScaleMin) { throw 'Kouku random scale or anchor is invalid.' }
            $minScale = Format-InvariantFloat $target.randomScaleMin 'Kouku minimum scale'
            $maxScale = Format-InvariantFloat $target.randomScaleMax 'Kouku maximum scale'
            $ordinal = 0
            foreach ($volley in @($target.randomVolleys)) {
                Assert-ExactProperties $volley (@('clientVisualId','lifetimeMs') + $(if ($null -ne $volley.PSObject.Properties['hits']) { @('hits') } else { @() })) 'Kouku random volley'
                Assert-JsonString $volley.clientVisualId 'Kouku random visual ID'
                Assert-StableId $volley.clientVisualId 'Kouku random visual ID'
                Assert-JsonInteger $volley.lifetimeMs 'Kouku random lifetime' 1 600000
                if (-not $Visuals.ContainsKey($volley.clientVisualId) -or
                    $Visuals[$volley.clientVisualId].combatObjectArchetypeId -cne 'combatobject.kouku.showtime.fixed' -or
                    $Visuals[$volley.clientVisualId].loop -ne $false -or $Visuals[$volley.clientVisualId].durationMs -ne $volley.lifetimeMs) {
                    throw 'Kouku random volley does not exact-join a finite fixed Client template.'
                }
                $rows.Add((@('PATTERNSHOWTIMERANDOM',$EncounterId,$Pattern.patternId,$target.occurrenceId,$ordinal,
                    $volley.clientVisualId,$volley.lifetimeMs,$target.randomSpawnIntervalMs,$radius,$height,$target.randomAnchorKind,$minScale,$maxScale) -join "`t"))
                foreach ($hitRow in @(New-KoukuAttackHitRows $volley.hits $EncounterId $Pattern.patternId $target.occurrenceId 'RANDOM' $ordinal $volley.lifetimeMs)) { $rows.Add($hitRow) }
                ++$ordinal
            }
        }
    }
    return $rows.ToArray()
}

function New-KoukuAlbionAirborneRow([object]$Trigger,[string]$EncounterId,[string]$PatternId,[uint32]$PatternDurationMs) {
    $fields = @('airbornePhase','airborneHeightM','airborneDurationMs')
    $present = @($fields | Where-Object { $null -ne $Trigger.PSObject.Properties[$_] }).Count
    $selectionFields = @('airborneTargetPositionPolicy','selectedEffectVisualId','selectedEffectLifetimeMs')
    $selectionPresent = @($selectionFields | Where-Object { $null -ne $Trigger.PSObject.Properties[$_] }).Count
    if ($Trigger.kind -cne 'ALBION_AIRBORNE') {
        if ($present -or $selectionPresent) { throw 'Only ALBION_AIRBORNE owns airborne phase values.' }
        return
    }
    if ($present -ne 3) { throw 'ALBION_AIRBORNE requires its three phase values together.' }
    Assert-JsonString $Trigger.airbornePhase 'Albion airborne phase'
    Assert-JsonNumber $Trigger.airborneHeightM 'Albion airborne height'
    Assert-JsonInteger $Trigger.airborneDurationMs 'Albion airborne duration' 0 600000
    $phase = [string]$Trigger.airbornePhase
    $height = [double]$Trigger.airborneHeightM
    $duration = [uint32]$Trigger.airborneDurationMs
    if ($phase -cnotin @('JUMP','SELECT_PLAYER','APPEAR_PLAYER','DISAPPEAR','CENTER','SLAM') -or
        $height -lt 0 -or $height -gt 100000 -or (($phase -cin @('JUMP','APPEAR_PLAYER')) -ne ($height -gt 0)) -or
        ($phase -ceq 'JUMP' -and ([uint64]$Trigger.startMs + $duration) -gt $PatternDurationMs) -or
        ($phase -cne 'JUMP' -and $duration -ne 0) -or $Trigger.hudMode -cne 'NONE' -or
        $Trigger.faceCenterYawOffsetDegrees -ne 0 -or
        @($Trigger.teleportPosition | Where-Object { [Math]::Abs([double]$_) -gt 100000 }).Count -ne 0 -or
        ($phase -cne 'CENTER' -and @($Trigger.teleportPosition | Where-Object { $_ -ne 0 }).Count -ne 0)) {
        throw 'Albion airborne phase, height, duration or coordinate ownership is invalid.'
    }
    $row = @('PATTERNALBIONAIRBORNE',$EncounterId,$PatternId,$Trigger.triggerId,$phase,
        (Format-InvariantFloat $height 'Albion airborne height'),$duration)
    if ($selectionPresent) {
        if ($phase -cne 'SELECT_PLAYER' -or $Trigger.airborneTargetPositionPolicy -cne 'SELECT') {
            throw 'Selected position fields require SELECT_PLAYER and SELECT policy.'
        }
        $visualId = ''; $lifetime = 0
        if ($null -ne $Trigger.PSObject.Properties['selectedEffectVisualId'] -or $null -ne $Trigger.PSObject.Properties['selectedEffectLifetimeMs']) {
            Assert-StableId $Trigger.selectedEffectVisualId 'Selected airborne Effect visual ID'
            Assert-JsonInteger $Trigger.selectedEffectLifetimeMs 'Selected airborne Effect lifetime' 1 600000
            $visualId = [string]$Trigger.selectedEffectVisualId; $lifetime = [uint32]$Trigger.selectedEffectLifetimeMs
            if (([uint64]$Trigger.startMs + $lifetime) -gt $PatternDurationMs) { throw 'Selected Effect exceeds Pattern lifetime.' }
        }
        $row += @('SELECT',$visualId,$lifetime)
    }
    return ($row -join "`t")
}

function Format-RootMotionSamples {
    param(
        [object[]]$Samples,
        [string]$SkillId,
        [uint32]$LimitMs,
        [switch]$IncludeUp
    )

    if ($Samples.Count -lt 2 -or $Samples.Count -gt 512) {
        throw "Root motion sample count is invalid: $SkillId"
    }
    $packed = [Collections.Generic.List[string]]::new()
    $previousMs = -1
    foreach ($sample in $Samples) {
        Assert-ExactProperties $sample @('timeMs','forward','lateral','up') 'root motion sample'
        Assert-JsonInteger $sample.timeMs "root motion $SkillId timeMs" 0 $LimitMs
        foreach ($axis in @('forward','lateral','up')) {
            Assert-JsonNumber $sample.$axis "root motion $SkillId $axis"
        }
        $timeMs = [int]$sample.timeMs
        if ($timeMs -le $previousMs -or $timeMs -gt $LimitMs) {
            throw "Root motion sample time is out of order or past the action: $SkillId"
        }
        $previousMs = $timeMs
        $row = ('{0}:{1}:{2}' -f $timeMs,
            (Format-InvariantSignedFloat $sample.forward "root motion $SkillId forward"),
            (Format-InvariantSignedFloat $sample.lateral "root motion $SkillId lateral"))
        if ($IncludeUp) {
            $row += ':' + (Format-InvariantSignedFloat $sample.up "root motion $SkillId up")
        }
        $packed.Add($row)
    }
    if ($IncludeUp -and ($Samples[0].timeMs -ne 0 -or $Samples[-1].timeMs -ne $LimitMs -or
            $Samples[0].forward -ne 0 -or $Samples[0].lateral -ne 0 -or $Samples[0].up -ne 0)) {
        throw "Server XYZ root motion needs a zero origin and the exact stage endpoint: $SkillId"
    }
    return ($packed -join ',')
}

function Get-BootstrapRowSortKey {
	param([Parameter(Mandatory = $true)][string]$Row)

	$fields = @($Row.Split("`t"))
    if ($fields[0] -cin @('RAIDGATE','RAIDFLOWSTEP','RAIDFLOWGROUP','RAIDARRIVAL','RAIDBINGOSPECIAL')) {
        # Gate definitions must precede their dense flow steps and arrival slots.
        $gateKey = if ($fields[0] -ceq 'RAIDGATE') { $fields[2] } else { $fields[1] }
        $rank = if ($fields[0] -ceq 'RAIDGATE') { 0 } elseif ($fields[0] -ceq 'RAIDFLOWSTEP') { 1 } elseif ($fields[0] -ceq 'RAIDFLOWGROUP') { 2 } elseif ($fields[0] -ceq 'RAIDBINGOSPECIAL') { 3 } else { 4 }
        $Row = (@('RAIDGATE',$gateKey,$rank) + @($fields[2..($fields.Count - 1)])) -join "`t"
    }
    if ($fields.Count -eq 8 -and $fields[0] -ceq 'PATTERNPARENTCHILD') {
        # The Server appends children in timeline order. Stable occurrence IDs
        # may be allocated after earlier boxes, so they only break time ties.
        $Row = (@($fields[0], $fields[1], $fields[2], $fields[5], $fields[3],
            $fields[4], $fields[6], $fields[7])) -join "`t"
    }
    if ($fields.Count -ge 5 -and $fields[0] -ceq 'PATTERNATTACKHIT') {
        $parent = if ($fields[4] -ceq 'PROJECTILE') { 'PATTERNPURSUITPROJECTILES' } elseif ($fields[4] -ceq 'ALBION') { 'PATTERNMECHANICTRIGGER' } else { 'PATTERNSHOWTIMETARGETS' }
        $Row = (@($parent,$fields[1],$fields[2],$fields[3],2) + @($fields[4..($fields.Count - 1)])) -join "`t"
    }
    if ($fields.Count -ge 4 -and $fields[0] -ceq 'PATTERNPURSUITCARDS') {
        $Row = (@('PATTERNPURSUITPROJECTILES',$fields[1],$fields[2],$fields[3],1) + @($fields[4..($fields.Count - 1)])) -join "`t"
    }
    if ($fields.Count -ge 4 -and $fields[0] -ceq 'PATTERNPURSUITPROJECTILES') {
        $Row = (@($fields[0],$fields[1],$fields[2],$fields[3],0) + @($fields[4..($fields.Count - 1)])) -join "`t"
    }
	if ($fields.Count -ge 4 -and $fields[0] -cin @('PATTERNBOSSMOTION','PATTERNBOSSMOTIONKEY')) {
		$dependencyOrder = if ($fields[0] -ceq 'PATTERNBOSSMOTION') { 0 } else { 1 }
		$Row = (@('PATTERNBOSSMOTION',$fields[1],$fields[2],$dependencyOrder) + @($fields[3..($fields.Count - 1)])) -join "`t"
	}
	if ($fields.Count -ge 4 -and $fields[0] -cin @('PATTERNMECHANICTRIGGER','PATTERNALBIONAIRBORNE','PATTERNCARDMAZESTAGING','PATTERNCARDRAINSOLDIERS')) {
		# Child settings resolve their exact, already loaded mechanic occurrence.
		$dependencyOrder = if ($fields[0] -ceq 'PATTERNMECHANICTRIGGER') { 0 } else { 1 }
		$Row = (@('PATTERNMECHANICTRIGGER',$fields[1],$fields[2],$fields[3],$dependencyOrder) +
			@($fields[4..($fields.Count - 1)])) -join "`t"
	}
	if ($fields.Count -ge 5 -and $fields[0] -cin @(
		'PATTERNSHOWTIMETARGETS','PATTERNSHOWTIMERANDOM')) {
		# Random volleys resolve an already loaded target occurrence. Keep the
		# parent first, then its dense numeric volley ordinals, for each owner.
		$dependencyOrder = if ($fields[0] -ceq 'PATTERNSHOWTIMETARGETS') { 0 } else { 1 }
		$Row = (@('PATTERNSHOWTIMETARGETS', $fields[1], $fields[2],
			$fields[3], $dependencyOrder) + @($fields[4..($fields.Count - 1)])) -join "`t"
	}
	if ($fields.Count -ge 4 -and $fields[0] -cin @(
		'PATTERNWORLDSEQUENCE','PATTERNWORLDPLACEMENT','PATTERNWORLDSUPPORT','PATTERNWORLDCOMBAT','PATTERNWORLDAUTHOREDMADNESS')) {
		# Placement/support rows resolve an already loaded World occurrence.
		# Preserve the original ordering inside each kind after its parent rank.
		$dependencyOrder = switch -CaseSensitive ($fields[0]) {
			'PATTERNWORLDSEQUENCE' { 0 }
			'PATTERNWORLDPLACEMENT' { 1 }
			'PATTERNWORLDSUPPORT' { 2 }
			'PATTERNWORLDCOMBAT' { 3 }
			'PATTERNWORLDAUTHOREDMADNESS' { 4 }
		}
		$Row = (@('PATTERNWORLDSEQUENCE', $dependencyOrder) +
			@($fields[1..($fields.Count - 1)])) -join "`t"
	}
	if ($fields.Count -ge 6 -and $fields[0] -cin @(
		'PATTERNLOGICOUTCOME','PATTERNLOGICCONTACTMOTION','PATTERNLOGICSIGNAL')) {
		# Contact mappings and completion signals decorate an already loaded
		# outcome slot. Admit every dense outcome ordinal before its dependents.
		$dependencyOrder = switch -CaseSensitive ($fields[0]) {
			'PATTERNLOGICOUTCOME' { 0 }
			'PATTERNLOGICCONTACTMOTION' { 1 }
			'PATTERNLOGICSIGNAL' { 2 }
		}
		$tail = if ($fields.Count -gt 6) { @($fields[6..($fields.Count - 1)]) }
			else { @() }
		$Row = (@(
			'PATTERNLOGICOUTCOME', $fields[1], $fields[2], $fields[3],
			$fields[4], $dependencyOrder, $fields[5], $fields[0]) + $tail) -join "`t"
	}
	if ($fields.Count -ge 5 -and $fields[0] -cin @(
		'PATTERNSTAGEACTION','PATTERNSTAGEVOLLEY')) {
		# Both row kinds append to the same Server stage action vector. Keep their
		# authored ordinal ahead of the concrete kind so mixed typed actions still
		# arrive as one dense 0..N-1 sequence.
		$tail = if ($fields.Count -gt 5) { @($fields[5..($fields.Count - 1)]) }
			else { @() }
		$Row = (@(
			'PATTERNSTAGEACTION', $fields[1], $fields[2], $fields[3],
			$fields[4], $fields[0]) + $tail) -join "`t"
	}
	$key = [Text.StringBuilder]::new()
	$digits = [Text.StringBuilder]::new()
	foreach ($character in $Row.ToCharArray()) {
		if ([char]::IsDigit($character)) {
			[void]$digits.Append($character)
			continue
		}
		if ($digits.Length -gt 0) {
			[void]$key.Append($digits.ToString().PadLeft(12, '0'))
			[void]$digits.Clear()
		}
		[void]$key.Append($character)
	}
	if ($digits.Length -gt 0) {
		[void]$key.Append($digits.ToString().PadLeft(12, '0'))
	}
	return $key.ToString()
}

function Add-KoukuBootstrapRows {
    param(
        [Parameter(Mandatory=$true)][object]$Encounter,
        [object]$Presentation,
        [Parameter(Mandatory=$true)][object]$BossProfiles,
        [Parameter(Mandatory=$true)][AllowEmptyCollection()][Collections.Generic.List[string]]$Rows,
        [ValidateRange(-1,100)][int]$MadnessGaugeAddPercent = -1,
        [object]$MadnessTuning = $null
    )
    $koukuEncounterDocument = $Encounter
    $koukuTargetBindings = $Presentation
    $bossDocument = $BossProfiles
    $bossIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($boss in @($bossDocument.bosses)) { [void]$bossIds.Add([string]$boss.archetypeId) }
    $patternRows = $Rows
$koukuInventoryProperties = @()
if ($null -ne $koukuEncounterDocument.PSObject.Properties['raidGates']) { $koukuInventoryProperties += 'raidGates' }
if ($null -ne $koukuEncounterDocument.PSObject.Properties['patternInventory']) {
	$koukuInventoryProperties += 'patternInventory'
	Assert-ExactProperties $koukuEncounterDocument.patternInventory @('folders','patterns','bundles') 'KoukuSaydon saved Pattern inventory'
	foreach ($field in @('folders','patterns','bundles')) {
		if ($koukuEncounterDocument.patternInventory.$field -isnot [Array] -or
			@($koukuEncounterDocument.patternInventory.$field).Count -gt 4096) {
			throw "KoukuSaydon saved Pattern inventory $field must be a bounded array."
		}
	}
	# The projector validation above compares the complete inventory and ready
	# graph with the saved composition. It is display metadata, never Server rows.
}
Assert-ExactProperties $koukuEncounterDocument (@(
	'schema','formatVersion','encounterId','bossArchetypeId','authority',
	'fixedTickHz','sourceRevision','madnessPolicy','playAllPatternIds',
	'patterns','folders','bundles') + $koukuInventoryProperties) `
	'KoukuSaydon encounter Product'
foreach ($field in @('schema','encounterId','bossArchetypeId','authority')) {
	Assert-JsonString $koukuEncounterDocument.$field `
		"KoukuSaydon encounter $field"
}
Assert-JsonInteger $koukuEncounterDocument.formatVersion `
	'KoukuSaydon encounter formatVersion' 4 4
Assert-JsonInteger $koukuEncounterDocument.fixedTickHz `
	'KoukuSaydon encounter fixedTickHz' 30 30
Assert-JsonInteger $koukuEncounterDocument.sourceRevision `
	'KoukuSaydon encounter sourceRevision' 1 ([uint32]::MaxValue)
if ([string]$koukuEncounterDocument.schema -cne 'lostark.encounter-profile' -or
	[string]$koukuEncounterDocument.encounterId -cne
		'ENCOUNTER_KAKULSAYDON_G1' -or
	[string]$koukuEncounterDocument.bossArchetypeId -cne
		'BOSS_KAKULSAYDON_G1_KOUKU' -or
	[string]$koukuEncounterDocument.authority -cne 'server' -or
	[uint32]$koukuEncounterDocument.fixedTickHz -ne 30 -or
	-not $bossIds.Contains([string]$koukuEncounterDocument.bossArchetypeId) -or
	$koukuEncounterDocument.patterns -isnot [Array] -or
	@($koukuEncounterDocument.patterns).Count -lt 1 -or
	@($koukuEncounterDocument.patterns).Count -gt 255 -or
	$koukuEncounterDocument.playAllPatternIds -isnot [Array]) {
	throw 'KoukuSaydon encounter Product header is invalid.'
}
$koukuEncounterBosses = @($bossDocument.bosses | Where-Object {
	[string]$_.archetypeId -ceq
		[string]$koukuEncounterDocument.bossArchetypeId -and
	[string]$_.encounterId -ceq [string]$koukuEncounterDocument.encounterId
})
if ($koukuEncounterBosses.Count -ne 1) {
	throw 'KoukuSaydon encounter does not exact-join its boss profile.'
}
$koukuPatternIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$koukuPatternActionIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$koukuProductOrder = [Collections.Generic.List[string]]::new()
$patternRows.Add((@(
	'KOUKUSAYDONPRODUCTREVISION', $koukuEncounterDocument.encounterId,
	$koukuEncounterDocument.bossArchetypeId,
	[uint32]$koukuEncounterDocument.sourceRevision) -join "`t"))
# The madness gauge maximum and the clown hold the encounter applies to every
# player of the arena. One row per encounter.
Assert-ExactProperties $koukuEncounterDocument.madnessPolicy @(
	'maximum','clownHoldMs') 'KoukuSaydon encounter madnessPolicy'
Assert-JsonInteger $koukuEncounterDocument.madnessPolicy.maximum `
	'KoukuSaydon madnessPolicy maximum' 1 1000000
Assert-JsonInteger $koukuEncounterDocument.madnessPolicy.clownHoldMs `
	'KoukuSaydon madnessPolicy clownHoldMs' 0 600000
$madnessFields = @('KOUKUMADNESS', $koukuEncounterDocument.encounterId,
    [uint32]$koukuEncounterDocument.madnessPolicy.maximum,
    [uint32]$koukuEncounterDocument.madnessPolicy.clownHoldMs)
if ($null -ne $MadnessTuning) {
    $madnessFields += @([uint32]$MadnessTuning.damageGainPercent,
        [uint32]$MadnessTuning.ballGainPercent, [uint32]$MadnessTuning.ballMultiplierPercent,
        (Format-InvariantFloat $MadnessTuning.ballRadiusM 'Madness ballRadiusM'),
        [uint32]$MadnessTuning.dollGainPercent, [uint32]$MadnessTuning.dollMultiplierPercent,
        (Format-InvariantFloat $MadnessTuning.dollRadiusM 'Madness dollRadiusM'), [uint32]$MadnessTuning.specialIntervalMs)
}
$patternRows.Add(($madnessFields -join "`t"))
$koukuPatternById = @{}
$koukuGateTargets = @{
    'GATE1|MN_RPCZ_00' = 'boss.kakulsaydon.g1.kouku'
    'GATE1|MN_RPCT_05' = 'boss.kakulsaydon.g1.saydon'
    'GATE2|MN_RPCZ_00' = 'boss.kakulsaydon.g2.kouku'
    'GATE2|MN_RPCT_06' = 'boss.kakulsaydon.g2.big-saydon'
    'GATE3|MN_RPCT_05' = 'boss.kakulsaydon.g3.saydon'
    'BINGO|MN_RPCT_05' = 'boss.kakulsaydon.bingo.saydon'
}
$koukuFollowupTargets = [Collections.Generic.List[string]]::new()
$koukuTargetedVisuals = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
if (@($koukuEncounterDocument.patterns | Where-Object { $null -ne $_.PSObject.Properties['showtimeTargets'] -or $null -ne $_.PSObject.Properties['pursuitProjectiles'] }).Count -gt 0) {
    $koukuTargetedVisuals = Get-KoukuTargetedVisualIndex $koukuTargetBindings $koukuEncounterDocument.sourceRevision
}
foreach ($koukuPattern in @($koukuEncounterDocument.patterns)) {
	$koukuOptionalProperties = @()
	if ($null -ne $koukuPattern.PSObject.Properties['resetBossYawDegrees']) { $koukuOptionalProperties += 'resetBossYawDegrees' }
	if ($null -ne $koukuPattern.PSObject.Properties['bossMotion']) { $koukuOptionalProperties += 'bossMotion' }
	if ($null -ne $koukuPattern.PSObject.Properties['folderId']) { $koukuOptionalProperties += 'folderId' }
	if ($null -ne $koukuPattern.PSObject.Properties['fixedTimeline']) { $koukuOptionalProperties += 'fixedTimeline' }
	if ($null -ne $koukuPattern.PSObject.Properties['timelineDurationMs']) { $koukuOptionalProperties += 'timelineDurationMs' }
	if ($null -ne $koukuPattern.PSObject.Properties['parentPatternSequence']) { $koukuOptionalProperties += 'parentPatternSequence' }
	if ($null -ne $koukuPattern.PSObject.Properties['showtimeTargets']) { $koukuOptionalProperties += 'showtimeTargets' }
	if ($null -ne $koukuPattern.PSObject.Properties['pursuitProjectiles']) { $koukuOptionalProperties += 'pursuitProjectiles' }
	Assert-ExactProperties $koukuPattern (@(
		'patternId','category','minimumPhase','maximumPhase','targetPolicy',
		'aimPolicy','displayName','actionId','sourceActionIds','selectionMode',
		'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
		'armorRequirement','phaseRequirement','invulnerableWhileRunning',
		'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
		'bossArchetypeIds','stages','logicWindows','worldSequences',
		'sceneProfiles','mechanicTriggers','resetBossToSpawn','gateId','targetBossPlacementId','actorProfileId') + $koukuOptionalProperties) 'KoukuSaydon encounter pattern'
	foreach ($field in @(
		'patternId','category','targetPolicy','aimPolicy','displayName','actionId',
		'selectionMode','armorRequirement','phaseRequirement')) {
		Assert-JsonString $koukuPattern.$field `
			"KoukuSaydon pattern $field"
	}
	foreach ($field in @(
		'minimumPhase','maximumPhase','minimumHealthBar','maximumHealthBar',
		'triggerHealthBar','triggerOrder','selectionWeight',
		'maximumConsecutiveUses')) {
		Assert-JsonInteger $koukuPattern.$field `
			"KoukuSaydon pattern $field" 0 ([uint32]::MaxValue)
	}
	Assert-JsonNumber $koukuPattern.minimumRange `
		'KoukuSaydon pattern minimumRange'
	Assert-JsonNumber $koukuPattern.maximumRange `
		'KoukuSaydon pattern maximumRange'
	Assert-StableId $koukuPattern.patternId 'KoukuSaydon patternId'
	Assert-StableId $koukuPattern.actionId 'KoukuSaydon pattern actionId'
	if ($koukuPattern.invulnerableWhileRunning -isnot [bool] -or
		-not $koukuPatternIds.Add([string]$koukuPattern.patternId) -or
		-not $koukuPatternActionIds.Add([string]$koukuPattern.actionId) -or
		[string]$koukuPattern.category -cne 'MECHANIC' -or
		[string]$koukuPattern.selectionMode -cne 'AUDITION_ONLY' -or
		[string]$koukuPattern.targetPolicy -cne 'NONE' -or
		[string]$koukuPattern.aimPolicy -cne 'NONE' -or
		[string]$koukuPattern.armorRequirement -cne 'ANY' -or
		[string]$koukuPattern.phaseRequirement -cne 'ANY' -or
		[bool]$koukuPattern.invulnerableWhileRunning -or
		[uint32]$koukuPattern.minimumPhase -ne 1 -or
		[uint32]$koukuPattern.maximumPhase -ne 1 -or
		[uint32]$koukuPattern.minimumHealthBar -ne 0 -or
		[uint32]$koukuPattern.maximumHealthBar -ne 0 -or
		[uint32]$koukuPattern.triggerHealthBar -ne 0 -or
		[uint32]$koukuPattern.triggerOrder -ne 0 -or
		[uint32]$koukuPattern.selectionWeight -ne 0 -or
		[uint32]$koukuPattern.maximumConsecutiveUses -ne 0 -or
		[double]$koukuPattern.minimumRange -ne 0.0 -or
		[double]$koukuPattern.maximumRange -ne 1.0 -or
		$koukuPattern.sourceActionIds -isnot [Array] -or
		@($koukuPattern.sourceActionIds).Count -gt 64 -or
		$koukuPattern.stages -isnot [Array] -or
		@($koukuPattern.stages).Count -lt 1 -or
		@($koukuPattern.stages).Count -gt 64) {
		throw "KoukuSaydon animation-audition pattern is invalid: $($koukuPattern.patternId)"
	}
	if ($koukuPattern.resetBossToSpawn -isnot [bool]) { throw 'KoukuSaydon resetBossToSpawn must be boolean' }
	if ($null -ne $koukuPattern.PSObject.Properties['fixedTimeline']) {
		if ($koukuPattern.fixedTimeline -isnot [bool]) { throw 'KoukuSaydon fixedTimeline must be boolean' }
		if ($koukuPattern.fixedTimeline) {
			$patternRows.Add((@('PATTERNFIXEDTIMELINE', $koukuEncounterDocument.encounterId, $koukuPattern.patternId) -join "`t"))
		}
	}
	$spawnResetRow = @('PATTERNSPAWNRESET', $koukuEncounterDocument.encounterId, $koukuPattern.patternId, 1)
	if ($null -ne $koukuPattern.PSObject.Properties['resetBossYawDegrees']) {
		Assert-JsonNumber $koukuPattern.resetBossYawDegrees 'KoukuSaydon resetBossYawDegrees'
		if (-not $koukuPattern.resetBossToSpawn -or [Math]::Abs([double]$koukuPattern.resetBossYawDegrees) -gt 360.0) {
			throw 'KoukuSaydon resetBossYawDegrees requires spawn reset and -360..360 degrees'
		}
		$spawnResetRow += Format-InvariantSignedFloat $koukuPattern.resetBossYawDegrees 'KoukuSaydon resetBossYawDegrees'
	}
	if ($koukuPattern.resetBossToSpawn) {
		$patternRows.Add(($spawnResetRow -join "`t"))
	}
    foreach ($targetField in @('gateId','targetBossPlacementId','actorProfileId')) {
        Assert-JsonString $koukuPattern.$targetField "KoukuSaydon pattern $targetField"
        Assert-StableId $koukuPattern.$targetField "KoukuSaydon pattern $targetField"
    }
    $targetKey = [string]$koukuPattern.gateId + '|' + [string]$koukuPattern.actorProfileId
    if (-not $koukuGateTargets.ContainsKey($targetKey) -or [string]$koukuGateTargets[$targetKey] -cne [string]$koukuPattern.targetBossPlacementId) {
        throw "KoukuSaydon Pattern Gate/target/model mismatch: $($koukuPattern.patternId)"
    }
    $koukuPatternById[[string]$koukuPattern.patternId] = $koukuPattern
    $patternRows.Add((@('PATTERNTARGET', $koukuPattern.patternId, $koukuPattern.gateId, $koukuPattern.targetBossPlacementId) -join "`t"))
    $koukuProductOrder.Add([string]$koukuPattern.patternId)
	$koukuSourceActionIds = [Collections.Generic.HashSet[uint32]]::new()
	foreach ($sourceActionId in @($koukuPattern.sourceActionIds)) {
		Assert-JsonInteger $sourceActionId `
			"KoukuSaydon $($koukuPattern.patternId) sourceActionId" `
			0 ([uint32]::MaxValue)
		if (-not $koukuSourceActionIds.Add([uint32]$sourceActionId)) {
			throw "KoukuSaydon pattern sourceActionId is duplicated: $($koukuPattern.patternId)"
		}
	}
	$patternRows.Add((@(
		'PATTERN', $koukuEncounterDocument.encounterId,
		$koukuPattern.patternId, $koukuPattern.actionId,
		$koukuPattern.selectionMode, 0, 0, 0, 0, 0, 0,
		(Format-InvariantFloat $koukuPattern.minimumRange `
			'KoukuSaydon pattern minimumRange'),
		(Format-InvariantFloat $koukuPattern.maximumRange `
			'KoukuSaydon pattern maximumRange'),
		@($koukuPattern.stages).Count, 'ANY', 'ANY', 0) -join "`t"))
	# The arena boss bodies this pattern's clips belong to. The Server audition
	# admits a live boss of one of these archetypes only; every entry must be a
	# KoukuSaydon arena boss profile of the same encounter.
	if ($koukuPattern.bossArchetypeIds -isnot [Array] -or
		@($koukuPattern.bossArchetypeIds).Count -lt 1 -or
		@($koukuPattern.bossArchetypeIds).Count -gt 8) {
		throw "KoukuSaydon pattern bossArchetypeIds must name 1-8 arena bosses: $($koukuPattern.patternId)"
	}
	$koukuPatternBossIds =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	foreach ($patternBossId in @($koukuPattern.bossArchetypeIds)) {
		Assert-JsonString $patternBossId `
			"KoukuSaydon $($koukuPattern.patternId) bossArchetypeId"
		Assert-StableId $patternBossId `
			"KoukuSaydon $($koukuPattern.patternId) bossArchetypeId"
		$patternBoss = @($bossDocument.bosses | Where-Object {
			[string]$_.archetypeId -ceq [string]$patternBossId })
		if ($patternBoss.Count -ne 1 -or
			[string]$patternBoss[0].encounterId -cne
				[string]$koukuEncounterDocument.encounterId -or
			[string]$patternBossId -cnotlike 'BOSS_KAKULSAYDON_*' -or
			-not $koukuPatternBossIds.Add([string]$patternBossId)) {
			throw "KoukuSaydon pattern bossArchetypeId is not an arena boss of its encounter: $($koukuPattern.patternId)/$patternBossId"
		}
		$patternRows.Add((@(
			'PATTERNBOSS', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, [string]$patternBossId) -join "`t"))
	}
	$patternRows.Add((@(
		'PATTERNPOLICY', $koukuEncounterDocument.encounterId,
		$koukuPattern.patternId, $koukuPattern.category, 1, 1,
		'NONE', 'NONE') -join "`t"))
	# Action reference supplies identity and animation duration only.  The v33
	# timing columns remain neutral instead of inventing hit/range/cooldown data.
	if (@($koukuPattern.sourceActionIds).Count -gt 0) {
	$patternRows.Add((@(
		'PATTERNSOURCE', $koukuEncounterDocument.encounterId,
		$koukuPattern.patternId, [uint32]$koukuPattern.sourceActionIds[0],
		0, 0, 0, 0, 0, 0) -join "`t"))
	}

	$koukuStageIds =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$koukuStageActionIds =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	for ($stageIndex = 0;
		$stageIndex -lt @($koukuPattern.stages).Count; ++$stageIndex) {
		$koukuStage = $koukuPattern.stages[$stageIndex]
		$koukuStageOptionalProperties = @()
		if ($null -ne $koukuStage.PSObject.Properties['actions']) { $koukuStageOptionalProperties += 'actions' }
		if ($null -ne $koukuStage.PSObject.Properties['rootMotionSamples']) { $koukuStageOptionalProperties += 'rootMotionSamples' }
		Assert-ExactProperties $koukuStage (@(
			'stageId','actionId','stageKind','durationMs','hitShape',
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
			'hitHalfWidth','hitCount','hitIntervalMs','hitDelayMs',
			'serverDamageProfileId','pushRangeM','pushMs','knockdown','downMs') + $koukuStageOptionalProperties) `
			'KoukuSaydon encounter pattern stage'
		if ($null -ne $koukuStage.PSObject.Properties['actions']) {
			if ($koukuStage.actions -isnot [Array] -or @($koukuStage.actions).Count -ne 1 -or
				$null -ne $koukuPattern.PSObject.Properties['bossMotion']) {
				throw 'KoukuSaydon stage actions require one retarget action and no BossMotion'
			}
			$koukuStageRetarget = $koukuStage.actions[0]
			Assert-ExactProperties $koukuStageRetarget @('trigger','kind','targetId','value','durationMs') 'KoukuSaydon stage retarget'
			foreach ($field in @('trigger','kind','targetId')) {
				Assert-JsonString $koukuStageRetarget.$field "KoukuSaydon stage retarget $field"
			}
			Assert-JsonInteger $koukuStageRetarget.value 'KoukuSaydon stage retarget value' 1 1
			Assert-JsonInteger $koukuStageRetarget.durationMs 'KoukuSaydon stage retarget durationMs' 0 0
			if ($koukuStageRetarget.trigger -cne 'ENTER' -or
				$koukuStageRetarget.kind -cne 'RETARGET_RANDOM_ALIVE' -or
				$koukuStageRetarget.targetId -cnotin @('boss.target.pattern','boss.target.nearest')) {
				throw 'KoukuSaydon stage retarget identity is invalid'
			}
		}
		foreach ($field in @(
			'stageId','actionId','stageKind','hitShape','serverDamageProfileId')) {
			Assert-JsonString $koukuStage.$field `
				"KoukuSaydon stage $field"
		}
		foreach ($field in @(
			'durationMs','hitCount','hitIntervalMs','hitDelayMs','pushMs','downMs')) {
			Assert-JsonInteger $koukuStage.$field `
				"KoukuSaydon stage $field" 0 ([uint32]::MaxValue)
		}
		foreach ($field in @(
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
			'hitHalfWidth','pushRangeM')) {
			Assert-JsonNumber $koukuStage.$field `
				"KoukuSaydon stage $field"
		}
		Assert-StableId $koukuStage.stageId 'KoukuSaydon stageId'
		Assert-StableId $koukuStage.actionId 'KoukuSaydon stage actionId'
		if ($koukuStage.knockdown -isnot [bool] -or
			-not $koukuStageIds.Add([string]$koukuStage.stageId) -or
			-not $koukuStageActionIds.Add([string]$koukuStage.actionId) -or
			[string]$koukuStage.stageKind -cnotin @(
				'WINDUP','ACTIVE','RECOVERY') -or
			[uint32]$koukuStage.durationMs -eq 0 -or
			[string]$koukuStage.hitShape -cne 'NONE' -or
			[double]$koukuStage.hitOuterRadius -ne 0.0 -or
			[double]$koukuStage.hitInnerRadius -ne 0.0 -or
			[double]$koukuStage.hitAngleDegrees -ne 0.0 -or
			[double]$koukuStage.hitLength -ne 0.0 -or
			[double]$koukuStage.hitHalfWidth -ne 0.0 -or
			[uint32]$koukuStage.hitCount -ne 0 -or
			[uint32]$koukuStage.hitIntervalMs -ne 0 -or
			[uint32]$koukuStage.hitDelayMs -ne 0 -or
			-not [string]::IsNullOrEmpty(
				[string]$koukuStage.serverDamageProfileId) -or
			[double]$koukuStage.pushRangeM -ne 0.0 -or
			[uint32]$koukuStage.pushMs -ne 0 -or
			[bool]$koukuStage.knockdown -or
			[uint32]$koukuStage.downMs -ne 0) {
			throw "KoukuSaydon stage must remain animation-only: $($koukuPattern.patternId)/$($koukuStage.stageId)"
		}
		$patternRows.Add((@(
			'PATTERNSTAGE', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, $stageIndex, $koukuStage.stageId,
			$koukuStage.actionId, $koukuStage.stageKind,
			[uint32]$koukuStage.durationMs, 'NONE', '0', '0', '0', '0',
			'0', 0, 0, 0, '-', '0', 0, 0, 0) -join "`t"))
		if ($null -ne $koukuStage.PSObject.Properties['rootMotionSamples']) {
			if ($koukuStage.rootMotionSamples -isnot [Array] -or
				$null -ne $koukuPattern.PSObject.Properties['bossMotion'] -or
				@($koukuPattern.logicWindows | Where-Object {
					$null -ne $_.PSObject.Properties['bossChargeDistanceM'] -and $_.bossChargeDistanceM -gt 0 }).Count -gt 0 -or
				@($koukuPattern.mechanicTriggers | Where-Object { $_.kind -ceq 'REAL_GAZE_TELEPORT' }).Count -gt 0) {
				throw 'KoukuSaydon animation root motion cannot share a Pattern with BossMotion, charge, or teleport'
			}
			$rootSamples = @($koukuStage.rootMotionSamples)
			$packedRoot = Format-RootMotionSamples -Samples $rootSamples `
				-SkillId "$($koukuPattern.patternId)/$($koukuStage.stageId)" -LimitMs $koukuStage.durationMs -IncludeUp
			$patternRows.Add((@('PATTERNSTAGEROOTMOTION', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $stageIndex, $rootSamples.Count, $packedRoot) -join "`t"))
		}
		if ($null -ne $koukuStage.PSObject.Properties['actions']) {
			$patternRows.Add((@(
				'PATTERNSTAGEACTION', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $koukuStage.actionId, 0,
				$koukuStageRetarget.trigger, $koukuStageRetarget.kind,
				$koukuStageRetarget.targetId, [uint32]$koukuStageRetarget.value,
				[uint32]$koukuStageRetarget.durationMs) -join "`t"))
		}
		$nextActionId = if ($stageIndex + 1 -lt @($koukuPattern.stages).Count) {
			[string]$koukuPattern.stages[$stageIndex + 1].actionId
		}
		else { '-' }
		$patternRows.Add((@(
			'PATTERNSTAGEBRANCH', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, $koukuStage.actionId, 'TIMEOUT',
			$nextActionId) -join "`t"))
	}

	# Pattern-clock lanes beside the stages: the judgement windows the Server
	# runs and the world sequence / scene profile cues it broadcasts. The
	# projector already derived every value; this only re-checks the Server row
	# grammar so a hand-edited Product cannot reach the bootstrap.
	$koukuPatternDurationMs = [uint64]0
	foreach ($koukuStage in @($koukuPattern.stages)) {
		$koukuPatternDurationMs += [uint64]$koukuStage.durationMs
	}
    if ($null -ne $koukuPattern.PSObject.Properties['parentPatternSequence']) {
        Assert-KoukuParentPatternSequence $koukuPattern @($koukuEncounterDocument.patterns)
        $parent = $koukuPattern.parentPatternSequence
        foreach ($entry in @($parent.entries)) {
            $isLoop = [int]([string]$entry.occurrenceId -ceq [string]$parent.loopStartOccurrenceId)
            $patternRows.Add((@('PATTERNPARENTCHILD', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                $entry.occurrenceId, $entry.patternId, $entry.startMs, $entry.durationMs, $isLoop) -join "`t"))
        }
    }
    if ($null -ne $koukuPattern.PSObject.Properties['timelineDurationMs']) {
        Assert-JsonInteger $koukuPattern.timelineDurationMs 'KoukuSaydon timelineDurationMs' 1 600000
        if ([uint64]$koukuPattern.timelineDurationMs -lt $koukuPatternDurationMs) { throw 'KoukuSaydon timelineDurationMs cannot trim Stage clocks' }
        $koukuPatternDurationMs = [uint64]$koukuPattern.timelineDurationMs
        $patternRows.Add((@('PATTERNTIMELINE', $koukuEncounterDocument.encounterId, $koukuPattern.patternId, $koukuPatternDurationMs) -join "`t"))
    }
	if ($null -ne $koukuPattern.PSObject.Properties['bossMotion']) {
		$bossMotion = $koukuPattern.bossMotion
		$motionProperties = @('startMs','endMs','startPosition','endPosition','yawDegrees')
		$hasMotionKeys = $null -ne $bossMotion.PSObject.Properties['keys']
		if ($hasMotionKeys) { $motionProperties += 'keys' }
		Assert-ExactProperties $bossMotion $motionProperties 'KoukuSaydon bossMotion'
		Assert-JsonInteger $bossMotion.startMs 'KoukuSaydon bossMotion startMs' 0 600000
		Assert-JsonInteger $bossMotion.endMs 'KoukuSaydon bossMotion endMs' 1 $koukuPatternDurationMs
		Assert-JsonNumber $bossMotion.yawDegrees 'KoukuSaydon bossMotion yawDegrees'
		if ($koukuPattern.resetBossToSpawn -or $null -ne $koukuPattern.PSObject.Properties['resetBossYawDegrees'] -or
			$bossMotion.startMs -ge $bossMotion.endMs -or [Math]::Abs([double]$bossMotion.yawDegrees) -gt 360) {
			throw 'KoukuSaydon bossMotion requires an ordered interval, valid yaw and no spawn reset'
		}
		foreach ($field in @('startPosition','endPosition')) {
			if ($bossMotion.$field -isnot [Array] -or @($bossMotion.$field).Count -ne 3) { throw "KoukuSaydon bossMotion $field needs XYZ" }
			foreach ($component in $bossMotion.$field) {
				Assert-JsonNumber $component "KoukuSaydon bossMotion $field"
				if ([Math]::Abs([double]$component) -gt 100000) { throw 'KoukuSaydon bossMotion position exceeds bounds' }
			}
		}
		if (-not $hasMotionKeys -and [double]$bossMotion.startPosition[1] -ne [double]$bossMotion.endPosition[1]) { throw 'KoukuSaydon bossMotion base Y must remain constant' }
		$motionKeyRows = [Collections.Generic.List[string]]::new()
		if ($hasMotionKeys) {
			if ($bossMotion.keys -isnot [Array] -or @($bossMotion.keys).Count -lt 2 -or @($bossMotion.keys).Count -gt 512) { throw 'Boss motion requires 2..512 keys' }
			$previousTime = -1
			$keyIndex = 0
			foreach ($key in $bossMotion.keys) {
				Assert-ExactProperties $key @('timeMs','position') 'Boss motion key'
				Assert-JsonInteger $key.timeMs 'Boss motion key time' $bossMotion.startMs $bossMotion.endMs
				if ($key.timeMs -le $previousTime -or $key.position -isnot [Array] -or @($key.position).Count -ne 3) { throw 'Boss motion keys require increasing times and XYZ' }
				$keyRow = @('PATTERNBOSSMOTIONKEY', $koukuEncounterDocument.encounterId, $koukuPattern.patternId, $keyIndex, $key.timeMs)
				foreach ($coordinate in $key.position) {
					Assert-JsonNumber $coordinate 'Boss motion key coordinate'
					if ([Math]::Abs([double]$coordinate) -gt 100000) { throw 'Boss motion key exceeds world bounds' }
					$keyRow += Format-InvariantSignedFloat $coordinate 'Boss motion key coordinate'
				}
				$motionKeyRows.Add(($keyRow -join "`t"))
				$previousTime = $key.timeMs
				++$keyIndex
			}
			$firstKey = $bossMotion.keys[0]; $lastKey = $bossMotion.keys[-1]
			if ($firstKey.timeMs -ne $bossMotion.startMs -or $lastKey.timeMs -ne $bossMotion.endMs) { throw 'Boss motion keys must match endpoint times' }
			for ($axis = 0; $axis -lt 3; ++$axis) {
				if ([double]$firstKey.position[$axis] -ne [double]$bossMotion.startPosition[$axis] -or
					[double]$lastKey.position[$axis] -ne [double]$bossMotion.endPosition[$axis]) { throw 'Boss motion keys must match endpoint positions' }
			}
		}
		if (@($koukuPattern.mechanicTriggers | Where-Object { $_.kind -cin @('REAL_GAZE_TELEPORT','BOSS_TELEPORT_XZ','BOSS_TELEPORT_GROUNDED') }).Count -gt 0) { throw 'KoukuSaydon bossMotion cannot also teleport the boss' }
		$bossMotionRow = @('PATTERNBOSSMOTION', $koukuEncounterDocument.encounterId, $koukuPattern.patternId, $bossMotion.startMs, $bossMotion.endMs)
		foreach ($component in @($bossMotion.startPosition) + @($bossMotion.endPosition) + @($bossMotion.yawDegrees)) {
			$bossMotionRow += Format-InvariantSignedFloat $component 'KoukuSaydon bossMotion'
		}
		$patternRows.Add(($bossMotionRow -join "`t"))
		foreach ($motionKeyRow in $motionKeyRows) { $patternRows.Add($motionKeyRow) }
	}
	if ($koukuPattern.logicWindows -isnot [Array] -or
		@($koukuPattern.logicWindows).Count -gt 128 -or
		$koukuPattern.worldSequences -isnot [Array] -or
		@($koukuPattern.worldSequences).Count -gt 128 -or
		$koukuPattern.sceneProfiles -isnot [Array] -or
		@($koukuPattern.sceneProfiles).Count -gt 16) {
		throw "KoukuSaydon pattern lanes are invalid: $($koukuPattern.patternId)"
	}
	$koukuWindowIds =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$koukuContactGroups = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
	$koukuHoldRows = [Collections.Generic.List[string]]::new()
	$koukuCaptureGrip = $null
	for ($windowIndex = 0;
		$windowIndex -lt @($koukuPattern.logicWindows).Count; ++$windowIndex) {
		$window = $koukuPattern.logicWindows[$windowIndex]
		$windowProperties = @(
			'windowId','kind','startMs','durationMs','sectorCount','sectorSymbols',
			'centerX','centerZ','outerRadiusM','stopYawDegrees','halfAngleDegrees',
			'maxDistanceM','poseIndex','threshold','shieldArcDegrees',
			'endsPatternOnSuccess','normalYawOffsetDegrees','insideOutcome','cardRegions','onSuccess','onFail','onTimeout')
		if ($null -ne $window.PSObject.Properties['bossChargeDistanceM']) { $windowProperties += 'bossChargeDistanceM' }
        if ($null -ne $window.PSObject.Properties['chargeYawOffsetDegrees']) { $windowProperties += 'chargeYawOffsetDegrees' }
		if ($null -ne $window.PSObject.Properties['rearmOnExit']) { $windowProperties += 'rearmOnExit' }
		if ($null -ne $window.PSObject.Properties['repeatAfterKnockback']) { $windowProperties += 'repeatAfterKnockback' }
		if ($null -ne $window.PSObject.Properties['repeatIntervalMs']) { $windowProperties += 'repeatIntervalMs' }
		if ($null -ne $window.PSObject.Properties['ownerWorldOccurrenceId']) { $windowProperties += 'ownerWorldOccurrenceId' }
		if ($null -ne $window.PSObject.Properties['holdLogicOccurrenceId']) { $windowProperties += 'holdLogicOccurrenceId' }
		if ($null -ne $window.PSObject.Properties['cancelAtEnd']) {
			$windowProperties += 'cancelAtEnd'
			if ($window.cancelAtEnd -isnot [bool]) { throw 'Logic cancelAtEnd must be boolean' }
		}
		if ($window.kind -ceq 'PATTERN_COMPLETION_COUNT') { $windowProperties += @('patternIds','completionCount') }
		if ($window.kind -ceq 'OBJECT_OVERLAP') { $windowProperties += @('targetWorldInstanceId','targetWorldX','targetWorldZ','targetRadiusM') }
		if ($window.kind -ceq 'OBJECT_CONTACT') { $windowProperties += @('contactTargets','contactGroupId','contactPriority') }
		Assert-ExactProperties $window $windowProperties 'KoukuSaydon logic window'
		Assert-JsonString $window.windowId 'KoukuSaydon logic window windowId'
		Assert-StableId $window.windowId 'KoukuSaydon logic window windowId'
		Assert-JsonString $window.kind 'KoukuSaydon logic window kind'
		foreach ($field in @('startMs','durationMs','sectorCount','poseIndex','threshold')) {
			Assert-JsonInteger $window.$field "KoukuSaydon logic window $field" `
				0 ([uint32]::MaxValue)
		}
		foreach ($field in @(
			'centerX','centerZ','outerRadiusM','stopYawDegrees','halfAngleDegrees',
			'maxDistanceM','shieldArcDegrees','normalYawOffsetDegrees')) {
			Assert-JsonNumber $window.$field "KoukuSaydon logic window $field"
		}
		$windowKind = [string]$window.kind
		$rearmOnExit = $false
		if ($null -ne $window.PSObject.Properties['rearmOnExit']) {
			if ($windowKind -cne 'ENTER_AREA' -or $window.rearmOnExit -isnot [bool]) {
				throw 'Only ENTER_AREA owns a boolean rearmOnExit'
			}
			$rearmOnExit = [bool]$window.rearmOnExit
		}
		if ($window.insideOutcome -cnotin @('SUCCESS','FAIL') -or ($window.insideOutcome -ceq 'FAIL' -and $windowKind -cnotin @('AREA_OVERLAP','OBJECT_OVERLAP','GAZE_REAL_BOSS'))) { throw 'KoukuSaydon insideOutcome is invalid' }
		$repeatAfterKnockback = $false
		if ($null -ne $window.PSObject.Properties['repeatAfterKnockback']) {
			if ($windowKind -cne 'ENTER_AREA' -or $window.repeatAfterKnockback -isnot [bool]) { throw 'Only ENTER_AREA owns boolean repeatAfterKnockback' }
			$repeatAfterKnockback = [bool]$window.repeatAfterKnockback
		}
		$repeatIntervalMs = 0
        if ($null -ne $window.PSObject.Properties['repeatIntervalMs']) {
            Assert-JsonInteger $window.repeatIntervalMs 'Collider tick interval' 0 600000
            $repeatIntervalMs = [uint32]$window.repeatIntervalMs
            if ($windowKind -cnotin @('ENTER_AREA','AREA_OVERLAP')) { throw 'Only ENTER_AREA or AREA_OVERLAP owns a tick interval' }
        }
        if (([int]$repeatAfterKnockback + [int]$rearmOnExit + [int]($repeatIntervalMs -gt 0)) -gt 1) { throw 'Choose one contact repeat policy' }
		if ($repeatAfterKnockback) {
			$repeatHits = @($window.onSuccess)
			if ($repeatHits.Count -ne 1 -or $repeatHits[0].kind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE') -or
				(($null -eq $repeatHits[0].PSObject.Properties['pushRangeM'] -or $repeatHits[0].pushRangeM -le 0) -and
                 ($null -eq $repeatHits[0].PSObject.Properties['pushHeightM'] -or $repeatHits[0].pushHeightM -le 0)) -or
				$null -eq $repeatHits[0].PSObject.Properties['pushMs'] -or $repeatHits[0].pushMs -le 0) { throw 'Repeat after knockback requires one damage Success with positive knockback' }
		}
		$insideFail = if ($window.insideOutcome -ceq 'FAIL') { 1 } else { 0 }
		$windowEndMs = [uint64]$window.startMs + [uint64]$window.durationMs
		if ($windowKind -cnotin @(
				'CARD_DICE_BIND','ROULETTE_CARD_MATCH','GAZE_REAL_BOSS','POSE_INPUT','STAGGER_WINDOW','COUNTER_WINDOW','AREA_OVERLAP','ENTER_AREA','OBJECT_OVERLAP','OBJECT_CONTACT','EXTERNAL_SIGNAL','ATTACHMENT_HOLD','PATTERN_COMPLETION_COUNT','INVULNERABILITY_ZONE','BINGO_COMPLETED_LINES') -or
			-not $koukuWindowIds.Add([string]$window.windowId) -or
			[uint32]$window.durationMs -eq 0 -or
			$windowEndMs -gt $koukuPatternDurationMs -or
			$window.endsPatternOnSuccess -isnot [bool] -or
			$window.sectorSymbols -isnot [Array] -or
			@($window.sectorSymbols).Count -ne [uint32]$window.sectorCount -or
			[uint32]$window.sectorCount -gt 64 -or
			[uint32]$window.poseIndex -gt 7 -or
			[double]$window.halfAngleDegrees -lt 0.0 -or
			[double]$window.halfAngleDegrees -gt 180.0 -or
			[double]$window.shieldArcDegrees -lt 0.0 -or
			[double]$window.shieldArcDegrees -gt 360.0 -or
			[double]$window.outerRadiusM -lt 0.0 -or
			[double]$window.maxDistanceM -lt 0.0 -or
			[double]$window.stopYawDegrees -lt 0.0 -or
			[double]$window.stopYawDegrees -ge 360.0 -or
			$window.onSuccess -isnot [Array] -or $window.onFail -isnot [Array] -or
			$window.onTimeout -isnot [Array] -or
			@($window.onSuccess).Count -gt 4 -or @($window.onFail).Count -gt 4 -or
			@($window.onTimeout).Count -gt 4) {
			throw "KoukuSaydon logic window is invalid: $($koukuPattern.patternId)/$($window.windowId)"
		}
		if ($windowKind -ceq 'PATTERN_COMPLETION_COUNT') {
			Assert-JsonInteger $window.completionCount 'Pattern completionCount' 1 16
			if ($window.patternIds -isnot [Array] -or @($window.patternIds).Count -lt $window.completionCount -or
				@($window.patternIds).Count -gt 16 -or @($window.onSuccess).Count -gt 1 -or
				(@($window.onSuccess).Count -eq 1 -and $window.onSuccess[0].kind -cne 'FOLLOWUP_PATTERN') -or
				@($window.onFail).Count -ne 0 -or @($window.onTimeout).Count -ne 0 -or @($window.cardRegions).Count -ne 0) { throw 'Pattern completion chain requires candidates, optional Success followup, and no Collider/Fail/Timeout' }
			$chainIds = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			foreach ($candidate in $window.patternIds) {
				Assert-StableId $candidate 'Pattern completion candidate'
				if ($candidate -ceq $koukuPattern.patternId -or -not $chainIds.Add($candidate)) { throw 'Pattern chain self reference or duplicate' }
				$koukuFollowupTargets.Add($candidate)
			}
		}
		# Kind rules mirror CKoukuSaydonBrain::Validate_AnimationOnlyPattern.
		if ($windowKind -ceq 'INVULNERABILITY_ZONE' -and (@($window.onSuccess).Count -ne 0 -or
			@($window.onFail).Count -ne 0 -or @($window.onTimeout).Count -ne 0 -or $window.endsPatternOnSuccess)) {
			throw 'INVULNERABILITY_ZONE requires collider regions and has no outcomes or Pattern completion'
		}
		if ($windowKind -ceq 'ATTACHMENT_HOLD' -and (@($window.onSuccess).Count -ne 0 -or
			@($window.onFail).Count -ne 0 -or @($window.onTimeout).Count -ne 0 -or
			@($window.cardRegions).Count -ne 0 -or $window.endsPatternOnSuccess)) {
			throw 'ATTACHMENT_HOLD has no regions or outcomes and cannot end the Pattern'
		}
		if ($null -ne $window.PSObject.Properties['holdLogicOccurrenceId']) {
			Assert-JsonString $window.holdLogicOccurrenceId 'Capture Hold window ID'
			Assert-StableId $window.holdLogicOccurrenceId 'Capture Hold window ID'
			$holds = @($koukuPattern.logicWindows | Where-Object { $_.windowId -ceq $window.holdLogicOccurrenceId })
			if ($windowKind -cne 'ENTER_AREA' -or $holds.Count -ne 1 -or $holds[0].kind -cne 'ATTACHMENT_HOLD' -or
				$holds[0].startMs -gt $window.startMs -or ([uint64]$holds[0].startMs + [uint64]$holds[0].durationMs) -lt $windowEndMs) {
				throw 'Capture Hold must be a same-pattern ATTACHMENT_HOLD covering the complete Trigger window'
			}
			$koukuHoldRows.Add((@('PATTERNLOGICHOLD', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
				$window.windowId, $window.holdLogicOccurrenceId) -join "`t"))
		}
		if ($windowKind -ceq 'GAZE_REAL_BOSS' -and
			@($window.onTimeout).Count -ne 0) {
			throw "KoukuSaydon end-tick window cannot carry a Timeout outcome: $($window.windowId)"
		}
		if ($windowKind -cin @('STAGGER_WINDOW','COUNTER_WINDOW','ENTER_AREA','OBJECT_CONTACT','EXTERNAL_SIGNAL') -and @($window.onFail).Count -ne 0) {
			throw "KoukuSaydon window cannot carry a Fail outcome: $($window.windowId)"
		}
		if ($windowKind -ceq 'OBJECT_CONTACT' -and @($window.onTimeout).Count -ne 0) { throw 'OBJECT_CONTACT has no Timeout results' }
		if ($windowKind -ceq 'OBJECT_CONTACT' -and @($window.onSuccess | Where-Object { $_.kind -ceq 'PLAY_CONTACT_WORLD_OBJECT_MOTION' }).Count -gt 1) { throw 'OBJECT_CONTACT takes at most one contact motion result' }
		if ($windowKind -cnotin @('STAGGER_WINDOW','COUNTER_WINDOW','EXTERNAL_SIGNAL','PATTERN_COMPLETION_COUNT') -and $window.endsPatternOnSuccess) { throw 'Only stagger or external signal may end the pattern on success' }
		if ($windowKind -ceq 'ROULETTE_CARD_MATCH' -and @($window.cardRegions).Count -ne 8) {
			throw "KoukuSaydon roulette window needs eight explicit regions: $($window.windowId)"
		}
		if ($windowKind -ceq 'STAGGER_WINDOW' -and [uint32]$window.threshold -eq 0) {
			throw "KoukuSaydon stagger window needs a threshold: $($window.windowId)"
		}
		if ($windowKind -ceq 'BINGO_COMPLETED_LINES' -and ($window.threshold -lt 1 -or $window.threshold -gt 10 -or
			@($window.cardRegions).Count -ne 0 -or @($window.onTimeout).Count -ne 0)) {
			throw 'Bingo line judgement needs 1..10 complete rows/columns and no Collider or Timeout'
		}
		$symbolText = '-'
		if (@($window.sectorSymbols).Count -gt 0) {
			foreach ($symbol in @($window.sectorSymbols)) {
				Assert-JsonString $symbol 'KoukuSaydon logic window sectorSymbol'
				if ([string]$symbol -cnotin @('HEART','SPADE','CLUB','DIAMOND')) {
					throw "KoukuSaydon logic window sector symbol is invalid: $($window.windowId)"
				}
			}
			$symbolText = @($window.sectorSymbols | ForEach-Object { [string]$_ }) -join ','
		}
		$objectTargetFields = @()
		if ($windowKind -ceq 'OBJECT_OVERLAP') {
			Assert-StableId $window.targetWorldInstanceId 'OBJECT_OVERLAP target instance'
			foreach ($field in @('targetWorldX','targetWorldZ','targetRadiusM')) { Assert-JsonNumber $window.$field "OBJECT_OVERLAP $field" }
			if ([Math]::Abs([double]$window.targetWorldX) -gt 100000 -or [Math]::Abs([double]$window.targetWorldZ) -gt 100000 -or
				[double]$window.targetRadiusM -lt .01 -or [double]$window.targetRadiusM -gt 1000) { throw 'OBJECT_OVERLAP target circle is invalid' }
			$objectTargetFields = @($window.targetWorldInstanceId,
				(Format-InvariantSignedFloat $window.targetWorldX 'OBJECT_OVERLAP target X'),
				(Format-InvariantSignedFloat $window.targetWorldZ 'OBJECT_OVERLAP target Z'),
				(Format-InvariantFloat $window.targetRadiusM 'OBJECT_OVERLAP target radius'))
		}
		$endsPatternFlag = if ([bool]$window.endsPatternOnSuccess) { 1 } else { 0 }
		$patternRows.Add((@(
			'PATTERNLOGIC', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, $windowIndex, $window.windowId, $windowKind,
			[uint32]$window.startMs, [uint32]$window.durationMs,
			[uint32]$window.sectorCount,
			(Format-InvariantSignedFloat $window.centerX 'KoukuSaydon logic window centerX'),
			(Format-InvariantSignedFloat $window.centerZ 'KoukuSaydon logic window centerZ'),
			(Format-InvariantFloat $window.outerRadiusM 'KoukuSaydon logic window outerRadiusM'),
			(Format-InvariantFloat $window.stopYawDegrees 'KoukuSaydon logic window stopYawDegrees'),
			(Format-InvariantFloat $window.halfAngleDegrees 'KoukuSaydon logic window halfAngleDegrees'),
			(Format-InvariantFloat $window.maxDistanceM 'KoukuSaydon logic window maxDistanceM'),
			[uint32]$window.poseIndex, [uint32]$window.threshold,
			(Format-InvariantFloat $window.shieldArcDegrees 'KoukuSaydon logic window shieldArcDegrees'),
			$endsPatternFlag, $symbolText, (Format-InvariantSignedFloat $window.normalYawOffsetDegrees 'KoukuSaydon shield normal offset'), $insideFail) + $objectTargetFields -join "`t"))
		if ($windowKind -ceq 'PATTERN_COMPLETION_COUNT') {
			$patternRows.Add((@('PATTERNLOGICCHAIN', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $window.windowId, $window.completionCount) + @($window.patternIds) -join "`t"))
		}
		if ($null -ne $window.PSObject.Properties['cancelAtEnd'] -and $window.cancelAtEnd) {
			$patternRows.Add((@('PATTERNLOGICCANCEL', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $window.windowId) -join "`t"))
		}
        if ($repeatIntervalMs -gt 0) {
            $patternRows.Add((@('PATTERNLOGICTICK', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                $window.windowId, $repeatIntervalMs) -join "`t"))
        }
        if ($null -ne $window.PSObject.Properties['ownerWorldOccurrenceId']) {
            Assert-StableId $window.ownerWorldOccurrenceId 'Contact owner WORLD occurrence'
            $contactOwners = @($koukuPattern.worldSequences | Where-Object { $_.occurrenceId -ceq $window.ownerWorldOccurrenceId })
            if ($windowKind -cnotin @('AREA_OVERLAP','ENTER_AREA') -or $contactOwners.Count -ne 1 -or
                $null -eq $contactOwners[0].PSObject.Properties['combatBody']) { throw 'Contact owner needs its exact damageable WORLD occurrence' }
            $patternRows.Add((@('PATTERNLOGICWORLDOWNER', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                $window.windowId, $window.ownerWorldOccurrenceId) -join "`t"))
        }
		if ($rearmOnExit -or $repeatAfterKnockback) {
			$repeatMode = if ($repeatAfterKnockback) { 'AFTER_KNOCKBACK' } else { 'ON_REENTER' }
			$patternRows.Add((@('PATTERNLOGICREARM', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $window.windowId, $repeatMode) -join "`t"))
		}
        $chargeYawOffset = 0.0
        if ($null -ne $window.PSObject.Properties['chargeYawOffsetDegrees']) {
            Assert-JsonNumber $window.chargeYawOffsetDegrees 'Boss charge yaw offset'
            if ([Math]::Abs([double]$window.chargeYawOffsetDegrees) -gt 360 -or
                $null -eq $window.PSObject.Properties['bossChargeDistanceM']) { throw 'Charge yaw needs a charge distance and -360..360 degrees' }
            $chargeYawOffset = [double]$window.chargeYawOffsetDegrees
        }
        if ($null -ne $window.PSObject.Properties['bossChargeDistanceM']) {
            Assert-JsonNumber $window.bossChargeDistanceM 'Boss charge distance'
            if ($windowKind -cne 'ENTER_AREA' -or $window.bossChargeDistanceM -le 0 -or $window.bossChargeDistanceM -gt 1000 -or
                $null -ne $koukuPattern.PSObject.Properties['bossMotion']) { throw 'Boss charge needs ENTER_AREA and 0..1000 m without absolute bossMotion' }
            $patternRows.Add((@('PATTERNLOGICCHARGE', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                $window.windowId, (Format-InvariantFloat $window.bossChargeDistanceM 'Boss charge distance'),
                (Format-InvariantSignedFloat $chargeYawOffset 'Boss charge yaw offset')) -join "`t"))
        }
		$contactTargetIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		if ($windowKind -ceq 'OBJECT_CONTACT') {
			Assert-JsonString $window.contactGroupId 'OBJECT_CONTACT group ID'
			Assert-JsonInteger $window.contactPriority 'OBJECT_CONTACT priority' 0 1000
			$groupText = '-'
			if (-not [string]::IsNullOrEmpty($window.contactGroupId)) {
				Assert-StableId $window.contactGroupId 'OBJECT_CONTACT group ID'
				$groupText = $window.contactGroupId
				$groupKey = $groupText + '/' + [string][Math]::Ceiling([double]$window.startMs * 30 / 1000)
				if (-not $koukuContactGroups.ContainsKey($groupKey)) {
					$koukuContactGroups.Add($groupKey, @{ Start = $window.startMs; Duration = $window.durationMs; Priorities = [Collections.Generic.HashSet[int]]::new() })
				}
				$group = $koukuContactGroups[$groupKey]
				if ($group.Start -ne $window.startMs -or $group.Duration -ne $window.durationMs -or -not $group.Priorities.Add([int]$window.contactPriority)) {
					throw 'OBJECT_CONTACT group needs identical timing and distinct priorities'
				}
			}
			$patternRows.Add((@('PATTERNLOGICCONTACTGROUP',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
				$window.windowId,$groupText,$window.contactPriority) -join "`t"))
			if ($window.contactTargets -isnot [Array] -or @($window.contactTargets).Count -eq 0 -or @($window.contactTargets).Count -gt 64) { throw 'OBJECT_CONTACT needs 1..64 targets' }
			foreach ($target in @($window.contactTargets)) {
				Assert-ExactProperties $target @('targetWorldOccurrenceId','targetWorldInstanceId','targetWorldX','targetWorldZ','targetRadiusM') 'OBJECT_CONTACT target'
				Assert-JsonString $target.targetWorldOccurrenceId 'OBJECT_CONTACT target occurrence'
				Assert-StableId $target.targetWorldOccurrenceId 'OBJECT_CONTACT target occurrence'
				Assert-JsonString $target.targetWorldInstanceId 'OBJECT_CONTACT target instance'
				Assert-StableId $target.targetWorldInstanceId 'OBJECT_CONTACT target instance'
				foreach ($field in @('targetWorldX','targetWorldZ','targetRadiusM')) { Assert-JsonNumber $target.$field "OBJECT_CONTACT $field" }
				if (-not $contactTargetIds.Add([string]$target.targetWorldOccurrenceId) -or
					[Math]::Abs([double]$target.targetWorldX) -gt 100000 -or [Math]::Abs([double]$target.targetWorldZ) -gt 100000 -or
					[double]$target.targetRadiusM -lt .01 -or [double]$target.targetRadiusM -gt 1000) { throw 'OBJECT_CONTACT target identity/circle is invalid' }
				$cues = @($koukuPattern.worldSequences | Where-Object { $_.occurrenceId -ceq $target.targetWorldOccurrenceId })
				if ($cues.Count -ne 1 -or $cues[0].sequenceInstanceId -cne $target.targetWorldInstanceId -or
					$cues[0].anchorKind -cne 'NONE' -or $cues[0].startMs -gt $window.startMs -or
					([uint64]$cues[0].startMs + [uint64]$cues[0].durationMs) -lt $windowEndMs) { throw 'OBJECT_CONTACT target must be an owned absolute WORLD cue covering the trigger' }
				$patternRows.Add((@('PATTERNLOGICCONTACTTARGET',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
					$window.windowId,$target.targetWorldOccurrenceId,$target.targetWorldInstanceId,
					(Format-InvariantSignedFloat $target.targetWorldX 'OBJECT_CONTACT target X'),
					(Format-InvariantSignedFloat $target.targetWorldZ 'OBJECT_CONTACT target Z'),
					(Format-InvariantFloat $target.targetRadiusM 'OBJECT_CONTACT radius')) -join "`t"))
			}
		}
		if ($window.cardRegions -isnot [Array] -or @($window.cardRegions).Count -gt 64 -or
			($windowKind -cin @('AREA_OVERLAP','ENTER_AREA','OBJECT_OVERLAP','OBJECT_CONTACT','INVULNERABILITY_ZONE') -and @($window.cardRegions).Count -eq 0)) {
			throw 'KoukuSaydon collider region list is invalid'
		}
		$regionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		$regionCards = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		for ($regionOrdinal = 0; $regionOrdinal -lt @($window.cardRegions).Count; ++$regionOrdinal) {
			$region = $window.cardRegions[$regionOrdinal]
			$regionFields = @('regionId','shape','anchorKind','center','yawDegrees','halfExtents','radiusM','halfAngleDegrees','cardSymbol','cardColor')
			$hasSectorAxes = $region.PSObject.Properties.Name -contains 'radiusXM'
			if ($hasSectorAxes -ne ($region.PSObject.Properties.Name -contains 'radiusZM')) { throw 'Sector axes must be supplied together' }
			if ($hasSectorAxes) { $regionFields += @('radiusXM','radiusZM') }
			$hasInnerRadius = $region.PSObject.Properties.Name -contains 'innerRadiusM'
			if ($hasInnerRadius) { $regionFields += 'innerRadiusM' }
			$hasWorldTrack = $region.PSObject.Properties.Name -contains 'worldTrack'
			if ($hasWorldTrack) { $regionFields += 'worldTrack' }
            $hasLinearMotion = $region.PSObject.Properties.Name -contains 'linearMotion'
            if ($hasLinearMotion) { $regionFields += 'linearMotion' }
            $hasAnchorCapture = $region.PSObject.Properties.Name -contains 'captureStartMs'
            if ($hasAnchorCapture) { $regionFields += 'captureStartMs' }
			if ($windowKind -ceq 'INVULNERABILITY_ZONE' -and ($region.anchorKind -cne 'WORLD' -or $hasWorldTrack)) {
				throw 'INVULNERABILITY_ZONE requires fixed WORLD collider regions'
			}
			Assert-ExactProperties $region $regionFields 'KoukuSaydon collider region'
			Assert-StableId $region.regionId 'KoukuSaydon region ID'
			if (-not $regionIds.Add([string]$region.regionId) -or $region.shape -cnotin @('BOX','SECTOR','REVERSE_SECTOR','CIRCLE','CYLINDER') -or
				$region.anchorKind -cnotin @('WORLD','BOSS_CURRENT','BOSS_SPAWN','BOSS_START') -or
				$region.cardSymbol -cnotin @('NONE','HEART','SPADE','CLUB','DIAMOND') -or
				$region.cardColor -cnotin @('NONE','RED','BLACK') -or
				(($region.cardSymbol -ceq 'NONE') -ne ($region.cardColor -ceq 'NONE')) -or
				@($region.center).Count -ne 3 -or @($region.halfExtents).Count -ne 3) {
				throw 'KoukuSaydon collider region identity, mapping or geometry is invalid'
			}
			$regionNumbers = @($region.center) + @($region.yawDegrees) + @($region.halfExtents) + @($region.radiusM,$region.halfAngleDegrees)
			foreach ($number in $regionNumbers) { Assert-JsonNumber $number 'KoukuSaydon region geometry' }
			if ([double]$region.radiusM -le 0 -or [double]$region.halfAngleDegrees -lt 0 -or ($region.shape -cne 'REVERSE_SECTOR' -and [double]$region.halfAngleDegrees -eq 0) -or [double]$region.halfAngleDegrees -gt 180 -or
				@($region.halfExtents | Where-Object { [double]$_ -le 0 }).Count -gt 0) { throw 'KoukuSaydon region dimensions must be positive' }
			if ($windowKind -ceq 'ROULETTE_CARD_MATCH') {
				if ($region.cardSymbol -ceq 'NONE' -or -not $regionCards.Add("$($region.cardSymbol)/$($region.cardColor)")) {
					throw 'KoukuSaydon roulette needs eight different suit/color pairs'
				}
			}
			$axisFields = @()
			if ($hasSectorAxes) {
				Assert-JsonNumber $region.radiusXM 'Sector radiusXM'
				Assert-JsonNumber $region.radiusZM 'Sector radiusZM'
				if ($region.shape -cnotin @('SECTOR','REVERSE_SECTOR') -or $region.radiusXM -le 0 -or $region.radiusZM -le 0) { throw 'Sector axes require positive radii' }
				$axisFields = @((Format-InvariantFloat $region.radiusXM 'Sector radiusXM'), (Format-InvariantFloat $region.radiusZM 'Sector radiusZM'))
			}
			$innerFields = @()
			if ($hasInnerRadius) {
				Assert-JsonNumber $region.innerRadiusM 'Collider innerRadiusM'
				if ($region.innerRadiusM -lt 0 -or $region.innerRadiusM -ge $region.radiusM -or
					($region.innerRadiusM -gt 0 -and ($region.shape -cnotin @('CIRCLE','SECTOR') -or
					 ($hasSectorAxes -and ([math]::Abs($region.radiusXM-$region.radiusM) -gt .0001 -or [math]::Abs($region.radiusZM-$region.radiusM) -gt .0001))))) {
					throw 'Collider inner radius requires an exact circular ring or annular sector'
				}
				$innerFields = @((Format-InvariantFloat $region.innerRadiusM 'Collider innerRadiusM'))
			}
			$formattedRegion = @($regionNumbers | ForEach-Object { Format-InvariantSignedFloat $_ 'KoukuSaydon region geometry' })
			$patternRows.Add((@('PATTERNLOGICREGION',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
				$window.windowId,$regionOrdinal,$region.regionId,$region.anchorKind,$region.shape) + $formattedRegion + @($region.cardSymbol,$region.cardColor) + $axisFields + $innerFields -join "`t"))
            if ($hasAnchorCapture) {
                Assert-JsonInteger $region.captureStartMs 'Collider captureStartMs'
                if ($region.anchorKind -cne 'BOSS_START' -or $hasWorldTrack -or
                    $windowKind -cnotin @('ENTER_AREA','AREA_OVERLAP','OBJECT_OVERLAP') -or
                    $region.captureStartMs -lt 0 -or $region.captureStartMs -gt $window.startMs) { throw 'Collider shared anchor must precede a fixed BOSS overlap window' }
                $patternRows.Add((@('PATTERNLOGICREGIONCAPTURE', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                    $window.windowId, $regionOrdinal, $region.captureStartMs) -join "`t"))
            }
            if ($hasLinearMotion) {
                $motion = $region.linearMotion
                Assert-ExactProperties $motion @('endPositionOffset','endScale') 'Collider linear motion'
                if ($windowKind -cnotin @('ENTER_AREA','AREA_OVERLAP','OBJECT_OVERLAP','OBJECT_CONTACT') -or
                    @($motion.endPositionOffset).Count -ne 3 -or @($motion.endScale).Count -ne 3) { throw 'Collider motion requires overlap and XYZ end values' }
                foreach ($number in @($motion.endPositionOffset)) {
                    Assert-JsonNumber $number 'Collider end position'
                    if ([Math]::Abs([double]$number) -gt 200000) { throw 'Collider end position exceeds bounds' }
                }
                foreach ($number in @($motion.endScale)) {
                    Assert-JsonNumber $number 'Collider end scale'
                    if ([double]$number -le 0 -or [double]$number -gt 10000000) { throw 'Collider end size must be positive' }
                }
                if (($region.shape -cin @('CIRCLE','CYLINDER') -or ($hasInnerRadius -and $region.innerRadiusM -gt 0)) -and
                    [Math]::Abs([double]$motion.endScale[0] - [double]$motion.endScale[2]) -gt .0001) { throw 'Circular Collider motion requires equal X/Z size ratios' }
                $motionNumbers = @($motion.endPositionOffset) + @($motion.endScale)
                $motionText = @($motionNumbers | ForEach-Object { Format-InvariantSignedFloat $_ 'Collider linear motion' })
                $patternRows.Add((@('PATTERNLOGICREGIONMOTION', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
                    $window.windowId, $regionOrdinal) + $motionText -join "`t"))
            }
			if ($hasWorldTrack) {
				$track = $region.worldTrack
				Assert-ExactProperties $track @('startMs','startDelayMs','durationMs','playbackSpeed','interpolation','baselinePosition','baselineYawDegrees','baselineScale','keys') 'Collider WORLD track'
				$isBossBoneTrack = $region.anchorKind -ceq 'BOSS_CURRENT'
				if ($windowKind -cnotin @('ENTER_AREA','AREA_OVERLAP','OBJECT_OVERLAP','OBJECT_CONTACT') -or ($isBossBoneTrack -and $windowKind -cnotin @('OBJECT_CONTACT','ENTER_AREA','AREA_OVERLAP')) -or
					$track.interpolation -cnotin @('LINEAR','SMOOTH_STEP') -or
					@($track.baselinePosition).Count -ne 3 -or @($track.baselineScale).Count -ne 3 -or
					$track.keys -isnot [Array] -or @($track.keys).Count -eq 0 -or @($track.keys).Count -gt 4096) { throw 'Collider WORLD track is invalid' }
				Assert-JsonInteger $track.startMs 'Collider WORLD startMs' 0 600000
				Assert-JsonInteger $track.startDelayMs 'Collider WORLD startDelayMs' 0 600000
				Assert-JsonInteger $track.durationMs 'Collider WORLD durationMs' 1 600000
				Assert-JsonNumber $track.playbackSpeed 'Collider WORLD playbackSpeed'
				if ($track.startMs -gt $window.startMs -or $track.playbackSpeed -le 0 -or
					@($track.baselineScale | Where-Object { [double]$_ -le 0 }).Count -gt 0 -or
					($region.shape -cne 'BOX' -and [Math]::Abs([double]$track.baselineScale[0]-[double]$track.baselineScale[2]) -gt 0.0001)) { throw 'Collider WORLD clock/scale is invalid' }
				$baselineNumbers = @($track.baselinePosition) + @($track.baselineYawDegrees) + @($track.baselineScale)
				foreach ($number in $baselineNumbers) { Assert-JsonNumber $number 'Collider WORLD baseline' }
				if ($isBossBoneTrack -and ($track.startMs -ne $window.startMs -or $track.durationMs -ne $window.durationMs -or
					$track.startDelayMs -ne 0 -or $track.playbackSpeed -ne 1 -or $track.interpolation -cne 'LINEAR' -or
					$track.baselineYawDegrees -ne 0 -or @($track.baselinePosition | Where-Object { [double]$_ -ne 0 }).Count -ne 0 -or
					@($track.baselineScale | Where-Object { [double]$_ -ne 1 }).Count -ne 0 -or
					$track.keys[0].timeMs -ne 0 -or $track.keys[-1].timeMs -ne $track.durationMs)) { throw 'Boss Bone Collider track must use the exact contact clock and identity baseline' }
				$baselineText = @($baselineNumbers | ForEach-Object { Format-InvariantSignedFloat $_ 'Collider WORLD baseline' })
				$smoothFlag = if ($track.interpolation -ceq 'SMOOTH_STEP') { 1 } else { 0 }
				$patternRows.Add((@('PATTERNLOGICREGIONWORLD',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
					$window.windowId,$region.regionId,$track.startMs,$track.startDelayMs,$track.durationMs,
					(Format-InvariantFloat $track.playbackSpeed 'Collider WORLD speed'),$smoothFlag) + $baselineText -join "`t"))
				$lastKeyTime = -1
				$trackKeys = @($track.keys)
				$trackHasGrip = $null -ne $trackKeys[0].PSObject.Properties['gripPosition']
				for ($keyOrdinal=0; $keyOrdinal -lt $trackKeys.Count; ++$keyOrdinal) {
					$key = $trackKeys[$keyOrdinal]
					$keyFields = @('timeMs','positionOffset','rotationY','rotationW','scaleMultiplier','visible')
					$hasGrip = $null -ne $key.PSObject.Properties['gripPosition']
					if ($hasGrip) { $keyFields += 'gripPosition' }
					Assert-ExactProperties $key $keyFields 'Collider WORLD key'
					Assert-JsonInteger $key.timeMs 'Collider WORLD key timeMs' 0 $track.durationMs
					if ($key.timeMs -le $lastKeyTime -or @($key.positionOffset).Count -ne 3 -or @($key.scaleMultiplier).Count -ne 3 -or $key.visible -isnot [bool]) { throw 'Collider WORLD key is invalid' }
					$lastKeyTime = $key.timeMs
					$keyNumbers = @($key.positionOffset) + @($key.rotationY,$key.rotationW) + @($key.scaleMultiplier)
					$keyText = Format-JsonSignedNumbers $keyNumbers 'Collider WORLD key geometry'
					# BONE bakes local TRS and sampled yaw; the unit-quaternion guard below applies to both anchor modes.
					if ($isBossBoneTrack -and (-not $key.visible -or
						[double]$key.scaleMultiplier[0] -ne 1 -or [double]$key.scaleMultiplier[1] -ne 1 -or
						[double]$key.scaleMultiplier[2] -ne 1)) { throw 'Boss Bone Collider keys must remain visible and preserve authored scale' }
					if ([double]$key.scaleMultiplier[0] -lt 0 -or [double]$key.scaleMultiplier[1] -lt 0 -or
						[double]$key.scaleMultiplier[2] -lt 0 -or
						($region.shape -cne 'BOX' -and [Math]::Abs([double]$key.scaleMultiplier[0]-[double]$key.scaleMultiplier[2]) -gt 0.0001) -or
						[Math]::Abs([double]$key.rotationY*[double]$key.rotationY+[double]$key.rotationW*[double]$key.rotationW-1) -gt 0.001) { throw 'Collider WORLD quaternion/scale is invalid' }
					$gripFields = @()
					if ($hasGrip) {
                        if ($keyOrdinal -eq 0 -and ($region.anchorKind -cne 'WORLD' -or $region.shape -cne 'BOX' -or $windowKind -cne 'ENTER_AREA' -or
                            @($window.onSuccess).Count -ne 1 -or $window.onSuccess[0].kind -cne 'GRAB_TO_WORLD_OBJECT' -or
                            @($window.onFail).Count -or @($window.onTimeout).Count -or
                            $track.startMs -ne $window.startMs -or $track.durationMs -lt $window.durationMs -or $track.startDelayMs -ne 0 -or
                            $track.playbackSpeed -ne 1 -or $track.interpolation -cne 'LINEAR' -or $track.baselineYawDegrees -ne 0 -or
                            @($track.baselinePosition | Where-Object { [double]$_ -ne 0 }).Count -or
                            @($track.baselineScale | Where-Object { [double]$_ -ne 1 }).Count -or
                            $track.keys[0].timeMs -ne 0 -or $track.keys[-1].timeMs -ne $track.durationMs)) { throw 'Physical grip needs an exact world hook BOX track' }
                        if (@($key.gripPosition).Count -ne 3) { throw 'Physical grip needs an exact world hook BOX track' }
                        $gripFields = Format-JsonSignedNumbers $key.gripPosition 'Hook grip XYZ'
                    }
                    if ($hasGrip -ne $trackHasGrip) { throw 'Hook grip positions must exist on every key' }
					$visibleFlag = if ($key.visible) { 1 } else { 0 }
					$patternRows.Add((@('PATTERNLOGICREGIONWORLDKEY',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
						$window.windowId,$region.regionId,$keyOrdinal,$key.timeMs) + $keyText + @($visibleFlag) + $gripFields -join "`t"))
				}
			}
		}
		foreach ($slotName in @('SUCCESS','FAIL','TIMEOUT')) {
			$slotProperty = 'onTimeout'
			if ($slotName -ceq 'SUCCESS') { $slotProperty = 'onSuccess' }
			elseif ($slotName -ceq 'FAIL') { $slotProperty = 'onFail' }
			$outcomes = @($window.$slotProperty)
			for ($ordinal = 0; $ordinal -lt $outcomes.Count; ++$ordinal) {
				$outcome = $outcomes[$ordinal]
				$outcomeProperties = @('kind','percent','durationMs','patternId')
				if ($outcome.kind -ceq 'FIXED_DAMAGE') { $outcomeProperties += 'damageAmount' }
				foreach ($field in @('pushRangeM','pushMs','pushDirection','forcePush','pushCanLeaveArena','pushBallistic','pushHeightM','pushYawOffsetDegrees')) {
					if ($null -ne $outcome.PSObject.Properties[$field]) { $outcomeProperties += $field }
				}
				if ($outcome.kind -ceq 'FEAR') { $outcomeProperties += 'presentationId' }
				if ($outcome.kind -ceq 'MARIO_ENTER' -and $null -ne $outcome.PSObject.Properties['marioStage']) { $outcomeProperties += 'marioStage' }
				if ($outcome.kind -ceq 'CAPTURE_PLAYER') { $outcomeProperties += @('attachmentSlot','gripLocalOffset') }
				if ($outcome.kind -ceq 'PLAY_WORLD_OBJECT_MOTION') { $outcomeProperties += @('targetWorldInstanceId','motionInstanceId') }
				if ($outcome.kind -ceq 'PLAY_CONTACT_WORLD_OBJECT_MOTION') { $outcomeProperties += 'contactMotions' }
				if ($outcome.kind -ceq 'COMPLETE_LOGIC_WINDOW') { $outcomeProperties += @('targetLogicOccurrenceId','contactTargetWorldOccurrenceId') }
				Assert-ExactProperties $outcome $outcomeProperties 'KoukuSaydon logic outcome'
				Assert-JsonString $outcome.kind 'KoukuSaydon logic outcome kind'
				Assert-JsonInteger $outcome.percent 'KoukuSaydon logic outcome percent' 0 100
				Assert-JsonInteger $outcome.durationMs `
					'KoukuSaydon logic outcome durationMs' 0 600000
				if ($outcome.patternId -isnot [string]) {
					throw "KoukuSaydon logic outcome patternId must be text: $($window.windowId)"
				}
				$outcomeKind = [string]$outcome.kind
				$outcomePushRangeM = 0.0
				$outcomePushMs = 0
				$outcomePushDirection = 'AWAY_FROM_BOSS'
				$outcomePushBallistic = $false
				if ($null -ne $outcome.PSObject.Properties['pushBallistic']) {
					if ($outcome.pushBallistic -isnot [bool]) { throw 'pushBallistic must be Boolean' }
					$outcomePushBallistic = [bool]$outcome.pushBallistic
				}
                $outcomePushHeightM = 0.0
                if ($null -ne $outcome.PSObject.Properties['pushHeightM']) {
                    Assert-JsonNumber $outcome.pushHeightM 'Ballistic height'
                    $outcomePushHeightM = [double]$outcome.pushHeightM
                    if ($outcomePushHeightM -lt 0 -or $outcomePushHeightM -gt 100 -or
                        ($outcomePushHeightM -gt 0 -and -not $outcomePushBallistic) -or $outcomeKind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE')) { throw 'Ballistic height requires damage push and height 0..100m' }
                }
				$outcomePushRangeLimit = $(if ($outcomePushBallistic) { 100 } else { 20 })
				if ($null -ne $outcome.PSObject.Properties['pushRangeM'] -or $null -ne $outcome.PSObject.Properties['pushMs']) {
					if ($outcomeKind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE')) { throw 'Only MAX_HP_PERCENT_DAMAGE owns push values' }
					if (($null -ne $outcome.PSObject.Properties['pushRangeM']) -ne ($null -ne $outcome.PSObject.Properties['pushMs'])) {
						throw 'KoukuSaydon outcome pushRangeM and pushMs must be supplied together'
					}
					if ($null -ne $outcome.PSObject.Properties['pushRangeM']) {
						Assert-JsonNumber $outcome.pushRangeM 'KoukuSaydon outcome pushRangeM'
						$outcomePushRangeM = [double]$outcome.pushRangeM
					}
					if ($null -ne $outcome.PSObject.Properties['pushMs']) {
						Assert-JsonInteger $outcome.pushMs 'KoukuSaydon outcome pushMs' 0 600000
						$outcomePushMs = [uint32]$outcome.pushMs
					}
					if ($outcomePushRangeM -lt 0 -or $outcomePushRangeM -gt $outcomePushRangeLimit -or
						(($outcomePushRangeM -eq 0 -and $outcomePushHeightM -eq 0) -ne ($outcomePushMs -eq 0))) {
						throw 'KoukuSaydon outcome pushRangeM exceeds its policy bound, or pushMs 0..600000 must both be zero or positive'
					}
				}
				if ($null -ne $outcome.PSObject.Properties['pushDirection']) {
					Assert-JsonString $outcome.pushDirection 'KoukuSaydon pushDirection'
					$outcomePushDirection = [string]$outcome.pushDirection
					if ($outcomeKind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE') -or $outcomePushDirection -cnotin @('AWAY_FROM_BOSS','BOSS_FORWARD','AWAY_FROM_CONTACT') -or
						($outcomePushDirection -cne 'AWAY_FROM_BOSS' -and ($outcomePushRangeM -le 0 -and $outcomePushHeightM -le 0))) { throw 'Invalid damage pushDirection' }
				}
				$outcomePushYawOffsetDegrees = 0.0
				if ($null -ne $outcome.PSObject.Properties['pushYawOffsetDegrees']) {
					Assert-JsonNumber $outcome.pushYawOffsetDegrees 'Push yaw offset'
					$outcomePushYawOffsetDegrees = [double]$outcome.pushYawOffsetDegrees
					if ([math]::Abs($outcomePushYawOffsetDegrees) -gt 360 -or $outcomeKind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE') -or
						($outcomePushYawOffsetDegrees -ne 0 -and ($outcomePushDirection -cne 'BOSS_FORWARD' -or ($outcomePushRangeM -le 0 -and $outcomePushHeightM -le 0)))) { throw 'Push yaw offset requires bounded positive BOSS_FORWARD damage push' }
				}
				$outcomeForcePush = $false
				$outcomePushCanLeaveArena = $false
				foreach ($policy in @('forcePush','pushCanLeaveArena','pushBallistic')) {
					if ($null -eq $outcome.PSObject.Properties[$policy]) { continue }
					if ($outcome.$policy -isnot [bool] -or $outcomeKind -cnotin @('MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE') -or
						($outcome.$policy -and ($outcomePushRangeM -le 0 -and $outcomePushHeightM -le 0))) { throw 'Push policy requires a Boolean on a positive damage push' }
				}
				if ($null -ne $outcome.PSObject.Properties['forcePush']) { $outcomeForcePush = [bool]$outcome.forcePush }
				if ($null -ne $outcome.PSObject.Properties['pushCanLeaveArena']) { $outcomePushCanLeaveArena = [bool]$outcome.pushCanLeaveArena }
				if ($outcomePushBallistic -and ($outcomePushMs -lt 100 -or $outcomePushMs -gt 5000)) {
					throw 'Ballistic push requires pushMs 100..5000'
				}
				$isContactResult = $outcomeKind -cin @('PLAY_CONTACT_WORLD_OBJECT_MOTION','COMPLETE_LOGIC_WINDOW')
				if ($isContactResult -ne ($windowKind -ceq 'OBJECT_CONTACT')) { throw 'OBJECT_CONTACT requires contact motion or window signal results' }
				if ($windowKind -ceq 'OBJECT_OVERLAP' -and ($outcomeKind -cne 'PLAY_WORLD_OBJECT_MOTION' -or
					$outcome.targetWorldInstanceId -cne $window.targetWorldInstanceId)) { throw 'OBJECT_OVERLAP Result must target the same World Object' }
				$followup = [string]$outcome.patternId
				$isFollowup = $outcomeKind -ceq 'FOLLOWUP_PATTERN'
				$hasFollowup = -not [string]::IsNullOrEmpty($followup)
				if ($outcomeKind -cnotin @(
						'INSTANT_DEATH','MAX_HP_PERCENT_DAMAGE','FIXED_DAMAGE','MADNESS_GAUGE_ADD_PERCENT',
						'CLOWN_TRANSFORM','FEAR','FOLLOWUP_PATTERN','PLAY_WORLD_OBJECT_MOTION','PLAY_CONTACT_WORLD_OBJECT_MOTION','COMPLETE_LOGIC_WINDOW','CAPTURE_PLAYER','GRAB_TO_WORLD_OBJECT','MARIO_ENTER','PLAYER_INVULNERABILITY') -or
					($outcomeKind -cne 'MARIO_ENTER' -and $isFollowup -ne $hasFollowup) -or
					($isFollowup -and $windowKind -cnotin @('STAGGER_WINDOW','COUNTER_WINDOW','EXTERNAL_SIGNAL','PATTERN_COMPLETION_COUNT')) -or
					($outcomeKind -cin @('MAX_HP_PERCENT_DAMAGE','MADNESS_GAUGE_ADD_PERCENT') -and
						[uint32]$outcome.percent -eq 0)) {
					throw "KoukuSaydon logic outcome is invalid: $($window.windowId)/$slotName/$ordinal"
				}
				if ($hasFollowup) {
					Assert-StableId $followup 'KoukuSaydon logic outcome patternId'
					$koukuFollowupTargets.Add($followup)
				}
				$motionIds = @()
				if ($outcomeKind -ceq 'PLAYER_INVULNERABILITY' -and ($outcome.durationMs -eq 0 -or $outcome.percent -ne 0 -or $outcome.patternId -ne '')) {
					throw 'Player invulnerability needs a positive duration without percent or target'
				}
				if ($outcomeKind -ceq 'FIXED_DAMAGE') {
					Assert-JsonInteger $outcome.damageAmount 'Fixed damage HP' 1 1000000000
					if ($outcome.percent -ne 0 -or $outcome.durationMs -ne 0) { throw 'Fixed damage takes HP, not percent or duration' }
					$motionIds = @([string][uint32]$outcome.damageAmount)
				}
				if ($outcomeKind -ceq 'PLAY_WORLD_OBJECT_MOTION') {
					Assert-StableId $outcome.targetWorldInstanceId 'KoukuSaydon motion target instance ID'
					Assert-StableId $outcome.motionInstanceId 'KoukuSaydon saved motion instance ID'
					if ($outcome.percent -ne 0 -or $outcome.durationMs -ne 0) { throw 'World Object motion outcome does not take percent or durationMs' }
					if (@($koukuPattern.worldSequences | Where-Object { $_.sequenceInstanceId -ceq $outcome.targetWorldInstanceId }).Count -gt 1) { throw 'Legacy World motion target is ambiguous; use contact occurrence binding' }
					$motionIds = @($outcome.targetWorldInstanceId, $outcome.motionInstanceId)
				}
                if ($outcomeKind -ceq 'FEAR') {
                    Assert-JsonString $outcome.presentationId 'Fear presentation ID'
                    Assert-StableId $outcome.presentationId 'Fear presentation ID'
                    if ($outcome.durationMs -eq 0 -or $outcome.percent -ne 0) { throw 'Fear needs a positive duration and no percent' }
                    $motionIds = @($outcome.presentationId)
                }
				if ($outcomeKind -ceq 'GRAB_TO_WORLD_OBJECT' -or $outcomeKind -ceq 'MARIO_ENTER') {
					if ($windowKind -cne 'ENTER_AREA' -or $slotName -cne 'SUCCESS' -or
						$outcome.percent -ne 0 -or $outcome.durationMs -ne 0) {
						throw 'A world-object grab only answers an ENTER_AREA success and carries no value'
					}
				}
				if ($outcomeKind -ceq 'MARIO_ENTER' -and $null -ne $outcome.PSObject.Properties['marioStage']) {
					# Authored stage 1..4 rides as an 11th field; 0 stays a 10-field row so older Server parsers keep reading it.
					Assert-JsonInteger $outcome.marioStage 'Mario entry stage' 0 4
					if ([uint32]$outcome.marioStage -gt 0) { $motionIds = @([string][uint32]$outcome.marioStage) }
				}
				if ($outcomeKind -ceq 'CAPTURE_PLAYER') {
					if ($windowKind -cne 'ENTER_AREA' -or $slotName -cne 'SUCCESS' -or $outcomes.Count -ne 1 -or
						$null -eq $window.PSObject.Properties['holdLogicOccurrenceId'] -or
						$outcome.percent -ne 0 -or $outcome.durationMs -ne 0 -or $outcome.attachmentSlot -cne 'BOSS_LEFT_HAND') {
						throw 'CAPTURE_PLAYER must be the sole ENTER_AREA Success with Hold and BOSS_LEFT_HAND, without percent or duration'
					}
					Assert-ExactProperties $outcome.gripLocalOffset @('forwardM','upM','rightM') 'Capture gripLocalOffset'
					$motionIds = @($outcome.attachmentSlot)
					foreach ($axis in @('forwardM','upM','rightM')) {
						Assert-JsonNumber $outcome.gripLocalOffset.$axis "Capture gripLocalOffset $axis"
						if ([Math]::Abs([double]$outcome.gripLocalOffset.$axis) -gt 10) { throw 'Capture gripLocalOffset must stay in -10..10 m' }
						$motionIds += Format-InvariantSignedFloat $outcome.gripLocalOffset.$axis "Capture gripLocalOffset $axis"
					}
					$gripKey = $motionIds -join '|'
					if ($null -ne $koukuCaptureGrip -and $koukuCaptureGrip -cne $gripKey) { throw 'A Pattern requires one consistent capture grip offset' }
					$koukuCaptureGrip = $gripKey
				}
				$followupText = if ($hasFollowup) { $followup } else { '-' }
				$outcomePercent = [uint32]$outcome.percent
				if ($outcomeKind -ceq 'MADNESS_GAUGE_ADD_PERCENT' -and $MadnessGaugeAddPercent -ge 0) {
					$outcomePercent = [uint32]$MadnessGaugeAddPercent
				}
				$patternRows.Add((@(
					'PATTERNLOGICOUTCOME', $koukuEncounterDocument.encounterId,
					$koukuPattern.patternId, $window.windowId, $slotName, $ordinal,
					$outcomeKind, $outcomePercent, [uint32]$outcome.durationMs,
					$followupText) + $motionIds -join "`t"))
				if ($outcomePushRangeM -gt 0 -or $outcomePushHeightM -gt 0) {
					$patternRows.Add((@('PATTERNLOGICPUSH', $koukuEncounterDocument.encounterId,
						$koukuPattern.patternId, $window.windowId, $slotName, $ordinal,
						(Format-InvariantFloat $outcomePushRangeM 'KoukuSaydon outcome pushRangeM'), $outcomePushMs) + $(if ($outcomePushHeightM -gt 0) { @($outcomePushDirection, [int]$outcomeForcePush, [int]$outcomePushCanLeaveArena, (Format-InvariantSignedFloat $outcomePushYawOffsetDegrees 'Push yaw offset'), 1, (Format-InvariantFloat $outcomePushHeightM 'Ballistic height')) } elseif ($outcomePushBallistic) { @($outcomePushDirection, [int]$outcomeForcePush, [int]$outcomePushCanLeaveArena, (Format-InvariantSignedFloat $outcomePushYawOffsetDegrees 'Push yaw offset'), 1) } elseif ($outcomePushYawOffsetDegrees -ne 0) { @($outcomePushDirection, [int]$outcomeForcePush, [int]$outcomePushCanLeaveArena, (Format-InvariantSignedFloat $outcomePushYawOffsetDegrees 'Push yaw offset')) } elseif ($outcomeForcePush -or $outcomePushCanLeaveArena) { @($outcomePushDirection, [int]$outcomeForcePush, [int]$outcomePushCanLeaveArena) } elseif ($outcomePushDirection -cne 'AWAY_FROM_BOSS') { @($outcomePushDirection) } else { @() }) -join "`t"))
				}
				if ($isContactResult -and ($outcome.percent -ne 0 -or $outcome.durationMs -ne 0)) { throw 'Contact result does not take percent or durationMs' }
				if ($outcomeKind -ceq 'PLAY_CONTACT_WORLD_OBJECT_MOTION') {
					if ($outcome.contactMotions -isnot [Array] -or @($outcome.contactMotions).Count -ne $contactTargetIds.Count) { throw 'Contact motion must map every candidate' }
					$mapped = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
					foreach ($mapping in @($outcome.contactMotions)) {
						Assert-ExactProperties $mapping @('targetWorldOccurrenceId','motionInstanceId') 'Contact motion'
						Assert-JsonString $mapping.targetWorldOccurrenceId 'Contact motion target'
						Assert-StableId $mapping.targetWorldOccurrenceId 'Contact motion target'
						Assert-JsonString $mapping.motionInstanceId 'Contact motion instance'
						Assert-StableId $mapping.motionInstanceId 'Contact motion instance'
						if (-not $contactTargetIds.Contains($mapping.targetWorldOccurrenceId) -or -not $mapped.Add($mapping.targetWorldOccurrenceId)) { throw 'Contact motion target is missing or duplicated' }
						$patternRows.Add((@('PATTERNLOGICCONTACTMOTION',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
							$window.windowId,$slotName,$ordinal,$mapping.targetWorldOccurrenceId,$mapping.motionInstanceId) -join "`t"))
					}
				} elseif ($outcomeKind -ceq 'COMPLETE_LOGIC_WINDOW') {
					Assert-JsonString $outcome.targetLogicOccurrenceId 'Complete window target'
					Assert-StableId $outcome.targetLogicOccurrenceId 'Complete window target'
					Assert-JsonString $outcome.contactTargetWorldOccurrenceId 'Complete window contact filter'
					$signalTargets = @($koukuPattern.logicWindows | Where-Object { $_.windowId -ceq $outcome.targetLogicOccurrenceId })
					if ($signalTargets.Count -ne 1 -or $signalTargets[0].kind -cne 'EXTERNAL_SIGNAL' -or
						$signalTargets[0].startMs -gt $window.startMs -or
						([uint64]$signalTargets[0].startMs + [uint64]$signalTargets[0].durationMs) -lt $windowEndMs) { throw 'Complete window target must be an enclosing EXTERNAL_SIGNAL window' }
					$contactFilter = '-'
					if (-not [string]::IsNullOrEmpty($outcome.contactTargetWorldOccurrenceId)) {
						Assert-StableId $outcome.contactTargetWorldOccurrenceId 'Complete window contact filter'
						if (-not $contactTargetIds.Contains($outcome.contactTargetWorldOccurrenceId)) { throw 'Complete window contact filter must name a candidate' }
						$contactFilter = $outcome.contactTargetWorldOccurrenceId
					}
					$patternRows.Add((@('PATTERNLOGICSIGNAL',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
						$window.windowId,$slotName,$ordinal,$outcome.targetLogicOccurrenceId,$contactFilter) -join "`t"))
				}
			}
		}
	}
	# Forward Hold references resolve only after every PATTERNLOGIC row exists.
	foreach ($holdRow in $koukuHoldRows) { $patternRows.Add($holdRow) }
	if ($null -ne $koukuPattern.PSObject.Properties['pursuitProjectiles']) {
		foreach ($targetRow in @(New-KoukuPursuitProjectileRows $koukuPattern $koukuEncounterDocument.encounterId $koukuPatternDurationMs $koukuTargetedVisuals)) { $patternRows.Add($targetRow) }
	}
	if ($null -ne $koukuPattern.PSObject.Properties['showtimeTargets']) {
		foreach ($targetRow in @(New-KoukuShowtimeTargetRows $koukuPattern $koukuEncounterDocument.encounterId $koukuPatternDurationMs $koukuTargetedVisuals)) {
			$patternRows.Add($targetRow)
		}
	}
	if ($koukuPattern.mechanicTriggers -isnot [Array] -or @($koukuPattern.mechanicTriggers).Count -gt 64) {
		throw "KoukuSaydon mechanic trigger list is invalid"
	}
	$triggerIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	foreach ($trigger in @($koukuPattern.mechanicTriggers)) {
		if ($trigger.kind -ceq 'CROSS_DIRECTION_CLONES') {
			Assert-ExactProperties $trigger @('triggerId','kind','startMs','durationMs','hudMode','teleportPosition','clonePatternId',
				'clockHours','faceCenterYawOffsetDegrees','countPerPlayer','radiusM','effectLifetimeMs','arenaRandomCount',
				'arenaRandomRadiusM','arenaHeightToleranceM','arenaMinimumSpacingM','randomPlayerOnly','directionPatternIds','cloneEndStageId') 'Cross direction Logic'
			Assert-StableId $trigger.triggerId 'Cross direction occurrence'
			Assert-StableId $trigger.cloneEndStageId 'Cross direction clone end Stage'
			Assert-JsonInteger $trigger.startMs 'Cross direction startMs' 0 600000
			Assert-JsonInteger $trigger.durationMs 'Cross direction durationMs' 1 600000
			if (-not $triggerIds.Add([string]$trigger.triggerId) -or $trigger.directionPatternIds -isnot [Array] -or
				@($trigger.directionPatternIds).Count -ne 4 -or @($trigger.directionPatternIds | Select-Object -Unique).Count -ne 4 -or
				([uint64]$trigger.startMs + [uint64]$trigger.durationMs) -gt $koukuPatternDurationMs -or
				$trigger.hudMode -cne 'NONE' -or $trigger.clonePatternId -cne '' -or @($trigger.clockHours).Count -ne 0 -or
				$trigger.teleportPosition -isnot [Array] -or @($trigger.teleportPosition).Count -ne 3 -or
				@($trigger.teleportPosition | Where-Object { $_ -ne 0 }).Count -ne 0 -or
				$trigger.faceCenterYawOffsetDegrees -ne 0 -or $trigger.countPerPlayer -ne 0 -or $trigger.radiusM -ne 0 -or
				$trigger.effectLifetimeMs -ne 0 -or $trigger.arenaRandomCount -ne 0 -or $trigger.arenaRandomRadiusM -ne 0 -or
				$trigger.arenaHeightToleranceM -ne 0 -or $trigger.arenaMinimumSpacingM -ne 0 -or $trigger.randomPlayerOnly) {
				throw 'Cross direction requires exactly four distinct candidates and a bounded duration without unrelated values'
			}
			foreach ($identity in $trigger.directionPatternIds) {
				Assert-StableId $identity 'Cross direction Pattern ID'
				$children = @($koukuEncounterDocument.patterns | Where-Object { $_.patternId -ceq $identity })
				if ($children.Count -ne 1 -or $identity -ceq $koukuPattern.patternId) { throw 'Cross direction child Pattern is unavailable' }
				$child = $children[0]; $childDuration = [uint64]0; $cutoff = [uint64]0
				foreach ($stage in @($child.stages)) {
					$childDuration += [uint64]$stage.durationMs
					if ($stage.stageId -ceq $trigger.cloneEndStageId) { $cutoff = $childDuration }
				}
				if ($child.gateId -cne $koukuPattern.gateId -or $child.actorProfileId -cne $koukuPattern.actorProfileId -or
					$child.targetBossPlacementId -cne $koukuPattern.targetBossPlacementId -or $child.category -cne 'MECHANIC' -or
					-not $child.fixedTimeline -or $cutoff -eq 0 -or $cutoff -ge $childDuration -or $childDuration -gt [uint64]$trigger.durationMs -or
					-not (Test-KoukuActorContactWindows $child) -or @($child.mechanicTriggers).Count -ne 0 -or
					@($child.worldSequences).Count -ne 0 -or @($child.sceneProfiles).Count -ne 0 -or
					$null -ne $child.PSObject.Properties['bossMotion'] -or $child.resetBossToSpawn) {
					throw 'Cross direction child must be a same-body Animation Pattern with a cutoff before its full ending'
				}
				$koukuFollowupTargets.Add([string]$identity)
			}
			$patternRows.Add((@('PATTERNCROSSDIRECTION',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
				$trigger.triggerId,[uint32]$trigger.startMs,[uint32]$trigger.durationMs) + @($trigger.directionPatternIds) +
				@($trigger.cloneEndStageId)) -join "`t")
			continue
		}
		$triggerOptionalProperties = @()
		foreach ($field in @('soldierCounts','spawnRadiusMinM','spawnRadiusMaxM')) {
			if ($null -ne $trigger.PSObject.Properties[$field]) { $triggerOptionalProperties += $field }
		}
		if ($trigger.kind -ceq 'CARD_RAIN_SOLDIERS') {
			if ($trigger.soldierCounts -isnot [Array] -or @($trigger.soldierCounts).Count -ne 3) { throw 'Card rain needs three soldier counts' }
			$totalSoldiers = 0
			foreach ($count in $trigger.soldierCounts) { Assert-JsonInteger $count 'Card rain soldier count' 0 32; $totalSoldiers += $count }
			Assert-JsonNumber $trigger.spawnRadiusMinM 'Card rain minimum radius'
			Assert-JsonNumber $trigger.spawnRadiusMaxM 'Card rain maximum radius'
			if ($totalSoldiers -lt 1 -or $totalSoldiers -gt 64 -or $trigger.spawnRadiusMinM -lt 0 -or
				$trigger.spawnRadiusMaxM -gt 100 -or $trigger.spawnRadiusMinM -gt $trigger.spawnRadiusMaxM) { throw 'Card rain counts or radii are invalid' }
		} elseif (@($triggerOptionalProperties).Count -ne 0) { throw 'Only card rain soldiers carry summon tuning' }
		if ($null -ne $trigger.PSObject.Properties['patternSpawns']) { $triggerOptionalProperties += 'patternSpawns' }
		if ($null -ne $trigger.PSObject.Properties['playerEntryPositions']) { $triggerOptionalProperties += 'playerEntryPositions' }
		foreach ($field in @('airbornePhase','airborneHeightM','airborneDurationMs','airborneTargetPositionPolicy','selectedEffectVisualId','selectedEffectLifetimeMs')) {
			if ($null -ne $trigger.PSObject.Properties[$field]) { $triggerOptionalProperties += $field }
		}
		$trackFollowSpeedScale = 0.0
		if ($null -ne $trigger.PSObject.Properties['followSpeedScale']) {
			$triggerOptionalProperties += 'followSpeedScale'
			Assert-JsonNumber $trigger.followSpeedScale 'Kouku boss tracking follow speed'
			if ($trigger.kind -cne 'BOSS_TRACK_TARGET' -or $trigger.followSpeedScale -lt .01 -or $trigger.followSpeedScale -gt 10) {
				throw 'Boss tracking follow speed needs BOSS_TRACK_TARGET and a .01..10 player-speed multiple'
			}
			$trackFollowSpeedScale = [double]$trigger.followSpeedScale
		}
		if ($null -ne $trigger.PSObject.Properties['fixedHits']) {
			if ($trigger.kind -cne 'ALBION_BLUE_CIRCLE') { throw 'Only Albion circles carry generic fixed hits' }
			$triggerOptionalProperties += 'fixedHits'
		}
		Assert-ExactProperties $trigger (@('triggerId','kind','startMs','durationMs',
			'hudMode','teleportPosition','clonePatternId','clockHours','faceCenterYawOffsetDegrees','countPerPlayer','radiusM','effectLifetimeMs',
			'arenaRandomCount','arenaRandomRadiusM','arenaHeightToleranceM','arenaMinimumSpacingM','randomPlayerOnly') + $triggerOptionalProperties) 'KoukuSaydon mechanic trigger'
		Assert-StableId $trigger.triggerId 'KoukuSaydon mechanic trigger ID'
		Assert-JsonInteger $trigger.startMs 'KoukuSaydon trigger startMs' 0 600000
		Assert-JsonInteger $trigger.durationMs 'KoukuSaydon trigger durationMs' 1 600000
		$modes = @('NONE','POLYMORPH','MARIO','DANCE','MAZE')
		$triggerHudMode = [Array]::IndexOf($modes, [string]$trigger.hudMode)
		if (-not $triggerIds.Add([string]$trigger.triggerId) -or $triggerHudMode -lt 0 -or
			$trigger.kind -cnotin @('REAL_GAZE_TELEPORT','BOSS_TELEPORT_FACE_CENTER','MARIO_PHASE2_PLAYERS','BOSS_TELEPORT_XZ','BOSS_TELEPORT_GROUNDED','BOSS_TRACK_TARGET','BINGO_BOARD','BINGO_DETONATION','HUD_ENTER','CARD_RAIN_SOLDIERS','CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','CARD_MAZE_STAGE_PLAYERS','ALBION_BLUE_CIRCLE','SUMMON_PATTERNS','ALBION_AIRBORNE') -or
			([uint64]$trigger.startMs + [uint64]$trigger.durationMs) -gt $koukuPatternDurationMs -or
			$trigger.teleportPosition -isnot [Array] -or @($trigger.teleportPosition).Count -ne 3 -or
			$trigger.clockHours -isnot [Array]) {
			throw "KoukuSaydon mechanic trigger is invalid: $($trigger.triggerId)"
		}
		$position = @($trigger.teleportPosition)
		foreach ($coordinate in $position) { Assert-JsonNumber $coordinate 'KoukuSaydon teleport coordinate' }
		if ($trigger.kind -cin @('BOSS_TELEPORT_XZ','BOSS_TELEPORT_GROUNDED') -and @($position | Where-Object { [Math]::Abs([double]$_) -gt 100000 }).Count -ne 0) { throw 'Boss XZ teleport coordinates exceed the world bounds' }
		if ($trigger.kind -cin @('CARD_RAIN_SOLDIERS','CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','CARD_MAZE_STAGE_PLAYERS','BOSS_TELEPORT_FACE_CENTER','MARIO_PHASE2_PLAYERS','BOSS_TELEPORT_XZ','BOSS_TELEPORT_GROUNDED','BOSS_TRACK_TARGET','BINGO_BOARD','BINGO_DETONATION') -and
			($triggerHudMode -ne 0 -or $trigger.faceCenterYawOffsetDegrees -ne 0 -or
			 ($trigger.kind -cin @('CARD_RAIN_SOLDIERS','CARD_MAZE_HIDE_NEXT','CARD_MAZE_STAGE_PLAYERS','BOSS_TRACK_TARGET','BINGO_BOARD','BINGO_DETONATION') -and @($position | Where-Object { $_ -ne 0 }).Count -ne 0))) {
			throw "KoukuSaydon card maze trigger carries unrelated values"
		}
		Assert-JsonInteger $trigger.countPerPlayer 'KoukuSaydon circles per player' 0 8
		Assert-JsonInteger $trigger.effectLifetimeMs 'KoukuSaydon effect lifetime' 0 600000
		Assert-JsonNumber $trigger.radiusM 'KoukuSaydon player effect radius'
		Assert-JsonInteger $trigger.arenaRandomCount 'KoukuSaydon arena random count' 0 32
		Assert-JsonNumber $trigger.arenaRandomRadiusM 'KoukuSaydon arena random radius'
		Assert-JsonNumber $trigger.arenaHeightToleranceM 'KoukuSaydon arena height tolerance'
		Assert-JsonNumber $trigger.arenaMinimumSpacingM 'KoukuSaydon arena minimum spacing'
		if ($trigger.randomPlayerOnly -isnot [bool] -or
			$trigger.arenaRandomRadiusM -lt 0 -or $trigger.arenaRandomRadiusM -gt 100 -or
			$trigger.arenaHeightToleranceM -lt 0 -or $trigger.arenaHeightToleranceM -gt 10 -or
			$trigger.arenaMinimumSpacingM -lt 0 -or $trigger.arenaMinimumSpacingM -gt 20 -or
			($trigger.arenaRandomCount -gt 0 -and ($trigger.arenaRandomRadiusM -le 0 -or $trigger.arenaHeightToleranceM -le 0 -or $trigger.arenaMinimumSpacingM -le 0)) -or
			($trigger.arenaRandomCount -eq 0 -and ($trigger.arenaRandomRadiusM -ne 0 -or $trigger.arenaHeightToleranceM -ne 0 -or $trigger.arenaMinimumSpacingM -ne 0)) -or
			($trigger.randomPlayerOnly -and ($trigger.countPerPlayer -ne 1 -or $trigger.radiusM -ne 0))) {
			throw 'KoukuSaydon arena random layout is invalid'
		}
		if ($trigger.kind -ceq 'ALBION_BLUE_CIRCLE') {
			if ($trigger.countPerPlayer -lt 1 -or $trigger.radiusM -lt 0 -or $trigger.radiusM -gt 20 -or
				(($trigger.countPerPlayer -eq 1) -ne ($trigger.radiusM -eq 0)) -or $trigger.effectLifetimeMs -lt 1 -or
				$triggerHudMode -ne 0 -or $trigger.faceCenterYawOffsetDegrees -ne 0 -or @($position | Where-Object { $_ -ne 0 }).Count -ne 0) {
				throw 'KoukuSaydon Albion player effect layout is invalid'
			}
		}
		elseif ($trigger.countPerPlayer -ne 0 -or $trigger.radiusM -ne 0 -or $trigger.effectLifetimeMs -ne 0 -or $trigger.arenaRandomCount -ne 0 -or $trigger.randomPlayerOnly) {
			throw 'Non-Albion mechanic trigger carries player effect values'
		}
		$hours = @(0,0,0)
		$clone = '-'
		if ($trigger.kind -ceq 'REAL_GAZE_TELEPORT') {
			Assert-StableId $trigger.clonePatternId 'KoukuSaydon clonePatternId'
			$clone = [string]$trigger.clonePatternId
			$koukuFollowupTargets.Add($clone)
			$hours = @($trigger.clockHours)
			if ($hours.Count -ne 3 -or @($hours | Select-Object -Unique).Count -ne 3) {
				throw "KoukuSaydon teleport needs three distinct clock hours"
			}
			foreach ($hour in $hours) { Assert-JsonInteger $hour 'KoukuSaydon clockHour' 2 12 }
		}
		elseif (@($trigger.clockHours).Count -ne 0 -or -not [string]::IsNullOrEmpty($trigger.clonePatternId)) {
			throw "KoukuSaydon HUD trigger carries clone values"
		}
		$spawnRows = [Collections.Generic.List[string]]::new()
		if ($trigger.kind -ceq 'CARD_MAZE_STAGE_PLAYERS') {
			if ($trigger.playerEntryPositions -isnot [Array] -or @($trigger.playerEntryPositions).Count -lt 1 -or
				@($trigger.playerEntryPositions).Count -gt 4) { throw 'Card maze staging needs one to four player entry positions' }
			$entrySlot = 0
			foreach ($entry in $trigger.playerEntryPositions) {
				if ($entry -isnot [Array] -or @($entry).Count -ne 3) { throw 'Card maze entry needs X/Y/Z' }
				foreach ($coordinate in $entry) {
					Assert-JsonNumber $coordinate 'Card maze entry coordinate'
					if ([Math]::Abs([double]$coordinate) -gt 100000) { throw 'Card maze entry exceeds world bounds' }
				}
				$spawnRows.Add((@('PATTERNCARDMAZESTAGING', $koukuEncounterDocument.encounterId,
					$koukuPattern.patternId, $trigger.triggerId, $entrySlot,
					(Format-InvariantSignedFloat $entry[0] 'Card maze entry X'),
					(Format-InvariantSignedFloat $entry[1] 'Card maze entry Y'),
					(Format-InvariantSignedFloat $entry[2] 'Card maze entry Z')) -join "`t"))
				++$entrySlot
			}
		}
		elseif ($null -ne $trigger.PSObject.Properties['playerEntryPositions']) { throw 'Only card maze staging can carry playerEntryPositions' }
		if ($trigger.kind -ceq 'SUMMON_PATTERNS') {
			if ($trigger.patternSpawns -isnot [Array] -or @($trigger.patternSpawns).Count -lt 1 -or
				@($trigger.patternSpawns).Count -gt 16 -or $triggerHudMode -ne 0 -or
				$trigger.faceCenterYawOffsetDegrees -ne 0 -or @($position | Where-Object { $_ -ne 0 }).Count -ne 0) {
				throw 'Summon requires one to sixteen Pattern spawns and no owner teleport'
			}
			$spawnIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			foreach ($spawn in @($trigger.patternSpawns)) {
				$spawnProperties = @('spawnId','patternId','positionOffset','yawOffsetDegrees')
				$spawnAnchor = 'BOSS'
				if ($null -ne $spawn.PSObject.Properties['anchorKind']) {
					$spawnProperties += 'anchorKind'
					if ($spawn.anchorKind -cnotin @('BOSS','MAP')) { throw 'Summon anchor must be BOSS or MAP' }
					$spawnAnchor = [string]$spawn.anchorKind
				}
				$spawnPositionLimit = if ($spawnAnchor -ceq 'MAP') { 100000 } else { 1000 }
				Assert-ExactProperties $spawn $spawnProperties 'Summon Pattern spawn'
				Assert-StableId $spawn.spawnId 'Summon spawnId'
				Assert-StableId $spawn.patternId 'Summon patternId'
				Assert-JsonNumber $spawn.yawOffsetDegrees 'Summon yaw offset'
				if (-not $spawnIds.Add([string]$spawn.spawnId) -or $spawn.patternId -ceq $koukuPattern.patternId -or
					$spawn.positionOffset -isnot [Array] -or @($spawn.positionOffset).Count -ne 3 -or
					[math]::Abs([double]$spawn.yawOffsetDegrees) -gt 360) { throw 'Summon Pattern spawn identity or transform is invalid' }
				foreach ($coordinate in $spawn.positionOffset) {
					Assert-JsonNumber $coordinate 'Summon position offset'
					if ([math]::Abs([double]$coordinate) -gt $spawnPositionLimit) { throw 'Summon position exceeds its anchor range' }
				}
				$children = @($koukuEncounterDocument.patterns | Where-Object { $_.patternId -ceq $spawn.patternId })
				if ($children.Count -ne 1) { throw 'Summon child Pattern is unavailable' }
				$child = $children[0]
				$childDuration = [uint64]0
				foreach ($stage in @($child.stages)) { $childDuration += [uint64]$stage.durationMs }
				if ($child.gateId -cne $koukuPattern.gateId -or $child.actorProfileId -cne $koukuPattern.actorProfileId -or
					$child.targetBossPlacementId -cne $koukuPattern.targetBossPlacementId -or $child.category -cne 'MECHANIC' -or
					$childDuration -eq 0 -or $childDuration -gt [uint64]$trigger.durationMs -or
					-not (Test-KoukuActorContactWindows $child) -or @($child.mechanicTriggers | Where-Object {
						$_.kind -cne 'ALBION_AIRBORNE' -or $_.airbornePhase -cnotin @('JUMP','SLAM') -or
						$null -ne $_.PSObject.Properties['airborneTargetPositionPolicy'] -or
						$null -ne $_.PSObject.Properties['selectedEffectVisualId']
					}).Count -ne 0 -or
					@($child.worldSequences).Count -ne 0 -or @($child.sceneProfiles).Count -ne 0 -or
					$null -ne $child.PSObject.Properties['bossMotion'] -or $child.resetBossToSpawn) {
					throw 'Summon child must be a same-Gate actor-local leaf within the Summon lifetime'
				}
				$koukuFollowupTargets.Add([string]$spawn.patternId)
				$spawnRow = @('PATTERNSUMMONSPAWN', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
					$trigger.triggerId, $spawn.spawnId, $spawn.patternId,
					(Format-InvariantSignedFloat $spawn.positionOffset[0] 'Summon offset X'),
					(Format-InvariantSignedFloat $spawn.positionOffset[1] 'Summon offset Y'),
					(Format-InvariantSignedFloat $spawn.positionOffset[2] 'Summon offset Z'),
					(Format-InvariantSignedFloat $spawn.yawOffsetDegrees 'Summon yaw offset'))
				if ($spawnAnchor -ceq 'MAP') { $spawnRow += 'MAP' }
				$spawnRows.Add(($spawnRow -join "`t"))
			}
		}
		elseif ($null -ne $trigger.PSObject.Properties['patternSpawns']) { throw 'Only a Summon trigger can carry patternSpawns' }
		$airborneRow = New-KoukuAlbionAirborneRow $trigger $koukuEncounterDocument.encounterId $koukuPattern.patternId $koukuPatternDurationMs
		$patternRows.Add((@('PATTERNMECHANICTRIGGER', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, $trigger.triggerId, $trigger.kind,
			[uint32]$trigger.startMs, [uint32]$trigger.durationMs, $triggerHudMode,
			(Format-InvariantSignedFloat $position[0] 'KoukuSaydon teleport X'),
			(Format-InvariantSignedFloat $position[1] 'KoukuSaydon teleport Y'),
			(Format-InvariantSignedFloat $position[2] 'KoukuSaydon teleport Z'),
			$clone, $hours[0], $hours[1], $hours[2], 1, (Format-InvariantSignedFloat $trigger.faceCenterYawOffsetDegrees 'KoukuSaydon face-center offset'),
			[uint32]$trigger.countPerPlayer, (Format-InvariantSignedFloat $trigger.radiusM 'KoukuSaydon player effect radius'), [uint32]$trigger.effectLifetimeMs,
			[uint32]$trigger.arenaRandomCount,
			(Format-InvariantSignedFloat $trigger.arenaRandomRadiusM 'KoukuSaydon arena random radius'),
			(Format-InvariantSignedFloat $trigger.arenaHeightToleranceM 'KoukuSaydon arena height tolerance'),
			(Format-InvariantSignedFloat $trigger.arenaMinimumSpacingM 'KoukuSaydon arena minimum spacing'), [int]$trigger.randomPlayerOnly) -join "`t"))
		foreach ($spawnRow in $spawnRows) { $patternRows.Add($spawnRow) }
		if ($trigger.kind -ceq 'CARD_RAIN_SOLDIERS') {
			$patternRows.Add((@('PATTERNCARDRAINSOLDIERS', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
				$trigger.triggerId, $trigger.soldierCounts[0], $trigger.soldierCounts[1], $trigger.soldierCounts[2],
				(Format-InvariantSignedFloat $trigger.spawnRadiusMinM 'Card rain minimum radius'),
				(Format-InvariantSignedFloat $trigger.spawnRadiusMaxM 'Card rain maximum radius')) -join "`t"))
		}
		if ($null -ne $trigger.PSObject.Properties['fixedHits']) {
			foreach ($hitRow in @(New-KoukuAttackHitRows $trigger.fixedHits $koukuEncounterDocument.encounterId $koukuPattern.patternId $trigger.triggerId 'ALBION' 0 $trigger.effectLifetimeMs)) { $patternRows.Add($hitRow) }
		}
		if ($airborneRow) { $patternRows.Add($airborneRow) }
		if ($trackFollowSpeedScale -gt 0) {
			$patternRows.Add((@('PATTERNTRACKMOVE', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
				$trigger.triggerId,
				(Format-InvariantFloat $trackFollowSpeedScale 'Kouku boss tracking follow speed')) -join "`t"))
		}
	}
	foreach ($worldSequence in @($koukuPattern.worldSequences)) {
		$worldSequenceProperties = @('sequenceInstanceId','occurrenceId','startMs','durationMs','playbackSpeed','positionOffset','anchorKind','anchorPosition')
		if ($null -ne $worldSequence.PSObject.Properties['walkableSurface']) { $worldSequenceProperties += 'walkableSurface' }
		$hasPlacement = $null -ne $worldSequence.PSObject.Properties['placement']
		if ($hasPlacement) { $worldSequenceProperties += 'placement' }
		$hasCombatBody = $null -ne $worldSequence.PSObject.Properties['combatBody']
		if ($hasCombatBody) { $worldSequenceProperties += 'combatBody' }
		if ($null -ne $worldSequence.PSObject.Properties['authoredMadness']) {
			$worldSequenceProperties += 'authoredMadness'
			if ($worldSequence.authoredMadness -isnot [bool] -or -not $worldSequence.authoredMadness) { throw 'Authored madness marker must be true' }
		}
		Assert-ExactProperties $worldSequence $worldSequenceProperties 'KoukuSaydon world sequence cue'
        Assert-JsonString $worldSequence.occurrenceId 'World source occurrenceId'
        Assert-StableId $worldSequence.occurrenceId 'World source occurrenceId'
		Assert-JsonString $worldSequence.sequenceInstanceId `
			'KoukuSaydon world sequence cue sequenceInstanceId'
		Assert-StableId $worldSequence.sequenceInstanceId `
			'KoukuSaydon world sequence cue sequenceInstanceId'
		Assert-JsonInteger $worldSequence.startMs `
			'KoukuSaydon world sequence cue startMs' 0 600000
		Assert-JsonInteger $worldSequence.durationMs 'KoukuSaydon world sequence cue durationMs' 1 600000
		Assert-JsonNumber $worldSequence.playbackSpeed `
			'KoukuSaydon world sequence cue playbackSpeed'
		if ([uint64]$worldSequence.startMs -gt $koukuPatternDurationMs -or
			[double]$worldSequence.playbackSpeed -lt 0.05 -or
			[double]$worldSequence.playbackSpeed -gt 16.0) {
			throw "KoukuSaydon world sequence cue is invalid: $($koukuPattern.patternId)/$($worldSequence.sequenceInstanceId)"
		}
		if (@($worldSequence.positionOffset).Count -ne 3 -or @($worldSequence.anchorPosition).Count -ne 3) { throw 'World sequence offset and anchor need three coordinates' }
		if ($worldSequence.anchorKind -notin @('NONE','BOSS_SPAWN')) { throw 'World sequence anchorKind is unsupported' }
		if ($null -ne $worldSequence.PSObject.Properties['authoredMadness']) {
			$patternRows.Add((@('PATTERNWORLDAUTHOREDMADNESS',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,$worldSequence.occurrenceId) -join "`t"))
		}
		if ($hasPlacement) {
			$placement = $worldSequence.placement
			Assert-ExactProperties $placement @('position','rotationDegrees','scale') 'WORLD occurrence placement'
			foreach ($field in @('position','rotationDegrees','scale')) {
				if ($placement.$field -isnot [Array] -or @($placement.$field).Count -ne 3) { throw "WORLD placement $field needs three coordinates" }
				foreach ($number in $placement.$field) { Assert-JsonNumber $number "WORLD placement $field" }
			}
			if (@($placement.position | Where-Object { [math]::Abs([double]$_) -gt 100000 }).Count -ne 0 -or
				@($placement.rotationDegrees | Where-Object { [math]::Abs([double]$_) -gt 36000 }).Count -ne 0 -or
				@($placement.scale | Where-Object { [double]$_ -lt .001 -or [double]$_ -gt 1000 }).Count -ne 0) { throw 'WORLD placement exceeds transform bounds' }
			if ($worldSequence.anchorKind -cne 'NONE' -or
				@($worldSequence.positionOffset | Where-Object { [double]$_ -ne 0 }).Count -ne 0 -or
				@($worldSequence.anchorPosition | Where-Object { [double]$_ -ne 0 }).Count -ne 0 -or
				$null -ne $worldSequence.PSObject.Properties['walkableSurface']) { throw 'WORLD placement replaces legacy offsets and cannot own a Map walkable surface' }
		}
		$patternRows.Add((@(
			'PATTERNWORLDSEQUENCE', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, [uint32]$worldSequence.startMs,
			$worldSequence.sequenceInstanceId,
			(Format-InvariantFloat $worldSequence.playbackSpeed `
				'KoukuSaydon world sequence cue playbackSpeed'),
			(Format-InvariantSignedFloat $worldSequence.positionOffset[0] 'World offset X'),
			(Format-InvariantSignedFloat $worldSequence.positionOffset[1] 'World offset Y'),
			(Format-InvariantSignedFloat $worldSequence.positionOffset[2] 'World offset Z'),
			$worldSequence.anchorKind,
			(Format-InvariantSignedFloat $worldSequence.anchorPosition[0] 'World anchor X'),
			(Format-InvariantSignedFloat $worldSequence.anchorPosition[1] 'World anchor Y'),
			(Format-InvariantSignedFloat $worldSequence.anchorPosition[2] 'World anchor Z'),
			[uint32]$worldSequence.durationMs, $worldSequence.occurrenceId) -join "`t"))
		if ($hasPlacement) {
			$placementNumbers = @($placement.position) + @($placement.rotationDegrees) + @($placement.scale)
			$placementText = @($placementNumbers | ForEach-Object { Format-InvariantSignedFloat $_ 'WORLD placement transform' })
			$patternRows.Add((@('PATTERNWORLDPLACEMENT',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,$worldSequence.occurrenceId) + $placementText -join "`t"))
		}
		if ($hasCombatBody) {
			$body = $worldSequence.combatBody
			Assert-ExactProperties $body @('maxHp','centerM','radiusM','lifetimePolicy') 'World combat body'
			Assert-JsonInteger $body.maxHp 'World combat HP' 1 1000000000
			Assert-JsonString $body.lifetimePolicy 'World combat lifetime'
			Assert-JsonNumber $body.radiusM 'World combat radius'
			if ($body.lifetimePolicy -cne 'UNTIL_DESTROYED' -or $body.radiusM -le .001 -or $body.radiusM -gt 1000 -or
				$body.centerM -isnot [Array] -or @($body.centerM).Count -ne 3 -or $worldSequence.anchorKind -cne 'NONE' -or
				$null -ne $worldSequence.PSObject.Properties['walkableSurface']) { throw 'World combat body requires one stationary non-support cue' }
			foreach ($number in $body.centerM) {
				Assert-JsonNumber $number 'World combat center'
				if ([math]::Abs([double]$number) -gt 100000) { throw 'World combat center exceeds bounds' }
			}
			$numbers = @($body.centerM) + @($body.radiusM)
			$text = @($numbers | ForEach-Object { Format-InvariantSignedFloat $_ 'World combat geometry' })
			$patternRows.Add((@('PATTERNWORLDCOMBAT',$koukuEncounterDocument.encounterId,$koukuPattern.patternId,
				$worldSequence.occurrenceId,[uint32]$body.maxHp) + $text -join "`t"))
		}
		if ($null -ne $worldSequence.PSObject.Properties['walkableSurface']) {
			$surface = $worldSequence.walkableSurface
			Assert-ExactProperties $surface @('centerX','centerZ','heightY','radiusM','windows') 'World walkable surface'
			foreach ($field in @('centerX','centerZ','heightY','radiusM')) {
				Assert-JsonNumber $surface.$field "World surface $field"
				if ([math]::Abs([double]$surface.$field) -gt 100000) { throw "World surface $field exceeds runtime range" }
			}
			if ($surface.radiusM -le 0 -or $surface.radiusM -gt 1000 -or $surface.windows -isnot [Array] -or
				@($surface.windows).Count -lt 1 -or @($surface.windows).Count -gt 32) { throw 'World surface radius/windows invalid' }
			$previousEnd = 0
			$cueEndTick = [uint32][math]::Ceiling([double]$worldSequence.durationMs * 30.0 / 1000.0)
			foreach ($window in $surface.windows) {
				Assert-ExactProperties $window @('startTick','endTick') 'World surface window'
				Assert-JsonInteger $window.startTick 'World surface startTick' 0 18000
				Assert-JsonInteger $window.endTick 'World surface endTick' 1 18000
				if ($window.startTick -lt $previousEnd -or $window.startTick -ge $window.endTick -or
					$window.endTick -gt $cueEndTick) { throw 'World surface windows overlap or exceed the WORLD cue' }
				$previousEnd = $window.endTick
				$patternRows.Add((@('PATTERNWORLDSUPPORT', $koukuEncounterDocument.encounterId,
					$koukuPattern.patternId, $worldSequence.occurrenceId, [uint32]$window.startTick, [uint32]$window.endTick,
					(Format-InvariantSignedFloat $surface.centerX 'World surface X'),
					(Format-InvariantSignedFloat $surface.centerZ 'World surface Z'),
					(Format-InvariantSignedFloat $surface.heightY 'World surface height'),
					(Format-InvariantFloat $surface.radiusM 'World surface radius')) -join "`t"))
			}
		}

	}
	foreach ($sceneProfile in @($koukuPattern.sceneProfiles)) {
		Assert-ExactProperties $sceneProfile @(
			'renderingProfileId','startMs','durationMs','blendMs') 'KoukuSaydon scene profile cue'
		Assert-JsonString $sceneProfile.renderingProfileId `
			'KoukuSaydon scene profile cue renderingProfileId'
		Assert-StableId $sceneProfile.renderingProfileId `
			'KoukuSaydon scene profile cue renderingProfileId'
		foreach ($field in @('startMs','durationMs','blendMs')) {
			Assert-JsonInteger $sceneProfile.$field `
				"KoukuSaydon scene profile cue $field" 0 600000
		}
		if ([uint32]$sceneProfile.durationMs -eq 0 -or
			([uint64]$sceneProfile.startMs + [uint64]$sceneProfile.durationMs) -gt
				$koukuPatternDurationMs) {
			throw "KoukuSaydon scene profile cue is invalid: $($koukuPattern.patternId)/$($sceneProfile.renderingProfileId)"
		}
		$patternRows.Add((@(
			'PATTERNSCENEPROFILE', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, [uint32]$sceneProfile.startMs,
			[uint32]$sceneProfile.durationMs, [uint32]$sceneProfile.blendMs,
			$sceneProfile.renderingProfileId) -join "`t"))
	}
}
foreach ($followupTarget in $koukuFollowupTargets) {
	if (-not $koukuPatternIds.Contains($followupTarget)) {
		throw "KoukuSaydon follow-up outcome names a pattern outside the Product: $followupTarget"
	}
}

# Bundle members resolve the same admitted Product patterns; they never clone their stage rows.
if ($koukuEncounterDocument.folders -isnot [Array] -or @($koukuEncounterDocument.folders).Count -gt 4096 -or
    $koukuEncounterDocument.bundles -isnot [Array] -or @($koukuEncounterDocument.bundles).Count -gt 4096) {
    throw 'KoukuSaydon hierarchy collections are invalid.'
}
$koukuFolderById = @{}
foreach ($folder in @($koukuEncounterDocument.folders)) {
    $folderProperties = @('folderId','gateId','displayName')
    if ($null -ne $folder.PSObject.Properties['timelinePatternId']) {
        $folderProperties += 'timelinePatternId'
        Assert-StableId $folder.timelinePatternId 'Parent timelinePatternId'
        $parentTimeline = @($koukuEncounterDocument.patterns | Where-Object { $_.patternId -ceq $folder.timelinePatternId })
        if ($parentTimeline.Count -ne 1 -or $parentTimeline[0].gateId -cne $folder.gateId -or
            $parentTimeline[0].folderId -cne $folder.folderId) { throw 'Parent timeline must name its own same-Gate Pattern' }
    }
    Assert-ExactProperties $folder $folderProperties 'KoukuSaydon folder'
    foreach ($field in @('folderId','gateId','displayName')) { Assert-JsonString $folder.$field "Folder $field" }
    Assert-StableId $folder.folderId 'folderId'
    if ($folder.gateId -cnotin @('GATE1','GATE2','GATE3','BINGO') -or $koukuFolderById.ContainsKey([string]$folder.folderId)) {
        throw 'Duplicate folder or unknown Gate.'
    }
    $koukuFolderById[[string]$folder.folderId] = $folder
}
# Parent classification does not alter the Pattern's authoritative playback rows.
foreach ($koukuPattern in @($koukuEncounterDocument.patterns)) {
    if ($null -eq $koukuPattern.PSObject.Properties['folderId']) { continue }
    Assert-JsonString $koukuPattern.folderId 'KoukuSaydon pattern folderId'
    Assert-StableId $koukuPattern.folderId 'KoukuSaydon pattern folderId'
    if (-not $koukuFolderById.ContainsKey([string]$koukuPattern.folderId) -or
        [string]$koukuFolderById[[string]$koukuPattern.folderId].gateId -cne [string]$koukuPattern.gateId) {
        throw "KoukuSaydon Pattern requires a same-Gate Parent: $($koukuPattern.patternId)"
    }
}
$koukuBundleIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($bundle in @($koukuEncounterDocument.bundles)) {
    Assert-ExactProperties $bundle @('bundleId','gateId','folderId','displayName','durationMs','members') 'KoukuSaydon bundle'
    foreach ($field in @('bundleId','gateId','folderId','displayName')) { Assert-JsonString $bundle.$field "Bundle $field" }
    Assert-StableId $bundle.bundleId 'bundleId'
    Assert-JsonInteger $bundle.durationMs 'bundle durationMs' 1 600000
    if (-not $koukuBundleIds.Add([string]$bundle.bundleId) -or -not $koukuFolderById.ContainsKey([string]$bundle.folderId) -or
        [string]$koukuFolderById[[string]$bundle.folderId].gateId -cne [string]$bundle.gateId -or
        $bundle.members -isnot [Array] -or @($bundle.members).Count -lt 1 -or @($bundle.members).Count -gt 8) {
        throw 'Bundle identity, folder/Gate or member count is invalid.'
    }
    $memberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $memberPatterns = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $memberTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $statefulOwners = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $sceneOwners = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $followupSceneOwners = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $sceneWindows = [Collections.Generic.List[object]]::new()
    $worldOwners = @{}
    $patternRows.Add((@('PATTERNBUNDLE', $bundle.bundleId, $koukuEncounterDocument.encounterId, $bundle.gateId) -join "`t"))
    foreach ($member in @($bundle.members)) {
        Assert-ExactProperties $member @('memberId','patternId','targetBossPlacementId','actorProfileId','startOffsetMs') 'Bundle member'
        foreach ($field in @('memberId','patternId','targetBossPlacementId','actorProfileId')) {
            Assert-JsonString $member.$field "Bundle member $field"
            Assert-StableId $member.$field "Bundle member $field"
        }
        Assert-JsonInteger $member.startOffsetMs 'member startOffsetMs' 0 600000
        if (-not $memberIds.Add([string]$member.memberId) -or -not $memberPatterns.Add([string]$member.patternId) -or
            -not $memberTargets.Add([string]$member.targetBossPlacementId) -or -not $koukuPatternById.ContainsKey([string]$member.patternId)) {
            throw 'Bundle contains duplicate member/pattern/target or missing Pattern.'
        }
        $child = $koukuPatternById[[string]$member.patternId]
        if ([string]$child.gateId -cne [string]$bundle.gateId -or [string]$child.targetBossPlacementId -cne [string]$member.targetBossPlacementId -or
            [string]$child.actorProfileId -cne [string]$member.actorProfileId) { throw 'Bundle child Gate/target/model mismatch.' }
        [uint64]$childDuration = 0
        foreach ($stage in @($child.stages)) { $childDuration += [uint64]$stage.durationMs }
        if ($childDuration + [uint64]$member.startOffsetMs -gt [uint64]$bundle.durationMs) { throw 'Bundle member exceeds preview span.' }
        # Member start alone uses ceil-to-30-Hz; presentation rows retain authored milliseconds.
        [uint64]$offsetUnits = [uint64][Math]::Ceiling([double]$member.startOffsetMs * 30.0 / 1000.0) * 1000
        foreach ($scene in @($child.sceneProfiles)) {
            $sceneWindows.Add([pscustomobject]@{ Owner = [string]$member.memberId;
                Start = $offsetUnits + [uint64]$scene.startMs * 30;
                End = $offsetUnits + ([uint64]$scene.startMs + [uint64]$scene.durationMs) * 30 })
        }
        $pending = [Collections.Generic.Stack[string]]::new()
        $pending.Push([string]$member.patternId)
        $visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        while ($pending.Count -gt 0) {
            $patternId = $pending.Pop()
            if (-not $visited.Add($patternId)) { continue }
            if (-not $koukuPatternById.ContainsKey($patternId)) { throw 'Bundle follow-up Pattern is missing.' }
            $reachable = $koukuPatternById[$patternId]
            if ([string]$reachable.gateId -cne [string]$bundle.gateId -or [string]$reachable.targetBossPlacementId -cne [string]$member.targetBossPlacementId) {
                throw 'Bundle follow-up must retain Gate and target boss.'
            }
            if (@($reachable.sceneProfiles).Count -gt 0) { [void]$sceneOwners.Add([string]$member.memberId) }
            foreach ($trigger in @($reachable.mechanicTriggers)) {
                if ($trigger.kind -cin @('HUD_ENTER','CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','CARD_MAZE_STAGE_PLAYERS')) { [void]$statefulOwners.Add([string]$member.memberId) }
            }
            foreach ($window in @($reachable.logicWindows)) {
                if ($window.kind -cin @('POSE_INPUT','ROULETTE_CARD_MATCH')) { [void]$statefulOwners.Add([string]$member.memberId) }
                foreach ($outcome in @($window.onSuccess) + @($window.onFail) + @($window.onTimeout)) {
                    if ($outcome.kind -ceq 'CLOWN_TRANSFORM') { [void]$statefulOwners.Add([string]$member.memberId) }
                    if ($outcome.kind -ceq 'FOLLOWUP_PATTERN' -or ($outcome.kind -ceq 'MARIO_ENTER' -and -not [string]::IsNullOrEmpty([string]$outcome.patternId))) {
                        $followupId = [string]$outcome.patternId
                        if ($koukuPatternById.ContainsKey($followupId) -and @($koukuPatternById[$followupId].sceneProfiles).Count -gt 0) {
                            [void]$followupSceneOwners.Add([string]$member.memberId)
                        }
                        $pending.Push($followupId)
                    }
                }
            }
            foreach ($worldCue in @($reachable.worldSequences)) {
                $worldId = [string]$worldCue.sequenceInstanceId
                if ($worldOwners.ContainsKey($worldId) -and [string]$worldOwners[$worldId] -cne [string]$member.memberId) {
                    throw 'Bundle members share a live WORLD sequence instance.'
                }
                $worldOwners[$worldId] = [string]$member.memberId
            }
        }
        $patternRows.Add((@('PATTERNBUNDLEMEMBER', $bundle.bundleId, $member.memberId, $member.patternId,
            $member.targetBossPlacementId, [uint32]$member.startOffsetMs) -join "`t"))
    }
    if ($statefulOwners.Count -gt 1) { throw 'Bundle contains multiple owners of shared player mode.' }
    if ($followupSceneOwners.Count -gt 0 -and $sceneOwners.Count -gt 1) { throw 'Conditional follow-up Scene Profile conflicts with another owner.' }
    for ($left = 0; $left -lt $sceneWindows.Count; ++$left) {
        for ($right = $left + 1; $right -lt $sceneWindows.Count; ++$right) {
            $a = $sceneWindows[$left]; $b = $sceneWindows[$right]
            if ($a.Owner -cne $b.Owner -and $a.Start -lt $b.End -and $b.Start -lt $a.End) {
                throw 'Bundle overlaps global Scene Profile owners.'
            }
        }
    }
}

Add-KoukuRaidRows $koukuEncounterDocument $patternRows

if (@($koukuEncounterDocument.playAllPatternIds).Count -ne
	$koukuProductOrder.Count) {
	throw 'KoukuSaydon playAllPatternIds count differs from Product patterns.'
}
for ($playAllIndex = 0; $playAllIndex -lt $koukuProductOrder.Count;
	++$playAllIndex) {
	Assert-JsonString $koukuEncounterDocument.playAllPatternIds[$playAllIndex] `
		'KoukuSaydon playAll patternId'
	if ([string]$koukuEncounterDocument.playAllPatternIds[$playAllIndex] -cne
		[string]$koukuProductOrder[$playAllIndex]) {
		throw 'KoukuSaydon playAllPatternIds must preserve Product order.'
	}
	$pursuitAfterMs = if ($playAllIndex + 1 -lt $koukuProductOrder.Count) {
		100
	}
	else { 0 }
	$patternRows.Add((@(
		'PATTERNSEQUENCESTEP', $koukuEncounterDocument.encounterId,
		'KAKULSAYDON_G1_PLAY_ALL', $playAllIndex,
		$koukuProductOrder[$playAllIndex], $pursuitAfterMs) -join "`t"))
}
$patternRows.Add((@(
	'PATTERNSEQUENCE', $koukuEncounterDocument.encounterId,
	'KAKULSAYDON_G1_PLAY_ALL', 'ORDERED_ONCE_THEN_IDLE', 100,
	$koukuProductOrder.Count) -join "`t"))

}
