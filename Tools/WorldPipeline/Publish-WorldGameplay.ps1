[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/World',
	[ValidateRange(0, 12)]
	[int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'

function Read-ProjectJson {
    param([string]$RelativePath)
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) {
        throw "Required JSON document is missing: $RelativePath"
    }
	return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties {
    param(
        [object]$Value,
        [string[]]$Expected,
        [string]$Context
    )
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context has missing or unknown fields. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId {
    param([string]$Value, [string]$Context, [switch]$AllowEmpty)
    if ($AllowEmpty -and [string]::IsNullOrEmpty($Value)) {
        return
    }
    if ($Value -notmatch $stableIdPattern) {
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
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonNumber {
    param([object]$Value, [string]$Context)
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

function Assert-JsonString {
    param([object]$Value, [string]$Context, [switch]$AllowNull)
    if ($AllowNull -and $null -eq $Value) { return }
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

function Get-ActorIds {
    $characterCatalog = Read-ProjectJson 'Data/Actors/CharacterCatalog.json'
    $bossCatalog = Read-ProjectJson 'Data/Actors/BossCatalog.json'
    $npcCatalog = Read-ProjectJson 'Data/Actors/NpcCatalog.json'
	$monsterCatalog = Read-ProjectJson 'Data/Actors/MonsterCatalog.json'

    $idsByKind = @{
        playerSpawn = @($characterCatalog.characters |
            Where-Object runtimeStatus -eq 'supported' |
            ForEach-Object archetypeId)
        boss = @($bossCatalog.bosses | ForEach-Object archetypeId)
        npc = @($npcCatalog.npcs |
			Where-Object runtimeStatus -eq 'supported' |
			ForEach-Object archetypeId)
		monster = @($monsterCatalog.monsters |
			Where-Object runtimeStatus -eq 'supported' |
			ForEach-Object archetypeId)
    }
    return $idsByKind
}

function Format-InvariantFloat {
    param([double]$Value)
    if ([double]::IsNaN($Value) -or [double]::IsInfinity($Value) -or [math]::Abs($Value) -gt 100000.0) {
        throw "World coordinate is invalid: $Value"
    }
    return $Value.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
}

function Get-EncounterProfiles {
    $documents = @(
        (Read-ProjectJson 'Data/Encounters/Valtan/ValtanEncounter.json')
    )
    $profiles = @{}
    foreach ($document in $documents) {
        Assert-ExactProperties $document @(
            'schema','formatVersion','encounterId','bossArchetypeId',
            'authority','fixedTickHz','introPatternId','states','patterns') 'encounter profile'
		Assert-JsonInteger $document.formatVersion "$($document.encounterId) formatVersion" 3 3
		Assert-JsonInteger $document.fixedTickHz "$($document.encounterId) fixedTickHz" 30 30
        Assert-StableId $document.encounterId 'encounterId'
        Assert-StableId $document.bossArchetypeId 'bossArchetypeId'
        if ($document.schema -ne 'lostark.encounter-profile' -or
			$document.formatVersion -ne 3 -or
            $document.authority -ne 'server' -or
            $document.fixedTickHz -ne 30) {
            throw "Encounter header is invalid: $($document.encounterId)"
        }
        if ($profiles.ContainsKey([string]$document.encounterId)) {
            throw "Duplicate encounter ID: $($document.encounterId)"
        }

        $stateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($state in @($document.states)) {
            Assert-ExactProperties $state @('id','actionId','next') "$($document.encounterId) state"
			Assert-JsonString $state.id "$($document.encounterId) state id"
			Assert-JsonString $state.actionId "$($document.encounterId) state actionId"
			Assert-JsonString $state.next "$($document.encounterId) state next" -AllowNull
            Assert-StableId $state.id "$($document.encounterId) state id"
            Assert-StableId $state.actionId "$($document.encounterId) state actionId"
            Assert-StableId $state.next "$($document.encounterId) state next" -AllowEmpty
            if (-not $stateIds.Add([string]$state.id)) {
                throw "Duplicate encounter state ID: $($state.id)"
            }
        }
        if ($stateIds.Count -eq 0) {
            throw "Encounter requires at least one state: $($document.encounterId)"
        }

        $patterns = @($document.patterns)
        if ($patterns.Count -eq 0) {
            throw "Encounter requires at least one pattern: $($document.encounterId)"
        }
		# introPatternId is owned and strictly validated by the gameplay balance
		# publisher; this document only needs to tolerate its presence.
		$null = $document.PSObject.Properties['introPatternId']
		$patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		foreach ($pattern in $patterns) {
			# serverMotion is owned and strictly validated by the gameplay balance
			# publisher; this document only needs to tolerate its presence.
			$patternProperties = @(
				'patternId','displayName','actionId','sourceActionIds','selectionMode',
				'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
				'armorRequirement','phaseRequirement','invulnerableWhileRunning',
				'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
				'stages')
			if ($null -ne $pattern.PSObject.Properties['serverMotion']) {
				$patternProperties += 'serverMotion'
			}
			Assert-ExactProperties $pattern $patternProperties "$($document.encounterId) pattern"
			Assert-JsonNumber $pattern.minimumRange "$($document.encounterId) minimumRange"
			Assert-JsonNumber $pattern.maximumRange "$($document.encounterId) maximumRange"
			Assert-StableId $pattern.patternId "$($document.encounterId) patternId"
			Assert-StableId $pattern.actionId "$($document.encounterId) actionId"
			if (-not $patternIds.Add([string]$pattern.patternId)) {
				throw "Duplicate encounter pattern ID: $($pattern.patternId)"
			}
			if ([double]$pattern.minimumRange -lt 0.0 -or
				[double]$pattern.maximumRange -le [double]$pattern.minimumRange -or
				@($pattern.sourceActionIds).Count -eq 0 -or
				@($pattern.stages).Count -eq 0) {
				throw "Encounter pattern timing or range is invalid: $($pattern.patternId)"
			}
			foreach ($stage in @($pattern.stages)) {
				Assert-StableId $stage.stageId "$($document.encounterId) stageId"
				Assert-StableId $stage.actionId "$($document.encounterId) stage actionId"
				Assert-JsonInteger $stage.durationMs "$($document.encounterId) stage durationMs" 1 ([uint32]::MaxValue)
			}
		}
        $profiles[[string]$document.encounterId] = $document
    }
    return $profiles
}

function Get-MonsterProfiles {
    $document = Read-ProjectJson 'Data/Balance/MonsterProfiles.json'
    Assert-ExactProperties $document @('schema','formatVersion','basis','profiles') 'monster profiles'
    if ($document.schema -ne 'lostark.monster-profiles' -or
        $document.formatVersion -ne 1 -or $document.basis -ne 'PROJECT_TUNED') {
        throw 'Monster profile header is invalid.'
    }
    $profiles = @{}
    foreach ($profile in @($document.profiles)) {
        Assert-ExactProperties $profile @(
            'archetypeId','maxHp','attackPower','defense','collisionRadius',
            'engageRange','moveSpeed','attackRange','attackWindupMs',
            'attackActiveMs','attackRecoveryMs','deadDespawnMs',
            'hitKnockbackScale','attackPushRangeM','attackPushMs',
            'attackKnockdown','attackDownMs') 'monster profile'
        Assert-StableId $profile.archetypeId 'monster profile archetypeId'
        Assert-JsonInteger $profile.maxHp "$($profile.archetypeId) maxHp" 1 2000000000
        Assert-JsonInteger $profile.attackPower "$($profile.archetypeId) attackPower" 1 2000000000
        Assert-JsonInteger $profile.defense "$($profile.archetypeId) defense" 0 2000000000
        foreach ($field in @('collisionRadius','engageRange','moveSpeed','attackRange')) {
            Assert-JsonNumber $profile.$field "$($profile.archetypeId) $field"
            if ([double]$profile.$field -le 0.0 -or [double]$profile.$field -gt 1000.0) {
                throw "Monster profile $field is out of range: $($profile.archetypeId)"
            }
        }
        foreach ($field in @('attackWindupMs','attackActiveMs','attackRecoveryMs','deadDespawnMs')) {
            Assert-JsonInteger $profile.$field "$($profile.archetypeId) $field" 1 600000
        }
        Assert-JsonNumber $profile.hitKnockbackScale "$($profile.archetypeId) hitKnockbackScale"
        if ([double]$profile.hitKnockbackScale -lt 0.0 -or [double]$profile.hitKnockbackScale -gt 10.0) {
            throw "Monster profile hitKnockbackScale is out of range: $($profile.archetypeId)"
        }
        Assert-JsonNumber $profile.attackPushRangeM "$($profile.archetypeId) attackPushRangeM"
        foreach ($field in @('attackPushMs','attackDownMs')) {
            Assert-JsonInteger $profile.$field "$($profile.archetypeId) $field" 0 600000
        }
        if ($profile.attackKnockdown -isnot [bool]) {
            throw "Monster profile attackKnockdown must be a JSON Boolean: $($profile.archetypeId)"
        }
        $attackPushRangeM = [double]$profile.attackPushRangeM
        if ([math]::Abs($attackPushRangeM) -gt 20.0 -or
            ($attackPushRangeM -ne 0.0 -and [uint32]$profile.attackPushMs -eq 0) -or
            ($attackPushRangeM -eq 0.0 -and [uint32]$profile.attackPushMs -ne 0) -or
            ([bool]$profile.attackKnockdown -and [uint32]$profile.attackDownMs -eq 0) -or
            (-not [bool]$profile.attackKnockdown -and [uint32]$profile.attackDownMs -ne 0)) {
            throw "Monster profile attack push contract is invalid: $($profile.archetypeId)"
        }
        if ($profiles.ContainsKey([string]$profile.archetypeId)) {
            throw "Duplicate monster profile: $($profile.archetypeId)"
        }
        $profiles[[string]$profile.archetypeId] = $profile
    }
    return $profiles
}

function Convert-SpawnGroupsDocument {
    param(
        [string]$AreaId,
        [string]$WorldId,
        [hashtable]$ActorIds,
        [hashtable]$MonsterProfiles
    )

    $relativePath = "Data/Worlds/$AreaId/SpawnGroups.world.json"
    $absolutePath = [IO.Path]::GetFullPath((Join-Path $repoRoot $relativePath))
    if (-not [IO.File]::Exists($absolutePath)) {
        return [ordered]@{
            WorldId = $WorldId; AreaId = $AreaId; Revision = 1
            Lines = [Collections.Generic.List[string]]::new()
            GroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            Count = 0; IsPresent = $false
        }
    }
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @(
        'schema','formatVersion','areaId','revision','anchors','spawnGroups') $relativePath
    if ($document.schema -ne 'lostark.world-spawn-groups' -or
        $document.formatVersion -ne 1 -or $document.areaId -ne $AreaId) {
        throw "Spawn group header is invalid: $relativePath"
    }
    Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if (@($document.anchors).Count -gt 128 -or @($document.spawnGroups).Count -gt 32) {
        throw "Spawn group document exceeds its limits: $relativePath"
    }

    $anchorIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $anchorRows = [Collections.Generic.List[string]]::new()
    foreach ($anchor in @($document.anchors)) {
        Assert-ExactProperties $anchor @('anchorId','position','yawDegrees') "$relativePath anchor"
        Assert-StableId $anchor.anchorId "$relativePath anchorId"
        if (-not $anchorIds.Add([string]$anchor.anchorId)) {
            throw "Duplicate spawn anchor ID: $($anchor.anchorId)"
        }
        if (@($anchor.position).Count -ne 3) { throw "Spawn anchor requires three coordinates: $($anchor.anchorId)" }
        for ($i = 0; $i -lt 3; $i++) { Assert-JsonNumber $anchor.position[$i] "$relativePath anchor position[$i]" }
        Assert-JsonNumber $anchor.yawDegrees "$relativePath anchor yawDegrees"
        $anchorRows.Add((@('ANCHOR',[string]$anchor.anchorId,
            (Format-InvariantFloat $anchor.position[0]),
            (Format-InvariantFloat $anchor.position[1]),
            (Format-InvariantFloat $anchor.position[2]),
            (Format-InvariantFloat $anchor.yawDegrees)) -join "`t"))
    }

    $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.spawnGroups)) {
        Assert-StableId $group.spawnGroupId "$relativePath spawnGroupId"
        if (-not $groupIds.Add([string]$group.spawnGroupId)) {
            throw "Duplicate spawn group ID: $($group.spawnGroupId)"
        }
    }
    $groupRows = [Collections.Generic.List[string]]::new()
    $usedArchetypes = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.spawnGroups)) {
        Assert-ExactProperties $group @(
            'spawnGroupId','requiredCompletedGroupId','maxAlive','repeatPolicy',
            'completionPolicy','waves') "$relativePath group"
        Assert-JsonString $group.requiredCompletedGroupId "$relativePath prerequisite" -AllowNull
        if ($null -ne $group.requiredCompletedGroupId) {
            Assert-StableId $group.requiredCompletedGroupId "$relativePath prerequisite"
            if (-not $groupIds.Contains([string]$group.requiredCompletedGroupId)) {
                throw "Unknown spawn group prerequisite: $($group.requiredCompletedGroupId)"
            }
        }
        Assert-JsonInteger $group.maxAlive "$relativePath maxAlive" 1 64
        if ($group.repeatPolicy -ne 'ONCE' -or $group.completionPolicy -ne 'ALL_WAVES_CLEARED') {
            throw "Unsupported spawn group policy: $($group.spawnGroupId)"
        }
        $waves = @($group.waves)
        if ($waves.Count -lt 1 -or $waves.Count -gt 16) { throw "Spawn group wave count is invalid: $($group.spawnGroupId)" }
        $prerequisite = if ($null -eq $group.requiredCompletedGroupId) { '-' } else { [string]$group.requiredCompletedGroupId }
        $groupRows.Add((@('GROUP',[string]$group.spawnGroupId,$prerequisite,[string][uint32]$group.maxAlive,[string]$waves.Count) -join "`t"))
        $waveIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $totalCount = 0
        for ($waveIndex = 0; $waveIndex -lt $waves.Count; $waveIndex++) {
            $wave = $waves[$waveIndex]
            Assert-ExactProperties $wave @('waveId','startDelayMs','nextWavePolicy','entries') "$relativePath wave"
            Assert-StableId $wave.waveId "$relativePath waveId"
            if (-not $waveIds.Add([string]$wave.waveId)) { throw "Duplicate wave ID in group $($group.spawnGroupId): $($wave.waveId)" }
            Assert-JsonInteger $wave.startDelayMs "$relativePath startDelayMs" 0 600000
            if ($wave.nextWavePolicy -ne 'ALL_DEAD') { throw "Unsupported next wave policy: $($wave.waveId)" }
            $entries = @($wave.entries)
            if ($entries.Count -lt 1 -or $entries.Count -gt 16) { throw "Spawn wave entry count is invalid: $($wave.waveId)" }
            $groupRows.Add((@('WAVE',[string]$group.spawnGroupId,[string]$wave.waveId,
                [string]$waveIndex,[string][uint32]$wave.startDelayMs,[string]$entries.Count) -join "`t"))
            for ($entryIndex = 0; $entryIndex -lt $entries.Count; $entryIndex++) {
                $entry = $entries[$entryIndex]
                Assert-ExactProperties $entry @('archetypeId','count','anchorId','initialDelayMs','spawnIntervalMs') "$relativePath entry"
                Assert-StableId $entry.archetypeId "$relativePath entry archetypeId"
                Assert-StableId $entry.anchorId "$relativePath entry anchorId"
                if ($entry.archetypeId -notin @($ActorIds.monster) -or -not $MonsterProfiles.ContainsKey([string]$entry.archetypeId)) {
                    throw "Spawn entry references an unsupported monster archetype: $($entry.archetypeId)"
                }
                if (-not $anchorIds.Contains([string]$entry.anchorId)) { throw "Spawn entry references an unknown anchor: $($entry.anchorId)" }
                Assert-JsonInteger $entry.count "$relativePath entry count" 1 1000
                Assert-JsonInteger $entry.initialDelayMs "$relativePath initialDelayMs" 0 600000
                Assert-JsonInteger $entry.spawnIntervalMs "$relativePath spawnIntervalMs" 0 600000
                $totalCount += [uint32]$entry.count
                if ($totalCount -gt 1000) { throw "Spawn group exceeds total spawn limit: $($group.spawnGroupId)" }
                [void]$usedArchetypes.Add([string]$entry.archetypeId)
                $groupRows.Add((@('ENTRY',[string]$group.spawnGroupId,[string]$wave.waveId,
                    [string]$entryIndex,[string]$entry.archetypeId,[string][uint32]$entry.count,
                    [string]$entry.anchorId,[string][uint32]$entry.initialDelayMs,
                    [string][uint32]$entry.spawnIntervalMs) -join "`t"))
            }
        }
    }

    foreach ($group in @($document.spawnGroups)) {
        $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $cursor = $group
        while ($null -ne $cursor.requiredCompletedGroupId) {
            if (-not $seen.Add([string]$cursor.spawnGroupId)) { throw "Spawn group prerequisite cycle: $($group.spawnGroupId)" }
            $nextId = [string]$cursor.requiredCompletedGroupId
            $cursor = @($document.spawnGroups | Where-Object spawnGroupId -eq $nextId)[0]
        }
    }

    $profileRows = [Collections.Generic.List[string]]::new()
    foreach ($archetypeId in @($usedArchetypes | Sort-Object)) {
        $profile = $MonsterProfiles[$archetypeId]
        $profileRows.Add((@('PROFILE',$archetypeId,[string][uint32]$profile.maxHp,
            [string][uint32]$profile.attackPower,[string][uint32]$profile.defense,
            (Format-InvariantFloat $profile.collisionRadius),
            (Format-InvariantFloat $profile.engageRange),
            (Format-InvariantFloat $profile.moveSpeed),
            (Format-InvariantFloat $profile.attackRange),
            [string][uint32]$profile.attackWindupMs,[string][uint32]$profile.attackActiveMs,
            [string][uint32]$profile.attackRecoveryMs,[string][uint32]$profile.deadDespawnMs,
            (Format-InvariantFloat $profile.hitKnockbackScale),
            (Format-InvariantFloat $profile.attackPushRangeM),
            [string][uint32]$profile.attackPushMs,
            $(if ([bool]$profile.attackKnockdown) { '1' } else { '0' }),
            [string][uint32]$profile.attackDownMs) -join "`t"))
    }
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_SPAWN_GROUP_BOOTSTRAP`t3`t$WorldId`t$AreaId`t$($document.revision)`t$($anchorRows.Count)`t$($groupIds.Count)`t$($profileRows.Count)")
    foreach ($row in @($profileRows | Sort-Object)) { $lines.Add($row) }
    foreach ($row in @($anchorRows | Sort-Object)) { $lines.Add($row) }
    foreach ($row in $groupRows) { $lines.Add($row) }
    return [ordered]@{
        WorldId = $WorldId; AreaId = $AreaId; Revision = [uint32]$document.revision
        Lines = $lines; GroupIds = $groupIds; Count = $groupIds.Count; IsPresent = $true
    }
}

function Convert-WorldDocument {
    param(
        [string]$AreaId,
        [string]$WorldId,
        [hashtable]$ActorIds,
        [hashtable]$EncounterProfiles,
        [Collections.Generic.HashSet[string]]$SpawnGroupIds
    )

    $relativePath = "Data/Worlds/$AreaId/Gameplay.world.json"
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @('schema','formatVersion','areaId','revision','placements') $relativePath
	Assert-JsonString $document.schema "$relativePath schema"
	Assert-JsonInteger $document.formatVersion "$relativePath formatVersion" 5 5
	Assert-JsonString $document.areaId "$relativePath areaId"
	Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if ($document.schema -ne 'lostark.world-gameplay' -or
		$document.formatVersion -ne 5 -or
        $document.areaId -ne $AreaId -or
        $document.revision -lt 1) {
        throw "World gameplay header is invalid: $relativePath"
    }

    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$npcPresentationEntries = [Collections.Generic.List[object]]::new()
	$placementIndex = @{}
	foreach ($placement in @($document.placements)) {
		if ($null -ne $placement.placementId) {
			$placementIndex[[string]$placement.placementId] = $placement
		}
	}
	$enabledPlayerSpawnCount = 0
    $rows = [Collections.Generic.List[string]]::new()
    foreach ($placement in @($document.placements)) {
		Assert-JsonString $placement.placementId "$relativePath placementId"
		Assert-JsonString $placement.kind "$relativePath kind"
		if ($placement.enabled -isnot [bool]) {
			throw "$relativePath enabled must be a JSON Boolean: $($placement.placementId)"
		}
        Assert-StableId $placement.placementId "$relativePath placementId"
        if (-not $ids.Add([string]$placement.placementId)) {
            throw "Duplicate world placement ID: $($placement.placementId)"
        }
		if ($placement.kind -notin @('playerSpawn','npc','boss','triggerBox','collisionBox')) {
			throw "Unknown world placement kind: $($placement.kind)"
		}
        if (@($placement.position).Count -ne 3) {
            throw "Placement position must contain exactly three numbers: $($placement.placementId)"
        }
		for ($coordinateIndex = 0; $coordinateIndex -lt 3; $coordinateIndex++) {
			Assert-JsonNumber $placement.position[$coordinateIndex] `
				"$relativePath position[$coordinateIndex]"
		}
		Assert-JsonNumber $placement.yawDegrees "$relativePath yawDegrees"
        $enabled = if ($placement.enabled) { 1 } else { 0 }
		$commonFields = @(
			$placement.placementId,
			$placement.kind,
			'-',
			'-',
			(Format-InvariantFloat $placement.position[0]),
			(Format-InvariantFloat $placement.position[1]),
			(Format-InvariantFloat $placement.position[2]),
			(Format-InvariantFloat $placement.yawDegrees),
			$enabled)
		if ($placement.kind -eq 'collisionBox') {
			Assert-ExactProperties $placement @(
				'placementId','kind','position','yawDegrees','enabled',
				'halfExtents') "$relativePath collisionBox"
			if (@($placement.halfExtents).Count -ne 3) {
				throw "Collision Box halfExtents must contain exactly three numbers: $($placement.placementId)"
			}
			$collisionFields = @()
			for ($extentIndex = 0; $extentIndex -lt 3; $extentIndex++) {
				Assert-JsonNumber $placement.halfExtents[$extentIndex] "$relativePath halfExtents[$extentIndex]"
				$extent = [double]$placement.halfExtents[$extentIndex]
				if ($extent -le 0.0 -or $extent -gt 1000.0) {
					throw "Collision Box half extent is out of range: $($placement.placementId)"
				}
				$collisionFields += Format-InvariantFloat $extent
			}
			$rows.Add((($commonFields + $collisionFields) -join "`t"))
			continue
		}
		if ($placement.kind -eq 'triggerBox') {
			Assert-ExactProperties $placement @(
				'placementId','kind','position','yawDegrees','enabled',
				'halfExtents','triggerOnce','events') "$relativePath triggerBox"
			if (@($placement.halfExtents).Count -ne 3) {
				throw "Trigger halfExtents must contain exactly three numbers: $($placement.placementId)"
			}
			for ($extentIndex = 0; $extentIndex -lt 3; $extentIndex++) {
				Assert-JsonNumber $placement.halfExtents[$extentIndex] "$relativePath halfExtents[$extentIndex]"
				$extent = [double]$placement.halfExtents[$extentIndex]
				if ($extent -le 0.0 -or $extent -gt 1000.0) {
					throw "Trigger half extent is out of range: $($placement.placementId)"
				}
			}
			if ($placement.triggerOnce -isnot [bool]) {
				throw "Trigger triggerOnce must be a JSON Boolean: $($placement.placementId)"
			}
			$events = @($placement.events)
			if ($events.Count -gt 1 -or ($placement.enabled -and $events.Count -ne 1)) {
				throw "Trigger Box requires exactly one event when enabled: $($placement.placementId)"
			}
			$triggerFields = @(
				(Format-InvariantFloat $placement.halfExtents[0]),
				(Format-InvariantFloat $placement.halfExtents[1]),
				(Format-InvariantFloat $placement.halfExtents[2]),
				$(if ($placement.triggerOnce) { '1' } else { '0' }),
				[string]$events.Count)
			foreach ($event in $events) {
				if ($event.type -eq 'movePlayer') {
					Assert-ExactProperties $event @(
						'type','targetPosition','durationSeconds','arcHeight') "$relativePath movePlayer event"
					if (@($event.targetPosition).Count -ne 3) {
						throw "movePlayer target requires three coordinates: $($placement.placementId)"
					}
					for ($targetIndex = 0; $targetIndex -lt 3; $targetIndex++) {
						Assert-JsonNumber $event.targetPosition[$targetIndex] "$relativePath targetPosition[$targetIndex]"
					}
					Assert-JsonNumber $event.durationSeconds "$relativePath durationSeconds"
					Assert-JsonNumber $event.arcHeight "$relativePath arcHeight"
					$duration = [double]$event.durationSeconds
					$arcHeight = [double]$event.arcHeight
					if ($duration -lt 0.05 -or $duration -gt 10.0 -or
						$arcHeight -lt 0.0 -or $arcHeight -gt 1000.0) {
						throw "movePlayer timing or arc is out of range: $($placement.placementId)"
					}
					$triggerFields += @(
						'movePlayer', '5',
						(Format-InvariantFloat $event.targetPosition[0]),
						(Format-InvariantFloat $event.targetPosition[1]),
						(Format-InvariantFloat $event.targetPosition[2]),
						(Format-InvariantFloat $duration),
						(Format-InvariantFloat $arcHeight))
				}
				elseif ($event.type -eq 'changeLevel') {
					Assert-ExactProperties $event @('type','targetWorldId') "$relativePath changeLevel event"
					Assert-JsonString $event.targetWorldId "$relativePath changeLevel targetWorldId"
					if ($event.targetWorldId -notin @('BERN','VALTAN_ARENA') -or
						[string]$event.targetWorldId -eq $WorldId) {
						throw "changeLevel target is unknown or equals the source world: $($placement.placementId)"
					}
					if ($WorldId -notin @('BERN','VALTAN_ARENA')) {
						throw "changeLevel is only supported between Bern and Valtan Arena."
					}
					$triggerFields += @('changeLevel', '1', [string]$event.targetWorldId)
				}
				elseif ($event.type -eq 'activateSpawnGroup') {
					Assert-ExactProperties $event @('type','spawnGroupId') "$relativePath activateSpawnGroup event"
					Assert-StableId $event.spawnGroupId "$relativePath spawnGroupId"
					if ($null -eq $SpawnGroupIds -or -not $SpawnGroupIds.Contains([string]$event.spawnGroupId)) {
						throw "Trigger references an unknown spawn group: $($event.spawnGroupId)"
					}
					$triggerFields += @('activateSpawnGroup', '1', [string]$event.spawnGroupId)
				}
				elseif ($event.type -eq 'activateEncounter') {
					Assert-ExactProperties $event @('type','targetPlacementId') "$relativePath activateEncounter event"
					Assert-StableId $event.targetPlacementId "$relativePath targetPlacementId"
					$target = $placementIndex[[string]$event.targetPlacementId]
					if ($null -eq $target -or $target.kind -ne 'boss' -or $target.enabled) {
						throw "Encounter trigger target must be a disabled boss placement: $($event.targetPlacementId)"
					}
					$triggerFields += @('activateEncounter', '1', [string]$event.targetPlacementId)
				}
				else {
					throw "Unsupported product trigger event: $($event.type)"
				}
			}
			$rows.Add((($commonFields + $triggerFields) -join "`t"))
			continue
		}

		if ($placement.kind -eq 'npc') {
			Assert-ExactProperties $placement @(
				'placementId','kind','archetypeId','encounterId',
				'idleClip','position','yawDegrees','enabled') "$relativePath placement"
			if ($null -ne $placement.idleClip) {
				Assert-JsonString $placement.idleClip "$relativePath idleClip"
				$clip = [string]$placement.idleClip
				if ($clip -notmatch '^[A-Za-z0-9_~.-]{1,64}$') {
					throw "NPC idleClip is not a stable clip name: $($placement.placementId)"
				}
				$npcPresentationEntries.Add(@{
					placementId = [string]$placement.placementId
					idleClip = $clip
				})
			}
		}
		else {
			Assert-ExactProperties $placement @(
				'placementId','kind','archetypeId','encounterId',
				'position','yawDegrees','enabled') "$relativePath placement"
		}
		Assert-JsonString $placement.archetypeId "$relativePath archetypeId" -AllowNull
		Assert-JsonString $placement.encounterId "$relativePath encounterId" -AllowNull
		if ($placement.kind -eq 'playerSpawn') {
			if ($null -ne $placement.archetypeId -and
				-not [string]::IsNullOrEmpty([string]$placement.archetypeId)) {
				throw "Player spawn must not own a character archetype: $($placement.placementId)"
			}
			if ($null -ne $placement.encounterId -and
				-not [string]::IsNullOrEmpty([string]$placement.encounterId)) {
				throw "Player spawn must not own an encounter: $($placement.placementId)"
			}
			if ($placement.enabled) {
				++$enabledPlayerSpawnCount
			}
		}
		else {
			Assert-StableId $placement.archetypeId "$relativePath archetypeId"
		}
		Assert-StableId $placement.encounterId "$relativePath encounterId" -AllowEmpty
		if ($placement.kind -ne 'playerSpawn' -and
			$placement.archetypeId -notin @($ActorIds[$placement.kind])) {
			throw "Archetype '$($placement.archetypeId)' is not available for kind '$($placement.kind)'."
		}
		$encounterId = if ($null -eq $placement.encounterId) { '' } else { [string]$placement.encounterId }
		if ($encounterId -and -not $EncounterProfiles.ContainsKey($encounterId)) {
			throw "Placement references an unknown encounter: $encounterId"
		}
        $serializedEncounterId = if ($encounterId) { $encounterId } else { '-' }
        if ($placement.kind -eq 'boss') {
            if (-not $encounterId) {
                throw "Boss placement requires an encounter ID: $($placement.placementId)"
            }
            $profile = $EncounterProfiles[$encounterId]
            if ($profile.bossArchetypeId -ne $placement.archetypeId) {
                throw "Boss placement archetype does not match encounter '$encounterId'."
            }
        }
        $serializedArchetypeId = if ($placement.kind -eq 'playerSpawn') { '-' } else { [string]$placement.archetypeId }
		$commonFields[2] = $serializedArchetypeId
		$commonFields[3] = $serializedEncounterId
		$rowFields = $commonFields
        $rows.Add(($rowFields -join "`t"))
    }
	if ($WorldId -eq 'VALTAN_ARENA' -and $enabledPlayerSpawnCount -ne 4) {
		throw "Valtan Arena requires exactly four enabled player spawns; got $enabledPlayerSpawnCount."
	}

    $sortedRows = @($rows | Sort-Object)
    $lines = [Collections.Generic.List[string]]::new()
	$lines.Add("LOSTARK_WORLD_BOOTSTRAP`t6`t$WorldId`t$AreaId`t$($document.revision)`t$($sortedRows.Count)")
    foreach ($row in $sortedRows) {
        $lines.Add($row)
    }
    return [ordered]@{
        WorldId = $WorldId
        AreaId = $AreaId
        Lines = $lines
        Count = $sortedRows.Count
        NpcPresentation = $npcPresentationEntries
    }
}

$actorIds = Get-ActorIds
$encounterProfiles = Get-EncounterProfiles
$monsterProfiles = Get-MonsterProfiles
$spawnDocuments = @(
    (Convert-SpawnGroupsDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles)
)
$spawnByWorld = @{}
foreach ($spawn in $spawnDocuments) { $spawnByWorld[$spawn.WorldId] = $spawn }
$worlds = @(
    (Convert-WorldDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.BERN.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.VALTAN_ARENA.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.TRAINING_GROUND.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.CHARACTER_SELECT_ARENA.GroupIds)
)

if ($Mode -eq 'Publish') {
    $resolvedOutputRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    [IO.Directory]::CreateDirectory($resolvedOutputRoot) | Out-Null
	$transactionId = [Guid]::NewGuid().ToString('N')
	$stagingRoot = Join-Path $resolvedOutputRoot ".staging.$transactionId"
	$promotions = [Collections.Generic.List[object]]::new()
	[IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
	try {
		foreach ($world in $worlds) {
			$staged = Join-Path $stagingRoot "$($world.WorldId).worldbootstrap"
			[IO.File]::WriteAllLines(
				$staged,
				$world.Lines,
				[Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $world
				Staged = $staged
				Destination = Join-Path $resolvedOutputRoot "$($world.WorldId).worldbootstrap"
				Rollback = Join-Path $resolvedOutputRoot ".$($world.WorldId).rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}
		foreach ($spawn in @($spawnDocuments | Where-Object IsPresent)) {
			$staged = Join-Path $stagingRoot "$($spawn.WorldId).spawngroupsbootstrap"
			[IO.File]::WriteAllLines(
				$staged,
				$spawn.Lines,
				[Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $spawn
				Staged = $staged
				Destination = Join-Path $resolvedOutputRoot "$($spawn.WorldId).spawngroupsbootstrap"
				Rollback = Join-Path $resolvedOutputRoot ".$($spawn.WorldId).spawngroups.rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}
		$clientWorldRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Client/Bin/DataFiles/World'))
		[IO.Directory]::CreateDirectory($clientWorldRoot) | Out-Null
		foreach ($world in $worlds) {
			$staged = Join-Path $stagingRoot "$($world.WorldId).npcpresentation.json"
			$jsonLines = [Collections.Generic.List[string]]::new()
			$jsonLines.Add('{')
			$jsonLines.Add('  "schema": "lostark.npc-placement-presentation",')
			$jsonLines.Add('  "formatVersion": 1,')
			$jsonLines.Add("  `"worldId`": `"$($world.WorldId)`",")
			$entries = @($world.NpcPresentation | Sort-Object { [string]$_.placementId })
			if ($entries.Count -eq 0) {
				$jsonLines.Add('  "entries": []')
			}
			else {
				$jsonLines.Add('  "entries": [')
				for ($entryIndex = 0; $entryIndex -lt $entries.Count; $entryIndex++) {
					$suffix = if ($entryIndex -lt $entries.Count - 1) { ',' } else { '' }
					$jsonLines.Add("    { `"placementId`": `"$($entries[$entryIndex].placementId)`", `"idleClip`": `"$($entries[$entryIndex].idleClip)`" }$suffix")
				}
				$jsonLines.Add('  ]')
			}
			$jsonLines.Add('}')
			[IO.File]::WriteAllLines($staged, $jsonLines, [Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $world
				Staged = $staged
				Destination = Join-Path $clientWorldRoot "$($world.WorldId).npcpresentation.json"
				Rollback = Join-Path $clientWorldRoot ".$($world.WorldId).npcpresentation.rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}

		$promotedCount = 0
		foreach ($promotion in $promotions) {
			if ([IO.File]::Exists($promotion.Destination)) {
				[IO.File]::Move($promotion.Destination, $promotion.Rollback)
				$promotion.HadPrevious = $true
			}
			[IO.File]::Move($promotion.Staged, $promotion.Destination)
			$promotion.Promoted = $true
			$promotedCount++
			if ($FailureAfterPromote -eq $promotedCount) {
				throw "Injected world publish failure after promotion $promotedCount."
			}
		}

		foreach ($promotion in $promotions) {
			if ([IO.File]::Exists($promotion.Rollback)) {
				[IO.File]::Delete($promotion.Rollback)
			}
			Write-Output "Published $($promotion.World.WorldId): $($promotion.World.Count) placements -> $($promotion.Destination)"
		}
	}
	catch {
		$publishFailure = $_
		$rollbackFailures = [Collections.Generic.List[string]]::new()
		for ($index = $promotions.Count - 1; $index -ge 0; --$index) {
			$promotion = $promotions[$index]
			try {
				if ($promotion.Promoted -and [IO.File]::Exists($promotion.Destination)) {
					[IO.File]::Delete($promotion.Destination)
				}
				if ($promotion.HadPrevious) {
					if (-not [IO.File]::Exists($promotion.Rollback)) {
						throw "Rollback backup is missing: $($promotion.Rollback)"
					}
					[IO.File]::Move($promotion.Rollback, $promotion.Destination)
				}
			}
			catch {
				$rollbackFailures.Add("$($promotion.Destination): $($_.Exception.Message)")
			}
		}
		if ($rollbackFailures.Count -ne 0) {
			$preserved = @($promotions | Where-Object { [IO.File]::Exists($_.Rollback) } |
				ForEach-Object Rollback)
			throw "World publish failed: $($publishFailure.Exception.Message) Rollback recovery was incomplete. Preserved backups=[$($preserved -join ',')]. Failures=[$($rollbackFailures -join '; ')]"
		}
		throw $publishFailure
	}
	finally {
		if ([IO.Directory]::Exists($stagingRoot)) {
			Remove-Item -LiteralPath $stagingRoot -Recurse -Force
		}
	}
}
else {
	foreach ($world in $worlds) {
		Write-Output "Validated $($world.WorldId): $($world.Count) placements"
	}
	foreach ($spawn in @($spawnDocuments | Where-Object IsPresent)) {
		Write-Output "Validated $($spawn.WorldId): $($spawn.Count) spawn groups"
	}
}
