# Shared by the authoring repair and the destruction publisher. A wall owns
# only base-walkable cells intersecting its own collision volume, never a
# nearest cell or another deck. Floor-collapse regions are separate.
function Get-ValtanNavigationBodySpan {
    $sourceRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
    $profiles = [IO.File]::ReadAllText(
        (Join-Path $sourceRoot 'Data/Balance/BossProfiles.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json
    $boss = @($profiles.bosses | Where-Object { $_.archetypeId -ceq 'BOSS_VALTAN' })
    if ($profiles.schema -cne 'lostark.boss-profiles' -or $boss.Count -ne 1) {
        throw 'Wall navigation needs the canonical Valtan body profile.'
    }
    $radius = [double]$boss[0].collisionRadius
    if ([double]::IsNaN($radius) -or [double]::IsInfinity($radius) -or $radius -le 0) {
        throw 'Wall navigation body radius must be finite and positive.'
    }
    # Consume the same fixed player span as ServerCollisionSystem instead of
    # introducing an independently tuned body height in navigation authoring.
    $contract = [IO.File]::ReadAllText(
        (Join-Path $sourceRoot 'Shared/Public/Gameplay/WorldCollisionContract.h'))
    $values = @{}
    foreach ($name in @('PLAYER_HALF_EXTENT_Y', 'PLAYER_CENTER_OFFSET_Y')) {
        $pattern = 'inline\s+constexpr\s+float\s+' + $name + '\s*=\s*([0-9]+(?:\.[0-9]+)?)f\s*;'
        $matches = [regex]::Matches($contract, $pattern)
        if ($matches.Count -ne 1) { throw "Cannot resolve the authoritative player body span: $name" }
        $values[$name] = [double]::Parse($matches[0].Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    }
    return [pscustomobject]@{
        MinimumY=[Math]::Min(0.0, $values.PLAYER_CENTER_OFFSET_Y - $values.PLAYER_HALF_EXTENT_Y)
        MaximumY=[Math]::Max(2.0 * $radius, $values.PLAYER_CENTER_OFFSET_Y + $values.PLAYER_HALF_EXTENT_Y)
    }
}

function Read-ValtanWallNavigationGrid([string]$Path) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 20) { throw 'Wall navigation grid header is truncated.' }
    $width = [BitConverter]::ToUInt32($bytes, 0)
    $height = [BitConverter]::ToUInt32($bytes, 4)
    $cellSize = [BitConverter]::ToSingle($bytes, 8)
    $originX = [BitConverter]::ToSingle($bytes, 12)
    $originZ = [BitConverter]::ToSingle($bytes, 16)
    $count = [long]$width * [long]$height
    if ($width -eq 0 -or $height -eq 0 -or $count -gt 1000000 -or
        $bytes.LongLength -ne 20 + $count * 5 -or
        [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or $cellSize -le 0 -or
        [single]::IsNaN($originX) -or [single]::IsInfinity($originX) -or
        [single]::IsNaN($originZ) -or [single]::IsInfinity($originZ)) {
        throw 'Wall navigation grid dimensions or payload are invalid.'
    }
    $bodySpan = Get-ValtanNavigationBodySpan
    return [pscustomobject]@{
        Width=$width; Height=$height; CellSize=[double]$cellSize
        OriginX=[double]$originX; OriginZ=[double]$originZ; Bytes=$bytes
        BodyMinimumY=$bodySpan.MinimumY; BodyMaximumY=$bodySpan.MaximumY
    }
}

function Get-ValtanWallNavigationCellKeys([object]$Collision, [object]$Grid) {
    if ($Collision.kind -cne 'collisionBox' -or -not $Collision.enabled -or
        @($Collision.position).Count -ne 3 -or @($Collision.halfExtents).Count -ne 3) {
        throw 'Wall navigation requires one enabled authored collisionBox.'
    }
    foreach ($value in (@($Collision.position) + @($Collision.halfExtents) + @($Collision.yawDegrees))) {
        if ([double]::IsNaN([double]$value) -or [double]::IsInfinity([double]$value)) {
            throw 'Wall navigation collision transform is non-finite.'
        }
    }
    foreach ($extent in @($Collision.halfExtents)) {
        if ([double]$extent -le 0) { throw 'Wall navigation collision extents must be positive.' }
    }
    $x = [double]$Collision.position[0]
    $z = [double]$Collision.position[2]
    $hx = [double]$Collision.halfExtents[0]
    $hz = [double]$Collision.halfExtents[2]
    $wallBottom = [double]$Collision.position[1] - [double]$Collision.halfExtents[1]
    $wallTop = [double]$Collision.position[1] + [double]$Collision.halfExtents[1]
    $angle = [double]$Collision.yawDegrees * [Math]::PI / 180.0
    $cos = [Math]::Cos($angle)
    $sin = [Math]::Sin($angle)
    $absCos = [Math]::Abs($cos)
    $absSin = [Math]::Abs($sin)
    $halfCell = $Grid.CellSize * 0.5
    $extentX = $absCos * $hx + $absSin * $hz
    $extentZ = $absSin * $hx + $absCos * $hz
    $minX = [Math]::Max(0, [int][Math]::Floor(($x - $extentX - $Grid.OriginX) / $Grid.CellSize))
    $maxX = [Math]::Min($Grid.Width - 1, [int][Math]::Floor(($x + $extentX - $Grid.OriginX) / $Grid.CellSize))
    $minZ = [Math]::Max(0, [int][Math]::Floor(($z - $extentZ - $Grid.OriginZ) / $Grid.CellSize))
    $maxZ = [Math]::Min($Grid.Height - 1, [int][Math]::Floor(($z + $extentZ - $Grid.OriginZ) / $Grid.CellSize))
    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    for ($cellZ = $minZ; $cellZ -le $maxZ; ++$cellZ) {
        for ($cellX = $minX; $cellX -le $maxX; ++$cellX) {
            $index = $cellZ * $Grid.Width + $cellX
            if ($Grid.Bytes[20 + $index] -ne 1) { continue }
            $groundY = [BitConverter]::ToSingle(
                $Grid.Bytes, 20 + $Grid.Width * $Grid.Height + $index * 4)
            if ([single]::IsNaN($groundY) -or [single]::IsInfinity($groundY)) {
                throw 'Wall navigation cell height is non-finite.'
            }
            if ($groundY + $Grid.BodyMinimumY -gt $wallTop + 0.000001 -or
                $groundY + $Grid.BodyMaximumY -lt $wallBottom - 0.000001) { continue }
            $dx = $Grid.OriginX + ($cellX + 0.5) * $Grid.CellSize - $x
            $dz = $Grid.OriginZ + ($cellZ + 0.5) * $Grid.CellSize - $z
            # SAT on the two world axes and two wall axes. Use the same yaw
            # convention as ServerCollisionSystem::To_BoxLocal (row vectors).
            $localX = $dx * $cos - $dz * $sin
            $localZ = $dx * $sin + $dz * $cos
            $cellProjection = $halfCell * ($absCos + $absSin)
            if ([Math]::Abs($dx) -gt $extentX + $halfCell + 0.000001 -or
                [Math]::Abs($dz) -gt $extentZ + $halfCell + 0.000001 -or
                [Math]::Abs($localX) -gt $hx + $cellProjection + 0.000001 -or
                [Math]::Abs($localZ) -gt $hz + $cellProjection + 0.000001) { continue }
            [void]$keys.Add("$cellX,$cellZ")
        }
    }
    return ,$keys
}
