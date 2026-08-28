[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'ContractTest')]
    [string]$Mode = 'Validate',
    [string]$WorldEventsPath = 'Data/Encounters/Valtan/ValtanWorldEvents.json',
    [string]$EncounterPath = 'Data/Encounters/Valtan/ValtanEncounter.json',
    [string]$SimulationPath = 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json',
    [string]$GameplayWorldPath = 'Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json',
    [string]$NavigationBlockersPath = 'Data/Navigation/LV_LUT_HEARTRB_ED.navblockers',
    [string]$DeployPlacementsPath = 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements',
    [string]$BaseNavGridPath = 'Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid',
    [string]$DebrisRecipeRoot = 'Data/Maps/Authoring/LV_LUT_HEARTRB_ED',
    [string]$CinematicCameraPath = 'Data/Encounters/Valtan/ValtanCinematicCamera.json',
    [string]$ServerOutputRoot = 'Server/Bin/DataFiles/World',
    [string]$ClientOutputRoot = 'Client/Bin/DataFiles/World',
    [ValidateRange(0, 3)]
    [int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
. (Join-Path $PSScriptRoot 'ValtanWallNavigation.ps1')
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$revisionPattern = '^[0-9a-f]{64}$'
$placementIdPattern = '^[1-9][0-9]{0,19}$'
$expectedAreaId = 'LV_LUT_HEARTRB_ED'
$expectedEncounterId = 'ENCOUNTER_VALTAN'
$firstGroupId = 'destroyable.group.valtan.wall.9335938568718910930'
$firstMutationId = 'mutation.valtan.wall.9335938568718910930.despawn'
$firstPatternId = 'VALTAN_ARENA_BREAK_109'
$firstStageId = 'IMPACT'
$firstActionId = 'valtan.mechanic.arena-break-109.impact'
$firstStageIndex = 2
$impactGroupIdPrefix = 'destroyable.group.valtan.wall159.'
$expectedImpactGroupCount = 10
$impactPatternId = 'VALTAN_ARMOR_BREAK_OPENING'
$impactStageId = 'WALL_CHARGE'
$impactActionId = 'valtan.mechanic.armor-break-opening.charge'
$impactStageIndex = 0
$dashImpactPatternId = 'VALTAN_DASH_CHARGE'
$dashImpactStageId = 'CHARGE'
$dashImpactActionId = 'valtan.attack.dash-charge.active'
$dashImpactStageIndex = 1
$expectedDashImpactGroupCount = 40
# The first-appearance sweep owns two single-wall groups of its own, so it can
# never share a group, a member or a receiver with the 159 charge wall.
$entranceGroupIdPrefix = 'destroyable.group.valtan.entrance.'
$entrancePatternId = 'VALTAN_ENTRANCE_WHIRLWIND'
$entranceStageId = 'SWEEP'
$entranceActionId = 'valtan.mechanic.entrance-whirlwind.sweep'
$entranceStageIndex = 1
$expectedEntranceGroupCount = 2
# The 109 collapse takes the whole arena with it: the outer ring and every
# interior wall still standing when the cutscene fires. The product contract is
# the exact shape of the enabled binding graph rather than a total row count of
# the source document, so both halves are named and counted separately.
$outerGroupIdPrefix = 'destroyable.group.valtan.outerwall109.'
$expectedOuterGroupCount = 30
$expectedOuterMemberCount = 60
$expectedOuterEmitterCount = 30
# Interior walls keep the contact bindings that already break them mid-fight.
# Destruction is one-way, so a wall a body already took down simply answers
# NO_CHANGE when the 109 stage reaches it, and the rest go with the ring.
$interiorGroupIdPrefixes = @(
    'destroyable.group.valtan.wall.',
    'destroyable.group.valtan.wall159.')
$expectedInterior109BindingCount = 67
$expectedOuterFillerAssetId = 'DEPLOY_ITR_02307'
$expectedOuterFillerPlacementIdOffset = [uint64]1000000000000
$expectedOuterRingRadiusMeters = 16.1
$expectedOuterFillerAngleRadians = [Math]::PI / 30.0
$expectedOuterTransformTolerance = 0.001
$expectedIndependentContactWallCount = 69
$expectedProductGroupCount = 99
# The arena floor is the opposite polarity of a wall: it is walkable while it is
# intact and blocks once it collapses. Stage A drops the northern half of the
# outer rail at 84 bars; stage B drops its southern half together with the brick
# ring at 30, leaving the safe core untouched. The two stages are separate
# sub-graphs so one can never reach the other's sectors.
$floorGroupIdPrefix = 'destroyable.group.valtan.floor'
$floorStageAGroupIdPrefix = 'destroyable.group.valtan.floor84.'
$floorStageBGroupIdPrefix = 'destroyable.group.valtan.floor30.'
$floorStageAPatternId = 'VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK'
$floorStageAStageId = 'IMPACT'
$floorStageAActionId = 'valtan.mechanic.terrain-destruction-3.impact'
$floorStageAStageIndex = 3
$floorStageBPatternId = 'VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK'
$floorStageBStageId = 'IMPACT'
$floorStageBActionId = 'valtan.mechanic.terrain-destruction-9.impact'
$floorStageBStageIndex = 3
$expectedFloorStageAGroupCount = 3
$expectedFloorStageBGroupCount = 3
# Each ring source owns one bound visual filler alias. Only the thirty sources
# emit debris, while all sixty placements follow the same thirty group states.
$expectedFragmentsPerEmitter = 12
$expectedOuterSuppressionAliasCount = 1

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
        # Floor collapse regions are validated cell by cell against the baked
        # navgrid, so the exact cells are kept instead of only their count.
        $cellKeys = [Collections.Generic.List[string]]::new()
        for ($cellIndex = 0; $cellIndex -lt $cellCount; ++$cellIndex) {
            $cellTokens = @(& $split $lines[$cursor + $cellIndex])
            if ($cellTokens.Count -ne 2) {
                throw 'Navigation blocker cell row is invalid.'
            }
            $cellKeys.Add("$([uint32]$cellTokens[0]),$([uint32]$cellTokens[1])")
        }
        $cursor += $cellCount
        $regions[[string]$tokens[1]] = [pscustomobject]@{
            RegionId=[string]$tokens[1]
            ConditionId=[string]$tokens[2]
            ActivateWhenTrue=([string]$tokens[3] -eq '1')
            CellCount=$cellCount
            CellKeys=$cellKeys
        }
    }
    if ($cursor -ne $lines.Count) { throw 'Navigation blocker document has trailing rows.' }
    return [pscustomobject]@{
        Regions=$regions; Width=[uint32]$header[3]; Height=[uint32]$header[4]
        CellSize=[double]$header[5]; OriginX=[double]$header[6]; OriginZ=[double]$header[7]
    }
}

# A floor collapse region may only paint cells that the baked navgrid already
# marks walkable. A cell that starts blocked would make the dynamic polarity
# meaningless and hide an authoring mistake behind a correct-looking publish.
function Test-BaseWalkableCell {
    param([string]$CellKey)
    if ($null -eq $script:BaseNavGrid) {
        $resolved = Resolve-RepoPath $BaseNavGridPath
        if (-not [IO.File]::Exists($resolved)) {
            throw "Required base navigation grid is missing: $BaseNavGridPath"
        }
        $bytes = [IO.File]::ReadAllBytes($resolved)
        if ($bytes.Length -lt 20) { throw 'Base navigation grid header is truncated.' }
        $script:BaseNavGrid = [pscustomobject]@{
            Width  = [BitConverter]::ToUInt32($bytes, 0)
            Height = [BitConverter]::ToUInt32($bytes, 4)
            Bytes  = $bytes
        }
        $expected = 20 + [long]$script:BaseNavGrid.Width * [long]$script:BaseNavGrid.Height * 5
        if ($bytes.LongLength -ne $expected) {
            throw 'Base navigation grid size does not match its header.'
        }
    }
    $grid = $script:BaseNavGrid
    $parts = $CellKey -split ','
    if ($parts.Count -ne 2) { throw "Navigation cell key is invalid: $CellKey" }
    $x = [int]$parts[0]
    $z = [int]$parts[1]
    if ($x -lt 0 -or $z -lt 0 -or $x -ge $grid.Width -or $z -ge $grid.Height) { return $false }
    return $grid.Bytes[20 + $z * $grid.Width + $x] -eq 1
}

