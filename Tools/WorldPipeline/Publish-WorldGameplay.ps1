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
$valtanRaidPlayerCapacity = 8

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
        (Read-ProjectJson 'Data/Encounters/Valtan/ValtanEncounter.json'),
        (Read-ProjectJson `
            'Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json')
    )
    # One encounter may own several boss archetypes (the KoukuSaydon arena
    # gate bosses share the Gate 1 audition encounter). The boss profile
    # document's encounterId is the join, so a placement archetype is admitted
    # when its Server profile names this encounter.
    $bossProfileDocument = Read-ProjectJson 'Data/Balance/BossProfiles.json'
    $bossArchetypesByEncounter = @{}
    foreach ($bossProfile in @($bossProfileDocument.bosses)) {
        $profileEncounterId = [string]$bossProfile.encounterId
        if (-not $bossArchetypesByEncounter.ContainsKey($profileEncounterId)) {
            $bossArchetypesByEncounter[$profileEncounterId] =
                [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        }
        $null = $bossArchetypesByEncounter[$profileEncounterId].Add(
            [string]$bossProfile.archetypeId)
    }
    $profiles = @{}
    foreach ($document in $documents) {
		$isKoukuSaydon = [string]$document.encounterId -ceq
			'ENCOUNTER_KAKULSAYDON_G1'
		$encounterProperties = @(
			'schema','formatVersion','encounterId','bossArchetypeId',
			'authority','fixedTickHz','patterns')
		if ($isKoukuSaydon) {
			$encounterProperties += @('sourceRevision','playAllPatternIds')
		}
		else {
			$encounterProperties += @('introPatternId','states')
		}
        Assert-ExactProperties $document $encounterProperties 'encounter profile'
		Assert-JsonInteger $document.formatVersion "$($document.encounterId) formatVersion" 4 4
		Assert-JsonInteger $document.fixedTickHz "$($document.encounterId) fixedTickHz" 30 30
        Assert-StableId $document.encounterId 'encounterId'
        Assert-StableId $document.bossArchetypeId 'bossArchetypeId'
        if ($document.schema -ne 'lostark.encounter-profile' -or
			$document.formatVersion -ne 4 -or
            $document.authority -ne 'server' -or
            $document.fixedTickHz -ne 30) {
            throw "Encounter header is invalid: $($document.encounterId)"
        }
		if ($isKoukuSaydon) {
			Assert-JsonInteger $document.sourceRevision `
				'KoukuSaydon encounter sourceRevision' 1 ([uint32]::MaxValue)
			if ([string]$document.bossArchetypeId -cne
				'BOSS_KAKULSAYDON_G1_KOUKU' -or
				$document.playAllPatternIds -isnot [Array] -or
				@($document.playAllPatternIds).Count -lt 1) {
				throw 'KoukuSaydon encounter Product identity is invalid.'
			}
		}
        if ($profiles.ContainsKey([string]$document.encounterId)) {
            throw "Duplicate encounter ID: $($document.encounterId)"
        }

        if (-not $isKoukuSaydon) {
			$stateIds =
				[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			foreach ($state in @($document.states)) {
				Assert-ExactProperties $state @('id','actionId','next') `
					"$($document.encounterId) state"
				Assert-JsonString $state.id "$($document.encounterId) state id"
				Assert-JsonString $state.actionId `
					"$($document.encounterId) state actionId"
				Assert-JsonString $state.next `
					"$($document.encounterId) state next" -AllowNull
				Assert-StableId $state.id "$($document.encounterId) state id"
				Assert-StableId $state.actionId `
					"$($document.encounterId) state actionId"
				Assert-StableId $state.next `
					"$($document.encounterId) state next" -AllowEmpty
				if (-not $stateIds.Add([string]$state.id)) {
					throw "Duplicate encounter state ID: $($state.id)"
				}
			}
			if ($stateIds.Count -eq 0) {
				throw "Encounter requires at least one state: $($document.encounterId)"
			}
		}

        $patterns = @($document.patterns)
        if ($patterns.Count -eq 0) {
            throw "Encounter requires at least one pattern: $($document.encounterId)"
        }
		# Valtan introPatternId is owned by the gameplay balance publisher. K's
		# animation-only Product intentionally has no automatic intro selector.
		if (-not $isKoukuSaydon) {
			$null = $document.PSObject.Properties['introPatternId']
		}
		$patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		foreach ($pattern in $patterns) {
			# serverMotion/finale are owned and strictly validated by the gameplay balance
			# publisher; this document only needs to tolerate its presence.
			$patternProperties = @(
				'patternId','displayName','actionId','sourceActionIds','selectionMode',
				'category','minimumPhase','maximumPhase','targetPolicy','aimPolicy',
				'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
				'armorRequirement','phaseRequirement','invulnerableWhileRunning',
				'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
				'stages')
			if ($null -ne $pattern.PSObject.Properties['serverMotion']) {
				$patternProperties += 'serverMotion'
			}
			if ($null -ne $pattern.PSObject.Properties['finale']) {
				$patternProperties += 'finale'
			}
			if ($null -ne $pattern.PSObject.Properties['bossArchetypeIds']) {
				# KoukuSaydon Product names the arena boss bodies a pattern may
				# play on; the gameplay balance publisher validates the values.
				$patternProperties += 'bossArchetypeIds'
			}
			if ($null -ne $pattern.PSObject.Properties['verticalOffsetM']) {
				# The gameplay-balance publisher owns the strict value and stage-response
				# contract.  The world publisher only needs to preserve compatibility
				# while resolving encounter IDs for placements.
				$patternProperties += 'verticalOffsetM'
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
				(-not $isKoukuSaydon -and @($pattern.sourceActionIds).Count -eq 0) -or
				@($pattern.stages).Count -eq 0) {
				throw "Encounter pattern timing or range is invalid: $($pattern.patternId)"
			}
			foreach ($stage in @($pattern.stages)) {
				Assert-StableId $stage.stageId "$($document.encounterId) stageId"
				Assert-StableId $stage.actionId "$($document.encounterId) stage actionId"
				Assert-JsonInteger $stage.durationMs "$($document.encounterId) stage durationMs" 1 ([uint32]::MaxValue)
			}
		}
		if ($isKoukuSaydon) {
			$playAllIds = @($document.playAllPatternIds)
			if ($playAllIds.Count -ne $patternIds.Count) {
				throw 'KoukuSaydon playAll inventory differs from Product patterns.'
			}
			for ($index = 0; $index -lt $playAllIds.Count; ++$index) {
				Assert-JsonString $playAllIds[$index] `
					'KoukuSaydon playAll patternId'
				Assert-StableId $playAllIds[$index] `
					'KoukuSaydon playAll patternId'
				if ([string]$playAllIds[$index] -cne
					[string]$patterns[$index].patternId) {
					throw 'KoukuSaydon playAll order differs from Product order.'
				}
			}
		}
        $encounterBossArchetypeIds =
            [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $null = $encounterBossArchetypeIds.Add([string]$document.bossArchetypeId)
        if ($bossArchetypesByEncounter.ContainsKey([string]$document.encounterId)) {
            foreach ($archetypeId in $bossArchetypesByEncounter[[string]$document.encounterId]) {
                $null = $encounterBossArchetypeIds.Add($archetypeId)
            }
        }
        $profiles[[string]$document.encounterId] = [pscustomobject]@{
            Document = $document
            BossArchetypeIds = $encounterBossArchetypeIds
        }
    }
    return $profiles
}

function Get-MonsterProfiles {
    $document = Read-ProjectJson 'Data/Balance/MonsterProfiles.json'
    Assert-ExactProperties $document @('schema','formatVersion','basis','profiles') 'monster profiles'
    if ($document.schema -ne 'lostark.monster-profiles' -or
        $document.formatVersion -ne 2 -or $document.basis -ne 'PROJECT_TUNED') {
        throw 'Monster profile header is invalid.'
    }
    $profiles = @{}
    foreach ($profile in @($document.profiles)) {
        $profileProperties = @(
            'archetypeId','maxHp','attackPower','defense','collisionRadius',
            'engageRange','targetReleaseRange','moveSpeed',
            'turnSpeedDegreesPerSecond','acceleration','deceleration',
            'arrivalSlowRadius','attackRange','attackWindupMs',
            'attackActiveMs','attackRecoveryMs','deadDespawnMs',
            'hitKnockbackScale','attackPushRangeM','attackPushMs',
            'attackKnockdown','attackDownMs')
        Assert-ExactProperties $profile $profileProperties 'monster profile'
        Assert-StableId $profile.archetypeId 'monster profile archetypeId'
        Assert-JsonInteger $profile.maxHp "$($profile.archetypeId) maxHp" 1 2000000000
        Assert-JsonInteger $profile.attackPower "$($profile.archetypeId) attackPower" 1 2000000000
        Assert-JsonInteger $profile.defense "$($profile.archetypeId) defense" 0 2000000000
        foreach ($field in @(
            'collisionRadius','engageRange','targetReleaseRange','moveSpeed',
            'turnSpeedDegreesPerSecond','acceleration','deceleration',
            'arrivalSlowRadius','attackRange')) {
            Assert-JsonNumber $profile.$field "$($profile.archetypeId) $field"
            if ([double]$profile.$field -le 0.0 -or [double]$profile.$field -gt 1000.0) {
                throw "Monster profile $field is out of range: $($profile.archetypeId)"
            }
        }
        if ([double]$profile.targetReleaseRange -lt [double]$profile.engageRange -or
            [double]$profile.arrivalSlowRadius -lt [double]$profile.collisionRadius) {
            throw "Monster target release or arrival radius is invalid: $($profile.archetypeId)"
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
            'repeatDelayMs','completionPolicy','waves') "$relativePath group"
        Assert-JsonString $group.requiredCompletedGroupId "$relativePath prerequisite" -AllowNull
        if ($null -ne $group.requiredCompletedGroupId) {
            Assert-StableId $group.requiredCompletedGroupId "$relativePath prerequisite"
            if (-not $groupIds.Contains([string]$group.requiredCompletedGroupId)) {
                throw "Unknown spawn group prerequisite: $($group.requiredCompletedGroupId)"
            }
        }
        Assert-JsonInteger $group.maxAlive "$relativePath maxAlive" 1 64
        if ($group.completionPolicy -ne 'ALL_WAVES_CLEARED') {
            throw "Unsupported spawn group completion policy: $($group.spawnGroupId)"
        }
        if ($group.repeatPolicy -ne 'ONCE' -and $group.repeatPolicy -ne 'REPEAT') {
            throw "Unsupported spawn group repeat policy: $($group.spawnGroupId)"
        }
        Assert-JsonInteger $group.repeatDelayMs "$relativePath repeatDelayMs" 0 600000
        # The delay only means something to REPEAT, so a ONCE group carrying one
        # is an authoring mistake rather than a value the runtime should ignore.
        if (($group.repeatPolicy -eq 'REPEAT') -ne ([uint32]$group.repeatDelayMs -ne 0)) {
            throw "Spawn group repeat delay contradicts its policy: $($group.spawnGroupId)"
        }
        $waves = @($group.waves)
        if ($waves.Count -lt 1 -or $waves.Count -gt 16) { throw "Spawn group wave count is invalid: $($group.spawnGroupId)" }
        $prerequisite = if ($null -eq $group.requiredCompletedGroupId) { '-' } else { [string]$group.requiredCompletedGroupId }
        $groupRows.Add((@('GROUP',[string]$group.spawnGroupId,$prerequisite,
            [string][uint32]$group.maxAlive,[string]$waves.Count,
            [string]$group.repeatPolicy,[string][uint32]$group.repeatDelayMs) -join "`t"))
        $waveIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $totalCount = 0
        for ($waveIndex = 0; $waveIndex -lt $waves.Count; $waveIndex++) {
            $wave = $waves[$waveIndex]
            Assert-ExactProperties $wave @('waveId','startDelayMs','nextWavePolicy','nextWaveDelayMs','entries') "$relativePath wave"
            Assert-StableId $wave.waveId "$relativePath waveId"
            if (-not $waveIds.Add([string]$wave.waveId)) { throw "Duplicate wave ID in group $($group.spawnGroupId): $($wave.waveId)" }
            Assert-JsonInteger $wave.startDelayMs "$relativePath startDelayMs" 0 600000
            if ($wave.nextWavePolicy -ne 'ALL_DEAD' -and $wave.nextWavePolicy -ne 'TIMER') {
                throw "Unsupported next wave policy: $($wave.waveId)"
            }
            Assert-JsonInteger $wave.nextWaveDelayMs "$relativePath nextWaveDelayMs" 0 600000
            if (($wave.nextWavePolicy -eq 'TIMER') -ne ([uint32]$wave.nextWaveDelayMs -ne 0)) {
                throw "Spawn wave delay contradicts its policy: $($wave.waveId)"
            }
            $entries = @($wave.entries)
            if ($entries.Count -lt 1 -or $entries.Count -gt 16) { throw "Spawn wave entry count is invalid: $($wave.waveId)" }
            $groupRows.Add((@('WAVE',[string]$group.spawnGroupId,[string]$wave.waveId,
                [string]$waveIndex,[string][uint32]$wave.startDelayMs,[string]$entries.Count,
                [string]$wave.nextWavePolicy,[string][uint32]$wave.nextWaveDelayMs) -join "`t"))
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
    # Kept apart from $profileRows because the header publishes that list's
    # count as the profile count, and the Server rejects a bootstrap whose
    # PROFILE rows do not match it.
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
            [string][uint32]$profile.attackDownMs,
            (Format-InvariantFloat $profile.targetReleaseRange),
            (Format-InvariantFloat $profile.turnSpeedDegreesPerSecond),
            (Format-InvariantFloat $profile.acceleration),
            (Format-InvariantFloat $profile.deceleration),
            (Format-InvariantFloat $profile.arrivalSlowRadius)) -join "`t"))
    }
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_SPAWN_GROUP_BOOTSTRAP`t4`t$WorldId`t$AreaId`t$($document.revision)`t$($anchorRows.Count)`t$($groupIds.Count)`t$($profileRows.Count)")
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
	Assert-JsonInteger $document.formatVersion "$relativePath formatVersion" 6 6
	Assert-JsonString $document.areaId "$relativePath areaId"
	Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if ($document.schema -ne 'lostark.world-gameplay' -or
		$document.formatVersion -ne 6 -or
        $document.areaId -ne $AreaId -or
        $document.revision -lt 1) {
        throw "World gameplay header is invalid: $relativePath"
    }

    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$npcPresentationEntries = [Collections.Generic.List[object]]::new()
	$enabledWorldEntityCount = 0
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
				elseif ($event.type -eq 'playSequence') {
					Assert-ExactProperties $event @('type','sequenceInstanceId') "$relativePath playSequence event"
					Assert-StableId $event.sequenceInstanceId "$relativePath sequenceInstanceId"
					# The instance must exist in this Area's authored world
					# sequence document, otherwise the trigger would fire into
					# nothing once the level is running.
					$sequencePath = "Data/Maps/Authoring/$AreaId/$AreaId.worldsequences.json"
					$sequenceFull = [IO.Path]::GetFullPath((Join-Path $repoRoot $sequencePath))
					if (-not [IO.File]::Exists($sequenceFull)) {
						throw "playSequence trigger requires an authored world sequence document: $sequencePath"
					}
					$sequenceDocument = Read-ProjectJson $sequencePath
					$known = @($sequenceDocument.instances | ForEach-Object { [string]$_.instanceId })
					if (-not $known.Contains([string]$event.sequenceInstanceId)) {
						throw "Trigger references an unknown world sequence instance: $($event.sequenceInstanceId)"
					}
					$triggerFields += @('playSequence', '1', [string]$event.sequenceInstanceId)
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
				'idleClip','behavior','position','yawDegrees','enabled') "$relativePath placement"
			$presentationEntry = [ordered]@{
				placementId = [string]$placement.placementId
				idleClip = $null
				walkClip = $null
				actions = @()
			}
			if ($null -ne $placement.idleClip) {
				Assert-JsonString $placement.idleClip "$relativePath idleClip"
				$clip = [string]$placement.idleClip
				if ($clip -notmatch '^[A-Za-z0-9_~.-]{1,64}$') {
					throw "NPC idleClip is not a stable clip name: $($placement.placementId)"
				}
				$presentationEntry.idleClip = $clip
			}

			$behaviorFields = @('0')
			if ($null -ne $placement.behavior) {
				$behavior = $placement.behavior
				Assert-ExactProperties -Value $behavior -Expected @(
					'mode','routeMode','actionSelection','walkClip','moveSpeed',
					'wanderRadius','randomSeed','startDelayMs','idleMinMs',
					'idleMaxMs','lookTargetPlacementId','waypoints','actions') `
					-Context "$relativePath NPC behavior"
				Assert-JsonString $behavior.mode "$relativePath behavior mode"
				Assert-JsonString $behavior.routeMode "$relativePath behavior routeMode"
				Assert-JsonString $behavior.actionSelection "$relativePath behavior actionSelection"
				Assert-JsonString $behavior.walkClip "$relativePath behavior walkClip" -AllowNull
				Assert-JsonNumber $behavior.moveSpeed "$relativePath behavior moveSpeed"
				Assert-JsonNumber $behavior.wanderRadius "$relativePath behavior wanderRadius"
				Assert-JsonInteger $behavior.randomSeed "$relativePath behavior randomSeed" 1 ([uint32]::MaxValue)
				Assert-JsonInteger $behavior.startDelayMs "$relativePath behavior startDelayMs" 0 600000
				Assert-JsonInteger $behavior.idleMinMs "$relativePath behavior idleMinMs" 0 600000
				Assert-JsonInteger $behavior.idleMaxMs "$relativePath behavior idleMaxMs" 0 600000
				Assert-JsonString $behavior.lookTargetPlacementId "$relativePath behavior lookTargetPlacementId" -AllowNull
				if ($behavior.mode -notin @('stationary','patrol','wander') -or
					$behavior.routeMode -notin @('loop','pingPong','once') -or
					$behavior.actionSelection -notin @('sequence','weighted')) {
					throw "NPC behavior enum is invalid: $($placement.placementId)"
				}
				$moveSpeed = [double]$behavior.moveSpeed
				$wanderRadius = [double]$behavior.wanderRadius
				if ($moveSpeed -lt 0.1 -or $moveSpeed -gt 10.0 -or
					$wanderRadius -lt 0.0 -or $wanderRadius -gt 100.0 -or
					[uint32]$behavior.idleMinMs -gt [uint32]$behavior.idleMaxMs) {
					throw "NPC behavior range is invalid: $($placement.placementId)"
				}
				$walkClip = $null
				if ($null -ne $behavior.walkClip) {
					$walkClip = [string]$behavior.walkClip
					if ($walkClip -notmatch '^[A-Za-z0-9_~.-]{1,64}$') {
						throw "NPC walkClip is not a stable clip name: $($placement.placementId)"
					}
				}
				$waypoints = @($behavior.waypoints)
				$actions = @($behavior.actions)
				if ($waypoints.Count -gt 64 -or $actions.Count -gt 32 -or
					($behavior.mode -eq 'stationary' -and
						($waypoints.Count -ne 0 -or $wanderRadius -ne 0.0)) -or
					($behavior.mode -eq 'patrol' -and
						($waypoints.Count -lt 2 -or $wanderRadius -ne 0.0 -or $null -eq $walkClip)) -or
					($behavior.mode -eq 'wander' -and
						($waypoints.Count -ne 0 -or $wanderRadius -lt 0.5 -or $null -eq $walkClip))) {
					throw "NPC behavior shape is invalid: $($placement.placementId)"
				}
				$lookTarget = '-'
				if ($null -ne $behavior.lookTargetPlacementId) {
					Assert-StableId $behavior.lookTargetPlacementId "$relativePath lookTargetPlacementId"
					$lookTargetPlacement = $placementIndex[[string]$behavior.lookTargetPlacementId]
					if ($null -eq $lookTargetPlacement -or
						$lookTargetPlacement.kind -ne 'npc' -or
						$lookTargetPlacement.enabled -ne $true -or
						[string]$lookTargetPlacement.placementId -eq [string]$placement.placementId) {
						throw "NPC look target must reference another enabled NPC placement: $($placement.placementId)"
					}
					$lookTarget = [string]$behavior.lookTargetPlacementId
				}

				$waypointFields = [Collections.Generic.List[string]]::new()
				$waypointIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
				foreach ($waypoint in $waypoints) {
					Assert-ExactProperties -Value $waypoint -Expected @(
						'waypointId','position','waitMs','lookYawDegrees') `
						-Context "$relativePath NPC waypoint"
					Assert-StableId $waypoint.waypointId "$relativePath waypointId"
					if (-not $waypointIds.Add([string]$waypoint.waypointId) -or
						@($waypoint.position).Count -ne 3) {
						throw "NPC waypoint ID or position is invalid: $($placement.placementId)"
					}
					for ($coordinateIndex = 0; $coordinateIndex -lt 3; $coordinateIndex++) {
						Assert-JsonNumber $waypoint.position[$coordinateIndex] "$relativePath waypoint position[$coordinateIndex]"
					}
					Assert-JsonInteger $waypoint.waitMs "$relativePath waypoint waitMs" 0 600000
					if ($null -ne $waypoint.lookYawDegrees) {
						Assert-JsonNumber $waypoint.lookYawDegrees "$relativePath waypoint lookYawDegrees"
						$lookYaw = [double]$waypoint.lookYawDegrees
						if ($lookYaw -lt -360.0 -or $lookYaw -gt 360.0) {
							throw "NPC waypoint look yaw is invalid: $($waypoint.waypointId)"
						}
						$hasLookYaw = '1'
					}
					else {
						$lookYaw = 0.0
						$hasLookYaw = '0'
					}
					foreach ($field in @(
						[string]$waypoint.waypointId,
						(Format-InvariantFloat $waypoint.position[0]),
						(Format-InvariantFloat $waypoint.position[1]),
						(Format-InvariantFloat $waypoint.position[2]),
						[string][uint32]$waypoint.waitMs,
						$hasLookYaw,
						(Format-InvariantFloat $lookYaw))) {
						$waypointFields.Add($field)
					}
				}

				$actionFields = [Collections.Generic.List[string]]::new()
				$actionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
				$presentationActions = [Collections.Generic.List[object]]::new()
				foreach ($action in $actions) {
					Assert-ExactProperties -Value $action -Expected @(
						'actionId','clipName','loop','durationMs','waitAfterMs',
						'weight','playbackRate','blendSeconds') `
						-Context "$relativePath NPC action"
					Assert-StableId $action.actionId "$relativePath NPC actionId"
					Assert-JsonString $action.clipName "$relativePath NPC action clipName"
					if (-not $actionIds.Add([string]$action.actionId) -or
						[string]$action.actionId -in @('npc.idle','npc.move.walk') -or
						[string]$action.clipName -notmatch '^[A-Za-z0-9_~.-]{1,64}$' -or
						$action.loop -isnot [bool]) {
						throw "NPC action identity or clip is invalid: $($placement.placementId)"
					}
					Assert-JsonInteger $action.durationMs "$relativePath NPC action durationMs" 1 600000
					Assert-JsonInteger $action.waitAfterMs "$relativePath NPC action waitAfterMs" 0 600000
					Assert-JsonInteger $action.weight "$relativePath NPC action weight" 1 100000
					Assert-JsonNumber $action.playbackRate "$relativePath NPC action playbackRate"
					Assert-JsonNumber $action.blendSeconds "$relativePath NPC action blendSeconds"
					$playbackRate = [double]$action.playbackRate
					$blendSeconds = [double]$action.blendSeconds
					if ($playbackRate -lt 0.1 -or $playbackRate -gt 4.0 -or
						$blendSeconds -lt 0.0 -or $blendSeconds -gt 2.0) {
						throw "NPC action presentation range is invalid: $($action.actionId)"
					}
					foreach ($field in @(
						[string]$action.actionId,
						[string][uint32]$action.durationMs,
						[string][uint32]$action.waitAfterMs,
						[string][uint32]$action.weight)) {
						$actionFields.Add($field)
					}
					$presentationActions.Add([ordered]@{
						actionId = [string]$action.actionId
						clipName = [string]$action.clipName
						loop = [bool]$action.loop
						playbackRate = $playbackRate
						blendSeconds = $blendSeconds
					})
				}

				$presentationEntry.walkClip = $walkClip
				$presentationEntry.actions = @($presentationActions)
				$behaviorFields = @(
					'1', [string]$behavior.mode, [string]$behavior.routeMode,
					[string]$behavior.actionSelection,
					(Format-InvariantFloat $moveSpeed),
					(Format-InvariantFloat $wanderRadius),
					[string][uint32]$behavior.randomSeed,
					[string][uint32]$behavior.startDelayMs,
					[string][uint32]$behavior.idleMinMs,
					[string][uint32]$behavior.idleMaxMs,
					$lookTarget,
					[string]$waypoints.Count) +
					@($waypointFields) +
					@([string]$actions.Count) +
					@($actionFields)
			}
			if ($null -ne $placement.idleClip -or $null -ne $placement.behavior) {
				$npcPresentationEntries.Add($presentationEntry)
			}
			$commonFields += $behaviorFields
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
            if (-not $profile.BossArchetypeIds.Contains([string]$placement.archetypeId)) {
                throw "Boss placement archetype does not match encounter '$encounterId'."
            }
        }
        $serializedArchetypeId = if ($placement.kind -eq 'playerSpawn') { '-' } else { [string]$placement.archetypeId }
		$commonFields[2] = $serializedArchetypeId
		$commonFields[3] = $serializedEncounterId
		$rowFields = $commonFields
		$rows.Add(($rowFields -join "`t"))
		if ($placement.enabled -and $placement.kind -in @('npc','boss')) {
			++$enabledWorldEntityCount
		}
    }
	if ($enabledWorldEntityCount -gt 256) {
		throw "World enabled persistent entity count exceeds snapshot capacity 256: $enabledWorldEntityCount"
	}
	if ($WorldId -eq 'VALTAN_ARENA' -and
		$enabledPlayerSpawnCount -ne $valtanRaidPlayerCapacity) {
		throw "Valtan Arena requires exactly $valtanRaidPlayerCapacity enabled player spawns; got $enabledPlayerSpawnCount."
	}

    $sortedRows = @($rows | Sort-Object)
    $lines = [Collections.Generic.List[string]]::new()
	$lines.Add("LOSTARK_WORLD_BOOTSTRAP`t7`t$WorldId`t$AreaId`t$($document.revision)`t$($sortedRows.Count)")
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

