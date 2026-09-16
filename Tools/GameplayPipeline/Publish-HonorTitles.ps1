[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/HonorTitles'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))

function Read-JsonDocument([string]$RelativePath) {
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) { throw "Missing honor title document: $RelativePath" }
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

$document = Read-JsonDocument 'Data/Titles/HonorTitles.json'
Assert-ExactProperties $document @('schema', 'formatVersion', 'source', 'strings', 'titles') 'honor title document'
if ($document.schema -cne 'lostark.honor-titles' -or $document.formatVersion -ne 1) {
    throw 'Honor title header is invalid.'
}
$titles = @($document.titles)
if ($titles.Count -eq 0 -or $titles.Count -gt 4096) {
    throw "Honor title count is out of range: $($titles.Count)"
}

$titleIds = [Collections.Generic.HashSet[uint32]]::new()
$rows = [Collections.Generic.List[string]]::new()
foreach ($title in $titles) {
    Assert-ExactProperties $title @('titleId', 'name', 'origin', 'nameKey') 'honor title'
    Assert-JsonInteger $title.titleId 'honor title titleId' 1 ([uint32]::MaxValue)
    if ($title.name -isnot [string] -or [string]::IsNullOrWhiteSpace($title.name) -or $title.name.Length -gt 32) {
        throw "Honor title $($title.titleId) name must be 1..32 characters."
    }
    if ($title.origin -ceq 'retail') {
        if ($title.nameKey -isnot [string] -or $title.nameKey -cnotmatch '^tip\.name\.honortitle_\d+$' -or
            [uint32]$title.titleId -ge 100000) {
            throw "Honor title $($title.titleId) is not a valid retail row."
        }
    }
    elseif ($title.origin -ceq 'project') {
        if ($null -ne $title.nameKey -or [uint32]$title.titleId -lt 100000) {
            throw "Honor title $($title.titleId) is not a valid project title (ids start at 100000, no nameKey)."
        }
    }
    else {
        throw "Honor title $($title.titleId) origin must be 'retail' or 'project'."
    }
    if (-not $titleIds.Add([uint32]$title.titleId)) {
        throw "Duplicate honor title ID: $($title.titleId)"
    }
    $rows.Add((@('TITLE', [uint32]$title.titleId) -join "`t"))
}

if ($Mode -eq 'Validate') {
    Write-Output "Honor title Validate succeeded: $($rows.Count) titles."
    return
}

if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Honor title OutputRoot must be repository-relative.'
}
$outputDirectory = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $outputDirectory.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Honor title OutputRoot escaped the repository.'
}
[IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$lines = [Collections.Generic.List[string]]::new()
$lines.Add("LOSTARK_HONOR_TITLE_BOOTSTRAP`t1`t$($rows.Count)")
foreach ($row in $rows) { $lines.Add($row) }

$destination = Join-Path $outputDirectory 'HonorTitles.bootstrap'
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
    Write-Output "Honor title Publish succeeded: $($rows.Count) titles -> $destination"
}
catch {
    if ([IO.File]::Exists($staged)) { [IO.File]::Delete($staged) }
    if ($hadPrevious -and [IO.File]::Exists($rollback) -and -not [IO.File]::Exists($destination)) {
        [IO.File]::Move($rollback, $destination)
    }
    throw
}
