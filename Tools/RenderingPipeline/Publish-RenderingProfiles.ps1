[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$SourcePath = '',
    [string]$DestinationPath = ''
)

$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if ([string]::IsNullOrWhiteSpace($SourcePath)) {
    $SourcePath = Join-Path $repoRoot 'Data\Rendering\Authored\RenderingProfiles.json'
}
if ([string]::IsNullOrWhiteSpace($DestinationPath)) {
    $DestinationPath = Join-Path $repoRoot `
        'Client\Bin\DataFiles\Rendering\RenderingProfiles.runtime.json'
}
$SourcePath = [IO.Path]::GetFullPath($SourcePath)
$DestinationPath = [IO.Path]::GetFullPath($DestinationPath)

function Assert-ExactProperties(
    [object]$Value,
    [string[]]$Expected,
    [string]$Context) {
    if ($null -eq $Value) {
        throw "$Context must be an object."
    }
    $actual = @($Value.PSObject.Properties.Name | Sort-Object -CaseSensitive)
    $wanted = @($Expected | Sort-Object -CaseSensitive)
    if (($actual -join '|') -cne ($wanted -join '|')) {
        throw "$Context fields are invalid. actual=$($actual -join ',')"
    }
}

function Assert-FiniteRange(
    [object]$Value,
    [double]$Minimum,
    [double]$Maximum,
    [string]$Context) {
    if ($null -eq $Value) {
        throw "$Context is missing."
    }
	if ($Value -is [string] -or $Value -is [bool] -or
		$Value -isnot [ValueType]) {
		throw "$Context must be a JSON number."
	}
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number) -or
        $number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context must be finite in [$Minimum, $Maximum]."
    }
}

function Assert-FiniteFloatRange(
    [object]$Value,
    [single]$Minimum,
    [single]$Maximum,
    [string]$Context) {
    if ($null -eq $Value) {
        throw "$Context is missing."
    }
    if ($Value -is [string] -or $Value -is [bool] -or
        $Value -isnot [ValueType]) {
        throw "$Context must be a JSON number."
    }

    # Workbench Save serializes the exact binary32 value (for example,
    # 0.0312f becomes 0.0311999992). Compare against the same binary32
    # boundaries used by CRenderingProfileService instead of wider decimal
    # literals, otherwise a valid Save cannot be published again.
    $number = [double]$Value
    $minimumFloat = [double]$Minimum
    $maximumFloat = [double]$Maximum
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number) -or
        $number -lt $minimumFloat -or $number -gt $maximumFloat) {
        throw "$Context must be finite in [$minimumFloat, $maximumFloat]."
    }
}

function Assert-Vector4(
    [object]$Value,
    [single]$Minimum,
    [single]$Maximum,
    [string]$Context) {
    $items = @($Value)
    if ($items.Count -ne 4) {
        throw "$Context must contain exactly four numbers."
    }
    for ($index = 0; $index -lt 4; ++$index) {
        Assert-FiniteFloatRange $items[$index] $Minimum $Maximum "$Context[$index]"
    }
}

function Assert-Vector3(
    [object]$Value,
    [single]$Minimum,
    [single]$Maximum,
    [string]$Context) {
    $items = @($Value)
    if ($items.Count -ne 3) {
        throw "$Context must contain exactly three numbers."
    }
    for ($index = 0; $index -lt 3; ++$index) {
        Assert-FiniteFloatRange $items[$index] $Minimum $Maximum "$Context[$index]"
    }
}

function Assert-Color([object]$Value, [string]$Context) {
    Assert-Vector4 $Value 0.0 64.0 $Context
    $items = @($Value)
    Assert-FiniteFloatRange $items[3] 0.0 1.0 "$Context[3]"
}

function Assert-NoDuplicateJsonObjectKeys([string]$Path) {
    $validatorPath = Join-Path $PSScriptRoot `
        'assert_no_duplicate_json_keys.py'
    if (-not (Test-Path -LiteralPath $validatorPath -PathType Leaf)) {
        throw "Strict JSON validator is missing: $validatorPath"
    }
    $python = Get-Command python.exe -ErrorAction SilentlyContinue
    if ($null -eq $python) {
        $python = Get-Command python -ErrorAction SilentlyContinue
    }
    if ($null -eq $python) {
        throw 'Python is required for strict duplicate JSON key validation.'
    }

    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $output = & $python.Source $validatorPath $Path 2>&1
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($exitCode -ne 0) {
        throw "Strict JSON validation failed: $($output -join ' ')"
    }
}

