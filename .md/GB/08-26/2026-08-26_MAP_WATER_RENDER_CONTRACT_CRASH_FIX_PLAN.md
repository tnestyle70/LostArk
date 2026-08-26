# Bern/Valtan map water render-contract crash fix plan

## 1. Goal and reproduced failure

- Lobby entry itself remains valid, but both `LEVEL::BERN` and `LEVEL::VALTAN_ARENA` terminate on their first scene render.
- `Client/Default/ClientExit.user.log` records `Render failed hr=0x80004005` for level 4 and level 5.
- `Client/Default/RendererExit.user.log` narrows every failure to `Render_Blend_Object` on `Client::CMapAssetObject`.
- `CMapAssetObject` clones `Prototype_Component_Shader_VtxMeshBinary`, but `Bind_WaterShaderResources` binds water variables that only exist in `Shader_VtxMeshMapInstance.hlsl`. The same object shader also has pass 15 occupied by `DeferredEmissiveOverlayPass`, while `CMapAssetRenderUtils::Select_Pass` selects pass 15 for water.

The change is complete when the fallback object shader owns the water variables, water pixel shader and passes 15-17, the Valtan emissive overlay moves to pass 18, a focused contract test prevents this shader/runtime mismatch, and Client x64 Debug links successfully.

## 2. Runtime flow being repaired

```text
Lobby command
-> CLoader::Ready_MapAuthoringCore
-> Prototype_Component_Shader_VtxMeshBinary
-> CMapAssetObject::Late_Update queues translucent/water placement in BLEND
-> CMapAssetObject::Render
-> Bind_WaterShaderResources
-> CMapAssetRenderUtils::Select_Pass
-> CShader::Begin
-> CModel::Render
```

The object path must bind and select resources that exist in `Shader_VtxMeshBinary.hlsl`. `Shader_VtxMeshMapInstance.hlsl` is consumed by `CMapStaticBatchObject`, which rejects every render mode except `DEFERRED`; its water pass cannot satisfy the fallback object path.

## 3. Existing-file changes

### `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`

Add the complete water constant/texture contract already consumed by `CMapAssetObject::Bind_WaterShaderResources`:

```hlsl
Texture2D g_DetailNormalTexture;
Texture2D g_ReflectionTexture;
vector g_vCamPosition;
float g_ElapsedTime = 0.f;
uint g_HasDetailNormalTexture = 0;
uint g_HasReflectionTexture = 0;
float g_WaterOpacity = 1.f;
float g_WaterOpacityPower = 1.f;
float g_WaterFresnelIntensity = 0.f;
float g_WaterFresnelPower = 1.f;
float g_WaterScreenDistortionIntensity = 0.f;
float g_WaterNormalIntensity = 0.f;
float g_WaterDetailNormalIntensity = 0.f;
float g_WaterReflectionIntensity = 0.f;
float g_WaterDiffuseTiling = 1.f;
float4 g_WaterDiffuseColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_WaterReflectionColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_WaterNormalTilingPanning = float4(1.f, 1.f, 0.f, 0.f);
float4 g_WaterDetailNormalTilingPanning = float4(1.f, 1.f, 0.f, 0.f);
float4 g_WaterReflectionTilingPanning = float4(1.f, 1.f, 0.f, 0.f);
```

Add the same `PS_OUT_WATER`, UV panning, normal sampling and `PS_MAIN_WATER` contract used by the instance shader. Insert `WaterBackPass`, `WaterFrontPass`, and `WaterTwoSidedPass` after the three shadow passes. Move `DeferredEmissiveOverlayPass` after those water passes so the established water base selected by `Select_Pass` remains 15.

### `Client/Private/DeployPropObject.cpp`

Replace the shader-specific overlay constant in `Render_DeferredEmissiveOverlay`:

```cpp
constexpr uint32_t DEFERRED_EMISSIVE_OVERLAY_PASS = 18u;
```

No other deploy-prop behavior changes. The overlay remains after the three water passes and continues to use the same pixel shader, depth bias and emissive-only write mask.

### `Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py`

Update the existing shader pass-index assertion from 15 to 18. All other floor-emissive assertions remain unchanged.

## 4. New executable contract test

### `Tools/ProjectAudit/Test-MapWaterRenderContract.ps1`

The new test reads current source files and fails unless all of the following are true:

- `CMapAssetObject` still clones `Prototype_Component_Shader_VtxMeshBinary`.
- every water variable bound by `Bind_WaterShaderResources` exists in `Shader_VtxMeshBinary.hlsl`.
- `Select_Pass` maps `WATER` to base pass 15.
- binary-shader pass order is shadow 12-14, water 15-17, overlay 18.
- `CDeployPropObject` selects overlay pass 18.
- static batches continue to reject non-deferred profiles, proving the instance shader is not an alternative runtime consumer for water placements.

The test is a source/runtime-contract audit and needs no project registration.

## 5. Verification

1. Run `Tools/ProjectAudit/Test-MapWaterRenderContract.ps1`.
2. Run `Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py`.
3. Compile `Shader_VtxMeshBinary.hlsl` as `fx_5_0` when the repository's shader compiler is available through the normal Client build.
4. Build Client x64 Debug from `Client/Default` contract.
5. Run `git diff --check` on the touched files.
6. User manually runs Server + Client, selects direct Bern, direct Valtan, and Character Create -> Bern. None may terminate during loading or first render. Visual water and Valtan emissive fidelity remain user-owned manual checks.

