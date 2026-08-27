[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Gameplay',
    [string]$InputOverlayRoot = '',
    [switch]$SkipValtanSplitProjection
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
. (Join-Path $PSScriptRoot 'Publish-FileTransaction.ps1')
$resolvedInputOverlayRoot = if ([string]::IsNullOrWhiteSpace($InputOverlayRoot)) {
    ''
}
elseif ([IO.Path]::IsPathRooted($InputOverlayRoot)) {
    [IO.Path]::GetFullPath($InputOverlayRoot)
}
else {
    [IO.Path]::GetFullPath((Join-Path $repoRoot $InputOverlayRoot))
}
$hasExplicitOverlay = -not [string]::IsNullOrWhiteSpace($resolvedInputOverlayRoot)
if (-not $hasExplicitOverlay -and -not $SkipValtanSplitProjection) {
    $valtanProjector = Join-Path $repoRoot `
        'Tools\ValtanPipeline\Project-ValtanPatternMaster.ps1'
    & $valtanProjector -Mode ValidateV2 -RepositoryRoot $repoRoot
    if ($LASTEXITCODE -ne 0) {
        throw 'Valtan split Product validation failed.'
    }
}
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [string]::IsNullOrWhiteSpace($resolvedInputOverlayRoot)) {
        $overlayPath = [IO.Path]::GetFullPath(
            (Join-Path $resolvedInputOverlayRoot $RelativePath))
        if ([IO.File]::Exists($overlayPath)) {
            $path = $overlayPath
        }
    }
    if (-not [IO.File]::Exists($path)) { throw "Missing gameplay document: $RelativePath" }
	return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    [string[]]$actual = @($Value.PSObject.Properties.Name)
    [string[]]$expectedSorted = @($Expected)
    [Array]::Sort($actual, [StringComparer]::Ordinal)
    [Array]::Sort($expectedSorted, [StringComparer]::Ordinal)
    if (($actual -join "`n") -cne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId([string]$Value, [string]$Context) {
    if ($Value -notmatch $stableIdPattern) { throw "$Context is not a stable ID: '$Value'" }
}

function Get-Fnv1a32([string]$Text) {
	[uint32]$hash = 2166136261
	foreach ($byte in [Text.Encoding]::UTF8.GetBytes($Text)) {
		$hash = [uint32]($hash -bxor [uint32]$byte)
		$hash = [uint32](([uint64]$hash * [uint64]16777619) -band
			[uint64]4294967295)
	}
	return $hash
}

function Assert-EffectTextureAssetId([string]$Value, [string]$Context) {
	if ([string]::IsNullOrWhiteSpace($Value) -or $Value.Length -gt 260 -or
		-not $Value.StartsWith('Effect/', [StringComparison]::Ordinal) -or
		-not $Value.EndsWith('.dds', [StringComparison]::OrdinalIgnoreCase) -or
		$Value.Contains('\') -or $Value.StartsWith('/') -or
		$Value -match '^[A-Za-z]:' -or @($Value.Split('/')) -contains '..') {
		throw "$Context is not a Resources-relative Effect texture asset ID: '$Value'"
	}
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

function Assert-DisplayText(
	[object]$Value,
	[int]$MaximumBytes,
	[string]$Context) {
	Assert-JsonString $Value $Context
	$text = [string]$Value
	$hasInvalidControl = $false
	foreach ($character in $text.ToCharArray()) {
		if ([int]$character -lt 0x20 -and $character -ne "`t") {
			$hasInvalidControl = $true
			break
		}
	}
	if ([string]::IsNullOrEmpty($text) -or $hasInvalidControl -or
		[Text.Encoding]::UTF8.GetByteCount($text) -gt $MaximumBytes) {
		throw "$Context is not bounded display text."
	}
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

function ConvertTo-CanonicalReceiptNode([object]$Value) {
    if ($null -eq $Value) { return $null }
    if ($Value -is [string] -or $Value -is [bool] -or
        $Value -is [byte] -or $Value -is [sbyte] -or
        $Value -is [int16] -or $Value -is [uint16] -or
        $Value -is [int32] -or $Value -is [uint32] -or
        $Value -is [int64] -or $Value -is [uint64] -or
        $Value -is [single] -or $Value -is [double] -or
        $Value -is [decimal]) {
        return $Value
    }
    if ($Value -is [Collections.IDictionary]) {
        $ordered = [ordered]@{}
        [string[]]$keys = @($Value.Keys | ForEach-Object { [string]$_ })
        [Array]::Sort($keys, [StringComparer]::Ordinal)
        foreach ($key in $keys) {
            $ordered[$key] = ConvertTo-CanonicalReceiptNode $Value[$key]
        }
        return [pscustomobject]$ordered
    }
    if ($Value -is [Collections.IEnumerable] -and $Value -isnot [string]) {
        return @($Value | ForEach-Object { ConvertTo-CanonicalReceiptNode $_ })
    }
    $object = [ordered]@{}
    [string[]]$propertyNames = @($Value.PSObject.Properties.Name)
    [Array]::Sort($propertyNames, [StringComparer]::Ordinal)
    foreach ($propertyName in $propertyNames) {
        $object[$propertyName] = ConvertTo-CanonicalReceiptNode $Value.$propertyName
    }
    return [pscustomobject]$object
}

function ConvertTo-ReceiptValue([object]$Value) {
    if ($null -eq $Value) { return 'null' }
    return ((ConvertTo-CanonicalReceiptNode $Value) |
        ConvertTo-Json -Compress -Depth 32)
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
    [object]$BossDocument,
    [object]$BossPartDocument,
    [object]$CombatObjectDocument) {
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
        'bossProfileCount','bossPartCount','encounterPatternCount',
        'bossCombatObjectCount','fieldEntryCount') 'balance provenance coverage'
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
    $bossPartPath = 'Data/Balance/ValtanBossParts.json'
    $bossPartRoot = "boss-parts:$($BossPartDocument.bossArchetypeId)"
    Add-Expected $bossPartPath $bossPartRoot 'bossArchetypeId' `
        $BossPartDocument.bossArchetypeId
    Add-Expected $bossPartPath $bossPartRoot 'parts.length' `
        @($BossPartDocument.parts).Count
    foreach ($part in @($BossPartDocument.parts)) {
        foreach ($property in $part.PSObject.Properties) {
            Add-Expected $bossPartPath "boss-part:$($part.partId)" `
                $property.Name $property.Value
        }
    }
	$combatObjectPath = 'Data/Encounters/Valtan/ValtanCombatObjects.json'
	$combatObjectRoot = "combat-objects:$($CombatObjectDocument.encounterId)"
	Add-Expected $combatObjectPath $combatObjectRoot 'encounterId' `
		$CombatObjectDocument.encounterId
	Add-Expected $combatObjectPath $combatObjectRoot 'objects.length' `
		@($CombatObjectDocument.objects).Count
	foreach ($combatObject in @($CombatObjectDocument.objects)) {
		foreach ($property in $combatObject.PSObject.Properties) {
			Add-Expected $combatObjectPath `
				"combat-object:$($combatObject.combatObjectArchetypeId)" `
				$property.Name $property.Value
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
    $liveEncounterPatterns = @($encounter.patterns | Where-Object {
        [string]$_.selectionMode -cne 'AUDITION_ONLY'
    })
    Add-Expected 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget 'patterns.length' @($encounter.patterns).Count
    for ($index = 0; $index -lt $liveEncounterPatterns.Count; $index++) {
        $pattern = $liveEncounterPatterns[$index]
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
        [uint32]$receipt.coverage.bossPartCount -ne @($BossPartDocument.parts).Count -or
        [uint32]$receipt.coverage.bossCombatObjectCount -ne @($CombatObjectDocument.objects).Count -or
        [uint32]$receipt.coverage.encounterPatternCount -ne $liveEncounterPatterns.Count) {
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
Assert-JsonInteger $skillDocument.formatVersion 'skill document formatVersion' 3 3
if ($skillDocument.schema -ne 'lostark.player-skills' -or $skillDocument.formatVersion -ne 3) {
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
$claimedStandupSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$skillRows = [Collections.Generic.List[string]]::new()
foreach ($skill in @($skillDocument.skills)) {
	Assert-ExactProperties $skill @(
		'skillId','characterClass','inputSlot','displayName','actionId','skillKind','requiredStance','setsStance',
		'cooldownMs','actionDurationMs',
        'hitTimeMs','resourceCost','identityCost','movementDistance','maximumRange','serverDamageProfileId',
		'staggerDamage','partDamage','counterPower','effectId','comboStages') 'player skill'
	Assert-JsonInteger $skill.skillId 'skillId' 1 ([uint32]::MaxValue)
	foreach ($stringField in @('characterClass','inputSlot','displayName','actionId','skillKind','serverDamageProfileId',
		'effectId','requiredStance','setsStance')) {
		Assert-JsonString $skill.$stringField "skill $($skill.skillId) $stringField"
	}
	if ($skill.requiredStance -notin $knownStances -or $skill.setsStance -notin $knownStances) {
		throw "Skill stance is unknown: $($skill.skillId)"
	}
	foreach ($integerField in @(
		'cooldownMs','actionDurationMs','hitTimeMs','resourceCost','identityCost',
		'staggerDamage','partDamage','counterPower')) {
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
	# Initial boss-interaction tuning is deliberately per landed damaging hit.
	# Multi-hit skills therefore contribute once for each hit that the Server
	# actually admits, rather than receiving a hidden whole-skill multiplier.
	$expectedStaggerDamage = if ($dealsDamage) { [uint32]10 } else { [uint32]0 }
	$expectedPartDamage = if ($dealsDamage) { [uint32]100 } else { [uint32]0 }
	$expectedCounterPower = if ([uint32]$skill.skillId -eq 34580) {
		[uint32]1
	} else {
		[uint32]0
	}
	if ([uint32]$skill.staggerDamage -ne $expectedStaggerDamage -or
		[uint32]$skill.partDamage -ne $expectedPartDamage -or
		[uint32]$skill.counterPower -ne $expectedCounterPower -or
		([uint32]$skill.counterPower -gt 0 -and
		 [string]$skill.skillKind -cne 'COUNTER')) {
		throw "Player skill boss combat traits violate the deterministic landed-hit policy: $($skill.skillId)"
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
    # A STANDUP skill shares its physical key with the slot's normal skill but is
    # gated by the KNOCKDOWN action instead of a stance, so it claims its own
    # per-slot domain rather than a stance.
    if ([string]$skill.skillKind -eq 'STANDUP') {
        if ($id -eq 0 -or -not $skillIds.Add($id) -or
            -not $claimedStandupSlots.Add($slotKey)) {
            throw "Duplicate skill ID or input slot: $id"
        }
    }
    else {
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
    }
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
	if ($skillKind -notin @('ACTIVE','COMBO','HOLD','COUNTER','STANDUP')) {
		throw "Unknown skillKind: $id $skillKind"
	}
	if ($skillKind -eq 'STANDUP' -and (
		$dealsDamage -or @($skill.comboStages).Count -ne 0 -or
		[uint32]$skill.cooldownMs -eq 0 -or
		[string]$skill.requiredStance -ne 'NONE' -or
		[string]$skill.setsStance -ne 'NONE')) {
		throw "STANDUP skill contract is invalid: $id"
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
				'actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs') 'counter stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.comboAdvanceMs -or
				[uint32]$stage.comboAdvanceMs -gt [uint32]$stage.actionDurationMs) {
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
				'actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs') 'hold stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.comboAdvanceMs -or
				[uint32]$stage.comboAdvanceMs -gt [uint32]$stage.actionDurationMs -or
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
	elseif ($skillKind -eq 'ACTIVE' -or $skillKind -eq 'STANDUP') {
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
				'actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs') 'combo stage'
			foreach ($stageField in @('actionDurationMs','hitTimeMs','comboAdvanceMs','inputOpenMs','inputCloseMs')) {
				Assert-JsonInteger $stage.$stageField "skill $id stage $stageIndex $stageField" 0 ([uint32]::MaxValue)
			}
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.comboAdvanceMs -or
				[uint32]$stage.comboAdvanceMs -gt [uint32]$stage.actionDurationMs) {
				throw "Combo stage timing is invalid: $id stage $stageIndex"
			}
			if ($stageIndex -eq $stages.Count - 1) {
				if ([uint32]$stage.comboAdvanceMs -ne [uint32]$stage.actionDurationMs -or
					[uint32]$stage.inputOpenMs -ne 0 -or [uint32]$stage.inputCloseMs -ne 0) {
					throw "Final combo stage must not open an input window: $id"
				}
			}
			else {
				$automaticStage = [uint32]$stage.inputOpenMs -eq 0 -and
					[uint32]$stage.inputCloseMs -eq 0
				if ($automaticStage) {
					if ([uint32]$stage.comboAdvanceMs -ne
						[uint32]$stage.actionDurationMs) {
						throw "Automatic combo stage must advance after its full motion: $id stage $stageIndex"
					}
				}
				elseif ([uint32]$stage.inputOpenMs -ge
						[uint32]$stage.inputCloseMs -or
					[uint32]$stage.inputCloseMs -gt
						[uint32]$stage.actionDurationMs) {
					throw "Combo input window is invalid: $id stage $stageIndex"
				}
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
	$skillRows.Add((@(
		'SKILLCOMBATTRAITS', $id, [uint32]$skill.staggerDamage,
		[uint32]$skill.partDamage, [uint32]$skill.counterPower) -join "`t"))
	for ($stageIndex = 0; $stageIndex -lt $stages.Count; $stageIndex++) {
		$stage = $stages[$stageIndex]
		$skillRows.Add((@(
			'SKILLSTAGE', $id, $stageIndex,
			[uint32]$stage.actionDurationMs, [uint32]$stage.hitTimeMs,
			[uint32]$stage.comboAdvanceMs,
			[uint32]$stage.inputOpenMs, [uint32]$stage.inputCloseMs) -join "`t"))
	}
}

# Ground targeting is a separate typed contract so the official PlayerSkills
# values and their provenance receipt remain byte-for-byte unchanged. The Server
# bootstrap receives only admission fields; texture IDs stay Client-only.
$targetingDocument = Read-JsonDocument 'Data/Balance/PlayerSkillTargeting.json'
Assert-ExactProperties $targetingDocument @(
	'schema','formatVersion','skills') 'player skill targeting document'
Assert-JsonString $targetingDocument.schema 'player skill targeting schema'
Assert-JsonInteger $targetingDocument.formatVersion `
	'player skill targeting formatVersion' 1 1
if ([string]$targetingDocument.schema -cne 'lostark.player-skill-targeting' -or
	[uint32]$targetingDocument.formatVersion -ne 1 -or
	$targetingDocument.skills -isnot [Array]) {
	throw 'Player skill targeting header is invalid.'
}
$targetingSkillIds = [Collections.Generic.HashSet[uint32]]::new()
foreach ($targeting in @($targetingDocument.skills)) {
	Assert-ExactProperties $targeting @(
		'skillId','targetingKind','maximumRange','requiresWalkable',
		'rangePreview','targetPreview') 'player skill targeting row'
	Assert-JsonInteger $targeting.skillId 'targeting skillId' 1 ([uint32]::MaxValue)
	Assert-JsonString $targeting.targetingKind `
		"targeting $($targeting.skillId) targetingKind"
	Assert-JsonNumber $targeting.maximumRange `
		"targeting $($targeting.skillId) maximumRange"
	if ($targeting.requiresWalkable -isnot [bool]) {
		throw "Targeting requiresWalkable must be a JSON Boolean: $($targeting.skillId)"
	}
	$targetingId = [uint32]$targeting.skillId
	$ownerSkills = @($skillDocument.skills | Where-Object {
		[uint32]$_.skillId -eq $targetingId })
	if (-not $targetingSkillIds.Add($targetingId) -or
		$ownerSkills.Count -ne 1 -or
		[string]$targeting.targetingKind -cne 'GROUND_POINT' -or
		-not [bool]$targeting.requiresWalkable -or
		[double]$targeting.maximumRange -le 0.0 -or
		[double]$targeting.maximumRange -ne [double]$ownerSkills[0].maximumRange -or
		[string]$ownerSkills[0].skillKind -cne 'ACTIVE') {
		throw "Player skill targeting owner, kind, range, or walkability is invalid: $targetingId"
	}
	foreach ($previewName in @('rangePreview','targetPreview')) {
		$preview = $targeting.$previewName
		Assert-ExactProperties $preview @(
			'assetId','diameter','coverageChannel','validTint','invalidTint','assetIdentityBasis',
			'usageBasis','sourceEvidence') "targeting $targetingId $previewName"
		foreach ($field in @(
			'assetId','coverageChannel','assetIdentityBasis','usageBasis',
			'sourceEvidence')) {
			Assert-JsonString $preview.$field `
				"targeting $targetingId $previewName $field"
		}
		Assert-JsonNumber $preview.diameter `
			"targeting $targetingId $previewName diameter"
		Assert-EffectTextureAssetId ([string]$preview.assetId) `
			"targeting $targetingId $previewName assetId"
		foreach ($tintName in @('validTint','invalidTint')) {
			$tint = @($preview.$tintName)
			if ($preview.$tintName -isnot [Array] -or $tint.Count -ne 4) {
				throw "Targeting preview tint must contain RGBA: $targetingId/$previewName/$tintName"
			}
			foreach ($component in $tint) {
				Assert-JsonNumber $component `
					"targeting $targetingId $previewName $tintName"
				if ([double]$component -lt 0.0 -or [double]$component -gt 1.0) {
					throw "Targeting preview tint is outside [0,1]: $targetingId/$previewName/$tintName"
				}
			}
		}
		if ([double]$preview.diameter -le 0.0 -or
			[string]$preview.coverageChannel -cne 'R' -or
			[string]$preview.usageBasis -cne 'PROJECT_TUNED' -or
			[string]$preview.assetIdentityBasis -notin @(
				'SOURCE_EXTRACTED','RUNTIME_RESOURCE')) {
			throw "Player skill targeting preview semantics are invalid: $targetingId/$previewName"
		}
		$evidence = [string]$preview.sourceEvidence
		if ([string]$preview.assetIdentityBasis -ceq 'SOURCE_EXTRACTED') {
			$evidencePath = [IO.Path]::GetFullPath((Join-Path $repoRoot $evidence))
			if (-not $evidence.StartsWith('Data/', [StringComparison]::Ordinal) -or
				@($evidence.Split('/')) -contains '..' -or
				-not [IO.File]::Exists($evidencePath)) {
				throw "SOURCE_EXTRACTED targeting preview lacks checked-in evidence: $targetingId/$previewName"
			}
			$evidenceText = [IO.File]::ReadAllText($evidencePath)
			$assetFieldPattern = '"assetId"\s*:\s*"' +
				[Regex]::Escape([string]$preview.assetId) + '"'
			if (-not [Regex]::IsMatch(
					$evidenceText, $assetFieldPattern,
					[Text.RegularExpressions.RegexOptions]::CultureInvariant)) {
				throw "SOURCE_EXTRACTED targeting preview evidence does not own assetId: $targetingId/$previewName"
			}
		}
		elseif ($evidence.Length -ne 0) {
			throw "RUNTIME_RESOURCE targeting preview must not claim source evidence: $targetingId/$previewName"
		}
	}
	if ([double]$targeting.rangePreview.diameter -ne
		(2.0 * [double]$targeting.maximumRange)) {
		throw "Targeting range preview diameter must be twice maximumRange: $targetingId"
	}
	$skillRows.Add((@(
		'SKILLTARGET', $targetingId, [string]$targeting.targetingKind,
		(Format-InvariantFloat $targeting.maximumRange `
			"targeting $targetingId maximumRange"),
		$(if ([bool]$targeting.requiresWalkable) { 1 } else { 0 })) -join "`t"))
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
Assert-JsonInteger $bossDocument.formatVersion 'boss document formatVersion' 4 4
if ($bossDocument.schema -ne 'lostark.boss-profiles' -or $bossDocument.formatVersion -ne 4) {
    throw 'Boss profile header is invalid.'
}
$bossIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$bossRows = [Collections.Generic.List[string]]::new()
foreach ($boss in @($bossDocument.bosses)) {
	Assert-ExactProperties $boss @(
		'archetypeId','encounterId','displayName','maximumHp','maximumHealthBars','attackPower','collisionRadius',
		'engageDistance','moveSpeed','phasePolicy','armorPlates') 'boss profile'
	foreach ($stringField in @('archetypeId','encounterId','displayName')) {
		Assert-JsonString $boss.$stringField "boss $stringField"
	}
	Assert-JsonInteger $boss.maximumHp 'boss maximumHp' 1 ([uint32]::MaxValue)
	Assert-JsonInteger $boss.maximumHealthBars 'boss maximumHealthBars' 1 1000
	Assert-JsonInteger $boss.attackPower 'boss attackPower' 1 ([uint32]::MaxValue)
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
	if ($null -eq $boss.phasePolicy -or
		$boss.phasePolicy -isnot [pscustomobject]) {
		throw "Boss phasePolicy must be a tagged object: $($boss.archetypeId)"
	}
	Assert-JsonString $boss.phasePolicy.kind 'boss phase policy kind'
	$phasePolicyKind = [string]$boss.phasePolicy.kind
	$phasePolicyThreshold = [uint32]0
	if ($phasePolicyKind -ceq 'HEALTH_PERCENT_THRESHOLD') {
		Assert-ExactProperties $boss.phasePolicy @(
			'kind','thresholdPercent') 'boss health-percent phase policy'
		Assert-JsonInteger $boss.phasePolicy.thresholdPercent `
			'boss phase policy thresholdPercent' 1 99
		$phasePolicyThreshold = [uint32]$boss.phasePolicy.thresholdPercent
	}
	elseif ($phasePolicyKind -ceq 'AUTHORED_PATTERN_EVENT') {
		Assert-ExactProperties $boss.phasePolicy @('kind') `
			'boss authored-event phase policy'
	}
	else {
		throw "Boss phase policy kind is invalid: $($boss.archetypeId)"
	}
	if ([string]$boss.archetypeId -ceq 'BOSS_VALTAN' -and
		([string]$boss.encounterId -cne 'ENCOUNTER_VALTAN' -or
		 $phasePolicyKind -cne 'AUTHORED_PATTERN_EVENT')) {
		throw 'BOSS_VALTAN must use the ENCOUNTER_VALTAN authored-event phase policy.'
	}
	if (-not $bossIds.Add([string]$boss.archetypeId) -or [uint32]$boss.maximumHp -eq 0 -or
		[uint32]$boss.maximumHealthBars -eq 0 -or
        [uint32]$boss.attackPower -eq 0 -or [double]$boss.collisionRadius -le 0.0 -or
		[double]$boss.engageDistance -le 0.0 -or [double]$boss.moveSpeed -le 0.0) {
        throw "Boss profile is invalid: $($boss.archetypeId)"
    }
    $bossRows.Add((@(
        'BOSS', $boss.archetypeId, $boss.encounterId, [uint32]$boss.maximumHp,
		[uint32]$boss.maximumHealthBars,
        [uint32]$boss.attackPower,
        (Format-InvariantFloat $boss.collisionRadius 'boss collisionRadius'),
        (Format-InvariantFloat $boss.engageDistance 'boss engageDistance'),
        (Format-InvariantFloat $boss.moveSpeed 'boss moveSpeed'),
		$phasePolicyKind, $phasePolicyThreshold) -join "`t"))
	# A plate is a destructible piece of this boss: it mitigates while intact and
	# only loses durability inside a GROGGY stage. plateIndex is also the client
	# part order, so it must stay dense and start at zero.
	$plates = @($boss.armorPlates)
	if ($plates.Count -gt 4) {
		throw "Boss declares more armour plates than the client can wear: $($boss.archetypeId)"
	}
	for ($plateIndex = 0; $plateIndex -lt $plates.Count; $plateIndex++) {
		$plate = $plates[$plateIndex]
		Assert-ExactProperties $plate @('plateIndex','durability','defense') 'boss armour plate'
		Assert-JsonInteger $plate.plateIndex 'boss armour plateIndex' 0 3
		Assert-JsonInteger $plate.durability 'boss armour durability' 1 ([uint32]::MaxValue)
		Assert-JsonInteger $plate.defense 'boss armour defense' 1 10000
		if ([uint32]$plate.plateIndex -ne $plateIndex) {
			throw "Boss armour plateIndex must be dense and ordered: $($boss.archetypeId)"
		}
		$bossRows.Add((@(
			'BOSSARMOR', $boss.archetypeId, [uint32]$plate.plateIndex,
			[uint32]$plate.durability, [uint32]$plate.defense) -join "`t"))
	}
}

$bossPartDocument = Read-JsonDocument 'Data/Balance/ValtanBossParts.json'
Assert-ExactProperties $bossPartDocument @(
	'schema','formatVersion','bossArchetypeId','parts') 'Valtan boss part document'
Assert-JsonString $bossPartDocument.schema 'Valtan boss part schema'
Assert-JsonInteger $bossPartDocument.formatVersion `
	'Valtan boss part formatVersion' 1 1
Assert-JsonString $bossPartDocument.bossArchetypeId `
	'Valtan boss part bossArchetypeId'
Assert-StableId $bossPartDocument.bossArchetypeId `
	'Valtan boss part bossArchetypeId'
if ([string]$bossPartDocument.schema -cne 'lostark.valtan-boss-parts' -or
	[uint32]$bossPartDocument.formatVersion -ne 1 -or
	-not $bossIds.Contains([string]$bossPartDocument.bossArchetypeId) -or
	$bossPartDocument.parts -isnot [Array] -or
	@($bossPartDocument.parts).Count -lt 1 -or
	@($bossPartDocument.parts).Count -gt 32) {
	throw 'Valtan boss part header or boss owner is invalid.'
}
$bossPartIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$bossPartMasks = [Collections.Generic.HashSet[uint32]]::new()
$bossPartRows = [Collections.Generic.List[string]]::new()
$totalPartDamageReductionPercent = [uint32]0
foreach ($part in @($bossPartDocument.parts)) {
	Assert-ExactProperties $part @(
		'partId','stateMask','maximumDurability','damageReductionPercent',
		'partDamageCondition') 'Valtan boss part'
	foreach ($field in @('partId','partDamageCondition')) {
		Assert-JsonString $part.$field "Valtan boss part $field"
	}
	Assert-StableId $part.partId 'Valtan boss partId'
	Assert-JsonInteger $part.stateMask `
		"Valtan boss part $($part.partId) stateMask" 1 ([uint32]::MaxValue)
	Assert-JsonInteger $part.maximumDurability `
		"Valtan boss part $($part.partId) maximumDurability" 1 1000000
	Assert-JsonInteger $part.damageReductionPercent `
		"Valtan boss part $($part.partId) damageReductionPercent" 1 90
	$stateMask = [uint32]$part.stateMask
	if (($stateMask -band ($stateMask - 1)) -ne 0 -or
		-not $bossPartIds.Add([string]$part.partId) -or
		-not $bossPartMasks.Add($stateMask) -or
		[string]$part.partDamageCondition -cne 'GROGGY_ONLY') {
		throw "Valtan boss part identity, mask, or condition is invalid: $($part.partId)"
	}
	$totalPartDamageReductionPercent += [uint32]$part.damageReductionPercent
	$bossPartRows.Add((@(
		'BOSSPART', $bossPartDocument.bossArchetypeId, $part.partId,
		$stateMask, [uint32]$part.maximumDurability,
		[uint32]$part.damageReductionPercent,
		[string]$part.partDamageCondition) -join "`t"))
}
if ($totalPartDamageReductionPercent -ge 100) {
	throw 'Valtan boss part damage reductions must total less than 100 percent.'
}

# Presentation owns modelAssetId, gameplay owns durability/reduction. The two
# authoring documents must still describe the exact same stable part identity
# and bit, while no model path crosses into the Server bootstrap.
$bossCatalogDocument = Read-JsonDocument 'Data/Actors/BossCatalog.json'
Assert-ExactProperties $bossCatalogDocument @(
	'schema','formatVersion','bosses') 'boss presentation catalog'
Assert-JsonString $bossCatalogDocument.schema 'boss presentation catalog schema'
Assert-JsonInteger $bossCatalogDocument.formatVersion `
	'boss presentation catalog formatVersion' 4 4
if ([string]$bossCatalogDocument.schema -cne 'lostark.boss-catalog' -or
	[uint32]$bossCatalogDocument.formatVersion -ne 4 -or
	$bossCatalogDocument.bosses -isnot [Array]) {
	throw 'Boss presentation catalog header is invalid.'
}
foreach ($presentationBoss in @($bossCatalogDocument.bosses)) {
	Assert-ExactProperties $presentationBoss @(
		'archetypeId','visualAssetId','presentationScale','bodyModel',
		'weaponModel','armorModels','armorParts','combatObjectVisuals',
		'animationSetId','serverProfileId','clientPresentationId',
		'presentationStatus','presentationClips') 'boss presentation row'
	Assert-JsonString $presentationBoss.archetypeId `
		'boss presentation archetypeId'
	Assert-StableId $presentationBoss.archetypeId `
		'boss presentation archetypeId'
	Assert-JsonNumber $presentationBoss.presentationScale `
		"boss presentation $($presentationBoss.archetypeId) presentationScale"
	$presentationScale = [double]$presentationBoss.presentationScale
	if ($presentationScale -le 0.0 -or $presentationScale -gt 100.0) {
		throw "Boss presentation scale is out of range: $($presentationBoss.archetypeId)"
	}
	Assert-ExactProperties $presentationBoss.presentationClips @(
		'idle','chase','patternWindup','patternActive','patternRecovery','dead') `
		"boss presentation $($presentationBoss.archetypeId) clips"
}
$bossCatalogOwners = @($bossCatalogDocument.bosses | Where-Object {
	[string]$_.archetypeId -ceq [string]$bossPartDocument.bossArchetypeId })
if ($bossCatalogOwners.Count -ne 1 -or
	$bossCatalogOwners[0].armorParts -isnot [Array] -or
	@($bossCatalogOwners[0].armorParts).Count -ne @($bossPartDocument.parts).Count -or
	$bossCatalogOwners[0].combatObjectVisuals -isnot [Array] -or
	@($bossCatalogOwners[0].combatObjectVisuals).Count -ne 2) {
	throw 'Valtan boss part presentation owner or count does not match gameplay.'
}
$presentationPartById = @{}
foreach ($presentationPart in @($bossCatalogOwners[0].armorParts)) {
	Assert-ExactProperties $presentationPart @(
		'partId','stateMask','modelAssetId') 'boss presentation armor part'
	Assert-JsonString $presentationPart.partId 'boss presentation armor partId'
	Assert-StableId $presentationPart.partId 'boss presentation armor partId'
	Assert-JsonInteger $presentationPart.stateMask `
		"boss presentation armor part $($presentationPart.partId) stateMask" `
		1 ([uint32]::MaxValue)
	Assert-JsonString $presentationPart.modelAssetId `
		"boss presentation armor part $($presentationPart.partId) modelAssetId"
	$modelAssetId = [string]$presentationPart.modelAssetId
	if ($presentationPartById.ContainsKey([string]$presentationPart.partId) -or
		[string]::IsNullOrWhiteSpace($modelAssetId) -or
		$modelAssetId.Length -gt 260 -or $modelAssetId.Contains('\') -or
		$modelAssetId.StartsWith('/') -or $modelAssetId -match '^[A-Za-z]:' -or
		@($modelAssetId.Split('/')) -contains '..') {
		throw "Boss presentation armor part is invalid: $($presentationPart.partId)"
	}
	$presentationPartById[[string]$presentationPart.partId] = $presentationPart
}
foreach ($part in @($bossPartDocument.parts)) {
	$presentationPart = $presentationPartById[[string]$part.partId]
	if ($null -eq $presentationPart -or
		[uint32]$presentationPart.stateMask -ne [uint32]$part.stateMask) {
		throw "Valtan boss part presentation join is invalid: $($part.partId)"
	}
}
$combatObjectVisualByArchetypeId = @{}
foreach ($visual in @($bossCatalogOwners[0].combatObjectVisuals)) {
	Assert-ExactProperties $visual @(
		'combatObjectArchetypeId','clientVisualId','effectAssetId') `
		'boss combat object visual'
	foreach ($field in @(
		'combatObjectArchetypeId','clientVisualId','effectAssetId')) {
		Assert-JsonString $visual.$field "boss combat object visual $field"
		Assert-StableId $visual.$field "boss combat object visual $field"
	}
	if ($combatObjectVisualByArchetypeId.ContainsKey(
		[string]$visual.combatObjectArchetypeId)) {
		throw "Duplicate boss combat object visual: $($visual.combatObjectArchetypeId)"
	}
	$combatObjectVisualByArchetypeId[
		[string]$visual.combatObjectArchetypeId] = $visual
}

$combatObjectDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanCombatObjects.json'
Assert-ExactProperties $combatObjectDocument @(
	'schema','formatVersion','encounterId','objects') `
	'Valtan combat object document'
Assert-JsonString $combatObjectDocument.schema 'Valtan combat object schema'
Assert-JsonInteger $combatObjectDocument.formatVersion `
	'Valtan combat object formatVersion' 1 1
Assert-JsonString $combatObjectDocument.encounterId `
	'Valtan combat object encounterId'
Assert-StableId $combatObjectDocument.encounterId `
	'Valtan combat object encounterId'
if ([string]$combatObjectDocument.schema -cne `
	'lostark.valtan-combat-objects' -or
	[uint32]$combatObjectDocument.formatVersion -ne 1 -or
	$combatObjectDocument.objects -isnot [Array] -or
	@($combatObjectDocument.objects).Count -ne 2) {
	throw 'Valtan combat object header or first-slice object count is invalid.'
}

$encounterDocument = Read-JsonDocument 'Data/Encounters/Valtan/ValtanEncounter.json'
Assert-ExactProperties $encounterDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','authority','fixedTickHz',
	'introPatternId','states','patterns') 'encounter document'