function Remove-FileBestEffort([string]$Path, [string]$Purpose) {
    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }
    try {
        Remove-Item -LiteralPath $Path -Force -ErrorAction Stop
    }
    catch {
        Write-Warning "$Purpose cleanup was deferred: $($_.Exception.Message)"
    }
}

function Assert-RenderingProfileDocument([object]$Document) {
    Assert-ExactProperties $Document @(
        'schema', 'formatVersion', 'revision', 'globalQuality', 'profiles') 'root'
    if ($Document.schema -isnot [string] -or
		[string]$Document.schema -cne 'lostark.rendering-profiles') {
        throw 'Rendering profile schema is invalid.'
    }
	Assert-FiniteRange $Document.formatVersion 1.0 1.0 'formatVersion'
    if ([double]$Document.formatVersion -ne 1.0) {
        throw 'Rendering profile formatVersion must be 1.'
    }
	Assert-FiniteRange $Document.revision 1.0 ([uint32]::MaxValue) 'revision'
    $revision = [double]$Document.revision
    if ([double]::IsNaN($revision) -or [double]::IsInfinity($revision) -or
        $revision -lt 1.0 -or $revision -gt [uint32]::MaxValue -or
        $revision -ne [math]::Floor($revision)) {
        throw 'Rendering profile revision must be a positive uint32 integer.'
    }

    $global = $Document.globalQuality
    Assert-ExactProperties $global @(
        'ssaoEnabled', 'ssaoRadius', 'ssaoBias', 'ssaoIntensity',
        'ssaoPower', 'ssaoDistanceFade',
        'bloomEnabled', 'bloomThreshold', 'bloomSoftKnee', 'bloomIntensity',
        'bloomScatter', 'exposure', 'whitePoint', 'gamma', 'fxaaEnabled',
        'fxaaSubpixel', 'fxaaEdgeThreshold', 'fxaaEdgeThresholdMin') 'globalQuality'
    if ($global.ssaoEnabled -isnot [bool] -or
        $global.bloomEnabled -isnot [bool] -or
        $global.fxaaEnabled -isnot [bool]) {
        throw 'globalQuality enabled fields must be booleans.'
    }
    Assert-FiniteFloatRange $global.ssaoRadius 0.01 8.0 'globalQuality.ssaoRadius'
    Assert-FiniteFloatRange $global.ssaoBias 0.0 1.0 'globalQuality.ssaoBias'
    Assert-FiniteFloatRange $global.ssaoIntensity 0.0 4.0 'globalQuality.ssaoIntensity'
    Assert-FiniteFloatRange $global.ssaoPower 0.1 8.0 'globalQuality.ssaoPower'
    Assert-FiniteFloatRange $global.ssaoDistanceFade 1.0 1000.0 `
        'globalQuality.ssaoDistanceFade'
    if ([double]$global.ssaoBias -ge [double]$global.ssaoRadius) {
        throw 'globalQuality.ssaoBias must be less than ssaoRadius.'
    }
    if ([double]$global.ssaoDistanceFade -lt [double]$global.ssaoRadius) {
        throw 'globalQuality.ssaoDistanceFade must be at least ssaoRadius.'
    }
    Assert-FiniteFloatRange $global.bloomThreshold 0.0 64.0 'globalQuality.bloomThreshold'
    Assert-FiniteFloatRange $global.bloomSoftKnee 0.0 1.0 'globalQuality.bloomSoftKnee'
    Assert-FiniteFloatRange $global.bloomIntensity 0.0 16.0 'globalQuality.bloomIntensity'
    Assert-FiniteFloatRange $global.bloomScatter 0.25 4.0 'globalQuality.bloomScatter'
    Assert-FiniteFloatRange $global.exposure 0.01 32.0 'globalQuality.exposure'
    Assert-FiniteFloatRange $global.whitePoint 1.0 64.0 'globalQuality.whitePoint'
    Assert-FiniteFloatRange $global.gamma 1.0 3.0 'globalQuality.gamma'
    Assert-FiniteFloatRange $global.fxaaSubpixel 0.0 1.0 'globalQuality.fxaaSubpixel'
    Assert-FiniteFloatRange $global.fxaaEdgeThreshold 0.0312 0.333 `
        'globalQuality.fxaaEdgeThreshold'
    Assert-FiniteFloatRange $global.fxaaEdgeThresholdMin 0.0156 0.0833 `
        'globalQuality.fxaaEdgeThresholdMin'

    $profiles = @($Document.profiles)
    if ($profiles.Count -lt 1 -or $profiles.Count -gt 32) {
        throw 'profiles must contain between 1 and 32 entries.'
    }
    $ids = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($profile in $profiles) {
        Assert-ExactProperties $profile @(
            'profileId', 'exposureMultiplier', 'bloomIntensityMultiplier',
            'light', 'shadow', 'fog') `
            'profile'
		if ($profile.profileId -isnot [string]) {
			throw 'profile.profileId must be a string.'
		}
        $profileId = [string]$profile.profileId
        if ($profileId -notmatch '^[A-Za-z0-9_.-]{1,128}$') {
            throw "Invalid rendering profile ID: $profileId"
        }
        if (-not $ids.Add($profileId)) {
            throw "Duplicate rendering profile ID: $profileId"
        }
        Assert-FiniteFloatRange $profile.exposureMultiplier 0.1 4.0 `
            "$profileId.exposureMultiplier"
        Assert-FiniteFloatRange $profile.bloomIntensityMultiplier 0.0 4.0 `
            "$profileId.bloomIntensityMultiplier"

        $light = $profile.light
        Assert-ExactProperties $light @(
            'type', 'direction', 'diffuse', 'ambient', 'specular') "$profileId.light"
        if ($light.type -isnot [string] -or
			[string]$light.type -cne 'directional') {
            throw "$profileId.light.type must be directional."
        }
        Assert-Vector4 $light.direction -64.0 64.0 "$profileId.light.direction"
        $direction = @($light.direction)
        $lengthSquared = [double]$direction[0] * [double]$direction[0] +
            [double]$direction[1] * [double]$direction[1] +
            [double]$direction[2] * [double]$direction[2]
        if ($lengthSquared -le 0.000001) {
            throw "$profileId.light.direction must be non-zero."
        }
        Assert-Color $light.diffuse "$profileId.light.diffuse"
        Assert-Color $light.ambient "$profileId.light.ambient"
        Assert-Color $light.specular "$profileId.light.specular"

        $shadow = $profile.shadow
        Assert-ExactProperties $shadow @(
            'enabled', 'focus', 'distance', 'orthographicWidth',
            'orthographicHeight', 'near', 'far', 'depthBias',
            'normalBias', 'strength') "$profileId.shadow"
        if ($shadow.enabled -isnot [bool]) {
            throw "$profileId.shadow.enabled must be boolean."
        }
        Assert-Vector3 $shadow.focus -100000.0 100000.0 `
            "$profileId.shadow.focus"
        Assert-FiniteFloatRange $shadow.distance 0.1 100000.0 `
            "$profileId.shadow.distance"
        Assert-FiniteFloatRange $shadow.orthographicWidth 0.1 10000.0 `
            "$profileId.shadow.orthographicWidth"
        Assert-FiniteFloatRange $shadow.orthographicHeight 0.1 10000.0 `
            "$profileId.shadow.orthographicHeight"
        Assert-FiniteFloatRange $shadow.near 0.0001 100000.0 `
            "$profileId.shadow.near"
        Assert-FiniteFloatRange $shadow.far 0.0001 100000.0 `
            "$profileId.shadow.far"
        if ([double]$shadow.far -le [double]$shadow.near) {
            throw "$profileId.shadow.far must be greater than near."
        }
        Assert-FiniteFloatRange $shadow.depthBias 0.0 0.05 `
            "$profileId.shadow.depthBias"
        Assert-FiniteFloatRange $shadow.normalBias 0.0 10.0 `
            "$profileId.shadow.normalBias"
        Assert-FiniteFloatRange $shadow.strength 0.0 1.0 `
            "$profileId.shadow.strength"

        $fog = $profile.fog
        Assert-ExactProperties $fog @(
            'enabled', 'color', 'density', 'heightFalloff',
            'topHeight', 'startDistance', 'maximumOpacity',
            'driftSpeed', 'driftHeightAmplitude',
            'driftDensityAmplitude', 'coveragePercent',
            'windDirectionX', 'windDirectionZ', 'windSpeed',
            'patchScale', 'patchSoftness') "$profileId.fog"
        if ($fog.enabled -isnot [bool]) {
            throw "$profileId.fog.enabled must be boolean."
        }
        Assert-Color $fog.color "$profileId.fog.color"
        Assert-FiniteRange $fog.density 0.0 8.0 `
            "$profileId.fog.density"
        Assert-FiniteRange $fog.heightFalloff 0.0001 4.0 `
            "$profileId.fog.heightFalloff"
        Assert-FiniteRange $fog.topHeight -10000.0 10000.0 `
            "$profileId.fog.topHeight"
        Assert-FiniteRange $fog.startDistance 0.0 100000.0 `
            "$profileId.fog.startDistance"
        Assert-FiniteRange $fog.maximumOpacity 0.0 1.0 `
            "$profileId.fog.maximumOpacity"
        Assert-FiniteRange $fog.driftSpeed 0.0 8.0 `
            "$profileId.fog.driftSpeed"
        Assert-FiniteRange $fog.driftHeightAmplitude 0.0 1000.0 `
            "$profileId.fog.driftHeightAmplitude"
        Assert-FiniteRange $fog.driftDensityAmplitude 0.0 8.0 `
            "$profileId.fog.driftDensityAmplitude"
        Assert-FiniteRange $fog.coveragePercent 0.0 1.0 `
            "$profileId.fog.coveragePercent"
        Assert-FiniteRange $fog.windDirectionX -1.0 1.0 `
            "$profileId.fog.windDirectionX"
        Assert-FiniteRange $fog.windDirectionZ -1.0 1.0 `
            "$profileId.fog.windDirectionZ"
        Assert-FiniteRange $fog.windSpeed 0.0 200.0 `
            "$profileId.fog.windSpeed"
        Assert-FiniteRange $fog.patchScale 0.0001 1.0 `
            "$profileId.fog.patchScale"
        Assert-FiniteRange $fog.patchSoftness 0.001 0.5 `
            "$profileId.fog.patchSoftness"

        $effectiveExposure = [double]$global.exposure *
            [double]$profile.exposureMultiplier
        $effectiveBloom = [double]$global.bloomIntensity *
            [double]$profile.bloomIntensityMultiplier
        Assert-FiniteFloatRange $effectiveExposure 0.01 32.0 "$profileId.effectiveExposure"
        Assert-FiniteFloatRange $effectiveBloom 0.0 16.0 "$profileId.effectiveBloomIntensity"
    }

    $requiredIds = @(
        'scene.loading.neutral.v1',
        'scene.lobby.neutral.v1',
        'scene.character-select.warm-high-key.v1',
        'scene.bern.neutral-day.v1',
        'scene.valtan.cool-low-key.v1',
        'scene.development.neutral.v1')
    foreach ($requiredId in $requiredIds) {
        if (-not $ids.Contains($requiredId)) {
            throw "Required rendering profile is missing: $requiredId"
        }
    }
}

if (-not (Test-Path -LiteralPath $SourcePath -PathType Leaf)) {
    throw "Rendering profile source is missing: $SourcePath"
}
$sourceText = [IO.File]::ReadAllText(
    $SourcePath, [Text.UTF8Encoding]::new($false, $true))
Assert-NoDuplicateJsonObjectKeys $SourcePath
$document = $sourceText | ConvertFrom-Json
Assert-RenderingProfileDocument $document

if ($Mode -eq 'Validate') {
    Write-Host "Rendering profile validation PASS: $SourcePath"
    exit 0
}
if ($SourcePath -eq $DestinationPath) {
    throw 'Rendering profile source and destination must be different files.'
}

$destinationDirectory = [IO.Path]::GetDirectoryName($DestinationPath)
[IO.Directory]::CreateDirectory($destinationDirectory) | Out-Null
$temporaryPath = "$DestinationPath.tmp.$PID"
$backupPath = "$DestinationPath.bak.$PID"
try {
    $normalized = ($document | ConvertTo-Json -Depth 12) + [Environment]::NewLine
    [IO.File]::WriteAllText(
        $temporaryPath, $normalized, [Text.UTF8Encoding]::new($false))
    $roundTrip = [IO.File]::ReadAllText(
        $temporaryPath, [Text.UTF8Encoding]::new($false, $true)) |
        ConvertFrom-Json
    Assert-RenderingProfileDocument $roundTrip
    if (Test-Path -LiteralPath $DestinationPath -PathType Leaf) {
        [IO.File]::Replace(
            $temporaryPath, $DestinationPath, $backupPath, $true)
    }
    else {
        [IO.File]::Move($temporaryPath, $DestinationPath)
    }
}
finally {
    Remove-FileBestEffort $temporaryPath 'temporary rendering profile'
    Remove-FileBestEffort $backupPath 'rendering profile backup'
}

Write-Host "Rendering profile publish PASS: $DestinationPath"
