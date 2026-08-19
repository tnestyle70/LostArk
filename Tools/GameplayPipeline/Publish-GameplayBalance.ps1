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

function Read-ValtanSkillTiming {
    $relativePath = 'Data/Animation/Reference/Valtan/Valtan.skilltiming'
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $relativePath))
    if (-not [IO.File]::Exists($path)) {
        throw "Missing Valtan source timing document: $relativePath"
    }
    $lines = [IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8)
    if ($lines.Count -eq 0 -or
        $lines[0] -notmatch '^LOSTARK_SKILL_TIMING 3 "Valtan" ([0-9]+)$') {
        throw 'Valtan source timing header is invalid.'
    }
    $expectedCount = [uint32]$Matches[1]
    $rows = @{}
    $active = $null
    $observedShapeCount = [uint32]0
    $commitActive = {
        if ($null -eq $active) { return }
        if ($observedShapeCount -ne [uint32]$active.ShapeCount) {
            throw "Valtan source timing shape count mismatch: $($active.ActionId)"
        }
        if ($rows.ContainsKey([uint32]$active.ActionId)) {
            throw "Duplicate Valtan source timing action: $($active.ActionId)"
        }
        $rows[[uint32]$active.ActionId] = $active
    }
    for ($lineIndex = 1; $lineIndex -lt $lines.Count; ++$lineIndex) {
        $line = $lines[$lineIndex]
        if ([string]::IsNullOrWhiteSpace($line)) { continue }
        if ($line -match '^([0-9]+) "([^"]*)" shapes=([0-9]+) cd=([0-9]+) range=([0-9]+) approach=([0-9]+) turn=([0-9]+)$') {
            & $commitActive
            $active = [pscustomobject]@{
                ActionId = [uint32]$Matches[1]
                DisplayName = [string]$Matches[2]
                ShapeCount = [uint32]$Matches[3]
                CooldownMs = [uint32]$Matches[4]
                RangeUnits = [uint32]$Matches[5]
                ApproachUnits = [uint32]$Matches[6]
                TurnDegrees = [uint32]$Matches[7]
            }
            if ($active.ActionId -eq 0 -or $active.ShapeCount -gt 256 -or
                $active.CooldownMs -gt 600000 -or
                $active.RangeUnits -gt 100000 -or
                $active.ApproachUnits -gt 100000 -or
                $active.TurnDegrees -gt 360) {
                throw "Valtan source timing row is out of range: $($active.ActionId)"
            }
            $observedShapeCount = [uint32]0
            continue
        }
        if ($line -notmatch '^  shape ' -or $null -eq $active) {
            throw "Valtan source timing row is malformed at line $($lineIndex + 1)."
        }
        ++$observedShapeCount
    }
    & $commitActive
    if ($rows.Count -ne $expectedCount) {
        throw "Valtan source timing count mismatch. expected=$expectedCount actual=$($rows.Count)"
    }
    return $rows
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
		'attackPower','defense','moveSpeed','defenseStanceMoveSpeedScale',
		'maximumIdentity','identityRegenPerSecond','identityDrainPerSecond',
		'identityStanceSwitchCost','identityCyclic','defaultStance') 'player profile'
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
	Assert-JsonNumber $player.defenseStanceMoveSpeedScale 'player defenseStanceMoveSpeedScale'
	# Identity is opt-in: a class without the resource stores 0 across all five
	# and the server never gives it a gauge. A class that has one spends it one
	# of four ways: identityDrainPerSecond bleeds it continuously while an
	# alternate stance is held (Warlord), identityStanceSwitchCost charges a
	# flat amount at the moment a stance-setting skill lands (LanceMaster's
	# long/short spear swap), a skill of that class carries its own
	# identityCost charged once on cast (Artist's moon/sun orbs), or
	# identityCyclic wraps the fill back to 0 at the maximum instead of holding
	# there, so the wrap itself is the spend (DimensionMaster's clock). The
	# skill-cost path needs the skill document, so it is checked after skills
	# load below.
	Assert-JsonInteger $player.maximumIdentity 'player maximumIdentity' 0 ([uint32]::MaxValue)
	Assert-JsonInteger $player.identityRegenPerSecond 'player identityRegenPerSecond' 0 ([uint32]::MaxValue)
	Assert-JsonInteger $player.identityDrainPerSecond 'player identityDrainPerSecond' 0 ([uint32]::MaxValue)
	Assert-JsonInteger $player.identityStanceSwitchCost 'player identityStanceSwitchCost' 0 ([uint32]::MaxValue)
	Assert-JsonInteger $player.identityCyclic 'player identityCyclic' 0 1
	if ([uint32]$player.maximumIdentity -eq 0) {
		if ([uint32]$player.identityRegenPerSecond -ne 0 -or
			[uint32]$player.identityDrainPerSecond -ne 0 -or
			[uint32]$player.identityStanceSwitchCost -ne 0 -or
			[uint32]$player.identityCyclic -ne 0) {
			throw "Player identity rates need a gauge to fill: $($player.characterClass)"
		}
	}
	else {
		if ([uint32]$player.identityStanceSwitchCost -gt [uint32]$player.maximumIdentity) {
			throw "Player identityStanceSwitchCost exceeds maximumIdentity: $($player.characterClass)"
		}
		if ([uint32]$player.identityCyclic -ne 0 -and
			([uint32]$player.identityDrainPerSecond -ne 0 -or
				[uint32]$player.identityStanceSwitchCost -ne 0 -or
				[uint32]$player.identityRegenPerSecond -eq 0)) {
			# A cyclic gauge spends itself by wrapping, so it cannot also drain
			# or charge a switch, and it needs a nonzero regen or it would never
			# reach the lap it wraps on.
			throw "Player identityCyclic conflicts with drain, switch cost, or has no regen: $($player.characterClass)"
		}
	}
	if ([double]$player.defenseStanceMoveSpeedScale -le 0.0 -or
		[double]$player.defenseStanceMoveSpeedScale -gt 1.0) {
		throw "Player defenseStanceMoveSpeedScale must be within (0,1]: $($player.characterClass)"
	}
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
		(Format-InvariantFloat $player.defenseStanceMoveSpeedScale 'player defenseStanceMoveSpeedScale'),
		[uint32]$player.maximumIdentity, [uint32]$player.identityRegenPerSecond,
		[uint32]$player.identityDrainPerSecond, [uint32]$player.identityStanceSwitchCost,
		[uint32]$player.identityCyclic,
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
# Same bound, for the identity gauge a skill can charge per cast (Artist's
# moon/sun orbs) instead of per second.
$maximumPlayerIdentity = (@($playerDocument.players) |
	ForEach-Object { [uint32]$_.maximumIdentity } | Measure-Object -Maximum).Maximum
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
        'hitTimeMs','resourceCost','identityCost','movementDistance','maximumRange','serverDamageProfileId',
		'effectId','comboStages') 'player skill'
	Assert-JsonInteger $skill.skillId 'skillId' 1 ([uint32]::MaxValue)
	foreach ($stringField in @('characterClass','inputSlot','displayName','actionId','skillKind','serverDamageProfileId',
		'effectId','requiredStance','setsStance')) {
		Assert-JsonString $skill.$stringField "skill $($skill.skillId) $stringField"
	}
	if ($skill.requiredStance -notin $knownStances -or $skill.setsStance -notin $knownStances) {
		throw "Skill stance is unknown: $($skill.skillId)"
	}
	foreach ($integerField in @('cooldownMs','actionDurationMs','hitTimeMs','resourceCost','identityCost')) {
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
		[uint32]$skill.identityCost -gt $maximumPlayerIdentity -or
		($dealsDamage -and ([double]$skill.maximumRange -le 0.0 -or
			-not $damageIds.Contains($damageProfileId))) -or
		(-not $dealsDamage -and ([double]$skill.maximumRange -ne 0.0 -or
			[uint32]$skill.hitTimeMs -ne 0))) {
        throw "Player skill timing, class, slot, resource, or damage reference is invalid: $id"
    }
	$skillKind = [string]$skill.skillKind
	if ($skillKind -notin @('ACTIVE','COMBO','HOLD','COUNTER')) {
		throw "Unknown skillKind: $id $skillKind"
	}
	$stages = @($skill.comboStages)
	if ($skillKind -eq 'COUNTER') {
		# Guard then counter. The guard stage reuses its input window as the
		# window a hit taken has to land in, and lands no damage of its own.
		if ($stages.Count -ne 2) {
			throw "COUNTER skill needs exactly 2 stages: $id"
		}
		if ([uint32]$skill.cooldownMs -eq 0) {
			throw "COUNTER skill needs a cooldown: $id"
		}
		for ($stageIndex = 0; $stageIndex -lt 2; $stageIndex++) {
			$stage = $stages[$stageIndex]
			Assert-ExactProperties $stage @(
				'actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs') 'counter stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.actionDurationMs) {
				throw "Counter stage timing is invalid: $id stage $stageIndex"
			}
			if ($stageIndex -eq 0) {
				if ([uint32]$stage.hitTimeMs -ne 0) {
					throw "A counter's guard stage lands no damage: $id"
				}
				if ([uint32]$stage.inputOpenMs -ge [uint32]$stage.inputCloseMs -or
					[uint32]$stage.inputCloseMs -gt [uint32]$stage.actionDurationMs) {
					throw "Counter guard window is invalid: $id"
				}
			}
			elseif ([uint32]$stage.hitTimeMs -eq 0 -or
				[uint32]$stage.inputOpenMs -ne 0 -or [uint32]$stage.inputCloseMs -ne 0) {
				throw "A counter's second stage lands the damage and opens no window: $id"
			}
		}
	}
	elseif ($skillKind -eq 'HOLD') {
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
        [uint32]$skill.resourceCost, [uint32]$skill.identityCost,
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

$classesWithIdentitySkillCost = [Collections.Generic.HashSet[string]]::new()
foreach ($skill in @($skillDocument.skills)) {
    if ([uint32]$skill.identityCost -gt 0) {
        [void]$classesWithIdentitySkillCost.Add([string]$skill.characterClass)
    }
}
foreach ($player in @($playerDocument.players)) {
    if ([uint32]$player.maximumIdentity -gt 0 -and
        [uint32]$player.identityDrainPerSecond -eq 0 -and
        [uint32]$player.identityStanceSwitchCost -eq 0 -and
        [uint32]$player.identityCyclic -eq 0 -and
        -not $classesWithIdentitySkillCost.Contains([string]$player.characterClass)) {
        # A gauge that never drains, never charges a switch, never wraps, and
        # backs no skill's identityCost would just sit there once full, which
        # is the one thing spending it exists to stop.
        throw "Player identity gauge never spends: $($player.characterClass)"
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
	'schema','formatVersion','encounterId','bossArchetypeId','authority','fixedTickHz',
	'introPatternId','states','patterns') 'encounter document'
Assert-StableId $encounterDocument.introPatternId 'encounter introPatternId'
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
$sourceTimingByActionId = Read-ValtanSkillTiming
$patternRows = [Collections.Generic.List[string]]::new()
$serverMotionAnchorIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$serverMotionByPatternId = @{}
foreach ($pattern in @($encounterDocument.patterns)) {
	# serverMotion is optional. Only a pattern whose boss motion the Server has to
	# compute itself, like the 109 leap to its landing anchor, carries one.
	$patternProperties = @(
		'patternId','displayName','actionId','sourceActionIds','selectionMode',
		'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
		'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
		'stages')
	$hasServerMotion = $null -ne $pattern.PSObject.Properties['serverMotion']
	if ($hasServerMotion) { $patternProperties += 'serverMotion' }
	Assert-ExactProperties $pattern $patternProperties 'encounter pattern'
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
		if (-not $sourceTimingByActionId.ContainsKey([uint32]$sourceActionId)) {
			throw "Pattern sourceActionId is absent from Valtan.skilltiming: $($pattern.patternId)/$sourceActionId"
		}
		$coveredSourceActionIds.Add([uint32]$sourceActionId) | Out-Null
	}
	$primarySourceActionId = [uint32]$pattern.sourceActionIds[0]
	$primarySourceTiming = $sourceTimingByActionId[$primarySourceActionId]
	$sourceCooldownTicks = if ([uint32]$primarySourceTiming.CooldownMs -eq 0) {
		[uint32]0
	} else {
		[uint32][Math]::Ceiling(
			([double][uint32]$primarySourceTiming.CooldownMs *
				[double][uint32]$encounterDocument.fixedTickHz) / 1000.0)
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
	# sourceActionIds[0] is the pattern entry skill. The remaining IDs are
	# continuations/variants and are still checked above, but only the entry
	# skill owns selection cooldown/range/approach/turn metadata.
	$patternRows.Add((@(
		'PATTERNSOURCE', $encounterDocument.encounterId, $pattern.patternId,
		$primarySourceActionId, [uint32]$primarySourceTiming.ShapeCount,
		[uint32]$primarySourceTiming.CooldownMs, $sourceCooldownTicks,
		[uint32]$primarySourceTiming.RangeUnits,
		[uint32]$primarySourceTiming.ApproachUnits,
		[uint32]$primarySourceTiming.TurnDegrees) -join "`t"))
	if ($hasServerMotion) {
		# One compiled anchor is the single source for the Server leap landing,
		# the camera lookAt and the radial wall launch directions, so none of the
		# three can drift into its own copy of the coordinate.
		$motion = $pattern.serverMotion
		Assert-ExactProperties $motion @(
			'kind','anchorId','landingPosition','apexHeight') "pattern $($pattern.patternId) serverMotion"
		Assert-JsonString $motion.kind "pattern $($pattern.patternId) serverMotion kind"
		Assert-StableId $motion.anchorId "pattern $($pattern.patternId) serverMotion anchorId"
		if ([string]$motion.kind -cne 'LEAP_TO_ANCHOR') {
			throw "Unknown serverMotion kind: $($pattern.patternId)"
		}
		if ($motion.landingPosition -isnot [Array] -or
			@($motion.landingPosition).Count -ne 3) {
			throw "serverMotion landingPosition must contain three numbers: $($pattern.patternId)"
		}
		foreach ($component in @($motion.landingPosition)) {
			Assert-JsonNumber $component "pattern $($pattern.patternId) serverMotion landingPosition"
			if ([double]::IsNaN([double]$component) -or
				[double]::IsInfinity([double]$component) -or
				[Math]::Abs([double]$component) -gt 100000.0) {
				throw "serverMotion landingPosition is out of range: $($pattern.patternId)"
			}
		}
		Assert-JsonNumber $motion.apexHeight "pattern $($pattern.patternId) serverMotion apexHeight"
		if ([double]$motion.apexHeight -le 0.0 -or [double]$motion.apexHeight -gt 200.0) {
			throw "serverMotion apexHeight is out of range: $($pattern.patternId)"
		}
		if (-not $serverMotionAnchorIds.Add([string]$motion.anchorId)) {
			throw "Duplicate serverMotion anchorId: $($motion.anchorId)"
		}
		$serverMotionByPatternId[[string]$pattern.patternId] = [pscustomobject]@{
			AnchorId = [string]$motion.anchorId
			X = [double]$motion.landingPosition[0]
			Y = [double]$motion.landingPosition[1]
			Z = [double]$motion.landingPosition[2]
		}
		$patternRows.Add((@(
			'PATTERNMOTION', $encounterDocument.encounterId, $pattern.patternId,
			[string]$motion.kind, [string]$motion.anchorId,
			(Format-InvariantSignedFloat $motion.landingPosition[0] 'serverMotion landing x'),
			(Format-InvariantSignedFloat $motion.landingPosition[1] 'serverMotion landing y'),
			(Format-InvariantSignedFloat $motion.landingPosition[2] 'serverMotion landing z'),
			(Format-InvariantFloat $motion.apexHeight 'serverMotion apexHeight')) -join "`t"))
	}
	$stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	for ($stageIndex = 0; $stageIndex -lt @($pattern.stages).Count; $stageIndex++) {
		$stage = $pattern.stages[$stageIndex]
		Assert-ExactProperties $stage @(
			'stageId','actionId','stageKind','durationMs','hitShape',
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength','hitHalfWidth',
			'hitCount','hitIntervalMs','serverDamageProfileId',
			'pushRangeM','pushMs','knockdown','downMs') 'encounter pattern stage'
		foreach ($field in @('stageId','actionId','stageKind','hitShape','serverDamageProfileId')) {
			Assert-JsonString $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field"
		}
		foreach ($field in @('durationMs','hitCount','hitIntervalMs','pushMs','downMs')) {
			Assert-JsonInteger $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field" 0 ([uint32]::MaxValue)
		}
		Assert-JsonNumber $stage.pushRangeM "pattern $($pattern.patternId) stage $stageIndex pushRangeM"
		if ($stage.knockdown -isnot [bool]) {
			throw "Pattern stage knockdown must be a JSON Boolean: $($pattern.patternId) stage $stageIndex"
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
			'SIX_DIRECTIONS' { $validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and $outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 }
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
		$pushRangeM = [double]$stage.pushRangeM
		$pushMs = [uint32]$stage.pushMs
		$knockdown = [bool]$stage.knockdown
		$downMs = [uint32]$stage.downMs
		if ([math]::Abs($pushRangeM) -gt 20.0 -or
			($pushRangeM -ne 0.0 -and $pushMs -eq 0) -or
			($pushRangeM -eq 0.0 -and $pushMs -ne 0) -or
			($knockdown -and $downMs -eq 0) -or
			(-not $knockdown -and $downMs -ne 0) -or
			($damageProfile.Length -eq 0 -and ($pushRangeM -ne 0.0 -or $knockdown))) {
			throw "Pattern stage push contract is invalid: $($pattern.patternId) stage $stageIndex"
		}
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
			$(if ($damageProfile.Length -eq 0) { '-' } else { $damageProfile }),
			(Format-InvariantSignedFloat $pushRangeM 'stage pushRangeM'),
			$pushMs,
			$(if ($knockdown) { 1 } else { 0 }),
			$downMs) -join "`t"))
	}
}
if ($patternRows.Count -eq 0) { throw 'Valtan encounter has no patterns.' }

