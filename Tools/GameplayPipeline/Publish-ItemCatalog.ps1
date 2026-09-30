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
$battleRows = [Collections.Generic.List[string]]::new()
$startingSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
# The Server's CHARACTER_CLASS_ID names as ItemCatalog.json spells characterClass.
$classNames = @('LanceMaster', 'Gunslinger', 'Slayer', 'Artist', 'DimensionMaster', 'Warlord', 'GuardianKnight')
$itemById = @{}
foreach ($item in $items) {
    # grade is a Client presentation field (inventory grade art). equipSlot and characterClass
    # also travel to the Server, which checks them on equip; startingEquippedSlot names the
    # equipment slot a fresh character already wears the item in. "-" marks an absent value.
    Assert-Properties $item @('itemId', 'displayName', 'maxStack', 'iconPath', 'healPercent', 'category') `
        @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot', 'battleUse', 'visualSetId', 'classVariants') 'item'
    Assert-JsonString $item.itemId 'item itemId'
    Assert-JsonString $item.displayName 'item displayName'
    Assert-JsonInteger $item.maxStack 'item maxStack' 1 ([uint32]::MaxValue)
    Assert-JsonString $item.iconPath 'item iconPath'
    Assert-JsonInteger $item.healPercent 'item healPercent' 0 100
    Assert-JsonString $item.category 'item category'
    if ($item.category -ne 'combat' -and $item.category -ne 'use') {
        throw "item category must be 'combat' or 'use': $($item.itemId)"
    }
    foreach ($optional in @('equipSlot', 'characterClass', 'grade', 'startingEquippedSlot', 'visualSetId')) {
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
    if ($null -ne $item.PSObject.Properties['characterClass']) {
        if ($classNames -cnotcontains $item.characterClass) { throw "item characterClass is unknown: $($item.itemId)" }
    }
    if ($null -ne $item.PSObject.Properties['visualSetId']) {
        if ($null -eq $item.PSObject.Properties['equipSlot'] -or
            @('avatarHead', 'avatarOutfit') -cnotcontains $item.equipSlot -or
            [string]::IsNullOrWhiteSpace([string]$item.visualSetId)) {
            throw "item visualSetId belongs only to an avatarHead/avatarOutfit item: $($item.itemId)"
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
    $itemById[[string]$item.itemId] = $item
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
    if ($null -ne $item.PSObject.Properties['battleUse']) {
        $battle = $item.battleUse
        $fields = @('kind', 'skillId', 'damageRatePercent', 'partDamage', 'staggerDamage', 'rangeCm', 'radiusCm', 'durationMs', 'cooldownMs', 'projectileSpeedCmPerSecond', 'projectileArcHeightCm', 'projectileLaunchHeightCm', 'staggerMaximumDivisor')
        Assert-ExactProperties $battle $fields 'battleUse'
        $expectedKinds = @{ BATTLE_DESTRUCTION_BOMB = 'DESTRUCTION'; BATTLE_WHIRLWIND_GRENADE = 'WHIRLWIND'; BATTLE_HOLY_CHARM = 'CLEANSE'; BATTLE_TIME_STOP_POTION = 'TIME_STOP' }
        if (-not $expectedKinds.ContainsKey([string]$item.itemId) -or $battle.kind -cne $expectedKinds[[string]$item.itemId] -or
            $item.category -cne 'use' -or $item.healPercent -ne 0 -or $equipSlotField -cne '-') { throw "battleUse kind/item mismatch: $($item.itemId)" }
        foreach ($field in @('skillId', 'damageRatePercent', 'partDamage', 'staggerDamage', 'rangeCm', 'radiusCm', 'durationMs', 'cooldownMs', 'projectileSpeedCmPerSecond', 'projectileArcHeightCm', 'projectileLaunchHeightCm', 'staggerMaximumDivisor')) {
            Assert-JsonInteger $battle.$field "battleUse $field" 0 1000000
        }
        if ((($battle.kind -eq 'WHIRLWIND') -and ($battle.staggerMaximumDivisor -ne 3 -or $battle.damageRatePercent -ne 0 -or $battle.staggerDamage -ne 0)) -or
            (($battle.kind -ne 'WHIRLWIND') -and $battle.staggerMaximumDivisor -ne 0) -or
            $battle.skillId -eq 0 -or $battle.cooldownMs -lt 1000 -or $battle.cooldownMs -gt 600000 -or
            $battle.rangeCm -gt 5000 -or $battle.radiusCm -gt 5000 -or $battle.durationMs -gt 600000 -or
            (($battle.kind -in @('DESTRUCTION', 'WHIRLWIND')) -and ($battle.rangeCm -eq 0 -or $battle.radiusCm -eq 0)) -or
            ($battle.kind -eq 'TIME_STOP' -and $battle.rangeCm -ne 0) -or
            (($battle.kind -in @('CLEANSE', 'TIME_STOP')) -and $battle.durationMs -eq 0) -or
            $battle.projectileSpeedCmPerSecond -gt 10000 -or $battle.projectileArcHeightCm -gt 1000 -or $battle.projectileLaunchHeightCm -gt 1000 -or
            (($battle.kind -in @('DESTRUCTION', 'WHIRLWIND')) -and $battle.projectileSpeedCmPerSecond -eq 0)) { throw "battleUse values are invalid: $($item.itemId)" }
        $battleRows.Add((@('BATTLEITEM', $item.itemId, $battle.kind, $battle.skillId, $battle.damageRatePercent, $battle.partDamage, $battle.staggerDamage,
            $battle.rangeCm, $battle.radiusCm, $battle.durationMs, $battle.cooldownMs, $battle.projectileSpeedCmPerSecond, $battle.projectileArcHeightCm, $battle.projectileLaunchHeightCm, $battle.staggerMaximumDivisor) -join "`t"))
    }
}