Assert-StableId $encounterDocument.introPatternId 'encounter introPatternId'
Assert-JsonString $encounterDocument.schema 'encounter schema'
Assert-JsonInteger $encounterDocument.formatVersion 'encounter formatVersion' 4 4
Assert-JsonString $encounterDocument.encounterId 'encounterId'
Assert-JsonString $encounterDocument.bossArchetypeId 'encounter bossArchetypeId'
Assert-JsonString $encounterDocument.authority 'encounter authority'
Assert-JsonInteger $encounterDocument.fixedTickHz 'encounter fixedTickHz' 30 30
Assert-StableId $encounterDocument.encounterId 'encounterId'
Assert-StableId $encounterDocument.bossArchetypeId 'encounter bossArchetypeId'
if ($encounterDocument.schema -ne 'lostark.encounter-profile' -or
	$encounterDocument.formatVersion -ne 4 -or
	$encounterDocument.authority -ne 'server' -or
	[uint32]$encounterDocument.fixedTickHz -ne 30 -or
	-not $bossIds.Contains([string]$encounterDocument.bossArchetypeId)) {
	throw 'Valtan encounter header or boss reference is invalid.'
}
if ([string]$combatObjectDocument.encounterId -cne `
	[string]$encounterDocument.encounterId) {
	throw 'Valtan combat object encounter owner does not match the encounter.'
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
$healthBarTriggerKeys =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$coveredSourceActionIds = [Collections.Generic.HashSet[uint32]]::new()
$sourceTimingByActionId = Read-ValtanSkillTiming
$patternRows = [Collections.Generic.List[string]]::new()
$serverMotionAnchorIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$serverMotionByPatternId = @{}
$patternById = @{}
$stageOwnerByKey = @{}
# Baked b_root travel is addressed by pattern and stage index, so the loop
# below records the gameplay duration each curve has to stay inside.
$patternStageDurationByKey = @{}
$spawnCombatObjectOwnerById = @{}
$gameplayPhaseActionCount = 0
$hasExactValtanArenaBreakPhaseAction = $false
$valtanHighJumpTypedVolleyCount = 0
$hasExactValtanHighJumpTypedVolleyOwner = $false
foreach ($pattern in @($encounterDocument.patterns)) {
	# serverMotion is optional. Only a pattern whose boss motion the Server has to
	# compute itself, like the 109 leap to its landing anchor, carries one.
	$patternProperties = @(
		'patternId','displayName','actionId','sourceActionIds','selectionMode',
		'category','minimumPhase','maximumPhase','targetPolicy','aimPolicy',
		'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
		'armorRequirement','phaseRequirement','invulnerableWhileRunning',
		'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
		'stages')
	$hasServerMotion = $null -ne $pattern.PSObject.Properties['serverMotion']
	if ($hasServerMotion) { $patternProperties += 'serverMotion' }
	Assert-ExactProperties $pattern $patternProperties 'encounter pattern'
	foreach ($field in @(
		'patternId','displayName','actionId','selectionMode','category',
		'targetPolicy','aimPolicy')) {
		Assert-JsonString $pattern.$field "pattern $($pattern.patternId) $field"
	}
	foreach ($field in @('minimumHealthBar','maximumHealthBar','triggerHealthBar',
		'triggerOrder','selectionWeight','maximumConsecutiveUses',
		'minimumPhase','maximumPhase')) {
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
	$patternById[[string]$pattern.patternId] = $pattern
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
	$category = [string]$pattern.category
	$targetPolicy = [string]$pattern.targetPolicy
	$aimPolicy = [string]$pattern.aimPolicy
	$targetNeedsNoTarget = $targetPolicy -eq 'NONE'
	$aimNeedsNoTarget = $aimPolicy -in @('NONE','FACE_MOTION_ANCHOR')
	$targetTracks = $targetPolicy -eq 'NEAREST_EACH_TICK'
	$aimTracks = $aimPolicy -eq 'TRACK_TARGET_EACH_TICK'
	$targetLocks = $targetPolicy -in @(
		'LOCK_NEAREST_ON_START','LOCK_RANDOM_ALIVE_ON_START',
		'LOCK_RANDOM_ALIVE_BEHIND_ON_START')
	$aimLocks = $aimPolicy -eq 'LOCK_FACING_ON_START'
	$policyIsCoherent =
		($targetNeedsNoTarget -and $aimNeedsNoTarget) -or
		($targetTracks -and $aimTracks) -or
		($targetLocks -and ($aimLocks -or $aimTracks))
	if ($category -notin @('NORMAL','IMPORTANT','MECHANIC') -or
		[uint32]$pattern.minimumPhase -lt 1 -or
		[uint32]$pattern.maximumPhase -lt [uint32]$pattern.minimumPhase -or
		[uint32]$pattern.maximumPhase -gt 3 -or
		$targetPolicy -notin @(
			'NONE','NEAREST_EACH_TICK','LOCK_NEAREST_ON_START',
			'LOCK_RANDOM_ALIVE_ON_START',
			'LOCK_RANDOM_ALIVE_BEHIND_ON_START') -or
		$aimPolicy -notin @(
			'NONE','TRACK_TARGET_EACH_TICK','LOCK_FACING_ON_START',
			'FACE_MOTION_ANCHOR') -or
		-not $policyIsCoherent) {
		throw "Pattern category, phase, target, or aim policy is invalid: $($pattern.patternId)"
	}
	Assert-JsonString $pattern.armorRequirement "pattern armorRequirement"
	$armorRequirement = [string]$pattern.armorRequirement
	if ($armorRequirement -cnotin @('ANY','ARMORED','STRIPPED')) {
		throw "Pattern armorRequirement is invalid: $($pattern.patternId)"
	}
	if ($selectionMode -ne 'NORMAL' -and $armorRequirement -cne 'ANY') {
		throw "Only a weighted pattern may require an armour state: $($pattern.patternId)"
	}
	Assert-JsonString $pattern.phaseRequirement "pattern phaseRequirement"
	$phaseRequirement = [string]$pattern.phaseRequirement
	if ($phaseRequirement -cnotin @('ANY','PHASE_ONE','PHASE_TWO')) {
		throw "Pattern phaseRequirement is invalid: $($pattern.patternId)"
	}
	if ($selectionMode -eq 'AUDITION_ONLY' -and $phaseRequirement -cne 'ANY') {
		throw "An audition-only pattern cannot require a gameplay phase: $($pattern.patternId)"
	}
	if ($pattern.invulnerableWhileRunning -isnot [bool]) {
		throw "Pattern invulnerableWhileRunning must be a JSON Boolean: $($pattern.patternId)"
	}
	$invulnerable = $(if ([bool]$pattern.invulnerableWhileRunning) { 1 } else { 0 })
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
		$hasNoHealthWindow =
			[uint32]$pattern.minimumHealthBar -eq 0 -and
			[uint32]$pattern.maximumHealthBar -eq 0
		$hasValidHealthWindow =
			[uint32]$pattern.minimumHealthBar -ge 1 -and
			[uint32]$pattern.maximumHealthBar -ge [uint32]$pattern.minimumHealthBar -and
			[uint32]$pattern.maximumHealthBar -le $maximumHealthBars
		if ((-not $hasNoHealthWindow -and -not $hasValidHealthWindow) -or
			[uint32]$pattern.triggerHealthBar -lt 1 -or
			[uint32]$pattern.triggerHealthBar -gt $maximumHealthBars -or
			[uint32]$pattern.triggerOrder -lt 1 -or
			[uint32]$pattern.selectionWeight -ne 0) {
			throw "Health-bar pattern selection fields are invalid: $($pattern.patternId)"
		}
		$triggerKey = "$( [uint32]$pattern.triggerHealthBar )/$( [uint32]$pattern.triggerOrder )"
		if (-not $healthBarTriggerKeys.Add($triggerKey)) {
			throw "Health-bar pattern trigger tuple is duplicated: $triggerKey"
		}
	}
	elseif ($selectionMode -eq 'AUDITION_ONLY') {
		if ([uint32]$pattern.minimumHealthBar -ne 0 -or
			[uint32]$pattern.maximumHealthBar -ne 0 -or
			[uint32]$pattern.triggerHealthBar -ne 0 -or
			[uint32]$pattern.triggerOrder -ne 0 -or
			[uint32]$pattern.selectionWeight -ne 0 -or
			[uint32]$pattern.maximumConsecutiveUses -ne 0) {
			throw "Audition-only pattern selection fields must be zero: $($pattern.patternId)"
		}
		if ($category -ceq 'MECHANIC') {
			throw "Audition-only pattern cannot use the MECHANIC category: $($pattern.patternId)"
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
		@($pattern.stages).Count, $armorRequirement,
		$phaseRequirement, $invulnerable) -join "`t"))
	$patternRows.Add((@(
		'PATTERNPOLICY', $encounterDocument.encounterId, $pattern.patternId,
		$category, [uint32]$pattern.minimumPhase, [uint32]$pattern.maximumPhase,
		$targetPolicy, $aimPolicy) -join "`t"))
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
			'kind','anchorId','landingPosition','apexHeight','travelStageId',
			'takeoffStartMs','takeoffEndMs','travelStartMs','travelEndMs') "pattern $($pattern.patternId) serverMotion"
		Assert-JsonString $motion.kind "pattern $($pattern.patternId) serverMotion kind"
		Assert-StableId $motion.anchorId "pattern $($pattern.patternId) serverMotion anchorId"
		Assert-JsonString $motion.travelStageId "pattern $($pattern.patternId) serverMotion travelStageId"
		Assert-StableId $motion.travelStageId "pattern $($pattern.patternId) serverMotion travelStageId"
		# LEAP_TO_TARGET lands where the pattern locked its target; the authored
		# position is the anchor it falls back to when there was no target, and it
		# still feeds the camera lookAt check below.
		if ([string]$motion.kind -cne 'LEAP_TO_ANCHOR' -and
			[string]$motion.kind -cne 'LEAP_TO_TARGET') {
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
		$travelStageIndex = -1
		for ($candidateStageIndex = 0; $candidateStageIndex -lt @($pattern.stages).Count; ++$candidateStageIndex) {
			if ([string]$pattern.stages[$candidateStageIndex].stageId -ceq
				[string]$motion.travelStageId) {
				$travelStageIndex = $candidateStageIndex
				break
			}
		}
		# The ordered first stage owns the takeoff window. Its stable stage ID is
		# authored per pattern (for example TAKEOFF or STEP_01), so do not encode
		# one pattern's label as a global runtime contract.
		if ($travelStageIndex -le 0) {
			throw "serverMotion travelStageId must resolve after the entry stage: $($pattern.patternId)/$($motion.travelStageId)"
		}
		foreach ($timingField in @(
			'takeoffStartMs','takeoffEndMs','travelStartMs','travelEndMs')) {
			Assert-JsonInteger $motion.$timingField `
				"pattern $($pattern.patternId) serverMotion $timingField" 0 ([uint32]::MaxValue)
		}
		$takeoffDurationMs = [uint32]$pattern.stages[0].durationMs
		$travelDurationMs = [uint32]$pattern.stages[$travelStageIndex].durationMs
		if ([uint32]$motion.takeoffStartMs -ge [uint32]$motion.takeoffEndMs -or
			[uint32]$motion.takeoffEndMs -gt $takeoffDurationMs -or
			[uint32]$motion.travelStartMs -ge [uint32]$motion.travelEndMs -or
			[uint32]$motion.travelEndMs -gt $travelDurationMs) {
			throw "serverMotion travel window is outside its authored stage: $($pattern.patternId)"
		}
		$serverMotionByPatternId[[string]$pattern.patternId] = [pscustomobject]@{
			Kind = [string]$motion.kind
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
			(Format-InvariantFloat $motion.apexHeight 'serverMotion apexHeight'),
			[uint32]$travelStageIndex,
			[uint32]$motion.takeoffStartMs, [uint32]$motion.takeoffEndMs,
			[uint32]$motion.travelStartMs, [uint32]$motion.travelEndMs) -join "`t"))
	}
	if ($aimPolicy -eq 'FACE_MOTION_ANCHOR' -and -not $hasServerMotion) {
		throw "FACE_MOTION_ANCHOR requires serverMotion: $($pattern.patternId)"
	}
	$stageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$stageActionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$activeStageActionKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	foreach ($stageIdentity in @($pattern.stages)) {
		Assert-JsonString $stageIdentity.actionId `
			"pattern $($pattern.patternId) stage actionId"
		Assert-StableId $stageIdentity.actionId 'pattern stage actionId'
		if (-not $stageActionIds.Add([string]$stageIdentity.actionId)) {
			throw "Duplicate pattern stage actionId: $($pattern.patternId)/$($stageIdentity.actionId)"
		}
		$stageOwnerByKey["$($pattern.patternId)`n$($stageIdentity.actionId)"] = `
			$stageIdentity
	}
	for ($stageIndex = 0; $stageIndex -lt @($pattern.stages).Count; $stageIndex++) {
		$stage = $pattern.stages[$stageIndex]
		$stageProperties = @(
			'stageId','actionId','stageKind','durationMs','hitShape',
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength','hitHalfWidth',
			'hitCount','hitIntervalMs','hitDelayMs','serverDamageProfileId',
			'pushRangeM','pushMs','knockdown','downMs')
		$hasStageBranches = $null -ne $stage.PSObject.Properties['branches']
		$hasStageActions = $null -ne $stage.PSObject.Properties['actions']
		$hasStageMotion = $null -ne $stage.PSObject.Properties['motion']
		$hasHitOffsets = $null -ne $stage.PSObject.Properties['hitOffsetsMs']
		$hasPartDamagePolicy =
			$null -ne $stage.PSObject.Properties['partDamagePolicy']
		$hasCounterProxy =
			$null -ne $stage.PSObject.Properties['counterProxy']
		$hasPlayerResponse =
			$null -ne $stage.PSObject.Properties['playerResponse']
		$hasAttachmentSlot =
			$null -ne $stage.PSObject.Properties['attachmentSlot']
		if ($hasPlayerResponse -ne $hasAttachmentSlot) {
			throw "Pattern stage playerResponse and attachmentSlot must be authored together: $($pattern.patternId) stage $stageIndex"
		}
		if ($hasStageBranches) { $stageProperties += 'branches' }
		if ($hasStageActions) { $stageProperties += 'actions' }
		if ($hasStageMotion) { $stageProperties += 'motion' }
		if ($hasHitOffsets) { $stageProperties += 'hitOffsetsMs' }
		if ($hasPartDamagePolicy) { $stageProperties += 'partDamagePolicy' }
		if ($hasCounterProxy) { $stageProperties += 'counterProxy' }
		if ($hasPlayerResponse) {
			$stageProperties += @('playerResponse','attachmentSlot')
		}
		Assert-ExactProperties $stage $stageProperties 'encounter pattern stage'
		$partDamagePolicy = 'NORMAL'
		if ($hasPartDamagePolicy) {
			Assert-JsonString $stage.partDamagePolicy `
				"pattern $($pattern.patternId) stage $stageIndex partDamagePolicy"
			$partDamagePolicy = [string]$stage.partDamagePolicy
			if ($partDamagePolicy -cnotin @(
				'NORMAL','DESTROY_FIRST_ELIGIBLE')) {
				throw "Pattern stage partDamagePolicy is invalid: $($pattern.patternId) stage $stageIndex"
			}
		}
		$counterProxyForwardOffsetM = 0.0
		$counterProxyRightOffsetM = 0.0
		$counterProxyRadiusM = 0.0
		if ($hasCounterProxy) {
			Assert-ExactProperties $stage.counterProxy @(
				'space','forwardOffsetM','rightOffsetM','radiusM') `
				'encounter pattern stage counter proxy'
			Assert-JsonString $stage.counterProxy.space `
				"pattern $($pattern.patternId) stage $stageIndex counter proxy space"
			foreach ($field in @(
				'forwardOffsetM','rightOffsetM','radiusM')) {
				Assert-JsonNumber $stage.counterProxy.$field `
					"pattern $($pattern.patternId) stage $stageIndex counter proxy $field"
			}
			$counterProxyForwardOffsetM =
				[double]$stage.counterProxy.forwardOffsetM
			$counterProxyRightOffsetM =
				[double]$stage.counterProxy.rightOffsetM
			$counterProxyRadiusM = [double]$stage.counterProxy.radiusM
			if ([string]$stage.counterProxy.space -cne 'BOSS_LOCAL' -or
				[Math]::Abs($counterProxyForwardOffsetM) -gt 20.0 -or
				[Math]::Abs($counterProxyRightOffsetM) -gt 20.0 -or
				$counterProxyRadiusM -le 0.0 -or
				$counterProxyRadiusM -gt 20.0) {
				throw "Pattern stage counter proxy is invalid: $($pattern.patternId) stage $stageIndex"
			}
		}
		foreach ($field in @('stageId','actionId','stageKind','hitShape','serverDamageProfileId')) {
			Assert-JsonString $stage.$field "pattern $($pattern.patternId) stage $stageIndex $field"
		}
		foreach ($field in @('durationMs','hitCount','hitIntervalMs','hitDelayMs','pushMs','downMs')) {
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
			[string]$stage.stageKind -notin @('WINDUP','ACTIVE','RECOVERY','GROGGY','PART_BREAK')) {
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
		$hitDelayMs = [uint32]$stage.hitDelayMs
		$hitOffsetsMs = @()
		if ($hasHitOffsets) {
			$hitOffsetsMs = @($stage.hitOffsetsMs)
			if ($hitOffsetsMs.Count -eq 0) {
				throw "Pattern stage explicit hitOffsetsMs is empty: $($pattern.patternId) stage $stageIndex"
			}
			$previousHitOffset = $null
			for ($hitOffsetIndex = 0; $hitOffsetIndex -lt $hitOffsetsMs.Count; $hitOffsetIndex++) {
				Assert-JsonInteger $hitOffsetsMs[$hitOffsetIndex] `
					"pattern $($pattern.patternId) stage $stageIndex hitOffsetsMs[$hitOffsetIndex]" `
					0 ([uint32]::MaxValue)
				$currentHitOffset = [uint32]$hitOffsetsMs[$hitOffsetIndex]
				if ($null -ne $previousHitOffset -and $currentHitOffset -le $previousHitOffset) {
					throw "Pattern stage hitOffsetsMs must be strictly increasing: $($pattern.patternId) stage $stageIndex"
				}
				$previousHitOffset = $currentHitOffset
			}
		}
		$damageProfile = [string]$stage.serverDamageProfileId
		$zeroShape = $outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0 -and
			$length -eq 0.0 -and $halfWidth -eq 0.0
		$validShape = $false
		switch ($shape) {
			'NONE' { $validShape = $zeroShape -and $hitCount -eq 0 -and $hitIntervalMs -eq 0 -and $hitDelayMs -eq 0 -and $damageProfile.Length -eq 0 }
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
			$validExplicitHits = $hasHitOffsets -and
				$hitOffsetsMs.Count -eq $hitCount -and
				$hitIntervalMs -eq 0 -and $hitDelayMs -eq 0 -and
				[uint32]$hitOffsetsMs[$hitOffsetsMs.Count - 1] -lt [uint32]$stage.durationMs
			$validLegacyHits = -not $hasHitOffsets -and
				(($hitCount -eq 1 -and $hitIntervalMs -eq 0) -or
				 ($hitCount -gt 1 -and $hitIntervalMs -gt 0)) -and
				([uint64]$hitDelayMs +
				 [uint64]($hitCount - 1) * [uint64]$hitIntervalMs) -lt
					[uint64][uint32]$stage.durationMs
			$validShape = $validShape -and $hitCount -gt 0 -and
				($validExplicitHits -or $validLegacyHits) -and
				$damageIds.Contains($damageProfile)
		}
		elseif ($hasHitOffsets) {
			$validShape = $false
		}
		if (-not $validShape) { throw "Pattern stage hit contract is invalid: $($pattern.patternId) stage $stageIndex" }
		$pushRangeM = [double]$stage.pushRangeM
		$pushMs = [uint32]$stage.pushMs
		$knockdown = [bool]$stage.knockdown
		$downMs = [uint32]$stage.downMs
		$playerResponse = 'DAMAGE'
		$attachmentSlot = 'NONE'
		if ($hasPlayerResponse) {
			Assert-JsonString $stage.playerResponse `
				"pattern $($pattern.patternId) stage $stageIndex playerResponse"
			Assert-JsonString $stage.attachmentSlot `
				"pattern $($pattern.patternId) stage $stageIndex attachmentSlot"
			$playerResponse = [string]$stage.playerResponse
			$attachmentSlot = [string]$stage.attachmentSlot
		}
		if ([math]::Abs($pushRangeM) -gt 20.0 -or
			($pushRangeM -ne 0.0 -and $pushMs -eq 0) -or
			($pushRangeM -eq 0.0 -and $pushMs -ne 0) -or
			($knockdown -and $downMs -eq 0) -or
			(-not $knockdown -and $downMs -ne 0) -or
			($damageProfile.Length -eq 0 -and ($pushRangeM -ne 0.0 -or $knockdown))) {
			throw "Pattern stage push contract is invalid: $($pattern.patternId) stage $stageIndex"
		}
		if (($playerResponse -ceq 'DAMAGE' -and
			 $attachmentSlot -cne 'NONE') -or
			($playerResponse -ceq 'CAPTURE' -and
			 ($attachmentSlot -cne 'BOSS_LEFT_HAND' -or $shape -ceq 'NONE' -or
			  $pushRangeM -ne 0.0 -or $pushMs -ne 0 -or $knockdown -or
			  $downMs -ne 0)) -or
			$playerResponse -cnotin @('DAMAGE','CAPTURE')) {
			throw "Pattern stage player response contract is invalid: $($pattern.patternId) stage $stageIndex"
		}
		$stageRowFields = @(
			'PATTERNSTAGE', $encounterDocument.encounterId, $pattern.patternId, $stageIndex,
			$stage.stageId, $stage.actionId, $stage.stageKind, [uint32]$stage.durationMs,
			$shape,
			(Format-InvariantFloat $outer 'stage hitOuterRadius'),
			(Format-InvariantFloat $inner 'stage hitInnerRadius'),
			(Format-InvariantFloat $angle 'stage hitAngleDegrees'),
			(Format-InvariantFloat $length 'stage hitLength'),
			(Format-InvariantFloat $halfWidth 'stage hitHalfWidth'),
			$hitCount, $hitIntervalMs, $hitDelayMs,
			$(if ($damageProfile.Length -eq 0) { '-' } else { $damageProfile }),
			(Format-InvariantSignedFloat $pushRangeM 'stage pushRangeM'),
			$pushMs,
			$(if ($knockdown) { 1 } else { 0 }),
			$downMs)
		if ($hasPlayerResponse) {
			$stageRowFields += @($playerResponse, $attachmentSlot)
		}
		$patternRows.Add(($stageRowFields -join "`t"))
		$patternStageDurationByKey[
			("{0}/{1}" -f $pattern.patternId, $stageIndex)] = [uint32]$stage.durationMs
		for ($hitOffsetIndex = 0; $hitOffsetIndex -lt $hitOffsetsMs.Count; $hitOffsetIndex++) {
			$patternRows.Add((@(
				'PATTERNSTAGEHITOFFSET', $encounterDocument.encounterId,
				$pattern.patternId, $stage.actionId, [uint32]$hitOffsetIndex,
				[uint32]$hitOffsetsMs[$hitOffsetIndex]) -join "`t"))
		}

		$stageMotionKind = 'NONE'
		if ($hasStageMotion) {
			Assert-ExactProperties $stage.motion @(
				'kind','distance') 'encounter pattern stage motion'
			Assert-JsonString $stage.motion.kind `
				"pattern $($pattern.patternId) stage $stageIndex motion kind"
			Assert-JsonNumber $stage.motion.distance `
				"pattern $($pattern.patternId) stage $stageIndex motion distance"
			$stageMotionKind = [string]$stage.motion.kind
			if ($stageMotionKind -cne 'FORWARD' -or
				[double]$stage.motion.distance -le 0.0 -or
				[double]$stage.motion.distance -gt 1000.0) {
				throw "Pattern stage motion is invalid: $($pattern.patternId) stage $stageIndex"
			}
			$patternRows.Add((@(
				'PATTERNSTAGEMOTION', $encounterDocument.encounterId,
				$pattern.patternId, $stage.actionId, $stageMotionKind,
				(Format-InvariantFloat $stage.motion.distance `
					"pattern $($pattern.patternId) stage motion distance")) -join "`t"))
		}

		$hasCounterableEnter = $false
		$hasCounterableExit = $false
		$hasGroggyEnter = $false
		$hasGroggyExit = $false
		$hasStaggerGaugeEnter = $false
		$hasStaggerGaugeExit = $false
		$hasSpawnCombatObjectAction = $false
		if ($hasStageActions) {
			if ($stage.actions -isnot [Array] -or
				@($stage.actions).Count -lt 1 -or
				@($stage.actions).Count -gt 8) {
				throw "Pattern stage actions must contain 1..8 closed typed actions: $($pattern.patternId) stage $stageIndex"
			}
			$stageActionKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			for ($actionIndex = 0; $actionIndex -lt @($stage.actions).Count; ++$actionIndex) {
				$stageAction = $stage.actions[$actionIndex]
				Assert-JsonString $stageAction.kind `
					"pattern $($pattern.patternId) stage $stageIndex action kind"
				$actionKind = [string]$stageAction.kind
				if ($actionKind -ceq 'SPAWN_COMBAT_OBJECT_VOLLEY') {
					Assert-ExactProperties $stageAction @(
						'trigger','kind','targetId','targetingPolicy',
						'countPerResolvedTarget','layout','radiusM',
						'startAngleDegrees','angleStepDegrees','allowOverlap',
						'maximumTotalObjects','spawnCount','spawnIntervalMs',
						'arenaRandomCount','arenaRandomRadiusM',
						'arenaHeightToleranceM','arenaAnchorPolicy') `
						'encounter pattern stage volley'
					foreach ($field in @(
						'trigger','targetId','targetingPolicy','layout',
						'arenaAnchorPolicy')) {
						Assert-JsonString $stageAction.$field `
							"pattern $($pattern.patternId) stage $stageIndex volley $field"
					}
					Assert-StableId $stageAction.targetId `
						'pattern stage volley targetId'
					Assert-JsonInteger $stageAction.countPerResolvedTarget `
						'pattern stage volley countPerResolvedTarget' 1 8
					Assert-JsonInteger $stageAction.maximumTotalObjects `
						'pattern stage volley maximumTotalObjects' 1 64
					Assert-JsonInteger $stageAction.spawnCount `
						'pattern stage volley spawnCount' 1 8
					Assert-JsonInteger $stageAction.spawnIntervalMs `
						'pattern stage volley spawnIntervalMs' 0 ([uint32]::MaxValue)
					Assert-JsonInteger $stageAction.arenaRandomCount `
						'pattern stage volley arenaRandomCount' 0 32
					foreach ($field in @(
						'radiusM','startAngleDegrees','angleStepDegrees',
						'arenaRandomRadiusM','arenaHeightToleranceM')) {
						Assert-JsonNumber $stageAction.$field `
							"pattern stage volley $field"
					}
					if ($stageAction.allowOverlap -isnot [bool]) {
						throw 'Pattern stage volley allowOverlap must be a JSON Boolean.'
					}
					$actionTrigger = [string]$stageAction.trigger
					$actionTargetId = [string]$stageAction.targetId
					$countPerTarget = [uint32]$stageAction.countPerResolvedTarget
					$maximumTotalObjects = [uint32]$stageAction.maximumTotalObjects
					$layout = [string]$stageAction.layout
					$radiusM = [double]$stageAction.radiusM
					$startAngleDegrees = [double]$stageAction.startAngleDegrees
					$angleStepDegrees = [double]$stageAction.angleStepDegrees
					$allowOverlap = [bool]$stageAction.allowOverlap
					$spawnCount = [uint32]$stageAction.spawnCount
					$spawnIntervalMs = [uint32]$stageAction.spawnIntervalMs
					$arenaRandomCount = [uint32]$stageAction.arenaRandomCount
					$arenaRandomRadiusM = [double]$stageAction.arenaRandomRadiusM
					$arenaHeightToleranceM = [double]$stageAction.arenaHeightToleranceM
					$arenaAnchorPolicy = [string]$stageAction.arenaAnchorPolicy
					$lastSpawnOffsetMs =
						[uint64]($spawnCount - 1) * [uint64]$spawnIntervalMs
					$isSingle = [uint32]1 -eq $countPerTarget
					if ($actionTrigger -cne 'ENTER' -or
						[string]$stageAction.targetingPolicy -cne 'PER_ALIVE_PLAYER' -or
						[uint64]$maximumTotalObjects -lt
							([uint64]$countPerTarget + [uint64]$arenaRandomCount) -or
						$radiusM -lt 0.0 -or
						($isSingle -and ($layout -cne 'SINGLE' -or
							$radiusM -ne 0.0 -or $startAngleDegrees -ne 0.0 -or
							$angleStepDegrees -ne 0.0 -or $allowOverlap)) -or
						(-not $isSingle -and ($layout -cne 'RADIAL' -or
							$radiusM -le 0.0 -or $angleStepDegrees -eq 0.0 -or
							$allowOverlap)) -or
						($spawnCount -eq 1 -and $spawnIntervalMs -ne 0) -or
						($spawnCount -gt 1 -and $spawnIntervalMs -eq 0) -or
						$arenaRandomCount -eq 0 -or
						$arenaRandomRadiusM -le 0.0 -or
						$arenaHeightToleranceM -le 0.0 -or
						$arenaAnchorPolicy -cne 'BOSS_SPAWN_POSITION' -or
						$lastSpawnOffsetMs -ge [uint64][uint32]$stage.durationMs) {
						throw "Pattern stage volley contract is invalid: $($pattern.patternId) stage $stageIndex"
					}
					$actionKey = "$actionTrigger/$actionTargetId"
					if (-not $stageActionKeys.Add($actionKey) -or
						$spawnCombatObjectOwnerById.ContainsKey($actionTargetId)) {
						throw "Pattern stage volley is duplicated: $($pattern.patternId) stage $stageIndex"
					}
					$spawnCombatObjectOwnerById[$actionTargetId] = [pscustomobject]@{
						PatternId = [string]$pattern.patternId
						StageActionId = [string]$stage.actionId
					}
					if ($actionTargetId -ceq
						'combatobject.valtan.high-jump.target-axe') {
						++$valtanHighJumpTypedVolleyCount
						$hasExactValtanHighJumpTypedVolleyOwner =
							[string]$pattern.patternId -ceq 'VALTAN_HIGH_JUMP' -and
							[string]$stage.stageId -ceq 'AIRBORNE' -and
							[string]$stage.actionId -ceq `
								'valtan.attack.high-jump.airborne' -and
							$actionIndex -eq 0 -and
							[uint32]$stage.durationMs -eq 4000 -and
							$spawnCount -eq 3 -and
							$spawnIntervalMs -eq 1333 -and
							$arenaRandomCount -eq 4 -and
							$arenaRandomRadiusM -eq 14.0 -and
							$arenaHeightToleranceM -eq 1.0 -and
							$arenaAnchorPolicy -ceq 'BOSS_SPAWN_POSITION'
					}
					$hasSpawnCombatObjectAction = $true
					$patternRows.Add((@(
						'PATTERNSTAGEVOLLEY', $encounterDocument.encounterId,
						$pattern.patternId, $stage.actionId, $actionIndex,
						$actionTrigger, $actionTargetId,
						[string]$stageAction.targetingPolicy, $countPerTarget, $layout,
						(Format-InvariantFloat $radiusM 'pattern stage volley radiusM'),
						(Format-InvariantSignedFloat $startAngleDegrees `
							'pattern stage volley startAngleDegrees'),
						(Format-InvariantSignedFloat $angleStepDegrees `
							'pattern stage volley angleStepDegrees'),
						$(if ($allowOverlap) { 1 } else { 0 }),
						$maximumTotalObjects, $spawnCount, $spawnIntervalMs,
						$arenaRandomCount,
						(Format-InvariantFloat $arenaRandomRadiusM `
							'pattern stage volley arenaRandomRadiusM'),
						(Format-InvariantFloat $arenaHeightToleranceM `
							'pattern stage volley arenaHeightToleranceM'),
						$arenaAnchorPolicy) -join "`t"))
					continue
				}

				$actionReleaseMode = 'NONE'
				$actionReleaseSpeedMps = 0.0
				if ($actionKind -ceq 'RELEASE_GRABBED_PLAYERS') {
					Assert-ExactProperties $stageAction @(
						'trigger','kind','targetId','releaseMode','speedMps',
						'durationMs') 'encounter grabbed-player release action'
					Assert-JsonString $stageAction.releaseMode `
						'grabbed-player releaseMode'
					Assert-JsonNumber $stageAction.speedMps `
						'grabbed-player release speedMps'
					Assert-JsonInteger $stageAction.durationMs `
						'grabbed-player release durationMs' 0 5000
					$actionValue = [uint32]0
					$actionReleaseMode = [string]$stageAction.releaseMode
					$actionReleaseSpeedMps = [double]$stageAction.speedMps
				}
				else {
					Assert-ExactProperties $stageAction @(
						'trigger','kind','targetId','value','durationMs') `
						'encounter pattern stage action'
					Assert-JsonInteger $stageAction.value `
						'pattern stage action value' 0 ([uint32]::MaxValue)
					Assert-JsonInteger $stageAction.durationMs `
						'pattern stage action durationMs' 0 0
					$actionValue = [uint32]$stageAction.value
				}
				foreach ($field in @('trigger','targetId')) {
					Assert-JsonString $stageAction.$field `
						"pattern $($pattern.patternId) stage $stageIndex action $field"
				}
				Assert-StableId $stageAction.targetId `
					'pattern stage action targetId'
				$actionTrigger = [string]$stageAction.trigger
				$actionTargetId = [string]$stageAction.targetId
				$actionDurationMs = [uint32]$stageAction.durationMs
				$actionKey = "$actionTrigger/$actionTargetId"
				$validTypedAction = $false
				switch ($actionKind) {
					'SET_BOSS_FLAG' {
						$validTypedAction = $actionTargetId -cin @(
							'boss.flag.groggy','boss.flag.invulnerable',
							'boss.flag.counterable') -and $actionValue -le 1
					}
					'SET_STAGGER_GAUGE' {
						$validTypedAction = $actionTargetId -ceq 'boss.gauge.stagger'
					}
					'SET_SHIELD' {
						$validTypedAction = $actionTargetId -ceq 'boss.gauge.shield'
					}
					'SPAWN_COMBAT_OBJECT' {
						$validTypedAction = $actionTrigger -ceq 'ENTER' -and
							$actionValue -eq 1
					}
					'SET_GAMEPLAY_PHASE' {
						$validTypedAction = $actionTrigger -ceq 'ENTER' -and
							$actionTargetId -ceq 'boss.phase.gameplay' -and
							$actionValue -eq 2
					}
					'RETARGET_RANDOM_ALIVE' {
						$validTypedAction = $actionTrigger -ceq 'ENTER' -and
							$actionTargetId -ceq 'boss.target.pattern' -and
							$actionValue -eq 1
					}
					'RELEASE_GRABBED_PLAYERS' {
						$validTypedAction =
							$actionTargetId -ceq 'boss.attachment.left-hand' -and
							(($actionReleaseMode -ceq 'HOLD' -and
							  $actionReleaseSpeedMps -eq 0.0 -and
							  $actionDurationMs -eq 0) -or
							 ($actionReleaseMode -ceq 'OPPOSITE_KNOCKBACK' -and
							  $actionReleaseSpeedMps -gt 0.0 -and
							  $actionReleaseSpeedMps -le 50.0 -and
							  $actionDurationMs -gt 0))
					}
				}
				if ($actionKind -cne 'SPAWN_COMBAT_OBJECT' -and
					$actionKind -cne 'SET_GAMEPLAY_PHASE' -and
					$actionKind -cne 'RETARGET_RANDOM_ALIVE' -and
					$actionKind -cne 'RELEASE_GRABBED_PLAYERS') {
					$validTypedAction = $validTypedAction -and
						(($actionTrigger -ceq 'ENTER' -and $actionValue -gt 0) -or
						 ($actionTrigger -ceq 'EXIT' -and $actionValue -eq 0))
				}
				if (-not $validTypedAction -or -not $stageActionKeys.Add($actionKey)) {
					throw "Pattern stage action is invalid or duplicated: $($pattern.patternId) stage $stageIndex"
				}
				if ($actionKind -ceq 'SPAWN_COMBAT_OBJECT') {
					if ($spawnCombatObjectOwnerById.ContainsKey($actionTargetId)) {
						throw "Combat object spawn action is duplicated: $actionTargetId"
					}
					$spawnCombatObjectOwnerById[$actionTargetId] = [pscustomobject]@{
						PatternId = [string]$pattern.patternId
						StageActionId = [string]$stage.actionId
					}
					$hasSpawnCombatObjectAction = $true
				}
				elseif ($actionKind -ceq 'SET_GAMEPLAY_PHASE') {
					++$gameplayPhaseActionCount
					$hasExactValtanArenaBreakPhaseAction =
						[string]$encounterDocument.encounterId -ceq 'ENCOUNTER_VALTAN' -and
						[string]$pattern.patternId -ceq 'VALTAN_ARENA_BREAK_109' -and
						[string]$stage.stageId -ceq 'IMPACT' -and
						[string]$stage.actionId -ceq `
							'valtan.mechanic.arena-break-109.impact' -and
						$actionIndex -eq 0
				}
				elseif ($actionKind -cne 'RETARGET_RANDOM_ALIVE' -and
					$actionKind -cne 'RELEASE_GRABBED_PLAYERS') {
					$stateKey = "$actionKind/$actionTargetId"
					if ($actionTrigger -ceq 'ENTER') {
						if (-not $activeStageActionKeys.Add($stateKey)) {
							throw "Pattern stage action overlaps an active lifetime: $($pattern.patternId) stage $stageIndex"
						}
					} elseif (-not $activeStageActionKeys.Remove($stateKey)) {
						throw "Pattern stage action clears an inactive lifetime: $($pattern.patternId) stage $stageIndex"
					}
				}
				$hasCounterableEnter = $hasCounterableEnter -or
					($actionTrigger -ceq 'ENTER' -and $actionKind -ceq 'SET_BOSS_FLAG' -and
					 $actionTargetId -ceq 'boss.flag.counterable')
				$hasCounterableExit = $hasCounterableExit -or
					($actionTrigger -ceq 'EXIT' -and $actionKind -ceq 'SET_BOSS_FLAG' -and
					 $actionTargetId -ceq 'boss.flag.counterable')
				$hasGroggyEnter = $hasGroggyEnter -or
					($actionTrigger -ceq 'ENTER' -and $actionKind -ceq 'SET_BOSS_FLAG' -and
					 $actionTargetId -ceq 'boss.flag.groggy')
				$hasGroggyExit = $hasGroggyExit -or
					($actionTrigger -ceq 'EXIT' -and $actionKind -ceq 'SET_BOSS_FLAG' -and
					 $actionTargetId -ceq 'boss.flag.groggy')
				$hasStaggerGaugeEnter = $hasStaggerGaugeEnter -or
					($actionTrigger -ceq 'ENTER' -and $actionKind -ceq 'SET_STAGGER_GAUGE')
				$hasStaggerGaugeExit = $hasStaggerGaugeExit -or
					($actionTrigger -ceq 'EXIT' -and $actionKind -ceq 'SET_STAGGER_GAUGE')
				$stageActionRowFields = @(
					'PATTERNSTAGEACTION', $encounterDocument.encounterId,
					$pattern.patternId, $stage.actionId, $actionIndex,
					$actionTrigger, $actionKind, $stageAction.targetId,
					$actionValue, $actionDurationMs)
				if ($actionKind -ceq 'RELEASE_GRABBED_PLAYERS') {
					$stageActionRowFields += @(
						$actionReleaseMode,
						(Format-InvariantFloat $actionReleaseSpeedMps `
							'grabbed-player release speedMps'))
				}
				$patternRows.Add(($stageActionRowFields -join "`t"))
			}
		}
		if ($hasSpawnCombatObjectAction -and $shape -cne 'NONE') {
			throw "Combat object spawn stage cannot also own an inline hit: $($pattern.patternId) stage $stageIndex"
		}

		$hasPartDestroyed = $false
		$hasCounterHit = $false
		if ($hasStageBranches) {
			if ($stage.branches -isnot [Array] -or
				@($stage.branches).Count -lt 1 -or
				@($stage.branches).Count -gt 8) {
				throw "Pattern stage branches are invalid: $($pattern.patternId) stage $stageIndex"
			}
			$branchOutcomes = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			$hasTimeout = $false
			$hasWallContact = $false
			$hasStaggerBroken = $false
			foreach ($branch in @($stage.branches)) {
				Assert-ExactProperties $branch @(
					'outcome','nextActionId') 'encounter pattern stage branch'
				Assert-JsonString $branch.outcome 'pattern stage branch outcome'
				$outcome = [string]$branch.outcome
				if ($outcome -notin @(
					'TIMEOUT','COUNTER_HIT','STAGGER_BROKEN','WALL_CONTACT',
					'PART_DESTROYED','PROP_DESTROYED','SUMMON_DEAD',
					'ALL_PLAYERS_GRABBED') -or
					-not $branchOutcomes.Add($outcome)) {
					throw "Pattern stage branch outcome is unknown or duplicated: $($pattern.patternId) stage $stageIndex"
				}
				$nextActionId = '-'
				if ($null -ne $branch.nextActionId) {
					Assert-JsonString $branch.nextActionId `
						'pattern stage branch nextActionId'
					Assert-StableId $branch.nextActionId `
						'pattern stage branch nextActionId'
					$nextActionId = [string]$branch.nextActionId
					if (-not $stageActionIds.Contains($nextActionId) -or
						$nextActionId -ceq [string]$stage.actionId) {
						throw "Pattern stage branch target is dangling or self-referential: $($pattern.patternId) stage $stageIndex"
					}
				}
				$hasTimeout = $hasTimeout -or $outcome -eq 'TIMEOUT'
				$hasWallContact = $hasWallContact -or $outcome -eq 'WALL_CONTACT'
				$hasCounterHit = $hasCounterHit -or $outcome -eq 'COUNTER_HIT'
				$hasPartDestroyed = $hasPartDestroyed -or
					$outcome -eq 'PART_DESTROYED'
				$hasStaggerBroken = $hasStaggerBroken -or $outcome -eq 'STAGGER_BROKEN'
				$patternRows.Add((@(
					'PATTERNSTAGEBRANCH', $encounterDocument.encounterId,
					$pattern.patternId, $stage.actionId, $outcome,
					$nextActionId) -join "`t"))
			}
			if (-not $hasTimeout -or
				($hasWallContact -and $stageMotionKind -cne 'FORWARD') -or
				($hasCounterHit -and (-not $hasCounterableEnter -or -not $hasCounterableExit)) -or
				($hasStaggerBroken -and (-not $hasStaggerGaugeEnter -or -not $hasStaggerGaugeExit))) {
				throw "Pattern stage branches need one TIMEOUT and WALL_CONTACT needs FORWARD motion: $($pattern.patternId) stage $stageIndex"
			}
		}
		else {
			$implicitNextActionId = if ($stageIndex + 1 -lt @($pattern.stages).Count) {
				[string]$pattern.stages[$stageIndex + 1].actionId
			} else {
				'-'
			}
			$patternRows.Add((@(
				'PATTERNSTAGEBRANCH', $encounterDocument.encounterId,
				$pattern.patternId, $stage.actionId, 'TIMEOUT',
				$implicitNextActionId) -join "`t"))
		}
		if ($partDamagePolicy -ceq 'DESTROY_FIRST_ELIGIBLE') {
			if (-not $hasPartDestroyed -or
				-not $hasGroggyEnter -or -not $hasGroggyExit) {
				throw "DESTROY_FIRST_ELIGIBLE requires PART_DESTROYED and a closed groggy flag lifetime: $($pattern.patternId) stage $stageIndex"
			}
			$patternRows.Add((@(
				'PATTERNSTAGEPARTDAMAGE', $encounterDocument.encounterId,
				$pattern.patternId, $stage.actionId,
				$partDamagePolicy) -join "`t"))
		}
		if ($hasCounterProxy) {
			if (-not $hasCounterHit -or
				-not $hasCounterableEnter -or -not $hasCounterableExit) {
				throw "Pattern stage counter proxy requires COUNTER_HIT and a closed counterable flag lifetime: $($pattern.patternId) stage $stageIndex"
			}
			$patternRows.Add((@(
				'PATTERNSTAGECOUNTERPROXY', $encounterDocument.encounterId,
				$pattern.patternId, $stage.actionId,
				(Format-InvariantSignedFloat $counterProxyForwardOffsetM `
					'pattern stage counter proxy forwardOffsetM'),
				(Format-InvariantSignedFloat $counterProxyRightOffsetM `
					'pattern stage counter proxy rightOffsetM'),
				(Format-InvariantFloat $counterProxyRadiusM `
					'pattern stage counter proxy radiusM')) -join "`t"))
		}
	}
	if ($activeStageActionKeys.Count -ne 0) {
		throw "Pattern stage action lifetime is not closed: $($pattern.patternId)"
	}
}
if ([string]$encounterBoss[0].phasePolicy.kind -ceq
	'AUTHORED_PATTERN_EVENT') {
	if ($gameplayPhaseActionCount -ne 1 -or
		-not $hasExactValtanArenaBreakPhaseAction) {
		throw ('Valtan authored phase policy requires exactly one ' +
			'109 IMPACT ENTER SET_GAMEPLAY_PHASE action.')
	}
}
elseif ($gameplayPhaseActionCount -ne 0) {
	throw 'Health-percent phase policy cannot also author SET_GAMEPLAY_PHASE.'
}
if ($valtanHighJumpTypedVolleyCount -ne 1 -or
	-not $hasExactValtanHighJumpTypedVolleyOwner) {
	throw ('Valtan HIGH_JUMP AIRBORNE must own exactly one typed target-axe ' +
		'volley at action ordinal zero.')
}
if ($patternRows.Count -eq 0) { throw 'Valtan encounter has no patterns.' }

# The combat-runtime branch owns these reactive/mechanic transitions.  A merge
# once retained the stage/clip rows but silently dropped every non-combat-object
# action and branch, which still passed the generic optional-field validator.
# Keep the exact compiled row set here so Validate fails before such a partial
# encounter can be published again.
$liveEncounterPatterns = @($encounterDocument.patterns | Where-Object {
	[string]$_.selectionMode -cne 'AUDITION_ONLY'
})
$authoredStageCount = 0
$authoredStageActionCount = 0
$authoredStageBranchCount = 0
$authoredStageMotionCount = 0
foreach ($pattern in $liveEncounterPatterns) {
	foreach ($stage in @($pattern.stages)) {
		++$authoredStageCount
		$actionsProperty = $stage.PSObject.Properties['actions']
		$branchesProperty = $stage.PSObject.Properties['branches']
		$motionProperty = $stage.PSObject.Properties['motion']
		if ($null -ne $actionsProperty) {
			$authoredStageActionCount += @($actionsProperty.Value).Count
		}
		if ($null -ne $branchesProperty) {
			$authoredStageBranchCount += @($branchesProperty.Value).Count
		}
		if ($null -ne $motionProperty) {
			++$authoredStageMotionCount
		}
	}
}
if ($liveEncounterPatterns.Count -ne 33 -or
	$authoredStageCount -ne 129 -or
	$authoredStageActionCount -ne 25 -or
	$authoredStageBranchCount -ne 24 -or
	$authoredStageMotionCount -ne 2) {
	throw ('Valtan reactive stage topology count drifted: ' +
		"livePatterns=$($liveEncounterPatterns.Count) " +
		"stages=$authoredStageCount actions=$authoredStageActionCount " +
		"branches=$authoredStageBranchCount motions=$authoredStageMotionCount")
}

$requiredReactiveRows = @(
	"PATTERNSTAGEHITOFFSET`tENCOUNTER_VALTAN`tVALTAN_FOUR_SLASH`tvaltan.attack.triple-slash.active`t0`t1790",
	"PATTERNSTAGEHITOFFSET`tENCOUNTER_VALTAN`tVALTAN_FOUR_SLASH`tvaltan.attack.triple-slash.active`t1`t2560",
	"PATTERNSTAGEHITOFFSET`tENCOUNTER_VALTAN`tVALTAN_FOUR_SLASH`tvaltan.attack.triple-slash.active`t2`t3330",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.groggy`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.groggy`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.groggy`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.groggy`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_PARRY`tvaltan.reactive.parry.stance`t0`tENTER`tSET_STAGGER_GAUGE`tboss.gauge.stagger`t30`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_PARRY`tvaltan.reactive.parry.stance`t1`tEXIT`tSET_STAGGER_GAUGE`tboss.gauge.stagger`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_ARENA_BREAK_109`tvaltan.mechanic.arena-break-109.impact`t0`tENTER`tSET_GAMEPLAY_PHASE`tboss.phase.gameplay`t2`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_RED_BLADE_WAVE`tvaltan.attack.red-blade-wave.active`t0`tENTER`tSPAWN_COMBAT_OBJECT`tcombatobject.valtan.red-blade-wave.projectile`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.first`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.counterable`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.first`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.counterable`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.second`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.counterable`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.second`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.counterable`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.third`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.counterable`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.third`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.counterable`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.groggy`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.groggy`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.groggy`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.groggy`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.shield`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.invulnerable`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.shield`t1`tENTER`tSET_SHIELD`tboss.gauge.shield`t6000`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`t0`tENTER`tSET_STAGGER_GAUGE`tboss.gauge.stagger`t100`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`t1`tEXIT`tSET_STAGGER_GAUGE`tboss.gauge.stagger`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`t2`tEXIT`tSET_SHIELD`tboss.gauge.shield`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`t3`tEXIT`tSET_BOSS_FLAG`tboss.flag.invulnerable`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.groggy`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.groggy`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.groggy`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.groggy`t0`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_CENTER_GRAB_COUNTER_64`tvaltan.mechanic.center-grab-counter-64.counter`t0`tENTER`tSET_BOSS_FLAG`tboss.flag.counterable`t1`t0",
	"PATTERNSTAGEACTION`tENCOUNTER_VALTAN`tVALTAN_CENTER_GRAB_COUNTER_64`tvaltan.mechanic.center-grab-counter-64.counter`t1`tEXIT`tSET_BOSS_FLAG`tboss.flag.counterable`t0`t0",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_PARRY`tvaltan.reactive.parry.stance`tSTAGGER_BROKEN`tvaltan.reactive.parry.slash",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.active`tWALL_CONTACT`tvaltan.attack.dash-charge.groggy",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.active`tTIMEOUT`tvaltan.attack.dash-charge.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.groggy`tPART_DESTROYED`tvaltan.attack.dash-charge.part-break",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.groggy`tTIMEOUT`tvaltan.attack.dash-charge.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.recovery`tTIMEOUT`t-",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.part-break`tTIMEOUT`t-",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_PARRY`tvaltan.reactive.parry.stance`tTIMEOUT`tvaltan.reactive.parry.normal-slash",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_PARRY`tvaltan.reactive.parry.slash`tTIMEOUT`tvaltan.reactive.parry.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.first`tCOUNTER_HIT`tvaltan.reactive.triple-counter.second",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.first`tTIMEOUT`tvaltan.reactive.triple-counter.first-fail",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.second`tCOUNTER_HIT`tvaltan.reactive.triple-counter.third",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.second`tTIMEOUT`tvaltan.reactive.triple-counter.second-fail",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.third`tCOUNTER_HIT`tvaltan.reactive.triple-counter.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_TRIPLE_COUNTER`tvaltan.reactive.triple-counter.third`tTIMEOUT`tvaltan.reactive.triple-counter.third-fail",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_COUNTER`tvaltan.sequence.counter.step-03`tTIMEOUT`t-",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.charge`tWALL_CONTACT`tvaltan.mechanic.armor-break-opening.groggy",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.charge`tTIMEOUT`t-",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.groggy`tPART_DESTROYED`tvaltan.mechanic.armor-break-opening.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.groggy`tTIMEOUT`tvaltan.mechanic.armor-break-opening.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`tSTAGGER_BROKEN`tvaltan.mechanic.magic-orb-stagger-76.groggy",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.window`tTIMEOUT`tvaltan.mechanic.magic-orb-stagger-76.wipe",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_MAGIC_ORB_STAGGER_76`tvaltan.mechanic.magic-orb-stagger-76.groggy`tTIMEOUT`tvaltan.mechanic.magic-orb-stagger-76.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_CENTER_GRAB_COUNTER_64`tvaltan.mechanic.center-grab-counter-64.counter`tCOUNTER_HIT`tvaltan.mechanic.center-grab-counter-64.recovery",
	"PATTERNSTAGEBRANCH`tENCOUNTER_VALTAN`tVALTAN_CENTER_GRAB_COUNTER_64`tvaltan.mechanic.center-grab-counter-64.counter`tTIMEOUT`tvaltan.mechanic.center-grab-counter-64.failed-charge",
	"PATTERNSTAGEPARTDAMAGE`tENCOUNTER_VALTAN`tVALTAN_DASH_CHARGE`tvaltan.attack.dash-charge.groggy`tDESTROY_FIRST_ELIGIBLE",
	"PATTERNSTAGECOUNTERPROXY`tENCOUNTER_VALTAN`tVALTAN_TRASH`tvaltan.sequence.center-trash-rush-if.step-06`t1`t-1.5`t2.25",
	"PATTERNSTAGEMOTION`tENCOUNTER_VALTAN`tVALTAN_ARMOR_BREAK_OPENING`tvaltan.mechanic.armor-break-opening.charge`tFORWARD`t20"
)
$missingReactiveRows = @($requiredReactiveRows | Where-Object {
	-not $patternRows.Contains([string]$_)
})
if ($missingReactiveRows.Count -ne 0) {
	throw ('Valtan reactive stage topology row is missing or changed: ' +
		($missingReactiveRows -join ', '))
}

$combatObjectIds = [Collections.Generic.HashSet[string]]::new(
	[StringComparer]::Ordinal)
$combatObjectVisualIds = [Collections.Generic.HashSet[string]]::new(
	[StringComparer]::Ordinal)
$combatObjectRows = [Collections.Generic.List[string]]::new()
foreach ($combatObject in @($combatObjectDocument.objects)) {
	Assert-ExactProperties $combatObject @(
		'combatObjectArchetypeId','clientVisualId','ownerPatternId',
		'ownerStageActionId','kind','originPolicy','directionPolicy',
		'offsetForwardM','offsetRightM','speedMps','maximumDistanceM',
		'lifeMs','hits') 'Valtan combat object'
	foreach ($field in @(
		'combatObjectArchetypeId','clientVisualId','ownerPatternId',
		'ownerStageActionId','kind','originPolicy','directionPolicy')) {
		Assert-JsonString $combatObject.$field "Valtan combat object $field"
	}
	foreach ($field in @(
		'combatObjectArchetypeId','clientVisualId','ownerPatternId',
		'ownerStageActionId')) {
		Assert-StableId $combatObject.$field "Valtan combat object $field"
	}
	foreach ($field in @(
		'offsetForwardM','offsetRightM','speedMps','maximumDistanceM')) {
		Assert-JsonNumber $combatObject.$field `
			"Valtan combat object $($combatObject.combatObjectArchetypeId) $field"
	}
	Assert-JsonInteger $combatObject.lifeMs `
		"Valtan combat object $($combatObject.combatObjectArchetypeId) lifeMs" `
		1 600000
	$combatObjectId = [string]$combatObject.combatObjectArchetypeId
	$clientVisualId = [string]$combatObject.clientVisualId
	$ownerPatternId = [string]$combatObject.ownerPatternId
	$ownerStageActionId = [string]$combatObject.ownerStageActionId
	if (-not $combatObjectIds.Add($combatObjectId) -or
		-not $combatObjectVisualIds.Add($clientVisualId) -or
		-not $patternById.ContainsKey($ownerPatternId) -or
		-not $stageOwnerByKey.ContainsKey(
			"$ownerPatternId`n$ownerStageActionId") -or
		-not $spawnCombatObjectOwnerById.ContainsKey($combatObjectId)) {
		throw "Valtan combat object identity or owner is invalid: $combatObjectId"
	}
	$spawnOwner = $spawnCombatObjectOwnerById[$combatObjectId]
	if ([string]$spawnOwner.PatternId -cne $ownerPatternId -or
		[string]$spawnOwner.StageActionId -cne $ownerStageActionId) {
		throw "Valtan combat object spawn action owner does not match: $combatObjectId"
	}
	$visual = $combatObjectVisualByArchetypeId[$combatObjectId]
	if ($null -eq $visual -or
		[string]$visual.clientVisualId -cne $clientVisualId) {
		throw "Valtan combat object visual join is invalid: $combatObjectId"
	}
	$ownerPattern = $patternById[$ownerPatternId]
	$ownerStage = $stageOwnerByKey["$ownerPatternId`n$ownerStageActionId"]
	if ([string]$ownerStage.hitShape -cne 'NONE') {
		throw "Valtan combat object owner stage also owns an inline hit: $combatObjectId"
	}

	$kind = [string]$combatObject.kind
	$originPolicy = [string]$combatObject.originPolicy
	$directionPolicy = [string]$combatObject.directionPolicy
	$offsetForwardM = [double]$combatObject.offsetForwardM
	$offsetRightM = [double]$combatObject.offsetRightM
	$speedMps = [double]$combatObject.speedMps
	$maximumDistanceM = [double]$combatObject.maximumDistanceM
	$lifeMs = [uint32]$combatObject.lifeMs
	$finiteMotion =
		-not [double]::IsNaN($offsetForwardM) -and
		-not [double]::IsInfinity($offsetForwardM) -and
		-not [double]::IsNaN($offsetRightM) -and
		-not [double]::IsInfinity($offsetRightM) -and
		-not [double]::IsNaN($speedMps) -and
		-not [double]::IsInfinity($speedMps) -and
		-not [double]::IsNaN($maximumDistanceM) -and
		-not [double]::IsInfinity($maximumDistanceM) -and
		[Math]::Abs($offsetForwardM) -le 100.0 -and
		[Math]::Abs($offsetRightM) -le 100.0 -and
		$speedMps -ge 0.0 -and $speedMps -le 1000.0 -and
		$maximumDistanceM -ge 0.0 -and $maximumDistanceM -le 1000.0
	$validMotion = $false
	if ($kind -ceq 'FIXED_AREA') {
		$stationary =
			$directionPolicy -ceq 'NONE' -and
			$offsetForwardM -eq 0.0 -and $offsetRightM -eq 0.0 -and
			$speedMps -eq 0.0 -and $maximumDistanceM -eq 0.0
		if ($originPolicy -ceq 'LOCKED_TARGET_PER_ALIVE_PLAYER') {
			# The volley deals one area per living player, so it never reads the
			# boss's single pattern target and imposes no target policy on it.
			$validMotion = $stationary
		}
		else {
			$validMotion = $stationary -and
				$originPolicy -ceq 'LOCKED_TARGET_UNTIL_FIRST_PULSE' -and
				[string]$ownerPattern.targetPolicy -cin @(
					'LOCK_NEAREST_ON_START','LOCK_RANDOM_ALIVE_ON_START')
		}
	}
	elseif ($kind -ceq 'MISSILE') {
		$maximumTravelM = $speedMps * ([double]$lifeMs / 1000.0)
		$validMotion = $originPolicy -ceq 'BOSS_POSITION' -and
			$directionPolicy -ceq 'PATTERN_FACING_AT_SPAWN' -and
			$speedMps -gt 0.0 -and $maximumDistanceM -gt 0.0 -and
			$maximumTravelM + 0.00001 -ge $maximumDistanceM -and
			[string]$ownerPattern.aimPolicy -ceq 'LOCK_FACING_ON_START'
	}
	if (-not $finiteMotion -or -not $validMotion -or
		$combatObject.hits -isnot [Array] -or
		@($combatObject.hits).Count -lt 1 -or
		@($combatObject.hits).Count -gt 16) {
		throw "Valtan combat object motion or hit count is invalid: $combatObjectId"
	}

	$combatObjectRows.Add((@(
		'BOSSCOMBATOBJECT', $combatObjectDocument.encounterId,
		$combatObjectId, $clientVisualId, $ownerPatternId,
		$ownerStageActionId, $kind, $originPolicy, $directionPolicy,
		(Format-InvariantSignedFloat $offsetForwardM `
			"combat object $combatObjectId offsetForwardM"),
		(Format-InvariantSignedFloat $offsetRightM `
			"combat object $combatObjectId offsetRightM"),
		(Format-InvariantFloat $speedMps `
			"combat object $combatObjectId speedMps"),
		(Format-InvariantFloat $maximumDistanceM `
			"combat object $combatObjectId maximumDistanceM"),
		$lifeMs, @($combatObject.hits).Count) -join "`t"))

	for ($hitIndex = 0; $hitIndex -lt @($combatObject.hits).Count;
		++$hitIndex) {
		$hit = $combatObject.hits[$hitIndex]
		Assert-ExactProperties $hit @(
			'trigger','atMs','repeatCount','repeatIntervalMs','hitShape',
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
			'hitHalfWidth','serverDamageProfileId','pushRangeM','pushMs',
			'knockdown','downMs') 'Valtan combat object hit'
		foreach ($field in @('trigger','hitShape','serverDamageProfileId')) {
			Assert-JsonString $hit.$field `
				"Valtan combat object $combatObjectId hit $field"
		}
		Assert-StableId $hit.serverDamageProfileId `
			"Valtan combat object $combatObjectId hit damage profile"
		foreach ($field in @('atMs','repeatCount','repeatIntervalMs','pushMs','downMs')) {
			Assert-JsonInteger $hit.$field `
				"Valtan combat object $combatObjectId hit $field" `
				0 ([uint32]::MaxValue)
		}
		foreach ($field in @(
			'hitOuterRadius','hitInnerRadius','hitAngleDegrees','hitLength',
			'hitHalfWidth','pushRangeM')) {
			Assert-JsonNumber $hit.$field `
				"Valtan combat object $combatObjectId hit $field"
		}
		if ($hit.knockdown -isnot [bool]) {
			throw "Valtan combat object knockdown must be Boolean: $combatObjectId"
		}
		$trigger = [string]$hit.trigger
		$atMs = [uint32]$hit.atMs
		$repeatCount = [uint32]$hit.repeatCount
		$repeatIntervalMs = [uint32]$hit.repeatIntervalMs
		if ($repeatCount -lt 1 -or $repeatCount -gt 64 -or
			(($repeatCount -eq 1) -ne ($repeatIntervalMs -eq 0))) {
			throw "Valtan combat object hit repeat is invalid: $combatObjectId/$hitIndex"
		}
		$lastPulseMs = [uint64]$atMs +
			([uint64]($repeatCount - 1) * [uint64]$repeatIntervalMs)
		if ($lastPulseMs -ge [uint64]$lifeMs -or
			($kind -ceq 'FIXED_AREA' -and $trigger -cne 'TIMED') -or
			($kind -ceq 'MISSILE' -and
				($trigger -cne 'CONTACT' -or $atMs -ne 0))) {
			throw "Valtan combat object hit timing is invalid: $combatObjectId/$hitIndex"
		}
		$shape = [string]$hit.hitShape
		$outer = [double]$hit.hitOuterRadius
		$inner = [double]$hit.hitInnerRadius
		$angle = [double]$hit.hitAngleDegrees
		$length = [double]$hit.hitLength
		$halfWidth = [double]$hit.hitHalfWidth
		$validShape = $false
		switch ($shape) {
			'CIRCLE' {
				$validShape = $outer -gt 0.0 -and $inner -eq 0.0 -and
					$angle -eq 0.0 -and $length -eq 0.0 -and $halfWidth -eq 0.0
			}
			'RING' {
				$validShape = $outer -gt $inner -and $inner -gt 0.0 -and
					$angle -eq 0.0 -and $length -eq 0.0 -and $halfWidth -eq 0.0
			}
			'CONE' {
				$validShape = $angle -gt 0.0 -and $angle -le 180.0 -and
					$length -gt 0.0 -and $outer -eq 0.0 -and
					$inner -eq 0.0 -and $halfWidth -eq 0.0
			}
			'BOX' {
				$validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and
					$outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0
			}
			'CROSS' {
				$validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and
					$outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0
			}
			'SIX_DIRECTIONS' {
				$validShape = $length -gt 0.0 -and $halfWidth -gt 0.0 -and
					$outer -eq 0.0 -and $inner -eq 0.0 -and $angle -eq 0.0
			}
		}
		$pushRangeM = [double]$hit.pushRangeM
		$pushMs = [uint32]$hit.pushMs
		$knockdown = [bool]$hit.knockdown
		$downMs = [uint32]$hit.downMs
		if (-not $validShape -or
			-not $damageIds.Contains([string]$hit.serverDamageProfileId) -or
			[Math]::Abs($pushRangeM) -gt 20.0 -or
			(($pushRangeM -eq 0.0) -ne ($pushMs -eq 0)) -or
			($knockdown -ne ($downMs -gt 0))) {
			throw "Valtan combat object hit shape or effect is invalid: $combatObjectId/$hitIndex"
		}
		$combatObjectRows.Add((@(
			'BOSSCOMBATOBJECTHIT', $combatObjectDocument.encounterId,
			$combatObjectId, $hitIndex, $trigger, $atMs, $repeatCount,
			$repeatIntervalMs, $shape,
			(Format-InvariantFloat $outer 'combat object hit outer radius'),
			(Format-InvariantFloat $inner 'combat object hit inner radius'),
			(Format-InvariantFloat $angle 'combat object hit angle'),
			(Format-InvariantFloat $length 'combat object hit length'),
			(Format-InvariantFloat $halfWidth 'combat object hit half width'),
			[string]$hit.serverDamageProfileId,
			(Format-InvariantSignedFloat $pushRangeM `
				'combat object hit pushRangeM'),
			$pushMs, $(if ($knockdown) { 1 } else { 0 }), $downMs) -join "`t"))
	}
}
if ($spawnCombatObjectOwnerById.Count -ne $combatObjectIds.Count -or
	$combatObjectVisualByArchetypeId.Count -ne $combatObjectIds.Count) {
	throw 'Valtan combat object definitions, spawn actions, and visuals must exact join.'
}

# The selectable Valtan timeline is a product-pattern schedule. It owns the
# chronological health-bar sections and the persistent arena/prop state that a
# standalone row must stage before running. Animation preview clips are review
# evidence only and are deliberately not joined to this Server contract.
$timelineDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanDebugAudition.json'
Assert-ExactProperties $timelineDocument @(
	'schema','formatVersion','authority','encounterId','timelineId','rows') `
	'Valtan pattern timeline document'
foreach ($field in @('schema','authority','encounterId','timelineId')) {
	Assert-JsonString $timelineDocument.$field "Valtan pattern timeline $field"
}
Assert-JsonInteger $timelineDocument.formatVersion `
	'Valtan pattern timeline formatVersion' 1 1
Assert-StableId $timelineDocument.timelineId 'Valtan pattern timeline timelineId'
$timelineRowCount = 52
if ([string]$timelineDocument.schema -cne `
	'lostark.valtan-pattern-timeline' -or
	[uint32]$timelineDocument.formatVersion -ne 1 -or
	[string]$timelineDocument.authority -cne 'server' -or
	[string]$timelineDocument.encounterId -cne `
		[string]$encounterDocument.encounterId -or
	[string]$timelineDocument.timelineId -cne 'VALTAN_AUDITION_TIMELINE' -or
	$timelineDocument.rows -isnot [Array] -or
	@($timelineDocument.rows).Count -ne $timelineRowCount) {
	throw 'Valtan pattern timeline header is invalid.'
}

$timelineEntryTypes =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($entryType in @('MECHANIC','NORMAL')) {
	[void]$timelineEntryTypes.Add($entryType)
}
$timelineArenaRanks =
	[Collections.Generic.Dictionary[string,uint32]]::new(
		[StringComparer]::Ordinal)
$timelineArenaRanks.Add('FRESH', [uint32]0)
$timelineArenaRanks.Add('ORDINARY_WALLS_GONE', [uint32]1)
$timelineArenaRanks.Add('ALL_WALLS_GONE', [uint32]2)
$timelineArenaRanks.Add('FLOOR84_GONE', [uint32]3)
$timelineArenaRanks.Add('FLOOR84_AND_30_GONE', [uint32]4)
$timelinePropStates =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($propState in @('HIDDEN','FOUR_PILLARS_INTACT')) {
	[void]$timelinePropStates.Add($propState)
}
$timelinePatternById =
	[Collections.Generic.Dictionary[string,object]]::new(
		[StringComparer]::Ordinal)
foreach ($pattern in @($encounterDocument.patterns)) {
	$timelinePatternById.Add([string]$pattern.patternId, $pattern)
}
$timelineRowIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$timelineCommandIds = [Collections.Generic.HashSet[uint32]]::new()
$timelineRows = [Collections.Generic.List[string]]::new()
$timelineRows.Add((@(
	'VALTANTIMELINE', $timelineDocument.encounterId,
	$timelineDocument.timelineId, $timelineRowCount) -join "`t"))
$previousSectionHealthBar = [uint32]$maximumHealthBars
$previousArenaRank = [uint32]0
for ($index = 0; $index -lt $timelineRowCount; ++$index) {
	$row = $timelineDocument.rows[$index]
	$expectedOrdinal = [uint32]($index + 1)
	Assert-ExactProperties $row @(
		'rowId','commandId','ordinal','sectionHealthBar','entryType','patterns',
		'arenaState','propState','displayLabel') 'Valtan pattern timeline row'
	foreach ($field in @('rowId','entryType','arenaState','propState')) {
		Assert-JsonString $row.$field "Valtan pattern timeline row $field"
	}
	Assert-StableId $row.rowId "Valtan pattern timeline rowId $expectedOrdinal"
	Assert-JsonInteger $row.commandId `
		'Valtan pattern timeline commandId' 1 4294967295
	Assert-JsonInteger $row.ordinal `
		'Valtan pattern timeline ordinal' 1 $timelineRowCount
	Assert-JsonInteger $row.sectionHealthBar `
		'Valtan pattern timeline sectionHealthBar' 1 $maximumHealthBars
	Assert-DisplayText $row.displayLabel 256 `
		'Valtan pattern timeline displayLabel'
	$rowId = [string]$row.rowId
	$commandId = [uint32]$row.commandId
	$expectedCommandId = Get-Fnv1a32 $rowId
	$entryType = [string]$row.entryType
	$arenaState = [string]$row.arenaState
	$propState = [string]$row.propState
	if ([uint32]$row.ordinal -ne $expectedOrdinal -or
		$rowId -cnotmatch '^[a-z0-9]+(?:[.-][a-z0-9]+)*$' -or
		[Text.Encoding]::UTF8.GetByteCount($rowId) -gt 64 -or
		$commandId -ne $expectedCommandId -or
		-not $timelineRowIds.Add($rowId) -or
		-not $timelineCommandIds.Add($commandId) -or
		-not $timelineEntryTypes.Contains($entryType) -or
		-not $timelineArenaRanks.ContainsKey($arenaState) -or
		-not $timelinePropStates.Contains($propState) -or
		$row.patterns -isnot [Array] -or
		@($row.patterns).Count -lt 1 -or
		@($row.patterns).Count -gt 8) {
		throw "Valtan pattern timeline row identity is invalid: $expectedOrdinal"
	}

	$sectionHealthBar = [uint32]$row.sectionHealthBar
	$arenaRank = [uint32]$timelineArenaRanks[$arenaState]
	if (0 -eq $index -and
		($sectionHealthBar -ne $maximumHealthBars -or 0 -ne $arenaRank)) {
		throw 'Valtan pattern timeline must begin at the fresh maximum-health state.'
	}
	if ($sectionHealthBar -gt $previousSectionHealthBar) {
		throw "Valtan pattern timeline health bars must not increase: $expectedOrdinal"
	}
	if ($arenaRank -lt $previousArenaRank -or
		$arenaRank -gt ($previousArenaRank + 1)) {
		throw "Valtan pattern timeline arena state is not monotonic: $expectedOrdinal"
	}
	$previousSectionHealthBar = $sectionHealthBar
	$previousArenaRank = $arenaRank

	$patternActions = @($row.patterns)
	$timelineRows.Add((@(
		'VALTANTIMELINEROW', $timelineDocument.encounterId,
		$timelineDocument.timelineId, $commandId, $expectedOrdinal, $rowId,
		$sectionHealthBar, $entryType, $arenaState, $propState,
		$patternActions.Count) -join "`t"))
	for ($actionIndex = 0; $actionIndex -lt $patternActions.Count;
		++$actionIndex) {
		$action = $patternActions[$actionIndex]
		Assert-ExactProperties $action @('patternId','repeat') `
			'Valtan pattern timeline action'
		Assert-JsonString $action.patternId `
			'Valtan pattern timeline action patternId'
		Assert-StableId $action.patternId `
			"Valtan pattern timeline patternId $expectedOrdinal"
		Assert-JsonInteger $action.repeat `
			'Valtan pattern timeline action repeat' 1 4
		$patternId = [string]$action.patternId
		if (-not $timelinePatternById.ContainsKey($patternId)) {
			throw "Valtan pattern timeline action has no product pattern: $expectedOrdinal/$($actionIndex + 1)"
		}
		$timelineRows.Add((@(
			'VALTANTIMELINEPATTERN', $timelineDocument.encounterId,
			$timelineDocument.timelineId, $expectedOrdinal,
			[uint32]($actionIndex + 1), $patternId,
			[uint32]$action.repeat) -join "`t"))
	}
}
if ($previousSectionHealthBar -ne 14 -or $previousArenaRank -ne 4) {
	throw 'Valtan pattern timeline does not reach the authored final state.'
}
$patternRows.AddRange($timelineRows)

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

# Cover is answered by a raised stele, but one authored stage is meant to reach
# through it. The allowlist is the only way a stage gains that, so a new pattern
# cannot silently ignore the raid's only protection.
$coverPierceDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanCoverPiercingActions.json'
Assert-ExactProperties $coverPierceDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','actions') `
	'Valtan cover piercing document'
Assert-JsonString $coverPierceDocument.schema 'Valtan cover piercing schema'
Assert-JsonInteger $coverPierceDocument.formatVersion `
	'Valtan cover piercing formatVersion' 1 1
if ([string]$coverPierceDocument.schema -cne `
	'lostark.valtan-cover-piercing-actions' -or
	[uint32]$coverPierceDocument.formatVersion -ne 1 -or
	[string]$coverPierceDocument.encounterId -cne `
		[string]$encounterDocument.encounterId -or
	[string]$coverPierceDocument.bossArchetypeId -cne `
		[string]$encounterDocument.bossArchetypeId -or
	$coverPierceDocument.actions -isnot [Array] -or
	@($coverPierceDocument.actions).Count -eq 0 -or
	@($coverPierceDocument.actions).Count -gt 16) {
	throw 'Valtan cover piercing document header is invalid.'
}
$coverPierceActionIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$previousCoverPierceActionId = ''
foreach ($coverPierce in @($coverPierceDocument.actions)) {
	Assert-ExactProperties $coverPierce @('patternId','stageId','actionId') `
		'Valtan cover piercing action'
	foreach ($field in @('patternId','stageId','actionId')) {
		Assert-JsonString $coverPierce.$field "Valtan cover piercing $field"
		Assert-StableId ([string]$coverPierce.$field) "Valtan cover piercing $field"
	}
	$pierceActionId = [string]$coverPierce.actionId
	if (-not $coverPierceActionIds.Add($pierceActionId) -or
		($previousCoverPierceActionId.Length -ne 0 -and
		 [StringComparer]::Ordinal.Compare(
			$previousCoverPierceActionId, $pierceActionId) -ge 0)) {
		throw "Valtan cover piercing actions are duplicated or not ordinal: $pierceActionId"
	}
	$previousCoverPierceActionId = $pierceActionId
	$ownerPatterns = @($encounterDocument.patterns | Where-Object {
		[string]$_.patternId -ceq [string]$coverPierce.patternId })
	if ($ownerPatterns.Count -ne 1) {
		throw "Valtan cover piercing pattern is missing: $($coverPierce.patternId)"
	}
	$ownerPattern = $ownerPatterns[0]
	$pierceStageIndex = -1
	for ($candidateIndex = 0;
		$candidateIndex -lt @($ownerPattern.stages).Count; ++$candidateIndex) {
		$candidate = $ownerPattern.stages[$candidateIndex]
		if ([string]$candidate.stageId -ceq [string]$coverPierce.stageId -and
			[string]$candidate.actionId -ceq $pierceActionId) {
			if ($pierceStageIndex -ne -1) {
				throw "Valtan cover piercing stage is duplicated: $pierceActionId"
			}
			$pierceStageIndex = $candidateIndex
		}
	}
	if ($pierceStageIndex -lt 0) {
		throw "Valtan cover piercing stage/action join failed: $pierceActionId"
	}
	$pierceStage = $ownerPattern.stages[$pierceStageIndex]
	if ([string]$pierceStage.hitShape -ceq 'NONE' -or
		[uint32]$pierceStage.hitCount -eq 0) {
		throw "Valtan cover piercing action has no Server hit pulse: $pierceActionId"
	}
	$patternRows.Add((@(
		'PATTERNSTAGECOVERPIERCE', $encounterDocument.encounterId,
		$ownerPattern.patternId, [uint32]$pierceStageIndex,
		$pierceStage.stageId, $pierceStage.actionId) -join "`t"))
}

# The four stele slots outlive the pattern that raises them, so a later
# pattern stage owns each shatter. The edge names the slots it breaks because a
# pair leaves the opposite pair standing, and that is the cover the raid moves
# to. A slot may be claimed by exactly one edge for the whole encounter.
$propBreakDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanPropBreakActions.json'
Assert-ExactProperties $propBreakDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','propSetId',
	'actions') 'Valtan prop break document'
foreach ($field in @('schema','encounterId','bossArchetypeId','propSetId')) {
	Assert-JsonString $propBreakDocument.$field "Valtan prop break $field"
}
Assert-JsonInteger $propBreakDocument.formatVersion `
	'Valtan prop break formatVersion' 1 1
if ([string]$propBreakDocument.schema -cne 'lostark.valtan-prop-break-actions' -or
	[uint32]$propBreakDocument.formatVersion -ne 1 -or
	[string]$propBreakDocument.encounterId -cne
		[string]$encounterDocument.encounterId -or
	[string]$propBreakDocument.bossArchetypeId -cne
		[string]$encounterDocument.bossArchetypeId -or
	$propBreakDocument.actions -isnot [Array] -or
	@($propBreakDocument.actions).Count -eq 0 -or
	@($propBreakDocument.actions).Count -gt 8) {
	throw 'Valtan prop break document header is invalid.'
}
Assert-StableId ([string]$propBreakDocument.propSetId) 'Valtan prop break propSetId'
$propBreakActionIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$propBreakSlotIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$previousPropBreakActionId = ''
foreach ($propBreak in @($propBreakDocument.actions)) {
	Assert-ExactProperties $propBreak @(
		'patternId','stageId','actionId','slotIds') 'Valtan prop break action'
	foreach ($field in @('patternId','stageId','actionId')) {
		Assert-JsonString $propBreak.$field "Valtan prop break $field"
		Assert-StableId ([string]$propBreak.$field) "Valtan prop break $field"
	}
	$propActionId = [string]$propBreak.actionId
	if (-not $propBreakActionIds.Add($propActionId) -or
		($previousPropBreakActionId.Length -ne 0 -and
		 [StringComparer]::Ordinal.Compare(
			$previousPropBreakActionId, $propActionId) -ge 0)) {
		throw "Valtan prop break actions are duplicated or not ordinal: $propActionId"
	}
	$previousPropBreakActionId = $propActionId
	if ($propBreak.slotIds -isnot [Array] -or
		@($propBreak.slotIds).Count -eq 0 -or
		@($propBreak.slotIds).Count -gt 8) {
		throw "Valtan prop break action names no slots: $propActionId"
	}
	$ownerPatterns = @($encounterDocument.patterns | Where-Object {
		[string]$_.patternId -ceq [string]$propBreak.patternId })
	if ($ownerPatterns.Count -ne 1) {
		throw "Valtan prop break pattern is missing: $($propBreak.patternId)"
	}
	$ownerPattern = $ownerPatterns[0]
	$propStageIndex = -1
	for ($candidateIndex = 0;
		$candidateIndex -lt @($ownerPattern.stages).Count; ++$candidateIndex) {
		$candidate = $ownerPattern.stages[$candidateIndex]
		if ([string]$candidate.stageId -ceq [string]$propBreak.stageId -and
			[string]$candidate.actionId -ceq $propActionId) {
			if ($propStageIndex -ne -1) {
				throw "Valtan prop break stage is duplicated: $propActionId"
			}
			$propStageIndex = $candidateIndex
		}
	}
	if ($propStageIndex -lt 0) {
		throw "Valtan prop break stage/action join failed: $propActionId"
	}
	foreach ($slotId in @($propBreak.slotIds)) {
		Assert-JsonString $slotId 'Valtan prop break slotId'
		Assert-StableId ([string]$slotId) 'Valtan prop break slotId'
		if (-not $propBreakSlotIds.Add([string]$slotId)) {
			throw "Valtan prop break slot is claimed twice: $slotId"
		}
		$patternRows.Add((@(
			'PATTERNSTAGEPROPBREAK', $encounterDocument.encounterId,
			$ownerPattern.patternId, [uint32]$propStageIndex,
			$propBreak.stageId, $propActionId,
			$propBreakDocument.propSetId, $slotId) -join "`t"))
	}
}

# Rotation Product v4 is a tagged union. Managed WEIGHTED_POOL rows retain the
# authoring window/set identities and each candidate's weight/enabled state.
# Legacy ORDERED_INTRO_THEN_WEIGHTED rows retain their ordered patternIds,
# including intentional duplicates. The v4 scriptedSequence is the exclusive
# automatic playback program; retained rotations remain authoring/debug input.
$rotationDocument = Read-JsonDocument `
	'Data/Encounters/Valtan/ValtanPatternRotations.json'
Assert-ExactProperties $rotationDocument @(
	'schema','formatVersion','encounterId','bossArchetypeId','rotations',
	'scriptedSequence') `
	'Valtan pattern rotation document'
Assert-JsonString $rotationDocument.schema 'Valtan pattern rotation schema'
Assert-JsonInteger $rotationDocument.formatVersion `
	'Valtan pattern rotation formatVersion' 4 4
Assert-JsonString $rotationDocument.encounterId `
	'Valtan pattern rotation encounterId'
Assert-JsonString $rotationDocument.bossArchetypeId `
	'Valtan pattern rotation bossArchetypeId'
if ([string]$rotationDocument.schema -cne `
	'lostark.valtan-pattern-rotations' -or
	[uint32]$rotationDocument.formatVersion -ne 4 -or
	[string]$rotationDocument.encounterId -cne `
		[string]$encounterDocument.encounterId -or
	[string]$rotationDocument.bossArchetypeId -cne `
		[string]$encounterDocument.bossArchetypeId -or
	$rotationDocument.rotations -isnot [Array] -or
	@($rotationDocument.rotations).Count -eq 0 -or
	@($rotationDocument.rotations).Count -gt 16) {
	throw 'Valtan pattern rotation document header is invalid.'
}
$rotationIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$previousRotationFromBar = [uint32]::MaxValue
$rotationFormatVersion = [uint32]$rotationDocument.formatVersion
foreach ($rotation in @($rotationDocument.rotations)) {
	Assert-JsonString $rotation.rotationId 'Valtan pattern rotation rotationId'
	Assert-StableId ([string]$rotation.rotationId) 'Valtan pattern rotation rotationId'
	Assert-JsonString $rotation.selectionMode 'Valtan pattern rotation selectionMode'
	if ([string]$rotation.selectionMode -cnotin @(
		'WEIGHTED_POOL','ORDERED_INTRO_THEN_WEIGHTED')) {
		throw "Valtan pattern rotation selectionMode is invalid: $($rotation.rotationId)"
	}
	Assert-JsonInteger $rotation.fromHealthBar `
		'Valtan pattern rotation fromHealthBar' 1 $maximumHealthBars
	Assert-JsonInteger $rotation.toHealthBar `
		'Valtan pattern rotation toHealthBar' 1 $maximumHealthBars
	$rotationFromBar = [uint32]$rotation.fromHealthBar
	$rotationToBar = [uint32]$rotation.toHealthBar
	if (-not $rotationIds.Add([string]$rotation.rotationId)) {
		throw "Valtan pattern rotation id is duplicated: $($rotation.rotationId)"
	}
	# Bars count down, so a span runs from the higher bar to the lower one and the
	# document is authored in the order the fight meets them.
	if ($rotationFromBar -le $rotationToBar -or
		$rotationFromBar -ge $previousRotationFromBar) {
		throw "Valtan pattern rotation span is invalid: $($rotation.rotationId)"
	}
	$previousRotationFromBar = $rotationFromBar
	$isWeightedV3 = $rotationFormatVersion -ge 3 -and
		[string]$rotation.selectionMode -ceq 'WEIGHTED_POOL'
	if ($isWeightedV3) {
		Assert-ExactProperties $rotation @(
			'rotationId','selectionMode','fromHealthBar','toHealthBar',
			'windowId','gameplayPhase','selectionSetId','candidates') `
			'Valtan weighted pattern rotation v3'
		Assert-JsonString $rotation.windowId 'Valtan rotation windowId'
		Assert-StableId ([string]$rotation.windowId) 'Valtan rotation windowId'
		Assert-JsonInteger $rotation.gameplayPhase `
			'Valtan rotation gameplayPhase' 1 3
		Assert-JsonString $rotation.selectionSetId 'Valtan rotation selectionSetId'
		Assert-StableId ([string]$rotation.selectionSetId) `
			'Valtan rotation selectionSetId'
		if ($rotation.candidates -isnot [Array] -or
			@($rotation.candidates).Count -eq 0 -or
			@($rotation.candidates).Count -gt 32) {
			throw "Valtan weighted candidate count is invalid: $($rotation.rotationId)"
		}
	}
	else {
		Assert-ExactProperties $rotation @(
			'rotationId','selectionMode','fromHealthBar','toHealthBar','patternIds') `
			'Valtan ordered/legacy pattern rotation'
		if ($rotation.patternIds -isnot [Array] -or
			@($rotation.patternIds).Count -eq 0 -or
			@($rotation.patternIds).Count -gt 32) {
			throw "Valtan pattern rotation step count is invalid: $($rotation.rotationId)"
		}
	}
	$weightedPoolIds =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$rotationStepIndex = 0
	$enabledPositiveWeight = [uint64]0
	$rotationEntries = if ($isWeightedV3) {
		@($rotation.candidates)
	}
	else {
		@($rotation.patternIds | ForEach-Object {
			[pscustomobject]@{ patternId = $_; weight = 0; enabled = $true }
		})
	}
	foreach ($rotationEntry in $rotationEntries) {
		if ($isWeightedV3) {
			Assert-ExactProperties $rotationEntry @(
				'patternId','weight','enabled') 'Valtan weighted candidate'
			Assert-JsonInteger $rotationEntry.weight `
				'Valtan weighted candidate weight' 1 100000
			if ($rotationEntry.enabled -isnot [bool]) {
				throw 'Valtan weighted candidate enabled must be a JSON Boolean.'
			}
		}
		$rotationPatternId = [string]$rotationEntry.patternId
		Assert-JsonString $rotationPatternId 'Valtan pattern rotation patternId'
		Assert-StableId ([string]$rotationPatternId) 'Valtan pattern rotation patternId'
		$rotationOwners = @($encounterDocument.patterns | Where-Object {
			[string]$_.patternId -ceq [string]$rotationPatternId })
		if ($rotationOwners.Count -ne 1) {
			throw "Valtan pattern rotation names no pattern: $rotationPatternId"
		}
		# A scripted mechanic is owned by its own bar, and the entrance is owned by
		# introPatternId. Neither may be replayed from a rotation step.
		if ([string]$rotationOwners[0].selectionMode -cne 'NORMAL' -or
			[string]$rotationPatternId -ceq `
				[string]$encounterDocument.introPatternId) {
			throw "Valtan pattern rotation step is not a normal pattern: $rotationPatternId"
		}
		if ([string]$rotation.selectionMode -ceq 'WEIGHTED_POOL' -and
			-not $weightedPoolIds.Add([string]$rotationPatternId)) {
			throw "Valtan weighted pattern pool duplicates a pattern: $rotationPatternId"
		}
		if ($isWeightedV3 -and [bool]$rotationEntry.enabled) {
			$enabledPositiveWeight += [uint64]$rotationEntry.weight
		}
		if ($isWeightedV3) {
			$patternRows.Add((@(
				'PATTERNROTATIONCANDIDATE', $encounterDocument.encounterId,
				$rotation.rotationId, $rotationStepIndex, $rotationPatternId,
				[uint32]$rotationEntry.weight,
				$(if ([bool]$rotationEntry.enabled) { 1 } else { 0 })) -join "`t"))
		}
		else {
			$patternRows.Add((@(
				'PATTERNROTATIONSTEP', $encounterDocument.encounterId,
				$rotation.rotationId, $rotationStepIndex,
				$rotationPatternId) -join "`t"))
		}
		++$rotationStepIndex
	}
	if ($isWeightedV3 -and 0 -eq $enabledPositiveWeight) {
		throw "Valtan weighted rotation has no enabled positive weight: $($rotation.rotationId)"
	}
	$patternRows.Add((@(
		'PATTERNROTATION', $encounterDocument.encounterId,
		$rotation.rotationId, $rotationFromBar, $rotationToBar,
		$rotation.selectionMode, $rotationStepIndex) -join "`t"))
	if ($isWeightedV3) {
		$patternRows.Add((@(
			'PATTERNROTATIONWINDOW', $encounterDocument.encounterId,
			$rotation.rotationId, $rotation.windowId,
			[uint32]$rotation.gameplayPhase, $rotation.selectionSetId,
			$rotationFromBar, $rotationToBar, $rotationStepIndex) -join "`t"))
	}
}

