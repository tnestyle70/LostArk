#Requires -Version 5.1

<#
.SYNOPSIS
Builds a validated staging WModel from ActorX PSK/PSA files.

.DESCRIPTION
Runs Blender headlessly to combine one PSK and one or more PSA files into an
FBX, then runs the repository ModelAssetConverter. The resulting FBX, JSON
report, WModel, converter info and copied textures are committed together as a
staging package only after converter validation succeeds.

The script refuses Client/Bin/Resources as a staging target. Existing staging
packages are preserved unless -OverwriteStagingOutput is explicitly supplied;
even then the old package is renamed to a recoverable backup.

.EXAMPLE
.\Cook-ActorXWModel.ps1 `
  -PskPath 'C:\LostArkExtract\ITR_02326\itr_02326_sk.psk' `
  -PsaPath 'C:\LostArkExtract\ITR_02326\itr_02326_ani.psa' `
  -StagingDirectory 'C:\LostArkExtract\ITR_02326_Cook' `
  -AssetName 'DEPLOY_ITR_02326_INTACT' `
  -TextureRoot 'C:\LostArkExtract\ITR_02326\Textures' `
  -RequiredActionName off,on,spawn,hit1_1 `
  -ExpectedAnimationCount 4 `
  -KeepBlend
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PskPath,

    [Parameter(Mandatory = $true)]
    [string[]]$PsaPath,

    [Parameter(Mandatory = $true)]
    [string]$StagingDirectory,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')]
    [string]$AssetName,

    [string]$TextureRoot,
    [string]$BlenderPath,
    [string]$AddonPath,
    [string]$ConverterPath,
    [string]$PreferredAction,
    [string[]]$RequiredActionName = @(),
    [string[]]$MaterialRemap = @(),
    [string[]]$NormalRemap = @(),
    [string[]]$SpecularRemap = @(),
    [string[]]$EmissiveRemap = @(),
    [string[]]$OpacityRemap = @(),
    [string[]]$OrmRemap = @(),
    [string[]]$MetallicRemap = @(),
    [string[]]$RoughnessRemap = @(),
    [string[]]$AoRemap = @(),

    [ValidateRange(-1, 100000)]
    [int]$ExpectedAnimationCount = -1,

    [switch]$PrefixActionsWithSource,
    [switch]$KeepBlend,
    [switch]$OverwriteStagingOutput
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-ExistingFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Label
    )

    try {
        $resolved = (Resolve-Path -LiteralPath $Path -ErrorAction Stop).Path
    }
    catch {
        throw "$Label file was not found: $Path"
    }
    if (-not (Test-Path -LiteralPath $resolved -PathType Leaf)) {
        throw "$Label path is not a file: $resolved"
    }
    return [IO.Path]::GetFullPath($resolved)
}

function Resolve-ExistingDirectory {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Label
    )

    try {
        $resolved = (Resolve-Path -LiteralPath $Path -ErrorAction Stop).Path
    }
    catch {
        throw "$Label directory was not found: $Path"
    }
    if (-not (Test-Path -LiteralPath $resolved -PathType Container)) {
        throw "$Label path is not a directory: $resolved"
    }
    return [IO.Path]::GetFullPath($resolved)
}

function Resolve-TextureRemap {
    param(
        [AllowEmptyCollection()][string[]]$Value = @(),
        [Parameter(Mandatory = $true)][string]$Label
    )

    $resolvedMappings = New-Object 'System.Collections.Generic.List[string]'
    foreach ($mapping in $Value) {
        $separator = $mapping.IndexOf('=')
        if ($separator -le 0 -or $separator -eq ($mapping.Length - 1)) {
            throw "$Label must use material-name=texture-path: $mapping"
        }

        $materialName = $mapping.Substring(0, $separator).Trim()
        $texturePath = $mapping.Substring($separator + 1).Trim()
        if ($materialName.Length -eq 0 -or $texturePath.Length -eq 0) {
            throw "$Label must use material-name=texture-path: $mapping"
        }

        $resolvedTexture = Resolve-ExistingFile -Path $texturePath -Label "$Label texture"
        $resolvedMappings.Add("$materialName=$resolvedTexture")
    }
    return @($resolvedMappings)
}

function Resolve-BlenderExecutable {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        return Resolve-ExistingFile -Path $ExplicitPath -Label 'Blender'
    }
    if ($env:BLENDER_EXE) {
        return Resolve-ExistingFile -Path $env:BLENDER_EXE -Label 'BLENDER_EXE'
    }

    $command = Get-Command blender.exe -CommandType Application -ErrorAction SilentlyContinue
    if ($command) {
        return [IO.Path]::GetFullPath($command.Source)
    }

    $candidates = New-Object 'System.Collections.Generic.List[string]'
    if ($env:ProgramFiles) {
        $preferred = Join-Path $env:ProgramFiles 'Blender Foundation\Blender 3.0\blender.exe'
        $candidates.Add($preferred)
        $root = Join-Path $env:ProgramFiles 'Blender Foundation'
        if (Test-Path -LiteralPath $root -PathType Container) {
            Get-ChildItem -LiteralPath $root -Directory -ErrorAction SilentlyContinue |
                Sort-Object Name -Descending |
                ForEach-Object { $candidates.Add((Join-Path $_.FullName 'blender.exe')) }
        }
    }

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [IO.Path]::GetFullPath($candidate)
        }
    }
    throw 'Blender was not found. Pass -BlenderPath or set BLENDER_EXE.'
}

function Resolve-ActorXAddon {
    param([string]$ExplicitPath)

    $candidates = New-Object 'System.Collections.Generic.List[string]'
    if ($ExplicitPath) {
        $candidates.Add($ExplicitPath)
    }
    if ($env:ACTORX_BLENDER_ADDON) {
        $candidates.Add($env:ACTORX_BLENDER_ADDON)
    }

    if ($env:APPDATA) {
        $blenderRoot = Join-Path $env:APPDATA 'Blender Foundation\Blender'
        if (Test-Path -LiteralPath $blenderRoot -PathType Container) {
            $allVersions = @(Get-ChildItem -LiteralPath $blenderRoot -Directory -ErrorAction SilentlyContinue)
            $versions = @($allVersions | Where-Object { $_.Name -eq '3.0' })
            $versions += @($allVersions | Where-Object { $_.Name -ne '3.0' } | Sort-Object Name -Descending)
            foreach ($version in $versions) {
                $addonRoot = Join-Path $version.FullName 'scripts\addons'
                $candidates.Add((Join-Path $addonRoot 'io_import_scene_unreal_psa_psk_280.py'))
                $candidates.Add((Join-Path $addonRoot 'io_scene_psk_psa\__init__.py'))
            }
        }
    }

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [IO.Path]::GetFullPath((Resolve-Path -LiteralPath $candidate).Path)
        }
        if (Test-Path -LiteralPath $candidate -PathType Container) {
            foreach ($child in @('io_import_scene_unreal_psa_psk_280.py', '__init__.py')) {
                $childPath = Join-Path $candidate $child
                if (Test-Path -LiteralPath $childPath -PathType Leaf) {
                    return [IO.Path]::GetFullPath((Resolve-Path -LiteralPath $childPath).Path)
                }
            }
        }
    }
    throw 'The PSK/PSA Blender importer was not found. Pass -AddonPath or set ACTORX_BLENDER_ADDON.'
}

