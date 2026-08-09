param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$DataRoot,
    [string]$ResourceRoot,
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ([string]::IsNullOrWhiteSpace($DataRoot)) {
    $DataRoot = Join-Path $repoRoot 'Data'
}
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
}
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $repoRoot 'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
}
$DataRoot = [IO.Path]::GetFullPath($DataRoot)
$ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
$catalogPath = Join-Path $DataRoot 'Effects\EffectCatalog.json'
$authoringRoot = [IO.Path]::GetFullPath((Join-Path $DataRoot 'Effects\Authored'))
$assemblyRoot = [IO.Path]::GetFullPath((Join-Path $DataRoot 'Effects\Assemblies'))
$componentRoot = [IO.Path]::GetFullPath((Join-Path $DataRoot 'Effects\Components'))
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$supportedSourceRuntimeShaderProfiles = @(
    'effect.ue3.reconstructed-standard.v1',
    'effect.ue3.fallback-blocked.v1',
    'effect.ue3.circle.v1',
    'effect.ue3.dot.v1',
    'effect.ue3.ring.v1',
    'effect.ue3.aura.v1',
    'effect.ue3.one-layer-distortion.v1',
    'effect.ue3.grouped-translucent.v1',
    'effect.ue3.shine.v1',
    'effect.ue3.blackline-aura.v1',
    'effect.ue3.linearflow-02.v1',
    'effect.ue3.local-crack.v1',
    'effect.ue3.procedural-center-glow.v1',
    'effect.ue3.slice.v1',
    'effect.ue3.missiletrail-01.v1'
)
$supportedSourceDynamicParameterSemantics = @(
    'unbound', 'opacity', 'emissive', 'dissolve', 'uv_pan',
    'distortion', 'radial_size',
    'mask_a_offset', 'mask_b_offset', 'mask_a_distort', 'mask_b_distort',
    'mask_a_pan', 'flow_strength', 'mask_b_pan', 'diffuse_pan',
    'missile_alpha_pan', 'missile_noise_strength', 'missile_noise_pan',
    'missile_dissolve'
)
$supportedSourceSubUVModes = @(
    'none', 'psuvim_linear_blend',
    'psuvim_linear_blend_random_flip_square'
)

function Read-JsonDocument([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing JSON document: $Path"
    }
    return [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8) |
        ConvertFrom-Json
}

function Get-RequiredProperty(
    [object]$Object,
    [string]$Name,
    [ValidateSet('String','Number','Boolean','Array','Object')]
    [string]$Kind) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        throw "Required property '$Name' has the wrong type."
    }
    $value = $property.Value
    $valid = switch ($Kind) {
        'String' { $value -is [string] }
        'Number' { $value -is [byte] -or $value -is [int16] -or
            $value -is [int32] -or $value -is [int64] -or
            $value -is [single] -or $value -is [double] -or
            $value -is [decimal] }
        'Boolean' { $value -is [bool] }
        'Array' { $null -eq $value -or $value -is [array] }
        'Object' { $value -is [pscustomobject] -or
            $value -is [Collections.IDictionary] }
    }
    if (-not $valid) {
        $actualType = if ($null -eq $value) { 'null' } else {
            $value.GetType().FullName
        }
        throw "Required property '$Name' has the wrong type: $actualType"
    }
    return $value
}

function Assert-StableId([string]$Value, [string]$Label) {
    if ($Value.Length -lt 1 -or $Value.Length -gt 128 -or
        $Value -notmatch '^[A-Za-z0-9_.-]+$') {
        throw "$Label is not a stable ID: $Value"
    }
}

function Get-NumberValue([object]$Object, [string]$Name, [string]$Label) {
    $value = [double](Get-RequiredProperty $Object $Name Number)
    if ([double]::IsNaN($value) -or [double]::IsInfinity($value)) {
        throw "$Label.$Name must be finite."
    }
    return $value
}

function Get-IntegerValue([object]$Object, [string]$Name, [string]$Label) {
    $value = Get-NumberValue $Object $Name $Label
    if ([Math]::Floor($value) -ne $value -or
        $value -lt [int64]::MinValue -or $value -gt [int64]::MaxValue) {
        throw "$Label.$Name must be an integer."
    }
    return [int64]$value
}

function Get-NumberVector(
    [object]$Object, [string]$Name, [int]$Count, [string]$Label) {
    $raw = @(Get-RequiredProperty $Object $Name Array)
    if ($raw.Count -ne $Count) {
        throw "$Label.$Name must contain $Count numbers."
    }
    $values = [Collections.Generic.List[double]]::new()
    foreach ($item in $raw) {
        if (-not ($item -is [byte] -or $item -is [int16] -or
            $item -is [int32] -or $item -is [int64] -or
            $item -is [single] -or $item -is [double] -or
            $item -is [decimal])) {
            throw "$Label.$Name must contain only numbers."
        }
        $number = [double]$item
        if ([double]::IsNaN($number) -or [double]::IsInfinity($number)) {
            throw "$Label.$Name must contain only finite numbers."
        }
        $values.Add($number)
    }
    return $values.ToArray()
}