# The stele slots are gameplay cover, but their position is already owned by
# the Deploy placement the Client renders. Storing it twice is how the two
# silently drift, so the document names the placement and the position is read
# from that one row here.
function Get-DeployPlacementXZ {
    param([string]$AreaId, [string]$PlacementId, [string]$ExpectedAssetId)
    $relativePath = "Data/Maps/Authoring/$AreaId/$AreaId.deployplacements"
    $absolutePath = [IO.Path]::GetFullPath((Join-Path $repoRoot $relativePath))
    if (-not [IO.File]::Exists($absolutePath)) {
        throw "Encounter prop slot has no Deploy placement document: $relativePath"
    }
    foreach ($line in [IO.File]::ReadAllLines($absolutePath)) {
        $fields = @($line -split '\s+' | Where-Object { $_.Length -gt 0 })
        if ($fields.Count -lt 8 -or $fields[0] -cne $PlacementId) { continue }
        $assetId = $fields[4].Trim('"')
        if ($assetId -cne $ExpectedAssetId) {
            throw "Encounter prop placement $PlacementId is $assetId, not $ExpectedAssetId."
        }
        $x = 0.0
        $z = 0.0
        $style = [Globalization.NumberStyles]::Float
        $culture = [Globalization.CultureInfo]::InvariantCulture
        if (-not [double]::TryParse($fields[5], $style, $culture, [ref]$x) -or
            -not [double]::TryParse($fields[7], $style, $culture, [ref]$z)) {
            throw "Encounter prop placement $PlacementId has an unreadable position."
        }
        return [ordered]@{ X = $x; Z = $z }
    }
    throw "Encounter prop slot names a Deploy placement that does not exist: $PlacementId"
}

