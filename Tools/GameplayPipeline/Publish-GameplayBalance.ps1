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

$damageDocument = Read-JsonDocument 'Data/Balance/DamageProfiles.json'
Assert-ExactProperties $damageDocument @('schema','formatVersion','profiles') 'damage document'
if ($damageDocument.schema -ne 'lostark.damage-profiles' -or $damageDocument.formatVersion -ne 1) {
    throw 'Damage profile header is invalid.'
}
$damageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$damageRows = [Collections.Generic.List[string]]::new()
foreach ($profile in @($damageDocument.profiles)) {
    Assert-ExactProperties $profile @('damageProfileId','amount') 'damage profile'
    Assert-StableId $profile.damageProfileId 'damageProfileId'
    if (-not $damageIds.Add([string]$profile.damageProfileId) -or [uint32]$profile.amount -eq 0) {
        throw "Duplicate or zero damage profile: $($profile.damageProfileId)"
    }
    $damageRows.Add("DAMAGE`t$($profile.damageProfileId)`t$([uint32]$profile.amount)")
}

$skillDocument = Read-JsonDocument 'Data/Balance/PlayerSkills.json'
Assert-ExactProperties $skillDocument @('schema','formatVersion','skills') 'skill document'
if ($skillDocument.schema -ne 'lostark.player-skills' -or $skillDocument.formatVersion -ne 1) {
    throw 'Player skill header is invalid.'
}

$playerDocument = Read-JsonDocument 'Data/Balance/PlayerProfiles.json'
Assert-ExactProperties $playerDocument @('schema','formatVersion','players') 'player profile document'
if ($playerDocument.schema -ne 'lostark.player-profiles' -or $playerDocument.formatVersion -ne 1) {
	throw 'Player profile header is invalid.'
}
$playerClasses = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$playerRows = [Collections.Generic.List[string]]::new()
$supportedPlayerClasses = @('LANCE_MASTER','GUNSLINGER','SLAYER','ARTIST')
foreach ($player in @($playerDocument.players)) {
	Assert-ExactProperties $player @('characterClass','maximumHp','maximumResource','moveSpeed') 'player profile'
	Assert-StableId $player.characterClass 'player characterClass'
	if ($player.characterClass -notin $supportedPlayerClasses -or
		-not $playerClasses.Add([string]$player.characterClass) -or
		[uint32]$player.maximumHp -eq 0 -or [uint32]$player.maximumResource -eq 0) {
		throw "Player profile is invalid: $($player.characterClass)"
	}
	$playerRows.Add((@(
		'PLAYER', $player.characterClass, [uint32]$player.maximumHp,
		[uint32]$player.maximumResource,
		(Format-InvariantFloat $player.moveSpeed 'player moveSpeed')) -join "`t"))
}
if ($playerClasses.Count -ne $supportedPlayerClasses.Count) {
	$missingClasses = @($supportedPlayerClasses | Where-Object { -not $playerClasses.Contains($_) })
	throw "Player profiles are incomplete. missing=[$($missingClasses -join ',')]"
}
$skillIds = [Collections.Generic.HashSet[uint32]]::new()
$inputSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$skillRows = [Collections.Generic.List[string]]::new()
foreach ($skill in @($skillDocument.skills)) {
    Assert-ExactProperties $skill @(
		'skillId','characterClass','inputSlot','displayName','actionId','cooldownMs','actionDurationMs',
        'hitTimeMs','resourceCost','movementDistance','maximumRange','serverDamageProfileId') 'player skill'
    Assert-StableId $skill.characterClass 'characterClass'
    Assert-StableId $skill.inputSlot 'inputSlot'
    Assert-StableId $skill.actionId 'actionId'
    Assert-StableId $skill.serverDamageProfileId 'serverDamageProfileId'
	if ([string]::IsNullOrWhiteSpace([string]$skill.displayName) -or ([string]$skill.displayName).Length -gt 64) {
		throw "Skill displayName is invalid: $($skill.skillId)"
	}
    $id = [uint32]$skill.skillId
    if ($id -eq 0 -or -not $skillIds.Add($id) -or
        -not $inputSlots.Add("$($skill.characterClass):$($skill.inputSlot)")) {
        throw "Duplicate skill ID or input slot: $id"
    }
    if ($skill.characterClass -ne 'LANCE_MASTER' -or $skill.inputSlot -notin @('Q','W') -or
        [uint32]$skill.cooldownMs -eq 0 -or [uint32]$skill.actionDurationMs -eq 0 -or
        [uint32]$skill.hitTimeMs -eq 0 -or [uint32]$skill.hitTimeMs -gt [uint32]$skill.actionDurationMs -or
        [uint32]$skill.resourceCost -gt 100 -or -not $damageIds.Contains([string]$skill.serverDamageProfileId)) {
        throw "Player skill timing, class, slot, resource, or damage reference is invalid: $id"
    }
    $skillRows.Add((@(
        'SKILL', $id, $skill.characterClass, $skill.inputSlot, $skill.actionId,
        [uint32]$skill.cooldownMs, [uint32]$skill.actionDurationMs, [uint32]$skill.hitTimeMs,
        [uint32]$skill.resourceCost,
        (Format-InvariantFloat $skill.movementDistance "skill $id movementDistance"),
        (Format-InvariantFloat $skill.maximumRange "skill $id maximumRange"),
        $skill.serverDamageProfileId) -join "`t"))
}

$bossDocument = Read-JsonDocument 'Data/Balance/BossProfiles.json'
Assert-ExactProperties $bossDocument @('schema','formatVersion','bosses') 'boss document'
if ($bossDocument.schema -ne 'lostark.boss-profiles' -or $bossDocument.formatVersion -ne 1) {
    throw 'Boss profile header is invalid.'
}
$bossIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$bossRows = [Collections.Generic.List[string]]::new()
foreach ($boss in @($bossDocument.bosses)) {
    Assert-ExactProperties $boss @(
		'archetypeId','encounterId','displayName','maximumHp','engageDistance','moveSpeed','phaseTwoHpPercent') 'boss profile'
    Assert-StableId $boss.archetypeId 'boss archetypeId'
    Assert-StableId $boss.encounterId 'boss encounterId'
	if ([string]::IsNullOrWhiteSpace([string]$boss.displayName) -or ([string]$boss.displayName).Length -gt 64) {
		throw "Boss displayName is invalid: $($boss.archetypeId)"
	}
    if (-not $bossIds.Add([string]$boss.archetypeId) -or [uint32]$boss.maximumHp -eq 0 -or
        [uint32]$boss.phaseTwoHpPercent -eq 0 -or [uint32]$boss.phaseTwoHpPercent -ge 100) {
        throw "Boss profile is invalid: $($boss.archetypeId)"
    }
    $bossRows.Add((@(
        'BOSS', $boss.archetypeId, $boss.encounterId, [uint32]$boss.maximumHp,
        (Format-InvariantFloat $boss.engageDistance 'boss engageDistance'),
        (Format-InvariantFloat $boss.moveSpeed 'boss moveSpeed'),
        [uint32]$boss.phaseTwoHpPercent) -join "`t"))
}

$rows = @($damageRows + $skillRows + $playerRows + $bossRows | Sort-Object)
$lines = @("LOSTARK_GAMEPLAY_BOOTSTRAP`t1`t$($rows.Count)") + $rows

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

Write-Host "Gameplay balance $Mode succeeded: $($playerRows.Count) player profiles, $($skillRows.Count) skills, $($damageRows.Count) damage profiles, $($bossRows.Count) bosses."
