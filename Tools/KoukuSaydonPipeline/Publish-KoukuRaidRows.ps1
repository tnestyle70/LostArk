# Called inside the gameplay publisher after normal Pattern/Bundle validation.
function Add-KoukuRaidRows([object]$Encounter, [object]$Rows) {
    if ($null -eq $Encounter.PSObject.Properties['raidGates']) { return }
    if ($Encounter.raidGates -isnot [Array] -or @($Encounter.raidGates).Count -gt 4) {
        throw 'Kouku raidGates must contain at most four gates.'
    }
    $seenGates = @{}
    foreach ($gate in $Encounter.raidGates) {
        $gateProperties = @('gateId','flowId','sequenceCompositionId','sequenceRevision',
            'introPatternId','introDurationMs','clearPatternId','clearDurationMs','primaryBossPlacementId',
            'entries','arrivals')
        if ($null -ne $gate.PSObject.Properties['entrySequenceInstanceId']) { $gateProperties += 'entrySequenceInstanceId' }
        if ($null -ne $gate.PSObject.Properties['loopStartEntryId']) { $gateProperties += 'loopStartEntryId' }
        if ($null -ne $gate.PSObject.Properties['entryGroups']) { $gateProperties += 'entryGroups' }
        if ($null -ne $gate.PSObject.Properties['bingoSpecialPatternId']) { $gateProperties += 'bingoSpecialPatternId' }
        Assert-ExactProperties $gate $gateProperties 'Kouku raid gate'
        if ($gate.gateId -cnotin @('GATE1','GATE2','GATE3','BINGO') -or $seenGates.ContainsKey($gate.gateId)) {
            throw 'Kouku raid gate is unknown or duplicated.'
        }
        $seenGates[$gate.gateId] = $true
        foreach ($field in @('flowId','sequenceCompositionId','primaryBossPlacementId')) {
            Assert-StableId $gate.$field "Kouku raid $field"
        }
        Assert-JsonInteger $gate.sequenceRevision 'Kouku raid sequence revision' 1 ([uint32]::MaxValue)
        if ($gate.gateId -ceq 'BINGO' -and $gate.introPatternId -ceq '') {
            if ($gate.introDurationMs -ne 0) { throw 'A missing Bingo intro must have zero duration.' }
        } else {
            Assert-StableId $gate.introPatternId 'Kouku raid intro pattern'
            Assert-JsonInteger $gate.introDurationMs 'Kouku raid intro duration' 1 600000
        }
        Assert-JsonInteger $gate.clearDurationMs 'Kouku raid clear duration' 0 600000
        if ($gate.clearPatternId -isnot [string] -or
            ([string]::IsNullOrEmpty($gate.clearPatternId) -ne ($gate.clearDurationMs -eq 0))) {
            throw 'Kouku raid clear sequence needs both ID and positive duration.'
        }
        if ($gate.clearPatternId) { Assert-StableId $gate.clearPatternId 'Kouku raid clear pattern' }
        if ($gate.entries -isnot [Array] -or @($gate.entries).Count -lt 1 -or @($gate.entries).Count -gt 256) {
            throw 'Kouku raid gate needs 1..256 saved flow entries.'
        }
        $clearId = if ($gate.clearPatternId) { $gate.clearPatternId } else { 'NONE' }
        $entrySequence = 'NONE'
        if ($null -ne $gate.PSObject.Properties['entrySequenceInstanceId']) {
            if ($gate.entrySequenceInstanceId -isnot [string]) { throw 'Kouku raid entry sequence must be a string.' }
            if ($gate.entrySequenceInstanceId) {
                if ($gate.gateId -cne 'GATE1') { throw 'Only Gate 1 admits a world collider raid entry.' }
                Assert-StableId $gate.entrySequenceInstanceId 'Kouku raid entry sequence'
                $entrySequence = $gate.entrySequenceInstanceId
            }
        }
        $loopStart = ''
        if ($null -ne $gate.PSObject.Properties['loopStartEntryId']) {
            if ($gate.loopStartEntryId -isnot [string]) { throw 'Kouku raid loop start must be a string.' }
            $loopStart = $gate.loopStartEntryId
            if ($loopStart) {
                Assert-StableId $loopStart 'Kouku raid loop start'
                if (@($gate.entries | Where-Object { $_.entryId -ceq $loopStart }).Count -ne 1) {
                    throw 'Kouku raid loop start must reference exactly one saved entry.'
                }
            }
        }
        $gateRow = @('RAIDGATE',$Encounter.encounterId,$gate.gateId,$gate.flowId,$gate.sequenceCompositionId,
            $gate.sequenceRevision,$(if ($gate.introPatternId) { $gate.introPatternId } else { 'NONE' }),$gate.introDurationMs,$clearId,$gate.clearDurationMs,
            $gate.primaryBossPlacementId,@($gate.entries).Count,$entrySequence)
        if ($loopStart) { $gateRow += $loopStart }
        $Rows.Add(($gateRow -join "`t"))
        if ($gate.gateId -ceq 'BINGO') {
            if ($null -eq $gate.PSObject.Properties['bingoSpecialPatternId']) { throw 'Bingo flow requires an explicit special Parent.' }
            Assert-StableId $gate.bingoSpecialPatternId 'Bingo special Parent'
            $specials = @($Encounter.patterns | Where-Object { $_.patternId -ceq $gate.bingoSpecialPatternId })
            if ($specials.Count -ne 1) { throw 'Bingo special Parent must reference one published pattern.' }
            $special = $specials[0]
            $specialDuration = [uint64]0
            foreach ($stage in $special.stages) { $specialDuration += [uint64]$stage.durationMs }
            if ($null -ne $special.PSObject.Properties['timelineDurationMs']) {
                $specialDuration = [Math]::Max($specialDuration, [uint64]$special.timelineDurationMs)
            }
            if ($special.gateId -cne 'BINGO' -or $special.targetBossPlacementId -cne $gate.primaryBossPlacementId -or
                $null -eq $special.PSObject.Properties['fixedTimeline'] -or -not $special.fixedTimeline -or
                $specialDuration -lt 1 -or $specialDuration -gt 600000) {
                throw 'Bingo special Parent must be finite and share the flow boss scope.'
            }
            $children = @()
            if ($null -ne $special.PSObject.Properties['parentPatternSequence']) {
                if (@($special.parentPatternSequence.entries).Count -eq 0 -or $special.parentPatternSequence.loopStartOccurrenceId) {
                    throw 'Bingo special Parent must have ordered finite children.'
                }
                $children = @($special.parentPatternSequence.entries)
            }
            $detonations = @($special.mechanicTriggers | Where-Object { $_.kind -ceq 'BINGO_DETONATION' }).Count
            foreach ($childRef in $children) {
                $childPatterns = @($Encounter.patterns | Where-Object { $_.patternId -ceq $childRef.patternId })
                if ($childPatterns.Count -ne 1 -or $childPatterns[0].gateId -cne 'BINGO' -or
                    $childPatterns[0].targetBossPlacementId -cne $gate.primaryBossPlacementId) { throw 'Bingo special child scope is invalid.' }
                $detonations += @($childPatterns[0].mechanicTriggers | Where-Object { $_.kind -ceq 'BINGO_DETONATION' }).Count
            }
            if ($detonations -ne 1) { throw 'Bingo special Parent must own exactly one detonation.' }
            $Rows.Add((@('RAIDBINGOSPECIAL',$gate.gateId,$gate.bingoSpecialPatternId) -join "`t"))
        } elseif ($null -ne $gate.PSObject.Properties['bingoSpecialPatternId']) {
            throw 'Only Bingo may bind a special Parent.'
        }
        $entryIds = @{}
        for ($index = 0; $index -lt @($gate.entries).Count; ++$index) {
            $entry = $gate.entries[$index]
            Assert-ExactProperties $entry @('entryId','kind','targetId','waitAfterMs') 'Kouku raid flow entry'
            Assert-StableId $entry.entryId 'Kouku raid entry ID'
            Assert-StableId $entry.targetId 'Kouku raid target ID'
            Assert-JsonInteger $entry.waitAfterMs 'Kouku raid entry wait' 0 600000
            if ($entryIds.ContainsKey($entry.entryId) -or $entry.kind -cnotin @('PATTERN','BUNDLE')) {
                throw 'Kouku raid flow entry is duplicated or has an unsupported kind.'
            }
            $entryIds[$entry.entryId] = $true
            $targets = if ($entry.kind -ceq 'PATTERN') {
                @($Encounter.patterns | Where-Object { $_.patternId -ceq $entry.targetId -and $_.gateId -ceq $gate.gateId })
            } else {
                @($Encounter.bundles | Where-Object { $_.bundleId -ceq $entry.targetId -and $_.gateId -ceq $gate.gateId })
            }
            if (@($targets).Count -ne 1) { throw "Kouku raid flow target must exact-join its published gate: $($gate.gateId) / $($entry.targetId), matches=$(@($targets).Count)." }
            if ($gate.gateId -ceq 'BINGO' -and (($entry.kind -ceq 'PATTERN' -and $entry.targetId -ceq $gate.bingoSpecialPatternId) -or
                ($entry.kind -ceq 'BUNDLE' -and @($targets[0].members | Where-Object { $_.patternId -ceq $gate.bingoSpecialPatternId }).Count -gt 0))) {
                throw 'Bingo special Parent cannot also be a normal flow entry or bundle member.'
            }
            $Rows.Add((@('RAIDFLOWSTEP',$gate.gateId,$index,$entry.entryId,$entry.kind,$entry.targetId,$entry.waitAfterMs) -join "`t"))
        }
        if ($null -ne $gate.PSObject.Properties['entryGroups']) {
            if ($gate.entryGroups -isnot [Array] -or @($gate.entryGroups).Count -gt 64) { throw 'Kouku raid supports at most 64 flow groups.' }
            $groupIds = @{}
            $entryIndices = @{}
            for ($index = 0; $index -lt @($gate.entries).Count; ++$index) { $entryIndices[$gate.entries[$index].entryId] = $index }
            $nextStart = 0
            $groupIndex = 0
            foreach ($group in $gate.entryGroups) {
                $properties = @('groupId','displayName','startEntryId','endEntryId')
                $repeat = $null -ne $group.PSObject.Properties['repeatUntilHealthBars']
                if ($repeat) { $properties += @('repeatUntilHealthBars','transitionAt') }
                Assert-ExactProperties $group $properties 'Kouku raid flow group'
                foreach ($field in @('groupId','startEntryId','endEntryId')) { Assert-StableId $group.$field "Kouku raid group $field" }
                if ($group.displayName -isnot [string] -or [string]::IsNullOrWhiteSpace($group.displayName) -or
                    $groupIds.ContainsKey($group.groupId) -or -not $entryIndices.ContainsKey($group.startEntryId) -or
                    -not $entryIndices.ContainsKey($group.endEntryId)) { throw 'Kouku raid group identity, name or entry reference is invalid.' }
                $first = $entryIndices[$group.startEntryId]
                $last = $entryIndices[$group.endEntryId]
                if ($first -lt $nextStart -or $last -lt $first) { throw 'Kouku raid group ranges must be ordered and disjoint.' }
                $nextStart = $last + 1
                $groupIds[$group.groupId] = $true
                $threshold = 'NONE'
                $boundary = 'PATTERN_END'
                if ($repeat) {
                    Assert-JsonInteger $group.repeatUntilHealthBars 'Kouku raid HP repeat threshold' 0 1000
                    if ($loopStart -or $group.transitionAt -cnotin @('PATTERN_END','GROUP_END')) { throw 'Kouku raid HP repeat cannot mix with legacy loops and requires a completion boundary.' }
                    $threshold = $group.repeatUntilHealthBars
                    $boundary = $group.transitionAt
                }
                $Rows.Add((@('RAIDFLOWGROUP',$gate.gateId,$groupIndex,$group.groupId,$group.startEntryId,$group.endEntryId,$threshold,$boundary) -join "`t"))
                ++$groupIndex
            }
        }
        if ($gate.arrivals -isnot [Array] -or @($gate.arrivals).Count -gt 8) { throw 'Kouku raid arrivals exceed four slots per segment.' }
        $arrivalSlots = @{}
        $arrivalIds = @{}
        foreach ($arrival in $gate.arrivals) {
            Assert-ExactProperties $arrival @('phase','occurrenceId','playerSlot','startMs','position') 'Kouku raid arrival'
            Assert-StableId $arrival.occurrenceId 'Kouku raid arrival ID'
            Assert-JsonInteger $arrival.playerSlot 'Kouku raid arrival slot' 0 3
            $key = "$($arrival.phase).$($arrival.playerSlot)"
            if ($arrival.phase -cnotin @('INTRO','CLEAR') -or $arrivalSlots.ContainsKey($key) -or
                $arrivalIds.ContainsKey($arrival.occurrenceId)) { throw 'Kouku raid arrival is unknown or duplicated.' }
            $arrivalSlots[$key] = $true
            $arrivalIds[$arrival.occurrenceId] = $true
            if ($arrival.phase -ceq 'CLEAR' -and -not $gate.clearPatternId) { throw 'Kouku raid CLEAR arrival requires a clear sequence.' }
            $duration = if ($arrival.phase -ceq 'INTRO') { $gate.introDurationMs } else { $gate.clearDurationMs }
            Assert-JsonInteger $arrival.startMs 'Kouku raid arrival clock' 0 $duration
            if ($arrival.position -isnot [Array] -or @($arrival.position).Count -ne 3) { throw 'Kouku raid arrival needs XYZ.' }
            foreach ($value in $arrival.position) {
                Assert-JsonNumber $value 'Kouku raid arrival coordinate'
                if ([Math]::Abs([double]$value) -gt 100000) { throw 'Kouku raid arrival is outside world bounds.' }
            }
            $xyz = @($arrival.position | ForEach-Object { ([double]$_).ToString('R',[Globalization.CultureInfo]::InvariantCulture) })
            $Rows.Add(((@('RAIDARRIVAL',$gate.gateId,$arrival.phase,$arrival.occurrenceId,$arrival.playerSlot,$arrival.startMs) + $xyz) -join "`t"))
        }
        foreach ($slot in 0..3) {
            if ($gate.gateId -cne 'BINGO' -and -not $arrivalSlots.ContainsKey("INTRO.$slot")) { throw 'Kouku raid intro must define all four room arrival slots.' }
        }
    }
}
