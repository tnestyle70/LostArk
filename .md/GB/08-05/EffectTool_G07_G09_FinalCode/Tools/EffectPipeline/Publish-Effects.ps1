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
$utf8NoBom = [Text.UTF8Encoding]::new($false)

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
        'Array' { $value -is [array] }
        'Object' { $null -ne $value -and
            $value -isnot [string] -and $value -isnot [array] -and
            $value.PSObject.Properties.Count -gt 0 }
    }
    if (-not $valid) {
        throw "Required property '$Name' has the wrong type."
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
                $documentVersion -ne 5 -or $documentId -cne $effectAssetId) {
                throw "Effect authoring header mismatch: $effectAssetId"
            }
            if ([Text.Encoding]::UTF8.GetByteCount($document.displayName) -lt 1 -or
                [Text.Encoding]::UTF8.GetByteCount($document.displayName) -gt 64 -or
                [string]::IsNullOrWhiteSpace($document.displayName) -or
                @($elements).Count -gt 256) {
                throw "Effect authoring display name or Element budget is invalid: $effectAssetId"
            }
            $elementIds = [Collections.Generic.HashSet[string]]::new(
                [StringComparer]::Ordinal)
            $dependencies = [Collections.Generic.SortedDictionary[string,string]]::new(
                [StringComparer]::Ordinal)
            [int64]$totalParticles = 0
            [int64]$totalTrailPoints = 0
            [int64]$totalAfterImages = 0
            foreach ($element in @($elements)) {
                $elementId = Get-RequiredProperty $element 'id' String
                $kind = Get-RequiredProperty $element 'kind' String
                $resources = Get-RequiredProperty $element 'resources' Array
                $material = Get-RequiredProperty $element 'material' Object
                $detail = Get-RequiredProperty $element 'detail' Object
                Assert-StableId $elementId 'Element ID'
                if (-not $elementIds.Add($elementId)) {
                    throw "Duplicate Element ID in ${effectAssetId}: $elementId"
                }
                if ($kind -notin @('mesh','sprite','particle','decal','trail')) {
                    throw "Unknown Effect kind in ${effectAssetId}: $kind"
                }
                $renderProfile = Get-RequiredProperty $material 'renderProfile' String
                if ($renderProfile -notin @('opaque_back_depth_write',
                    'alpha_two_sided_depth_read','additive_two_sided_depth_read')) {
                    throw "Unknown render profile in ${effectAssetId}: $renderProfile"
                }
                $claimedSlots = [Collections.Generic.HashSet[string]]::new(
                    [StringComparer]::Ordinal)
                foreach ($binding in @($resources)) {
                    $slot = Get-RequiredProperty $binding 'slot' String
                    $assetId = Get-RequiredProperty $binding 'assetId' String
                    if ($slot -notin @('meshModel','base','noise','mask','emissive','dissolve') -or
                        -not $claimedSlots.Add($slot)) {
                        throw "Invalid or duplicate resource slot in ${effectAssetId}: $slot"
                    }
                    if ([Text.Encoding]::UTF8.GetByteCount($assetId) -gt 512) {
                        throw "Effect resource asset ID is too long: $assetId"
                    }
                    if ($slot -eq 'meshModel' -and $kind -ne 'mesh') {
                        throw "meshModel is only valid on a mesh Element."
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
                $requiredSlot = if ($kind -eq 'mesh') { 'meshModel' } else { 'base' }
                if (-not $claimedSlots.Contains($requiredSlot)) {
                    throw "$kind Element requires '$requiredSlot' in ${effectAssetId}: $elementId"
                }
                Assert-EffectDetail $detail $kind "${effectAssetId}/$elementId"
                if ($kind -eq 'mesh' -and
                    -not [bool]$detail.mesh.useModelMaterial -and
                    -not $claimedSlots.Contains('base')) {
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
            $runtimeEffects.Add([ordered]@{
                effectAssetId = $effectAssetId
                authoringFormatVersion = 5
                contentSha256 = (Get-FileHash -LiteralPath $authoringFile -Algorithm SHA256).Hash.ToLowerInvariant()
                dependencies = $dependencyRows
                document = $document
            })
        }
        finally {
            $documentHandle = $null
        }
    }

    $runtime = [ordered]@{
        formatVersion = 1
        effects = @($runtimeEffects | Sort-Object effectAssetId)
    }
    $json = $runtime | ConvertTo-Json -Depth 100
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
        if ([int](Get-RequiredProperty $roundTrip 'formatVersion' Number) -ne 1 -or
            $roundEffects.Count -ne $runtimeEffects.Count) {
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
                [int](Get-RequiredProperty $entry 'authoringFormatVersion' Number) -ne 5 -or
                (Get-RequiredProperty $entry 'contentSha256' String) -cne
                    [string]$expected.contentSha256 -or
                @(Get-RequiredProperty $entry 'dependencies' Array).Count -ne
                    @($expected.dependencies).Count) {
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
    Write-Host "PASS: published $($runtimeEffects.Count) Effects to $OutputPath"
}
finally {
    $catalog = $null
}
