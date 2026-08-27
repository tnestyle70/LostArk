[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$receiptPath = Join-Path $repoRoot 'Data\Balance\Reference\Official\2026-08-05.balance-provenance.receipt.json'

function Read-Json([string]$relativePath) {
    $path = Join-Path $repoRoot $relativePath
    if (-not [IO.File]::Exists($path)) { throw "Missing authoring document: $relativePath" }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function ConvertTo-Comparable([object]$value) {
    if ($null -eq $value) { return 'null' }
    return ($value | ConvertTo-Json -Compress -Depth 32)
}

function Get-CanonicalTextSha256([string]$path) {
	$strictUtf8 = [Text.UTF8Encoding]::new($false, $true)
	$text = [IO.File]::ReadAllText($path, $strictUtf8)
	$bytes = [Text.UTF8Encoding]::new($false).GetBytes(
		$text.Replace("`r`n", "`n").Replace("`r", "`n"))
	$sha256 = [Security.Cryptography.SHA256]::Create()
	try {
		return ([BitConverter]::ToString($sha256.ComputeHash($bytes))).Replace('-', '').ToLowerInvariant()
	}
	finally { $sha256.Dispose() }
}

$playerDocument = Read-Json 'Data\Balance\PlayerProfiles.json'
$skillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
$damageDocument = Read-Json 'Data\Balance\DamageProfiles.json'
$bossDocument = Read-Json 'Data\Balance\BossProfiles.json'
$bossPartDocument = Read-Json 'Data\Balance\ValtanBossParts.json'
$combatObjectDocument = Read-Json `
	'Data\Encounters\Valtan\ValtanCombatObjects.json'
$encounter = Read-Json 'Data\Encounters\Valtan\ValtanEncounter.json'
$receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json

$current = @{}
$currentMetadata = @{}
function Add-Current([string]$document, [string]$targetId, [string]$field, [object]$value) {
    $key = "$document#$targetId.$field"
    if ($current.ContainsKey($key)) { throw "Duplicate authored field key: $key" }
    $current[$key] = $value
	$currentMetadata[$key] = [pscustomobject]@{
		document = $document
		targetId = $targetId
		field = $field
	}
}

foreach ($player in @($playerDocument.players)) {
    foreach ($property in $player.PSObject.Properties) {
        Add-Current 'Data/Balance/PlayerProfiles.json' "player:$($player.characterClass)" $property.Name $property.Value
    }
}
foreach ($skill in @($skillDocument.skills)) {
    $targetId = "skill:$($skill.skillId)"
    foreach ($property in $skill.PSObject.Properties) {
        if ($property.Name -ne 'comboStages') {
            Add-Current 'Data/Balance/PlayerSkills.json' $targetId $property.Name $property.Value
        }
    }
    $stages = @($skill.comboStages)
    Add-Current 'Data/Balance/PlayerSkills.json' $targetId 'comboStages.length' $stages.Count
    for ($index = 0; $index -lt $stages.Count; $index++) {
        foreach ($property in $stages[$index].PSObject.Properties) {
            Add-Current 'Data/Balance/PlayerSkills.json' $targetId "comboStages[$index].$($property.Name)" $property.Value
        }
    }
}
foreach ($profile in @($damageDocument.profiles)) {
    foreach ($property in $profile.PSObject.Properties) {
        Add-Current 'Data/Balance/DamageProfiles.json' "damage:$($profile.damageProfileId)" $property.Name $property.Value
    }
}
foreach ($boss in @($bossDocument.bosses)) {
    foreach ($property in $boss.PSObject.Properties) {
        Add-Current 'Data/Balance/BossProfiles.json' "boss:$($boss.archetypeId)" $property.Name $property.Value
    }
}
$bossPartPath = 'Data/Balance/ValtanBossParts.json'
$bossPartRoot = "boss-parts:$($bossPartDocument.bossArchetypeId)"
Add-Current $bossPartPath $bossPartRoot 'bossArchetypeId' `
	$bossPartDocument.bossArchetypeId
Add-Current $bossPartPath $bossPartRoot 'parts.length' `
	@($bossPartDocument.parts).Count
foreach ($part in @($bossPartDocument.parts)) {
	foreach ($property in $part.PSObject.Properties) {
		Add-Current $bossPartPath "boss-part:$($part.partId)" `
			$property.Name $property.Value
	}
}
$combatObjectPath = 'Data/Encounters/Valtan/ValtanCombatObjects.json'
$combatObjectRoot = "combat-objects:$($combatObjectDocument.encounterId)"
Add-Current $combatObjectPath $combatObjectRoot 'encounterId' `
	$combatObjectDocument.encounterId
Add-Current $combatObjectPath $combatObjectRoot 'objects.length' `
	@($combatObjectDocument.objects).Count
