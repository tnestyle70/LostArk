[CmdletBinding()]
param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

function Get-TextSha256([string]$Text) {
    $bytes = [Text.Encoding]::UTF8.GetBytes($Text)
    $hasher = [Security.Cryptography.SHA256]::Create()
    try {
        return (($hasher.ComputeHash($bytes) |
            ForEach-Object { $_.ToString('x2') }) -join '')
    }
    finally {
        $hasher.Dispose()
    }
}

function Get-NormalizedTextSha256([string]$Path) {
    return Get-TextSha256 ([IO.File]::ReadAllText($Path).Replace("`r`n", "`n"))
}

function Assert-Near([double]$Actual, [double]$Expected,
    [double]$Tolerance, [string]$Label) {
    if ([double]::IsNaN($Actual) -or [double]::IsInfinity($Actual) -or
        [Math]::Abs($Actual - $Expected) -gt $Tolerance) {
        throw "$Label changed: actual=$Actual expected=$Expected tolerance=$Tolerance"
    }
}

function Assert-ExactOrderedProperties([object]$Object,
    [string[]]$ExpectedNames, [string]$Label) {
    $actualNames = @($Object.PSObject.Properties.Name)
    if ($actualNames.Count -ne $ExpectedNames.Count) {
        throw "$Label property count changed: actual=$($actualNames.Count) expected=$($ExpectedNames.Count)"
    }
    for ($index = 0; $index -lt $ExpectedNames.Count; ++$index) {
        if ([string]$actualNames[$index] -cne [string]$ExpectedNames[$index]) {
            throw "$Label property order changed at $index`: actual=$($actualNames[$index]) expected=$($ExpectedNames[$index])"
        }
    }
}

function Assert-ExactBooleanProperty([object]$Object, [string]$Name,
    [bool]$Expected, [string]$Label) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property -or -not ($property.Value -is [bool]) -or
        [bool]$property.Value -ne $Expected) {
        throw "$Label boolean changed or lost its native JSON type: $Name"
    }
}

function Assert-ExactInt32Property([object]$Object, [string]$Name,
    [int]$Expected, [string]$Label) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property -or -not ($property.Value -is [int]) -or
        [int]$property.Value -ne $Expected) {
        throw "$Label integer changed or lost its native JSON type: $Name"
    }
}

function Assert-ExactStringProperty([object]$Object, [string]$Name,
    [string]$Expected, [string]$Label) {
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property -or -not ($property.Value -is [string]) -or
        [string]$property.Value -cne $Expected) {
        throw "$Label string changed or lost its native JSON type: $Name"
    }
}

function Get-RequiredTextSlice([string]$Text, [string]$StartToken,
    [string]$EndToken, [int]$SearchFrom, [string]$Label) {
    $start = $Text.IndexOf($StartToken, $SearchFrom,
        [StringComparison]::Ordinal)
    if ($start -lt 0) {
        throw "$Label start token is missing: $StartToken"
    }
    $end = $Text.IndexOf($EndToken, $start + $StartToken.Length,
        [StringComparison]::Ordinal)
    if ($end -lt 0 -or $end -le $start) {
        throw "$Label end token is missing: $EndToken"
    }
    return [pscustomobject]@{
        Start = $start
        End = $end
        Text = $Text.Substring($start, $end - $start)
    }
}

function Assert-TextTokens([string]$Text, [string[]]$Tokens,
    [string]$Label) {
    foreach ($token in $Tokens) {
        if (-not $Text.Contains($token)) {
            throw "$Label token missing: $token"
        }
    }
}

function Get-UniqueRow([object[]]$Rows, [string]$PropertyName,
    [string]$ExpectedValue, [string]$Label) {
    $matches = @($Rows | Where-Object {
        [string]$_.$PropertyName -ceq $ExpectedValue
    })
    if ($matches.Count -ne 1) {
        throw "$Label is not unique: $PropertyName=$ExpectedValue count=$($matches.Count)"
    }
    return $matches[0]
}

function Get-RecipeProjection([object]$Program, [string[]]$OccurrenceIds) {
    $lines = [Collections.Generic.List[string]]::new()
    $occurrences = [Collections.Generic.List[object]]::new()
    foreach ($occurrenceId in $OccurrenceIds) {
        $occurrence = Get-UniqueRow @($Program.materialOccurrences) `
            'occurrenceId' $occurrenceId 'Runtime Material v2 occurrence'
        $occurrences.Add($occurrence)
        $lines.Add(('occurrence:{0}:{1}:{2}:{3}' -f
            $occurrence.occurrenceId, $occurrence.rowSha256,
            $occurrence.recipeId, $occurrence.familyId))
    }

    $recipeId = [string]$occurrences[0].recipeId
    $familyId = [string]$occurrences[0].familyId
    foreach ($occurrence in $occurrences) {
        if ([string]$occurrence.recipeId -cne $recipeId -or
            [string]$occurrence.familyId -cne $familyId) {
            throw 'Runtime Material v2 occurrences no longer share one recipe/family.'
        }
    }
    $recipe = Get-UniqueRow @($Program.materialRecipes) 'recipeId' $recipeId `
        'Runtime Material v2 recipe'
    $family = Get-UniqueRow @($Program.materialFamilies) 'familyId' $familyId `
        'Runtime Material v2 family'
    $inputIds = @($recipe.inputIds)
    $staticIds = @($recipe.staticBindingIds)
    $renderIds = @($recipe.renderBindingIds)
    $lines.Add(('recipe:{0}:{1}:{2}:{3}:{4}' -f
        $recipe.recipeId, $recipe.rowSha256, $inputIds.Count,
        $staticIds.Count, $renderIds.Count))
    $lines.Add(('family:{0}:{1}:{2}:{3}:{4}' -f
        $family.familyId, $family.rowSha256, $family.evaluatorId,
        $family.evaluatorSha256, $family.featureMask))

    foreach ($fieldId in $inputIds) {
        $row = Get-UniqueRow @($Program.materialInputs) 'fieldId' `
            ([string]$fieldId) 'Runtime Material v2 input'
        $lines.Add(('input:{0}:{1}' -f $fieldId, $row.rowSha256))
    }
    foreach ($fieldId in $staticIds) {
        $row = Get-UniqueRow @($Program.materialStaticBindings) 'fieldId' `
            ([string]$fieldId) 'Runtime Material v2 static input'
        $selected = ([string]$row.selectedValue).ToLowerInvariant()
        $lines.Add(('static:{0}:{1}:{2}' -f
            $fieldId, $row.rowSha256, $selected))
    }
    foreach ($renderBindingId in $renderIds) {
        $row = Get-UniqueRow @($Program.materialRenderBindings) `
            'renderBindingId' ([string]$renderBindingId) `
            'Runtime Material v2 render input'
        $lines.Add(('render:{0}:{1}' -f
            $renderBindingId, $row.rowSha256))
    }

    $textureBindings = @($Program.materialTextureBindings |
        Where-Object { [string]$_.recipeId -ceq $recipeId } |
        Sort-Object { [int]$_.order })
    foreach ($row in $textureBindings) {
        $runtimeAssetId = if ($null -eq $row.runtimeAssetId) {
            ''
        }
        else {
            [string]$row.runtimeAssetId
        }
        $lines.Add(('texture:{0}:{1}:{2}:{3}:{4}' -f
            $row.bindingId, $row.rowSha256, $row.materialInputFieldId,
            $runtimeAssetId, $row.samplerPolicyRowId))
    }

    return [pscustomobject]@{
        Sha256 = Get-TextSha256 ($lines -join "`n")
        OccurrenceCount = $occurrences.Count
        RecipeId = $recipeId
        FamilyId = $familyId
        InputCount = $inputIds.Count
        StaticCount = $staticIds.Count
        RenderCount = $renderIds.Count
        TextureCount = $textureBindings.Count
    }
}

function Get-DynamicProjection([object]$Program, [string]$OccurrenceId) {
    $occurrence = Get-UniqueRow @($Program.materialOccurrences) `
        'occurrenceId' $OccurrenceId 'Runtime Material v2 dynamic occurrence'
    $modules = @($Program.modules | Where-Object {
        [string]$_.emitterId -ceq [string]$occurrence.emitterId -and
        ([string]$_.exactSourceClass).ToLowerInvariant().EndsWith(
            'particlemoduleparameterdynamic')
    })
    if ($modules.Count -ne 1) {
        throw "Runtime Material v2 dynamic module is not unique: $OccurrenceId"
    }
    $module = $modules[0]
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add(('module:{0}:{1}:{2}' -f
        $module.moduleId, $module.rowSha256, $module.order))
    $distributionIds = @($module.distributionIds)
    for ($index = 0; $index -lt $distributionIds.Count; ++$index) {
        $distribution = Get-UniqueRow @($Program.distributions) `
            'distributionId' ([string]$distributionIds[$index]) `
            'Runtime Material v2 dynamic distribution'
        $path = "dynamicparams[$index].paramname"
        $literalRows = @($Program.literals | Where-Object {
            [string]$_.moduleId -ceq [string]$module.moduleId -and
            [string]$_.propertyPath -ceq $path
        })
        if ($literalRows.Count -ne 1) {
            throw "Runtime Material v2 dynamic literal is not unique: $OccurrenceId/$index"
        }
        $literal = $literalRows[0]
        $lines.Add(('distribution:{0}:{1}:{2}:{3}' -f
            $index, $distribution.distributionId,
            $distribution.rowSha256, $distribution.propertyPath))
        $lines.Add(('literal:{0}:{1}:{2}' -f
            $index, $literal.enumStringValue, $literal.rowSha256))
    }
    return Get-TextSha256 ($lines -join "`n")
}

function Assert-RecipeProjection([object]$Program, [string]$ProgramLabel,
    [string[]]$OccurrenceIds, [string]$ExpectedSha256,
    [int]$ExpectedInputs, [int]$ExpectedStatics,
    [int]$ExpectedRender, [int]$ExpectedTextures) {
    $projection = Get-RecipeProjection $Program $OccurrenceIds
    if ([string]$projection.Sha256 -cne $ExpectedSha256 -or
        [int]$projection.OccurrenceCount -ne $OccurrenceIds.Count -or
        [int]$projection.InputCount -ne $ExpectedInputs -or
        [int]$projection.StaticCount -ne $ExpectedStatics -or
        [int]$projection.RenderCount -ne $ExpectedRender -or
        [int]$projection.TextureCount -ne $ExpectedTextures) {
        throw ("$ProgramLabel Runtime Material v2 recipe projection changed: " +
            "occurrences=$($projection.OccurrenceCount) inputs=$($projection.InputCount) " +
            "statics=$($projection.StaticCount) render=$($projection.RenderCount) " +
            "textures=$($projection.TextureCount) sha=$($projection.Sha256)")
    }
}