$scriptedSequence = $rotationDocument.scriptedSequence
Assert-ExactProperties $scriptedSequence @(
	'sequenceId','mode','interStepPursuitMs','patternIds') `
	'Valtan scripted pattern sequence'
Assert-JsonString $scriptedSequence.sequenceId `
	'Valtan scripted pattern sequenceId'
Assert-StableId ([string]$scriptedSequence.sequenceId) `
	'Valtan scripted pattern sequenceId'
Assert-JsonString $scriptedSequence.mode 'Valtan scripted pattern mode'
Assert-JsonInteger $scriptedSequence.interStepPursuitMs `
	'Valtan scripted inter-step pursuit milliseconds' 100 10000
if ([string]$scriptedSequence.mode -cne 'ORDERED_ONCE_THEN_IDLE' -or
	$scriptedSequence.patternIds -isnot [Array] -or
	@($scriptedSequence.patternIds).Count -eq 0 -or
	@($scriptedSequence.patternIds).Count -gt 32) {
	throw 'Valtan scripted pattern sequence contract is invalid.'
}
$scriptedPatternIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$scriptedStepIndex = 0
foreach ($scriptedPatternIdValue in @($scriptedSequence.patternIds)) {
	Assert-JsonString $scriptedPatternIdValue 'Valtan scripted patternId'
	$scriptedPatternId = [string]$scriptedPatternIdValue
	Assert-StableId $scriptedPatternId 'Valtan scripted patternId'
	$scriptedOwners = @($encounterDocument.patterns | Where-Object {
		[string]$_.patternId -ceq $scriptedPatternId
	})
	if ($scriptedOwners.Count -ne 1 -or
		-not $scriptedPatternIds.Add($scriptedPatternId)) {
		throw "Valtan scripted sequence step is invalid: $scriptedPatternId"
	}
	$patternRows.Add((@(
		'PATTERNSEQUENCESTEP', $encounterDocument.encounterId,
		$scriptedSequence.sequenceId, $scriptedStepIndex,
		$scriptedPatternId) -join "`t"))
	++$scriptedStepIndex
}
$patternRows.Add((@(
	'PATTERNSEQUENCE', $encounterDocument.encounterId,
	$scriptedSequence.sequenceId, $scriptedSequence.mode,
	[uint32]$scriptedSequence.interStepPursuitMs,
	$scriptedStepIndex) -join "`t"))

