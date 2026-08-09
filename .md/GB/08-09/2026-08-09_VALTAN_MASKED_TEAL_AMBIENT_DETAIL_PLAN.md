# Valtan 몸·도끼 masked teal ambient 디테일 계획서

## G00. `DeferredMaterialRenderUtils.cpp`

### 목표와 종료 증거

Valtan profile resolve만 바꿔 다섯 exact material의 E texture가 선택한 부위에 R=0, G/B 청록 가중치를 적용한다. 다른 profile과 다른 세션의 transient full-surface override는 바꾸지 않는다.

### 변경 후 전체 코드

```cpp
#include "DeferredMaterialRenderUtils.h"

#include "Model.h"
#include "Shader.h"

#include <cmath>

namespace
{
	constexpr std::string_view VALTAN_MATERIAL_PROFILE =
		"material.valtan.monster-base.v1";
	constexpr float4_t VALTAN_MASKED_TEAL_COLOR =
		float4_t(0.f, 1.35f, 1.55f, 1.f);

	f32_t Resolve_ValtanEmissiveIntensity(std::string_view strMaterialName)
	{
		if (strMaterialName == "mn_rpbf_01_2_mi")
			return 15.f;
		if (strMaterialName == "mn_rpbf_01_1_mi" ||
			strMaterialName == "wp_mn_rpbf_01_1_mi")
			return 10.f;
		if (strMaterialName == "mn_rpbf_01_mi" ||
			strMaterialName == "wp_mn_rpbf_01_mi")
			return 5.f;
		return 0.f;
	}
}

Client::DEFERRED_MATERIAL_PROFILE Client::Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName)
{
	DEFERRED_MATERIAL_PROFILE Profile{};
	if (strProfileId == VALTAN_MATERIAL_PROFILE)
	{
		/* The deferred path has no per-material ambient RGB target.  The
		   authored E map is the exact body/axe part mask for unlit ambient
		   energy, so keep the mask and apply the teal weights here. */
		Profile.vEmissiveColor = VALTAN_MASKED_TEAL_COLOR;
		Profile.fEmissiveIntensity =
			Resolve_ValtanEmissiveIntensity(strMaterialName);
	}
	return Profile;
}

HRESULT Client::Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile,
	const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride)
{
	if (nullptr == pShader || iMeshIndex >= Model.Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	const uint32_t iHasNormal = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_NORMALS) ? 1u : 0u;
	const uint32_t iHasSpecular = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_SPECULAR) ? 1u : 0u;
	const uint32_t iHasEmissive = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;
	const bool_t hasValidOverride =
		nullptr != pEmissiveOverride && pEmissiveOverride->isEnabled &&
		std::isfinite(pEmissiveOverride->fIntensity) &&
		pEmissiveOverride->fIntensity > 0.f;
	const uint32_t iHasFullSurfaceEmissiveOverride =
		hasValidOverride ? 1u : 0u;
	const float4_t vFullSurfaceEmissiveColor = hasValidOverride ?
		pEmissiveOverride->vColor : float4_t(1.f, 1.f, 1.f, 1.f);
	const f32_t fFullSurfaceEmissiveIntensity = hasValidOverride ?
		pEmissiveOverride->fIntensity : 0.f;

	if (FAILED(Model.Bind_Material(
		pShader, "g_DiffuseTexture", iMeshIndex, aiTextureType_DIFFUSE, 0)) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal))) ||
		(0u != iHasNormal && FAILED(Model.Bind_Material(
			pShader, "g_NormalTexture", iMeshIndex, aiTextureType_NORMALS, 0))) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasSpecularTexture", &iHasSpecular, sizeof(iHasSpecular))) ||
		FAILED(pShader->Bind_RawValue("g_SpecularIntensity",
			&Profile.fSpecularIntensity, sizeof(Profile.fSpecularIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_SpecularPower",
			&Profile.fSpecularPower, sizeof(Profile.fSpecularPower))) ||
		(0u != iHasSpecular && FAILED(Model.Bind_Material(
			pShader, "g_SpecularTexture", iMeshIndex, aiTextureType_SPECULAR, 0))) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasEmissiveTexture", &iHasEmissive, sizeof(iHasEmissive))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveColor",
			&Profile.vEmissiveColor, sizeof(Profile.vEmissiveColor))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity",
			&Profile.fEmissiveIntensity, sizeof(Profile.fEmissiveIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_HasFullSurfaceEmissiveOverride",
			&iHasFullSurfaceEmissiveOverride,
			sizeof(iHasFullSurfaceEmissiveOverride))) ||
		FAILED(pShader->Bind_RawValue("g_FullSurfaceEmissiveColor",
			&vFullSurfaceEmissiveColor,
			sizeof(vFullSurfaceEmissiveColor))) ||
		FAILED(pShader->Bind_RawValue("g_FullSurfaceEmissiveIntensity",
			&fFullSurfaceEmissiveIntensity,
			sizeof(fFullSurfaceEmissiveIntensity))) ||
		(0u != iHasEmissive && FAILED(Model.Bind_Material(
			pShader, "g_EmissiveTexture", iMeshIndex, aiTextureType_EMISSIVE, 0))))
	{
		return E_FAIL;
	}
	return S_OK;
}
```

## G01. `Test-ValtanMaskedTealAmbient.ps1`

### 새 파일 전체 코드

```powershell
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
```

### 검증 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-ValtanMaskedTealAmbient.ps1
git diff --check
```

## G02. 전체 Rendering Workbench audit 연결

`Tools/ProjectAudit/Test-RenderQualityWorkbench.ps1`의 기존 Valtan assertion 두 줄만 다음 코드로 교체한다. 이 파일의 다른 세션 변경은 보존한다.

```powershell
Assert-Match $materialBinder 'VALTAN_MASKED_TEAL_COLOR\s*=\s*[\r\n\t ]*float4_t\(0\.f, 1\.35f, 1\.55f, 1\.f\)' `
    'Valtan body/axe E-mask tint must preserve the exact R0 teal contract.'
```
