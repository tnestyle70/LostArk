[CmdletBinding()]
param(
    [string]$RepoRoot = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}

function Read-Utf8Strict([string]$RelativePath) {
    return [IO.File]::ReadAllText(
        (Join-Path $RepoRoot $RelativePath),
        [Text.UTF8Encoding]::new($false, $true))
}

function Assert-Condition(
    [bool]$Condition,
    [string]$Message) {
    if (-not $Condition) {
        throw $Message
    }
}

function Test-FalloffExponent([double]$Exponent) {
    return -not [double]::IsNaN($Exponent) -and
        -not [double]::IsInfinity($Exponent) -and
        $Exponent -gt 0.0
}

function Test-PositiveFiniteRange([double]$Range) {
    return -not [double]::IsNaN($Range) -and
        -not [double]::IsInfinity($Range) -and
        $Range -gt 0.0
}

function Get-LegacyPointAttenuation(
    [double]$Range,
    [double]$Distance) {
    $Linear = ($Range - $Distance) / $Range
    return [Math]::Max(0.0, [Math]::Min(1.0, $Linear))
}

function Get-PointAttenuation(
    [double]$Range,
    [double]$Distance,
    [double]$Exponent) {
    if (-not (Test-FalloffExponent $Exponent)) {
        throw 'PointLight falloff exponent must be finite and positive.'
    }

    if (-not (Test-PositiveFiniteRange $Range)) {
        return 0.0
    }

    $Linear = Get-LegacyPointAttenuation $Range $Distance
    if ($Exponent -eq 1.0) {
        return $Linear
    }
    return [Math]::Pow($Linear, $Exponent)
}