# A phase-changing health mechanic cannot move independently from both sides of
# its rotation topology.  The authoring pipeline currently exposes no atomic
# operation that promotes the managed window and the sealed legacy span
# together, so a partial Product overlay must fail before a bootstrap is built.
if ($rotationFormatVersion -ge 3) {
	$arenaBreakPatterns = @($encounterDocument.patterns | Where-Object {
		[string]$_.patternId -ceq 'VALTAN_ARENA_BREAK_109'
	})
	$phaseOneManagedRotations = @($rotationDocument.rotations | Where-Object {
		[string]$_.selectionMode -ceq 'WEIGHTED_POOL' -and
		[uint32]$_.gameplayPhase -eq 1
	})
	$legacyOrderedRotations = @($rotationDocument.rotations | Where-Object {
		[string]$_.selectionMode -ceq 'ORDERED_INTRO_THEN_WEIGHTED'
	})
	if ($arenaBreakPatterns.Count -ne 1 -or
		$phaseOneManagedRotations.Count -eq 0 -or
		$legacyOrderedRotations.Count -eq 0) {
		throw 'Valtan phase-boundary topology inventory is incomplete.'
	}
	$arenaBreakBar = [uint32]$arenaBreakPatterns[0].triggerHealthBar
	$finalPhaseOneBoundary = [uint32]$phaseOneManagedRotations[
		$phaseOneManagedRotations.Count - 1].toHealthBar
	$firstLegacyBoundary = [uint32]$legacyOrderedRotations[0].fromHealthBar + 1
	if ($arenaBreakBar -ne $finalPhaseOneBoundary -or
		$arenaBreakBar -ne $firstLegacyBoundary) {
		throw "VALTAN_ARENA_BREAK_109 triggerHealthBar must equal the final phase-1 WINDOW.toHealthBar and first ORDERED.fromHealthBar+1. arena=$arenaBreakBar phase1=$finalPhaseOneBoundary legacy=$firstLegacyBoundary"
	}
}

