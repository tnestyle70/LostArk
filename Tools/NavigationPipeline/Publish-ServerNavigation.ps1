[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'ContractTest')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Navigation'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

function New-UniformNavigationGrid {
    param([string]$RelativeAuthoringPath)

    $path = Join-Path $repoRoot $RelativeAuthoringPath
    $source = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
    $properties = @($source.PSObject.Properties.Name | Sort-Object)
    $expected = @(
        'areaId','cellSize','defaultHeight','formatVersion','height',
        'originX','originZ','schema','width') | Sort-Object
    if (($properties -join "`n") -ne ($expected -join "`n") -or
        $source.schema -ne 'lostark.navigation-grid-authoring' -or
        $source.formatVersion -ne 1 -or
        $source.areaId -notmatch '^[A-Za-z0-9_.-]{1,128}$' -or
        [uint32]$source.width -eq 0 -or [uint32]$source.height -eq 0 -or
        ([uint64]$source.width * [uint64]$source.height) -gt 1000000 -or
        [double]$source.cellSize -le 0.0 -or
        [double]::IsNaN([double]$source.cellSize) -or
        [double]::IsInfinity([double]$source.cellSize) -or
        [double]::IsNaN([double]$source.originX) -or
        [double]::IsNaN([double]$source.originZ) -or
        [double]::IsNaN([double]$source.defaultHeight)) {
        throw "Training navigation authoring contract is invalid: $RelativeAuthoringPath"
    }

    $stream = [IO.MemoryStream]::new()
    $writer = [IO.BinaryWriter]::new($stream)
    try {
        $writer.Write([uint32]$source.width)
        $writer.Write([uint32]$source.height)
        $writer.Write([single]$source.cellSize)
        $writer.Write([single]$source.originX)
        $writer.Write([single]$source.originZ)
        $cellCount = [uint32]$source.width * [uint32]$source.height
        for ($index = 0; $index -lt $cellCount; ++$index) {
            $writer.Write([byte]1)
        }
        for ($index = 0; $index -lt $cellCount; ++$index) {
            $writer.Write([single]$source.defaultHeight)
        }
        $writer.Flush()
        return [pscustomobject]@{
            AreaId = [string]$source.areaId
            Bytes = $stream.ToArray()
            WorldPath = "Data/Worlds/$($source.areaId)/Gameplay.world.json"
        }
    }
    finally {
        $writer.Dispose()
        $stream.Dispose()
    }
}

function Split-NavigationTokens {
    param([string]$Line)
    return @([regex]::Matches($Line, '"[^"]*"|\S+') | ForEach-Object {
        $_.Value.Trim('"')
    })
}

