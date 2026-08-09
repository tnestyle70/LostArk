[CmdletBinding()]
param(
    [string]$RepoRoot = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
}

function Read-Utf8([string]$RelativePath) {
    return [IO.File]::ReadAllText(
        (Join-Path $RepoRoot $RelativePath),
        [Text.UTF8Encoding]::new($false, $true))
}

function Read-Cp949([string]$RelativePath) {
    return [IO.File]::ReadAllText(
        (Join-Path $RepoRoot $RelativePath),
        [Text.Encoding]::GetEncoding(949))
}

function Assert-Match(
    [string]$Text,
    [string]$Pattern,
    [string]$Message) {
    if ($Text -notmatch $Pattern) {
        throw $Message
    }
}

function Assert-NoMatch(
    [string]$Text,
    [string]$Pattern,
    [string]$Message) {
    if ($Text -match $Pattern) {
        throw $Message
    }
}

$engineStruct = Read-Utf8 'Engine\Public\Engine_Struct.h'
$engineEnum = Read-Cp949 'Engine\Public\Engine_Enum.h'
$renderer = Read-Cp949 'Engine\Private\Renderer.cpp'
$rendererHeader = Read-Cp949 'Engine\Public\Renderer.h'
$gameInstance = Read-Cp949 'Engine\Private\GameInstance.cpp'
$shader = Read-Utf8 'Engine\Bin\ShaderFiles\Shader_Deferred.hlsl'
$mainApp = Read-Utf8 'Client\Private\MainApp.cpp'
$mainAppHeader = Read-Utf8 'Client\Public\MainApp.h'
$renderingProfiles = Read-Utf8 'Client\Private\RenderingProfileService.cpp'
$levelRegistry = Read-Utf8 'Client\Private\LevelRegistry.cpp'
$characterSelect = Read-Utf8 'Client\Private\Level_CharacterSelect.cpp'
$animatedShader = Read-Utf8 'Client\Bin\ShaderFiles\Shader_VtxAnimMeshBinary.hlsl'
$staticShader = Read-Utf8 'Client\Bin\ShaderFiles\Shader_VtxMeshBinary.hlsl'
$materialBinder = Read-Utf8 'Client\Private\DeferredMaterialRenderUtils.cpp'
$characterSpec = Read-Utf8 'Client\Public\CharacterSpec.h'
$dimensionMasterLogic = Read-Utf8 'Client\Private\Logic_DimensionMaster.cpp'
$character = Read-Utf8 'Client\Private\Character.cpp'
$shadow = Read-Utf8 'Engine\Private\Shadow.cpp'
$targetManager = Read-Utf8 'Engine\Private\Target_Manager.cpp'
$lightManager = Read-Utf8 'Engine\Private\Light_Manager.cpp'
$partBody = Read-Utf8 'Client\Private\Part_Body.cpp'
$partEquipment = Read-Utf8 'Client\Private\Part_Equipment.cpp'
$mapAsset = Read-Utf8 'Client\Private\MapAssetObject.cpp'
$mapBatch = Read-Utf8 'Client\Private\MapStaticBatchObject.cpp'
$mapInstanceShader = Read-Utf8 'Client\Bin\ShaderFiles\Shader_VtxMeshMapInstance.hlsl'
$valtanBody = Read-Utf8 'Client\Private\Body_Valtan.cpp'
$valtanBodyHeader = Read-Utf8 'Client\Public\Body_Valtan.h'
$deployProp = Read-Utf8 'Client\Private\DeployPropObject.cpp'
$deployPropHeader = Read-Utf8 'Client\Public\DeployPropObject.h'

Assert-Match $engineStruct 'bBloomEnabled\s*=\s*true' `
    'Render-quality defaults must keep bloom enabled.'
Assert-Match $engineStruct 'fBloomThreshold\s*=\s*1\.f' `
    'Render-quality defaults must preserve threshold 1.0.'
Assert-Match $engineStruct 'fBloomIntensity\s*=\s*0\.8f' `
    'Render-quality defaults must preserve bloom intensity 0.8.'
Assert-Match $engineStruct 'fExposure\s*=\s*2\.f' `
    'Render-quality defaults must preserve exposure 2.0.'