# A wall placement only fractures into pieces if its deploy asset owns a debris
# recipe, so the publisher joins placement -> asset -> recipe here instead of
# letting the Client discover a missing recipe at the moment of the collapse.
function Get-DeployPlacementAssetIds {
    if ($null -ne $script:DeployPlacementAssetIds) {
        return $script:DeployPlacementAssetIds
    }
    $resolved = Resolve-RepoPath $DeployPlacementsPath
    if (-not [IO.File]::Exists($resolved)) {
        throw "Required deploy placement document is missing: $DeployPlacementsPath"
    }
    $lines = @([IO.File]::ReadAllLines($resolved, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) { throw 'Deploy placement document is empty.' }
    $split = { param([string]$line) @([regex]::Matches($line, '"[^"]*"|\S+') | ForEach-Object { $_.Value.Trim('"') }) }
    $header = @(& $split $lines[0])
    if ($header.Count -ne 4 -or $header[0] -ne 'LOSTARK_DEPLOY_PROP_PLACEMENTS' -or
        $header[1] -ne '1' -or $header[2] -cne $expectedAreaId) {
        throw 'Deploy placement header is invalid.'
    }
    $rowCount = [uint32]$header[3]
    if ($lines.Count -ne $rowCount + 1) {
        throw "Deploy placement row count does not match its header: $DeployPlacementsPath"
    }
    $assets = @{}
    for ($rowIndex = 1; $rowIndex -le $rowCount; ++$rowIndex) {
        $tokens = @(& $split $lines[$rowIndex])
        if ($tokens.Count -lt 5) { throw 'Deploy placement row is invalid.' }
        $placementId = [string]$tokens[0]
        if ($assets.ContainsKey($placementId)) {
            throw "Duplicate deploy placementId: $placementId"
        }
        Assert-StableId $tokens[4] "$placementId deploy assetId"
        $assets[$placementId] = [string]$tokens[4]
    }
    $script:DeployPlacementAssetIds = $assets
    return $assets
}

# Each ring slab flies straight away from the one point the boss lands on, so the
# collapse reads as a single radial shock instead of a directional sweep. The
# anchor is the pattern's compiled serverMotion landing, never a copy of it.
function Get-ArenaBreakLandingAnchor {
    param([object]$Encounter)
    $anchored = @($Encounter.patterns | Where-Object {
        $_.patternId -ceq $firstPatternId -and
        $null -ne $_.PSObject.Properties['serverMotion']
    })
    if ($anchored.Count -ne 1) {
        throw "Exactly one $firstPatternId pattern must declare a serverMotion landing anchor."
    }
    $motion = $anchored[0].serverMotion
    if ([string]$motion.kind -cne 'LEAP_TO_ANCHOR' -or
        @($motion.landingPosition).Count -ne 3) {
        throw 'Arena break serverMotion is not a well-formed LEAP_TO_ANCHOR.'
    }
    Assert-StableId $motion.anchorId 'serverMotion anchorId'
    return [pscustomobject]@{
        AnchorId = [string]$motion.anchorId
        X = Assert-JsonNumber $motion.landingPosition[0] 'anchor x' -100000.0 100000.0
        Y = Assert-JsonNumber $motion.landingPosition[1] 'anchor y' -100000.0 100000.0
        Z = Assert-JsonNumber $motion.landingPosition[2] 'anchor z' -100000.0 100000.0
    }
}

function Get-DeployPlacementPositions {
    if ($null -ne $script:DeployPlacementPositions) {
        return $script:DeployPlacementPositions
    }
    $resolved = Resolve-RepoPath $DeployPlacementsPath
    $lines = @([IO.File]::ReadAllLines($resolved, [Text.Encoding]::UTF8))
    $split = { param([string]$line) @([regex]::Matches($line, '"[^"]*"|\S+') | ForEach-Object { $_.Value.Trim('"') }) }
    $positions = @{}
    for ($rowIndex = 1; $rowIndex -lt $lines.Count; ++$rowIndex) {
        $tokens = @(& $split $lines[$rowIndex])
        if ($tokens.Count -lt 8) { throw 'Deploy placement row is too short for a transform.' }
        $positions[[string]$tokens[0]] = [pscustomobject]@{
            X = [double]$tokens[5]
            Y = [double]$tokens[6]
            Z = [double]$tokens[7]
        }
    }
    $script:DeployPlacementPositions = $positions
    return $positions
}

function Get-DebrisRecipePieceCount {
    param([string]$AssetId)
    if ($null -eq $script:DebrisRecipePieceCounts) {
        $script:DebrisRecipePieceCounts = @{}
    }
    if ($script:DebrisRecipePieceCounts.ContainsKey($AssetId)) {
        return [int]$script:DebrisRecipePieceCounts[$AssetId]
    }
    $resolved = Resolve-RepoPath (Join-Path $DebrisRecipeRoot "$AssetId.debrisrecipe.json")
    if (-not [IO.File]::Exists($resolved)) {
        throw "Destruction source asset has no authored debris recipe: $AssetId"
    }
    $recipe = Get-Content -LiteralPath $resolved -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($recipe.schema -cne 'lostark.deploy-wall-debris-recipe' -or
        [string]$recipe.sourceAssetId -cne $AssetId) {
        throw "Debris recipe identity is invalid: $AssetId"
    }
    $pieceCount = @($recipe.pieces).Count
    $script:DebrisRecipePieceCounts[$AssetId] = $pieceCount
    return [int]$pieceCount
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

    # Every canonical group owns exactly one debris simulation profile. A floor
    # sector has no fractured mesh of its own, so its profile drives the shipped
    # Valtan rubble meshes instead, and it is no longer exempt from coverage.
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
        'fixedTickHz','introPatternId','states','patterns') 'encounter root'
    Assert-JsonInteger $Encounter.formatVersion 'encounter formatVersion' 4 4
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
                'hitHalfWidth','hitCount','hitIntervalMs','hitDelayMs','serverDamageProfileId',
                'pushRangeM','pushMs','knockdown','downMs')
			if ($null -ne $stage.PSObject.Properties['branches']) {
				$expectedStageProperties += 'branches'
			}
			if ($null -ne $stage.PSObject.Properties['actions']) {
				$expectedStageProperties += 'actions'
			}
			if ($null -ne $stage.PSObject.Properties['motion']) {
				$expectedStageProperties += 'motion'
			}
			if ($null -ne $stage.PSObject.Properties['hitOffsetsMs']) {
				$expectedStageProperties += 'hitOffsetsMs'
			}
			if ($null -ne $stage.PSObject.Properties['partDamagePolicy']) {
				$expectedStageProperties += 'partDamagePolicy'
				if ([string]$stage.partDamagePolicy -cnotin @(
					'NORMAL','DESTROY_FIRST_ELIGIBLE')) {
					throw "Encounter partDamagePolicy is invalid: $($pattern.patternId)/$($stage.stageId)"
				}
			}
			if ($null -ne $stage.PSObject.Properties['counterProxy']) {
				$expectedStageProperties += 'counterProxy'
				Assert-ExactProperties -Value $stage.counterProxy -Expected @(
					'space','forwardOffsetM','rightOffsetM','radiusM') `
					-Context "$($pattern.patternId) counterProxy"
				if ([string]$stage.counterProxy.space -cne 'BOSS_LOCAL') {
					throw "Encounter counterProxy space is invalid: $($pattern.patternId)/$($stage.stageId)"
				}
				# Numeric validators return their input; keep compiler output to one artifact set.
				$null = Assert-JsonNumber $stage.counterProxy.forwardOffsetM `
					"$($pattern.patternId) counterProxy forwardOffsetM" -20.0 20.0
				$null = Assert-JsonNumber $stage.counterProxy.rightOffsetM `
					"$($pattern.patternId) counterProxy rightOffsetM" -20.0 20.0
				$null = Assert-JsonNumber $stage.counterProxy.radiusM `
					"$($pattern.patternId) counterProxy radiusM" 0.0 20.0 `
					-MinimumExclusive
			}
			$hasPlayerResponse =
				$null -ne $stage.PSObject.Properties['playerResponse']
			$hasAttachmentSlot =
				$null -ne $stage.PSObject.Properties['attachmentSlot']
			if ($hasPlayerResponse -ne $hasAttachmentSlot) {
				throw "Encounter capture fields must be authored together: $($pattern.patternId)/$($stage.stageId)"
			}
			if ($hasPlayerResponse) {
				$expectedStageProperties += @('playerResponse','attachmentSlot')
				if ([string]$stage.playerResponse -cne 'CAPTURE' -or
					[string]$stage.attachmentSlot -cne 'BOSS_LEFT_HAND') {
					throw "Encounter capture fields are invalid: $($pattern.patternId)/$($stage.stageId)"
				}
			}
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
        [int]$Gameplay.formatVersion -ne 6 -or
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
    $wallNavigationGrid = Read-ValtanWallNavigationGrid (Resolve-RepoPath $BaseNavGridPath)
    if ($NavigationBlockers.Width -ne $wallNavigationGrid.Width -or
        $NavigationBlockers.Height -ne $wallNavigationGrid.Height -or
        $NavigationBlockers.CellSize -ne $wallNavigationGrid.CellSize -or
        $NavigationBlockers.OriginX -ne $wallNavigationGrid.OriginX -or
        $NavigationBlockers.OriginZ -ne $wallNavigationGrid.OriginZ) {
        throw 'Destruction navigation blockers do not match the published base grid.'
    }
    $wallNavigationCellsByGroup = @{}

    $groupById = @{}
    $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($WorldEvents.groups)) {
        Assert-ExactProperties $group @(
            'groupId','memberPlacementIds','navigationRegionIds','navPolarity','initialState') 'world destruction group'
        Assert-StableId $group.groupId 'groupId'
        Assert-UniqueId $groupIds ([string]$group.groupId) 'groupId'
        if ($group.initialState -cne 'INTACT' -or
            $group.navPolarity -cnotin @('BLOCK_WHILE_INTACT','BLOCK_WHILE_FRACTURED')) {
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
        $sourceCollisionIds = [Collections.Generic.List[string]]::new()
        foreach ($element in @($profile.elements)) {
            $sourceId = [string]$element.sourceRuntimePlacementId
            if ($sourceId -cnotmatch $placementIdPattern) {
                throw "Invalid collision source placement ID: $sourceId"
            }
            # Group IDs are presentation ownership, not collision identity.
            # Once every wall is its own group the collision box keeps its
            # authored stable ID, so resolve it by the exact placement suffix.
            $sourceSuffix = ".$sourceId"
            $matches = @($collisionById.Keys | Where-Object {
                ([string]$_).EndsWith($sourceSuffix, [StringComparison]::Ordinal)
            })
            if ($matches.Count -gt 1) {
                throw "More than one collision box owns source placement $sourceId."
            }
            $resolvedSourceCollisionId = ''
            if ($matches.Count -eq 1) {
                $resolvedSourceCollisionId = [string]$matches[0]
            }
            $sourceCollisionIds.Add($resolvedSourceCollisionId)
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
        Assert-StableId $binding.receiverCollisionId "$($binding.bindingId) receiverCollisionId" -AllowEmpty
        Assert-UniqueId $bindingIds ([string]$binding.bindingId) 'bindingId'
        Assert-JsonInteger $binding.offsetMs "$($binding.bindingId) offsetMs" 0 60000
        if ($binding.enabled -isnot [bool]) { throw "$($binding.bindingId) enabled must be boolean." }
        if ($binding.triggerKind -cnotin @(
            'STAGE_TIME','STAGE_ENTER','COLLISION_IMPACT','COLLIDER_CONTACT')) {
            throw "Unknown triggerKind: $($binding.triggerKind)"
        }
        # A contact break answers geometry, not a schedule. It names no pattern
        # and no stage on purpose, so the same wall falls to whichever animation
        # actually reaches it.
        $isContactBinding = $binding.triggerKind -ceq 'COLLIDER_CONTACT'
        if ($isContactBinding) {
            if ([string]$binding.patternId -cne '' -or [string]$binding.stageId -cne '') {
                throw "COLLIDER_CONTACT binding must carry no pattern or stage: $($binding.bindingId)"
            }
            if ([string]$binding.receiverCollisionId -ceq '') {
                throw "COLLIDER_CONTACT binding needs a receiverCollisionId: $($binding.bindingId)"
            }
            if (0 -ne [int]$binding.offsetMs) {
                throw "COLLIDER_CONTACT binding cannot delay a contact: $($binding.bindingId)"
            }
        } else {
            Assert-StableId $binding.patternId "$($binding.bindingId) patternId"
            Assert-StableId $binding.stageId "$($binding.bindingId) stageId"
        }
        if (-not $mutationById.ContainsKey([string]$binding.mutationId)) {
            throw "Binding references an unknown mutation: $($binding.bindingId)"
        }
        if (-not $isContactBinding -and (
            -not $patterns.ContainsKey([string]$binding.patternId) -or
            -not $patterns[[string]$binding.patternId].ContainsKey([string]$binding.stageId))) {
            throw "Binding references an unknown encounter action tuple: $($binding.bindingId)"
        }
        if ([bool]$binding.enabled) { $enabledBindings.Add($binding) }
    }

    # Dormant and the single-group authoring slice stay available; anything else
    # has to satisfy the exact 109 product contract asserted after compilation.
    $isProductCandidate = $enabledBindings.Count -gt 1

    $serverGroups = [Collections.Generic.List[object]]::new()
    $serverMutations = [Collections.Generic.List[object]]::new()
    $serverBindings = [Collections.Generic.List[object]]::new()
    $includedGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $includedMutationIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($binding in @(Sort-OrdinalByProperty @($enabledBindings) 'bindingId')) {
        $mutation = $mutationById[[string]$binding.mutationId]
        $group = $groupById[[string]$mutation.groupId]
        $isContactBinding = $binding.triggerKind -ceq 'COLLIDER_CONTACT'
        $resolvedStage = if ($isContactBinding) { $null } else {
            $patterns[[string]$binding.patternId][[string]$binding.stageId]
        }
        $isImpactGroup = ([string]$group.groupId).StartsWith(
            $impactGroupIdPrefix, [StringComparison]::Ordinal)
        $isOuterGroup = ([string]$group.groupId).StartsWith(
            $outerGroupIdPrefix, [StringComparison]::Ordinal)
        $isEntranceGroup = ([string]$group.groupId).StartsWith(
            $entranceGroupIdPrefix, [StringComparison]::Ordinal)
        $isImpactBinding = $binding.triggerKind -ceq 'COLLISION_IMPACT'
        $isFloorGroup = ([string]$group.groupId).StartsWith(
            $floorGroupIdPrefix, [StringComparison]::Ordinal)
        $hasExpectedSchedule = if ($isFloorGroup) {
            # The floor collapses on the authored impact edge of its own health
            # bar pattern and nothing else. A floor never breaks on contact and
            # never carries a receiver, so both are refused here.
            $isStageAGroup = ([string]$group.groupId).StartsWith(
                $floorStageAGroupIdPrefix, [StringComparison]::Ordinal)
            $isStageBGroup = ([string]$group.groupId).StartsWith(
                $floorStageBGroupIdPrefix, [StringComparison]::Ordinal)
            $expectedFloorPatternId = if ($isStageAGroup) { $floorStageAPatternId }
                elseif ($isStageBGroup) { $floorStageBPatternId } else { $null }
            $expectedFloorStageId = if ($isStageAGroup) { $floorStageAStageId }
                elseif ($isStageBGroup) { $floorStageBStageId } else { $null }
            $expectedFloorActionId = if ($isStageAGroup) { $floorStageAActionId }
                elseif ($isStageBGroup) { $floorStageBActionId } else { $null }
            $expectedFloorStageIndex = if ($isStageAGroup) { $floorStageAStageIndex }
                elseif ($isStageBGroup) { $floorStageBStageIndex } else { -1 }
            ($null -ne $expectedFloorPatternId) -and
            ($binding.triggerKind -ceq 'STAGE_ENTER') -and
            ([int]$binding.offsetMs -eq 0) -and
            [string]::IsNullOrEmpty([string]$binding.receiverCollisionId) -and
            ([string]$binding.patternId -ceq $expectedFloorPatternId) -and
            ([string]$binding.stageId -ceq $expectedFloorStageId) -and
            ([string]$resolvedStage.ActionId -ceq $expectedFloorActionId) -and
            ([int]$resolvedStage.StageIndex -eq [int]$expectedFloorStageIndex)
        } elseif ($isContactBinding) {
            # A contact break owns no schedule at all. Its whole contract is that
            # the receiver is one of this group's own authored collision boxes,
            # which is asserted below against the resolved source box list.
            $true
        } elseif ($isImpactBinding -and $isEntranceGroup) {
            # The first-appearance sweep breaks its own two walls through their
            # own receivers, never through the 159 charge wall's receiver.
            $binding.patternId -ceq $entrancePatternId -and
            $binding.stageId -ceq $entranceStageId -and
            $resolvedStage.ActionId -ceq $entranceActionId -and
            [uint32]$resolvedStage.StageIndex -eq [uint32]$entranceStageIndex
        } elseif ($isImpactBinding) {
            (($isImpactGroup -and
              $binding.patternId -ceq $impactPatternId -and
              $binding.stageId -ceq $impactStageId -and
              $resolvedStage.ActionId -ceq $impactActionId -and
              [uint32]$resolvedStage.StageIndex -eq [uint32]$impactStageIndex) -or
             (($isImpactGroup -or $isOuterGroup) -and
              $binding.patternId -ceq $dashImpactPatternId -and
              $binding.stageId -ceq $dashImpactStageId -and
              $resolvedStage.ActionId -ceq $dashImpactActionId -and
              [uint32]$resolvedStage.StageIndex -eq [uint32]$dashImpactStageIndex))
        } else {
            $binding.triggerKind -ceq 'STAGE_ENTER' -and
            $binding.patternId -ceq $firstPatternId -and
            $binding.stageId -ceq $firstStageId -and
            $resolvedStage.ActionId -ceq $firstActionId -and
            [uint32]$resolvedStage.StageIndex -eq [uint32]$firstStageIndex
        }
        $groupKey = ([string]$group.groupId -split '\.')[-1]
        $navigationStateId = '-'
        # A floor sector owns no collision box at all. It never blocks movement
        # while it is intact and it is never an impact receiver, so requiring a
        # wall's collision contract here would reject it for the wrong reason.
        if ($isFloorGroup) {
            $sourceCollisionIds = @()
            $collisionStateId = ''
        } else {
        if (-not $sourceCollisionIdsByGroup.ContainsKey([string]$group.groupId) -or
            @($sourceCollisionIdsByGroup[[string]$group.groupId]).Count -eq 0) {
            throw "Destruction group has no authored source collision boxes: $($group.groupId)"
        }
        foreach ($sourceCollisionId in @($sourceCollisionIdsByGroup[[string]$group.groupId])) {
            if ([string]::IsNullOrEmpty([string]$sourceCollisionId) -or
                -not $collisionById.ContainsKey([string]$sourceCollisionId) -or
                -not [bool]$collisionById[[string]$sourceCollisionId].enabled) {
                throw "Destruction source collision box is missing or disabled: $sourceCollisionId"
            }
        }
        $sourceCollisionIds = @($sourceCollisionIdsByGroup[[string]$group.groupId])
        if ($isEntranceGroup) {
            # Entrance receivers are siblings of the source leaf. Own their
            # exact group prefix so the final mutation clears both together.
            $collisionStateId = "collision.valtan.wallgroup.$groupKey"
            $ownedCollisionIds = @($sourceCollisionIds) + @($collisionStateId + '.receiver')
            $matchedCollisionIds = @($collisionById.Keys | Where-Object {
                ([string]$_).StartsWith($collisionStateId + '.', [StringComparison]::Ordinal)
            })
            if ($matchedCollisionIds.Count -ne $ownedCollisionIds.Count -or
                @($matchedCollisionIds | Where-Object { $_ -cnotin $ownedCollisionIds }).Count -ne 0) {
                throw "Entrance collision prefix does not own exactly its source and receiver: $($group.groupId)"
            }
        } elseif ($sourceCollisionIds.Count -eq 1) {
            # Exact leaf ownership also reaches its `.receiver` child when the
            # wall commits, so the receiver cannot remain after the mesh goes.
            $collisionStateId = [string]$sourceCollisionIds[0]
        } else {
            $collisionParents = @($sourceCollisionIds | ForEach-Object {
                $lastDot = ([string]$_).LastIndexOf('.')
                if ($lastDot -le 0) { throw "Collision source has no stable parent: $_" }
                ([string]$_).Substring(0, $lastDot)
            } | Select-Object -Unique)
            if ($collisionParents.Count -ne 1) {
                throw "Multi-member destruction group does not own one collision parent: $($group.groupId)"
            }
            $collisionStateId = [string]$collisionParents[0]
        }
        }
        if ($isContactBinding) {
            # The receiver has to be one of this group's own authored collision
            # boxes, so a collider touching one wall can never break another.
            if ([string]$binding.receiverCollisionId -cnotin $sourceCollisionIds) {
                throw "Contact destruction binding does not receive on its own wall: $($binding.bindingId)"
            }
        } elseif ($isEntranceGroup) {
            # Each entrance wall owns exactly one receiver named after its own
            # group, so one contact can never resolve to the other wall.
            $expectedReceiverId = "collision.valtan.wallgroup.$groupKey.receiver"
            if (-not $isImpactBinding -or
                [string]$binding.receiverCollisionId -cne $expectedReceiverId -or
                -not $collisionById.ContainsKey($expectedReceiverId) -or
                -not [bool]$collisionById[$expectedReceiverId].enabled) {
                throw "Entrance destruction binding is missing its own receiver: $($binding.bindingId)"
            }
        } elseif ($isImpactGroup -or $isOuterGroup) {
            $expectedReceiverId = $collisionStateId + '.receiver'
            $receiverContractValid = if ($isImpactBinding) {
                [string]$binding.receiverCollisionId -ceq $expectedReceiverId
            } else {
                [string]::IsNullOrEmpty([string]$binding.receiverCollisionId)
            }
            if (-not $receiverContractValid -or
                -not $collisionById.ContainsKey($expectedReceiverId) -or
                -not [bool]$collisionById[$expectedReceiverId].enabled) {
                throw "Impact destruction binding is missing its receiver: $($binding.bindingId)"
            }
        } elseif ($isImpactBinding -or
            -not [string]::IsNullOrEmpty([string]$binding.receiverCollisionId)) {
            throw "Stage destruction binding has unexpected impact state: $($binding.bindingId)"
        }
        if (-not $isFloorGroup -and
            -not [string]::IsNullOrEmpty([string]$binding.receiverCollisionId) -and
            [string]$binding.receiverCollisionId -cne $collisionStateId -and
            -not ([string]$binding.receiverCollisionId).StartsWith(
                $collisionStateId + '.', [StringComparison]::Ordinal)) {
            throw "Destruction mutation does not own its collision receiver: $($binding.bindingId)"
        }
        # A wall standing on authored floor owns a blocker region whatever
        # trigger breaks it, so pathfinding and collision agree while it is
        # intact. A wall with a cliff behind it authors none, because removing
        # it must not make the drop walkable.
        $navRegionIds = @($group.navigationRegionIds)
        if ($navRegionIds.Count -gt 1) {
            throw "Destruction group declares more than one navigation region: $($group.groupId)"
        }
        if ($navRegionIds.Count -eq 1) {
            $navRegionId = [string]$navRegionIds[0]
            if (-not $NavigationBlockers.Regions.ContainsKey($navRegionId)) {
                throw "Destruction group references an unknown navigation region: $navRegionId"
            }
            $navRegion = $NavigationBlockers.Regions[$navRegionId]
            # A wall blocks while its condition is false and opens when it
            # breaks. A floor is the mirror image, so the region has to activate
            # on the true condition or the hole stays walkable.
            $expectsActivateWhenTrue =
                $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED'
            if ([uint32]$navRegion.CellCount -eq 0 -or
                [bool]$navRegion.ActivateWhenTrue -ne [bool]$expectsActivateWhenTrue) {
                throw "Destruction navigation polarity is invalid: $navRegionId"
            }
            $navigationStateId = [string]$navRegion.ConditionId
        }
        if (-not $isFloorGroup -and
            -not $wallNavigationCellsByGroup.ContainsKey([string]$group.groupId)) {
            $expectedCells = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($sourceCollisionId in $sourceCollisionIds) {
                $sourceCells = Get-ValtanWallNavigationCellKeys `
                    $collisionById[[string]$sourceCollisionId] $wallNavigationGrid
                $expectedCells.UnionWith($sourceCells)
            }
            $actualCells = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            if ($navRegionIds.Count -eq 1) {
                foreach ($key in $NavigationBlockers.Regions[[string]$navRegionIds[0]].CellKeys) {
                    if (-not $actualCells.Add([string]$key)) {
                        throw "Wall navigation repeats one cell: $($group.groupId) / $key"
                    }
                }
            }
            if (-not $expectedCells.SetEquals($actualCells)) {
                throw "Wall navigation does not exactly cover its own collision footprint: $($group.groupId). Run Split-ValtanIndependentWallGroups.ps1 -Mode RebuildNavigation."
            }
            $wallNavigationCellsByGroup[[string]$group.groupId] = $expectedCells
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
                # The Server bootstrap spells "no collision channel" as the dash
                # marker, so a floor sector must never compile an empty column.
                CollisionStateId = if ([string]::IsNullOrEmpty($collisionStateId)) {
                    '-' } else { $collisionStateId }
                NavigationStateId = $navigationStateId
                # BLOCK_WHILE_FRACTURED is the authored polarity that says the
                # cells become impassable once the group is gone: the ground
                # itself was taken away rather than an obstacle beside it.
                # That is the only thing a player can fall through.
                RemovesGround = if (
                    $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED') { 1 } else { 0 }
            })
        }
        # A contact binding compiles with the empty marker in every schedule
        # column, which is what tells the Server to match on geometry alone.
        $compiledTriggerKind = switch ([string]$binding.triggerKind) {
            'COLLISION_IMPACT' { 'BOSS_IMPACT' }
            'COLLIDER_CONTACT' { 'COLLIDER_CONTACT' }
            default { 'STAGE' }
        }
        $serverBindings.Add([ordered]@{
            BindingId = [string]$binding.bindingId
            MutationId = [string]$binding.mutationId
            TriggerKind = $compiledTriggerKind
            PatternId = if ($isContactBinding) { '-' } else { [string]$binding.patternId }
            StageId = if ($isContactBinding) { '-' } else { [string]$binding.stageId }
            ActionId = if ($isContactBinding) { '-' } else { [string]$resolvedStage.ActionId }
            StageIndex = if ($isContactBinding) { [uint32]0 } else { [uint32]$resolvedStage.StageIndex }
            ReceiverId = if ([string]::IsNullOrEmpty([string]$binding.receiverCollisionId)) { '-' } else { [string]$binding.receiverCollisionId }
        })
    }

    $compiledMemberCount = @($serverGroups | ForEach-Object { $_.Members }).Count
    if ($enabledBindings.Count -eq 1 -and
        ($serverGroups.Count -ne 1 -or $serverGroups[0].GroupId -cne $firstGroupId)) {
        throw 'The one-group destruction slice must be the exact 3705102 group.'
    }

    # The 109 stage collapse and charge collision are different triggers. The
    # outer groups intentionally belong to both: stage entry collapses the whole
    # ring, while Dash Charge may commit exactly the first swept slab.
    $arenaBreakBindings = @($serverBindings | Where-Object {
        $_.PatternId -ceq $firstPatternId -and $_.StageId -ceq $firstStageId -and
        $_.ActionId -ceq $firstActionId -and [uint32]$_.StageIndex -eq [uint32]$firstStageIndex -and
        $_.TriggerKind -ceq 'STAGE' -and $_.ReceiverId -ceq '-'
    })
    $impactBindings = @($serverBindings | Where-Object {
        $_.PatternId -ceq $impactPatternId -and $_.StageId -ceq $impactStageId -and
        $_.ActionId -ceq $impactActionId -and [uint32]$_.StageIndex -eq [uint32]$impactStageIndex -and
        $_.TriggerKind -ceq 'BOSS_IMPACT' -and $_.ReceiverId -cne '-'
    })
    $dashImpactBindings = @($serverBindings | Where-Object {
        $_.PatternId -ceq $dashImpactPatternId -and
        $_.StageId -ceq $dashImpactStageId -and
        $_.ActionId -ceq $dashImpactActionId -and
        [uint32]$_.StageIndex -eq [uint32]$dashImpactStageIndex -and
        $_.TriggerKind -ceq 'BOSS_IMPACT' -and $_.ReceiverId -cne '-'
    })
    $entranceBindings = @($serverBindings | Where-Object {
        $_.PatternId -ceq $entrancePatternId -and $_.StageId -ceq $entranceStageId -and
        $_.ActionId -ceq $entranceActionId -and
        [uint32]$_.StageIndex -eq [uint32]$entranceStageIndex -and
        $_.TriggerKind -ceq 'BOSS_IMPACT'
    })
    $entranceGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $entranceReceiverIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $entranceMemberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($binding in $entranceBindings) {
        $reachedGroupId = [string]$mutationById[[string]$binding.MutationId].groupId
        if (-not $reachedGroupId.StartsWith($entranceGroupIdPrefix, [StringComparison]::Ordinal)) {
            throw "The entrance sweep reaches a group outside its own pair: $reachedGroupId"
        }
        if (-not $entranceGroupIds.Add($reachedGroupId)) {
            throw "Two entrance bindings reach the same wall: $reachedGroupId"
        }
        if (-not $entranceReceiverIds.Add([string]$binding.ReceiverId)) {
            throw "Two entrance walls share one receiver: $($binding.ReceiverId)"
        }
        foreach ($memberId in @($groupById[$reachedGroupId].memberPlacementIds)) {
            if (-not $entranceMemberIds.Add([string]$memberId)) {
                throw "Entrance member is claimed twice: $memberId"
            }
        }
    }
    # Every ordinary destructible source wall owns exactly one contact binding,
    # receiving on its own collision box. The thirty additional 109 source
    # groups and their bound filler aliases are stage-only and must not enter
    # this set. Each group still owns only its source collision.
    $contactBindings = @($serverBindings | Where-Object {
        $_.TriggerKind -ceq 'COLLIDER_CONTACT'
    })
    $contactGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $contactReceiverIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($binding in $contactBindings) {
        $reachedGroupId = [string]$mutationById[[string]$binding.MutationId].groupId
        if (-not $contactGroupIds.Add($reachedGroupId)) {
            throw "Two contact bindings reach the same wall: $reachedGroupId"
        }
        if (-not $contactReceiverIds.Add([string]$binding.ReceiverId)) {
            throw "Two walls share one contact receiver: $($binding.ReceiverId)"
        }
        $ownedSourceCollisionIds = @($sourceCollisionIdsByGroup[$reachedGroupId])
        if ($ownedSourceCollisionIds.Count -ne 1 -or
            [string]$binding.ReceiverId -cne [string]$ownedSourceCollisionIds[0]) {
            throw "A contact binding must receive on its own wall collision box: $($binding.BindingId)"
        }
    }
    # The two floor stages are their own reachable sub-graphs. Stage A drops the
    # yaw-0 half of the arena disc at 84 bars and stage B drops the yaw-180 half at
    # 30, each half being its own outer rail plus two brick sectors. Neither may
    # reach a wall group, share a sector or repeat a navigation cell. The SL00 inner
    # wedge and centre cap are Map placements, so they are outside this contract and
    # stay standing as the platform the fight ends on.
    $floorBindings = @($serverBindings | Where-Object {
        $_.TriggerKind -ceq 'STAGE' -and $_.ReceiverId -ceq '-' -and
        (($_.PatternId -ceq $floorStageAPatternId -and $_.StageId -ceq $floorStageAStageId) -or
         ($_.PatternId -ceq $floorStageBPatternId -and $_.StageId -ceq $floorStageBStageId))
    })
    $floorStageGroupIds = @{
        $floorStageAGroupIdPrefix = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $floorStageBGroupIdPrefix = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    }
    $floorMemberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $floorCellOwners = @{}
    foreach ($binding in $floorBindings) {
        $reachedGroupId = [string]$mutationById[[string]$binding.MutationId].groupId
        $reachedGroup = $groupById[$reachedGroupId]
        if (-not $reachedGroupId.StartsWith($floorGroupIdPrefix, [StringComparison]::Ordinal)) {
            throw "A floor collapse binding reaches a non-floor group: $reachedGroupId"
        }
        if ($reachedGroup.navPolarity -cne 'BLOCK_WHILE_FRACTURED') {
            throw "A floor collapse group must use the fractured polarity: $reachedGroupId"
        }
        $stagePrefix = if ($reachedGroupId.StartsWith(
            $floorStageAGroupIdPrefix, [StringComparison]::Ordinal)) {
            $floorStageAGroupIdPrefix
        } elseif ($reachedGroupId.StartsWith(
            $floorStageBGroupIdPrefix, [StringComparison]::Ordinal)) {
            $floorStageBGroupIdPrefix
        } else {
            throw "A floor collapse group uses an unknown stage prefix: $reachedGroupId"
        }
        $expectedPatternId = if ($stagePrefix -ceq $floorStageAGroupIdPrefix) {
            $floorStageAPatternId } else { $floorStageBPatternId }
        if ([string]$binding.PatternId -cne $expectedPatternId) {
            throw "A floor sector is bound to the other stage's pattern: $reachedGroupId"
        }
        if (-not $floorStageGroupIds[$stagePrefix].Add($reachedGroupId)) {
            throw "Two floor bindings reach the same sector: $reachedGroupId"
        }
        foreach ($memberId in @($reachedGroup.memberPlacementIds)) {
            if (-not $floorMemberIds.Add([string]$memberId)) {
                throw "A floor sector placement is claimed twice: $memberId"
            }
        }
        $mutation = $mutationById[[string]$binding.MutationId]
        if ($mutation.targetState -cne 'DESPAWNED') {
            throw "A floor collapse mutation must end at DESPAWNED: $($mutation.mutationId)"
        }
        foreach ($navRegionId in @($reachedGroup.navigationRegionIds)) {
            $region = $NavigationBlockers.Regions[$navRegionId]
            foreach ($cellKey in @($region.CellKeys)) {
                if ($floorCellOwners.ContainsKey($cellKey)) {
                    throw "Floor sectors share navigation cell ${cellKey}: $reachedGroupId and $($floorCellOwners[$cellKey])"
                }
                $floorCellOwners[$cellKey] = $reachedGroupId
                if (-not (Test-BaseWalkableCell $cellKey)) {
                    throw "A floor collapse region paints a base non-walkable cell: $reachedGroupId $cellKey"
                }
            }
        }
    }
    foreach ($contactBinding in $contactBindings) {
        $contactGroupId = [string]$mutationById[[string]$contactBinding.MutationId].groupId
        if ($contactGroupId.StartsWith($floorGroupIdPrefix, [StringComparison]::Ordinal)) {
            throw "A collider contact binding must never target a floor sector: $contactGroupId"
        }
    }

    $outerFragmentActorCount = 0
    $outerGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $outerMemberIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $interior109GroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $interior109BindingCount = 0
    foreach ($binding in $arenaBreakBindings) {
        $reachedGroupId = [string]$mutationById[[string]$binding.MutationId].groupId
        if (-not $reachedGroupId.StartsWith($outerGroupIdPrefix, [StringComparison]::Ordinal)) {
            # Naming the interior families here is what keeps a floor sector or
            # an entrance wall from riding the collapse just by being non-outer.
            $isInteriorGroupId = $false
            foreach ($interiorGroupIdPrefix in $interiorGroupIdPrefixes) {
                if ($reachedGroupId.StartsWith(
                    $interiorGroupIdPrefix, [StringComparison]::Ordinal)) {
                    $isInteriorGroupId = $true
                    break
                }
            }
            if (-not $isInteriorGroupId) {
                throw "The 109 collapse reaches a group that is neither the outer ring nor an interior wall: $reachedGroupId"
            }
            if (-not $interior109GroupIds.Add($reachedGroupId)) {
                throw "Two enabled 109 bindings reach the same interior wall: $reachedGroupId"
            }
            $interior109BindingCount++
            continue
        }
        if (-not $outerGroupIds.Add($reachedGroupId)) {
            throw "Two enabled 109 bindings reach the same outer ring group: $reachedGroupId"
        }
        foreach ($memberId in @($groupById[$reachedGroupId].memberPlacementIds)) {
            if (-not $outerMemberIds.Add([string]$memberId)) {
                throw "Outer ring member is claimed twice: $memberId"
            }
        }
    }

    if ($isProductCandidate) {
        if ($arenaBreakBindings.Count + $impactBindings.Count +
            $dashImpactBindings.Count +
            $entranceBindings.Count + $contactBindings.Count +
            $floorBindings.Count -ne $enabledBindings.Count) {
            throw 'Every enabled destruction binding must belong to the contact, entrance, 109 stage, one of the exact 159/opening or 159+outer/dash impacts, or floor collapse contract.'
        }
        if ($floorBindings.Count -gt 0) {
            if ($floorStageGroupIds[$floorStageAGroupIdPrefix].Count -ne $expectedFloorStageAGroupCount) {
                throw "Floor stage A must reach exactly $expectedFloorStageAGroupCount yaw-0 half sectors, not $($floorStageGroupIds[$floorStageAGroupIdPrefix].Count)."
            }
            if ($floorStageGroupIds[$floorStageBGroupIdPrefix].Count -ne $expectedFloorStageBGroupCount) {
                throw "Floor stage B must reach exactly $expectedFloorStageBGroupCount yaw-180 half sectors, not $($floorStageGroupIds[$floorStageBGroupIdPrefix].Count)."
            }
            foreach ($floorMemberId in $floorMemberIds) {
                if ($outerMemberIds.Contains([string]$floorMemberId)) {
                    throw "A floor sector is also a 109 ring slab: $floorMemberId"
                }
            }
        }
        if ($entranceBindings.Count -ne $expectedEntranceGroupCount -or
            $entranceGroupIds.Count -ne $expectedEntranceGroupCount -or
            $entranceMemberIds.Count -ne $expectedEntranceGroupCount) {
            throw "The first-appearance sweep must break exactly $expectedEntranceGroupCount separate front walls."
        }
        foreach ($entranceMemberId in $entranceMemberIds) {
            if ($outerMemberIds.Contains($entranceMemberId)) {
                throw "An entrance wall is also a 109 ring slab: $entranceMemberId"
            }
        }
        if ($interior109BindingCount -ne $expectedInterior109BindingCount -or
            $interior109GroupIds.Count -ne $expectedInterior109BindingCount) {
            throw "The 109 collapse must also reach exactly $expectedInterior109BindingCount interior wall groups, not $($interior109GroupIds.Count)."
        }
        if ($outerGroupIds.Count -ne $expectedOuterGroupCount) {
            throw "The 109 collapse must reach exactly $expectedOuterGroupCount outer ring groups, not $($outerGroupIds.Count)."
        }
        if ($outerMemberIds.Count -ne $expectedOuterMemberCount) {
            throw "The 109 outer ring must own exactly $expectedOuterMemberCount wall placements, not $($outerMemberIds.Count)."
        }
        $impactGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $impactReceiverIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($impactBinding in $impactBindings) {
            $impactReachedGroupId = [string]$mutationById[[string]$impactBinding.MutationId].groupId
            if (-not $impactReachedGroupId.StartsWith(
                $impactGroupIdPrefix, [StringComparison]::Ordinal) -or
                -not $impactGroupIds.Add($impactReachedGroupId) -or
                -not $impactReceiverIds.Add([string]$impactBinding.ReceiverId)) {
                throw "The 159 impact walls must own unique groups and receivers: $($impactBinding.BindingId)"
            }
        }
        if ($impactBindings.Count -ne $expectedImpactGroupCount -or
            $impactGroupIds.Count -ne $expectedImpactGroupCount -or
            $impactReceiverIds.Count -ne $expectedImpactGroupCount) {
            throw "The opening destruction schedule must contain exactly $expectedImpactGroupCount independent 159 wall-charge bindings."
        }
        $dashImpactGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $dashImpactReceiverIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $dashImpact159GroupCount = 0
        $dashImpactOuterGroupCount = 0
        foreach ($dashImpactBinding in $dashImpactBindings) {
            $dashImpactReachedGroupId = [string]$mutationById[[string]$dashImpactBinding.MutationId].groupId
            $isDashImpact159Group = $dashImpactReachedGroupId.StartsWith(
                $impactGroupIdPrefix, [StringComparison]::Ordinal)
            $isDashImpactOuterGroup = $dashImpactReachedGroupId.StartsWith(
                $outerGroupIdPrefix, [StringComparison]::Ordinal)
            if ((-not $isDashImpact159Group -and
                 -not $isDashImpactOuterGroup) -or
                -not $dashImpactGroupIds.Add($dashImpactReachedGroupId) -or
                -not $dashImpactReceiverIds.Add([string]$dashImpactBinding.ReceiverId)) {
                throw "The general dash impact walls must own unique groups and receivers: $($dashImpactBinding.BindingId)"
            }
            $dashImpactGroupKey =
                ($dashImpactReachedGroupId -split '\.')[-1]
            $dashImpactNamespace = if ($isDashImpact159Group) {
                'wall159'
            } else {
                'outerwall109'
            }
            $expectedDashBindingId =
                "binding.valtan.${dashImpactNamespace}.${dashImpactGroupKey}.dash-charge-impact"
            $expectedDashMutationId =
                "mutation.valtan.${dashImpactNamespace}.${dashImpactGroupKey}.despawn"
            $ownedDashCollisionIds =
                @($sourceCollisionIdsByGroup[$dashImpactReachedGroupId])
            $expectedDashReceiverId =
                if (1 -eq $ownedDashCollisionIds.Count) {
                    [string]$ownedDashCollisionIds[0] + '.receiver'
                } else {
                    ''
                }
            if ([string]$dashImpactBinding.BindingId -cne
                    $expectedDashBindingId -or
                [string]$dashImpactBinding.MutationId -cne
                    $expectedDashMutationId -or
                [string]$dashImpactBinding.ReceiverId -cne
                    $expectedDashReceiverId) {
                throw "The general dash impact binding does not exact-join its stable binding/mutation/receiver identity: $($dashImpactBinding.BindingId)"
            }
            if ($isDashImpact159Group) { ++$dashImpact159GroupCount }
            if ($isDashImpactOuterGroup) { ++$dashImpactOuterGroupCount }
        }
        if ($dashImpactBindings.Count -ne $expectedDashImpactGroupCount -or
            $dashImpactGroupIds.Count -ne $expectedDashImpactGroupCount -or
            $dashImpactReceiverIds.Count -ne $expectedDashImpactGroupCount -or
            $dashImpact159GroupCount -ne $expectedImpactGroupCount -or
            $dashImpactOuterGroupCount -ne $expectedOuterGroupCount) {
            throw "The general dash destruction schedule must contain exactly $expectedImpactGroupCount independent 159 bindings and $expectedOuterGroupCount independent outer-ring bindings."
        }
        $expectedContactBindingCount = $expectedIndependentContactWallCount
        if ($contactBindings.Count -ne $expectedContactBindingCount -or
            $contactGroupIds.Count -ne $expectedContactBindingCount -or
            $contactReceiverIds.Count -ne $expectedContactBindingCount) {
            throw "Every ordinary wall must own exactly one independent contact binding."
        }
    }

    # A hole is the cells of a navigation condition. A mutation that claims to
    # take ground away without owning one has nothing to open, so it can never
    # reach the Server as a fall region.
    $removedGroundMutations = @(@($serverMutations) | Where-Object { [int]$_.RemovesGround -eq 1 })
    foreach ($removedGroundMutation in $removedGroundMutations) {
        if ([string]::IsNullOrEmpty([string]$removedGroundMutation.NavigationStateId) -or
            [string]$removedGroundMutation.NavigationStateId -ceq '-') {
            throw "A mutation that removes ground must own a navigation condition: $($removedGroundMutation.MutationId)"
        }
    }

    $semanticLines = [Collections.Generic.List[string]]::new()
    $semanticLines.Add("IDENTITY`t$expectedEncounterId`t$expectedAreaId`t$($Encounter.fixedTickHz)")
    foreach ($group in @(Sort-OrdinalByProperty @($serverGroups) 'GroupId')) {
        $semanticLines.Add((@('G',$group.GroupId,$group.InitialState,$group.Members.Count) + @($group.Members)) -join "`t")
    }
    foreach ($mutation in @(Sort-OrdinalByProperty @($serverMutations) 'MutationId')) {
        $semanticLines.Add((@('M',$mutation.MutationId,$mutation.GroupId,$mutation.FinalState,$mutation.BreakingTicks,$mutation.CollisionStateId,$mutation.NavigationStateId,$mutation.RemovesGround)) -join "`t")
    }
    foreach ($binding in @(Sort-OrdinalByProperty @($serverBindings) 'BindingId')) {
        $semanticLines.Add((@('B',$binding.BindingId,$binding.MutationId,$binding.TriggerKind,$binding.PatternId,$binding.StageId,$binding.ActionId,$binding.StageIndex,$binding.ReceiverId)) -join "`t")
    }
    $semanticBytes = [Text.UTF8Encoding]::new($false).GetBytes(($semanticLines -join "`n") + "`n")
    $revision = Get-Sha256LowerHex $semanticBytes
    if ($revision -cnotmatch $revisionPattern) { throw 'Generated revision is invalid.' }

    $serverLines = [Collections.Generic.List[string]]::new()
    $serverLines.Add("LOSTARK_WORLD_DESTRUCTION_BOOTSTRAP`t2`t$expectedEncounterId`t$expectedAreaId`t$revision`t$($Encounter.fixedTickHz)`t$($serverGroups.Count)`t$($serverMutations.Count)`t$($serverBindings.Count)")
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

    # One emitter per ring source, one bound suppress-only filler alias, and one
    # complete 12-piece recipe per source. The alias closes the intact visual
    # ring without doubling the one-tick PhysX debris budget.
    if ($isProductCandidate) {
        $deployAssetIds = Get-DeployPlacementAssetIds
        $anchor = Get-ArenaBreakLandingAnchor $Encounter
        $placementPositions = Get-DeployPlacementPositions
        $outerEmitterIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $outerAliasIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($groupId in @($outerGroupIds | Sort-Object -CaseSensitive)) {
            $outerProfile = $debrisProfileByGroupId[$groupId]
            if ($null -eq $outerProfile) {
                throw "Outer ring group has no debris presentation profile: $groupId"
            }
            foreach ($emitter in @($outerProfile.emitters)) {
                $sourceId = [string]$emitter.sourceRuntimePlacementId
                $parsedSourceId = Assert-CanonicalPlacementId `
                    $sourceId "outer ring source placementId"
                if (-not $outerMemberIds.Contains($sourceId) -or
                    -not $outerEmitterIds.Add($sourceId)) {
                    throw "Outer ring emitter is outside its group or duplicated: $groupId/$sourceId"
                }
                if (-not $deployAssetIds.ContainsKey($sourceId)) {
                    throw "Outer ring wall has no deploy placement row: $sourceId"
                }
                $aliasIds = @($emitter.suppressionAliasPlacementIds)
                if ($aliasIds.Count -ne $expectedOuterSuppressionAliasCount) {
                    throw "Outer ring source must own exactly $expectedOuterSuppressionAliasCount filler alias: $sourceId"
                }
                $fillerId = ''
                foreach ($aliasIdValue in $aliasIds) {
                    $aliasId = [string]$aliasIdValue
                    $parsedAliasId = Assert-CanonicalPlacementId `
                        $aliasId "outer ring filler placementId"
                    if (-not $outerMemberIds.Contains($aliasId) -or
                        -not $outerAliasIds.Add($aliasId)) {
                        throw "Outer ring filler alias is outside its group or duplicated: $groupId/$aliasId"
                    }
                    if (-not $deployAssetIds.ContainsKey($aliasId)) {
                        throw "Outer ring filler alias has no deploy placement row: $aliasId"
                    }
                    if ($parsedAliasId -ne
                        ($parsedSourceId + $expectedOuterFillerPlacementIdOffset)) {
                        throw "Outer ring filler does not share its source suffix: $sourceId/$aliasId"
                    }
                    if ([string]$deployAssetIds[$aliasId] -cne
                        $expectedOuterFillerAssetId) {
                        throw "Outer ring filler must use ${expectedOuterFillerAssetId}: $aliasId"
                    }
                    $fillerId = $aliasId
                }
                $pieceCount = Get-DebrisRecipePieceCount $deployAssetIds[$sourceId]
                if ($pieceCount -ne $expectedFragmentsPerEmitter) {
                    throw "Outer ring wall $sourceId fractures into $pieceCount pieces instead of $expectedFragmentsPerEmitter."
                }
                $outerFragmentActorCount += $pieceCount

                if (-not $placementPositions.ContainsKey($sourceId)) {
                    throw "Outer ring wall has no deploy transform: $sourceId"
                }
                $wall = $placementPositions[$sourceId]
                $radialX = $wall.X - $anchor.X
                $radialZ = $wall.Z - $anchor.Z
                $radialLength = [Math]::Sqrt($radialX * $radialX + $radialZ * $radialZ)
                if ([Math]::Abs($radialLength - $expectedOuterRingRadiusMeters) -gt
                    $expectedOuterTransformTolerance) {
                    throw "Outer ring source is outside the canonical radius: $sourceId/$radialLength"
                }
                if (-not $placementPositions.ContainsKey($fillerId)) {
                    throw "Outer ring filler has no deploy transform: $fillerId"
                }
                $filler = $placementPositions[$fillerId]
                $fillerRadialX = $filler.X - $anchor.X
                $fillerRadialZ = $filler.Z - $anchor.Z
                $fillerRadialLength = [Math]::Sqrt(
                    $fillerRadialX * $fillerRadialX +
                    $fillerRadialZ * $fillerRadialZ)
                if ([Math]::Abs(
                    $fillerRadialLength - $expectedOuterRingRadiusMeters) -gt
                    $expectedOuterTransformTolerance -or
                    [Math]::Abs($filler.Y - $wall.Y) -gt
                    $expectedOuterTransformTolerance) {
                    throw "Outer ring filler is outside its source ring: $sourceId/$fillerId"
                }
                $pairDot = $radialX * $fillerRadialX +
                    $radialZ * $fillerRadialZ
                $pairCross = $radialX * $fillerRadialZ -
                    $radialZ * $fillerRadialX
                $pairAngle = [Math]::Atan2($pairCross, $pairDot)
                if ($pairAngle -lt 0.0) { $pairAngle += 2.0 * [Math]::PI }
                if ([Math]::Abs(
                    $pairAngle - $expectedOuterFillerAngleRadians) -gt 0.0001) {
                    throw "Outer ring filler is not six degrees after its source: $sourceId/$fillerId"
                }
                $emitterX = [double]$emitter.direction[0]
                $emitterZ = [double]$emitter.direction[2]
                $emitterLength = [Math]::Sqrt(
                    $emitterX * $emitterX + $emitterZ * $emitterZ)
                if ($emitterLength -lt 0.001) {
                    throw "Outer ring debris has no horizontal direction: $sourceId"
                }
                $alignment = (($radialX / $radialLength) * ($emitterX / $emitterLength)) +
                    (($radialZ / $radialLength) * ($emitterZ / $emitterLength))
                if ($alignment -lt 0.999) {
                    throw "Outer ring wall $sourceId does not launch radially outward from $($anchor.AnchorId) (alignment $alignment)."
                }
                if ([double]$emitter.direction[1] -le 0.0) {
                    throw "Outer ring wall $sourceId has no upward lift."
                }
            }
        }
        if ($outerEmitterIds.Count -ne $expectedOuterEmitterCount -or
            $outerAliasIds.Count -ne
                ($expectedOuterEmitterCount * $expectedOuterSuppressionAliasCount)) {
            throw "The 109 outer ring must compile exactly $expectedOuterEmitterCount debris emitters and $($expectedOuterEmitterCount * $expectedOuterSuppressionAliasCount) filler aliases."
        }
        if ($outerFragmentActorCount -ne
            $expectedOuterEmitterCount * $expectedFragmentsPerEmitter) {
            throw "The 109 collapse must stage exactly $($expectedOuterEmitterCount * $expectedFragmentsPerEmitter) debris actors, not $outerFragmentActorCount."
        }
    }

    $removesGroundByGroupId = @{}
    foreach ($serverMutation in $serverMutations) {
        $removesGroundByGroupId[[string]$serverMutation.GroupId] =
            ([int]$serverMutation.RemovesGround -eq 1)
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
            removesGround = [bool]$removesGroundByGroupId[[string]$_.GroupId]
            suppressionAliasPlacementIds = @($aliasIds | Sort-Object { [uint64]$_ })
            memberPlacementIds = @($memberIds)
        }
    })
    $projection = [ordered]@{
        schema = 'lostark.world-destruction-client-projection'
        formatVersion = 3
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
        OuterGroupCount = $outerGroupIds.Count
        OuterMemberCount = $outerMemberIds.Count
        OuterEmitterCount = @($debrisProfiles | Where-Object {
            ([string]$_.groupId).StartsWith(
                $outerGroupIdPrefix, [StringComparison]::Ordinal)
        } | ForEach-Object { $_.emitters }).Count
        OuterSuppressionAliasCount = @($projectionGroups | Where-Object {
            ([string]$_.groupId).StartsWith(
                $outerGroupIdPrefix, [StringComparison]::Ordinal)
        } | ForEach-Object { $_.suppressionAliasPlacementIds }).Count
        OuterFragmentActorCount = $outerFragmentActorCount
        Interior109BindingCount = $interior109BindingCount
        ImpactBindingCount = $impactBindings.Count
        DashImpactBindingCount = $dashImpactBindings.Count
        ContactBindingCount = $contactBindings.Count
        FloorBindingCount = $floorBindings.Count
        FloorStageAGroupCount = $floorStageGroupIds[$floorStageAGroupIdPrefix].Count
        FloorStageBGroupCount = $floorStageGroupIds[$floorStageBGroupIdPrefix].Count
        FloorCellCount = $floorCellOwners.Count
        RemovedGroundMutationCount = $removedGroundMutations.Count
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
    $compilerResults = @(Compile-ValtanWorldDestruction $source $encounter $simulation)
    if ($compilerResults.Count -ne 1) {
        throw "World destruction compiler must return one artifact set, got $($compilerResults.Count)."
    }
    $canonical = $compilerResults[0]
    $gameplay = Read-JsonDocument $GameplayWorldPath
    $wallMutationOwners = @($canonical.ServerLines | Where-Object {
        $_.StartsWith("M`t", [StringComparison]::Ordinal)
    } | ForEach-Object {
        $columns = $_.Split("`t")
        if ($columns[5] -cne '-') { $columns[5] }
    })
    foreach ($box in @($gameplay.placements | Where-Object {
        $_.kind -ceq 'collisionBox' -and $_.enabled -and
        ([string]$_.placementId).StartsWith('collision.valtan.wallgroup.', [StringComparison]::Ordinal)
    })) {
        $boxId = [string]$box.placementId
        $owners = @($wallMutationOwners | Where-Object {
            $boxId -ceq [string]$_ -or $boxId.StartsWith([string]$_ + '.', [StringComparison]::Ordinal)
        })
        if ($owners.Count -ne 1) {
            throw "Every wall source and receiver must be cleared by exactly one final mutation: $boxId"
        }
    }
    $foreignEntranceChild = Copy-JsonObject $gameplay
    $foreignBox = Copy-JsonObject ($gameplay.placements | Where-Object {
        $_.placementId -ceq 'collision.valtan.wallgroup.frontwallA.receiver'
    } | Select-Object -First 1)
    $foreignBox.placementId = 'collision.valtan.wallgroup.frontwallA.foreign'
    $foreignEntranceChild.placements = @($foreignEntranceChild.placements) + @($foreignBox)
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $simulation $foreignEntranceChild
    } 'entrance prefix capturing an unrelated collision'

    $wrongWallCells = Read-NavigationBlockerCatalog $NavigationBlockersPath
    $wrongRegion = $wrongWallCells.Regions['navregion.valtan.wall159.15221142224278623810']
    $wrongRegion.CellKeys[0] = '342,70'
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $simulation $gameplay $wrongWallCells
    } 'old nearest-cell fallback several metres away from its wall'
    $missingWallNavigation = Copy-JsonObject $source
    ($missingWallNavigation.groups | Where-Object {
        $_.groupId -ceq 'destroyable.group.valtan.wall.15675921917269125843'
    }).navigationRegionIds = @()
    Assert-Throws {
        Compile-ValtanWorldDestruction $missingWallNavigation $encounter $simulation
    } 'wall standing on walkable cells without its own blocker region'
    $wrongWallGrid = Read-NavigationBlockerCatalog $NavigationBlockersPath
    $wrongWallGrid.OriginX += 0.5
    Assert-Throws {
        Compile-ValtanWorldDestruction $source $encounter $simulation $gameplay $wrongWallGrid
    } 'wall blocker grid with a shifted origin'

    # This real outer wall sits above another deck. XZ overlap alone used to
    # assign 29 cells to it, blocking the lower floor before its destruction.
    $deckGrid = Read-ValtanWallNavigationGrid (Resolve-RepoPath $BaseNavGridPath)
    $upperWall = @($gameplay.placements | Where-Object {
        $_.placementId -ceq 'collision.valtan.wallgroup.sector03.1090000000000012'
    })
    if ($upperWall.Count -ne 1 -or
        (Get-ValtanWallNavigationCellKeys $upperWall[0] $deckGrid).Count -ne 0) {
        throw 'An upper-deck wall must not own lower-deck navigation cells.'
    }
    $heightBlindGrid = $deckGrid.PSObject.Copy()
    $heightBlindGrid.BodyMinimumY = -100.0
    $heightBlindGrid.BodyMaximumY = 100.0
    $xzCells = Get-ValtanWallNavigationCellKeys $upperWall[0] $heightBlindGrid
    if ($xzCells.Count -eq 0) { throw 'The deck-height regression must exercise real XZ overlap.' }
    $probeCell = @($xzCells | Sort-Object)[0].Split(',')
    $probeIndex = [int]$probeCell[1] * $deckGrid.Width + [int]$probeCell[0]
    $heightOffset = 20 + $deckGrid.Width * $deckGrid.Height + $probeIndex * 4
    $lowerWall = Copy-JsonObject $upperWall[0]
    $lowerWall.position[1] = [BitConverter]::ToSingle($deckGrid.Bytes, $heightOffset) +
        [double]$lowerWall.halfExtents[1]
    if ((Get-ValtanWallNavigationCellKeys $lowerWall $deckGrid).Count -eq 0) {
        throw 'A wall lowered onto the same deck must block its intersecting cells.'
    }
    $invalidHeightGrid = Read-ValtanWallNavigationGrid (Resolve-RepoPath $BaseNavGridPath)
    [Array]::Copy([BitConverter]::GetBytes([single]::NaN), 0,
        $invalidHeightGrid.Bytes, $heightOffset, 4)
    Assert-Throws {
        Get-ValtanWallNavigationCellKeys $lowerWall $invalidHeightGrid
    } 'wall navigation with a non-finite ground height'
    $expectedFragmentActorCount =
        $expectedOuterEmitterCount * $expectedFragmentsPerEmitter
    # Every source wall is an independent group. The sixty-nine ordinary walls
    # own contact bindings; the thirty 109 source groups and their thirty visual
    # fillers share their mutations with one exact Dash receiver binding. The
    # graph also carries ten opening and forty general Dash impact bindings plus
    # two first-appearance bindings.
    $expectedFloorGroupCount =
        $expectedFloorStageAGroupCount + $expectedFloorStageBGroupCount
    $expectedCanonicalGroupCount =
        $expectedProductGroupCount + $expectedFloorGroupCount
    $expectedCanonicalBindingCount =
        $expectedIndependentContactWallCount + $expectedOuterGroupCount +
        $expectedInterior109BindingCount +
        $expectedImpactGroupCount + $expectedDashImpactGroupCount +
        $expectedEntranceGroupCount +
        $expectedFloorGroupCount
    if ($canonical.OuterGroupCount -ne $expectedOuterGroupCount -or
        $canonical.OuterMemberCount -ne $expectedOuterMemberCount -or
        $canonical.OuterEmitterCount -ne $expectedOuterEmitterCount -or
        $canonical.OuterSuppressionAliasCount -ne
            ($expectedOuterEmitterCount * $expectedOuterSuppressionAliasCount) -or
        $canonical.OuterFragmentActorCount -ne $expectedFragmentActorCount -or
        $canonical.Interior109BindingCount -ne
            $expectedInterior109BindingCount -or
        $canonical.ImpactBindingCount -ne $expectedImpactGroupCount -or
        $canonical.DashImpactBindingCount -ne $expectedDashImpactGroupCount -or
        $canonical.ContactBindingCount -ne $expectedIndependentContactWallCount -or
        $canonical.FloorBindingCount -ne $expectedFloorGroupCount -or
        $canonical.FloorStageAGroupCount -ne $expectedFloorStageAGroupCount -or
        $canonical.FloorStageBGroupCount -ne $expectedFloorStageBGroupCount -or
        $canonical.RemovedGroundMutationCount -ne $expectedFloorGroupCount -or
        $canonical.GroupCount -ne $expectedCanonicalGroupCount -or
        $canonical.BindingCount -ne $expectedCanonicalBindingCount -or
        $canonical.Revision -cnotmatch $revisionPattern) {
        throw "Canonical source did not compile $expectedProductGroupCount independent walls, $expectedIndependentContactWallCount ordinary contact bindings, $expectedFloorGroupCount floor collapse sectors and the exact 109/entrance/159+outer charge schedules."
    }
    $canonicalProjection = $canonical.ClientJson | ConvertFrom-Json
	$canonicalPresentation = $canonical.PresentationJson | ConvertFrom-Json
    if ($canonicalProjection.formatVersion -ne 3 -or
        @($canonicalProjection.groups).Count -ne $expectedCanonicalGroupCount) {
        throw 'Canonical projection did not compile the exact formatVersion 3 group contract.'
    }
	$projectedGroundGroups = @($canonicalProjection.groups | Where-Object {
		[bool]$_.removesGround
	})
	if ($projectedGroundGroups.Count -ne $expectedFloorGroupCount -or
		@($projectedGroundGroups | Where-Object {
			-not ([string]$_.groupId).StartsWith(
				$floorGroupIdPrefix, [StringComparison]::Ordinal)
		}).Count -ne 0) {
		throw 'Canonical projection ground-removal owners do not exactly match the six floor sectors.'
	}
	$projectionGroupIds = @($canonicalProjection.groups | ForEach-Object { [string]$_.groupId })
	$presentationGroupIds = @($canonicalPresentation.profiles | ForEach-Object { [string]$_.groupId })
	$projectionGroupIdSet = [Collections.Generic.HashSet[string]]::new(
		[StringComparer]::Ordinal)
	foreach ($groupId in $projectionGroupIds) { [void]$projectionGroupIdSet.Add($groupId) }
	$projectionOnlyGroupIds = @($projectionGroupIds | Where-Object {
		$_ -cnotin $presentationGroupIds
	})
	if ($canonicalPresentation.formatVersion -ne 1 -or
		$presentationGroupIds.Count -ne $expectedCanonicalGroupCount -or
		@($presentationGroupIds | Where-Object {
			-not $projectionGroupIdSet.Contains($_)
		}).Count -ne 0 -or
		$projectionOnlyGroupIds.Count -ne 0 -or
		@($presentationGroupIds | Where-Object {
			$_.StartsWith($floorGroupIdPrefix, [StringComparison]::Ordinal)
		}).Count -ne $expectedFloorGroupCount) {
		throw 'Canonical debris presentation must cover every projection group, including every floor collapse sector.'
	}
    $canonicalOuterGroups = @($canonicalProjection.groups | Where-Object {
        ([string]$_.groupId).StartsWith($outerGroupIdPrefix, [StringComparison]::Ordinal)
    })
    $canonicalOuterProfiles = @($canonicalPresentation.profiles | Where-Object {
        ([string]$_.groupId).StartsWith($outerGroupIdPrefix, [StringComparison]::Ordinal)
    })
    if ($canonicalOuterGroups.Count -ne $expectedOuterGroupCount -or
        @($canonicalOuterGroups | ForEach-Object {
            $_.suppressionAliasPlacementIds
        }).Count -ne
            ($expectedOuterEmitterCount * $expectedOuterSuppressionAliasCount) -or
        @($canonicalOuterGroups | ForEach-Object {
            $_.memberPlacementIds
        }).Count -ne $expectedOuterMemberCount -or
        @($canonicalOuterProfiles | ForEach-Object {
            $_.emitters
        }).Count -ne $expectedOuterEmitterCount) {
        throw 'Canonical projection did not compile the exact outer ring suppression contract.'
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
        $compiled.MemberCount -ne 1 -or $compiled.Revision -cnotmatch $revisionPattern) {
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

    # The collapse now takes the interior walls down with the ring, so the
    # regression to stop is no longer an interior wall joining it. It is a wall
    # silently dropping out of the batch, or a group that is not a wall at all
    # riding in on the same stage.
    $interiorDropout = Copy-JsonObject $source
    $interiorBinding = $interiorDropout.bindings | Where-Object {
        $_.mutationId -CEQ $firstMutationId -and
        $_.triggerKind -CEQ 'STAGE_ENTER' } | Select-Object -First 1
    $interiorBinding.enabled = $false
    Assert-Throws {
        Compile-ValtanWorldDestruction $interiorDropout $encounter $simulation
    } 'interior wall dropping out of the 109 batch'

    $floorRideAlong = Copy-JsonObject $source
    $floorRideAlongBinding = $floorRideAlong.bindings | Where-Object {
        $_.patternId -CEQ $floorStageAPatternId } | Select-Object -First 1
    $floorRideAlongBinding.patternId = $firstPatternId
    $floorRideAlongBinding.stageId = $firstStageId
    Assert-Throws {
        Compile-ValtanWorldDestruction $floorRideAlong $encounter $simulation
    } 'floor sector riding the 109 collapse'

    # A ring wall dropped from the batch leaves a permanent hole in the wall.
    $missingSector = Copy-JsonObject $source
    $droppedSector = $missingSector.bindings | Where-Object {
        $_.bindingId -CEQ 'binding.valtan.outerwall109.1090000000000016.impact'
    } | Select-Object -First 1
    $droppedSector.enabled = $false
    Assert-Throws {
        Compile-ValtanWorldDestruction $missingSector $encounter $simulation
    } 'missing independent outer ring wall'

    $invalidDashIdentity = Copy-JsonObject $source
    $invalidDashIdentityBinding = $invalidDashIdentity.bindings |
        Where-Object {
            $_.bindingId -CEQ
                'binding.valtan.outerwall109.1090000000000016.dash-charge-impact'
        } | Select-Object -First 1
    $invalidDashIdentityBinding.bindingId =
        'binding.valtan.outerwall109.1090000000000016.dash-charge-impact-renamed'
    Assert-Throws {
        Compile-ValtanWorldDestruction $invalidDashIdentity $encounter $simulation
    } 'renamed outer-ring Dash impact binding'

    $invalidDashMutation = Copy-JsonObject $source
    $invalidDashMutationBinding = $invalidDashMutation.bindings |
        Where-Object {
            $_.bindingId -CEQ
                'binding.valtan.outerwall109.1090000000000016.dash-charge-impact'
        } | Select-Object -First 1
    $invalidDashMutationBinding.mutationId =
        'mutation.valtan.outerwall109.1090000000000017.despawn'
    Assert-Throws {
        Compile-ValtanWorldDestruction $invalidDashMutation $encounter $simulation
    } 'misjoined outer-ring Dash impact mutation'

    $invalidDashReceiver = Copy-JsonObject $source
    $invalidDashReceiverBinding = $invalidDashReceiver.bindings |
        Where-Object {
            $_.bindingId -CEQ
                'binding.valtan.outerwall109.1090000000000016.dash-charge-impact'
        } | Select-Object -First 1
    $invalidDashReceiverBinding.receiverCollisionId =
        'collision.valtan.wallgroup.sector05.1090000000000017.receiver'
    Assert-Throws {
        Compile-ValtanWorldDestruction $invalidDashReceiver $encounter $simulation
    } 'misjoined outer-ring Dash impact receiver'

    # Removing one leaf from all three source collections is still a 29-wall
    # ring and must not publish as the product set.
    $missingWall = Copy-JsonObject $source
    $missingGroupId = 'destroyable.group.valtan.outerwall109.1090000000000027'
    $missingMutationId = 'mutation.valtan.outerwall109.1090000000000027.despawn'
    $missingWall.groups = @($missingWall.groups | Where-Object {
        $_.groupId -cne $missingGroupId })
    $missingWall.mutations = @($missingWall.mutations | Where-Object {
        $_.mutationId -cne $missingMutationId })
    $missingWall.bindings = @($missingWall.bindings | Where-Object {
        $_.mutationId -cne $missingMutationId })
    $missingWallSimulation = Copy-JsonObject $simulation
    $missingWallSimulation.profiles = @(
        $missingWallSimulation.profiles | Where-Object {
            $_.groupId -cne $missingGroupId })
    Assert-Throws {
        Compile-ValtanWorldDestruction $missingWall $encounter $missingWallSimulation
    } 'outer ring wall removed from its sector'

    # Removing a bound filler from both documents still leaves thirty source
    # emitters, but reopens one visible ring gap and must not publish.
    $missingFiller = Copy-JsonObject $source
    $missingFillerSimulation = Copy-JsonObject $simulation
    $missingFillerProfile = $missingFillerSimulation.profiles | Where-Object {
        ([string]$_.groupId).StartsWith(
            $outerGroupIdPrefix, [StringComparison]::Ordinal) -and
        @($_.elements | ForEach-Object {
            $_.suppressionAliasPlacementIds
        }).Count -gt 0
    } | Select-Object -First 1
    $missingFillerElement = $missingFillerProfile.elements | Where-Object {
        @($_.suppressionAliasPlacementIds).Count -gt 0
    } | Select-Object -First 1
    $missingFillerId = [string]$missingFillerElement.suppressionAliasPlacementIds[0]
    $missingFillerElement.suppressionAliasPlacementIds = @()
    $missingFillerGroup = $missingFiller.groups | Where-Object {
        $_.groupId -ceq $missingFillerProfile.groupId
    } | Select-Object -First 1
    $missingFillerGroup.memberPlacementIds = @(
        $missingFillerGroup.memberPlacementIds | Where-Object {
            [string]$_ -cne $missingFillerId
        })
    Assert-Throws {
        Compile-ValtanWorldDestruction `
            $missingFiller $encounter $missingFillerSimulation
    } 'outer ring filler removed from its source'

    # An alias is a runtime Deploy object even though it emits no fragments.
    # Refuse a projection that would name a filler the Client cannot suppress.
    $outerAliasId = [string](
        $simulation.profiles | Where-Object {
            ([string]$_.groupId).StartsWith(
                $outerGroupIdPrefix, [StringComparison]::Ordinal)
        } | ForEach-Object { $_.elements } | ForEach-Object {
            $_.suppressionAliasPlacementIds
        } | Select-Object -First 1)
    $preservedAssetIds = $script:DeployPlacementAssetIds
    try {
        $missingAliasAssetIds = @{}
        foreach ($entry in (Get-DeployPlacementAssetIds).GetEnumerator()) {
            if ([string]$entry.Key -cne $outerAliasId) {
                $missingAliasAssetIds[$entry.Key] = $entry.Value
            }
        }
        $script:DeployPlacementAssetIds = $missingAliasAssetIds
        Assert-Throws {
            Compile-ValtanWorldDestruction $source $encounter $simulation
        } 'outer ring filler without a deploy placement'

        $wrongAliasAssetIds = @{}
        foreach ($entry in $preservedAssetIds.GetEnumerator()) {
            $wrongAliasAssetIds[$entry.Key] = $entry.Value
        }
        $wrongAliasAssetIds[$outerAliasId] = 'DEPLOY_ITR_02306'
        $script:DeployPlacementAssetIds = $wrongAliasAssetIds
        Assert-Throws {
            Compile-ValtanWorldDestruction $source $encounter $simulation
        } 'outer ring filler with the source wall asset'
    }
    finally { $script:DeployPlacementAssetIds = $preservedAssetIds }

    $preservedPlacementPositions = $script:DeployPlacementPositions
    try {
        $offRingPositions = @{}
        foreach ($entry in (Get-DeployPlacementPositions).GetEnumerator()) {
            $position = $entry.Value
            $offRingPositions[$entry.Key] = [pscustomobject]@{
                X = [double]$position.X
                Y = [double]$position.Y
                Z = [double]$position.Z
            }
        }
        $offRingPositions[$outerAliasId].X += 0.25
        $script:DeployPlacementPositions = $offRingPositions
        Assert-Throws {
            Compile-ValtanWorldDestruction $source $encounter $simulation
        } 'outer ring filler outside its canonical half-angle'
    }
    finally { $script:DeployPlacementPositions = $preservedPlacementPositions }

    # A ring slab whose deploy asset owns no 12-piece recipe would vanish rather
    # than fracture, so the publisher rejects it before the Client ever sees it.
    $preservedAssetIds = $script:DeployPlacementAssetIds
    try {
        $injectedAssetIds = @{}
        foreach ($entry in (Get-DeployPlacementAssetIds).GetEnumerator()) {
            $injectedAssetIds[$entry.Key] = $entry.Value
        }
        $injectedAssetIds['1090000000000025'] = 'DEPLOY_ITR_02326'
        $script:DeployPlacementAssetIds = $injectedAssetIds
        Assert-Throws {
            Compile-ValtanWorldDestruction $source $encounter $simulation
        } 'outer ring wall without a debris recipe'
    }
    finally { $script:DeployPlacementAssetIds = $preservedAssetIds }

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
Write-Host "Valtan world destruction $Mode succeeded. groups=$($result.GroupCount) bindings=$($result.BindingCount) emitters=$($result.EmitterCount) outer109=$($result.OuterGroupCount)groups/$($result.OuterMemberCount)placements/$($result.OuterEmitterCount)emitters/$($result.OuterSuppressionAliasCount)aliases/$($result.OuterFragmentActorCount)fragments interior109=$($result.Interior109BindingCount) openingImpact159=$($result.ImpactBindingCount) dashImpact159PlusOuter=$($result.DashImpactBindingCount) contacts=$($result.ContactBindingCount) revision=$($result.Revision)"
