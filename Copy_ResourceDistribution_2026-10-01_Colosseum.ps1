# Colosseum delta resources, preserving paths relative to Client/Bin/Resources.
# No deletes, no runtime-data publishing, no upload, and no replacement of different files.
[CmdletBinding()]
param(
    [string]$Source = (Join-Path $PSScriptRoot 'Client/Bin/Resources'),
    [Parameter(Mandatory = $true)][string]$Destination,
    [switch]$InspectOnly
)
$ErrorActionPreference = 'Stop'
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
$mapRoot = Join-Path $resourceRoot 'Map/LV_PVP_COLOSSEUM'
$models = 0
foreach ($file in Get-ChildItem -LiteralPath $mapRoot -Recurse -File) {
    if ($file.Extension -notin @('.wmodel', '.dds', '.png')) { continue }
    if ($file.Extension -eq '.wmodel') { ++$models }
    Add-Resource $file.FullName.Substring($sourcePrefix.Length)
}
if ($models -ne 103) { throw "Recheck Colosseum map selection: expected 103 models, found $models" }
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
