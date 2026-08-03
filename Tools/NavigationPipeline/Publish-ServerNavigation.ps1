[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Navigation'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$source = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Client/Bin/DataFiles/Navigation/ValtanArena.navgrid'))
if (-not [IO.File]::Exists($source)) { throw "Server navigation source is missing: $source" }

$bytes = [IO.File]::ReadAllBytes($source)
if ($bytes.Length -lt 20) { throw 'Navigation grid header is truncated.' }
$width = [BitConverter]::ToUInt32($bytes, 0)
$height = [BitConverter]::ToUInt32($bytes, 4)
$cellSize = [BitConverter]::ToSingle($bytes, 8)
$cellCount = [uint64]$width * [uint64]$height
$expectedSize = 20L + [int64]$cellCount + 4L * [int64]$cellCount
if ($width -eq 0 -or $height -eq 0 -or $cellCount -gt 1000000 -or
    [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or $cellSize -le 0 -or
    $bytes.LongLength -ne $expectedSize) {
    throw "Navigation grid contract is invalid. width=$width height=$height cellSize=$cellSize bytes=$($bytes.LongLength) expected=$expectedSize"
}

$originX = [BitConverter]::ToSingle($bytes, 12)
$originZ = [BitConverter]::ToSingle($bytes, 16)
$worldPath = Join-Path $repoRoot 'Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json'
$world = Get-Content -LiteralPath $worldPath -Raw -Encoding UTF8 | ConvertFrom-Json
foreach ($placement in @($world.placements | Where-Object { $_.enabled -and $_.kind -in @('playerSpawn','boss') })) {
	$cellX = [int][Math]::Floor(([double]$placement.position[0] - $originX) / $cellSize)
	$cellZ = [int][Math]::Floor(([double]$placement.position[2] - $originZ) / $cellSize)
	if ($cellX -lt 0 -or $cellZ -lt 0 -or $cellX -ge $width -or $cellZ -ge $height) {
		throw "Gameplay placement is outside server navigation: $($placement.placementId)"
	}
	$index = $cellZ * $width + $cellX
	if ($bytes[20 + $index] -ne 1) {
		throw "Gameplay placement is not on a walkable server cell: $($placement.placementId)"
	}
	$heightOffset = 20 + [int]$cellCount + 4 * $index
	$cellHeight = [BitConverter]::ToSingle($bytes, $heightOffset)
	if ([Math]::Abs([double]$placement.position[1] - $cellHeight) -gt 0.25) {
		throw "Gameplay placement height differs from server navigation: $($placement.placementId)"
	}
}

if ($Mode -eq 'Publish') {
    $root = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    [IO.Directory]::CreateDirectory($root) | Out-Null
    $destination = Join-Path $root 'LV_LUT_HEARTRB_ED.navgrid'
    $staged = Join-Path $root ('.ValtanArena.staging.' + [Guid]::NewGuid().ToString('N'))
    $rollback = Join-Path $root ('.ValtanArena.rollback.' + [Guid]::NewGuid().ToString('N'))
    try {
        [IO.File]::WriteAllBytes($staged, $bytes)
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

Write-Host "Server navigation $Mode succeeded: ${width}x${height}, cellSize=$cellSize."
