[CmdletBinding()]
param(
    [ValidateSet('Migrate', 'RebuildNavigation', 'CheckNavigation')]
    [string]$Mode = 'Migrate',
    [string]$BaseNavGridPath = 'Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$eventsPath = Join-Path $repoRoot 'Data\Encounters\Valtan\ValtanWorldEvents.json'
$simulationPath = Join-Path $repoRoot 'Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.destructionsimulation.json'
$gameplayPath = Join-Path $repoRoot 'Data\Worlds\LV_LUT_HEARTRB_ED\Gameplay.world.json'
$navPath = Join-Path $repoRoot 'Data\Navigation\LV_LUT_HEARTRB_ED.navblockers'
. (Join-Path $PSScriptRoot 'ValtanWallNavigation.ps1')
$resolvedNavGridPath = if ([IO.Path]::IsPathRooted($BaseNavGridPath)) {
    [IO.Path]::GetFullPath($BaseNavGridPath)
} else { Join-Path $repoRoot $BaseNavGridPath }
$wallNavigationGrid = Read-ValtanWallNavigationGrid $resolvedNavGridPath

$combinedImpactGroupId = 'destroyable.group.valtan.deploy.11047903315509031966'
$outerPrefix = 'destroyable.group.valtan.outerwall109.sector'
$oldImpactReceiverId = 'collision.valtan.wallgroup.11047903315509031966.receiver'

function Read-Json([string]$path) {
    return [IO.File]::ReadAllText($path, [Text.Encoding]::UTF8) | ConvertFrom-Json
}

function Clone-Object([object]$value) {
    return $value | ConvertTo-Json -Depth 100 | ConvertFrom-Json
}

function Write-StagedJson([string]$path, [object]$value, [string]$stagingRoot) {
    $staged = Join-Path $stagingRoot ([IO.Path]::GetFileName($path))
    [IO.File]::WriteAllText(
        $staged,
        ($value | ConvertTo-Json -Depth 100) + "`n",
        [Text.UTF8Encoding]::new($false))
    return $staged
}

function Get-Tokens([string]$line) {
    return @([regex]::Matches($line, '"[^"]*"|\S+') |
        ForEach-Object { $_.Value.Trim('"') })
}

function Read-NavDocument([string]$path) {
    $lines = @([IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8))
    $header = @(Get-Tokens $lines[0])
    if ($header.Count -ne 9 -or $header[0] -ne 'LOSTARK_NAVGRID_BLOCKERS') {
        throw 'Unexpected navigation blocker header.'
    }
    $regions = [Collections.Generic.List[object]]::new()
    $cursor = 1
    for ($index = 0; $index -lt [int]$header[8]; ++$index) {
        $tokens = @(Get-Tokens $lines[$cursor++])
        if ($tokens.Count -ne 5 -or $tokens[0] -ne 'REGION') {
            throw 'Unexpected navigation blocker region row.'
        }
        $cells = [Collections.Generic.List[object]]::new()
        for ($cellIndex = 0; $cellIndex -lt [int]$tokens[4]; ++$cellIndex) {
            $cell = @(Get-Tokens $lines[$cursor++])
            if ($cell.Count -ne 2) { throw 'Unexpected navigation blocker cell row.' }
            $cells.Add([pscustomobject]@{ X=[int]$cell[0]; Y=[int]$cell[1] })
        }
        $regions.Add([pscustomobject]@{
            RegionId=[string]$tokens[1]
            ConditionId=[string]$tokens[2]
            ActivateWhenTrue=[int]$tokens[3]
            Cells=@($cells)
        })
    }
    if ($cursor -ne $lines.Count) { throw 'Navigation blocker document has trailing rows.' }
    return [pscustomobject]@{
        AreaId=[string]$header[2]
        Width=[int]$header[3]
        Height=[int]$header[4]
        CellSize=[double]$header[5]
        OriginX=[double]$header[6]
        OriginZ=[double]$header[7]
        Regions=@($regions)
    }
}

function Write-StagedNav([object]$document, [string]$stagingRoot) {
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add(('LOSTARK_NAVGRID_BLOCKERS 1 "{0}" {1} {2} {3} {4} {5} {6}' -f
        $document.AreaId, $document.Width, $document.Height,
        $document.CellSize.ToString('R',[Globalization.CultureInfo]::InvariantCulture),
        $document.OriginX.ToString('R',[Globalization.CultureInfo]::InvariantCulture),
        $document.OriginZ.ToString('R',[Globalization.CultureInfo]::InvariantCulture),
        @($document.Regions).Count))
    foreach ($region in @($document.Regions)) {
        $lines.Add(('REGION "{0}" "{1}" {2} {3}' -f
            $region.RegionId, $region.ConditionId,
            $region.ActivateWhenTrue, @($region.Cells).Count))
        foreach ($cell in @($region.Cells)) {
            $lines.Add("$($cell.X) $($cell.Y)")
        }
    }
    $staged = Join-Path $stagingRoot ([IO.Path]::GetFileName($navPath))
    [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
    return $staged
}

function Find-SourceCollision([object]$gameplay, [string]$sourceId) {
    $suffix = ".$sourceId"
    $matches = @($gameplay.placements | Where-Object {
        $_.kind -ceq 'collisionBox' -and
        ([string]$_.placementId).EndsWith($suffix, [StringComparison]::Ordinal)
    })
    if ($matches.Count -ne 1) {
        throw "Expected one source collision box for $sourceId, found $($matches.Count)."
    }
    return $matches[0]
}

function Split-RegionByWallFootprint(
    [object]$region,
    [object[]]$elements,
    [object]$gameplay,
    [object]$nav,
    [string]$kind) {
    $result = @{}
    foreach ($element in $elements) {
        $sourceId = [string]$element.sourceRuntimePlacementId
        $collision = Find-SourceCollision $gameplay $sourceId
        $keys = Get-ValtanWallNavigationCellKeys $collision $wallNavigationGrid
        if ($keys.Count -eq 0) {
            $result[$sourceId] = $null
            continue
        }
        $cells = @($keys | ForEach-Object {
            $parts = $_.Split(',')
            [pscustomobject]@{ X=[int]$parts[0]; Y=[int]$parts[1] }
        } | Sort-Object Y,X)
        $result[$sourceId] = [pscustomobject]@{
            RegionId="navregion.valtan.$kind.$sourceId"
            ConditionId="condition.valtan.$kind.$sourceId.destroyed"
            ActivateWhenTrue=0
            Cells=$cells
        }
    }
    return $result
}

function Write-StagedWallNavigationEvents([byte[]]$OriginalBytes, [string]$StagingRoot) {
    $text = [Text.Encoding]::UTF8.GetString($OriginalBytes)
    $original = $text | ConvertFrom-Json
    foreach ($group in $events.groups) {
        $previous = @($original.groups | Where-Object { $_.groupId -ceq $group.groupId })
        if ($previous.Count -ne 1) { throw 'Wall navigation lost a stable group identity.' }
        if ((@($previous[0].navigationRegionIds) -join '|') -ceq (@($group.navigationRegionIds) -join '|')) { continue }
        $pattern = '(?s)("groupId"\s*:\s*"' + [regex]::Escape([string]$group.groupId) +
            '".*?"navigationRegionIds"\s*:\s*\[)(.*?)(\])'
        $matches = [regex]::Matches($text, $pattern)
        if ($matches.Count -ne 1) { throw 'Wall navigation cannot locate its exact JSON field.' }
        $field = $matches[0].Groups[2]
        $indent = [regex]::Match($field.Value, '[ \t]*$').Value
        $newline = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
        $rows = @($group.navigationRegionIds | ForEach-Object { $indent + '    "' + [string]$_ + '"' })
        $replacement = $newline + ($rows -join (',' + $newline)) + $newline + $indent
        $text = $text.Remove($field.Index, $field.Length).Insert($field.Index, $replacement)
    }
    $staged = Join-Path $StagingRoot ([IO.Path]::GetFileName($eventsPath))
    [IO.File]::WriteAllText($staged, $text, [Text.UTF8Encoding]::new($false))
    return $staged
}

function Update-IndependentWallNavigation([bool]$CheckOnly) {
    if ($nav.Width -ne $wallNavigationGrid.Width -or $nav.Height -ne $wallNavigationGrid.Height -or
        $nav.CellSize -ne $wallNavigationGrid.CellSize -or
        $nav.OriginX -ne $wallNavigationGrid.OriginX -or $nav.OriginZ -ne $wallNavigationGrid.OriginZ) {
        throw 'Wall blocker grid does not match the published base navigation.'
    }
    $oldRegions = @{}
    foreach ($region in $nav.Regions) { $oldRegions[$region.RegionId] = $region }
    $wallGroups = @($events.groups | Where-Object { $_.navPolarity -ceq 'BLOCK_WHILE_INTACT' })
    if ($wallGroups.Count -ne 99) { throw 'Navigation repair requires all 99 independent Valtan walls.' }
    $wallRegionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $stagedRegions = [Collections.Generic.List[object]]::new()
    $changedGroups = 0
    foreach ($group in $wallGroups) {
        $profiles = @($simulation.profiles | Where-Object { $_.groupId -ceq $group.groupId })
        if ($profiles.Count -ne 1 -or @($profiles[0].elements).Count -ne 1 -or
            @($group.navigationRegionIds).Count -gt 1) {
            throw "Wall navigation needs one exact source emitter: $($group.groupId)"
        }
        $sourceId = [string]$profiles[0].elements[0].sourceRuntimePlacementId
        $collision = Find-SourceCollision $gameplay $sourceId
        $keys = Get-ValtanWallNavigationCellKeys $collision $wallNavigationGrid
        $stem = ([string]$group.groupId).Substring('destroyable.group.'.Length)
        $regionId = "navregion.$stem"
        $conditionId = "condition.$stem.destroyed"
        $previousKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($oldId in @($group.navigationRegionIds)) {
            if (-not $wallRegionIds.Add([string]$oldId) -or -not $oldRegions.ContainsKey([string]$oldId)) {
                throw "Wall navigation region is shared or missing: $oldId"
            }
            $previous = $oldRegions[[string]$oldId]
            if ($previous.ActivateWhenTrue -ne 0) { throw "Wall navigation polarity is invalid: $oldId" }
            $regionId = [string]$oldId
            $conditionId = [string]$previous.ConditionId
            foreach ($cell in $previous.Cells) { [void]$previousKeys.Add("$($cell.X),$($cell.Y)") }
        }
        if (-not $previousKeys.SetEquals($keys)) { ++$changedGroups }
        if ($keys.Count -eq 0) {
            $group.navigationRegionIds = @()
            continue
        }
        $group.navigationRegionIds = @($regionId)
        $cells = @($keys | ForEach-Object {
            $parts = $_.Split(',')
            [pscustomobject]@{ X=[int]$parts[0]; Y=[int]$parts[1] }
        } | Sort-Object Y,X)
        $stagedRegions.Add([pscustomobject]@{
            RegionId=$regionId; ConditionId=$conditionId; ActivateWhenTrue=0; Cells=$cells
        })
    }
    if ($CheckOnly) {
        if ($changedGroups -ne 0) { throw "Wall navigation differs from its collision footprints: $changedGroups groups. Run -Mode RebuildNavigation." }
        Write-Host "Wall navigation Check succeeded: $($wallGroups.Count) exact wall footprints; floor regions preserved."
        return
    }
    if ($changedGroups -eq 0) {
        Write-Host 'Wall navigation already matches its collision footprints; no files changed.'
        return
    }
    # Preserve every floor region and its cell order. Only wall ownership is
    # rebuilt; this never turns base-blocked ground or collapsed floors on.
    $nav.Regions = @($nav.Regions | Where-Object { -not $wallRegionIds.Contains($_.RegionId) }) +
        @($stagedRegions | Sort-Object RegionId -CaseSensitive)
    $stagingRoot = Join-Path $repoRoot ('_work/valtan-wall-nav-' + [Guid]::NewGuid().ToString('N'))
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    $beforeEvents = [IO.File]::ReadAllBytes($eventsPath)
    $beforeNav = [IO.File]::ReadAllBytes($navPath)
    try {
        $stagedEvents = Write-StagedWallNavigationEvents $beforeEvents $stagingRoot
        $stagedNav = Write-StagedNav $nav $stagingRoot
        # Both complete documents are prepared before either canonical file is
        # replaced. Restore both originals if either promotion fails.
        [IO.File]::Copy($stagedEvents, $eventsPath, $true)
        [IO.File]::Copy($stagedNav, $navPath, $true)
    }
    catch {
        [IO.File]::WriteAllBytes($eventsPath, $beforeEvents)
        [IO.File]::WriteAllBytes($navPath, $beforeNav)
        throw
    }
    finally {
        $safeRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot '_work')) + [IO.Path]::DirectorySeparatorChar
        $resolvedStaging = [IO.Path]::GetFullPath($stagingRoot)
        if (-not $resolvedStaging.StartsWith($safeRoot, [StringComparison]::OrdinalIgnoreCase)) {
            throw 'Wall navigation staging cleanup escaped the workspace.'
        }
        [IO.Directory]::Delete($resolvedStaging, $true)
    }
    Write-Host "Wall navigation rebuilt: $changedGroups changed walls; $($stagedRegions.Count) wall regions; floor regions preserved."
}

