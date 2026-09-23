[CmdletBinding()]
param([string]$EncounterPath = 'Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json')
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$script:projectJsonSnapshots = @{}
. (Join-Path $repoRoot 'Tools/KoukuSaydonPipeline/KoukuParentSequenceContract.ps1')

# Import only the actual publisher functions: never execute its Publish body.
$publisher = Join-Path $PSScriptRoot 'Publish-WorldGameplay.ps1'
$tokens = $null; $errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile($publisher, [ref]$tokens, [ref]$errors)
if (@($errors).Count) { throw ($errors -join "`n") }
foreach ($function in $ast.FindAll({ param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] }, $false)) {
    . ([scriptblock]::Create($function.Extent.Text))
}
$script:originalReadProjectJson = (Get-Command Read-ProjectJson).ScriptBlock
$resolved = if ([IO.Path]::IsPathRooted($EncounterPath)) { $EncounterPath } else { Join-Path $repoRoot $EncounterPath }
$script:parentContractCandidate = [IO.File]::ReadAllText($resolved, [Text.Encoding]::UTF8) | ConvertFrom-Json
function Read-ProjectJson([string]$RelativePath) {
    if ($RelativePath -ceq 'Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json') { return $script:parentContractCandidate }
    return & $script:originalReadProjectJson $RelativePath
}
$profiles = Get-EncounterProfiles
$document = $profiles['ENCOUNTER_KAKULSAYDON_G1'].Document
$parentCount = @($document.patterns | Where-Object { $null -ne $_.PSObject.Properties['parentPatternSequence'] }).Count
Write-Output "World encounter consumer PASS: source=$($document.sourceRevision), patterns=$(@($document.patterns).Count), parents=$parentCount"

$fixture = @'
{"patterns":[
 {"patternId":"parent","gateId":"BINGO","targetBossPlacementId":"boss","actorProfileId":"actor","timelineDurationMs":2000,
  "parentPatternSequence":{"loopStartOccurrenceId":"second","entries":[
   {"occurrenceId":"first","patternId":"child1","startMs":0,"durationMs":1000,"repeat":false},
   {"occurrenceId":"second","patternId":"child2","startMs":1000,"durationMs":1000,"repeat":false}]}},
 {"patternId":"child1","gateId":"BINGO","targetBossPlacementId":"boss","actorProfileId":"actor","stages":[{"durationMs":1000}]},
 {"patternId":"child2","gateId":"BINGO","targetBossPlacementId":"boss","actorProfileId":"actor","stages":[{"durationMs":1000}]}]}
'@
$valid = $fixture | ConvertFrom-Json
Assert-KoukuParentPatternSequence $valid.patterns[0] $valid.patterns
$valid.patterns[0].parentPatternSequence.loopStartOccurrenceId = ''
foreach ($pattern in $valid.patterns) { $pattern.gateId = 'GATE1' }
Assert-KoukuParentPatternSequence $valid.patterns[0] $valid.patterns
$mutations = @{
    missingChild = { param($d) $d.patterns[0].parentPatternSequence.entries[0].patternId = 'missing' }
    wrongBoss = { param($d) $d.patterns[1].targetBossPlacementId = 'different' }
    overlap = { param($d) $d.patterns[0].parentPatternSequence.entries[1].startMs = 999 }
    trimmedDuration = { param($d) $d.patterns[0].parentPatternSequence.entries[0].durationMs = 999 }
    duplicateOccurrence = { param($d) $d.patterns[0].parentPatternSequence.entries[1].occurrenceId = 'first' }
    missingLoop = { param($d) $d.patterns[0].parentPatternSequence.loopStartOccurrenceId = 'missing' }
    nonBingoLoop = { param($d) foreach ($p in $d.patterns) { $p.gateId = 'GATE1' } }
    invalidRepeat = { param($d) $d.patterns[0].parentPatternSequence.entries[0].repeat = 1 }
    invalidTotal = { param($d) $d.patterns[0].timelineDurationMs = 2001 }
    unknownProperty = { param($d) $d.patterns[0].parentPatternSequence | Add-Member -NotePropertyName extra -NotePropertyValue 1 }
    nestedParent = { param($d) $d.patterns[1] | Add-Member -NotePropertyName parentPatternSequence -NotePropertyValue $d.patterns[0].parentPatternSequence }
}
foreach ($name in $mutations.Keys) {
    $invalid = $fixture | ConvertFrom-Json
    & $mutations[$name] $invalid
    $rejected = $false
    try { Assert-KoukuParentPatternSequence $invalid.patterns[0] $invalid.patterns } catch { $rejected = $true }
    if (-not $rejected) { throw "Parent contract accepted invalid case: $name" }
}
Write-Output "Shared Parent contract PASS: 2 valid modes, $($mutations.Count) invalid cases; no authoring or runtime writes"
