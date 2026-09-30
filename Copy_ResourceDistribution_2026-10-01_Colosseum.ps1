# Colosseum delta resources, preserving paths relative to Client/Bin/Resources.
# No deletes, no runtime-data publishing, no upload, and no replacement of different files.
[CmdletBinding()]
param(
    [string]$Source = '',
    [Parameter(Mandatory = $true)][string]$Destination,
    [switch]$InspectOnly
)
$ErrorActionPreference = 'Stop'
# PSScriptRoot is initialized in the script body on Windows PowerShell 5.1.
if ([string]::IsNullOrWhiteSpace($Source)) { $Source = Join-Path $PSScriptRoot 'Client/Bin/Resources' }
$resourceRoot = [IO.Path]::GetFullPath($Source).TrimEnd([char[]]'\/')
$deliveryRoot = [IO.Path]::GetFullPath($Destination).TrimEnd([char[]]'\/')
$sourcePrefix = $resourceRoot + [IO.Path]::DirectorySeparatorChar
$deliveryPrefix = $deliveryRoot + [IO.Path]::DirectorySeparatorChar
if ($resourceRoot -eq $deliveryRoot -or
    $deliveryRoot.StartsWith($sourcePrefix, [StringComparison]::OrdinalIgnoreCase) -or
    $resourceRoot.StartsWith($deliveryPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Source and destination must be separate non-nested directories.'
}
$assets = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
function Add-Resource([string]$relative) {
    if ([IO.Path]::IsPathRooted($relative) -or $relative.Contains(':') -or
        ($relative -split '[/\\]') -contains '..') { throw "Unsafe resource path: $relative" }
    $path = [IO.Path]::GetFullPath((Join-Path $resourceRoot $relative))
    if (-not $path.StartsWith($sourcePrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Resource escapes source root: $relative"
    }
    if (-not (Test-Path -LiteralPath $path -PathType Leaf) -or
        (Get-Item -LiteralPath $path).Length -eq 0) { throw "Missing/empty resource: $relative" }
    [void]$assets.Add($relative.Replace('\', '/'))
}
function Visit-AssetStrings($value) {
    if ($null -eq $value) { return }
    if ($value -is [string]) {
        # keyframe JSON paths belong to Data, not Resources; they are delivered through Git.
        if ($value -match '^UI/.+\.(png|dds|tga)$' -or
            $value -match '^Character/[^/]+/AnimSets/[^/]+_ColosseumVictoryAnimSet\.wmodel$') {
            Add-Resource $value
        }
        return
    }
    if ($value -is [System.Collections.IDictionary]) {
        foreach ($child in $value.Values) { Visit-AssetStrings $child }
    } elseif ($value -is [System.Collections.IEnumerable]) {
        foreach ($child in $value) { Visit-AssetStrings $child }
    } elseif ($value -is [pscustomobject]) {
        foreach ($property in $value.PSObject.Properties) { Visit-AssetStrings $property.Value }
    }
}
# Preserve the original 103 exact model IDs, plus only the three restored PR #496 models.
$baselineReceipt = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'Data/Maps/Imported/LV_PVP_COLOSSEUM/LV_PVP_COLOSSEUM.build.receipt.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$baselineModelIds = @($baselineReceipt.runtimeMaterialAdmission.PSObject.Properties.Name)
if ($baselineReceipt.areaId -ne 'LV_PVP_COLOSSEUM' -or $baselineModelIds.Count -ne 103) {
    throw 'Recheck the Colosseum baseline model selection: expected 103 model IDs.'
}
$expectedModelIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($id in $baselineModelIds) { [void]$expectedModelIds.Add($id) }
foreach ($id in @(
    'MAP_AA31466B4FAB_BG_LUT_LUCASTLE_CASTLEDECO01_SM_PSY',
    'MAP_CBADB2627CFB_BG_LUT_LUCASTLE_HEROSTATUE05_SM_ARTREE',
    'MAP_CBADB2627CFB_BG_LUT_LUCASTLE_HEROSTATUE05_SM_ARTREE_OVR_72E9DCA8A6BB'
)) { [void]$expectedModelIds.Add($id) }
if ($expectedModelIds.Count -ne 106) { throw 'Recheck the integrated Colosseum model ID selection.' }
$mapRoot = Join-Path $resourceRoot 'Map/LV_PVP_COLOSSEUM'
$models = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($file in Get-ChildItem -LiteralPath $mapRoot -Recurse -File) {
    if ($file.Extension -notin @('.wmodel', '.dds', '.png')) { continue }
    if ($file.Extension -eq '.wmodel') {
        if (-not $expectedModelIds.Contains($file.BaseName) -or -not $models.Add($file.BaseName)) {
            throw "Unexpected or duplicate Colosseum model: $($file.BaseName)"
        }
    }
    Add-Resource $file.FullName.Substring($sourcePrefix.Length)
}
if ($models.Count -ne 106) { throw "Recheck Colosseum map selection: expected 106 models, found $($models.Count)" }
foreach ($document in Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'Data/UI/Colosseum') -Filter '*.json' -File) {
    Visit-AssetStrings (Get-Content -LiteralPath $document.FullName -Raw -Encoding UTF8 | ConvertFrom-Json)
}
$catalog = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'Data/Actors/CharacterCatalog.json') -Raw -Encoding UTF8 | ConvertFrom-Json
foreach ($character in $catalog.characters) {
    foreach ($animation in $character.animationSetModels) {
        if ($animation -match '_ColosseumVictoryAnimSet\.wmodel$') { Add-Resource $animation }
    }
}
Add-Resource 'UI/Loading/Loading_Background_Colosseum.png'
$ordered = @($assets | Sort-Object)
$animationCount = @($ordered | Where-Object { $_ -match '_ColosseumVictoryAnimSet\.wmodel$' }).Count
if ($animationCount -ne 7) { throw "Recheck Colosseum cheer selection: expected 7, found $animationCount" }

# Preflight every selected destination before writing anything. Unrelated files are untouched.
$copies = [Collections.Generic.List[string]]::new()
foreach ($relative in $ordered) {
    $target = [IO.Path]::GetFullPath((Join-Path $deliveryRoot $relative))
    if (-not $target.StartsWith($deliveryPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Resource escapes delivery root: $relative"
    }
    if (Test-Path -LiteralPath $target) {
        if (-not (Test-Path -LiteralPath $target -PathType Leaf) -or
            (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath (Join-Path $resourceRoot $relative) -Algorithm SHA256).Hash) {
            throw "Different existing destination preserved; resolve before copying: $relative"
        }
    } else { $copies.Add($relative) }
}
$bytes = 0L
foreach ($relative in $ordered) { $bytes += (Get-Item -LiteralPath (Join-Path $resourceRoot $relative)).Length }
$groups = @($ordered | Group-Object { ($_ -split '/')[0] } | ForEach-Object {
    [pscustomobject]@{ Folder = $_.Name; Files = $_.Count }
})
if (-not $InspectOnly) {
    foreach ($relative in $copies) {
        $target = Join-Path $deliveryRoot $relative
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target) | Out-Null
        if (Test-Path -LiteralPath $target) { throw "Destination appeared during copy: $relative" }
        Copy-Item -LiteralPath (Join-Path $resourceRoot $relative) -Destination $target
    }
    foreach ($relative in $ordered) {
        if ((Get-FileHash -LiteralPath (Join-Path $resourceRoot $relative) -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath (Join-Path $deliveryRoot $relative) -Algorithm SHA256).Hash) {
            throw "Copy verification failed: $relative"
        }
    }
}
[pscustomobject]@{
    Source = $resourceRoot; Destination = $deliveryRoot; Files = $ordered.Count; Bytes = $bytes
    Folders = $groups; NewFiles = $copies.Count; Verified = -not $InspectOnly
} | ConvertTo-Json -Depth 4
