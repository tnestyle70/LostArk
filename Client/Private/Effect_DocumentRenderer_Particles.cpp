#include "Effect_DocumentRenderer_Internal.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "Render_OutputContract.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
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
#include "Model.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_Rect.h"

HRESULT Client::CEffectDocumentRenderer::Render_AfterImages(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_AFTERIMAGE> AfterImages)
{
	bool_t bSubmitted = false;
	for (const EFFECT_EVALUATED_AFTERIMAGE& AfterImage : AfterImages)
	{
		if (nullptr == AfterImage.pElement)
			return Fail_RenderOperation(
				"Afterimage element contract is missing.", E_INVALIDARG, true);
		if (!std::isfinite(AfterImage.fAlpha) || AfterImage.fAlpha < 0.f ||
			AfterImage.fAlpha > 1.f)
		{
			return Fail_RenderOperation(
				"Afterimage alpha contract is invalid.", E_INVALIDARG, true);
		}
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(AfterImage.pElement->strElementId);
		if (nullptr == pResource)
			return Fail_RenderOperation(
				"Afterimage resource contract is missing.", E_FAIL, true);
		if (pResource->bSourceMaterialFallbackBlocked ||
			pResource->bOccurrenceVisualSuppressed)
			continue;
		EFFECT_EVALUATED_ELEMENT Element;
		Element.pElement = AfterImage.pElement;
		Element.World = AfterImage.World;
		Element.Color = AfterImage.pElement->Detail.Color;
		Element.fLocalTimeSeconds = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			AfterImage.pElement->Detail.Timing.fStartDelaySeconds);
		Element.fNormalizedLife = 1.f - AfterImage.fAlpha;
		HRESULT Result = E_INVALIDARG;
		switch (Element.pElement->eKind)
		{
		case EFFECT_ELEMENT_KIND::MESH:
			Result = Render_Mesh(
				Element, *pResource, AfterImage.fAlpha, &AfterImage.World);
			break;
		case EFFECT_ELEMENT_KIND::SPRITE:
			Result = Render_Rect(
				Element, *pResource, AfterImage.fAlpha, &AfterImage.World);
			break;
		default:
			return Fail_RenderOperation(
				"Afterimage renderer kind is not Mesh or Sprite.",
				E_INVALIDARG, true);
		}
		if (FAILED(Result))
			return Result;
		if (S_OK == Result)
			bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CEffectDocumentRenderer::Try_RenderNativeMeshParticles(
    const EFFECT_EVALUATED_FRAME& Frame,
    const std::span<const EFFECT_EVALUATED_PARTICLE> Particles,
    const ELEMENT_RESOURCE& Resource, bool_t& bOutHandled)
{
    bOutHandled = false;
    const EFFECT_SHADER_PROGRAM_DESC* pProgram = nullptr;
    const auto Shader = Resolve_DrawShader(Resource, EFFECT_SHADER_CARRIER::MESH, pProgram);
    if (nullptr == Shader || nullptr == pProgram ||
        pProgram->eFamily == EFFECT_SHADER_FAMILY::GENERIC ||
        nullptr == Resource.pModel || Resource.pModel->Is_Skinned() ||
        !Resource.SourceMaterialSlots.empty() || nullptr != Resource.pMaterialProgramBinding ||
        Resource.pModel->Get_NumMeshes() == 0u)
        return S_FALSE;
    const EFFECT_ELEMENT_DESC* pSource = nullptr;
    for (const auto& Particle : Particles)
    {
        if (nullptr == Particle.pElement) continue;
        if (nullptr == pSource) pSource = Particle.pElement;
        else if (Particle.pElement != pSource) return S_FALSE;
    }
    if (nullptr == pSource) return S_FALSE;
    const uint32_t iNominalPass = Select_Pass(pSource->Material.eRenderProfile);
    if (iNominalPass == UINT32_MAX) return S_FALSE;

    uint32_t iOrderedGeometryHandle = UINT32_MAX;
    if (Resource.pModel->Get_NumMeshes() > 1u)
    {
        // Native families sample SourceTextures, never the slot-specific g_BaseTexture.
        // With no per-slot overrides their effective material is identical. Joining
        // original vertex/index sequences preserves p0m0,p0m1,p1m0,p1m1 blend order.
        const HRESULT hPrepared = Resource.pModel->Prepare_OrderedStaticGeometry(
            0u, Resource.pModel->Get_NumMeshes(), iOrderedGeometryHandle);
        if (hPrepared == S_FALSE) return S_FALSE;
        if (FAILED(hPrepared))
            return Fail_RenderOperation("Native mesh ordered geometry preparation failed.", hPrepared, true);
    }
    bOutHandled = true;
    const auto& Profile = pSource->Material.SourceMaterial.strRuntimeShaderProfileId;
    const bool_t bMirrorAware = Find_ArtistProgram(Profile) ||
        Find_WarlordNativeProgram(Profile) || Find_LanceMasterVAProgram(Profile) ||
        Resource.iSourceMaterialProfile == 42u || Resource.iSourceMaterialProfile == 50u ||
        Resource.iSourceMaterialProfile == 60u || Resource.iSourceMaterialProfile == 66u ||
        Resource.iSourceMaterialProfile == 70u;
    const bool_t bLockedAxisMeshFacing = Uses_SourceLockedAxisMeshFacing(*pSource);
    const SOURCE_SUBUV_LAYOUT SubUVLayout = Resolve_SourceSubUVLayout(*pSource);
    auto& Instances = m_NativeMeshInstanceScratch;
    auto& Passes = m_NativeMeshPassScratch;
    Instances.clear();
    Passes.clear();
    Instances.reserve(Particles.size());
    Passes.reserve(Particles.size());
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Mesh.InstanceBuild");
    for (const auto& Particle : Particles)
    {
        if (nullptr == Particle.pElement) continue;
        const EFFECT_SUBUV_FRAME_DESC SourceSubUV = Resolve_SubUVFrames(Particle, SubUVLayout);
        EFFECT_SUBUV_FRAME_DESC SubUV;
        if (!CEffectPlayback::Resolve_ParticleSpriteSubUV(Particle, SourceSubUV, SubUV))
            return Fail_RenderOperation("Mesh particle source SubUV contract is invalid.", E_INVALIDARG, true);
        EFFECT_NATIVE_MESH_INSTANCE Instance{};
        Instance.World = Apply_ParticleCameraOffset(Particle);
        if (bLockedAxisMeshFacing && !Make_SourceLockedAxisMeshWorld(Particle, Instance.World))
            return Fail_RenderOperation("Source locked-axis mesh facing is invalid.", E_INVALIDARG, true);
        Apply_StartingCaptureCameraFraming(Get_StagedDocument(), Particle, *Resource.pModel,
            Frame.fSampleTimeSeconds, Instance.World);
        const matrix_t World = XMLoadFloat4x4(&Instance.World);
        const f32_t fDeterminant = XMVectorGetX(XMMatrixDeterminant(World));
        if (!std::isfinite(fDeterminant))
            return Fail_RenderOperation("Mesh world determinant is non-finite.", E_INVALIDARG, true);
        // Match Render_Mesh's valid zero-size suppression before inverting.
        if (std::abs(fDeterminant) <= std::numeric_limits<f32_t>::epsilon()) continue;
        XMStoreFloat4x4(&Instance.NormalMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, World)));
        Instance.Color = Particle.Color;
        Instance.DynamicParameter = Particle.vDynamicParameter;
        Instance.SubUVCurrent = SubUV.Current;
        Instance.SubUVNext = SubUV.Next;
        // Native mesh VS does not consume the sprite atlas unless StandardColorV1
        // is explicitly selected. Keep that same gate and its zero blend value.
        Instance.LifeBlend = { Particle.fNormalizedLife,
            Resource.iStandardColorV1Enabled != 0u ? SubUV.fBlend : 0.f };
        uint32_t iPass = iNominalPass;
        if (bMirrorAware && fDeterminant < 0.f)
        {
            if (iPass == 3u) iPass = 5u;
            else if (iPass == 4u) iPass = 6u;
        }
        Instances.push_back(Instance);
        Passes.push_back(iPass);
    }
    }
    if (Instances.empty()) return S_FALSE;
    if (Instances.size() > UINT32_MAX / sizeof(EFFECT_NATIVE_MESH_INSTANCE))
        return Fail_RenderOperation("Native mesh instance upload is too large.", E_INVALIDARG, true);
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Mesh.InstanceUpload");
    if (Instances.size() > m_iNativeMeshInstanceCapacity)
    {
        const uint32_t Capacity = (std::min)(
            std::bit_ceil((std::max)(256u, static_cast<uint32_t>(Instances.size()))),
            static_cast<uint32_t>(UINT32_MAX / sizeof(EFFECT_NATIVE_MESH_INSTANCE)));
        D3D11_BUFFER_DESC Desc{};
        Desc.ByteWidth = Capacity * sizeof(EFFECT_NATIVE_MESH_INSTANCE);
        Desc.Usage = D3D11_USAGE_DYNAMIC;
        Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ComPtr<ID3D11Buffer> Staged;
        const HRESULT Result = m_pDevice->CreateBuffer(&Desc, nullptr, &Staged);
        if (FAILED(Result))
            return Fail_RenderOperation("Native mesh instance buffer creation failed.", Result);
        m_pNativeMeshInstanceBuffer = std::move(Staged);
        m_iNativeMeshInstanceCapacity = Capacity;
    }
    D3D11_MAPPED_SUBRESOURCE Mapped{};
    const HRESULT hMapped = m_pContext->Map(m_pNativeMeshInstanceBuffer.Get(),
        0u, D3D11_MAP_WRITE_DISCARD, 0u, &Mapped);
    if (FAILED(hMapped))
        return Fail_RenderOperation("Native mesh instance upload failed.", hMapped);
    std::memcpy(Mapped.pData, Instances.data(), Instances.size() * sizeof(EFFECT_NATIVE_MESH_INSTANCE));
    m_pContext->Unmap(m_pNativeMeshInstanceBuffer.Get(), 0u);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
    Record_TestGeometryUpload();
