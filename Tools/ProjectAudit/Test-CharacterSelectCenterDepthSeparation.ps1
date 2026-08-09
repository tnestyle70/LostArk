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

function Require-Header([string[]]$Lines, [string]$Label) {
    if ($Lines.Count -ne 804 -or
        $Lines[0] -ne 'LOSTARK_MAP_PLACEMENTS 2 "LV_LOBBY_CLASSSELECT_SL00" 803') {
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

Require-Header $imported 'imported evidence'
Require-Header $authoring 'authoring'
Require-Header $runtime 'runtime publish'

$raw490 = '11968900681581939590 "LV_LOBBY_CLASSSELECT_SL00:export:490" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.711572 197.537871 0 0.923879533 0 0.382683432 -0.605117977 0.631070793 0.907677352 1'
$raw495 = '10547857777741800178 "LV_LOBBY_CLASSSELECT_SL00:export:495" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.71916 197.537813 0 0.923879533 0 0.382683432 0.672353625 0.383848429 0.907677352 1'
$corrected490 = '11968900681581939590 "LV_LOBBY_CLASSSELECT_SL00:export:490" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.713572 197.537871 0 0.923879533 0 0.382683432 -0.605117977 0.631070793 0.907677352 1'
$corrected495 = '10547857777741800178 "LV_LOBBY_CLASSSELECT_SL00:export:495" "LV_LOBBY_CLASSSELECT_SL00" "actor" "MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM" -772.017422 -142.72116 197.537813 0 0.923879533 0 0.382683432 0.672353625 0.383848429 0.907677352 1'

Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:490' $raw490 'imported evidence'
Require-Record $imported 'LV_LOBBY_CLASSSELECT_SL00:export:495' $raw495 'imported evidence'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:490' $corrected490 'authoring'
Require-Record $authoring 'LV_LOBBY_CLASSSELECT_SL00:export:495' $corrected495 'authoring'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:490' $corrected490 'runtime publish'
Require-Record $runtime 'LV_LOBBY_CLASSSELECT_SL00:export:495' $corrected495 'runtime publish'

foreach ($survivor in @(
    'LV_LOBBY_CLASSSELECT_SL00:export:410',
    'LV_LOBBY_CLASSSELECT_SL00:export:411'
)) {
    $authoringCount = @($authoring | Where-Object { $_.Contains('"' + $survivor + '"') }).Count
    $runtimeCount = @($runtime | Where-Object { $_.Contains('"' + $survivor + '"') }).Count
    if ($authoringCount -ne 1 -or $runtimeCount -ne 1) {
        Fail "surviving source layer must remain unique: $survivor"
    }
}

if ([string]::Join("`n", $authoring) -cne [string]::Join("`n", $runtime)) {
    Fail 'authoring and runtime publish must be byte-equivalent by line content'
}

Write-Host '[CharacterSelectCenterDepthSeparation] PASS: imported evidence preserved; two crossing layers have deterministic 2mm separation.'
