[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Valtan'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,64}$'

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing document: $RelativePath" }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

$rewardsDocument = Read-JsonDocument 'Data/Valtan/Valtan.clearrewards.json'
Assert-ExactProperties $rewardsDocument @('schema', 'formatVersion', 'classes') 'clear rewards document'
if ($rewardsDocument.schema -ne 'lostark.valtan-clear-rewards' -or $rewardsDocument.formatVersion -ne 2) {
    throw 'Clear rewards header is invalid.'
}

# Each class gets its own list: its class-bound gear plus class-free accessories.
# CHARACTER_CLASS_ID names -> the item catalog's characterClass spelling.
$classCatalogNames = @{
    'LANCE_MASTER' = 'LanceMaster'; 'GUNSLINGER' = 'Gunslinger'; 'SLAYER' = 'Slayer';
    'ARTIST' = 'Artist'; 'DIMENSIONMASTER' = 'DimensionMaster'; 'WARLORD' = 'Warlord';
    'GUARDIANKNIGHT' = 'GuardianKnight'
}

# Cross-reference every itemId against the real item catalog -- a reward list
# that names an item nobody defined, or another class's gear, is a real
# authoring bug, not something to silently accept.
$catalogDocument = Read-JsonDocument 'Data/Items/ItemCatalog.json'
$catalogById = @{}
foreach ($catalogItem in @($catalogDocument.items)) {
    $catalogById[[string]$catalogItem.itemId] = $catalogItem
}

$rewardRows = [Collections.Generic.List[string]]::new()
$classNames = @($rewardsDocument.classes.PSObject.Properties.Name)
if ($classNames.Count -eq 0) { throw 'Clear rewards document names no class.' }
foreach ($className in $classNames) {
    if (-not $classCatalogNames.ContainsKey($className)) {
        throw "Clear rewards class is unknown: $className"
    }
    $items = @($rewardsDocument.classes.$className)
    if ($items.Count -eq 0 -or $items.Count -gt 64) {
        throw "Clear rewards item count is out of range for $className`: $($items.Count)"
    }
    foreach ($itemId in $items) {
        if ($itemId -isnot [string]) { throw 'Clear rewards item must be a JSON string.' }
        if ($itemId -notmatch $stableIdPattern) {
            throw "Clear rewards item is not a stable ID: '$itemId'"
        }
        if (-not $catalogById.ContainsKey($itemId)) {
            throw "Clear rewards item is not in Data/Items/ItemCatalog.json: $itemId"
        }
        $boundClass = $catalogById[$itemId].PSObject.Properties['characterClass']
        if ($null -ne $boundClass -and $boundClass.Value -ne $classCatalogNames[$className]) {
            throw "Clear rewards item $itemId belongs to $($boundClass.Value), not $className"
        }
        $rewardRows.Add("$className`t$itemId")
    }
}

if ($Mode -eq 'Validate') {
    Write-Output "Valtan clear rewards Validate succeeded: $($rewardRows.Count) items."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Clear rewards OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Clear rewards OutputRoot escaped the repository.'
}
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_VALTAN_CLEAR_REWARDS_BOOTSTRAP`t2`t$($rewardRows.Count)")
foreach ($row in $rewardRows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'ClearRewards.bootstrap'
$transactionId = [Guid]::NewGuid().ToString('N')
$staged = "$destination.staging.$transactionId"
$rollback = "$destination.rollback.$transactionId"
$hadPrevious = $false
try {
    [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
    if ([IO.File]::Exists($destination)) {
        [IO.File]::Move($destination, $rollback)
        $hadPrevious = $true
    }
    [IO.File]::Move($staged, $destination)
    if ($hadPrevious) { [IO.File]::Delete($rollback) }
    Write-Output "Valtan clear rewards Publish succeeded: $($rewardRows.Count) items -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