#endif
    }
    bool_t bSubmitted = false;
    for (size_t Begin = 0u; Begin < Instances.size();)
    {
        size_t End = Begin + 1u;
        while (End < Instances.size() && Passes[End] == Passes[Begin]) ++End;
        const auto& First = Instances[Begin];
        EFFECT_EVALUATED_ELEMENT Element;
        Element.pElement = pSource;
        Element.World = First.World;
        Element.Color = pSource->Detail.Color;
        Element.Color.vColorMultiply = First.Color;
        Element.fLocalTimeSeconds = (std::max)(0.f,
            Frame.fSampleTimeSeconds - pSource->Detail.Timing.fStartDelaySeconds);
        Element.fNormalizedLife = First.LifeBlend.x;
        EFFECT_SUBUV_FRAME_DESC SubUV;
        SubUV.Current = First.SubUVCurrent;
        SubUV.Next = First.SubUVNext;
        SubUV.fBlend = First.LifeBlend.y;
        const HRESULT Result = Render_Mesh(Element, Resource, 1.f, nullptr,
            &First.DynamicParameter, &SubUV, nullptr, UINT32_MAX,
            std::span<const EFFECT_NATIVE_MESH_INSTANCE>(Instances).subspan(Begin, End - Begin),
            static_cast<uint32_t>(Begin * sizeof(EFFECT_NATIVE_MESH_INSTANCE)), iOrderedGeometryHandle);
        if (FAILED(Result)) return Result;
        bSubmitted |= Result == S_OK;
        Begin = End;
    }
    return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CEffectDocumentRenderer::Render_Particles(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_PARTICLE> Particles)
{
	const EFFECT_ELEMENT_DESC* pSource = nullptr;
	for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
	{
		if (nullptr != Particle.pElement)
		{
			pSource = Particle.pElement;
			break;
		}
	}
	if (nullptr == pSource)
		return S_FALSE;
	Engine::CProfilerScope particleRenderProfile(
		CGameInstance::Get().Get_Profiler(), "Effect.Particle.Render");
	const ELEMENT_RESOURCE* pResource = Find_Resource(pSource->strElementId);
	if (nullptr == pResource)
		return Fail_RenderOperation(
			"Particle resource contract is missing.", E_FAIL, true);
	if (pResource->bSourceMaterialFallbackBlocked ||
		pResource->bOccurrenceVisualSuppressed)
		return S_FALSE;
	const bool_t bTypedDirectSmoke =
		0u != pResource->iRuntimeMaterialV2Enabled &&
		10u == pResource->iRuntimeMaterialV2Opcode &&
		pResource->iRuntimeMaterialV2TextureLaneCount == 1u &&
		pResource->iRuntimeMaterialV2TextureMask == 0x01u;
	if (pSource->Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
		!pSource->Material.SourceMaterial.bEnabled && !bTypedDirectSmoke)
	{
		// Version 10 and older source-material documents are intentionally
		// fail-closed.  They remain loadable for migration, but must not turn
		// into a white fallback particle after the v11 renderer is enabled.
		return S_FALSE;
	}
	const SOURCE_SUBUV_LAYOUT SubUVLayout = Resolve_SourceSubUVLayout(*pSource);
	if (nullptr != pResource->pModel)
	{
        bool_t bNativeBatchHandled = false;
        const HRESULT hNativeBatch = Try_RenderNativeMeshParticles(
            Frame, Particles, *pResource, bNativeBatchHandled);
        if (FAILED(hNativeBatch) || bNativeBatchHandled)
            return hNativeBatch;
		const bool_t bLockedAxisMeshFacing = Uses_SourceLockedAxisMeshFacing(*pSource);
		bool_t bSubmitted = false;
		for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
		{
			if (nullptr == Particle.pElement)
				continue;
			EFFECT_EVALUATED_ELEMENT MeshParticle;
			MeshParticle.pElement = Particle.pElement;
			MeshParticle.World = Apply_ParticleCameraOffset(Particle);
			if (bLockedAxisMeshFacing && !Make_SourceLockedAxisMeshWorld(Particle, MeshParticle.World))
				return Fail_RenderOperation("Source locked-axis mesh facing is invalid.", E_INVALIDARG, true);
            Apply_StartingCaptureCameraFraming(Get_StagedDocument(), Particle, *pResource->pModel,
                Frame.fSampleTimeSeconds, MeshParticle.World);
			MeshParticle.Color = Particle.pElement->Detail.Color;
			MeshParticle.Color.vColorMultiply = Particle.Color;
			MeshParticle.fLocalTimeSeconds = (std::max)(0.f,
				Frame.fSampleTimeSeconds -
				Particle.pElement->Detail.Timing.fStartDelaySeconds);
			MeshParticle.fNormalizedLife = Particle.fNormalizedLife;
			const EFFECT_SUBUV_FRAME_DESC SourceSubUV =
				Resolve_SubUVFrames(Particle, SubUVLayout);
			EFFECT_SUBUV_FRAME_DESC SubUV;
			if (!CEffectPlayback::Resolve_ParticleSpriteSubUV(
					Particle, SourceSubUV, SubUV))
			{
				return Fail_RenderOperation(
					"Mesh particle source SubUV contract is invalid.",
					E_INVALIDARG, true);
			}
			const HRESULT Result = Render_Mesh(MeshParticle, *pResource,
				1.f, nullptr, &Particle.vDynamicParameter, &SubUV);
			if (FAILED(Result))
				return Result;
			bSubmitted = bSubmitted || S_OK == Result;
		}
		return bSubmitted ? S_OK : S_FALSE;
	}
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram = nullptr;
    const shared_ptr<Engine::CShader> pDrawShader = Resolve_DrawShader(
        *pResource, EFFECT_SHADER_CARRIER::PARTICLE, pShaderProgram);
    if (nullptr == pDrawShader)
        return Fail_RenderOperation("Prepared particle shader family is unavailable.", E_FAIL, true);
	// These admitted native programs use the unexpanded four-vertex rect in
	// Shader_EffectParticleFamilyCarrier, with GeometryShader = NULL.
	// Use the selected executable identity; authored native flags are insufficient.
	PARTICLE_CLIP_CONTEXT ClipContext;
	if (nullptr != pShaderProgram &&
		pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE &&
		pShaderProgram->eFamily == EFFECT_SHADER_FAMILY::ARTIST &&
		pShaderProgram == Get_EffectShaderProgram(pResource->iShaderProgramIndex) &&
		pResource->iShaderProgramIndex < m_ShaderPrograms.size() &&
		pDrawShader == m_ShaderPrograms[pResource->iShaderProgramIndex] &&
		pResource->iSourceMaterialProfile >= pShaderProgram->iFirstProfile &&
		pResource->iSourceMaterialProfile <= pShaderProgram->iLastProfile)
	{
		const float4x4_t* View = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
		const float4x4_t* Projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
		if (nullptr != View && nullptr != Projection)
		{
			ClipContext.View = XMLoadFloat4x4(View);
			ClipContext.Projection = XMLoadFloat4x4(Projection);
			matrix_t MagnitudeView, MagnitudeProjection;
			for (uint32_t Row = 0u; Row < 4u; ++Row)
			{
				MagnitudeView.r[Row] = XMVectorAbs(ClipContext.View.r[Row]);
				MagnitudeProjection.r[Row] = XMVectorAbs(ClipContext.Projection.r[Row]);
			}
			ClipContext.MagnitudeViewProjection = MagnitudeView * MagnitudeProjection;
			ClipContext.bEnabled = true;
		}
	}
	std::vector<SOURCE_SPRITE_DEPTH_ORDER> SourceDepthOrder;
	const bool_t bSortSourceDepth = Requires_SourceSpriteDepthSort(*pSource);
	if (bSortSourceDepth)
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Sprite.DepthSort");
		const float4x4_t* pView = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
		const float4x4_t* pProjection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
		if (nullptr == pView || nullptr == pProjection)
			return Fail_RenderOperation(
				"Source sprite depth sort camera matrices are unavailable.", E_FAIL, true);
		std::string SortError;
		if (!Build_SourceSpriteDepthOrder(
			Particles, *pSource, *pView, *pProjection, SourceDepthOrder, SortError))
			return Fail_RenderOperation(std::move(SortError), E_INVALIDARG, true);
	}
	std::vector<Engine::VTXEFFECT_PARTICLE>& Instances =
		m_ParticleInstanceScratch;
	Instances.clear();
	if (Instances.capacity() < Particles.size())
		Instances.reserve(Particles.size());
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Sprite.InstanceBuild");
	PARTICLE_SPRITE_WORLD_CONTEXT SpriteWorldContext;
	for (size_t iParticle = 0u; iParticle < Particles.size(); ++iParticle)
	{
		const EFFECT_EVALUATED_PARTICLE& Particle = bSortSourceDepth ?
			*SourceDepthOrder[iParticle].pParticle : Particles[iParticle];
		if (nullptr == Particle.pElement)
			continue;
		if (0u != pResource->iRuntimeMaterialV2Enabled &&
			4u == pResource->iRuntimeMaterialV2Opcode)
		{
			const float4_t& Color = Particle.Color;
			const float4_t& Dynamic = Particle.vDynamicParameter;
			if (!std::isfinite(Color.x) || !std::isfinite(Color.y) ||
				!std::isfinite(Color.z) || !std::isfinite(Color.w) ||
				!std::isfinite(Dynamic.x) || !std::isfinite(Dynamic.y) ||
				!std::isfinite(Dynamic.z) || !std::isfinite(Dynamic.w))
			{
				return Fail_RenderOperation(
					"Particle RuntimeMaterialV2 color/dynamic input is non-finite.",
					E_INVALIDARG, true);
			}
			// active016's bounded opacity is the product of the exact named
			// alpha-dissolve lane and the particle alpha carrier.  Suppress the
			// deterministic zero envelope before issuing a zero-pixel draw.
			if (Color.w <= 0.f || Dynamic.x <= 0.f)
				continue;
		}
		pSource = Particle.pElement;
		float4x4_t World = Particle.World;
		if (Particle.pElement->Detail.Particle.bBillboard &&
			!Make_ParticleSpriteWorld(Particle, World, pResource->iSourceMaterialProfile, SpriteWorldContext))
		{
			return Fail_RenderOperation(
				"Particle billboard world reconstruction failed.",
				E_INVALIDARG, true);
		}
		const EFFECT_SUBUV_FRAME_DESC SourceSubUV =
			Resolve_SubUVFrames(Particle, SubUVLayout);
		EFFECT_SUBUV_FRAME_DESC SubUV;
		if (!CEffectPlayback::Resolve_ParticleSpriteSubUV(
			Particle, SourceSubUV, SubUV))
		{
			return Fail_RenderOperation(
				"Particle source image-flip contract is invalid.",
				E_INVALIDARG, true);
		}
		// Keep source/UV validation above even for an offscreen quad. Camera
		// facing is already final; no color/opacity/size heuristic participates.
		if (Is_ParticleQuadOutsideViewXY(World, ClipContext))
			continue;
		Instances.push_back({ World, Particle.Color,
			Particle.vDynamicParameter,
			SubUV.Current,
			SubUV.Next,
			{ Particle.fNormalizedLife, SubUV.fBlend } });
	}
	}
	if (nullptr == pSource || Instances.empty())
		return S_FALSE;

	const EFFECT_ELEMENT_DESC& Source = *pSource;
	HRESULT hResult;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Sprite.InstanceUpload");
	hResult = m_pParticleBuffer->Update_Instances(
		std::span<const Engine::VTXEFFECT_PARTICLE>(
			Instances.data(), Instances.size()));
	}
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle instance-buffer update failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestGeometryUpload();
#endif
	const f32_t LocalTime = (std::max)(0.f,
		Frame.fSampleTimeSeconds - Source.Detail.Timing.fStartDelaySeconds);
	const f32_t Normalized = std::clamp(
		LocalTime / Source.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
	EFFECT_COLOR_DESC CommonColor =
		Evaluate_CommonColor(Source, Normalized);
	CommonColor.vColorMultiply = float4_t(1.f, 1.f, 1.f, 1.f);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = pResource->pMaterialProgramBinding;
	uint32_t iPass = UINT32_MAX;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			pResource->iStandardColorV1Enabled == 1u &&
			pResource->StandardColorV1Header[0u] == 1u &&
			pResource->StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			pResource->StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != pResource->iRuntimeMaterialV2Enabled &&
			pResource->iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Source.strElementId).get() != pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE ||
			Source.eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			!Source.SourceRecipe.bEnabled ||
			Source.SourceRecipe.strRendererShape != "sprite" ||
			nullptr != Find_Binding(Source, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			nullptr != pResource->pModel || nullptr == pDrawShader ||
			nullptr == m_pParticleBuffer ||
			Source.Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			Select_Pass(Source.Material.eRenderProfile) != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
				SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound Sprite material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	if (341u == pResource->iSourceMaterialProfile || 361u == pResource->iSourceMaterialProfile ||
		2349u == pResource->iSourceMaterialProfile)
	{
		// Native FMaterialShaderParameters binds BatchElement.WorldToLocal.
		// The batch uses emitter space only for local-space particles; Playback
		// carries that exact transform separately from the billboard instance.
		const matrix_t EmitterWorld = XMLoadFloat4x4(&Particles.front().SourceEmitterWorld);
		const f32_t Determinant = XMVectorGetX(XMMatrixDeterminant(EmitterWorld));
		if (!std::isfinite(Determinant) || std::abs(Determinant) <= 0.00000001f)
			return Fail_RenderOperation("Native particle emitter transform is singular.", E_INVALIDARG, true);
		const matrix_t ClientToSource = XMMatrixRotationX(XM_PIDIV2);
		float4x4_t SourceInverse{};
		XMStoreFloat4x4(&SourceInverse, XMMatrixTranspose(ClientToSource) *
			XMMatrixInverse(nullptr, EmitterWorld) * ClientToSource);
		const f32_t* Values = &SourceInverse._11;
		if (!std::all_of(Values, Values + 12u, [](f32_t Value) { return std::isfinite(Value); }))
			return Fail_RenderOperation("Native particle inverse is non-finite.", E_INVALIDARG, true);
		const char* const WorldToLocalUniform = 2349u == pResource->iSourceMaterialProfile ?
			"g_ArtistSourceWorldToLocal" : "g_SDSourceWorldToLocal";
		hResult = pDrawShader->Bind_RawValue(WorldToLocalUniform, Values, sizeof(float4_t) * 3u);
		if (FAILED(hResult))
			return Fail_RenderOperation("Native particle WorldToLocal binding failed.", hResult, true);
	}
	if (2360u == pResource->iSourceMaterialProfile)
	{
		// ParticleMacroUV is centred on the source ParticleSystem occurrence,
		// even when its already-spawned particles simulate in world space.
		const auto& Macro = Particles.front();
		const auto* View = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
		const auto* Projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
		if (!Macro.bSourceMacroUV || !View || !Projection ||
			!std::isfinite(Macro.fSourceMacroUVWorldRadius) || Macro.fSourceMacroUVWorldRadius <= 0.f)
			return Fail_RenderOperation("Source ParticleMacroUV occurrence input is unavailable.", E_INVALIDARG, true);
		const vector_t Centre = XMVectorSet(Macro.vSourceMacroUVWorldCenter.x,
			Macro.vSourceMacroUVWorldCenter.y, Macro.vSourceMacroUVWorldCenter.z, 1.f);
		const vector_t ViewCentre = XMVector4Transform(Centre, XMLoadFloat4x4(View));
		const matrix_t Project = XMLoadFloat4x4(Projection);
		const vector_t Clip = XMVector4Transform(ViewCentre, Project);
		const vector_t Right = XMVector4Transform(ViewCentre +
			XMVectorSet(Macro.fSourceMacroUVWorldRadius, 0.f, 0.f, 0.f), Project);
		const vector_t Up = XMVector4Transform(ViewCentre +
			XMVectorSet(0.f, Macro.fSourceMacroUVWorldRadius, 0.f, 0.f), Project);
		const f32_t W = XMVectorGetW(Clip), RightW = XMVectorGetW(Right), UpW = XMVectorGetW(Up);
		if (!std::isfinite(W) || !std::isfinite(RightW) || !std::isfinite(UpW))
			return Fail_RenderOperation("Source ParticleMacroUV projection is non-finite.", E_INVALIDARG, true);
		// A centre on the camera plane has no finite screen-space UV frame.
		// Skip this draw until it projects; do not poison the remaining Effect.
		if (std::abs(W) < 0.000001f || std::abs(RightW) < 0.000001f || std::abs(UpW) < 0.000001f)
			return S_FALSE;
		const f32_t X = XMVectorGetX(Clip) / W, Y = XMVectorGetY(Clip) / W;
		const f32_t RadiusX = std::abs(XMVectorGetX(Right) / RightW - X);
		const f32_t RadiusY = std::abs(XMVectorGetY(Up) / UpW - Y);
		if (!std::isfinite(RadiusX) || !std::isfinite(RadiusY) || RadiusX < 0.000001f || RadiusY < 0.000001f)
			return Fail_RenderOperation("Source ParticleMacroUV projected radius is invalid.", E_INVALIDARG, true);
		// Native PS adds 0.5 after this scale. NDC is +Y up; texture V is down.
		const float4_t MacroUV = { X, Y, 0.5f / RadiusX, -0.5f / RadiusY };
		hResult = pDrawShader->Bind_RawValue("g_ArtistSourceMacroUV", &MacroUV, sizeof(MacroUV));
		if (FAILED(hResult))
			return Fail_RenderOperation("Source ParticleMacroUV binding failed.", hResult, true);
	}
	hResult = Bind_Common(pDrawShader, Source, CommonColor,
		LocalTime, Normalized, *pResource, 1.f, nullptr, pShaderProgram);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle common/material shader bind failed.", hResult);
	ComPtr<ID3D11ShaderResourceView> SourceSceneDepth;
	if (pResource->bSourceRequiresSceneDepth ||
		(pResource->iSourceMaterialProfile >= 320u && pResource->iSourceMaterialProfile <= 323u) ||
		43u == pResource->iSourceMaterialProfile ||
		(pResource->iSourceMaterialProfile >= 44u && pResource->iSourceMaterialProfile <= 76u) ||
		(pResource->iSourceMaterialProfile >= 80u && pResource->iSourceMaterialProfile <= 205u) ||
		(pResource->iSourceMaterialProfile >= 208u && pResource->iSourceMaterialProfile <= 263u || (pResource->iSourceMaterialProfile >= 277u && pResource->iSourceMaterialProfile <= 280u)))
	{
		SourceSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		if (nullptr == SourceSceneDepth)
			return Fail_RenderOperation("Slice scene-depth input is unavailable.", E_FAIL, true);
	}
	ComPtr<ID3D11ShaderResourceView> SourceSceneColor;
	if (69u == pResource->iSourceMaterialProfile || pResource->bSourceRequiresSceneColor)
	{
		SourceSceneColor = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneColor"));
		if (nullptr == SourceSceneColor)
			return Fail_RenderOperation("Native V sprite scene-color input is unavailable.", E_FAIL, true);
	}
	hResult = pDrawShader->Bind_Texture("g_EffectSceneColorTexture", SourceSceneColor);
	if (FAILED(hResult))
		return Fail_RenderOperation("Native V sprite scene-color binding failed.", hResult, true);
	const auto SourceSceneBloom = SourceSceneColor ?
		CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneBloom")) : nullptr;
	if ((SourceSceneColor && !SourceSceneBloom) ||
		FAILED(pDrawShader->Bind_Texture("g_EffectSceneBloomTexture", SourceSceneBloom)))
		return Fail_RenderOperation("Native sprite scene bloom binding failed.", E_FAIL, true);
	// Clear the FX resource on other sprite families as well.
	hResult = pDrawShader->Bind_Texture("g_EffectSceneDepthTexture", SourceSceneDepth);
	if (FAILED(hResult))
		return Fail_RenderOperation("Slice scene-depth shader binding failed.", hResult, true);
	if (nullptr == pMaterialProgramBinding)
		iPass = Select_Pass(Source.Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Particle render-profile pass is invalid.", E_INVALIDARG, true);
	hResult = pDrawShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle shader pass apply failed.", hResult);
	if (nullptr != pMaterialProgramBinding)
	{
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (!Validate_ActualMaterialAdapterPipeline(
				m_pContext.Get(), pMaterialProgramBinding->Adapter, iPass))
		{
			return Fail_RenderOperation(
				"Bound Sprite material adapter actual pass/state/MRT changed.",
				E_FAIL, true);
		}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestCompiledAdapterPipelineValidation();
#endif
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if ((0u != pResource->iRuntimeMaterialV2Enabled &&
		0u != pResource->iRuntimeMaterialV2TextureLaneCount) ||
		0u != pResource->iArtistVisualV4Opcode ||
		0u != pResource->iStandardColorV1Enabled)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			0u != pResource->iStandardColorV1Enabled ?
				pResource->StandardColorV1Header[2u] :
			(0u != pResource->iRuntimeMaterialV2Enabled ?
				pResource->iRuntimeMaterialV2TextureLaneCount :
				std::popcount(pResource->iArtistVisualV4TextureMask)));
		if (iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Particle typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE, iPass);
#endif
	std::optional<CReconstructedPipelineStateGuard> NativeOneLayerGuard;
	if (nullptr != pResource->pNativeOneLayerBlend)
	{
		NativeOneLayerGuard.emplace(m_pContext.Get());
		const FLOAT BlendFactor[4]{};
		m_pContext->OMSetBlendState(pResource->pNativeOneLayerBlend.Get(), BlendFactor, 0xffffffffu);
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Sprite.DrawSubmission");
		hResult = m_pParticleBuffer->Render();
	}
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Particle instance-buffer draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestIssuedDraw(std::span<const Engine::VTXEFFECT_PARTICLE>(
		Instances.data(), Instances.size()));
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Trails(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_TRAIL> Trails)
{
	if (Trails.empty())
		return S_FALSE;
	Engine::CProfilerScope trailRenderProfile(
		CGameInstance::Get().Get_Profiler(), "Effect.Trail.Render");
	bool_t bSubmitted = false;
	const vector_t CameraPosition = XMLoadFloat4(
		CGameInstance::Get().Get_CamPosition());
	for (const EFFECT_EVALUATED_TRAIL& Trail : Trails)
	{
		const bool_t bBakedEdgeHistory = Trail.EdgePairs.size() >= 2u;
		if (nullptr == Trail.pElement ||
			(!bBakedEdgeHistory && Trail.Points.size() < 2u))
			continue;
		if (bBakedEdgeHistory && !Trail.Points.empty())
			return Fail_RenderOperation(
				"Trail carries both centerline and baked-edge geometry.",
				E_INVALIDARG, true);
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(Trail.pElement->strElementId);
		if (nullptr == pResource)
			return Fail_RenderOperation(
				"Trail resource contract is missing.", E_FAIL, true);
		if (pResource->bSourceMaterialFallbackBlocked ||
			pResource->bOccurrenceVisualSuppressed)
			continue;
		const bool_t bRuntimeMaterialV2Ribbon =
			0u != pResource->iRuntimeMaterialV2Enabled &&
			(9u == pResource->iRuntimeMaterialV2Opcode ||
			 20u == pResource->iRuntimeMaterialV2Opcode);
		const bool_t bRibbonLiquid01ParentDefault =
			bRuntimeMaterialV2Ribbon &&
			20u == pResource->iRuntimeMaterialV2Opcode;
		const bool_t bStandardColorV1 =
			0u != pResource->iStandardColorV1Enabled;
		const bool_t bTypedArtistRibbon =
			bRuntimeMaterialV2Ribbon && !bBakedEdgeHistory;
		const bool_t bFlowRibbon01 =
			35u == pResource->iSourceMaterialProfile && !bBakedEdgeHistory;
        const auto* SourceTrailProgram = Find_ArtistProgram(
            Trail.pElement->Material.SourceMaterial.strRuntimeShaderProfileId);
        const bool_t bSourceBeam = SourceTrailProgram && SourceTrailProgram->strRendererShape == "beam" &&
            Trail.pElement->RuntimeCarrier.eKind == EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1;
        const bool_t bKoukuNativeRibbon = !bBakedEdgeHistory && (bSourceBeam ||
            (SourceTrailProgram && SourceTrailProgram->strRendererShape == "ribbon" &&
             Trail.pElement->RuntimeCarrier.eKind == EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1));
		const bool_t bTypedSourceRibbon =
			bTypedArtistRibbon || bFlowRibbon01 || bKoukuNativeRibbon;
		if (35u == pResource->iSourceMaterialProfile && bBakedEdgeHistory)
		{
			return Fail_RenderOperation(
				"FlowRibbon01 cannot consume baked-edge AnimationTrail geometry.",
				E_INVALIDARG, true);
		}
		if (0u != pResource->iRuntimeMaterialV2Enabled &&
			!bRuntimeMaterialV2Ribbon)
		{
			return Fail_RenderOperation(
				"Trail RuntimeMaterialV2 opcode is invalid.", E_INVALIDARG, true);
		}
		const f32_t fTilingDistance =
			Trail.pElement->Detail.Trail.fTilingDistanceWorldUnits;
		const f32_t fTessellationStep =
			Trail.pElement->Detail.Trail.fDistanceTessellationStepWorldUnits;
		constexpr f32_t ARTIST_RIBBON_TILING_DISTANCE = 6.f;
		constexpr f32_t RIBBON_LIQUID_TILING_DISTANCE = 3.f;
		constexpr f32_t ARTIST_RIBBON_TESSELLATION_STEP = 0.05f;
		constexpr uint32_t ARTIST_RIBBON_MAX_SUBDIVISIONS = 25u;
		const f32_t fExpectedTypedTilingDistance =
			bRibbonLiquid01ParentDefault ? RIBBON_LIQUID_TILING_DISTANCE :
			ARTIST_RIBBON_TILING_DISTANCE;
		if (bTypedArtistRibbon &&
			(!std::isfinite(fTilingDistance) ||
				!std::isfinite(fTessellationStep) ||
				std::abs(fTilingDistance - fExpectedTypedTilingDistance) > 1e-6f ||
				std::abs(fTessellationStep - ARTIST_RIBBON_TESSELLATION_STEP) > 1e-6f))
		{
			return Fail_RenderOperation(
				"Trail typed tiling/tessellation contract is invalid.",
				E_INVALIDARG, true);
		}
		if (!bSourceBeam && (bFlowRibbon01 || bKoukuNativeRibbon) &&
			(!std::isfinite(fTilingDistance) || fTilingDistance < 0.f ||
			 !std::isfinite(fTessellationStep) || fTessellationStep < 0.f))
		{
			return Fail_RenderOperation(
				"FlowRibbon01 tiling/tessellation contract is invalid.",
				E_INVALIDARG, true);
		}

		std::vector<EFFECT_EVALUATED_TRAIL_POINT>& TessellatedPoints =
			m_TrailPointScratch;
		TessellatedPoints.clear();
		std::span<const EFFECT_EVALUATED_TRAIL_POINT> RenderPoints(
			Trail.Points.data(), Trail.Points.size());
		if (bTypedSourceRibbon)
		{
			const auto ValidatePoint = [pResource, &Trail, bFlowRibbon01,
				bKoukuNativeRibbon, bRibbonLiquid01ParentDefault](
				const EFFECT_EVALUATED_TRAIL_POINT& Point)
			{
				const uint32_t iColorMask =
					Point.iSourceColorComponentMask & 0x0fu;
				const uint32_t iDynamicMask =
					Point.iDynamicParameterComponentMask & 0x0fu;
				const bool_t bArtistCarrierContract =
					Trail.pElement->SourceRecipe.bEnabled &&
					!bRibbonLiquid01ParentDefault ?
					(iColorMask == 0x08u && iDynamicMask == 0x0fu) :
					((iColorMask &
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask) ==
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask &&
					 (iDynamicMask &
						pResource->iRuntimeMaterialV2DynamicConsumedMask) ==
							pResource->iRuntimeMaterialV2DynamicConsumedMask);
				const bool_t bKoukuCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f;
				const bool_t bFlowCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.y) <= 1e-4f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.z) <= 1e-4f &&
					std::abs(Point.vDynamicParameter.x - 0.02f) <= 1e-5f &&
					std::abs(Point.vDynamicParameter.y - 1.f) <= 1e-5f &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f;
				return (bKoukuNativeRibbon ? bKoukuCarrierContract :
					bFlowRibbon01 ? bFlowCarrierContract : bArtistCarrierContract) &&
					std::isfinite(Point.vWorldPosition.x) &&
					std::isfinite(Point.vWorldPosition.y) &&
					std::isfinite(Point.vWorldPosition.z) &&
					std::isfinite(Point.fNormalizedAge) &&
					Point.fNormalizedAge >= 0.f && Point.fNormalizedAge < 1.f &&
					std::isfinite(Point.fCumulativeDistance) &&
					Point.fCumulativeDistance >= 0.f &&
					std::isfinite(Point.vSourceColor.x) &&
					std::isfinite(Point.vSourceColor.y) &&
					std::isfinite(Point.vSourceColor.z) &&
					std::isfinite(Point.vSourceColor.w) &&
					std::isfinite(Point.vDynamicParameter.x) &&
					std::isfinite(Point.vDynamicParameter.y) &&
					std::isfinite(Point.vDynamicParameter.z) &&
					std::isfinite(Point.vDynamicParameter.w);
			};
			if (!std::all_of(Trail.Points.begin(), Trail.Points.end(), ValidatePoint))
				return Fail_RenderOperation(
					"Trail typed point payload is invalid.", E_INVALIDARG, true);
		}

		const bool_t bDistanceTessellated = (!bSourceBeam && bTypedSourceRibbon && fTessellationStep > 0.f) ||
			(std::isfinite(fTilingDistance) && fTilingDistance > 0.f &&
			 std::isfinite(fTessellationStep) && fTessellationStep > 0.f);
		if (bDistanceTessellated)
		{
			TessellatedPoints.reserve(1u +
				(Trail.Points.size() - 1u) * ARTIST_RIBBON_MAX_SUBDIVISIONS);
			TessellatedPoints.push_back(Trail.Points.front());
			for (size_t iPoint = 1u; iPoint < Trail.Points.size(); ++iPoint)
			{
				const EFFECT_EVALUATED_TRAIL_POINT& PreviousPoint =
					Trail.Points[iPoint - 1u];
				const EFFECT_EVALUATED_TRAIL_POINT& NextPoint =
					Trail.Points[iPoint];
				if (NextPoint.fCumulativeDistance + 1e-6f <
					PreviousPoint.fCumulativeDistance)
				{
					return Fail_RenderOperation(
						"Trail cumulative distance is not monotonic.",
						E_INVALIDARG, true);
				}
				const vector_t PreviousPosition =
					XMLoadFloat3(&PreviousPoint.vWorldPosition);
				const vector_t NextPosition = XMLoadFloat3(&NextPoint.vWorldPosition);
				const f32_t fSegmentDistance = XMVectorGetX(
					XMVector3Length(NextPosition - PreviousPosition));
				if (!std::isfinite(fSegmentDistance))
					return Fail_RenderOperation(
						"Trail segment distance is non-finite.",
						E_INVALIDARG, true);
				const f32_t fSubdivisionCount = std::clamp(
					static_cast<f32_t>(std::ceil(
						fSegmentDistance / fTessellationStep)),
					1.f, static_cast<f32_t>(ARTIST_RIBBON_MAX_SUBDIVISIONS));
				const uint32_t iSubdivisions =
					static_cast<uint32_t>(fSubdivisionCount);
				for (uint32_t iSubdivision = 1u;
					iSubdivision <= iSubdivisions; ++iSubdivision)
				{
					const f32_t fRatio = static_cast<f32_t>(iSubdivision) /
						static_cast<f32_t>(iSubdivisions);
					EFFECT_EVALUATED_TRAIL_POINT Point;
					XMStoreFloat3(&Point.vWorldPosition,
						XMVectorLerp(PreviousPosition, NextPosition, fRatio));
					Point.fNormalizedAge = PreviousPoint.fNormalizedAge +
						(NextPoint.fNormalizedAge - PreviousPoint.fNormalizedAge) * fRatio;
					Point.fCumulativeDistance = PreviousPoint.fCumulativeDistance +
						(NextPoint.fCumulativeDistance -
							PreviousPoint.fCumulativeDistance) * fRatio;
					Point.fSourceWidth = PreviousPoint.fSourceWidth +
						(NextPoint.fSourceWidth - PreviousPoint.fSourceWidth) * fRatio;
					XMStoreFloat4(&Point.vSourceColor,
						XMVectorLerp(XMLoadFloat4(&PreviousPoint.vSourceColor),
							XMLoadFloat4(&NextPoint.vSourceColor), fRatio));
					XMStoreFloat4(&Point.vDynamicParameter,
						XMVectorLerp(XMLoadFloat4(&PreviousPoint.vDynamicParameter),
							XMLoadFloat4(&NextPoint.vDynamicParameter), fRatio));
					Point.iSourceColorComponentMask =
						PreviousPoint.iSourceColorComponentMask &
						NextPoint.iSourceColorComponentMask;
					Point.iDynamicParameterComponentMask =
						PreviousPoint.iDynamicParameterComponentMask &
						NextPoint.iDynamicParameterComponentMask;
					TessellatedPoints.push_back(Point);
				}
			}
			RenderPoints = std::span<const EFFECT_EVALUATED_TRAIL_POINT>(
				TessellatedPoints.data(), TessellatedPoints.size());
		}

		std::vector<Engine::VTXEFFECT_TRAIL>& Vertices = m_TrailVertexScratch;
		std::vector<uint32_t>& Indices = m_TrailIndexScratch;
		Vertices.clear();
		Indices.clear();
		const size_t iGeometryPointCount = bBakedEdgeHistory ?
			Trail.EdgePairs.size() : RenderPoints.size();
		if (Vertices.capacity() < iGeometryPointCount * 2u)
			Vertices.reserve(iGeometryPointCount * 2u);
		if (Indices.capacity() < (iGeometryPointCount - 1u) * 6u)
			Indices.reserve((iGeometryPointCount - 1u) * 6u);
		if (bBakedEdgeHistory)
		{
			for (size_t iPair = 0u; iPair < Trail.EdgePairs.size(); ++iPair)
			{
				const EFFECT_EVALUATED_TRAIL_EDGE_PAIR& Pair =
					Trail.EdgePairs[iPair];
				const EFFECT_EVALUATED_TRAIL_POINT& Point = Pair.Payload;
				const auto IsFinite3 = [](const float3_t& Value)
				{
					return std::isfinite(Value.x) && std::isfinite(Value.y) &&
						std::isfinite(Value.z);
				};
				if (!IsFinite3(Pair.vFirstEdgeWorld) ||
					!IsFinite3(Pair.vControlPointWorld) ||
					!IsFinite3(Pair.vSecondEdgeWorld) ||
					!std::isfinite(Point.fNormalizedAge) ||
					Point.fNormalizedAge < 0.f || Point.fNormalizedAge >= 1.f ||
					!std::isfinite(Point.fCumulativeDistance) ||
					Point.fCumulativeDistance < 0.f)
				{
					return Fail_RenderOperation(
						"Baked-edge AnimationTrail geometry is invalid.",
						E_INVALIDARG, true);
				}
				if (bRuntimeMaterialV2Ribbon)
				{
					const uint32_t iColorMask =
						Point.iSourceColorComponentMask & 0x0fu;
					const uint32_t iDynamicMask =
						Point.iDynamicParameterComponentMask & 0x0fu;
					if ((iColorMask &
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask) !=
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask ||
						(iDynamicMask &
							pResource->iRuntimeMaterialV2DynamicConsumedMask) !=
						pResource->iRuntimeMaterialV2DynamicConsumedMask)
					{
						return Fail_RenderOperation(
							"Baked-edge AnimationTrail material carrier is incomplete.",
							E_INVALIDARG, true);
					}
				}
				const f32_t U = fTilingDistance > 0.f ?
					Point.fCumulativeDistance / fTilingDistance :
					static_cast<f32_t>(iPair);
                const bool bKoukuNativeTrail = pResource->iSourceMaterialProfile >= 2304u &&
                    pResource->iSourceMaterialProfile <= 3711u;
                if (bKoukuNativeTrail && Point.iSourceColorComponentMask != 0x0fu)
                    return Fail_RenderOperation("Kouku source trail color payload is incomplete.", E_INVALIDARG, true);
                const float4_t Color = bKoukuNativeTrail ? Point.vSourceColor : bRuntimeMaterialV2Ribbon ?
                    float4_t(1.f, 1.f, 1.f, Point.vSourceColor.w) :
					float4_t(1.f, 1.f, 1.f, 1.f - Point.fNormalizedAge);
				Vertices.push_back({ Pair.vFirstEdgeWorld, float2_t(U, 0.f),
					Color, Point.vDynamicParameter });
				Vertices.push_back({ Pair.vSecondEdgeWorld, float2_t(U, 1.f),
					Color, Point.vDynamicParameter });
			}
		}
		else for (size_t iPoint = 0u; iPoint < RenderPoints.size(); ++iPoint)
		{
			const EFFECT_EVALUATED_TRAIL_POINT& Point = RenderPoints[iPoint];
			if (bTypedSourceRibbon)
			{
				const uint32_t iColorMask =
					Point.iSourceColorComponentMask & 0x0fu;
				const uint32_t iDynamicMask =
					Point.iDynamicParameterComponentMask & 0x0fu;
				const bool_t bArtistCarrierContract =
					Trail.pElement->SourceRecipe.bEnabled &&
					!bRibbonLiquid01ParentDefault ?
					(iColorMask == 0x08u && iDynamicMask == 0x0fu) :
					((iColorMask &
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask) ==
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask &&
					 (iDynamicMask &
						pResource->iRuntimeMaterialV2DynamicConsumedMask) ==
							pResource->iRuntimeMaterialV2DynamicConsumedMask);
				const bool_t bKoukuCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f;
				const bool_t bFlowCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.y) <= 1e-4f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.z) <= 1e-4f &&
					std::abs(Point.vDynamicParameter.x - 0.02f) <= 1e-5f &&
					std::abs(Point.vDynamicParameter.y - 1.f) <= 1e-5f;
				if (!(bKoukuNativeRibbon ? bKoukuCarrierContract :
						bFlowRibbon01 ? bFlowCarrierContract : bArtistCarrierContract) ||
					!std::isfinite(Point.fCumulativeDistance) ||
					Point.fCumulativeDistance < 0.f ||
					!std::isfinite(Point.vSourceColor.w) ||
					!std::isfinite(Point.vDynamicParameter.x) ||
					!std::isfinite(Point.vDynamicParameter.y) ||
					!std::isfinite(Point.vDynamicParameter.z) ||
					!std::isfinite(Point.vDynamicParameter.w))
				{
					return Fail_RenderOperation(
						"Trail tessellated point payload is invalid.",
						E_INVALIDARG, true);
				}
			}
			const vector_t Position = XMLoadFloat3(
				&Point.vWorldPosition);
			const vector_t Previous = XMLoadFloat3(&RenderPoints[
				iPoint > 0u ? iPoint - 1u : iPoint].vWorldPosition);
			const vector_t Next = XMLoadFloat3(&RenderPoints[
				iPoint + 1u < RenderPoints.size() ? iPoint + 1u : iPoint].vWorldPosition);
			vector_t Tangent;
			if (!Normalize_Safe(Next - Previous, Tangent))
				continue;
			vector_t Side;
			if (Trail.pElement->Detail.Trail.bFaceCamera)
			{
				const vector_t View = CameraPosition - Position;
				if (!Normalize_Safe(XMVector3Cross(View, Tangent), Side))
					continue;
			}
			else if (!Normalize_Safe(XMVector3Cross(
				XMVectorSet(0.f, 1.f, 0.f, 0.f), Tangent), Side) &&
				!Normalize_Safe(XMVector3Cross(
					XMVectorSet(1.f, 0.f, 0.f, 0.f), Tangent), Side))
			{
				continue;
			}
			const f32_t Age = Point.fNormalizedAge;
			const f32_t Width = (bFlowRibbon01 || bKoukuNativeRibbon) ? Point.fSourceWidth :
				Trail.pElement->Detail.Trail.fStartWidth +
				(Trail.pElement->Detail.Trail.fEndWidth -
					Trail.pElement->Detail.Trail.fStartWidth) * Age;
			const vector_t HalfSide = Side * (Width * 0.5f);
			const f32_t U = fTilingDistance > 0.f ?
				Point.fCumulativeDistance / fTilingDistance :
				static_cast<f32_t>(iPoint);
			const float4_t Color = bKoukuNativeRibbon ? Point.vSourceColor : bFlowRibbon01 ?
				float4_t(Point.vSourceColor.x, Point.vDynamicParameter.z,
					Point.vDynamicParameter.w, Point.vSourceColor.w) :
				(bTypedArtistRibbon ?
					float4_t(1.f, 1.f, 1.f, Point.vSourceColor.w) :
					float4_t(1.f, 1.f, 1.f, 1.f - Age));
			Vertices.push_back({ To_Float3(Position - HalfSide),
				float2_t(U, 0.f), Color, Point.vDynamicParameter });
			Vertices.push_back({ To_Float3(Position + HalfSide),
				float2_t(U, 1.f), Color, Point.vDynamicParameter });
		}
		if (Vertices.size() < 4u)
			continue;
		const uint32_t iPairs = static_cast<uint32_t>(Vertices.size() / 2u);
		for (uint32_t iPair = 0u; iPair + 1u < iPairs; ++iPair)
		{
			const uint32_t Base = iPair * 2u;
			Indices.insert(Indices.end(),
				{ Base, Base + 1u, Base + 2u,
				  Base + 1u, Base + 3u, Base + 2u });
		}
		HRESULT hResult = m_pTrailBuffer->Update_Geometry(
			std::span<const Engine::VTXEFFECT_TRAIL>(
				Vertices.data(), Vertices.size()),
			std::span<const uint32_t>(Indices.data(), Indices.size()));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail geometry-buffer update failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestTrailGeometryUpload(
			std::span<const Engine::VTXEFFECT_TRAIL>(
				Vertices.data(), Vertices.size()));