# Only an explicitly authored physical axe action may project its Server hit
# volume onto breakable walls. Player damage shapes also describe roars, magic,
# delayed waves and floor mechanics, so treating every ACTIVE hit as a weapon
# contact would destroy unrelated walls.
$wallContactDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanWallContactActions.json'
Assert-ExactProperties $wallContactDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','actions') `
	'Valtan wall contact document'
Assert-JsonString $wallContactDocument.schema 'Valtan wall contact schema'
Assert-JsonInteger $wallContactDocument.formatVersion `
	'Valtan wall contact formatVersion' 1 1
Assert-JsonString $wallContactDocument.encounterId `
	'Valtan wall contact encounterId'
Assert-JsonString $wallContactDocument.bossArchetypeId `
	'Valtan wall contact bossArchetypeId'
if ([string]$wallContactDocument.schema -cne `
	'lostark.valtan-wall-contact-actions' -or
	[uint32]$wallContactDocument.formatVersion -ne 1 -or
	[string]$wallContactDocument.encounterId -cne `
		[string]$encounterDocument.encounterId -or
	[string]$wallContactDocument.bossArchetypeId -cne `
		[string]$encounterDocument.bossArchetypeId -or
	$wallContactDocument.actions -isnot [Array] -or
	@($wallContactDocument.actions).Count -eq 0 -or
	@($wallContactDocument.actions).Count -gt 64) {
	throw 'Valtan wall contact document header is invalid.'
}
$wallContactActionIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$previousWallContactActionId = ''
foreach ($wallContact in @($wallContactDocument.actions)) {
	Assert-ExactProperties $wallContact @('patternId','stageId','actionId') `
		'Valtan wall contact action'
	foreach ($field in @('patternId','stageId','actionId')) {
		Assert-JsonString $wallContact.$field "Valtan wall contact $field"
		Assert-StableId ([string]$wallContact.$field) "Valtan wall contact $field"
	}
	$wallActionId = [string]$wallContact.actionId
	if (-not $wallContactActionIds.Add($wallActionId) -or
		($previousWallContactActionId.Length -ne 0 -and
		 [StringComparer]::Ordinal.Compare(
			$previousWallContactActionId, $wallActionId) -ge 0)) {
		throw "Valtan wall contact actions are duplicated or not ordinal: $wallActionId"
	}
	$previousWallContactActionId = $wallActionId
	$ownerPatterns = @($encounterDocument.patterns | Where-Object {
		[string]$_.patternId -ceq [string]$wallContact.patternId })
	if ($ownerPatterns.Count -ne 1) {
		throw "Valtan wall contact pattern is missing: $($wallContact.patternId)"
	}
	$ownerPattern = $ownerPatterns[0]
	$stageIndex = -1
	for ($candidateIndex = 0;
		$candidateIndex -lt @($ownerPattern.stages).Count; ++$candidateIndex) {
		$candidate = $ownerPattern.stages[$candidateIndex]
		if ([string]$candidate.stageId -ceq [string]$wallContact.stageId -and
			[string]$candidate.actionId -ceq $wallActionId) {
			if ($stageIndex -ne -1) {
				throw "Valtan wall contact stage is duplicated: $wallActionId"
			}
			$stageIndex = $candidateIndex
		}
	}
	if ($stageIndex -lt 0) {
		throw "Valtan wall contact stage/action join failed: $wallActionId"
	}
	$contactStage = $ownerPattern.stages[$stageIndex]
	if ([string]$contactStage.stageKind -cne 'ACTIVE' -or
		[string]$contactStage.hitShape -ceq 'NONE' -or
		[uint32]$contactStage.hitCount -eq 0) {
		throw "Valtan wall contact action has no physical Server hit pulse: $wallActionId"
	}
	$patternRows.Add((@(
		'PATTERNWALLCONTACT', $encounterDocument.encounterId,
		$ownerPattern.patternId, [uint32]$stageIndex,
		$contactStage.stageId, $contactStage.actionId) -join "`t"))
}

