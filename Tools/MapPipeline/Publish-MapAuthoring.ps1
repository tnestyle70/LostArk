[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9_.-]+$')]
    [string]$AreaId,

    [string]$ProjectRoot,

    [ValidateRange(0, 65537)]
    [int]$FailureAfterPromote = 0,

    [ValidateSet('Validate', 'Check', 'Publish')]
    [string]$Mode = 'Publish'
)

$ErrorActionPreference = 'Stop'
if ($Mode -ne 'Publish' -and $FailureAfterPromote -ne 0) {
    throw 'FailureAfterPromote is only supported in Publish mode.'
}
if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}
$ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
$authoringPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapplacements"
$authoringDeployPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.deployplacements"
$authoringLightPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.maplights.json"
$authoringEffectPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapeffects.json"
$authoringWaterPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.mapwater.json"
$authoringSequencePath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.worldsequences.json"
$authoringCameraShotPath = Join-Path $ProjectRoot "Data\Maps\Authoring\$AreaId\$AreaId.camerashots.json"
$importRoot = Join-Path $ProjectRoot "Data\Maps\Imported\$AreaId"
$sourceCatalogPath = Join-Path $importRoot "$AreaId.mapassets"
$sourceShardSetPath = Join-Path $importRoot "$AreaId.mapset"
$sourceDeployCatalogPath = Join-Path $importRoot "$AreaId.deployassets"
$runtimeRoot = Join-Path $ProjectRoot 'Client\Bin\DataFiles\Map'
$runtimePath = Join-Path $runtimeRoot "$AreaId.mapplacements"
$runtimeLightPath = Join-Path $runtimeRoot "$AreaId.maplights.json"
$runtimeEffectPath = Join-Path $runtimeRoot "$AreaId.mapeffects.json"
$runtimeWaterPath = Join-Path $runtimeRoot "$AreaId.mapwater.json"
$runtimeSequencePath = Join-Path $runtimeRoot "$AreaId.worldsequences.json"
$runtimeCameraShotPath = Join-Path $runtimeRoot "$AreaId.camerashots.json"
$mapCatalogPath = Join-Path $ProjectRoot 'Data\Maps\MapCatalog.json'
$worldDestructionPath = Join-Path $ProjectRoot "Client\Bin\DataFiles\World\$AreaId.worlddestruction.json"
$worldDestructionSourcePath = Join-Path $ProjectRoot 'Data\Encounters\Valtan\ValtanWorldEvents.json'
$effectCatalogPath = Join-Path $ProjectRoot 'Data\Effects\EffectCatalog.json'
$valtanEncounterPath = Join-Path $ProjectRoot 'Data\Encounters\Valtan\ValtanEncounter.json'
$runtimeResourceRoot = Join-Path $ProjectRoot 'Client\Bin\Resources'
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
    if (-not $header.Success -or $header.Groups['area'].Value -cne $AreaId) {
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
			[double]::IsNaN($value) -or [double]::IsInfinity($value) -or
			[math]::Abs($value) -gt [single]::MaxValue) {
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

function Read-MapAssetCatalog {
    param([string]$Path)
    $lines = @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
    $header = if ($lines.Count -gt 0) {
        [regex]::Match($lines[0],
            '^LOSTARK_MAP_ASSET_CATALOG\s+[1-4]\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
    }
    if ($null -eq $header -or -not $header.Success -or
        $header.Groups['area'].Value -cne $AreaId -or
        [uint32]$header.Groups['count'].Value -ne ($lines.Count - 1) -or
        $lines.Count -lt 2 -or $lines.Count -gt 513) {
        throw "Asset catalog header/count is invalid: $Path"
    }
    $assetIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $resourcePrefix = [IO.Path]::GetFullPath($runtimeResourceRoot).TrimEnd('\') + '\'
    foreach ($row in @($lines | Select-Object -Skip 1)) {
        $match = [regex]::Match($row,
            '^"(?<id>[A-Za-z0-9_.:-]+)"\s+"(?:[^"\\\r\n]|\\.)+"\s+"(?<model>(?:[^"\\\r\n]|\\.)+)"\s+')
        if (-not $match.Success) { throw "Invalid asset catalog row: $Path" }
        # Match std::quoted's escape handling before checking the runtime path.
        $modelPath = [regex]::Replace($match.Groups['model'].Value, '\\(.)', '$1')
        if ([IO.Path]::IsPathRooted($modelPath) -or $modelPath.Contains(':') -or
            '..' -in @($modelPath -split '[\\/]') -or
            -not [IO.Path]::GetFullPath((Join-Path $runtimeResourceRoot $modelPath)).StartsWith(
                $resourcePrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Asset model path must stay Resources-relative: $modelPath"
        }
        if (-not $assetIds.Add($match.Groups['id'].Value)) {
            throw "Duplicate asset ID in catalog: $Path"
        }
    }
    return [pscustomobject]@{ Lines = $lines; AssetIds = $assetIds }
}

function Assert-ImportedLeaf {
	param([string]$Name, [string]$Extension)
	if ($Name.Length -gt 260 -or $Name -notmatch '^[A-Za-z0-9_.-]+$' -or
		[IO.Path]::GetFileName($Name) -ne $Name -or
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

function Test-JsonNumber {
    param([object]$Value)
    if ($Value -is [bool] -or $null -eq $Value) { return $false }
    if ($Value -isnot [byte] -and $Value -isnot [sbyte] -and
        $Value -isnot [int16] -and $Value -isnot [uint16] -and
        $Value -isnot [int32] -and $Value -isnot [uint32] -and
        $Value -isnot [int64] -and $Value -isnot [uint64] -and
        $Value -isnot [single] -and $Value -isnot [double] -and
        $Value -isnot [decimal]) {
        return $false
    }
    $number = [double]$Value
    return -not [double]::IsNaN($number) -and
        -not [double]::IsInfinity($number)
}

function Assert-ExactJsonProperties {
    param([object]$Value, [string[]]$Expected, [string]$Context)
    if ($null -eq $Value) { throw "$Context is null" }
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if ($actual.Count -ne $expectedSorted.Count -or
        (Compare-Object $actual $expectedSorted)) {
        throw "$Context has unexpected properties"
    }
}

function Split-DeployAuthoringTokens {
    param([string]$Line, [string]$Context)
    if ($null -eq $Line -or $Line.Length -eq 0) {
        throw "Deploy authoring row is empty: $Context"
    }
    $pattern = '"(?:\\.|[^"\\\r\n])*"|[^\s"]+'
    $matches = [regex]::Matches($Line, $pattern)
    $residue = [regex]::Replace($Line, $pattern, '')
    if ($matches.Count -eq 0 -or $residue -match '\S') {
        throw "Deploy authoring row has malformed quoting: $Context"
    }
    $tokens = [Collections.Generic.List[string]]::new()
    foreach ($match in $matches) {
        $raw = [string]$match.Value
        if ($raw.StartsWith('"') -and $raw.EndsWith('"')) {
            $body = $raw.Substring(1, $raw.Length - 2)
            $tokens.Add([regex]::Replace($body, '\\(.)', '$1'))
        }
        else {
            $tokens.Add($raw)
        }
    }
    return $tokens.ToArray()
}

function Convert-DeployUInt64 {
    param([string]$Value, [string]$Context, [bool]$AllowZero = $false)
    $parsed = [uint64]0
    if ($Value -cnotmatch '^[0-9]{1,20}$' -or
        -not [uint64]::TryParse(
            $Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed) -or (-not $AllowZero -and 0 -eq $parsed)) {
        throw "Deploy unsigned integer is invalid: $Context"
    }
    return $parsed
}

function Convert-DeployUInt32 {
    param([string]$Value, [string]$Context, [bool]$AllowZero = $true)
    $parsed = [uint32]0
    if ($Value -cnotmatch '^[0-9]{1,10}$' -or
        -not [uint32]::TryParse(
            $Value,
            [Globalization.NumberStyles]::None,
            [Globalization.CultureInfo]::InvariantCulture,
            [ref]$parsed) -or (-not $AllowZero -and 0 -eq $parsed)) {
        throw "Deploy unsigned integer is invalid: $Context"
    }
    return $parsed
}

function Convert-DeployFiniteNumber {
    param([string]$Value, [string]$Context)
    $parsed = 0.0
    if (-not [double]::TryParse(
        $Value,
        [Globalization.NumberStyles]::Float,
        [Globalization.CultureInfo]::InvariantCulture,
        [ref]$parsed) -or [double]::IsNaN($parsed) -or
        [double]::IsInfinity($parsed) -or
        [math]::Abs($parsed) -gt [single]::MaxValue) {
        throw "Deploy floating-point value is invalid: $Context"
    }
    return $parsed
}

function Assert-DeployWModelPath {
    param([string]$Value, [string]$Context)
    if ([string]::IsNullOrWhiteSpace($Value) -or
        [IO.Path]::IsPathRooted($Value) -or $Value.Contains(':') -or
        '..' -in @($Value -split '[\\/]') -or
        [IO.Path]::GetExtension($Value) -cne '.wmodel') {
        throw "Deploy model path must be a Resources-relative WModel: $Context"
    }
}

function Read-DeployAuthoringPair {
    $catalogLines = @([IO.File]::ReadAllLines(
        $sourceDeployCatalogPath, [Text.Encoding]::UTF8))
    if ($catalogLines.Count -lt 2) {
        throw "Deploy catalog is empty: $sourceDeployCatalogPath"
    }
    $catalogHeader = @(Split-DeployAuthoringTokens `
        $catalogLines[0] "$sourceDeployCatalogPath header")
    if ($catalogHeader.Count -ne 4 -or
        $catalogHeader[0] -cne 'LOSTARK_DEPLOY_PROP_CATALOG' -or
        $catalogHeader[1] -notin @('2','3') -or
        $catalogHeader[2] -cne $AreaId) {
        throw "Deploy catalog header is invalid: $sourceDeployCatalogPath"
    }
    $catalogVersion = [uint32]$catalogHeader[1]
    $assetCount = Convert-DeployUInt32 $catalogHeader[3] `
        "$sourceDeployCatalogPath asset count" $false
    if ($assetCount -gt 64 -or $assetCount -ne ($catalogLines.Count - 1)) {
        throw "Deploy catalog count is invalid: $sourceDeployCatalogPath"
    }

    $assets = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::Ordinal)
    $prototypeTags = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    for ($index = 0; $index -lt $assetCount; ++$index) {
        $context = "$sourceDeployCatalogPath row $index"
        $tokens = @(Split-DeployAuthoringTokens $catalogLines[$index + 1] $context)
        $expectedCount = if ($catalogVersion -eq 3) { 12 } else { 10 }
        if ($tokens.Count -ne $expectedCount -or
            $tokens[0] -cnotmatch '^[A-Za-z0-9_.:-]{1,160}$' -or
            $tokens[1] -notin @('STATIC','ANIM') -or
            [string]::IsNullOrWhiteSpace($tokens[2]) -or
            [string]::IsNullOrWhiteSpace($tokens[4]) -or
            [string]::IsNullOrWhiteSpace($tokens[9]) -or
            $assets.ContainsKey($tokens[0]) -or
            -not $prototypeTags.Add($tokens[4])) {
            throw "Deploy catalog identity is invalid or duplicated: $context"
        }
        Assert-DeployWModelPath $tokens[3] "$context intact model"
        $declaresFractured = $tokens[5].Length -ne 0 -or $tokens[6].Length -ne 0
        if ($declaresFractured) {
            if ($tokens[1] -ne 'STATIC' -or $tokens[5].Length -eq 0 -or
                $tokens[6].Length -eq 0 -or -not $prototypeTags.Add($tokens[6])) {
                throw "Deploy fractured model pair is invalid: $context"
            }
            Assert-DeployWModelPath $tokens[5] "$context fractured model"
        }
        $emissive = Convert-DeployFiniteNumber $tokens[7] "$context emissive"
        if ($emissive -lt 0.0 -or $tokens[8] -notin @('0','1') -or
            ($tokens[8] -eq '1' -and $tokens[1] -ne 'STATIC')) {
            throw "Deploy catalog presentation fields are invalid: $context"
        }
        $intactClip = if ($catalogVersion -eq 3) { $tokens[10] } `
            elseif ($tokens[1] -eq 'ANIM') { 'on' } else { '' }
        $fracturedClip = if ($catalogVersion -eq 3) { $tokens[11] } `
            elseif ($tokens[1] -eq 'ANIM') { 'off' } else { '' }
        if (($tokens[1] -eq 'STATIC' -and
                ($intactClip.Length -ne 0 -or $fracturedClip.Length -ne 0)) -or
            $intactClip.Length -gt 256 -or $fracturedClip.Length -gt 256 -or
            $intactClip -match '[\x00-\x1f]' -or
            $fracturedClip -match '[\x00-\x1f]') {
            throw "Deploy animation roles are invalid: $context"
        }
        $assets.Add($tokens[0], [pscustomobject]@{
            Kind = $tokens[1]
            DeferredEmissiveOverlay = $tokens[8] -eq '1'
            IntactClip = $intactClip
            FracturedClip = $fracturedClip
        })
    }

    $placementLines = @([IO.File]::ReadAllLines(
        $authoringDeployPath, [Text.Encoding]::UTF8))
    if ($placementLines.Count -lt 1) {
        throw "Deploy placement document is empty: $authoringDeployPath"
    }
    $placementHeader = @(Split-DeployAuthoringTokens `
        $placementLines[0] "$authoringDeployPath header")
    if ($placementHeader.Count -ne 4 -or
        $placementHeader[0] -cne 'LOSTARK_DEPLOY_PROP_PLACEMENTS' -or
        $placementHeader[1] -notin @('1','2') -or
        $placementHeader[2] -cne $AreaId) {
        throw "Deploy placement header is invalid: $authoringDeployPath"
    }
    $placementVersion = [uint32]$placementHeader[1]
    $placementCount = Convert-DeployUInt32 $placementHeader[3] `
        "$authoringDeployPath placement count" $true
    if ($placementCount -gt 4096 -or
        $placementCount -ne ($placementLines.Count - 1)) {
        throw "Deploy placement count is invalid: $authoringDeployPath"
    }

    $placements = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::Ordinal)
    for ($index = 0; $index -lt $placementCount; ++$index) {
        $context = "$authoringDeployPath row $index"
        $tokens = @(Split-DeployAuthoringTokens $placementLines[$index + 1] $context)
        $expectedCount = if ($placementVersion -eq 2) { 17 } else { 16 }
        if ($tokens.Count -ne $expectedCount) {
            throw "Deploy placement row width is invalid: $context"
        }
        $runtimeId = Convert-DeployUInt64 $tokens[0] "$context runtime ID"
        $deployActorId = Convert-DeployUInt32 $tokens[1] "$context actor ID" $true
        $definitionId = Convert-DeployUInt32 $tokens[2] "$context definition ID" $true
        if ([string]::IsNullOrWhiteSpace($tokens[3]) -or
            $tokens[3].Length -gt 256 -or -not $assets.ContainsKey($tokens[4]) -or
            $placements.ContainsKey($tokens[0])) {
            throw "Deploy placement identity is invalid or duplicated: $context"
        }
        $numeric = for ($field = 5; $field -le 12; ++$field) {
            Convert-DeployFiniteNumber $tokens[$field] "$context field $field"
        }
        $quaternionLength = [math]::Sqrt(
            $numeric[3] * $numeric[3] + $numeric[4] * $numeric[4] +
            $numeric[5] * $numeric[5] + $numeric[6] * $numeric[6])
        if ($quaternionLength -le 0.000001 -or $numeric[7] -le 0.000001 -or
            $tokens[13] -notin @('0','1')) {
            throw "Deploy placement transform is invalid: $context"
        }
        $stateOffAction = Convert-DeployUInt32 $tokens[14] `
            "$context state-off action" $true
        $occurrenceCount = Convert-DeployUInt32 $tokens[15] `
            "$context occurrence count" $true
        $provenance = if ($placementVersion -eq 2) { $tokens[16] } `
            else { 'SOURCE_EXACT' }
        if ($provenance -eq 'SOURCE_EXACT') {
            if (0 -eq $deployActorId -or 0 -eq $definitionId) {
                throw "SOURCE_EXACT Deploy placement lacks source IDs: $context"
            }
        }
        elseif ($provenance -eq 'PROJECT_AUTHORED') {
            if ($placementVersion -ne 2 -or 0 -ne $deployActorId -or
                0 -ne $definitionId -or 0 -ne $stateOffAction -or
                0 -ne $occurrenceCount) {
                throw "PROJECT_AUTHORED Deploy placement impersonates source data: $context"
            }
        }
        else {
            throw "Deploy placement provenance is invalid: $context"
        }
        $placements.Add($tokens[0], [pscustomobject]@{
            AssetId = $tokens[4]
            Destructible = $tokens[13] -eq '1'
            Provenance = $provenance
        })
    }
    return [pscustomobject]@{
        CatalogLines = $catalogLines
        PlacementLines = $placementLines
        Assets = $assets
        Placements = $placements
    }
}

function Read-MapLightDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map light JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','provenance','lights') `
        'Map light root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-light-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        $document.provenance -isnot [string] -or
        $document.provenance -notin @(
            'SOURCE_EXACT',
            'SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED',
            'PROJECT_AUTHORED')) {
        throw "Map light header is invalid: $Path"
    }
    if ($document.lights -isnot [System.Array]) {
        throw "Map light lights property must be an array: $Path"
    }
    $lights = @($document.lights)
    if ($lights.Count -lt 1 -or $lights.Count -gt 64) {
        throw "Map light count is invalid: $Path"
    }
    $lightIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $sourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($light in $lights) {
        Assert-ExactJsonProperties $light `
            @('lightId','sourceLevel','sourceObjectId','position','radiusMeters',
              'falloffExponent','color','brightness') 'Map point light'
        if ($light.lightId -isnot [string] -or
            $light.lightId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            $light.sourceLevel -isnot [string] -or
            $light.sourceLevel -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            $light.sourceObjectId -isnot [string] -or
            $light.sourceObjectId -notmatch '^[A-Za-z0-9._:-]{1,256}$' -or
            -not $lightIds.Add($light.lightId) -or
            -not $sourceIds.Add($light.sourceObjectId)) {
            throw "Map point light identity is invalid or duplicated: $Path"
        }
        if ($light.position -isnot [System.Array] -or
            $light.color -isnot [System.Array]) {
            throw "Map point light vectors must be arrays: $($light.lightId)"
        }
        $position = @($light.position)
        $color = @($light.color)
        if ($position.Count -ne 3 -or $color.Count -ne 4) {
            throw "Map point light vector width is invalid: $($light.lightId)"
        }
        foreach ($component in $position) {
            if (-not (Test-JsonNumber $component) -or
                [double]$component -lt -100000.0 -or
                [double]$component -gt 100000.0) {
                throw "Map point light position is invalid: $($light.lightId)"
            }
        }
        foreach ($component in $color) {
            if (-not (Test-JsonNumber $component) -or
                [double]$component -lt 0.0 -or [double]$component -gt 1.0) {
                throw "Map point light color is invalid: $($light.lightId)"
            }
        }
        if (-not (Test-JsonNumber $light.radiusMeters) -or
            [double]$light.radiusMeters -lt 0.01 -or
            [double]$light.radiusMeters -gt 1000.0 -or
            -not (Test-JsonNumber $light.falloffExponent) -or
            [double]$light.falloffExponent -lt 0.01 -or
            [double]$light.falloffExponent -gt 64.0 -or
            -not (Test-JsonNumber $light.brightness) -or
            [double]$light.brightness -lt 0.0 -or
            [double]$light.brightness -gt 64.0) {
            throw "Map point light scalar is invalid: $($light.lightId)"
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-MapLightPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapLightsDeclared) {
        if (-not [IO.File]::Exists($authoringLightPath)) {
            throw "Declared map light authoring source is missing: $authoringLightPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.maplights.json"
            Lines = Read-MapLightDocument $authoringLightPath
        })
    }
}

function Get-MapPublishSha256 {
    param([string]$Path)
    $stream = [IO.File]::OpenRead($Path)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString($algorithm.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $algorithm.Dispose()
        $stream.Dispose()
    }
}

function Read-MapEffectDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map Effect JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','presentations') `
        'Map Effect root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-effect-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        $document.presentations -isnot [System.Array]) {
        throw "Map Effect header is invalid: $Path"
    }
    $presentations = @($document.presentations)
    if ($presentations.Count -lt 1 -or $presentations.Count -gt 64) {
        throw "Map Effect presentation count is invalid: $Path"
    }
    $independentIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $worldPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $surfacePlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($presentation in $presentations) {
        if ($presentation.independentEffectId -isnot [string] -or
            $presentation.independentEffectId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            -not $independentIds.Add($presentation.independentEffectId) -or
            $presentation.displayName -isnot [string] -or
            [string]::IsNullOrWhiteSpace($presentation.displayName) -or
            $presentation.displayName.Length -gt 160 -or
            $presentation.presentationKind -isnot [string]) {
            throw "Map Effect identity is invalid or duplicated: $Path"
        }
        if ($presentation.presentationKind -eq 'DEPLOY_SURFACE_OVERLAY') {
            Assert-ExactJsonProperties $presentation `
                @('independentEffectId','displayName','presentationKind','owners',
                  'visibleStates','materialIndex','emissiveIntensity',
                  'emissiveColor','maskPower') 'Map Effect surface row'
            if ($presentation.owners -isnot [System.Array] -or
                $presentation.visibleStates -isnot [System.Array]) {
                throw "Map Effect surface arrays are invalid: $Path"
            }
            $owners = @($presentation.owners)
            $states = @($presentation.visibleStates)
            if ($owners.Count -lt 1 -or $owners.Count -gt 256 -or
                $states.Count -ne 1 -or $states[0] -isnot [string] -or
                $states[0] -cne 'INTACT' -or
                -not (Test-JsonNumber $presentation.materialIndex) -or
                [double]$presentation.materialIndex -lt 0 -or
                [double]$presentation.materialIndex -gt 255 -or
                [math]::Floor([double]$presentation.materialIndex) -ne
                    [double]$presentation.materialIndex -or
                -not (Test-JsonNumber $presentation.emissiveIntensity) -or
                [double]$presentation.emissiveIntensity -lt 0 -or
                [double]$presentation.emissiveIntensity -gt 64 -or
                -not (Test-JsonNumber $presentation.maskPower) -or
                [double]$presentation.maskPower -lt 0.01 -or
                [double]$presentation.maskPower -gt 32) {
                throw "Map Effect surface values are invalid: $Path"
            }
            $color = @($presentation.emissiveColor)
            if ($presentation.emissiveColor -isnot [System.Array] -or
                $color.Count -ne 4 -or
                @($color | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt 0 -or [double]$_ -gt 16
                }).Count -gt 0) {
                throw "Map Effect surface color is invalid: $Path"
            }
            $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($owner in $owners) {
                Assert-ExactJsonProperties $owner @('groupId','placementId') 'Map Effect surface owner'
                if ($owner.groupId -isnot [string] -or
                    $owner.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                    -not $groupIds.Add($owner.groupId) -or
                    $owner.placementId -isnot [string] -or
                    $owner.placementId -notmatch '^[1-9][0-9]{0,19}$' -or
                    -not $surfacePlacementIds.Add($owner.placementId)) {
                    throw "Map Effect surface owner is invalid or duplicated: $Path"
                }
                $parsedPlacementId = [uint64]0
                if (-not [uint64]::TryParse(
                    $owner.placementId,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedPlacementId) -or 0 -eq $parsedPlacementId) {
                    throw "Map Effect surface placement ID overflows uint64: $($owner.placementId)"
                }
            }
            continue
        }
        if ($presentation.presentationKind -eq 'EFFECT_DOCUMENT') {
            Assert-ExactJsonProperties $presentation `
                @('independentEffectId','displayName','presentationKind','placementId',
                  'effectAssetId','position','rotationQuaternion','scale',
                  'orientationPolicy','activationPolicy','activationSetId',
                  'activationWindows','playbackPolicy') 'Map Effect world row'
            if ($presentation.placementId -isnot [string] -or
                $presentation.placementId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                -not $worldPlacementIds.Add($presentation.placementId) -or
                $presentation.effectAssetId -isnot [string] -or
                $presentation.effectAssetId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                $presentation.orientationPolicy -notin @('WORLD','CAMERA_FACING_WORLD') -or
                $presentation.activationPolicy -notin @('LEVEL_ACTIVE','SERVER_PATTERN_WINDOW') -or
                $presentation.playbackPolicy -notin @('LOCAL_LOOP','SERVER_CLOCK_SAMPLE') -or
                (($presentation.activationPolicy -eq 'SERVER_PATTERN_WINDOW') -ne
                 ($presentation.playbackPolicy -eq 'SERVER_CLOCK_SAMPLE'))) {
                throw "Map Effect world identity/policy is invalid: $Path"
            }
            $position = @($presentation.position)
            $rotation = @($presentation.rotationQuaternion)
            $scale = @($presentation.scale)
            if ($presentation.position -isnot [System.Array] -or
                $presentation.rotationQuaternion -isnot [System.Array] -or
                $presentation.scale -isnot [System.Array] -or
                $position.Count -ne 3 -or $rotation.Count -ne 4 -or $scale.Count -ne 3 -or
                @($position | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt -100000 -or [double]$_ -gt 100000
                }).Count -gt 0 -or
                @($rotation | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt -1 -or [double]$_ -gt 1
                }).Count -gt 0 -or
                @($scale | Where-Object {
                    -not (Test-JsonNumber $_) -or [double]$_ -lt 0.001 -or [double]$_ -gt 1000
                }).Count -gt 0) {
                throw "Map Effect world transform is invalid: $Path"
            }
            $quaternionLengthSquared = 0.0
            foreach ($component in $rotation) {
                $quaternionLengthSquared += [double]$component * [double]$component
            }
            if ($quaternionLengthSquared -lt 0.999 -or $quaternionLengthSquared -gt 1.001) {
                throw "Map Effect world quaternion is not normalized: $Path"
            }
            if ($presentation.activationWindows -isnot [System.Array]) {
                throw "Map Effect activation windows must be an array: $Path"
            }
            $windows = @($presentation.activationWindows)
            if ($presentation.activationPolicy -eq 'LEVEL_ACTIVE') {
                if (-not [string]::IsNullOrEmpty($presentation.activationSetId) -or
                    $windows.Count -ne 0) {
                    throw "LEVEL_ACTIVE Map Effect cannot declare a Server set: $Path"
                }
            }
            elseif ($presentation.activationSetId -isnot [string] -or
                $presentation.activationSetId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
                $windows.Count -lt 1 -or $windows.Count -gt 32) {
                throw "Map Effect Server activation set is invalid: $Path"
            }
            $windowKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            $previousOffset = -1
            foreach ($window in $windows) {
                Assert-ExactJsonProperties $window `
                    @('patternId','stageId','effectTimelineOffsetMs') `
                    'Map Effect activation window'
                $windowKey = "$($window.patternId)`n$($window.stageId)"
                if ($window.patternId -isnot [string] -or
                    $window.patternId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
                    $window.stageId -isnot [string] -or
                    $window.stageId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
                    -not $windowKeys.Add($windowKey) -or
                    -not (Test-JsonNumber $window.effectTimelineOffsetMs) -or
                    [double]$window.effectTimelineOffsetMs -lt 0 -or
                    [double]$window.effectTimelineOffsetMs -gt 3600000 -or
                    [math]::Floor([double]$window.effectTimelineOffsetMs) -ne
                        [double]$window.effectTimelineOffsetMs -or
                    [double]$window.effectTimelineOffsetMs -le $previousOffset) {
                    throw "Map Effect activation window is invalid or unordered: $Path"
                }
                $previousOffset = [double]$window.effectTimelineOffsetMs
            }
            continue
        }
        throw "Unsupported Map Effect presentation kind: $($presentation.presentationKind)"
    }
    Assert-MapEffectDomainJoins $document $Path
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Read-MapEffectDeployCatalog {
    if (-not [IO.File]::Exists($sourceDeployCatalogPath)) {
        throw "Map Effect surface rows require the Deploy catalog: $sourceDeployCatalogPath"
    }
    $pair = Read-DeployAuthoringPair
    return ,$pair.Assets
}

function Read-MapEffectDeployPlacements {
    if (-not [IO.File]::Exists($authoringDeployPath)) {
        throw "Map Effect surface rows require Deploy placements: $authoringDeployPath"
    }
    $pair = Read-DeployAuthoringPair
    return ,$pair.Placements
}

function Read-MapEffectWorldDestruction {
    if (-not [IO.File]::Exists($worldDestructionPath)) {
        throw "Map Effect surface rows require the published destruction projection: $worldDestructionPath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $worldDestructionPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "World destruction projection JSON parse failed: $worldDestructionPath" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','combatRuntimeRevision','groups') `
        'World destruction projection root'
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.world-destruction-client-projection' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 3.0 -or
        $document.areaId -isnot [string] -or $document.areaId -cne $AreaId -or
        $document.combatRuntimeRevision -isnot [string] -or
        $document.combatRuntimeRevision -notmatch '^[0-9a-f]{64}$' -or
        $document.groups -isnot [System.Array]) {
        throw "World destruction projection header is invalid: $worldDestructionPath"
    }
    $groups = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.groups)) {
        Assert-ExactJsonProperties $group `
            @('groupId','mutationId','removesGround','suppressionAliasPlacementIds',
              'memberPlacementIds') 'World destruction projection group'
        if ($group.groupId -isnot [string] -or
            $group.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.mutationId -isnot [string] -or
            $group.mutationId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.removesGround -isnot [bool] -or
            $group.memberPlacementIds -isnot [System.Array] -or
            $group.suppressionAliasPlacementIds -isnot [System.Array] -or
            $groups.ContainsKey($group.groupId)) {
            throw "World destruction projection group is invalid or duplicated: $worldDestructionPath"
        }
        $members = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($member in @($group.memberPlacementIds)) {
            $parsedId = [uint64]0
            if ($member -isnot [string] -or $member -notmatch '^[1-9][0-9]{0,19}$' -or
                -not [uint64]::TryParse(
                    $member,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedId) -or 0 -eq $parsedId -or
                -not $members.Add($member) -or -not $ownedMembers.Add($member)) {
                throw "World destruction member is invalid or multiply owned: $($group.groupId)/$member"
            }
        }
        if (0 -eq $members.Count) {
            throw "World destruction projection group has no members: $($group.groupId)"
        }
        $aliases = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($alias in @($group.suppressionAliasPlacementIds)) {
            if ($alias -isnot [string] -or -not $members.Contains($alias) -or
                -not $aliases.Add($alias)) {
                throw "World destruction suppression alias is invalid: $($group.groupId)/$alias"
            }
        }
        $groups.Add($group.groupId, [pscustomobject]@{
            MutationId = [string]$group.mutationId
            RemovesGround = [bool]$group.removesGround
            Members = $members
        })
    }
    return ,$groups
}

function Read-MapEffectWorldDestructionSource {
    if (-not [IO.File]::Exists($worldDestructionSourcePath)) {
        throw "Map Effect surface rows require the destruction source document: $worldDestructionSourcePath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $worldDestructionSourcePath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "World destruction source JSON parse failed: $worldDestructionSourcePath" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','encounterId','provenance','groups',
          'mutations','bindings') 'World destruction source root'
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.world-destruction-events' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -cne $AreaId -or
        $document.encounterId -isnot [string] -or
        $document.encounterId -cne 'ENCOUNTER_VALTAN' -or
        $document.groups -isnot [System.Array] -or
        $document.mutations -isnot [System.Array] -or
        $document.bindings -isnot [System.Array]) {
        throw "World destruction source header is invalid: $worldDestructionSourcePath"
    }
    $mutationsByGroup = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    foreach ($mutation in @($document.mutations)) {
        Assert-ExactJsonProperties $mutation `
            @('mutationId','groupId','targetState','breakingDurationMs') `
            'World destruction source mutation'
        if ($mutation.mutationId -isnot [string] -or
            $mutation.mutationId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $mutation.groupId -isnot [string] -or
            $mutation.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $mutation.targetState -isnot [string] -or
            $mutation.targetState -notin @('FRACTURED','DESPAWNED') -or
            -not (Test-JsonNumber $mutation.breakingDurationMs) -or
            [double]$mutation.breakingDurationMs -lt 0.0 -or
            [math]::Floor([double]$mutation.breakingDurationMs) -ne
                [double]$mutation.breakingDurationMs -or
            $mutationsByGroup.ContainsKey($mutation.groupId)) {
            throw "World destruction source mutation is invalid or duplicated for its group."
        }
        $mutationsByGroup.Add($mutation.groupId, $mutation)
    }
    $groups = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $ownedMembers = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.groups)) {
        Assert-ExactJsonProperties $group `
            @('groupId','memberPlacementIds','navigationRegionIds','navPolarity',
              'initialState') 'World destruction source group'
        if ($group.groupId -isnot [string] -or
            $group.groupId -notmatch '^[A-Za-z0-9._-]{1,160}$' -or
            $group.memberPlacementIds -isnot [System.Array] -or
            $group.navigationRegionIds -isnot [System.Array] -or
            $group.navPolarity -isnot [string] -or
            $group.navPolarity -notin @('BLOCK_WHILE_INTACT','BLOCK_WHILE_FRACTURED') -or
            $group.initialState -isnot [string] -or $group.initialState -cne 'INTACT' -or
            $groups.ContainsKey($group.groupId) -or
            -not $mutationsByGroup.ContainsKey($group.groupId)) {
            throw "World destruction source group is invalid or has no exact mutation: $($group.groupId)"
        }
        $members = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($member in @($group.memberPlacementIds)) {
            $parsedId = [uint64]0
            if ($member -isnot [string] -or $member -notmatch '^[1-9][0-9]{0,19}$' -or
                -not [uint64]::TryParse(
                    $member,
                    [Globalization.NumberStyles]::None,
                    [Globalization.CultureInfo]::InvariantCulture,
                    [ref]$parsedId) -or 0 -eq $parsedId -or
                -not $members.Add($member) -or -not $ownedMembers.Add($member)) {
                throw "World destruction source member is invalid or multiply owned: $($group.groupId)/$member"
            }
        }
        if (0 -eq $members.Count) {
            throw "World destruction source group has no members: $($group.groupId)"
        }
        $mutation = $mutationsByGroup[$group.groupId]
        $groups.Add($group.groupId, [pscustomobject]@{
            MutationId = [string]$mutation.mutationId
            TargetState = [string]$mutation.targetState
            RemovesGround = $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED'
            Members = $members
        })
    }
    return ,$groups
}

function Add-MapEffectRuntimeResourceIds {
    param(
        [object]$Value,
        [string]$Context,
        [Collections.Generic.HashSet[string]]$ResourceIds)
    if ($null -eq $Value) { return }
    if ($Value -is [System.Array]) {
        for ($index = 0; $index -lt $Value.Count; ++$index) {
            Add-MapEffectRuntimeResourceIds $Value[$index] "$Context[$index]" $ResourceIds
        }
        return
    }
    if ($Value -isnot [pscustomobject]) { return }
    foreach ($property in $Value.PSObject.Properties) {
        $field = "$Context.$($property.Name)"
        if ($property.Name -in @('assetId','modelAssetId','textureAssetId') -and
            -not [string]::IsNullOrEmpty([string]$property.Value)) {
            if ($property.Value -isnot [string]) {
                throw "Map Effect runtime resource ID must be a string: $field"
            }
            $resourceId = [string]$property.Value
            if ($resourceId.Contains('\') -or $resourceId.Contains(':') -or
                $resourceId.StartsWith('/') -or
                $resourceId -match '(^|/)\.\.?(/|$)' -or
                $resourceId -notmatch '^[A-Za-z0-9_.\/-]+\.(dds|wmodel)$') {
                throw "Map Effect runtime resource ID is unsafe or unsupported: $field=$resourceId"
            }
            [void]$ResourceIds.Add($resourceId)
        }
        if ($property.Value -is [pscustomobject] -or
            $property.Value -is [System.Array]) {
            Add-MapEffectRuntimeResourceIds $property.Value $field $ResourceIds
        }
    }
}

function Assert-MapEffectRuntimeResourceFiles {
    param([Collections.Generic.HashSet[string]]$ResourceIds, [string]$EffectAssetId)
    if (0 -eq $ResourceIds.Count) {
        throw "Map Effect direct-authored document has no runtime resource binding: $EffectAssetId"
    }
    $root = [IO.Path]::GetFullPath($runtimeResourceRoot)
    $rootPrefix = $root.TrimEnd('\') + '\'
    foreach ($resourceId in $ResourceIds) {
        $relative = $resourceId.Replace('/', [IO.Path]::DirectorySeparatorChar)
        $path = [IO.Path]::GetFullPath((Join-Path $root $relative))
        if (-not $path.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase) -or
            -not [IO.File]::Exists($path)) {
            throw "Map Effect runtime resource file is missing or outside Resources: $resourceId"
        }
        $stream = [IO.File]::OpenRead($path)
        try {
            $magic = [byte[]]::new(4)
            if ($stream.Read($magic, 0, 4) -ne 4) {
                throw "Map Effect runtime resource file is truncated: $resourceId"
            }
            $expected = if ($resourceId.EndsWith('.dds', [StringComparison]::OrdinalIgnoreCase)) {
                [Text.Encoding]::ASCII.GetBytes('DDS ')
            }
            else { [Text.Encoding]::ASCII.GetBytes('WINT') }
            $matchesMagic = $true
            for ($index = 0; $index -lt 4; ++$index) {
                if ($magic[$index] -ne $expected[$index]) {
                    $matchesMagic = $false
                    break
                }
            }
            if (-not $matchesMagic) {
                throw "Map Effect runtime resource file magic is invalid: $resourceId"
            }
        }
        finally { $stream.Dispose() }
    }
}

function Resolve-MapEffectAuthoredDocument {
    param([string]$EffectAssetId)
    if (-not [IO.File]::Exists($effectCatalogPath)) {
        throw "Effect catalog is missing for Map Effect validation: $effectCatalogPath"
    }
    try {
        $catalog = [IO.File]::ReadAllText(
            $effectCatalogPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Effect catalog JSON parse failed: $effectCatalogPath" }
    if (-not (Test-JsonNumber $catalog.formatVersion) -or
        [double]$catalog.formatVersion -ne 1.0 -or
        $catalog.effects -isnot [System.Array]) {
        throw "Effect catalog header is invalid: $effectCatalogPath"
    }
    $matches = @($catalog.effects | Where-Object {
        $_.effectAssetId -is [string] -and $_.effectAssetId -ceq $EffectAssetId
    })
    if ($matches.Count -ne 1) {
        throw "Map Effect target must resolve exactly once in EffectCatalog: $EffectAssetId"
    }
    $entry = $matches[0]
    Assert-ExactJsonProperties $entry `
        @('effectAssetId','payloadKind','authoringPath') `
        'Map Effect direct-authored catalog row'
    $expectedAuthoringPath = "Effects/Authored/$EffectAssetId.effect.json"
    if ($entry.payloadKind -isnot [string] -or
        $entry.payloadKind -cne 'DIRECT_AUTHORED_DOCUMENT' -or
        $entry.authoringPath -isnot [string] -or
        $entry.authoringPath -cne $expectedAuthoringPath -or
        $entry.authoringPath.Contains('\') -or
        $entry.authoringPath -match '(^|/)\.\.(/|$)' -or
        $entry.authoringPath -match '^[A-Za-z]:') {
        throw "Map Effect target must be a canonical direct-authored catalog row: $EffectAssetId"
    }
    $dataRoot = [IO.Path]::GetFullPath((Join-Path $ProjectRoot 'Data'))
    $relativePath = $entry.authoringPath.Replace('/', [IO.Path]::DirectorySeparatorChar)
    $authoredPath = [IO.Path]::GetFullPath((Join-Path $dataRoot $relativePath))
    $rootPrefix = $dataRoot.TrimEnd('\') + '\'
    if (-not $authoredPath.StartsWith($rootPrefix, [StringComparison]::OrdinalIgnoreCase) -or
        -not [IO.File]::Exists($authoredPath)) {
        throw "Map Effect authored document is missing or outside Data: $authoredPath"
    }
    try {
        $document = [IO.File]::ReadAllText(
            $authoredPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Map Effect authored document JSON parse failed: $authoredPath" }
    if ($document.schema -isnot [string] -or
        $document.schema -cne 'lostark.effect-authoring' -or
        -not (Test-JsonNumber $document.version) -or
        [double]$document.version -ne 13.0 -or
        $document.effectAssetId -isnot [string] -or
        $document.effectAssetId -cne $EffectAssetId -or
        $document.elements -isnot [System.Array]) {
        throw "Map Effect authored identity is invalid: $authoredPath"
    }
    $maximumDurationSeconds = 0.0
    $drawableCount = 0
    $elementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($element in @($document.elements)) {
        if ($element -isnot [pscustomobject] -or
            $element.id -isnot [string] -or
            $element.id -notmatch '^[a-z0-9][a-z0-9._-]{0,127}$' -or
            -not $elementIds.Add($element.id) -or
            $element.visible -isnot [bool] -or
            $element.kind -isnot [string] -or
            $element.kind -notin @('particle','trail','mesh','decal','light','screenPost') -or
            $element.material -isnot [pscustomobject] -or
            $element.material.templateId -isnot [string] -or
            [string]::IsNullOrWhiteSpace($element.material.templateId) -or
            $element.material.renderProfile -isnot [string] -or
            [string]::IsNullOrWhiteSpace($element.material.renderProfile) -or
            $element.detail -isnot [pscustomobject]) {
            throw "Map Effect authored element identity/material/detail is invalid: $EffectAssetId"
        }
        if (-not $element.visible) { continue }
        ++$drawableCount
        if ($null -eq $element.detail -or $null -eq $element.detail.timing -or
            -not (Test-JsonNumber $element.detail.timing.startDelaySeconds) -or
            -not (Test-JsonNumber $element.detail.timing.lifeTimeSeconds) -or
            -not (Test-JsonNumber $element.detail.timing.afterImageSeconds)) {
            throw "Map Effect authored visible element has no finite timing: $EffectAssetId"
        }
        $endSeconds = [double]$element.detail.timing.startDelaySeconds +
            [double]$element.detail.timing.lifeTimeSeconds +
            [double]$element.detail.timing.afterImageSeconds
        if ($endSeconds -le 0.0) {
            throw "Map Effect authored visible element has no positive lifetime: $EffectAssetId"
        }
        $maximumDurationSeconds = [Math]::Max($maximumDurationSeconds, $endSeconds)
    }
    if (0 -eq $drawableCount) {
        throw "Map Effect direct-authored document has no visible drawable element: $EffectAssetId"
    }
    $resourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    Add-MapEffectRuntimeResourceIds $document $EffectAssetId $resourceIds
    Assert-MapEffectRuntimeResourceFiles $resourceIds $EffectAssetId
    return [pscustomobject]@{
        Path = $authoredPath
        DurationSeconds = $maximumDurationSeconds
    }
}

function Assert-MapEffectActivationWindows {
    param([object]$Presentation, [double]$EffectDurationSeconds)
    if (-not [IO.File]::Exists($valtanEncounterPath)) {
        throw "Valtan encounter is missing for Map Effect activation validation: $valtanEncounterPath"
    }
    try {
        $encounter = [IO.File]::ReadAllText(
            $valtanEncounterPath, [Text.Encoding]::UTF8) | ConvertFrom-Json
    }
    catch { throw "Valtan encounter JSON parse failed: $valtanEncounterPath" }
    if ($encounter.schema -isnot [string] -or
        $encounter.schema -cne 'lostark.encounter-profile' -or
        -not (Test-JsonNumber $encounter.formatVersion) -or
        [double]$encounter.formatVersion -ne 4.0 -or
        $encounter.encounterId -isnot [string] -or
        $encounter.encounterId -cne 'ENCOUNTER_VALTAN' -or
        $encounter.patterns -isnot [System.Array]) {
        throw "Valtan encounter header is invalid for Map Effect activation validation."
    }
    $windows = @($Presentation.activationWindows)
    $patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($window in $windows) { [void]$patternIds.Add([string]$window.patternId) }
    if ($patternIds.Count -ne 1) {
        throw "A Server-clock Map Effect activation set must target exactly one encounter pattern."
    }
    $patternId = [string]$windows[0].patternId
    $matches = @($encounter.patterns | Where-Object {
        $_.patternId -is [string] -and $_.patternId -ceq $patternId
    })
    if ($matches.Count -ne 1 -or $matches[0].stages -isnot [System.Array]) {
        throw "Map Effect activation pattern does not resolve exactly once: $patternId"
    }
    $stages = @($matches[0].stages)
    if ($stages.Count -eq 0 -or $windows.Count -ne $stages.Count) {
        throw "Map Effect activation windows must exactly cover every pattern stage: $patternId"
    }
    $expectedOffsetMs = [uint64]0
    for ($index = 0; $index -lt $stages.Count; ++$index) {
        $stage = $stages[$index]
        $window = $windows[$index]
        if ($stage.stageId -isnot [string] -or
            $stage.stageId -notmatch '^[A-Za-z0-9._-]{1,128}$' -or
            -not (Test-JsonNumber $stage.durationMs) -or
            [double]$stage.durationMs -lt 1.0 -or
            [math]::Floor([double]$stage.durationMs) -ne [double]$stage.durationMs -or
            $window.patternId -cne $patternId -or
            $window.stageId -cne $stage.stageId -or
            [uint64][double]$window.effectTimelineOffsetMs -ne $expectedOffsetMs) {
            throw "Map Effect activation window does not exactly join encounter stage offset/duration: $patternId/$($stage.stageId)"
        }
        $expectedOffsetMs += [uint64][double]$stage.durationMs
        if ($expectedOffsetMs -gt 3600000) {
            throw "Map Effect activation pattern duration exceeds the supported timeline: $patternId"
        }
    }
    if (($EffectDurationSeconds * 1000.0) + 0.001 -lt [double]$expectedOffsetMs) {
        throw "Map Effect authored duration does not cover the complete activation pattern: $patternId"
    }
}

function Assert-MapEffectDomainJoins {
    param([object]$Document, [string]$DocumentPath)
    $surfaceRows = @($Document.presentations | Where-Object {
        $_.presentationKind -ceq 'DEPLOY_SURFACE_OVERLAY'
    })
    if ($surfaceRows.Count -gt 0) {
        $assets = Read-MapEffectDeployCatalog
        $placements = Read-MapEffectDeployPlacements
        $groups = Read-MapEffectWorldDestruction
        $sourceGroups = Read-MapEffectWorldDestructionSource
        foreach ($surface in $surfaceRows) {
            if (@($surface.visibleStates).Count -ne 1 -or
                [string]$surface.visibleStates[0] -cne 'INTACT' -or
                [uint32][double]$surface.materialIndex -ne [uint32]1) {
                throw "Map Effect surface supports only INTact material-1 deferred overlay presentation: $($surface.independentEffectId)"
            }
            foreach ($owner in @($surface.owners)) {
                $placementId = [string]$owner.placementId
                $groupId = [string]$owner.groupId
                if (-not $placements.ContainsKey($placementId)) {
                    throw "Map Effect surface owner does not resolve to a Deploy placement: $placementId"
                }
                $placement = $placements[$placementId]
                if (-not $assets.ContainsKey([string]$placement.AssetId)) {
                    throw "Map Effect surface owner references an unknown Deploy asset: $placementId/$($placement.AssetId)"
                }
                $asset = $assets[[string]$placement.AssetId]
                if ($asset.Kind -cne 'STATIC' -or
                    -not [bool]$asset.DeferredEmissiveOverlay -or
                    -not [bool]$placement.Destructible) {
                    throw "Map Effect surface owner must be a static destructible deferred-emissive Deploy placement: $placementId"
                }
                if (-not $groups.ContainsKey($groupId)) {
                    throw "Map Effect surface owner references an unknown destruction group: $groupId"
                }
                $group = $groups[$groupId]
                if (-not [bool]$group.RemovesGround -or
                    -not $group.Members.Contains($placementId)) {
                    throw "Map Effect surface owner must be a member of a removesGround destruction group: $groupId/$placementId"
                }
                if (-not $sourceGroups.ContainsKey($groupId)) {
                    throw "Map Effect destruction projection has no current source group: $groupId"
                }
                $sourceGroup = $sourceGroups[$groupId]
                if (-not [bool]$sourceGroup.RemovesGround -or
                    $sourceGroup.TargetState -cne 'DESPAWNED' -or
                    $sourceGroup.MutationId -cne $group.MutationId -or
                    -not $sourceGroup.Members.SetEquals($group.Members)) {
                    throw "Map Effect destruction projection is stale against its current source group/mutation: $groupId"
                }
            }
        }
    }

    foreach ($world in @($Document.presentations | Where-Object {
        $_.presentationKind -ceq 'EFFECT_DOCUMENT'
    })) {
        $authored = Resolve-MapEffectAuthoredDocument ([string]$world.effectAssetId)
        if ($world.activationPolicy -ceq 'SERVER_PATTERN_WINDOW') {
            Assert-MapEffectActivationWindows $world ([double]$authored.DurationSeconds)
        }
    }
}

function Add-MapEffectPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapEffectsDeclared) {
        if (-not [IO.File]::Exists($authoringEffectPath)) {
            throw "Declared Map Effect authoring source is missing: $authoringEffectPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.mapeffects.json"
            Lines = Read-MapEffectDocument $authoringEffectPath
        })
    }
}

function Read-MapWaterDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "Map water JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','revision','unclassifiedAssetCount',
          'waters','deferred') `
        'Map water root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.map-water-presentation' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        -not (Test-JsonNumber $document.unclassifiedAssetCount)) {
        throw "Map water header is invalid: $Path"
    }
    if ($document.waters -isnot [System.Array] -or
        $document.deferred -isnot [System.Array]) {
        throw "Map water waters/deferred properties must be arrays: $Path"
    }
    $waters = @($document.waters)
    if ($waters.Count -lt 1 -or $waters.Count -gt 64) {
        throw "Map water material count is invalid: $Path"
    }
    $scalarNames = @(
        'opacity','opacityPower','fresnelIntensity','fresnelPower',
        'screenDistortionIntensity','normalIntensity','detailNormalIntensity',
        'normalDistortionIntensity','reflectionIntensity','reflectionUv',
        'depthBias','diffuseTiling')
    $vectorNames = @(
        'diffuseColor','reflectionColor','normalTilingPanning',
        'detailNormalTilingPanning','reflectionTilingPanning')
    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($water in $waters) {
        $required = @(
            'assetId','materialName','provenance','sourceParentChain',
            'sourceBlendMode','twoSided','detailNormalTexture','reflectionTexture',
            'foamTexture') + $scalarNames + $vectorNames
        $present = @($water.PSObject.Properties.Name)
        $extra = @($present | Where-Object {
            $_ -ne 'missingSourceTextures' -and $required -notcontains $_ })
        if ($extra.Count -gt 0) {
            throw "Map water row has unknown properties: $($extra -join ', ')"
        }
        foreach ($name in $required) {
            if ($present -notcontains $name) {
                throw "Map water row is missing $name in $Path"
            }
        }
        if ($water.assetId -isnot [string] -or
            [string]::IsNullOrWhiteSpace($water.assetId) -or
            $water.materialName -isnot [string] -or
            [string]::IsNullOrWhiteSpace($water.materialName)) {
            throw "Map water row identity is invalid: $Path"
        }
        # The runtime picks one shader pass per asset, so two rows for the same
        # asset would silently make one of them unreachable.
        if (-not $keys.Add($water.assetId)) {
            throw "Duplicate map water asset row: $($water.assetId)"
        }
        if ($water.sourceBlendMode -ne 'BLEND_Translucent') {
            throw "Map water row is not translucent in the source: $($water.assetId)"
        }
        if ($water.provenance -isnot [string] -or
            $water.provenance -notin @('SOURCE_MATERIAL_EXACT','PROJECT_AUTHORED')) {
            throw "Map water provenance is invalid: $($water.assetId)"
        }
        foreach ($name in @('detailNormalTexture','reflectionTexture','foamTexture')) {
            $value = $water.$name
            if ($value -isnot [string]) {
                throw "Map water $name must be a string: $($water.assetId)"
            }
            if ([string]::IsNullOrEmpty($value)) { continue }
            if ($value -notlike 'Map/*' -or $value -match '\.\.' -or
                $value -match '^[A-Za-z]:' -or $value.Contains('\')) {
                throw "Map water $name is not a Resources-relative id: $value"
            }
        }
        foreach ($name in $scalarNames) {
            if (-not (Test-JsonNumber $water.$name)) {
                throw "Map water $name is not finite: $($water.assetId)"
            }
        }
        foreach ($name in $vectorNames) {
            $vector = @($water.$name)
            if ($vector.Count -ne 4) {
                throw "Map water $name must have four components: $($water.assetId)"
            }
            foreach ($component in $vector) {
                if (-not (Test-JsonNumber $component)) {
                    throw "Map water $name component is not finite: $($water.assetId)"
                }
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Read-WorldSequenceDocument {
    param([string]$Path)
    $raw = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $raw | ConvertFrom-Json }
    catch { throw "World sequence JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','revision','templates','instances') `
        'World sequence root'
    if ($document.schema -isnot [string] -or
        $document.schema -ne 'lostark.world-sequences' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 2.0 -or
        $document.areaId -isnot [string] -or $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        [double]$document.revision -lt 0) {
        throw "World sequence header is invalid: $Path"
    }
    if ($document.templates -isnot [System.Array] -or
        $document.instances -isnot [System.Array]) {
        throw "World sequence templates and instances must be arrays: $Path"
    }
    # WorldSequenceDocument.cpp 의 상한과 동일하게 검사한다.
    $templates = @($document.templates)
    $instances = @($document.instances)
    if ($templates.Count -gt 256 -or $instances.Count -gt 2048) {
        throw "World sequence document exceeds its limits: $Path"
    }
    $stableId = '^[A-Za-z0-9._-]{1,128}$'
    $trackCounts = @{}
    $templateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($template in $templates) {
        Assert-ExactJsonProperties $template `
            @('sequenceId','displayName','category','durationMs','interpolation',
              'tracks','animationTracks') 'World sequence template'
        if ($template.sequenceId -isnot [string] -or
            $template.sequenceId -notmatch $stableId -or
            -not $templateIds.Add([string]$template.sequenceId) -or
            $template.displayName -isnot [string] -or
            $template.displayName.Length -lt 1 -or
            $template.displayName.Length -gt 128 -or
            $template.category -isnot [string] -or
            $template.category.Length -lt 1 -or $template.category.Length -gt 64 -or
            -not (Test-JsonNumber $template.durationMs) -or
            [double]$template.durationMs -le 0 -or
            [double]$template.durationMs -gt 600000 -or
            $template.interpolation -isnot [string] -or
            $template.interpolation -notin @('LINEAR','SMOOTH_STEP') -or
            $template.tracks -isnot [System.Array] -or
            $template.animationTracks -isnot [System.Array]) {
            throw "World sequence template is invalid: $($template.sequenceId)"
        }
        $tracks = @($template.tracks)
        $animationTracks = @($template.animationTracks)
        $total = $tracks.Count + $animationTracks.Count
        if ($total -lt 1 -or $total -gt 32) {
            throw "World sequence template track count is invalid: $($template.sequenceId)"
        }
        $trackCounts[[string]$template.sequenceId] = $total
        $slotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($track in $tracks) {
            Assert-ExactJsonProperties $track @('slotId','keys') 'World sequence track'
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                -not $slotIds.Add([string]$track.slotId) -or
                $track.keys -isnot [System.Array]) {
                throw "World sequence track is invalid: $($template.sequenceId)"
            }
            $keys = @($track.keys)
            if ($keys.Count -lt 2 -or $keys.Count -gt 256) {
                throw "World sequence key count is invalid: $($template.sequenceId)/$($track.slotId)"
            }
            $previous = -1
            foreach ($key in $keys) {
                Assert-ExactJsonProperties $key `
                    @('timeMs','positionOffset','rotationQuaternion',
                      'scaleMultiplier','visible') 'World sequence key'
                if (-not (Test-JsonNumber $key.timeMs) -or
                    [double]$key.timeMs -le $previous -or
                    [double]$key.timeMs -gt [double]$template.durationMs -or
                    $key.visible -isnot [bool] -or
                    $key.positionOffset -isnot [System.Array] -or
                    @($key.positionOffset).Count -ne 3 -or
                    $key.rotationQuaternion -isnot [System.Array] -or
                    @($key.rotationQuaternion).Count -ne 4 -or
                    $key.scaleMultiplier -isnot [System.Array] -or
                    @($key.scaleMultiplier).Count -ne 3) {
                    throw "World sequence key is invalid: $($template.sequenceId)/$($track.slotId)"
                }
                foreach ($component in (@($key.positionOffset) +
                        @($key.rotationQuaternion) + @($key.scaleMultiplier))) {
                    if (-not (Test-JsonNumber $component)) {
                        throw "World sequence key component is not finite: $($template.sequenceId)/$($track.slotId)"
                    }
                }
                $previous = [double]$key.timeMs
            }
            if ([double]$keys[0].timeMs -ne 0 -or
                [double]$keys[$keys.Count - 1].timeMs -ne [double]$template.durationMs) {
                throw "World sequence track must span the whole duration: $($template.sequenceId)/$($track.slotId)"
            }
        }
        foreach ($track in $animationTracks) {
            Assert-ExactJsonProperties $track `
                @('slotId','clipName','playbackRate','loop','holdLastFrame') `
                'World sequence animation track'
            if ($track.slotId -isnot [string] -or $track.slotId -notmatch $stableId -or
                -not $slotIds.Add([string]$track.slotId) -or
                $track.clipName -isnot [string] -or
                $track.clipName.Length -lt 1 -or $track.clipName.Length -gt 128 -or
                -not (Test-JsonNumber $track.playbackRate) -or
                [double]$track.playbackRate -lt 0.05 -or
                [double]$track.playbackRate -gt 8.0 -or
                $track.loop -isnot [bool] -or
                $track.holdLastFrame -isnot [bool]) {
                throw "World sequence animation track is invalid: $($template.sequenceId)"
            }
        }
    }
    $instanceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($instance in $instances) {
        Assert-ExactJsonProperties $instance `
            @('instanceId','templateId','enabled','startDelayMs','playbackSpeed',
              'bindings') 'World sequence instance'
        if ($instance.instanceId -isnot [string] -or
            $instance.instanceId -notmatch $stableId -or
            -not $instanceIds.Add([string]$instance.instanceId) -or
            $instance.templateId -isnot [string] -or
            -not $trackCounts.ContainsKey([string]$instance.templateId) -or
            $instance.enabled -isnot [bool] -or
            -not (Test-JsonNumber $instance.startDelayMs) -or
            [double]$instance.startDelayMs -lt 0 -or
            [double]$instance.startDelayMs -gt 600000 -or
            -not (Test-JsonNumber $instance.playbackSpeed) -or
            [double]$instance.playbackSpeed -lt 0.05 -or
            [double]$instance.playbackSpeed -gt 8.0 -or
            $instance.bindings -isnot [System.Array]) {
            throw "World sequence instance is invalid: $($instance.instanceId)"
        }
        $bindings = @($instance.bindings)
        if ($bindings.Count -ne $trackCounts[[string]$instance.templateId]) {
            throw "World sequence instance binding count does not match its template: $($instance.instanceId)"
        }
        $boundSlots = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $boundTargets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($binding in $bindings) {
            Assert-ExactJsonProperties $binding `
                @('slotId','targetKind','targetId') 'World sequence binding'
            if ($binding.slotId -isnot [string] -or
                -not $boundSlots.Add([string]$binding.slotId) -or
                $binding.targetKind -isnot [string] -or
                $binding.targetKind -notin @('MAP_PLACEMENT','DEPLOY_PLACEMENT') -or
                $binding.targetId -isnot [string] -or
                $binding.targetId -notmatch '^[0-9]{1,20}$' -or
                -not $boundTargets.Add("$($binding.targetKind):$($binding.targetId)")) {
                throw "World sequence binding is invalid: $($instance.instanceId)"
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-WorldSequencePublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:worldSequencesDeclared) {
        if (-not [IO.File]::Exists($authoringSequencePath)) {
            throw "Declared world sequence authoring source is missing: $authoringSequencePath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.worldsequences.json"
            Lines = Read-WorldSequenceDocument $authoringSequencePath
        })
    }
}

function Read-CameraShotDocument {
    param([string]$Path)
    $text = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    try { $document = $text | ConvertFrom-Json }
    catch { throw "Camera shot JSON parse failed: $Path" }
    Assert-ExactJsonProperties $document `
        @('schema','formatVersion','areaId','revision','shots') 'Camera shot root'
    if ($document.schema -ne 'lostark.camera-shots' -or
        -not (Test-JsonNumber $document.formatVersion) -or
        [double]$document.formatVersion -ne 1 -or
        $document.areaId -ne $AreaId -or
        -not (Test-JsonNumber $document.revision) -or
        [double]$document.revision -lt 1 -or
        [math]::Floor([double]$document.revision) -ne [double]$document.revision) {
        throw "Camera shot header is invalid: $Path"
    }
    # Level_KakulSaydonArena.cpp 의 상한과 동일하게 검사한다.
    $shots = @($document.shots)
    if ($shots.Count -gt 64) {
        throw "Camera shot document exceeds its limits: $Path"
    }
    $stableId = '^[A-Za-z0-9._-]{1,128}$'
    $shotIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($shot in $shots) {
        Assert-ExactJsonProperties $shot `
            @('shotId','sequenceInstanceId','box','eye','lookAt','fovYDegrees','blendInMs','blendOutMs','priority') `
            'Camera shot'
        if ($shot.shotId -isnot [string] -or
            $shot.shotId -notmatch $stableId -or
            -not $shotIds.Add([string]$shot.shotId)) {
            throw "Camera shot id is invalid or duplicated: $($shot.shotId)"
        }
        if ($shot.sequenceInstanceId -isnot [string] -or
            ($shot.sequenceInstanceId -ne '' -and
                $shot.sequenceInstanceId -notmatch $stableId)) {
            throw "Camera shot sequence binding is invalid: $($shot.shotId)"
        }
        Assert-ExactJsonProperties $shot.box `
            @('center','halfExtents','yawDegrees') "Camera shot box $($shot.shotId)"
        foreach ($triplet in @(@($shot.box.center), @($shot.box.halfExtents),
                @($shot.eye), @($shot.lookAt))) {
            if ($triplet.Count -ne 3) {
                throw "Camera shot vector must hold three numbers: $($shot.shotId)"
            }
            foreach ($component in $triplet) {
                if (-not (Test-JsonNumber $component)) {
                    throw "Camera shot vector component is not finite: $($shot.shotId)"
                }
            }
        }
        foreach ($extent in @($shot.box.halfExtents)) {
            if ([double]$extent -le 0 -or [double]$extent -gt 1000) {
                throw "Camera shot half extent is out of range: $($shot.shotId)"
            }
        }
        # An eye sitting on its own target has no direction and the engine
        # would reject the pose every frame.
        $delta = 0.0
        for ($axis = 0; $axis -lt 3; $axis++) {
            $step = [double]$shot.lookAt[$axis] - [double]$shot.eye[$axis]
            $delta += $step * $step
        }
        if ($delta -le 0.000001) {
            throw "Camera shot eye and lookAt coincide: $($shot.shotId)"
        }
        if (-not (Test-JsonNumber $shot.box.yawDegrees) -or
            [math]::Abs([double]$shot.box.yawDegrees) -gt 360) {
            throw "Camera shot yaw is out of range: $($shot.shotId)"
        }
        if (-not (Test-JsonNumber $shot.fovYDegrees) -or
            [double]$shot.fovYDegrees -le 1 -or [double]$shot.fovYDegrees -ge 179) {
            throw "Camera shot field of view is out of range: $($shot.shotId)"
        }
        foreach ($pair in @(@('blendInMs', 10000), @('blendOutMs', 10000), @('priority', 1000))) {
            $value = $shot.($pair[0])
            if (-not (Test-JsonNumber $value) -or
                [double]$value -lt 0 -or [double]$value -gt $pair[1] -or
                [math]::Floor([double]$value) -ne [double]$value) {
                throw "Camera shot $($pair[0]) is out of range: $($shot.shotId)"
            }
        }
    }
    return @([IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8))
}

function Add-CameraShotPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:cameraShotsDeclared) {
        if (-not [IO.File]::Exists($authoringCameraShotPath)) {
            throw "Declared camera shot authoring source is missing: $authoringCameraShotPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.camerashots.json"
            Lines = Read-CameraShotDocument $authoringCameraShotPath
        })
    }
}