foreach ($combatObject in @($combatObjectDocument.objects)) {
	foreach ($property in $combatObject.PSObject.Properties) {
		Add-Current $combatObjectPath `
			"combat-object:$($combatObject.combatObjectArchetypeId)" `
			$property.Name $property.Value
	}
}
$encounterTarget = "encounter:$($encounter.encounterId)"
foreach ($field in @('encounterId','bossArchetypeId','authority','fixedTickHz')) {
    Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget $field $encounter.$field
}
Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget 'states.length' @($encounter.states).Count
for ($index = 0; $index -lt @($encounter.states).Count; $index++) {
    foreach ($property in $encounter.states[$index].PSObject.Properties) {
        Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget "states[$index].$($property.Name)" $property.Value
    }
}
Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' $encounterTarget 'patterns.length' @($encounter.patterns).Count
$liveEncounterPatterns = @($encounter.patterns | Where-Object {
	[string]$_.selectionMode -cne 'AUDITION_ONLY'
})
for ($index = 0; $index -lt $liveEncounterPatterns.Count; $index++) {
    $pattern = $liveEncounterPatterns[$index]
    foreach ($property in $pattern.PSObject.Properties) {
        Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' "pattern:$($pattern.patternId)" "patterns[$index].$($property.Name)" $property.Value
    }
}

$entryByKey = @{}
$retainedEntries = [Collections.Generic.List[object]]::new()
foreach ($entry in @($receipt.entries)) {
    $key = "$($entry.targetDocument)#$($entry.targetId).$($entry.targetField)"
    if ($entryByKey.ContainsKey($key)) { throw "Duplicate receipt key: $key" }
	if (-not $current.ContainsKey($key)) { continue }
    $entryByKey[$key] = $entry
	$retainedEntries.Add($entry)
}
$receipt.entries = @($retainedEntries)

$changed = 0
foreach ($key in $current.Keys) {
	if (-not $entryByKey.ContainsKey($key)) {
		$value = $current[$key]
		$metadata = $currentMetadata[$key]
		$entry = [pscustomobject][ordered]@{
			targetDocument = $metadata.document
			targetId = $metadata.targetId
			targetField = $metadata.field
			basis = 'PROJECT_TUNED'
			source = [pscustomobject]@{
				type = 'project-policy'
				policyId = 'balance-tool-authored-override-v1'
			}
			sourceValue = $value
			transform = 'Balance Tool authored override'
			resultValue = $value
			note = 'Added through the F1 Balance Tool authoring contract; official source binding is not claimed.'
		}
		$entryByKey[$key] = $entry
		$receipt.entries += $entry
		++$changed
		continue
	}
    $entry = $entryByKey[$key]
    $value = $current[$key]
    if ((ConvertTo-Comparable $entry.resultValue) -cne (ConvertTo-Comparable $value)) {
        $entry.basis = 'PROJECT_TUNED'
        $entry.source = [pscustomobject]@{
            type = 'project-policy'
            policyId = 'balance-tool-authored-override-v1'
        }
        $entry.sourceValue = $value
        $entry.transform = 'Balance Tool authored override'
        $entry.resultValue = $value
        if ($entry.PSObject.Properties.Name -contains 'note') {
            $entry.note = 'Changed through the F1 Balance Tool; re-export official sources to restore an official basis.'
        }
        else {
            $entry | Add-Member -NotePropertyName note -NotePropertyValue `
                'Changed through the F1 Balance Tool; re-export official sources to restore an official basis.'
        }
        ++$changed
    }
}

$receipt.entries = @($receipt.entries | Sort-Object targetDocument,targetId,targetField)
$receipt.extractorSha256 = Get-CanonicalTextSha256 (
	Join-Path $repoRoot 'Tools\GameplayPipeline\Export-OfficialBalanceReceipt.py')
$receipt.coverage.playerProfileCount = @($playerDocument.players).Count
$receipt.coverage.skillDefinitionCount = @($skillDocument.skills).Count
$receipt.coverage.damageProfileCount = @($damageDocument.profiles).Count
$receipt.coverage.bossProfileCount = @($bossDocument.bosses).Count
if ($receipt.coverage.PSObject.Properties.Name -notcontains 'bossPartCount') {
	$receipt.coverage | Add-Member -NotePropertyName bossPartCount `
		-NotePropertyValue 0
}
$receipt.coverage.bossPartCount = @($bossPartDocument.parts).Count
if ($receipt.coverage.PSObject.Properties.Name -notcontains `
	'bossCombatObjectCount') {
	$receipt.coverage | Add-Member -NotePropertyName bossCombatObjectCount `
		-NotePropertyValue 0
}
$receipt.coverage.bossCombatObjectCount = `
	@($combatObjectDocument.objects).Count
$receipt.coverage.encounterPatternCount = $liveEncounterPatterns.Count
$receipt.coverage.fieldEntryCount = $current.Count
$serialized = $receipt | ConvertTo-Json -Depth 32
$temporary = "$receiptPath.tmp.$PID"
$backup = "$receiptPath.rollback.$PID"
[IO.File]::WriteAllText($temporary, $serialized + "`n", [Text.UTF8Encoding]::new($false))
try {
    $verify = Get-Content -LiteralPath $temporary -Raw -Encoding UTF8 | ConvertFrom-Json
    if (@($verify.entries).Count -ne $current.Count) { throw 'Temporary receipt verification failed.' }
    [IO.File]::Replace($temporary, $receiptPath, $backup)
    if ([IO.File]::Exists($backup)) { [IO.File]::Delete($backup) }
}
finally {
    if ([IO.File]::Exists($temporary)) { [IO.File]::Delete($temporary) }
}

Write-Host "Balance provenance synchronized: $changed field(s) changed to PROJECT_TUNED."