$events = Read-Json $eventsPath
$simulation = Read-Json $simulationPath
$gameplay = Read-Json $gameplayPath
$nav = Read-NavDocument $navPath
if ($Mode -ne 'Migrate') {
    Update-IndependentWallNavigation ($Mode -eq 'CheckNavigation')
    return
}

$alreadySplit = @($events.groups | Where-Object {
    ([string]$_.groupId).StartsWith('destroyable.group.valtan.wall159.', [StringComparison]::Ordinal)
}).Count -eq 10 -and
    @($events.groups | Where-Object {
        ([string]$_.groupId).StartsWith('destroyable.group.valtan.outerwall109.109', [StringComparison]::Ordinal)
    }).Count -eq 30
if ($alreadySplit) {
    $repairRegions = [Collections.Generic.List[object]]::new()
    foreach ($group in @($events.groups | Where-Object {
        @($_.navigationRegionIds).Count -eq 0 -and (
            ([string]$_.groupId).StartsWith('destroyable.group.valtan.wall159.', [StringComparison]::Ordinal) -or
            ([string]$_.groupId).StartsWith('destroyable.group.valtan.outerwall109.', [StringComparison]::Ordinal))
    })) {
        $isImpact = ([string]$group.groupId).StartsWith(
            'destroyable.group.valtan.wall159.', [StringComparison]::Ordinal)
        $kind = if ($isImpact) { 'wall159' } else { 'outerwall109' }
        $sourceId = ([string]$group.groupId).Substring(
            ([string]$group.groupId).LastIndexOf('.') + 1)
        $collision = Find-SourceCollision $gameplay $sourceId
        $keys = Get-ValtanWallNavigationCellKeys $collision $wallNavigationGrid
        if ($keys.Count -eq 0) { continue }
        $ownedCells = @($keys | ForEach-Object {
            $parts = $_.Split(',')
            [pscustomobject]@{ X=[int]$parts[0]; Y=[int]$parts[1] }
        } | Sort-Object Y,X)
        $regionId = "navregion.valtan.$kind.$sourceId"
        $conditionId = "condition.valtan.$kind.$sourceId.destroyed"
        $group.navigationRegionIds = @($regionId)
        $repairRegions.Add([pscustomobject]@{
            RegionId=$regionId
            ConditionId=$conditionId
            ActivateWhenTrue=0
            Cells=$ownedCells
        })
    }
    $nav.Regions = @($nav.Regions) + @($repairRegions | Sort-Object -Property RegionId -CaseSensitive)

    # The first migration only split the ten 159 walls and thirty authored 109
    # ring slabs.  The recovered arena still contained fifty-seven source walls
    # in twelve preview-era groups.  Those groups cannot participate in a
    # geometry contact contract: one contact would destroy every source member.
    # Split every remaining profile by its source emitter, while keeping a
    # source's suppress-only duplicate placements in the same leaf group.
    $legacyProfiles = @($simulation.profiles | Where-Object {
        ([string]$_.groupId -ceq 'destroyable.group.valtan.wall.3705102') -or
        (-not ([string]$_.groupId).StartsWith(
            'destroyable.group.valtan.wall159.', [StringComparison]::Ordinal) -and
         -not ([string]$_.groupId).StartsWith(
            'destroyable.group.valtan.outerwall109.', [StringComparison]::Ordinal) -and
         -not ([string]$_.groupId).StartsWith(
            'destroyable.group.valtan.entrance.', [StringComparison]::Ordinal) -and
         -not ([string]$_.groupId).StartsWith(
            'destroyable.group.valtan.wall.', [StringComparison]::Ordinal))
    })
    $contactBindingCount = @($events.bindings | Where-Object {
        $_.enabled -and $_.triggerKind -ceq 'COLLIDER_CONTACT'
    }).Count
    $removedOuterContacts = $false
    if (99 -eq $contactBindingCount -or 42 -eq $contactBindingCount) {
        $outerMutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($mutation in @($events.mutations | Where-Object {
            ([string]$_.groupId).StartsWith(
                'destroyable.group.valtan.outerwall109.', [StringComparison]::Ordinal)
        })) {
            [void]$outerMutationIds.Add([string]$mutation.mutationId)
        }
        $events.bindings = @($events.bindings | Where-Object {
            -not ($_.enabled -and $_.triggerKind -ceq 'COLLIDER_CONTACT' -and
                $outerMutationIds.Contains([string]$_.mutationId))
        })
        $contactBindingCount = @($events.bindings | Where-Object {
            $_.enabled -and $_.triggerKind -ceq 'COLLIDER_CONTACT'
        }).Count
        $removedOuterContacts = $true
    }
    if (0 -eq $legacyProfiles.Count) {
        if (69 -ne $contactBindingCount) {
            throw "The fully split wall graph must own exactly 69 ordinary contact bindings, found $contactBindingCount."
        }
        if (0 -eq $repairRegions.Count -and -not $removedOuterContacts) {
            Write-Host 'All 99 Valtan source walls are independent; the 30 outer walls remain 109-pattern-only.'
            return
        }
        $stagingRoot = Join-Path ([IO.Path]::GetTempPath()) ('LostArkValtanWallRepair.' + [Guid]::NewGuid().ToString('N'))
        [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
        try {
            $stagedEvents = Write-StagedJson $eventsPath $events $stagingRoot
            $stagedNav = Write-StagedNav $nav $stagingRoot
            [IO.File]::Copy($stagedEvents, $eventsPath, $true)
            [IO.File]::Copy($stagedNav, $navPath, $true)
        }
        finally {
            [IO.Directory]::Delete($stagingRoot, $true)
        }
        Write-Host "Updated the independent wall graph: navigation repairs=$($repairRegions.Count), outer contact bindings removed=$removedOuterContacts."
        return
    }
    if (12 -ne $contactBindingCount) {
        throw "The partial wall graph must own exactly 12 ordinary contact bindings before legacy migration, found $contactBindingCount."
    }

    $legacyGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $legacyMutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $legacyRegionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $legacyGroups = [Collections.Generic.List[object]]::new()
    $legacyMutations = [Collections.Generic.List[object]]::new()
    $legacyBindings = [Collections.Generic.List[object]]::new()
    $legacyLeafProfiles = [Collections.Generic.List[object]]::new()
    $legacyLeafRegions = [Collections.Generic.List[object]]::new()
    $legacySourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)

    foreach ($profile in $legacyProfiles) {
        $group = @($events.groups | Where-Object groupId -CEQ $profile.groupId)
        $mutation = @($events.mutations | Where-Object groupId -CEQ $profile.groupId)
        if (1 -ne $group.Count -or 1 -ne $mutation.Count -or
            0 -eq @($profile.elements).Count) {
            throw "Legacy profile has no exact group/mutation graph: $($profile.groupId)"
        }
        $sourceAndAliases = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($element in @($profile.elements)) {
            if (-not $sourceAndAliases.Add([string]$element.sourceRuntimePlacementId)) {
                throw "Legacy profile repeats a source emitter: $($profile.groupId)"
            }
            foreach ($aliasId in @($element.suppressionAliasPlacementIds)) {
                if (-not $sourceAndAliases.Add([string]$aliasId)) {
                    throw "Legacy profile repeats a suppression alias: $($profile.groupId)"
                }
            }
        }
        $authoredMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($memberId in @($group[0].memberPlacementIds)) {
            [void]$authoredMembers.Add([string]$memberId)
        }
        if (-not $authoredMembers.SetEquals($sourceAndAliases)) {
            throw "Legacy group members do not equal source plus aliases: $($profile.groupId)"
        }
        [void]$legacyGroupIds.Add([string]$profile.groupId)
        [void]$legacyMutationIds.Add([string]$mutation[0].mutationId)

        $navBySource = @{}
        if (0 -ne @($group[0].navigationRegionIds).Count) {
            if (1 -ne @($group[0].navigationRegionIds).Count) {
                throw "Legacy group must own at most one source nav region: $($profile.groupId)"
            }
            $oldRegion = @($nav.Regions | Where-Object {
                $_.RegionId -ceq [string]$group[0].navigationRegionIds[0]
            })
            if (1 -ne $oldRegion.Count) {
                throw "Legacy group nav region is missing: $($profile.groupId)"
            }
            [void]$legacyRegionIds.Add([string]$oldRegion[0].RegionId)
            $navBySource = Split-RegionByWallFootprint `
                $oldRegion[0] @($profile.elements) $gameplay $nav 'wall'
        }

        $oldBindings = @($events.bindings | Where-Object {
            $_.mutationId -ceq [string]$mutation[0].mutationId
        })
        if (0 -eq $oldBindings.Count -or
            0 -ne @($oldBindings | Where-Object enabled).Count) {
            throw "Legacy grouped schedule must remain dormant while it is split: $($profile.groupId)"
        }
        foreach ($element in @($profile.elements)) {
            $sourceId = [string]$element.sourceRuntimePlacementId
            if (-not $legacySourceIds.Add($sourceId)) {
                throw "A legacy wall source is owned twice: $sourceId"
            }
            $leafGroupId = "destroyable.group.valtan.wall.$sourceId"
            $leafMutationId = "mutation.valtan.wall.$sourceId.despawn"
            $leafMembers = @($sourceId) + @($element.suppressionAliasPlacementIds)
            $leafRegionIds = [Collections.Generic.List[string]]::new()
            if ($navBySource.ContainsKey($sourceId) -and
                $null -ne $navBySource[$sourceId]) {
                $leafRegionIds.Add([string]$navBySource[$sourceId].RegionId)
                $legacyLeafRegions.Add($navBySource[$sourceId])
            }
            $legacyGroups.Add([pscustomobject][ordered]@{
                groupId=$leafGroupId
                memberPlacementIds=@($leafMembers)
                navigationRegionIds=@($leafRegionIds)
                navPolarity='BLOCK_WHILE_INTACT'
                initialState='INTACT'
            })
            $legacyMutations.Add([pscustomobject][ordered]@{
                mutationId=$leafMutationId
                groupId=$leafGroupId
                targetState=[string]$mutation[0].targetState
                breakingDurationMs=[int]$mutation[0].breakingDurationMs
            })
            foreach ($oldBinding in $oldBindings) {
                $legacyBindings.Add([pscustomobject][ordered]@{
                    bindingId="binding.valtan.wall.$sourceId.preview"
                    mutationId=$leafMutationId
                    patternId=[string]$oldBinding.patternId
                    stageId=[string]$oldBinding.stageId
                    triggerKind=[string]$oldBinding.triggerKind
                    offsetMs=[int]$oldBinding.offsetMs
                    receiverCollisionId=[string]$oldBinding.receiverCollisionId
                    enabled=$false
                })
            }
            $sourceCollision = Find-SourceCollision $gameplay $sourceId
            $legacyBindings.Add([pscustomobject][ordered]@{
                bindingId="binding.valtan.wall.$sourceId.contact"
                mutationId=$leafMutationId
                patternId=''
                stageId=''
                triggerKind='COLLIDER_CONTACT'
                offsetMs=0
                receiverCollisionId=[string]$sourceCollision.placementId
                enabled=$true
            })
            $leafProfile = Clone-Object $profile
            $leafProfile.profileId = "$leafGroupId.preview"
            $leafProfile.groupId = $leafGroupId
            $leafProfile.elements = @($element)
            $legacyLeafProfiles.Add($leafProfile)
        }
    }
    if (57 -ne $legacySourceIds.Count) {
        throw "Legacy migration must split exactly 57 source walls, found $($legacySourceIds.Count)."
    }

    $events.groups = @($events.groups | Where-Object {
        -not $legacyGroupIds.Contains([string]$_.groupId)
    }) + @($legacyGroups)
    $events.mutations = @($events.mutations | Where-Object {
        -not $legacyMutationIds.Contains([string]$_.mutationId)
    }) + @($legacyMutations)
    $events.bindings = @($events.bindings | Where-Object {
        -not $legacyMutationIds.Contains([string]$_.mutationId)
    }) + @($legacyBindings)
    $simulation.profiles = @($simulation.profiles | Where-Object {
        -not $legacyGroupIds.Contains([string]$_.groupId)
    }) + @($legacyLeafProfiles)
    $nav.Regions = @($nav.Regions | Where-Object {
        -not $legacyRegionIds.Contains([string]$_.RegionId)
    }) + @($legacyLeafRegions)

    if (99 -ne @($events.groups).Count -or
        99 -ne @($events.mutations).Count -or
        99 -ne @($simulation.profiles).Count -or
        0 -ne @($simulation.profiles | Where-Object { 1 -ne @($_.elements).Count }).Count -or
        69 -ne @($events.bindings | Where-Object {
            $_.enabled -and $_.triggerKind -ceq 'COLLIDER_CONTACT'
        }).Count) {
        throw 'Legacy migration did not produce 99 independent profiles with 69 ordinary contact walls.'
    }

    $stagingRoot = Join-Path ([IO.Path]::GetTempPath()) ('LostArkValtanAllWallSplit.' + [Guid]::NewGuid().ToString('N'))
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    try {
        $stagedEvents = Write-StagedJson $eventsPath $events $stagingRoot
        $stagedSimulation = Write-StagedJson $simulationPath $simulation $stagingRoot
        $stagedNav = Write-StagedNav $nav $stagingRoot
        [IO.File]::Copy($stagedEvents, $eventsPath, $true)
        [IO.File]::Copy($stagedSimulation, $simulationPath, $true)
        [IO.File]::Copy($stagedNav, $navPath, $true)
    }
    finally {
        [IO.Directory]::Delete($stagingRoot, $true)
    }
    Write-Host 'Split all 57 recovered arena walls; all 99 Valtan walls are independent and 69 ordinary walls own contact bindings.'
    return
}

$impactGroup = @($events.groups | Where-Object groupId -CEQ $combinedImpactGroupId)
$impactMutation = @($events.mutations | Where-Object groupId -CEQ $combinedImpactGroupId)
$impactProfile = @($simulation.profiles | Where-Object groupId -CEQ $combinedImpactGroupId)
$outerGroups = @($events.groups | Where-Object {
    ([string]$_.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
})
$outerProfiles = @($simulation.profiles | Where-Object {
    ([string]$_.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
})
if ($impactGroup.Count -ne 1 -or $impactMutation.Count -ne 1 -or
    $impactProfile.Count -ne 1 -or @($impactProfile[0].elements).Count -ne 10 -or
    $outerGroups.Count -ne 8 -or $outerProfiles.Count -ne 8 -or
    @($outerProfiles | ForEach-Object { $_.elements }).Count -ne 30) {
    throw 'Source does not match the expected combined 10-wall/8-sector graph.'
}

$oldRegionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$newRegionsBySource = @{}
$impactOldRegion = @($nav.Regions | Where-Object {
    $_.RegionId -ceq [string]$impactGroup[0].navigationRegionIds[0]
})
if ($impactOldRegion.Count -ne 1) { throw 'Combined 159 navigation region is missing.' }
[void]$oldRegionIds.Add($impactOldRegion[0].RegionId)
$impactNavBySource = Split-RegionByWallFootprint $impactOldRegion[0] @($impactProfile[0].elements) $gameplay $nav 'wall159'
foreach ($entry in $impactNavBySource.GetEnumerator()) { $newRegionsBySource[$entry.Key] = $entry.Value }

foreach ($profile in $outerProfiles) {
    $group = $outerGroups | Where-Object groupId -CEQ $profile.groupId
    if ($null -eq $group -or @($group.navigationRegionIds).Count -ne 1) {
        throw "Outer source group has no exact navigation region: $($profile.groupId)"
    }
    $oldRegion = @($nav.Regions | Where-Object {
        $_.RegionId -ceq [string]$group.navigationRegionIds[0]
    })
    if ($oldRegion.Count -ne 1) { throw "Outer navigation region is missing: $($group.navigationRegionIds[0])" }
    [void]$oldRegionIds.Add($oldRegion[0].RegionId)
    $split = Split-RegionByWallFootprint $oldRegion[0] @($profile.elements) $gameplay $nav 'outerwall109'
    foreach ($entry in $split.GetEnumerator()) { $newRegionsBySource[$entry.Key] = $entry.Value }
}

$newGroups = [Collections.Generic.List[object]]::new()
$newMutations = [Collections.Generic.List[object]]::new()
$newBindings = [Collections.Generic.List[object]]::new()
$newProfiles = [Collections.Generic.List[object]]::new()

foreach ($group in @($events.groups)) {
    $isImpact = [string]$group.groupId -ceq $combinedImpactGroupId
    $isOuter = ([string]$group.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
    if (-not $isImpact -and -not $isOuter) { $newGroups.Add($group); continue }
    $profile = if ($isImpact) { $impactProfile[0] } else {
        $outerProfiles | Where-Object groupId -CEQ $group.groupId
    }
    $kind = if ($isImpact) { 'wall159' } else { 'outerwall109' }
    foreach ($element in @($profile.elements)) {
        $sourceId = [string]$element.sourceRuntimePlacementId
        $groupId = "destroyable.group.valtan.$kind.$sourceId"
        $memberIds = @($sourceId) + @($element.suppressionAliasPlacementIds)
        $region = $newRegionsBySource[$sourceId]
        $leafNavigationRegionIds = [Collections.Generic.List[string]]::new()
        if ($null -ne $region) {
            $leafNavigationRegionIds.Add([string]$region.RegionId)
        }
        $leafGroup = [pscustomobject][ordered]@{
            groupId=$groupId
            memberPlacementIds=@($memberIds)
            navigationRegionIds=$leafNavigationRegionIds
            navPolarity='BLOCK_WHILE_INTACT'
            initialState='INTACT'
        }
        $newGroups.Add($leafGroup)
    }
}

foreach ($mutation in @($events.mutations)) {
    $isImpact = [string]$mutation.groupId -ceq $combinedImpactGroupId
    $isOuter = ([string]$mutation.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
    if (-not $isImpact -and -not $isOuter) { $newMutations.Add($mutation); continue }
    $profile = if ($isImpact) { $impactProfile[0] } else {
        $outerProfiles | Where-Object groupId -CEQ $mutation.groupId
    }
    $kind = if ($isImpact) { 'wall159' } else { 'outerwall109' }
    foreach ($element in @($profile.elements)) {
        $sourceId = [string]$element.sourceRuntimePlacementId
        $newMutations.Add([pscustomobject][ordered]@{
            mutationId="mutation.valtan.$kind.$sourceId.despawn"
            groupId="destroyable.group.valtan.$kind.$sourceId"
            targetState=[string]$mutation.targetState
            breakingDurationMs=[int]$mutation.breakingDurationMs
        })
    }
}

$replacedMutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($mutation in $impactMutation) { [void]$replacedMutationIds.Add([string]$mutation.mutationId) }
foreach ($mutation in @($events.mutations | Where-Object {
    ([string]$_.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
})) { [void]$replacedMutationIds.Add([string]$mutation.mutationId) }
foreach ($binding in @($events.bindings)) {
    if (-not $replacedMutationIds.Contains([string]$binding.mutationId)) {
        $newBindings.Add($binding)
        continue
    }
    $oldMutation = $events.mutations | Where-Object mutationId -CEQ $binding.mutationId
    $isImpact = [string]$oldMutation.groupId -ceq $combinedImpactGroupId
    $profile = if ($isImpact) { $impactProfile[0] } else {
        $outerProfiles | Where-Object groupId -CEQ $oldMutation.groupId
    }
    $kind = if ($isImpact) { 'wall159' } else { 'outerwall109' }
    foreach ($element in @($profile.elements)) {
        $sourceId = [string]$element.sourceRuntimePlacementId
        $mutationId = "mutation.valtan.$kind.$sourceId.despawn"
        if ($isImpact -and $binding.enabled) {
            $sourceCollision = Find-SourceCollision $gameplay $sourceId
            $newBindings.Add([pscustomobject][ordered]@{
                bindingId="binding.valtan.wall159.$sourceId.impact"
                mutationId=$mutationId
                patternId=[string]$binding.patternId
                stageId=[string]$binding.stageId
                triggerKind=[string]$binding.triggerKind
                offsetMs=[int]$binding.offsetMs
                receiverCollisionId=([string]$sourceCollision.placementId + '.receiver')
                enabled=$true
            })
        } elseif ($isImpact) {
            $newBindings.Add([pscustomobject][ordered]@{
                bindingId="binding.valtan.wall159.$sourceId.arena-break-109"
                mutationId=$mutationId
                patternId=[string]$binding.patternId
                stageId=[string]$binding.stageId
                triggerKind=[string]$binding.triggerKind
                offsetMs=[int]$binding.offsetMs
                receiverCollisionId=''
                enabled=$false
            })
        } else {
            $newBindings.Add([pscustomobject][ordered]@{
                bindingId="binding.valtan.outerwall109.$sourceId.impact"
                mutationId=$mutationId
                patternId=[string]$binding.patternId
                stageId=[string]$binding.stageId
                triggerKind=[string]$binding.triggerKind
                offsetMs=[int]$binding.offsetMs
                receiverCollisionId=''
                enabled=[bool]$binding.enabled
            })
        }
    }
}

# Ordinary walls own one pattern-independent body contact binding. The thirty
# additional 109 outer walls intentionally do not: only the authored 109 stage
# can remove that ring, while ordinary Valtan contact breaks one ordinary wall.
foreach ($group in @($newGroups | Where-Object {
    ([string]$_.groupId).StartsWith('destroyable.group.valtan.wall159.', [StringComparison]::Ordinal) -or
    ([string]$_.groupId).StartsWith('destroyable.group.valtan.entrance.', [StringComparison]::Ordinal)
})) {
    $mutation = @($newMutations | Where-Object groupId -CEQ $group.groupId)
    if ($mutation.Count -ne 1) {
        throw "Independent wall has no exact mutation: $($group.groupId)"
    }
    if (@($newBindings | Where-Object {
        $_.mutationId -ceq $mutation[0].mutationId -and
        $_.triggerKind -ceq 'COLLIDER_CONTACT'
    }).Count -ne 0) {
        continue
    }
    $sourceId = [string]$group.memberPlacementIds[0]
    $sourceCollision = Find-SourceCollision $gameplay $sourceId
    $bindingStem = ([string]$group.groupId).Substring('destroyable.group.'.Length)
    $newBindings.Add([pscustomobject][ordered]@{
        bindingId="binding.$bindingStem.contact"
        mutationId=[string]$mutation[0].mutationId
        patternId=''
        stageId=''
        triggerKind='COLLIDER_CONTACT'
        offsetMs=0
        receiverCollisionId=[string]$sourceCollision.placementId
        enabled=$true
    })
}

foreach ($profile in @($simulation.profiles)) {
    $isImpact = [string]$profile.groupId -ceq $combinedImpactGroupId
    $isOuter = ([string]$profile.groupId).StartsWith($outerPrefix, [StringComparison]::Ordinal)
    if (-not $isImpact -and -not $isOuter) { $newProfiles.Add($profile); continue }
    $kind = if ($isImpact) { 'wall159' } else { 'outerwall109' }
    foreach ($element in @($profile.elements)) {
        $sourceId = [string]$element.sourceRuntimePlacementId
        $leaf = Clone-Object $profile
        $leaf.groupId = "destroyable.group.valtan.$kind.$sourceId"
        $leaf.profileId = "$($leaf.groupId).preview"
        $leaf.elements = @($element)
        $newProfiles.Add($leaf)
    }
}

$newGameplayPlacements = [Collections.Generic.List[object]]::new()
foreach ($placement in @($gameplay.placements)) {
    if ([string]$placement.placementId -ceq $oldImpactReceiverId) { continue }
    $newGameplayPlacements.Add($placement)
    if ($placement.kind -ceq 'collisionBox' -and
        @($impactProfile[0].elements | Where-Object {
            ([string]$placement.placementId).EndsWith(
                ".$(($_.sourceRuntimePlacementId))", [StringComparison]::Ordinal)
        }).Count -eq 1) {
        $receiver = Clone-Object $placement
        $receiver.placementId = [string]$placement.placementId + '.receiver'
        $newGameplayPlacements.Add($receiver)
    }
}

$newNavRegions = [Collections.Generic.List[object]]::new()
foreach ($region in @($nav.Regions)) {
    if (-not $oldRegionIds.Contains([string]$region.RegionId)) {
        $newNavRegions.Add($region)
    }
}
foreach ($region in @($newRegionsBySource.Values | Where-Object { $null -ne $_ } |
    Sort-Object -Property RegionId -CaseSensitive)) {
    $newNavRegions.Add($region)
}

$events.groups = @($newGroups)
$events.mutations = @($newMutations)
$events.bindings = @($newBindings)
$simulation.profiles = @($newProfiles)
$gameplay.placements = @($newGameplayPlacements)
$nav.Regions = @($newNavRegions)

if (@($events.groups | Where-Object {
        ([string]$_.groupId).StartsWith('destroyable.group.valtan.wall159.', [StringComparison]::Ordinal)
    }).Count -ne 10 -or
    @($events.groups | Where-Object {
        ([string]$_.groupId).StartsWith('destroyable.group.valtan.outerwall109.109', [StringComparison]::Ordinal)
    }).Count -ne 30 -or
    @($events.bindings | Where-Object enabled).Count -ne 54 -or
    @($events.bindings | Where-Object {
        $_.enabled -and $_.triggerKind -ceq 'COLLIDER_CONTACT'
    }).Count -ne 12 -or
    @($simulation.profiles | Where-Object { @($_.elements).Count -ne 1 -and (
        ([string]$_.groupId).StartsWith('destroyable.group.valtan.wall159.', [StringComparison]::Ordinal) -or
        ([string]$_.groupId).StartsWith('destroyable.group.valtan.outerwall109.109', [StringComparison]::Ordinal))
    }).Count -ne 0 -or
    @($gameplay.placements | Where-Object {
        ([string]$_.placementId).StartsWith('collision.valtan.wallgroup.11047903315509031966.', [StringComparison]::Ordinal) -and
        ([string]$_.placementId).EndsWith('.receiver', [StringComparison]::Ordinal)
    }).Count -ne 10) {
    throw 'Independent wall migration did not produce 42 leaf groups with the 109 outer ring protected from ordinary contact.'
}

$stagingRoot = Join-Path ([IO.Path]::GetTempPath()) ('LostArkValtanWallSplit.' + [Guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
try {
    $stagedEvents = Write-StagedJson $eventsPath $events $stagingRoot
    $stagedSimulation = Write-StagedJson $simulationPath $simulation $stagingRoot
    $stagedGameplay = Write-StagedJson $gameplayPath $gameplay $stagingRoot
    $stagedNav = Write-StagedNav $nav $stagingRoot
    [IO.File]::Copy($stagedEvents, $eventsPath, $true)
    [IO.File]::Copy($stagedSimulation, $simulationPath, $true)
    [IO.File]::Copy($stagedGameplay, $gameplayPath, $true)
    [IO.File]::Copy($stagedNav, $navPath, $true)
}
finally {
    [IO.Directory]::Delete($stagingRoot, $true)
}

Write-Host 'Split 10 wall-charge walls and 30 outer-ring walls into independent destruction units.'