function Evaluate-Active004Opacity([double]$DiffuseAlpha,
    [double]$DissolveMask, [double]$DynamicAlpha) {
    $lineThickness = 1.1
    $transitionArea = 1.0
    $diffuseColorAlpha = 1.0
    $dynamic = [Math]::Max(0.0, [Math]::Min(1.0, $DynamicAlpha))
    $transition = ($DissolveMask - (1.0 - $dynamic)) *
        $lineThickness / $transitionArea
    $transition = [Math]::Max(0.0, [Math]::Min(1.0, $transition))
    return [Math]::Min($DiffuseAlpha, $transition) * $diffuseColorAlpha
}

function Evaluate-Active009010Opacity([double]$NoiseR,
    [double]$MainR, [double]$ParticleAlpha, [double]$DynamicThreshold,
    [double]$ViewNormalZ, [double]$U, [double]$V) {
    $dissolve = [Math]::Max(0.0, [Math]::Min(1.0,
        ($NoiseR + 1.0 - $DynamicThreshold) * 2.0))
    $boundary = [Math]::Max(0.0, [Math]::Min(1.0,
        $U * (1.0 - $U) * $V * (1.0 - $V) * 40.0))
    $fresnel = if ([Math]::Abs($ViewNormalZ) -lt 1e-6) { 0.0 } else {
        [Math]::Pow([Math]::Abs($ViewNormalZ), 1.2)
    }
    return [Math]::Max(0.0, [Math]::Min(1.0,
        $fresnel * $dissolve * ($MainR * $ParticleAlpha) * $boundary))
}

function Evaluate-Active023Shape([double]$U, [double]$V,
    [double]$SphereEnvelope) {
    $radius = 0.5
    $spherePower = 4.0
    $sphereStrength = 1.0
    $fresnelPower = 1.0
    $distance = [Math]::Sqrt(
        (($U - 0.5) * ($U - 0.5)) + (($V - 0.5) * ($V - 0.5)))
    $normalizedRadius = $distance / [Math]::Max($radius, 0.0001)
    $carrier = [Math]::Max(0.0, [Math]::Min(1.0, 1.0 - $normalizedRadius))
    $center = [Math]::Pow($carrier, [Math]::Max($spherePower, 0.0001)) *
        [Math]::Max($sphereStrength, 0.0)
    $rim = [Math]::Pow(
        [Math]::Max(0.0, [Math]::Min(1.0, $normalizedRadius)),
        [Math]::Max($fresnelPower, 0.0001)) * $carrier
    return [Math]::Max(0.0, [Math]::Min(1.0, $center + $rim)) *
        [Math]::Max(0.0, [Math]::Min(1.0, $SphereEnvelope))
}

function Evaluate-Active016Sample([double]$DynamicAlpha,
    [double]$DynamicEdge) {
    $localU = 0.25
    $localV = 0.75
    $time = 0.25
    $dynamicPan = 1.0
    $dynamicDistortion = 1.0
    $noise = @(0.8, 0.2, 0.6, 1.0)
    $mask = @(0.1, 0.3, 0.7, 0.9)
    $spec = @(0.6, 0.5, 0.4, 1.0)
    $base = @(0.7, 0.2, 0.1, 0.8)
    $emissive = @(0.2, 0.4, 0.8, 0.5)
    $particle = @(0.9, 0.8, 1.0, 0.75)

    $noiseU = (($localU * 2.0) + (-0.02 * $dynamicPan * $time)) % 1.0
    $noiseV = (($localV * 2.0) + (0.1 * $dynamicPan * $time)) % 1.0
    $maskU = (($localU * 1.0) + (0.03 * $dynamicPan * $time)) % 1.0
    $maskV = (($localV * 1.0) + (0.01 * $dynamicPan * $time)) % 1.0
    $warpU = [Math]::Max(-0.25, [Math]::Min(0.25,
        (($noise[0] * 2.0) - 1.0) * 0.05 * $dynamicDistortion))
    $warpV = [Math]::Max(-0.25, [Math]::Min(0.25,
        (($noise[1] * 2.0) - 1.0) * 0.05 * $dynamicDistortion))
    $baseLocalU = (($localU * 0.5) + (0.0 * $dynamicPan * $time) +
        $warpU) % 1.0
    $baseLocalV = (($localV * 0.5) + (0.2 * $dynamicPan * $time) +
        $warpV) % 1.0
    $baseAtlasU = ($baseLocalU * 0.5) + 0.5
    $baseAtlasV = ($baseLocalV * 0.5) + 0.5

    $maskMean = ($mask[0] + $mask[1] + $mask[2] + $mask[3]) * 0.25
    $maskShape = [Math]::Pow(
        [Math]::Max(0.0, [Math]::Min(1.0, $maskMean)), 1.0)
    $baseLuma = $base[0] * 0.299 + $base[1] * 0.587 +
        $base[2] * 0.114
    $specLuma = $spec[0] * 0.299 + $spec[1] * 0.587 +
        $spec[2] * 0.114
    $rgb = [double[]]::new(3)
    for ($channel = 0; $channel -lt 3; ++$channel) {
        $baseShaped = $base[$channel] * 0.15 + $baseLuma * 0.85
        $noiseMod = 0.5 + 0.5 * $noise[$channel]
        $specShaped = $spec[$channel] * 0.25 + $specLuma * 0.75
        $specTerm = [Math]::Pow(
            [Math]::Max(0.0, [Math]::Min(1.0, $specShaped)), 4.0) * 0.5
        $edge = [Math]::Max(0.0, [Math]::Min(1.0,
            $maskShape * [Math]::Max($DynamicEdge, 0.0)))
        # The cooked ColorOverLife RGB default is unresolved and currently
        # evaluates to zero in Playback.  The bounded material policy does not
        # claim that the stripped graph consumed ParticleColor RGB.
        $rgb[$channel] = [Math]::Max(0.0, [Math]::Min(16.0,
            $baseShaped * $noiseMod + $emissive[$channel] +
                $specTerm + $edge))
    }
    $alpha = [Math]::Max(0.0, [Math]::Min(1.0,
        $base[3] * $maskShape *
        [Math]::Max(0.0, [Math]::Min(1.0, $DynamicAlpha)) * $particle[3]))
    return [pscustomobject]@{
        NoiseUV = @($noiseU, $noiseV)
        MaskUV = @($maskU, $maskV)
        Warp = @($warpU, $warpV)
        BaseAtlasUV = @($baseAtlasU, $baseAtlasV)
        Rgba = @($rgb[0], $rgb[1], $rgb[2], $alpha)
    }
}

function Evaluate-Active016PackedAxisProbe([double]$EmissivePackedY,
    [double]$EmissivePackedX,
    [double]$SpecPackedY,
    [double]$SpecPackedX) {
    $localU = 0.25
    $localV = 0.75
    $emissiveU = ($localU * $EmissivePackedX) % 1.0
    $emissiveV = ($localV * $EmissivePackedY) % 1.0
    $specU = ($localU * $SpecPackedX) % 1.0
    $specV = ($localV * $SpecPackedY) % 1.0
    return [pscustomobject]@{
        EmissiveUV = @($emissiveU, $emissiveV)
        SpecUV = @($specU, $specV)
    }
}

function Evaluate-Active030Sample([double]$DynamicAlpha,
    [double]$DynamicEdge, [double]$DynamicDistortion) {
    $localU = 0.25
    $localV = 0.75
    $time = 0.25
    $base = @(0.2, 0.4, 0.8, 0.5)
    $noise = @(0.8, 0.2, 0.6, 1.0)
    $particle = @(0.9, 0.8, 1.0, 0.75)

    $noiseU = (($localU * 2.0) + (-0.02 * $time) + 0.03) % 1.0
    $noiseV = (($localV * 2.0) + (0.1 * $time) + 0.03) % 1.0
    $warpU = [Math]::Max(-0.25, [Math]::Min(0.25,
        (($noise[0] * 2.0) - 1.0) * 0.05 * $DynamicDistortion))
    $warpV = [Math]::Max(-0.25, [Math]::Min(0.25,
        (($noise[1] * 2.0) - 1.0) * 0.05 * $DynamicDistortion))
    $baseU = ((($localU + $warpU) % 1.0) * 0.5) + 0.5
    $baseV = ((($localV + $warpV) % 1.0) * 0.5) + 0.5

    $luma = $base[0] * 0.299 + $base[1] * 0.587 +
        $base[2] * 0.114
    $mask = [Math]::Pow(
        [Math]::Max(0.0, [Math]::Min(1.0, $luma)), 1.0)
    $radial = [Math]::Min(1.0, [Math]::Sqrt(
        (($localU - 0.5) * ($localU - 0.5)) +
        (($localV - 0.5) * ($localV - 0.5))) * 2.0)
    $rgb = [double[]]::new(3)
    for ($channel = 0; $channel -lt 3; ++$channel) {
        $shaped = $base[$channel] * 0.15 + $luma * 0.85
        $noiseMod = 0.5 + 0.5 * $noise[$channel]
        $edge = $radial * [Math]::Max(0.0,
            [Math]::Min(1.0, $DynamicEdge))
        $rgb[$channel] = [Math]::Max(0.0, [Math]::Min(16.0,
            ($shaped * $noiseMod + $edge) * 3.0 * $particle[$channel]))
    }
    $alpha = [Math]::Max(0.0, [Math]::Min(1.0,
        $base[3] * $mask * 50.0 *
        [Math]::Max(0.0, [Math]::Min(1.0, $DynamicAlpha)) * $particle[3]))
    return [pscustomobject]@{
        NoiseUV = @($noiseU, $noiseV)
        Warp = @($warpU, $warpV)
        BaseUV = @($baseU, $baseV)
        Rgba = @($rgb[0], $rgb[1], $rgb[2], $alpha)
    }
}

