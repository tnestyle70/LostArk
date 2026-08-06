[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Gameplay'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing gameplay document: $RelativePath" }
	return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId([string]$Value, [string]$Context) {
    if ($Value -notmatch $stableIdPattern) { throw "$Context is not a stable ID: '$Value'" }
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

function Assert-JsonString([object]$Value, [string]$Context) {
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

function ConvertTo-ReceiptValue([object]$Value) {
    if ($null -eq $Value) { return 'null' }
    return ($Value | ConvertTo-Json -Compress -Depth 32)
}

function Get-CanonicalTextFileSha256([string]$Path) {
    $strictUtf8 = [Text.UTF8Encoding]::new($false, $true)
    $text = [IO.File]::ReadAllText($Path, $strictUtf8)
    $canonicalText = $text.Replace("`r`n", "`n").Replace("`r", "`n")
    $canonicalBytes = [Text.UTF8Encoding]::new($false).GetBytes($canonicalText)
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString($sha256.ComputeHash($canonicalBytes))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha256.Dispose()
    }
}

function Assert-BalanceProvenance(
    [object]$PlayerDocument,
    [object]$SkillDocument,
    [object]$DamageDocument,
    [object]$BossDocument) {
    $receipt = Read-JsonDocument 'Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json'
    Assert-ExactProperties $receipt @(
        'schema','formatVersion','sourceBuildId','referenceSkillLevel',
        'extractorSha256','sourceFiles','coverage','entries') 'balance provenance receipt'
    if ($receipt.schema -ne 'lostark.balance-provenance-receipt' -or
        [uint32]$receipt.formatVersion -ne 1 -or
        [uint32]$receipt.referenceSkillLevel -ne 10 -or
        [string]$receipt.sourceBuildId -notmatch '^data2-[0-9a-f]{16}_data3-[0-9a-f]{16}$' -or
        [string]$receipt.extractorSha256 -notmatch '^[0-9a-f]{64}$') {
        throw 'Balance provenance receipt header is invalid.'
    }
    $extractorPath = Join-Path $repoRoot 'Tools\GameplayPipeline\Export-OfficialBalanceReceipt.py'
    $currentExtractorHash = Get-CanonicalTextFileSha256 $extractorPath
    if ([string]$receipt.extractorSha256 -cne $currentExtractorHash) {
        throw 'Balance provenance receipt is stale for the current extractor.'
    }
    Assert-ExactProperties $receipt.coverage @(
        'playerProfileCount','skillDefinitionCount','damageProfileCount',
        'bossProfileCount','encounterPatternCount','fieldEntryCount') 'balance provenance coverage'
    $sourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$sourceHashes = @{}
    foreach ($sourceFile in @($receipt.sourceFiles)) {
        Assert-ExactProperties $sourceFile @('id','sha256') 'balance provenance source file'
        if (-not $sourceIds.Add([string]$sourceFile.id) -or
            [string]$sourceFile.sha256 -notmatch '^[0-9a-f]{64}$') {
            throw "Balance provenance source file is invalid: $($sourceFile.id)"
        }
		$sourceHashes[[string]$sourceFile.id] = [string]$sourceFile.sha256
    }
    foreach ($requiredSource in @(
        'data2.lpk','data3.lpk','EFTable_PC.db','EFTable_Skill.db',
        'EFTable_SkillEffect.db','EFTable_Npc.db','EFTable_NpcBalance.db',
        'EFTable_NpcStat.db','MN_RPBF_01-1.loa')) {
        if (-not $sourceIds.Contains($requiredSource)) {
            throw "Balance provenance source file is missing: $requiredSource"
        }
    }
    $expectedBuildId = "data2-$($sourceHashes['data2.lpk'].Substring(0, 16))_data3-$($sourceHashes['data3.lpk'].Substring(0, 16))"
    if ([string]$receipt.sourceBuildId -cne $expectedBuildId) {
        throw 'Balance provenance sourceBuildId does not match the recorded LPK hashes.'
    }

    $allowedBases = @(
        'OFFICIAL_EXTRACTED','OFFICIAL_DERIVED','OFFICIAL_SCALED',
        'PROJECT_TUNED','REFERENCE_ONLY')
    $receiptValues = @{}
    foreach ($entry in @($receipt.entries)) {
        $actualFields = @($entry.PSObject.Properties.Name | Where-Object { $_ -ne 'note' } | Sort-Object)
        $expectedFields = @(
            'targetDocument','targetId','targetField','basis','source',
            'sourceValue','transform','resultValue' | Sort-Object)
        if (($actualFields -join "`n") -ne ($expectedFields -join "`n") -or
            [string]$entry.basis -notin $allowedBases -or
            [string]::IsNullOrWhiteSpace([string]$entry.targetDocument) -or
            [string]::IsNullOrWhiteSpace([string]$entry.targetId) -or
            [string]::IsNullOrWhiteSpace([string]$entry.targetField) -or
            [string]::IsNullOrWhiteSpace([string]$entry.transform) -or
            $null -eq $entry.source) {
            throw "Balance provenance field entry is invalid: $($entry.targetId).$($entry.targetField)"
        }
        $key = "$($entry.targetDocument)#$($entry.targetId).$($entry.targetField)"
        if ($receiptValues.ContainsKey($key)) {
            throw "Duplicate balance provenance field entry: $key"
        }
		$sourceType = [string]$entry.source.type
		if ($sourceType -eq 'official-table-cell') {
			Assert-ExactProperties $entry.source @(
				'type','database','databaseSha256','table','primaryKey',
				'secondaryKey','column') "balance provenance official source $key"
			$expectedDatabase = "EFTable_$($entry.source.table).db"
			if ([string]$entry.source.database -cne $expectedDatabase -or
				-not $sourceHashes.ContainsKey($expectedDatabase) -or
				[string]$entry.source.databaseSha256 -cne $sourceHashes[$expectedDatabase] -or
				[string]::IsNullOrWhiteSpace([string]$entry.source.column)) {
				throw "Balance provenance official source hash/table is invalid: $key"
			}
		}
		elseif ($sourceType -eq 'project-policy') {
			Assert-ExactProperties $entry.source @('type','policyId') `
				"balance provenance project source $key"
			if ([string]$entry.source.policyId -notmatch '^[A-Za-z0-9_.-]{1,128}$') {
				throw "Balance provenance project policy is invalid: $key"
			}
		}
		else {
			throw "Balance provenance source type is unknown: $key"
		}
		if ([string]$entry.basis -in @(
			'OFFICIAL_EXTRACTED','OFFICIAL_DERIVED','OFFICIAL_SCALED') -and
			$sourceType -ne 'official-table-cell') {
			throw "Official provenance basis requires an official table cell: $key"
		}
		if ([string]$entry.basis -eq 'OFFICIAL_EXTRACTED' -and
			([string]$entry.transform -cne 'identity' -or
				(ConvertTo-ReceiptValue $entry.sourceValue) -cne
				(ConvertTo-ReceiptValue $entry.resultValue))) {
			throw "OFFICIAL_EXTRACTED provenance must be an identity transform: $key"
		}
		if ([string]$entry.basis -eq 'PROJECT_TUNED' -and
			$sourceType -eq 'official-table-cell' -and
			[string]$entry.transform -notmatch '(?i)project') {
			throw "PROJECT_TUNED official comparison needs an explicit project transform: $key"
		}
        $receiptValues[$key] = ConvertTo-ReceiptValue $entry.resultValue
    }

    $expectations = @{}
    function Add-Expected([string]$Document, [string]$TargetId, [string]$Field, [object]$Value) {
        $key = "$Document#$TargetId.$Field"
        if ($expectations.ContainsKey($key)) { throw "Duplicate balance provenance expectation: $key" }
        $expectations[$key] = ConvertTo-ReceiptValue $Value
    }
    foreach ($player in @($PlayerDocument.players)) {
        foreach ($property in $player.PSObject.Properties) {
            Add-Expected 'Data/Balance/PlayerProfiles.json' "player:$($player.characterClass)" $property.Name $property.Value
        }
    }
    foreach ($skill in @($SkillDocument.skills)) {
        $targetId = "skill:$($skill.skillId)"
        foreach ($property in $skill.PSObject.Properties) {
            if ($property.Name -ne 'comboStages') {
                Add-Expected 'Data/Balance/PlayerSkills.json' $targetId $property.Name $property.Value
            }
        }
        $stages = @($skill.comboStages)
        Add-Expected 'Data/Balance/PlayerSkills.json' $targetId 'comboStages.length' $stages.Count
        for ($index = 0; $index -lt $stages.Count; $index++) {
            foreach ($property in $stages[$index].PSObject.Properties) {
                Add-Expected 'Data/Balance/PlayerSkills.json' $targetId "comboStages[$index].$($property.Name)" $property.Value
            }
        }
    }
    foreach ($profile in @($DamageDocument.profiles)) {
        foreach ($property in $profile.PSObject.Properties) {
            Add-Expected 'Data/Balance/DamageProfiles.json' "damage:$($profile.damageProfileId)" $property.Name $property.Value
        }
    }
    foreach ($boss in @($BossDocument.bosses)) {
        foreach ($property in $boss.PSObject.Properties) {
            Add-Expected 'Data/Balance/BossProfiles.json' "boss:$($boss.archetypeId)" $property.Name $property.Value
        }
    }
    $encounter = Read-JsonDocument 'Data/Encounters/Valtan/ValtanEncounter.json'
    $encounterTarget = "encounter:$($encounter.encounterId)"
    foreach ($field in @('encounterId','bossArchetypeId','authority','fixedTickHz')) {
        Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget $field $encounter.$field
    }
    Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget 'states.length' @($encounter.states).Count
    for ($index = 0; $index -lt @($encounter.states).Count; $index++) {
        foreach ($property in $encounter.states[$index].PSObject.Properties) {
            Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget "states[$index].$($property.Name)" $property.Value
        }
    }
    Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget 'patterns.length' @($encounter.patterns).Count
    for ($index = 0; $index -lt @($encounter.patterns).Count; $index++) {
        $pattern = $encounter.patterns[$index]
        foreach ($property in $pattern.PSObject.Properties) {
            Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' "pattern:$($pattern.patternId)" "patterns[$index].$($property.Name)" $property.Value
        }
    }
    if ($receiptValues.Count -ne $expectations.Count -or
        [uint32]$receipt.coverage.fieldEntryCount -ne $expectations.Count) {
        throw "Balance provenance coverage count mismatch. receipt=$($receiptValues.Count) expected=$($expectations.Count)"
    }
    foreach ($key in $expectations.Keys) {
        if (-not $receiptValues.ContainsKey($key) -or $receiptValues[$key] -cne $expectations[$key]) {
            throw "Balance provenance is missing or stale: $key"
        }
    }
    if ([uint32]$receipt.coverage.playerProfileCount -ne @($PlayerDocument.players).Count -or
        [uint32]$receipt.coverage.skillDefinitionCount -ne @($SkillDocument.skills).Count -or
        [uint32]$receipt.coverage.damageProfileCount -ne @($DamageDocument.profiles).Count -or
        [uint32]$receipt.coverage.bossProfileCount -ne @($BossDocument.bosses).Count -or
        [uint32]$receipt.coverage.encounterPatternCount -ne @($encounter.patterns).Count) {
        throw 'Balance provenance object coverage is stale.'
    }
}

# A damage profile stores the official Lost Ark damage rate as a percentage of
# the caster's attack power, not a finished number: EFTable_SkillEffect only ever
# supplies the rate, and the caster's attack power is what turns it into damage.
# The 34010 basic attack is rate 100, so 100 is exactly one attack power.
$maximumDamageRatePercent = 100000
$damageDocument = Read-JsonDocument 'Data/Balance/DamageProfiles.json'
Assert-ExactProperties $damageDocument @('schema','formatVersion','profiles') 'damage document'
Assert-JsonString $damageDocument.schema 'damage document schema'
Assert-JsonInteger $damageDocument.formatVersion 'damage document formatVersion' 2 2
if ($damageDocument.schema -ne 'lostark.damage-profiles' -or $damageDocument.formatVersion -ne 2) {
    throw 'Damage profile header is invalid.'
}
$damageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$damageRows = [Collections.Generic.List[string]]::new()
foreach ($profile in @($damageDocument.profiles)) {
    Assert-ExactProperties $profile @('damageProfileId','damageRatePercent') 'damage profile'
    Assert-JsonString $profile.damageProfileId 'damageProfileId'
    Assert-JsonInteger $profile.damageRatePercent 'damageRatePercent' 1 $maximumDamageRatePercent
    Assert-StableId $profile.damageProfileId 'damageProfileId'
    $ratePercent = [uint32]$profile.damageRatePercent
    if (-not $damageIds.Add([string]$profile.damageProfileId) -or $ratePercent -eq 0 -or
        $ratePercent -gt $maximumDamageRatePercent) {
        throw "Duplicate, zero, or out-of-range damage profile: $($profile.damageProfileId)"
    }
    $damageRows.Add("DAMAGE`t$($profile.damageProfileId)`t$ratePercent")
}

$skillDocument = Read-JsonDocument 'Data/Balance/PlayerSkills.json'
Assert-ExactProperties $skillDocument @('schema','formatVersion','skills') 'skill document'
Assert-JsonString $skillDocument.schema 'skill document schema'
Assert-JsonInteger $skillDocument.formatVersion 'skill document formatVersion' 2 2
if ($skillDocument.schema -ne 'lostark.player-skills' -or $skillDocument.formatVersion -ne 2) {
    throw 'Player skill header is invalid.'
}

$playerDocument = Read-JsonDocument 'Data/Balance/PlayerProfiles.json'
Assert-ExactProperties $playerDocument @('schema','formatVersion','players') 'player profile document'
Assert-JsonString $playerDocument.schema 'player profile schema'
Assert-JsonInteger $playerDocument.formatVersion 'player profile formatVersion' 2 2
if ($playerDocument.schema -ne 'lostark.player-profiles' -or $playerDocument.formatVersion -ne 2) {
	throw 'Player profile header is invalid.'
}
$playerClasses = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$playerRows = [Collections.Generic.List[string]]::new()
$supportedPlayerClasses = @(
	'LANCE_MASTER',
	'GUNSLINGER',
	'SLAYER',
	'ARTIST',
	'DIMENSIONMASTER',
	'WARLORD'
)
$knownStances = @('NONE', 'LANCE_MASTER_LONG_SPEAR', 'LANCE_MASTER_SHORT_SPEAR',
	'WARLORD_NORMAL', 'WARLORD_DEFENSE')
foreach ($player in @($playerDocument.players)) {
	Assert-ExactProperties $player @(
		'characterClass','maximumHp','maximumResource','resourceRegenPerSecond',
		'attackPower','defense','moveSpeed','defaultStance') 'player profile'
	Assert-JsonString $player.defaultStance 'player defaultStance'
	if ($player.defaultStance -notin $knownStances) {
		throw "Player defaultStance is unknown: $($player.characterClass) $($player.defaultStance)"
	}
	Assert-JsonString $player.characterClass 'player characterClass'
	Assert-JsonInteger $player.maximumHp 'player maximumHp' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $player.maximumResource 'player maximumResource' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $player.resourceRegenPerSecond 'player resourceRegenPerSecond' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $player.attackPower 'player attackPower' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $player.defense 'player defense' 1 ([uint32]::MaxValue)
	Assert-JsonNumber $player.moveSpeed 'player moveSpeed'
	Assert-StableId $player.characterClass 'player characterClass'
	# attackPower is the multiplicand every damage rate is applied to, so a zero
	# would silently disarm the class rather than fail loudly at load.
	if ($player.characterClass -notin $supportedPlayerClasses -or
		-not $playerClasses.Add([string]$player.characterClass) -or
		[uint32]$player.maximumHp -eq 0 -or [uint32]$player.maximumResource -eq 0 -or
		[uint32]$player.attackPower -eq 0 -or [uint32]$player.defense -eq 0 -or
		[uint32]$player.resourceRegenPerSecond -eq 0 -or
		[uint32]$player.resourceRegenPerSecond -gt [uint32]$player.maximumResource -or
		[double]$player.moveSpeed -le 0.0) {
		throw "Player profile is invalid: $($player.characterClass)"
	}
	$playerRows.Add((@(
		'PLAYER', $player.characterClass, [uint32]$player.maximumHp,
		[uint32]$player.maximumResource, [uint32]$player.resourceRegenPerSecond,
		[uint32]$player.attackPower, [uint32]$player.defense,
		(Format-InvariantFloat $player.moveSpeed 'player moveSpeed'),
		$player.defaultStance) -join "`t"))
}
if ($playerClasses.Count -ne $supportedPlayerClasses.Count) {
	$missingClasses = @($supportedPlayerClasses | Where-Object { -not $playerClasses.Contains($_) })
	throw "Player profiles are incomplete. missing=[$($missingClasses -join ',')]"
}
# Official CostMp at skill level 10 runs into the hundreds, so a skill's cost is
# bounded by the largest pool any class actually has rather than by a literal.
$maximumPlayerResource = (@($playerDocument.players) |
	ForEach-Object { [uint32]$_.maximumResource } | Measure-Object -Maximum).Maximum
# Quick-slot names a loadout may bind. Modifier combinations use an underscore
# (ALT_V), and the two mouse buttons are spelled out so the set stays a stable ID.
$playerSkillSlots = @(
	'Q','W','E','R','A','S','D','F','T','X','Z','V','ALT_V','SPACE','LMB','RMB')
$skillIds = [Collections.Generic.HashSet[uint32]]::new()
$claimedSlotStances = @{}
$skillRows = [Collections.Generic.List[string]]::new()
foreach ($skill in @($skillDocument.skills)) {
    Assert-ExactProperties $skill @(
		'skillId','characterClass','inputSlot','displayName','actionId','skillKind','requiredStance','setsStance',
		'cooldownMs','actionDurationMs',
        'hitTimeMs','resourceCost','movementDistance','maximumRange','serverDamageProfileId','effectId','comboStages') 'player skill'
	Assert-JsonInteger $skill.skillId 'skillId' 1 ([uint32]::MaxValue)
	foreach ($stringField in @('characterClass','inputSlot','displayName','actionId','skillKind','serverDamageProfileId',
		'effectId','requiredStance','setsStance')) {
		Assert-JsonString $skill.$stringField "skill $($skill.skillId) $stringField"
	}
	if ($skill.requiredStance -notin $knownStances -or $skill.setsStance -notin $knownStances) {
		throw "Skill stance is unknown: $($skill.skillId)"
	}
	foreach ($integerField in @('cooldownMs','actionDurationMs','hitTimeMs','resourceCost')) {
		Assert-JsonInteger $skill.$integerField "skill $($skill.skillId) $integerField" 0 ([uint32]::MaxValue)
	}
	Assert-JsonNumber $skill.movementDistance "skill $($skill.skillId) movementDistance"
	Assert-JsonNumber $skill.maximumRange "skill $($skill.skillId) maximumRange"
    Assert-StableId $skill.characterClass 'characterClass'
    Assert-StableId $skill.inputSlot 'inputSlot'
    Assert-StableId $skill.actionId 'actionId'
	# Empty means the skill never resolves a hit, so hitTimeMs and maximumRange,
	# which exist only to describe that hit, have to be zero as well.
	$damageProfileId = [string]$skill.serverDamageProfileId
	$dealsDamage = $damageProfileId.Length -gt 0
	if ($dealsDamage) {
		Assert-StableId $damageProfileId 'serverDamageProfileId'
	}
	if ([string]::IsNullOrWhiteSpace([string]$skill.displayName) -or ([string]$skill.displayName).Length -gt 64) {
		throw "Skill displayName is invalid: $($skill.skillId)"
	}
	# Client presentation reference. Empty means the skill has no authored effect
	# yet, which is the current state of every row; a non-empty value still has to
	# be a stable id so a file path or a display string cannot be stored here.
	$effectId = [string]$skill.effectId
	if ($effectId.Length -gt 0) {
		Assert-StableId $effectId 'effectId'
	}
    $id = [uint32]$skill.skillId
    $slotKey = "$($skill.characterClass):$($skill.inputSlot)"
    $claimedStances = $claimedSlotStances[$slotKey]
    if ($null -eq $claimedStances) {
        $claimedStances = [Collections.Generic.List[string]]::new()
        $claimedSlotStances[$slotKey] = $claimedStances
    }
    $slotConflicts = $claimedStances | Where-Object {
        $_ -eq $skill.requiredStance -or $_ -eq 'NONE' -or $skill.requiredStance -eq 'NONE'
    }
    if ($id -eq 0 -or -not $skillIds.Add($id) -or $null -ne $slotConflicts) {
        throw "Duplicate skill ID or input slot: $id"
    }
    $claimedStances.Add([string]$skill.requiredStance)
    if ($skill.characterClass -notin $supportedPlayerClasses -or $skill.inputSlot -notin $playerSkillSlots -or
        [uint32]$skill.actionDurationMs -eq 0 -or
        [uint32]$skill.hitTimeMs -gt [uint32]$skill.actionDurationMs -or
        [uint32]$skill.resourceCost -gt $maximumPlayerResource -or
		($dealsDamage -and ([double]$skill.maximumRange -le 0.0 -or
			-not $damageIds.Contains($damageProfileId))) -or
		(-not $dealsDamage -and ([double]$skill.maximumRange -ne 0.0 -or
			[uint32]$skill.hitTimeMs -ne 0))) {
        throw "Player skill timing, class, slot, resource, or damage reference is invalid: $id"
    }
	$skillKind = [string]$skill.skillKind
	if ($skillKind -notin @('ACTIVE','COMBO','HOLD')) {
		throw "Unknown skillKind: $id $skillKind"
	}
	$stages = @($skill.comboStages)
	if ($skillKind -eq 'HOLD') {
		if ($stages.Count -ne 3) {
			throw "HOLD skill needs exactly 3 stages: $id"
		}
		if ([uint32]$skill.cooldownMs -eq 0) {
			throw "HOLD skill needs a cooldown: $id"
		}
		$holdTotal = 0
		for ($stageIndex = 0; $stageIndex -lt 3; $stageIndex++) {
			$stage = $stages[$stageIndex]
			Assert-ExactProperties $stage @(
				'actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs') 'hold stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.actionDurationMs -or
				[uint32]$stage.inputOpenMs -ne 0 -or [uint32]$stage.inputCloseMs -ne 0) {
				throw "Hold stage timing is invalid: $id stage $stageIndex"
			}
			if (($stageIndex -ne 2) -ne ([uint32]$stage.hitTimeMs -eq 0)) {
				throw "A hold skill lands its damage in the end stage only: $id"
			}
			$holdTotal += [uint32]$stage.actionDurationMs
		}
		if ($holdTotal -ne [uint32]$skill.actionDurationMs) {
			throw "Hold stage durations must sum to actionDurationMs: $id"
		}
	}
	elseif ($skillKind -eq 'ACTIVE') {
		if ($stages.Count -ne 0) {
			throw "ACTIVE skill must not carry comboStages: $id"
		}
		if ([uint32]$skill.cooldownMs -eq 0) {
			throw "ACTIVE skill needs a cooldown: $id"
		}
	}
	else {
		if ($stages.Count -lt 2 -or $stages.Count -gt 8) {
			throw "COMBO skill needs 2..8 stages: $id"
		}
		for ($stageIndex = 0; $stageIndex -lt $stages.Count; $stageIndex++) {
			$stage = $stages[$stageIndex]
			Assert-ExactProperties $stage @(
				'actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs') 'combo stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.actionDurationMs) {
				throw "Combo stage timing is invalid: $id stage $stageIndex"
			}
			if ($stageIndex -eq $stages.Count - 1) {
				if ([uint32]$stage.inputOpenMs -ne 0 -or [uint32]$stage.inputCloseMs -ne 0) {
					throw "Final combo stage must not open an input window: $id"
				}
			}
			elseif ([uint32]$stage.inputOpenMs -ge [uint32]$stage.inputCloseMs -or
				[uint32]$stage.inputCloseMs -gt [uint32]$stage.actionDurationMs) {
				throw "Combo input window is invalid: $id stage $stageIndex"
			}
		}
	}
    $skillRows.Add((@(
        'SKILL', $id, $skill.characterClass, $skill.inputSlot, $skill.actionId,
        [uint32]$skill.cooldownMs, [uint32]$skill.actionDurationMs, [uint32]$skill.hitTimeMs,
        [uint32]$skill.resourceCost,
        (Format-InvariantFloat $skill.movementDistance "skill $id movementDistance"),
        (Format-InvariantFloat $skill.maximumRange "skill $id maximumRange"),
        $skill.serverDamageProfileId,
        $skillKind,
        $skill.requiredStance,
        $skill.setsStance) -join "`t"))
	for ($stageIndex = 0; $stageIndex -lt $stages.Count; $stageIndex++) {
		$stage = $stages[$stageIndex]
		$skillRows.Add((@(
			'SKILLSTAGE', $id, $stageIndex,
			[uint32]$stage.actionDurationMs, [uint32]$stage.hitTimeMs,
			[uint32]$stage.inputOpenMs, [uint32]$stage.inputCloseMs) -join "`t"))
	}
}

$bossDocument = Read-JsonDocument 'Data/Balance/BossProfiles.json'
Assert-ExactProperties $bossDocument @('schema','formatVersion','bosses') 'boss document'
Assert-JsonString $bossDocument.schema 'boss document schema'
Assert-JsonInteger $bossDocument.formatVersion 'boss document formatVersion' 3 3
if ($bossDocument.schema -ne 'lostark.boss-profiles' -or $bossDocument.formatVersion -ne 3) {
    throw 'Boss profile header is invalid.'
}
$bossIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$bossRows = [Collections.Generic.List[string]]::new()
foreach ($boss in @($bossDocument.bosses)) {
	Assert-ExactProperties $boss @(
		'archetypeId','encounterId','displayName','maximumHp','maximumHealthBars','attackPower','collisionRadius',
		'engageDistance','moveSpeed','phaseTwoHpPercent') 'boss profile'
	foreach ($stringField in @('archetypeId','encounterId','displayName')) {
		Assert-JsonString $boss.$stringField "boss $stringField"
	}
	Assert-JsonInteger $boss.maximumHp 'boss maximumHp' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $boss.maximumHealthBars 'boss maximumHealthBars' 1 1000
	Assert-JsonInteger $boss.attackPower 'boss attackPower' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $boss.phaseTwoHpPercent 'boss phaseTwoHpPercent' 1 99
	Assert-JsonNumber $boss.collisionRadius 'boss collisionRadius'
	Assert-JsonNumber $boss.engageDistance 'boss engageDistance'
	Assert-JsonNumber $boss.moveSpeed 'boss moveSpeed'
    Assert-StableId $boss.archetypeId 'boss archetypeId'
    Assert-StableId $boss.encounterId 'boss encounterId'
	if ([string]::IsNullOrWhiteSpace([string]$boss.displayName) -or ([string]$boss.displayName).Length -gt 64) {
		throw "Boss displayName is invalid: $($boss.archetypeId)"
	}
	# collisionRadius closes the gap between an official skill range, which is
	# measured to the edge of the target, and this server's centre-to-centre reach
	# test. A zero would put every melee skill out of range of its own boss.
    if (-not $bossIds.Add([string]$boss.archetypeId) -or [uint32]$boss.maximumHp -eq 0 -or
		[uint32]$boss.maximumHealthBars -eq 0 -or
        [uint32]$boss.attackPower -eq 0 -or [double]$boss.collisionRadius -le 0.0 -or
		[double]$boss.engageDistance -le 0.0 -or [double]$boss.moveSpeed -le 0.0 -or
        [uint32]$boss.phaseTwoHpPercent -eq 0 -or [uint32]$boss.phaseTwoHpPercent -ge 100) {
        throw "Boss profile is invalid: $($boss.archetypeId)"
    }
    $bossRows.Add((@(
        'BOSS', $boss.archetypeId, $boss.encounterId, [uint32]$boss.maximumHp,
		[uint32]$boss.maximumHealthBars,
        [uint32]$boss.attackPower,
        (Format-InvariantFloat $boss.collisionRadius 'boss collisionRadius'),
        (Format-InvariantFloat $boss.engageDistance 'boss engageDistance'),
        (Format-InvariantFloat $boss.moveSpeed 'boss moveSpeed'),
        [uint32]$boss.phaseTwoHpPercent) -join "`t"))
}

$encounterDocument = Read-JsonDocument 'Data/Encounters/Valtan/ValtanEncounter.json'
Assert-ExactProperties $encounterDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','authority','fixedTickHz','states','patterns') 'encounter document'
Assert-JsonString $encounterDocument.schema 'encounter schema'
Assert-JsonInteger $encounterDocument.formatVersion 'encounter formatVersion' 3 3
Assert-JsonString $encounterDocument.encounterId 'encounterId'
Assert-JsonString $encounterDocument.bossArchetypeId 'encounter bossArchetypeId'
Assert-JsonString $encounterDocument.authority 'encounter authority'
Assert-JsonInteger $encounterDocument.fixedTickHz 'encounter fixedTickHz' 30 30
Assert-StableId $encounterDocument.encounterId 'encounterId'
Assert-StableId $encounterDocument.bossArchetypeId 'encounter bossArchetypeId'
if ($encounterDocument.schema -ne 'lostark.encounter-profile' -or
	$encounterDocument.formatVersion -ne 3 -or
	$encounterDocument.authority -ne 'server' -or
	[uint32]$encounterDocument.fixedTickHz -ne 30 -or
	-not $bossIds.Contains([string]$encounterDocument.bossArchetypeId)) {
	throw 'Valtan encounter header or boss reference is invalid.'
}
$encounterBoss = @($bossDocument.bosses | Where-Object {
	$_.archetypeId -eq $encounterDocument.bossArchetypeId -and
	$_.encounterId -eq $encounterDocument.encounterId })
if ($encounterBoss.Count -ne 1) {
	throw 'Valtan encounter does not match exactly one boss profile.'
}
$maximumHealthBars = [uint32]$encounterBoss[0].maximumHealthBars
$patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$actionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$coveredSourceActionIds = [Collections.Generic.HashSet[uint32]]::new()
$patternRows = [Collections.Generic.List[string]]::new()
foreach ($pattern in @($encounterDocument.patterns)) {
	Assert-ExactProperties $pattern @(
		'patternId','displayName','actionId','sourceActionIds','selectionMode',
		'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
		'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
		'stages') 'encounter pattern'
	foreach ($field in @('patternId','displayName','actionId','selectionMode')) {
		Assert-JsonString $pattern.$field "pattern $($pattern.patternId) $field"
	}
	foreach ($field in @('minimumHealthBar','maximumHealthBar','triggerHealthBar',
		'triggerOrder','selectionWeight','maximumConsecutiveUses')) {
		Assert-JsonInteger $pattern.$field "pattern $($pattern.patternId) $field" 0 ([uint32]::MaxValue)
	}
	Assert-JsonNumber $pattern.minimumRange "pattern $($pattern.patternId) minimumRange"
	Assert-JsonNumber $pattern.maximumRange "pattern $($pattern.patternId) maximumRange"
	Assert-StableId $pattern.patternId 'patternId'
	Assert-StableId $pattern.actionId 'pattern actionId'
	if ([string]::IsNullOrWhiteSpace([string]$pattern.displayName) -or
		([string]$pattern.displayName).Length -gt 64 -or
		-not $patternIds.Add([string]$pattern.patternId) -or
		-not $actionIds.Add([string]$pattern.actionId) -or
		[double]$pattern.minimumRange -lt 0.0 -or
		[double]$pattern.maximumRange -le [double]$pattern.minimumRange -or
		$pattern.sourceActionIds -isnot [Array] -or
		@($pattern.sourceActionIds).Count -eq 0 -or
		$pattern.stages -isnot [Array] -or @($pattern.stages).Count -eq 0 -or
		@($pattern.stages).Count -gt 64) {
		throw "Encounter pattern base fields are invalid: $($pattern.patternId)"
	}
	$patternSourceIds = [Collections.Generic.HashSet[uint32]]::new()
	foreach ($sourceActionId in @($pattern.sourceActionIds)) {
		Assert-JsonInteger $sourceActionId "pattern $($pattern.patternId) sourceActionId" 1 ([uint32]::MaxValue)
		if (-not $patternSourceIds.Add([uint32]$sourceActionId)) {
			throw "Pattern sourceActionIds contain a duplicate: $($pattern.patternId)"
		}
		$coveredSourceActionIds.Add([uint32]$sourceActionId) | Out-Null
	}
	$selectionMode = [string]$pattern.selectionMode
	if ($selectionMode -eq 'NORMAL') {
		if ([uint32]$pattern.minimumHealthBar -lt 1 -or
			[uint32]$pattern.maximumHealthBar -lt [uint32]$pattern.minimumHealthBar -or
			[uint32]$pattern.maximumHealthBar -gt $maximumHealthBars -or
			[uint32]$pattern.triggerHealthBar -ne 0 -or [uint32]$pattern.triggerOrder -ne 0 -or
			[uint32]$pattern.selectionWeight -eq 0 -or [uint32]$pattern.maximumConsecutiveUses -eq 0) {
			throw "Normal pattern selection fields are invalid: $($pattern.patternId)"
		}
	}
	elseif ($selectionMode -eq 'HEALTH_BAR') {
		if ([uint32]$pattern.minimumHealthBar -ne 0 -or [uint32]$pattern.maximumHealthBar -ne 0 -or
			[uint32]$pattern.triggerHealthBar -lt 1 -or
			[uint32]$pattern.triggerHealthBar -gt $maximumHealthBars -or
			[uint32]$pattern.triggerOrder -lt 1 -or [uint32]$pattern.selectionWeight -ne 0 -or
			[uint32]$pattern.maximumConsecutiveUses -ne 0) {
			throw "Health-bar pattern selection fields are invalid: $($pattern.patternId)"
		}
	}
	else { throw "Unknown pattern selection mode: $($pattern.patternId)" }
	$patternRows.Add((@(
		'PATTERN', $encounterDocument.encounterId, $pattern.patternId, $pattern.actionId, $selectionMode,
		[uint32]$pattern.minimumHealthBar, [uint32]$pattern.maximumHealthBar,
		[uint32]$pattern.triggerHealthBar, [uint32]$pattern.triggerOrder,
		[uint32]$pattern.selectionWeight, [uint32]$pattern.maximumConsecutiveUses,
		(Format-InvariantFloat $pattern.minimumRange "pattern minimumRange"),
		(Format-InvariantFloat $pattern.maximumRange "pattern maximumRange"),
		@($pattern.stages).Count) -join "`t"))
	$stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	for ($stageIndex = 0; $stageIndex -lt @($pattern.stages).Count; $stageIndex++) {
		$stage = $pattern.stages[$stageIndex]
		Assert-ExactProperties $stage @(
			'stageId','actionId','stageKind','durationMs','hitShape',
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength','hitHalfWidth',
			'hitCount','hitIntervalMs','serverDamageProfileId') 'encounter pattern stage'
		foreach ($field in @('stageId','actionId','stageKind','hitShape','serverDamageProfileId')) {
			Assert-JsonString $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field"
		}
		foreach ($field in @('durationMs','hitCount','hitIntervalMs')) {
			Assert-JsonInteger $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field" 0 ([uint32]::MaxValue)
		}
		foreach ($field in @('hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength','hitHalfWidth')) {
			Assert-JsonNumber $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field"
		}
		Assert-StableId $stage.stageId 'pattern stageId'
		Assert-StableId $stage.actionId 'pattern stage actionId'
		if (-not $stageIds.Add([string]$stage.stageId) -or [uint32]$stage.durationMs -eq 0 -or
			[string]$stage.stageKind -notin @('WINDUP','ACTIVE','RECOVERY')) {
			throw "Pattern stage identity is invalid: $($pattern.patternId) stage $stageIndex"
		}
		$shape = [string]$stage.hitShape
		$outer = [double]$stage.hitOuterRadius
		$inner = [double]$stage.hitInnerRadius
		$angle = [double]$stage.hitAngleDegrees
		$length = [double]$stage.hitLength
		$halfWidth = [double]$stage.hitHalfWidth
		$hitCount = [uint32]$stage.hitCount
		$hitIntervalMs = [uint32]$stage.hitIntervalMs
		$damageProfile = [string]$stage.serverDamageProfileId
		$zeroShape = $outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 -and
			$length -eq 0.0 -and $halfWidth -eq 0.0
		$validShape = $false
		switch ($shape) {
			'NONE' { $validShape = $zeroShape -and $hitCount -eq 0 -and $hitIntervalMs -eq 0 -and $damageProfile.Length -eq 0 }
			'CIRCLE' { $validShape = $outer -gt 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 -and $length -eq 0.0 -and $halfWidth -eq 0.0 }
			'RING' { $validShape = $outer -gt $inner -and $inner -gt 0.0 -and $angle -eq 0.0 -and $length -eq 0.0 -and $halfWidth -eq 0.0 }
			'CONE' { $validShape = $angle -gt 0.0 -and $angle -le 180.0 -and $length -gt 0.0 -and $outer -eq 0.0 -and $inner -eq 0.0 -and $halfWidth -eq 0.0 }
			'BOX' { $validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and $outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 }
			'CROSS' { $validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and $outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 }
			default { throw "Unknown pattern stage hit shape: $($pattern.patternId) stage $stageIndex" }
		}
		if ($shape -ne 'NONE') {
			if ($damageProfile.Length -gt 0) { Assert-StableId $damageProfile 'pattern stage damage profile' }
			$validShape = $validShape -and $hitCount -gt 0 -and
				(($hitCount -eq 1 -and $hitIntervalMs -eq 0) -or ($hitCount -gt 1 -and $hitIntervalMs -gt 0)) -and
				([uint64]($hitCount - 1) * [uint64]$hitIntervalMs) -lt [uint64][uint32]$stage.durationMs -and
				$damageIds.Contains($damageProfile)
		}
		if (-not $validShape) { throw "Pattern stage hit contract is invalid: $($pattern.patternId) stage $stageIndex" }
		$patternRows.Add((@(
			'PATTERNSTAGE', $encounterDocument.encounterId, $pattern.patternId, $stageIndex,
			$stage.stageId, $stage.actionId, $stage.stageKind, [uint32]$stage.durationMs,
			$shape,
			(Format-InvariantFloat $outer 'stage hitOuterRadius'),
			(Format-InvariantFloat $inner 'stage hitInnerRadius'),
			(Format-InvariantFloat $angle 'stage hitAngleDegrees'),
			(Format-InvariantFloat $length 'stage hitLength'),
			(Format-InvariantFloat $halfWidth 'stage hitHalfWidth'),
			$hitCount, $hitIntervalMs,
			$(if ($damageProfile.Length -eq 0) { '-' } else { $damageProfile })) -join "`t"))
	}
}
if ($patternRows.Count -eq 0) { throw 'Valtan encounter has no patterns.' }

$expectedNormalActionIds = @((420601..420647) + (420651..420666)) |
	Where-Object { $_ -notin @(420648,420649,420650) }
foreach ($requiredActionId in $expectedNormalActionIds) {
	if (-not $coveredSourceActionIds.Contains([uint32]$requiredActionId)) {
		throw "Valtan Normal Action coverage is missing sourceActionId $requiredActionId"
	}
}

Assert-BalanceProvenance $playerDocument $skillDocument $damageDocument $bossDocument

$skillDurationById = @{}
foreach ($skill in @($skillDocument.skills)) {
    $skillDurationById[[string]$skill.skillId] = [uint32]$skill.actionDurationMs
}
$rootMotionRows = [Collections.Generic.List[string]]::new()
$rootMotionSeen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($path in @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'Data\Animation\RootMotion') `
        -Filter '*.rootmotion.json' -File -ErrorAction SilentlyContinue | Sort-Object Name)) {
    $document = Read-JsonDocument ('Data/Animation/RootMotion/' + $path.Name)
    Assert-ExactProperties $document @(
        'schema','formatVersion','animationAssetId','characterClass','skills') 'root motion document'
    if ($document.schema -ne 'lostark.animation-root-motion' -or
        [uint32]$document.formatVersion -ne 1) {
        throw "Root motion header is invalid: $($path.Name)"
    }
    foreach ($entry in @($document.skills)) {
        Assert-ExactProperties $entry @('skillId','durationMs','samples') 'root motion skill'
        $id = [string]$entry.skillId
        if (-not $skillDurationById.ContainsKey($id)) {
            throw "Root motion targets an unknown skill: $id"
        }
        if (-not $rootMotionSeen.Add($id)) {
            throw "Duplicate root motion entry: $id"
        }
        $samples = @($entry.samples)
        if ($samples.Count -lt 2 -or $samples.Count -gt 512) {
            throw "Root motion sample count is invalid: $id"
        }
        $packed = [Collections.Generic.List[string]]::new()
        $previousMs = -1
        foreach ($sample in $samples) {
            Assert-ExactProperties $sample @('timeMs','forward','lateral','up') 'root motion sample'
            $timeMs = [int]$sample.timeMs
            if ($timeMs -le $previousMs -or $timeMs -gt $skillDurationById[$id]) {
                throw "Root motion sample time is out of order or past the action: $id"
            }
            $previousMs = $timeMs
            $packed.Add(('{0}:{1}:{2}' -f $timeMs,
                (Format-InvariantSignedFloat $sample.forward "root motion $id forward"),
                (Format-InvariantSignedFloat $sample.lateral "root motion $id lateral")))
        }
        $rootMotionRows.Add((@(
            'SKILLROOTMOTION', $id, $samples.Count, ($packed -join ',')) -join "`t"))
    }
}