# The intro pattern runs exactly once per encounter epoch, before normal
# selection starts, so the first-appearance sweep cannot be rolled again later.
if (-not $patternIds.Contains([string]$encounterDocument.introPatternId)) {
	throw "Encounter introPatternId names no pattern: $($encounterDocument.introPatternId)"
}
$patternRows.Add((@(
	'ENCOUNTERINTRO', $encounterDocument.encounterId,
	$encounterDocument.introPatternId) -join "`t"))

# A pattern that owns a landing anchor also owns every cinematic camera cue bound
# to it, so the shot cannot frame a different point than the one the boss lands
# on. Only the horizontal position is joined: the cues deliberately raise their
# lookAt height to follow the boss while it is still in the air.
if ($serverMotionByPatternId.Count -ne 0) {
	$cameraDocument = Read-JsonDocument 'Data/Encounters/Valtan/ValtanCinematicCamera.json'
	$anchoredCueCount = 0
	foreach ($cue in @($cameraDocument.cues)) {
		$cuePatternId = [string]$cue.patternId
		if (-not $serverMotionByPatternId.ContainsKey($cuePatternId)) { continue }
		$anchor = $serverMotionByPatternId[$cuePatternId]
		foreach ($keyframe in @($cue.keyframes)) {
			if (@($keyframe.lookAt).Count -ne 3) {
				throw "Cinematic keyframe lookAt is malformed: $($cue.cueId)"
			}
			if ([Math]::Abs([double]$keyframe.lookAt[0] - $anchor.X) -gt 0.05 -or
				[Math]::Abs([double]$keyframe.lookAt[2] - $anchor.Z) -gt 0.05) {
				throw "Cinematic cue does not look at its pattern landing anchor $($anchor.AnchorId): $($cue.cueId)"
			}
		}
		$anchoredCueCount++
	}
	if ($anchoredCueCount -eq 0) {
		throw 'A pattern declares a landing anchor but owns no cinematic camera cue.'
	}
}