function Test-PathInside {
    param(
        [Parameter(Mandatory = $true)][string]$Candidate,
        [Parameter(Mandatory = $true)][string]$Root
    )

    $candidateFull = [IO.Path]::GetFullPath($Candidate).TrimEnd('\')
    $rootFull = [IO.Path]::GetFullPath($Root).TrimEnd('\')
    return $candidateFull.Equals($rootFull, [StringComparison]::OrdinalIgnoreCase) -or
        $candidateFull.StartsWith($rootFull + '\', [StringComparison]::OrdinalIgnoreCase)
}

function Assert-NoReparsePoint {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Label
    )

    $current = [IO.Path]::GetFullPath($Path).TrimEnd('\')
    while (-not (Test-Path -LiteralPath $current)) {
        $parent = [IO.Path]::GetDirectoryName($current)
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $current) {
            throw "$Label has no existing ancestor: $Path"
        }
        $current = $parent.TrimEnd('\')
    }
    while (-not [string]::IsNullOrWhiteSpace($current)) {
        $item = Get-Item -Force -LiteralPath $current
        if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
            throw "$Label traverses a junction or symbolic link: $($item.FullName)"
        }
        $parent = [IO.Path]::GetDirectoryName($current)
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $current) {
            break
        }
        $current = $parent.TrimEnd('\')
    }
}

function Assert-ReplaceableActorXPackage {
    param(
        [Parameter(Mandatory = $true)][string]$Package,
        [Parameter(Mandatory = $true)][string]$Asset
    )

    Assert-NoReparsePoint -Path $Package -Label 'Existing staging package'
    $marker = Join-Path $Package "$Asset.actorx.json"
    if (-not (Test-Path -LiteralPath $marker -PathType Leaf)) {
        throw "Refusing to replace an unmarked directory: $Package"
    }
    try {
        $existingReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $marker | ConvertFrom-Json
    }
    catch {
        throw "Refusing to replace a staging package with an unreadable marker: $marker"
    }
    if ($existingReport.schema -ne 'LOSTARK_ACTORX_FBX_REPORT' -or
        [int]$existingReport.version -ne 2 -or
        [string]::IsNullOrWhiteSpace([string]$existingReport.output_fbx)) {
        throw "Refusing to replace a staging package with an unknown marker: $marker"
    }
    $reportedFbx = [IO.Path]::GetFullPath([string]$existingReport.output_fbx)
    $expectedFbx = [IO.Path]::GetFullPath((Join-Path $Package "$Asset.fbx"))
    if (-not $reportedFbx.Equals($expectedFbx, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to replace a staging package whose marker belongs to another asset: $marker"
    }
}

function Invoke-NativeLogged {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [Parameter(Mandatory = $true)][string]$WorkingDirectory
    )

    $lines = New-Object 'System.Collections.Generic.List[string]'
    Push-Location -LiteralPath $WorkingDirectory
    try {
        $savedErrorActionPreference = $ErrorActionPreference
        $ErrorActionPreference = 'Continue'
        try {
            & $FilePath @ArgumentList 2>&1 | ForEach-Object {
                $line = $_.ToString()
                $lines.Add($line)
                Write-Host $line
            }
            $exitCode = $LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $savedErrorActionPreference
        }
    }
    finally {
        Pop-Location
    }

    return [PSCustomObject]@{
        ExitCode = $exitCode
        Lines = @($lines)
        Text = ($lines -join [Environment]::NewLine)
    }
}

function Read-ExactBytes {
    param(
        [Parameter(Mandatory = $true)][IO.BinaryReader]$Reader,
        [Parameter(Mandatory = $true)][int]$Count,
        [Parameter(Mandatory = $true)][string]$Label
    )

    $bytes = $Reader.ReadBytes($Count)
    if ($bytes.Length -ne $Count) {
        throw "Truncated WModel while reading $Label."
    }
    return $bytes
}

function Read-FixedAscii {
    param(
        [Parameter(Mandatory = $true)][IO.BinaryReader]$Reader,
        [Parameter(Mandatory = $true)][int]$Count,
        [Parameter(Mandatory = $true)][string]$Label
    )

    $bytes = Read-ExactBytes -Reader $Reader -Count $Count -Label $Label
    $length = [Array]::IndexOf($bytes, [byte]0)
    if ($length -lt 0) {
        $length = $bytes.Length
    }
    return [Text.Encoding]::ASCII.GetString($bytes, 0, $length)
}

function Read-FixedUnicode {
    param(
        [Parameter(Mandatory = $true)][IO.BinaryReader]$Reader,
        [Parameter(Mandatory = $true)][int]$CharacterCount,
        [Parameter(Mandatory = $true)][string]$Label
    )

    $bytes = Read-ExactBytes -Reader $Reader -Count ($CharacterCount * 2) -Label $Label
    $value = [Text.Encoding]::Unicode.GetString($bytes)
    $terminator = $value.IndexOf([char]0)
    if ($terminator -ge 0) {
        $value = $value.Substring(0, $terminator)
    }
    return $value
}

function Test-FiniteDouble {
    param([double]$Value)
    return -not [double]::IsNaN($Value) -and -not [double]::IsInfinity($Value)
}

function Read-WModelMetadata {
    param([Parameter(Mandatory = $true)][string]$Path)

    $stream = [IO.File]::Open($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
    $reader = New-Object IO.BinaryReader($stream)
    try {
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'outer magic') -ne 'WINT') {
            throw 'WModel outer magic is not WINT.'
        }
        $major = $reader.ReadUInt16()
        $minor = $reader.ReadUInt16()
        $flags = $reader.ReadUInt32()
        $contentSize = $reader.ReadUInt32()
        if ($major -ne 1 -or $flags -ne 0 -or $contentSize -ne ($stream.Length - 16)) {
            throw "WModel outer header is invalid. version=$major.$minor flags=$flags content=$contentSize"
        }

        $contentStart = 16L
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'model magic') -ne 'WMOD') {
            throw 'WModel package magic is not WMOD.'
        }
        $sectionCount = $reader.ReadUInt32()
        $declaredAnimationCount = $reader.ReadUInt32()
        $modelFlags = $reader.ReadUInt32()
        $reader.BaseStream.Seek(16, [IO.SeekOrigin]::Current) | Out-Null
        if ($sectionCount -lt 2 -or $sectionCount -gt 4096) {
            throw "WModel package header is invalid. sections=$sectionCount flags=$modelFlags"
        }

        $sections = New-Object 'System.Collections.Generic.List[object]'
        for ($index = 0; $index -lt $sectionCount; ++$index) {
            $sections.Add([PSCustomObject]@{
                Type = $reader.ReadUInt32()
                Index = $reader.ReadUInt32()
                Offset = $reader.ReadUInt64()
                Size = $reader.ReadUInt64()
                Name = Read-FixedAscii -Reader $reader -Count 40 -Label 'section name'
            })
        }
        foreach ($section in $sections) {
            if ([uint64]$section.Offset -gt [uint64]$contentSize -or
                [uint64]$section.Size -gt ([uint64]$contentSize - [uint64]$section.Offset)) {
                throw "WModel section is outside the package: type=$($section.Type) name=$($section.Name)"
            }
        }

        $skeletonSections = @($sections | Where-Object { $_.Type -eq 3 })
        if ($skeletonSections.Count -ne 1) {
            throw "WModel must contain exactly one skeleton section; got $($skeletonSections.Count)."
        }
        $skeletonSection = $skeletonSections[0]
        $reader.BaseStream.Position = $contentStart + [int64]$skeletonSection.Offset
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'skeleton outer magic') -ne 'WINT') {
            throw 'Skeleton section outer magic is invalid.'
        }
        $embeddedMajor = $reader.ReadUInt16()
        $reader.ReadUInt16() | Out-Null
        $embeddedFlags = $reader.ReadUInt32()
        $embeddedContentSize = $reader.ReadUInt32()
        if ($embeddedMajor -ne 1 -or $embeddedFlags -ne 0 -or
            ([int64]$embeddedContentSize + 16L) -ne [int64]$skeletonSection.Size) {
            throw 'Skeleton embedded WINT header does not match its section.'
        }
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'skeleton magic') -ne 'WSKL') {
            throw 'Skeleton section magic is invalid.'
        }
        $boneCount = $reader.ReadUInt32()
        $socketCount = $reader.ReadUInt32()
        Read-ExactBytes -Reader $reader -Count 20 -Label 'skeleton reserved bytes' | Out-Null
        if ($boneCount -le 0 -or $boneCount -gt 4096 -or $socketCount -gt 4096) {
            throw "WModel skeleton counts are invalid. bones=$boneCount sockets=$socketCount"
        }
        $expectedSkeletonContent = 32L + 256L * [int64]$boneCount + 128L + 128L * [int64]$socketCount
        if ($expectedSkeletonContent -ne [int64]$embeddedContentSize) {
            throw "WModel skeleton payload size is invalid. expected=$expectedSkeletonContent actual=$embeddedContentSize"
        }

        $skeletonBones = New-Object 'System.Collections.Generic.List[object]'
        $skeletonHashes = @{}
        $skeletonNames = @{}
        for ($index = 0; $index -lt $boneCount; ++$index) {
            $boneHash = $reader.ReadUInt64()
            $boneName = Read-FixedAscii -Reader $reader -Count 64 -Label 'skeleton bone name'
            $parentIndex = $reader.ReadInt32()
            Read-ExactBytes -Reader $reader -Count 180 -Label 'skeleton bone payload' | Out-Null
            $hashKey = '{0:X16}' -f $boneHash
            if ([string]::IsNullOrWhiteSpace($boneName) -or
                $skeletonHashes.ContainsKey($hashKey) -or $skeletonNames.ContainsKey($boneName)) {
                throw "WModel skeleton contains an empty or duplicate bone: $boneName"
            }
            $bone = [PSCustomObject]@{
                Index = $index
                Hash = $boneHash
                HashKey = $hashKey
                Name = $boneName
                ParentIndex = $parentIndex
            }
            $skeletonBones.Add($bone)
            $skeletonHashes[$hashKey] = $bone
            $skeletonNames[$boneName] = $bone
        }

        $animationSections = @($sections | Where-Object { $_.Type -eq 4 })
        if ($animationSections.Count -ne $declaredAnimationCount) {
            throw "WModel animation table mismatch. declared=$declaredAnimationCount actual=$($animationSections.Count)"
        }

        $animations = New-Object 'System.Collections.Generic.List[object]'
        foreach ($section in $animationSections) {
            $reader.BaseStream.Position = $contentStart + [int64]$section.Offset
            if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'animation outer magic') -ne 'WINT') {
                throw "Animation section outer magic is invalid: $($section.Name)"
            }
            $embeddedMajor = $reader.ReadUInt16()
            $reader.ReadUInt16() | Out-Null
            $embeddedFlags = $reader.ReadUInt32()
            $embeddedContentSize = $reader.ReadUInt32()
            if ($embeddedMajor -ne 1 -or $embeddedFlags -ne 0 -or
                ([int64]$embeddedContentSize + 16L) -ne [int64]$section.Size) {
                throw "Animation embedded WINT header does not match its section: $($section.Name)"
            }
            if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'animation magic') -ne 'WANM') {
                throw "Animation section magic is invalid: $($section.Name)"
            }
            $channelCount = $reader.ReadUInt32()
            $durationTicks = [double]$reader.ReadSingle()
            $ticksPerSecond = [double]$reader.ReadSingle()
            $totalKeyCount = $reader.ReadUInt32()
            $eventCount = $reader.ReadUInt32()
            $isLoop = $reader.ReadByte()
            Read-ExactBytes -Reader $reader -Count 7 -Label 'animation reserved bytes' | Out-Null
            if ($channelCount -gt 65536 -or $eventCount -gt 65536 -or
                -not (Test-FiniteDouble $durationTicks) -or $durationTicks -le 0.0 -or
                -not (Test-FiniteDouble $ticksPerSecond) -or $ticksPerSecond -le 0.0) {
                throw "Animation metadata is invalid: $($section.Name)"
            }

            $channels = New-Object 'System.Collections.Generic.List[object]'
            $channelHashes = @{}
            $decodedKeyCount = 0L
            for ($channelIndex = 0; $channelIndex -lt $channelCount; ++$channelIndex) {
                $boneHash = $reader.ReadUInt64()
                $hashKey = '{0:X16}' -f $boneHash
                if (-not $skeletonHashes.ContainsKey($hashKey) -or $channelHashes.ContainsKey($hashKey)) {
                    throw "Animation contains an unresolved or duplicate bone channel: $($section.Name) hash=$hashKey"
                }
                $positionCount = $reader.ReadUInt32()
                $positionOffset = $reader.ReadUInt32()
                $rotationCount = $reader.ReadUInt32()
                $rotationOffset = $reader.ReadUInt32()
                $scaleCount = $reader.ReadUInt32()
                $scaleOffset = $reader.ReadUInt32()
                $cachedBoneIndex = $reader.ReadInt32()
                $reader.ReadUInt32() | Out-Null
                $bone = $skeletonHashes[$hashKey]
                if ($cachedBoneIndex -ge 0 -and
                    ($cachedBoneIndex -ge $skeletonBones.Count -or
                    $skeletonBones[$cachedBoneIndex].HashKey -cne $hashKey)) {
                    throw "Animation cached bone index is invalid: $($section.Name) bone=$($bone.Name)"
                }
                $decodedKeyCount += [int64]$positionCount + [int64]$rotationCount + [int64]$scaleCount
                $channel = [PSCustomObject]@{
                    BoneHash = $boneHash
                    BoneHashKey = $hashKey
                    BoneName = $bone.Name
                    PositionKeyCount = $positionCount
                    PositionOffset = $positionOffset
                    RotationKeyCount = $rotationCount
                    RotationOffset = $rotationOffset
                    ScaleKeyCount = $scaleCount
                    ScaleOffset = $scaleOffset
                    CachedBoneIndex = $cachedBoneIndex
                    ScaleMinimum = $null
                    ScaleMaximum = $null
                    NonUnitScaleComponentCount = 0
                }
                $channels.Add($channel)
                $channelHashes[$hashKey] = $true
            }
            if ($decodedKeyCount -ne [int64]$totalKeyCount) {
                throw "Animation total key count does not match its channels: $($section.Name)"
            }

            $fixedBytes = 32L + 40L * [int64]$channelCount + 32L * [int64]$eventCount + 8L
            if ($fixedBytes -gt [int64]$embeddedContentSize) {
                throw "Animation key/event/trailer layout is truncated: $($section.Name)"
            }
            $keyBlockSize = [int64]$embeddedContentSize - $fixedBytes
            $keyBlockStart = $reader.BaseStream.Position
            foreach ($channel in $channels) {
                foreach ($span in @(
                    @([int64]$channel.PositionOffset, [int64]$channel.PositionKeyCount, 16L, 'position'),
                    @([int64]$channel.RotationOffset, [int64]$channel.RotationKeyCount, 20L, 'rotation'),
                    @([int64]$channel.ScaleOffset, [int64]$channel.ScaleKeyCount, 16L, 'scale')
                )) {
                    $offset = [int64]$span[0]
                    $byteCount = [int64]$span[1] * [int64]$span[2]
                    if ($offset -gt $keyBlockSize -or $byteCount -gt ($keyBlockSize - $offset)) {
                        throw "Animation $($span[3]) key span is invalid: $($section.Name) bone=$($channel.BoneName)"
                    }
                }

                if ($channel.ScaleKeyCount -gt 0) {
                    $reader.BaseStream.Position = $keyBlockStart + [int64]$channel.ScaleOffset
                    $minimum = @([double]::PositiveInfinity, [double]::PositiveInfinity, [double]::PositiveInfinity)
                    $maximum = @([double]::NegativeInfinity, [double]::NegativeInfinity, [double]::NegativeInfinity)
                    $previousTime = -1.0
                    $nonUnit = 0
                    for ($keyIndex = 0; $keyIndex -lt $channel.ScaleKeyCount; ++$keyIndex) {
                        $time = [double]$reader.ReadSingle()
                        $values = @([double]$reader.ReadSingle(), [double]$reader.ReadSingle(), [double]$reader.ReadSingle())
                        if (-not (Test-FiniteDouble $time) -or $time -lt 0.0 -or
                            $time -gt ($durationTicks + 0.001) -or $time -lt $previousTime) {
                            throw "Animation scale key time is invalid: $($section.Name) bone=$($channel.BoneName)"
                        }
                        for ($axis = 0; $axis -lt 3; ++$axis) {
                            if (-not (Test-FiniteDouble $values[$axis])) {
                                throw "Animation scale key value is invalid: $($section.Name) bone=$($channel.BoneName)"
                            }
                            $minimum[$axis] = [Math]::Min($minimum[$axis], $values[$axis])
                            $maximum[$axis] = [Math]::Max($maximum[$axis], $values[$axis])
                            if ([Math]::Abs($values[$axis] - 1.0) -gt 0.00001) {
                                ++$nonUnit
                            }
                        }
                        $previousTime = $time
                    }
                    $channel.ScaleMinimum = $minimum
                    $channel.ScaleMaximum = $maximum
                    $channel.NonUnitScaleComponentCount = $nonUnit
                }
            }

            $animations.Add([PSCustomObject]@{
                Name = [string]$section.Name
                Index = [uint32]$section.Index
                ChannelCount = $channelCount
                DurationTicks = $durationTicks
                TicksPerSecond = $ticksPerSecond
                TotalKeyCount = $totalKeyCount
                EventCount = $eventCount
                IsLoop = ($isLoop -ne 0)
                Channels = @($channels | ForEach-Object { $_ })
            })
        }

        $materialSections = @($sections | Where-Object { $_.Type -eq 2 })
        if ($materialSections.Count -ne 1) {
            throw "WModel must contain exactly one material section; got $($materialSections.Count)."
        }
        $materialSection = $materialSections[0]
        $reader.BaseStream.Position = $contentStart + [int64]$materialSection.Offset
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'material outer magic') -ne 'WINT') {
            throw 'Material section outer magic is invalid.'
        }
        $embeddedMajor = $reader.ReadUInt16()
        $reader.ReadUInt16() | Out-Null
        $embeddedFlags = $reader.ReadUInt32()
        $embeddedContentSize = $reader.ReadUInt32()
        if ($embeddedMajor -ne 1 -or $embeddedFlags -ne 0 -or
            ([int64]$embeddedContentSize + 16L) -ne [int64]$materialSection.Size) {
            throw 'Material embedded WINT header does not match its section.'
        }
        if ((Read-FixedAscii -Reader $reader -Count 4 -Label 'material magic') -ne 'WMA2') {
            throw 'ActorX cook requires WMA2 material metadata.'
        }
        $materialCount = $reader.ReadUInt32()
        if ($materialCount -gt 4096 -or
            (8L + 4756L * [int64]$materialCount) -ne [int64]$embeddedContentSize) {
            throw "WModel material count or payload size is invalid: $materialCount"
        }
        $materials = New-Object 'System.Collections.Generic.List[object]'
        for ($index = 0; $index -lt $materialCount; ++$index) {
            $materialIndex = $reader.ReadUInt32()
            $reader.ReadUInt64() | Out-Null
            $name = Read-FixedAscii -Reader $reader -Count 64 -Label 'material name'
            $materials.Add([PSCustomObject]@{
                Index = $materialIndex
                Name = $name
                Base = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'base texture path'
                Normal = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'normal texture path'
                Specular = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'specular texture path'
                Emissive = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'emissive texture path'
                Opacity = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'opacity texture path'
                Orm = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'ORM texture path'
                Metallic = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'metallic texture path'
                Roughness = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'roughness texture path'
                AmbientOcclusion = Read-FixedUnicode -Reader $reader -CharacterCount 260 -Label 'AO texture path'
            })
        }

        return [PSCustomObject]@{
            Sections = @($sections | ForEach-Object { $_ })
            SkeletonBones = @($skeletonBones | ForEach-Object { $_ })
            Animations = @($animations | ForEach-Object { $_ })
            Materials = @($materials | ForEach-Object { $_ })
        }
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

