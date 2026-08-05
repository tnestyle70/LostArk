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

$playerDocument = Read-Json 'Data\Balance\PlayerProfiles.json'
$skillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
$damageDocument = Read-Json 'Data\Balance\DamageProfiles.json'
$bossDocument = Read-Json 'Data\Balance\BossProfiles.json'
$encounter = Read-Json 'Data\Encounters\Valtan\ValtanEncounter.json'
$receipt = Get-Content -LiteralPath $receiptPath -Raw -Encoding UTF8 | ConvertFrom-Json

$current = @{}
function Add-Current([string]$document, [string]$targetId, [string]$field, [object]$value) {
    $key = "$document#$targetId.$field"
    if ($current.ContainsKey($key)) { throw "Duplicate authored field key: $key" }
    $current[$key] = $value
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
for ($index = 0; $index -lt @($encounter.patterns).Count; $index++) {
    $pattern = $encounter.patterns[$index]
    foreach ($property in $pattern.PSObject.Properties) {
        Add-Current 'Data/Encounters/Valtan/ValtanEncounter.json' "pattern:$($pattern.patternId)" "patterns[$index].$($property.Name)" $property.Value
    }
}

$entryByKey = @{}
foreach ($entry in @($receipt.entries)) {
    $key = "$($entry.targetDocument)#$($entry.targetId).$($entry.targetField)"
    if ($entryByKey.ContainsKey($key)) { throw "Duplicate receipt key: $key" }
    $entryByKey[$key] = $entry
}
if ($entryByKey.Count -ne $current.Count) {
    throw "Receipt coverage differs from authored fields. receipt=$($entryByKey.Count) authored=$($current.Count)"
}

$changed = 0
foreach ($key in $current.Keys) {
    if (-not $entryByKey.ContainsKey($key)) { throw "Receipt is missing authored field: $key" }
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