function Test-SourceContract([hashtable]$Sources) {
    $StructValid =
        $Sources.Struct -match 'fFalloffExponent\s*=\s*1\.f\s*;' -and
        $Sources.Struct -match 'static_assert\(sizeof\(LIGHT_DESC\)\s*==\s*92u\);' -and
        $Sources.Struct -match 'static_assert\(offsetof\(LIGHT_DESC,\s*fRange\)\s*==\s*36u\);' -and
        $Sources.Struct -match 'static_assert\(offsetof\(LIGHT_DESC,\s*fFalloffExponent\)\s*==\s*40u\);' -and
        $Sources.Struct -match 'static_assert\(offsetof\(LIGHT_DESC,\s*vDiffuse\)\s*==\s*44u\);'
    $LightValid =
        $Sources.Light -match 'IsValidLightAttenuation' -and
        $Sources.Light -match
            'std::isfinite\(LightDesc\.fFalloffExponent\)' -and
        $Sources.Light -match
            'LightDesc\.fFalloffExponent\s*<=\s*0\.f' -and
        $Sources.Light -match
            'LIGHT::POINT\s*!=\s*LightDesc\.eType\s*\|\|[\s\S]{0,120}std::isfinite\(LightDesc\.fRange\)[\s\S]{0,80}LightDesc\.fRange\s*>\s*0\.f' -and
        $Sources.Light -match
            'HRESULT CLight::Initialize[\s\S]{0,180}!IsValidLightAttenuation\(LightDesc\)[\s\S]{0,80}return E_INVALIDARG;[\s\S]{0,80}m_LightDesc\s*=\s*LightDesc' -and
        $Sources.Light -match
            'HRESULT CLight::Render_Desc[\s\S]{0,260}!IsValidLightAttenuation\(LightDesc\)[\s\S]{0,80}return E_INVALIDARG;[\s\S]{0,500}Bind_RawValue' -and
        $Sources.Light -match
            'unique_ptr<CLight> CLight::Create[\s\S]{0,180}!IsValidLightAttenuation\(LightDesc\)[\s\S]{0,80}return nullptr;[\s\S]{0,120}new CLight' -and
        $Sources.Light -match
            '"g_fLightFalloffExponent"[\s\S]{0,160}&LightDesc\.fFalloffExponent'
    $SceneValid =
        $Sources.Scene -match
            '!std::isfinite\(Light\.fFalloffExponent\)[\s\S]{0,100}Light\.fFalloffExponent\s*<=\s*0\.f' -and
        $Sources.Scene -match
            'std::isfinite\(Light\.fRange\)[\s\S]{0,80}Light\.fRange\s*>\s*0\.f' -and
        $Sources.Scene -match
            'for \(const LIGHT_DESC& LightDesc : SceneLights\)[\s\S]{0,220}!IsValidSceneLight\(LightDesc\)[\s\S]{0,100}return E_INVALIDARG;[\s\S]{0,100}m_SceneLights\.swap\(SceneLights\)'
    $PresentationValid =
        $Sources.Presentation -match
            '!std::isfinite\(LightDesc\.fFalloffExponent\)[\s\S]{0,100}LightDesc\.fFalloffExponent\s*<=\s*0\.f' -and
        $Sources.Presentation -match
            'LightDesc\.fFalloffExponent\s*<=\s*0\.f[\s\S]{0,400}return E_FAIL;[\s\S]{0,120}m_TransientLights\.push_back\(LightDesc\)'
    $ShaderValid =
        $Sources.Shader -match
            'float\s+g_fLightFalloffExponent\s*;' -and
        $Sources.Shader -match
            'float\s+Resolve_PointLightAttenuation\(float fDistance\)' -and
        $Sources.Shader -match
            'float fAttenuation\s*=\s*0\.f;[\s\S]{0,100}isfinite\(g_fLightRange\)\s*&&\s*g_fLightRange\s*>\s*0\.f' -and
        $Sources.Shader -match
            'fAttenuation\s*=\s*saturate\(\s*\(g_fLightRange - fDistance\) / g_fLightRange\);' -and
        $Sources.Shader -match
            'if \(1\.f != g_fLightFalloffExponent\)[\s\S]{0,100}fAttenuation\s*=\s*pow\(\s*fAttenuation, g_fLightFalloffExponent\);[\s\S]{0,100}return fAttenuation;' -and
        $Sources.Shader -match
            'float fAtt\s*=\s*Resolve_PointLightAttenuation\(length\(vLightDir\)\);' -and
        $Sources.Shader -match
            'Out\.vShade\s*=[^;]{0,400}\*\s*fAtt\s*;' -and
        $Sources.Shader -match
            'Out\.vSpecular\s*=[^;]{0,400}\*\s*fAtt\s*;'
    $HarnessValid =
        $Sources.Harness -match 'static_assert\(sizeof\(LIGHT_DESC\)\s*==\s*92u\);' -and
        $Sources.Harness -match 'static_assert\(offsetof\(LIGHT_DESC,\s*fRange\)\s*==\s*36u\);' -and
        $Sources.Harness -match 'static_assert\(offsetof\(LIGHT_DESC,\s*fFalloffExponent\)\s*==\s*40u\);' -and
        $Sources.Harness -match 'DefaultDesc\.fFalloffExponent[\s\S]{0,160}1\.f' -and
        $Sources.Harness -match 'nullptr\s*!=\s*CLight::Create\(Invalid\)' -and
        $Sources.Harness -match 'E_INVALIDARG\s*!=\s*Light->Initialize\(Invalid\)' -and
        $Sources.Harness -match 'E_INVALIDARG\s*!=\s*CLight::Render_Desc\(Invalid,\s*nullptr,\s*nullptr\)' -and
        $Sources.Harness -match 'scene invalid range changed prior state' -and
        ([regex]::Matches(
            $Sources.Harness,
            '2u\s*!=\s*LightManager->Get_SceneLightCount\(\)').Count -ge 2) -and
        $Sources.Harness -match 'transient invalid range pushed or changed prior state' -and
        $Sources.Harness -match '1u\s*!=\s*Presentation\.Get_TransientLights\(\)\.size\(\)' -and
        $Sources.HarnessProject -match 'Engine\\Private\\Light\.cpp' -and
        $Sources.HarnessProject -match 'Engine\\Private\\Light_Manager\.cpp' -and
        $Sources.HarnessProject -match 'Engine\\Default\\Engine\.vcxproj' -and
        $Sources.HarnessFilters -match 'PointLightFalloffContractHarness\.cpp' -and
        $Sources.HarnessRunner -match 'Engine\\Bin\\\$Configuration' -and
        $Sources.HarnessRunner -match 'PointLightFalloffContractHarness\.exe' -and
        $Sources.Solution -match 'PointLightFalloffContractHarness'
    Write-Verbose (
        "struct=$StructValid light=$LightValid scene=$SceneValid " +
        "presentation=$PresentationValid shader=$ShaderValid harness=$HarnessValid")
    return $StructValid -and $LightValid -and $SceneValid -and
        $PresentationValid -and $ShaderValid -and $HarnessValid
}

