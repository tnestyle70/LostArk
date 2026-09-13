#include "Effect_DocumentRenderer_Internal.h"
#include "DeferredMaterialRenderUtils.h"
#include "Effect_NativeScreenPostMaterial.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
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
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_Rect.h"

bool_t Client::CEffectDocumentRenderer::Has_NonBlendModelCues() const
{
	if (m_bOccurrenceElementSelected) return false;
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	return std::ranges::any_of(Document.ModelCues,
		[&Document](const EFFECT_MODEL_CUE_DESC& Cue)
		{
			return Is_DimensionSummonCharacterSurfaceCue(Document, Cue);
		});
}

bool_t Client::CEffectDocumentRenderer::Has_WorldMarkElements() const
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	return std::ranges::any_of(Document.Elements,
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.bVisible && (Element.eCompositionLayer ==
                EFFECT_COMPOSITION_LAYER::WORLD_MARK || Element.eCompositionLayer ==
                EFFECT_COMPOSITION_LAYER::SCENE_BACKDROP);
		});
}

bool_t Client::CEffectDocumentRenderer::Has_ActiveSceneBackdrop(
    const EFFECT_EVALUATED_FRAME& Frame) const
{
    return std::ranges::any_of(Frame.Elements, [this](const auto& evaluated)
        {
            const EFFECT_ELEMENT_DESC* element = evaluated.pElement;
            if (!element || !element->bVisible ||
                element->eCompositionLayer != EFFECT_COMPOSITION_LAYER::SCENE_BACKDROP ||
                !Is_EffectSceneBackdropCarrier(*element) ||
                !Should_SubmitPreviewOccurrence(*element, Resolve_GpuRenderFamily(*element)))
                return false;
            const ELEMENT_RESOURCE* resource = Find_Resource(element->strElementId);
            return resource && !resource->bSourceMaterialFallbackBlocked &&
                !resource->bOccurrenceVisualSuppressed;
        });
}

HRESULT Client::CEffectDocumentRenderer::Render_NonBlendModelCues(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	if (!Has_NonBlendModelCues())
		return S_FALSE;
	std::string GateStatus;
	if (!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		m_bLastRenderFailureObjectLocal = true;
		return E_FAIL;
	}
	const HRESULT hResult = Render_ModelCues(Frame, true);
	if (FAILED(hResult))
	{
		m_strStatus = "Effect non-blend animated model-cue rendering failed.";
		if (!m_strRenderFailureDetail.empty())
			m_strStatus += " Operation: " + m_strRenderFailureDetail;
	}
	else
	{
		m_strStatus =
			"Effect non-blend animated model-cue rendering completed.";
	}
	return hResult;
}

bool_t Client::CEffectDocumentRenderer::Sample_ModelCuePose(
	const EFFECT_MODEL_CUE_DESC& Cue, MODEL_CUE_RESOURCE& Resource,
	const f32_t fSampleTimeSeconds, const float4x4_t& RootWorld,
	float4x4_t& OutWorld, std::string& strOutError)
{
	if (!Resource.pModel || !std::isfinite(fSampleTimeSeconds) ||
		!std::isfinite(Resource.fDurationSeconds) || Resource.fDurationSeconds <= 0.f)
	{
		strOutError = "Model Cue pose input is invalid: " + Cue.strCueId;
		return false;
	}
	// Outside the visible cue window, anchors retain its first/last source pose.
	// Emitters keep their own timing and may have a tail after the model disappears.
	const f32_t fLocalTime = std::clamp(fSampleTimeSeconds - Cue.fStartDelaySeconds,
		0.f, Cue.fDurationSeconds);
	Engine::CModel& Model = *Resource.pModel;
	// Seek from the effect clock so scrubbing and replay use the same loop phase.
	// Authored translation uses cue time. Optional root suppression is already
	// configured on this clone so source X/Z locomotion cannot rewind at a wrap.
	const f32_t fAnimationTime = Cue.bLoop ?
		std::fmod(fLocalTime, Resource.fDurationSeconds) :
		(Cue.bHoldLastFrame ? (std::min)(fLocalTime, Resource.fDurationSeconds) :
		 fLocalTime);
	const f32_t fTrackRequest = fAnimationTime * Resource.fTicksPerSecond;
	// Each occurrence owns this clone. Only this function changes its pose;
	// anchors and rendering can therefore share an identical explicit seek.
	// Keep the exact request as the key, so CAnimation retains its own clamping.
	const bool_t bReusablePose = std::isfinite(fTrackRequest) &&
		Model.Get_CurrentAnimIndex() == Resource.iAnimationIndex &&
		!Model.Is_AnimLoop() && !Model.Is_AnimBlending();
	if (!bReusablePose || !Resource.bHasSampledPose ||
		Resource.iSampledAnimationIndex != Resource.iAnimationIndex ||
		Resource.fSampledTrackRequest != fTrackRequest)
	{
		if (!Model.Set_AnimTrackPosition(Resource.iAnimationIndex, fTrackRequest))
		{
			strOutError = "Animated model-cue track position is invalid: " + Cue.strCueId;
			return false;
		}
		Model.Play_Animation(0.f);
		Resource.iSampledAnimationIndex = Resource.iAnimationIndex;
		Resource.fSampledTrackRequest = fTrackRequest;
		Resource.bHasSampledPose = bReusablePose;
	}
	const EFFECT_TRANSFORM_DESC& Transform = Cue.LocalTransform;
	const float3_t Position = {
		Transform.vPosition.x + Transform.vVelocityPerSecond.x * fLocalTime,
		Transform.vPosition.y + Transform.vVelocityPerSecond.y * fLocalTime,
		Transform.vPosition.z + Transform.vVelocityPerSecond.z * fLocalTime };
	const float3_t Rotation = {
		Transform.vRotationDegrees.x +
			Transform.vRevolutionDegreesPerSecond.x * fLocalTime,
		Transform.vRotationDegrees.y +
			Transform.vRevolutionDegreesPerSecond.y * fLocalTime,
		Transform.vRotationDegrees.z +
			Transform.vRevolutionDegreesPerSecond.z * fLocalTime };
	const matrix_t Local =
		XMMatrixScaling(Transform.vScale.x, Transform.vScale.y,
			Transform.vScale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Rotation.x),
			XMConvertToRadians(Rotation.y),
			XMConvertToRadians(Rotation.z)) *
		XMMatrixTranslation(Position.x, Position.y, Position.z);

	XMStoreFloat4x4(&OutWorld, Local * XMLoadFloat4x4(&RootWorld));
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Collect_ModelCueAnchorWorlds(
	const f32_t fSampleTimeSeconds, const float4x4_t& RootWorld,
	std::unordered_map<std::string, float4x4_t>& InOutAnchorWorlds,
	std::string& strOutError)
{
	std::unordered_map<std::string, float4x4_t> CueWorlds;
	std::unordered_map<std::string, float4x4_t> Anchors;
	const auto& Document = Get_StagedDocument();
	for (const auto& Element : Document.Elements)
	{
		const auto& Attachment = Element.ActionCueAttachment;
		if (!Element.bVisible || Attachment.strModelCueId.empty())
			continue;
		if (Anchors.contains(Attachment.strRuntimeAnchorSlotId))
			continue;
		auto Resource = m_ModelCueResources.find(Attachment.strModelCueId);
		const auto Cue = std::find_if(Document.ModelCues.begin(), Document.ModelCues.end(),
			[&](const EFFECT_MODEL_CUE_DESC& Value) { return Value.strCueId == Attachment.strModelCueId; });
		if (Cue == Document.ModelCues.end() || Resource == m_ModelCueResources.end() ||
			!Resource->second.pModel ||
			!Resource->second.pModel->Has_Bone(Attachment.strRuntimeBoneName.c_str()))
		{
			strOutError = "Model Cue anchor owner or bone is unavailable: " + Attachment.strRuntimeAnchorSlotId;
			return false;
		}
		auto World = CueWorlds.find(Cue->strCueId);
		if (World == CueWorlds.end())
		{
			float4x4_t SampledWorld{};
			if (!Sample_ModelCuePose(*Cue, Resource->second, fSampleTimeSeconds,
				RootWorld, SampledWorld, strOutError))
				return false;
			World = CueWorlds.emplace(Cue->strCueId, SampledWorld).first;
		}
		const auto& Socket = Attachment.SocketLocalTransform;
		const matrix_t Local = XMMatrixScaling(Socket.vScale.x, Socket.vScale.y, Socket.vScale.z) *
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(Socket.vRotationDegrees.x),
				XMConvertToRadians(Socket.vRotationDegrees.y), XMConvertToRadians(Socket.vRotationDegrees.z)) *
			XMMatrixTranslation(Socket.vPosition.x, Socket.vPosition.y, Socket.vPosition.z);
		float4x4_t Anchor{};
		XMStoreFloat4x4(&Anchor, Local * Resource->second.pModel->Get_BoneMatrix(
			Attachment.strRuntimeBoneName.c_str()) * XMLoadFloat4x4(&World->second));
		for (size_t Row = 0u; Row < 4u; ++Row)
			for (size_t Column = 0u; Column < 4u; ++Column)
				if (!std::isfinite(Anchor.m[Row][Column]))
				{
					strOutError = "Model Cue anchor is non-finite: " + Attachment.strRuntimeAnchorSlotId;
					return false;
				}
		Anchors.emplace(Attachment.strRuntimeAnchorSlotId, Anchor);
	}
	for (const auto& [Id, World] : Anchors)
		InOutAnchorWorlds.insert_or_assign(Id, World);
	strOutError.clear();
	return true;
}

