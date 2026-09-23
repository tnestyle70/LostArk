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

function Assert-SourceCharacterAmbient([object]$Value, [string]$Context) {
    Assert-Vector4 $Value 0.0 64.0 $Context
    Assert-FiniteFloatRange $Value[3] 0.0 0.0 "$Context reserved component"
}

function Assert-SourceFog([object]$Value) {
    Assert-Color $Value.inscatteringColor 'sourceFog inscatteringColor'
    Assert-Vector4 $Value.lightDirection -1 1 'sourceFog lightDirection'
    $d = $Value.lightDirection
    if ([Math]::Abs($d[0]*$d[0]+$d[1]*$d[1]+$d[2]*$d[2]-1) -gt .001) { throw 'Source fog light direction is not normalized.' }
}

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

    # Workbench writes nine significant digits. Read the value as binary32
    # before comparing, as CRenderingProfileService::Read_Float does; the
    # decimal spelling may be just outside the exact binary32 boundary.
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number)) {
        throw "$Context must be finite in [$Minimum, $Maximum]."
    }
    try { $runtimeValue = [single]$number }
    catch { throw "$Context must fit a finite float32 value." }
    if ([single]::IsNaN($runtimeValue) -or [single]::IsInfinity($runtimeValue) -or
        $runtimeValue -lt $Minimum -or $runtimeValue -gt $Maximum) {
        throw "$Context must be finite in [$Minimum, $Maximum]."
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
        Assert-FiniteRange $items[$index] -64.0 64.0 "$Context[$index]"
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
        Assert-FiniteRange $items[$index] -100000.0 100000.0 "$Context[$index]"
        Assert-FiniteFloatRange $items[$index] $Minimum $Maximum "$Context[$index]"
    }
}

function Assert-Color([object]$Value, [string]$Context) {
    Assert-Vector4 $Value 0.0 64.0 $Context
    $items = @($Value)
    Assert-FiniteFloatRange $items[3] 0.0 1.0 "$Context[3]"
}