$expectedNormalActionIds = @((420601..420647) + (420651..420666)) |
	Where-Object { $_ -notin @(420648,420649,420650) }
foreach ($requiredActionId in $expectedNormalActionIds) {
	if (-not $coveredSourceActionIds.Contains([uint32]$requiredActionId)) {
		throw "Valtan Normal Action coverage is missing sourceActionId $requiredActionId"
	}
}

Assert-BalanceProvenance $playerDocument $skillDocument $damageDocument $bossDocument

$skillDurationById = @{}
$skillStageDurationsById = @{}
foreach ($skill in @($skillDocument.skills)) {
    $skillDurationById[[string]$skill.skillId] = [uint32]$skill.actionDurationMs
    $skillStageDurationsById[[string]$skill.skillId] = @(
        @($skill.comboStages) | ForEach-Object { [uint32]$_.actionDurationMs })
}

function Format-RootMotionSamples {
    param(
        [object[]]$Samples,
        [string]$SkillId,
        [uint32]$LimitMs
    )

    if ($Samples.Count -lt 2 -or $Samples.Count -gt 512) {
        throw "Root motion sample count is invalid: $SkillId"
    }
    $packed = [Collections.Generic.List[string]]::new()
    $previousMs = -1
    foreach ($sample in $Samples) {
        Assert-ExactProperties $sample @('timeMs','forward','lateral','up') 'root motion sample'
        $timeMs = [int]$sample.timeMs
        if ($timeMs -le $previousMs -or $timeMs -gt $LimitMs) {
            throw "Root motion sample time is out of order or past the action: $SkillId"
        }
        $previousMs = $timeMs
        $packed.Add(('{0}:{1}:{2}' -f $timeMs,
            (Format-InvariantSignedFloat $sample.forward "root motion $SkillId forward"),
            (Format-InvariantSignedFloat $sample.lateral "root motion $SkillId lateral")))
    }
    return ($packed -join ',')
}

