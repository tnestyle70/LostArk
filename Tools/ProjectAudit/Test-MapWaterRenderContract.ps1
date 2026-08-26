param()

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path

function Read-RequiredFile([string]$relativePath) {
    $path = Join-Path $root $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Missing required file: $relativePath"
    }
    return Get-Content -LiteralPath $path -Raw
}

function Require-Text(
    [string]$content,
    [string]$pattern,
    [string]$message) {
    if ($content -notmatch $pattern) {
        throw $message
    }
}

function Require-IncreasingMarkers(
    [string]$content,
    [string[]]$markers,
    [string]$message) {
    $previous = -1
    foreach ($marker in $markers) {
        $index = $content.IndexOf($marker, [StringComparison]::Ordinal)
        if ($index -le $previous) {
            throw "$message Missing or out-of-order marker: $marker"
        }
        $previous = $index
    }
}

$loader = Read-RequiredFile 'Client\Private\Loader.cpp'
$mapObject = Read-RequiredFile 'Client\Private\MapAssetObject.cpp'
$mapBatch = Read-RequiredFile 'Client\Private\MapStaticBatchObject.cpp'
$mapUtils = Read-RequiredFile 'Client\Private\MapAssetRenderUtils.cpp'
$deployProp = Read-RequiredFile 'Client\Private\DeployPropObject.cpp'
$shader = Read-RequiredFile 'Client\Bin\ShaderFiles\Shader_VtxMeshBinary.hlsl'

Require-Text $loader `
    'Prototype_Component_Shader_VtxMeshBinary[\s\S]*Shader_VtxMeshBinary\.hlsl' `
    'The fallback map-object shader prototype no longer resolves to Shader_VtxMeshBinary.hlsl.'
Require-Text $mapObject `
    'CMapAssetObject::Ready_Components[\s\S]*Prototype_Component_Shader_VtxMeshBinary' `
    'CMapAssetObject no longer consumes the fallback VTXMESH shader contract.'
Require-Text $mapBatch `
    'MAP_ASSET_RENDER_MODE::DEFERRED\s*!=\s*desc\.RenderProfile\.renderMode' `
    'Static map batches no longer reject translucent and water render profiles.'
Require-Text $mapUtils `
    'MAP_ASSET_RENDER_MODE::WATER\s*==\s*profile\.renderMode\s*\?\s*15u' `
    'The authored water pass base is not 15.'
Require-Text $deployProp `
    'DEFERRED_EMISSIVE_OVERLAY_PASS\s*=\s*18u' `
    'The Valtan deferred emissive overlay still overlaps water passes 15-17.'

$waterBindings = @(
    'g_WaterOpacity',
    'g_WaterOpacityPower',
    'g_WaterFresnelIntensity',
    'g_WaterFresnelPower',
    'g_WaterScreenDistortionIntensity',
    'g_WaterNormalIntensity',
    'g_WaterDetailNormalIntensity',
    'g_WaterReflectionIntensity',
    'g_WaterDiffuseTiling',
    'g_ElapsedTime',
    'g_WaterDiffuseColor',
    'g_WaterReflectionColor',
    'g_WaterNormalTilingPanning',
    'g_WaterDetailNormalTilingPanning',
    'g_WaterReflectionTilingPanning',
    'g_HasDetailNormalTexture',
    'g_HasReflectionTexture'
)
foreach ($binding in $waterBindings) {
    Require-Text $mapObject `
        ('Bind_(?:RawValue|CamPosition)\(\s*"' + [regex]::Escape($binding) + '"|\{\s*"' + [regex]::Escape($binding) + '"') `
        "CMapAssetObject does not bind $binding."
    Require-Text $shader `
        ('\b' + [regex]::Escape($binding) + '\b') `
        "Shader_VtxMeshBinary.hlsl does not declare or consume $binding."
}

Require-Text $mapObject `
    'Bind_CamPosition\(\s*m_pShaderCom,\s*"g_vCamPosition"' `
    'Water rendering does not bind the camera position.'
Require-Text $shader '\bg_vCamPosition\b' `
    'Shader_VtxMeshBinary.hlsl does not own the water camera position.'
Require-Text $shader 'PS_OUT_WATER\s+PS_MAIN_WATER' `
    'The fallback object shader has no water pixel shader.'

Require-IncreasingMarkers $shader @(
    'pass ShadowBackPass',
    'pass ShadowFrontPass',
    'pass ShadowTwoSidedPass',
    'pass WaterBackPass',
    'pass WaterFrontPass',
    'pass WaterTwoSidedPass',
    'pass DeferredEmissiveOverlayPass'
) 'The fallback object shader pass contract is invalid.'

Write-Host 'Map water render contract: PASS'

