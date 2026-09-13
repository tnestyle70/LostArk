#include "Effect_DocumentRenderer_Internal.h"
#include "GameInstance.h"
#include "Model.h"
#include "Profiler.h"
#include "Render_OutputContract.h"
#include "Shader.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

shared_ptr<Engine::CShader> Client::CEffectDocumentRenderer::Resolve_DrawShader(
    const ELEMENT_RESOURCE& Resource, const EFFECT_SHADER_CARRIER eCarrier,
    const EFFECT_SHADER_PROGRAM_DESC*& pOutProgram) const
{
    pOutProgram = Get_EffectShaderProgram(Resource.iShaderProgramIndex);
    if (nullptr == pOutProgram || pOutProgram->eCarrier != eCarrier ||
        Resource.iShaderProgramIndex >= m_ShaderPrograms.size())
        return nullptr;
    // Native contracts reject authored execution. Keep that stage invariant at
    // the actual draw boundary, including reconstructed and material-slot rows.
    if (pOutProgram->eFamily != EFFECT_SHADER_FAMILY::GENERIC &&
        (0u != Resource.iStandardColorV1Enabled ||
         0u != Resource.iRuntimeMaterialV2Enabled ||
         0u != Resource.iArtistVisualV4Opcode ||
         0u != Resource.iReconstructedMaterialEvaluatorEnabled))
        return nullptr;
    return m_ShaderPrograms[Resource.iShaderProgramIndex];
}