$rootMotionRows = [Collections.Generic.List[string]]::new()
$rootMotionSeen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($path in @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'Data\Animation\RootMotion') `
        -Filter '*.rootmotion.json' -File -ErrorAction SilentlyContinue | Sort-Object Name)) {
    $document = Read-JsonDocument ('Data/Animation/RootMotion/' + $path.Name)
    Assert-ExactProperties $document @(
        'schema','formatVersion','animationAssetId','characterClass','skills') 'root motion document'
    if ($document.schema -ne 'lostark.animation-root-motion' -or
        [uint32]$document.formatVersion -ne 2) {
        throw "Root motion header is invalid: $($path.Name)"
    }
    foreach ($entry in @($document.skills)) {
        $id = [string]$entry.skillId
        if (-not $skillDurationById.ContainsKey($id)) {
            throw "Root motion targets an unknown skill: $id"
        }
        if (-not $rootMotionSeen.Add($id)) {
            throw "Duplicate root motion entry: $id"
        }
        # A staged skill runs each stage on its own clock, so it carries one
        # curve per stage instead of the single action-long curve.
        if ($null -ne $entry.stages) {
            Assert-ExactProperties $entry @('skillId','stages') 'root motion skill'
            $stageDurations = @($skillStageDurationsById[$id])
            if ($stageDurations.Count -lt 1) {
                throw "Root motion stages target a skill without combo stages: $id"
            }
            $seenStages = [Collections.Generic.HashSet[int]]::new()
            foreach ($stage in @($entry.stages)) {
                Assert-ExactProperties $stage @(
                    'stageIndex','durationMs','samples') 'root motion stage'
                $stageIndex = [int]$stage.stageIndex
                if ($stageIndex -lt 0 -or $stageIndex -ge $stageDurations.Count -or
                    -not $seenStages.Add($stageIndex)) {
                    throw "Root motion stage index is invalid or duplicated: $id"
                }
                $samples = @($stage.samples)
                $packed = Format-RootMotionSamples `
                    -Samples $samples -SkillId $id `
                    -LimitMs $stageDurations[$stageIndex]
                $rootMotionRows.Add((@(
                    'SKILLSTAGEROOTMOTION', $id, $stageIndex,
                    $samples.Count, $packed) -join "`t"))
            }
            continue
        }
        Assert-ExactProperties $entry @('skillId','durationMs','samples') 'root motion skill'
        if (@($skillStageDurationsById[$id]).Count -ne 0) {
            throw "A staged skill must carry per-stage root motion: $id"
        }
        $samples = @($entry.samples)
        $packed = Format-RootMotionSamples `
            -Samples $samples -SkillId $id -LimitMs $skillDurationById[$id]
        $rootMotionRows.Add((@(
            'SKILLROOTMOTION', $id, $samples.Count, $packed) -join "`t"))
    }
}

