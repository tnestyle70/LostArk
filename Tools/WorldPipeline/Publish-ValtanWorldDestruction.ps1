[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'ContractTest')]
    [string]$Mode = 'Validate',
    [string]$WorldEventsPath = 'Data/Encounters/Valtan/ValtanWorldEvents.json',
    [string]$EncounterPath = 'Data/Encounters/Valtan/ValtanEncounter.json',
    [string]$SimulationPath = 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json',
    [string]$GameplayWorldPath = 'Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json',
    [string]$NavigationBlockersPath = 'Data/Navigation/LV_LUT_HEARTRB_ED.navblockers',
    [string]$ServerOutputRoot = 'Server/Bin/DataFiles/World',
    [string]$ClientOutputRoot = 'Client/Bin/DataFiles/World',
    [ValidateRange(0, 3)]
    [int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$revisionPattern = '^[0-9a-f]{64}$'
$placementIdPattern = '^[1-9][0-9]{0,19}$'
$expectedAreaId = 'LV_LUT_HEARTRB_ED'
$expectedEncounterId = 'ENCOUNTER_VALTAN'
$firstGroupId = 'destroyable.group.valtan.wall.3705102'
$firstMutationId = 'mutation.valtan.wall.3705102.break'
$firstPatternId = 'VALTAN_ARENA_BREAK_109'
$firstStageId = 'IMPACT'
$firstActionId = 'valtan.mechanic.arena-break-109.impact'
$firstStageIndex = 2
$impactGroupId = 'destroyable.group.valtan.deploy.11047903315509031966'
$impactPatternId = 'VALTAN_ARMOR_BREAK_OPENING'
$impactStageId = 'WALL_CHARGE'
$impactActionId = 'valtan.mechanic.armor-break-opening.charge'
$impactStageIndex = 0
$impactReceiverId = 'collision.valtan.wallgroup.11047903315509031966.receiver'
# 13 authored interior groups plus the eight 109 outer ring sectors, and the
# 77 interior members plus the ring's 24 slabs.
$expectedProductGroupCount = 21
$expectedProductBindingCount = 22
$expectedProductMemberCount = 101
# Every ring slab is its own emitter; only the interior keeps duplicate walls.
$expectedProductEmitterCount = 93
$expectedProductSuppressionAliasCount = 8

function Resolve-RepoPath {
    param([string]$Path)
    if ([IO.Path]::IsPathRooted($Path)) {
        return [IO.Path]::GetFullPath($Path)
    }
    return [IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

function Read-JsonDocument {
    param([string]$Path)
    $resolved = Resolve-RepoPath $Path
    if (-not [IO.File]::Exists($resolved)) {
        throw "Required JSON document is missing: $Path"
    }
    return Get-Content -LiteralPath $resolved -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Read-NavigationBlockerCatalog {
    param([string]$Path)
    $resolved = Resolve-RepoPath $Path
    if (-not [IO.File]::Exists($resolved)) {
        throw "Required navigation blocker document is missing: $Path"
    }
    $lines = @([IO.File]::ReadAllLines($resolved, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) { throw 'Navigation blocker document is empty.' }
    $split = { param([string]$line) @([regex]::Matches($line, '"[^"]*"|\S+') | ForEach-Object { $_.Value.Trim('"') }) }
    $header = @(& $split $lines[0])
    if ($header.Count -ne 9 -or $header[0] -ne 'LOSTARK_NAVGRID_BLOCKERS' -or
        $header[1] -ne '1' -or $header[2] -cne $expectedAreaId -or
        [uint32]$header[8] -gt 256) {
        throw 'Navigation blocker header is invalid.'
    }
    $regions = @{}
    $cursor = 1
    for ($regionIndex = 0; $regionIndex -lt [uint32]$header[8]; ++$regionIndex) {
        if ($cursor -ge $lines.Count) { throw 'Navigation blocker region is truncated.' }
        $tokens = @(& $split $lines[$cursor++])
        if ($tokens.Count -ne 5 -or $tokens[0] -ne 'REGION' -or
            $tokens[1] -cnotmatch $stableIdPattern -or
            $tokens[2] -cnotmatch $stableIdPattern -or
            $tokens[3] -notin @('0','1') -or [uint32]$tokens[4] -eq 0 -or
            $regions.ContainsKey([string]$tokens[1])) {
            throw 'Navigation blocker region is invalid.'
        }
        $cellCount = [uint32]$tokens[4]
        if ($cursor + $cellCount -gt $lines.Count) {
            throw 'Navigation blocker cells are truncated.'
        }
        $cursor += $cellCount
        $regions[[string]$tokens[1]] = [pscustomobject]@{
            RegionId=[string]$tokens[1]
            ConditionId=[string]$tokens[2]
            ActivateWhenTrue=([string]$tokens[3] -eq '1')
            CellCount=$cellCount
        }
    }
    if ($cursor -ne $lines.Count) { throw 'Navigation blocker document has trailing rows.' }
    return [pscustomobject]@{ Regions=$regions }
}

function Assert-ExactProperties {
    param([object]$Value, [string[]]$Expected, [string]$Context)
    if ($null -eq $Value) { throw "$Context is null." }
    $actual = @($Value.PSObject.Properties.Name | Sort-Object -CaseSensitive)
    $expectedSorted = @($Expected | Sort-Object -CaseSensitive)
    if (($actual -join "`n") -cne ($expectedSorted -join "`n")) {
        throw "$Context has missing or unknown fields. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId {
    param([object]$Value, [string]$Context, [switch]$AllowEmpty)
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
    if ($AllowEmpty -and [string]::IsNullOrEmpty([string]$Value)) { return }
    if ([string]$Value -cnotmatch $stableIdPattern) {
        throw "$Context is not a stable ID: '$Value'"
    }
}

function Assert-JsonInteger {
    param([object]$Value, [string]$Context, [long]$Minimum = 0, [long]$Maximum = [long]::MaxValue)
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context is out of range: $number"
    }
}

function Assert-JsonNumber {
    param(
        [object]$Value,
        [string]$Context,
        [double]$Minimum,
        [double]$Maximum,
        [switch]$MinimumExclusive)
    if (($Value -isnot [byte]) -and ($Value -isnot [sbyte]) -and
        ($Value -isnot [int16]) -and ($Value -isnot [uint16]) -and
        ($Value -isnot [int]) -and ($Value -isnot [uint32]) -and
        ($Value -isnot [long]) -and ($Value -isnot [uint64]) -and
        ($Value -isnot [single]) -and ($Value -isnot [double]) -and
        ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number) -or
        ($MinimumExclusive -and $number -le $Minimum) -or
        (-not $MinimumExclusive -and $number -lt $Minimum) -or
        $number -gt $Maximum) {
        throw "$Context is out of range: $number"
    }
    return $number
}

function Assert-CanonicalPlacementId {
    param([object]$Value, [string]$Context)
    Assert-StableId $Value $Context
    [uint64]$parsed = 0
    if ([string]$Value -cnotmatch $placementIdPattern -or
        -not [uint64]::TryParse([string]$Value, [ref]$parsed) -or
        [uint64]0 -eq $parsed) {
        throw "$Context is not a canonical uint64: '$Value'"
    }
    return $parsed
}

function Assert-UniqueId {
    param([Collections.Generic.HashSet[string]]$Set, [string]$Value, [string]$Context)
    if (-not $Set.Add($Value)) { throw "Duplicate ${Context}: $Value" }
}

function Sort-OrdinalByProperty {
    param([object[]]$Values, [string]$PropertyName)
    $result = @($Values)
    [Array]::Sort($result, [Comparison[object]]{
        param($left, $right)
        return [string]::CompareOrdinal(
            [string]$left.$PropertyName, [string]$right.$PropertyName)
    })
    return $result
}

function Read-FiniteVector3 {
    param([object]$Value, [string]$Context, [double]$MaximumAbsolute)
    $components = @($Value)
    if ($components.Count -ne 3) { throw "$Context must contain exactly three numbers." }
    $result = [Collections.Generic.List[double]]::new()
    for ($index = 0; $index -lt 3; $index++) {
        $component = Assert-JsonNumber $components[$index] "$Context[$index]" (-$MaximumAbsolute) $MaximumAbsolute
        $result.Add($component)
    }
    return ,$result.ToArray()
}

function Compile-ValtanDebrisProfiles {
    param(
        [object]$Simulation,
        [Collections.IDictionary]$WorldGroupById,
        [object[]]$IncludedGroups,
        [Collections.IDictionary]$MutationByGroupId,
        [Collections.IDictionary]$BindingByMutationId)

    Assert-ExactProperties $Simulation @('schema','formatVersion','areaId','profiles') 'destruction simulation root'
    if ($Simulation.schema -cne 'lostark.destruction-simulation' -or
        $Simulation.areaId -cne $expectedAreaId) {
        throw 'Destruction simulation identity is invalid.'
    }
    Assert-JsonInteger $Simulation.formatVersion 'destruction simulation formatVersion' 2 2

    $profileByGroupId = @{}
    $profileIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $profileGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($profile in @($Simulation.profiles)) {
        Assert-ExactProperties $profile @(
            'profileId','groupId','durationSeconds','previewGroundEnabled',
            'previewGroundHeight','previewGroundHalfExtents','elements') 'destruction simulation profile'
        Assert-StableId $profile.profileId 'destruction simulation profileId'
        Assert-StableId $profile.groupId "$($profile.profileId) groupId"
        Assert-UniqueId $profileIds ([string]$profile.profileId) 'destruction simulation profileId'
        Assert-UniqueId $profileGroupIds ([string]$profile.groupId) 'destruction simulation groupId'
        if (-not $WorldGroupById.ContainsKey([string]$profile.groupId)) {
            throw "Destruction simulation references an unknown group: $($profile.groupId)"
        }
        $duration = Assert-JsonNumber $profile.durationSeconds "$($profile.profileId) durationSeconds" (1.0 / 60.0) 60.0
        if ($profile.previewGroundEnabled -isnot [bool]) {
            throw "$($profile.profileId) previewGroundEnabled must be boolean."
        }
        $previewHeight = Assert-JsonNumber $profile.previewGroundHeight "$($profile.profileId) previewGroundHeight" -1000.0 1000.0
        $previewExtents = @($profile.previewGroundHalfExtents)
        if ($previewExtents.Count -ne 2) {
            throw "$($profile.profileId) previewGroundHalfExtents must contain exactly two numbers."
        }
        foreach ($extent in $previewExtents) {
            $validatedExtent = Assert-JsonNumber $extent "$($profile.profileId) previewGroundHalfExtents" 0.0 1000.0 -MinimumExclusive
        }

        $elements = @($profile.elements)
        if ($elements.Count -eq 0 -or $elements.Count -gt 256) {
            throw "$($profile.profileId) must contain 1..256 debris elements."
        }
        $elementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $coveredPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $compiledEmitters = [Collections.Generic.List[object]]::new()
        foreach ($element in $elements) {
            Assert-ExactProperties $element @(
                'elementId','sourceRuntimePlacementId','suppressionAliasPlacementIds',
                'spawnOffset','direction','speedMetersPerSecond','gravityScale',
                'lifetimeSeconds','trigger') 'destruction simulation element'
            Assert-StableId $element.elementId 'destruction simulation elementId'
            Assert-UniqueId $elementIds ([string]$element.elementId) 'destruction simulation elementId'
            $sourceId = [string]$element.sourceRuntimePlacementId
            $parsedSourceId = Assert-CanonicalPlacementId $sourceId "$($element.elementId) sourceRuntimePlacementId"
            Assert-UniqueId $coveredPlacementIds $sourceId "$($profile.groupId) covered placementId"

            $aliases = [Collections.Generic.List[string]]::new()
            foreach ($aliasValue in @($element.suppressionAliasPlacementIds)) {
                $aliasId = [string]$aliasValue
                $parsedAliasId = Assert-CanonicalPlacementId $aliasId "$($element.elementId) suppression alias"
                Assert-UniqueId $coveredPlacementIds $aliasId "$($profile.groupId) covered placementId"
                $aliases.Add($aliasId)
            }
            $sortedAliases = @($aliases | Sort-Object { [uint64]$_ })

            $spawnOffset = Read-FiniteVector3 $element.spawnOffset "$($element.elementId) spawnOffset" 1000.0
            $direction = Read-FiniteVector3 $element.direction "$($element.elementId) direction" 1.0
            $directionLength = [Math]::Sqrt(
                $direction[0] * $direction[0] +
                $direction[1] * $direction[1] +
                $direction[2] * $direction[2])
            if ([Math]::Abs($directionLength - 1.0) -gt 0.001) {
                throw "$($element.elementId) direction must be normalized."
            }
            $speed = Assert-JsonNumber $element.speedMetersPerSecond "$($element.elementId) speedMetersPerSecond" 0.0 250.0
            $gravity = Assert-JsonNumber $element.gravityScale "$($element.elementId) gravityScale" 0.0 10.0
            $lifetime = Assert-JsonNumber $element.lifetimeSeconds "$($element.elementId) lifetimeSeconds" 0.0 $duration -MinimumExclusive

            Assert-ExactProperties $element.trigger @('kind','timeSeconds','receiverCollisionId') "$($element.elementId) trigger"
            $triggerTime = Assert-JsonNumber $element.trigger.timeSeconds "$($element.elementId) trigger timeSeconds" 0.0 $duration
            if ($element.trigger.kind -cne 'TIMELINE_TIME' -or
                $element.trigger.receiverCollisionId -isnot [string] -or
                -not [string]::IsNullOrEmpty([string]$element.trigger.receiverCollisionId) -or
                $triggerTime + $lifetime -gt $duration + 0.000001) {
                throw "$($element.elementId) product debris trigger is invalid."
            }

            $compiledEmitters.Add([ordered]@{
                sourceRuntimePlacementId = $sourceId
                suppressionAliasPlacementIds = @($sortedAliases)
                spawnOffset = @($spawnOffset)
                direction = @($direction)
                speedMetersPerSecond = $speed
                gravityScale = $gravity
                lifetimeSeconds = $lifetime
            })
        }

        $group = $WorldGroupById[[string]$profile.groupId]
        $expectedMembers = @($group.memberPlacementIds | Sort-Object { [uint64]$_ })
        $coveredMembers = @($coveredPlacementIds | Sort-Object { [uint64]$_ })
        if (($expectedMembers -join "`n") -cne ($coveredMembers -join "`n")) {
            throw "Destruction simulation coverage does not exactly match group: $($profile.groupId)"
        }

        $sortedEmitters = @($compiledEmitters |
            Sort-Object { [uint64]$_.sourceRuntimePlacementId })
        $profileByGroupId[[string]$profile.groupId] = @($sortedEmitters)
    }

    if ($profileByGroupId.Count -ne $WorldGroupById.Count) {
        throw 'Destruction simulation must cover every canonical world destruction group exactly once.'
    }
    $compiledProfiles = [Collections.Generic.List[object]]::new()
    foreach ($includedGroup in @(Sort-OrdinalByProperty @($IncludedGroups) 'GroupId')) {
        if (-not $profileByGroupId.ContainsKey([string]$includedGroup.GroupId)) {
            throw "Enabled destruction group has no debris presentation profile: $($includedGroup.GroupId)"
        }
        $mutationId = [string]$MutationByGroupId[[string]$includedGroup.GroupId]
        $bindingId = [string]$BindingByMutationId[$mutationId]
        if ([string]::IsNullOrEmpty($mutationId) -or
            [string]::IsNullOrEmpty($bindingId)) {
            throw "Enabled destruction group is missing mutation/binding identity: $($includedGroup.GroupId)"
        }
        $compiledProfiles.Add([ordered]@{
            groupId = [string]$includedGroup.GroupId
            mutationId = $mutationId
            bindingId = $bindingId
            emitters = @($profileByGroupId[[string]$includedGroup.GroupId])
        })
    }
    return ,$compiledProfiles.ToArray()
}

function Get-Sha256LowerHex {
    param([byte[]]$Bytes)
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        return (($sha.ComputeHash($Bytes) | ForEach-Object { $_.ToString('x2') }) -join '')
    }
    finally { $sha.Dispose() }
}

function Convert-ToTicks {
    param([int]$DurationMs, [int]$FixedTickHz)
    return [uint32][Math]::Ceiling(([double]$DurationMs * [double]$FixedTickHz) / 1000.0)
}

function Compile-ValtanWorldDestruction {
    param(
        [object]$WorldEvents,
        [object]$Encounter,
        [object]$Simulation,
        [object]$Gameplay = $null,
        [object]$NavigationBlockers = $null)

    if ($null -eq $Gameplay) { $Gameplay = Read-JsonDocument $GameplayWorldPath }
    if ($null -eq $NavigationBlockers) {
        $NavigationBlockers = Read-NavigationBlockerCatalog $NavigationBlockersPath
    }

    Assert-ExactProperties $WorldEvents @(
        'schema','formatVersion','areaId','encounterId','provenance',
        'groups','mutations','bindings') 'world destruction root'
    if ($WorldEvents.schema -cne 'lostark.world-destruction-events') {
        throw 'World destruction schema is invalid.'
    }
    Assert-JsonInteger $WorldEvents.formatVersion 'world destruction formatVersion' 1 1
    if ($WorldEvents.areaId -cne $expectedAreaId -or
        $WorldEvents.encounterId -cne $expectedEncounterId -or
        $WorldEvents.provenance -cne 'PROJECT_TUNED') {
        throw 'World destruction identity or provenance is invalid.'
    }

    Assert-ExactProperties $Encounter @(
        'schema','formatVersion','encounterId','bossArchetypeId','authority',
        'fixedTickHz','states','patterns') 'encounter root'
    Assert-JsonInteger $Encounter.formatVersion 'encounter formatVersion' 3 3
    Assert-JsonInteger $Encounter.fixedTickHz 'encounter fixedTickHz' 30 30
    if ($Encounter.schema -cne 'lostark.encounter-profile' -or
        $Encounter.encounterId -cne $expectedEncounterId -or
        $Encounter.authority -cne 'server') {
        throw 'Encounter identity or authority is invalid.'
    }

    $patterns = @{}
    $patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($pattern in @($Encounter.patterns)) {
        Assert-StableId $pattern.patternId 'encounter patternId'
        Assert-UniqueId $patternIds ([string]$pattern.patternId) 'encounter patternId'
        $stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $stages = @{}
        $stageIndex = 0
        foreach ($stage in @($pattern.stages)) {
            $expectedStageProperties = @(
                'stageId','actionId','stageKind','durationMs','hitShape',
                'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
                'hitHalfWidth','hitCount','hitIntervalMs','serverDamageProfileId')
            Assert-ExactProperties -Value $stage -Expected $expectedStageProperties -Context "$($pattern.patternId) stage"
            Assert-StableId $stage.stageId "$($pattern.patternId) stageId"
            Assert-StableId $stage.actionId "$($pattern.patternId) actionId"
            Assert-JsonInteger $stage.durationMs "$($pattern.patternId) durationMs" 1 600000
            Assert-UniqueId $stageIds ([string]$stage.stageId) "$($pattern.patternId) stageId"
            $stages[[string]$stage.stageId] = [ordered]@{
                ActionId = [string]$stage.actionId
                StageIndex = [uint32]$stageIndex
            }
            $stageIndex++
        }
        $patterns[[string]$pattern.patternId] = $stages
    }

    if ($Gameplay.schema -cne 'lostark.world-gameplay' -or
        [int]$Gameplay.formatVersion -ne 5 -or
        $Gameplay.areaId -cne $expectedAreaId) {
        throw 'Gameplay world identity is invalid for destruction publishing.'
    }
    $collisionById = @{}
    foreach ($placement in @($Gameplay.placements)) {
        if ($placement.kind -cne 'collisionBox') { continue }
        Assert-StableId $placement.placementId 'collisionBox placementId'
        if ($collisionById.ContainsKey([string]$placement.placementId)) {
            throw "Duplicate collisionBox placementId: $($placement.placementId)"
        }
        $collisionById[[string]$placement.placementId] = $placement
    }

    $groupById = @{}
    $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($WorldEvents.groups)) {
        Assert-ExactProperties $group @(
            'groupId','memberPlacementIds','navigationRegionIds','navPolarity','initialState') 'world destruction group'
        Assert-StableId $group.groupId 'groupId'
        Assert-UniqueId $groupIds ([string]$group.groupId) 'groupId'
        if ($group.initialState -cne 'INTACT' -or
            $group.navPolarity -cnotin @('BLOCK_WHILE_INTACT','WALKABLE_WHILE_INTACT')) {
            throw "Group state or navigation polarity is invalid: $($group.groupId)"
        }
        $members = @($group.memberPlacementIds)
        if ($members.Count -eq 0) { throw "Group has no members: $($group.groupId)" }
        $localMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($memberId in $members) {
            Assert-StableId $memberId "$($group.groupId) memberPlacementId"
            [uint64]$parsedMemberId = 0
            if ([string]$memberId -cnotmatch $placementIdPattern -or
                -not [uint64]::TryParse([string]$memberId, [ref]$parsedMemberId) -or
                [uint64]0 -eq $parsedMemberId) {
                throw "$($group.groupId) memberPlacementId is not a canonical uint64: '$memberId'"
            }
            Assert-UniqueId $localMembers ([string]$memberId) "$($group.groupId) memberPlacementId"
            Assert-UniqueId $ownedMembers ([string]$memberId) 'owned memberPlacementId'
        }
        $navIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($navId in @($group.navigationRegionIds)) {
            Assert-StableId $navId "$($group.groupId) navigationRegionId"
            Assert-UniqueId $navIds ([string]$navId) "$($group.groupId) navigationRegionId"
        }
        $groupById[[string]$group.groupId] = $group
    }

    $sourceCollisionIdsByGroup = @{}
    foreach ($profile in @($Simulation.profiles)) {
        $profileGroupId = [string]$profile.groupId
        if (-not $groupById.ContainsKey($profileGroupId)) { continue }
        $groupKey = ($profileGroupId -split '\.')[-1]
        $sourceCollisionIds = [Collections.Generic.List[string]]::new()
        foreach ($element in @($profile.elements)) {
            $sourceId = [string]$element.sourceRuntimePlacementId
            if ($sourceId -cnotmatch $placementIdPattern) {
                throw "Invalid collision source placement ID: $sourceId"
            }
            $sourceCollisionIds.Add("collision.valtan.wallgroup.$groupKey.$sourceId")
        }
        $sourceCollisionIdsByGroup[$profileGroupId] = @($sourceCollisionIds)
    }

    $mutationById = @{}
    $mutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($mutation in @($WorldEvents.mutations)) {
        Assert-ExactProperties $mutation @(
            'mutationId','groupId','targetState','breakingDurationMs') 'world destruction mutation'
        Assert-StableId $mutation.mutationId 'mutationId'
        Assert-StableId $mutation.groupId "$($mutation.mutationId) groupId"
        Assert-UniqueId $mutationIds ([string]$mutation.mutationId) 'mutationId'
        Assert-JsonInteger $mutation.breakingDurationMs "$($mutation.mutationId) breakingDurationMs" 0 60000
        if ($mutation.targetState -cnotin @('FRACTURED','DESPAWNED') -or
            -not $groupById.ContainsKey([string]$mutation.groupId)) {
            throw "Mutation target or group is invalid: $($mutation.mutationId)"
        }
        $mutationById[[string]$mutation.mutationId] = $mutation
    }

    $bindingIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $enabledBindings = [Collections.Generic.List[object]]::new()
    foreach ($binding in @($WorldEvents.bindings)) {
        Assert-ExactProperties $binding @(
            'bindingId','mutationId','patternId','stageId','triggerKind',
            'offsetMs','receiverCollisionId','enabled') 'world destruction binding'
        Assert-StableId $binding.bindingId 'bindingId'
        Assert-StableId $binding.mutationId "$($binding.bindingId) mutationId"
        Assert-StableId $binding.patternId "$($binding.bindingId) patternId"
        Assert-StableId $binding.stageId "$($binding.bindingId) stageId"
        Assert-StableId $binding.receiverCollisionId "$($binding.bindingId) receiverCollisionId" -AllowEmpty
        Assert-UniqueId $bindingIds ([string]$binding.bindingId) 'bindingId'
        Assert-JsonInteger $binding.offsetMs "$($binding.bindingId) offsetMs" 0 60000
        if ($binding.enabled -isnot [bool]) { throw "$($binding.bindingId) enabled must be boolean." }
        if ($binding.triggerKind -cnotin @('STAGE_TIME','STAGE_ENTER','COLLISION_IMPACT')) {
            throw "Unknown triggerKind: $($binding.triggerKind)"
        }
        if (-not $mutationById.ContainsKey([string]$binding.mutationId)) {
            throw "Binding references an unknown mutation: $($binding.bindingId)"
        }
        if (-not $patterns.ContainsKey([string]$binding.patternId) -or
            -not $patterns[[string]$binding.patternId].ContainsKey([string]$binding.stageId)) {
            throw "Binding references an unknown encounter action tuple: $($binding.bindingId)"
        }
        if ([bool]$binding.enabled) { $enabledBindings.Add($binding) }
    }

    if ($enabledBindings.Count -notin @(0, 1, $expectedProductBindingCount)) {
        throw "Enabled destruction bindings must be dormant, the one-group slice, or the exact $expectedProductBindingCount-binding product set."
    }
    if ($enabledBindings.Count -eq $expectedProductBindingCount -and
        @($WorldEvents.bindings).Count -ne $expectedProductBindingCount) {
        throw "The product set must enable every one of the exact $expectedProductBindingCount canonical bindings."
    }

    $serverGroups = [Collections.Generic.List[object]]::new()
    $serverMutations = [Collections.Generic.List[object]]::new()
    $serverBindings = [Collections.Generic.List[object]]::new()
    $includedGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $includedMutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($binding in @(Sort-OrdinalByProperty @($enabledBindings) 'bindingId')) {
        $mutation = $mutationById[[string]$binding.mutationId]
        $group = $groupById[[string]$mutation.groupId]
        $resolvedStage = $patterns[[string]$binding.patternId][[string]$binding.stageId]
        $isImpactGroup = $group.groupId -ceq $impactGroupId
        $isImpactBinding = $binding.triggerKind -ceq 'COLLISION_IMPACT'
        $hasExpectedSchedule = if ($isImpactBinding) {
            $isImpactGroup -and
            $binding.patternId -ceq $impactPatternId -and
            $binding.stageId -ceq $impactStageId -and
            $resolvedStage.ActionId -ceq $impactActionId -and
            [uint32]$resolvedStage.StageIndex -eq [uint32]$impactStageIndex
        } else {
            $binding.triggerKind -ceq 'STAGE_ENTER' -and
            $binding.patternId -ceq $firstPatternId -and
            $binding.stageId -ceq $firstStageId -and
            $resolvedStage.ActionId -ceq $firstActionId -and
            [uint32]$resolvedStage.StageIndex -eq [uint32]$firstStageIndex
        }
        $groupKey = ([string]$group.groupId -split '\.')[-1]
        $collisionStateId = "collision.valtan.wallgroup.$groupKey"
        $navigationStateId = '-'
        if (-not $sourceCollisionIdsByGroup.ContainsKey([string]$group.groupId) -or
            @($sourceCollisionIdsByGroup[[string]$group.groupId]).Count -eq 0) {
            throw "Destruction group has no authored source collision boxes: $($group.groupId)"
        }
        foreach ($sourceCollisionId in @($sourceCollisionIdsByGroup[[string]$group.groupId])) {
            if (-not $collisionById.ContainsKey([string]$sourceCollisionId) -or
                -not [bool]$collisionById[[string]$sourceCollisionId].enabled) {
                throw "Destruction source collision box is missing or disabled: $sourceCollisionId"
            }
        }
        if ($isImpactGroup) {
            $receiverContractValid = if ($isImpactBinding) {
                [string]$binding.receiverCollisionId -ceq $impactReceiverId
            } else {
                [string]::IsNullOrEmpty([string]$binding.receiverCollisionId)
            }
            if (-not $receiverContractValid -or
                -not $collisionById.ContainsKey($impactReceiverId) -or
                -not [bool]$collisionById[$impactReceiverId].enabled) {
                throw "Impact destruction binding is missing its receiver: $($binding.bindingId)"
            }
        } elseif ($isImpactBinding -or
            -not [string]::IsNullOrEmpty([string]$binding.receiverCollisionId)) {
            throw "Stage destruction binding has unexpected impact state: $($binding.bindingId)"
        }
        # A wall standing on authored floor owns a blocker region whatever
        # trigger breaks it, so pathfinding and collision agree while it is
        # intact. A wall with a cliff behind it authors none, because removing
        # it must not make the drop walkable.
        $navRegionIds = @($group.navigationRegionIds)
        if ($navRegionIds.Count -gt 1) {
            throw "Destruction group declares more than one navigation region: $($group.groupId)"
        }
        if ($isImpactGroup -and $navRegionIds.Count -ne 1) {
            throw "Impact destruction binding is missing its navigation region: $($binding.bindingId)"
        }
        if ($navRegionIds.Count -eq 1) {
            $navRegionId = [string]$navRegionIds[0]
            if (-not $NavigationBlockers.Regions.ContainsKey($navRegionId)) {
                throw "Destruction group references an unknown navigation region: $navRegionId"
            }
            $navRegion = $NavigationBlockers.Regions[$navRegionId]
            if ([uint32]$navRegion.CellCount -eq 0 -or $navRegion.ActivateWhenTrue -or
                $group.navPolarity -cne 'BLOCK_WHILE_INTACT') {
                throw "Destruction navigation polarity is invalid: $navRegionId"
            }
            $navigationStateId = [string]$navRegion.ConditionId
        }
        if (-not $hasExpectedSchedule -or [int]$binding.offsetMs -ne 0) {
            throw "Enabled binding is outside the admitted Valtan destruction schedule: $($binding.bindingId)"
        }

        if ($includedGroupIds.Add([string]$group.groupId)) {
            $serverGroups.Add([ordered]@{
                GroupId = [string]$group.groupId
                InitialState = [string]$group.initialState
                Members = @($group.memberPlacementIds | Sort-Object { [uint64]$_ })
            })
        }
        if ($includedMutationIds.Add([string]$mutation.mutationId)) {
            $serverMutations.Add([ordered]@{
                MutationId = [string]$mutation.mutationId
                GroupId = [string]$mutation.groupId
                FinalState = [string]$mutation.targetState
                BreakingTicks = Convert-ToTicks ([int]$mutation.breakingDurationMs) ([int]$Encounter.fixedTickHz)
                CollisionStateId = $collisionStateId
                NavigationStateId = $navigationStateId
            })
        }
        $serverBindings.Add([ordered]@{
            BindingId = [string]$binding.bindingId
            MutationId = [string]$binding.mutationId
            TriggerKind = if ($binding.triggerKind -ceq 'COLLISION_IMPACT') { 'BOSS_IMPACT' } else { 'STAGE' }
            PatternId = [string]$binding.patternId
            StageId = [string]$binding.stageId
            ActionId = [string]$resolvedStage.ActionId
            StageIndex = [uint32]$resolvedStage.StageIndex
            ReceiverId = if ([string]::IsNullOrEmpty([string]$binding.receiverCollisionId)) { '-' } else { [string]$binding.receiverCollisionId }
        })
    }

    $compiledMemberCount = @($serverGroups | ForEach-Object { $_.Members }).Count
    if ($enabledBindings.Count -eq 1 -and
        ($serverGroups.Count -ne 1 -or $serverGroups[0].GroupId -cne $firstGroupId)) {
        throw 'The one-group destruction slice must be the exact 3705102 group.'
    }
    if ($enabledBindings.Count -eq $expectedProductBindingCount -and
        ($serverGroups.Count -ne $expectedProductGroupCount -or
         $serverMutations.Count -ne $expectedProductGroupCount -or
         $compiledMemberCount -ne $expectedProductMemberCount)) {
        throw "The final-phase destruction set must compile exactly $expectedProductGroupCount groups and $expectedProductMemberCount unique members."
    }
    if ($enabledBindings.Count -eq $expectedProductBindingCount) {
        $arenaBreakBindings = @($serverBindings | Where-Object {
            $_.PatternId -ceq $firstPatternId -and $_.StageId -ceq $firstStageId -and
            $_.ActionId -ceq $firstActionId -and [uint32]$_.StageIndex -eq [uint32]$firstStageIndex -and
            $_.TriggerKind -ceq 'STAGE' -and $_.ReceiverId -ceq '-'
        })
        $impactBindings = @($serverBindings | Where-Object {
            $_.PatternId -ceq $impactPatternId -and $_.StageId -ceq $impactStageId -and
            $_.ActionId -ceq $impactActionId -and [uint32]$_.StageIndex -eq [uint32]$impactStageIndex -and
            $_.TriggerKind -ceq 'BOSS_IMPACT' -and $_.ReceiverId -ceq $impactReceiverId
        })
        if ($arenaBreakBindings.Count -ne $expectedProductGroupCount -or
            $impactBindings.Count -ne 1) {
            throw 'The destruction schedule must contain thirteen 80-bar landing bindings and one opening impact binding.'
        }
    }

    $semanticLines = [Collections.Generic.List[string]]::new()
    $semanticLines.Add("IDENTITY`t$expectedEncounterId`t$expectedAreaId`t$($Encounter.fixedTickHz)")
    foreach ($group in @(Sort-OrdinalByProperty @($serverGroups) 'GroupId')) {
        $semanticLines.Add((@('G',$group.GroupId,$group.InitialState,$group.Members.Count) + @($group.Members)) -join "`t")
    }
    foreach ($mutation in @(Sort-OrdinalByProperty @($serverMutations) 'MutationId')) {
        $semanticLines.Add((@('M',$mutation.MutationId,$mutation.GroupId,$mutation.FinalState,$mutation.BreakingTicks,$mutation.CollisionStateId,$mutation.NavigationStateId)) -join "`t")
    }
    foreach ($binding in @(Sort-OrdinalByProperty @($serverBindings) 'BindingId')) {
        $semanticLines.Add((@('B',$binding.BindingId,$binding.MutationId,$binding.TriggerKind,$binding.PatternId,$binding.StageId,$binding.ActionId,$binding.StageIndex,$binding.ReceiverId)) -join "`t")
    }
    $semanticBytes = [Text.UTF8Encoding]::new($false).GetBytes(($semanticLines -join "`n") + "`n")
    $revision = Get-Sha256LowerHex $semanticBytes
    if ($revision -cnotmatch $revisionPattern) { throw 'Generated revision is invalid.' }

    $serverLines = [Collections.Generic.List[string]]::new()
    $serverLines.Add("LOSTARK_WORLD_DESTRUCTION_BOOTSTRAP`t1`t$expectedEncounterId`t$expectedAreaId`t$revision`t$($Encounter.fixedTickHz)`t$($serverGroups.Count)`t$($serverMutations.Count)`t$($serverBindings.Count)")
    foreach ($line in $semanticLines | Select-Object -Skip 1) { $serverLines.Add($line) }

    $mutationByGroupId = @{}
    foreach ($mutation in $serverMutations) { $mutationByGroupId[$mutation.GroupId] = $mutation.MutationId }
    $bindingByMutationId = @{}
    foreach ($binding in $serverBindings) {
        if (-not $bindingByMutationId.ContainsKey($binding.MutationId) -or
            $binding.TriggerKind -ceq 'STAGE') {
            $bindingByMutationId[$binding.MutationId] = $binding.BindingId
        }
    }
    $debrisProfiles = Compile-ValtanDebrisProfiles $Simulation $groupById `
        @($serverGroups) $mutationByGroupId $bindingByMutationId
    $debrisProfileByGroupId = @{}
    foreach ($profile in $debrisProfiles) {
        if ($debrisProfileByGroupId.ContainsKey([string]$profile.groupId)) {
            throw "Debris presentation profile is duplicated: $($profile.groupId)"
        }
        $debrisProfileByGroupId[[string]$profile.groupId] = $profile
    }
    $projectionGroups = @(Sort-OrdinalByProperty @($serverGroups) 'GroupId' | ForEach-Object {
        $profile = $debrisProfileByGroupId[[string]$_.GroupId]
        if ($null -eq $profile) {
            throw "Projection group is missing its debris presentation profile: $($_.GroupId)"
        }
        $memberIds = @($_.Members | Sort-Object { [uint64]$_ })
        $memberSet = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($memberId in $memberIds) { [void]$memberSet.Add([string]$memberId) }
        $coveredIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $aliasIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($emitter in @($profile.emitters)) {
            $sourceId = [string]$emitter.sourceRuntimePlacementId
            if (-not $memberSet.Contains($sourceId) -or -not $coveredIds.Add($sourceId)) {
                throw "Projection debris source is outside or duplicated in its group: $($_.GroupId)/$sourceId"
            }
            foreach ($aliasValue in @($emitter.suppressionAliasPlacementIds)) {
                $aliasId = [string]$aliasValue
                if (-not $memberSet.Contains($aliasId) -or
                    -not $coveredIds.Add($aliasId) -or -not $aliasIds.Add($aliasId)) {
                    throw "Projection suppression alias is outside or duplicated in its group: $($_.GroupId)/$aliasId"
                }
            }
        }
        $coveredMembers = @($coveredIds | Sort-Object { [uint64]$_ })
        if (($coveredMembers -join "`n") -cne ($memberIds -join "`n")) {
            throw "Projection source and suppression alias union does not exactly cover its group: $($_.GroupId)"
        }
        [ordered]@{
            groupId = $_.GroupId
            mutationId = [string]$mutationByGroupId[$_.GroupId]
            suppressionAliasPlacementIds = @($aliasIds | Sort-Object { [uint64]$_ })
            memberPlacementIds = @($memberIds)
        }
    })
    $projection = [ordered]@{
        schema = 'lostark.world-destruction-client-projection'
        formatVersion = 2
        areaId = $expectedAreaId
        combatRuntimeRevision = $revision
        groups = $projectionGroups
    }
    $clientJson = $projection | ConvertTo-Json -Depth 8
    $presentation = [ordered]@{
        schema = 'lostark.world-destruction-debris-presentation'
        formatVersion = 1
        areaId = $expectedAreaId
        combatRuntimeRevision = $revision
        profiles = @($debrisProfiles)
    }
    $presentationJson = $presentation | ConvertTo-Json -Depth 12
    return [ordered]@{
        Revision = $revision
        ServerLines = $serverLines
        ClientJson = $clientJson + "`n"
        PresentationJson = $presentationJson + "`n"
        GroupCount = $serverGroups.Count
        BindingCount = $serverBindings.Count
        MemberCount = $compiledMemberCount
        EmitterCount = @($debrisProfiles | ForEach-Object { $_.emitters }).Count
        SuppressionAliasCount = @($projectionGroups | ForEach-Object {
            $_.suppressionAliasPlacementIds
        }).Count
    }
}

function Publish-CompiledArtifacts {
    param([object]$Compiled, [string]$ServerRoot, [string]$ClientRoot, [int]$InjectAfter)
    $serverResolved = Resolve-RepoPath $ServerRoot
    $clientResolved = Resolve-RepoPath $ClientRoot
    [IO.Directory]::CreateDirectory($serverResolved) | Out-Null
    [IO.Directory]::CreateDirectory($clientResolved) | Out-Null
    $transactionId = [Guid]::NewGuid().ToString('N')
    $stagingRoot = Join-Path ([IO.Path]::GetTempPath()) "LostArkWorldDestruction.$transactionId"
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    $promotions = [Collections.Generic.List[object]]::new()
    try {
        $serverStaged = Join-Path $stagingRoot 'VALTAN_ARENA.worlddestructionbootstrap'
        $clientStaged = Join-Path $stagingRoot 'LV_LUT_HEARTRB_ED.worlddestruction.json'
        $presentationStaged = Join-Path $stagingRoot 'LV_LUT_HEARTRB_ED.worlddestructionpresentation.json'
        [IO.File]::WriteAllLines($serverStaged, $Compiled.ServerLines, [Text.UTF8Encoding]::new($false))
        [IO.File]::WriteAllText($clientStaged, $Compiled.ClientJson, [Text.UTF8Encoding]::new($false))
        [IO.File]::WriteAllText($presentationStaged, $Compiled.PresentationJson, [Text.UTF8Encoding]::new($false))
        $promotions.Add([pscustomobject]@{
            Staged=$serverStaged; Destination=(Join-Path $serverResolved 'VALTAN_ARENA.worlddestructionbootstrap');
            Rollback=(Join-Path $serverResolved ".VALTAN_ARENA.worlddestruction.rollback.$transactionId"); HadPrevious=$false; Promoted=$false })
        $promotions.Add([pscustomobject]@{
            Staged=$clientStaged; Destination=(Join-Path $clientResolved 'LV_LUT_HEARTRB_ED.worlddestruction.json');
            Rollback=(Join-Path $clientResolved ".LV_LUT_HEARTRB_ED.worlddestruction.rollback.$transactionId"); HadPrevious=$false; Promoted=$false })
        $promotions.Add([pscustomobject]@{
            Staged=$presentationStaged; Destination=(Join-Path $clientResolved 'LV_LUT_HEARTRB_ED.worlddestructionpresentation.json');
            Rollback=(Join-Path $clientResolved ".LV_LUT_HEARTRB_ED.worlddestructionpresentation.rollback.$transactionId"); HadPrevious=$false; Promoted=$false })
        $promotedCount = 0
        foreach ($promotion in $promotions) {
            if ([IO.File]::Exists($promotion.Destination)) {
                [IO.File]::Move($promotion.Destination, $promotion.Rollback)
                $promotion.HadPrevious = $true
            }
            [IO.File]::Move($promotion.Staged, $promotion.Destination)
            $promotion.Promoted = $true
            $promotedCount++
            if ($InjectAfter -eq $promotedCount) { throw "Injected destruction publish failure after promotion $promotedCount." }
        }
        foreach ($promotion in $promotions) {
            if ($promotion.HadPrevious -and [IO.File]::Exists($promotion.Rollback)) {
                [IO.File]::Delete($promotion.Rollback)
            }
        }
    }
    catch {
        for ($promotionIndex = $promotions.Count - 1; $promotionIndex -ge 0; $promotionIndex--) {
            $promotion = $promotions[$promotionIndex]
            if ($promotion.Promoted -and [IO.File]::Exists($promotion.Destination)) {
                [IO.File]::Delete($promotion.Destination)
            }
            if ($promotion.HadPrevious -and [IO.File]::Exists($promotion.Rollback)) {
                [IO.File]::Move($promotion.Rollback, $promotion.Destination)
            }
        }
        throw
    }
    finally {
        if ([IO.Directory]::Exists($stagingRoot)) { [IO.Directory]::Delete($stagingRoot, $true) }
    }
}

function Copy-JsonObject {
    param([object]$Value)
    return ($Value | ConvertTo-Json -Depth 100 | ConvertFrom-Json)
}

function Assert-Throws {
    param([scriptblock]$Action, [string]$Name)
    try { & $Action; throw "Contract test '$Name' did not reject invalid input." }
    catch {
        if ($_.Exception.Message -eq "Contract test '$Name' did not reject invalid input.") { throw }
    }
}

function Invoke-ContractTests {
    $source = Read-JsonDocument $WorldEventsPath
    $encounter = Read-JsonDocument $EncounterPath
    $simulation = Read-JsonDocument $SimulationPath
    $canonical = Compile-ValtanWorldDestruction $source $encounter $simulation
    if ($canonical.GroupCount -ne $expectedProductGroupCount -or
        $canonical.BindingCount -ne $expectedProductBindingCount -or
        $canonical.MemberCount -ne $expectedProductMemberCount -or
        $canonical.EmitterCount -ne $expectedProductEmitterCount -or
        $canonical.SuppressionAliasCount -ne $expectedProductSuppressionAliasCount -or
        $canonical.Revision -cnotmatch $revisionPattern) {
        throw "Canonical source did not compile the exact $expectedProductGroupCount-group/$expectedProductBindingCount-binding/$expectedProductMemberCount-member/$expectedProductSuppressionAliasCount-alias product set."
    }
    $canonicalProjection = $canonical.ClientJson | ConvertFrom-Json
    if ($canonicalProjection.formatVersion -ne 2 -or
        @($canonicalProjection.groups).Count -ne $expectedProductGroupCount -or
        @($canonicalProjection.groups | ForEach-Object {
            $_.suppressionAliasPlacementIds
        }).Count -ne 8) {
        throw 'Canonical projection did not compile the exact formatVersion 2 suppression contract.'
    }
    $dormantSource = Copy-JsonObject $source
    foreach ($binding in @($dormantSource.bindings)) { $binding.enabled = $false }
    $dormant = Compile-ValtanWorldDestruction $dormantSource $encounter $simulation
    if ($dormant.GroupCount -ne 0 -or $dormant.BindingCount -ne 0) {
        throw 'Dormant canonical source unexpectedly compiled active content.'
    }

    $enabled = Copy-JsonObject $source
    foreach ($binding in @($enabled.bindings)) { $binding.enabled = $false }
    $first = $enabled.bindings | Where-Object mutationId -CEQ $firstMutationId | Select-Object -First 1
    $first.patternId = $firstPatternId
    $first.stageId = $firstStageId
    $first.triggerKind = 'STAGE_ENTER'
    $first.offsetMs = 0
    $first.receiverCollisionId = ''
    $first.enabled = $true
    $compiled = Compile-ValtanWorldDestruction $enabled $encounter $simulation
    if ($compiled.GroupCount -ne 1 -or $compiled.BindingCount -ne 1 -or
        $compiled.MemberCount -ne 5 -or $compiled.Revision -cnotmatch $revisionPattern) {
        throw 'Enabled first-slice fixture did not compile exactly one group.'
    }

    $partial = Copy-JsonObject $enabled
    $partialBinding = $partial.bindings | Where-Object mutationId -CNE $firstMutationId | Select-Object -First 1
    $partialBinding.patternId = $firstPatternId
    $partialBinding.stageId = $firstStageId
    $partialBinding.triggerKind = 'STAGE_ENTER'
    $partialBinding.offsetMs = 0
    $partialBinding.receiverCollisionId = ''
    $partialBinding.enabled = $true
    Assert-Throws { Compile-ValtanWorldDestruction $partial $encounter $simulation } 'partial product set'

    $invalidVersion = Copy-JsonObject $enabled
    $invalidVersion.formatVersion = 2
    Assert-Throws { Compile-ValtanWorldDestruction $invalidVersion $encounter $simulation } 'version'

    $unknownField = Copy-JsonObject $enabled
    $unknownField | Add-Member -NotePropertyName unexpected -NotePropertyValue 1
    Assert-Throws { Compile-ValtanWorldDestruction $unknownField $encounter $simulation } 'unknown field'

    $duplicate = Copy-JsonObject $enabled
    $duplicate.groups = @($duplicate.groups) + @(Copy-JsonObject $duplicate.groups[0])
    Assert-Throws { Compile-ValtanWorldDestruction $duplicate $encounter $simulation } 'duplicate group'

    $dangling = Copy-JsonObject $enabled
    $firstDangling = $dangling.bindings | Where-Object enabled -CEQ $true | Select-Object -First 1
    $firstDangling.mutationId = 'mutation.valtan.missing'
    Assert-Throws { Compile-ValtanWorldDestruction $dangling $encounter $simulation } 'dangling mutation'

    $actionMismatchEncounter = Copy-JsonObject $encounter
    $landing = ($actionMismatchEncounter.patterns | Where-Object patternId -CEQ $firstPatternId).stages |
        Where-Object stageId -CEQ $firstStageId | Select-Object -First 1
    $landing.actionId = 'valtan.mechanic.arena-break-109.mismatch'
    Assert-Throws { Compile-ValtanWorldDestruction $enabled $actionMismatchEncounter $simulation } 'action mismatch'

    $unknownSimulationField = Copy-JsonObject $simulation
    $unknownSimulationField.profiles[0].elements[0] |
        Add-Member -NotePropertyName unexpected -NotePropertyValue 1
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $unknownSimulationField
    } 'simulation unknown field'

    $missingSimulationCoverage = Copy-JsonObject $simulation
    $missingSimulationCoverage.profiles[0].elements =
        @($missingSimulationCoverage.profiles[0].elements | Select-Object -Skip 1)
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $missingSimulationCoverage
    } 'simulation missing coverage'

    $invalidSimulationDirection = Copy-JsonObject $simulation
    $invalidSimulationDirection.profiles[0].elements[0].direction = @(0, 0, 0)
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $invalidSimulationDirection
    } 'simulation direction'

    $duplicateSimulationAlias = Copy-JsonObject $simulation
    $profileWithAlias = $duplicateSimulationAlias.profiles |
        Where-Object { @($_.elements | ForEach-Object {
            $_.suppressionAliasPlacementIds
        }).Count -gt 0 } | Select-Object -First 1
    $elementWithAlias = $profileWithAlias.elements |
        Where-Object { @($_.suppressionAliasPlacementIds).Count -gt 0 } |
        Select-Object -First 1
    $elementWithAlias.suppressionAliasPlacementIds = @(
        $elementWithAlias.suppressionAliasPlacementIds[0],
        $elementWithAlias.suppressionAliasPlacementIds[0])
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $duplicateSimulationAlias
    } 'simulation duplicate suppression alias'

    $tempRoot = Join-Path ([IO.Path]::GetTempPath()) ("LostArkWorldDestructionContract." + [Guid]::NewGuid().ToString('N'))
    $serverRoot = Join-Path $tempRoot 'Server'
    $clientRoot = Join-Path $tempRoot 'Client'
    [IO.Directory]::CreateDirectory($serverRoot) | Out-Null
    [IO.Directory]::CreateDirectory($clientRoot) | Out-Null
    $serverFile = Join-Path $serverRoot 'VALTAN_ARENA.worlddestructionbootstrap'
    $clientFile = Join-Path $clientRoot 'LV_LUT_HEARTRB_ED.worlddestruction.json'
    $presentationFile = Join-Path $clientRoot 'LV_LUT_HEARTRB_ED.worlddestructionpresentation.json'
    [IO.File]::WriteAllText($serverFile, 'server-before', [Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText($clientFile, 'client-before', [Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText($presentationFile, 'presentation-before', [Text.UTF8Encoding]::new($false))
    try {
        Assert-Throws { Publish-CompiledArtifacts $canonical $serverRoot $clientRoot 2 } 'atomic rollback injection'
        if ([IO.File]::ReadAllText($serverFile) -cne 'server-before' -or
            [IO.File]::ReadAllText($clientFile) -cne 'client-before' -or
            [IO.File]::ReadAllText($presentationFile) -cne 'presentation-before') {
            throw 'Atomic rollback did not preserve all three previous artifacts.'
        }
    }
    finally { if ([IO.Directory]::Exists($tempRoot)) { [IO.Directory]::Delete($tempRoot, $true) } }

    Write-Host "Valtan world destruction publisher contract tests passed. revision=$($canonical.Revision)"
}

if ($Mode -eq 'ContractTest') {
    Invoke-ContractTests
    return
}

$worldEvents = Read-JsonDocument $WorldEventsPath
$encounter = Read-JsonDocument $EncounterPath
$simulation = Read-JsonDocument $SimulationPath
$result = Compile-ValtanWorldDestruction $worldEvents $encounter $simulation
if ($Mode -eq 'Publish') {
    Publish-CompiledArtifacts $result $ServerOutputRoot $ClientOutputRoot $FailureAfterPromote
}
Write-Host "Valtan world destruction $Mode succeeded. groups=$($result.GroupCount) bindings=$($result.BindingCount) emitters=$($result.EmitterCount) revision=$($result.Revision)"