function Assert-CookedTexturePath {
    param(
        [Parameter(Mandatory = $true)][string]$StoredPath,
        [Parameter(Mandatory = $true)][string]$PackageRoot,
        [Parameter(Mandatory = $true)][string]$Label
    )

    if ([IO.Path]::IsPathRooted($StoredPath)) {
        throw "$Label stores an absolute texture path: $StoredPath"
    }
    $candidate = [IO.Path]::GetFullPath((Join-Path $PackageRoot $StoredPath))
    if (-not (Test-PathInside -Candidate $candidate -Root $PackageRoot)) {
        throw "$Label escapes the staging package: $StoredPath"
    }
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf) -or
        (Get-Item -LiteralPath $candidate).Length -le 0) {
        throw "$Label texture is missing or empty: $candidate"
    }
    return $candidate
}

function Assert-RemapApplied {
    param(
        [Parameter(Mandatory = $true)][object[]]$Materials,
        [AllowEmptyCollection()][string[]]$Mappings = @(),
        [Parameter(Mandatory = $true)][string]$Property,
        [Parameter(Mandatory = $true)][string]$PackageRoot
    )

    foreach ($mapping in $Mappings) {
        $separator = $mapping.IndexOf('=')
        $materialName = $mapping.Substring(0, $separator)
        $sourcePath = $mapping.Substring($separator + 1)
        $matched = @($Materials | Where-Object { $_.Name -ceq $materialName })
        if ($matched.Count -ne 1) {
            throw "Remap material does not resolve exactly once. material=$materialName slot=$Property"
        }
        $storedPath = [string]$matched[0].$Property
        if ([string]::IsNullOrWhiteSpace($storedPath)) {
            throw "Required remap slot is empty. material=$materialName slot=$Property"
        }
        $cookedPath = Assert-CookedTexturePath -StoredPath $storedPath -PackageRoot $PackageRoot -Label "$materialName.$Property"
        $sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $sourcePath).Hash
        $cookedHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $cookedPath).Hash
        if ($sourceHash -cne $cookedHash) {
            throw "Cooked texture differs from the requested remap. material=$materialName slot=$Property"
        }
    }
}

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$runtimeResourceRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Client\Bin\Resources'))
$pythonScript = Resolve-ExistingFile -Path (Join-Path $PSScriptRoot 'build_actorx_fbx.py') -Label 'Blender script'
$psk = Resolve-ExistingFile -Path $PskPath -Label 'PSK'
$psaFiles = @($PsaPath | ForEach-Object { Resolve-ExistingFile -Path $_ -Label 'PSA' })
if ($psaFiles.Count -eq 0) {
    throw 'At least one PSA file is required.'
}
$requiredActionNames = @(
    $RequiredActionName |
        ForEach-Object { $_ -split ',' } |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_.Length -gt 0 }
)
$materialMappings = @(Resolve-TextureRemap -Value $MaterialRemap -Label 'MaterialRemap')
$normalMappings = @(Resolve-TextureRemap -Value $NormalRemap -Label 'NormalRemap')
$specularMappings = @(Resolve-TextureRemap -Value $SpecularRemap -Label 'SpecularRemap')
$emissiveMappings = @(Resolve-TextureRemap -Value $EmissiveRemap -Label 'EmissiveRemap')
$opacityMappings = @(Resolve-TextureRemap -Value $OpacityRemap -Label 'OpacityRemap')
$ormMappings = @(Resolve-TextureRemap -Value $OrmRemap -Label 'OrmRemap')
$metallicMappings = @(Resolve-TextureRemap -Value $MetallicRemap -Label 'MetallicRemap')
$roughnessMappings = @(Resolve-TextureRemap -Value $RoughnessRemap -Label 'RoughnessRemap')
$aoMappings = @(Resolve-TextureRemap -Value $AoRemap -Label 'AoRemap')

