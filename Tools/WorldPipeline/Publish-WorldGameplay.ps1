[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/World',
	[ValidateRange(0, 4)]
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
    return Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
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

    $idsByKind = @{
        playerSpawn = @($characterCatalog.characters |
            Where-Object runtimeStatus -eq 'supported' |
            ForEach-Object archetypeId)
        boss = @($bossCatalog.bosses | ForEach-Object archetypeId)
        npc = @($npcCatalog.npcs | ForEach-Object archetypeId)
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
            'authority','fixedTickHz','states','patterns') 'encounter profile'
		Assert-JsonInteger $document.formatVersion "$($document.encounterId) formatVersion" 1 1
		Assert-JsonInteger $document.fixedTickHz "$($document.encounterId) fixedTickHz" 30 30
        Assert-StableId $document.encounterId 'encounterId'
        Assert-StableId $document.bossArchetypeId 'bossArchetypeId'
        if ($document.schema -ne 'lostark.encounter-profile' -or
            $document.formatVersion -ne 1 -or
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
        if ($patterns.Count -ne 1) {
            throw "Bootstrap v2 requires exactly one entry pattern: $($document.encounterId)"
        }
        foreach ($pattern in $patterns) {
            Assert-ExactProperties $pattern @(
                'patternId','actionId','minimumRange','maximumRange',
                'telegraphMs','activeMs','recoveryMs','serverDamageProfileId') "$($document.encounterId) pattern"
			Assert-JsonNumber $pattern.minimumRange "$($document.encounterId) minimumRange"
			Assert-JsonNumber $pattern.maximumRange "$($document.encounterId) maximumRange"
			Assert-JsonInteger $pattern.telegraphMs "$($document.encounterId) telegraphMs" 1 ([uint32]::MaxValue)
			Assert-JsonInteger $pattern.activeMs "$($document.encounterId) activeMs" 1 ([uint32]::MaxValue)
			Assert-JsonInteger $pattern.recoveryMs "$($document.encounterId) recoveryMs" 1 ([uint32]::MaxValue)
            Assert-StableId $pattern.patternId "$($document.encounterId) patternId"
            Assert-StableId $pattern.actionId "$($document.encounterId) actionId"
            Assert-StableId $pattern.serverDamageProfileId "$($document.encounterId) damage profile"
            if ([double]$pattern.minimumRange -lt 0.0 -or
                [double]$pattern.maximumRange -le [double]$pattern.minimumRange -or
                [uint32]$pattern.telegraphMs -eq 0 -or
                [uint32]$pattern.activeMs -eq 0 -or
                [uint32]$pattern.recoveryMs -eq 0) {
                throw "Encounter pattern timing or range is invalid: $($pattern.patternId)"
            }
        }
        $profiles[[string]$document.encounterId] = $document
    }
    return $profiles
}

function Convert-WorldDocument {
    param(
        [string]$AreaId,
        [string]$WorldId,
        [hashtable]$ActorIds,
        [hashtable]$EncounterProfiles
    )

    $relativePath = "Data/Worlds/$AreaId/Gameplay.world.json"
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @('schema','formatVersion','areaId','revision','placements') $relativePath
	Assert-JsonString $document.schema "$relativePath schema"
	Assert-JsonInteger $document.formatVersion "$relativePath formatVersion" 2 2
	Assert-JsonString $document.areaId "$relativePath areaId"
	Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if ($document.schema -ne 'lostark.world-gameplay' -or
        $document.formatVersion -ne 2 -or
        $document.areaId -ne $AreaId -or
        $document.revision -lt 1) {
        throw "World gameplay header is invalid: $relativePath"
    }

    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
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
		if ($placement.kind -notin @('playerSpawn','npc','boss','triggerBox')) {
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
				throw "movePlayer trigger requires exactly one event when enabled: $($placement.placementId)"
			}
			$triggerFields = @(
				(Format-InvariantFloat $placement.halfExtents[0]),
				(Format-InvariantFloat $placement.halfExtents[1]),
				(Format-InvariantFloat $placement.halfExtents[2]),
				$(if ($placement.triggerOnce) { '1' } else { '0' }),
				[string]$events.Count)
			foreach ($event in $events) {
				Assert-ExactProperties $event @(
					'type','targetPosition','durationSeconds','arcHeight') "$relativePath movePlayer event"
				if ($event.type -ne 'movePlayer' -or @($event.targetPosition).Count -ne 3) {
					throw "Only movePlayer trigger events are admitted: $($placement.placementId)"
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
					'movePlayer',
					(Format-InvariantFloat $event.targetPosition[0]),
					(Format-InvariantFloat $event.targetPosition[1]),
					(Format-InvariantFloat $event.targetPosition[2]),
					(Format-InvariantFloat $duration),
					(Format-InvariantFloat $arcHeight))
			}
			$rows.Add((($commonFields + $triggerFields) -join "`t"))
			continue
		}

		Assert-ExactProperties $placement @(
			'placementId','kind','archetypeId','encounterId',
			'position','yawDegrees','enabled') "$relativePath placement"
		Assert-JsonString $placement.archetypeId "$relativePath archetypeId" -AllowNull
		Assert-JsonString $placement.encounterId "$relativePath encounterId" -AllowNull
		if ($placement.kind -eq 'playerSpawn') {
			if ($null -ne $placement.archetypeId -and
				-not [string]::IsNullOrEmpty([string]$placement.archetypeId)) {
				throw "Player spawn must not own a character archetype: $($placement.placementId)"
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
        $patternFields = @('-', '-', '0', '0', '0', '0', '0', '-')
        if ($placement.kind -eq 'boss') {
            if (-not $encounterId) {
                throw "Boss placement requires an encounter ID: $($placement.placementId)"
            }
            $profile = $EncounterProfiles[$encounterId]
            if ($profile.bossArchetypeId -ne $placement.archetypeId) {
                throw "Boss placement archetype does not match encounter '$encounterId'."
            }
            $pattern = @($profile.patterns)[0]
            $patternFields = @(
                [string]$pattern.patternId,
                [string]$pattern.actionId,
                (Format-InvariantFloat $pattern.minimumRange),
                (Format-InvariantFloat $pattern.maximumRange),
                [string][uint32]$pattern.telegraphMs,
                [string][uint32]$pattern.activeMs,
                [string][uint32]$pattern.recoveryMs,
                [string]$pattern.serverDamageProfileId)
        }
        $serializedArchetypeId = if ($placement.kind -eq 'playerSpawn') { '-' } else { [string]$placement.archetypeId }
		$commonFields[2] = $serializedArchetypeId
		$commonFields[3] = $serializedEncounterId
		$rowFields = $commonFields + $patternFields
        $rows.Add(($rowFields -join "`t"))
    }

    $sortedRows = @($rows | Sort-Object)
    $lines = [Collections.Generic.List[string]]::new()
	$lines.Add("LOSTARK_WORLD_BOOTSTRAP`t3`t$WorldId`t$AreaId`t$($document.revision)`t$($sortedRows.Count)")
    foreach ($row in $sortedRows) {
        $lines.Add($row)
    }
    return [ordered]@{
        WorldId = $WorldId
        AreaId = $AreaId
        Lines = $lines
        Count = $sortedRows.Count
    }
}

$actorIds = Get-ActorIds
$encounterProfiles = Get-EncounterProfiles
$worlds = @(
    (Convert-WorldDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -EncounterProfiles $encounterProfiles),
    (Convert-WorldDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles),
    (Convert-WorldDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -EncounterProfiles $encounterProfiles),
    (Convert-WorldDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles)
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
}