function Convert-NavigationAuthoringGrid {
    param(
        [string]$RelativeSourcePath,
        [string]$RelativePaintPath,
        [single]$MaximumStepHeight = 0.0,
        [switch]$RequireSingleComponent,
        [string]$AuthoringRoot = $repoRoot
    )

    $sourcePath = [IO.Path]::GetFullPath((Join-Path $AuthoringRoot $RelativeSourcePath))
    if (-not [IO.File]::Exists($sourcePath)) {
        throw "Navigation authoring source is missing: $sourcePath"
    }
    $lines = @([IO.File]::ReadAllLines($sourcePath, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 2) {
        throw "Navigation authoring source is empty: $sourcePath"
    }

    $header = @(Split-NavigationTokens $lines[0])
    if ($header.Count -lt 9 -or $header[0] -ne 'LOSTARK_NAVGRID_SOURCE' -or
        $header[1] -notin @('1', '2')) {
        throw "Navigation authoring header is invalid: $sourcePath"
    }
    $version = [uint32]$header[1]
    # Version 2 adds bake position/size/yaw/maxSlope/isReady to the v1 header.
    # CNavGridBaker::Save_Source writes ten of those fields, not eleven: the bake
    # cell size is not serialised because CNavGridPaintDocument restores it from
    # the grid cell size on load.
    $expectedHeaderCount = if ($version -eq 1) { 9 } else { 18 }
    if ($header.Count -ne $expectedHeaderCount) {
        throw "Navigation authoring header field count is invalid: $sourcePath"
    }

    $culture = [Globalization.CultureInfo]::InvariantCulture
    $areaId = [string]$header[2]
    $width = [uint32]::Parse($header[3], $culture)
    $height = [uint32]::Parse($header[4], $culture)
    $cellSize = [single]::Parse($header[5], $culture)
    $originX = [single]::Parse($header[6], $culture)
    $originZ = [single]::Parse($header[7], $culture)
    $declaredCellCount = [uint64]::Parse($header[-1], $culture)
    $cellCount = [uint64]$width * [uint64]$height
    if ($areaId -notmatch '^[A-Za-z0-9_.-]{1,128}$' -or
        $width -eq 0 -or $height -eq 0 -or $cellCount -gt 1000000 -or
        $declaredCellCount -ne $cellCount -or
        $lines.Count -ne 1 + $cellCount -or
        [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or
        $cellSize -le 0) {
        throw "Navigation authoring dimensions are invalid: $sourcePath"
    }

    $resolved = [byte[]]::new([int]$cellCount)
    $baseWalkable = [byte[]]::new([int]$cellCount)
    $heights = [single[]]::new([int]$cellCount)
    $seen = [byte[]]::new([int]$cellCount)
    for ($row = 0; $row -lt $cellCount; ++$row) {
        $tokens = @(Split-NavigationTokens $lines[$row + 1])
        $expectedRowCount = if ($version -eq 1) { 4 } else { 5 }
        if ($tokens.Count -ne $expectedRowCount) {
            throw "Navigation authoring row is invalid: $sourcePath row=$row"
        }
        $cellX = [int32]::Parse($tokens[0], $culture)
        $cellZ = [int32]::Parse($tokens[1], $culture)
        $surface = [uint32]::Parse($tokens[2], $culture)
        $walkable = if ($version -eq 1) { $surface } else {
            [uint32]::Parse($tokens[3], $culture)
        }
        $heightToken = if ($version -eq 1) { $tokens[3] } else { $tokens[4] }
        $cellHeight = [single]::Parse($heightToken, $culture)
        if ($cellX -lt 0 -or $cellZ -lt 0 -or
            $cellX -ge $width -or $cellZ -ge $height -or
            $surface -gt 1 -or $walkable -gt $surface -or
            [single]::IsNaN($cellHeight) -or [single]::IsInfinity($cellHeight)) {
            throw "Navigation authoring cell is invalid: $sourcePath row=$row"
        }
        $index = $cellZ * $width + $cellX
        if ($seen[$index] -ne 0) {
            throw "Navigation authoring has duplicate cells: $sourcePath"
        }
        $seen[$index] = 1
        $resolved[$index] = [byte]$surface
        $baseWalkable[$index] = [byte]$walkable
        $heights[$index] = if ($surface -eq 0) { [single]0 } else { $cellHeight }
    }

    # 0 = inherit baked state, 1 = force blocked, 2 = force walkable.
    $overrides = [byte[]]::new([int]$cellCount)
    $paintPath = [IO.Path]::GetFullPath((Join-Path $AuthoringRoot $RelativePaintPath))
    if ([IO.File]::Exists($paintPath)) {
        $paintLines = @([IO.File]::ReadAllLines($paintPath, [Text.Encoding]::UTF8))
        if ($paintLines.Count -lt 1) { throw "Navigation paint is empty: $paintPath" }
        $paintHeader = @(Split-NavigationTokens $paintLines[0])
        if ($paintHeader.Count -ne 9 -or
            $paintHeader[0] -ne 'LOSTARK_NAVGRID_PAINT' -or
            $paintHeader[1] -notin @('1', '2') -or
            $paintHeader[2] -ne $areaId -or
            [uint32]$paintHeader[3] -ne $width -or
            [uint32]$paintHeader[4] -ne $height -or
            [single]$paintHeader[5] -ne $cellSize -or
            [single]$paintHeader[6] -ne $originX -or
            [single]$paintHeader[7] -ne $originZ) {
            throw "Navigation paint header does not match source: $paintPath"
        }
        $paintVersion = [uint32]::Parse($paintHeader[1], $culture)
        $overrideCount = [uint32]::Parse($paintHeader[8], $culture)
        if ($overrideCount -gt $cellCount -or
            $paintLines.Count -ne 1 + $overrideCount) {
            throw "Navigation paint count is invalid: $paintPath"
        }
        for ($row = 0; $row -lt $overrideCount; ++$row) {
            $tokens = @(Split-NavigationTokens $paintLines[$row + 1])
            $expectedPaintRowCount = if ($paintVersion -eq 1) { 2 } else { 3 }
            if ($tokens.Count -ne $expectedPaintRowCount) {
                throw "Navigation paint row is invalid: $paintPath"
            }
            $cellX = [int32]::Parse($tokens[0], $culture)
            $cellZ = [int32]::Parse($tokens[1], $culture)
            if ($cellX -lt 0 -or $cellZ -lt 0 -or
                $cellX -ge $width -or $cellZ -ge $height) {
                throw "Navigation paint cell is outside the grid: $paintPath"
            }
            $index = $cellZ * $width + $cellX
            if ($overrides[$index] -ne 0) {
                throw "Navigation paint has duplicate cells: $paintPath"
            }
            if ($resolved[$index] -eq 0) {
                throw "Navigation paint targets an unresolved cell: $paintPath"
            }

            if ($paintVersion -eq 1 -or $tokens[2] -eq 'BLOCKED') {
                $overrides[$index] = 1
            }
            elseif ($tokens[2] -eq 'WALKABLE') {
                $overrides[$index] = 2
            }
            else {
                throw "Navigation paint override is invalid: $paintPath"
            }
        }
    }

    $stream = [IO.MemoryStream]::new()
    $writer = [IO.BinaryWriter]::new($stream)
    try {
        $writer.Write($width)
        $writer.Write($height)
        $writer.Write($cellSize)
        $writer.Write($originX)
        $writer.Write($originZ)
        for ($index = 0; $index -lt $cellCount; ++$index) {
            $isWalkable = $false
            if ($resolved[$index] -ne 0) {
                if ($overrides[$index] -eq 2) {
                    $isWalkable = $true
                }
                elseif ($overrides[$index] -eq 0) {
                    $isWalkable = $baseWalkable[$index] -ne 0
                }
            }
            $walkableByte = if ($isWalkable) { [byte]1 } else { [byte]0 }
            $writer.Write($walkableByte)
        }
        for ($index = 0; $index -lt $cellCount; ++$index) {
            $writer.Write($heights[$index])
        }
        $writer.Flush()
        return [pscustomobject]@{
            AreaId = $areaId
            Bytes = $stream.ToArray()
            WorldPath = "Data/Worlds/$areaId/Gameplay.world.json"
            MaximumStepHeight = $MaximumStepHeight
            RequireSingleComponent = [bool]$RequireSingleComponent
        }
    }
    finally {
        $writer.Dispose()
        $stream.Dispose()
    }
}

function Assert-NavigationGrid {
    param([object]$Grid)

    $bytes = [byte[]]$Grid.Bytes
    if ($bytes.Length -lt 20) { throw "Navigation grid header is truncated: $($Grid.AreaId)" }
    $width = [BitConverter]::ToUInt32($bytes, 0)
    $height = [BitConverter]::ToUInt32($bytes, 4)
    $cellSize = [BitConverter]::ToSingle($bytes, 8)
    $originX = [BitConverter]::ToSingle($bytes, 12)
    $originZ = [BitConverter]::ToSingle($bytes, 16)
    $cellCount = [uint64]$width * [uint64]$height
    $expectedSize = 20L + [int64]$cellCount + 4L * [int64]$cellCount
    if ($width -eq 0 -or $height -eq 0 -or $cellCount -gt 1000000 -or
        [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or
        $cellSize -le 0 -or $bytes.LongLength -ne $expectedSize) {
        throw "Navigation grid contract is invalid: $($Grid.AreaId)"
    }

    $walkableOffset = 20
    $heightOffset = $walkableOffset + [int]$cellCount
    $walkableCount = 0
    $maximumObservedStep = 0.0
    for ($cellZ = 0; $cellZ -lt $height; ++$cellZ) {
        for ($cellX = 0; $cellX -lt $width; ++$cellX) {
            $index = $cellZ * $width + $cellX
            if ($bytes[$walkableOffset + $index] -ne 1) { continue }
            ++$walkableCount
            $cellHeight = [BitConverter]::ToSingle(
                $bytes, $heightOffset + 4 * $index)
            foreach ($neighborIndex in @(
                $(if ($cellX + 1 -lt $width) { $index + 1 } else { -1 }),
                $(if ($cellZ + 1 -lt $height) { $index + $width } else { -1 })
            )) {
                if ($neighborIndex -lt 0 -or
                    $bytes[$walkableOffset + $neighborIndex] -ne 1) {
                    continue
                }
                $neighborHeight = [BitConverter]::ToSingle(
                    $bytes, $heightOffset + 4 * $neighborIndex)
                $maximumObservedStep = [Math]::Max(
                    $maximumObservedStep,
                    [Math]::Abs([double]$cellHeight - $neighborHeight))
            }
        }
    }
    if ($Grid.MaximumStepHeight -gt 0.0 -and
        $maximumObservedStep -gt $Grid.MaximumStepHeight + 0.000001) {
        throw "Navigation grid has an unsafe adjacent step: $($Grid.AreaId) max=$maximumObservedStep limit=$($Grid.MaximumStepHeight)"
    }

    if ($Grid.RequireSingleComponent) {
        $firstWalkable = -1
        for ($index = 0; $index -lt $cellCount; ++$index) {
            if ($bytes[$walkableOffset + $index] -eq 1) {
                $firstWalkable = $index
                break
            }
        }
        if ($firstWalkable -lt 0) {
            throw "Navigation grid has no walkable component: $($Grid.AreaId)"
        }
        $visited = [byte[]]::new([int]$cellCount)
        $queue = [Collections.Generic.Queue[int]]::new()
        $queue.Enqueue($firstWalkable)
        $visited[$firstWalkable] = 1
        $visitedCount = 0
        while ($queue.Count -ne 0) {
            $current = $queue.Dequeue()
            ++$visitedCount
            $cellX = $current % $width
            $cellZ = [Math]::Floor($current / $width)
            foreach ($neighborIndex in @(
                $(if ($cellX + 1 -lt $width) { $current + 1 } else { -1 }),
                $(if ($cellX -gt 0) { $current - 1 } else { -1 }),
                $(if ($cellZ + 1 -lt $height) { $current + $width } else { -1 }),
                $(if ($cellZ -gt 0) { $current - $width } else { -1 })
            )) {
                if ($neighborIndex -lt 0 -or
                    $bytes[$walkableOffset + $neighborIndex] -ne 1 -or
                    $visited[$neighborIndex] -ne 0) {
                    continue
                }
                $visited[$neighborIndex] = 1
                $queue.Enqueue($neighborIndex)
            }
        }
        if ($visitedCount -ne $walkableCount) {
            throw "Navigation grid has disconnected walkable islands: $($Grid.AreaId) connected=$visitedCount total=$walkableCount"
        }
    }

    $world = Get-Content -LiteralPath (Join-Path $repoRoot $Grid.WorldPath) -Raw -Encoding UTF8 | ConvertFrom-Json
    foreach ($placement in @($world.placements | Where-Object {
        $_.kind -eq 'boss' -or ($_.enabled -and $_.kind -eq 'playerSpawn') })) {
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
    return "${width}x${height}, cellSize=$cellSize, walkable=$walkableCount, maxStep=$maximumObservedStep"
}

function Invoke-NavigationPaintContractTest {
    $fixtureRoot = Join-Path ([IO.Path]::GetTempPath()) (
        'lostark-navpaint-contract-' + [Guid]::NewGuid().ToString('N'))
    $navigationRoot = Join-Path $fixtureRoot 'Data/Navigation'
    [IO.Directory]::CreateDirectory($navigationRoot) | Out-Null
    $sourcePath = Join-Path $navigationRoot 'TEST_NAV_PAINT.navsource'
    $paintPath = Join-Path $navigationRoot 'TEST_NAV_PAINT.navpaint'
    $sourceText = @(
        'LOSTARK_NAVGRID_SOURCE 2 "TEST_NAV_PAINT" 2 1 1 0 0 0 0 0 2 2 2 0 50 1 2',
        '0 0 1 1 0',
        '1 0 1 0 0'
    ) -join "`n"
    $validPaint = @(
        'LOSTARK_NAVGRID_PAINT 2 "TEST_NAV_PAINT" 2 1 1 0 0 2',
        '0 0 BLOCKED',
        '1 0 WALKABLE'
    ) -join "`n"

    try {
        [IO.File]::WriteAllText($sourcePath, $sourceText, [Text.Encoding]::UTF8)
        [IO.File]::WriteAllText($paintPath, $validPaint, [Text.Encoding]::UTF8)
        $grid = Convert-NavigationAuthoringGrid `
            -RelativeSourcePath 'Data/Navigation/TEST_NAV_PAINT.navsource' `
            -RelativePaintPath 'Data/Navigation/TEST_NAV_PAINT.navpaint' `
            -AuthoringRoot $fixtureRoot
        if ($grid.Bytes[20] -ne 0 -or $grid.Bytes[21] -ne 1) {
            throw 'Navigation paint v2 did not override baked walkability'
        }

        $invalidCases = @(
            @(
                'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                '0 0 WALKABLE'),
            @(
                'LOSTARK_NAVGRID_PAINT 2 "TEST_NAV_PAINT" 2 1 1 0 0 2',
                '0 0 WALKABLE',
                '0 0 BLOCKED'),
            @(
                'LOSTARK_NAVGRID_PAINT 2 "WRONG_AREA" 2 1 1 0 0 1',
                '0 0 WALKABLE')
        )
        foreach ($invalidCase in $invalidCases) {
            [IO.File]::WriteAllText(
                $paintPath,
                ($invalidCase -join "`n"),
                [Text.Encoding]::UTF8)
            $rejected = $false
            try {
                Convert-NavigationAuthoringGrid `
                    -RelativeSourcePath 'Data/Navigation/TEST_NAV_PAINT.navsource' `
                    -RelativePaintPath 'Data/Navigation/TEST_NAV_PAINT.navpaint' `
                    -AuthoringRoot $fixtureRoot | Out-Null
            }
            catch {
                $rejected = $true
            }
            if (-not $rejected) {
                throw 'Navigation paint invalid contract was accepted'
            }
        }
    }
    finally {
        if ([IO.Directory]::Exists($fixtureRoot)) {
            [IO.Directory]::Delete($fixtureRoot, $true)
        }
    }
}

if ($Mode -eq 'ContractTest') {
    Invoke-NavigationPaintContractTest
    Write-Host 'Server navigation ContractTest succeeded: navpaint v2 overrides and rejection cases.'
    return
}

$grids = @(
    (Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_LUT_HEARTRB_ED.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_LUT_HEARTRB_ED.navpaint'),
    (New-UniformNavigationGrid `
		-RelativeAuthoringPath 'Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json'),
    (Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint' `
        -MaximumStepHeight 0.6 `
        -RequireSingleComponent)
)

$validated = foreach ($grid in $grids) {
    [pscustomobject]@{
        Grid = $grid
        Detail = Assert-NavigationGrid $grid
    }
}

if ($Mode -eq 'Publish') {
    $root = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    [IO.Directory]::CreateDirectory($root) | Out-Null
    foreach ($entry in $validated) {
        $destinations = @(
            (Join-Path $root "$($entry.Grid.AreaId).navgrid")
        )
        $clientRoot = Join-Path $repoRoot 'Client/Bin/DataFiles/Navigation'
        [IO.Directory]::CreateDirectory($clientRoot) | Out-Null
        $destinations += Join-Path $clientRoot "$($entry.Grid.AreaId).navgrid"
        foreach ($destination in $destinations) {
            $destinationRoot = [IO.Path]::GetDirectoryName($destination)
            $token = [Guid]::NewGuid().ToString('N')
            $staged = Join-Path $destinationRoot ".$($entry.Grid.AreaId).staging.$token"
            $rollback = Join-Path $destinationRoot ".$($entry.Grid.AreaId).rollback.$token"
            try {
                [IO.File]::WriteAllBytes($staged, [byte[]]$entry.Grid.Bytes)
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
    }
}

foreach ($entry in $validated) {
    Write-Host "Server navigation $Mode succeeded: $($entry.Grid.AreaId) $($entry.Detail)."
}
