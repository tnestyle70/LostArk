#requires -Version 7.0

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$StageRoot,

    [Parameter()]
    [string]$TxtpRoot = '',

    [Parameter()]
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\..\Client\Bin\Resources\Sound'),

    [Parameter()]
    [string]$WorkRoot = '',

    [Parameter()]
    [string]$VgmstreamPath = 'vgmstream-cli',

    [Parameter()]
    [string]$FfmpegPath = 'ffmpeg',

    [Parameter()]
    [ValidateRange(1, 32)]
    [int]$ThrottleLimit = 8,

    [Parameter()]
    [switch]$Replace,

    [Parameter()]
    [switch]$PlanOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-AbsolutePath {
    param([Parameter(Mandatory = $true)][string]$Path)

    if ([System.IO.Path]::IsPathFullyQualified($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Resolve-Executable {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (Test-Path -LiteralPath $Path -PathType Leaf) {
        return Resolve-AbsolutePath $Path
    }
    $command = Get-Command -Name $Path -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($null -eq $command) {
        throw "Executable was not found: $Path"
    }
    return $command.Source
}

function ConvertTo-SafeAudioLeaf {
    param([Parameter(Mandatory = $true)][string]$Value)

    $leaf = $Value.Trim()
    $leaf = [regex]::Replace($leaf, '[<>:"/\\|?*\x00-\x1f]', '_')
    $leaf = [regex]::Replace($leaf, '\s+', '_')
    $leaf = [regex]::Replace($leaf, '_{2,}', '_')
    $leaf = $leaf.Trim([char[]]@(' ', '.', '_'))
    if ([string]::IsNullOrWhiteSpace($leaf)) {
        $leaf = 'unnamed'
    }
    if ($leaf.Length -gt 120) {
        $leaf = $leaf.Substring(0, 120).TrimEnd([char[]]@(' ', '.', '_'))
    }
    return $leaf
}

function Normalize-WwiserTitle {
    param([Parameter(Mandatory = $true)][string]$Title)

    # wwiser appends switch values and generation markers such as
    # "[state=value] {r1} {!} {d}". They are variant metadata, not event names.
    return ([regex]::Replace($Title.Trim(), '\s+(?:\[|\{).*$', '')).Trim()
}

function Get-TxtpRecords {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [Parameter(Mandatory = $true)][string]$Group
    )

    $directory = Join-Path $Root $Group
    $tagPath = Join-Path $directory '!tags.m3u'
    if (-not (Test-Path -LiteralPath $tagPath -PathType Leaf)) {
        throw "wwiser tag list does not exist for group '$Group': $tagPath"
    }

    $records = [System.Collections.Generic.List[object]]::new()
    $rawTitle = $null
    foreach ($line in Get-Content -LiteralPath $tagPath -Encoding UTF8) {
        if ($line -match '^# %TITLE\s+(.+)$') {
            $rawTitle = [string]$matches[1]
            continue
        }
        if ($null -eq $rawTitle -or $line -notmatch '\.txtp\s*$') {
            continue
        }

        $txtpPath = Join-Path $directory $line.Trim()
        if (-not (Test-Path -LiteralPath $txtpPath -PathType Leaf)) {
            throw "wwiser tag list references a missing TXTP: $txtpPath"
        }
        $text = [System.IO.File]::ReadAllText($txtpPath)
        $eventMatch = [regex]::Match($text, 'CAkEvent\[\d+\]\s+(\d+)')
        $eventId = if ($eventMatch.Success) { [uint64]$eventMatch.Groups[1].Value } else { $null }
        $hashNameMatch = [regex]::Match($text, '(?m)^#\s+- hashname:\s+(.+?)\s*$')
        $eventName = if ($hashNameMatch.Success) {
            $hashNameMatch.Groups[1].Value.Trim()
        }
        else {
            Normalize-WwiserTitle $rawTitle
        }

        $mediaIds = [System.Collections.Generic.HashSet[uint64]]::new()
        foreach ($match in [regex]::Matches($text, '(?i)(?:[/\\]media[/\\]|\?)(\d+)\.wem')) {
            $null = $mediaIds.Add([uint64]$match.Groups[1].Value)
        }
        if ($mediaIds.Count -eq 0) {
            throw "TXTP does not reference any Wwise media: $txtpPath"
        }

        $records.Add([pscustomobject]@{
            Group = $Group
            Title = $eventName
            RawTitle = $rawTitle.Trim()
            EventId = $eventId
            MediaIds = @($mediaIds | Sort-Object)
            TxtpPath = $txtpPath
        })
        $rawTitle = $null
    }
    return @($records)
}

function Get-UiCategory {
    param(
        [Parameter(Mandatory = $true)][string]$Group,
        [Parameter(Mandatory = $true)][string]$Title
    )

    if ($Title -match '(?i)^sys_enhance_') {
        return 'UI/Enhancement'
    }
    if ($Group -eq 'UiItem' -or
        $Title -match '(?i)^sys_(?:abilitystone|bracelet|engrave|item_craft|item_polish|itemreadjust|jewel|tripod|advanoement)') {
        return 'UI/Equipment'
    }
    if ($Group -in @('UiInterface', 'UiCore')) {
        return 'UI/Select'
    }
    return 'UI/System'
}

function Get-ValtanCategory {
    param([Parameter(Mandatory = $true)][string]$Title)

    if ($Title -match '(?i)(?:ghost)?voltan1') {
        return 'Boss/Valtan/Voltan1'
    }
    if ($Title -match '(?i)voltan2') {
        return 'Boss/Valtan/Voltan2'
    }
    if ($Title -match '(?i)voltan3') {
        return 'Boss/Valtan/Voltan3'
    }
    return 'Boss/Valtan/Common'
}

function Add-MediaAssociation {
    param(
        [Parameter(Mandatory = $true)][hashtable]$Index,
        [Parameter(Mandatory = $true)][uint64]$MediaId,
        [Parameter(Mandatory = $true)][string]$Category,
        [Parameter(Mandatory = $true)][object]$Record
    )

    $key = '{0}|{1}' -f $Category.ToLowerInvariant(), $MediaId
    if (-not $Index.ContainsKey($key)) {
        $Index[$key] = [pscustomobject]@{
            MediaId = $MediaId
            Category = $Category
            Associations = [System.Collections.Generic.List[object]]::new()
            AssociationKeys = [System.Collections.Generic.HashSet[string]]::new(
                [System.StringComparer]::OrdinalIgnoreCase)
        }
    }
    $item = $Index[$key]
    $associationKey = '{0}|{1}|{2}' -f $Record.Group, $Record.Title, $Record.EventId
    if ($item.AssociationKeys.Add($associationKey)) {
        $item.Associations.Add([pscustomobject]@{
            group = $Record.Group
            eventName = $Record.Title
            eventId = $Record.EventId
        })
    }
}

function Get-WaveHeader {
    param([Parameter(Mandatory = $true)][string]$Path)

    $stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
    $reader = [System.IO.BinaryReader]::new($stream)
    try {
        if ([System.Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ne 'RIFF') {
            throw "WAV does not start with RIFF: $Path"
        }
        $null = $reader.ReadUInt32()
        if ([System.Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ne 'WAVE') {
            throw "RIFF file is not WAVE: $Path"
        }

        $format = $null
        [uint64]$dataBytes = 0
        while ($stream.Position + 8 -le $stream.Length) {
            $chunkId = [System.Text.Encoding]::ASCII.GetString($reader.ReadBytes(4))
            [uint64]$chunkBytes = $reader.ReadUInt32()
            $chunkStart = $stream.Position
            if ($chunkStart + $chunkBytes -gt $stream.Length) {
                throw "WAV chunk '$chunkId' exceeds file bounds: $Path"
            }
            if ($chunkId -eq 'fmt ') {
                if ($chunkBytes -lt 16) {
                    throw "WAV fmt chunk is too short: $Path"
                }
                $format = [pscustomobject]@{
                    AudioFormat = $reader.ReadUInt16()
                    Channels = $reader.ReadUInt16()
                    SampleRate = $reader.ReadUInt32()
                    ByteRate = $reader.ReadUInt32()
                    BlockAlign = $reader.ReadUInt16()
                    BitsPerSample = $reader.ReadUInt16()
                }
            }
            elseif ($chunkId -eq 'data') {
                $dataBytes = $chunkBytes
            }
            $stream.Position = $chunkStart + $chunkBytes + ($chunkBytes % 2)
        }
        if ($null -eq $format -or $dataBytes -eq 0) {
            throw "WAV is missing fmt or non-empty data chunk: $Path"
        }
        if ($format.ByteRate -eq 0) {
            throw "WAV byte rate is zero: $Path"
        }
        return [pscustomobject]@{
            AudioFormat = $format.AudioFormat
            Channels = $format.Channels
            SampleRate = $format.SampleRate
            BitsPerSample = $format.BitsPerSample
            DataBytes = $dataBytes
            DurationSeconds = [double]$dataBytes / [double]$format.ByteRate
        }
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

$StageRoot = Resolve-AbsolutePath $StageRoot
if ([string]::IsNullOrWhiteSpace($TxtpRoot)) {
    $TxtpRoot = Join-Path $StageRoot 'txtp_full'
}
$TxtpRoot = Resolve-AbsolutePath $TxtpRoot
$OutputRoot = Resolve-AbsolutePath $OutputRoot
if ([string]::IsNullOrWhiteSpace($WorkRoot)) {
    $WorkRoot = Join-Path ([System.IO.Path]::GetTempPath()) 'LostArkSoundPublish'
}
$WorkRoot = Resolve-AbsolutePath $WorkRoot
$VgmstreamPath = Resolve-Executable $VgmstreamPath
$FfmpegPath = Resolve-Executable $FfmpegPath

foreach ($requiredDirectory in @($StageRoot, $TxtpRoot)) {
    if (-not (Test-Path -LiteralPath $requiredDirectory -PathType Container)) {
        throw "Required directory does not exist: $requiredDirectory"
    }
}
$stageManifestPath = Join-Path $StageRoot 'StageManifest.json'
if (-not (Test-Path -LiteralPath $stageManifestPath -PathType Leaf)) {
    throw "Stage manifest does not exist: $stageManifestPath"
}
if ([System.IO.Path]::GetFileName($OutputRoot) -cne 'Sound' -or
    [System.IO.Path]::GetFileName([System.IO.Path]::GetDirectoryName($OutputRoot)) -cne 'Resources') {
    throw "OutputRoot must be the Sound child of a Resources directory: $OutputRoot"
}
$outputPrefix = $OutputRoot.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar
if ($WorkRoot.StartsWith($outputPrefix, [System.StringComparison]::OrdinalIgnoreCase) -or
    $OutputRoot.StartsWith(($WorkRoot.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar),
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'WorkRoot and OutputRoot must not contain each other.'
}

$stageManifest = Get-Content -LiteralPath $stageManifestPath -Raw -Encoding UTF8 |
    ConvertFrom-Json -Depth 30
$mediaById = @{}
foreach ($media in $stageManifest.media) {
    $mediaById[[string]$media.mediaId] = $media
}

Write-Host 'Reading wwiser event graphs...'
$recordsByGroup = @{}
foreach ($group in @(
    'BgmLobbyCharacter', 'BgmTrision', 'BgmValtan', 'BgmBern',
    'LanceMaster', 'Artist', 'Warlord', 'DimensionMaster', 'Common', 'Valtan',
    'UiSystems', 'UiInterface', 'UiMinigame', 'UiSystems1', 'UiCore', 'UiItem'
)) {
    $recordsByGroup[$group] = @(Get-TxtpRecords -Root $TxtpRoot -Group $group)
}

$mediaIndex = @{}
$classGroups = [ordered]@{
    LanceMaster = 'Character/LanceMaster'
    Artist = 'Character/Artist'
    Warlord = 'Character/Warlord'
    DimensionMaster = 'Character/DimensionMaster'
    Common = 'Character/Common'
}
foreach ($pair in $classGroups.GetEnumerator()) {
    foreach ($record in $recordsByGroup[$pair.Key]) {
        foreach ($mediaId in $record.MediaIds) {
            Add-MediaAssociation -Index $mediaIndex -MediaId $mediaId -Category $pair.Value -Record $record
        }
    }
}
foreach ($record in $recordsByGroup.Valtan) {
    $category = Get-ValtanCategory $record.Title
    foreach ($mediaId in $record.MediaIds) {
        Add-MediaAssociation -Index $mediaIndex -MediaId $mediaId -Category $category -Record $record
    }
}
foreach ($group in @('UiSystems', 'UiInterface', 'UiMinigame', 'UiSystems1', 'UiCore', 'UiItem')) {
    foreach ($record in $recordsByGroup[$group]) {
        $category = Get-UiCategory -Group $group -Title $record.Title
        foreach ($mediaId in $record.MediaIds) {
            Add-MediaAssociation -Index $mediaIndex -MediaId $mediaId -Category $category -Record $record
        }
    }
}

$candidates = [System.Collections.Generic.List[object]]::new()
foreach ($item in $mediaIndex.Values | Sort-Object Category, MediaId) {
    if (-not $mediaById.ContainsKey([string]$item.MediaId)) {
        throw "Event graph references media $($item.MediaId), but StageManifest does not contain it."
    }
    $primary = $item.Associations |
        Sort-Object @{ Expression = { if ($_.eventName -match '^\d+-\d+-event') { 1 } else { 0 } } },
            @{ Expression = { if ($_.eventName -match '(?i)_2d$') { 1 } else { 0 } } },
            @{ Expression = { $_.eventName.Length } }, eventName |
        Select-Object -First 1
    $leaf = '{0}__{1}.wav' -f (ConvertTo-SafeAudioLeaf $primary.eventName), $item.MediaId
    $relativePath = ($item.Category.Trim('/') + '/' + $leaf).Replace('\', '/')
    $sourcePath = Join-Path $StageRoot ([string]$mediaById[[string]$item.MediaId].stagedFile)
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        throw "Staged WEM is missing: $sourcePath"
    }
    $candidates.Add([pscustomobject]@{
        Kind = 'media'
        Category = $item.Category
        RelativePath = $relativePath
        SourcePath = $sourcePath
        EventNames = @($item.Associations.eventName | Sort-Object -Unique)
        EventIds = @($item.Associations.eventId | Where-Object { $null -ne $_ } | Sort-Object -Unique)
        MediaIds = @([uint64]$item.MediaId)
        Origins = @($mediaById[[string]$item.MediaId].origins | Sort-Object -Unique)
    })
}

$bgmRecords = [System.Collections.Generic.List[object]]::new()
foreach ($record in $recordsByGroup.BgmLobbyCharacter) {
    if ($record.Title -match '(?i)^bgm_(?:login_|wallpaper|intro_)') {
        $bgmRecords.Add([pscustomobject]@{ Category = 'BGM/Lobby'; Record = $record })
    }
    elseif ($record.Title -match '(?i)^(?:bgm_classmovie_|pcselect_lobby$)') {
        $bgmRecords.Add([pscustomobject]@{ Category = 'BGM/CharacterSelect'; Record = $record })
    }
}
foreach ($record in $recordsByGroup.BgmTrision) {
    $bgmRecords.Add([pscustomobject]@{ Category = 'BGM/CharacterSelect/Trision'; Record = $record })
}
foreach ($record in $recordsByGroup.BgmValtan) {
    if ($record.Title -match '(?i)^(?:bgm_heartrb_|heartrb_ed$)') {
        $bgmRecords.Add([pscustomobject]@{ Category = 'BGM/Valtan/EventMixes'; Record = $record })
    }
}
foreach ($record in $recordsByGroup.BgmBern) {
    if ($record.Title -match '(?i)^(?:berntown|bgm_berntown_)') {
        $bgmRecords.Add([pscustomobject]@{ Category = 'BGM/BernCastle'; Record = $record })
    }
}

$bgmTitleCounts = @{}
foreach ($entry in $bgmRecords | Sort-Object Category, @{ Expression = { $_.Record.Title } },
        @{ Expression = { $_.Record.TxtpPath } }) {
    $record = $entry.Record
    $titleKey = '{0}|{1}' -f $entry.Category.ToLowerInvariant(), $record.Title.ToLowerInvariant()
    if (-not $bgmTitleCounts.ContainsKey($titleKey)) {
        $bgmTitleCounts[$titleKey] = 0
    }
    $bgmTitleCounts[$titleKey]++
    $variant = $bgmTitleCounts[$titleKey]
    $suffix = if ($variant -gt 1) { '_v{0:D2}' -f $variant } else { '' }
    $leaf = '{0}{1}.wav' -f (ConvertTo-SafeAudioLeaf $record.Title), $suffix
    $relativePath = ($entry.Category.Trim('/') + '/' + $leaf).Replace('\', '/')
    $origins = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($mediaId in $record.MediaIds) {
        if (-not $mediaById.ContainsKey([string]$mediaId)) {
            throw "BGM event '$($record.Title)' references missing media $mediaId."
        }
        foreach ($origin in $mediaById[[string]$mediaId].origins) {
            $null = $origins.Add([string]$origin)
        }
    }
    $candidates.Add([pscustomobject]@{
        Kind = 'txtp'
        Category = $entry.Category
        RelativePath = $relativePath
        SourcePath = $record.TxtpPath
        EventNames = @($record.Title)
        EventIds = @($record.EventId | Where-Object { $null -ne $_ })
        MediaIds = @($record.MediaIds)
        Origins = @($origins | Sort-Object)
    })
}

$bernStateTracks = @(
    [pscustomobject]@{
        Name = 'M00_Capital_A'
        MediaId = [uint64]377323806
        Events = @('bgm_berntown_m00')
    },
    [pscustomobject]@{
        Name = 'M00_Capital_B'
        MediaId = [uint64]528842124
        Events = @('bgm_berntown_m00')
    },
    [pscustomobject]@{
        Name = 'M01_M03_M04_M05_Office_Market_Center_Magic'
        MediaId = [uint64]442447881
        Events = @(
            'bgm_berntown_m01_office', 'bgm_berntown_m03_market',
            'bgm_berntown_m04_center', 'bgm_berntown_m05_magic'
        )
    },
    [pscustomobject]@{
        Name = 'M02_NeriaPub'
        MediaId = [uint64]53915863
        Events = @('bgm_berntown_m02_neriapub')
    },
    [pscustomobject]@{
        Name = 'M06_Airdalin'
        MediaId = [uint64]346735131
        Events = @('bgm_berntown_m06_airdalin')
    }
)
foreach ($track in $bernStateTracks) {
    if (-not $mediaById.ContainsKey([string]$track.MediaId)) {
        throw "Mapped Bern Castle BGM media is missing from the stage: $($track.MediaId)"
    }
    $media = $mediaById[[string]$track.MediaId]
    $candidates.Add([pscustomobject]@{
        Kind = 'media'
        Category = 'BGM/BernCastle/StateTracks'
        RelativePath = ('BGM/BernCastle/StateTracks/{0}__{1}.wav' -f $track.Name, $track.MediaId)
        SourcePath = (Join-Path $StageRoot ([string]$media.stagedFile))
        EventNames = @($track.Events)
        EventIds = @()
        MediaIds = @([uint64]$track.MediaId)
        Origins = @($media.origins | Sort-Object -Unique)
    })
}

$bernAmbience = @(
    [pscustomobject]@{ Category = 'Air'; Name = 'Room10'; MediaId = [uint64]28797507; Events = @('amb_ber_t_b_berncastle_air_room10_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room1'; MediaId = [uint64]262028757; Events = @('amb_ber_t_b_berncastle_air_room1_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Rooms2_5_8'; MediaId = [uint64]14481271; Events = @('amb_ber_t_b_berncastle_air_room2_start', 'amb_ber_t_b_berncastle_air_room5_start', 'amb_ber_t_b_berncastle_air_room8_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room3'; MediaId = [uint64]531060647; Events = @('amb_ber_t_b_berncastle_air_room3_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room4'; MediaId = [uint64]194843752; Events = @('amb_ber_t_b_berncastle_air_room4_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room6'; MediaId = [uint64]482888230; Events = @('amb_ber_t_b_berncastle_air_room6_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room7'; MediaId = [uint64]321588397; Events = @('amb_ber_t_b_berncastle_air_room7_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Room9'; MediaId = [uint64]268859908; Events = @('amb_ber_t_b_berncastle_air_room9_start') },
    [pscustomobject]@{ Category = 'Air'; Name = 'Wind1'; MediaId = [uint64]1020393573; Events = @('amb_ber_t_b_berncastle_air_wind1_start') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'Candle2'; MediaId = [uint64]379979438; Events = @('s_ambburn_candle2') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'Candle3'; MediaId = [uint64]632501962; Events = @('s_ambburn_candle3') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'CandleS2'; MediaId = [uint64]599189021; Events = @('s_ambburn_s2') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'MachineRoll3'; MediaId = [uint64]303830908; Events = @('s_ambmetal_machineroll3') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'HarmonySquare'; MediaId = [uint64]660274823; Events = @('s_ambobj_bcharmonysquare1_2d') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'QueenHall1'; MediaId = [uint64]627624760; Events = @('s_ambobj_bcqueenhall1_2d') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'QueenHall2'; MediaId = [uint64]72776438; Events = @('s_ambobj_bcqueenhall2_2d') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'MagicLoop20'; MediaId = [uint64]1069007975; Events = @('s_ambobj_magic_loop20') },
    [pscustomobject]@{ Category = 'Objects'; Name = 'BossRushPortal'; MediaId = [uint64]1048315405; Events = @('s_ambobj_portal_bossrush1') },
    [pscustomobject]@{ Category = 'Crowd'; Name = 'CircusWalla1'; MediaId = [uint64]927084699; Events = @('s_ambwalla_circuswalla1') },
    [pscustomobject]@{ Category = 'Water'; Name = 'FallDrop4'; MediaId = [uint64]449202676; Events = @('s_ambwater_falldrop4') },
    [pscustomobject]@{ Category = 'Water'; Name = 'FallDrop5'; MediaId = [uint64]83182118; Events = @('s_ambwater_falldrop5') },
    [pscustomobject]@{ Category = 'Water'; Name = 'Fountain1'; MediaId = [uint64]309797110; Events = @('s_ambwater_fountain1') },
    [pscustomobject]@{ Category = 'Water'; Name = 'Fountain2'; MediaId = [uint64]535262030; Events = @('s_ambwater_fountain2', 's_ambwater_fountain2_01') }
)
foreach ($sound in $bernAmbience) {
    if (-not $mediaById.ContainsKey([string]$sound.MediaId)) {
        throw "Mapped Bern Castle ambience media is missing from the stage: $($sound.MediaId)"
    }
    $media = $mediaById[[string]$sound.MediaId]
    $category = 'Ambience/BernCastle/{0}' -f $sound.Category
    $candidates.Add([pscustomobject]@{
        Kind = 'media'
        Category = $category
        RelativePath = ('{0}/{1}__{2}.wav' -f $category, $sound.Name, $sound.MediaId)
        SourcePath = (Join-Path $StageRoot ([string]$media.stagedFile))
        EventNames = @($sound.Events)
        EventIds = @()
        MediaIds = @([uint64]$sound.MediaId)
        Origins = @($media.origins | Sort-Object -Unique)
    })
}

$valtanBgm = @(
    [pscustomobject]@{ Name = 'M01_KeepGoing'; MediaId = [uint64]992459057 },
    [pscustomobject]@{ Name = 'M02_LictusAppear_A'; MediaId = [uint64]371505226 },
    [pscustomobject]@{ Name = 'M02_LictusAppear_B'; MediaId = [uint64]512503051 },
    [pscustomobject]@{ Name = 'M03_LictusBattle'; MediaId = [uint64]89206595 },
    [pscustomobject]@{ Name = 'M04_KeepGoing2'; MediaId = [uint64]106505321 },
    [pscustomobject]@{ Name = 'M05_M06_ValtanRevive_Phase1'; MediaId = [uint64]91819552 },
    [pscustomobject]@{ Name = 'M05_ValtanRevive_B'; MediaId = [uint64]993715841 },
    [pscustomobject]@{ Name = 'M07_FakeDead'; MediaId = [uint64]477456395 },
    [pscustomobject]@{ Name = 'M08_ValtanPhase2'; MediaId = [uint64]575767475 },
    [pscustomobject]@{ Name = 'M09_ValtanDead'; MediaId = [uint64]79146529 }
)
foreach ($track in $valtanBgm) {
    if (-not $mediaById.ContainsKey([string]$track.MediaId)) {
        throw "Mapped Valtan BGM media is missing from the stage: $($track.MediaId)"
    }
    $media = $mediaById[[string]$track.MediaId]
    $candidates.Add([pscustomobject]@{
        Kind = 'media'
        Category = 'BGM/Valtan'
        RelativePath = ('BGM/Valtan/{0}__{1}.wav' -f $track.Name, $track.MediaId)
        SourcePath = (Join-Path $StageRoot ([string]$media.stagedFile))
        EventNames = @($track.Name)
        EventIds = @()
        MediaIds = @([uint64]$track.MediaId)
        Origins = @($media.origins | Sort-Object -Unique)
    })
}

$caseInsensitivePaths = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase)
$orderedCandidates = @($candidates | Sort-Object Category, RelativePath)
for ($index = 0; $index -lt $orderedCandidates.Count; $index++) {
    $candidate = $orderedCandidates[$index]
    if (-not $caseInsensitivePaths.Add($candidate.RelativePath)) {
        throw "Duplicate case-insensitive output path: $($candidate.RelativePath)"
    }
    $candidate | Add-Member -NotePropertyName Index -NotePropertyValue $index
}

if ($PlanOnly) {
    Write-Host 'Lost Ark Sound publish plan'
    Write-Host "  Candidates: $($orderedCandidates.Count)"
    $orderedCandidates | Group-Object Category | Sort-Object Name | ForEach-Object {
        Write-Host ("  {0}: {1}" -f $_.Name, $_.Count)
    }
    return
}

$stageOutput = Join-Path $WorkRoot 'Sound'
$rawRoot = Join-Path $WorkRoot 'raw'
$inputRoot = Join-Path $WorkRoot 'txtp'
New-Item -ItemType Directory -Path $stageOutput, $rawRoot, $inputRoot -Force | Out-Null

$stageMediaRootSlash = (Join-Path $StageRoot 'media').Replace('\', '/')
foreach ($candidate in $orderedCandidates | Where-Object Kind -eq 'txtp') {
    $text = [System.IO.File]::ReadAllText($candidate.SourcePath)
    $patched = [regex]::Replace(
        $text,
        '(?i)(?:[A-Za-z]:[/\\][^#\r\n]*?[/\\]media[/\\]|\?)(\d+)\.wem',
        { param($match) '{0}/{1}.wem' -f $stageMediaRootSlash, $match.Groups[1].Value }
    )
    if ($patched -match '(?i)\?\d+\.wem') {
        throw "TXTP still contains an unresolved ?media reference after patching: $($candidate.SourcePath)"
    }
    foreach ($mediaId in $candidate.MediaIds) {
        $expectedPath = Join-Path (Join-Path $StageRoot 'media') ("$mediaId.wem")
        if (-not (Test-Path -LiteralPath $expectedPath -PathType Leaf)) {
            throw "TXTP patch target is missing: $expectedPath"
        }
    }
    $patchedInput = Join-Path $inputRoot ('{0:D6}.txtp' -f $candidate.Index)
    [System.IO.File]::WriteAllText($patchedInput, $patched, [System.Text.UTF8Encoding]::new($false))
    $candidate.SourcePath = $patchedInput
}

Write-Host ("Rendering {0} unique Sound Manager WAV assets with throttle {1}..." -f
    $orderedCandidates.Count, $ThrottleLimit)
$renderResults = [System.Collections.Generic.List[object]]::new()
$renderedCount = 0
$orderedCandidates |
    ForEach-Object -Parallel {
        $candidate = $_
        $vgmstream = $using:VgmstreamPath
        $ffmpeg = $using:FfmpegPath
        $outputStage = $using:stageOutput
        $rawDirectory = $using:rawRoot

        function Read-WaveHeaderLocal {
            param([string]$Path)
            $stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open,
                [System.IO.FileAccess]::Read, [System.IO.FileShare]::Read)
            $reader = [System.IO.BinaryReader]::new($stream)
            try {
                if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ne 'RIFF') { throw 'missing RIFF' }
                $null = $reader.ReadUInt32()
                if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -ne 'WAVE') { throw 'missing WAVE' }
                $format = $null
                [uint64]$dataBytes = 0
                while ($stream.Position + 8 -le $stream.Length) {
                    $id = [Text.Encoding]::ASCII.GetString($reader.ReadBytes(4))
                    [uint64]$size = $reader.ReadUInt32()
                    $start = $stream.Position
                    if ($start + $size -gt $stream.Length) { throw "chunk $id exceeds bounds" }
                    if ($id -eq 'fmt ') {
                        if ($size -lt 16) { throw 'short fmt chunk' }
                        $format = [pscustomobject]@{
                            AudioFormat = $reader.ReadUInt16()
                            Channels = $reader.ReadUInt16()
                            SampleRate = $reader.ReadUInt32()
                            ByteRate = $reader.ReadUInt32()
                            BlockAlign = $reader.ReadUInt16()
                            BitsPerSample = $reader.ReadUInt16()
                        }
                    }
                    elseif ($id -eq 'data') { $dataBytes = $size }
                    $stream.Position = $start + $size + ($size % 2)
                }
                if ($null -eq $format -or $dataBytes -eq 0 -or $format.ByteRate -eq 0) {
                    throw 'missing fmt or data'
                }
                return [pscustomobject]@{
                    AudioFormat = $format.AudioFormat
                    Channels = $format.Channels
                    SampleRate = $format.SampleRate
                    BitsPerSample = $format.BitsPerSample
                    DataBytes = $dataBytes
                    DurationSeconds = [double]$dataBytes / [double]$format.ByteRate
                }
            }
            finally {
                $reader.Dispose()
                $stream.Dispose()
            }
        }

        try {
            $outputPath = Join-Path $outputStage $candidate.RelativePath.Replace('/', [IO.Path]::DirectorySeparatorChar)
            $outputDirectory = [IO.Path]::GetDirectoryName($outputPath)
            $null = [IO.Directory]::CreateDirectory($outputDirectory)

            $resume = $false
            if (Test-Path -LiteralPath $outputPath -PathType Leaf) {
                try {
                    $existing = Read-WaveHeaderLocal $outputPath
                    $resume = $existing.AudioFormat -eq 1 -and $existing.BitsPerSample -eq 16 -and
                        $existing.SampleRate -eq 48000 -and $existing.Channels -in @(1, 2) -and
                        $existing.DurationSeconds -gt 0
                }
                catch {
                    $resume = $false
                }
            }

            if (-not $resume) {
                $rawPath = Join-Path $rawDirectory ('{0:D6}.wav' -f $candidate.Index)
                $vgmOutput = & $vgmstream -i -D 2 -W 1 -o $rawPath $candidate.SourcePath 2>&1
                if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $rawPath -PathType Leaf)) {
                    throw "vgmstream failed with exit code ${LASTEXITCODE}: $($vgmOutput -join ' ')"
                }
                $rawInfo = Read-WaveHeaderLocal $rawPath
                if ($rawInfo.AudioFormat -eq 1 -and $rawInfo.BitsPerSample -eq 16 -and
                    $rawInfo.SampleRate -eq 48000 -and $rawInfo.Channels -in @(1, 2)) {
                    Move-Item -LiteralPath $rawPath -Destination $outputPath -Force
                }
                else {
                    $channels = if ($rawInfo.Channels -eq 1) { 1 } else { 2 }
                    $ffmpegOutput = & $ffmpeg -hide_banner -loglevel error -y -i $rawPath -map 0:a:0 `
                        -c:a pcm_s16le -ar 48000 -ac $channels $outputPath 2>&1
                    $ffmpegExitCode = $LASTEXITCODE
                    Remove-Item -LiteralPath $rawPath -Force
                    if ($ffmpegExitCode -ne 0 -or -not (Test-Path -LiteralPath $outputPath -PathType Leaf)) {
                        throw "ffmpeg failed with exit code ${ffmpegExitCode}: $($ffmpegOutput -join ' ')"
                    }
                }
            }

            $info = Read-WaveHeaderLocal $outputPath
            if ($info.AudioFormat -ne 1 -or $info.BitsPerSample -ne 16 -or
                $info.SampleRate -ne 48000 -or $info.Channels -notin @(1, 2) -or
                $info.DurationSeconds -le 0) {
                throw 'rendered WAV does not satisfy PCM16/48kHz/mono-or-stereo contract'
            }
            $hashStream = [IO.File]::OpenRead($outputPath)
            $sha256 = [Security.Cryptography.SHA256]::Create()
            try {
                $hash = [Convert]::ToHexString($sha256.ComputeHash($hashStream))
            }
            finally {
                $sha256.Dispose()
                $hashStream.Dispose()
            }
            [pscustomobject]@{
                Index = $candidate.Index
                Success = $true
                Resumed = $resume
                Bytes = ([IO.FileInfo]$outputPath).Length
                Sha256 = $hash
                Channels = $info.Channels
                SampleRate = $info.SampleRate
                BitsPerSample = $info.BitsPerSample
                DurationSeconds = $info.DurationSeconds
                Error = $null
            }
        }
        catch {
            [pscustomobject]@{
                Index = $candidate.Index
                Success = $false
                Resumed = $false
                Bytes = 0
                Sha256 = $null
                Channels = 0
                SampleRate = 0
                BitsPerSample = 0
                DurationSeconds = 0.0
                Error = $_.Exception.Message
            }
        }
    } -ThrottleLimit $ThrottleLimit |
    ForEach-Object {
        $renderedCount++
        $renderResults.Add($_)
        if ($renderedCount % 100 -eq 0 -or $renderedCount -eq $orderedCandidates.Count) {
            Write-Host ("Rendered WAV {0}/{1}" -f $renderedCount, $orderedCandidates.Count)
        }
    }

$failed = @($renderResults | Where-Object { -not $_.Success })
if ($failed.Count -gt 0) {
    foreach ($failure in $failed | Select-Object -First 20) {
        $candidate = $orderedCandidates[$failure.Index]
        [Console]::Error.WriteLine("ERROR: $($candidate.RelativePath): $($failure.Error)")
    }
    throw "WAV rendering failed for $($failed.Count) candidate(s). WorkRoot is preserved for resume: $WorkRoot"
}

$resultByIndex = @{}
foreach ($result in $renderResults) {
    $resultByIndex[[int]$result.Index] = $result
}
$fileEntries = [System.Collections.Generic.List[object]]::new()
foreach ($candidate in $orderedCandidates) {
    $result = $resultByIndex[$candidate.Index]
    $assetId = ('Sound/' + $candidate.RelativePath).Replace('\', '/')
    $fileEntries.Add([pscustomobject][ordered]@{
        assetId = $assetId
        category = $candidate.Category
        sourceKind = $candidate.Kind
        renderMode = if ($candidate.Kind -eq 'txtp') { 'finite-event-mix-no-loop' } else { 'direct-media' }
        eventNames = @($candidate.EventNames)
        eventIds = @($candidate.EventIds)
        mediaIds = @($candidate.MediaIds)
        origins = @($candidate.Origins)
        bytes = [long]$result.Bytes
        sha256 = $result.Sha256
        sampleRate = [int]$result.SampleRate
        bitsPerSample = [int]$result.BitsPerSample
        channels = [int]$result.Channels
        durationSeconds = [Math]::Round([double]$result.DurationSeconds, 6)
    })
}

$categorySummary = @($fileEntries |
    Group-Object category |
    Sort-Object Name |
    ForEach-Object {
        [ordered]@{
            category = $_.Name
            wavCount = $_.Count
            bytes = [long](($_.Group | Measure-Object -Property bytes -Sum).Sum)
            durationSeconds = [Math]::Round([double](($_.Group | Measure-Object -Property durationSeconds -Sum).Sum), 3)
        }
    })
$catalog = [ordered]@{
    formatVersion = 1
    generatedAtKst = [DateTimeOffset]::Now.ToString('o')
    audioFormat = [ordered]@{
        codec = 'pcm_s16le'
        sampleRate = 48000
        channels = 'mono-or-stereo'
    }
    entries = @($fileEntries | ForEach-Object {
        [ordered]@{
            id = ('sound:{0}:{1}' -f
                ($_.category.Replace('/', '.').ToLowerInvariant()),
                ([IO.Path]::GetFileNameWithoutExtension($_.assetId)).ToLowerInvariant())
            category = $_.category
            assetId = $_.assetId
            eventNames = @($_.eventNames)
            mediaIds = @($_.mediaIds)
        }
    })
}
$stageManifestHash = (Get-FileHash -LiteralPath $stageManifestPath -Algorithm SHA256).Hash
$manifest = [ordered]@{
    formatVersion = 1
    generatedAtKst = [DateTimeOffset]::Now.ToString('o')
    source = [ordered]@{
        stageManifestSha256 = $stageManifestHash
        keySha256 = [string]$stageManifest.key.sha256
        packages = @($stageManifest.packages.logicalName | Sort-Object)
    }
    selection = [ordered]@{
        lobby = 'bgm_login_*, bgm_wallpaper*, bgm_intro_*'
        characterSelect = 'bgm_classmovie_*, pcselect_lobby, SOUND_BGM_TRISION'
        bernCastleBgm = 'berntown, bgm_berntown_*, and direct m00-m06 state media'
        bernCastleAmbience = '23 unique level-referenced AIR1/ACT media; stop events are control-only'
        valtanBgm = 'CommanderRaid M01-M09 mapped media'
        character = 'all media reachable from four class and common banks'
        valtan = 'all media reachable from Voltan1/2/3 banks'
        ui = 'all media reachable from selected UI banks, classified by event name and bank'
    }
    summary = [ordered]@{
        wavCount = $fileEntries.Count
        bytes = [long](($fileEntries | Measure-Object -Property bytes -Sum).Sum)
        durationSeconds = [Math]::Round([double](($fileEntries | Measure-Object -Property durationSeconds -Sum).Sum), 3)
        categories = $categorySummary
    }
    files = @($fileEntries)
}

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
[IO.File]::WriteAllText((Join-Path $stageOutput 'SoundCatalog.json'),
    (($catalog | ConvertTo-Json -Depth 12) + [Environment]::NewLine), $utf8NoBom)
[IO.File]::WriteAllText((Join-Path $stageOutput 'ExtractionManifest.json'),
    (($manifest | ConvertTo-Json -Depth 16) + [Environment]::NewLine), $utf8NoBom)

$allWavs = @(Get-ChildItem -LiteralPath $stageOutput -Recurse -File -Filter '*.wav')
if ($allWavs.Count -ne $fileEntries.Count) {
    throw "Staged WAV count $($allWavs.Count) does not match manifest count $($fileEntries.Count)."
}
if (Test-Path -LiteralPath $OutputRoot) {
    if (-not $Replace) {
        throw "OutputRoot already exists. Re-run with -Replace after reviewing it: $OutputRoot"
    }
    $backupRoot = Join-Path $WorkRoot 'previous-Sound'
    if (Test-Path -LiteralPath $backupRoot) {
        throw "Replacement backup path already exists: $backupRoot"
    }
    Move-Item -LiteralPath $OutputRoot -Destination $backupRoot
}

$outputParent = [IO.Path]::GetDirectoryName($OutputRoot)
$null = [IO.Directory]::CreateDirectory($outputParent)
try {
    Move-Item -LiteralPath $stageOutput -Destination $OutputRoot
}
catch {
    $backupRoot = Join-Path $WorkRoot 'previous-Sound'
    if ((Test-Path -LiteralPath $backupRoot) -and -not (Test-Path -LiteralPath $OutputRoot)) {
        Move-Item -LiteralPath $backupRoot -Destination $OutputRoot
    }
    throw
}

Write-Host 'Lost Ark Sound publish complete.'
Write-Host "  Output:   $OutputRoot"
Write-Host "  WAVs:     $($fileEntries.Count)"
Write-Host ("  Bytes:    {0:N0}" -f (($fileEntries | Measure-Object -Property bytes -Sum).Sum))
Write-Host ("  Duration: {0:N3} seconds" -f (($fileEntries | Measure-Object -Property durationSeconds -Sum).Sum))
Write-Host "  Catalog:  $(Join-Path $OutputRoot 'SoundCatalog.json')"
Write-Host "  Manifest: $(Join-Path $OutputRoot 'ExtractionManifest.json')"