# A WALL_CONTACT branch is the typed charge-impact marker. Derive the Server
# row from the final Encounter graph so split Valtan gameplay and sealed legacy
# patterns cannot drift behind a second hand-maintained allowlist.
$chargeImpactActionIds =
	[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($chargeOwner in @($encounterDocument.patterns)) {
	$ownerStages = @($chargeOwner.stages)
	for ($chargeStageIndex = 0; $chargeStageIndex -lt $ownerStages.Count;
		++$chargeStageIndex) {
		$chargeStage = $ownerStages[$chargeStageIndex]
		$branches = if ($null -eq $chargeStage.PSObject.Properties['branches']) {
			@()
		}
		else { @($chargeStage.branches) }
		$wallContacts = @($branches | Where-Object {
			[string]$_.outcome -ceq 'WALL_CONTACT' })
		if ($wallContacts.Count -eq 0) { continue }
		$chargeActionId = [string]$chargeStage.actionId
		if ($wallContacts.Count -ne 1) {
			throw "Valtan charge stage must have exactly one WALL_CONTACT branch: $chargeActionId"
		}
		$motion = if ($null -eq $chargeStage.PSObject.Properties['motion']) {
			$null
		}
		else { $chargeStage.motion }
		if ($null -eq $motion -or [string]$motion.kind -cne 'FORWARD') {
			throw "Valtan WALL_CONTACT stage must use FORWARD motion: $chargeActionId"
		}
		Assert-JsonNumber $motion.distance "Valtan charge distance $chargeActionId"
		if ([double]$motion.distance -le 0.0) {
			throw "Valtan charge distance must be positive: $chargeActionId"
		}
		$nextActionId = [string]$wallContacts[0].nextActionId
		$expectedTargetActionId = ''
		if ([string]$chargeOwner.patternId -ceq 'VALTAN_DASH_CHARGE') {
			$expectedTargetActionId = 'valtan.attack.dash-charge.groggy'
		}
		elseif ([string]$chargeOwner.patternId -ceq
			'VALTAN_ARMOR_BREAK_OPENING') {
			$expectedTargetActionId =
				'valtan.mechanic.armor-break-opening.groggy'
		}
		else {
			throw "Valtan WALL_CONTACT pattern has no fixed impact target contract: $($chargeOwner.patternId)"
		}
		$targetStageIndex = $chargeStageIndex + 1
		$targetOwners = @($ownerStages | Where-Object {
			[string]$_.actionId -ceq $nextActionId })
		if ($nextActionId -cne $expectedTargetActionId -or
			$targetOwners.Count -ne 1 -or
			$targetStageIndex -ge $ownerStages.Count -or
			[string]$ownerStages[$targetStageIndex].actionId -cne
				$expectedTargetActionId -or
			[string]$ownerStages[$targetStageIndex].stageKind -cne 'GROGGY') {
			throw "Valtan WALL_CONTACT must target its immediate fixed GROGGY stage: $chargeActionId"
		}
		$partBreakCount = @($ownerStages | Where-Object {
			[string]$_.stageKind -ceq 'PART_BREAK' }).Count
		if ($partBreakCount -ne 1) {
			throw "Valtan charge impact pattern needs exactly one PART_BREAK stage: $chargeActionId"
		}
		Assert-JsonNumber $chargeOwner.maximumRange `
			"Valtan charge maximumRange $chargeActionId"
		if ([double]$chargeOwner.maximumRange -le 0.0) {
			throw "Valtan charge impact pattern has no travel distance: $chargeActionId"
		}
		if (-not $chargeImpactActionIds.Add($chargeActionId)) {
			throw "Valtan charge impact action is duplicated: $chargeActionId"
		}
		$patternRows.Add((@(
			'PATTERNSTAGECHARGE', $encounterDocument.encounterId,
			$chargeOwner.patternId, [uint32]$chargeStageIndex,
			$chargeStage.stageId, $chargeActionId) -join "`t"))
	}
}
if ($chargeImpactActionIds.Count -eq 0 -or $chargeImpactActionIds.Count -gt 16) {
	throw 'Valtan Encounter has an invalid charge-impact inventory.'
}

# The intro pattern runs exactly once per encounter epoch, before normal
# selection starts, so the first-appearance sweep cannot be rolled again later.
if (-not $patternIds.Contains([string]$encounterDocument.introPatternId)) {
	throw "Encounter introPatternId names no pattern: $($encounterDocument.introPatternId)"
}
$patternRows.Add((@(
	'ENCOUNTERINTRO', $encounterDocument.encounterId,
	$encounterDocument.introPatternId) -join "`t"))

# Missing tracking fields preserve the legacy absolute WORLD shot. Every
# presentation-relative tracking mode carries its authored origin as a pair,
# so a typo cannot silently turn a moving shot back into an absolute one.
function Read-CinematicTracking(
	[object]$Value,
	[string[]]$BaseProperties,
	[string]$Context) {
	$hasMode = $null -ne $Value.PSObject.Properties['trackingMode']
	$hasOrigin = $null -ne $Value.PSObject.Properties['trackingOrigin']
	$hasTransition = $null -ne $Value.PSObject.Properties['transitionInMs']
	$hasExitTransition = $null -ne $Value.PSObject.Properties['transitionOutMs']
	if ($hasMode -ne $hasOrigin) {
		throw "$Context must author trackingMode and trackingOrigin together."
	}
	$expectedProperties = @($BaseProperties)
	if ($hasTransition) { $expectedProperties += 'transitionInMs' }
	if ($hasExitTransition) { $expectedProperties += 'transitionOutMs' }
	if ($hasMode) { $expectedProperties += @('trackingMode','trackingOrigin') }
	Assert-ExactProperties $Value $expectedProperties $Context
	if ($hasTransition) {
		Assert-JsonInteger $Value.transitionInMs "$Context transitionInMs" 0 1000
		if ([uint32]$Value.transitionInMs -gt [uint32]$Value.durationMs) {
			throw "$Context transitionInMs exceeds durationMs."
		}
	}
	if ($hasExitTransition) {
		Assert-JsonInteger $Value.transitionOutMs "$Context transitionOutMs" 0 1000
	}
	if (-not $hasMode) {
		return [pscustomobject]@{ Mode = 'WORLD'; Origin = $null }
	}
	Assert-JsonString $Value.trackingMode "$Context trackingMode"
	$trackingModes = @('BOSS_XZ','BOSS_FACING','PLAYER_BOSS_FRAME')
	if ([string]$Value.trackingMode -cnotin $trackingModes -or
		$Value.trackingOrigin -isnot [Array] -or
		@($Value.trackingOrigin).Count -ne 3) {
		throw "$Context tracking contract is invalid."
	}
	foreach ($component in @($Value.trackingOrigin)) {
		Assert-JsonNumber $component "$Context trackingOrigin"
		if ([double]::IsNaN([double]$component) -or
			[double]::IsInfinity([double]$component) -or
			[Math]::Abs([double]$component) -gt 100000.0) {
			throw "$Context trackingOrigin is out of range."
		}
	}
	return [pscustomobject]@{
		Mode = [string]$Value.trackingMode
		Origin = @($Value.trackingOrigin)
	}
}

# A pattern that owns a landing anchor also owns every cinematic camera cue
# bound to it. WORLD cues frame the anchor directly. BOSS_XZ follows the
# replicated boss horizontally and remains exclusive to target-locked leaps.
# BOSS_FACING and PLAYER_BOSS_FRAME are presentation-relative coordinate bases,
# so their authored points are not compared with an absolute gameplay anchor.
if ($serverMotionByPatternId.Count -ne 0) {
	$cameraDocument = Read-JsonDocument 'Data/Encounters/Valtan/ValtanCinematicCamera.json'
	Assert-ExactProperties $cameraDocument @(
		'schema','formatVersion','encounterId','provenance','cues','deathCue') `
		'Valtan cinematic camera document'
	if ([string]$cameraDocument.schema -cne 'lostark.encounter-cinematic-camera' -or
		[uint32]$cameraDocument.formatVersion -ne 5 -or
		[string]$cameraDocument.encounterId -cne [string]$encounterDocument.encounterId -or
		[string]$cameraDocument.provenance -cne 'PROJECT_AUTHORED' -or
		$cameraDocument.cues -isnot [Array] -or
		@($cameraDocument.cues).Count -eq 0) {
		throw 'Valtan cinematic camera document header is invalid.'
	}
	$fourPillarsCameraStages =
		[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	foreach ($cue in @($cameraDocument.cues)) {
		$tracking = Read-CinematicTracking $cue @(
			'cueId','patternId','stageId','durationMs','easing',
			'shakeAmplitude','shakeDurationMs','keyframes') `
			"cinematic camera cue $($cue.cueId)"
		$cuePatternId = [string]$cue.patternId
		$ownsMotion = $serverMotionByPatternId.ContainsKey($cuePatternId)
		if ($tracking.Mode -ceq 'BOSS_XZ' -and -not $ownsMotion) {
			throw "BOSS_XZ cinematic cue has no pattern motion: $($cue.cueId)"
		}
		if ($cuePatternId -ceq 'VALTAN_FOUR_PILLARS_105') {
			if ($tracking.Mode -cne 'BOSS_XZ') {
				throw "The 100-bar target-leap camera cue must use BOSS_XZ tracking: $($cue.cueId)"
			}
			if (-not $fourPillarsCameraStages.Add([string]$cue.stageId)) {
				throw "Duplicate 100-bar cinematic camera stage: $($cue.stageId)"
			}
		}
		$anchor = if ($ownsMotion) {
			$serverMotionByPatternId[$cuePatternId]
		} else { $null }
		if ($tracking.Mode -ceq 'BOSS_XZ') {
			if ($anchor.Kind -cne 'LEAP_TO_TARGET' -or
				[Math]::Abs([double]$tracking.Origin[0] - $anchor.X) -gt 0.05 -or
				[Math]::Abs([double]$tracking.Origin[2] - $anchor.Z) -gt 0.05) {
				throw "BOSS_XZ cinematic origin disagrees with target-leap anchor $($anchor.AnchorId): $($cue.cueId)"
			}
		}
		foreach ($keyframe in @($cue.keyframes)) {
			if (@($keyframe.lookAt).Count -ne 3) {
				throw "Cinematic keyframe lookAt is malformed: $($cue.cueId)"
			}
			if ($ownsMotion -and $tracking.Mode -in @('WORLD','BOSS_XZ') -and
				([Math]::Abs([double]$keyframe.lookAt[0] - $anchor.X) -gt 0.05 -or
				[Math]::Abs([double]$keyframe.lookAt[2] - $anchor.Z) -gt 0.05)) {
				throw "Cinematic cue does not look at its pattern landing anchor $($anchor.AnchorId): $($cue.cueId)"
			}
		}
	}
	$expectedFourPillarsCameraStages = @('TAKEOFF','YELLOW_ZONE')
	if ($fourPillarsCameraStages.Count -ne $expectedFourPillarsCameraStages.Count -or
		@($expectedFourPillarsCameraStages | Where-Object {
			-not $fourPillarsCameraStages.Contains($_) }).Count -ne 0) {
		throw 'The 100-bar target leap camera stages must be exactly TAKEOFF,YELLOW_ZONE.'
	}
}

