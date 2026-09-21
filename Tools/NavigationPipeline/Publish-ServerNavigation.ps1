[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish', 'ContractTest')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Navigation',
    # The Client copy is written by the same transaction. Pointing both roots
    # at a scratch folder stages a publish without touching live outputs.
    [string]$ClientOutputRoot = 'Client/Bin/DataFiles/Navigation',
    [ValidateSet('', 'LV_LUT_HEARTRB_ED', 'LV_LUT_MIDNIGHTC_ED',
        'LV_DEV_TRAINING_GROUND', 'LV_LOBBY_CLASSSELECT_SL00', 'LV_BER_BERNCASTLE',
        'LV_OCN_EVENTIS_MHP')]
    [string]$AreaId = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

# Keep the authoring/paint/placement contract in this publisher. Only the dense
# cell loops run as compiled code: invoking a PowerShell pipeline for every cell
# dominated publication of the large base and detail grids.
if (-not ('LostArk.NavigationPublish.CellBuffer' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Globalization;
using System.IO;
using System.Text.RegularExpressions;

namespace LostArk.NavigationPublish
{
    public sealed class CellBuffer
    {
        public byte[] Resolved;
        public byte[] Walkable;
        public float[] Heights;
        private static readonly Regex Tokens = new Regex("\"[^\"]*\"|\\S+", RegexOptions.Compiled);

        public static CellBuffer Parse(string[] lines, uint version, uint width, uint height, string path)
        {
            int count = checked((int)((ulong)width * height));
            var cells = new CellBuffer {
                Resolved = new byte[count], Walkable = new byte[count], Heights = new float[count]
            };
            var seen = new byte[count];
            var culture = CultureInfo.InvariantCulture;
            for (int row = 0; row < count; ++row)
            {
                var tokens = Tokens.Matches(lines[row + 1]);
                if (tokens.Count != (version == 1 ? 4 : 5))
                    throw new InvalidDataException("Navigation authoring row is invalid: " + path + " row=" + row);
                int x = int.Parse(tokens[0].Value.Trim('"'), culture);
                int z = int.Parse(tokens[1].Value.Trim('"'), culture);
                uint surface = uint.Parse(tokens[2].Value.Trim('"'), culture);
                uint walkable = version == 1 ? surface : uint.Parse(tokens[3].Value.Trim('"'), culture);
                float level = float.Parse(tokens[version == 1 ? 3 : 4].Value.Trim('"'), culture);
                if (x < 0 || z < 0 || x >= width || z >= height || surface > 1 || walkable > surface ||
                    float.IsNaN(level) || float.IsInfinity(level))
                    throw new InvalidDataException("Navigation authoring cell is invalid: " + path + " row=" + row);
                int index = checked(z * (int)width + x);
                if (seen[index] != 0)
                    throw new InvalidDataException("Navigation authoring has duplicate cells: " + path);
                seen[index] = 1;
                cells.Resolved[index] = (byte)surface;
                cells.Walkable[index] = (byte)walkable;
                cells.Heights[index] = surface == 0 ? 0.0f : level;
            }
            return cells;
        }

        public static byte[] Serialize(uint width, uint height, float cellSize, float originX, float originZ,
            byte[] resolved, byte[] walkable, float[] heights, byte[] overrides)
        {
            int count = heights.Length;
            using (var stream = new MemoryStream(checked(20 + count * 5)))
            using (var writer = new BinaryWriter(stream))
            {
                writer.Write(width); writer.Write(height); writer.Write(cellSize);
                writer.Write(originX); writer.Write(originZ);
                for (int index = 0; index < count; ++index)
                    writer.Write((byte)(resolved[index] != 0 && (overrides[index] == 2 ||
                        (overrides[index] == 0 && walkable[index] != 0)) ? 1 : 0));
                foreach (float level in heights) writer.Write(level);
                writer.Flush();
                return stream.ToArray();
            }
        }

        public static double[] Analyze(byte[] bytes, uint width, uint height, bool requireSingleComponent, string areaId)
        {
            int count = checked((int)((ulong)width * height));
            int stride = checked((int)width);
            int heightOffset = 20 + count;
            int walkableCount = 0;
            double maximumStep = 0.0;
            for (int z = 0; z < height; ++z)
            for (int x = 0; x < width; ++x)
            {
                int index = z * stride + x;
                if (bytes[20 + index] != 1) continue;
                ++walkableCount;
                double level = BitConverter.ToSingle(bytes, heightOffset + 4 * index);
                if (x + 1 < width && bytes[20 + index + 1] == 1)
                    maximumStep = Math.Max(maximumStep, Math.Abs(level - BitConverter.ToSingle(bytes, heightOffset + 4 * (index + 1))));
                if (z + 1 < height && bytes[20 + index + stride] == 1)
                    maximumStep = Math.Max(maximumStep, Math.Abs(level - BitConverter.ToSingle(bytes, heightOffset + 4 * (index + stride))));
            }
            if (requireSingleComponent)
            {
                int first = -1;
                for (int index = 0; index < count; ++index)
                    if (bytes[20 + index] == 1) { first = index; break; }
                if (first < 0)
                    throw new InvalidDataException("Navigation grid has no walkable component: " + areaId);
                var visited = new byte[count];
                var queue = new int[count];
                queue[0] = first;
                visited[first] = 1;
                int head = 0, tail = 1;
                while (head < tail)
                {
                    int current = queue[head++];
                    int x = current % stride, z = current / stride;
                    Visit(x + 1 < width ? current + 1 : -1, bytes, visited, queue, ref tail);
                    Visit(x > 0 ? current - 1 : -1, bytes, visited, queue, ref tail);
                    Visit(z + 1 < height ? current + stride : -1, bytes, visited, queue, ref tail);
                    Visit(z > 0 ? current - stride : -1, bytes, visited, queue, ref tail);
                }
                if (tail != walkableCount)
                    throw new InvalidDataException("Navigation grid has disconnected walkable islands: " + areaId +
                        " connected=" + tail + " total=" + walkableCount);
            }
            return new double[] { walkableCount, maximumStep };
        }

        // Ground height of the walkable cell under (x, z), NaN when outside the grid or not walkable.
        public static double GroundAt(byte[] bytes, double x, double z)
        {
            uint width = BitConverter.ToUInt32(bytes, 0), height = BitConverter.ToUInt32(bytes, 4);
            float cell = BitConverter.ToSingle(bytes, 8);
            float originX = BitConverter.ToSingle(bytes, 12), originZ = BitConverter.ToSingle(bytes, 16);
            if (double.IsNaN(x) || double.IsNaN(z) || x < originX || z < originZ ||
                x >= (double)originX + (double)width * cell || z >= (double)originZ + (double)height * cell)
                return double.NaN;
            int cellX = (int)Math.Floor((x - originX) / cell), cellZ = (int)Math.Floor((z - originZ) / cell);
            if (cellX < 0 || cellZ < 0 || cellX >= width || cellZ >= height) return double.NaN;
            int index = cellZ * (int)width + cellX;
            if (bytes[20 + index] != 1) return double.NaN;
            return BitConverter.ToSingle(bytes, 20 + checked((int)((ulong)width * height)) + 4 * index);
        }

        // [minimum, maximum] height over the walkable cells; minimum > maximum when there is none.
        public static double[] WalkableBand(byte[] bytes)
        {
            uint width = BitConverter.ToUInt32(bytes, 0), height = BitConverter.ToUInt32(bytes, 4);
            int count = checked((int)((ulong)width * height));
            double minimum = double.MaxValue, maximum = double.MinValue;
            for (int index = 0; index < count; ++index)
            {
                if (bytes[20 + index] != 1) continue;
                double level = BitConverter.ToSingle(bytes, 20 + count + 4 * index);
                if (level < minimum) minimum = level;
                if (level > maximum) maximum = level;
            }
            return new double[] { minimum, maximum };
        }

        // Ground two grids share at one XZ, measured from the centre of every walkable cell of each grid
        // against the other grid (the rule CServerNavigation::Find_AmbiguousLayerOverlap uses).
        // Returns [pairs, closest separation, pairs closer than minimumSeparation, x, z, height1, height2];
        // x, z and the heights describe the first too-close pair, or the closest pair when none is too close.
        public static double[] MeasureLayerOverlap(byte[] first, byte[] second, double minimumSeparation)
        {
            double pairs = 0.0, tooClose = 0.0, closest = double.MaxValue;
            double[] closestPair = new double[4], firstTooClose = new double[4];
            bool haveTooClose = false;
            for (int pass = 0; pass < 2; ++pass)
            {
                byte[] from = pass == 0 ? first : second;
                byte[] to = pass == 0 ? second : first;
                uint fromWidth = BitConverter.ToUInt32(from, 0), fromHeight = BitConverter.ToUInt32(from, 4);
                float fromCell = BitConverter.ToSingle(from, 8);
                float fromOriginX = BitConverter.ToSingle(from, 12), fromOriginZ = BitConverter.ToSingle(from, 16);
                int fromCount = checked((int)((ulong)fromWidth * fromHeight));
                uint toWidth = BitConverter.ToUInt32(to, 0), toHeight = BitConverter.ToUInt32(to, 4);
                int toCount = checked((int)((ulong)toWidth * toHeight));
                for (int z = 0; z < fromHeight; ++z)
                {
                    float centerZ = fromOriginZ + ((float)z + 0.5f) * fromCell;
                    for (int x = 0; x < fromWidth; ++x)
                    {
                        int index = z * (int)fromWidth + x;
                        if (from[20 + index] != 1) continue;
                        float centerX = fromOriginX + ((float)x + 0.5f) * fromCell;
                        double other = GroundAt(to, centerX, centerZ);
                        if (double.IsNaN(other)) continue;
                        double level = BitConverter.ToSingle(from, 20 + fromCount + 4 * index);
                        double separation = Math.Abs(level - other);
                        pairs += 1.0;
                        if (separation < closest)
                        {
                            closest = separation;
                            closestPair = new double[] { centerX, centerZ, level, other };
                        }
                        if (separation < minimumSeparation)
                        {
                            tooClose += 1.0;
                            if (!haveTooClose)
                            {
                                haveTooClose = true;
                                firstTooClose = new double[] { centerX, centerZ, level, other };
                            }
                        }
                    }
                }
            }
            double[] report = haveTooClose ? firstTooClose : closestPair;
            return new double[] { pairs, pairs == 0.0 ? 0.0 : closest, tooClose, report[0], report[1], report[2], report[3] };
        }

        private static void Visit(int index, byte[] bytes, byte[] visited, int[] queue, ref int tail)
        {
            if (index < 0 || bytes[20 + index] != 1 || visited[index] != 0) return;
            visited[index] = 1;
            queue[tail++] = index;
        }
    }
}
'@
}

function New-UniformNavigationGrid {
    param(
        [string]$RelativeAuthoringPath,
        [single]$RuntimeMaximumStepHeight = 0.0
    )

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
        [double]::IsNaN([double]$source.defaultHeight) -or
        [single]::IsNaN($RuntimeMaximumStepHeight) -or
        [single]::IsInfinity($RuntimeMaximumStepHeight) -or
        $RuntimeMaximumStepHeight -lt 0.0) {
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
            RuntimeMaximumStepHeight = $RuntimeMaximumStepHeight
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

function Convert-NavigationRuntimeBlockers {
    param(
        [string]$AreaId,
        [byte[]]$GridBytes,
        [string]$AuthoringRoot = $repoRoot
    )

    $width = [BitConverter]::ToUInt32($GridBytes, 0)
    $height = [BitConverter]::ToUInt32($GridBytes, 4)
    $cellSize = [BitConverter]::ToSingle($GridBytes, 8)
    $originX = [BitConverter]::ToSingle($GridBytes, 12)
    $originZ = [BitConverter]::ToSingle($GridBytes, 16)
    $cellSizeText = $cellSize.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
    $originXText = $originX.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
    $originZText = $originZ.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
    $cellCount = [uint64]$width * [uint64]$height
    $path = [IO.Path]::GetFullPath((Join-Path $AuthoringRoot "Data/Navigation/$AreaId.navblockers"))
    if (-not [IO.File]::Exists($path)) {
        return [string[]]@("LOSTARK_NAVGRID_BLOCKERS 1 `"$AreaId`" $width $height $cellSizeText $originXText $originZText 0")
    }

    $lines = @([IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) { throw "Navigation blockers are empty: $path" }
    $header = @(Split-NavigationTokens $lines[0])
    if ($header.Count -ne 9 -or $header[0] -ne 'LOSTARK_NAVGRID_BLOCKERS' -or
        $header[1] -ne '1' -or $header[2] -cne $AreaId -or
        [uint32]$header[3] -ne $width -or [uint32]$header[4] -ne $height -or
        [single]$header[5] -ne $cellSize -or [single]$header[6] -ne $originX -or
        [single]$header[7] -ne $originZ -or [uint32]$header[8] -gt 256) {
        throw "Navigation blocker header does not match the published grid: $path"
    }
    $regionCount = [uint32]$header[8]
    $cursor = 1
    $stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
    $regionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $normalized = [Collections.Generic.List[string]]::new()
    $normalized.Add("LOSTARK_NAVGRID_BLOCKERS 1 `"$AreaId`" $width $height $cellSizeText $originXText $originZText $regionCount")
    for ($regionIndex = 0; $regionIndex -lt $regionCount; ++$regionIndex) {
        if ($cursor -ge $lines.Count) { throw "Navigation blocker region is truncated: $path" }
        $tokens = @(Split-NavigationTokens $lines[$cursor++])
        if ($tokens.Count -ne 5 -or $tokens[0] -ne 'REGION' -or
            $tokens[1] -cnotmatch $stableIdPattern -or $tokens[2] -cnotmatch $stableIdPattern -or
            $tokens[3] -notin @('0','1') -or [uint32]$tokens[4] -eq 0 -or
            [uint32]$tokens[4] -gt $cellCount -or -not $regionIds.Add([string]$tokens[1])) {
            throw "Navigation blocker region is invalid: $path row=$regionIndex"
        }
        $regionCellCount = [uint32]$tokens[4]
        $cells = [Collections.Generic.HashSet[uint32]]::new()
        $cellRows = [Collections.Generic.List[object]]::new()
        for ($cellRow = 0; $cellRow -lt $regionCellCount; ++$cellRow) {
            if ($cursor -ge $lines.Count) { throw "Navigation blocker cells are truncated: $path" }
            $cellTokens = @(Split-NavigationTokens $lines[$cursor++])
            if ($cellTokens.Count -ne 2) { throw "Navigation blocker cell is invalid: $path" }
            $cellX = [int32]$cellTokens[0]
            $cellZ = [int32]$cellTokens[1]
            if ($cellX -lt 0 -or $cellZ -lt 0 -or $cellX -ge $width -or $cellZ -ge $height) {
                throw "Navigation blocker cell is outside the grid: $path"
            }
            $cellIndex = [uint32]($cellZ * $width + $cellX)
            if (-not $cells.Add($cellIndex) -or $GridBytes[20 + $cellIndex] -ne 1) {
                throw "Navigation blocker cell is duplicate or not base walkable: $path"
            }
            $cellRows.Add([pscustomobject]@{ X=$cellX; Z=$cellZ; Index=$cellIndex })
        }
        $normalized.Add("REGION `"$($tokens[1])`" `"$($tokens[2])`" $($tokens[3]) $regionCellCount")
        foreach ($cell in @($cellRows | Sort-Object Index)) {
            $normalized.Add("$($cell.X) $($cell.Z)")
        }
    }
    if ($cursor -ne $lines.Count) { throw "Navigation blockers have trailing rows: $path" }
    return [string[]]$normalized.ToArray()
}

function Convert-NavigationRuntimePolicy {
    param([object]$Grid)

    $maximumStep = [single]$Grid.RuntimeMaximumStepHeight
    if ([single]::IsNaN($maximumStep) -or
        [single]::IsInfinity($maximumStep) -or
        $maximumStep -lt 0.0) {
        throw "Navigation runtime policy is invalid: $($Grid.AreaId)"
    }
    $maximumStepText = $maximumStep.ToString(
        'R', [Globalization.CultureInfo]::InvariantCulture)
    return [string[]]@(
        "LOSTARK_NAVIGATION_POLICY 1 `"$($Grid.AreaId)`" $maximumStepText")
}

function Convert-NavigationAuthoringGrid {
    param(
        [string]$RelativeSourcePath,
        [string]$RelativePaintPath,
        [single]$MaximumStepHeight = 0.0,
        [single]$RuntimeMaximumStepHeight = 0.0,
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
        $cellSize -le 0 -or
        [single]::IsNaN($RuntimeMaximumStepHeight) -or
        [single]::IsInfinity($RuntimeMaximumStepHeight) -or
        $RuntimeMaximumStepHeight -lt 0.0) {
        throw "Navigation authoring dimensions are invalid: $sourcePath"
    }

    $cells = [LostArk.NavigationPublish.CellBuffer]::Parse(
        $lines, $version, $width, $height, $sourcePath)
    $resolved = $cells.Resolved
    $baseWalkable = $cells.Walkable
    $heights = $cells.Heights

    # 0 = inherit baked state, 1 = force blocked, 2 = force walkable.
    $overrides = [byte[]]::new([int]$cellCount)
    $paintSeen = [byte[]]::new([int]$cellCount)
    # A pinhole close reads the baked surface, never a value an earlier
    # paint row already healed, so the published grid does not depend on
    # the order the override rows happen to appear in.
    $bakedResolved = [byte[]]::new([int]$cellCount)
    $bakedHeights = [single[]]::new([int]$cellCount)
    [Array]::Copy($resolved, $bakedResolved, [int]$cellCount)
    [Array]::Copy($heights, $bakedHeights, [int]$cellCount)
    $seamHeightTolerance = [single]0.5
    $paintPath = [IO.Path]::GetFullPath((Join-Path $AuthoringRoot $RelativePaintPath))
    if ([IO.File]::Exists($paintPath)) {
        $paintLines = @([IO.File]::ReadAllLines($paintPath, [Text.Encoding]::UTF8))
        if ($paintLines.Count -lt 1) { throw "Navigation paint is empty: $paintPath" }
        $paintHeader = @(Split-NavigationTokens $paintLines[0])
        if ($paintHeader.Count -ne 9 -or
            $paintHeader[0] -ne 'LOSTARK_NAVGRID_PAINT' -or
            $paintHeader[1] -notin @('1', '2', '3') -or
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
            $minimumPaintRowCount = if ($paintVersion -eq 1) { 2 } else { 3 }
            $maximumPaintRowCount = if ($paintVersion -eq 3) { 4 } else {
                $minimumPaintRowCount
            }
            if ($tokens.Count -lt $minimumPaintRowCount -or
                $tokens.Count -gt $maximumPaintRowCount) {
                throw "Navigation paint row is invalid: $paintPath"
            }
            $cellX = [int32]::Parse($tokens[0], $culture)
            $cellZ = [int32]::Parse($tokens[1], $culture)
            if ($cellX -lt 0 -or $cellZ -lt 0 -or
                $cellX -ge $width -or $cellZ -ge $height) {
                throw "Navigation paint cell is outside the grid: $paintPath"
            }
            $index = $cellZ * $width + $cellX
            if ($paintSeen[$index] -ne 0) {
                throw "Navigation paint has duplicate cells: $paintPath"
            }
            $paintSeen[$index] = 1
            # A bake ray that slips through a floor crack leaves the cell it
            # crossed either with no surface at all or with the height of
            # whatever it finally hit far below. WALKABLE paint declares such
            # a cell ordinary floor, so it also levels the cell with the
            # ground around it instead of leaving a body to drop through the
            # seam. The level comes from the median of the baked ground
            # around it rather than from every neighbour agreeing, because a
            # seam runs as a line and its own cells sit next to each other;
            # requiring unanimity would let two neighbouring scars hold each
            # other down forever. At least five of the eight neighbours must
            # carry a baked surface and three quarters of those must land on
            # the median, so a cell standing in genuinely varied ground - a
            # ramp, a ledge, a stair - never qualifies and is published
            # exactly as the bake found it. Reading the baked snapshot keeps
            # the result independent of the order the override rows appear in.
            $paintState = if ($paintVersion -eq 1) { 'BLOCKED' } else { $tokens[2] }
            if ($paintState -notin @('BLOCKED', 'WALKABLE') -and
                -not ($paintVersion -eq 3 -and $paintState -eq 'HEIGHT')) {
                throw "Navigation paint override is invalid: $paintPath"
            }
            $hasExplicitHeight = $paintVersion -eq 3 -and $tokens.Count -eq 4
            if ($paintState -eq 'HEIGHT' -and -not $hasExplicitHeight) {
                throw "Navigation HEIGHT row is missing its height: $paintPath"
            }
            $explicitHeight = [single]0
            if ($hasExplicitHeight) {
                # A WALKABLE row that carries its own height authors floor where
                # the bake produced none, so it is the one case allowed to name
                # an unresolved cell. An isolated platform - a floating card in
                # the maze, a prop the bake ray missed - has no ring of baked
                # neighbours to borrow a level from, so the seam median can
                # never reach it and the hand-written height is the only way to
                # place it. HEIGHT stays a correction for a surface the bake
                # already found and still requires a resolved cell.
                if (-not [single]::TryParse(
                    $tokens[3],
                    [Globalization.NumberStyles]::Float,
                    $culture,
                    [ref]$explicitHeight) -or
                    [single]::IsNaN($explicitHeight) -or
                    [single]::IsInfinity($explicitHeight) -or
                    ($resolved[$index] -eq 0 -and $paintState -ne 'WALKABLE')) {
                    throw "Navigation paint height override is invalid: $paintPath"
                }
            }
            $isWalkablePaint = $paintState -eq 'WALKABLE'
            $seamHeight = $null
            if ($isWalkablePaint -and -not $hasExplicitHeight -and
                $cellX -gt 0 -and $cellZ -gt 0 -and
                $cellX -lt $width - 1 -and $cellZ -lt $height - 1) {
                $ringHeights = [Collections.Generic.List[single]]::new()
                foreach ($offsetZ in -1, 0, 1) {
                    foreach ($offsetX in -1, 0, 1) {
                        if ($offsetX -eq 0 -and $offsetZ -eq 0) { continue }
                        $ringIndex = $index + $offsetZ * $width + $offsetX
                        if ($bakedResolved[$ringIndex] -ne 0) {
                            $ringHeights.Add($bakedHeights[$ringIndex])
                        }
                    }
                }
                if ($ringHeights.Count -ge 5) {
                    $ringHeights.Sort()
                    $ringCount = $ringHeights.Count
                    $ringMedian = if (1 -eq $ringCount % 2) {
                        [double]$ringHeights[[int]($ringCount / 2)]
                    }
                    else {
                        ([double]$ringHeights[$ringCount / 2 - 1] +
                            [double]$ringHeights[$ringCount / 2]) / 2
                    }
                    $onMedian = 0
                    foreach ($ringHeight in $ringHeights) {
                        if ([Math]::Abs([double]$ringHeight - $ringMedian) -le
                            [double]$seamHeightTolerance) {
                            ++$onMedian
                        }
                    }
                    if ($onMedian -ge [Math]::Ceiling(3 * $ringCount / 4)) {
                        $seamHeight = [single]$ringMedian
                    }
                }
            }
            if ($hasExplicitHeight) {
                $heights[$index] = $explicitHeight
                # Only a WALKABLE row reaches here unresolved; the validation
                # above rejects every other state. Marking it resolved is what
                # makes the authored cell survive into the runtime grid, whose
                # writer drops any cell the bake never resolved.
                $resolved[$index] = 1
            }
            elseif ($resolved[$index] -eq 0) {
                if ($null -eq $seamHeight) {
                    throw "Navigation paint targets an unresolved cell: $paintPath"
                }
                $resolved[$index] = 1
                $heights[$index] = $seamHeight
            }
            elseif ($null -ne $seamHeight -and
                ([double]$seamHeight - [double]$bakedHeights[$index]) -gt
                [double]$seamHeightTolerance) {
                $heights[$index] = $seamHeight
            }
            if ($paintState -eq 'BLOCKED') {
                $overrides[$index] = 1
            }
            elseif ($paintState -eq 'WALKABLE') {
                $overrides[$index] = 2
            }
        }
    }

    return [pscustomobject]@{
        AreaId = $areaId
        Bytes = [LostArk.NavigationPublish.CellBuffer]::Serialize(
            $width, $height, $cellSize, $originX, $originZ,
            $resolved, $baseWalkable, $heights, $overrides)
        WorldPath = "Data/Worlds/$areaId/Gameplay.world.json"
        MaximumStepHeight = $MaximumStepHeight
        RuntimeMaximumStepHeight = $RuntimeMaximumStepHeight
        RequireSingleComponent = [bool]$RequireSingleComponent
    }
}

function Read-NavigationRegionManifest {
    param(
        [string]$AreaId,
        [string]$AuthoringRoot = $repoRoot
    )
    $path = [IO.Path]::GetFullPath(
        (Join-Path $AuthoringRoot "Data/Navigation/$AreaId.navregions"))
    if (-not [IO.File]::Exists($path)) { return @() }
    $lines = @([IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) { throw "Navigation region manifest is empty: $path" }
    $header = @(Split-NavigationTokens $lines[0])
    if ($header.Count -ne 4 -or $header[0] -ne 'LOSTARK_NAVGRID_REGIONS' -or
        $header[1] -ne '1' -or $header[2] -cne $AreaId -or
        [uint32]$header[3] -gt 64) {
        throw "Navigation region manifest header is invalid: $path"
    }
    $regionCount = [uint32]$header[3]
    if ($lines.Count -ne 1 + $regionCount) {
        throw "Navigation region manifest row count is invalid: $path"
    }
    $culture = [Globalization.CultureInfo]::InvariantCulture
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $regions = [Collections.Generic.List[object]]::new()
    for ($index = 0; $index -lt $regionCount; ++$index) {
        $tokens = @(Split-NavigationTokens $lines[$index + 1])
        if ($tokens.Count -ne 3 -or $tokens[0] -ne 'REGION' -or
            $tokens[1] -cnotmatch '^[A-Za-z0-9_-]{1,32}$' -or
            -not $seen.Add([string]$tokens[1])) {
            throw "Navigation region manifest row is invalid: $path row=$index"
        }
        $step = [single]::Parse($tokens[2], $culture)
        if ([single]::IsNaN($step) -or [single]::IsInfinity($step) -or
            $step -lt 0.0) {
            throw "Navigation region step height is invalid: $path row=$index"
        }
        $regions.Add([pscustomobject]@{
            RegionId = [string]$tokens[1]
            StepHeight = $step
        })
    }
    return @($regions.ToArray())
}

function Convert-NavigationRegionGrids {
    param(
        [string]$AreaId,
        [string]$AuthoringRoot = $repoRoot
    )
    $manifest = @(Read-NavigationRegionManifest `
        -AreaId $AreaId -AuthoringRoot $AuthoringRoot)
    $grids = [Collections.Generic.List[object]]::new()
    foreach ($region in $manifest) {
        $gridId = "$AreaId.$($region.RegionId)"
        # A region grid is a normal authoring grid whose id carries the dot, so
        # the same converter and the same paint contract apply unchanged.
        $grid = Convert-NavigationAuthoringGrid `
            -RelativeSourcePath "Data/Navigation/$gridId.navsource" `
            -RelativePaintPath "Data/Navigation/$gridId.navpaint" `
            -RuntimeMaximumStepHeight $region.StepHeight `
            -AuthoringRoot $AuthoringRoot
        if ($grid.AreaId -cne $gridId) {
            throw "Navigation region source declares a different id: $gridId"
        }
        $grids.Add($grid)
    }
    return @($grids.ToArray())
}

function Get-NavigationGridBounds {
    param([object]$Grid)
    $bytes = [byte[]]$Grid.Bytes
    $width = [BitConverter]::ToUInt32($bytes, 0)
    $height = [BitConverter]::ToUInt32($bytes, 4)
    $cellSize = [BitConverter]::ToSingle($bytes, 8)
    $originX = [BitConverter]::ToSingle($bytes, 12)
    $originZ = [BitConverter]::ToSingle($bytes, 16)
    return [pscustomobject]@{
        MinX = [double]$originX
        MinZ = [double]$originZ
        MaxX = [double]$originX + [double]$width * $cellSize
        MaxZ = [double]$originZ + [double]$height * $cellSize
        CellSize = [double]$cellSize
        Width = $width
        Height = $height
    }
}

# Two regions may share an XZ footprint only as separate height layers. A query tells them apart by
# its height hint, which is off by up to one step on a real deck, so the ground of the two layers at
# one XZ must be at least twice the larger step policy apart, and never less than 2 m. The Server
# applies the same rule when it loads the manifest (CServerNavigation::Find_AmbiguousLayerOverlap).
function Get-NavigationLayerMinimumSeparation {
    param([object]$First, [object]$Second)
    $step = [Math]::Max([double]$First.RuntimeMaximumStepHeight,
        [double]$Second.RuntimeMaximumStepHeight)
    return [Math]::Max(2.0, 2.0 * $step)
}

function Assert-NavigationRegionFootprints {
    param([object[]]$RegionGrids)
    for ($outer = 0; $outer -lt $RegionGrids.Count; ++$outer) {
        $a = Get-NavigationGridBounds $RegionGrids[$outer]
        for ($inner = $outer + 1; $inner -lt $RegionGrids.Count; ++$inner) {
            $b = Get-NavigationGridBounds $RegionGrids[$inner]
            if (-not ($a.MinX -lt $b.MaxX -and $b.MinX -lt $a.MaxX -and
                $a.MinZ -lt $b.MaxZ -and $b.MinZ -lt $a.MaxZ)) { continue }
            $minimum = Get-NavigationLayerMinimumSeparation `
                $RegionGrids[$outer] $RegionGrids[$inner]
            $measure = [LostArk.NavigationPublish.CellBuffer]::MeasureLayerOverlap(
                [byte[]]$RegionGrids[$outer].Bytes, [byte[]]$RegionGrids[$inner].Bytes, $minimum)
            if ($measure[2] -gt 0) {
                throw ("Navigation regions overlap: " +
                    "$($RegionGrids[$outer].AreaId) and " +
                    "$($RegionGrids[$inner].AreaId) " +
                    "($([int]$measure[2]) cells have ground on both layers closer than " +
                    "$minimum m; first at x=$([Math]::Round($measure[3], 3)) " +
                    "z=$([Math]::Round($measure[4], 3)) heights " +
                    "$([Math]::Round($measure[5], 3)) and $([Math]::Round($measure[6], 3)))")
            }
        }
    }
}

# Same dispatch as CServerNavigation::Select_Region with the point height as the hint: the region whose
# footprint contains (x, z), the base grid when none does. Several regions may share the footprint; a
# region with ground at that XZ beats one without, then the nearest ground wins, then manifest order.
function Select-NavigationOwnerGrid {
    param(
        [object]$BaseGrid,
        [object[]]$RegionGrids,
        [double]$X,
        [double]$Y,
        [double]$Z
    )
    $candidates = @(foreach ($region in $RegionGrids) {
        $regionBounds = Get-NavigationGridBounds $region
        if ($X -ge $regionBounds.MinX -and $X -lt $regionBounds.MaxX -and
            $Z -ge $regionBounds.MinZ -and $Z -lt $regionBounds.MaxZ) { $region }
    })
    if ($candidates.Count -eq 0) { return $BaseGrid }
    if ($candidates.Count -eq 1) { return $candidates[0] }
    $best = $null
    $bestHasGround = $false
    $bestDistance = 0.0
    foreach ($candidate in $candidates) {
        $ground = [LostArk.NavigationPublish.CellBuffer]::GroundAt([byte[]]$candidate.Bytes, $X, $Z)
        $hasGround = -not [double]::IsNaN($ground)
        if ($hasGround) { $distance = [Math]::Abs($ground - $Y) }
        else {
            $band = [LostArk.NavigationPublish.CellBuffer]::WalkableBand([byte[]]$candidate.Bytes)
            $distance = if ($band[0] -gt $band[1]) { [double]::PositiveInfinity }
                else { [Math]::Max([Math]::Max($band[0] - $Y, 0.0), $Y - $band[1]) }
        }
        if ($null -eq $best -or ($hasGround -and -not $bestHasGround) -or
            ($hasGround -eq $bestHasGround -and $distance -lt $bestDistance)) {
            $best = $candidate
            $bestHasGround = $hasGround
            $bestDistance = $distance
        }
    }
    return $best
}

function Convert-NavigationRuntimeRegionManifest {
    param(
        [string]$AreaId,
        [object[]]$RegionGrids
    )
    $culture = [Globalization.CultureInfo]::InvariantCulture
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_NAVGRID_REGIONS 1 `"$AreaId`" $($RegionGrids.Count)")
    foreach ($grid in $RegionGrids) {
        $regionId = $grid.AreaId.Substring($AreaId.Length + 1)
        $stepText = ([single]$grid.RuntimeMaximumStepHeight).ToString(
            'R', $culture)
        $lines.Add("REGION `"$regionId`" $stepText")
    }
    return [string[]]$lines.ToArray()
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

    $analysis = [LostArk.NavigationPublish.CellBuffer]::Analyze(
        $bytes, $width, $height, [bool]$Grid.RequireSingleComponent, $Grid.AreaId)
    $walkableCount = [int]$analysis[0]
    $maximumObservedStep = $analysis[1]
    if ($Grid.MaximumStepHeight -gt 0.0 -and
        $maximumObservedStep -gt $Grid.MaximumStepHeight + 0.000001) {
        throw "Navigation grid has an unsafe adjacent step: $($Grid.AreaId) max=$maximumObservedStep limit=$($Grid.MaximumStepHeight)"
    }

    return "${width}x${height}, cellSize=$cellSize, walkable=$walkableCount, maxStep=$maximumObservedStep"
}

function Assert-NavigationPlacements {
    param(
        [object]$BaseGrid,
        [object[]]$RegionGrids,
        [string]$AuthoringRoot = $repoRoot
    )
    $world = Get-Content -LiteralPath (Join-Path $AuthoringRoot $BaseGrid.WorldPath) `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    foreach ($placement in @($world.placements | Where-Object {
        $_.kind -eq 'boss' -or ($_.enabled -and $_.kind -eq 'playerSpawn') })) {
        $x = [double]$placement.position[0]
        $y = [double]$placement.position[1]
        $z = [double]$placement.position[2]
        if ([double]::IsNaN($x) -or [double]::IsInfinity($x) -or
            [double]::IsNaN($y) -or [double]::IsInfinity($y) -or
            [double]::IsNaN($z) -or [double]::IsInfinity($z)) {
            throw "Gameplay placement position is not finite: $($placement.placementId)"
        }
        # Same dispatch as CServerNavigation::Select_Region, with the placement height as the hint.
        $owner = Select-NavigationOwnerGrid -BaseGrid $BaseGrid -RegionGrids $RegionGrids `
            -X $x -Y $y -Z $z
        $bytes = [byte[]]$owner.Bytes
        $bounds = Get-NavigationGridBounds $owner
        $cellX = [int][Math]::Floor(($x - $bounds.MinX) / $bounds.CellSize)
        $cellZ = [int][Math]::Floor(($z - $bounds.MinZ) / $bounds.CellSize)
        if ($cellX -lt 0 -or $cellZ -lt 0 -or
            $cellX -ge $bounds.Width -or $cellZ -ge $bounds.Height) {
            throw "Gameplay placement is outside server navigation: $($placement.placementId)"
        }
        $index = $cellZ * $bounds.Width + $cellX
        if ($bytes[20 + $index] -ne 1) {
            throw "Gameplay placement is not on a walkable server cell: $($placement.placementId) grid=$($owner.AreaId)"
        }
        $cellCount = [uint64]$bounds.Width * [uint64]$bounds.Height
        $heightOffset = 20 + [int]$cellCount + 4 * $index
        $cellHeight = [BitConverter]::ToSingle($bytes, $heightOffset)
        # Match CGameRoom::Build_WorldEntity: Big Saydon owns its authored
        # height above the G2 floor; its XZ footprint must still be walkable.
        $preserveAuthoredHeight =
            $BaseGrid.AreaId -ceq 'LV_LUT_MIDNIGHTC_ED' -and
            $world.areaId -ceq 'LV_LUT_MIDNIGHTC_ED' -and
            $placement.kind -ceq 'boss' -and
            $placement.archetypeId -ceq 'BOSS_KAKULSAYDON_G2_BIG_SAYDON'
        if (-not $preserveAuthoredHeight -and
            [Math]::Abs($y - $cellHeight) -gt 0.25) {
            throw "Gameplay placement height differs from server navigation: $($placement.placementId) grid=$($owner.AreaId)"
        }
    }
}

function Invoke-NavigationPaintContractTest {
    $fixtureRoot = Join-Path ([IO.Path]::GetTempPath()) (
        'lostark-navpaint-contract-' + [Guid]::NewGuid().ToString('N'))
    $navigationRoot = Join-Path $fixtureRoot 'Data/Navigation'
    [IO.Directory]::CreateDirectory($navigationRoot) | Out-Null
    $sourcePath = Join-Path $navigationRoot 'TEST_NAV_PAINT.navsource'
    $paintPath = Join-Path $navigationRoot 'TEST_NAV_PAINT.navpaint'
    $blockerPath = Join-Path $navigationRoot 'TEST_NAV_PAINT.navblockers'
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

        $heightPaint = @(
            'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT" 2 1 1 0 0 2',
            '0 0 WALKABLE -0.25',
            '1 0 WALKABLE 0.5'
        ) -join "`n"
        [IO.File]::WriteAllText($paintPath, $heightPaint, [Text.Encoding]::UTF8)
        $heightGrid = Convert-NavigationAuthoringGrid `
            -RelativeSourcePath 'Data/Navigation/TEST_NAV_PAINT.navsource' `
            -RelativePaintPath 'Data/Navigation/TEST_NAV_PAINT.navpaint' `
            -RuntimeMaximumStepHeight 0.75 `
            -AuthoringRoot $fixtureRoot
        $heightOffset = 22
        if ($heightGrid.Bytes[20] -ne 1 -or $heightGrid.Bytes[21] -ne 1 -or
            [BitConverter]::ToSingle($heightGrid.Bytes, $heightOffset) -ne -0.25 -or
            [BitConverter]::ToSingle($heightGrid.Bytes, $heightOffset + 4) -ne 0.5) {
            throw 'Navigation paint v3 did not publish explicit cell heights'
        }
        $policy = @(Convert-NavigationRuntimePolicy -Grid $heightGrid)
        if ($policy.Count -ne 1 -or
            $policy[0] -ne 'LOSTARK_NAVIGATION_POLICY 1 "TEST_NAV_PAINT" 0.75') {
            throw 'Navigation runtime step policy was not emitted canonically'
        }

        $placementPath = Join-Path $fixtureRoot 'placement.world.json'
        $placementCases = @(
            @{ Name='authored boss height'; Error=$null },
            @{ Name='other boss'; Archetype='BOSS_KAKULSAYDON_G2_KOUKU'; Error='height differs' },
            @{ Name='other area'; Area='OTHER_AREA'; Error='height differs' },
            @{ Name='other kind'; Kind='playerSpawn'; Error='height differs' },
            @{ Name='blocked footprint'; Blocked=$true; X=0.5; Error='not on a walkable' },
            @{ Name='outside footprint'; X=3.5; Error='outside server navigation' },
            @{ Name='non-finite height'; Y='NaN'; Error='not finite' }
        )
        foreach ($case in $placementCases) {
            $area = if ($case.ContainsKey('Area')) { $case.Area } else { 'LV_LUT_MIDNIGHTC_ED' }
            $kind = if ($case.ContainsKey('Kind')) { $case.Kind } else { 'boss' }
            $archetype = if ($case.ContainsKey('Archetype')) { $case.Archetype } else {
                'BOSS_KAKULSAYDON_G2_BIG_SAYDON'
            }
            $x = if ($case.ContainsKey('X')) { $case.X } else { 1.5 }
            $y = if ($case.ContainsKey('Y')) { $case.Y } else { 8.63 }
            $placementDocument = @{
                areaId=$area
                placements=@(@{
                    placementId='boss.height.contract'; kind=$kind; archetypeId=$archetype
                    enabled=($kind -eq 'playerSpawn'); position=@($x, $y, 0.5)
                })
            }
            [IO.File]::WriteAllText($placementPath,
                ($placementDocument | ConvertTo-Json -Depth 8), [Text.Encoding]::UTF8)
            $placementGrid = [pscustomobject]@{
                AreaId=$area; WorldPath='placement.world.json'
                Bytes=$(if ($case.Blocked) { $grid.Bytes } else { $heightGrid.Bytes })
            }
            $failure = $null
            try {
                Assert-NavigationPlacements -BaseGrid $placementGrid -RegionGrids @() `
                    -AuthoringRoot $fixtureRoot
            }
            catch {
                $failure = $_.Exception.Message
            }
            if (($null -eq $case.Error -and $null -ne $failure) -or
                ($null -ne $case.Error -and
                    ($null -eq $failure -or $failure -notlike "*$($case.Error)*"))) {
                throw "Navigation placement contract failed: $($case.Name): $failure"
            }
        }

        $regionSource = @(
            'LOSTARK_NAVGRID_SOURCE 2 "TEST_NAV_PAINT.fine" 2 1 0.5 10 10 0 0 0 1 1 1 0 50 1 2',
            '0 0 1 1 3',
            '1 0 1 1 3'
        ) -join "`n"
        $regionPaint = 'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT.fine" 2 1 0.5 10 10 0'
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.fine.navsource'),
            $regionSource, [Text.Encoding]::UTF8)
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.fine.navpaint'),
            $regionPaint, [Text.Encoding]::UTF8)
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'),
            (@(
                'LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1',
                'REGION "fine" 0.75'
            ) -join "`n"), [Text.Encoding]::UTF8)
        $regionGrids = @(Convert-NavigationRegionGrids `
            -AreaId 'TEST_NAV_PAINT' -AuthoringRoot $fixtureRoot)
        if ($regionGrids.Count -ne 1 -or
            $regionGrids[0].AreaId -cne 'TEST_NAV_PAINT.fine' -or
            [BitConverter]::ToSingle([byte[]]$regionGrids[0].Bytes, 8) -ne 0.5) {
            throw 'Navigation region grid did not convert from its manifest'
        }
        Assert-NavigationRegionFootprints -RegionGrids $regionGrids
        $manifestLines = @(Convert-NavigationRuntimeRegionManifest `
            -AreaId 'TEST_NAV_PAINT' -RegionGrids $regionGrids)
        if ($manifestLines.Count -ne 2 -or
            $manifestLines[0] -ne 'LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1' -or
            $manifestLines[1] -ne 'REGION "fine" 0.75') {
            throw 'Navigation runtime region manifest was not emitted canonically'
        }
        $invalidManifests = @(
            @('LOSTARK_NAVGRID_REGIONS 2 "TEST_NAV_PAINT" 1', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "WRONG_AREA" 1', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1', 'REGION "fine.dotted" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 2', 'REGION "fine" 0.75', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1', 'REGION "absent" 0.75')
        )
        foreach ($invalidManifest in $invalidManifests) {
            [IO.File]::WriteAllText(
                (Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'),
                ($invalidManifest -join "`n"), [Text.Encoding]::UTF8)
            $rejected = $false
            try {
                Convert-NavigationRegionGrids `
                    -AreaId 'TEST_NAV_PAINT' -AuthoringRoot $fixtureRoot | Out-Null
            }
            catch {
                $rejected = $true
            }
            if (-not $rejected) {
                throw 'Navigation region manifest invalid contract was accepted'
            }
        }
        [IO.File]::Delete((Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'))
        if (@(Convert-NavigationRegionGrids `
            -AreaId 'TEST_NAV_PAINT' -AuthoringRoot $fixtureRoot).Count -ne 0) {
            throw 'Navigation region conversion without a manifest was not empty'
        }
        [IO.File]::WriteAllText($paintPath, $validPaint, [Text.Encoding]::UTF8)
        $validBlocker = @(
            'LOSTARK_NAVGRID_BLOCKERS 1 "TEST_NAV_PAINT" 2 1 1 0 0 1',
            'REGION "navregion.test.wall" "condition.test.wall.destroyed" 0 1',
            '1 0'
        ) -join "`n"
        [IO.File]::WriteAllText($blockerPath, $validBlocker, [Text.Encoding]::UTF8)
        $normalizedBlockers = @(Convert-NavigationRuntimeBlockers `
            -AreaId 'TEST_NAV_PAINT' -GridBytes $grid.Bytes `
            -AuthoringRoot $fixtureRoot)
        if ($normalizedBlockers.Count -ne 3 -or
            $normalizedBlockers[1] -notmatch 'navregion\.test\.wall') {
            throw 'Navigation runtime blocker did not compile canonically'
        }

        $invalidBlockers = @(
            @(
                'LOSTARK_NAVGRID_BLOCKERS 1 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                'REGION "navregion.test.wall" "condition.test.wall.destroyed" 0 2',
                '1 0',
                '1 0'),
            @(
                'LOSTARK_NAVGRID_BLOCKERS 1 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                'REGION "navregion.test.wall" "condition.test.wall.destroyed" 0 1',
                '0 0'),
            @(
                'LOSTARK_NAVGRID_BLOCKERS 1 "WRONG_AREA" 2 1 1 0 0 1',
                'REGION "navregion.test.wall" "condition.test.wall.destroyed" 0 1',
                '1 0')
        )
        foreach ($invalidBlocker in $invalidBlockers) {
            [IO.File]::WriteAllText(
                $blockerPath,
                ($invalidBlocker -join "`n"),
                [Text.Encoding]::UTF8)
            $rejected = $false
            try {
                Convert-NavigationRuntimeBlockers `
                    -AreaId 'TEST_NAV_PAINT' -GridBytes $grid.Bytes `
                    -AuthoringRoot $fixtureRoot | Out-Null
            }
            catch {
                $rejected = $true
            }
            if (-not $rejected) {
                throw 'Navigation blocker invalid contract was accepted'
            }
        }

        $invalidCases = @(
            @(
                'LOSTARK_NAVGRID_PAINT 4 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                '0 0 WALKABLE'),
            @(
                'LOSTARK_NAVGRID_PAINT 2 "TEST_NAV_PAINT" 2 1 1 0 0 2',
                '0 0 WALKABLE',
                '0 0 BLOCKED'),
            @(
                'LOSTARK_NAVGRID_PAINT 2 "WRONG_AREA" 2 1 1 0 0 1',
                '0 0 WALKABLE'),
            @(
                'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                '0 0 HEIGHT'),
            @(
                'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT" 2 1 1 0 0 1',
                '0 0 WALKABLE not-a-height')
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

function Invoke-NavigationCellContractTest {
    # Source v1/v2, quoted tokens and row order must produce the same bytes.
    # The second cell has no surface: its authored height must stay discarded.
    $expected = $null
    foreach ($sourceCase in @(
        @{ Version=1; Rows=@('1 0 0 99', '0 0 1 1.25') },
        @{ Version=2; Rows=@('0 0 1 1 1.25', '1 0 0 0 99') },
        @{ Version=2; Rows=@('"1" "0" "0" "0" "99"', '"0" "0" "1" "1" "1.25"') }
    )) {
        $cells = [LostArk.NavigationPublish.CellBuffer]::Parse(
            [string[]](@('header') + $sourceCase.Rows), $sourceCase.Version, 2, 1, 'cell-contract')
        $bytes = [LostArk.NavigationPublish.CellBuffer]::Serialize(
            2, 1, 0.5, -2, 3, $cells.Resolved, $cells.Walkable, $cells.Heights, [byte[]]@(0, 0))
        if ($bytes.Length -ne 30 -or $bytes[20] -ne 1 -or $bytes[21] -ne 0 -or
            [BitConverter]::ToSingle($bytes, 22) -ne 1.25 -or
            [BitConverter]::ToSingle($bytes, 26) -ne 0 -or
            [BitConverter]::ToSingle($bytes, 8) -ne 0.5 -or
            [BitConverter]::ToSingle($bytes, 12) -ne -2 -or
            [BitConverter]::ToSingle($bytes, 16) -ne 3) {
            throw 'Navigation dense cell conversion changed the binary contract'
        }
        $actual = [BitConverter]::ToString($bytes)
        if ($null -ne $expected -and $actual -cne $expected) {
            throw 'Navigation dense cell conversion depends on source version or row order'
        }
        $expected = $actual
    }
    foreach ($badRows in @(
        @('0 0 1 1 0', '0 0 1 1 0'),
        @('0 0 1 1 0', '2 0 1 1 0'),
        @('0 0 1 1 0', '1 -1 1 1 0'),
        @('0 0 1 1 0', '1 0 2 1 0'),
        @('0 0 1 1 0', '1 0 0 1 0'),
        @('0 0 1 1 0', '1 0 1 1 NaN'),
        @('0 0 1 1 0', '1 0 1 1 Infinity'),
        @('0 0 1 1 0', '1 0 1 1'),
        @('0 0 1 1 0', '1 0 1 1 0 extra')
    )) {
        $rejected = $false
        try {
            [LostArk.NavigationPublish.CellBuffer]::Parse(
                [string[]](@('header') + $badRows), 2, 2, 1, 'invalid-cell-contract') | Out-Null
        }
        catch { $rejected = $true }
        if (-not $rejected) { throw 'Navigation dense cell parser accepted an invalid source' }
    }
    foreach ($case in @(
        @{ Cells=@(1,1,0,0,1,1); Error=$null; Count=4 },
        @{ Cells=@(0,0,1,1,0,0); Error='disconnected'; Count=2 },
        @{ Cells=@(0,0,0,0,0,0); Error='no walkable'; Count=0 }
    )) {
        $bytes = [LostArk.NavigationPublish.CellBuffer]::Serialize(
            3, 2, 1, 0, 0, [byte[]]$case.Cells, [byte[]]$case.Cells,
            [single[]]@(0,0.5,0,0,1,1.5), [byte[]]::new(6))
        $failure = $null
        try {
            $analysis = [LostArk.NavigationPublish.CellBuffer]::Analyze($bytes, 3, 2, $true, 'component-contract')
            if ($analysis[0] -ne $case.Count -or $analysis[1] -ne 0.5) {
                throw 'Navigation dense grid statistics differ'
            }
        }
        catch { $failure = $_.Exception.Message }
        if (($null -eq $case.Error -and $null -ne $failure) -or
            ($null -ne $case.Error -and ($null -eq $failure -or $failure -notlike "*$($case.Error)*"))) {
            throw "Navigation dense grid component contract failed: $failure"
        }
    }
}

function New-NavigationLayerTestGrid {
    param(
        [string]$AreaId,
        [double]$Height,
        [double]$Step,
        [uint32]$Width = 4,
        [uint32]$Depth = 4,
        [double]$OriginX = 1.0,
        [double]$OriginZ = 1.0,
        [int]$HoleCell = -1
    )
    $count = [int]($Width * $Depth)
    $ground = [byte[]]::new($count)
    for ($index = 0; $index -lt $count; ++$index) { $ground[$index] = $(if ($index -eq $HoleCell) { 0 } else { 1 }) }
    $levels = [single[]]::new($count)
    for ($index = 0; $index -lt $count; ++$index) { $levels[$index] = [single]$Height }
    $bytes = [LostArk.NavigationPublish.CellBuffer]::Serialize(
        $Width, $Depth, 0.5, [single]$OriginX, [single]$OriginZ,
        [byte[]]$ground, [byte[]]$ground, [single[]]$levels, [byte[]]::new($count))
    return [pscustomobject]@{
        AreaId = $AreaId; Bytes = [byte[]]$bytes; RuntimeMaximumStepHeight = [single]$Step }
}

function Invoke-NavigationLayerContractTest {
    $accepts = {
        param($UpperHeight, $LowerHeight, $Step)
        try {
            Assert-NavigationRegionFootprints -RegionGrids @(
                (New-NavigationLayerTestGrid 'T.upper' $UpperHeight $Step),
                (New-NavigationLayerTestGrid 'T.lower' $LowerHeight $Step))
            return $true
        } catch { return $false }
    }
    if (-not (& $accepts 10.0 2.0 0.75)) { throw 'Stacked layers 8 m apart were rejected' }
    if (& $accepts 3.0 2.0 0.75) { throw 'Stacked layers 1 m apart were accepted' }
    if (& $accepts 3.9 2.0 0.75) { throw 'Stacked layers 1.9 m apart were accepted' }
    if (-not (& $accepts 4.0 2.0 0.75)) { throw 'Stacked layers exactly 2 m apart were rejected' }
    if (& $accepts 4.5 2.0 1.5) { throw 'Stacked layers closer than twice a 1.5 m step were accepted' }
    if (-not (& $accepts 5.0 2.0 1.5)) { throw 'Stacked layers exactly twice a 1.5 m step apart were rejected' }
    if (& $accepts 2.0 2.0 0.75) { throw 'Two same-height regions on one footprint were accepted' }
    $message = $null
    try {
        Assert-NavigationRegionFootprints -RegionGrids @(
            (New-NavigationLayerTestGrid 'T.upper' 3.0 0.75),
            (New-NavigationLayerTestGrid 'T.lower' 2.0 0.75))
    } catch { $message = $_.Exception.Message }
    if ($message -notlike '*T.upper and T.lower*' -or $message -notlike '*closer than 2 m*') {
        throw "Ambiguous layer rejection does not name the pair and the rule: $message"
    }
    # Footprints that only touch or do not overlap never need a separation.
    Assert-NavigationRegionFootprints -RegionGrids @(
        (New-NavigationLayerTestGrid 'T.left' 2.0 0.75),
        (New-NavigationLayerTestGrid 'T.right' 2.0 0.75 -OriginX 3.0))
    Assert-NavigationRegionFootprints -RegionGrids @(
        (New-NavigationLayerTestGrid 'T.near' 2.0 0.75),
        (New-NavigationLayerTestGrid 'T.far' 2.0 0.75 -OriginX 20.0 -OriginZ 20.0))

    $base = New-NavigationLayerTestGrid 'T' 0.0 1.0 -Width 4 -Depth 4 -OriginX 0.0 -OriginZ 0.0
    $upper = New-NavigationLayerTestGrid 'T.upper' 10.0 0.75
    $lower = New-NavigationLayerTestGrid 'T.lower' 2.0 0.75
    $pick = {
        param($Regions, $Y, $X = 1.25, $Z = 1.25)
        return (Select-NavigationOwnerGrid -BaseGrid $base -RegionGrids $Regions -X $X -Y $Y -Z $Z).AreaId
    }
    if ((& $pick @($upper, $lower) 10.3) -ne 'T.upper' -or (& $pick @($upper, $lower) 1.7) -ne 'T.lower') {
        throw 'A stacked placement was not assigned to the layer nearest its height'
    }
    if ((& $pick @($upper, $lower) ([double]::NaN)) -ne 'T.upper' -or
        (& $pick @($lower, $upper) ([double]::NaN)) -ne 'T.lower') {
        throw 'Manifest order is not the tie-break of an unhinted stacked placement'
    }
    if ((& $pick @($upper, $lower) 10.3 0.25 0.25) -ne 'T') { throw 'A point outside every region did not fall back to the base grid' }
    $upperWithHole = New-NavigationLayerTestGrid 'T.upper' 10.0 0.75 -HoleCell 0
    if ((& $pick @($upperWithHole, $lower) 10.3) -ne 'T.lower' -or
        (& $pick @($upperWithHole, $lower) 10.3 1.75 1.25) -ne 'T.upper') {
        throw 'Ground under the XZ did not beat a nearer layer without ground'
    }
}

if ($Mode -eq 'ContractTest') {
    Invoke-NavigationCellContractTest
    Invoke-NavigationPaintContractTest
    Invoke-NavigationLayerContractTest
    Write-Host 'Server navigation ContractTest succeeded: source cells, connectivity, navpaint, runtime blockers, placement height acceptance/rejection cases and stacked layer overlap acceptance/rejection.'
    return
}

$gridFactories = [ordered]@{
    'LV_LUT_HEARTRB_ED' = { Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_LUT_HEARTRB_ED.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_LUT_HEARTRB_ED.navpaint' }
    # The recovered Kakul geometry contains five intentionally disconnected
    # source-level islands.  Do not require a single component or infer Mario
    # semantics; StageMarkers names the exact source-level identities instead.
    'LV_LUT_MIDNIGHTC_ED' = { Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_LUT_MIDNIGHTC_ED.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_LUT_MIDNIGHTC_ED.navpaint' `
        -RuntimeMaximumStepHeight 1.0 }
    'LV_DEV_TRAINING_GROUND' = { New-UniformNavigationGrid `
		-RelativeAuthoringPath 'Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json' `
        -RuntimeMaximumStepHeight 0.6 }
    'LV_LOBBY_CLASSSELECT_SL00' = { Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint' `
        -MaximumStepHeight 0.6 `
        -RuntimeMaximumStepHeight 0.6 `
        -RequireSingleComponent }
    # Bern's single-height XZ bake sees bridges, terraces and archways above the
    # ground walkway. Version 3 paint corrects the three admitted corridor ridges
    # to their ground-deck heights. The runtime 1 m edge guard then separates the
    # remaining overlapping decks while preserving the authored staircase.
    'LV_BER_BERNCASTLE' = { Convert-NavigationAuthoringGrid `
        -RelativeSourcePath 'Data/Navigation/LV_BER_BERNCASTLE.navsource' `
        -RelativePaintPath 'Data/Navigation/LV_BER_BERNCASTLE.navpaint' `
        -RuntimeMaximumStepHeight 1.0 }
    # Maharaka starts on a flat grid at the plaza floor (20.48 m, the height of
    # most source spots) until the island gets a MapTool bake.
    'LV_OCN_EVENTIS_MHP' = { New-UniformNavigationGrid `
        -RelativeAuthoringPath 'Data/Navigation/LV_OCN_EVENTIS_MHP.navgrid.json' `
        -RuntimeMaximumStepHeight 0.6 }
}
$grids = @(foreach ($factory in $gridFactories.GetEnumerator()) {
    if (-not $AreaId -or $factory.Key -eq $AreaId) { & $factory.Value }
})

$validated = foreach ($grid in $grids) {
    $regionGrids = @(Convert-NavigationRegionGrids -AreaId $grid.AreaId)
    Assert-NavigationRegionFootprints -RegionGrids $regionGrids
    $detail = Assert-NavigationGrid $grid
    $regionEntries = foreach ($regionGrid in $regionGrids) {
        [pscustomobject]@{
            Grid = $regionGrid
            Detail = Assert-NavigationGrid $regionGrid
            BlockerLines = @(Convert-NavigationRuntimeBlockers `
                -AreaId $regionGrid.AreaId -GridBytes ([byte[]]$regionGrid.Bytes))
            PolicyLines = @(Convert-NavigationRuntimePolicy -Grid $regionGrid)
        }
    }
    Assert-NavigationPlacements -BaseGrid $grid -RegionGrids $regionGrids
    [pscustomobject]@{
        Grid = $grid
        Detail = $detail
        BlockerLines = @(Convert-NavigationRuntimeBlockers `
            -AreaId $grid.AreaId -GridBytes ([byte[]]$grid.Bytes))
        PolicyLines = @(Convert-NavigationRuntimePolicy -Grid $grid)
        Regions = @($regionEntries)
        ManifestLines = $(if ($regionGrids.Count -gt 0) {
            @(Convert-NavigationRuntimeRegionManifest `
                -AreaId $grid.AreaId -RegionGrids $regionGrids)
        } else { @() })
    }
}

if ($Mode -eq 'Publish') {
    $root = if ([IO.Path]::IsPathRooted($OutputRoot)) {
        [IO.Path]::GetFullPath($OutputRoot)
    } else { [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot)) }
    [IO.Directory]::CreateDirectory($root) | Out-Null
    $clientRoot = if ([IO.Path]::IsPathRooted($ClientOutputRoot)) {
        [IO.Path]::GetFullPath($ClientOutputRoot)
    } else { [IO.Path]::GetFullPath((Join-Path $repoRoot $ClientOutputRoot)) }
    foreach ($entry in $validated) {
        [IO.Directory]::CreateDirectory($clientRoot) | Out-Null
        $token = [Guid]::NewGuid().ToString('N')
        $targets = [Collections.Generic.List[object]]::new()
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navgrid"); Bytes=[byte[]]$entry.Grid.Bytes })
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navblockers"); Lines=[string[]]$entry.BlockerLines })
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navpolicy"); Lines=[string[]]$entry.PolicyLines })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navgrid"); Bytes=[byte[]]$entry.Grid.Bytes })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navblockers"); Lines=[string[]]$entry.BlockerLines })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navpolicy"); Lines=[string[]]$entry.PolicyLines })
        foreach ($regionEntry in @($entry.Regions)) {
            $regionGridId = $regionEntry.Grid.AreaId
            $targets.Add(@{ Destination=(Join-Path $root "$regionGridId.navgrid"); Bytes=[byte[]]$regionEntry.Grid.Bytes })
            $targets.Add(@{ Destination=(Join-Path $root "$regionGridId.navblockers"); Lines=[string[]]$regionEntry.BlockerLines })
            $targets.Add(@{ Destination=(Join-Path $root "$regionGridId.navpolicy"); Lines=[string[]]$regionEntry.PolicyLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionGridId.navgrid"); Bytes=[byte[]]$regionEntry.Grid.Bytes })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionGridId.navblockers"); Lines=[string[]]$regionEntry.BlockerLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionGridId.navpolicy"); Lines=[string[]]$regionEntry.PolicyLines })
        }
        if (@($entry.ManifestLines).Count -gt 0) {
            $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navregions"); Lines=[string[]]$entry.ManifestLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navregions"); Lines=[string[]]$entry.ManifestLines })
        }
        else {
            # An Area published without regions must not keep the manifest of an
            # earlier publish, or the loaders would still dispatch into those
            # region grids. Removing it is part of this transaction and rolls back.
            foreach ($manifestRoot in @($root, $clientRoot)) {
                $staleManifest = Join-Path $manifestRoot "$($entry.Grid.AreaId).navregions"
                if ([IO.File]::Exists($staleManifest)) {
                    $targets.Add(@{ Destination=$staleManifest; Delete=$true })
                }
            }
        }
        $promotions = [Collections.Generic.List[object]]::new()
        try {
            foreach ($target in $targets) {
                if ($target.Delete) {
                    $promotions.Add([pscustomobject]@{
                        Staged=$null; Destination=$target.Destination; Delete=$true;
                        Rollback="$($target.Destination).rollback.$token"; HadPrevious=$false; Promoted=$false })
                    continue
                }
                $destinationRoot = [IO.Path]::GetDirectoryName($target.Destination)
                $staged = Join-Path $destinationRoot ".$([IO.Path]::GetFileName($target.Destination)).staging.$token"
                if ($null -ne $target.Bytes) {
                    [IO.File]::WriteAllBytes($staged, [byte[]]$target.Bytes)
                } else {
                    [IO.File]::WriteAllLines($staged, [string[]]$target.Lines, [Text.UTF8Encoding]::new($false))
                }
                $promotions.Add([pscustomobject]@{
                    Staged=$staged; Destination=$target.Destination; Delete=$false;
                    Rollback="$($target.Destination).rollback.$token"; HadPrevious=$false; Promoted=$false })
            }
            foreach ($promotion in $promotions) {
                if ([IO.File]::Exists($promotion.Destination)) {
                    [IO.File]::Move($promotion.Destination, $promotion.Rollback)
                    $promotion.HadPrevious = $true
                }
                if (-not $promotion.Delete) {
                    [IO.File]::Move($promotion.Staged, $promotion.Destination)
                }
                $promotion.Promoted = $true
                if ($promotion.Delete) {
                    Write-Host "Removed stale navigation region manifest: $($promotion.Destination)"
                }
            }
            foreach ($promotion in $promotions) {
                if ([IO.File]::Exists($promotion.Rollback)) { [IO.File]::Delete($promotion.Rollback) }
            }
        }
        catch {
            for ($index = $promotions.Count - 1; $index -ge 0; --$index) {
                $promotion = $promotions[$index]
                if ($promotion.Promoted -and [IO.File]::Exists($promotion.Destination)) {
                    [IO.File]::Delete($promotion.Destination)
                }
                if ($promotion.HadPrevious -and [IO.File]::Exists($promotion.Rollback)) {
                    [IO.File]::Move($promotion.Rollback, $promotion.Destination)
                }
                if ($null -ne $promotion.Staged -and [IO.File]::Exists($promotion.Staged)) { [IO.File]::Delete($promotion.Staged) }
            }
            throw
        }
    }
}

foreach ($entry in $validated) {
    Write-Host "Server navigation $Mode succeeded: $($entry.Grid.AreaId) $($entry.Detail)."
    foreach ($regionEntry in @($entry.Regions)) {
        Write-Host "  region: $($regionEntry.Grid.AreaId) $($regionEntry.Detail)."
    }
}
