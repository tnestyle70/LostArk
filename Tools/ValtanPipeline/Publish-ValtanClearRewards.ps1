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
Assert-ExactProperties $rewardsDocument @('schema', 'formatVersion', 'items') 'clear rewards document'
if ($rewardsDocument.schema -ne 'lostark.valtan-clear-rewards' -or $rewardsDocument.formatVersion -ne 1) {
    throw 'Clear rewards header is invalid.'
}

$items = @($rewardsDocument.items)
if ($items.Count -eq 0 -or $items.Count -gt 64) {
    throw "Clear rewards item count is out of range: $($items.Count)"
}

# Cross-reference every itemId against the real item catalog -- a reward list
# that names an item nobody defined is a real authoring bug, not something to
# silently accept.
$catalogDocument = Read-JsonDocument 'Data/Items/ItemCatalog.json'
$catalogIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($catalogItem in @($catalogDocument.items)) {
    [void]$catalogIds.Add([string]$catalogItem.itemId)
}

$rewardRows = [Collections.Generic.List[string]]::new()
foreach ($itemId in $items) {
    if ($itemId -isnot [string]) { throw 'Clear rewards item must be a JSON string.' }
    if ($itemId -notmatch $stableIdPattern) {
        throw "Clear rewards item is not a stable ID: '$itemId'"
    }
    if (-not $catalogIds.Contains($itemId)) {
        throw "Clear rewards item is not in Data/Items/ItemCatalog.json: $itemId"
    }
    $rewardRows.Add($itemId)
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
$lines.Add("LOSTARK_VALTAN_CLEAR_REWARDS_BOOTSTRAP`t1`t$($rewardRows.Count)")
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
