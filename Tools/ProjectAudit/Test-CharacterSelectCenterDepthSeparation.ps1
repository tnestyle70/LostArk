param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
)

$ErrorActionPreference = 'Stop'

function Fail([string]$Message) {
    throw "[CharacterSelectCenterDepthSeparation] $Message"
}

function Read-Lines([string]$RelativePath) {
    $path = Join-Path $RepoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        Fail "missing file: $RelativePath"
    }
    return [System.IO.File]::ReadAllLines($path)
}

function Require-PlacementHeader([string[]]$Lines, [string]$Label) {
    if ($Lines.Count -ne 804 -or
        $Lines[0] -cne 'LOSTARK_MAP_PLACEMENTS 2 "LV_LOBBY_CLASSSELECT_SL00" 803') {
        Fail "$Label must keep formatVersion 2 and exactly 803 placements"
    }
}

function Require-Record(
    [string[]]$Lines,
    [string]$SourcePlacementId,
    [string]$Expected,
    [string]$Label
) {
    $needle = '"' + $SourcePlacementId + '"'
    $matches = @($Lines | Where-Object { $_.Contains($needle) })
    if ($matches.Count -ne 1) {
        Fail "$Label must contain exactly one $SourcePlacementId record"
    }
    if ($matches[0] -cne $Expected) {
        Fail "$Label record drifted: $SourcePlacementId"
    }
}

$imported = Read-Lines 'Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements'
$authoring = Read-Lines 'Data\Maps\Authoring\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapplacements'
$runtime = Read-Lines 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements'
$sourceCatalog = Read-Lines 'Data\Maps\Imported\LV_LOBBY_CLASSSELECT_SL00\LV_LOBBY_CLASSSELECT_SL00.mapassets'
$runtimeCatalog = Read-Lines 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets'

Require-PlacementHeader $imported 'imported evidence'
Require-PlacementHeader $authoring 'authoring'
Require-PlacementHeader $runtime 'runtime publish'

$raw490 = '11968900681581939590 "LV_LOBBY_CLASSSELECT_SL00:export:490" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.711572 197.537871 0 0.923879533 0 0.382683432 -0.605117977 0.631070793 0.907677352 1'
$raw495 = '10547857777741800178 "LV_LOBBY_CLASSSELECT_SL00:export:495" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.71916 197.537813 0 0.923879533 0 0.382683432 0.672353625 0.383848429 0.907677352 1'
$raw444 = '9728074828520549347 "LV_LOBBY_CLASSSELECT_SL00:export:444" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -771.535938 -142.735918 198.048906 0 -0.382683432 0 0.923879533 -0.632373989 0.33149299 0.850475848 1'
$raw458 = '17033911184007117021 "LV_LOBBY_CLASSSELECT_SL00:export:458" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.507812 -142.735918 197.035352 0 0.923879533 0 0.382683432 -0.632373989 0.33149299 0.850475848 1'
$raw474 = '9567686591551344922 "LV_LOBBY_CLASSSELECT_SL00:export:474" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.519922 -142.735918 198.028262 -0 -0.923879533 -0 0.382683432 -0.632373989 0.33149299 0.850476027 1'
$corrected490 = '11968900681581939590 "LV_LOBBY_CLASSSELECT_SL00:export:490" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.713572 197.537871 0 0.923879533 0 0.382683432 -0.605117977 0.631070793 0.907677352 1'
$corrected495 = '10547857777741800178 "LV_LOBBY_CLASSSELECT_SL00:export:495" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.72116 197.537813 0 0.923879533 0 0.382683432 0.672353625 0.383848429 0.907677352 1'
$corrected444 = '9728074828520549347 "LV_LOBBY_CLASSSELECT_SL00:export:444" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -771.535938 -142.737918 198.048906 0 -0.382683432 0 0.923879533 -0.632373989 0.33149299 0.850475848 1'
$corrected458 = '17033911184007117021 "LV_LOBBY_CLASSSELECT_SL00:export:458" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.507812 -142.739918 197.035352 0 0.923879533 0 0.382683432 -0.632373989 0.33149299 0.850475848 1'
$corrected474 = '9567686591551344922 "LV_LOBBY_CLASSSELECT_SL00:export:474" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.519922 -142.741918 198.028262 -0 -0.923879533 -0 0.382683432 -0.632373989 0.33149299 0.850476027 1'

Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:444' $raw444 'imported evidence'
Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:458' $raw458 'imported evidence'
Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:474' $raw474 'imported evidence'
Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:490' $raw490 'imported evidence'
Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:495' $raw495 'imported evidence'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:444' $corrected444 'authoring'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:458' $corrected458 'authoring'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:474' $corrected474 'authoring'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:490' $corrected490 'authoring'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:495' $corrected495 'authoring'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:444' $corrected444 'runtime publish'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:458' $corrected458 'runtime publish'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:474' $corrected474 'runtime publish'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:490' $corrected490 'runtime publish'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:495' $corrected495 'runtime publish'

$placementDifferences = @()
for ($index = 0; $index -lt $imported.Count; ++$index) {
    if ($imported[$index] -cne $authoring[$index]) {
        $placementDifferences += $index
    }
}

$expectedDifferenceIndexes = @(248, 262, 278, 294, 299)
if ($placementDifferences.Count -ne $expectedDifferenceIndexes.Count) {
    Fail 'authoring may differ from imported evidence only at export 444, 458, 474, 490, and 495'
}
for ($index = 0; $index -lt $expectedDifferenceIndexes.Count; ++$index) {
    if ($placementDifferences[$index] -ne $expectedDifferenceIndexes[$index]) {
        Fail 'authoring contains a non-depth placement change'
    }
}

if ([string]::Join("`n", $authoring) -cne [string]::Join("`n", $runtime)) {
    Fail 'authoring and runtime placement publish must be byte-equivalent by line content'
}

$bridgePlacementIds = @('410', '411', '442', '444', '458', '474', '490', '495')
$baseAssetId = 'MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM'
foreach ($exportId in $bridgePlacementIds) {
    $sourcePlacementId = "LV_LOBBY_CLASSSELECT_SL00:export:$exportId"
    $matches = @($authoring | Where-Object { $_.Contains('"' + $sourcePlacementId + '"') })
    if ($matches.Count -ne 1 -or -not $matches[0].Contains('"' + $baseAssetId + '"')) {
        Fail "center bridge placement must keep the original textured asset: $sourcePlacementId"
    }
}

if ($sourceCatalog.Count -ne 56 -or
    $sourceCatalog[0] -cne 'LOSTARK_MAP_ASSET_CATALOG 4 "LV_LOBBY_CLASSSELECT_SL00" 55') {
    Fail 'source asset catalog must keep exactly 55 assets'
}
if ([string]::Join("`n", $sourceCatalog) -cne [string]::Join("`n", $runtimeCatalog)) {
    Fail 'source and runtime asset catalogs must be byte-equivalent by line content'
}

$mapCatalogPath = Join-Path $RepoRoot 'Data\Maps\MapCatalog.json'
$mapCatalog = Get-Content -LiteralPath $mapCatalogPath -Raw | ConvertFrom-Json
$area = @($mapCatalog.areas | Where-Object { $_.id -eq 'LV_LOBBY_CLASSSELECT_SL00' })
if ($area.Count -ne 1 -or $area[0].placementCount -ne 803 -or $area[0].assetCount -ne 55) {
    Fail 'MapCatalog must keep Character Select at 803 placements and 55 assets'
}

Write-Host '[CharacterSelectCenterDepthSeparation] PASS: only five crossing placements have deterministic 2mm depth steps; all original textured assets are preserved.'
