[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'CheckPublished')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Items'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,64}$'
. (Join-Path $PSScriptRoot 'Publish-FileTransaction.ps1')
$publishSources = @{}

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing item document: $RelativePath" }
    return Read-PublishJsonSnapshot $path $publishSources
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

# Required fields must all be present; optional ones may be absent. Nothing else is allowed.
function Assert-Properties([object]$Value, [string[]]$Required, [string[]]$Optional, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name)
    foreach ($name in $Required) {
        if ($actual -cnotcontains $name) { throw "$Context is missing field '$name'." }
    }
    foreach ($name in $actual) {
        if (($Required -cnotcontains $name) -and ($Optional -cnotcontains $name)) {
            throw "$Context has an unknown field '$name'."
        }
    }
}

$itemDocument = Read-JsonDocument 'Data/Items/ItemCatalog.json'
Assert-Properties $itemDocument @('schema', 'formatVersion', 'items') @('currencies', 'shops') 'item catalog document'
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
$startingSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($item in $items) {
    # grade is a Client presentation field (inventory grade art). equipSlot and characterClass
    # also travel to the Server, which checks them on equip; startingEquippedSlot names the
    # equipment slot a fresh character already wears the item in. "-" marks an absent value.
    Assert-Properties $item @('itemId', 'displayName', 'maxStack', 'iconPath', 'healPercent', 'category') `
        @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot') 'item'
    Assert-JsonString $item.itemId 'item itemId'
    Assert-JsonString $item.displayName 'item displayName'
    Assert-JsonInteger $item.maxStack 'item maxStack' 1 ([uint32]::MaxValue)
    Assert-JsonString $item.iconPath 'item iconPath'
    Assert-JsonInteger $item.healPercent 'item healPercent' 0 100
    Assert-JsonString $item.category 'item category'
    if ($item.category -ne 'combat' -and $item.category -ne 'use') {
        throw "item category must be 'combat' or 'use': $($item.itemId)"
    }
    foreach ($optional in @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot')) {
        if ($null -ne $item.PSObject.Properties[$optional]) {
            Assert-JsonString $item.$optional "item $optional"
        }
    }
    if ($null -ne $item.PSObject.Properties['equipSlot']) {
        $slots = @('weapon', 'helmet', 'shoulder', 'top', 'pants', 'gloves', 'necklace', 'earring', 'ring', 'stone', 'bracelet', 'avatarHead', 'avatarOutfit')
        if ($slots -cnotcontains $item.equipSlot) { throw "item equipSlot is unknown: $($item.itemId)" }
    }
    if ($null -ne $item.PSObject.Properties['grade']) {
        if (@('normal', 'rare', 'epic', 'legend', 'relic', 'ancient', 'avatar') -cnotcontains $item.grade) {
            throw "item grade is unknown: $($item.itemId)"
        }
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
    $equipSlotField = if ($null -ne $item.PSObject.Properties['equipSlot']) { [string]$item.equipSlot } else { '-' }
    $classField = if ($null -ne $item.PSObject.Properties['characterClass']) { [string]$item.characterClass } else { '-' }
    $startingField = '-'
    if ($null -ne $item.PSObject.Properties['startingEquippedSlot']) {
        # The slot must take the item's kind (earring1/earring2 take "earring", ring1/ring2 "ring"),
        # the item must fit every class, and no two items may start in one slot.
        $startingField = [string]$item.startingEquippedSlot
        $startingKind = $startingField -replace '[12]$', ''
        if ($equipSlotField -ceq '-' -or $startingKind -cne $equipSlotField -or
            @('earring', 'ring') -ccontains $startingField -or
            @('helmet', 'shoulder', 'top', 'pants', 'gloves', 'weapon', 'necklace', 'earring', 'ring', 'stone', 'bracelet') -cnotcontains $startingKind) {
            throw "item startingEquippedSlot does not fit its equipSlot: $($item.itemId)"
        }
        if ($classField -cne '-') { throw "item startingEquippedSlot must be class-free: $($item.itemId)" }
        if (-not $startingSlots.Add($startingField)) { throw "Two items start in slot $startingField" }
    }
    $itemRows.Add((@('ITEM', $item.itemId, [uint32]$item.maxStack, [uint32]$item.healPercent, $equipSlotField, $classField, $startingField) -join "`t"))
}