$expectedNormalActionIds = @((420601..420647) + (420651..420666)) |
	Where-Object { $_ -notin @(420648,420649,420650) }
foreach ($requiredActionId in $expectedNormalActionIds) {
	if (-not $coveredSourceActionIds.Contains([uint32]$requiredActionId)) {
		throw "Valtan Normal Action coverage is missing sourceActionId $requiredActionId"
	}
}

Assert-BalanceProvenance $playerDocument $skillDocument $damageDocument `
	$bossDocument $bossPartDocument $combatObjectDocument

$skillDurationById = @{}
$skillStageDurationsById = @{}
$skillStageAdvanceById = @{}
$skillKindById = @{}
foreach ($skill in @($skillDocument.skills)) {
    $skillDurationById[[string]$skill.skillId] = [uint32]$skill.actionDurationMs
    $skillStageDurationsById[[string]$skill.skillId] = @(
        @($skill.comboStages) | ForEach-Object { [uint32]$_.actionDurationMs })
    $skillStageAdvanceById[[string]$skill.skillId] = @(
        @($skill.comboStages) | ForEach-Object { [uint32]$_.comboAdvanceMs })
    $skillKindById[[string]$skill.skillId] = [string]$skill.skillKind
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
    # The boss curve lives beside the class curves but is addressed by pattern
    # and stage instead of by skill, so it is emitted by the encounter block
    # below and skipped here rather than failing this document check.
    if ([string]$document.schema -ceq 'lostark.valtan-pattern-root-motion') { continue }
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
                Assert-JsonInteger $stage.durationMs `
                    "root motion $id stage $stageIndex durationMs" 1 ([uint32]::MaxValue)
                $authoredDurationMs = [uint32]$stage.durationMs
                if ($authoredDurationMs -gt [uint32]$stageDurations[$stageIndex]) {
                    throw "Root motion stage duration exceeds gameplay duration: $id stage $stageIndex"
                }
                $samples = @($stage.samples)
                $packed = Format-RootMotionSamples `
                    -Samples $samples -SkillId $id `
                    -LimitMs $authoredDurationMs
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
        Assert-JsonInteger $entry.durationMs `
            "root motion $id durationMs" 1 ([uint32]::MaxValue)
        $authoredDurationMs = [uint32]$entry.durationMs
        if ($authoredDurationMs -gt [uint32]$skillDurationById[$id]) {
            throw "Root motion duration exceeds gameplay duration: $id"
        }
        $samples = @($entry.samples)
        $packed = Format-RootMotionSamples `
            -Samples $samples -SkillId $id -LimitMs $authoredDurationMs
        $rootMotionRows.Add((@(
            'SKILLROOTMOTION', $id, $samples.Count, $packed) -join "`t"))
    }
}