$Sources = @{
    Struct = Read-Utf8Strict 'Engine\Public\Engine_Struct.h'
    Light = Read-Utf8Strict 'Engine\Private\Light.cpp'
    Scene = Read-Utf8Strict 'Engine\Private\Light_Manager.cpp'
    Presentation = Read-Utf8Strict 'Engine\Private\Presentation_Manager.cpp'
    Shader = Read-Utf8Strict 'Engine\Bin\ShaderFiles\Shader_Deferred.hlsl'
    Harness = Read-Utf8Strict `
        'Tools\PointLightFalloffContractHarness\Private\PointLightFalloffContractHarness.cpp'
    HarnessProject = Read-Utf8Strict `
        'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
    HarnessFilters = Read-Utf8Strict `
        'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj.filters'
    HarnessRunner = Read-Utf8Strict `
        'Tools\PointLightFalloffContractHarness\Run-PointLightFalloffContractHarness.ps1'
    Solution = Read-Utf8Strict 'Framework.sln'
}
$ClientShader = Read-Utf8Strict 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'

Assert-Condition (Test-SourceContract $Sources) `
    'PointLight falloff struct, validation, binding, or shader contract is incomplete.'
Assert-Condition ($Sources.Shader -ceq $ClientShader) `
    'Engine and Client deferred shaders must be byte-equivalent UTF-8 text.'

$Range = 10.0
$Distances = @(0.0, 1.25, 5.0, 9.5, 10.0, 12.0)
foreach ($Distance in $Distances) {
    $Legacy = Get-LegacyPointAttenuation $Range $Distance
    $Default = Get-PointAttenuation $Range $Distance 1.0
    Assert-Condition (
        [BitConverter]::DoubleToInt64Bits($Legacy) -eq
        [BitConverter]::DoubleToInt64Bits($Default)) `
        "Default exponent changed legacy attenuation at distance $Distance."
}

$LinearInterior = Get-PointAttenuation $Range 5.0 1.0
$SquaredInterior = Get-PointAttenuation $Range 5.0 2.0
$RootInterior = Get-PointAttenuation $Range 5.0 0.5
Assert-Condition ($SquaredInterior -lt $LinearInterior) `
    'Exponent 2.0 must reduce attenuation at an interior distance.'
Assert-Condition ($RootInterior -gt $LinearInterior) `
    'Exponent 0.5 must increase attenuation at an interior distance.'

foreach ($InvalidExponent in @(
    [double]::NaN,
    [double]::PositiveInfinity,
    [double]::NegativeInfinity,
    0.0,
    -1.0)) {
    Assert-Condition (-not (Test-FalloffExponent $InvalidExponent)) `
        'A nonfinite or nonpositive exponent was accepted.'
}

$InvalidRanges = @(
    0.0,
    -1.0,
    [double]::NaN,
    [double]::PositiveInfinity,
    [double]::NegativeInfinity)
foreach ($InvalidRange in $InvalidRanges) {
    Assert-Condition (-not (Test-PositiveFiniteRange $InvalidRange)) `
        'A nonfinite or nonpositive point-light range was accepted.'
    $Resolved = Get-PointAttenuation $InvalidRange 1.0 1.0
    Assert-Condition (
        [BitConverter]::DoubleToInt64Bits($Resolved) -eq
        [BitConverter]::DoubleToInt64Bits(0.0)) `
        'An invalid point-light range did not resolve to zero attenuation.'
}

