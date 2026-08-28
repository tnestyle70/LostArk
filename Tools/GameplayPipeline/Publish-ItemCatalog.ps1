[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Items'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,64}$'

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing item document: $RelativePath" }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-JsonInteger([object]$Value, [string]$Context, [long]$Minimum, [long]$Maximum) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonString([object]$Value, [string]$Context) {
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

$itemDocument = Read-JsonDocument 'Data/Items/ItemCatalog.json'
Assert-ExactProperties $itemDocument @('schema', 'formatVersion', 'items') 'item catalog document'
Assert-JsonString $itemDocument.schema 'item catalog schema'
Assert-JsonInteger $itemDocument.formatVersion 'item catalog formatVersion' 2 2
if ($itemDocument.schema -ne 'lostark.item-catalog' -or $itemDocument.formatVersion -ne 2) {
    throw 'Item catalog header is invalid.'
}

$items = @($itemDocument.items)
if ($items.Count -eq 0 -or $items.Count -gt 4096) {
    throw "Item catalog item count is out of range: $($items.Count)"
}

$itemIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$itemRows = [Collections.Generic.List[string]]::new()
foreach ($item in $items) {
    Assert-ExactProperties $item @('itemId', 'displayName', 'maxStack', 'iconPath', 'healPercent', 'category') 'item'
    Assert-JsonString $item.itemId 'item itemId'
    Assert-JsonString $item.displayName 'item displayName'
    Assert-JsonInteger $item.maxStack 'item maxStack' 1 ([uint32]::MaxValue)
    Assert-JsonString $item.iconPath 'item iconPath'
    Assert-JsonInteger $item.healPercent 'item healPercent' 0 100
    Assert-JsonString $item.category 'item category'
    if ($item.category -ne 'combat' -and $item.category -ne 'use') {
        throw "item category must be 'combat' or 'use': $($item.itemId)"
    }
    if ($item.itemId -notmatch $stableIdPattern) {
        throw "item itemId is not a stable ID: '$($item.itemId)'"
    }
    if ([string]::IsNullOrWhiteSpace([string]$item.displayName) -or
        ([string]$item.displayName).Length -gt 64) {
        throw "item displayName is invalid: $($item.itemId)"
    }
    if (-not $itemIds.Add([string]$item.itemId)) {
        throw "Duplicate item ID: $($item.itemId)"
    }
    $itemRows.Add((@('ITEM', $item.itemId, [uint32]$item.maxStack, [uint32]$item.healPercent) -join "`t"))
}

if ($Mode -eq 'Validate') {
    Write-Output "Item catalog Validate succeeded: $($itemRows.Count) items."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Item catalog OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Item catalog OutputRoot escaped the repository.'
}
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_ITEM_BOOTSTRAP`t2`t$($itemRows.Count)")
foreach ($row in $itemRows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Items.bootstrap'
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
    Write-Output "Item catalog Publish succeeded: $($itemRows.Count) items -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