Assert-Match $engineStruct 'fWhitePoint\s*=\s*11\.2f' `
    'Render-quality defaults must preserve Hable white point 11.2.'
Assert-Match $engineStruct 'fGamma\s*=\s*2\.2f' `
    'Render-quality defaults must preserve display gamma 2.2.'
Assert-Match $engineStruct 'bFXAAEnabled\s*=\s*false' `
    'FXAA must remain opt-in for default pixel compatibility.'
Assert-Match $engineStruct 'bSSAOEnabled\s*=\s*true' `
    'SSAO must have an explicit typed default.'
Assert-Match $engineStruct 'fSSAORadius\s*=\s*0\.75f' `
    'SSAO radius must have a finite authored seed.'
Assert-Match $engineStruct 'tagShadowSettings[\s\S]*?bEnabled[\s\S]*?fOrthographicWidth[\s\S]*?fDepthBias[\s\S]*?fNormalBias[\s\S]*?fStrength' `
    'Directional shadow settings must be typed and independently tunable.'
Assert-Match $engineEnum 'SSAO_RAW\s*=\s*12[\s\S]*?SSAO_BLUR\s*=\s*13' `
    'SSAO passes must append after the existing 0-11 deferred pass contract.'

Assert-Match $renderer 'Target_Shade[\s\S]*?DXGI_FORMAT_R16G16B16A16_FLOAT' `
    'Shade accumulation must preserve HDR values in FP16.'
Assert-Match $renderer 'Target_Specular[\s\S]*?DXGI_FORMAT_R16G16B16A16_FLOAT' `
    'Specular accumulation must preserve HDR values in FP16.'
Assert-Match $renderer 'IsValidRenderQualitySettings' `
    'Renderer must validate the full render-quality profile before commit.'
Assert-Match $renderer 'bBloomEnabled\s*&&\s*FAILED\(Render_Bloom\(\)\)' `
    'Bloom disabled must skip the three half-resolution passes.'
Assert-Match $renderer 'g_fToneMapExposure' `
    'Renderer must bind tone-map settings to the final shader.'
Assert-Match $renderer 'Target_SSAORaw[\s\S]*?DXGI_FORMAT_R16_FLOAT' `
    'SSAO raw target must use a finite half-resolution single-channel format.'
Assert-Match $renderer 'Target_SSAOBlur[\s\S]*?DXGI_FORMAT_R16_FLOAT' `
    'SSAO bilateral target must use the same single-channel format.'
Assert-Match $renderer 'Render_NonBlend\(\)[\s\S]*?Render_SSAO\(\)[\s\S]*?Render_Lights\(\)' `
    'SSAO must execute after GBuffer production and before light accumulation.'
Assert-Match $renderer 'bSSAOEnabled\s*&&\s*FAILED\(Render_SSAO\(\)\)' `
    'SSAO disabled must skip its raw and bilateral passes.'
Assert-Match $rendererHeader 'Apply_RenderQualitySettings' `
    'Renderer must expose a typed atomic apply boundary.'
Assert-Match $renderer 'DXGI_FORMAT_R32_TYPELESS[\s\S]*?DXGI_FORMAT_D32_FLOAT[\s\S]*?DXGI_FORMAT_R32_FLOAT' `
    'Shadow storage must be one typeless texture with true depth and read-only views.'
Assert-Match $rendererHeader 'm_iShadowMapSize\s*=\s*2048u' `
    'Stage0 directional shadow resolution must stay square and bounded.'
Assert-NoMatch $renderer 'Target_LightDepth|MRT_ShadowObject' `
    'Legacy viewport-sized RGBA light-depth MRT must not return.'
Assert-Match $gameInstance 'm_pRenderer->Apply_RenderQualitySettings' `
    'Client tools must reach settings through CGameInstance, not Renderer ownership.'
Assert-Match $targetManager 'Begin_DepthOnly[\s\S]*?D3D11_CLEAR_DEPTH[\s\S]*?OMSetRenderTargets\(0, nullptr' `
    'Shadow production must use a balanced depth-only output scope.'
Assert-Match $shadow 'XMMatrixOrthographicLH' `
    'Directional shadows must use a validated orthographic projection.'

Assert-Match $shader 'float3\s+Resolve_FinalFXAA' `
    'The final shader must contain an actual FXAA edge evaluation path.'