function Assert-EffectDetail(
    [object]$Detail, [string]$Kind, [string]$Label) {
    $transform = Get-RequiredProperty $Detail 'transform' Object
    $color = Get-RequiredProperty $Detail 'color' Object
    $uv = Get-RequiredProperty $Detail 'uv' Object
    $timing = Get-RequiredProperty $Detail 'timing' Object
    $mesh = Get-RequiredProperty $Detail 'mesh' Object
    $sprite = Get-RequiredProperty $Detail 'sprite' Object
    $decal = Get-RequiredProperty $Detail 'decal' Object
    $lerp = Get-RequiredProperty $Detail 'linearLerp' Object
    $particle = Get-RequiredProperty $Detail 'particle' Object
    $trail = Get-RequiredProperty $Detail 'trail' Object
    $afterImage = Get-RequiredProperty $Detail 'afterImage' Object

    [void](Get-NumberVector $transform 'position' 3 $Label)
    [void](Get-NumberVector $transform 'rotationDegrees' 3 $Label)
    [void](Get-NumberVector $transform 'revolutionDegreesPerSecond' 3 $Label)
    $scale = Get-NumberVector $transform 'scale' 3 $Label
    [void](Get-NumberVector $transform 'velocityPerSecond' 3 $Label)
    if (@($scale | Where-Object { $_ -le 0 }).Count -ne 0) {
        throw "$Label transform.scale must be positive."
    }

    [void](Get-NumberVector $color 'offset' 4 $Label)
    [void](Get-NumberVector $color 'multiply' 4 $Label)
    $clip = Get-NumberValue $color 'clip' $Label
    $emissive = Get-NumberValue $color 'emissiveIntensity' $Label
    $distortion = Get-NumberValue $color 'distortionIntensity' $Label
    [void](Get-RequiredProperty $color 'distortionOnBaseMaterial' Boolean)
    [void](Get-NumberValue $color 'radialTime' $Label)
    [void](Get-NumberValue $color 'radialIntensity' $Label)
    if ($clip -lt 0 -or $clip -gt 1 -or $emissive -lt 0 -or $distortion -lt 0) {
        throw "$Label color range is invalid."
    }

    [void](Get-NumberVector $uv 'start' 2 $Label)
    [void](Get-NumberVector $uv 'speed' 2 $Label)
    [void](Get-RequiredProperty $uv 'wave' Boolean)
    [void](Get-NumberVector $uv 'waveAmplitude' 2 $Label)
    $waveFrequency = Get-NumberValue $uv 'waveFrequency' $Label
    [void](Get-RequiredProperty $uv 'sequence' Boolean)
    [void](Get-RequiredProperty $uv 'loop' Boolean)
    $sequenceTerm = Get-NumberValue $uv 'sequenceTerm' $Label
    $tileColumns = Get-IntegerValue $uv 'tileColumns' $Label
    $tileRows = Get-IntegerValue $uv 'tileRows' $Label
    $tileIndex = Get-IntegerValue $uv 'tileIndex' $Label
    $tileCount = $tileColumns * $tileRows
    if ($waveFrequency -lt 0 -or $sequenceTerm -le 0 -or
        $tileColumns -le 0 -or $tileRows -le 0 -or
        $tileIndex -lt 0 -or $tileIndex -ge $tileCount) {
        throw "$Label UV range is invalid."
    }

    $startDelay = Get-NumberValue $timing 'startDelaySeconds' $Label
    $lifeTime = Get-NumberValue $timing 'lifeTimeSeconds' $Label
    $afterImageSeconds = Get-NumberValue $timing 'afterImageSeconds' $Label
    $dissolveStart = Get-NumberValue $timing 'dissolveStartNormalized' $Label
    if ($startDelay -lt 0 -or $lifeTime -le 0 -or $afterImageSeconds -lt 0 -or
        $dissolveStart -lt 0 -or $dissolveStart -gt 1) {
        throw "$Label timing range is invalid."
    }

    [void](Get-RequiredProperty $mesh 'useModelMaterial' Boolean)
    [void](Get-RequiredProperty $sprite 'billboard' Boolean)
    $decalSize = Get-NumberVector $decal 'size' 2 $Label
    $decalDepth = Get-NumberValue $decal 'depth' $Label
    if (@($decalSize | Where-Object { $_ -le 0 }).Count -ne 0 -or
        $decalDepth -le 0) {
        throw "$Label decal range is invalid."
    }

    foreach ($name in @('position','rotation','revolution','scale','velocity',
        'colorOffset','colorMultiply','emissiveIntensity')) {
        [void](Get-RequiredProperty $lerp $name Boolean)
    }
    [void](Get-NumberVector $lerp 'endPosition' 3 $Label)
    [void](Get-NumberVector $lerp 'endRotationDegrees' 3 $Label)
    [void](Get-NumberVector $lerp 'endRevolutionDegreesPerSecond' 3 $Label)
    $endScale = Get-NumberVector $lerp 'endScale' 3 $Label
    [void](Get-NumberVector $lerp 'endVelocityPerSecond' 3 $Label)
    [void](Get-NumberVector $lerp 'endColorOffset' 4 $Label)
    [void](Get-NumberVector $lerp 'endColorMultiply' 4 $Label)
    $endEmissive = Get-NumberValue $lerp 'endEmissiveIntensity' $Label
    if (@($endScale | Where-Object { $_ -le 0 }).Count -ne 0 -or
        $endEmissive -lt 0) {
        throw "$Label linearLerp range is invalid."
    }

    $maxParticles = Get-IntegerValue $particle 'maxParticles' $Label
    $spawnRate = Get-NumberValue $particle 'spawnRatePerSecond' $Label
    $burstCount = Get-IntegerValue $particle 'burstCount' $Label
    $randomSeed = Get-IntegerValue $particle 'randomSeed' $Label
	$particleLife = Get-NumberVector $particle 'lifeTimeSeconds' 2 $Label
	$positionMin = if ($null -ne $particle.initialPositionMin) {
		Get-NumberVector $particle 'initialPositionMin' 3 $Label
	} else { @(0.0, 0.0, 0.0) }
	$positionMax = if ($null -ne $particle.initialPositionMax) {
		Get-NumberVector $particle 'initialPositionMax' 3 $Label
	} else { @(0.0, 0.0, 0.0) }
	$velocityMin = Get-NumberVector $particle 'initialVelocityMin' 3 $Label
    $velocityMax = Get-NumberVector $particle 'initialVelocityMax' 3 $Label
    [void](Get-NumberVector $particle 'acceleration' 3 $Label)
    $startSize = Get-NumberVector $particle 'startSize' 2 $Label
    $endSize = Get-NumberVector $particle 'endSize' 2 $Label
    [void](Get-RequiredProperty $particle 'localSpace' Boolean)
    [void](Get-RequiredProperty $particle 'billboard' Boolean)
    if ($maxParticles -lt 1 -or $maxParticles -gt 2048 -or
        $spawnRate -lt 0 -or $spawnRate -gt 2048 -or
        $burstCount -lt 0 -or $burstCount -gt $maxParticles -or
        $randomSeed -le 0 -or $particleLife[0] -le 0 -or
		$particleLife[1] -lt $particleLife[0] -or $particleLife[1] -gt 30 -or
		$positionMax[0] -lt $positionMin[0] -or
		$positionMax[1] -lt $positionMin[1] -or
		$positionMax[2] -lt $positionMin[2] -or
        $velocityMax[0] -lt $velocityMin[0] -or
        $velocityMax[1] -lt $velocityMin[1] -or
        $velocityMax[2] -lt $velocityMin[2] -or
        @($startSize | Where-Object { $_ -le 0 }).Count -ne 0 -or
        @($endSize | Where-Object { $_ -lt 0 }).Count -ne 0) {
        throw "$Label particle budget or range is invalid."
    }

    $maxPoints = Get-IntegerValue $trail 'maxPoints' $Label
    $pointLife = Get-NumberValue $trail 'pointLifeTimeSeconds' $Label
    $sampleInterval = Get-NumberValue $trail 'sampleIntervalSeconds' $Label
    $minimumDistance = Get-NumberValue $trail 'minimumDistance' $Label
    $startWidth = Get-NumberValue $trail 'startWidth' $Label
    $endWidth = Get-NumberValue $trail 'endWidth' $Label
    [void](Get-RequiredProperty $trail 'faceCamera' Boolean)
    if ($maxPoints -lt 2 -or $maxPoints -gt 256 -or $pointLife -le 0 -or
        $sampleInterval -le 0 -or $minimumDistance -lt 0 -or
        $startWidth -le 0 -or $endWidth -lt 0) {
        throw "$Label trail budget or range is invalid."
    }

    $afterSample = Get-NumberValue $afterImage 'sampleIntervalSeconds' $Label
    $maxCopies = Get-IntegerValue $afterImage 'maxCopies' $Label
    $alphaExponent = Get-NumberValue $afterImage 'alphaExponent' $Label
    if ($afterSample -le 0 -or $maxCopies -lt 0 -or $maxCopies -gt 32 -or
        $alphaExponent -le 0 -or
        ($afterImageSeconds -gt 0 -and $maxCopies -gt 0 -and
            $Kind -notin @('mesh','sprite'))) {
        throw "$Label after-image budget or range is invalid."
    }
}

