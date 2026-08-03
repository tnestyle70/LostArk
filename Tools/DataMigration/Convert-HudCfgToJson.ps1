param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

function Convert-ResourcePath {
    param([string]$Value)

    $normalized = $Value.Replace('\', '/')
    foreach ($prefix in @(
        '../Bin/Resources/LostArk/',
        'Bin/Resources/LostArk/',
        'Client/Bin/Resources/LostArk/',
        '../Bin/Resources/',
        'Bin/Resources/',
        'Client/Bin/Resources/')) {
        if ($normalized.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
            return $normalized.Substring($prefix.Length)
        }
    }
    return $normalized
}

function New-Layer {
    param([string]$Path)
    return [ordered]@{
        path = Convert-ResourcePath $Path
        hoverPath = $null
        tint = @(1.0, 1.0, 1.0, 1.0)
        additive = $false
        flipX = $false
    }
}

$resolvedInput = [IO.Path]::GetFullPath($InputPath)
$resolvedOutput = [IO.Path]::GetFullPath($OutputPath)
if (-not [IO.File]::Exists($resolvedInput)) {
    throw "HUD cfg does not exist: $resolvedInput"
}

$classes = [Collections.Generic.List[string]]::new()
$slots = [Collections.Generic.List[object]]::new()
$resolutionWidth = 1280.0
$resolutionHeight = 720.0
$currentSlot = $null

foreach ($rawLine in [IO.File]::ReadAllLines($resolvedInput)) {
    $line = $rawLine.Trim()
    if ([string]::IsNullOrWhiteSpace($line)) {
        continue
    }

    $parts = $line -split '\s+'
    $tag = $parts[0]
    switch ($tag) {
        'RESOLUTION' {
            $resolutionWidth = [double]::Parse($parts[1], [Globalization.CultureInfo]::InvariantCulture)
            $resolutionHeight = [double]::Parse($parts[2], [Globalization.CultureInfo]::InvariantCulture)
        }
        'CLASS' {
            $classes.Add($parts[1])
        }
        'SLOT' {
            if ($parts.Count -lt 14) {
                throw "Invalid SLOT row: $line"
            }
            $owner = if ($parts[6] -eq '-') { $null } else { $parts[6] }
            $currentSlot = [ordered]@{
                id = $parts[1]
                ownerClass = $owner
                type = [int]$parts[7]
                rect = [ordered]@{
                    x = [double]::Parse($parts[2], [Globalization.CultureInfo]::InvariantCulture)
                    y = [double]::Parse($parts[3], [Globalization.CultureInfo]::InvariantCulture)
                    width = [double]::Parse($parts[4], [Globalization.CultureInfo]::InvariantCulture)
                    height = [double]::Parse($parts[5], [Globalization.CultureInfo]::InvariantCulture)
                }
                rotation = [double]::Parse($parts[8], [Globalization.CultureInfo]::InvariantCulture)
                stages = [ordered]@{
                    baseFrom = [int]$parts[9]
                    shineFrom = [int]$parts[13]
                }
                layers = [Collections.Generic.List[object]]::new()
                shine = [ordered]@{
                    texture = $null
                    additive = $false
                }
                animation = [ordered]@{
                    fps = 10.0
                    scale = [double]::Parse($parts[10], [Globalization.CultureInfo]::InvariantCulture)
                    offset = [ordered]@{
                        x = [double]::Parse($parts[11], [Globalization.CultureInfo]::InvariantCulture)
                        y = [double]::Parse($parts[12], [Globalization.CultureInfo]::InvariantCulture)
                    }
                    frames = [Collections.Generic.List[string]]::new()
                }
            }
            $slots.Add($currentSlot)
        }
        'TEXTURE' {
            if ($null -eq $currentSlot) { throw "TEXTURE before SLOT: $line" }
            $path = $line.Substring(($parts[0] + ' ' + $parts[1] + ' ').Length)
            $currentSlot.layers.Add((New-Layer $path))
        }
        'LAYERHOVER' {
            $layerIndex = [int]$parts[2]
            $path = $line.Substring(($parts[0] + ' ' + $parts[1] + ' ' + $parts[2] + ' ').Length)
            $currentSlot.layers[$layerIndex].hoverPath = Convert-ResourcePath $path
        }
        'LAYERTINT' {
            $layerIndex = [int]$parts[2]
            $currentSlot.layers[$layerIndex].tint = @(
                [double]::Parse($parts[3], [Globalization.CultureInfo]::InvariantCulture),
                [double]::Parse($parts[4], [Globalization.CultureInfo]::InvariantCulture),
                [double]::Parse($parts[5], [Globalization.CultureInfo]::InvariantCulture),
                [double]::Parse($parts[6], [Globalization.CultureInfo]::InvariantCulture))
        }
        'LAYERADDITIVE' {
            $currentSlot.layers[[int]$parts[2]].additive = $true
        }
        'LAYERFLIPX' {
            $currentSlot.layers[[int]$parts[2]].flipX = $true
        }
        'SHINETEX' {
            $path = $line.Substring(($parts[0] + ' ' + $parts[1] + ' ').Length)
            $currentSlot.shine.texture = Convert-ResourcePath $path
        }
        'SHINEADDITIVE' {
            $currentSlot.shine.additive = $true
        }
        'ANIMFPS' {
            $currentSlot.animation.fps = [double]::Parse($parts[2], [Globalization.CultureInfo]::InvariantCulture)
        }
        'ANIMFRAME' {
            $path = $line.Substring(($parts[0] + ' ' + $parts[1] + ' ').Length)
            $currentSlot.animation.frames.Add((Convert-ResourcePath $path))
        }
        default {
            throw "Unknown HUD cfg tag '$tag': $line"
        }
    }
}

if ($classes.Count -eq 0) {
    $classes.Add('Default')
}
if ($slots.Count -eq 0) {
    throw 'HUD cfg contains no slots.'
}

$document = [ordered]@{
    schema = 'lostark.ui-layout'
    formatVersion = 1
    resolution = [ordered]@{
        width = $resolutionWidth
        height = $resolutionHeight
    }
    classes = $classes
    slots = $slots
}

$outputDirectory = [IO.Path]::GetDirectoryName($resolvedOutput)
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
$json = $document | ConvertTo-Json -Depth 12
[IO.File]::WriteAllText(
    $resolvedOutput,
    $json + [Environment]::NewLine,
    [Text.UTF8Encoding]::new($false))

Write-Output "Converted $resolvedInput -> $resolvedOutput ($($slots.Count) slots)"