function Convert-EncounterPropsDocument {
    param([string]$AreaId, [string]$WorldId)
    $relativePath = "Data/Worlds/$AreaId/EncounterProps.world.json"
    $absolutePath = [IO.Path]::GetFullPath((Join-Path $repoRoot $relativePath))
    if (-not [IO.File]::Exists($absolutePath)) {
        return [ordered]@{
            WorldId = $WorldId; AreaId = $AreaId
            Lines = [Collections.Generic.List[string]]::new()
            SetCount = 0; SlotCount = 0; IsPresent = $false
        }
    }
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @(
        'schema','formatVersion','areaId','revision','propSets') $relativePath
    if ($document.schema -cne 'lostark.world-encounter-props' -or
        $document.formatVersion -ne 1 -or $document.areaId -cne $AreaId) {
        throw "Encounter prop header is invalid: $relativePath"
    }
    Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if ($document.propSets -isnot [Array] -or
        @($document.propSets).Count -eq 0 -or
        @($document.propSets).Count -gt 8) {
        throw "Encounter prop document exceeds its limits: $relativePath"
    }
    $setRows = [Collections.Generic.List[string]]::new()
    $slotRows = [Collections.Generic.List[string]]::new()
    $setIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $slotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $culture = [Globalization.CultureInfo]::InvariantCulture
    foreach ($propSet in @($document.propSets)) {
        Assert-ExactProperties $propSet @(
            'propSetId','encounterId','deployAssetId','coverRadiusMeters',
            'slots') "$relativePath propSet"
        foreach ($field in @('propSetId','encounterId','deployAssetId')) {
            Assert-StableId ([string]$propSet.$field) "$relativePath propSet $field"
        }
        if (-not $setIds.Add([string]$propSet.propSetId)) {
            throw "Encounter prop set is duplicated: $($propSet.propSetId)"
        }
        $coverRadius = [double]$propSet.coverRadiusMeters
        if ([double]::IsNaN($coverRadius) -or [double]::IsInfinity($coverRadius) -or
            $coverRadius -le 0.0 -or $coverRadius -gt 8.0) {
            throw "Encounter prop cover radius is invalid: $($propSet.propSetId)"
        }
        if ($propSet.slots -isnot [Array] -or
            @($propSet.slots).Count -eq 0 -or
            @($propSet.slots).Count -gt 16) {
            throw "Encounter prop set slot count is invalid: $($propSet.propSetId)"
        }
        $setRows.Add((@(
            'PROPSET', $propSet.propSetId, $propSet.encounterId,
            $propSet.deployAssetId,
            $coverRadius.ToString('0.######', $culture)) -join "`t"))
        $previousSlotId = ''
        foreach ($slot in @($propSet.slots)) {
            Assert-ExactProperties $slot @('slotId','deployPlacementId') "$relativePath slot"
            Assert-StableId ([string]$slot.slotId) "$relativePath slotId"
            $placementId = [string]$slot.deployPlacementId
            if ($placementId -cnotmatch '^[0-9]{1,20}$') {
                throw "Encounter prop slot placement id is invalid: $placementId"
            }
            if (-not $slotIds.Add([string]$slot.slotId)) {
                throw "Encounter prop slot is duplicated: $($slot.slotId)"
            }
            if ($previousSlotId.Length -ne 0 -and
                [StringComparer]::Ordinal.Compare(
                    $previousSlotId, [string]$slot.slotId) -ge 0) {
                throw "Encounter prop slots are not ordinal: $($slot.slotId)"
            }
            $previousSlotId = [string]$slot.slotId
            $position = Get-DeployPlacementXZ -AreaId $AreaId `
                -PlacementId $placementId -ExpectedAssetId $propSet.deployAssetId
            $slotRows.Add((@(
                'PROPSLOT', $propSet.propSetId, $slot.slotId,
                ([double]$position.X).ToString('0.######', $culture),
                ([double]$position.Z).ToString('0.######', $culture)) -join "`t"))
        }
    }
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_ENCOUNTER_PROP_BOOTSTRAP`t1`t$WorldId`t$AreaId`t$($document.revision)`t$($setRows.Count)`t$($slotRows.Count)")
    foreach ($row in $setRows) { $lines.Add($row) }
    foreach ($row in $slotRows) { $lines.Add($row) }
    return [ordered]@{
        WorldId = $WorldId; AreaId = $AreaId; Lines = $lines
        SetCount = $setRows.Count; SlotCount = $slotRows.Count; IsPresent = $true
    }
}