$skillDealsDamageById = @{}
foreach ($skill in @($skillDocument.skills)) {
    $skillDealsDamageById[[string]$skill.skillId] = -not [string]::IsNullOrEmpty([string]$skill.serverDamageProfileId)
}

function Assert-HitShapeExtent([object]$Hit, [string]$SkillId) {
    Assert-JsonInteger $Hit.areaType "hit shape $SkillId areaType" 1 3
    Assert-JsonInteger $Hit.angle "hit shape $SkillId angle" 0 360
    Assert-JsonInteger $Hit.maxTargets "hit shape $SkillId maxTargets" 0 64
    Assert-JsonInteger $Hit.pushMs "hit shape $SkillId pushMs" 0 10000
    foreach ($field in @('range','width','height','offset','inner','pushRange')) {
        Assert-JsonNumber $Hit.$field "hit shape $SkillId $field"
    }
    if ([double]$Hit.pushRange -lt -50.0 -or [double]$Hit.pushRange -gt 50.0 -or
        ([int]$Hit.pushMs -eq 0 -and [double]$Hit.pushRange -ne 0.0)) {
        throw "Hit shape push is invalid: $SkillId"
    }
    # areaType 1 = circle/ring (range, inner), 2 = forward box (range = length,
    # width), 3 = fan (range, angle sweep, inner); the other extents stay 0.
    $areaType = [int]$Hit.areaType
    if ([double]$Hit.range -le 0.0 -or [double]$Hit.inner -ge [double]$Hit.range -or
        [double]$Hit.width -lt 0.0 -or [double]$Hit.height -lt 0.0 -or
        ($areaType -eq 2 -and [double]$Hit.width -le 0.0) -or
        ($areaType -ne 2 -and [double]$Hit.width -ne 0.0) -or
        ($areaType -ne 3 -and [int]$Hit.angle -ne 0) -or
        ($areaType -eq 2 -and [double]$Hit.inner -ne 0.0)) {
        throw "Hit shape extent is invalid: $SkillId"
    }
}

function Format-HitShapeExtent([object]$Hit, [string]$SkillId) {
    return ('{0}:{1}:{2}:{3}:{4}:{5}:{6}:{7}:{8}:{9}' -f [int]$Hit.areaType,
        (Format-InvariantFloat $Hit.range "hit shape $SkillId range"),
        [int]$Hit.angle,
        (Format-InvariantFloat $Hit.width "hit shape $SkillId width"),
        (Format-InvariantFloat $Hit.height "hit shape $SkillId height"),
        (Format-InvariantSignedFloat $Hit.offset "hit shape $SkillId offset"),
        (Format-InvariantFloat $Hit.inner "hit shape $SkillId inner"),
        [int]$Hit.maxTargets,
        [int]$Hit.pushMs,
        (Format-InvariantSignedFloat $Hit.pushRange "hit shape $SkillId pushRange"))
}

$projectileKinds = @('MISSILE','FIXAREA','GRENADE','TRACE')