function Assert-BloomTint([object]$Value, [string]$Context) {
    Assert-Vector4 $Value 0.0 1.0 $Context
    Assert-FiniteFloatRange @($Value)[3] 1.0 1.0 "$Context[3]"
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

function Assert-SourcePostProcess([object]$Value) {
    Assert-ExactProperties $Value @('enabled','toneCurve','toneScale','toneRange','toneToe','highlights','midtones','shadows','colorize','desaturation','colorGradingLut') 'sourcePostProcess'
    if ($Value.enabled -isnot [bool] -or $Value.toneCurve -cne 'UE3_CUSTOMIZABLE') { throw 'sourcePostProcess requires boolean enabled and UE3_CUSTOMIZABLE toneCurve.' }
    Assert-FiniteFloatRange $Value.toneScale .000001 64 'sourcePostProcess.toneScale'
    Assert-FiniteFloatRange $Value.toneRange .000001 128 'sourcePostProcess.toneRange'
    Assert-FiniteFloatRange $Value.toneToe 0 1 'sourcePostProcess.toneToe'
    Assert-FiniteFloatRange $Value.desaturation 0 1 'sourcePostProcess.desaturation'
    Assert-Vector3 $Value.highlights .000001 64 'sourcePostProcess.highlights'
    Assert-Vector3 $Value.midtones .000001 64 'sourcePostProcess.midtones'
    Assert-Vector3 $Value.shadows -64 64 'sourcePostProcess.shadows'
    Assert-Vector3 $Value.colorize 0 64 'sourcePostProcess.colorize'
    $assetId = $Value.colorGradingLut
    if ($assetId -isnot [string]) { throw 'sourcePostProcess.colorGradingLut must be a string.' }
    if ($assetId.Length -gt 0) {
        if ($assetId.Length -gt 1024 -or $assetId -match '[\\:\x00-\x1f]' -or $assetId.StartsWith('/') -or
            $assetId -match '(^|/)\.\.?(/|$)' -or -not $assetId.EndsWith('.dds',[StringComparison]::Ordinal)) { throw 'Source LUT must use a Resources-relative DDS asset ID.' }
        $lutPath = Join-Path (Join-Path $repoRoot 'Client\Bin\Resources') $assetId
        if (-not (Test-Path -LiteralPath $lutPath -PathType Leaf)) { throw "Source LUT is missing: $assetId" }
        $bytes = [IO.File]::ReadAllBytes($lutPath)
        if ($bytes.Length -lt 128) { throw 'Source LUT DDS header is truncated.' }
        $h = @(0..31 | ForEach-Object { [BitConverter]::ToUInt32($bytes,4*$_) })
        if ($h[0] -ne 0x20534444 -or $h[1] -ne 124 -or $h[3] -ne 16 -or $h[4] -ne 256 -or
            $h[6] -gt 1 -or $h[7] -gt 1 -or $h[19] -ne 32 -or $h[28] -ne 0) { throw 'Source LUT requires a 256x16 no-mip DDS atlas.' }
        $offset = 128
        if ($h[21] -eq 0x30315844) {
            if ($bytes.Length -lt 148) { throw 'Source LUT DX10 header is truncated.' }
            $dx = @(0..4 | ForEach-Object { [BitConverter]::ToUInt32($bytes,128+4*$_) })
            if ($dx[0] -notin @(28,87) -or $dx[1] -ne 3 -or $dx[2] -ne 0 -or $dx[3] -ne 1) { throw 'Source LUT must use linear RGBA8/BGRA8 DDS.' }
            $offset = 148
        } else {
            $bgra = $h[23] -eq 16711680 -and $h[25] -eq 255
            $rgba = $h[23] -eq 255 -and $h[25] -eq 16711680
            if ($h[21] -ne 0 -or ($h[20] -band 64) -eq 0 -or $h[22] -ne 32 -or
                $h[24] -ne 65280 -or $h[26] -ne 4278190080 -or -not ($bgra -or $rgba)) { throw 'Source LUT must use linear RGBA8/BGRA8 DDS.' }
        }
        if ($bytes.Length -ne $offset+16384) { throw 'Source LUT DDS payload size is invalid.' }
    }
}

function Assert-RenderingQuality([object]$global) {
    $qualityFields = @(
        'ssaoEnabled', 'ssaoRadius', 'ssaoBias', 'ssaoIntensity',
        'ssaoPower', 'ssaoDistanceFade',
        'bloomEnabled', 'bloomThreshold', 'bloomSoftKnee', 'bloomIntensity',
        'bloomScatter', 'exposure', 'whitePoint', 'gamma', 'fxaaEnabled',
        'fxaaSubpixel', 'fxaaEdgeThreshold', 'fxaaEdgeThresholdMin')
    if ($null -ne $global.PSObject.Properties['colorAdjustment']) {
        $qualityFields += 'colorAdjustment'
        $adjustment = $global.colorAdjustment
        Assert-ExactProperties $adjustment @('bloomTint', 'desaturation') 'quality.colorAdjustment'
        Assert-BloomTint $adjustment.bloomTint 'quality.colorAdjustment.bloomTint'
        Assert-FiniteFloatRange $adjustment.desaturation 0.0 1.0 'quality.colorAdjustment.desaturation'
    }
    if ($null -ne $global.PSObject.Properties['sourcePostProcess']) {
        $qualityFields += 'sourcePostProcess'
        Assert-SourcePostProcess $global.sourcePostProcess
    }
    Assert-ExactProperties $global $qualityFields 'globalQuality'
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
    Assert-RenderingQuality $global

    $profiles = @($Document.profiles)
    if ($profiles.Count -lt 1 -or $profiles.Count -gt 32) {
        throw 'profiles must contain between 1 and 32 entries.'
    }
    $ids = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($profile in $profiles) {
        $profileFields = @('profileId', 'exposureMultiplier', 'bloomIntensityMultiplier', 'light', 'shadow', 'fog')
        if ($null -ne $profile.PSObject.Properties['environment']) {
            $profileFields += 'environment'
            $environment = $profile.environment
            $environmentFields = @('cubeTexture', 'color', 'rotationIntensity')
            if ($null -ne $environment.PSObject.Properties['useSourcePBRIndirect']) {
                $environmentFields += 'useSourcePBRIndirect'
                if ($environment.useSourcePBRIndirect -isnot [bool]) {
                    throw 'profile.environment.useSourcePBRIndirect must be a boolean.'
                }
            }
            if ($null -ne $environment.PSObject.Properties['cubeDiffuse']) {
                $environmentFields += 'cubeDiffuse'
                $diffuse = $environment.cubeDiffuse
                Assert-ExactProperties $diffuse @('model', 'intensity', 'packedSH') 'profile.environment.cubeDiffuse'
                if ($diffuse.model -isnot [string] -or $diffuse.model -cne 'RGBM6_LAMBERT_SH3') {
                    throw 'profile.environment.cubeDiffuse.model must be RGBM6_LAMBERT_SH3.'
                }
                Assert-FiniteFloatRange $diffuse.intensity 0.0 4.0 'profile.environment.cubeDiffuse.intensity'
                if ($diffuse.packedSH -isnot [Array] -or $diffuse.packedSH.Count -ne 7) {
                    throw 'profile.environment.cubeDiffuse.packedSH requires seven float4 rows.'
                }
                for ($rowIndex = 0; $rowIndex -lt 7; ++$rowIndex) {
                    $row = $diffuse.packedSH[$rowIndex]
                    if ($row -isnot [Array]) { throw 'profile.environment.cubeDiffuse.packedSH row must be an array.' }
                    Assert-Vector4 $row -64.0 64.0 "profile.environment.cubeDiffuse.packedSH[$rowIndex]"
                }
                Assert-FiniteRange $diffuse.packedSH[6][3] 0.0 0.0 'profile.environment.cubeDiffuse.packedSH[6][3]'
            }
            Assert-ExactProperties $environment $environmentFields 'profile.environment'
            $assetId = $environment.cubeTexture
            if ($assetId -isnot [string] -or [string]::IsNullOrEmpty($assetId) -or
                $assetId.Length -gt 1024 -or $assetId -match '[\\\x00-\x1f:]' -or
                $assetId.StartsWith('/') -or $assetId -match '(^|/)\.{1,2}(/|$)' -or
                -not $assetId.EndsWith('.dds', [StringComparison]::Ordinal)) {
                throw 'profile.environment.cubeTexture must be a Resources-relative DDS asset ID.'
            }
            Assert-Vector4 $environment.color 0.0 64.0 'profile.environment.color'
            $rotation = @($environment.rotationIntensity)
            if ($rotation.Count -ne 4) { throw 'profile.environment.rotationIntensity requires four numbers.' }
            foreach ($component in $rotation) {
                Assert-FiniteRange $component -64.0 64.0 'profile.environment.rotationIntensity'
            }
            Assert-FiniteFloatRange $rotation[0] -1.0 1.0 'profile.environment.rotationIntensity[0]'
            Assert-FiniteFloatRange $rotation[1] -1.0 1.0 'profile.environment.rotationIntensity[1]'
            Assert-FiniteFloatRange $rotation[2] 0.0 64.0 'profile.environment.rotationIntensity[2]'
            Assert-FiniteFloatRange $rotation[3] 0.0 0.0 'profile.environment.rotationIntensity[3]'
            if ([Math]::Abs([single]$rotation[0] * [single]$rotation[0] +
                [single]$rotation[1] * [single]$rotation[1] - 1.0) -gt 0.001) {
                throw 'profile.environment.rotationIntensity.xy must be a unit rotation.'
            }
        }
        if ($null -ne $profile.PSObject.Properties['displayName']) {
            $profileFields += 'displayName'
            if ($profile.displayName -isnot [string] -or [string]::IsNullOrEmpty($profile.displayName) -or
                $profile.displayName.Contains([string][char]0) -or
                [Text.UTF8Encoding]::new($false, $true).GetByteCount($profile.displayName) -gt 256) {
                throw 'profile.displayName must contain 1 to 256 valid UTF-8 bytes without NUL.'
            }
        }
        if ($null -ne $profile.PSObject.Properties['mapLightIntensityMultiplier']) {
            $profileFields += 'mapLightIntensityMultiplier'
            Assert-FiniteFloatRange $profile.mapLightIntensityMultiplier 0.0 4.0 `
                'profile.mapLightIntensityMultiplier'
        }
        $quality = $global
        if ($null -ne $profile.PSObject.Properties['qualityOverride']) {
            $profileFields += 'qualityOverride'
            Assert-RenderingQuality $profile.qualityOverride
            $quality = $profile.qualityOverride
        }
        if ($null -ne $profile.PSObject.Properties['environmentRegions']) {
            $profileFields += 'environmentRegions'
            $regions = @($profile.environmentRegions)
            if ($regions.Count -lt 1 -or $regions.Count -gt 64) { throw 'environmentRegions requires 1 to 64 volumes.' }
            $regionIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            foreach ($region in $regions) {
                $regionFields = @('regionId','boundsMinimum','boundsMaximum','planes','fog','directionalColor','ambientColor','blendTimeIn','blendTimeOut')
                if ($null -ne $region.PSObject.Properties['priority']) {
                    $regionFields += 'priority'
                    Assert-FiniteFloatRange $region.priority -100000.0 100000.0 'environmentRegion.priority'
                }
                if ($null -ne $region.PSObject.Properties['receiver']) {
                    $regionFields += 'receiver'
                    if ($region.receiver -isnot [string] -or
                        @('ALL', 'SOURCE_CHARACTER', 'UNBAKED') -cnotcontains $region.receiver) {
                        throw 'environmentRegion.receiver must be ALL, SOURCE_CHARACTER or UNBAKED.'
                    }
                }
                if ($null -ne $region.PSObject.Properties['sourceCharacterAmbient']) {
                    $regionFields += 'sourceCharacterAmbient'
                    Assert-SourceCharacterAmbient $region.sourceCharacterAmbient 'environmentRegion.sourceCharacterAmbient'
                }
                if ($null -ne $region.PSObject.Properties['specularColor']) {
                    $regionFields += 'specularColor'
                    Assert-Color $region.specularColor 'environmentRegion.specularColor'
                }
                if ($null -ne $region.PSObject.Properties['qualityOverride']) {
                    $regionFields += 'qualityOverride'
                    Assert-RenderingQuality $region.qualityOverride
                    $regionExposure = [single]([single]$region.qualityOverride.exposure * [single]$profile.exposureMultiplier)
                    $regionQualityBloom = [single]([single]$region.qualityOverride.bloomIntensity * [single]$profile.bloomIntensityMultiplier)
                    Assert-FiniteFloatRange $regionExposure 0.01 32.0 'environmentRegion.qualityOverride.effectiveExposure'
                    Assert-FiniteFloatRange $regionQualityBloom 0.0 16.0 'environmentRegion.qualityOverride.effectiveBloomIntensity'
                }
                if ($null -ne $region.PSObject.Properties['postProcess']) {
                    $regionFields += 'postProcess'
                    $postProcess = $region.postProcess
                    $postFields = @('bloomThreshold','bloomIntensity','bloomTint','desaturation')
                    if ($null -ne $postProcess.PSObject.Properties['sourcePostProcess']) {
                        $postFields += 'sourcePostProcess'
                        Assert-SourcePostProcess $postProcess.sourcePostProcess
                    }
                    Assert-ExactProperties $postProcess $postFields 'environmentRegion.postProcess'
                    Assert-FiniteFloatRange $postProcess.bloomThreshold 0.0 64.0 'environmentRegion.postProcess.bloomThreshold'
                    Assert-FiniteFloatRange $postProcess.bloomIntensity 0.0 16.0 'environmentRegion.postProcess.bloomIntensity'
                    $regionBloom = [single]([single]$postProcess.bloomIntensity * [single]$profile.bloomIntensityMultiplier)
                    Assert-FiniteFloatRange $regionBloom 0.0 16.0 'environmentRegion.postProcess.effectiveBloomIntensity'
                    Assert-BloomTint $postProcess.bloomTint 'environmentRegion.postProcess.bloomTint'
                    Assert-FiniteFloatRange $postProcess.desaturation 0.0 1.0 'environmentRegion.postProcess.desaturation'
                }
                Assert-ExactProperties $region $regionFields 'environmentRegion'
                if ($region.regionId -isnot [string] -or $region.regionId -cnotmatch '^[A-Za-z0-9_.-]{1,128}$' -or !$regionIds.Add($region.regionId)) { throw 'Invalid or duplicate environment region ID.' }
                foreach ($bound in @('boundsMinimum','boundsMaximum')) {
                    if (@($region.$bound).Count -ne 3) { throw 'Environment bound requires 3 numbers.' }
                    foreach ($number in $region.$bound) { Assert-FiniteRange $number -100000 100000 'environment bound' }
                }
                for ($axis=0;$axis -lt 3;$axis++) { if ($region.boundsMinimum[$axis] -ge $region.boundsMaximum[$axis]) { throw 'Empty environment volume bounds.' } }
                $planes = @($region.planes)
                if ($planes.Count -lt 4 -or $planes.Count -gt 64) { throw 'Environment volume requires 4 to 64 planes.' }
                foreach ($plane in $planes) {
                    if (@($plane).Count -ne 4) { throw 'Environment plane requires 4 numbers.' }
                    # The runtime bounds these raw double coordinates before conversion.
                    foreach ($coordinate in $plane) { Assert-FiniteRange $coordinate -100000 100000 'environment plane' }
                    if ([Math]::Abs($plane[0]*$plane[0]+$plane[1]*$plane[1]+$plane[2]*$plane[2]-1) -gt .001) { throw 'Environment plane normal is not normalized.' }
                }
                Assert-FiniteFloatRange $region.blendTimeIn 0 60 'environment blendTimeIn'
                Assert-FiniteFloatRange $region.blendTimeOut 0 60 'environment blendTimeOut'
                Assert-Color $region.directionalColor 'environment directionalColor'
                Assert-Color $region.ambientColor 'environment ambientColor'
                $regionFog = $region.fog
                Assert-ExactProperties $regionFog @('density','heightFalloff','topHeight','startDistance','maximumOpacity','color','inscatteringColor','lightDirection') 'environment fog'
                foreach ($v in @(@('density',0,8),@('heightFalloff',.0001,4),@('topHeight',-10000,10000),@('startDistance',0,100000),@('maximumOpacity',0,1))) {
                    Assert-FiniteFloatRange $regionFog.($v[0]) $v[1] $v[2] ('environment '+$v[0])
                }
                Assert-Color $regionFog.color 'environment color'
                Assert-SourceFog $regionFog
            }
        }
        Assert-ExactProperties $profile $profileFields 'profile'
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
        $lightFields = @('type', 'direction', 'diffuse', 'ambient', 'specular')
        if ($null -ne $light.PSObject.Properties['receiver']) {
            $lightFields += 'receiver'
            if ($light.receiver -isnot [string] -or
                @('ALL', 'SOURCE_CHARACTER', 'UNBAKED') -cnotcontains $light.receiver) {
                throw "$profileId.light.receiver must be ALL, SOURCE_CHARACTER or UNBAKED."
            }
        }
        if ($null -ne $light.PSObject.Properties['sourceCharacterAmbient']) {
            $lightFields += 'sourceCharacterAmbient'
            Assert-SourceCharacterAmbient $light.sourceCharacterAmbient "$profileId.light.sourceCharacterAmbient"
        }
        Assert-ExactProperties $light $lightFields "$profileId.light"
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
        $shadowFields = @('enabled', 'focus', 'distance', 'orthographicWidth',
            'orthographicHeight', 'near', 'far', 'depthBias', 'normalBias', 'strength')
        if ($shadow.PSObject.Properties.Name -contains 'dynamicBakedStrength') {
            $shadowFields += 'dynamicBakedStrength'
            Assert-FiniteFloatRange $shadow.dynamicBakedStrength 0.0 1.0 "$profileId.shadow.dynamicBakedStrength"
        }
        Assert-ExactProperties $shadow $shadowFields "$profileId.shadow"
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
        if ([single]$shadow.far -le [single]$shadow.near) {
            throw "$profileId.shadow.far must be greater than near."
        }
        Assert-FiniteFloatRange $shadow.depthBias 0.0 0.05 `
            "$profileId.shadow.depthBias"
        Assert-FiniteFloatRange $shadow.normalBias 0.0 10.0 `
            "$profileId.shadow.normalBias"
        Assert-FiniteFloatRange $shadow.strength 0.0 1.0 `
            "$profileId.shadow.strength"

        $fog = $profile.fog
        $fogFields = @(
            'enabled', 'color', 'density', 'heightFalloff',
            'topHeight', 'startDistance', 'maximumOpacity',
            'driftSpeed', 'driftHeightAmplitude',
            'driftDensityAmplitude', 'coveragePercent',
            'windDirectionX', 'windDirectionZ', 'windSpeed',
            'patchScale', 'patchSoftness')
        if ($null -ne $fog.PSObject.Properties['sourceExponential']) {
            $fogFields += 'sourceExponential'
            Assert-ExactProperties $fog.sourceExponential @('inscatteringColor','lightDirection') 'sourceExponential fog'
            Assert-SourceFog $fog.sourceExponential
        }
        Assert-ExactProperties $fog $fogFields "$profileId.fog"
        if ($fog.enabled -isnot [bool]) {
            throw "$profileId.fog.enabled must be boolean."
        }
        Assert-Color $fog.color "$profileId.fog.color"
        Assert-FiniteFloatRange $fog.density 0.0 8.0 `
            "$profileId.fog.density"
        Assert-FiniteFloatRange $fog.heightFalloff 0.0001 4.0 `
            "$profileId.fog.heightFalloff"
        Assert-FiniteFloatRange $fog.topHeight -10000.0 10000.0 `
            "$profileId.fog.topHeight"
        Assert-FiniteFloatRange $fog.startDistance 0.0 100000.0 `
            "$profileId.fog.startDistance"
        Assert-FiniteFloatRange $fog.maximumOpacity 0.0 1.0 `
            "$profileId.fog.maximumOpacity"
        Assert-FiniteFloatRange $fog.driftSpeed 0.0 8.0 `
            "$profileId.fog.driftSpeed"
        Assert-FiniteFloatRange $fog.driftHeightAmplitude 0.0 1000.0 `
            "$profileId.fog.driftHeightAmplitude"
        Assert-FiniteFloatRange $fog.driftDensityAmplitude 0.0 8.0 `
            "$profileId.fog.driftDensityAmplitude"
        Assert-FiniteFloatRange $fog.coveragePercent 0.0 1.0 `
            "$profileId.fog.coveragePercent"
        Assert-FiniteFloatRange $fog.windDirectionX -1.0 1.0 `
            "$profileId.fog.windDirectionX"
        Assert-FiniteFloatRange $fog.windDirectionZ -1.0 1.0 `
            "$profileId.fog.windDirectionZ"
        Assert-FiniteFloatRange $fog.windSpeed 0.0 200.0 `
            "$profileId.fog.windSpeed"
        Assert-FiniteFloatRange $fog.patchScale 0.0001 1.0 `
            "$profileId.fog.patchScale"
        Assert-FiniteFloatRange $fog.patchSoftness 0.001 0.5 `
            "$profileId.fog.patchSoftness"

        $effectiveExposure = [single]([single]$quality.exposure *
            [single]$profile.exposureMultiplier)
        $effectiveBloom = [single]([single]$quality.bloomIntensity *
            [single]$profile.bloomIntensityMultiplier)
        Assert-FiniteFloatRange $effectiveExposure 0.01 32.0 "$profileId.effectiveExposure"
        Assert-FiniteFloatRange $effectiveBloom 0.0 16.0 "$profileId.effectiveBloomIntensity"
    }

    $requiredIds = @(
        'scene.loading.neutral.v1',
        'scene.lobby.neutral.v1',
        'scene.character-select.warm-high-key.v1',
        'scene.bern.neutral-day.v1',
        'scene.valtan.cool-low-key.v1',
        'scene.development.neutral.v1',
        'scene.kakulsaydon.g1.base.v1')
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