$blender = Resolve-BlenderExecutable -ExplicitPath $BlenderPath
$addon = Resolve-ActorXAddon -ExplicitPath $AddonPath
if (-not $ConverterPath) {
    $ConverterPath = Join-Path $repoRoot 'Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe'
}
$converter = Resolve-ExistingFile -Path $ConverterPath -Label 'ModelAssetConverter'

$textureDirectory = $null
if ($TextureRoot) {
    $textureDirectory = Resolve-ExistingDirectory -Path $TextureRoot -Label 'TextureRoot'
}

$stage = [IO.Path]::GetFullPath($StagingDirectory)
if (Test-PathInside -Candidate $stage -Root $runtimeResourceRoot) {
    throw "The runtime Resources directory cannot be used as staging: $stage"
}
if (Test-PathInside -Candidate $stage -Root $repoRoot) {
    throw "The repository cannot be used as ActorX staging: $stage"
}
if ($stage.TrimEnd('\') -eq [IO.Path]::GetPathRoot($stage).TrimEnd('\')) {
    throw "A drive root cannot be used as ActorX staging: $stage"
}
if (Test-Path -LiteralPath $stage) {
    if (-not (Test-Path -LiteralPath $stage -PathType Container)) {
        throw "StagingDirectory is not a directory: $stage"
    }
}
else {
    New-Item -ItemType Directory -Path $stage | Out-Null
}
Assert-NoReparsePoint -Path $stage -Label 'StagingDirectory'

$finalPackage = [IO.Path]::GetFullPath((Join-Path $stage $AssetName))
if (Test-Path -LiteralPath $finalPackage) {
    if (-not $OverwriteStagingOutput) {
        throw "The staging package already exists. Choose another path or explicitly pass -OverwriteStagingOutput: $finalPackage"
    }
    Assert-ReplaceableActorXPackage -Package $finalPackage -Asset $AssetName
}

$workName = '.{0}.actorx-work-{1}' -f $AssetName, ([Guid]::NewGuid().ToString('N'))
$workDirectory = Join-Path $stage $workName
New-Item -ItemType Directory -Path $workDirectory | Out-Null

$fbxName = "$AssetName.fbx"
$reportName = "$AssetName.actorx.json"
$blendName = "$AssetName.blend"
$wmodelName = "$AssetName.wmodel"
$infoName = "$AssetName.wmodel.info.txt"
$fbxOutput = Join-Path $workDirectory $fbxName
$reportOutput = Join-Path $workDirectory $reportName
$blendOutput = Join-Path $workDirectory $blendName
$wmodelOutput = Join-Path $workDirectory $wmodelName
$infoOutput = Join-Path $workDirectory $infoName

try {
    Write-Host "Blender: $blender"
    Write-Host "ActorX addon: $addon"
    Write-Host "Temporary staging package: $workDirectory"

    $blenderArguments = @(
        '--background',
        '--factory-startup',
        '--python', $pythonScript,
        '--',
        '--psk', $psk
    )
    foreach ($psa in $psaFiles) {
        $blenderArguments += @('--psa', $psa)
    }
    $blenderArguments += @(
        '--output-fbx', $fbxOutput,
        '--report', $reportOutput,
        '--addon', $addon
    )
    if ($KeepBlend) {
        $blenderArguments += @('--output-blend', $blendOutput)
    }
    if ($PreferredAction) {
        $blenderArguments += @('--preferred-action', $PreferredAction)
    }
    if ($PrefixActionsWithSource) {
        $blenderArguments += '--prefix-actions-with-source'
    }

    $blenderResult = Invoke-NativeLogged -FilePath $blender -ArgumentList $blenderArguments -WorkingDirectory $workDirectory
    if ($blenderResult.ExitCode -ne 0) {
        throw "Blender ActorX assembly failed. exit=$($blenderResult.ExitCode)"
    }
    if (-not (Test-Path -LiteralPath $fbxOutput -PathType Leaf)) {
        throw "Blender did not create the FBX: $fbxOutput"
    }
    if (-not (Test-Path -LiteralPath $reportOutput -PathType Leaf)) {
        throw "Blender did not create the JSON report: $reportOutput"
    }

    $report = Get-Content -Raw -Encoding UTF8 -LiteralPath $reportOutput | ConvertFrom-Json
    if ($report.schema -ne 'LOSTARK_ACTORX_FBX_REPORT' -or [int]$report.version -ne 2) {
        throw "Unknown ActorX report format: $reportOutput"
    }
    $sourceFps = [double]$report.settings.source_fps
    $effectiveFps = [double]$report.settings.effective_fps
    if ([double]::IsNaN($sourceFps) -or [double]::IsInfinity($sourceFps) -or
        $sourceFps -le 0.0 -or [Math]::Abs($sourceFps - $effectiveFps) -gt 0.0001) {
        throw "Blender FPS does not preserve the PSA rate. source=$sourceFps effective=$effectiveFps"
    }
    $actions = @($report.actions)
    $actionCount = $actions.Count
    if ($actionCount -le 0) {
        throw 'The Blender report contains no Actions.'
    }
    if ($ExpectedAnimationCount -ge 0 -and $actionCount -ne $ExpectedAnimationCount) {
        throw "Blender Action count mismatch. expected=$ExpectedAnimationCount actual=$actionCount"
    }

    $sourceBones = @($report.bones | ForEach-Object { [string]$_ })
    if ($sourceBones.Count -ne [int]$report.bone_count -or
        @($sourceBones | Select-Object -Unique).Count -ne $sourceBones.Count) {
        throw 'The Blender report contains an invalid or duplicate source skeleton.'
    }
    foreach ($action in $actions) {
        if ([bool]$action.source_has_scale_keys) {
            $expectedScaleCurves = 3 * $sourceBones.Count
            $expectedScaleKeyframes = $expectedScaleCurves * [int]$action.source_raw_frame_count
            if ([int]$action.scale_fcurve_count -ne $expectedScaleCurves -or
                [int]$action.scale_keyframe_count -ne $expectedScaleKeyframes -or
                @($action.source_scale_bones).Count -ne $sourceBones.Count) {
                throw "Blender Action did not preserve every PSA scale track: $($action.name)"
            }
        }
    }

    $actionNames = @($actions | ForEach-Object { [string]$_.name })
    foreach ($requiredName in $requiredActionNames) {
        if (-not ($actionNames -ccontains $requiredName)) {
            throw "Required Action was not found: $requiredName (actual: $($actionNames -join ', '))"
        }
    }

    # Run the converter from the work directory and use ASCII asset-relative
    # names. This also avoids the converter's known absolute Unicode path issue.
    $converterArguments = @($fbxName, '-o', $wmodelName)
    if ($textureDirectory) {
        $converterArguments += @('--texture-root', $textureDirectory)
    }
    foreach ($mapping in $materialMappings) {
        $converterArguments += @('--material-remap', $mapping)
    }
    foreach ($mapping in $normalMappings) {
        $converterArguments += @('--normal-remap', $mapping)
    }
    foreach ($mapping in $specularMappings) {
        $converterArguments += @('--specular-remap', $mapping)
    }
    foreach ($mapping in $emissiveMappings) {
        $converterArguments += @('--emissive-remap', $mapping)
    }
    foreach ($mapping in $opacityMappings) {
        $converterArguments += @('--opacity-remap', $mapping)
    }
    foreach ($mapping in $ormMappings) {
        $converterArguments += @('--orm-remap', $mapping)
    }
    foreach ($mapping in $metallicMappings) {
        $converterArguments += @('--metallic-remap', $mapping)
    }
    foreach ($mapping in $roughnessMappings) {
        $converterArguments += @('--roughness-remap', $mapping)
    }
    foreach ($mapping in $aoMappings) {
        $converterArguments += @('--ao-remap', $mapping)
    }
    if (($materialMappings.Count + $normalMappings.Count +
        $specularMappings.Count + $emissiveMappings.Count +
        $opacityMappings.Count + $ormMappings.Count + $metallicMappings.Count +
        $roughnessMappings.Count + $aoMappings.Count) -gt 0) {
        $converterArguments += '--no-auto-textures'
    }
    $converterResult = Invoke-NativeLogged -FilePath $converter -ArgumentList $converterArguments -WorkingDirectory $workDirectory
    if ($converterResult.ExitCode -ne 0) {
        throw "ModelAssetConverter cook failed. exit=$($converterResult.ExitCode)"
    }
    if (-not (Test-Path -LiteralPath $wmodelOutput -PathType Leaf)) {
        throw "ModelAssetConverter did not create the WModel: $wmodelOutput"
    }

    $infoResult = Invoke-NativeLogged -FilePath $converter -ArgumentList @('info', $wmodelName) -WorkingDirectory $workDirectory
    if ($infoResult.ExitCode -ne 0) {
        throw "ModelAssetConverter info failed. exit=$($infoResult.ExitCode)"
    }
    [IO.File]::WriteAllText($infoOutput, $infoResult.Text + [Environment]::NewLine, (New-Object Text.UTF8Encoding($false)))

    $modelMatch = [regex]::Match(
        $infoResult.Text,
        'sections=(?<sections>\d+)\s+animations=(?<animations>\d+)\s+skeleton=(?<skeleton>yes|no)',
        [Text.RegularExpressions.RegexOptions]::IgnoreCase
    )
    if (-not $modelMatch.Success) {
        throw 'Could not parse the Model summary from ModelAssetConverter info.'
    }
    $sectionCount = [int]$modelMatch.Groups['sections'].Value
    $animationCount = [int]$modelMatch.Groups['animations'].Value
    $hasSkeleton = $modelMatch.Groups['skeleton'].Value -ieq 'yes'
    $wmodelAnimationNames = @(
        [regex]::Matches(
            $infoResult.Text,
            'section\s+type=4\s+index=\d+\s+size=\d+\s+name=(?<name>[^\r\n]*)',
            [Text.RegularExpressions.RegexOptions]::IgnoreCase
        ) | ForEach-Object { $_.Groups['name'].Value.Trim() }
    )
    if (-not $hasSkeleton) {
        throw 'The cooked WModel has no skeleton.'
    }
    if ($animationCount -ne $actionCount) {
        throw "FBX Action and WModel animation counts differ. actions=$actionCount animations=$animationCount"
    }
    if ($ExpectedAnimationCount -ge 0 -and $animationCount -ne $ExpectedAnimationCount) {
        throw "WModel animation count mismatch. expected=$ExpectedAnimationCount actual=$animationCount"
    }
    if ($sectionCount -lt (3 + $animationCount)) {
        throw "WModel section count is too small. sections=$sectionCount minimum=$([int](3 + $animationCount))"
    }
    if ($wmodelAnimationNames.Count -ne $animationCount) {
        throw "Could not account for every WModel animation name. animations=$animationCount names=$($wmodelAnimationNames.Count)"
    }
    $matchedWModelNames = New-Object 'System.Collections.Generic.List[string]'
    $armatureName = [string]$report.armature
    foreach ($actionName in $actionNames) {
        $expectedNames = @(
            $actionName,
            "${armatureName}_$actionName",
            "${armatureName}|$actionName"
        )
        $matches = @($wmodelAnimationNames | Where-Object { $expectedNames -ccontains $_ })
        if ($matches.Count -ne 1) {
            throw "Action does not map to exactly one WModel animation. action=$actionName matches=$($matches -join ', ')"
        }
        $matchedWModelNames.Add($matches[0])
    }
    if (@($matchedWModelNames | Select-Object -Unique).Count -ne $animationCount -or
        @(Compare-Object -ReferenceObject @($wmodelAnimationNames) -DifferenceObject @($matchedWModelNames)).Count -ne 0) {
        throw 'FBX Actions and WModel animation names are not a one-to-one mapping.'
    }

    $packageMetadata = Read-WModelMetadata -Path $wmodelOutput
    if ($packageMetadata.Animations.Count -ne $animationCount) {
        throw "WModel inspector animation count mismatch. info=$animationCount binary=$($packageMetadata.Animations.Count)"
    }
    $lastSourceBoneIndex = -1
    foreach ($sourceBoneName in $sourceBones) {
        $matchedSkeletonBones = @($packageMetadata.SkeletonBones | Where-Object { $_.Name -ceq $sourceBoneName })
        if ($matchedSkeletonBones.Count -ne 1 -or $matchedSkeletonBones[0].Index -le $lastSourceBoneIndex) {
            throw "WModel skeleton does not preserve the PSA bone order: $sourceBoneName"
        }
        $lastSourceBoneIndex = [int]$matchedSkeletonBones[0].Index
    }
    foreach ($animation in $packageMetadata.Animations) {
        if ([Math]::Abs($animation.TicksPerSecond - $sourceFps) -gt 0.0001) {
            throw "WModel animation rate mismatch. name=$($animation.Name) source=$sourceFps cooked=$($animation.TicksPerSecond)"
        }
        $sourceAction = @($actions | Where-Object {
            $_.name -ceq $animation.Name -or
            "${armatureName}_$($_.name)" -ceq $animation.Name -or
            "${armatureName}|$($_.name)" -ceq $animation.Name
        })
        if ($sourceAction.Count -ne 1) {
            throw "WModel timing could not resolve its source Action: $($animation.Name)"
        }
        $expectedDurationTicks = [double]$sourceAction[0].frame_end - [double]$sourceAction[0].frame_start
        if ([Math]::Abs($animation.DurationTicks - $expectedDurationTicks) -gt 0.0001) {
            throw "WModel duration differs from the source Action. name=$($animation.Name) source=$expectedDurationTicks cooked=$($animation.DurationTicks)"
        }
        if ([bool]$sourceAction[0].source_has_scale_keys) {
            $expectedScaleKeysPerBone = [int]$sourceAction[0].source_raw_frame_count
            $sourceScaleBones = @($sourceAction[0].source_scale_bones)
            $cookedScaleBones = New-Object 'System.Collections.Generic.List[object]'
            foreach ($sourceScaleBone in $sourceScaleBones) {
                $boneName = [string]$sourceScaleBone.name
                $matchedChannels = @($animation.Channels | Where-Object { $_.BoneName -ceq $boneName })
                if ($matchedChannels.Count -ne 1) {
                    throw "WModel animation does not contain exactly one PSA bone channel. animation=$($animation.Name) bone=$boneName"
                }
                $channel = $matchedChannels[0]
                if ([int]$channel.ScaleKeyCount -ne $expectedScaleKeysPerBone -or
                    $null -eq $channel.ScaleMinimum -or $null -eq $channel.ScaleMaximum) {
                    throw "WModel animation scale key count is incomplete. animation=$($animation.Name) bone=$boneName"
                }
                $sourceMinimum = @($sourceScaleBone.minimum)
                $sourceMaximum = @($sourceScaleBone.maximum)
                for ($axis = 0; $axis -lt 3; ++$axis) {
                    if (-not (Test-FiniteDouble ([double]$sourceMinimum[$axis])) -or
                        -not (Test-FiniteDouble ([double]$sourceMaximum[$axis])) -or
                        [Math]::Abs([double]$channel.ScaleMinimum[$axis] - [double]$sourceMinimum[$axis]) -gt 0.0002 -or
                        [Math]::Abs([double]$channel.ScaleMaximum[$axis] - [double]$sourceMaximum[$axis]) -gt 0.0002) {
                        throw "WModel scale values differ from the PSA. animation=$($animation.Name) bone=$boneName axis=$axis"
                    }
                }
                $cookedScaleBones.Add([PSCustomObject]@{
                    Name = $boneName
                    KeyCount = [int]$channel.ScaleKeyCount
                    Minimum = @($channel.ScaleMinimum)
                    Maximum = @($channel.ScaleMaximum)
                    NonUnitComponentCount = [int]$channel.NonUnitScaleComponentCount
                })
            }
            $validatedScaleArray = @($cookedScaleBones | ForEach-Object { $_ })
            $animation | Add-Member -NotePropertyName ValidatedSourceBoneScales -NotePropertyValue $validatedScaleArray -Force
        }
    }

    foreach ($material in $packageMetadata.Materials) {
        foreach ($slot in @('Base', 'Normal', 'Specular', 'Emissive', 'Opacity', 'Orm', 'Metallic', 'Roughness', 'AmbientOcclusion')) {
            $storedPath = [string]$material.$slot
            if (-not [string]::IsNullOrWhiteSpace($storedPath)) {
                Assert-CookedTexturePath -StoredPath $storedPath -PackageRoot $workDirectory -Label "$($material.Name).$slot" | Out-Null
            }
        }
    }
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $materialMappings -Property 'Base' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $normalMappings -Property 'Normal' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $specularMappings -Property 'Specular' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $emissiveMappings -Property 'Emissive' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $opacityMappings -Property 'Opacity' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $ormMappings -Property 'Orm' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $metallicMappings -Property 'Metallic' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $roughnessMappings -Property 'Roughness' -PackageRoot $workDirectory
    Assert-RemapApplied -Materials $packageMetadata.Materials -Mappings $aoMappings -Property 'AmbientOcclusion' -PackageRoot $workDirectory

    # The Blender report is retained in the final package, so record final
    # package paths and the exact runtime animation names before moving it.
    $report.output_fbx = Join-Path $finalPackage $fbxName
    $report.output_blend = if ($KeepBlend) { Join-Path $finalPackage $blendName } else { $null }
    $report | Add-Member -NotePropertyName wmodel -NotePropertyValue (Join-Path $finalPackage $wmodelName) -Force
    $report | Add-Member -NotePropertyName wmodel_animations -NotePropertyValue $wmodelAnimationNames -Force
    $report | Add-Member -NotePropertyName wmodel_animation_timing -NotePropertyValue @($packageMetadata.Animations) -Force
    $report | Add-Member -NotePropertyName wmodel_materials -NotePropertyValue @($packageMetadata.Materials) -Force
    $report | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $reportOutput -Encoding UTF8

    $backupPackage = $null
    if (Test-Path -LiteralPath $finalPackage) {
        if (-not $OverwriteStagingOutput) {
            throw "The staging package appeared while Cook was running; refusing to replace it: $finalPackage"
        }
        Assert-ReplaceableActorXPackage -Package $finalPackage -Asset $AssetName
        $backupName = '.{0}.actorx-backup-{1}-{2}' -f $AssetName, (Get-Date -Format 'yyyyMMdd-HHmmss'), ([Guid]::NewGuid().ToString('N').Substring(0, 8))
        $backupPackage = Join-Path $stage $backupName
        Move-Item -LiteralPath $finalPackage -Destination $backupPackage
    }

    try {
        Move-Item -LiteralPath $workDirectory -Destination $finalPackage
    }
    catch {
        if ($backupPackage -and
            (Test-Path -LiteralPath $backupPackage) -and
            -not (Test-Path -LiteralPath $finalPackage)) {
            Move-Item -LiteralPath $backupPackage -Destination $finalPackage
        }
        throw
    }

    Write-Host ''
    Write-Host "Validated staging package: $finalPackage"
    Write-Host "WModel: $(Join-Path $finalPackage $wmodelName)"
    Write-Host "Actions: $($actionNames -join ', ')"
    Write-Host "WModel animation names: $($wmodelAnimationNames -join ', ')"
    Write-Host "sections=$sectionCount animations=$animationCount skeleton=yes"
    if ($backupPackage) {
        Write-Host "Previous package backup: $backupPackage"
    }

    [PSCustomObject]@{
        Package = $finalPackage
        WModel = (Join-Path $finalPackage $wmodelName)
        Fbx = (Join-Path $finalPackage $fbxName)
        Report = (Join-Path $finalPackage $reportName)
        Info = (Join-Path $finalPackage $infoName)
        Actions = $actionNames
        WModelAnimations = $wmodelAnimationNames
        Sections = $sectionCount
        Animations = $animationCount
        Skeleton = $true
        Backup = $backupPackage
    }
}
catch {
    Write-Warning "The failed staging work directory was preserved for diagnosis: $workDirectory"
    throw
}
