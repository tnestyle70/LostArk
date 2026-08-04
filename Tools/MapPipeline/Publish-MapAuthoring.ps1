[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9_.-]+$')]
    [string]$AreaId,

    [string]$ProjectRoot,

    [ValidateRange(0, 65537)]
    [int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
$ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
$authoringPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapplacements"
$authoringDeployPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.deployplacements"
$importRoot = Join-Path $ProjectRoot "Data\Maps\Imported\$AreaId"
$sourceCatalogPath = Join-Path $importRoot "$AreaId.mapassets"
$sourceShardSetPath = Join-Path $importRoot "$AreaId.mapset"
$sourceDeployCatalogPath = Join-Path $importRoot "$AreaId.deployassets"
$runtimeRoot = Join-Path $ProjectRoot 'Client\Bin\DataFiles\Map'
$runtimePath = Join-Path $runtimeRoot "$AreaId.mapplacements"
$shardSetPath = Join-Path $runtimeRoot "$AreaId.mapset"
$utf8 = [Text.UTF8Encoding]::new($false)
$importedPlacementMask = [uint64]::Parse(
    '9223372036854775808',
    [Globalization.CultureInfo]::InvariantCulture)

function Read-PlacementDocument {
    param([string]$Path)
    $lines = @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) {
        throw "Placement document is empty: $Path"
    }
    $header = [regex]::Match(
        $lines[0],
        '^LOSTARK_MAP_PLACEMENTS\s+2\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
    if (-not $header.Success -or $header.Groups['area'].Value -ne $AreaId) {
        throw "Placement header does not match area '$AreaId': $Path"
    }
    $rows = @($lines | Select-Object -Skip 1)
    if ([uint32]$header.Groups['count'].Value -ne $rows.Count -or
        $rows.Count -gt 65536) {
        throw "Placement count mismatch: $Path"
    }
    return $rows
}

function Parse-PlacementRow {
    param([string]$Row, [string]$Context)
	$number = '-?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?'
    $match = [regex]::Match(
        $Row,
		('^(?<id>[0-9]+)\s+"(?<source>[^"\r\n]{1,256})"\s+' +
		 '"(?<level>[A-Za-z0-9_.-]{1,128})"\s+' +
		 '"(?<transform>[A-Za-z0-9_.-]{1,32})"\s+' +
		 '"(?<asset>[A-Za-z0-9_.:-]+)"\s+' +
		 "(?<px>$number)\s+(?<py>$number)\s+(?<pz>$number)\s+" +
		 "(?<qx>$number)\s+(?<qy>$number)\s+(?<qz>$number)\s+(?<qw>$number)\s+" +
		 "(?<sx>$number)\s+(?<sy>$number)\s+(?<sz>$number)\s+(?<visible>[01])$") )
    if (-not $match.Success) {
        throw "Invalid placement row in $Context"
    }
	$values = @('px','py','pz','qx','qy','qz','qw','sx','sy','sz') | ForEach-Object {
		$value = 0.0
		if (-not [double]::TryParse(
			$match.Groups[$_].Value,
			[Globalization.NumberStyles]::Float,
			[Globalization.CultureInfo]::InvariantCulture,
			[ref]$value) -or
			[double]::IsNaN($value) -or [double]::IsInfinity($value)) {
			throw "Non-finite placement value in $Context"
		}
		$value
	}
	$placementId = [uint64]$match.Groups['id'].Value
	$transformSource = $match.Groups['transform'].Value
	$importedSource = $transformSource -in @('actor','component')
	$editorSource = $transformSource -in @('editor','legacy','overlay')
	$validIdDomain =
		($importedSource -and 0 -ne ($placementId -band $importedPlacementMask)) -or
		($editorSource -and $placementId -le [uint64]0x7fffffffffffffff)
	$quaternionLength = [math]::Sqrt(
		$values[3] * $values[3] + $values[4] * $values[4] +
		$values[5] * $values[5] + $values[6] * $values[6])
	if (0 -eq $placementId -or -not $validIdDomain -or
		$quaternionLength -lt 0.000001 -or
		[math]::Abs($values[7]) -lt 0.000001 -or
		[math]::Abs($values[8]) -lt 0.000001 -or
		[math]::Abs($values[9]) -lt 0.000001) {
		throw "Placement transform or ID domain is invalid in $Context"
	}
    return [pscustomobject]@{
		PlacementId = $placementId
		SourcePlacementId = $match.Groups['source'].Value
        AssetId = $match.Groups['asset'].Value
        Row = $Row
    }
}

function Assert-ImportedLeaf {
	param([string]$Name, [string]$Extension)
	if ([IO.Path]::GetFileName($Name) -ne $Name -or
		[IO.Path]::GetExtension($Name) -ne $Extension) {
		throw "Shard path must be a leaf $Extension filename: $Name"
	}
	$fullPath = [IO.Path]::GetFullPath((Join-Path $importRoot $Name))
	if ([IO.Path]::GetDirectoryName($fullPath) -ne
		[IO.Path]::GetFullPath($importRoot).TrimEnd('\')) {
		throw "Shard path escapes the imported map root: $Name"
	}
	return $fullPath
}

function Invoke-FileSetTransaction {
    param([object[]]$Files)
    $transactionId = [Guid]::NewGuid().ToString('N')
    $stagingRoot = Join-Path $runtimeRoot ".map-publish.staging.$AreaId.$transactionId"
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    $entries = [Collections.Generic.List[object]]::new()
    try {
        foreach ($file in $Files) {
            $staged = Join-Path $stagingRoot $file.Name
            [IO.File]::WriteAllText(
                $staged,
                (([string[]]$file.Lines -join "`n") + "`n"),
                $utf8)
            $entries.Add([ordered]@{
                Staged = $staged
                Destination = Join-Path $runtimeRoot $file.Name
                Rollback = Join-Path $runtimeRoot ".$($file.Name).rollback.$transactionId"
                HadPrevious = $false
                Promoted = $false
            })
        }

        $promotedCount = 0
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Destination)) {
                [IO.File]::Move($entry.Destination, $entry.Rollback)
                $entry.HadPrevious = $true
            }
            [IO.File]::Move($entry.Staged, $entry.Destination)
            $entry.Promoted = $true
            ++$promotedCount
            if ($FailureAfterPromote -eq $promotedCount) {
                throw "Injected map publish failure after $promotedCount promotion(s)."
            }
        }
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Rollback)) {
                [IO.File]::Delete($entry.Rollback)
            }
        }
    }
    catch {
        $failure = $_
        for ($index = $entries.Count - 1; $index -ge 0; --$index) {
            $entry = $entries[$index]
            if ($entry.Promoted -and [IO.File]::Exists($entry.Destination)) {
                [IO.File]::Delete($entry.Destination)
            }
            if ($entry.HadPrevious -and [IO.File]::Exists($entry.Rollback)) {
                [IO.File]::Move($entry.Rollback, $entry.Destination)
            }
        }
        throw $failure
    }
    finally {
        if ([IO.Directory]::Exists($stagingRoot)) {
            Remove-Item -LiteralPath $stagingRoot -Recurse -Force
        }
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Rollback)) {
                [IO.File]::Delete($entry.Rollback)
            }
        }
    }
}