function Evaluate-Active031Sample([double]$Time, [double[]]$Dynamic,
    [double[]]$Particle, [double[]]$Edge, [double[]]$Sparkle) {
    $localU = 0.375
    $localV = 0.625
    $intensity = 30.0
    $tiling = 1.5
    $panning = 0.10000000149011612
    $transition = [Math]::Max(0.0, [Math]::Min(1.0, $Dynamic[0]))
    $alphaPower = [Math]::Max(0.001, [Math]::Min(64.0,
        [Math]::Abs($Dynamic[1])))
    $uvNoise01 = [Math]::Max(0.0, [Math]::Min(1.0,
        [Math]::Max($Dynamic[2], 0.0) * 0.5))
    $smokePan = [Math]::Max(-8.0, [Math]::Min(8.0, $Dynamic[3]))
    $phase = $panning * $smokePan * $Time
    $edgeU = (($localU * $tiling) + $phase) % 1.0
    $edgeV = (($localV * $tiling) - $phase) % 1.0
    $warpU = [Math]::Max(-0.05, [Math]::Min(0.05,
        (($Edge[0] * 2.0) - 1.0) * (0.05 * $uvNoise01)))
    $warpV = [Math]::Max(-0.05, [Math]::Min(0.05,
        (($Edge[1] * 2.0) - 1.0) * (0.05 * $uvNoise01)))
    $sparkleU = (($localU * $tiling) + $phase + $warpU) % 1.0
    $sparkleV = (($localV * $tiling) + $warpV) % 1.0
    $combined = [double[]]::new(3)
    for ($channel = 0; $channel -lt 3; ++$channel) {
        $combined[$channel] = [Math]::Max(0.0, [Math]::Min(1.0,
            $Sparkle[$channel] * (0.5 + (0.5 * $Edge[$channel]))))
    }
    $carrier = ($combined[0] * 0.299) + ($combined[1] * 0.587) +
        ($combined[2] * 0.114)
    $gate = [Math]::Max(0.0, [Math]::Min(1.0,
        ($carrier - $transition) * $intensity))
    $coverage = [Math]::Pow($gate, $alphaPower)
    $particlePeak = [Math]::Max(1.0,
        [Math]::Max($Particle[0], [Math]::Max($Particle[1], $Particle[2])))
    $rgb = [double[]]::new(3)
    for ($channel = 0; $channel -lt 3; ++$channel) {
        $tint = [Math]::Max(0.0, [Math]::Min(1.0,
            [Math]::Max($Particle[$channel], 0.0) / $particlePeak))
        $rgb[$channel] = [Math]::Max(0.0, [Math]::Min(16.0,
            $combined[$channel] * $intensity * $tint))
    }
    $particleAlpha = [Math]::Max(0.0, [Math]::Min(1.0,
        [Math]::Max($Particle[3], 0.0) / 50.0))
    return [pscustomobject]@{
        EdgeUV = @($edgeU, $edgeV)
        Warp = @($warpU, $warpV)
        SparkleUV = @($sparkleU, $sparkleV)
        Rgba = @($rgb[0], $rgb[1], $rgb[2],
            [Math]::Max(0.0, [Math]::Min(1.0, $coverage * $particleAlpha)))
    }
}

$catalogRelative = 'Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json'
$candidateRelative =
    'Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json'
$hlslRelative = 'Client/Bin/ShaderFiles/Shader_Artist31470RuntimeMaterial.hlsli'
$active011HlslRelative =
    'Client/Bin/ShaderFiles/Shader_Artist31470Active011OuterMaterial.hlsli'
$meshShaderRelative = 'Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl'
$particleShaderRelative = 'Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl'
$decalShaderRelative = 'Client/Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl'
$trailShaderRelative = 'Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl'
$rendererRelative = 'Client/Private/Effect_DocumentRenderer.cpp'
$rendererHeaderRelative = 'Client/Public/Effect_DocumentRenderer.h'
$runtimeFactoryRelative = 'Client/Private/Effect_ReconstructedExecution.cpp'
$expectedHlslSha256 =
	'ae9115a0baa61c7f301c207d5355af95d3b8202540872d6c535038fff18b74fa'
$expectedActive011HlslSha256 =
    'dd9f1e41a1eb84f6135633120ae3ec6fda03e47a2b697dda129d84cdd06cc031'
$expectedMeshShaderSha256 =
	'a91a82901dcdabeb63f6a2b9ac07048ad90c704b73e33ffcc4809ea01cd20e62'
$expectedParticleShaderSha256 =
    '95497261843f6dcc90c17796a91dabfd16fb64a0c1557ce0bbf3f383c1f45422'