Assert-Match $shader 'fLumaRange\s*<\s*fRequiredRange' `
    'FXAA must use local contrast early-out rather than unconditional blur.'
Assert-Match $shader 'g_fBloomThreshold' `
    'Bloom threshold must be a runtime uniform.'
Assert-Match $shader 'g_fBloomScatter' `
    'Bloom scatter must affect the blur sample step.'
Assert-Match $shader 'g_iBloomEnabled' `
    'Final resolve must ignore stale bloom when bloom is disabled.'
Assert-Match $shader 'PS_MAIN_SSAO_RAW' `
    'Deferred shader must contain an actual SSAO estimate pass.'
Assert-Match $shader 'PS_MAIN_SSAO_BLUR' `
    'Deferred shader must contain the depth/normal-aware SSAO resolve pass.'
Assert-Match $shader 'Decode_SSAOViewNormal[\s\S]*?g_CameraViewMatrix' `
    'SSAO must transform decoded normals into the camera basis.'
Assert-Match $shader 'Reconstruct_SSAOViewPosition[\s\S]*?g_ProjMatrixInverse' `
    'SSAO must reconstruct view-space positions from the depth buffer.'
Assert-Match $shader 'dot\(vCenterNormal, vCenterToSample\)[\s\S]*?fHemisphereOcclusion' `
    'SSAO raw sampling must evaluate the center-normal hemisphere instead of rejecting corner normals.'
Assert-Match $shader 'Resolve_AmbientOcclusion[\s\S]*?fAmbientOcclusion\s*=\s*1\.f[\s\S]*?0u != g_iSSAOEnabled' `
    'SSAO OFF must resolve to fully unoccluded ambient lighting.'
Assert-Match $shader 'g_vLightAmbient \* g_vMtrlAmbient\) \* fAmbientOcclusion' `
    'SSAO must multiply the ambient term rather than direct light or emissive.'
Assert-Match $shader 'ShadowSampler[\s\S]*?AddressU\s*=\s*BORDER[\s\S]*?BorderColor\s*=\s*float4\(1\.f' `
    'Out-of-bounds PCF taps must resolve lit instead of repeating edge depth.'
Assert-Match $shader 'for \(int iY = -1; iY <= 1; \+\+iY\)[\s\S]*?for \(int iX = -1; iX <= 1; \+\+iX\)[\s\S]*?fLitSamples / 9\.f' `
    'Directional shadow resolve must perform a 3x3 PCF kernel.'
Assert-Match $shader 'fDirectDiffuse \* fDirectionalShadow' `
    'Shadow must attenuate directional direct diffuse only.'
Assert-Match $shader 'vNormalDesc\.a \* fDirectionalShadow' `
    'Shadow must attenuate directional specular only.'
Assert-Match $lightManager 'Get_TransientLights\(\)[\s\S]*?Render_Desc\([\s\S]*?false\)' `
    'Transient Effect lights must never consume the scene directional shadow map.'
Assert-Match $animatedShader 'g_HasSpecularTexture' `
    'Animated WModel rendering must consume the extracted specular slot.'
Assert-Match $animatedShader 'g_EmissiveColor\.rgb\s*\*\s*g_EmissiveIntensity' `
    'Animated WModel emissive masks must support parent-MI tint and intensity.'
Assert-Match $animatedShader 'g_HasFullSurfaceEmissiveOverride[\s\S]*?output\.vEmissive\.rgb\s*\+=' `
    'Animated character parts must support the action-scoped surface emissive override.'
Assert-Match $staticShader 'g_HasFullSurfaceEmissiveOverride[\s\S]*?output\.vEmissive\.rgb\s*\+=' `
    'Socketed character parts must support the same surface emissive override.'
Assert-Match $animatedShader 'pass Shadow[\s\S]*?PS_MAIN_SHADOW' `
    'Animated Character Select parts must preserve their existing shadow pass index.'
Assert-Match $staticShader 'pass ShadowBackPass[\s\S]*?pass ShadowFrontPass[\s\S]*?pass ShadowTwoSidedPass' `
    'Static caster passes must append all authored cull variants.'
Assert-Match $mapInstanceShader 'pass ShadowBackPass[\s\S]*?pass ShadowFrontPass[\s\S]*?pass ShadowTwoSidedPass' `
    'Instanced map caster passes must append all authored cull variants.'
Assert-Match $partBody 'RENDERGROUP::SHADOW[\s\S]*?Render_Shadow' `
    'Character Select body parts must submit and render animated shadow casters.'