if (-not [IO.File]::Exists($authoringPath)) {
    throw "Authoring placement is missing: $authoringPath"
}
if (-not [IO.Directory]::Exists($importRoot)) {
    throw "Imported map source is missing: $importRoot"
}
[IO.Directory]::CreateDirectory($runtimeRoot) | Out-Null
$authoringRows = @(Read-PlacementDocument $authoringPath)

if (-not [IO.File]::Exists($sourceShardSetPath)) {
    if (-not [IO.File]::Exists($sourceCatalogPath)) {
        throw "Imported map catalog is missing: $sourceCatalogPath"
    }
    $lines = @("LOSTARK_MAP_PLACEMENTS 2 `"$AreaId`" $($authoringRows.Count)") + $authoringRows
    $files = [Collections.Generic.List[object]]::new()
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapassets"
        Lines = [IO.File]::ReadAllLines($sourceCatalogPath, [Text.Encoding]::UTF8)
    })
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapplacements"
        Lines = $lines
    })

    $hasDeployCatalog = [IO.File]::Exists($sourceDeployCatalogPath)
    $hasDeployPlacements = [IO.File]::Exists($authoringDeployPath)
    if ($hasDeployCatalog -ne $hasDeployPlacements) {
        throw "Deploy authoring pair is incomplete for $AreaId"
    }
    if ($hasDeployCatalog) {
        $files.Add([pscustomobject]@{
            Name = "$AreaId.deployassets"
            Lines = [IO.File]::ReadAllLines($sourceDeployCatalogPath, [Text.Encoding]::UTF8)
        })
        $files.Add([pscustomobject]@{
            Name = "$AreaId.deployplacements"
            Lines = [IO.File]::ReadAllLines($authoringDeployPath, [Text.Encoding]::UTF8)
        })
    }

    Invoke-FileSetTransaction $files
    [pscustomobject]@{
        AreaId = $AreaId
        CatalogType = 'single'
        PlacementCount = $authoringRows.Count
        RuntimePath = $runtimePath
        Sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $runtimePath).Hash.ToLowerInvariant()
    }
    return
}

$mapSetLines = @([IO.File]::ReadAllLines($sourceShardSetPath, [Text.Encoding]::UTF8))
$mapSetHeader = [regex]::Match(
    $mapSetLines[0],
    '^LOSTARK_MAP_SHARD_SET\s+1\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
if (-not $mapSetHeader.Success -or
    $mapSetHeader.Groups['area'].Value -ne $AreaId -or
    [uint32]$mapSetHeader.Groups['count'].Value -ne ($mapSetLines.Count - 1)) {
    throw "Shard set header is invalid: $sourceShardSetPath"
}

$shards = [Collections.Generic.List[object]]::new()
$assetShards = @{}
$existingPlacementShard = @{}
foreach ($line in @($mapSetLines | Select-Object -Skip 1)) {
    $match = [regex]::Match(
        $line,
        '^"(?<id>[A-Za-z0-9_.-]+)"\s+"(?<catalog>[^"]+)"\s+"(?<placements>[^"]+)"\s+(?<assets>[0-9]+)\s+(?<count>[0-9]+)$')
    if (-not $match.Success) {
        throw "Invalid shard-set row: $line"
    }
    $shard = [pscustomobject]@{
        Id = $match.Groups['id'].Value
        CatalogName = $match.Groups['catalog'].Value
        PlacementName = $match.Groups['placements'].Value
        AssetCount = [uint32]$match.Groups['assets'].Value
        Rows = [Collections.Generic.List[string]]::new()
        CatalogLines = @()
    }
	$catalogPath = Assert-ImportedLeaf $shard.CatalogName '.mapassets'
	$placementPath = Assert-ImportedLeaf $shard.PlacementName '.mapplacements'
    if (-not [IO.File]::Exists($catalogPath) -or
        -not [IO.File]::Exists($placementPath)) {
        throw "Shard source is missing: $($shard.Id)"
    }
    $catalogLines = @([IO.File]::ReadAllLines($catalogPath, [Text.Encoding]::UTF8))
    $shard.CatalogLines = $catalogLines
    $assetIds = @($catalogLines | Select-Object -Skip 1 | ForEach-Object {
        $assetMatch = [regex]::Match($_, '^"(?<id>[A-Za-z0-9_.:-]+)"\s+')
        if (-not $assetMatch.Success) { throw "Invalid asset catalog row: $catalogPath" }
        $assetMatch.Groups['id'].Value
    })
    if ($assetIds.Count -ne $shard.AssetCount) {
        throw "Shard asset count mismatch: $($shard.Id)"
    }
    foreach ($assetId in $assetIds) {
        if (-not $assetShards.ContainsKey($assetId)) {
            $assetShards[$assetId] = [Collections.Generic.List[string]]::new()
        }
        $assetShards[$assetId].Add($shard.Id)
    }
    foreach ($existingRow in @(Read-PlacementDocument $placementPath)) {
        $parsed = Parse-PlacementRow $existingRow $placementPath
        if ($existingPlacementShard.ContainsKey($parsed.PlacementId)) {
            throw "Duplicate placement ID across current shards: $($parsed.PlacementId)"
        }
        $existingPlacementShard[$parsed.PlacementId] = $shard.Id
    }
    $shards.Add($shard)
}

$shardsById = @{}
foreach ($shard in $shards) { $shardsById[$shard.Id] = $shard }
$seenPlacementIds = [Collections.Generic.HashSet[uint64]]::new()
$seenSourcePlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($row in $authoringRows) {
    $parsed = Parse-PlacementRow $row $authoringPath
	if (-not $seenPlacementIds.Add($parsed.PlacementId) -or
		-not $seenSourcePlacementIds.Add($parsed.SourcePlacementId)) {
		throw "Duplicate authoring placement ID: $($parsed.PlacementId)"
    }
    if (-not $assetShards.ContainsKey($parsed.AssetId)) {
        throw "Authoring placement references an asset outside the shard set: $($parsed.AssetId)"
    }
    if ($existingPlacementShard.ContainsKey($parsed.PlacementId)) {
        $targetShardId = $existingPlacementShard[$parsed.PlacementId]
        if ($targetShardId -notin @($assetShards[$parsed.AssetId])) {
            throw "Existing placement changed to an asset unavailable in its shard: $($parsed.PlacementId)"
        }
    }
    else {
        $candidates = @($assetShards[$parsed.AssetId] | Sort-Object)
        $targetShardId = $candidates[[int]($parsed.PlacementId % [uint64]$candidates.Count)]
    }
    $shardsById[$targetShardId].Rows.Add($parsed.Row)
}

$files = [Collections.Generic.List[object]]::new()
$newMapSetLines = [Collections.Generic.List[string]]::new()
$newMapSetLines.Add("LOSTARK_MAP_SHARD_SET 1 `"$AreaId`" $($shards.Count)")
foreach ($shard in $shards) {
    $placementLines = [Collections.Generic.List[string]]::new()
    $placementLines.Add("LOSTARK_MAP_PLACEMENTS 2 `"$AreaId`" $($shard.Rows.Count)")
    foreach ($row in $shard.Rows) { $placementLines.Add($row) }
    $files.Add([pscustomobject]@{
        Name = $shard.PlacementName
        Lines = $placementLines
    })
    $files.Add([pscustomobject]@{
        Name = $shard.CatalogName
        Lines = $shard.CatalogLines
    })
    $newMapSetLines.Add(
        "`"$($shard.Id)`" `"$($shard.CatalogName)`" `"$($shard.PlacementName)`" $($shard.AssetCount) $($shard.Rows.Count)")
}
$files.Add([pscustomobject]@{
    Name = "$AreaId.mapset"
    Lines = $newMapSetLines
})
Invoke-FileSetTransaction $files

[pscustomobject]@{
    AreaId = $AreaId
    CatalogType = 'shard-set'
    PlacementCount = $authoringRows.Count
    ShardCount = $shards.Count
    RuntimePath = $shardSetPath
    Sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $shardSetPath).Hash.ToLowerInvariant()
}