Push-Location $RepositoryRoot
try {
    $catalogPath = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $catalogRelative))
    $candidatePath = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $candidateRelative))
    $hlslPath = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $hlslRelative))
    $active011HlslPath = [IO.Path]::GetFullPath(
        (Join-Path $RepositoryRoot $active011HlslRelative))
    $meshShaderPath = [IO.Path]::GetFullPath(
        (Join-Path $RepositoryRoot $meshShaderRelative))
	$particleShaderPath = [IO.Path]::GetFullPath(
		(Join-Path $RepositoryRoot $particleShaderRelative))
	$decalShaderPath = [IO.Path]::GetFullPath(
		(Join-Path $RepositoryRoot $decalShaderRelative))
	$trailShaderPath = [IO.Path]::GetFullPath(
		(Join-Path $RepositoryRoot $trailShaderRelative))
    $rendererPath = [IO.Path]::GetFullPath((Join-Path $RepositoryRoot $rendererRelative))
    $rendererHeaderPath = [IO.Path]::GetFullPath(
        (Join-Path $RepositoryRoot $rendererHeaderRelative))
    $runtimeFactoryPath = [IO.Path]::GetFullPath(
        (Join-Path $RepositoryRoot $runtimeFactoryRelative))

    $catalog = Get-Content -LiteralPath $catalogPath -Raw -Encoding UTF8 |
        ConvertFrom-Json
    $effectRows = @($catalog.effects | Where-Object {
        [string]$_.effectAssetId -ceq 'effect.artist.skill.31470'
    })
    if ($effectRows.Count -ne 1) {
        throw 'Artist F Catalog runtime entry is not unique.'
    }
    Assert-ExactOrderedProperties $effectRows[0] @(
        'payloadKind', 'effectAssetId', 'artifactRevision', 'compilerRevision',
        'sourceExact', 'runtimeExecutionAdmission', 'productAdmission',
        'publishReceiptSha256', 'publishReceipt', 'reconstructedRuntimeProgram',
        'renderResourcePublishReceiptSha256', 'renderResourcePublishReceipt',
        'reconstructedRenderResourceAuthority') 'Artist F Catalog outer entry'
    Assert-ExactStringProperty $effectRows[0] 'effectAssetId' `
        'effect.artist.skill.31470' 'Artist F Catalog outer entry'
    Assert-ExactStringProperty $effectRows[0] 'payloadKind' `
        'IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM' 'Artist F Catalog outer entry'
    Assert-ExactInt32Property $effectRows[0] 'artifactRevision' 1 `
        'Artist F Catalog outer entry'
    Assert-ExactStringProperty $effectRows[0] 'compilerRevision' `
        'artist31470.reconstructed-runtime-program-link-v1' `
        'Artist F Catalog outer entry'
    Assert-ExactStringProperty $effectRows[0] 'publishReceiptSha256' `
        '5812cbc6794705e2644853db596e3c55ebace8faf4079a4a0b3f374b1c203836' `
        'Artist F Catalog outer entry'
    Assert-ExactStringProperty $effectRows[0] `
        'renderResourcePublishReceiptSha256' `
        '01cd26412754ffd8e7acb4e7c1fe4280ec2dfab65077a664c86eac6a415b8541' `
        'Artist F Catalog outer entry'
    foreach ($name in @('sourceExact', 'runtimeExecutionAdmission',
        'productAdmission')) {
        Assert-ExactBooleanProperty $effectRows[0] $name $false `
            'Artist F Catalog outer entry'
    }
    $publishReceiptProjection = Get-TextSha256 `
        ($effectRows[0].publishReceipt | ConvertTo-Json -Compress -Depth 100)
    $renderPublishReceiptProjection = Get-TextSha256 `
        ($effectRows[0].renderResourcePublishReceipt |
            ConvertTo-Json -Compress -Depth 100)
    if ($publishReceiptProjection -cne
            '6fabc270a84cd8d0901139e66993230b0e1323ce7f22868945b59f45f3540c23' -or
        $renderPublishReceiptProjection -cne
            '5b940befc9bb8355a7c43c466062633a3463c43195c81d64eb6ff9c28cff89bc') {
        throw ("Artist F Catalog nested publication receipt changed: " +
            "program=$publishReceiptProjection render=$renderPublishReceiptProjection")
    }
    $runtimeEntry = $effectRows[0].reconstructedRuntimeProgram
    Assert-ExactOrderedProperties $runtimeEntry @(
        'schema', 'formatVersion', 'encoding', 'effectAssetId',
        'candidateBuilderCommitId', 'candidateBuilderTreeId', 'candidateBlobId',
        'resourceBindingHash', 'inputArtifactCount',
        'inputArtifactsOrderedSha256', 'programId', 'programVersion',
        'programSha256', 'candidateRawSha256', 'candidateByteCount',
        'candidateUtf8Json') 'Artist F Catalog Program link'
    $expectedProgramStrings = [ordered]@{
        schema = 'lostark.effect-reconstructed-runtime-program-link'
        encoding = 'UTF8_JSON_EXACT'
        effectAssetId = 'effect.artist.skill.31470'
        candidateBuilderCommitId = '31ecc2edc328347ac6e3bf6fe444c270d463ef40'
        candidateBuilderTreeId = '6d8853d989bd71a988eaebf254398ae08599ad0d'
        candidateBlobId = 'a9655729578d847d323a32c904c70d10baee9102'
        resourceBindingHash = 'df15009e41b6c1fe9161af873b96dfc428771944786c14f9435f7c0ffa4d869c'
        inputArtifactsOrderedSha256 = 'da83b7d05b8d97357fa379b3a5c48bdb4296883647e455babc55adaff09b8ef6'
        programId = 'effect.artist.skill.31470.reconstructed-approved-v1'
        programSha256 = '8e618a53242fb2fee9b13528d9696182038ded977454d98ff49ff500570ebeb8'
        candidateRawSha256 = 'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6'
    }
    foreach ($pair in $expectedProgramStrings.GetEnumerator()) {
        Assert-ExactStringProperty $runtimeEntry ([string]$pair.Key) `
            ([string]$pair.Value) 'Artist F Catalog Program link'
    }
    Assert-ExactInt32Property $runtimeEntry 'formatVersion' 1 `
        'Artist F Catalog Program link'
    Assert-ExactInt32Property $runtimeEntry 'inputArtifactCount' 13 `
        'Artist F Catalog Program link'
    Assert-ExactInt32Property $runtimeEntry 'programVersion' 1 `
        'Artist F Catalog Program link'
    Assert-ExactInt32Property $runtimeEntry 'candidateByteCount' 15117436 `
        'Artist F Catalog Program link'
    $embeddedJson = [string]$runtimeEntry.candidateUtf8Json
    $embeddedBytes = [Text.Encoding]::UTF8.GetBytes($embeddedJson)
    $embeddedRawSha256 = Get-TextSha256 $embeddedJson
    if ($embeddedBytes.Length -ne 15117436 -or
        $embeddedRawSha256 -cne
            'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6') {
        throw ("Artist F Catalog embedded Program bytes changed: " +
            "bytes=$($embeddedBytes.Length) raw=$embeddedRawSha256")
    }
    $program = $embeddedJson | ConvertFrom-Json
    if ([string]$program.programSha256 -cne
        [string]$runtimeEntry.programSha256) {
        throw 'Artist F Catalog link and embedded Program self hash diverged.'
    }

    $candidateBytes = [IO.File]::ReadAllBytes($candidatePath)
    $candidateRawSha256 = (Get-FileHash -LiteralPath $candidatePath `
        -Algorithm SHA256).Hash.ToLowerInvariant()
    $candidateJson = [Text.Encoding]::UTF8.GetString($candidateBytes)
    $candidateProgram = $candidateJson | ConvertFrom-Json
    if ($candidateBytes.Length -ne 15117436 -or
        $candidateRawSha256 -cne
            'bdeccba5b204ffae0bc88469b90158ff3479da0a113c437c2842f1f91f5f04f6' -or
        [string]$candidateProgram.programSha256 -cne
            '8e618a53242fb2fee9b13528d9696182038ded977454d98ff49ff500570ebeb8') {
        throw ("Artist F standalone candidate identity changed: " +
            "bytes=$($candidateBytes.Length) raw=$candidateRawSha256 " +
            "program=$($candidateProgram.programSha256)")
    }

    # Catalog and standalone Program now share the corrected snapshot-root and
    # material authority byte-for-byte.  Section parity prevents this visual
    # packet test from silently validating a different execution artifact.
    $embeddedSections = @{}
    foreach ($row in @($program.sectionDigests)) {
        $embeddedSections[[string]$row.sectionName] = $row
    }
    $candidateSections = @{}
    foreach ($row in @($candidateProgram.sectionDigests)) {
        $candidateSections[[string]$row.sectionName] = $row
    }
    foreach ($sectionName in $embeddedSections.Keys) {
        if (-not $candidateSections.ContainsKey($sectionName)) {
            throw "Artist F standalone candidate section is missing: $sectionName"
        }
        $embeddedSection = $embeddedSections[$sectionName]
        $candidateSection = $candidateSections[$sectionName]
        if ([int]$embeddedSection.rowCount -ne
                [int]$candidateSection.rowCount -or
            [string]$embeddedSection.orderedSha256 -cne
                [string]$candidateSection.orderedSha256) {
            throw "Artist F non-emitter section diverged: $sectionName"
        }
    }

    $sidecarLink = $effectRows[0].reconstructedRenderResourceAuthority
    Assert-ExactOrderedProperties $sidecarLink @(
        'schema', 'formatVersion', 'encoding', 'effectAssetId', 'programId',
        'programVersion', 'programSha256', 'sidecarSchema',
        'sidecarFormatVersion', 'sidecarAuthorityId',
        'sidecarDecisionProjectionSha256', 'sidecarReceiptSha256',
        'sidecarRawSha256', 'sidecarByteCount', 'sourceExact',
        'runtimeExecutionAdmission', 'executeAdmission', 'submitAdmission',
        'renderAdmission', 'productAdmission', 'sidecarUtf8Json') `
        'Artist F Catalog render-resource link'
    $expectedSidecarStrings = [ordered]@{
        schema = 'lostark.effect-reconstructed-render-resource-authority-link'
        encoding = 'UTF8_JSON_EXACT'
        effectAssetId = 'effect.artist.skill.31470'
        programId = 'effect.artist.skill.31470.reconstructed-approved-v1'
        programSha256 = '8e618a53242fb2fee9b13528d9696182038ded977454d98ff49ff500570ebeb8'
        sidecarSchema = 'lostark.artist-31470-reconstructed-render-resource-authority-receipt'
        sidecarAuthorityId = 'ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1'
        sidecarDecisionProjectionSha256 = 'fcef9bb95c5412f1d25f206e207b6eccd8198a26a8994a6ee5ac179498b001de'
        sidecarReceiptSha256 = '6f4ed12c7c5b6499ece7cf520436f747e4877a4a89a1584ba57de7324adf8ac4'
        sidecarRawSha256 = '1567c622876f74018ac9a21a4ba9e04dd8a3fd08f0bfe934698a65b8185d2660'
    }
    foreach ($pair in $expectedSidecarStrings.GetEnumerator()) {
        Assert-ExactStringProperty $sidecarLink ([string]$pair.Key) `
            ([string]$pair.Value) 'Artist F Catalog render-resource link'
    }
    foreach ($pair in @(@('formatVersion', 1), @('programVersion', 1),
        @('sidecarFormatVersion', 1), @('sidecarByteCount', 774127))) {
        Assert-ExactInt32Property $sidecarLink ([string]$pair[0]) `
            ([int]$pair[1]) 'Artist F Catalog render-resource link'
    }
    foreach ($name in @('sourceExact', 'runtimeExecutionAdmission',
        'executeAdmission', 'submitAdmission', 'renderAdmission',
        'productAdmission')) {
        Assert-ExactBooleanProperty $sidecarLink $name $false `
            'Artist F Catalog render-resource link'
    }
    $sidecarJson = [string]$sidecarLink.sidecarUtf8Json
    $sidecarBytes = [Text.Encoding]::UTF8.GetBytes($sidecarJson)
    $sidecarRawSha256 = Get-TextSha256 $sidecarJson
    if ($sidecarBytes.Length -ne 774127 -or
        $sidecarRawSha256 -cne
            '1567c622876f74018ac9a21a4ba9e04dd8a3fd08f0bfe934698a65b8185d2660' -or
        [string]$sidecarLink.sidecarRawSha256 -cne $sidecarRawSha256 -or
        [string]$sidecarLink.sidecarReceiptSha256 -cne
            '6f4ed12c7c5b6499ece7cf520436f747e4877a4a89a1584ba57de7324adf8ac4' -or
        [string]$sidecarLink.sidecarDecisionProjectionSha256 -cne
            'fcef9bb95c5412f1d25f206e207b6eccd8198a26a8994a6ee5ac179498b001de') {
        throw ("Artist F Catalog embedded render-resource sidecar changed: " +
            "bytes=$($sidecarBytes.Length) raw=$sidecarRawSha256")
    }
    $sidecar = $sidecarJson | ConvertFrom-Json
    if ([string]$sidecar.authorityId -cne
            'ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1' -or
        [string]$sidecar.decisionProjectionSha256 -cne
            [string]$sidecarLink.sidecarDecisionProjectionSha256 -or
        [string]$sidecar.receiptSha256 -cne
            [string]$sidecarLink.sidecarReceiptSha256 -or
        @($sidecar.textureResources).Count -ne 52 -or
        @($sidecar.textureBindings).Count -ne 77 -or
        @($sidecar.neutralProviders).Count -ne 4 -or
        @($sidecar.recipeTextureBindings).Count -ne 27 -or
        @($sidecar.rendererSlotBindings).Count -ne 57 -or
        @($sidecar.renderStateDescriptors).Count -ne 46) {
        throw 'Artist F render-resource sidecar denominator changed.'
    }
    $expectedTextureDecisions = @(
        @('material-recipe-a4ee2b242b08bb39', 'recipe-texture-binding-17',
            'cc912fcefccac31c276a5881f910985ddacf6991b9dff0c560c1719e5b8f1669', 4),
        @('material-recipe-03cc03b86c1a4c8f', 'recipe-texture-binding-00',
            '96fe88183fe5c496409628a822d7ddd6aed8b3fc71759faabd7e570442b9f43e', 2),
        @('material-recipe-ff6a10a52995006e', 'recipe-texture-binding-26',
            'dfcc7625543fd0abbc1f5c13efda128c48f4144655cc534cac10587a45fea1eb', 0),
        @('material-recipe-4070769760015ff3', 'recipe-texture-binding-05',
            '1c8f5a32cdf5b763a27bf545f7e054dfb78245fdebd692ca062b3a006a6a035c', 5),
        @('material-recipe-9c82043000d98e73', 'recipe-texture-binding-15',
            '38804278c1cf4be745df460ee1a2cde564ab88680689733d6313cd4009b59b51', 4),
		@('material-recipe-aa19ee0d487380ca', 'recipe-texture-binding-19',
			'e8fe3b0a327f9699f587e9cbcb1b9a7b1f681c2af846f5f4af1c58a10b43a667', 3),
		@('material-recipe-b508403ea55fedbc', 'recipe-texture-binding-20',
			'df8512f2e414900b981a492537db001fc0dfba75bb1af26b4ba08e3ea06808f2', 2),
		@('material-recipe-daf220acad2b656e', 'recipe-texture-binding-23',
			'764cd92a5151d4d017f403108e7ec0eae3bcd54bacd555eeb44c159274891222', 4),
		@('material-recipe-99ec58031edc07b9', 'recipe-texture-binding-14',
			'd78aaec1f03d22ef2b3063ef1e3c2b34fa5a2d8a5721348fa54560ee72ab18f0', 1)
    )
    foreach ($expected in $expectedTextureDecisions) {
        $decision = Get-UniqueRow @($sidecar.recipeTextureBindings) `
            'recipeId' ([string]$expected[0]) `
            'Artist F Runtime Material v2 texture decision'
        if ([string]$decision.recipeTextureDecisionId -cne [string]$expected[1] -or
            [string]$decision.rowSha256 -cne [string]$expected[2] -or
            @($decision.candidateTextureBindingIds).Count -ne [int]$expected[3]) {
            throw "Artist F Runtime Material v2 texture decision changed: $($expected[0])"
        }
    }

    $recipeContracts = @(
        [pscustomobject]@{
            Label = 'active004'
            Occurrences = @('source-active-004')
            Sha256 = '99ee7aba33121a1c35eb4bb675f36ac0a2c731d40227c5004f4b04468238b536'
            Inputs = 11; Statics = 0; Render = 6; Textures = 4
        },
        [pscustomobject]@{
            Label = 'active009/010'
            Occurrences = @('source-active-009', 'source-active-010')
            Sha256 = '2a7ab5563859fb7d2c222ceada3ff58f9697512bd8f7fd109667e256f8da779a'
            Inputs = 32; Statics = 14; Render = 6; Textures = 2
        },
        [pscustomobject]@{
            Label = 'active023'
            Occurrences = @('source-active-023')
            Sha256 = '9e48f27c525912d9ffaee52d54d32468906190ce4a76fd6c823dd4f6ef56ae26'
            Inputs = 5; Statics = 6; Render = 6; Textures = 0
        },
        [pscustomobject]@{
            Label = 'active016'
            Occurrences = @('source-active-016')
            Sha256 = 'b0508306fdc91d28173f90f4f98306e99cbdb86280051e310910ce2c83224bdc'
            Inputs = 55; Statics = 7; Render = 6; Textures = 5
        },
        [pscustomobject]@{
            Label = 'active030'
            Occurrences = @('source-active-030')
            Sha256 = 'ea24bad2ed9548d13ae862b86d17b8b733abae84675fdeacf4ec04e8a8d8638e'
            Inputs = 54; Statics = 7; Render = 6; Textures = 4
        },
		[pscustomobject]@{
			Label = 'active031'
            Occurrences = @('source-active-031')
            Sha256 = '372b5f5342ac583f2d286bee20b95408dd5ff86cb714cdaa0b4e6e3047985bce'
			Inputs = 6; Statics = 0; Render = 6; Textures = 3
		},
		[pscustomobject]@{
			Label = 'active003'
			Occurrences = @('source-active-003')
			Sha256 = 'ddee5897d519d937920ae23a7451fd94eef16bab3eaa9aa392c694c32f86974c'
			Inputs = 17; Statics = 1; Render = 6; Textures = 2
		},
		[pscustomobject]@{
			Label = 'active011'
			Occurrences = @('source-active-011')
			Sha256 = 'a02788d4ecd643d6fb640d9d71d8f784a632e9a8a2878ae54edb25441436c021'
			Inputs = 55; Statics = 9; Render = 6; Textures = 4
		},
		[pscustomobject]@{
			Label = 'active022'
			Occurrences = @('source-active-022')
			Sha256 = 'ee9938e1fae27af25576861fb8ce47d158adc4d76a1ffd20b202c81af229e565'
			Inputs = 29; Statics = 0; Render = 6; Textures = 1
		}
    )
    foreach ($contract in $recipeContracts) {
        Assert-RecipeProjection $program 'Catalog embedded Program' `
            $contract.Occurrences $contract.Sha256 $contract.Inputs `
            $contract.Statics $contract.Render $contract.Textures
        Assert-RecipeProjection $candidateProgram 'Standalone candidate' `
            $contract.Occurrences $contract.Sha256 $contract.Inputs `
            $contract.Statics $contract.Render $contract.Textures
    }

    $dynamicContracts = [ordered]@{
        'source-active-004' =
            '103b4716b9c09e532041df0e3e7f29fc2b1c08c576e42cba5278732de82e0103'
        'source-active-009' =
            '24ebeadfd2277aaf5ef47a82bf004dcc99bd033f0f1245ba5dae34b71d85b62d'
        'source-active-010' =
            '0a71931e88dd24ff1e7f80496be6e3b7f168497351731f78d22355690bc304b1'
        'source-active-023' =
            'ac0a4d5c7260aacfe46d254797a1e31d1ca822f00fe0c64fdd5b7aecb8cf2430'
        'source-active-016' =
            '8783aecb86f5197ac6349e43b5bb71a96db3062a054b85f9c4f1a676e3e6ebed'
        'source-active-030' =
            'a73ddcd9518bba2b4fe4ae9c0a3513c78ab2a75c754764ab97fb31ed9d422872'
        'source-active-031' =
            '84ff00fde4587d5e40dea4bf913a707ce7e4919abbaf7a06c6ecc5c30610af79'
    }
    foreach ($entry in $dynamicContracts.GetEnumerator()) {
        $embeddedSha = Get-DynamicProjection $program $entry.Key
        $candidateSha = Get-DynamicProjection $candidateProgram $entry.Key
        if ($embeddedSha -cne $entry.Value -or $candidateSha -cne $entry.Value) {
            throw ("Artist F Runtime Material v2 dynamic projection changed: " +
                "$($entry.Key) embedded=$embeddedSha candidate=$candidateSha")
        }
    }

    # Opcode 1: masked Mesh must have a visible admitted sample and a fully
    # suppressed zero-dynamic sample.  The opacity clip is 0.3330000043.
    Assert-Near (Evaluate-Active004Opacity 0.7 0.8 1.0) 0.7 1e-9 `
        'Artist F active004 visible opacity sample'
    Assert-Near (Evaluate-Active004Opacity 0.7 0.8 0.0) 0.0 1e-9 `
        'Artist F active004 zero-dynamic opacity sample'

    # Opcode 3 is the recovered original DXBC opacity slice with c0.x held at
    # the explicit neutral runtime boundary one.  Dynamic X is a UV offset;
    # Dynamic Z is the sampled dissolve threshold, not a CPU visibility gate.
    $sourceReplayOpacity = Evaluate-Active009010Opacity `
        0.8 0.9 0.75 0.0 0.6 0.5 0.5
	Assert-Near $sourceReplayOpacity 0.365666582836211 1e-9 `
		'Artist F active009/010 source-replay neutral-c0 opacity sample'
	Assert-Near (Evaluate-Active009010Opacity `
		0.0 0.9 0.75 0.0 0.6 0.5 0.5) 0.365666582836211 1e-9 `
		'Artist F active009/010 dynamic-threshold-zero remains visible'

    # Opcode 2: textureless radial carrier.  The large horizontal geometry is
    # preserved; only its all-quad white fallback is removed.
    Assert-Near (Evaluate-Active023Shape 0.5 0.5 1.0) 1.0 1e-9 `
        'Artist F active023 center sample'
    Assert-Near (Evaluate-Active023Shape 0.75 0.5 1.0) 0.3125 1e-9 `
        'Artist F active023 mid-radius sample'
    Assert-Near (Evaluate-Active023Shape 1.0 0.5 1.0) 0.0 1e-9 `
        'Artist F active023 edge sample'
    Assert-Near (Evaluate-Active023Shape 0.0 0.0 1.0) 0.0 1e-9 `
        'Artist F active023 corner sample'
    Assert-Near (Evaluate-Active023Shape 0.5 0.5 0.3) 0.3 1e-9 `
        'Artist F active023 dynamic envelope sample'

    # Opcode 4: five logical lanes remain distinct even though spec/base alias
    # the same physical DDS.  The currently selected 2x2 frame transform crops
    # the base lane only; RandomImageTime reselection cadence remains outside
    # this material opcode.  Four dynamic lanes independently control alpha,
    # pan, edge and bounded warp.
    $active016Full = Evaluate-Active016Sample 0.8999999761581421 1.0
    Assert-Near $active016Full.NoiseUV[0] 0.495 1e-6 `
        'Artist F active016 noise U'
    Assert-Near $active016Full.NoiseUV[1] 0.525 1e-6 `
        'Artist F active016 noise V'
    Assert-Near $active016Full.MaskUV[0] 0.2575 1e-6 `
        'Artist F active016 mask U'
    Assert-Near $active016Full.MaskUV[1] 0.7525 1e-6 `
        'Artist F active016 mask V'
    Assert-Near $active016Full.Warp[0] 0.03 1e-6 `
        'Artist F active016 warp U'
    Assert-Near $active016Full.Warp[1] -0.03 1e-6 `
        'Artist F active016 warp V'
    Assert-Near $active016Full.BaseAtlasUV[0] 0.5775 1e-6 `
        'Artist F active016 atlas U'
    Assert-Near $active016Full.BaseAtlasUV[1] 0.6975 1e-6 `
        'Artist F active016 atlas V'
    foreach ($sample in @(
        @($active016Full.Rgba, @(1.0953085290, 1.1252968759,
            1.5704681873, 0.2699999809), 'full'),
        @((Evaluate-Active016Sample 0.3747076094 1.0).Rgba,
            @(1.0953085290, 1.1252968759, 1.5704681873,
                0.1124122888), 'partial'),
        @((Evaluate-Active016Sample 0.0 1.0).Rgba,
            @(1.0953085290, 1.1252968759, 1.5704681873, 0.0), 'zero-alpha'),
        @((Evaluate-Active016Sample 0.3747076094 0.0).Rgba,
            @(0.5953086084, 0.6252969056, 1.0704681873,
                0.1124122888), 'zero-edge'))) {
        for ($channel = 0; $channel -lt 4; ++$channel) {
            Assert-Near ([double]$sample[0][$channel]) `
                ([double]$sample[1][$channel]) 1e-6 `
                "Artist F active016 $($sample[2]) channel $channel"
        }
    }

    # Opcode 6 is the two-provider wind/sparkle reconstruction.  The source
    # textures carry opaque alpha, so coverage is derived from RGB while the
    # explicit HDR particle color is normalized to tint + a 0..50 alpha range.
    $active031Samples = @(
        @((Evaluate-Active031Sample 0.0 `
            @(0.0, 0.0014656512066721916, 50.479740142822266,
                1.2971277982397516) `
            @(20.0, 20.0, 60.0, 0.0) `
            @(0.83, 0.19, 0.61, 0.97) @(0.17, 0.41, 0.73, 0.29)),
            @(@(0.5625, 0.9375),
                @(0.032999999821186066, -0.03100000135600567),
                @(0.5954999923706055, 0.906499981880188),
                @(1.5554999113082886, 2.439500331878662, 16.0, 0.0)), 'start'),
        @((Evaluate-Active031Sample 0.25 `
            @(0.08280456066131592, 1.6853822469711304,
                50.479740142822266, 1.4867103099822998) `
            @(20.0, 20.0, 60.0, 12.759305953979492) `
            @(0.5, 0.25, 0.75, 1.0) @(0.12, 0.15, 0.18, 1.0)),
            @(@(0.5996677875518799, 0.9003322124481201),
                @(0.0, -0.02500000037252903),
                @(0.5996677875518799, 0.9125000238418579),
                @(0.8999999761581421, 0.9375, 4.724999904632568,
                    0.08278238028287888)), 'mid'),
        @((Evaluate-Active031Sample 1.0 `
            @(0.48035117983818054, 6.755776882171631,
                50.479740142822266, 2.037233352661133) `
            @(20.0, 20.0, 60.0, 50.0) `
            @(0.8, 0.8, 0.8, 0.0) @(0.56, 0.56, 0.56, 0.0)),
            @(@(0.7662233114242554, 0.7337766885757446),
                @(0.030000001192092896, 0.030000001192092896),
                @(0.7962232828140259, 0.9674999713897705),
                @(5.039999961853027, 5.039999961853027,
                    15.119999885559082, 0.09838350862264633)), 'end'))
    foreach ($sample in $active031Samples) {
        $actual = $sample[0]
        $expected = $sample[1]
        $fieldNames = @('EdgeUV', 'Warp', 'SparkleUV', 'Rgba')
        for ($fieldIndex = 0; $fieldIndex -lt $fieldNames.Count; ++$fieldIndex) {
            $field = $fieldNames[$fieldIndex]
            for ($index = 0; $index -lt $expected[$fieldIndex].Count; ++$index) {
                Assert-Near ([double]$actual.$field[$index]) `
                    ([double]$expected[$fieldIndex][$index]) 2e-6 `
                    "Artist F active031 $($sample[2]) $field/$index"
            }
        }
    }

    $active016Axis = Evaluate-Active016PackedAxisProbe 2.0 3.0 4.0 5.0
    Assert-Near $active016Axis.EmissiveUV[0] 0.75 1e-9 `
        'Artist F active016 packed emissive X axis'
    Assert-Near $active016Axis.EmissiveUV[1] 0.5 1e-9 `
        'Artist F active016 packed emissive Y axis'
    Assert-Near $active016Axis.SpecUV[0] 0.25 1e-9 `
        'Artist F active016 packed spec X axis'
    Assert-Near $active016Axis.SpecUV[1] 0.0 1e-9 `
        'Artist F active016 packed spec Y axis'

    # Opcode 5 is a separate two-provider reconstruction for active030.  It
    # deliberately preserves the frozen recipe's suppressed map_f_panning_y
    # input and consumes particle RGBA, unlike active016's blocked RGB policy.
    $active030Full = Evaluate-Active030Sample 1.0 1.0 1.0
    Assert-Near $active030Full.NoiseUV[0] 0.525 1e-6 `
        'Artist F active030 noise U'
    Assert-Near $active030Full.NoiseUV[1] 0.555 1e-6 `
        'Artist F active030 noise V'
    Assert-Near $active030Full.Warp[0] 0.03 1e-6 `
        'Artist F active030 warp U'
    Assert-Near $active030Full.Warp[1] -0.03 1e-6 `
        'Artist F active030 warp V'
    Assert-Near $active030Full.BaseUV[0] 0.64 1e-6 `
        'Artist F active030 atlas U'
    Assert-Near $active030Full.BaseUV[1] 0.86 1e-6 `
        'Artist F active030 atlas V'
    foreach ($sample in @(
        @($active030Full.Rgba,
            @(2.7789582092, 2.2556754748, 3.1963523436, 1.0), 'full'),
        @((Evaluate-Active030Sample 0.7314829230 1.0 1.0).Rgba,
            @(2.7789582092, 2.2556754748, 3.1963523436, 1.0), 'partial'),
        @((Evaluate-Active030Sample 0.0 1.0 1.0).Rgba,
            @(2.7789582092, 2.2556754748, 3.1963523436, 0.0), 'zero-alpha'),
        @((Evaluate-Active030Sample 0.7314829230 0.0 1.0).Rgba,
            @(0.8697699, 0.5586192, 1.075032, 1.0), 'zero-edge'))) {
        for ($channel = 0; $channel -lt 4; ++$channel) {
            Assert-Near ([double]$sample[0][$channel]) `
                ([double]$sample[1][$channel]) 1e-6 `
                "Artist F active030 $($sample[2]) channel $channel"
        }
    }

    $hlslSha256 = Get-NormalizedTextSha256 $hlslPath
    if ($hlslSha256 -cne $expectedHlslSha256) {
        throw "Artist F Runtime Material v2 HLSL changed: $hlslSha256"
    }
    $hlsl = [IO.File]::ReadAllText($hlslPath)
	$active011HlslSha256 = Get-NormalizedTextSha256 $active011HlslPath
	if ($active011HlslSha256 -cne $expectedActive011HlslSha256) {
		throw "Artist F active011 source-replay HLSL changed: $active011HlslSha256"
	}
	$active011Hlsl = [IO.File]::ReadAllText($active011HlslPath)
	$meshShaderSha256 = Get-NormalizedTextSha256 $meshShaderPath
	$particleShaderSha256 = Get-NormalizedTextSha256 $particleShaderPath
	$decalShaderSha256 = Get-NormalizedTextSha256 $decalShaderPath
	$trailShaderSha256 = Get-NormalizedTextSha256 $trailShaderPath
	if ($meshShaderSha256 -cne $expectedMeshShaderSha256 -or
		$particleShaderSha256 -cne $expectedParticleShaderSha256 -or
		$decalShaderSha256 -cne
			'2a4012d4099380098a5c2e80e49c13e66df8584a3ce3c6798d61dae3dea57f9c' -or
		$trailShaderSha256 -cne
			'9a614d524332ce4be7e63b7fa234db6f7e47ced01a54fecffec069b0375fbc83') {
		throw ("Artist F Runtime Material v2 parent shader changed: " +
			"mesh=$meshShaderSha256 particle=$particleShaderSha256 " +
			"decal=$decalShaderSha256 trail=$trailShaderSha256")
    }
    $meshShader = [IO.File]::ReadAllText($meshShaderPath)
	$particleShader = [IO.File]::ReadAllText($particleShaderPath)
	$decalShader = [IO.File]::ReadAllText($decalShaderPath)
	$trailShader = [IO.File]::ReadAllText($trailShaderPath)
    foreach ($shaderContract in @(
        @('Mesh', $meshShader, 'Shade_RuntimeMaterialV2Mesh'),
        @('Particle', $particleShader, 'Shade_RuntimeMaterialV2Particle'))) {
        foreach ($token in @(
            '#include "Shader_Artist31470RuntimeMaterial.hlsli"',
            'if (0u != g_RuntimeMaterialV2Enabled)',
            [string]$shaderContract[2])) {
			if (-not ([string]$shaderContract[1]).Contains($token)) {
				throw "Artist F Runtime Material v2 $($shaderContract[0]) entrypoint token missing: $token"
			}
		}
	}
	foreach ($shaderContract in @(
		@('Decal', $decalShader, 'Shade_Artist31470RuntimeMaterialV2Active022Decal'),
		@('Trail', $trailShader, 'Shade_Artist31470Active003RibbonMaterial'))) {
		foreach ($token in @(
			'#include "Shader_Artist31470RuntimeMaterial.hlsli"',
			'if (0u != g_RuntimeMaterialV2Enabled)',
			[string]$shaderContract[2])) {
			if (-not ([string]$shaderContract[1]).Contains($token)) {
				throw "Artist F Runtime Material v2 $($shaderContract[0]) entrypoint token missing: $token"
	}
        }
    }
    $renderer = [IO.File]::ReadAllText($rendererPath)
    $rendererHeader = [IO.File]::ReadAllText($rendererHeaderPath)
    $runtimeFactory = [IO.File]::ReadAllText($runtimeFactoryPath)

    $runtimeV2ProfileSet = Get-RequiredTextSlice $runtimeFactory `
        "`t`t`t`tconst bool_t bRuntimeMaterialV2 =" `
        "`t`t`t`tif (bMainCompositeMaterial)" 0 `
        'Runtime Material v2 factory profile set'
    $runtimeV2Orders = @([regex]::Matches($runtimeV2ProfileSet.Text,
        'Occurrence->Row\.iOrder == ([0-9]+)u') |
        ForEach-Object { [int]$_.Groups[1].Value })
    $expectedRuntimeV2Orders = @(3, 4, 9, 10, 11, 16, 22, 23, 30, 31)
    if ($runtimeV2Orders.Count -ne $expectedRuntimeV2Orders.Count) {
        throw "Runtime Material v2 factory profile denominator changed: actual=$($runtimeV2Orders.Count) expected=$($expectedRuntimeV2Orders.Count)"
    }
    for ($index = 0; $index -lt $expectedRuntimeV2Orders.Count; ++$index) {
        if ($runtimeV2Orders[$index] -ne $expectedRuntimeV2Orders[$index]) {
            throw "Runtime Material v2 factory profile order changed at $index`: actual=$($runtimeV2Orders[$index]) expected=$($expectedRuntimeV2Orders[$index])"
        }
    }
    Assert-TextTokens $runtimeFactory @(
        'else if (bMainCompositeMaterial || bRuntimeMaterialV2)',
        'iRuntimeMaterialV2ProfileProjectionCount',
        'iRuntimeMaterialV2ProfileProjectionCount != 10u') `
        'Runtime Material v2 factory/profile seam'
    foreach ($token in @(
        'RUNTIME_MATERIAL_V2_ACTIVE004_MASKED_MESH',
        'g_RuntimeMaterialV2InputConsumedMask != uint2(0x7ffu, 0u)',
        'g_RuntimeMaterialV2DynamicSuppressedMask != 0x0eu',
        'RUNTIME_MATERIAL_V2_ACTIVE009_010_INK_CORE',
        'uint2(0xcffffff7u, 0u)',
        'uint2(0x30000008u, 0u)',
        'g_RuntimeMaterialV2StaticSelectedMask != 0x33ffu',
        'g_RuntimeMaterialV2StaticConsumedMask != 0x3fffu',
        'g_RuntimeMaterialV2StaticSuppressedMask != 0u',
        'RUNTIME_MATERIAL_V2_ACTIVE023_PROCEDURAL_GLOW',
        'g_RuntimeMaterialV2DynamicConsumedMask != 0x04u',
        'g_RuntimeMaterialV2DynamicSuppressedMask != 0x0bu',
        'RUNTIME_MATERIAL_V2_ACTIVE016_LAYERED_SUBUV',
        'RUNTIME_MATERIAL_V2_BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1',
        'g_RuntimeMaterialV2ParticleColorConsumedMask != 0x08u',
        'g_RuntimeMaterialV2ParticleColorSuppressedMask != 0x07u',
        'g_RuntimeMaterialV2TextureMask != 0x1fu',
        'uint2(0x3fff7fffu, 0x00001800u)',
        'uint2(0xc0008000u, 0x007fe7ffu)',
        'uint3(0x07u, 0x07u, 0u)',
        'uint3(0x08u, 0x08u, 0x0fu)',
        'Shade_RuntimeMaterialV2Active016',
        'g_RuntimeMaterialV2ScalarBlocks[5].z,',
        'g_RuntimeMaterialV2ScalarBlocks[5].y);',
        'g_RuntimeMaterialV2ScalarBlocks[8].w,',
        'g_RuntimeMaterialV2ScalarBlocks[8].z);',
        'suppresses that unresolved lane',
        'RUNTIME_MATERIAL_V2_ACTIVE030_LAYERED_SUBUV',
        'RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1',
        'uint2(0x001c353fu, 0x00000001u)',
        'uint2(0xffe3cac0u, 0x003ffffeu)',
        'uint3(0x0fu, 0u, 0u)',
        'uint3(0u, 0x0fu, 0x0fu)',
        'Shade_RuntimeMaterialV2Active030',
        'Input 9 (map_f_panning_y) remains staged and',
        'RUNTIME_MATERIAL_V2_ACTIVE031_WIND_SPARKLE',
		'RUNTIME_MATERIAL_V2_ACTIVE031_HDR_NORMALIZED_V1',
		'Shade_RuntimeMaterialV2Active031',
		'output.SceneColor.rgb = max(particleColor.rgb, 0.f);',
		'RUNTIME_MATERIAL_V2_ACTIVE022_DECAL',
		'RUNTIME_MATERIAL_V2_ACTIVE011_OUTER_MESH',
		'RUNTIME_MATERIAL_V2_ACTIVE003_RIBBON')) {
        if (-not $hlsl.Contains($token)) {
            throw "Artist F Runtime Material v2 HLSL contract token missing: $token"
        }
    }
	Assert-TextTokens $active011Hlsl @(
		'SOURCE_SHADERMAP_DXBC_REPLAYED_CANDIDATE',
		'g_RuntimeMaterialV2DynamicConsumedMask != 0x07u',
		'g_RuntimeMaterialV2DynamicSuppressedMask != 0x08u',
		'RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1',
		'uint2(0xffffffffu, 0x0000007fu)',
		'uint2(0u, 0x007fff80u)',
		'g_RuntimeMaterialV2StaticConsumedMask != 0x1ffu',
		'g_SourceTexture0.SampleLevel(',
		'g_SourceTexture3.Sample(',
		'g_SourceTexture2.Sample(',
		'dissolveSample.r + 1.1f - dynamicParameter.y',
		'mainSample.r * b3.w * particleColor.a') `
		'active011 source ShaderMap replay contract'
    foreach ($token in @(
        'const bool_t bActive004RuntimeV2 = Emitter.Row.iOrder == 4u;',
        'const bool_t bActive009010RuntimeV2 =',
        'const bool_t bActive023RuntimeV2 = Emitter.Row.iOrder == 23u;',
        'const bool_t bActive016RuntimeV2 = Emitter.Row.iOrder == 16u;',
        'const bool_t bActive030RuntimeV2 = Emitter.Row.iOrder == 30u;',
		'const bool_t bActive031RuntimeV2 = Emitter.Row.iOrder == 31u;',
		'const bool_t bActive003RuntimeV2 = Emitter.Row.iOrder == 3u;',
		'const bool_t bActive011RuntimeV2 = Emitter.Row.iOrder == 11u;',
		'const bool_t bActive022RuntimeV2 = Emitter.Row.iOrder == 22u;',
		'(bActive011RuntimeV2 && Element.Material.eRenderProfile !=',
		'EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ) ||',
        'Resource.iRuntimeMaterialV2Opcode = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 2u;',
        'Resource.iRuntimeMaterialV2Opcode = 3u;',
        'Resource.iRuntimeMaterialV2Opcode = 4u;',
        'Resource.iRuntimeMaterialV2Opcode = 5u;',
		'Resource.iRuntimeMaterialV2Opcode = 6u;',
		'Resource.iRuntimeMaterialV2Opcode = 7u;',
		'Resource.iRuntimeMaterialV2Opcode = 8u;',
		'Resource.iRuntimeMaterialV2Opcode = 9u;',
        'BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1',
        'c7f7677dc678d2673ab60741c0c922910ba8e1f30aca4cb2c9a87496194c63d5',
		'OutPrepared->iReconstructedNeutralBaseCount != 0u',
		'OutPrepared->iReconstructedMaterialEvaluatorCount != 17u',
		'OutPrepared->iRuntimeMaterialV2Count != 10u')) {
        if (-not $renderer.Contains($token)) {
            throw "Artist F Runtime Material v2 C++ contract token missing: $token"
        }
    }
	if (([regex]::Matches($renderer,
			[regex]::Escape('Resource.iRuntimeMaterialV2Enabled = 1u;'))).Count -ne 9) {
        throw 'Artist F Runtime Material v2 enabled policy branch count changed.'
    }
    $active004Packet = Get-RequiredTextSlice $renderer `
        "`t`tif (bActive004RuntimeV2)" `
        "`t`telse if (bActive009010RuntimeV2)" 0 'active004 packet branch'
    $active009010Packet = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive009010RuntimeV2)" `
        "`t`telse if (bActive016RuntimeV2)" $active004Packet.End `
        'active009/010 packet branch'
    $active016Packet = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive016RuntimeV2)" `
        "`t`telse if (bActive030RuntimeV2)" $active009010Packet.End `
        'active016 packet branch'
    $active030Packet = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive030RuntimeV2)" `
        "`t`telse if (bActive031RuntimeV2)" $active016Packet.End `
        'active030 packet branch'
	$active031Packet = Get-RequiredTextSlice $renderer `
		"`t`telse if (bActive031RuntimeV2)" `
		"`t`telse if (bActive003RuntimeV2 || bActive011RuntimeV2 ||" $active030Packet.End `
		'active031 packet branch'
	$active003011022Packet = Get-RequiredTextSlice $renderer `
		"`t`telse if (bActive003RuntimeV2 || bActive011RuntimeV2 ||" `
		"`t`telse if (bActive023RuntimeV2)" $active031Packet.End `
		'active003/011/022 packet branch'
	$active023Packet = Get-RequiredTextSlice $renderer `
		"`t`telse if (bActive023RuntimeV2)" `
		"`t`tconst auto StageTextureLane" $active003011022Packet.End `
        'active023 packet branch'
    $textureStageProjection = Get-RequiredTextSlice $renderer `
        "`t`tconst auto StageTextureLane" `
        "`t`tResource.iReconstructedMaterialEvaluatorEnabled" `
        $active023Packet.End 'Runtime Material v2 texture-stage projection'
    $packetProjectionShas = [ordered]@{
        active004 = @($active004Packet.Text,
            '1ed4b4a0bdcddbbea188c7ef831021a8c85271dc224d64afb982a861a3c5eeaf')
        active009010 = @($active009010Packet.Text,
            '4a17812bf6017827d56798eb8e68e279103d953dac9575aa5ae21a14afc12579')
		active016 = @($active016Packet.Text,
			'83c6f00944195d8b8bf51c3459c9e797c482cf972f8f62af1d497675a798f3e1')
		active030 = @($active030Packet.Text,
			'12e2ce0a43f6944ae04613ec289e726c2ddcef2cc4be8612aef6ef5516a5e614')
		active031 = @($active031Packet.Text,
			'b8b6b2394a719fe948889bcbaa95fe90dbb53c9d0a6e5dbbb0598615f0deb0fe')
		active003011022 = @($active003011022Packet.Text,
			'00863d4d50bae3969a1e764421a33fb2289d7c29cc454a8d10f3e85328cf218a')
        active023 = @($active023Packet.Text,
            '875cb5b58163cfeae79541ce87900b435261bff01afcf26a7a42db43a368d248')
		textureStage = @($textureStageProjection.Text,
			'36a93766707ad636e0ea57a0aee1756453a447a70b5439f0e7ea0bb476e1bec5')
    }
	foreach ($projection in $packetProjectionShas.GetEnumerator()) {
        $actualSha = Get-TextSha256 `
            ([string]$projection.Value[0]).Replace("`r`n", "`n")
		if ($actualSha -cne [string]$projection.Value[1]) {
			throw "Artist F Runtime Material v2 $($projection.Key) packing projection changed: $actualSha"
		}
	}
	Assert-TextTokens $textureStageProjection.Text @(
		'iLane >= Resource.SourceTextures.size() ||',
		'iLane >= Resource.RuntimeMaterialV2Samplers.size() ||') `
		'Runtime Material v2 texture-stage bounds'
    Assert-TextTokens $active004Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x01u;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0eu;',
        'Resource.iRuntimeMaterialV2ScalarCount = 6u;',
        'Resource.iRuntimeMaterialV2VectorCount = 1u;',
        'Resource.iRuntimeMaterialV2InputCount = 11u;',
        'Resource.RuntimeMaterialV2InputConsumedMask = { 0x7ffu, 0u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask = { 0u, 0u };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 0u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0u;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0u;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x3fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0u;') `
        'active004 prepared packet'
    Assert-TextTokens $active009010Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ScalarCount = 29u;',
        'Resource.iRuntimeMaterialV2VectorCount = 1u;',
        'Resource.iRuntimeMaterialV2InputCount = 32u;',
        'Resource.RuntimeMaterialV2InputConsumedMask = { 0xcffffff7u, 0u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask = { 0x30000008u, 0u };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 14u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0x33ffu;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0x3fffu;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;') `
        'active009/010 prepared packet'
    Assert-TextTokens $active023Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x04u;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0bu;',
        'Resource.iRuntimeMaterialV2ScalarCount = 5u;',
        'Resource.iRuntimeMaterialV2VectorCount = 0u;',
        'Resource.iRuntimeMaterialV2InputCount = 5u;',
        'Resource.RuntimeMaterialV2InputConsumedMask = { 0x1eu, 0u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask = { 0x01u, 0u };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 6u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0x3fu;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0x3fu;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;') `
        'active023 prepared packet'
    Assert-TextTokens $active016Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ParticleColorPolicy = 1u;',
        'Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x08u;',
        'Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0x07u;',
        'Resource.iRuntimeMaterialV2ScalarCount = 44u;',
        'Resource.iRuntimeMaterialV2VectorCount = 3u;',
        'Resource.iRuntimeMaterialV2InputCount = 55u;',
        'Resource.RuntimeMaterialV2InputConsumedMask =',
        '{ 0x3fff7fffu, 0x00001800u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask =',
        '{ 0xc0008000u, 0x007fe7ffu };',
        'Resource.RuntimeMaterialV2VectorComponentConsumedMask =',
        '{ 0x07u, 0x07u, 0u };',
        'Resource.RuntimeMaterialV2VectorComponentSuppressedMask =',
        '{ 0x08u, 0x08u, 0x0fu };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 7u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0x7bu;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0x6bu;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0x14u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;') `
        'active016 prepared packet'
    Assert-TextTokens $active030Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;',
        'Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ScalarCount = 43u;',
        'Resource.iRuntimeMaterialV2VectorCount = 3u;',
        'Resource.iRuntimeMaterialV2InputCount = 54u;',
        'Resource.RuntimeMaterialV2InputConsumedMask =',
        '{ 0x001c353fu, 0x00000001u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask =',
        '{ 0xffe3cac0u, 0x003ffffeu };',
        'Resource.RuntimeMaterialV2VectorComponentConsumedMask =',
        '{ 0x0fu, 0u, 0u };',
        'Resource.RuntimeMaterialV2VectorComponentSuppressedMask =',
        '{ 0u, 0x0fu, 0x0fu };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 7u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0x7bu;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0x6bu;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0x14u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;') `
        'active030 prepared packet'
    Assert-TextTokens $active031Packet.Text @(
        'Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ParticleColorPolicy = 3u;',
        'Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;',
        'Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2ScalarCount = 3u;',
        'Resource.iRuntimeMaterialV2VectorCount = 0u;',
        'Resource.iRuntimeMaterialV2InputCount = 6u;',
        'Resource.RuntimeMaterialV2InputConsumedMask = { 0x37u, 0u };',
        'Resource.RuntimeMaterialV2InputSuppressedMask = { 0x08u, 0u };',
        'Resource.iRuntimeMaterialV2StaticInputCount = 0u;',
        'Resource.iRuntimeMaterialV2StaticSelectedMask = 0u;',
        'Resource.iRuntimeMaterialV2StaticConsumedMask = 0u;',
        'Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;',
        'Resource.iRuntimeMaterialV2RenderInputCount = 6u;',
        'Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;',
        'Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;') `
        'active031 prepared packet'

    $enable004 = Get-RequiredTextSlice $renderer `
        "`t`tif (bActive004RuntimeV2)" `
        "`t`telse if (bActive023RuntimeV2)" $active023Packet.End `
        'active004 v2 enable branch'
    $enable023 = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive023RuntimeV2)" `
        "`t`telse if (bActive009010RuntimeV2)" $enable004.End `
        'active023 v2 enable branch'
    $enable009010 = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive009010RuntimeV2)" `
        "`t`telse if (bActive016RuntimeV2)" $enable023.End `
        'active009/010 v2 enable branch'
    $enable016 = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive016RuntimeV2)" `
        "`t`telse if (bActive030RuntimeV2)" $enable009010.End `
        'active016 v2 enable branch'
    $enable030 = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive030RuntimeV2)" `
        "`t`telse if (bActive031RuntimeV2)" $enable016.End `
        'active030 v2 enable branch'
    $enable031 = Get-RequiredTextSlice $renderer `
        "`t`telse if (bActive031RuntimeV2)" `
        "`t`tResource.iReconstructedMaterialEvaluatorEnabled" $enable030.End `
        'active031 v2 enable branch'
    Assert-TextTokens $enable004.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 1u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 4u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0x0fu;') `
        'active004 v2 enable branch'
    Assert-TextTokens $enable023.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 2u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 0u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0u;') `
        'active023 v2 enable branch'
    Assert-TextTokens $enable009010.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 3u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 2u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0x03u;') `
        'active009/010 v2 enable branch'
    Assert-TextTokens $enable016.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 4u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 5u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0x1fu;') `
        'active016 v2 enable branch'
    Assert-TextTokens $enable030.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 5u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 2u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0x03u;') `
        'active030 v2 enable branch'
    Assert-TextTokens $enable031.Text @(
        'Resource.iRuntimeMaterialV2Enabled = 1u;',
        'Resource.iRuntimeMaterialV2Opcode = 6u;',
        'Resource.iRuntimeMaterialV2TextureLaneCount = 2u;',
        'Resource.iRuntimeMaterialV2TextureMask = 0x03u;') `
        'active031 v2 enable branch'
    foreach ($token in @(
        'std::array<uint32_t, 2u> RuntimeMaterialV2InputConsumedMask{};',
        'uint32_t iRuntimeMaterialV2ParticleColorPolicy = 0u;',
        'uint32_t iRuntimeMaterialV2ParticleColorConsumedMask = 0u;',
        'uint32_t iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;',
        'std::array<float4_t, 13u> RuntimeMaterialV2ScalarBlocks{};',
        'std::array<float4_t, 3u> RuntimeMaterialV2Vectors{};',
        'RuntimeMaterialV2VectorComponentConsumedMask{};',
        'RuntimeMaterialV2VectorComponentSuppressedMask{};',
        'std::array<ComPtr<ID3D11SamplerState>, 5u>')) {
        if (-not $rendererHeader.Contains($token)) {
            throw "Artist F Runtime Material v2 ABI token missing: $token"
        }
    }
    foreach ($token in @(
        'if (!bRuntimeMaterialV2)',
        'const auto& FirstSample = Recipe->NumericBindingSamples.front();',
        'Bind_RawValue("g_RuntimeMaterialV2Enabled"',
        'Bind_RawValue("g_RuntimeMaterialV2Opcode"',
        'Bind_RawValue("g_RuntimeMaterialV2TextureLaneCount"',
        'Bind_RawValue("g_RuntimeMaterialV2TextureMask"',
        'Bind_RawValue("g_RuntimeMaterialV2DynamicConsumedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2DynamicSuppressedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2ParticleColorPolicy"',
        'g_RuntimeMaterialV2ParticleColorConsumedMask',
        'g_RuntimeMaterialV2ParticleColorSuppressedMask',
        'Bind_RawValue("g_RuntimeMaterialV2ScalarCount"',
        'Bind_RawValue("g_RuntimeMaterialV2VectorCount"',
        'Bind_RawValue("g_RuntimeMaterialV2InputCount"',
        'Bind_RawValue("g_RuntimeMaterialV2InputConsumedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2InputSuppressedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2StaticInputCount"',
        'Bind_RawValue("g_RuntimeMaterialV2StaticSelectedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2StaticConsumedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2StaticSuppressedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2RenderInputCount"',
        'Bind_RawValue("g_RuntimeMaterialV2RenderConsumedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2RenderSuppressedMask"',
        'Bind_RawValue("g_RuntimeMaterialV2ScalarBlocks"',
        'g_RuntimeMaterialV2VectorComponentConsumedMask',
        'g_RuntimeMaterialV2VectorComponentSuppressedMask',
        'Bind_RawValue("g_RuntimeMaterialV2Vectors"',
        'Bind_Texture("g_SourceTexture0"',
        'Bind_Texture("g_SourceTexture1"',
        'Bind_Texture("g_SourceTexture2"',
        'Bind_Texture("g_SourceTexture3"',
        'Bind_RawValue("g_EffectDynamicParameter"',
        '&Particle.vDynamicParameter',
        'Particle.vDynamicParameter,',
        'PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());',
        'SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(',
        'const bool_t bMainSourceReplay =',
        '"Mesh source-replay dynamic payload is missing."',
        'if (Element.Color.vColorMultiply.w <= 0.f)',
        '4u == pResource->iRuntimeMaterialV2Opcode',
        'if (Color.w <= 0.f || Dynamic.x <= 0.f)')) {
        if (-not $renderer.Contains($token)) {
            throw "Artist F Runtime Material v2 execution token missing: $token"
        }
    }
    if (([regex]::Matches($renderer,
            [regex]::Escape('m_pContext->PSGetSamplers(5u,'))).Count -ne 2 -or
        ([regex]::Matches($renderer,
            [regex]::Escape('m_pContext->PSSetSamplers(5u,'))).Count -ne 2) {
        throw 'Artist F Runtime Material v2 sampler slot-5 apply/readback/restore contract changed.'
    }
    $sourceTextureProjection = Get-RequiredTextSlice $renderer `
        'BindFailed(pShader->Bind_Texture("g_SourceTexture0"' `
        'return bBindFailed ? Fail_RenderOperation(' 0 `
        'Runtime Material v2 source-texture bind projection'
    $meshRenderProjection = Get-RequiredTextSlice $renderer `
        'HRESULT Client::CEffectDocumentRenderer::Render_Mesh(' `
        'HRESULT Client::CEffectDocumentRenderer::Render_Decal(' 0 `
        'Runtime Material v2 Mesh transport projection'
    $particleRenderProjection = Get-RequiredTextSlice $renderer `
        'HRESULT Client::CEffectDocumentRenderer::Render_Particles(' `
        'HRESULT Client::CEffectDocumentRenderer::Render_Trails(' 0 `
        'Runtime Material v2 Particle transport projection'
    $samplerProjection = Get-RequiredTextSlice $renderer `
        "`tclass PIXEL_SHADER_SAMPLER_SCOPE final" `
        "`tClient::EFFECT_GPU_RENDER_FAMILY Resolve_GpuRenderFamily" 0 `
        'Runtime Material v2 sampler scope projection'
    $runtimeBindProjection = Get-RequiredTextSlice $renderer `
        "`tif (pShader == m_pMeshShader || pShader == m_pParticleShader)" `
        "`tconst EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial" 0 `
        'Runtime Material v2 raw bind projection'
    $transportProjectionShas = [ordered]@{
        sourceTexture = @($sourceTextureProjection.Text,
            'ec09df4fe79ee3aa5df24263d10f2ae43c250a5479682f18cd0cc51855ce82aa')
		mesh = @($meshRenderProjection.Text,
			'86c4623f3db6aa47b65c737f26a87ac7bc9c5f8cfcad1900485a14ae17eba7a6')
		particles = @($particleRenderProjection.Text,
			'872580c21d12cc1ab0ab8f716ab2f3769e1245572d6eb1912c52686865dbf726')
		sampler = @($samplerProjection.Text,
			'df7b8336a532d96905a0321a7dc46fb8c8426bb18f3ed9e0dff21d6f54ecad3c')
		rawBind = @($runtimeBindProjection.Text,
			'02c3c7dc506b5f1314f7727187094df91286aee666977857c1337ef96d6c2ea5')
    }
    foreach ($projection in $transportProjectionShas.GetEnumerator()) {
        $actualSha = Get-TextSha256 `
            ([string]$projection.Value[0]).Replace("`r`n", "`n")
        if ($actualSha -cne [string]$projection.Value[1]) {
            throw "Artist F Runtime Material v2 $($projection.Key) transport projection changed: $actualSha"
        }
    }
    $stagePrefixProjection = Get-RequiredTextSlice $renderer `
        "`tconst auto StageReconstructedMaterialEvaluator = [&]" `
        "`t`tif (bActive004RuntimeV2)" 0 `
        'Runtime Material v2 stage-prefix wiring projection'
    $stageSuffixProjection = Get-RequiredTextSlice $renderer `
        "`t`tResource.iReconstructedMaterialEvaluatorEnabled = 1u;" `
        "`tfor (const EFFECT_ELEMENT_DESC& Element : Document.Elements)" 0 `
        'Runtime Material v2 stage-suffix wiring projection'
    $stageCallerProjection = Get-RequiredTextSlice $renderer `
        "`t`tbool_t bReconstructedMaterialEvaluatorStaged = false;" `
        "`t`tif (nullptr != pPreparation &&" 0 `
        'Runtime Material v2 stage caller/count projection'
    $denominatorProjection = Get-RequiredTextSlice $renderer `
        "`tif (nullptr == OutPrepared ||" "`tstrOutError.clear();" 0 `
        'Runtime Material v2 prepared denominator projection'
    $headerProjection = Get-RequiredTextSlice $rendererHeader `
        "`t`tuint32_t iRuntimeMaterialV2Enabled = 0u;" `
        "`t`tfloat2_t vReconstructedUVScale" 0 `
        'Runtime Material v2 header ABI/default projection'
    $wiringProjectionShas = [ordered]@{
		stagePrefix = @($stagePrefixProjection.Text,
			'7a1a8f629756334c792a26a41407c7c7a59f0f268984501905b675c6e57bb45c')
        stageSuffix = @($stageSuffixProjection.Text,
            'e93ff2e33336bcb25d45fa77512552011b06d1ad7040d357c4edf262e301c0ae')
        callerCount = @($stageCallerProjection.Text,
            'd6f0f87066add2efd099adfcbb3a5e757176e457b02a538a93327b3e0f08622e')
		denominator = @($denominatorProjection.Text,
			'2fcfd05f2eaa602b9a81333a99f5949e0a0f215d72577ee8446efdc0c15cdcd9')
        headerDefaults = @($headerProjection.Text,
            'a1a29b5a6c00038a4c31ae4cce851e5c1d7f217e000cfed40a90e9a75717face')
    }
    foreach ($projection in $wiringProjectionShas.GetEnumerator()) {
        $actualSha = Get-TextSha256 `
            ([string]$projection.Value[0]).Replace("`r`n", "`n")
        if ($actualSha -cne [string]$projection.Value[1]) {
            throw "Artist F Runtime Material v2 $($projection.Key) wiring projection changed: $actualSha"
        }
    }

	Write-Host ('Artist F Runtime Material v2 focused contract PASS: Catalog ' +
		'Program+sidecar exact, candidate material-subprojection parity; ' +
		'active003/004/009/010/011/016/022/023/030/031 packet contracts; recipe 9/9; occurrences 10/10; ' +
		'unique recipe inputs 264/statics 44/render 54/textures 25; ' +
		'occurrence packet inputs 296/statics 58/render 60/texture lanes 24; ' +
		'dynamic modules 7/named lanes 28/literal rows 28/distribution rows 28; ' +
		'CPU opcode samples 20/20; sidecar neutral providers 4/prepared neutral-base 0; v2 10. ' +
        'GPU pixel/WARP and user eye approval remain separate gates.')
}
finally {
    Pop-Location
}