function Add-MapWaterPublishFile {
    param([Collections.Generic.List[object]]$Files)
    if ($script:mapWaterDeclared) {
        if (-not [IO.File]::Exists($authoringWaterPath)) {
            throw "Declared map water authoring source is missing: $authoringWaterPath"
        }
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.mapwater.json"
            Lines = Read-MapWaterDocument $authoringWaterPath
        })
    }
}

if (-not [IO.File]::Exists($mapCatalogPath)) {
    throw "Map catalog is missing: $mapCatalogPath"
}
try { $mapCatalog = [IO.File]::ReadAllText(
        $mapCatalogPath, [Text.Encoding]::UTF8) | ConvertFrom-Json }
catch { throw "Map catalog JSON parse failed: $mapCatalogPath" }
$areaEntries = @($mapCatalog.areas | Where-Object { $_.id -eq $AreaId })
if (1 -ne $areaEntries.Count) {
    throw "Map catalog must declare Area exactly once: $AreaId"
}
$areaEntry = $areaEntries[0]
$sourceLightsProperty = $areaEntry.PSObject.Properties['sourceLights']
$runtimeLightsProperty = $areaEntry.PSObject.Properties['lights']
if (($null -eq $sourceLightsProperty) -ne ($null -eq $runtimeLightsProperty)) {
    throw "Map catalog light source/runtime declaration is incomplete: $AreaId"
}
$script:mapLightsDeclared = $null -ne $sourceLightsProperty
if ($AreaId -eq 'LV_LUT_HEARTRB_ED' -and -not $script:mapLightsDeclared) {
    throw "Valtan source/runtime map light declarations are required: $AreaId"
}
if ($script:mapLightsDeclared) {
    $expectedSourceLights = "Data/Maps/Authoring/$AreaId/$AreaId.maplights.json"
    $expectedRuntimeLights = "Client/Bin/DataFiles/Map/$AreaId.maplights.json"
    if ($areaEntry.sourceLights -isnot [string] -or
        $areaEntry.sourceLights -ne $expectedSourceLights -or
        $areaEntry.lights -isnot [string] -or
        $areaEntry.lights -ne $expectedRuntimeLights) {
        throw "Map catalog light paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringLightPath)) {
    throw "Map light source exists without a MapCatalog declaration: $AreaId"
}

$sourceEffectsProperty = $areaEntry.PSObject.Properties['sourceEffects']
$runtimeEffectsProperty = $areaEntry.PSObject.Properties['effects']
if (($null -eq $sourceEffectsProperty) -ne ($null -eq $runtimeEffectsProperty)) {
    throw "Map catalog Effect source/runtime declaration is incomplete: $AreaId"
}
$script:mapEffectsDeclared = $null -ne $sourceEffectsProperty
if ($AreaId -eq 'LV_LUT_HEARTRB_ED' -and -not $script:mapEffectsDeclared) {
    throw "Valtan source/runtime Map Effect declarations are required: $AreaId"
}
if ($script:mapEffectsDeclared) {
    $expectedSourceEffects = "Data/Maps/Authoring/$AreaId/$AreaId.mapeffects.json"
    $expectedRuntimeEffects = "Client/Bin/DataFiles/Map/$AreaId.mapeffects.json"
    if ($areaEntry.sourceEffects -isnot [string] -or
        $areaEntry.sourceEffects -ne $expectedSourceEffects -or
        $areaEntry.effects -isnot [string] -or
        $areaEntry.effects -ne $expectedRuntimeEffects) {
        throw "Map catalog Effect paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringEffectPath)) {
    throw "Map Effect source exists without a MapCatalog declaration: $AreaId"
}

$sourceWaterProperty = $areaEntry.PSObject.Properties['sourceWater']
$runtimeWaterProperty = $areaEntry.PSObject.Properties['water']
if (($null -eq $sourceWaterProperty) -ne ($null -eq $runtimeWaterProperty)) {
    throw "Map catalog water source/runtime declaration is incomplete: $AreaId"
}
$script:mapWaterDeclared = $null -ne $sourceWaterProperty
if ($script:mapWaterDeclared) {
    $expectedSourceWater = "Data/Maps/Authoring/$AreaId/$AreaId.mapwater.json"
    $expectedRuntimeWater = "Client/Bin/DataFiles/Map/$AreaId.mapwater.json"
    if ($areaEntry.sourceWater -isnot [string] -or
        $areaEntry.sourceWater -ne $expectedSourceWater -or
        $areaEntry.water -isnot [string] -or
        $areaEntry.water -ne $expectedRuntimeWater) {
        throw "Map catalog water paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringWaterPath)) {
    throw "Map water source exists without a MapCatalog declaration: $AreaId"
}

$sourceSequencesProperty = $areaEntry.PSObject.Properties['sourceSequences']
$runtimeSequencesProperty = $areaEntry.PSObject.Properties['sequences']
if (($null -eq $sourceSequencesProperty) -ne ($null -eq $runtimeSequencesProperty)) {
    throw "Map catalog world sequence source/runtime declaration is incomplete: $AreaId"
}
$script:worldSequencesDeclared = $null -ne $sourceSequencesProperty
if ($script:worldSequencesDeclared) {
    $expectedSourceSequences = "Data/Maps/Authoring/$AreaId/$AreaId.worldsequences.json"
    $expectedRuntimeSequences = "Client/Bin/DataFiles/Map/$AreaId.worldsequences.json"
    if ($areaEntry.sourceSequences -isnot [string] -or
        $areaEntry.sourceSequences -ne $expectedSourceSequences -or
        $areaEntry.sequences -isnot [string] -or
        $areaEntry.sequences -ne $expectedRuntimeSequences) {
        throw "Map catalog world sequence paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringSequencePath)) {
    throw "World sequence source exists without a MapCatalog declaration: $AreaId"
}

$sourceCameraShotsProperty = $areaEntry.PSObject.Properties['sourceCameraShots']
$runtimeCameraShotsProperty = $areaEntry.PSObject.Properties['cameraShots']
if (($null -eq $sourceCameraShotsProperty) -ne ($null -eq $runtimeCameraShotsProperty)) {
    throw "Map catalog camera shot source/runtime declaration is incomplete: $AreaId"
}
$script:cameraShotsDeclared = $null -ne $sourceCameraShotsProperty
if ($script:cameraShotsDeclared) {
    $expectedSourceCameraShots = "Data/Maps/Authoring/$AreaId/$AreaId.camerashots.json"
    $expectedRuntimeCameraShots = "Client/Bin/DataFiles/Map/$AreaId.camerashots.json"
    if ($areaEntry.sourceCameraShots -isnot [string] -or
        $areaEntry.sourceCameraShots -ne $expectedSourceCameraShots -or
        $areaEntry.cameraShots -isnot [string] -or
        $areaEntry.cameraShots -ne $expectedRuntimeCameraShots) {
        throw "Map catalog camera shot paths are not canonical: $AreaId"
    }
}
elseif ([IO.File]::Exists($authoringCameraShotPath)) {
    throw "Camera shot source exists without a MapCatalog declaration: $AreaId"
}

function Invoke-FileSetTransaction {
    param([object[]]$Files)
    $transactionId = [Guid]::NewGuid().ToString('N')
    $stagingRoot = [IO.Path]::GetFullPath(
        (Join-Path $runtimeRoot ".map-publish.staging.$AreaId.$transactionId"))
    if ([IO.Path]::GetDirectoryName($stagingRoot) -ne
        [IO.Path]::GetFullPath($runtimeRoot).TrimEnd('\')) {
        throw 'Map publish staging directory escapes the runtime root.'
    }
    [IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
    $entries = [Collections.Generic.List[object]]::new()
    $committed = $false
    try {
        foreach ($file in $Files) {
            $staged = Join-Path $stagingRoot $file.Name
            [IO.File]::WriteAllBytes($staged, $file.Bytes)
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
        $committed = $true
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
            try {
                Remove-Item -LiteralPath $stagingRoot -Recurse -Force
            }
            catch {
                Write-Warning "Map publish staging cleanup failed: $($_.Exception.Message)"
            }
        }
    }

    if ($committed) {
        foreach ($entry in $entries) {
            if ([IO.File]::Exists($entry.Rollback)) {
                try {
                    [IO.File]::Delete($entry.Rollback)
                }
                catch {
                    Write-Warning (
                        "Map publish committed, but backup cleanup failed for " +
                        "'$($entry.Rollback)': $($_.Exception.Message)")
                }
            }
        }
    }
}

function Add-DeployPublishFiles {
    param([Collections.Generic.List[object]]$Files)
    $hasCatalog = [IO.File]::Exists($sourceDeployCatalogPath)
    $hasPlacements = [IO.File]::Exists($authoringDeployPath)
    if ($hasCatalog -ne $hasPlacements) {
        throw "Deploy authoring pair is incomplete for $AreaId"
    }
    if ($hasCatalog) {
        $pair = Read-DeployAuthoringPair
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.deployassets"
            Lines = $pair.CatalogLines
        })
        $Files.Add([pscustomobject]@{
            Name = "$AreaId.deployplacements"
            Lines = $pair.PlacementLines
        })
    }
}

function Complete-MapPublish {
    param([object[]]$Files, [string]$CatalogType, [string]$RuntimeEntry,
        [int]$ShardCount = 0)
    # Every mode consumes the same normalized bytes. This stage performs no I/O.
    $expectedFiles = [Collections.Generic.List[object]]::new()
    $names = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($file in $Files) {
        if ($file.Name -notmatch '^[A-Za-z0-9_.-]+$' -or
            [IO.Path]::GetFileName($file.Name) -ne $file.Name -or
            -not $names.Add($file.Name)) {
            throw "Invalid or duplicate map output filename: $($file.Name)"
        }
        $expectedFiles.Add([pscustomobject]@{
            Name = $file.Name
            Bytes = $utf8.GetBytes((([string[]]$file.Lines -join "`n") + "`n"))
        })
    }
    if ($Mode -eq 'Check') {
        foreach ($file in $expectedFiles) {
            $destination = Join-Path $runtimeRoot $file.Name
            if (-not [IO.File]::Exists($destination)) {
                throw "Map runtime output is missing: $destination"
            }
            $actual = [IO.File]::ReadAllBytes($destination)
            if ($actual.Length -ne $file.Bytes.Length -or
                [Convert]::ToBase64String($actual) -cne
                [Convert]::ToBase64String($file.Bytes)) {
                throw "Map runtime output differs from authoring: $destination"
            }
        }
    }
    elseif ($Mode -eq 'Publish') {
        Invoke-FileSetTransaction $expectedFiles
    }
    $result = [ordered]@{
        AreaId = $AreaId
        Mode = $Mode
        CatalogType = $CatalogType
        PlacementCount = $authoringRows.Count
        FileCount = $expectedFiles.Count
        RuntimePath = $RuntimeEntry
    }
    if ($ShardCount -gt 0) { $result.ShardCount = $ShardCount }
    if ($Mode -eq 'Publish') { $result.Sha256 = Get-MapPublishSha256 $RuntimeEntry }
    [pscustomobject]$result
}

if (-not [IO.File]::Exists($authoringPath)) {
    throw "Authoring placement is missing: $authoringPath"
}
if (-not [IO.Directory]::Exists($importRoot)) {
    throw "Imported map source is missing: $importRoot"
}
$authoringRows = @(Read-PlacementDocument $authoringPath)
$parsedAuthoringRows = [Collections.Generic.List[object]]::new()
$seenPlacementIds = [Collections.Generic.HashSet[uint64]]::new()
$seenSourcePlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($row in $authoringRows) {
    $parsed = Parse-PlacementRow $row $authoringPath
    if (-not $seenPlacementIds.Add($parsed.PlacementId)) {
        throw "Duplicate authoring placement ID: $($parsed.PlacementId)"
    }
    if (-not $seenSourcePlacementIds.Add($parsed.SourcePlacementId)) {
        throw "Duplicate authoring source placement ID: $($parsed.SourcePlacementId)"
    }
    $parsedAuthoringRows.Add($parsed)
}

if (-not [IO.File]::Exists($sourceShardSetPath)) {
    if (-not [IO.File]::Exists($sourceCatalogPath)) {
        throw "Imported map catalog is missing: $sourceCatalogPath"
    }
    $catalog = Read-MapAssetCatalog $sourceCatalogPath
    foreach ($parsed in $parsedAuthoringRows) {
        if (-not $catalog.AssetIds.Contains($parsed.AssetId)) {
            throw "Authoring placement references an asset outside the catalog: $($parsed.AssetId)"
        }
    }
    $lines = @("LOSTARK_MAP_PLACEMENTS 2 `"$AreaId`" $($authoringRows.Count)") + $authoringRows
    $files = [Collections.Generic.List[object]]::new()
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapassets"
        Lines = $catalog.Lines
    })
    $files.Add([pscustomobject]@{
        Name = "$AreaId.mapplacements"
        Lines = $lines
    })

    Add-DeployPublishFiles $files
    Add-MapLightPublishFile $files
    Add-MapEffectPublishFile $files
    Add-MapWaterPublishFile $files
    Add-WorldSequencePublishFile $files
    Add-CameraShotPublishFile $files

    Complete-MapPublish $files 'single' $runtimePath
    return
}

$mapSetLines = @([IO.File]::ReadAllLines($sourceShardSetPath, [Text.Encoding]::UTF8))
if ($mapSetLines.Count -lt 2 -or $mapSetLines.Count -gt 65) {
    throw "Shard set count is invalid: $sourceShardSetPath"
}
$mapSetHeader = [regex]::Match(
    $mapSetLines[0],
    '^LOSTARK_MAP_SHARD_SET\s+1\s+"(?<area>[A-Za-z0-9_.-]+)"\s+(?<count>[0-9]+)$')
if (-not $mapSetHeader.Success -or
    $mapSetHeader.Groups['area'].Value -cne $AreaId -or
    [uint32]$mapSetHeader.Groups['count'].Value -ne ($mapSetLines.Count - 1)) {
    throw "Shard set header is invalid: $sourceShardSetPath"
}

$shards = [Collections.Generic.List[object]]::new()
$assetShards = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
$existingPlacementShard = @{}
$seenShardIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$seenBaselineSources = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($line in @($mapSetLines | Select-Object -Skip 1)) {
    $match = [regex]::Match(
        $line,
        '^"(?<id>[A-Za-z0-9_.-]+)"\s+"(?<catalog>[^"]+)"\s+"(?<placements>[^"]+)"\s+(?<assets>[0-9]+)\s+(?<count>[0-9]+)$')
    if (-not $match.Success) {
        throw "Invalid shard-set row: $line"
    }
    if (-not $seenShardIds.Add($match.Groups['id'].Value)) {
        throw "Duplicate shard ID: $($match.Groups['id'].Value)"
    }
    $shard = [pscustomobject]@{
        Id = $match.Groups['id'].Value
        CatalogName = $match.Groups['catalog'].Value
        PlacementName = $match.Groups['placements'].Value
        AssetCount = [uint32]$match.Groups['assets'].Value
        PlacementCount = [uint32]$match.Groups['count'].Value
        Rows = [Collections.Generic.List[string]]::new()
        CatalogLines = @()
    }
	$catalogPath = Assert-ImportedLeaf $shard.CatalogName '.mapassets'
	$placementPath = Assert-ImportedLeaf $shard.PlacementName '.mapplacements'
    if (-not [IO.File]::Exists($catalogPath) -or
        -not [IO.File]::Exists($placementPath)) {
        throw "Shard source is missing: $($shard.Id)"
    }
    $catalog = Read-MapAssetCatalog $catalogPath
    $shard.CatalogLines = $catalog.Lines
    if ($catalog.AssetIds.Count -ne $shard.AssetCount) {
        throw "Shard asset count mismatch: $($shard.Id)"
    }
    foreach ($assetId in $catalog.AssetIds) {
        if (-not $assetShards.ContainsKey($assetId)) {
            $assetShards[$assetId] = [Collections.Generic.List[string]]::new()
        }
        $assetShards[$assetId].Add($shard.Id)
    }
    $baselineRows = @(Read-PlacementDocument $placementPath)
    if ($baselineRows.Count -ne $shard.PlacementCount) {
        throw "Shard placement count mismatch: $($shard.Id)"
    }
    foreach ($existingRow in $baselineRows) {
        $parsed = Parse-PlacementRow $existingRow $placementPath
        if ($existingPlacementShard.ContainsKey($parsed.PlacementId)) {
            throw "Duplicate placement ID across current shards: $($parsed.PlacementId)"
        }
        if (-not $seenBaselineSources.Add($parsed.SourcePlacementId)) {
            throw "Duplicate source placement ID across current shards: $($parsed.SourcePlacementId)"
        }
        if (-not $catalog.AssetIds.Contains($parsed.AssetId)) {
            throw "Baseline placement references an asset outside its shard: $($parsed.AssetId)"
        }
        $existingPlacementShard[$parsed.PlacementId] = $shard.Id
    }
    $shards.Add($shard)
}

$shardsById = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
foreach ($shard in $shards) { $shardsById[$shard.Id] = $shard }
foreach ($parsed in $parsedAuthoringRows) {
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
Add-DeployPublishFiles $files
Add-MapLightPublishFile $files
Add-MapEffectPublishFile $files
Add-MapWaterPublishFile $files
Add-WorldSequencePublishFile $files
Add-CameraShotPublishFile $files
Complete-MapPublish $files 'shard-set' $shardSetPath $shards.Count
