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

$engineStruct = Read-Utf8 'Engine\Public\Engine_Struct.h'
$renderer = Read-Cp949 'Engine\Private\Renderer.cpp'
$rendererHeader = Read-Cp949 'Engine\Public\Renderer.h'
$gameInstance = Read-Cp949 'Engine\Private\GameInstance.cpp'
$shader = Read-Utf8 'Engine\Bin\ShaderFiles\Shader_Deferred.hlsl'
$mainApp = Read-Utf8 'Client\Private\MainApp.cpp'
$mainAppHeader = Read-Utf8 'Client\Public\MainApp.h'
$animatedShader = Read-Utf8 'Client\Bin\ShaderFiles\Shader_VtxAnimMeshBinary.hlsl'
$materialBinder = Read-Utf8 'Client\Private\DeferredMaterialRenderUtils.cpp'

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
Assert-Match $rendererHeader 'Apply_RenderQualitySettings' `
    'Renderer must expose a typed atomic apply boundary.'
Assert-Match $gameInstance 'm_pRenderer->Apply_RenderQualitySettings' `
    'Client tools must reach settings through CGameInstance, not Renderer ownership.'

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
Assert-Match $animatedShader 'g_HasSpecularTexture' `
    'Animated WModel rendering must consume the extracted specular slot.'
Assert-Match $animatedShader 'g_EmissiveColor\.rgb\s*\*\s*g_EmissiveIntensity' `
    'Animated WModel emissive masks must support parent-MI tint and intensity.'
Assert-Match $materialBinder 'material\.valtan\.monster-base\.v1' `
    'Valtan material restoration must use a stable finite profile token.'
Assert-Match $materialBinder '0\.15f, 1\.5f, 0\.9f' `
    'Valtan emissive tint must preserve the extracted parent-MI evidence.'

Assert-Match $mainAppHeader 'RENDERING' `
    'Rendering Workbench must be a first-class F1 debug tool.'
Assert-Match $mainApp 'Rendering Workbench' `
    'Rendering Workbench window is missing.'
Assert-Match $mainApp 'Apply_RenderQualitySettings' `
    'Rendering controls must apply through the typed renderer boundary.'
Assert-Match $mainApp 'Effect Presentation' `
    'Rendering Workbench must keep typed Effect Light/Post diagnostics visible.'
Assert-Match $mainApp 'scene\.character-select\.warm-high-key\.v1' `
    'Character Select must use an explicit scene rendering profile.'
Assert-Match $mainApp 'scene\.valtan\.cool-low-key\.v1' `
    'Valtan must use a distinct low-key scene rendering profile.'
Assert-Match $shader 'Out\.vBackBuffer\s*=\s*vLitColor\s*\+\s*vEmissive' `
    'Shadow composition must preserve object emissive for Effect/Bloom readability.'
Assert-Match $mainApp 'Session-only in this slice' `
    'The tool must not imply that session tuning was saved or published.'

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
Write-Host '  F1 workbench: typed live apply'
Write-Host '  Effect diagnostics: typed Light/Post toggles and submitted counts'
Write-Host '  scene lighting: Character Select/Valtan profiles separated'
Write-Host '  shader deployment hash: equal'
