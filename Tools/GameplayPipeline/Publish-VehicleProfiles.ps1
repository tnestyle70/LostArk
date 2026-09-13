[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/Vehicles'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing vehicle document: $RelativePath" }
    return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
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

$document = Read-JsonDocument 'Data/Vehicles/VehicleProfiles.json'
Assert-ExactProperties $document @('schema', 'formatVersion', 'vehicles') 'vehicle profile document'
if ($document.schema -cne 'lostark.vehicle-profiles' -or $document.formatVersion -ne 1) {
    throw 'Vehicle profile header is invalid.'
}
$vehicles = @($document.vehicles)
if ($vehicles.Count -eq 0 -or $vehicles.Count -gt 4096) {
    throw "Vehicle profile count is out of range: $($vehicles.Count)"
}

$vehicleIds = [Collections.Generic.HashSet[uint32]]::new()
$rows = [Collections.Generic.List[string]]::new()
foreach ($vehicle in $vehicles) {
    Assert-ExactProperties $vehicle @('vehicleId', 'moveSpeed', 'source') 'vehicle profile'
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
    $rows.Add((@('VEHICLE', [uint32]$vehicle.vehicleId,
        $speed.ToString('R', [Globalization.CultureInfo]::InvariantCulture)) -join "`t"))
}

if ($Mode -eq 'Validate') {
    Write-Output "Vehicle profile Validate succeeded: $($rows.Count) vehicles."
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
$lines.Add("LOSTARK_VEHICLE_BOOTSTRAP`t1`t$($rows.Count)")
foreach ($row in $rows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'Vehicles.bootstrap'
$transactionId = [Guid]::NewGuid().ToString('N')
$staged = "$destination.staging.$transactionId"
$rollback = "$destination.rollback.$transactionId"
$hadPrevious = $false
try {
    [IO.File]::WriteAllLines($staged, $lines, [Text.UTF8Encoding]::new($false))
    if ([IO.File]::Exists($destination)) {
        [IO.File]::Move($destination, $rollback)
        $hadPrevious = $true
    }
    [IO.File]::Move($staged, $destination)
    if ($hadPrevious) { [IO.File]::Delete($rollback) }
    Write-Output "Vehicle profile Publish succeeded: $($rows.Count) vehicles -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