function Resolve-SafeResource([string]$AssetId) {
    if (-not $AssetId.StartsWith('Effect/', [StringComparison]::Ordinal) -or
        $AssetId.Contains('\') -or $AssetId.Contains(':') -or
        [IO.Path]::IsPathRooted($AssetId)) {
        throw "Unsafe Effect resource asset ID: $AssetId"
    }
    foreach ($segment in $AssetId.Split('/')) {
        if ([string]::IsNullOrWhiteSpace($segment) -or
            $segment -eq '.' -or $segment -eq '..') {
            throw "Unsafe Effect resource path segment: $AssetId"
        }
    }
    $candidate = [IO.Path]::GetFullPath((Join-Path $ResourceRoot $AssetId))
    $prefix = $ResourceRoot.TrimEnd('\') + '\'
    if (-not $candidate.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Effect resource escaped Resources: $AssetId"
    }
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw "Missing Effect resource: $AssetId"
    }
    return $candidate
}

function Resolve-SafeModelCueResource([string]$AssetId) {
    if (-not $AssetId.StartsWith('Character/', [StringComparison]::Ordinal) -or
        $AssetId.Contains('\') -or $AssetId.Contains(':') -or
        [IO.Path]::IsPathRooted($AssetId) -or
        [IO.Path]::GetExtension($AssetId) -cne '.wmodel') {
        throw "Unsafe Effect Model Cue asset ID: $AssetId"
    }
    foreach ($segment in $AssetId.Split('/')) {
        if ([string]::IsNullOrWhiteSpace($segment) -or
            $segment -eq '.' -or $segment -eq '..') {
            throw "Unsafe Effect Model Cue path segment: $AssetId"
        }
    }
    $candidate = [IO.Path]::GetFullPath((Join-Path $ResourceRoot $AssetId))
    $prefix = $ResourceRoot.TrimEnd('\') + '\'
    if (-not $candidate.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Effect Model Cue resource escaped Resources: $AssetId"
    }
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw "Missing Effect Model Cue resource: $AssetId"
    }
    return $candidate
}

function Copy-JsonValue([object]$Value) {
    return ($Value | ConvertTo-Json -Depth 100 -Compress) | ConvertFrom-Json
}

function Test-JsonNumericIdentity([object]$Expected, [object]$Actual) {
    $expectedDouble = [double]$Expected
    $actualDouble = [double]$Actual
    if ($expectedDouble -eq $actualDouble) { return $true }
    if ([double]::IsNaN($expectedDouble) -or
        [double]::IsNaN($actualDouble) -or
        [double]::IsInfinity($expectedDouble) -or
        [double]::IsInfinity($actualDouble) -or
        (($expectedDouble -lt 0) -ne ($actualDouble -lt 0))) {
        return $false
    }
    $expectedBits = [BitConverter]::ToInt64(
        [BitConverter]::GetBytes($expectedDouble), 0)
    $actualBits = [BitConverter]::ToInt64(
        [BitConverter]::GetBytes($actualDouble), 0)
    $distance = [Math]::Abs([decimal]$expectedBits - [decimal]$actualBits)
    # ConvertFrom-Json in Windows PowerShell 5.1 parses long JSON fractions as
    # Decimal before the validator casts them to Double. Direct JSON readers
    # (and the Python component compiler) round those literals straight to
    # Double, which can differ by exactly one ULP. Anything larger remains a
    # lossless compile failure.
    return $distance -le 1
}

function Assert-JsonEquivalent(
    [object]$Expected,
    [object]$Actual,
    [string]$Path = '$') {
    if ($null -eq $Expected -or $null -eq $Actual) {
        if ($null -eq $Expected -and $null -eq $Actual) { return }
        throw "JSON identity mismatch at ${Path}: null differs."
    }
    $expectedIsNumber = $Expected -is [byte] -or $Expected -is [int16] -or
        $Expected -is [int32] -or $Expected -is [int64] -or
        $Expected -is [single] -or $Expected -is [double] -or
        $Expected -is [decimal]
    $actualIsNumber = $Actual -is [byte] -or $Actual -is [int16] -or
        $Actual -is [int32] -or $Actual -is [int64] -or
        $Actual -is [single] -or $Actual -is [double] -or
        $Actual -is [decimal]
    if ($expectedIsNumber -or $actualIsNumber) {
        if (-not $expectedIsNumber -or -not $actualIsNumber -or
            -not (Test-JsonNumericIdentity $Expected $Actual)) {
            throw "JSON numeric identity mismatch at ${Path}: $Expected != $Actual"
        }
        return
    }
    if ($Expected -is [string] -or $Expected -is [bool] -or
        $Actual -is [string] -or $Actual -is [bool]) {
        if ($Expected.GetType() -ne $Actual.GetType() -or $Expected -cne $Actual) {
            throw "JSON scalar identity mismatch at ${Path}: $Expected != $Actual"
        }
        return
    }
    if ($Expected -is [array] -or $Actual -is [array]) {
        if ($Expected -isnot [array] -or $Actual -isnot [array] -or
            $Expected.Count -ne $Actual.Count) {
            throw "JSON array identity mismatch at ${Path}."
        }
        for ($index = 0; $index -lt $Expected.Count; ++$index) {
            Assert-JsonEquivalent $Expected[$index] $Actual[$index] "${Path}[$index]"
        }
        return
    }
    $expectedProperties = @($Expected.PSObject.Properties | Sort-Object Name)
    $actualProperties = @($Actual.PSObject.Properties | Sort-Object Name)
    if ($expectedProperties.Count -ne $actualProperties.Count) {
        throw "JSON object property count mismatch at ${Path}."
    }
    for ($index = 0; $index -lt $expectedProperties.Count; ++$index) {
        $expectedProperty = $expectedProperties[$index]
        $actualProperty = $actualProperties[$index]
        if ($expectedProperty.Name -cne $actualProperty.Name) {
            throw "JSON object property mismatch at ${Path}: $($expectedProperty.Name) != $($actualProperty.Name)"
        }
        Assert-JsonEquivalent $expectedProperty.Value $actualProperty.Value `
            "${Path}.$($expectedProperty.Name)"
    }
}

function Assert-IdentityTransform([object]$Transform, [string]$Label) {
    $position = Get-NumberVector $Transform 'position' 3 $Label
    $rotation = Get-NumberVector $Transform 'rotationDegrees' 3 $Label
    $scale = Get-NumberVector $Transform 'scale' 3 $Label
    if (@($position | Where-Object { $_ -ne 0 }).Count -ne 0 -or
        @($rotation | Where-Object { $_ -ne 0 }).Count -ne 0 -or
        @($scale | Where-Object { $_ -ne 1 }).Count -ne 0) {
        throw "$Label is not supported until component transform execution is exact."
    }
}

function Compile-EffectAssembly(
    [object]$Assembly,
    [Collections.Generic.Dictionary[string,object]]$ComponentsById) {
    if ((Get-RequiredProperty $Assembly 'schema' String) -cne
            'lostark.effect-assembly' -or
        [int](Get-RequiredProperty $Assembly 'version' Number) -ne 1) {
        throw 'Unsupported Effect assembly.'
    }
    $assemblyDisplayName = Get-RequiredProperty $Assembly 'displayName' String
    if ([string]::IsNullOrWhiteSpace($assemblyDisplayName) -or
        [Text.Encoding]::UTF8.GetByteCount($assemblyDisplayName) -gt 64) {
        throw 'Effect Assembly displayName must be 1-64 UTF-8 bytes.'
    }
    $rows = [Collections.Generic.List[object]]::new()
    $compiledIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    foreach ($cue in @(Get-RequiredProperty $Assembly 'componentCues' Array)) {
        $cueId = Get-RequiredProperty $cue 'cueId' String
        $componentId = Get-RequiredProperty $cue 'componentAssetId' String
        Assert-StableId $cueId 'Effect Component cue ID'
        Assert-StableId $componentId 'Effect Component asset ID'
        $offset = Get-NumberValue $cue 'startDelaySeconds' $cueId
        if ($offset -lt 0 -or
            (Get-RequiredProperty $cue 'anchor' String) -cne 'root') {
            throw "Component cue timing or anchor is invalid: $cueId"
        }
        [void](Get-RequiredProperty $cue 'visible' Boolean)
        Assert-IdentityTransform `
            (Get-RequiredProperty $cue 'localTransform' Object) $cueId
        $component = $null
        if (-not $ComponentsById.TryGetValue($componentId, [ref]$component)) {
            throw "Missing Effect Component: $componentId"
        }
        if ((Get-RequiredProperty $component 'schema' String) -cne
                'lostark.effect-component' -or
            [int](Get-RequiredProperty $component 'version' Number) -ne 1 -or
            (Get-RequiredProperty $component 'componentAssetId' String) -cne
                $componentId) {
            throw "Effect Component header mismatch: $componentId"
        }
        $componentDisplayName = Get-RequiredProperty $component 'displayName' String
        $document = Get-RequiredProperty $component 'document' Object
        if ((Get-RequiredProperty $document 'effectAssetId' String) -cne
                $componentId) {
            throw "Effect Component Document identity mismatch: $componentId"
        }
        $documentDisplayName = Get-RequiredProperty $document 'displayName' String
        if ([string]::IsNullOrWhiteSpace($componentDisplayName) -or
            [Text.Encoding]::UTF8.GetByteCount($componentDisplayName) -gt 64 -or
            [string]::IsNullOrWhiteSpace($documentDisplayName) -or
            [Text.Encoding]::UTF8.GetByteCount($documentDisplayName) -gt 64) {
            throw "Effect Component and its Document displayName must each be 1-64 UTF-8 bytes: $componentId"
        }
        $elementsById = [Collections.Generic.Dictionary[string,object]]::new(
            [StringComparer]::Ordinal)
        foreach ($element in @(Get-RequiredProperty $document 'elements' Array)) {
            $elementId = Get-RequiredProperty $element 'id' String
            if ($elementsById.ContainsKey($elementId)) {
                throw "Duplicate Component Element ID: $componentId/$elementId"
            }
            $elementsById.Add($elementId, $element)
        }
        $emitters = @(Get-RequiredProperty $component 'emitters' Array)
        if ($emitters.Count -ne $elementsById.Count) {
            throw "Component Emitter/Element count mismatch: $componentId"
        }
        foreach ($emitter in $emitters) {
            $emitterId = Get-RequiredProperty $emitter 'emitterId' String
            $elementId = Get-RequiredProperty $emitter 'elementId' String
            Assert-StableId $emitterId 'Effect Emitter ID'
            Assert-StableId $elementId 'Effect Component Element ID'
            $sourceIndex = Get-IntegerValue $emitter 'sourceElementIndex' $emitterId
            $element = $null
            if ($sourceIndex -lt 0 -or
                -not $elementsById.TryGetValue($elementId, [ref]$element)) {
                throw "Component Emitter has no source Element: $componentId/$emitterId"
            }
            $copy = Copy-JsonValue $element
            $copy.detail.timing.startDelaySeconds =
                [double]$copy.detail.timing.startDelaySeconds + $offset
            if (-not $compiledIds.Add([string]$copy.id)) {
                throw "Duplicate compiled Effect Element ID: $($copy.id)"
            }
            $rows.Add([pscustomobject]@{
                sourceElementIndex = $sourceIndex
                element = $copy
            })
        }
    }
    $orderedElements = @($rows | Sort-Object sourceElementIndex | ForEach-Object {
        $_.element
    })
    $sourceAuthoringVersion =
        [int](Get-RequiredProperty $Assembly 'sourceAuthoringVersion' Number)
    $compiledDocument = [ordered]@{
        schema = 'lostark.effect-authoring'
        version = $sourceAuthoringVersion
        effectAssetId = Get-RequiredProperty $Assembly 'effectAssetId' String
        displayName = Get-RequiredProperty $Assembly 'displayName' String
    }
    if ($sourceAuthoringVersion -ge 8) {
        $compiledDocument['particleSystem'] =
            Get-RequiredProperty $Assembly 'particleSystem' Object
    }
    if ($sourceAuthoringVersion -ge 7) {
        $compiledDocument['modelCues'] =
            @(Get-RequiredProperty $Assembly 'modelCues' Array)
    }
    $compiledDocument['elements'] = $orderedElements
    # Keep IEEE-754 values produced by local-delay + cue-offset intact for the
    # identity check. Windows PowerShell 5.1 ConvertTo-Json rounds some doubles
    # to 15 decimal digits, so a final JSON copy here can introduce a one-ULP
    # mismatch even when the compiled value is exactly the source value.
    return [pscustomobject]$compiledDocument
}

$assemblyById = [Collections.Generic.Dictionary[string,object]]::new(
    [StringComparer]::Ordinal)
foreach ($path in @(Get-ChildItem -LiteralPath $assemblyRoot -Recurse `
        -Filter '*.assembly.json' -File)) {
    $assembly = Read-JsonDocument $path.FullName
    $assemblyId = Get-RequiredProperty $assembly 'effectAssetId' String
    if ($assemblyById.ContainsKey($assemblyId)) {
        throw "Duplicate Effect Assembly ID: $assemblyId"
    }
    $assemblyById.Add($assemblyId, $assembly)
}
$componentById = [Collections.Generic.Dictionary[string,object]]::new(
    [StringComparer]::Ordinal)
foreach ($path in @(Get-ChildItem -LiteralPath $componentRoot -Recurse `
        -Filter '*.wfx.json' -File)) {
    $component = Read-JsonDocument $path.FullName
    $componentId = Get-RequiredProperty $component 'componentAssetId' String
    if ($componentById.ContainsKey($componentId)) {
        throw "Duplicate Effect Component ID: $componentId"
    }
    $componentById.Add($componentId, $component)
}

$catalog = Read-JsonDocument $catalogPath
try {
    $root = $catalog
    $version = Get-RequiredProperty $root 'formatVersion' Number
    $effects = Get-RequiredProperty $root 'effects' Array
    if ([int]$version -ne 1) {
        throw 'EffectCatalog.json must be formatVersion 1.'
    }

    $claimedIds = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::Ordinal)
    $runtimeEffects = [Collections.Generic.List[object]]::new()
    $runtimeComponentsById = [Collections.Generic.Dictionary[string,object]]::new(
        [StringComparer]::Ordinal)
    foreach ($entry in @($effects)) {
        $effectAssetId = Get-RequiredProperty $entry 'effectAssetId' String
        $authoringPath = Get-RequiredProperty $entry 'authoringPath' String
        Assert-StableId $effectAssetId 'EffectAssetId'
        if (-not $claimedIds.Add($effectAssetId)) {
            throw "Duplicate EffectAssetId: $effectAssetId"
        }
        if ([IO.Path]::IsPathRooted($authoringPath) -or
            $authoringPath.Contains('\') -or $authoringPath.Contains(':') -or
            -not $authoringPath.StartsWith('Effects/Authored/', [StringComparison]::Ordinal)) {
            throw "Unsafe authoringPath: $authoringPath"
        }
        foreach ($segment in $authoringPath.Split('/')) {
            if ([string]::IsNullOrWhiteSpace($segment) -or
                $segment -eq '.' -or $segment -eq '..') {
                throw "Unsafe authoringPath segment: $authoringPath"
            }
        }
        $authoringFile = [IO.Path]::GetFullPath((Join-Path $DataRoot $authoringPath))
        $authoringPrefix = $authoringRoot.TrimEnd('\') + '\'
        if (-not $authoringFile.StartsWith($authoringPrefix,
            [StringComparison]::OrdinalIgnoreCase)) {
            throw "Authoring path escaped Data/Effects/Authored: $authoringPath"
        }
        $expectedFile = "$effectAssetId.effect.json"
        if ([IO.Path]::GetFileName($authoringFile) -cne $expectedFile) {
            throw "Authoring filename must match EffectAssetId: $effectAssetId"
        }

        $documentHandle = Read-JsonDocument $authoringFile
        try {
            $document = $documentHandle
            $schema = Get-RequiredProperty $document 'schema' String
            $documentVersion = [int](Get-RequiredProperty $document 'version' Number)
            $documentId = Get-RequiredProperty $document 'effectAssetId' String
            [void](Get-RequiredProperty $document 'displayName' String)
            $elements = Get-RequiredProperty $document 'elements' Array
            if ($schema -cne 'lostark.effect-authoring' -or
                $documentVersion -notin @(5, 6, 7, 8, 9, 10, 11, 12) -or
                $documentId -cne $effectAssetId) {
                throw "Effect authoring header mismatch: $effectAssetId"
            }
            if ([Text.Encoding]::UTF8.GetByteCount($document.displayName) -lt 1 -or
                [Text.Encoding]::UTF8.GetByteCount($document.displayName) -gt 64 -or
                [string]::IsNullOrWhiteSpace($document.displayName) -or
                @($elements).Count -gt 2048) {
                throw "Effect authoring display name or Element budget is invalid: $effectAssetId"
            }
            $elementIds = [Collections.Generic.HashSet[string]]::new(
                [StringComparer]::Ordinal)
            $dependencies = [Collections.Generic.SortedDictionary[string,string]]::new(
                [StringComparer]::Ordinal)
            [int64]$totalParticles = 0
            [int64]$totalTrailPoints = 0
            [int64]$totalAfterImages = 0
            if ($documentVersion -ge 8) {
                $particleSystem = Get-RequiredProperty $document 'particleSystem' Object
                $uniformScale = Get-NumberValue $particleSystem `
                    'uniformScaleMultiplier' 'particleSystem'
                $yawOffset = Get-NumberValue $particleSystem `
                    'yawOffsetDegrees' 'particleSystem'
                $directionYaw = Get-NumberValue $particleSystem `
                    'directionYawDegrees' 'particleSystem'
                $initialSpeed = Get-NumberValue $particleSystem `
                    'initialSpeedMultiplier' 'particleSystem'
                if ($uniformScale -le 0 -or $uniformScale -gt 100 -or
                    [Math]::Abs($yawOffset) -gt 3600 -or
                    [Math]::Abs($directionYaw) -gt 3600 -or
                    $initialSpeed -lt 0 -or $initialSpeed -gt 100) {
                    throw "Effect Particle System modifier is invalid: $effectAssetId"
                }
            }
            if ($documentVersion -ge 7) {
                $modelCues = @(Get-RequiredProperty $document 'modelCues' Array)
                if ($modelCues.Count -gt 8) {
                    throw "Effect Model Cue budget is invalid: $effectAssetId"
                }
                $cueIds = [Collections.Generic.HashSet[string]]::new(
                    [StringComparer]::Ordinal)
                foreach ($cue in $modelCues) {
                    $cueId = Get-RequiredProperty $cue 'cueId' String
                    $modelAssetId = Get-RequiredProperty $cue 'modelAssetId' String
                    $clipName = Get-RequiredProperty $cue 'clipName' String
                    $startDelay = Get-NumberValue $cue 'startDelaySeconds' $cueId
                    $duration = Get-NumberValue $cue 'durationSeconds' $cueId
                    [void](Get-RequiredProperty $cue 'visible' Boolean)
                    $localTransform = Get-RequiredProperty $cue 'localTransform' Object
                    $assetPreTransform = Get-RequiredProperty $cue 'assetPreTransform' Object
                    Assert-StableId $cueId 'Effect Model Cue ID'
                    if (-not $cueIds.Add($cueId) -or
                        [string]::IsNullOrWhiteSpace($clipName) -or
                        [Text.Encoding]::UTF8.GetByteCount($clipName) -gt 128 -or
                        $startDelay -lt 0 -or $duration -le 0 -or $duration -gt 30) {
                        throw "Effect Model Cue identity or time is invalid: $cueId"
                    }
                    [void](Get-NumberVector $localTransform 'position' 3 $cueId)
                    [void](Get-NumberVector $localTransform 'rotationDegrees' 3 $cueId)
                    $localScale = Get-NumberVector $localTransform 'scale' 3 $cueId
                    $preScale = Get-NumberVector $assetPreTransform 'scale' 3 $cueId
                    [void](Get-NumberVector $assetPreTransform 'rotationDegrees' 3 $cueId)
                    if (@($localScale + $preScale | Where-Object { $_ -le 0 }).Count -ne 0) {
                        throw "Effect Model Cue scale must be positive: $cueId"
                    }
                    $modelFile = Resolve-SafeModelCueResource $modelAssetId
                    $dependencies[$modelAssetId] =
                        (Get-FileHash -LiteralPath $modelFile -Algorithm SHA256).Hash.ToLowerInvariant()
                }
            }
            foreach ($element in @($elements)) {
                $elementId = Get-RequiredProperty $element 'id' String
                $kind = Get-RequiredProperty $element 'kind' String
                $resources = Get-RequiredProperty $element 'resources' Array
                $material = Get-RequiredProperty $element 'material' Object
                $detail = Get-RequiredProperty $element 'detail' Object
                $sourceProfileEnabled = $false
                $shaderProfileId = ''
                $groupedHasAlpha = $false
                $groupedHasEmissive = $false
                $sourceTextureNames = [Collections.Generic.HashSet[string]]::new(
                    [StringComparer]::Ordinal)
                $resolvedSourceTextureNames = [Collections.Generic.HashSet[string]]::new(
                    [StringComparer]::Ordinal)
                $sourceTexturesByName =
                    [Collections.Generic.Dictionary[string,object]]::new(
                        [StringComparer]::Ordinal)
                Assert-StableId $elementId 'Element ID'
                if ($documentVersion -ge 6) {
                    $elementDisplayName = Get-RequiredProperty $element 'displayName' String
                    $groupId = Get-RequiredProperty $element 'groupId' String
                    [void](Get-RequiredProperty $element 'sourceNode' String)
                    [void](Get-RequiredProperty $element 'visible' Boolean)
                    if ([Text.Encoding]::UTF8.GetByteCount($elementDisplayName) -lt 1 -or
                        [Text.Encoding]::UTF8.GetByteCount($elementDisplayName) -gt 64 -or
                        [string]::IsNullOrWhiteSpace($elementDisplayName)) {
                        throw "Element displayName is invalid in ${effectAssetId}: $elementId"
                    }
                    if (-not [string]::IsNullOrEmpty($groupId)) {
                        Assert-StableId $groupId 'Element groupId'
                    }
                    $materialTemplateId = Get-RequiredProperty $material 'templateId' String
                    $sourceMaterialPath = if ($documentVersion -ge 10) {
                        Get-RequiredProperty $material 'sourceMaterialPath' String
                    } else { '' }
                    if ($materialTemplateId -notin @('effect.standard','effect.source_material')) {
                        throw "Unknown Material Template in ${effectAssetId}: $materialTemplateId"
                    }
                    if ($materialTemplateId -eq 'effect.source_material' -and
                        [string]::IsNullOrWhiteSpace($sourceMaterialPath)) {
                        throw "Source Material Template requires sourceMaterialPath in ${effectAssetId}: $elementId"
                    }
					if ($documentVersion -ge 11) {
						$sourceProfile = Get-RequiredProperty $material 'sourceProfile' Object
						$sourceProfileEnabled = [bool](Get-RequiredProperty `
							$sourceProfile 'enabled' Boolean)
						if ($materialTemplateId -eq 'effect.source_material' -and
							-not $sourceProfileEnabled) {
							throw "Source Material Template requires an enabled profile in ${effectAssetId}: $elementId"
						}
						if ($sourceProfileEnabled) {
							$profileId = Get-RequiredProperty $sourceProfile 'profileId' String
							$shaderProfileId = Get-RequiredProperty `
								$sourceProfile 'runtimeShaderProfileId' String
							$parentMaterialPath = Get-RequiredProperty `
								$sourceProfile 'parentMaterialPath' String
							$semanticStatus = Get-RequiredProperty `
								$sourceProfile 'semanticStatus' String
							$subUVMode = Get-RequiredProperty $sourceProfile 'subUVMode' String
							Assert-StableId $profileId 'Source Material profile ID'
							Assert-StableId $shaderProfileId 'Source Material shader profile ID'
							Assert-StableId $subUVMode 'Source Material SubUV mode'
							if ($shaderProfileId -notin $supportedSourceRuntimeShaderProfiles) {
								throw "Unsupported Source Material shader profile in ${effectAssetId}: $elementId ($shaderProfileId)"
							}
							if ($subUVMode -notin $supportedSourceSubUVModes) {
								throw "Unsupported Source Material SubUV mode in ${effectAssetId}: $elementId ($subUVMode)"
							}
							if ([string]::IsNullOrWhiteSpace($parentMaterialPath) -or
								$semanticStatus -notin @('source_exact','runtime_exact','reconstructed_profile')) {
								throw "Source Material profile metadata is invalid in ${effectAssetId}: $elementId"
							}
							$dynamicSemantics = @(Get-RequiredProperty `
								$sourceProfile 'dynamicParameterSemantics' Array)
							if ($dynamicSemantics.Count -ne 4) {
								throw "Source Material Dynamic Parameter contract must have four channels: $elementId"
							}
							foreach ($semantic in $dynamicSemantics) {
								Assert-StableId ([string]$semantic) `
									'Source Material Dynamic Parameter semantic'
								if ([string]$semantic -notin $supportedSourceDynamicParameterSemantics) {
									throw "Unsupported Source Material Dynamic Parameter semantic in ${effectAssetId}: $elementId ($semantic)"
								}
							}
							foreach ($scalar in @(Get-RequiredProperty $sourceProfile 'scalars' Array)) {
								$scalarName = Get-RequiredProperty $scalar 'name' String
								[void](Get-RequiredProperty $scalar 'value' Number)
								$scalarGroup = if ($null -ne $scalar.PSObject.Properties['group']) {
									[string]$scalar.group
								} else { '' }
								$scalarIdentity = ($scalarName + ' ' + $scalarGroup).ToLowerInvariant()
								if ($scalarIdentity.Contains('alpha') -or
									$scalarIdentity.Contains('mask') -or
									$scalarIdentity.Contains('opacity') -or
									$scalarIdentity.Contains('density')) {
									$groupedHasAlpha = $true
								}
								if ($scalarIdentity.Contains('emiss')) {
									$groupedHasEmissive = $true
								}
							}
							# Named source textures were added as a backward-compatible v12
							# extension.  Older authored v12 documents legitimately omit the
							# property; a profile that depends on named inputs validates its own
							# required set below.
							$sourceTextures = @()
							$sourceTexturesProperty = $sourceProfile.PSObject.Properties['textures']
							if ($null -ne $sourceTexturesProperty) {
								$sourceTexturesValue = $sourceTexturesProperty.Value
								if ($null -ne $sourceTexturesValue -and
									-not ($sourceTexturesValue -is [array])) {
									throw "Source Material textures must be an array in ${effectAssetId}: $elementId"
								}
								$sourceTextures = @($sourceTexturesValue)
							}
							if ($sourceTextures.Count -gt 32) {
								throw "Source Material texture count exceeds 32: $elementId"
							}
							foreach ($texture in $sourceTextures) {
								$textureName = Get-RequiredProperty $texture 'name' String
								$textureSourcePath = Get-RequiredProperty `
									$texture 'sourceObjectPath' String
								$textureAssetId = Get-RequiredProperty $texture 'assetId' String
								if ([string]::IsNullOrWhiteSpace($textureName) -or
									-not $sourceTextureNames.Add($textureName)) {
									throw "Invalid or duplicate Source Material texture in ${effectAssetId}: $elementId/$textureName"
								}
								$sourceTexturesByName[$textureName] = $texture
								if (-not [string]::IsNullOrWhiteSpace($textureAssetId) -and
									[string]::IsNullOrWhiteSpace($textureSourcePath)) {
									throw "Resolved Source Material texture has no source object path in ${effectAssetId}: $elementId/$textureName"
								}
								if (-not [string]::IsNullOrWhiteSpace($textureAssetId)) {
									[void]$resolvedSourceTextureNames.Add($textureName)
									$sourceTextureFile = Resolve-SafeResource $textureAssetId
									if ([IO.Path]::GetExtension($sourceTextureFile).ToLowerInvariant() -ne '.dds') {
										throw "Source Material texture is not DDS: $textureAssetId"
									}
									$dependencies[$textureAssetId] =
										(Get-FileHash -LiteralPath $sourceTextureFile -Algorithm SHA256).Hash.ToLowerInvariant()
								}
							}
							foreach ($vector in @(Get-RequiredProperty $sourceProfile 'vectors' Array)) {
								$vectorName = Get-RequiredProperty $vector 'name' String
								[void](Get-NumberVector $vector 'value' 4 $elementId)
								$vectorGroup = if ($null -ne $vector.PSObject.Properties['group']) {
									[string]$vector.group
								} else { '' }
								$vectorIdentity = ($vectorName + ' ' + $vectorGroup).ToLowerInvariant()
								if ($vectorIdentity.Contains('emiss')) {
									$groupedHasEmissive = $true
								}
							}
							foreach ($switch in @(Get-RequiredProperty $sourceProfile 'staticSwitches' Array)) {
								[void](Get-RequiredProperty $switch 'name' String)
								[void](Get-RequiredProperty $switch 'value' Boolean)
							}
							if ($shaderProfileId -eq 'effect.ue3.linearflow-02.v1') {
								foreach ($requiredName in @(
									'diff_tex','diff_noise_tex','a_mask_tex','a_noise_01_tex',
									'b_mask_tex','b_noise_01_tex','dissolve_tex')) {
									if (-not $resolvedSourceTextureNames.Contains($requiredName)) {
										throw "LinearFlow profile is missing Source Material texture '$requiredName' in ${effectAssetId}: $elementId"
									}
								}
							}
							if ($shaderProfileId -eq 'effect.ue3.local-crack.v1' -and
								$sourceTextures.Count -gt 0) {
								foreach ($requiredName in @(
									'normal_tex','refle_tex','dissolve_tex')) {
									if (-not $resolvedSourceTextureNames.Contains($requiredName)) {
										throw "LocalCrack profile is missing Source Material texture '$requiredName' in ${effectAssetId}: $elementId"
									}
									$requiredTexture = $sourceTexturesByName[$requiredName]
									$addressU = Get-RequiredProperty `
										$requiredTexture 'addressU' String
									$addressV = Get-RequiredProperty `
										$requiredTexture 'addressV' String
									$colorSpace = Get-RequiredProperty `
										$requiredTexture 'colorSpace' String
									$samplingEvidence = Get-RequiredProperty `
										$requiredTexture 'samplingEvidence' String
									if ($addressU -notin @('wrap','clamp') -or
										$addressV -notin @('wrap','clamp') -or
										$colorSpace -notin @('linear','srgb') -or
										[string]::IsNullOrWhiteSpace($samplingEvidence) -or
										$samplingEvidence -eq 'legacy_default') {
										throw "LocalCrack texture sampling contract is incomplete in ${effectAssetId}: $elementId/$requiredName"
									}
								}
							}
						}
					}
                }
                if (-not $elementIds.Add($elementId)) {
                    throw "Duplicate Element ID in ${effectAssetId}: $elementId"
                }
                if ($kind -notin @('mesh','sprite','particle','decal','trail','light','screenPost')) {
                    throw "Unknown Effect kind in ${effectAssetId}: $kind"
                }
                $renderProfile = Get-RequiredProperty $material 'renderProfile' String
                if ($renderProfile -notin @('opaque_back_depth_write',
                    'alpha_two_sided_depth_read','additive_two_sided_depth_read',
                    'alpha_one_sided_depth_read','additive_one_sided_depth_read')) {
                    throw "Unknown render profile in ${effectAssetId}: $renderProfile"
                }
                $claimedSlots = [Collections.Generic.HashSet[string]]::new(
                    [StringComparer]::Ordinal)
                $claimedAssets = [Collections.Generic.Dictionary[string,string]]::new(
                    [StringComparer]::Ordinal)
                foreach ($binding in @($resources)) {
                    $slotProperty = if ($documentVersion -ge 6) { 'slotId' } else { 'slot' }
                    $slot = Get-RequiredProperty $binding $slotProperty String
                    $assetId = Get-RequiredProperty $binding 'assetId' String
                    if ($slot -notin @('meshModel','base','noise','mask','emissive','dissolve') -or
                        -not $claimedSlots.Add($slot)) {
                        throw "Invalid or duplicate resource slot in ${effectAssetId}: $slot"
                    }
                    $claimedAssets[$slot] = $assetId
                    if ([Text.Encoding]::UTF8.GetByteCount($assetId) -gt 512) {
                        throw "Effect resource asset ID is too long: $assetId"
                    }
					if ($slot -eq 'meshModel' -and $kind -notin @('mesh','particle')) {
						throw "meshModel is only valid on a mesh or particle Element."
                    }
                    $resourceFile = Resolve-SafeResource $assetId
                    $extension = [IO.Path]::GetExtension($resourceFile).ToLowerInvariant()
                    if (($slot -eq 'meshModel' -and $extension -ne '.wmodel') -or
                        ($slot -ne 'meshModel' -and $extension -ne '.dds')) {
                        throw "Resource extension does not match slot: $assetId"
                    }
                    $dependencies[$assetId] =
                        (Get-FileHash -LiteralPath $resourceFile -Algorithm SHA256).Hash.ToLowerInvariant()
                }
				$isMeshParticle = $kind -eq 'particle' -and $claimedSlots.Contains('meshModel')
				$profileRequiredSlots = @()
				if ($sourceProfileEnabled) {
					$profileRequiredSlots = switch ($shaderProfileId) {
						'effect.ue3.reconstructed-standard.v1' { @('base') }
						'effect.ue3.ring.v1' { @('base','noise') }
						'effect.ue3.aura.v1' { @('base','noise') }
						'effect.ue3.one-layer-distortion.v1' { @('noise') }
						'effect.ue3.shine.v1' { @('base','mask') }
						'effect.ue3.blackline-aura.v1' { @('mask','dissolve') }
						'effect.ue3.linearflow-02.v1' { @() }
						'effect.ue3.missiletrail-01.v1' { @('base','mask','noise','dissolve') }
						'effect.ue3.local-crack.v1' {
							if ($sourceTextures.Count -eq 0) { @('dissolve') } else { @() }
						}
						'effect.ue3.procedural-center-glow.v1' { @() }
						default { @() }
					}
				} elseif ($kind -notin @('mesh','light','screenPost') -and
					-not $isMeshParticle -and
					$materialTemplateId -ne 'effect.source_material') {
					$profileRequiredSlots = @('base')
				}
				if ($kind -eq 'mesh' -or $isMeshParticle) {
					$profileRequiredSlots = @('meshModel') + $profileRequiredSlots
				}
				foreach ($requiredSlot in @($profileRequiredSlots | Select-Object -Unique)) {
					if (-not $claimedSlots.Contains($requiredSlot)) {
						throw "$kind Element requires '$requiredSlot' for $shaderProfileId in ${effectAssetId}: $elementId"
					}
				}
				$baseAssetId = if ($claimedAssets.ContainsKey('base')) {
					$claimedAssets['base']
				} else { '' }
				$hasSafeGroupedBase = -not [string]::IsNullOrEmpty($baseAssetId) -and
					$baseAssetId -notmatch '(?i)(blankwhite|normal|bump)'
				$hasGroupedMask = $claimedSlots.Contains('mask')
				$hasGroupedEmissive = $claimedSlots.Contains('emissive')
				$hasGroupedDissolve = $claimedSlots.Contains('dissolve')
				$hasGroupedCarrier = $hasSafeGroupedBase -or
					$claimedSlots.Contains('mask') -or
					$claimedSlots.Contains('emissive')
				if ($sourceProfileEnabled -and
					$shaderProfileId -eq 'effect.ue3.grouped-translucent.v1' -and
					-not $hasGroupedCarrier) {
					throw "Grouped translucent profile requires Base/Mask/Emissive carrier in ${effectAssetId}: $elementId"
				}
				if ($sourceProfileEnabled -and
					$shaderProfileId -eq 'effect.ue3.grouped-translucent.v1' -and
					$groupedHasAlpha -and
					-not ($hasSafeGroupedBase -or $hasGroupedMask -or $hasGroupedDissolve)) {
					throw "Grouped alpha profile has no runtime alpha carrier in ${effectAssetId}: $elementId"
				}
				if ($sourceProfileEnabled -and
					$shaderProfileId -eq 'effect.ue3.grouped-translucent.v1' -and
					$groupedHasEmissive -and
					-not ($hasSafeGroupedBase -or $hasGroupedEmissive)) {
					throw "Grouped emissive profile has no runtime emission carrier in ${effectAssetId}: $elementId"
				}
                Assert-EffectDetail $detail $kind "${effectAssetId}/$elementId"
				if (($kind -eq 'mesh' -or $isMeshParticle) -and
                    -not [bool]$detail.mesh.useModelMaterial -and
				    -not $claimedSlots.Contains('base') -and
					-not ($sourceProfileEnabled -and
						$shaderProfileId -in @(
							'effect.ue3.fallback-blocked.v1',
								'effect.ue3.grouped-translucent.v1',
								'effect.ue3.blackline-aura.v1',
								'effect.ue3.linearflow-02.v1',
								'effect.ue3.missiletrail-01.v1',
								'effect.ue3.local-crack.v1'))) {
                    throw "Mesh useModelMaterial=false requires 'base' in ${effectAssetId}: $elementId"
                }
                if ($kind -eq 'particle') {
                    $totalParticles += [int64]$detail.particle.maxParticles
                }
                if ($kind -eq 'trail') {
                    $totalTrailPoints += [int64]$detail.trail.maxPoints
                }
                if ([double]$detail.timing.afterImageSeconds -gt 0 -and
                    [int64]$detail.afterImage.maxCopies -gt 0) {
                    $totalAfterImages += [int64]$detail.afterImage.maxCopies
                }
            }
            if ($totalParticles -gt 8192 -or $totalTrailPoints -gt 2048 -or
                $totalAfterImages -gt 256) {
                throw "Effect Document exceeds particle/trail/after-image budget: $effectAssetId"
            }
            $dependencyRows = @($dependencies.GetEnumerator() | ForEach-Object {
                [ordered]@{ assetId = $_.Key; sha256 = $_.Value }
            })
            $assembly = $null
            if (-not $assemblyById.TryGetValue($effectAssetId, [ref]$assembly)) {
                throw "Missing Effect Assembly: $effectAssetId"
            }
            if ([int](Get-RequiredProperty $assembly `
                    'sourceAuthoringVersion' Number) -ne $documentVersion) {
                throw "Effect Assembly authoring version mismatch: $effectAssetId"
            }
            $authoringSha = (Get-FileHash -LiteralPath $authoringFile `
                -Algorithm SHA256).Hash.ToLowerInvariant()
            if ((Get-RequiredProperty $assembly 'sourceDocumentFileSha256' String) -cne
                $authoringSha) {
                throw "Effect Assembly source file hash mismatch: $effectAssetId"
            }
            if ($null -ne $assembly.PSObject.Properties['sourceActionCues']) {
                foreach ($sourceCue in @(Get-RequiredProperty $assembly `
                        'sourceActionCues' Array)) {
                    $sourceCueId = Get-RequiredProperty $sourceCue 'cueId' String
                    if ([string]::IsNullOrWhiteSpace($sourceCueId) -or
                        [Text.Encoding]::UTF8.GetByteCount($sourceCueId) -gt 256) {
                        throw "Source Action cue ID is invalid: $effectAssetId"
                    }
                    $localTime = Get-NumberValue $sourceCue `
                        'localTimeSeconds' $sourceCueId
                    $globalTime = Get-NumberValue $sourceCue `
                        'globalTimeSeconds' $sourceCueId
                    $duration = Get-NumberValue $sourceCue `
                        'durationSeconds' $sourceCueId
                    $executionEnabled = [bool](Get-RequiredProperty `
                        $sourceCue 'executionEnabled' Boolean)
                    $timeIsExecutable = $localTime -ge 0 -and $localTime -le 60 -and
                        $globalTime -ge 0 -and $globalTime -le 60 -and
                        $duration -ge 0 -and $duration -le 60
                    if ($executionEnabled -and -not $timeIsExecutable) {
                        throw "Executable Source Action cue timing is invalid: ${effectAssetId}/$sourceCueId"
                    }
                    if (-not $timeIsExecutable) {
                        $executionStatus = Get-RequiredProperty $sourceCue `
                            'sourceExecutionStatus' String
                        $disabledReason = Get-RequiredProperty $sourceCue `
                            'executionDisabledReason' String
                        if ($executionStatus -cne 'INVALID_SOURCE_TIME_FAIL_CLOSED' -or
                            $disabledReason -cne 'SOURCE_ACTION_CUE_TIME_OUTSIDE_FINITE_0_TO_60_SECONDS') {
                            throw "Invalid Source Action cue timing is not explicitly fail-closed: ${effectAssetId}/$sourceCueId"
                        }
                    }
                }
            }
            foreach ($cue in @(Get-RequiredProperty $assembly `
                    'componentCues' Array)) {
                $componentId = Get-RequiredProperty $cue 'componentAssetId' String
                $component = $null
                if (-not $componentById.TryGetValue($componentId, [ref]$component)) {
                    throw "Missing Effect Component: $effectAssetId/$componentId"
                }
                if (-not $runtimeComponentsById.ContainsKey($componentId)) {
                    $runtimeComponentsById.Add($componentId, $component)
                }
            }
            $compiledDocument = Compile-EffectAssembly `
                $assembly $runtimeComponentsById
            Assert-JsonEquivalent $document $compiledDocument `
                "Effect Assembly compiled document ${effectAssetId}"
            $runtimeEffects.Add([ordered]@{
                effectAssetId = $effectAssetId
                authoringFormatVersion = $documentVersion
                contentSha256 = $authoringSha
                dependencies = $dependencyRows
                assembly = $assembly
            })
        }
        finally {
            $documentHandle = $null
        }
    }

    $runtimeComponents = @($runtimeComponentsById.GetEnumerator() |
        Sort-Object Key | ForEach-Object { $_.Value })
    $runtime = [ordered]@{
        formatVersion = 2
        components = $runtimeComponents
        effects = @($runtimeEffects | Sort-Object effectAssetId)
    }
    $json = $runtime | ConvertTo-Json -Depth 100 -Compress
    if ($Mode -eq 'Validate') {
        Write-Host "PASS: validated $($runtimeEffects.Count) admitted Effects."
        return
    }

    $directory = Split-Path -Parent $OutputPath
    [IO.Directory]::CreateDirectory($directory) | Out-Null
    $temporary = "$OutputPath.tmp"
    $backup = "$OutputPath.bak"
    [IO.File]::WriteAllText($temporary, $json + [Environment]::NewLine, $utf8NoBom)
    $roundTrip = Read-JsonDocument $temporary
    try {
        $roundEffects = @(Get-RequiredProperty $roundTrip 'effects' Array)
        $roundComponents = @(Get-RequiredProperty $roundTrip 'components' Array)
        if ([int](Get-RequiredProperty $roundTrip 'formatVersion' Number) -ne 2 -or
            $roundEffects.Count -ne $runtimeEffects.Count -or
            $roundComponents.Count -ne $runtimeComponents.Count) {
            throw 'Generated runtime catalog failed round-trip validation.'
        }
        $expectedById = @{}
        foreach ($expected in $runtimeEffects) {
            $expectedById[[string]$expected.effectAssetId] = $expected
        }
        foreach ($entry in $roundEffects) {
            $id = Get-RequiredProperty $entry 'effectAssetId' String
            $expected = $expectedById[$id]
            if ($null -eq $expected -or
                [int](Get-RequiredProperty $entry 'authoringFormatVersion' Number) -ne
                    [int]$expected.authoringFormatVersion -or
                (Get-RequiredProperty $entry 'contentSha256' String) -cne
                    [string]$expected.contentSha256 -or
                @(Get-RequiredProperty $entry 'dependencies' Array).Count -ne
                    @($expected.dependencies).Count -or
                (Get-RequiredProperty $entry 'assembly' Object).effectAssetId -cne
                    $id) {
                throw "Generated runtime catalog entry failed round-trip validation: $id"
            }
        }
    }
    finally {
        $roundTrip = $null
    }
    if (Test-Path -LiteralPath $backup) {
        Remove-Item -LiteralPath $backup -Force
    }
    $hadDestination = Test-Path -LiteralPath $OutputPath -PathType Leaf
    if ($hadDestination) {
        Move-Item -LiteralPath $OutputPath -Destination $backup
    }
    try {
        Move-Item -LiteralPath $temporary -Destination $OutputPath
    }
    catch {
        if ($hadDestination -and (Test-Path -LiteralPath $backup)) {
            Move-Item -LiteralPath $backup -Destination $OutputPath
        }
        throw
    }
    if (Test-Path -LiteralPath $backup) {
        Remove-Item -LiteralPath $backup -Force
    }
    Write-Host "PASS: published $($runtimeEffects.Count) Effects and $($runtimeComponents.Count) Components to $OutputPath"
}
finally {
    $catalog = $null
}