function Format-Projectiles {
    param(
        [object[]]$Projectiles,
        [string]$SkillId,
        [uint32]$LimitMs
    )

    if ($Projectiles.Count -lt 1 -or $Projectiles.Count -gt 8) {
        throw "Projectile count is invalid: $SkillId"
    }
    $rows = [Collections.Generic.List[string]]::new()
    $previousMs = -1
    $index = 0
    foreach ($projectile in $Projectiles) {
        Assert-ExactProperties $projectile @('timeMs','kind','origin','offsetForward','offsetRight',
            'speed','minDistance','maxDistance','lifeMs','radius','hits') 'projectile'
        Assert-JsonInteger $projectile.timeMs "projectile $SkillId timeMs" 0 $LimitMs
        Assert-JsonInteger $projectile.lifeMs "projectile $SkillId lifeMs" 1 600000
        foreach ($field in @('speed','minDistance','maxDistance','radius','offsetForward','offsetRight')) {
            Assert-JsonNumber $projectile.$field "projectile $SkillId $field"
        }
        $kind = [string]$projectile.kind
        if ($projectileKinds -notcontains $kind) {
            throw "Projectile kind is invalid: $SkillId $kind"
        }
        # CASTER: on the caster, offset forward/right; AIM: where the aim points
        $origin = [string]$projectile.origin
        if ($origin -ne 'CASTER' -and $origin -ne 'AIM') {
            throw "Projectile origin is invalid: $SkillId $origin"
        }
        if ([double]$projectile.offsetForward -lt -50.0 -or [double]$projectile.offsetForward -gt 50.0 -or
            [double]$projectile.offsetRight -lt -50.0 -or [double]$projectile.offsetRight -gt 50.0) {
            throw "Projectile offset is invalid: $SkillId"
        }
        # A fixed area does not move; every other kind needs a speed to travel.
        if (($kind -eq 'FIXAREA') -ne ([double]$projectile.speed -eq 0.0) -or
            [double]$projectile.speed -lt 0.0 -or [double]$projectile.speed -gt 1000.0 -or
            [double]$projectile.minDistance -lt 0.0 -or
            [double]$projectile.maxDistance -lt 0.0 -or
            [double]$projectile.radius -lt 0.0) {
            throw "Projectile motion is invalid: $SkillId"
        }
        $timeMs = [int]$projectile.timeMs
        if ($timeMs -lt $previousMs) {
            throw "Projectile spawn times are out of order: $SkillId"
        }
        $previousMs = $timeMs
        $hits = @($projectile.hits)
        if ($hits.Count -lt 1 -or $hits.Count -gt 16) {
            throw "Projectile hit count is invalid: $SkillId"
        }
        $packed = [Collections.Generic.List[string]]::new()
        $sawTimed = $false
        $previousAtMs = -1
        foreach ($hit in $hits) {
            Assert-ExactProperties $hit @('trigger','atMs','count','everyMs','areaType','range','angle',
                'width','height','offset','inner','maxTargets','pushMs','pushRange') 'projectile hit'
            $trigger = [string]$hit.trigger
            if ($trigger -ne 'CONTACT' -and $trigger -ne 'TIMED') {
                throw "Projectile hit trigger is invalid: $SkillId"
            }
            Assert-JsonInteger $hit.atMs "projectile hit $SkillId atMs" 0 600000
            Assert-JsonInteger $hit.count "projectile hit $SkillId count" 1 64
            Assert-JsonInteger $hit.everyMs "projectile hit $SkillId everyMs" 0 100000
            if (($trigger -eq 'CONTACT' -and [int]$hit.atMs -ne 0) -or
                ([int]$hit.count -gt 1 -and [int]$hit.everyMs -le 0)) {
                throw "Projectile hit schedule is invalid: $SkillId"
            }
            # contact hits lead, then timed hits in schedule order
            if ($trigger -eq 'TIMED') {
                if ([int]$hit.atMs -lt $previousAtMs) {
                    throw "Projectile timed hits are out of order: $SkillId"
                }
                $previousAtMs = [int]$hit.atMs
                $sawTimed = $true
            }
            elseif ($sawTimed) {
                throw "Projectile contact hits must precede timed hits: $SkillId"
            }
            Assert-HitShapeExtent $hit $SkillId
            $triggerCode = 1
            if ($trigger -eq 'CONTACT') { $triggerCode = 0 }
            $packed.Add(('{0}:{1}:{2}:{3}:{4}' -f $triggerCode,
                [int]$hit.atMs, [int]$hit.count, [int]$hit.everyMs,
                (Format-HitShapeExtent $hit $SkillId)))
        }
        $originCode = 1
        if ($origin -eq 'CASTER') { $originCode = 0 }
        $rows.Add((@(
            $index, $timeMs, $kind, $originCode,
            (Format-InvariantSignedFloat $projectile.offsetForward "projectile $SkillId offsetForward"),
            (Format-InvariantSignedFloat $projectile.offsetRight "projectile $SkillId offsetRight"),
            (Format-InvariantFloat $projectile.speed "projectile $SkillId speed"),
            (Format-InvariantFloat $projectile.minDistance "projectile $SkillId minDistance"),
            (Format-InvariantFloat $projectile.maxDistance "projectile $SkillId maxDistance"),
            [int]$projectile.lifeMs,
            (Format-InvariantFloat $projectile.radius "projectile $SkillId radius"),
            $hits.Count, ($packed -join ',')) -join "`t"))
        $index++
    }
    return $rows
}