# The boss moves the way the player does: a curve the animator baked into the
# clip wins over a constant slide.  Valtan.rootmotion.json is produced by
# Tools/ValtanActionExtractor/build_valtan_rootmotion.py straight out of the
# cooked animation set, so a stage that already declares its own
# motion.distance is deliberately absent from it and keeps the authored slide.
$patternRootMotionPath = Join-Path $repoRoot "Data\Animation\RootMotion\Valtan.rootmotion.json"
if (Test-Path -LiteralPath $patternRootMotionPath) {
	$patternRootMotion = Read-JsonDocument 'Data/Animation/RootMotion/Valtan.rootmotion.json'
	Assert-ExactProperties $patternRootMotion @(
		'schema','formatVersion','bossArchetypeId','animationAssetId','patterns') 'valtan root motion document'
	if ($patternRootMotion.schema -ne 'lostark.valtan-pattern-root-motion' -or
		[uint32]$patternRootMotion.formatVersion -ne 1 -or
		[string]$patternRootMotion.bossArchetypeId -cne [string]$encounterDocument.bossArchetypeId) {
		throw "Valtan root motion header is invalid"
	}
	$rootMotionStageSeen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	foreach ($entry in @($patternRootMotion.patterns)) {
		Assert-ExactProperties $entry @('patternId','stages') 'valtan root motion pattern'
		$patternId = [string]$entry.patternId
		if (-not $patternIds.Contains($patternId)) {
			throw "Valtan root motion targets an unknown pattern: $patternId"
		}
		foreach ($stage in @($entry.stages)) {
			Assert-ExactProperties $stage @(
				'stageIndex','stageId','clip','durationMs','samples') 'valtan root motion stage'
			$stageIndex = [int]$stage.stageIndex
			$key = "{0}/{1}" -f $patternId, $stageIndex
			if (-not $patternStageDurationByKey.ContainsKey($key)) {
				throw "Valtan root motion targets an unknown stage: $key"
			}
			if (-not $rootMotionStageSeen.Add($key)) {
				throw "Duplicate Valtan root motion stage: $key"
			}
			Assert-JsonInteger $stage.durationMs "valtan root motion $key durationMs" 1 ([uint32]::MaxValue)
			$authoredDurationMs = [uint32]$stage.durationMs
			if ($authoredDurationMs -ne [uint32]$patternStageDurationByKey[$key]) {
				throw "Valtan root motion duration disagrees with the stage: $key"
			}
			$samples = @($stage.samples)
			$packed = Format-RootMotionSamples -Samples $samples -SkillId $key -LimitMs $authoredDurationMs
			$patternRows.Add((@(
				'PATTERNSTAGEROOTMOTION', $encounterDocument.encounterId,
				$patternId, $stageIndex, $samples.Count, $packed) -join "`t"))
		}
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
        [uint64]$lastFireMs = [uint64][uint32]$hit.timeMs +
            ([uint64][uint32]$hit.repeatCount - 1) *
            [uint64][uint32]$hit.repeatMs
        if ($lastFireMs -gt [uint64]$LimitMs) {
            throw "Hit shape repeat exceeds its action/stage duration: $SkillId"
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
                if ($skillKindById[$id] -eq 'COMBO') {
                    [uint64]$latestRequiredFireMs = 0
                    foreach ($hit in $hits) {
                        [uint64]$lastFireMs = [uint64][uint32]$hit.timeMs +
                            ([uint64][uint32]$hit.repeatCount - 1) *
                            [uint64][uint32]$hit.repeatMs
                        if ($lastFireMs -gt $latestRequiredFireMs) {
                            $latestRequiredFireMs = $lastFireMs
                        }
                    }
                    foreach ($projectile in $projectiles) {
                        [uint64]$spawnMs = [uint64][uint32]$projectile.timeMs
                        if ($spawnMs -gt $latestRequiredFireMs) {
                            $latestRequiredFireMs = $spawnMs
                        }
                    }
                    [uint64]$comboAdvanceMs =
                        [uint64][uint32]$skillStageAdvanceById[$id][$stageIndex]
                    if ($comboAdvanceMs -lt $latestRequiredFireMs) {
                        throw "comboAdvanceMs precedes the last caster hit/projectile spawn: $id stage $stageIndex advance=$comboAdvanceMs required=$latestRequiredFireMs"
                    }
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

# The loader reads an indexed row kind in file order and requires each index to
# equal the count it has already appended. A plain text sort puts "10" between
# "1" and "2", so a rotation, stage or hit list that reaches ten entries would
# arrive out of order. Sort on a key that pads every digit run instead, which
# leaves the emitted row text alone and keeps the row-kind grouping.
function Get-BootstrapRowSortKey {
	param([Parameter(Mandatory = $true)][string]$Row)

	$fields = @($Row.Split("`t"))
	if ($fields.Count -ge 5 -and $fields[0] -cin @(
		'PATTERNSTAGEACTION','PATTERNSTAGEVOLLEY')) {
		# Both row kinds append to the same Server stage action vector. Keep their
		# authored ordinal ahead of the concrete kind so mixed typed actions still
		# arrive as one dense 0..N-1 sequence.
		$tail = if ($fields.Count -gt 5) { @($fields[5..($fields.Count - 1)]) }
			else { @() }
		$Row = (@(
			'PATTERNSTAGEACTION', $fields[1], $fields[2], $fields[3],
			$fields[4], $fields[0]) + $tail) -join "`t"
	}
	$key = [Text.StringBuilder]::new()
	$digits = [Text.StringBuilder]::new()
	foreach ($character in $Row.ToCharArray()) {
		if ([char]::IsDigit($character)) {
			[void]$digits.Append($character)
			continue
		}
		if ($digits.Length -gt 0) {
			[void]$key.Append($digits.ToString().PadLeft(12, '0'))
			[void]$digits.Clear()
		}
		[void]$key.Append($character)
	}
	if ($digits.Length -gt 0) {
		[void]$key.Append($digits.ToString().PadLeft(12, '0'))
	}
	return $key.ToString()
}

# The merged row set carries both this branch's pattern rotations and the
# combat-runtime boss part / combat object rows, so the version moves past
# either side rather than reusing a number that meant a narrower shape.
$rows = @($damageRows + $skillRows + $playerRows + $bossRows +
	$bossPartRows + $combatObjectRows + $rootMotionRows + $hitShapeRows +
	$patternRows | Sort-Object -Property @{
		Expression = { Get-BootstrapRowSortKey -Row $_ } })
$gameplayBootstrapVersion = if ($rotationFormatVersion -eq 4) { 24 } elseif (
	$rotationFormatVersion -eq 3) { 21 } else { 18 }
$lines = @("LOSTARK_GAMEPLAY_BOOTSTRAP`t$gameplayBootstrapVersion`t$($rows.Count)") + $rows

if ($Mode -eq 'Publish') {
    $root = if ([IO.Path]::IsPathRooted($OutputRoot)) {
        [IO.Path]::GetFullPath($OutputRoot)
    }
    else {
        [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    }
    [IO.Directory]::CreateDirectory($root) | Out-Null
    $destination = Join-Path $root 'Gameplay.bootstrap'
    $staged = Join-Path $root ('.Gameplay.staging.' + [Guid]::NewGuid().ToString('N'))
    $rollback = Join-Path $root ('.Gameplay.rollback.' + [Guid]::NewGuid().ToString('N'))
	$publishMutex = $null
	$publishFailure = $null
    try {
		$mutexName = Get-PublishDestinationMutexName $destination
		$publishMutex = Enter-PublishDestinationMutex $mutexName
        [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
		$stagedHash = Get-PublishFileSha256 $staged
		$matchesPublishedOutput = Test-PublishFileHash $destination $stagedHash `
			'Gameplay balance bootstrap hash read'
		if (-not $matchesPublishedOutput) {
			$promotionCompleted = $false
			if ([IO.File]::Exists($destination)) {
				try {
					Invoke-PublishFileOperation {
						[IO.File]::Replace($staged, $destination, $rollback, $true)
					} 'Gameplay balance bootstrap replacement'
					$promotionCompleted = $true
				}
				catch {
					if (Test-PublishFileHash $destination $stagedHash `
						'Gameplay balance replacement postcondition') {
						$promotionCompleted = $true
					}
					else { throw }
				}
			}
			else {
				try {
					Invoke-PublishFileOperation {
						[IO.File]::Move($staged, $destination)
					} 'Gameplay balance bootstrap promotion'
					$promotionCompleted = $true
				}
				catch {
					if (Test-PublishFileHash $destination $stagedHash `
						'Gameplay balance promotion postcondition') {
						$promotionCompleted = $true
					}
					else { throw }
				}
			}
			if (-not $promotionCompleted) {
				throw 'Gameplay balance bootstrap promotion did not commit.'
			}
		}
    }
    catch {
		$publishFailure = $_
    }
	finally {
		foreach ($temporaryPath in @($staged, $rollback)) {
			if (-not [IO.File]::Exists($temporaryPath)) { continue }
			try {
				[IO.File]::Delete($temporaryPath)
			}
			catch {
				Write-Warning "Gameplay publisher temporary cleanup failed: $temporaryPath ($($_.Exception.Message))"
			}
		}
		Close-PublishDestinationMutex $publishMutex `
			'Gameplay balance publisher mutex'
    }
	if ($null -ne $publishFailure) { throw $publishFailure }
}

$patternCount = @($encounterDocument.patterns).Count
$stageCount = @($encounterDocument.patterns | ForEach-Object { @($_.stages).Count } |
	Measure-Object -Sum).Sum
Write-Host "Gameplay balance $Mode succeeded: $($playerRows.Count) player profiles, $($skillRows.Count) skills, $($damageRows.Count) damage profiles, $($bossIds.Count) bosses, $($bossRows.Count - $bossIds.Count) boss armour plates, $patternCount boss patterns, $stageCount pattern stages, $($timelineDocument.rows.Count) Valtan audition timeline rows."
Write-Host "Gameplay balance $Mode succeeded: $($playerRows.Count) player profiles, $($skillRows.Count) skill rows, $($damageRows.Count) damage profiles, $($bossRows.Count) bosses, $($bossPartRows.Count) boss parts, $($combatObjectIds.Count) boss combat objects, $patternCount boss patterns, $stageCount pattern stages, $($timelineDocument.rows.Count) Valtan audition timeline rows."