# Currencies are the player's purse (실링, 골드), not bag items. The Server knows exactly these
# two; startingAmount is what a fresh character is given.
$currencyRows = [Collections.Generic.List[string]]::new()
$currencyIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($currency in @($itemDocument.currencies | Where-Object { $null -ne $_ })) {
    Assert-ExactProperties $currency @('currencyId', 'displayName', 'iconPath', 'startingAmount') 'currency'
    Assert-JsonString $currency.currencyId 'currency currencyId'
    Assert-JsonString $currency.displayName 'currency displayName'
    Assert-JsonString $currency.iconPath 'currency iconPath'
    Assert-JsonInteger $currency.startingAmount 'currency startingAmount' 0 999999999
    if (@('SILVER', 'GOLD') -cnotcontains $currency.currencyId) { throw "currency is unknown: $($currency.currencyId)" }
    if (-not $currencyIds.Add([string]$currency.currencyId)) { throw "Duplicate currency: $($currency.currencyId)" }
    $currencyRows.Add((@('CURRENCY', $currency.currencyId, [uint32]$currency.startingAmount) -join "`t"))
}

# NPC shops: which NPC placements run each shop, and what each sells for which currency item.
# The shop window lays stock out on a ten-cell page, so a shop sells at most ten lines.
$shopRows = [Collections.Generic.List[string]]::new()
$shopIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$shopNpcIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($shop in @($itemDocument.shops | Where-Object { $null -ne $_ })) {
    Assert-ExactProperties $shop @('shopId', 'npcPlacementIds', 'items') 'shop'
    Assert-JsonString $shop.shopId 'shop shopId'
    if ($shop.shopId -notmatch $stableIdPattern) { throw "shop shopId is not a stable ID: '$($shop.shopId)'" }
    if (-not $shopIds.Add([string]$shop.shopId)) { throw "Duplicate shop ID: $($shop.shopId)" }
    $npcIds = @($shop.npcPlacementIds)
    if ($npcIds.Count -eq 0) { throw "shop names no NPC: $($shop.shopId)" }
    foreach ($npcId in $npcIds) {
        Assert-JsonString $npcId 'shop npcPlacementId'
        if ($npcId -notmatch $stableIdPattern) { throw "shop npcPlacementId is not a stable ID: '$npcId'" }
        if (-not $shopNpcIds.Add([string]$npcId)) { throw "NPC runs two shops: $npcId" }
        $shopRows.Add((@('SHOPNPC', $shop.shopId, $npcId) -join "`t"))
    }
    $stock = @($shop.items)
    if ($stock.Count -eq 0 -or $stock.Count -gt 10) { throw "shop stock count is out of range: $($shop.shopId)" }
    $stockIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($line in $stock) {
        Assert-ExactProperties $line @('itemId', 'currencyId', 'price') 'shop item'
        Assert-JsonString $line.itemId 'shop item itemId'
        Assert-JsonString $line.currencyId 'shop item currencyId'
        Assert-JsonInteger $line.price 'shop item price' 1 999999999
        if (-not $itemIds.Contains([string]$line.itemId)) { throw "shop sells an unknown item: $($line.itemId)" }
        if (-not $currencyIds.Contains([string]$line.currencyId)) { throw "shop charges an unknown currency: $($line.currencyId)" }
        if (-not $stockIds.Add([string]$line.itemId)) { throw "shop lists an item twice: $($shop.shopId) $($line.itemId)" }
        $shopRows.Add((@('SHOPITEM', $shop.shopId, $line.itemId, $line.currencyId, [uint32]$line.price) -join "`t"))
    }
}

if ($Mode -eq 'Validate') {
    Write-Output "Item catalog Validate succeeded: $($itemRows.Count) items, $($currencyRows.Count) currencies, $($shopIds.Count) shops."
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

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_ITEM_BOOTSTRAP`t5`t$($itemRows.Count + $currencyRows.Count + $shopRows.Count)")
foreach ($row in $itemRows) { $lines.Add($row) }
foreach ($row in $currencyRows) { $lines.Add($row) }
foreach ($row in $shopRows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Items.bootstrap'
Write-PublishTextCatalog -Mode $Mode -Destination $destination -Lines $lines `
    -Sources $publishSources -Context 'Item catalog' `
    -RepairCommand 'powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish'