HRESULT Client::CEffectDocumentRenderer::Build_NativeScreenPost(
    const EFFECT_EVALUATED_SCREEN_POST& Evaluated,
    std::shared_ptr<const Engine::IPresentationScreenPostMaterial>& OutMaterial,
    std::string& strOutError) const
{
    OutMaterial.reset();
    if (!Evaluated.pElement) { strOutError="Native screen-post has no source element."; return E_INVALIDARG; }
    const auto& Element=*Evaluated.pElement;
    const auto& Source=Element.Material.SourceMaterial;
    const auto* V=Find_DimensionMasterVProgram(Source.strRuntimeShaderProfileId);
    const auto* ALTV=Find_DimensionMasterALTVProgram(Source.strRuntimeShaderProfileId);
    const auto* Warlord=Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId);
    const auto* Artist=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
    const auto* Lance=Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
    if (!V && !ALTV && !Warlord && !Artist && !Lance) return S_FALSE;
    const bool Valid=(V && V->bScreenPost && Has_DimensionMasterVMaterialContract(Element)) ||
        (ALTV && ALTV->strRendererShape=="screenPost" && Has_DimensionMasterALTVMaterialContract(Element)) ||
        (Warlord && Warlord->strRendererShape=="screenPost" && Has_WarlordNativeMaterialContract(Element)) ||
        (Artist && Artist->strRendererShape=="screenPost" && Has_ArtistMaterialContract(Element)) ||
        (Lance && Lance->strRendererShape=="screenPost" && Has_LanceMasterVAMaterialContract(Element));
    const auto* Resource=Find_Resource(Element.strElementId);
    if (!Valid || !Resource || !m_pNativeScreenPostShader ||
        !Is_NativeScreenPostShaderProfile(Resource->iSourceMaterialProfile))
    { strOutError="Native screen-post material inputs are not prepared: "+Element.strElementId; return E_INVALIDARG; }
    EFFECT_NATIVE_SCREEN_POST_SNAPSHOT Snapshot;
    Snapshot.pShader=m_pNativeScreenPostShader;
    Snapshot.SourceTextures=Resource->SourceTextures;
    for (auto& Texture : Snapshot.SourceTextures) if (!Texture) Texture=m_pBlackTexture;
    Snapshot.Parameters=Lance ? Resource->LanceVASourceMaterialParameters : Artist ? Resource->ArtistSourceMaterialParameters :
        ALTV ? Resource->ALTVSourceMaterialParameters : Resource->VSourceMaterialParameters;
    Snapshot.iProfile=Resource->iSourceMaterialProfile;
    Snapshot.iTextureMask=Resource->iSourceTextureMask;
    Snapshot.iClampUMask=Resource->iSourceTextureClampUMask;
    Snapshot.iClampVMask=Resource->iSourceTextureClampVMask;
    Snapshot.vSourceColor=Evaluated.vSourceColor;
    Snapshot.vDynamicParameter=Evaluated.vSourceDynamicParameter;
    Snapshot.fLocalTimeSeconds=Evaluated.fSampleTimeSeconds;
    Snapshot.fBloomIntensity=Get_BloomIntensity();
    const auto* View=Engine::CGameInstance::Get().Get_Transform(D3DTS::VIEW);
    const auto* Projection=Engine::CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    const auto* ViewInverse=Engine::CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
    if (!View || !Projection || !ViewInverse)
    { strOutError="Native screen-post has no active scene camera."; return E_FAIL; }
    vector_t Position=XMLoadFloat4x4(&Evaluated.SourceWorld).r[3];
    const vector_t ToCamera=XMLoadFloat4x4(ViewInverse).r[3]-Position;
    const float Distance=XMVectorGetX(XMVector3Length(ToCamera));
    if (!std::isfinite(Distance) || !std::isfinite(Evaluated.fSourceCameraOffset))
    { strOutError="Native screen-post camera offset is invalid."; return E_INVALIDARG; }
    if (Distance>1.e-6f) Position+=ToCamera*(Evaluated.fSourceCameraOffset/Distance);
    Position=XMVectorSetW(Position,1.f);
    const vector_t Clip=XMVector4Transform(Position,
        XMLoadFloat4x4(View)*XMLoadFloat4x4(Projection));
    Snapshot.fProjectionW=XMVectorGetW(Clip);
    if (!std::isfinite(Snapshot.fProjectionW) ||
        !std::isfinite(Snapshot.fLocalTimeSeconds) || Snapshot.fLocalTimeSeconds<0.f)
    { strOutError="Native screen-post source sample is invalid."; return E_INVALIDARG; }
    auto Candidate=std::make_shared<CEffectNativeScreenPostMaterial>(std::move(Snapshot));
    OutMaterial=std::move(Candidate);
    return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Bind_ModelCueNativeMaterial(
	const ELEMENT_RESOURCE& Resource, const f32_t fLocalTimeSeconds)
{
	// UE skylight has no current scene owner. Feed the committed scene ambient
	// into its separate ambient term once; leave both sky hemispheres disabled.
	float4_t Ambient{ 0.f, 0.f, 0.f, 1.f };
	for (const auto& Light : CGameInstance::Get().Get_SceneLights())
	{
		if (Light.eType != LIGHT::DIRECTIONAL) continue;
		Ambient.x += Light.vAmbient.x;
		Ambient.y += Light.vAmbient.y;
		Ambient.z += Light.vAmbient.z;
	}
	const auto& Shader = m_pAnimatedModelShader;
	ComPtr<ID3D11ShaderResourceView> SourceSceneDepth;
	if (Resource.bSourceRequiresSceneDepth)
	{
		SourceSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		if (nullptr == SourceSceneDepth)
			return Fail_RenderOperation("Animated model-cue scene-depth input is unavailable.", E_FAIL, true);
	}
	// Clear the prior cue's depth SRV when the current material does not use it.
	if (FAILED(Shader->Bind_Texture("g_EffectSceneDepthTexture", SourceSceneDepth)))
		return Fail_RenderOperation("Animated model-cue scene-depth binding failed.", E_FAIL, true);

	if (FAILED(Shader->Bind_RawValue("g_ArtistModelCueProfile", &Resource.iSourceMaterialProfile, sizeof(Resource.iSourceMaterialProfile))) ||
		FAILED(Shader->Bind_RawValue("g_ArtistSourceMaterialParameters", Resource.ArtistSourceMaterialParameters.data(), sizeof(Resource.ArtistSourceMaterialParameters))) ||
		FAILED(Shader->Bind_RawValue("g_ArtistSourceMaterialTime", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		FAILED(Shader->Bind_RawValue("g_ArtistModelAmbient", &Ambient, sizeof(Ambient))) ||
		FAILED(Shader->Bind_RawValue("g_LanceVASourceMaterialParameters", Resource.LanceVASourceMaterialParameters.data(), sizeof(Resource.LanceVASourceMaterialParameters))) ||
		FAILED(Shader->Bind_RawValue("g_LanceVASourceMaterialTime", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
        FAILED(Shader->Bind_RawValue("g_ALTVSourceMaterialParameters", Resource.ALTVSourceMaterialParameters.data(), sizeof(Resource.ALTVSourceMaterialParameters))) ||
        FAILED(Shader->Bind_RawValue("g_ALTVSourceMaterialTime", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		FAILED(Shader->Bind_RawValue("g_SourceTextureClampUMask", &Resource.iSourceTextureClampUMask, sizeof(Resource.iSourceTextureClampUMask))) ||
		FAILED(Shader->Bind_RawValue("g_SourceTextureClampVMask", &Resource.iSourceTextureClampVMask, sizeof(Resource.iSourceTextureClampVMask))))
		return E_FAIL;
	if ((Resource.iSourceMaterialProfile == 178u && !m_pStartingSceneBloomCapture) ||
		FAILED(Shader->Bind_Texture("g_EffectStartingSceneBloomTexture",
			Resource.iSourceMaterialProfile == 178u ? m_pStartingSceneBloomCapture : nullptr))) return E_FAIL;
	for (size_t iLane = 0u; iLane < Resource.SourceTextures.size(); ++iLane)
	{
		const std::string Name = "g_SourceTexture" + std::to_string(iLane);
        const bool bFrozenCapture = Resource.iSourceMaterialProfile == 178u && iLane == 2u;
        if (bFrozenCapture && nullptr == m_pStartingSceneCapture)
            return Fail_RenderOperation("Animated capture cube has no starting scene snapshot.", E_FAIL, true);
        const auto& Texture = bFrozenCapture ? m_pStartingSceneCapture : Resource.SourceTextures[iLane];
        if (FAILED(Shader->Bind_Texture(Name.c_str(), Texture ? Texture : m_pBlackTexture))) return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_ModelCues(
	const EFFECT_EVALUATED_FRAME& Frame,
	const bool_t bNonBlendCharacterSurfaceOnly)
{
	// Keep model cues available for anchor sampling, but an explicitly selected
	// occurrence Element must not draw the source document's standalone models.
	if (m_bOccurrenceElementSelected) return S_FALSE;
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		const bool_t bCharacterSurface =
			Is_DimensionSummonCharacterSurfaceCue(Document, Cue);
		if (bCharacterSurface != bNonBlendCharacterSurfaceOnly)
			continue;
		const f32_t fLocalTime =
			Frame.fSampleTimeSeconds - Cue.fStartDelaySeconds;
		if (!Cue.bVisible || fLocalTime < 0.f ||
			fLocalTime > Cue.fDurationSeconds)
		{
			continue;
		}
		auto Resource = m_ModelCueResources.find(Cue.strCueId);
		if (Resource == m_ModelCueResources.end() ||
			nullptr == Resource->second.pModel)
		{
			return Fail_RenderOperation(
				"Animated model-cue resource contract is missing.", E_FAIL, true);
		}
		Engine::CModel& Model = *Resource->second.pModel;
		float4x4_t World{};
		std::string PoseError;
		if (!Sample_ModelCuePose(Cue, Resource->second, Frame.fSampleTimeSeconds,
			Frame.RootWorld, World, PoseError))
			return Fail_RenderOperation(std::move(PoseError), E_FAIL, true);
		HRESULT hResult = m_pAnimatedModelShader->Bind_Matrix(
			"g_WorldMatrix", &World);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_WorldMatrix.", hResult);
		hResult = CGameInstance::Get().Bind_Transform(
			m_pAnimatedModelShader, "g_ViewMatrix", D3DTS::VIEW);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_ViewMatrix.", hResult);
		hResult = CGameInstance::Get().Bind_Transform(
			m_pAnimatedModelShader, "g_ProjMatrix", D3DTS::PROJ);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_ProjMatrix.", hResult);
		hResult = m_pAnimatedModelShader->Bind_RawValue(
			"g_EffectModelCueColorMultiply", &Cue.vColorMultiply,
			sizeof(Cue.vColorMultiply));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: color multiply.", hResult);
		hResult = m_pAnimatedModelShader->Bind_RawValue(
			"g_EffectModelCueOpacity", &Cue.fOpacity, sizeof(Cue.fOpacity));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: opacity.", hResult);
		for (uint32_t iMesh = 0u; iMesh < Model.Get_NumMeshes(); ++iMesh)
		{
			hResult = Resource->second.pMaterialResource ?
				Bind_ModelCueNativeMaterial(*Resource->second.pMaterialResource, fLocalTime) :
				Bind_DeferredMaterialInputs(Model, m_pAnimatedModelShader, iMesh);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue material bind failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			hResult = Model.Bind_BoneMatrices(
				m_pAnimatedModelShader, "g_BoneMatrices", iMesh);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue bone bind failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			uint32_t iPass =
				Cue.eAlphaMode == EFFECT_MODEL_CUE_ALPHA_MODE::OPAQUE_SURFACE ?
					2u : 0u;
			if (Cue.eAlphaMode ==
				EFFECT_MODEL_CUE_ALPHA_MODE::TRANSLUCENT_SURFACE)
			{
				iPass = 4u;
			}
			if (Resource->second.pMaterialResource)
				iPass = Cue.Material->eRenderProfile == EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ? 8u : 7u;
			hResult = Bind_BloomInputs(m_pAnimatedModelShader);
			if (FAILED(hResult)) return hResult;
			hResult = m_pAnimatedModelShader->Begin(iPass);
			if (SUCCEEDED(hResult)) hResult = Model.Render(iMesh);
			// Shared shaders return to the scene/global intensity even after a failed draw.
			const f32_t defaultBloom = -1.f;
			const HRESULT resetResult = m_pAnimatedModelShader->Bind_RawValue(
				"g_fEffectBloomIntensity", &defaultBloom, sizeof(defaultBloom));
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue shader/draw failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			if (FAILED(resetResult))
				return Fail_RenderOperation("Animated model-cue bloom reset failed.", resetResult);
		}
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_ReconstructedDiagnostic(
	const float4x4_t& RootWorld,
	const RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo)
{
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	const auto FailDiagnostic = [this](std::string strDetail,
		const HRESULT hResult, const bool_t bObjectLocal)
	{
		const HRESULT hFailure = Fail_RenderOperation(
			std::move(strDetail), hResult, bObjectLocal);
		m_strStatus = "Reconstructed diagnostic failed: " +
			m_strRenderFailureDetail;
		return hFailure;
	};
	if (nullptr == m_pReconstructedDiagnostic ||
		nullptr == m_pReconstructedDiagnostic->pFrame ||
		eSolo >= RECONSTRUCTED_DIAGNOSTIC_SOLO::END)
	{
		return FailDiagnostic(
			"Reconstructed diagnostic draw was not staged.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND eRequiredKind =
		eSolo == RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH ?
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH :
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE;
	const auto& Packets = m_pReconstructedDiagnostic->pFrame->Get_Packets();
	const auto PacketIterator = std::find_if(Packets.begin(), Packets.end(),
		[eRequiredKind](const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet)
		{
			return Packet.Get_Kind() == eRequiredKind;
		});
	if (PacketIterator == Packets.end())
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Solo packet is unavailable.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet = *PacketIterator;
	const uint32_t iSelection = Packet.Get_SelectionIndex();
	if (iSelection >= m_pReconstructedDiagnostic->Resources.size() ||
		nullptr == Packet.Get_Preparation() ||
		iSelection >= Packet.Get_Preparation()->Get_Request().Emitters.size())
	{
		return FailDiagnostic(
			"Reconstructed diagnostic selection join is invalid.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection =
		Packet.Get_Preparation()->Get_Request().Emitters[iSelection];
	RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource =
		m_pReconstructedDiagnostic->Resources[iSelection];
	const shared_ptr<Engine::CShader> Shader =
		eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ?
			m_pMeshShader : m_pParticleShader;
	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == Shader ||
		nullptr == Resource.Textures[0u] ||
		nullptr == Resource.Textures[1u] || nullptr == Resource.Samplers[0u] ||
		nullptr == Resource.pPipelineStatisticsQuery)
	{
		return FailDiagnostic(
			"Reconstructed diagnostic GPU composite is incomplete.",
			E_FAIL, true);
	}
	if (eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH &&
		(nullptr == Resource.pModel ||
			!Packet.Get_Values().vMeshDimensionlessScaleXzy.has_value()))
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Mesh packet contract is incomplete.",
			E_FAIL, true);
	}
	if (eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE &&
		(nullptr == m_pParticleBuffer || !Packet.Get_SpriteSink().has_value() ||
			!Packet.Get_Values().vSpriteSignedWorldSizeXzy.has_value()))
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Sprite packet contract is incomplete.",
			E_FAIL, true);
	}

	if (Resource.bPipelineStatisticsPending)
	{
		D3D11_QUERY_DATA_PIPELINE_STATISTICS Statistics{};
		const HRESULT StatisticsResult = m_pContext->GetData(
			Resource.pPipelineStatisticsQuery.Get(), &Statistics,
			sizeof(Statistics), D3D11_ASYNC_GETDATA_DONOTFLUSH);
		if (S_OK == StatisticsResult)
		{
			Resource.bPipelineStatisticsPending = false;
			Resource.PipelineStatistics = Statistics;
			if (0u == Statistics.VSInvocations || 0u == Statistics.PSInvocations)
			{
				return FailDiagnostic(
					"Reconstructed diagnostic draw produced no shader invocations.",
					E_FAIL, true);
			}
		}
		else if (FAILED(StatisticsResult))
		{
			return FailDiagnostic(
				"Reconstructed diagnostic pipeline-statistics readback failed.",
				StatisticsResult, false);
		}
	}

	const auto ToFloat4 = [](const std::array<double, 4u>& Value)
	{
		return float4_t(static_cast<f32_t>(Value[0u]),
			static_cast<f32_t>(Value[1u]), static_cast<f32_t>(Value[2u]),
			static_cast<f32_t>(Value[3u]));
	};
	const EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING& Material =
		Selection.Material;
	const float2_t UvScale{
		static_cast<f32_t>(Material.Constants.vUvScale[0u]),
		static_cast<f32_t>(Material.Constants.vUvScale[1u]) };
	const float4_t PanRotationAux = ToFloat4(
		Material.Constants.vPanRotationAux);
	const float4_t MaterialColor = ToFloat4(Material.Constants.vColor);
	const float4_t Params0 = ToFloat4(Material.Constants.vParams0);
	const float4_t Params1 = ToFloat4(Material.Constants.vParams1);
	const float2_t LegacyUvScale{ 1.f, 1.f };
	const float2_t LegacyUvOffset{};
	const f32_t fSampleTime = static_cast<f32_t>(
		m_pReconstructedDiagnostic->pFrame->Get_SampleTimeSeconds());
	const uint32_t iEnabled = 1u;
	const uint32_t iDisabled = 0u;
	const uint32_t iSourceMaterialProfile = 0u;
	const uint32_t iSourceTextureClampMask = 0u;
	const uint32_t iUseBaseOverride = 1u;

	HRESULT DrawResult = S_OK;
	std::string strDrawFailureDetail;
	bool_t bDrawFailureObjectLocal = false;
	const auto RecordDrawFailure = [&](const HRESULT hResult,
		std::string strDetail, const bool_t bObjectLocal,
		const bool_t bOverrideObjectLocalFailure = false)
	{
		if (SUCCEEDED(hResult))
			return;
		if (FAILED(DrawResult) &&
			!(bOverrideObjectLocalFailure && bDrawFailureObjectLocal))
		{
			return;
		}
		DrawResult = hResult;
		strDrawFailureDetail = std::move(strDetail);
		bDrawFailureObjectLocal = bObjectLocal;
	};
	const auto TryGlobalOperation = [&](const auto& Operation,
		const char* const pFailureDetail)
	{
		if (FAILED(DrawResult))
			return;
		RecordDrawFailure(Operation(), pFailureDetail, false);
	};
	const auto RecordLocalContractFailure = [&](const char* const pFailureDetail)
	{
		RecordDrawFailure(E_FAIL, pFailureDetail, true);
	};

	TryGlobalOperation([&]() { return Shader->Bind_Matrix("g_ViewMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)); },
		"Reconstructed diagnostic shader bind failed: g_ViewMatrix.");
	TryGlobalOperation([&]() { return Shader->Bind_Matrix("g_ProjMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)); },
		"Reconstructed diagnostic shader bind failed: g_ProjMatrix.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_UVScale", &LegacyUvScale, sizeof(LegacyUvScale)); },
		"Reconstructed diagnostic shader bind failed: g_UVScale.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_UVOffset", &LegacyUvOffset, sizeof(LegacyUvOffset)); },
		"Reconstructed diagnostic shader bind failed: g_UVOffset.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_EffectLocalTime", &fSampleTime, sizeof(fSampleTime)); },
		"Reconstructed diagnostic shader bind failed: g_EffectLocalTime.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceMaterialProfile", &iSourceMaterialProfile,
		sizeof(iSourceMaterialProfile)); },
		"Reconstructed diagnostic shader bind failed: g_SourceMaterialProfile.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceTextureClampUMask", &iSourceTextureClampMask,
		sizeof(iSourceTextureClampMask)); },
		"Reconstructed diagnostic shader bind failed: g_SourceTextureClampUMask.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceTextureClampVMask", &iSourceTextureClampMask,
		sizeof(iSourceTextureClampMask)); },
		"Reconstructed diagnostic shader bind failed: g_SourceTextureClampVMask.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strFeatureMaskVariable.c_str(),
		&Material.iFeatureMask, sizeof(Material.iFeatureMask)); },
		"Reconstructed diagnostic feature-mask bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strUvScaleVariable.c_str(), &UvScale,
		sizeof(UvScale)); },
		"Reconstructed diagnostic UV-scale bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strPanRotationAuxVariable.c_str(),
		&PanRotationAux, sizeof(PanRotationAux)); },
		"Reconstructed diagnostic pan/rotation bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strColorVariable.c_str(), &MaterialColor,
		sizeof(MaterialColor)); },
		"Reconstructed diagnostic color bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strParams0Variable.c_str(), &Params0,
		sizeof(Params0)); },
		"Reconstructed diagnostic params0 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strParams1Variable.c_str(), &Params1,
		sizeof(Params1)); },
		"Reconstructed diagnostic params1 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_Texture(
		Material.TextureLanes[0u].strShaderVariableName.c_str(),
		Resource.Textures[0u]); },
		"Reconstructed diagnostic texture lane 0 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_Texture(
		Material.TextureLanes[1u].strShaderVariableName.c_str(),
		Resource.Textures[1u]); },
		"Reconstructed diagnostic texture lane 1 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strEvaluatorEnabledVariable.c_str(),
		&iEnabled, sizeof(iEnabled)); },
		"Reconstructed diagnostic evaluator-enable bind failed.");
	if (SUCCEEDED(DrawResult) &&
		eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH)
	{
		const auto& Scale =
			*Packet.Get_Values().vMeshDimensionlessScaleXzy;
		const auto& Position = Packet.Get_Values().vLocalPosition;
		float4x4_t World{};
		XMStoreFloat4x4(&World,
			XMMatrixScaling(static_cast<f32_t>(Scale[0u]),
				static_cast<f32_t>(Scale[1u]),
				static_cast<f32_t>(Scale[2u])) *
			XMMatrixRotationY(XMConvertToRadians(static_cast<f32_t>(
				Packet.Get_Values().fRotationDegrees))) *
			XMMatrixTranslation(static_cast<f32_t>(Position[0u]),
				static_cast<f32_t>(Position[1u]),
				static_cast<f32_t>(Position[2u])) *
			XMLoadFloat4x4(&RootWorld));
		const float4_t CameraPosition =
			*CGameInstance::Get().Get_CamPosition();
		const float4_t Dynamic = ToFloat4(
			Packet.Get_Values().vDynamicParameter);
		TryGlobalOperation([&]() { return Shader->Bind_Matrix(
			"g_WorldMatrix", &World); },
			"Reconstructed diagnostic Mesh world bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition)); },
			"Reconstructed diagnostic Mesh camera bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_EffectDynamicParameter", &Dynamic, sizeof(Dynamic)); },
			"Reconstructed diagnostic Mesh dynamic bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_UseBaseOverride", &iUseBaseOverride,
			sizeof(iUseBaseOverride)); },
			"Reconstructed diagnostic Mesh base-override bind failed.");
	}

	ComPtr<ID3D11InfoQueue> InfoQueue;
	const bool_t bHasInfoQueue = SUCCEEDED(m_pDevice.As(&InfoQueue));
	const uint64_t iMessageBegin = bHasInfoQueue ?
		InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter() : 0u;
	if ((Resource.bHasBlendDescriptor && nullptr == Resource.pBlendState) ||
		(Resource.bHasRasterizerDescriptor &&
			nullptr == Resource.pRasterizerState) ||
		(Resource.bHasDepthStencilDescriptor &&
			nullptr == Resource.pDepthStencilState))
	{
		RecordLocalContractFailure(
			"Reconstructed diagnostic pipeline-state contract is incomplete.");
	}
	bool_t bQueryBegun = false;
	{
		CReconstructedPipelineStateGuard StateGuard(m_pContext.Get());
		TryGlobalOperation([&]() {
			return Shader->Begin(Material.Shader.iPassIndex);
		}, "Reconstructed diagnostic shader pass apply failed.");
		if (SUCCEEDED(DrawResult))
		{
			ID3D11SamplerState* pSampler = Resource.Samplers[0u].Get();
			m_pContext->PSSetSamplers(0u, 1u, &pSampler);
			if (Resource.bHasBlendDescriptor)
			{
				const float BlendFactor[4u]{};
				m_pContext->OMSetBlendState(
					Resource.pBlendState.Get(), BlendFactor, 0xffffffffu);
			}
			if (Resource.bHasRasterizerDescriptor)
				m_pContext->RSSetState(Resource.pRasterizerState.Get());
			if (Resource.bHasDepthStencilDescriptor)
				m_pContext->OMSetDepthStencilState(
					Resource.pDepthStencilState.Get(), 0u);

			ComPtr<ID3D11SamplerState> ActualSampler;
			m_pContext->PSGetSamplers(0u, 1u, &ActualSampler);
			D3D11_SAMPLER_DESC ActualSamplerDescriptor{};
			if (nullptr == ActualSampler)
				RecordLocalContractFailure(
					"Reconstructed diagnostic sampler readback is null.");
			else
				ActualSampler->GetDesc(&ActualSamplerDescriptor);
			if (SUCCEEDED(DrawResult) && !Same_SamplerDescriptor(
				ActualSamplerDescriptor, Resource.SamplerDescriptors[0u]))
			{
				RecordLocalContractFailure(
					"Reconstructed diagnostic sampler descriptor changed.");
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasBlendDescriptor)
			{
				ComPtr<ID3D11BlendState> Actual;
				float Factor[4u]{};
				uint32_t iMask = 0u;
				m_pContext->OMGetBlendState(&Actual, Factor, &iMask);
				D3D11_BLEND_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || iMask != 0xffffffffu ||
					!Same_BlendDescriptor(
						Descriptor, Resource.BlendDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic blend-state readback changed.");
				}
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasRasterizerDescriptor)
			{
				ComPtr<ID3D11RasterizerState> Actual;
				m_pContext->RSGetState(&Actual);
				D3D11_RASTERIZER_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || !Same_RasterizerDescriptor(
					Descriptor, Resource.RasterizerDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic rasterizer-state readback changed.");
				}
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasDepthStencilDescriptor)
			{
				ComPtr<ID3D11DepthStencilState> Actual;
				uint32_t iStencilReference = 0u;
				m_pContext->OMGetDepthStencilState(
					&Actual, &iStencilReference);
				D3D11_DEPTH_STENCIL_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || 0u != iStencilReference ||
					!Same_DepthStencilDescriptor(
						Descriptor, Resource.DepthStencilDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic depth-state readback changed.");
				}
			}
		}

		if (SUCCEEDED(DrawResult) &&
			!Resource.bPipelineStatisticsPending)
		{
			m_pContext->Begin(Resource.pPipelineStatisticsQuery.Get());
			bQueryBegun = true;
		}
		if (SUCCEEDED(DrawResult) &&
			eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH)
		{
			for (uint32_t iMesh = 0u;
				SUCCEEDED(DrawResult) && iMesh < Resource.pModel->Get_NumMeshes();
				++iMesh)
			{
				const HRESULT hModelResult = Resource.pModel->Render(iMesh);
				if (FAILED(hModelResult))
				{
					RecordDrawFailure(hModelResult,
						"Reconstructed diagnostic Mesh draw failed at submesh " +
						std::to_string(iMesh) + ".", false);
				}
			}
		}
		else if (SUCCEEDED(DrawResult))
		{
			const auto& Size =
				*Packet.Get_Values().vSpriteSignedWorldSizeXzy;
			const auto& Position = Packet.Get_Values().vLocalPosition;
			const auto& Velocity = Packet.Get_Values().vVelocityPerSecond;
			const EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK& Sink =
				*Packet.Get_SpriteSink();
			const matrix_t Root = XMLoadFloat4x4(&RootWorld);
			const vector_t WorldPosition = XMVector3TransformCoord(
				XMVectorSet(static_cast<f32_t>(Position[0u]),
					static_cast<f32_t>(Position[1u]),
					static_cast<f32_t>(Position[2u]), 1.f), Root);
			const vector_t WorldVelocity = XMVector3TransformNormal(
				XMVectorSet(static_cast<f32_t>(Velocity[0u]),
					static_cast<f32_t>(Velocity[1u]),
					static_cast<f32_t>(Velocity[2u]), 0.f), Root);
			matrix_t CameraWorld = XMLoadFloat4x4(
				CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			const f32_t fRight = XMVectorGetX(XMVector3Dot(
				WorldVelocity, CameraWorld.r[0u]));
			const f32_t fUp = XMVectorGetX(XMVector3Dot(
				WorldVelocity, CameraWorld.r[1u]));
			const f32_t fVelocityRoll =
				std::abs(fRight) + std::abs(fUp) > 1.e-6f ?
					std::atan2(fUp, fRight) : 0.f;
			const f32_t fRoll = fVelocityRoll + XMConvertToRadians(
				static_cast<f32_t>(Packet.Get_Values().fRotationDegrees +
					Sink.fBillboardRollDegrees));
			const matrix_t Pivot = XMMatrixTranslation(
				0.5f - static_cast<f32_t>(Sink.vPivotCenter[0u]),
				static_cast<f32_t>(Sink.vPivotCenter[1u]) - 0.5f, 0.f);
			float4x4_t World{};
			XMStoreFloat4x4(&World,
				Pivot * XMMatrixScaling(static_cast<f32_t>(Size[0u]),
					static_cast<f32_t>(Size[2u]), 1.f) *
				XMMatrixRotationZ(fRoll) * CameraWorld *
				XMMatrixTranslationFromVector(WorldPosition));
			const float4_t Color = ToFloat4(Packet.Get_Values().vColor);
			const float4_t Dynamic = ToFloat4(
				Packet.Get_Values().vDynamicParameter);
			const f32_t fNormalizedLife = static_cast<f32_t>(std::clamp(
				Packet.Get_Timing().fAgeSeconds /
					Packet.Get_Timing().fLifetimeSeconds, 0.0, 1.0));
			const Engine::VTXEFFECT_PARTICLE Instance{
				World, Color, Dynamic,
				{ 1.f, 1.f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.f },
				{ fNormalizedLife, 0.f } };
			const HRESULT hUpdateResult = m_pParticleBuffer->Update_Instances(
				std::span<const Engine::VTXEFFECT_PARTICLE>(&Instance, 1u));
			RecordDrawFailure(hUpdateResult,
				"Reconstructed diagnostic Sprite instance update failed.", false);
			if (SUCCEEDED(DrawResult))
			{
				const HRESULT hSpriteDrawResult = m_pParticleBuffer->Render();
				RecordDrawFailure(hSpriteDrawResult,
					"Reconstructed diagnostic Sprite draw failed.", false);
			}
		}
		if (bQueryBegun)
		{
			m_pContext->End(Resource.pPipelineStatisticsQuery.Get());
			Resource.bPipelineStatisticsPending = true;
		}
	}

	const HRESULT hEvaluatorResetResult = Shader->Bind_RawValue(
		Material.Shader.strEvaluatorEnabledVariable.c_str(),
		&iDisabled, sizeof(iDisabled));
	if (FAILED(hEvaluatorResetResult))
	{
		RecordDrawFailure(hEvaluatorResetResult,
			"Reconstructed diagnostic evaluator reset failed.", false, true);
	}
	if (SUCCEEDED(DrawResult) && bHasInfoQueue)
	{
		const uint64_t iMessageEnd =
			InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
		for (uint64_t iMessage = iMessageBegin; iMessage < iMessageEnd;
			++iMessage)
		{
			size_t iMessageSize = 0u;
			const HRESULT hMessageSizeResult =
				InfoQueue->GetMessage(iMessage, nullptr, &iMessageSize);
			if (FAILED(hMessageSizeResult))
			{
				RecordDrawFailure(hMessageSizeResult,
					"Reconstructed diagnostic D3D info-queue size read failed.",
					false);
				break;
			}
			if (0u == iMessageSize)
				continue;
			std::vector<uint8_t> MessageBytes(iMessageSize);
			D3D11_MESSAGE* pMessage =
				reinterpret_cast<D3D11_MESSAGE*>(MessageBytes.data());
			const HRESULT hMessageResult = InfoQueue->GetMessage(
				iMessage, pMessage, &iMessageSize);
			if (FAILED(hMessageResult))
			{
				RecordDrawFailure(hMessageResult,
					"Reconstructed diagnostic D3D info-queue read failed.", false);
				break;
			}
			if (pMessage->Severity == D3D11_MESSAGE_SEVERITY_ERROR ||
				pMessage->Severity == D3D11_MESSAGE_SEVERITY_CORRUPTION)
			{
				RecordDrawFailure(E_FAIL,
					"Reconstructed diagnostic D3D debug layer reported an error.",
					false);
				break;
			}
		}
	}
	if (FAILED(DrawResult))
	{
		if (strDrawFailureDetail.empty())
			strDrawFailureDetail =
				"Reconstructed diagnostic GPU draw or state readback failed.";
		return FailDiagnostic(std::move(strDrawFailureDetail), DrawResult,
			bDrawFailureObjectLocal);
	}
	++Resource.iDrawCount;
	const char* pKind =
		eSolo == RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH ? "Mesh" : "Sprite";
	m_strStatus = std::string("Reconstructed ") + pKind +
		" Solo draw submitted; draw count " +
		std::to_string(Resource.iDrawCount) + ", VS/PS invocations " +
		std::to_string(Resource.PipelineStatistics.VSInvocations) + "/" +
		std::to_string(Resource.PipelineStatistics.PSInvocations) + ".";
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_WorldMarks(
	const EFFECT_EVALUATED_FRAME& Frame,
	const uint64_t iSubmissionSerial)
{
	if (!Has_WorldMarkElements())
	{
		m_bWorldMarkSubmissionPending = false;
		return S_FALSE;
	}
	m_bWorldMarkSubmissionPending = false;
	const HRESULT hResult = Render_CompositionPhase(Frame,
		EFFECT_COMPOSITION_LAYER::WORLD_MARK, true, false,
		iSubmissionSerial);
	if (SUCCEEDED(hResult))
	{
		m_bWorldMarkSubmissionPending = true;
		m_iWorldMarkSubmissionSerial = iSubmissionSerial;
	}
	return hResult;
}

HRESULT Client::CEffectDocumentRenderer::Render(
	const EFFECT_EVALUATED_FRAME& Frame,
	const uint64_t iSubmissionSerial)
{
	const bool_t bHasWorldMarks = Has_WorldMarkElements();
	if (bHasWorldMarks &&
		(!m_bWorldMarkSubmissionPending ||
		 m_iWorldMarkSubmissionSerial != iSubmissionSerial))
	{
		const HRESULT hWorldMarkResult =
			Render_WorldMarks(Frame, iSubmissionSerial);
		if (FAILED(hWorldMarkResult))
			return hWorldMarkResult;
	}
	if (!bHasWorldMarks)
		m_bWorldMarkSubmissionPending = false;
	const HRESULT hResult = Render_CompositionPhase(Frame,
		EFFECT_COMPOSITION_LAYER::NORMAL, !bHasWorldMarks, true,
		iSubmissionSerial);
	m_bWorldMarkSubmissionPending = false;
	return hResult;
}

HRESULT Client::CEffectDocumentRenderer::Render_CompositionPhase(
	const EFFECT_EVALUATED_FRAME& Frame,
	const EFFECT_COMPOSITION_LAYER ePhase,
	const bool_t bBeginSubmission,
	const bool_t bFinalizeSubmission,
	const uint64_t iSubmissionSerial)
{
	m_vSourceActorPosition = { Frame.RootWorld._41 * 100.f, -Frame.RootWorld._43 * 100.f, Frame.RootWorld._42 * 100.f, 1.f };
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	if (ePhase >= EFFECT_COMPOSITION_LAYER::END ||
		bFinalizeSubmission !=
			(ePhase == EFFECT_COMPOSITION_LAYER::NORMAL) ||
		(!bBeginSubmission &&
		 (!m_bWorldMarkSubmissionPending ||
		  m_iWorldMarkSubmissionSerial != iSubmissionSerial)))
	{
		m_bWorldMarkSubmissionPending = false;
		m_strStatus = "Effect composition phase submission is inconsistent.";
		m_bLastRenderFailureObjectLocal = true;
		return E_INVALIDARG;
	}
	if (bBeginSubmission)
		m_LastRenderSubmissionStats = {};
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	m_pActiveOccurrenceStats = nullptr;
	if (bBeginSubmission)
	{
		m_iTestIssuedDrawOrdinal = 0u;
		m_LastRenderSubmissionStats.Occurrences.reserve(
			Document.Elements.size());
	}
#endif
	if (bBeginSubmission)
	{
		m_strRenderFailureDetail.clear();
		m_bLastRenderFailureObjectLocal = false;
	}
	size_t iConfiguredGpuOccurrenceCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible)
			continue;
		const EFFECT_GPU_RENDER_FAMILY eFamily =
			Resolve_GpuRenderFamily(Element);
		if (EFFECT_GPU_RENDER_FAMILY::END != eFamily)
		{
			++iConfiguredGpuOccurrenceCount;
			if (bBeginSubmission)
			{
				++m_LastRenderSubmissionStats.Families[
					static_cast<size_t>(eFamily)].iConfigured;
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			if (bBeginSubmission)
			{
				EFFECT_GPU_RENDER_OCCURRENCE_STATS Occurrence;
				Occurrence.strElementId = Element.strElementId;
				Occurrence.eFamily = eFamily;
				Occurrence.eCompositionLayer = Element.eCompositionLayer;
				Occurrence.iConfigured = 1u;
				if (const ELEMENT_RESOURCE* pResource =
					Find_Resource(Element.strElementId))
				{
					Occurrence.iSourceMaterialProfile =
						pResource->iSourceMaterialProfile;
					Occurrence.iSourceTextureMask =
						pResource->iSourceTextureMask;
					Occurrence.bSourceMaterialFallbackBlocked =
						pResource->bSourceMaterialFallbackBlocked;
				}
				m_LastRenderSubmissionStats.Occurrences.emplace_back(
					std::move(Occurrence));
			}
#endif
		}
	}
	const auto FailFrame = [this](std::string strStatus,
		const HRESULT hResult = E_FAIL,
		const bool_t bObjectLocal = false) -> HRESULT
	{
		m_bWorldMarkSubmissionPending = false;
		m_LastRenderSubmissionStats.bCompleted = true;
		m_LastRenderSubmissionStats.bCommitted = false;
		m_bLastRenderFailureObjectLocal =
			m_bLastRenderFailureObjectLocal || bObjectLocal;
		if (!m_strRenderFailureDetail.empty())
			strStatus += " Operation: " + m_strRenderFailureDetail;
		m_strStatus = std::move(strStatus);
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	for (const EFFECT_EVALUATED_ELEMENT& Element : Frame.Elements)
	{
		if (nullptr == Element.pElement)
		{
			return FailFrame(
				"Effect frame contains an element row without a descriptor.",
				E_INVALIDARG, true);
		}
	}
	for (const EFFECT_EVALUATED_AFTERIMAGE& AfterImage : Frame.AfterImages)
	{
		if (nullptr == AfterImage.pElement ||
			!std::isfinite(AfterImage.fAlpha) || AfterImage.fAlpha < 0.f ||
			AfterImage.fAlpha > 1.f)
		{
			return FailFrame(
				"Effect frame contains an invalid afterimage row.",
				E_INVALIDARG, true);
		}
	}
	if (Frame.GpuOccurrences.size() != iConfiguredGpuOccurrenceCount)
	{
		return FailFrame(
			"Effect GPU occurrence evaluation count does not match the document.",
			E_FAIL, true);
	}
	std::string GateStatus;
	if (!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		return FailFrame(std::move(GateStatus), E_FAIL, true);
	}
	const HRESULT hModelCueResult =
		ePhase == EFFECT_COMPOSITION_LAYER::NORMAL ?
			Render_ModelCues(Frame, false) : S_FALSE;
	if (FAILED(hModelCueResult))
	{
		return FailFrame(
			"Effect animated model-cue rendering failed.", hModelCueResult);
	}
    // Keep frame row order intact while submitting capture meshes before sprites.
    // Other documents and world-mark phase preserve their authored draw order.
    const bool bCaptureMeshFirst = ePhase == EFFECT_COMPOSITION_LAYER::NORMAL &&
        Requires_StartingSceneCapture(Document);
    const uint32_t iCapturePassCount = bCaptureMeshFirst ? 2u : 1u;
    for (uint32_t iCapturePass = 0u; iCapturePass < iCapturePassCount; ++iCapturePass)
    {
		size_t iElement = 0u;
		size_t iParticle = 0u;
		size_t iTrail = 0u;
		size_t iAfterImage = 0u;
		size_t iGpuOccurrence = 0u;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		struct ACTIVE_OCCURRENCE_SCOPE final
		{
			EFFECT_GPU_RENDER_OCCURRENCE_STATS*& pSlot;
			explicit ACTIVE_OCCURRENCE_SCOPE(
				EFFECT_GPU_RENDER_OCCURRENCE_STATS*& pActiveSlot)
				: pSlot(pActiveSlot)
			{
			}
			~ACTIVE_OCCURRENCE_SCOPE() { pSlot = nullptr; }
		};
		size_t iOccurrenceStats = 0u;
#endif
		for (const EFFECT_ELEMENT_DESC& DocumentElement : Document.Elements)
		{
			const std::string& strElementId = DocumentElement.strElementId;
			const EFFECT_GPU_RENDER_FAMILY eFamily = DocumentElement.bVisible ?
				Resolve_GpuRenderFamily(DocumentElement) :
				EFFECT_GPU_RENDER_FAMILY::END;
			const bool_t bHasGpuFamily =
				EFFECT_GPU_RENDER_FAMILY::END != eFamily;
			const bool_t bOwnsPhase =
	            (DocumentElement.eCompositionLayer == ePhase ||
	             (ePhase == EFFECT_COMPOSITION_LAYER::WORLD_MARK &&
	              DocumentElement.eCompositionLayer == EFFECT_COMPOSITION_LAYER::SCENE_BACKDROP)) &&
	            (!bCaptureMeshFirst || ((iCapturePass == 0u) == (eFamily == EFFECT_GPU_RENDER_FAMILY::MESH)));
			EFFECT_GPU_RENDER_FAMILY_STATS* pFamilyStats =
				!bHasGpuFamily || !bOwnsPhase ? nullptr :
					&m_LastRenderSubmissionStats.Families[
						static_cast<size_t>(eFamily)];
			const bool_t bSubmitPreviewOccurrence =
				bOwnsPhase &&
				Should_SubmitPreviewOccurrence(DocumentElement, eFamily);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			EFFECT_GPU_RENDER_OCCURRENCE_STATS* pOccurrenceStats = nullptr;
			if (bHasGpuFamily)
			{
				if (iOccurrenceStats >=
					m_LastRenderSubmissionStats.Occurrences.size())
				{
					return FailFrame(
						"Effect GPU occurrence probe denominator overflow.",
						E_FAIL, true);
				}
				EFFECT_GPU_RENDER_OCCURRENCE_STATS* pDocumentOccurrenceStats =
					&m_LastRenderSubmissionStats.Occurrences[iOccurrenceStats++];
				if (pDocumentOccurrenceStats->strElementId != strElementId ||
					pDocumentOccurrenceStats->eFamily != eFamily)
				{
					return FailFrame(
						"Effect GPU occurrence probe order diverged.", E_FAIL, true);
				}
				if (bOwnsPhase)
					pOccurrenceStats = pDocumentOccurrenceStats;
			}
			m_pActiveOccurrenceStats = pOccurrenceStats;
			ACTIVE_OCCURRENCE_SCOPE ActiveOccurrenceScope(
				m_pActiveOccurrenceStats);
#endif
			const EFFECT_EVALUATED_GPU_OCCURRENCE* pGpuOccurrence = nullptr;
			if (bHasGpuFamily)
			{
				if (iGpuOccurrence >= Frame.GpuOccurrences.size())
				{
					return FailFrame(
						"Effect GPU occurrence evaluation order overflowed.",
						E_FAIL, true);
				}
				pGpuOccurrence = &Frame.GpuOccurrences[iGpuOccurrence++];
			}
			if (bHasGpuFamily != (nullptr != pGpuOccurrence) ||
				(nullptr != pGpuOccurrence &&
				 (nullptr == pGpuOccurrence->pElement ||
				  pGpuOccurrence->pElement->strElementId != strElementId ||
				  eFamily != Resolve_GpuRenderFamily(*pGpuOccurrence->pElement))))
			{
				return FailFrame(
					"Effect GPU occurrence evaluation does not match the document.",
					E_FAIL, true);
			}
			if (nullptr != pFamilyStats)
				++pFamilyStats->iEvaluated;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			if (nullptr != pOccurrenceStats)
				++pOccurrenceStats->iEvaluated;
#endif
			bool_t bOccurrenceActive =
				nullptr != pGpuOccurrence && pGpuOccurrence->bActive;
			bool_t bOccurrenceSubmitted = false;
			bool_t bOccurrenceSuppressed = false;
			const auto HasOccurrenceRow = [&strElementId](const auto& Rows, const size_t iRow)
			{
				return iRow < Rows.size() && nullptr != Rows[iRow].pElement &&
					Rows[iRow].pElement->strElementId == strElementId;
			};
			if (bSubmitPreviewOccurrence &&
				(HasOccurrenceRow(Frame.Elements, iElement) ||
				 HasOccurrenceRow(Frame.Particles, iParticle) ||
				 HasOccurrenceRow(Frame.Trails, iTrail) ||
				 HasOccurrenceRow(Frame.AfterImages, iAfterImage)))
			{
				const ELEMENT_RESOURCE* pResource = Find_Resource(strElementId);
				if (nullptr != pResource && !pResource->bSourceMaterialFallbackBlocked &&
					!pResource->bOccurrenceVisualSuppressed &&
					(pResource->bSourceRequiresSceneColor ||
					 pResource->iSourceMaterialProfile == 42u ||
					 pResource->iSourceMaterialProfile == 69u))
				{
					// Include earlier translucent occurrences; all particles/material slots
					// of this occurrence share one snapshot without sampling their own draw.
					const HRESULT hSnapshot = CGameInstance::Get().Refresh_SceneColorSnapshot();
					if (FAILED(hSnapshot))
						return FailFrame("Effect scene-color refresh failed: " + strElementId,
							hSnapshot, true);
				}
			}
			const size_t iAfterImageBegin = iAfterImage;
			while (iAfterImage < Frame.AfterImages.size() &&
				nullptr != Frame.AfterImages[iAfterImage].pElement &&
				Frame.AfterImages[iAfterImage].pElement->strElementId == strElementId)
			{
				++iAfterImage;
			}
			const HRESULT hAfterImageResult = bSubmitPreviewOccurrence ?
				Render_AfterImages(Frame,
					std::span<const EFFECT_EVALUATED_AFTERIMAGE>(Frame.AfterImages)
						.subspan(iAfterImageBegin,
							iAfterImage - iAfterImageBegin)) : S_FALSE;
			if (iAfterImageBegin != iAfterImage)
				bOccurrenceActive = true;
			if (FAILED(hAfterImageResult))
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect afterimage rendering failed: " + strElementId,
					hAfterImageResult);
			}
			bOccurrenceSubmitted = bOccurrenceSubmitted ||
				S_OK == hAfterImageResult;
			bOccurrenceSuppressed = bOccurrenceSuppressed ||
				S_FALSE == hAfterImageResult && iAfterImageBegin != iAfterImage;

			const size_t iElementBegin = iElement;
			while (iElement < Frame.Elements.size() &&
				nullptr != Frame.Elements[iElement].pElement &&
				Frame.Elements[iElement].pElement->strElementId == strElementId)
			{
				if (!bSubmitPreviewOccurrence)
				{
					++iElement;
					continue;
				}
				const ELEMENT_RESOURCE* pResource = Find_Resource(strElementId);
				if (nullptr == pResource)
				{
					if (nullptr != pFamilyStats)
						++pFamilyStats->iFailed;
					return FailFrame(
						"Effect element resource is missing: " + strElementId,
						E_FAIL, true);
				}
				if (pResource->bSourceMaterialFallbackBlocked ||
					pResource->bOccurrenceVisualSuppressed)
				{
					bOccurrenceSuppressed = true;
					++iElement;
					continue;
				}
				const HRESULT hElementResult =
					Render_Element(Frame.Elements[iElement], *pResource);
				if (FAILED(hElementResult))
				{
					if (nullptr != pFamilyStats)
						++pFamilyStats->iFailed;
					return FailFrame(
						"Effect element rendering failed: " + strElementId,
						hElementResult);
				}
				bOccurrenceSubmitted = bOccurrenceSubmitted ||
					S_OK == hElementResult;
				bOccurrenceSuppressed = bOccurrenceSuppressed ||
					S_FALSE == hElementResult;
				++iElement;
			}
			bOccurrenceActive = bOccurrenceActive || iElementBegin != iElement;

			const size_t iParticleBegin = iParticle;
			while (iParticle < Frame.Particles.size() &&
				nullptr != Frame.Particles[iParticle].pElement &&
				Frame.Particles[iParticle].pElement->strElementId == strElementId)
			{
				++iParticle;
			}
			const size_t iTrailBegin = iTrail;
			while (iTrail < Frame.Trails.size() &&
				nullptr != Frame.Trails[iTrail].pElement &&
				Frame.Trails[iTrail].pElement->strElementId == strElementId)
			{
				++iTrail;
			}
			const HRESULT hParticleResult = bSubmitPreviewOccurrence ?
				Render_Particles(Frame,
					std::span<const EFFECT_EVALUATED_PARTICLE>(Frame.Particles)
						.subspan(iParticleBegin,
							iParticle - iParticleBegin)) : S_FALSE;
			if (iParticleBegin != iParticle)
				bOccurrenceActive = true;
			if (FAILED(hParticleResult))
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect particle rendering failed: " + strElementId,
					hParticleResult);
			}
			bOccurrenceSubmitted = bOccurrenceSubmitted ||
				S_OK == hParticleResult && iParticleBegin != iParticle;
			bOccurrenceSuppressed = bOccurrenceSuppressed ||
				S_FALSE == hParticleResult && iParticleBegin != iParticle;
			const HRESULT hTrailResult = bSubmitPreviewOccurrence ?
				Render_Trails(Frame,
					std::span<const EFFECT_EVALUATED_TRAIL>(Frame.Trails)
						.subspan(iTrailBegin, iTrail - iTrailBegin)) : S_FALSE;
			if (iTrailBegin != iTrail)
				bOccurrenceActive = true;
			if (FAILED(hTrailResult))
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect trail rendering failed: " + strElementId,
					hTrailResult);
			}
			bOccurrenceSubmitted = bOccurrenceSubmitted ||
				S_OK == hTrailResult && iTrailBegin != iTrail;
			bOccurrenceSuppressed = bOccurrenceSuppressed ||
				S_FALSE == hTrailResult && iTrailBegin != iTrail;
			const size_t iCandidateRowCount =
				(iAfterImage - iAfterImageBegin) +
				(iElement - iElementBegin) +
				(iParticle - iParticleBegin) +
				(iTrail - iTrailBegin);
			if (nullptr != pGpuOccurrence &&
				pGpuOccurrence->iCandidateRowCount != iCandidateRowCount)
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect GPU occurrence candidate-row denominator mismatch: " +
					strElementId, E_FAIL, true);
			}
			if (0u < iCandidateRowCount && !bOccurrenceActive)
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect inactive GPU occurrence produced candidate rows: " +
					strElementId, E_FAIL, true);
			}
			if (bOccurrenceActive && 0u == iCandidateRowCount)
				bOccurrenceSuppressed = true;
			if (!bSubmitPreviewOccurrence && bOccurrenceActive)
				bOccurrenceSuppressed = true;
			if (bOccurrenceActive && bOwnsPhase)
			{
				if (nullptr == pFamilyStats)
				{
					return FailFrame(
						"Effect active GPU occurrence has no typed family: " +
						strElementId, E_FAIL, true);
				}
				++pFamilyStats->iActive;
				if (0u < iCandidateRowCount)
					++pFamilyStats->iCandidate;
				else
					++pFamilyStats->iZeroCandidate;
				++pFamilyStats->iAttempted;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
				++pOccurrenceStats->iActive;
				pOccurrenceStats->iCandidateRowCount += iCandidateRowCount;
				++pOccurrenceStats->iAttempted;
#endif
				if (bOccurrenceSubmitted)
				{
					++pFamilyStats->iSubmitted;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
					++pOccurrenceStats->iSubmitted;
					if (0u == pOccurrenceStats->iMaterialBindCount ||
						0u == pOccurrenceStats->iTextureSrvBindCount ||
						0u == pOccurrenceStats->iSamplerBindCount ||
						0u == pOccurrenceStats->iShaderPassApplyCount ||
						0u == pOccurrenceStats->iVIBufferBindCount ||
						0u == pOccurrenceStats->iVIBufferDrawCount ||
						0u == pOccurrenceStats->iIssuedDrawCallCount ||
						0u == pOccurrenceStats->iDrawSelectionCount ||
						pOccurrenceStats->iVIBufferBindCount !=
							pOccurrenceStats->iVIBufferDrawCount ||
						pOccurrenceStats->iVIBufferDrawCount !=
							pOccurrenceStats->iIssuedDrawCallCount ||
						pOccurrenceStats->iDrawSelectionCount !=
							pOccurrenceStats->iIssuedDrawCallCount ||
						pOccurrenceStats->bDrawSelectionDiverged ||
						EFFECT_GPU_RENDER_CARRIER::END ==
							pOccurrenceStats->eCarrier ||
						UINT32_MAX == pOccurrenceStats->iSelectedPassIndex ||
						!pOccurrenceStats->bHasSubmittedPosition)
					{
						++pOccurrenceStats->iFailed;
						++pFamilyStats->iFailed;
						return FailFrame(
							"Effect submitted occurrence bypassed material/pass/VF/draw evidence: " +
							strElementId, E_FAIL, true);
					}
#endif
				}
				else if (bOccurrenceSuppressed)
				{
					++pFamilyStats->iSuppressed;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
					++pOccurrenceStats->iSuppressed;
#endif
				}
				else
				{
					++pFamilyStats->iFailed;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
					++pOccurrenceStats->iFailed;
#endif
					return FailFrame(
						"Effect active GPU occurrence produced no disposition: " +
						strElementId, E_FAIL, true);
				}
			}
		}
		if (iElement != Frame.Elements.size() ||
			iParticle != Frame.Particles.size() ||
			iTrail != Frame.Trails.size() ||
			iAfterImage != Frame.AfterImages.size() ||
			iGpuOccurrence != Frame.GpuOccurrences.size())
		{
			return FailFrame(
				"Effect frame contains unconsumed evaluated GPU rows.",
				E_FAIL, true);
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (iOccurrenceStats != m_LastRenderSubmissionStats.Occurrences.size())
		{
			return FailFrame(
				"Effect GPU occurrence probe denominator underflow.", E_FAIL, true);
		}
#endif
    }
	if (!bFinalizeSubmission)
		return S_OK;
	uint64_t iActive = 0u;
	uint64_t iSubmitted = 0u;
	uint64_t iSuppressed = 0u;
	for (const EFFECT_GPU_RENDER_FAMILY_STATS& Stats :
		m_LastRenderSubmissionStats.Families)
	{
		if (Stats.iEvaluated != Stats.iConfigured ||
			Stats.iCandidate + Stats.iZeroCandidate != Stats.iActive ||
			Stats.iAttempted != Stats.iActive ||
			Stats.iSubmitted + Stats.iSuppressed != Stats.iAttempted ||
			0u != Stats.iFailed)
		{
			return FailFrame(
				"Effect GPU family occurrence denominator mismatch.",
				E_FAIL, true);
		}
		iActive += Stats.iActive;
		iSubmitted += Stats.iSubmitted;
		iSuppressed += Stats.iSuppressed;
	}
	m_LastRenderSubmissionStats.bCompleted = true;
	m_LastRenderSubmissionStats.bCommitted = true;
	const uint64_t iEvaluated = Frame.GpuOccurrences.size();
	if (m_iStatusEvaluated != iEvaluated || m_iStatusActive != iActive ||
		m_iStatusSubmitted != iSubmitted ||
		m_iStatusSuppressed != iSuppressed)
	{
		m_strStatus = "Effect GPU occurrence disposition: evaluated " +
			std::to_string(iEvaluated) + ", active " +
			std::to_string(iActive) + ", submitted " +
			std::to_string(iSubmitted) + ", suppressed " +
			std::to_string(iSuppressed) + ".";
		m_iStatusEvaluated = iEvaluated;
		m_iStatusActive = iActive;
		m_iStatusSubmitted = iSubmitted;
		m_iStatusSuppressed = iSuppressed;
	}
	return 0u < iActive && 0u == iSubmitted ? S_FALSE : S_OK;
}
