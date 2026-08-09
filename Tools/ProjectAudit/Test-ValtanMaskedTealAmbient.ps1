param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
)

$ErrorActionPreference = 'Stop'

function Fail([string]$Message) {
    throw "[ValtanMaskedTealAmbient] $Message"
}

function Read-Text([string]$RelativePath) {
    $path = Join-Path $RepoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        Fail "missing file: $RelativePath"
    }
    return [System.IO.File]::ReadAllText($path)
}

function Require-Match([string]$Text, [string]$Pattern, [string]$Message) {
    if ($Text -notmatch $Pattern) {
        Fail $Message
    }
}

$utility = Read-Text 'Client\Private\DeferredMaterialRenderUtils.cpp'
$body = Read-Text 'Client\Private\Body_Valtan.cpp'
$valtan = Read-Text 'Client\Private\Valtan.cpp'
$animShader = Read-Text 'Client\Bin\ShaderFiles\Shader_VtxAnimMeshBinary.hlsl'
$meshShader = Read-Text 'Client\Bin\ShaderFiles\Shader_VtxMeshBinary.hlsl'

Require-Match $utility 'VALTAN_MASKED_TEAL_COLOR\s*=\s*\r?\n?\s*float4_t\(0\.f,\s*1\.35f,\s*1\.55f,\s*1\.f\)' 'teal weights must keep R=0 and exact G/B values'
Require-Match $utility 'mn_rpbf_01_2_mi"\)\s*\r?\n?\s*return 15\.f' 'body material _2 intensity is missing'
Require-Match $utility 'mn_rpbf_01_1_mi"\s*\|\|[\s\S]{0,100}wp_mn_rpbf_01_1_mi"\)[\s\S]{0,40}return 10\.f' 'body/axe material _1 intensity is missing'
Require-Match $utility 'mn_rpbf_01_mi"\s*\|\|[\s\S]{0,100}wp_mn_rpbf_01_mi"\)[\s\S]{0,40}return 5\.f' 'body/axe base intensity is missing'
Require-Match $utility 'return 0\.f;\s*\r?\n\s*\}' 'unknown Valtan material must fail closed'
Require-Match $body 'Resolve_DeferredMaterialProfile\(\s*"material\.valtan\.monster-base\.v1"' 'Valtan body must consume the exact material profile'
Require-Match $valtan 'weaponDesc\.strMaterialProfileId\s*=\s*"material\.valtan\.monster-base\.v1"' 'Valtan axe must consume the exact material profile'

foreach ($shader in @($animShader, $meshShader)) {
    Require-Match $shader 'g_EmissiveTexture\.Sample' 'binary shader must sample the authored E mask'
    Require-Match $shader 'emissive\s*\*\s*g_EmissiveColor\.rgb\s*\*\s*g_EmissiveIntensity' 'binary shader must multiply the E mask by profile color and intensity'
}

$textures = @(
    'Client\Bin\Resources\Character\Valtan\textures\mn_rpbf_01_e.tga',
    'Client\Bin\Resources\Character\Valtan\textures\mn_rpbf_01-1_e_loc_int.tga',
    'Client\Bin\Resources\Character\Valtan\textures\mn_rpbf_01-2_e.tga',
    'Client\Bin\Resources\Character\Valtan\textures\wp_mn_rpbf_01_e.dds',
    'Client\Bin\Resources\Character\Valtan\textures\wp_mn_rpbf_01-1_e.dds'
)
foreach ($texture in $textures) {
    if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot $texture) -PathType Leaf)) {
        Fail "missing authored E mask: $texture"
    }
}

Write-Host '[ValtanMaskedTealAmbient] PASS: exact body/axe E-mask slots use fail-closed R0 teal weighting.'