Assert-Match $partEquipment 'RENDERGROUP::SHADOW[\s\S]*?Render_Shadow' `
    'Character Select equipment must submit static or animated shadow casters.'
Assert-Match $valtanBodyHeader 'HRESULT\s+Render_Shadow\(\)\s+override' `
    'Valtan body must expose the shared shadow-caster contract.'
Assert-Match $valtanBody 'Late_Update[\s\S]*?Is_ShadowLightEnabled\(\)[\s\S]*?RENDERGROUP::SHADOW[\s\S]*?Render_Shadow[\s\S]*?ANIMATED_SHADOW_PASS\s*=\s*1u[\s\S]*?Bind_BoneMatrices[\s\S]*?Begin\(ANIMATED_SHADOW_PASS\)' `
    'Valtan body and weapon must both submit the existing animated shadow pass.'
Assert-Match $deployPropHeader 'HRESULT\s+Render_Shadow\(\)\s+override' `
    'Deploy props must expose the shared shadow-caster contract.'
Assert-Match $deployProp 'Late_Update[\s\S]*?sourceVisible[\s\S]*?Has_VisibleDebrisPreviewInstance\(\)[\s\S]*?Is_ShadowLightEnabled\(\)[\s\S]*?RENDERGROUP::SHADOW' `
    'Deploy shadow submission must use the same source/debris visibility gate as normal draw.'
Assert-Match $deployProp 'Render_Shadow[\s\S]*?ANIMATED_SHADOW_PASS\s*=\s*1u[\s\S]*?STATIC_SHADOW_PASS\s*=\s*12u[\s\S]*?DEPLOY_PROP_STATE::DESPAWNED[\s\S]*?m_bDebrisSuppressSource[\s\S]*?Render_Animated\(ANIMATED_SHADOW_PASS\)[\s\S]*?Render_Static\([\s\S]*?STATIC_SHADOW_PASS\)[\s\S]*?Render_DebrisPreview\(true\)' `
    'Deploy shadows must preserve animated/static, intact/fractured, suppressed-source, and debris state.'
Assert-Match $deployProp 'Render_DebrisPreview[\s\S]*?shadowPass\s*\?[\s\S]*?Bind_ShadowShaderResources[\s\S]*?STATIC_SHADOW_PASS\s*:\s*0u' `
    'Visible debris must reuse the static shadow pass instead of a second renderer path.'
Assert-Match $mapAsset 'camera frustum[\s\S]*?RENDERGROUP::SHADOW' `
    'Fallback opaque map casters must not depend on camera visibility.'
Assert-Match $mapBatch 'Upload_ShadowInstances[\s\S]*?m_Instances[\s\S]*?instance\.Visible[\s\S]*?m_ShadowInstances\.push_back' `
    'Static map batches must keep an authored-visible shadow instance list.'
Assert-Match $materialBinder 'g_FullSurfaceEmissiveIntensity' `
    'Character material binding must submit the transient surface emissive intensity.'
Assert-Match $characterSpec 'SKILL_SURFACE_EMISSIVE_SPEC' `
    'Class-owned skill surface emissive presentation must have a typed spec.'
Assert-Match $dimensionMasterLogic '2050210u[\s\S]*?float4_t\(1\.f, 1\.f, 1\.f, 1\.f\)[\s\S]*?4\.f' `
    'DimensionMaster 2050210 must own the white surface emissive presentation row.'
Assert-Match $character 'Update_ActionEmissiveOverride\(action, skillId\)' `
    'Authoritative action edges must update and reset the transient emissive override.'
Assert-Match $materialBinder 'material\.valtan\.monster-base\.v1' `
    'Valtan material restoration must use a stable finite profile token.'
Assert-Match $materialBinder 'VALTAN_MASKED_TEAL_COLOR\s*=\s*[\r\n\t ]*float4_t\(0\.f, 1\.35f, 1\.55f, 1\.f\)' `
    'Valtan body/axe E-mask tint must preserve the exact R0 teal contract.'

Assert-Match $mainAppHeader 'RENDERING' `
    'Rendering Workbench must be a first-class F1 debug tool.'