function Format-HitShapes {
    param(
        [object[]]$Hits,
        [string]$SkillId,
        [uint32]$LimitMs
    )

    if ($Hits.Count -lt 1 -or $Hits.Count -gt 64) {
        throw "Hit shape count is invalid: $SkillId"
    }
    $packed = [Collections.Generic.List[string]]::new()
    $previousMs = -1
    $subHits = 0
    foreach ($hit in $Hits) {
        Assert-ExactProperties $hit @('timeMs','repeatCount','repeatMs','areaType','range','angle',
            'width','height','offset','inner','maxTargets','pushMs','pushRange') 'hit shape'
        Assert-JsonInteger $hit.timeMs "hit shape $SkillId timeMs" 0 $LimitMs
        Assert-JsonInteger $hit.repeatCount "hit shape $SkillId repeatCount" 1 64
        Assert-JsonInteger $hit.repeatMs "hit shape $SkillId repeatMs" 0 100000
        Assert-HitShapeExtent $hit $SkillId
        $timeMs = [int]$hit.timeMs
        if ($timeMs -lt $previousMs) {
            throw "Hit shape times are out of order: $SkillId"
        }
        $previousMs = $timeMs
        if ([int]$hit.repeatCount -gt 1 -and [int]$hit.repeatMs -le 0) {
            throw "Hit shape repeat needs a positive interval: $SkillId"
        }
        $subHits += [int]$hit.repeatCount
        $packed.Add(('{0}:{1}:{2}:{3}' -f $timeMs,
            [int]$hit.repeatCount, [int]$hit.repeatMs, (Format-HitShapeExtent $hit $SkillId)))
    }
    if ($subHits -gt 64) {
        throw "Hit shape sub-hit count exceeds 64: $SkillId"
    }
    return ($packed -join ',')
}

$hitShapeRows = [Collections.Generic.List[string]]::new()
$hitShapeSeen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($path in @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'Data\Animation\HitShapes') `
        -Filter '*.hitshapes.json' -File -ErrorAction SilentlyContinue | Sort-Object Name)) {
    $document = Read-JsonDocument ('Data/Animation/HitShapes/' + $path.Name)
    Assert-ExactProperties $document @(
        'schema','formatVersion','animationAssetId','characterClass','skills') 'hit shape document'
    if ($document.schema -ne 'lostark.animation-hit-shapes' -or
        [uint32]$document.formatVersion -ne 3) {
        throw "Hit shape header is invalid: $($path.Name)"
    }
    foreach ($entry in @($document.skills)) {
        $id = [string]$entry.skillId
        if (-not $skillDurationById.ContainsKey($id)) {
            throw "Hit shapes target an unknown skill: $id"
        }
        if (-not $skillDealsDamageById[$id]) {
            throw "Hit shapes target a skill without a damage profile: $id"
        }
        if (-not $hitShapeSeen.Add($id)) {
            throw "Duplicate hit shape entry: $id"
        }
        if ($null -ne $entry.stages) {
            Assert-ExactProperties $entry @('skillId','stages') 'hit shape skill'
            $stageDurations = @($skillStageDurationsById[$id])
            if ($stageDurations.Count -lt 1) {
                throw "Hit shape stages target a skill without combo stages: $id"
            }
            $seenStages = [Collections.Generic.HashSet[int]]::new()
            foreach ($stage in @($entry.stages)) {
                $stageFields = @('stageIndex','hits')
                if ($null -ne $stage.projectiles) { $stageFields += 'projectiles' }
                Assert-ExactProperties $stage $stageFields 'hit shape stage'
                $stageIndex = [int]$stage.stageIndex
                if ($stageIndex -lt 0 -or $stageIndex -ge $stageDurations.Count -or
                    -not $seenStages.Add($stageIndex)) {
                    throw "Hit shape stage index is invalid or duplicated: $id"
                }
                $hits = @($stage.hits)
                $projectiles = @()
                if ($null -ne $stage.projectiles) { $projectiles = @($stage.projectiles) }
                if ($hits.Count -eq 0 -and $projectiles.Count -eq 0) {
                    throw "Hit shape stage has neither hits nor projectiles: $id"
                }
                if ($hits.Count -gt 0) {
                    $packed = Format-HitShapes -Hits $hits -SkillId $id -LimitMs $stageDurations[$stageIndex]
                    $hitShapeRows.Add((@(
                        'SKILLSTAGEHIT', $id, $stageIndex, $hits.Count, $packed) -join "`t"))
                }
                if ($projectiles.Count -gt 0) {
                    foreach ($row in @(Format-Projectiles -Projectiles $projectiles -SkillId $id `
                            -LimitMs $stageDurations[$stageIndex])) {
                        $hitShapeRows.Add((@('SKILLSTAGEPROJ', $id, $stageIndex, $row) -join "`t"))
                    }
                }
            }
            continue
        }
        $skillFields = @('skillId','hits')
        if ($null -ne $entry.projectiles) { $skillFields += 'projectiles' }
        Assert-ExactProperties $entry $skillFields 'hit shape skill'
        if (@($skillStageDurationsById[$id]).Count -ne 0) {
            throw "A staged skill must carry per-stage hit shapes: $id"
        }
        $hits = @($entry.hits)
        $projectiles = @()
        if ($null -ne $entry.projectiles) { $projectiles = @($entry.projectiles) }
        if ($hits.Count -eq 0 -and $projectiles.Count -eq 0) {
            throw "Hit shape skill has neither hits nor projectiles: $id"
        }
        if ($hits.Count -gt 0) {
            $packed = Format-HitShapes -Hits $hits -SkillId $id -LimitMs $skillDurationById[$id]
            $hitShapeRows.Add((@('SKILLHIT', $id, $hits.Count, $packed) -join "`t"))
        }
        if ($projectiles.Count -gt 0) {
            foreach ($row in @(Format-Projectiles -Projectiles $projectiles -SkillId $id `
                    -LimitMs $skillDurationById[$id])) {
                $hitShapeRows.Add((@('SKILLPROJ', $id, $row) -join "`t"))
            }
        }
    }
}

$rows = @($damageRows + $skillRows + $playerRows + $bossRows + $rootMotionRows + $hitShapeRows + $patternRows | Sort-Object)
$lines = @("LOSTARK_GAMEPLAY_BOOTSTRAP`t10`t$($rows.Count)") + $rows

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