HRESULT Client::CEffectDocumentRenderer::Render_Mesh(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride,
	const float4_t* pDynamicParameter,
	const EFFECT_SUBUV_FRAME_DESC* pSubUVOverride,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
	const uint32_t iSourceMaterialIndex,
	const std::span<const EFFECT_NATIVE_MESH_INSTANCE> Instances,
	const uint32_t iInstanceByteOffset,
	const uint32_t iOrderedGeometryHandle)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Mesh.Render");
	if (nullptr == Resource.pModel || nullptr == Element.pElement)
		return Fail_RenderOperation(
			"Mesh resource/model/shader contract is missing.", E_FAIL, true);
	if (!Resource.SourceMaterialSlots.empty())
	{
		const auto& Slots = Element.pElement->Detail.Mesh.SourceMaterialSlots;
		if (Slots.size() != Resource.SourceMaterialSlots.size())
			return Fail_RenderOperation("Prepared mesh source slots changed.", E_FAIL, true);
		bool_t bSubmitted = false;
		for (const auto& PreparedSlot : Resource.SourceMaterialSlots)
		{
			const auto Slot = std::find_if(Slots.begin(), Slots.end(), [&](const auto& Row)
				{ return Row.iSourceMaterialIndex == PreparedSlot.iSourceMaterialIndex; });
			if (Slot == Slots.end() || nullptr == PreparedSlot.pResource ||
				!PreparedSlot.pResource->SourceMaterialSlots.empty() ||
				PreparedSlot.pResource->pModel != Resource.pModel)
				return Fail_RenderOperation("Prepared mesh source slot is missing.", E_FAIL, true);
			const HRESULT Result = Render_Mesh(Element, *PreparedSlot.pResource,
				fAlphaScale, pWorldOverride, pDynamicParameter, pSubUVOverride,
				&Slot->Material, Slot->iSourceMaterialIndex);
			if (FAILED(Result)) return Result;
			bSubmitted |= Result == S_OK;
		}
		return bSubmitted ? S_OK : S_FALSE;
	}
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram = nullptr;
    const shared_ptr<Engine::CShader> pDrawShader = Resolve_DrawShader(
        Resource, EFFECT_SHADER_CARRIER::MESH, pShaderProgram);
    if (nullptr == pDrawShader)
        return Fail_RenderOperation("Prepared mesh shader family is unavailable.", E_FAIL, true);
	const EFFECT_MATERIAL_DESC& Material = nullptr != pMaterialOverride ?
		*pMaterialOverride : Element.pElement->Material;
	const auto& NativeProfile = Material.SourceMaterial.strRuntimeShaderProfileId;
	const bool_t bNativeMesh = Find_ArtistProgram(NativeProfile) ||
		Find_WarlordNativeProgram(NativeProfile) || Find_LanceMasterVAProgram(NativeProfile);
	uint32_t iPass = Select_Pass(
		Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Mesh render-profile pass is invalid.", E_INVALIDARG, true);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = Resource.pMaterialProgramBinding;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Resource.iStandardColorV1Enabled == 1u &&
			Resource.StandardColorV1Header[0u] == 1u &&
			Resource.StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			Resource.StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != Resource.iRuntimeMaterialV2Enabled &&
			Resource.iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Element.pElement->strElementId).get() !=
				pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL ||
			Element.pElement->eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			!Element.pElement->SourceRecipe.bEnabled ||
			Element.pElement->SourceRecipe.strRendererShape != "mesh" ||
			nullptr == Find_Binding(
				*Element.pElement, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			iPass != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
					SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound Mesh material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	const float4x4_t& World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	const bool_t bMainSourceReplay =
		0u != Resource.iRuntimeMaterialV2Enabled &&
		(3u == Resource.iRuntimeMaterialV2Opcode ||
			8u == Resource.iRuntimeMaterialV2Opcode);
	const bool_t bLanceDragonMaskedReplay =
		0u != Resource.iRuntimeMaterialV2Enabled &&
		LANCE_DRAGON_MASKED_OPCODE == Resource.iRuntimeMaterialV2Opcode;
	const bool_t bFlow02RecoveredEquation =
		7u == Resource.iArtistVisualV4Opcode;
	if ((bMainSourceReplay || bLanceDragonMaskedReplay ||
		bFlow02RecoveredEquation) &&
		nullptr == pDynamicParameter)
	{
		// These source occurrences carry ParameterDynamic.  UE3's missing
		// payload fallback is ones, not this renderer's generic zero value;
		// reject the missing carrier instead of silently selecting a default.
		return Fail_RenderOperation(
			"Mesh source-replay dynamic payload is missing.", E_INVALIDARG, true);
	}
	const float4_t DynamicParameter = nullptr == pDynamicParameter ?
		float4_t{} : *pDynamicParameter;
	if (0u != Resource.iRuntimeMaterialV2Enabled &&
		1u == Resource.iRuntimeMaterialV2Opcode)
	{
		if (!std::isfinite(DynamicParameter.x))
			return Fail_RenderOperation(
				"Mesh RuntimeMaterialV2 dynamic alpha is non-finite.",
				E_INVALIDARG, true);
		// active004's exact named dynamic `alpha` lane owns visibility.  At
		// zero the dissolve equation is guaranteed to clip every pixel, so
		// report a typed suppression instead of laundering a zero-pixel draw
		// into a submitted occurrence.
		if (DynamicParameter.x <= 0.f)
			return S_FALSE;
	}
	if (bMainSourceReplay)
	{
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Mesh source-replay carrier is non-finite.",
				E_INVALIDARG, true);
		}
		// Particle alpha is multiplicative in both recovered opacity programs.
		// Dynamic X is a UV offset and Dynamic Z is a dissolve threshold, so
		// neither lane is a valid CPU zero-pixel predicate.
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	if (bLanceDragonMaskedReplay)
	{
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Lance dragon typed mesh carrier is non-finite.",
				E_INVALIDARG, true);
		}
		/* Dynamic W owns dissolve while ParticleColor alpha owns the lifetime
		   envelope.  Only the latter has a texture-independent zero predicate. */
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	if (bFlow02RecoveredEquation)
	{
		constexpr f32_t fDynamicTolerance = 1e-5f;
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			std::abs(DynamicParameter.x - 1.f) > fDynamicTolerance ||
			DynamicParameter.y < -fDynamicTolerance ||
			DynamicParameter.y > 2.f + fDynamicTolerance ||
			std::abs(DynamicParameter.z - 1.f) > fDynamicTolerance ||
			std::abs(DynamicParameter.w - 1.f) > fDynamicTolerance ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Mesh flow-02 recovered dynamic carrier changed.",
				E_INVALIDARG, true);
		}
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
	float4x4_t NormalMatrix = World;
	const matrix_t LoadedWorld = XMLoadFloat4x4(&World);
	const vector_t Determinant = XMMatrixDeterminant(LoadedWorld);
	const f32_t fDeterminant = XMVectorGetX(Determinant);
	if (!std::isfinite(fDeterminant))
		return Fail_RenderOperation(
			"Mesh world determinant is non-finite.", E_INVALIDARG, true);
	// A zero SizeMultiplyLife sample is a valid no-pixel state at the
	// occurrence boundary.  It must not abort the whole effect frame.
	if (std::abs(fDeterminant) <= std::numeric_limits<f32_t>::epsilon())
		return S_FALSE;
	if ((bNativeMesh || nullptr != pMaterialProgramBinding || 42u == Resource.iSourceMaterialProfile ||
		50u == Resource.iSourceMaterialProfile || 60u == Resource.iSourceMaterialProfile ||
		66u == Resource.iSourceMaterialProfile || 70u == Resource.iSourceMaterialProfile) && fDeterminant < 0.f)
	{
		/* Registry pass 3/4 is the nominal one-sided policy. A negative world
		   determinant reverses winding, so the actual CModel draw must use the
		   shader's matching front-cull receipt. Two-sided pass 1/2 is invariant. */
		if (iPass == 3u)
			iPass = 5u;
		else if (iPass == 4u)
			iPass = 6u;
	}
	XMStoreFloat4x4(&NormalMatrix,
		XMMatrixTranspose(XMMatrixInverse(nullptr, LoadedWorld)));
	HRESULT hResult = pDrawShader->Bind_Matrix("g_WorldMatrix", &World);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_WorldMatrix.", hResult);
	hResult = pDrawShader->Bind_Matrix("g_NormalMatrix", &NormalMatrix);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_NormalMatrix.", hResult);
	hResult = pDrawShader->Bind_RawValue("g_CameraPosition",
		&CameraPosition, sizeof(CameraPosition));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_CameraPosition.", hResult);
	hResult = pDrawShader->Bind_RawValue("g_EffectDynamicParameter",
		&DynamicParameter, sizeof(DynamicParameter));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_EffectDynamicParameter.", hResult);
	// Native local-mesh VS programs preserve geometry UVs. A computed SubUV
	// payload alone does not prove that the selected source shader consumes it.
	const bool_t bStandardColorSubUV = nullptr != pSubUVOverride &&
		0u != Resource.iStandardColorV1Enabled;
	const uint32_t iStandardColorSubUVEnabled =
		bStandardColorSubUV ? 1u : 0u;
	const EFFECT_SUBUV_FRAME_DESC IdentitySubUV{};
	const EFFECT_SUBUV_FRAME_DESC& StandardColorSubUV =
		bStandardColorSubUV ? *pSubUVOverride : IdentitySubUV;
	const auto IsFiniteFloat4 = [](const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	};
	if (!IsFiniteFloat4(StandardColorSubUV.Current) ||
		!IsFiniteFloat4(StandardColorSubUV.Next) ||
		!std::isfinite(StandardColorSubUV.fBlend) ||
		StandardColorSubUV.fBlend < 0.f ||
		StandardColorSubUV.fBlend > 1.f)
	{
		return Fail_RenderOperation(
			"Mesh StandardColorV1 SubUV carrier is invalid.",
			E_INVALIDARG, true);
	}
	hResult = pDrawShader->Bind_RawValue(
		"g_StandardColorV1MeshSubUVEnabled",
		&iStandardColorSubUVEnabled,
		sizeof(iStandardColorSubUVEnabled));
	if (SUCCEEDED(hResult))
		hResult = pDrawShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVCurrent",
			&StandardColorSubUV.Current,
			sizeof(StandardColorSubUV.Current));
	if (SUCCEEDED(hResult))
		hResult = pDrawShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVNext",
			&StandardColorSubUV.Next,
			sizeof(StandardColorSubUV.Next));
	if (SUCCEEDED(hResult))
		hResult = pDrawShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVBlend",
			&StandardColorSubUV.fBlend,
			sizeof(StandardColorSubUV.fBlend));
	if (FAILED(hResult))
	{
		return Fail_RenderOperation(
			"Mesh shader bind failed: StandardColorV1 SubUV packet.",
			hResult, true);
	}
	hResult = Bind_Common(pDrawShader, Element, Resource, fAlphaScale, pMaterialOverride, pShaderProgram);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh common/material shader bind failed.", hResult);

	if (178u == Resource.iSourceMaterialProfile)
	{
		if (nullptr == m_pStartingSceneCapture)
			return Fail_RenderOperation("Native capture box has no starting scene snapshot.", E_FAIL, true);
		if (!m_pStartingSceneBloomCapture)
			return Fail_RenderOperation("Native capture box has no matching bloom snapshot.", E_FAIL, true);
		hResult = pDrawShader->Bind_Texture("g_SourceTexture2", m_pStartingSceneCapture);
		if (FAILED(hResult))
			return Fail_RenderOperation("Native capture box snapshot binding failed.", hResult, true);
	}

	hResult = pDrawShader->Bind_RawValue("g_SourceMeshHasUV1",
		&Resource.iSourceMeshHasUV1, sizeof(Resource.iSourceMeshHasUV1));
	if (FAILED(hResult))
		return Fail_RenderOperation("Native Q mesh UV-set binding failed.", hResult, true);
	// The source samples scene color, not a material cube map. Bind a separate
	// pre-BLEND snapshot; sampling the active SceneHDR render target is invalid.
	ComPtr<ID3D11ShaderResourceView> SceneColorSnapshot;
	if (42u == Resource.iSourceMaterialProfile || Resource.bSourceRequiresSceneColor)
	{
		SceneColorSnapshot = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneColor"));
		if (nullptr == SceneColorSnapshot)
			return Fail_RenderOperation("CubeSample scene-color snapshot is unavailable.", E_FAIL, true);
	}
	ComPtr<ID3D11ShaderResourceView> SourceSceneDepth;
	if (Resource.bSourceRequiresSceneDepth || (Resource.iSourceMaterialProfile == 324u) ||
		(Resource.iSourceMaterialProfile >= 44u && Resource.iSourceMaterialProfile <= 76u) ||
		(Resource.iSourceMaterialProfile >= 80u && Resource.iSourceMaterialProfile <= 205u) ||
		(Resource.iSourceMaterialProfile >= 208u && Resource.iSourceMaterialProfile <= 263u || (Resource.iSourceMaterialProfile >= 277u && Resource.iSourceMaterialProfile <= 280u)))
	{
		SourceSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		if (nullptr == SourceSceneDepth)
			return Fail_RenderOperation("Native Q mesh scene-depth input is unavailable.", E_FAIL, true);
	}
	hResult = pDrawShader->Bind_Texture("g_EffectSceneDepthTexture", SourceSceneDepth);
	if (FAILED(hResult))
		return Fail_RenderOperation("Native Q mesh scene-depth binding failed.", hResult, true);
	// Also clear the FX resource for every other mesh, so the prior Q binding
	// cannot become an implicit input of another material.
	hResult = pDrawShader->Bind_Texture("g_EffectSceneColorTexture", SceneColorSnapshot);
	if (FAILED(hResult))
		return Fail_RenderOperation("CubeSample scene-color shader binding failed.", hResult, true);

	const auto SceneBloomSnapshot = SceneColorSnapshot ?
		CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneBloom")) : nullptr;
	if ((SceneColorSnapshot && !SceneBloomSnapshot) ||
		FAILED(pDrawShader->Bind_Texture("g_EffectSceneBloomTexture", SceneBloomSnapshot)) ||
		FAILED(pDrawShader->Bind_Texture("g_EffectStartingSceneBloomTexture",
			Resource.iSourceMaterialProfile == 178u ? m_pStartingSceneBloomCapture : nullptr)))
		return Fail_RenderOperation("Mesh scene bloom snapshot binding failed.", E_FAIL, true);

	const ComPtr<ID3D11ShaderResourceView> BaseOverride =
		0u != Resource.iStandardColorV1Enabled ? Resource.SourceTextures[0u] :
		(Element.pElement->Detail.Mesh.bUseModelMaterial ? nullptr :
		 Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE));
	const uint32_t iUseBaseOverride = nullptr != BaseOverride ? 1u : 0u;
	hResult = pDrawShader->Bind_RawValue(
		"g_UseBaseOverride", &iUseBaseOverride, sizeof(iUseBaseOverride));
	if (FAILED(hResult))
	{
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_UseBaseOverride.", hResult);
	}
	bool_t bSubmitted = false;
	const uint32_t iMeshDrawCount = Instances.empty() ? Resource.pModel->Get_NumMeshes() : 1u;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Mesh.BindAndDraw");
	for (uint32_t iMesh = 0u; iMesh < iMeshDrawCount; ++iMesh)
	{
		if (iSourceMaterialIndex != UINT32_MAX)
		{
			uint32_t iActualSlot = 0u;
			if (!Resource.pModel->Try_GetSourceMaterialIndex(iMesh, iActualSlot))
				return Fail_RenderOperation("CModel source material index is invalid.", E_FAIL, true);
			if (iActualSlot != iSourceMaterialIndex)
				continue;
		}
		if (iUseBaseOverride)
		{
			hResult = pDrawShader->Bind_Texture("g_BaseTexture", BaseOverride);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Mesh base-override texture bind failed.", hResult);
		}
		else
		{
			hResult = Resource.pModel->Bind_Material(pDrawShader,
				"g_BaseTexture", iMesh, aiTextureType_DIFFUSE);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Mesh model diffuse-material bind failed.", hResult);
		}
		hResult = pDrawShader->Begin(iPass + (Instances.empty() ? 0u : 7u));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Mesh shader pass apply failed.", hResult);
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (nullptr != pMaterialProgramBinding &&
			!Validate_ActualMaterialAdapterPipeline(
				m_pContext.Get(), pMaterialProgramBinding->Adapter,
				iPass))
		{
			return Fail_RenderOperation(
				"Bound Mesh material adapter actual pass/state/MRT changed.",
				E_FAIL, true);
		}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (nullptr != pMaterialProgramBinding)
			Record_TestCompiledAdapterPipelineValidation();
		Record_TestShaderPassApplication();
#endif
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (0u != Resource.iRuntimeMaterialV2Enabled ||
			0u != Resource.iArtistVisualV4Opcode ||
			0u != Resource.iStandardColorV1Enabled)
		{
			const size_t iSamplerCount = static_cast<size_t>(
				0u != Resource.iStandardColorV1Enabled ?
					Resource.StandardColorV1Header[2u] :
				(0u != Resource.iRuntimeMaterialV2Enabled ?
					Resource.iRuntimeMaterialV2TextureLaneCount :
					std::popcount(Resource.iArtistVisualV4TextureMask)));
			if (iSamplerCount == 0u ||
				iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Mesh typed material sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			Record_TestSamplerBinding();
#endif
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestDrawSelection(
			EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL, iPass + (Instances.empty() ? 0u : 7u));
#endif
		{
			Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Mesh.DrawSubmission");
		if (Instances.empty())
            hResult = Resource.pModel->Render(iMesh);
        else if (iOrderedGeometryHandle != UINT32_MAX)
            hResult = Resource.pModel->Render_OrderedStaticGeometryInstanced(
                iOrderedGeometryHandle, m_pNativeMeshInstanceBuffer.Get(),
                sizeof(EFFECT_NATIVE_MESH_INSTANCE), static_cast<uint32_t>(Instances.size()),
                iInstanceByteOffset);
        else
            hResult = Resource.pModel->Render_Instanced(iMesh,
                m_pNativeMeshInstanceBuffer.Get(), sizeof(EFFECT_NATIVE_MESH_INSTANCE),
                static_cast<uint32_t>(Instances.size()), iInstanceByteOffset);
		}
		if (S_OK != hResult)
			return Fail_RenderOperation("Mesh model draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestVIBufferBinding();
        Record_TestIssuedDraw(World);
        if (nullptr != m_pActiveOccurrenceStats)
            for (const auto& Instance : Instances)
                Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
                    { Instance.World._41, Instance.World._42, Instance.World._43 });
#endif
		bSubmitted = true;
	}
	}
	return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CEffectDocumentRenderer::Render_Rect(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Rect.Render");
	if (nullptr == Element.pElement || nullptr == m_pRectShader ||
		nullptr == m_pRect)
		return Fail_RenderOperation(
			"Sprite element/shader/buffer contract is missing.",
			E_INVALIDARG, true);
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	float4x4_t World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	if (Element.pElement->Detail.Sprite.bBillboard)
		World = Make_BillboardWorld(World,
			Element.pElement->Detail.Sprite.fBillboardRollDegrees);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Sprite render-profile pass is invalid.", E_INVALIDARG, true);
	HRESULT hResult = m_pRectShader->Bind_Matrix("g_WorldMatrix", &World);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Sprite shader bind failed: g_WorldMatrix.", hResult);
	hResult = Bind_Common(m_pRectShader, Element, Resource, fAlphaScale);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Sprite common/material shader bind failed.", hResult);
	hResult = m_pRectShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation("Sprite shader pass apply failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if (0u != Resource.iRuntimeMaterialV2Enabled &&
		0u != Resource.iRuntimeMaterialV2TextureLaneCount)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			Resource.iRuntimeMaterialV2TextureLaneCount);
		if (iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Sprite Rect typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
	hResult = m_pRect->Bind_Resources();
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Sprite rectangle buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::SPRITE_RECT, iPass);
#endif
	hResult = m_pRect->Render();
	if (S_OK != hResult)
		return Fail_RenderOperation("Sprite rectangle draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestIssuedDraw(World);
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Decal(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Decal.Render");
	if (nullptr == Element.pElement || nullptr == m_pDecalShader ||
		nullptr == m_pRect)
		return Fail_RenderOperation("Decal element/shader/buffer contract is missing.",
			E_INVALIDARG, true);
	uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation("Decal render-profile pass is invalid.",
			E_INVALIDARG, true);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = Resource.pMaterialProgramBinding;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Resource.iStandardColorV1Enabled == 1u &&
			Resource.StandardColorV1Header[0u] == 1u &&
			Resource.StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			Resource.StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != Resource.iRuntimeMaterialV2Enabled &&
			Resource.iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Element.pElement->strElementId).get() !=
				pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR ||
			Element.pElement->eKind != EFFECT_ELEMENT_KIND::DECAL ||
			nullptr != Find_Binding(
				*Element.pElement, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			nullptr != Resource.pModel ||
			Element.pElement->Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			iPass != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
					SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound LocalDecal material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	const matrix_t World = XMLoadFloat4x4(&Element.World);
	const HRESULT WorldStatus = Validate_DecalProjectionWorld(Element);
	if (S_OK != WorldStatus)
		return FAILED(WorldStatus) ? Fail_RenderOperation(
			"Decal projection world is invalid.", WorldStatus, true) : WorldStatus;
	float4x4_t InverseDecal{};
	const matrix_t Inverse = XMMatrixInverse(nullptr, World);
	XMStoreFloat4x4(&InverseDecal, Inverse);
	EFFECT_DECAL_SHADER_PROJECTION_DESC Projection{};
	if (!Resolve_DecalShaderProjection(Element, Projection))
		return Fail_RenderOperation("Decal shader projection is invalid.",
			E_INVALIDARG, true);
	HRESULT hResult = Bind_MaterialInputs(m_pDecalShader,
		*Element.pElement, Element.Color,
		Element.fLocalTimeSeconds, Element.fNormalizedLife, Resource);
	if (FAILED(hResult))
		return Fail_RenderOperation("Decal material shader bind failed.", hResult);
    if (Resource.iSourceMaterialProfile >= 2304u && Resource.iSourceMaterialProfile <= 3711u &&
        FAILED(m_pDecalShader->Bind_RawValue("g_KoukuDecalProjection",
            &Projection.vSourceProjection, sizeof(Projection.vSourceProjection))))
        return Fail_RenderOperation("Kouku source decal plane binding failed.", E_FAIL);
	hResult = m_pDecalShader->Bind_Matrix(
		"g_DecalWorldInverse", &InverseDecal);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalWorldInverse.", hResult);
	hResult = m_pDecalShader->Bind_Matrix("g_ViewMatrixInverse",
		CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_ViewMatrixInverse.", hResult);
	hResult = m_pDecalShader->Bind_Matrix("g_ProjMatrixInverse",
		CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_ProjMatrixInverse.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalSize", &Projection.vSize, sizeof(Projection.vSize));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalSize.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalDepth", &Projection.fDepth, sizeof(Projection.fDepth));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalDepth.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalEdgeFade", &Projection.fEdgeFade,
		sizeof(Projection.fEdgeFade));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalEdgeFade.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalUp", &Projection.vUp, sizeof(Projection.vUp));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalUp.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalNormalCutoff", &Projection.fNormalCutoff,
		sizeof(Projection.fNormalCutoff));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalNormalCutoff.", hResult);
	hResult = CGameInstance::Get().Bind_RT_SRV(
		TEXT("Target_Depth"), m_pDecalShader, "g_DepthTexture");
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal depth render-target bind failed.", hResult);
	hResult = CGameInstance::Get().Bind_RT_SRV(
		TEXT("Target_Normal"), m_pDecalShader, "g_NormalTexture");
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal normal render-target bind failed.", hResult);
	hResult = m_pDecalShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation("Decal shader pass apply failed.", hResult);
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	if (nullptr != pMaterialProgramBinding &&
		!Validate_ActualMaterialAdapterPipeline(
			m_pContext.Get(), pMaterialProgramBinding->Adapter, iPass))
	{
		return Fail_RenderOperation(
			"Bound LocalDecal material adapter actual pass/state/MRT changed.",
			E_FAIL, true);
	}
	if (nullptr != pMaterialProgramBinding &&
		pMaterialProgramBinding->Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL &&
		!Validate_ActualLocalDecalSceneShaderResources(m_pContext.Get()))
	{
		return Fail_RenderOperation(
			"Bound LocalDecal material adapter actual depth/normal SRV changed.",
			E_FAIL, true);
	}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	if (nullptr != pMaterialProgramBinding)
		Record_TestCompiledAdapterPipelineValidation();
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if ((0u != Resource.iRuntimeMaterialV2Enabled &&
		 0u != Resource.iRuntimeMaterialV2TextureLaneCount) ||
		0u != Resource.iStandardColorV1Enabled)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			0u != Resource.iStandardColorV1Enabled ?
				Resource.StandardColorV1Header[2u] :
				Resource.iRuntimeMaterialV2TextureLaneCount);
		if (iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Decal typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
	hResult = m_pRect->Bind_Resources();
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Decal rectangle buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::DECAL_RECT, iPass);
#endif
	hResult = m_pRect->Render();
	if (S_OK != hResult)
		return Fail_RenderOperation("Decal rectangle draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestIssuedDraw(Element.World);
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Element(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement)
		return Fail_RenderOperation(
			"Effect element descriptor is missing.", E_INVALIDARG, true);
	if (Resource.bOccurrenceVisualSuppressed)
		return S_FALSE;
	switch (Element.pElement->eKind)
	{
	case EFFECT_ELEMENT_KIND::MESH:
		return Render_Mesh(Element, Resource);
	case EFFECT_ELEMENT_KIND::SPRITE:
		return Render_Rect(Element, Resource);
	case EFFECT_ELEMENT_KIND::DECAL:
		return Render_Decal(Element, Resource);
	case EFFECT_ELEMENT_KIND::PARTICLE:
	case EFFECT_ELEMENT_KIND::TRAIL:
	case EFFECT_ELEMENT_KIND::LIGHT:
	case EFFECT_ELEMENT_KIND::SCREEN_POST:
		return S_FALSE;
	case EFFECT_ELEMENT_KIND::END:
	default:
		return Fail_RenderOperation(
			"Effect element kind is invalid for rendering.", E_INVALIDARG, true);
	}
}