Assert-Match $mainApp 'Rendering Workbench' `
    'Rendering Workbench window is missing.'
Assert-Match $renderingProfiles 'Apply_RenderQualitySettings' `
    'Rendering profile service must apply through the typed renderer boundary.'
Assert-Match $mainApp 'Effect Presentation' `
    'Rendering Workbench must keep typed Effect Light/Post diagnostics visible.'
Assert-Match $levelRegistry 'scene\.character-select\.warm-high-key\.v1' `
    'CLIENT_LEVEL_DESCRIPTOR must own the Character Select rendering profile ID.'
Assert-Match $levelRegistry 'scene\.valtan\.cool-low-key\.v1' `
    'CLIENT_LEVEL_DESCRIPTOR must own the distinct Valtan rendering profile ID.'
Assert-Match $renderingProfiles 'OutEffective\s*=\s*GlobalQuality' `
    'Scene resolution must start from the full global technical snapshot.'
Assert-Match $renderingProfiles 'GlobalQuality\.fExposure\s*\*\s*Profile\.fExposureMultiplier' `
    'Scene exposure must be derived from the global base without cumulative drift.'
Assert-Match $renderingProfiles 'GlobalQuality\.fBloomIntensity\s*\*\s*Profile\.fBloomIntensityMultiplier' `
    'Scene bloom intensity must be derived from the global base without cumulative drift.'
Assert-Match $renderingProfiles '"ssaoEnabled"[\s\S]*?"ssaoDistanceFade"' `
    'Rendering profiles must persist every live SSAO control.'
Assert-Match $mainApp 'SSAO Radius[\s\S]*?SSAO Distance Fade' `
    'Rendering Workbench must expose the wired SSAO controls.'
Assert-Match $mainApp 'Reset Global Legacy Defaults[\s\S]{0,180}bSSAOEnabled\s*=\s*false' `
    'Legacy A/B reset must explicitly disable the newly added SSAO pass.'
Assert-Match $mainApp 'Save Authored' `
    'Rendering Workbench must expose authored save.'
Assert-Match $mainApp 'Publish Runtime' `
    'Rendering Workbench must expose validated runtime publish.'
Assert-Match $mainApp 'Reload Runtime' `
    'Rendering Workbench must expose atomic runtime reload.'
Assert-Match $characterSelect 'CharacterSelectCameraPositionOffset\(\)' `
    'Character Select Server Arena must use one initial/rebind camera position preset.'
Assert-Match $characterSelect 'CHARACTER_SELECT_CAMERA_FOV_Y\s*=\s*45\.f' `
    'Character Select shared camera preset must own its fixed FOV.'
Assert-Match $shader 'Out\.vBackBuffer\s*=\s*vLitColor\s*\+\s*vEmissive' `
    'Shadow composition must preserve object emissive for Effect/Bloom readability.'
Assert-Match $mainApp 'Save changes Authored only; Publish validates/promotes Runtime' `
    'The workbench must state its authored/runtime persistence boundary.'

$engineShaderPath = Join-Path $RepoRoot 'Engine\Bin\ShaderFiles\Shader_Deferred.hlsl'
$clientShaderPath = Join-Path $RepoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'
$engineHash = (Get-FileHash -LiteralPath $engineShaderPath -Algorithm SHA256).Hash
$clientHash = (Get-FileHash -LiteralPath $clientShaderPath -Algorithm SHA256).Hash
if ($engineHash -ne $clientHash) {
    throw 'Engine Shader_Deferred source and Client runtime copy differ; run UpdateLib.'
}

Write-Host 'Render Quality Workbench audit PASS'
Write-Host '  defaults: legacy-compatible'
Write-Host '  light accumulation: FP16'
Write-Host '  bloom/hable: runtime-bound'
Write-Host '  FXAA: edge-aware, opt-in'
Write-Host '  SSAO: half-resolution, bilateral, ambient-only, persisted'
Write-Host '  shadow: 2048 true depth, orthographic, 3x3 PCF, directional-only'
Write-Host '  F1 workbench: typed live apply'
Write-Host '  Effect diagnostics: typed Light/Post toggles and submitted counts'
Write-Host '  scene lighting: descriptor-owned Character Select/Valtan profiles'
Write-Host '  persistence: authored save, validated publish, atomic reload'
Write-Host '  Character Select camera: one Server Arena initial/rebind preset'
Write-Host '  shader deployment hash: equal'