$rows = @($damageRows + $skillRows + $playerRows + $bossRows + $rootMotionRows + $patternRows | Sort-Object)
$lines = @("LOSTARK_GAMEPLAY_BOOTSTRAP`t3`t$($rows.Count)") + $rows

if ($Mode -eq 'Publish') {
    $root = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    [IO.Directory]::CreateDirectory($root) | Out-Null
    $destination = Join-Path $root 'Gameplay.bootstrap'
    $staged = Join-Path $root ('.Gameplay.staging.' + [Guid]::NewGuid().ToString('N'))
    $rollback = Join-Path $root ('.Gameplay.rollback.' + [Guid]::NewGuid().ToString('N'))
    try {
        [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
        if ([IO.File]::Exists($destination)) { [IO.File]::Move($destination, $rollback) }
        [IO.File]::Move($staged, $destination)
        if ([IO.File]::Exists($rollback)) { [IO.File]::Delete($rollback) }
    }
    catch {
        if ([IO.File]::Exists($destination)) { [IO.File]::Delete($destination) }
        if ([IO.File]::Exists($rollback)) { [IO.File]::Move($rollback, $destination) }
        if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
        throw
    }
}

$patternCount = @($encounterDocument.patterns).Count
$stageCount = @($encounterDocument.patterns | ForEach-Object { @($_.stages).Count } |
	Measure-Object -Sum).Sum
Write-Host "Gameplay balance $Mode succeeded: $($playerRows.Count) player profiles, $($skillRows.Count) skills, $($damageRows.Count) damage profiles, $($bossRows.Count) bosses, $patternCount boss patterns, $stageCount pattern stages."
