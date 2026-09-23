# One Parent/child Product contract shared by Gameplay and World publication.
# No file reads, output writes or gameplay row emission occur in this helper.
function Assert-KoukuParentPatternSequence {
    param([object]$Pattern, [object[]]$Patterns)
    if ($null -eq $Pattern.PSObject.Properties['parentPatternSequence']) { return }
    $exact = {
        param($Value, [string[]]$Expected, [string]$Context)
        [string[]]$actual = @($Value.PSObject.Properties.Name)
        [Array]::Sort($actual, [StringComparer]::Ordinal)
        [Array]::Sort($Expected, [StringComparer]::Ordinal)
        if (($actual -join "`n") -cne ($Expected -join "`n")) { throw "$Context has missing or unknown fields." }
    }
    $integer = {
        param($Value, [string]$Context, [long]$Minimum, [long]$Maximum)
        if (($Value -isnot [int]) -and ($Value -isnot [long]) -and ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
            throw "$Context must be a JSON integer."
        }
        if ([decimal]$Value -lt $Minimum -or [decimal]$Value -gt $Maximum) { throw "$Context is out of range." }
    }
    $parent = $Pattern.parentPatternSequence
    & $exact $parent @('loopStartOccurrenceId','entries') 'Parent sequence'
    if ($parent.loopStartOccurrenceId -isnot [string] -or
        ($parent.loopStartOccurrenceId -cne '' -and $parent.loopStartOccurrenceId -cnotmatch '^[A-Za-z0-9_.-]{1,128}$')) {
        throw 'Parent loop start must be an empty string or stable occurrence ID.'
    }
    if (($parent.loopStartOccurrenceId -cne '' -and $Pattern.gateId -cne 'BINGO') -or
        $parent.entries -isnot [Array] -or @($parent.entries).Count -lt 1 -or @($parent.entries).Count -gt 128) {
        throw 'Invalid retained Bingo Parent sequence.'
    }
    & $integer $Pattern.timelineDurationMs 'Parent timeline duration' 1 600000
    $entryIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    [uint64]$parentEnd = 0
    foreach ($entry in @($parent.entries)) {
        & $exact $entry @('occurrenceId','patternId','startMs','durationMs','repeat') 'Parent child'
        foreach ($field in @('occurrenceId','patternId')) {
            if ($entry.$field -isnot [string] -or $entry.$field -cnotmatch '^[A-Za-z0-9_.-]{1,128}$') {
                throw "Parent child $field must be a stable ID."
            }
        }
        & $integer $entry.startMs 'Parent child start' 0 600000
        & $integer $entry.durationMs 'Parent child duration' 1 600000
        if (-not $entryIds.Add([string]$entry.occurrenceId) -or $entry.repeat -isnot [bool] -or $entry.repeat -or
            [uint64]$entry.startMs -lt $parentEnd) { throw 'Parent children overlap or repeat an invalid identity.' }
        $child = @($Patterns | Where-Object { $_.patternId -ceq $entry.patternId })
        if ($child.Count -ne 1 -or $child[0].patternId -ceq $Pattern.patternId -or
            $null -ne $child[0].PSObject.Properties['parentPatternSequence'] -or
            $child[0].gateId -cne $Pattern.gateId -or $child[0].targetBossPlacementId -cne $Pattern.targetBossPlacementId -or
            $child[0].actorProfileId -cne $Pattern.actorProfileId) { throw 'Parent child must be an independent Pattern on the same boss.' }
        [uint64]$childMs = 0
        foreach ($stage in @($child[0].stages)) {
            & $integer $stage.durationMs 'Parent referenced child stage duration' 1 600000
            $childMs += [uint64]$stage.durationMs
        }
        if ($null -ne $child[0].PSObject.Properties['timelineDurationMs']) {
            & $integer $child[0].timelineDurationMs 'Parent referenced child timeline duration' 1 600000
            $childMs = [Math]::Max($childMs, [uint64]$child[0].timelineDurationMs)
        }
        if ([uint64]$entry.durationMs -ne $childMs) { throw 'Parent child must retain its complete authored duration.' }
        $parentEnd = [uint64]$entry.startMs + [uint64]$entry.durationMs
    }
    if (($parent.loopStartOccurrenceId -cne '' -and -not $entryIds.Contains([string]$parent.loopStartOccurrenceId)) -or
        $parentEnd -ne [uint64]$Pattern.timelineDurationMs) { throw 'Parent loop reference or total duration is invalid.' }
}
