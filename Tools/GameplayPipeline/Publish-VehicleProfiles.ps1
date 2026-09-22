[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Vehicles'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$inputHashes = @{}

function Get-FileFingerprint([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-', '') }
    finally { $sha.Dispose(); $stream.Dispose() }
}

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing vehicle document: $RelativePath" }
    $before = (Get-FileFingerprint $path)
    $value = Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($before -cne (Get-FileFingerprint $path)) {
        throw "Vehicle input changed while reading: $RelativePath"
    }
    $inputHashes[$path] = $before
    return $value
}

function Assert-ExactProperties([object]$Value, [string[]]$Expected, [string]$Context) {
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context fields are invalid. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-JsonInteger([object]$Value, [string]$Context, [long]$Minimum, [long]$Maximum) {
    if (($Value -isnot [int]) -and ($Value -isnot [long])) {
        throw "$Context must be a JSON integer."
    }
    if ([long]$Value -lt $Minimum -or [long]$Value -gt $Maximum) {
        throw "$Context integer is out of range: $Value"
    }
}

function Assert-JsonNumber([object]$Value, [string]$Context) {
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and ($Value -isnot [double]) -and ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    if ([double]::IsNaN([double]$Value) -or [double]::IsInfinity([double]$Value)) {
        throw "$Context must be finite."
    }
}

function Format-Invariant([double]$Value) {
    return $Value.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
}

function Get-VehicleClipSeconds([object]$Vehicle, [string]$Clip) {
    $authoredRelative = "Data/Animation/Authored/Vehicle_$($Vehicle.vehicleId)/Vehicle_$($Vehicle.vehicleId).boneclips.json"
    $authoredPath = Join-Path $repoRoot $authoredRelative
    if ([IO.File]::Exists($authoredPath)) {
        $authored = Read-JsonDocument $authoredRelative
        $matches = @($authored.clips | Where-Object { $_.name -ceq $Clip })
        if ($matches.Count -gt 1) { throw "Ambiguous authored flight clip: $Clip" }
        if ($matches.Count -eq 1) {
            Assert-JsonInteger $matches[0].durationMs 'authored flight clip durationMs' 1 600000
            return [double]$matches[0].durationMs / 1000.0
        }
    }
    $asset = [string]$Vehicle.modelAssetId
    if ([IO.Path]::IsPathRooted($asset) -or $asset.Contains(':') -or ($asset -split '[/\\]') -contains '..') {
        throw 'Flight model asset must be Resources-relative.'
    }
    $path = Join-Path (Join-Path $repoRoot 'Client/Bin/Resources') $asset
    $stream = [IO.File]::OpenRead($path)
    $reader = [IO.BinaryReader]::new($stream)
    try {
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WINT') { throw 'Invalid flight WModel.' }
        $stream.Position = 16
        if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WMOD') { throw 'Invalid flight model header.' }
        $count = $reader.ReadUInt32()
        if ($count -eq 0 -or $count -gt 65536) { throw 'Invalid flight section count.' }
        for ($index = 0; $index -lt $count; ++$index) {
            $stream.Position = 48 + $index * 64
            $kind = $reader.ReadUInt32(); $null = $reader.ReadUInt32()
            $offset = $reader.ReadUInt64(); $size = $reader.ReadUInt64()
            $name = [Text.Encoding]::UTF8.GetString($reader.ReadBytes(40)).TrimEnd([char]0)
            if ($name -cne $Clip) { continue }
            if ($offset -gt $stream.Length - 48 -or $size -lt 48) { throw 'Invalid flight animation range.' }
            $stream.Position = 32 + $offset
            if ([Text.Encoding]::ASCII.GetString($reader.ReadBytes(4)) -cne 'WANM') { throw 'Flight section is not animation.' }
            $null = $reader.ReadUInt32()
            $duration = [double]$reader.ReadSingle(); $rate = [double]$reader.ReadSingle()
            if ([double]::IsNaN($duration) -or [double]::IsInfinity($duration) -or $duration -le 0 -or
                [double]::IsNaN($rate) -or [double]::IsInfinity($rate) -or $rate -le 0) { throw 'Invalid flight clip timing.' }
            return $duration / $rate
        }
        throw "Flight clip is missing: $Clip"
    } finally { $reader.Dispose(); $stream.Dispose() }
}

$presentation = Read-JsonDocument 'Data/Actors/VehicleCatalog.json'
$document = Read-JsonDocument 'Data/Vehicles/VehicleProfiles.json'
Assert-ExactProperties $document @('schema', 'formatVersion', 'vehicles') 'vehicle profile document'
if ($document.schema -cne 'lostark.vehicle-profiles' -or $document.formatVersion -ne 2) {
    throw 'Vehicle profile header is invalid.'
}
$skillSlots = @('SPACE', 'Q', 'W', 'E')
$skillCount = 0
$vehicles = @($document.vehicles)
if ($vehicles.Count -eq 0 -or $vehicles.Count -gt 4096) {
    throw "Vehicle profile count is out of range: $($vehicles.Count)"
}

$vehicleIds = [Collections.Generic.HashSet[uint32]]::new()
$rows = [Collections.Generic.List[string]]::new()
foreach ($vehicle in $vehicles) {
    $profileFields = @('vehicleId', 'moveSpeed', 'source', 'skills')
    if ($null -ne $vehicle.flight) { $profileFields += 'flight' }
    Assert-ExactProperties $vehicle $profileFields 'vehicle profile'
    $flightFields = @('hoverHeight', 'maximumHeight', 'speed', 'verticalSpeed')
    $flightValues = @(0, 0, 0, 0, 0, 0)
    if ($null -ne $vehicle.flight) {
        Assert-ExactProperties $vehicle.flight $flightFields 'vehicle flight'
        if ($vehicle.vehicleId -ne 9523) { throw 'Only Ancient Sea supports flight.' }
        $presentationRows = @($presentation.vehicles | Where-Object { $_.vehicleId -eq $vehicle.vehicleId })
        if ($presentationRows.Count -ne 1) { throw 'Flight presentation vehicle is missing or ambiguous.' }
        $flightSkills = @($presentationRows[0].skills | Where-Object { $_.inputSlot -ceq 'E' })
        if ($flightSkills.Count -ne 1 -or @($flightSkills[0].vehicleClips).Count -ne 1) { throw 'Flight needs one E clip.' }
        $flightSkill = $flightSkills[0]
        $window = $flightSkill.flightWindow
        if ($null -eq $window) { throw 'Flight presentation window is required.' }
        Assert-ExactProperties $window @('loopStartSeconds', 'loopEndSeconds', 'landingStartSeconds') 'flight window'
        foreach ($key in @('loopStartSeconds', 'loopEndSeconds', 'landingStartSeconds')) { Assert-JsonNumber $window.$key "flight window $key" }
        $clipSeconds = Get-VehicleClipSeconds $presentationRows[0] $flightSkill.vehicleClips[0]
        if ($window.loopStartSeconds -le 0 -or $window.loopEndSeconds -le $window.loopStartSeconds -or
            $window.landingStartSeconds -lt $window.loopEndSeconds -or $window.landingStartSeconds -ge $clipSeconds) {
            throw 'Flight window exceeds the actual clip or is out of order.'
        }
        $profileFlightSkills = @($vehicle.skills | Where-Object { $_.inputSlot -ceq 'E' -and $_.skillId -eq $flightSkill.skillId })
        if ($profileFlightSkills.Count -ne 1) { throw 'Flight skill ID does not join the Server profile.' }
        $flightValues = @((Format-Invariant $window.loopStartSeconds),
            (Format-Invariant ($clipSeconds - [double]$window.landingStartSeconds)))
        $flightValues += @($flightFields | ForEach-Object {
            Assert-JsonNumber $vehicle.flight.$_ "flight $_"
            if ($vehicle.flight.$_ -le 0 -or $vehicle.flight.$_ -gt 60) { throw "flight $_ out of range" }
            Format-Invariant $vehicle.flight.$_
        })
        if ($vehicle.flight.hoverHeight -lt 0.1 -or $vehicle.flight.maximumHeight -lt $vehicle.flight.hoverHeight -or
            $vehicle.flight.maximumHeight / $vehicle.flight.verticalSpeed -gt 600.0) { throw 'Flight height or landing duration bounds are invalid.' }
        if ($window.loopStartSeconds -gt 60 -or $clipSeconds - [double]$window.landingStartSeconds -gt 60) {
            throw 'Flight phase timing exceeds the Server catalog range.'
        }
    }
    Assert-JsonInteger $vehicle.vehicleId 'vehicle vehicleId' 1 ([uint32]::MaxValue)
    Assert-JsonNumber $vehicle.moveSpeed "vehicle $($vehicle.vehicleId) moveSpeed"
    Assert-ExactProperties $vehicle.source @('table', 'primaryKey', 'column', 'value', 'divisor') "vehicle $($vehicle.vehicleId) source"
    Assert-JsonInteger $vehicle.source.primaryKey "vehicle $($vehicle.vehicleId) source primaryKey" 1 ([uint32]::MaxValue)
    Assert-JsonInteger $vehicle.source.value "vehicle $($vehicle.vehicleId) source value" 1 100000
    Assert-JsonInteger $vehicle.source.divisor "vehicle $($vehicle.vehicleId) source divisor" 1 100000
    $speed = [double]$vehicle.moveSpeed
    if ($vehicle.source.table -cne 'EFTable_Vehicle' -or $vehicle.source.column -cne 'MoveSpeed' -or
        [uint32]$vehicle.source.primaryKey -ne [uint32]$vehicle.vehicleId -or
        [Math]::Abs(([double]$vehicle.source.value / [double]$vehicle.source.divisor) - $speed) -gt 0.000001 -or
        $speed -le 0.0 -or $speed -gt 30.0) {
        throw "Vehicle $($vehicle.vehicleId) speed does not match its EFTable_Vehicle source."
    }
    if (-not $vehicleIds.Add([uint32]$vehicle.vehicleId)) {
        throw "Duplicate vehicle ID: $($vehicle.vehicleId)"
    }
    $skills = @($vehicle.skills)
    if ($skills.Count -gt $skillSlots.Count) {
        throw "Vehicle $($vehicle.vehicleId) has more skills than slots."
    }
    $rows.Add((@('VEHICLE', [uint32]$vehicle.vehicleId, (Format-Invariant $speed), $skills.Count) + $flightValues -join "`t"))
    $usedSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $skillIds = [Collections.Generic.HashSet[uint32]]::new()
    foreach ($skill in $skills) {
        $context = "vehicle $($vehicle.vehicleId) skill $($skill.skillId)"
        Assert-ExactProperties $skill @('skillId', 'inputSlot', 'cooldownMs', 'actionDurationMs', 'source', 'rootMotionSamples') $context
        Assert-JsonInteger $skill.skillId "$context skillId" 1 ([uint32]::MaxValue)
        Assert-JsonInteger $skill.cooldownMs "$context cooldownMs" 0 600000
        Assert-JsonInteger $skill.actionDurationMs "$context actionDurationMs" 1 60000
        Assert-ExactProperties $skill.source @('vehicleColumn', 'skillTable', 'cooldownColumn', 'action') "$context source"
        if ($skillSlots -cnotcontains $skill.inputSlot -or -not $usedSlots.Add([string]$skill.inputSlot)) {
            throw "$context inputSlot is invalid or repeated: $($skill.inputSlot)"
        }
        if ($skill.source.skillTable -cne 'EFTable_Skill' -or $skill.source.cooldownColumn -cne 'Cooltime' -or
            @('MovingSkill', 'SkillId0', 'SkillId1', 'SkillId2') -cnotcontains $skill.source.vehicleColumn) {
            throw "$context source is invalid."
        }
        if (-not $skillIds.Add([uint32]$skill.skillId)) {
            throw "Duplicate skill ID within vehicle $($vehicle.vehicleId): $($skill.skillId)"
        }
        ++$skillCount
        $samples = @($skill.rootMotionSamples)
        $packed = '-'
        if ($samples.Count -gt 0) {
            if ($samples.Count -lt 2 -or $samples.Count -gt 512) {
                throw "$context root motion sample count is invalid."
            }
            $previousMs = -1
            $tokens = [Collections.Generic.List[string]]::new()
            foreach ($sample in $samples) {
                Assert-ExactProperties $sample @('timeMs', 'forward', 'lateral', 'up') "$context root motion sample"
                Assert-JsonInteger $sample.timeMs "$context root motion timeMs" 0 ([long]$skill.actionDurationMs)
                foreach ($axis in @('forward', 'lateral', 'up')) {
                    Assert-JsonNumber $sample.$axis "$context root motion $axis"
                }
                if ([int]$sample.timeMs -le $previousMs) {
                    throw "$context root motion samples are out of order."
                }
                $previousMs = [int]$sample.timeMs
                $tokens.Add(('{0}:{1}:{2}:{3}' -f [int]$sample.timeMs, (Format-Invariant ([double]$sample.forward)),
                    (Format-Invariant ([double]$sample.lateral)), (Format-Invariant ([double]$sample.up))))
            }
            $packed = $tokens -join ','
        }
        $rows.Add((@('VEHICLESKILL', [uint32]$vehicle.vehicleId, [uint32]$skill.skillId, $skill.inputSlot,
            [uint32]$skill.cooldownMs, [uint32]$skill.actionDurationMs, $samples.Count, $packed) -join "`t"))
    }
}

if ($Mode -eq 'Validate') {
    Write-Output "Vehicle profile Validate succeeded: $($vehicleIds.Count) vehicles, $skillCount skills."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Vehicle profile OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Vehicle profile OutputRoot escaped the repository.'
}
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_VEHICLE_BOOTSTRAP`t3`t$($rows.Count)")
foreach ($row in $rows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Vehicles.bootstrap'
$previousHash = if ([IO.File]::Exists($destination)) { (Get-FileFingerprint $destination) } else { '' }
$transactionId = [Guid]::NewGuid().ToString('N')
$staged = "$destination.staging.$transactionId"
$rollback = "$destination.rollback.$transactionId"
$hadPrevious = $false
try {
    [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
    foreach ($inputPath in $inputHashes.Keys) {
        if (-not [IO.File]::Exists($inputPath) -or
            $inputHashes[$inputPath] -cne (Get-FileFingerprint $inputPath)) {
            throw "Vehicle input changed before publish: $inputPath"
        }
    }
    $currentHash = if ([IO.File]::Exists($destination)) { (Get-FileFingerprint $destination) } else { '' }
    if ($currentHash -cne $previousHash) { throw 'Vehicle bootstrap changed before publish.' }
    if ([IO.File]::Exists($destination)) {
        [IO.File]::Move($destination, $rollback)
        $hadPrevious = $true
    }
    [IO.File]::Move($staged, $destination)
    if ($hadPrevious) { [IO.File]::Delete($rollback) }
    Write-Output "Vehicle profile Publish succeeded: $($vehicleIds.Count) vehicles, $skillCount skills -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
