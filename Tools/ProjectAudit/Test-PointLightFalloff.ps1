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

function Test-ImportedEnabledLightContract([string]$Source) {
    try {
        $Document = $Source | ConvertFrom-Json
        $EnabledLights = @($Document.elements | Where-Object {
            $_.kind -eq 'light' -and $_.detail.light.enabled -eq $true
        })
        if (2 -ne $EnabledLights.Count) {
            return $false
        }

        $ExpectedIds = @(
            'fx_pc_sdm_09.par_m_flowergarden_light_04.particlespriteemitter_2',
            'fx_pc_sdm_09.par_m_flowergarden_light_04.particlespriteemitter_2.event_source-event-014'
        )
        $ActualIds = @($EnabledLights | ForEach-Object { $_.id } | Sort-Object)
        if (($ActualIds -join "`n") -cne (($ExpectedIds | Sort-Object) -join "`n")) {
            return $false
        }

        foreach ($Light in $EnabledLights) {
            if ([BitConverter]::DoubleToInt64Bits(
                [double]$Light.detail.light.falloffExponent) -ne
                [BitConverter]::DoubleToInt64Bits(1.0)) {
                return $false
            }
        }
        return $true
    }
    catch {
        return $false
    }
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
            '"g_fLightFalloffExponent"[\s\S]{0,160}&LightDesc\.fFalloffExponent' -and
        $Sources.Light -match
            'else if \(LIGHT::POINT == LightDesc\.eType\)[\s\S]{0,160}const uint32_t iApplyDirectionalShadow\s*=\s*0u;[\s\S]{0,180}"g_iApplyDirectionalShadow"'
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
    $EffectHeaderValid =
        $Sources.EffectHeader -match
            'bool_t\s+Try_BuildEffectPointLightDesc\([\s\S]{0,160}const EFFECT_EVALUATED_LIGHT&\s+Evaluated,[\s\S]{0,100}LIGHT_DESC&\s+OutLight\);'
    $EffectSourceValid =
        $Sources.EffectSource -match
            'bool_t Client::Try_BuildEffectPointLightDesc' -and
        $Sources.EffectSource -match
            '!std::isfinite\(Evaluated\.fRange\)[\s\S]{0,80}Evaluated\.fRange\s*<=\s*0\.f' -and
        $Sources.EffectSource -match
            '!std::isfinite\(Evaluated\.fFalloffExponent\)[\s\S]{0,80}Evaluated\.fFalloffExponent\s*<=\s*0\.f' -and
        $Sources.EffectSource -match
            'Staged\.eType\s*=\s*LIGHT::POINT\s*;' -and
        $Sources.EffectSource -match
            'Staged\.vPosition\s*=\s*\{\s*Evaluated\.vWorldPosition\.x,\s*Evaluated\.vWorldPosition\.y,\s*Evaluated\.vWorldPosition\.z,\s*1\.f\s*\};' -and
        $Sources.EffectSource -match
            'Staged\.fRange\s*=\s*Evaluated\.fRange\s*;' -and
        $Sources.EffectSource -match
            'Staged\.fFalloffExponent\s*=\s*Evaluated\.fFalloffExponent\s*;' -and
        $Sources.EffectSource -match
            'Staged\.vDiffuse\s*=\s*\{\s*Evaluated\.vColor\.x\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vColor\.y\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vColor\.z\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vColor\.w\s*\};' -and
        $Sources.EffectSource -match
            'Staged\.vAmbient\s*=\s*\{\s*Evaluated\.vAmbient\.x\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vAmbient\.y\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vAmbient\.z\s*\*\s*Evaluated\.fIntensity,\s*Evaluated\.vAmbient\.w\s*\};' -and
        $Sources.EffectSource -match
            'Staged\.vSpecular\s*=\s*\{\s*0\.f,\s*0\.f,\s*0\.f,\s*0\.f\s*\};' -and
        $Sources.EffectSource -match
            '!IsFinite4\(Staged\.vDiffuse\)[\s\S]{0,100}!IsFinite4\(Staged\.vAmbient\)[\s\S]{0,100}return false;[\s\S]{0,100}OutLight\s*=\s*Staged\s*;[\s\S]{0,60}return true;'
    $EffectObjectValid =
        $Sources.EffectObject -match '#include "Effect_LightPresentation\.h"' -and
        $Sources.EffectObject -match
            'HRESULT Client::CEffectObject::Submit_Presentation\(\)[\s\S]{0,220}Admit_Submit\(GateStatus\)[\s\S]{0,220}return E_FAIL;' -and
        $Sources.EffectObject -match
            'for \(const EFFECT_EVALUATED_LIGHT& Evaluated : Frame\.Lights\)[\s\S]{0,160}Try_BuildEffectPointLightDesc\(Evaluated, Light\)[\s\S]{0,120}Add_TransientLight\(Light\)'
    $ClientProjectValid =
        $Sources.ClientProject -match
            'ClInclude Include="\.\.\\Public\\Effect_LightPresentation\.h"' -and
        $Sources.ClientProject -match
            'ClCompile Include="\.\.\\Private\\Effect_LightPresentation\.cpp"' -and
        $Sources.ClientFilters -match
            'ClInclude Include="\.\.\\Public\\Effect_LightPresentation\.h"[\s\S]{0,120}03\. Tools\\02\. Effect' -and
        $Sources.ClientFilters -match
            'ClCompile Include="\.\.\\Private\\Effect_LightPresentation\.cpp"[\s\S]{0,120}03\. Tools\\02\. Effect'
    $AdmissionValid =
        ([regex]::Matches(
            $Sources.Catalog,
            'Program->Admission\.bRuntimeExecution\s*\|\|\s*Program->Admission\.bProduct').Count -eq 3) -and
        $Sources.Catalog -match
            'CEffectReconstructedRuntimeBoundary::Admit_Submit[\s\S]{0,300}presentation submit is not admitted\.[\s\S]{0,80}return false;' -and
        $Sources.Catalog -match
            'CEffectReconstructedRuntimeBoundary::Admit_Render[\s\S]{0,300}rendering is not admitted\.[\s\S]{0,80}return false;'
    $AuthoredExponentValid =
        $Sources.AuthoringHeader -match
            'struct EFFECT_LIGHT_DETAIL_DESC final[\s\S]{0,500}fFalloffExponent\s*=\s*1\.f\s*;' -and
        $Sources.DocumentCodec -match
            'D\.Light\.bEnabled[\s\S]{0,500}std::isfinite\(D\.Light\.fFalloffExponent\)\s*&&\s*D\.Light\.fFalloffExponent\s*>\s*0\.f' -and
        $Sources.FrontendHarness -match
            'light\.Detail\.Light\.fFalloffExponent[\s\S]{0,120}std::bit_cast<uint32_t>\(1\.f\)[\s\S]{0,120}"Effect Light Detail Default Falloff Is Bit-Exact One"' -and
        $Sources.FrontendHarness -match
            'invalidExponent\.Elements\.front\(\)\.Detail\.Light\.fFalloffExponent\s*=\s*0\.f;[\s\S]{0,160}!CEffectDocumentCodec::Validate\(invalidExponent, status\)' -and
        $Sources.FrontendHarness -match
            'const std::string preservedSerialization[\s\S]{0,300}!CEffectDocumentCodec::Parse\([\s\S]{0,240}CEffectDocumentCodec::Serialize\(preservedAfterFailedParse\)\s*==\s*preservedSerialization[\s\S]{0,100}"Effect Typed Light Invalid Parse Preserves Prior Document"'
    $ImportedGeneratorValid =
        $Sources.ImportedBuilder -match
            'def base_property_name\(value: str\)\s*->\s*str:[\s\S]{0,100}return value\.split\("\[", 1\)\[0\]\.casefold\(\)' -and
        $Sources.ImportedBuilder -match
            'falloff_property_keys\s*=\s*\[\s*key\s+for\s+key\s+in\s+component\.properties\s+if\s+base_property_name\(str\(key\)\)\s*==\s*property_name\s*\]' -and
        $Sources.ImportedBuilder -match
            'if len\(falloff_property_keys\)\s*==\s*0:[\s\S]{0,360}"unresolved_class_default"[\s\S]{0,180}return None' -and
        $Sources.ImportedBuilder -match
            'indexed_falloff_property_keys\s*=\s*\[\s*key\s+for\s+key\s+in\s+falloff_property_keys\s+if\s+"\["\s+in\s+str\(key\)\s*\][\s\S]{0,120}if indexed_falloff_property_keys:[\s\S]{0,220}"indexed property aliases"' -and
        $Sources.ImportedBuilder -match
            'if len\(falloff_property_keys\)\s*>\s*1:[\s\S]{0,220}"case-insensitive property aliases"' -and
        $Sources.ImportedBuilder -match
            'source_property_key\s*=\s*falloff_property_keys\[0\][\s\S]{0,180}value\s*=\s*finite_number\(unwrap\([\s\S]{0,100}component\.properties\[source_property_key\][\s\S]{0,80}\)\)[\s\S]{0,100}if value is None:[\s\S]{0,180}"a finite number"' -and
        $Sources.ImportedBuilder -match
            'if value <= 0\.0:[\s\S]{0,160}"enabled PointLight explicit falloffExponent must be positive"[\s\S]{0,240}"source_explicit"[\s\S]{0,160}return value' -and
        $Sources.ImportedBuilder -match
            'falloff\s*=\s*explicit_falloff_exponent\(\)' -and
        $Sources.ImportedBuilder -match
            '"falloffExponent": falloff if falloff is not None else 1\.0' -and
        $Sources.ImportedBuilderTests -match
            'self\.assertEqual\(light\["detail"\]\["light"\]\["falloffExponent"\], 1\.0\)' -and
        $Sources.ImportedBuilderTests -match
            'invalid_falloff_values\s*=\s*\(\s*False,\s*True,\s*"2\.0",\s*None,\s*float\("nan"\),\s*float\("inf"\),\s*float\("-inf"\),\s*0\.0,\s*-1\.0,\s*\)' -and
        $Sources.ImportedBuilderTests -match
            'for invalid_falloff in invalid_falloff_values:[\s\S]{0,500}invalid_light_component\["properties"\]\["falloffexponent"\]\s*=\s*value\([\s\S]{0,100}"floatproperty", invalid_falloff[\s\S]{0,220}"\(\?:a finite number\|positive\)"' -and
        $Sources.ImportedBuilderTests -cmatch
            'falloff_property_spellings\s*=\s*\(\s*"FalloffExponent",\s*"falloffExponent",\s*"FALLOFFEXPONENT",\s*\)' -and
        $Sources.ImportedBuilderTests -match
            'valid_light_component\["properties"\]\[property_key\]\s*=\s*value\([\s\S]{0,80}"floatproperty", 2\.0[\s\S]{0,500}valid_light\["detail"\]\["light"\]\["falloffExponent"\][\s\S]{0,80}2\.0' -and
        $Sources.ImportedBuilderTests -match
            'invalid_zero_component\["properties"\]\[property_key\]\s*=\s*value\([\s\S]{0,80}"floatproperty", 0\.0[\s\S]{0,260}"enabled PointLight explicit falloffExponent must be positive"' -and
        $Sources.ImportedBuilderTests -cmatch
            'indexed_falloff_property_spellings\s*=\s*\(\s*"falloffexponent\[0\]",\s*"falloffexponent\[1\]",\s*"FalloffExponent\[0\]",\s*"FALLOFFEXPONENT\[1\]",\s*\)' -and
        $Sources.ImportedBuilderTests -match
            'indexed_light_component\["properties"\]\[property_key\]\s*=\s*value\([\s\S]{0,80}"floatproperty", 2\.0[\s\S]{0,260}"scalar falloffExponent must not use indexed property aliases"' -and
        $Sources.ImportedBuilderTests -cmatch
            'canonical_with_indexed_component\["properties"\]\s*\[\s*"falloffexponent"\s*\][\s\S]{0,120}value\("floatproperty", 2\.0\)[\s\S]{0,180}canonical_with_indexed_component\["properties"\]\s*\[\s*"FalloffExponent\[0\]"\s*\][\s\S]{0,120}value\("floatproperty", 0\.0\)[\s\S]{0,240}"scalar falloffExponent must not use indexed property aliases"' -and
        $Sources.ImportedBuilderTests -cmatch
            'duplicate_alias_component\["properties"\]\["falloffexponent"\][\s\S]{0,180}duplicate_alias_component\["properties"\]\["FalloffExponent"\][\s\S]{0,260}"duplicate case-insensitive property aliases"' -and
        $Sources.ImportedBuilderTests -match 'import json' -and
        $Sources.ImportedBuilderTests -match 'json\.loads\(' -and
        $Sources.ImportedBuilderTests -cmatch 'Falloff\\u0045xponent' -and
        $Sources.ImportedBuilderTests -cmatch 'falloff\\u0065xponent' -and
        (Test-ImportedEnabledLightContract $Sources.Imported31930)
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
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_SOURCE_RADIUS_UE\s*=\s*200\.f' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_UE_SCALE\s*=\s*0\.01f' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_RANGE\s*=\s*ARTIST_POINT_LIGHT_SOURCE_RADIUS_UE\s*\*\s*ARTIST_POINT_LIGHT_UE_SCALE' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_INTENSITY\s*=\s*10\.f' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_POSITION_X\s*=\s*1\.25f' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_POSITION_Y\s*=\s*-2\.5f' -and
        $Sources.Harness -match 'ARTIST_POINT_LIGHT_POSITION_Z\s*=\s*3\.75f' -and
        $Sources.Harness -match 'Evaluated\.vColor\s*=\s*\{\s*1\.f,\s*1\.f,\s*1\.f,\s*0\.f\s*\};' -and
        $Sources.Harness -match 'Evaluated\.vAmbient\s*=\s*\{\s*0\.f,\s*0\.f,\s*0\.f,\s*0\.f\s*\};' -and
        $Sources.Harness -match 'LIGHT::POINT\s*==\s*Desc\.eType' -and
        $Sources.Harness -match 'SameBits\(Desc\.vPosition\.x,\s*ARTIST_POINT_LIGHT_POSITION_X\)' -and
        $Sources.Harness -match 'SameBits\(Desc\.vPosition\.y,\s*ARTIST_POINT_LIGHT_POSITION_Y\)' -and
        $Sources.Harness -match 'SameBits\(Desc\.vPosition\.z,\s*ARTIST_POINT_LIGHT_POSITION_Z\)' -and
        $Sources.Harness -match 'SameBits\(Desc\.vPosition\.w,\s*1\.f\)' -and
        $Sources.Harness -match 'SameBits\(Desc\.fRange,\s*2\.f\)' -and
        $Sources.Harness -match 'SameBits\(Desc\.fFalloffExponent,\s*fExpectedFalloffExponent\)' -and
        ([regex]::Matches($Sources.Harness, 'SameBits\(Desc\.vDiffuse\.[xyzw],').Count -eq 4) -and
        ([regex]::Matches($Sources.Harness, 'SameBits\(Desc\.vAmbient\.[xyzw],').Count -eq 4) -and
        ([regex]::Matches($Sources.Harness, 'SameBits\(Desc\.vSpecular\.[xyzw],\s*0\.f\)').Count -eq 4) -and
        $Sources.Harness -match 'Try_BuildEffectPointLightDesc\(EvaluatedTwo, MappedTwo\)' -and
        $Sources.Harness -match 'MatchesArtistPointLightDesc\(MappedTwo,\s*2\.f\)' -and
        $Sources.Harness -match 'MatchesArtistPointLightDesc\([\s\S]{0,120}Presentation\.Get_TransientLights\(\)\.front\(\),\s*2\.f\)' -and
        $Sources.Harness -match 'legacy exponent 1\.0 was not preserved bit-exact' -and
        $Sources.Harness -match 'invalid evaluated exponent changed staged output' -and
        $Sources.Harness -match 'D3D11CreateDevice\(' -and
        $Sources.Harness -match 'CShader::Create' -and
        $Sources.Harness -match 'CLight::Render_Desc\(\s*Reconstructed,\s*Shader,\s*Buffer,\s*true\)' -and
        $Sources.HarnessProject -match 'Engine\\Private\\Light\.cpp' -and
        $Sources.HarnessProject -match 'Engine\\Private\\Light_Manager\.cpp' -and
        $Sources.HarnessProject -match 'Client\\Private\\Effect_LightPresentation\.cpp' -and
        $Sources.HarnessProject -match 'Client\\Public' -and
        $Sources.HarnessProject -match 'd3d11\.lib' -and
        $Sources.HarnessProject -match 'Engine\\Default\\Engine\.vcxproj' -and
        $Sources.HarnessFilters -match 'PointLightFalloffContractHarness\.cpp' -and
        $Sources.HarnessFilters -match 'Client\\Private\\Effect_LightPresentation\.cpp' -and
        $Sources.HarnessRunner -match 'Engine\\Bin\\\$Configuration' -and
        $Sources.HarnessRunner -match 'PointLightFalloffContractHarness\.exe' -and
        $Sources.HarnessRunner -match '& \$executable \$repoRoot' -and
        $Sources.Solution -match 'PointLightFalloffContractHarness'
    Write-Verbose (
        "struct=$StructValid light=$LightValid scene=$SceneValid " +
        "presentation=$PresentationValid effectHeader=$EffectHeaderValid " +
        "effectSource=$EffectSourceValid effectObject=$EffectObjectValid " +
        "clientProject=$ClientProjectValid admission=$AdmissionValid " +
        "authoredExponent=$AuthoredExponentValid " +
        "importedGenerator=$ImportedGeneratorValid shader=$ShaderValid " +
        "harness=$HarnessValid")
    return $StructValid -and $LightValid -and $SceneValid -and
        $PresentationValid -and $EffectHeaderValid -and $EffectSourceValid -and
        $EffectObjectValid -and $ClientProjectValid -and $AdmissionValid -and
        $AuthoredExponentValid -and $ImportedGeneratorValid -and
        $ShaderValid -and $HarnessValid
}