#endif
		const f32_t LocalTime = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			Trail.pElement->Detail.Timing.fStartDelaySeconds);
		const f32_t Normalized = std::clamp(LocalTime /
			Trail.pElement->Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());
		const EFFECT_COLOR_DESC CommonColor =
			Evaluate_CommonColor(*Trail.pElement, Normalized);
		hResult = m_pTrailShader->Bind_Matrix("g_WorldMatrix", &Identity);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail shader bind failed: g_WorldMatrix.", hResult);
		hResult = Bind_Common(m_pTrailShader, *Trail.pElement,
			CommonColor, LocalTime, Normalized, *pResource);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail common/material shader bind failed.", hResult);
		const uint32_t iPass = Select_Pass(
			Trail.pElement->Material.eRenderProfile);
		if (UINT32_MAX == iPass)
			return Fail_RenderOperation(
				"Trail render-profile pass is invalid.", E_INVALIDARG, true);
		hResult = m_pTrailShader->Begin(iPass);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail shader pass apply failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestShaderPassApplication();
#endif
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (bTypedArtistRibbon || bStandardColorV1)
		{
			const size_t iSamplerCount = static_cast<size_t>(
				bStandardColorV1 ? pResource->StandardColorV1Header[2u] :
					pResource->iRuntimeMaterialV2TextureLaneCount);
			const size_t iExpectedRuntimeSamplerCount =
				bRibbonLiquid01ParentDefault ? 4u : 2u;
			if ((!bStandardColorV1 &&
				 iSamplerCount != iExpectedRuntimeSamplerCount) ||
				iSamplerCount == 0u ||
				iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Trail typed material sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			Record_TestSamplerBinding();
#endif
		}
		hResult = m_pTrailBuffer->Bind_Resources();
		if (S_OK != hResult)
			return Fail_RenderOperation(
				"Trail geometry-buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestVIBufferBinding();
		Record_TestDrawSelection(
			EFFECT_GPU_RENDER_CARRIER::RIBBON_DYNAMIC_TRAIL, iPass);
#endif
		hResult = m_pTrailBuffer->Render();
		if (S_OK != hResult)
			return Fail_RenderOperation(
				"Trail geometry-buffer draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestIssuedDraw(std::span<const Engine::VTXEFFECT_TRAIL>(
			Vertices.data(), Vertices.size()));
#endif
		bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}