function Convert-KakulStageMarkersDocument {
    $areaId = 'LV_LUT_MIDNIGHTC_ED'
    $worldId = 'KAKULSAYDON_ARENA'
    $relativePath = "Data/Worlds/$areaId/StageMarkers.json"
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @(
        'schema','formatVersion','areaId','revision','semanticStatus','stages') `
        'Kakul StageMarkers root'
    Assert-JsonInteger $document.formatVersion 'Kakul StageMarkers formatVersion' 1 1
    Assert-JsonInteger $document.revision 'Kakul StageMarkers revision' 1 2147483647
    if ($document.schema -ne 'lostark.kakul-stage-markers' -or
        $document.areaId -ne $areaId -or
        $document.semanticStatus -ne 'SOURCE_LEVEL_ID_ONLY') {
        throw 'Kakul StageMarkers header is invalid.'
    }

    $gameplay = Read-ProjectJson "Data/Worlds/$areaId/Gameplay.world.json"
    $placements = @{}
    foreach ($placement in @($gameplay.placements)) {
        $placements[[string]$placement.placementId] = $placement
    }

    $stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $placementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $runtimeStages = [Collections.Generic.List[object]]::new()
    foreach ($stage in @($document.stages)) {
        Assert-ExactProperties $stage @(
            'stageId','placementId','displayNameKo','sourceLevelId',
            'componentCellCount','evidence') 'Kakul StageMarkers stage'
        Assert-StableId $stage.stageId 'Kakul stageId'
        Assert-StableId $stage.placementId 'Kakul stage placementId'
        Assert-StableId $stage.sourceLevelId 'Kakul sourceLevelId'
        Assert-JsonInteger $stage.componentCellCount `
            "Kakul stage $($stage.stageId) componentCellCount" 1 2147483647
        Assert-JsonString $stage.displayNameKo `
            "Kakul stage $($stage.stageId) displayNameKo"
        Assert-JsonString $stage.evidence `
            "Kakul stage $($stage.stageId) evidence"
        if (-not ([string]$stage.stageId).StartsWith('stage.kakul.', [StringComparison]::Ordinal) -or
            $stage.stageId -ne $stage.placementId -or
            [string]::IsNullOrWhiteSpace([string]$stage.displayNameKo) -or
            [string]::IsNullOrWhiteSpace([string]$stage.evidence) -or
            -not $stageIds.Add([string]$stage.stageId) -or
            -not $placementIds.Add([string]$stage.placementId)) {
            throw "Kakul stage identity is invalid or duplicated: $($stage.stageId)"
        }
        if (-not $placements.ContainsKey([string]$stage.placementId)) {
            throw "Kakul stage placement is missing from Gameplay.world.json: $($stage.placementId)"
        }
        $placement = $placements[[string]$stage.placementId]
        if ($placement.kind -ne 'playerSpawn' -or [bool]$placement.enabled) {
            throw "Kakul stage placement must be a disabled playerSpawn: $($stage.placementId)"
        }
        $runtimeStages.Add([ordered]@{
            stageId = [string]$stage.stageId
            placementId = [string]$stage.placementId
            displayNameKo = [string]$stage.displayNameKo
            sourceLevelId = [string]$stage.sourceLevelId
        })
    }
    if ($runtimeStages.Count -eq 0 -or $runtimeStages.Count -gt 64) {
        throw "Kakul StageMarkers count is invalid: $($runtimeStages.Count)"
    }

    $runtime = [ordered]@{
        schema = 'lostark.kakul-stage-markers-runtime'
        formatVersion = 1
        worldId = $worldId
        areaId = $areaId
        revision = [int]$document.revision
        semanticStatus = 'SOURCE_LEVEL_ID_ONLY'
        stages = @($runtimeStages)
    }
    return [pscustomobject]@{
        WorldId = $worldId
        AreaId = $areaId
        Count = $runtimeStages.Count
        Json = ($runtime | ConvertTo-Json -Depth 6)
    }
}