$Sources = @{
    Struct = Read-Utf8Strict 'Engine\Public\Engine_Struct.h'
    Light = Read-Utf8Strict 'Engine\Private\Light.cpp'
    Scene = Read-Utf8Strict 'Engine\Private\Light_Manager.cpp'
    Presentation = Read-Utf8Strict 'Engine\Private\Presentation_Manager.cpp'
    EffectHeader = Read-Utf8Strict `
        'Client\Public\Effect_LightPresentation.h'
    EffectSource = Read-Utf8Strict `
        'Client\Private\Effect_LightPresentation.cpp'
    EffectObject = Read-Utf8Strict 'Client\Private\Effect_Object.cpp'
    ClientProject = Read-Utf8Strict 'Client\Default\Client.vcxproj'
    ClientFilters = Read-Utf8Strict 'Client\Default\Client.vcxproj.filters'
    Catalog = Read-Utf8Strict 'Client\Private\Effect_Catalog.cpp'
    AuthoringHeader = Read-Utf8Strict `
        'Client\Public\Effect_AuthoringDocument.h'
    DocumentCodec = Read-Utf8Strict `
        'Client\Private\Effect_DocumentCodec.cpp'
    FrontendHarness = Read-Utf8Strict `
        'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'
    ImportedBuilder = Read-Utf8Strict `
        'Tools\LevelPlacementExtractor\build_imported_effect_documents.py'
    ImportedBuilderTests = Read-Utf8Strict `
        'Tools\LevelPlacementExtractor\test_build_imported_effect_documents.py'
    Imported31930 = Read-Utf8Strict `
        'Data\Effects\Imported\Artist\CurrentCombat\Converted\effect.artist.skill.31930.imported.effect.json'
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
        Name = 'cpp-point-shadow-disabled'
        Key = 'Light'
        Pattern = 'const uint32_t iApplyDirectionalShadow\s*=\s*0u;'
        Replacement = 'const uint32_t iApplyDirectionalShadow = 1u;'
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
    },
    @{
        Name = 'effect-exponent-copy'
        Key = 'EffectSource'
        Pattern = 'Staged\.fFalloffExponent\s*=\s*Evaluated\.fFalloffExponent\s*;'
        Replacement = 'Staged.fFalloffExponent = 1.f;'
    },
    @{
        Name = 'effect-position-copy'
        Key = 'EffectSource'
        Pattern = 'Staged\.vPosition\s*=\s*\{\s*Evaluated\.vWorldPosition\.x,'
        Replacement = 'Staged.vPosition = { Evaluated.vWorldPosition.y,'
    },
    @{
        Name = 'effect-range-copy'
        Key = 'EffectSource'
        Pattern = 'Staged\.fRange\s*=\s*Evaluated\.fRange\s*;'
        Replacement = 'Staged.fRange = 1.f;'
    },
    @{
        Name = 'effect-color-copy'
        Key = 'EffectSource'
        Pattern = 'Evaluated\.vColor\.x\s*\*\s*Evaluated\.fIntensity,'
        Replacement = 'Evaluated.vColor.y * Evaluated.fIntensity,'
    },
    @{
        Name = 'effect-ambient-copy'
        Key = 'EffectSource'
        Pattern = 'Evaluated\.vAmbient\.x\s*\*\s*Evaluated\.fIntensity,'
        Replacement = 'Evaluated.vAmbient.y * Evaluated.fIntensity,'
    },
    @{
        Name = 'effect-specular-zero'
        Key = 'EffectSource'
        Pattern = 'Staged\.vSpecular\s*=\s*\{\s*0\.f,\s*0\.f,\s*0\.f,\s*0\.f\s*\};'
        Replacement = 'Staged.vSpecular = { 1.f, 0.f, 0.f, 0.f };'
    },
    @{
        Name = 'effect-exponent-finite-validation'
        Key = 'EffectSource'
        Pattern = '!std::isfinite\(Evaluated\.fFalloffExponent\)'
        Replacement = 'false'
    },
    @{
        Name = 'effect-output-commit'
        Key = 'EffectSource'
        Pattern = 'OutLight\s*=\s*Staged\s*;'
        Replacement = 'OutLight = LIGHT_DESC{};'
    },
    @{
        Name = 'effect-object-delegation'
        Key = 'EffectObject'
        Pattern = 'Try_BuildEffectPointLightDesc\(Evaluated, Light\)'
        Replacement = 'true'
    },
    @{
        Name = 'compiled-effect-shader-path'
        Key = 'Harness'
        Pattern = 'D3D11CreateDevice\('
        Replacement = 'D3D11CreateDevice_Mutated('
    },
    @{
        Name = 'compiled-effect-source-registration'
        Key = 'HarnessProject'
        Pattern = 'Client\\Private\\Effect_LightPresentation\.cpp'
        Replacement = 'Client\Private\Effect_LightPresentation_Mutated.cpp'
    },
    @{
        Name = 'reconstructed-admission-remains-false'
        Key = 'Catalog'
        Pattern = 'Program->Admission\.bRuntimeExecution\s*\|\|\s*Program->Admission\.bProduct'
        Replacement = 'false'
    },
    @{
        Name = 'authored-light-default-one'
        Key = 'AuthoringHeader'
        Pattern = 'fFalloffExponent\s*=\s*1\.f\s*;'
        Replacement = 'fFalloffExponent = 0.f;'
    },
    @{
        Name = 'authored-enabled-light-positive-validation'
        Key = 'DocumentCodec'
        Pattern = 'D\.Light\.fFalloffExponent\s*>\s*0\.f'
        Replacement = 'D.Light.fFalloffExponent >= 0.f'
    },
    @{
        Name = 'authored-default-compiled-assertion'
        Key = 'FrontendHarness'
        Pattern = 'std::bit_cast<uint32_t>\(1\.f\)'
        Replacement = 'std::bit_cast<uint32_t>(0.f)'
    },
    @{
        Name = 'authored-invalid-parse-rollback-assertion'
        Key = 'FrontendHarness'
        Pattern = 'CEffectDocumentCodec::Serialize\(preservedAfterFailedParse\)\s*==\s*preservedSerialization'
        Replacement = 'CEffectDocumentCodec::Serialize(preservedAfterFailedParse) != preservedSerialization'
    },
    @{
        Name = 'imported-explicit-zero-rejection'
        Key = 'ImportedBuilder'
        Pattern = 'if value <= 0\.0:'
        Replacement = 'if value < 0.0:'
    },
    @{
        Name = 'imported-semantic-base-helper'
        Key = 'ImportedBuilder'
        Pattern = 'return value\.split\("\[", 1\)\[0\]\.casefold\(\)'
        Replacement = 'return value.casefold()'
    },
    @{
        Name = 'imported-semantic-base-key-match'
        Key = 'ImportedBuilder'
        Pattern = 'base_property_name\(str\(key\)\)\s*==\s*property_name'
        Replacement = 'str(key).casefold() == property_name'
    },
    @{
        Name = 'imported-property-presence-predicate'
        Key = 'ImportedBuilder'
        Pattern = 'if len\(falloff_property_keys\)\s*==\s*0:'
        Replacement = 'if False:'
    },
    @{
        Name = 'imported-indexed-key-collection'
        Key = 'ImportedBuilder'
        Pattern = 'if\s+"\["\s+in\s+str\(key\)'
        Replacement = 'if False'
    },
    @{
        Name = 'imported-indexed-scalar-rejection'
        Key = 'ImportedBuilder'
        Pattern = 'if indexed_falloff_property_keys:'
        Replacement = 'if False:'
    },
    @{
        Name = 'imported-case-alias-duplicate-rejection'
        Key = 'ImportedBuilder'
        Pattern = 'if len\(falloff_property_keys\)\s*>\s*1:'
        Replacement = 'if False:'
    },
    @{
        Name = 'imported-invalid-finite-rejection'
        Key = 'ImportedBuilder'
        Pattern = 'value\s*=\s*finite_number\(unwrap\(\s*component\.properties\[source_property_key\]\s*\)\)\s*if value is None:'
        Replacement = "value = finite_number(unwrap(`n            component.properties[source_property_key]`n        ))`n        if False:"
    },
    @{
        Name = 'imported-invalid-value-matrix-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'invalid_falloff_values\s*=\s*\(\s*False,'
        Replacement = 'invalid_falloff_values = ('
    },
    @{
        Name = 'imported-case-spelling-matrix-test'
        Key = 'ImportedBuilderTests'
        Pattern = '"FalloffExponent",\s*"falloffExponent",\s*"FALLOFFEXPONENT",'
        Replacement = '"falloffexponent",'
    },
    @{
        Name = 'imported-explicit-two-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'valid_light_component\["properties"\]\[property_key\]\s*=\s*value\('
        Replacement = 'valid_light_component["properties"][property_key] = missing_value('
    },
    @{
        Name = 'imported-indexed-case-matrix-test'
        Key = 'ImportedBuilderTests'
        Pattern = '"falloffexponent\[0\]",\s*"falloffexponent\[1\]",\s*"FalloffExponent\[0\]",\s*"FALLOFFEXPONENT\[1\]",'
        Replacement = '"unrelated[0]",'
    },
    @{
        Name = 'imported-canonical-plus-indexed-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'canonical_with_indexed_component\["properties"\]\s*\[\s*"FalloffExponent\[0\]"\s*\]'
        Replacement = 'canonical_with_indexed_component["properties"]["Unrelated[0]"]'
    },
    @{
        Name = 'imported-case-alias-duplicate-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'duplicate_alias_component\["properties"\]\["FalloffExponent"\]\s*=\s*value\('
        Replacement = 'duplicate_alias_component["properties"]["falloffexponent"] = value('
    },
    @{
        Name = 'imported-escaped-upper-alias-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'Falloff\\u0045xponent'
        Replacement = 'FalloffExponent'
    },
    @{
        Name = 'imported-escaped-lower-alias-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'falloff\\u0065xponent'
        Replacement = 'falloffexponent'
    },
    @{
        Name = 'imported-missing-default-one'
        Key = 'ImportedBuilder'
        Pattern = '"falloffExponent": falloff if falloff is not None else 1\.0'
        Replacement = '"falloffExponent": falloff if falloff is not None else 0.0'
    },
    @{
        Name = 'imported-generator-default-test'
        Key = 'ImportedBuilderTests'
        Pattern = 'self\.assertEqual\(light\["detail"\]\["light"\]\["falloffExponent"\], 1\.0\)'
        Replacement = 'self.assertEqual(light["detail"]["light"]["falloffExponent"], 0.0)'
    },
    @{
        Name = 'imported-31930-enabled-row'
        Key = 'Imported31930'
        Pattern = '"falloffExponent": 1\.0'
        Replacement = '"falloffExponent": 0.0'
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