$Mutations = @(
    @{
        Name = 'struct-default'
        Key = 'Struct'
        Pattern = 'fFalloffExponent\s*=\s*1\.f\s*;'
        Replacement = 'fFalloffExponent = 2.f;'
    },
    @{
        Name = 'cpp-shader-binding'
        Key = 'Light'
        Pattern = '"g_fLightFalloffExponent"'
        Replacement = '"g_fLightFalloffExponent_Mutated"'
    },
    @{
        Name = 'cpp-final-range-validation'
        Key = 'Light'
        Pattern = 'std::isfinite\(LightDesc\.fRange\)\s*&&\s*LightDesc\.fRange\s*>\s*0\.f'
        Replacement = 'true'
    },
    @{
        Name = 'scene-finite-validation'
        Key = 'Scene'
        Pattern = '!std::isfinite\(Light\.fFalloffExponent\)'
        Replacement = 'false'
    },
    @{
        Name = 'scene-positive-range-validation'
        Key = 'Scene'
        Pattern = 'Light\.fRange\s*>\s*0\.f'
        Replacement = 'Light.fRange >= 0.f'
    },
    @{
        Name = 'transient-finite-validation'
        Key = 'Presentation'
        Pattern = '!std::isfinite\(LightDesc\.fFalloffExponent\)'
        Replacement = 'false'
    },
    @{
        Name = 'shader-uniform'
        Key = 'Shader'
        Pattern = 'float\s+g_fLightFalloffExponent\s*;'
        Replacement = 'float g_fLightFalloffExponent_Mutated;'
    },
    @{
        Name = 'shader-exponent-application'
        Key = 'Shader'
        Pattern = 'fAttenuation\s*=\s*pow\(\s*fAttenuation, g_fLightFalloffExponent\);'
        Replacement = 'fAttenuation = fAttenuation;'
    },
    @{
        Name = 'shader-range-guard'
        Key = 'Shader'
        Pattern = 'isfinite\(g_fLightRange\)\s*&&\s*g_fLightRange\s*>\s*0\.f'
        Replacement = 'true'
    },
    @{
        Name = 'shader-helper-result-discard'
        Key = 'Shader'
        Pattern = 'float fAtt\s*=\s*Resolve_PointLightAttenuation\(length\(vLightDir\)\);'
        Replacement = "Resolve_PointLightAttenuation(length(vLightDir));`n    float fAtt = 1.f;"
    },
    @{
        Name = 'shader-point-diffuse-att-consumption'
        Key = 'Shader'
        Pattern = 'Out\.vShade\s*=[^;]{0,400}\*\s*fAtt\s*;'
        Replacement = 'Out.vShade = 1.f;'
    },
    @{
        Name = 'shader-point-specular-att-consumption'
        Key = 'Shader'
        Pattern = 'Out\.vSpecular\s*=[^;]{0,400}\*\s*fAtt\s*;'
        Replacement = 'Out.vSpecular = 1.f;'
    },
    @{
        Name = 'compiled-scene-rollback-assertion'
        Key = 'Harness'
        Pattern = '2u\s*!=\s*LightManager->Get_SceneLightCount\(\)'
        Replacement = '2u == LightManager->Get_SceneLightCount()'
    })

foreach ($Mutation in $Mutations) {
    $Mutated = @{}
    foreach ($Key in $Sources.Keys) {
        $Mutated[$Key] = $Sources[$Key]
    }
    $Original = [string]$Mutated[$Mutation.Key]
    $MutationRegex = [regex]::new([string]$Mutation.Pattern)
    $Changed = $MutationRegex.Replace(
        $Original, [string]$Mutation.Replacement, 1)
    Assert-Condition ($Changed -cne $Original) `
        "Mutation $($Mutation.Name) did not reach its target."
    $Mutated[$Mutation.Key] = $Changed
    Assert-Condition (-not (Test-SourceContract $Mutated)) `
        "Mutation $($Mutation.Name) escaped the focused audit."
}

$Summary =
    "PointLight falloff exponent audit PASS; " +
    "legacyDefaultSamples=$($Distances.Count); " +
    'exponents=0.5/1.0/2.0; ' +
    'invalidExponents=5; ' +
    "invalidRanges=$($InvalidRanges.Count); " +
    "mutations=$($Mutations.Count); " +
    'engineClientShaderEqual=true'
Write-Output $Summary