# Currencies are the player's purse (실링, 골드), not bag items. The Server knows exactly these
# two; startingAmount is what a fresh character is given.
# Shop templates: a class-neutral item whose purchase gives the buyer the class variant.
# The template itself is never equippable (no equipSlot, no class); every variant is a
# real equippable item of exactly that class, with the same equipSlot for every class.
$variantRows = [Collections.Generic.List[string]]::new()
foreach ($item in $items) {
    if ($null -eq $item.PSObject.Properties['classVariants']) { continue }
    if ($null -ne $item.PSObject.Properties['equipSlot'] -or $null -ne $item.PSObject.Properties['characterClass'] -or
        $null -ne $item.PSObject.Properties['startingEquippedSlot'] -or $null -ne $item.PSObject.Properties['visualSetId'] -or
        $null -ne $item.PSObject.Properties['battleUse']) {
        throw "item classVariants template must not be equipment itself: $($item.itemId)"
    }
    $variants = $item.classVariants
    if ($variants -isnot [PSCustomObject]) { throw "item classVariants must be an object: $($item.itemId)" }
    $classKeys = @($variants.PSObject.Properties.Name)
    if ($classKeys.Count -eq 0) { throw "item classVariants is empty: $($item.itemId)" }
    $sharedSlot = $null
    foreach ($className in $classKeys) {
        if ($classNames -cnotcontains $className) { throw "item classVariants names an unknown class '$className': $($item.itemId)" }
        $variantId = $variants.$className
        Assert-JsonString $variantId "item classVariants.$className"
        if (-not $itemById.ContainsKey([string]$variantId)) { throw "item classVariants names an unknown item '$variantId': $($item.itemId)" }
        $variant = $itemById[[string]$variantId]
        if ($null -eq $variant.PSObject.Properties['equipSlot'] -or $null -eq $variant.PSObject.Properties['characterClass'] -or
            [string]$variant.characterClass -cne $className -or $null -ne $variant.PSObject.Properties['classVariants']) {
            throw "item classVariants.$className must be an equippable item of that class: $($item.itemId) -> $variantId"
        }
        if ($null -eq $sharedSlot) { $sharedSlot = [string]$variant.equipSlot }
        elseif ($sharedSlot -cne [string]$variant.equipSlot) { throw "item classVariants mix equipSlots: $($item.itemId)" }
        $variantRows.Add((@('ITEMVARIANT', $item.itemId, $className, $variantId) -join "`t"))
    }
}

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
    if ($stock.Count -eq 0 -or $stock.Count -gt 40) { throw "shop stock count is out of range: $($shop.shopId)" }
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
$lines.Add("LOSTARK_ITEM_BOOTSTRAP`t7`t$($itemRows.Count + $battleRows.Count + $variantRows.Count + $currencyRows.Count + $shopRows.Count)")
foreach ($row in $itemRows) { $lines.Add($row) }
foreach ($row in $battleRows) { $lines.Add($row) }
foreach ($row in $variantRows) { $lines.Add($row) }
foreach ($row in $currencyRows) { $lines.Add($row) }
foreach ($row in $shopRows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Items.bootstrap'
Write-PublishTextCatalog -Mode $Mode -Destination $destination -Lines $lines `
    -Sources $publishSources -Context 'Item catalog' `
    -RepairCommand 'powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-ItemCatalog.ps1 -Mode Publish'