$actorIds = Get-ActorIds
$encounterProfiles = Get-EncounterProfiles
$monsterProfiles = Get-MonsterProfiles
$kakulStageMarkers = Convert-KakulStageMarkersDocument
$spawnDocuments = @(
    (Convert-SpawnGroupsDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles)
)
$spawnByWorld = @{}
foreach ($spawn in $spawnDocuments) { $spawnByWorld[$spawn.WorldId] = $spawn }
$encounterPropDocuments = @(
    (Convert-EncounterPropsDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN'),
    (Convert-EncounterPropsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA'),
    (Convert-EncounterPropsDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA'),
    (Convert-EncounterPropsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND'),
    (Convert-EncounterPropsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA')
)
$worlds = @(
    (Convert-WorldDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.BERN.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.VALTAN_ARENA.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LUT_MIDNIGHTC_ED' -WorldId 'KAKULSAYDON_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.KAKULSAYDON_ARENA.GroupIds),
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
		foreach ($props in @($encounterPropDocuments | Where-Object IsPresent)) {
			$staged = Join-Path $stagingRoot "$($props.WorldId).encounterpropsbootstrap"
			[IO.File]::WriteAllLines(
				$staged,
				$props.Lines,
				[Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $props
				Staged = $staged
				Destination = Join-Path $resolvedOutputRoot "$($props.WorldId).encounterpropsbootstrap"
				Rollback = Join-Path $resolvedOutputRoot ".$($props.WorldId).encounterprops.rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}
		$clientWorldRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Client/Bin/DataFiles/World'))
		[IO.Directory]::CreateDirectory($clientWorldRoot) | Out-Null
		$stagedKakulMarkers = Join-Path $stagingRoot 'KAKULSAYDON_ARENA.stagemarkers.json'
		[IO.File]::WriteAllText(
			$stagedKakulMarkers,
			$kakulStageMarkers.Json,
			[Text.UTF8Encoding]::new($false))
		$promotions.Add([ordered]@{
			World = $kakulStageMarkers
			Staged = $stagedKakulMarkers
			Destination = Join-Path $clientWorldRoot 'KAKULSAYDON_ARENA.stagemarkers.json'
			Rollback = Join-Path $clientWorldRoot ".KAKULSAYDON_ARENA.stagemarkers.rollback.$transactionId"
			HadPrevious = $false
			Promoted = $false
		})
		foreach ($world in $worlds) {
			$staged = Join-Path $stagingRoot "$($world.WorldId).npcpresentation.json"
			$jsonLines = [Collections.Generic.List[string]]::new()
			$jsonLines.Add('{')
			$jsonLines.Add('  "schema": "lostark.npc-placement-presentation",')
			$jsonLines.Add('  "formatVersion": 2,')
			$jsonLines.Add("  `"worldId`": `"$($world.WorldId)`",")
			$entries = @($world.NpcPresentation | Sort-Object { [string]$_.placementId })
			if ($entries.Count -eq 0) {
				$jsonLines.Add('  "entries": []')
			}
			else {
				$jsonLines.Add('  "entries": [')
				for ($entryIndex = 0; $entryIndex -lt $entries.Count; $entryIndex++) {
					$entry = $entries[$entryIndex]
					$jsonLines.Add('    {')
					$jsonLines.Add("      `"placementId`": `"$($entry.placementId)`",")
					if ($null -eq $entry.idleClip) {
						$jsonLines.Add('      "idleClip": null,')
					}
					else {
						$jsonLines.Add("      `"idleClip`": `"$($entry.idleClip)`",")
					}
					if ($null -eq $entry.walkClip) {
						$jsonLines.Add('      "walkClip": null,')
					}
					else {
						$jsonLines.Add("      `"walkClip`": `"$($entry.walkClip)`",")
					}
					$actions = @($entry.actions)
					if ($actions.Count -eq 0) {
						$jsonLines.Add('      "actions": []')
					}
					else {
						$jsonLines.Add('      "actions": [')
						for ($actionIndex = 0; $actionIndex -lt $actions.Count; $actionIndex++) {
							$action = $actions[$actionIndex]
							$actionSuffix = if ($actionIndex -lt $actions.Count - 1) { ',' } else { '' }
							$loopText = if ($action.loop) { 'true' } else { 'false' }
							$playbackText = Format-InvariantFloat $action.playbackRate
							$blendText = Format-InvariantFloat $action.blendSeconds
							$jsonLines.Add("        { `"actionId`": `"$($action.actionId)`", `"clipName`": `"$($action.clipName)`", `"loop`": $loopText, `"playbackRate`": $playbackText, `"blendSeconds`": $blendText }$actionSuffix")
						}
						$jsonLines.Add('      ]')
					}
					$entrySuffix = if ($entryIndex -lt $entries.Count - 1) { ',' } else { '' }
					$jsonLines.Add("    }$entrySuffix")
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
			Write-Output "Published $($promotion.World.WorldId): $($promotion.World.Count) placements -> $($promotion.Destination)"
		}
		# Every destination is committed at this point. Backup cleanup must never
		# enter the rollback catch after an earlier backup was already deleted,
		# because that could remove both the committed output and its old copy.
		foreach ($promotion in $promotions) {
			try {
				if ([IO.File]::Exists($promotion.Rollback)) {
					[IO.File]::Delete($promotion.Rollback)
				}
			}
			catch {
				Write-Warning "Published output is committed, but rollback backup cleanup was deferred: $($promotion.Rollback): $($_.Exception.Message)"
			}
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
	foreach ($props in @($encounterPropDocuments | Where-Object IsPresent)) {
		Write-Output "Validated $($props.WorldId): $($props.SetCount) prop sets, $($props.SlotCount) slots"
	}
}
