#include "Effect_PresentationService.h"

#include "Character.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "Effect_ReconstructedExecution.h"
#include "GameInstance.h"
#include "Model.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr const wchar_t* EFFECT_LAYER = L"Layer_Effect";
	constexpr const char_t* RECONSTRUCTED_ARTIST_31470_ASSET_ID =
		"effect.artist.skill.31470";
	// The playable Artist prototype contributes a 0.0001 admission transform,
	// while the Artist rig's sdm root contributes a scale of 100. b_wp_1 is a
	// combined bone matrix, so its admitted effective basis is 0.01. Keep this
	// contract Artist-specific and fail closed instead of normalizing an
	// arbitrary observed scale.
	constexpr f32_t ARTIST_SOURCE_BONE_COMBINED_SCALE = 0.01f;
	constexpr f32_t ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE = 0.00005f;
	constexpr f32_t ARTIST_SOURCE_BONE_UNIFORM_SCALE_TOLERANCE = 0.000005f;
	constexpr f32_t SOURCE_BONE_ORTHOGONAL_TOLERANCE = 0.001f;
	constexpr f32_t SOURCE_BONE_AFFINE_TOLERANCE = 0.00001f;
	constexpr f32_t SOURCE_BONE_MINIMUM_BASIS_LENGTH = 0.00000001f;
	constexpr f32_t SOURCE_BONE_MINIMUM_NORMALIZED_DETERMINANT = 0.0001f;

    struct SOURCE_ANCHOR_REQUEST final
    {
        std::string strRuntimeAnchorSlotId;
        std::string strRuntimeBoneName;
        Client::EFFECT_TRANSFORM_DESC SocketLocalTransform{};
		bool_t bNormalizeSourceImportScale = false;
    };

    struct ACTIVE_EFFECT final
    {
        std::shared_ptr<Client::CEffectObject> pObject;
        std::weak_ptr<Client::CCharacter> pOwner;
        uint32_t iLevelIndex = UINT32_MAX;
        std::string strEffectAssetId;
        std::string strAnchorSlotId;
        Client::EFFECT_TRANSFORM_DESC LocalTransform{};
        Client::EFFECT_FOLLOW_POLICY eFollowPolicy =
            Client::EFFECT_FOLLOW_POLICY::FOLLOW;
        Client::EFFECT_STOP_POLICY eStopPolicy =
            Client::EFFECT_STOP_POLICY::NATURAL;
        uint32_t iCueDurationMs = 0u;
        uint32_t iActionStartTick = 0u;
        uint32_t iCueStartMs = 0u;
		std::string strOccurrenceId;
        f32_t fPlaybackRate = 1.f;
        f32_t fElapsedCueTimeSeconds = 0.f;
        f32_t fPendingInitialSampleTimeSeconds = 0.f;
        bool_t bPendingInitialSeek = true;
        bool_t bFollowAnchorMissing = false;
        std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests;
    };

	struct RECONSTRUCTED_SOURCE_RUNTIME_CACHE final
	{
		std::shared_ptr<const Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation;
		std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> pDocument;
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pPrepared;
	};

	struct PENDING_EFFECT_SPAWN final
	{
		Client::EFFECT_SPAWN_DESC Desc;
		uint32_t iLevelIndex = UINT32_MAX;
	};

    std::vector<ACTIVE_EFFECT> g_ActiveEffects;
	std::vector<PENDING_EFFECT_SPAWN> g_PendingEffectSpawns;
    std::set<std::string, std::less<>> g_ProductEffectTargets;
	RECONSTRUCTED_SOURCE_RUNTIME_CACHE g_ReconstructedArtist31470;
    std::string g_strStatus = "Effect presentation service is idle.";

	bool_t Is_FiniteMatrix(const float4x4_t& Value)
	{
		const f32_t* const pValues = &Value._11;
		return std::all_of(pValues, pValues + 16u,
			[](const f32_t fValue)
			{
				return std::isfinite(fValue);
			});
	}

	f32_t Basis_Length(
		const f32_t X,
		const f32_t Y,
		const f32_t Z)
	{
		return std::sqrt(X * X + Y * Y + Z * Z);
	}

	f32_t Basis_Dot(
		const f32_t AX,
		const f32_t AY,
		const f32_t AZ,
		const f32_t BX,
		const f32_t BY,
		const f32_t BZ)
	{
		return AX * BX + AY * BY + AZ * BZ;
	}

	f32_t Basis_Determinant(const float4x4_t& Value)
	{
		return
			Value._11 * (Value._22 * Value._33 - Value._23 * Value._32) -
			Value._12 * (Value._21 * Value._33 - Value._23 * Value._31) +
			Value._13 * (Value._21 * Value._32 - Value._22 * Value._31);
	}

	bool_t Is_NonDegenerateAffineMatrix(const float4x4_t& Value)
	{
		if (!Is_FiniteMatrix(Value) ||
			std::abs(Value._14) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._24) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._34) > SOURCE_BONE_AFFINE_TOLERANCE ||
			std::abs(Value._44 - 1.f) > SOURCE_BONE_AFFINE_TOLERANCE)
		{
			return false;
		}

		const f32_t fScaleX = Basis_Length(
			Value._11, Value._12, Value._13);
		const f32_t fScaleY = Basis_Length(
			Value._21, Value._22, Value._23);
		const f32_t fScaleZ = Basis_Length(
			Value._31, Value._32, Value._33);
		if (!std::isfinite(fScaleX) || !std::isfinite(fScaleY) ||
			!std::isfinite(fScaleZ) ||
			fScaleX <= SOURCE_BONE_MINIMUM_BASIS_LENGTH ||
			fScaleY <= SOURCE_BONE_MINIMUM_BASIS_LENGTH ||
			fScaleZ <= SOURCE_BONE_MINIMUM_BASIS_LENGTH)
		{
			return false;
		}

		const f32_t fScaleProduct = fScaleX * fScaleY * fScaleZ;
		const f32_t fNormalizedDeterminant =
			std::abs(Basis_Determinant(Value)) / fScaleProduct;
		return std::isfinite(fNormalizedDeterminant) &&
			fNormalizedDeterminant >=
				SOURCE_BONE_MINIMUM_NORMALIZED_DETERMINANT;
	}

	bool_t Has_ExpectedArtistSourceBoneCombinedScale(const float4x4_t& RawBone)
	{
		if (!Is_NonDegenerateAffineMatrix(RawBone))
			return false;

		const f32_t fScaleX = Basis_Length(
			RawBone._11, RawBone._12, RawBone._13);
		const f32_t fScaleY = Basis_Length(
			RawBone._21, RawBone._22, RawBone._23);
		const f32_t fScaleZ = Basis_Length(
			RawBone._31, RawBone._32, RawBone._33);
		const f32_t fMinimumScale = (std::min)({ fScaleX, fScaleY, fScaleZ });
		const f32_t fMaximumScale = (std::max)({ fScaleX, fScaleY, fScaleZ });
		if (std::abs(fScaleX - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			std::abs(fScaleY - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			std::abs(fScaleZ - ARTIST_SOURCE_BONE_COMBINED_SCALE) >
				ARTIST_SOURCE_BONE_COMBINED_SCALE_TOLERANCE ||
			fMaximumScale - fMinimumScale >
				ARTIST_SOURCE_BONE_UNIFORM_SCALE_TOLERANCE)
		{
			return false;
		}

		const f32_t fXY = std::abs(Basis_Dot(
			RawBone._11, RawBone._12, RawBone._13,
			RawBone._21, RawBone._22, RawBone._23) /
			(fScaleX * fScaleY));
		const f32_t fXZ = std::abs(Basis_Dot(
			RawBone._11, RawBone._12, RawBone._13,
			RawBone._31, RawBone._32, RawBone._33) /
			(fScaleX * fScaleZ));
		const f32_t fYZ = std::abs(Basis_Dot(
			RawBone._21, RawBone._22, RawBone._23,
			RawBone._31, RawBone._32, RawBone._33) /
			(fScaleY * fScaleZ));
		return fXY <= SOURCE_BONE_ORTHOGONAL_TOLERANCE &&
			fXZ <= SOURCE_BONE_ORTHOGONAL_TOLERANCE &&
			fYZ <= SOURCE_BONE_ORTHOGONAL_TOLERANCE;
	}

    bool_t Prepare_TargetSet(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::set<std::string, std::less<>>& Targets,
        std::string& strOutStatus)
    {
        if (Targets.empty())
        {
            strOutStatus = "No admitted animation Effect targets require prewarm.";
            return true;
        }
        std::vector<std::pair<std::string,
            std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC>>> Documents;
        Documents.reserve(Targets.size());
        for (const std::string& EffectId : Targets)
        {
            const std::shared_ptr<const Client::EFFECT_DOCUMENT_DESC> Document =
                Client::CEffectCatalog::Find(EffectId);
            if (nullptr == Document)
            {
                strOutStatus =
                    "Animation Effect target is absent from the runtime catalog: " +
                    EffectId;
                return false;
            }
            Documents.emplace_back(EffectId, Document);
        }
        return Client::CEffectDocumentRenderer::Prepare_Catalog(
            std::move(pDevice), std::move(pContext),
            Client::CEffectCatalog::Get_RuntimeRevision(),
            Documents, strOutStatus);
    }

    bool Resolve_Anchor(
        const std::shared_ptr<Client::CCharacter>& pOwner,
        const std::string& strAnchorSlotId,
        float4x4_t& Out)
    {
        if (nullptr == pOwner || nullptr == pOwner->Get_Transform())
            return false;
        Out = *pOwner->Get_Transform()->Get_WorldMatrixPtr();
        if ("root" == strAnchorSlotId)
            return true;
        const std::shared_ptr<Engine::CModel> pModel =
            pOwner->Get_BodyModel();
        if (nullptr == pModel || !pModel->Has_Bone(strAnchorSlotId.c_str()))
            return false;
        XMStoreFloat4x4(&Out,
            pModel->Get_BoneMatrix(strAnchorSlotId.c_str()) *
            XMLoadFloat4x4(&Out));
        return true;
    }

    std::vector<SOURCE_ANCHOR_REQUEST> Collect_SourceAnchorRequests(
        const Client::EFFECT_DOCUMENT_DESC& Document)
    {
        std::vector<SOURCE_ANCHOR_REQUEST> Requests;
        const auto AddRequest = [&Requests](SOURCE_ANCHOR_REQUEST Request)
        {
            if (Request.strRuntimeAnchorSlotId.empty() ||
                Request.strRuntimeBoneName.empty())
            {
                return;
            }
            const auto Existing = std::find_if(
                Requests.begin(), Requests.end(),
                [&Request](const SOURCE_ANCHOR_REQUEST& Value)
                {
                    return Value.strRuntimeAnchorSlotId ==
                        Request.strRuntimeAnchorSlotId;
                });
            if (Existing == Requests.end())
                Requests.push_back(std::move(Request));
        };
        for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
        {
            if (Element.ActionCueAttachment.bEnabled &&
                Element.ActionCueAttachment.bFollow)
            {
                AddRequest({
                    Element.ActionCueAttachment.strRuntimeAnchorSlotId,
                    Element.ActionCueAttachment.strRuntimeBoneName,
                    Element.ActionCueAttachment.SocketLocalTransform });
            }
            for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
                Element.SourceRecipe.Modules)
            {
                if (Module.strClassName != "particlemodulelocationbonesocket")
                    continue;
                for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
                    Module.Literals)
                {
                    if (Client::EFFECT_SOURCE_LITERAL_KIND::STRING !=
                        Literal.eKind ||
                        !Literal.strPropertyPath.starts_with("sourcelocations[") ||
                        !Literal.strPropertyPath.ends_with("].bonesocketname") ||
                        Literal.strString.empty())
                    {
                        continue;
                    }
                    AddRequest({ Literal.strString, Literal.strString, {} });
                }
            }
        }
        std::sort(Requests.begin(), Requests.end(),
            [](const SOURCE_ANCHOR_REQUEST& Left,
                const SOURCE_ANCHOR_REQUEST& Right)
            {
                return Left.strRuntimeAnchorSlotId <
                    Right.strRuntimeAnchorSlotId;
            });
        return Requests;
    }

	std::vector<SOURCE_ANCHOR_REQUEST> Collect_ReconstructedAnchorRequests(
		const std::shared_ptr<const
			Client::EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation)
	{
		std::vector<SOURCE_ANCHOR_REQUEST> Requests;
		if (nullptr == pPreparation ||
			5u != pPreparation->Get_AnchorRequests().size())
		{
			return Requests;
		}
		Requests.reserve(pPreparation->Get_AnchorRequests().size());
		for (const Client::EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
			pPreparation->Get_AnchorRequests())
		{
			const Client::EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Source =
				Binding.Request;
			if (!Source.bFollow || Source.strRuntimeAnchorSlotId.empty() ||
				Source.strRuntimeBoneName.empty())
			{
				Requests.clear();
				return Requests;
			}
			SOURCE_ANCHOR_REQUEST Request;
			Request.strRuntimeAnchorSlotId = Source.strRuntimeAnchorSlotId;
			Request.strRuntimeBoneName = Source.strRuntimeBoneName;
			Request.SocketLocalTransform.vPosition = {
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vPosition[2]) };
			Request.SocketLocalTransform.vRotationDegrees = {
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vRotationDegrees[2]) };
			Request.SocketLocalTransform.vScale = {
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[0]),
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[1]),
				static_cast<f32_t>(Source.SocketLocalTransform.vScale[2]) };
			Request.bNormalizeSourceImportScale = true;
			Requests.push_back(std::move(Request));
		}
		return Requests;
	}

    std::unordered_map<std::string, float4x4_t> Resolve_SourceAnchors(
        const std::shared_ptr<Client::CCharacter>& pOwner,
        const std::vector<SOURCE_ANCHOR_REQUEST>& Requests)
    {
        std::unordered_map<std::string, float4x4_t> Result;
        if (nullptr == pOwner || nullptr == pOwner->Get_Transform())
            return Result;
        const std::shared_ptr<Engine::CModel> pModel = pOwner->Get_BodyModel();
        if (nullptr == pModel)
            return Result;
		const matrix_t OwnerWorld = XMLoadFloat4x4(
			pOwner->Get_Transform()->Get_WorldMatrixPtr());
        for (const SOURCE_ANCHOR_REQUEST& Request : Requests)
        {
            if (!pModel->Has_Bone(Request.strRuntimeBoneName.c_str()))
                continue;
			matrix_t BoneAnchorWorld;
			if (Request.bNormalizeSourceImportScale)
			{
				Client::EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
				XMStoreFloat4x4(&AnchorBuild.RawBone,
					pModel->Get_BoneMatrix(
						Request.strRuntimeBoneName.c_str()));
				XMStoreFloat4x4(&AnchorBuild.OwnerWorld, OwnerWorld);
				float4x4_t NormalizedBoneAnchorWorld{};
				if (!Client::CEffectPresentationService::
					Build_SourceBoneAnchorWorld(
						AnchorBuild, NormalizedBoneAnchorWorld))
				{
					continue;
				}
				BoneAnchorWorld =
					XMLoadFloat4x4(&NormalizedBoneAnchorWorld);
			}
			else
			{
				BoneAnchorWorld =
					pModel->Get_BoneMatrix(
						Request.strRuntimeBoneName.c_str()) *
					OwnerWorld;
			}
            const Client::EFFECT_TRANSFORM_DESC& Local =
                Request.SocketLocalTransform;
            const matrix_t SocketLocal = XMMatrixScaling(
                Local.vScale.x, Local.vScale.y, Local.vScale.z) *
                XMMatrixRotationRollPitchYaw(
                    XMConvertToRadians(Local.vRotationDegrees.x),
                    XMConvertToRadians(Local.vRotationDegrees.y),
                    XMConvertToRadians(Local.vRotationDegrees.z)) *
                XMMatrixTranslation(
                    Local.vPosition.x,
                    Local.vPosition.y,
                    Local.vPosition.z);
            float4x4_t World{};
            XMStoreFloat4x4(&World,
                SocketLocal *
				BoneAnchorWorld);
			if (Request.bNormalizeSourceImportScale &&
				!Is_NonDegenerateAffineMatrix(World))
				continue;
            Result.emplace(Request.strRuntimeAnchorSlotId, World);
        }
        return Result;
    }

    float4x4_t Compose_Local(
        const Client::EFFECT_TRANSFORM_DESC& Local,
        const float4x4_t& Anchor)
    {
        const matrix_t Scale = XMMatrixScaling(
            Local.vScale.x, Local.vScale.y, Local.vScale.z);
        const matrix_t Rotation = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(Local.vRotationDegrees.x),
            XMConvertToRadians(Local.vRotationDegrees.y),
            XMConvertToRadians(Local.vRotationDegrees.z));
        const matrix_t Translation = XMMatrixTranslation(
            Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
        float4x4_t Result{};
        XMStoreFloat4x4(&Result,
            Scale * Rotation * Translation * XMLoadFloat4x4(&Anchor));
        return Result;
    }

    void Remove_At(const size_t iIndex)
    {
        ACTIVE_EFFECT& Effect = g_ActiveEffects[iIndex];
        if (nullptr != Effect.pObject &&
            Effect.iLevelIndex == CGameInstance::Get().Get_CurrentLevelID())
        {
            CGameInstance::Get().Remove_GameObject_from_Layer(
                Effect.iLevelIndex, EFFECT_LAYER, Effect.pObject);
        }
        g_ActiveEffects.erase(g_ActiveEffects.begin() + iIndex);
    }
}

bool_t Client::CEffectPresentationService::Prepare_ProductCues(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    const std::vector<ANIMATION_EFFECT_CUE>& Cues,
    std::string& strOutStatus)
{
    std::set<std::string, std::less<>> StagedTargets =
        g_ProductEffectTargets;
    for (const ANIMATION_EFFECT_CUE& Cue : Cues)
    {
        if (Cue.strEffectAssetId.empty() ||
            !CEffectCatalog::Contains(Cue.strEffectAssetId))
        {
            strOutStatus =
                "Animation Effect cue target is not admitted by the catalog.";
            g_strStatus = strOutStatus;
            return false;
        }
        StagedTargets.insert(Cue.strEffectAssetId);
    }
    if (StagedTargets == g_ProductEffectTargets)
    {
        strOutStatus = "Animation Effect cue targets are already prepared.";
        g_strStatus = strOutStatus;
        return true;
    }
    if (!Prepare_TargetSet(
        std::move(pDevice), std::move(pContext), StagedTargets, strOutStatus))
    {
        g_strStatus = "Animation Effect prewarm failed; previous cache preserved: " +
            strOutStatus;
        strOutStatus = g_strStatus;
        return false;
    }
    g_ProductEffectTargets = std::move(StagedTargets);
    g_strStatus = strOutStatus;
    return true;
}

bool_t Client::CEffectPresentationService::Reprepare_ProductTargets(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    const std::vector<std::string>& AdditionalEffectAssetIds,
    std::string& strOutStatus)
{
    std::set<std::string, std::less<>> StagedTargets =
        g_ProductEffectTargets;
    for (const std::string& EffectId : AdditionalEffectAssetIds)
    {
        if (EffectId.empty())
        {
            strOutStatus = "Additional Effect prewarm target is empty.";
            g_strStatus = strOutStatus;
            return false;
        }
        StagedTargets.insert(EffectId);
    }
    if (!Prepare_TargetSet(
        std::move(pDevice), std::move(pContext), StagedTargets, strOutStatus))
    {
        g_strStatus = "Reloaded Effect prewarm failed; previous cache preserved: " +
            strOutStatus;
        strOutStatus = g_strStatus;
        return false;
    }
    g_ProductEffectTargets = std::move(StagedTargets);
    g_strStatus = strOutStatus;
    return true;
}

bool_t Client::CEffectPresentationService::Build_SourceBoneAnchorWorld(
	const EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC& Desc,
	float4x4_t& OutWorld)
{
	OutWorld = {};
	if (!Has_ExpectedArtistSourceBoneCombinedScale(Desc.RawBone) ||
		!Is_NonDegenerateAffineMatrix(Desc.OwnerWorld))
	{
		return false;
	}

	float4x4_t BoneWithoutImportScale = Desc.RawBone;
	constexpr f32_t COMBINED_SCALE_RECIPROCAL =
		1.f / ARTIST_SOURCE_BONE_COMBINED_SCALE;
	BoneWithoutImportScale._11 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._12 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._13 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._21 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._22 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._23 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._31 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._32 *= COMBINED_SCALE_RECIPROCAL;
	BoneWithoutImportScale._33 *= COMBINED_SCALE_RECIPROCAL;
	XMStoreFloat4x4(&OutWorld,
		XMLoadFloat4x4(&BoneWithoutImportScale) *
		XMLoadFloat4x4(&Desc.OwnerWorld));
	if (!Is_NonDegenerateAffineMatrix(OutWorld))
	{
		OutWorld = {};
		return false;
	}
	return true;
}

bool_t Client::CEffectPresentationService::Prepare_ReconstructedRuntimeProgram(
	const std::string& strEffectAssetId,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Prepare_Presentation(
		strEffectAssetId, OutPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	strOutStatus = "Prepared reconstructed Effect program for inspection.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Prepare_ReconstructedArtist31470(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::string& strOutStatus)
{
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation;
	if (!CEffectReconstructedRuntimeBoundary::Prepare_Presentation(
		std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID),
		pPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		pPreparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		pPreparation->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram ||
		pEntry->Get_Program().get() != pProgram.get() ||
		pEntry->Get_Identity().strEffectAssetId !=
			RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
		pProgram->Admission.bRuntimeExecution || pProgram->Admission.bProduct)
	{
		strOutStatus =
			"Artist 31470 reconstructed preparation identity is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	if (nullptr != g_ReconstructedArtist31470.pPreparation &&
		g_ReconstructedArtist31470.pPreparation->Get_CatalogEntry().get() ==
			pEntry.get() && nullptr != g_ReconstructedArtist31470.pDocument &&
		nullptr != g_ReconstructedArtist31470.pPrepared)
	{
		strOutStatus =
			"Artist 31470 reconstructed source runtime is already prepared.";
		g_strStatus = strOutStatus;
		return true;
	}

	EFFECT_DOCUMENT_DESC Document;
	if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
		pPreparation, Document, strOutStatus,
		EFFECT_RECONSTRUCTED_VISUAL_SCOPE::V4_ARTIST_F_MAIN_REVIEW))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	auto pDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(Document));
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT> pPrepared;
	if (!CEffectDocumentRenderer::Prepare_ReconstructedSourceRuntime(
		std::move(pDevice), std::move(pContext), pPreparation, *pDocument,
		pPrepared, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	if (nullptr == pPrepared)
	{
		strOutStatus =
			"Artist 31470 reconstructed resource preparation returned no result.";
		g_strStatus = strOutStatus;
		return false;
	}

	RECONSTRUCTED_SOURCE_RUNTIME_CACHE Staged;
	Staged.pPreparation = std::move(pPreparation);
	Staged.pDocument = std::move(pDocument);
	Staged.pPrepared = std::move(pPrepared);
	g_ReconstructedArtist31470 = std::move(Staged);
	strOutStatus =
		"Artist 31470 V4 material-composition review (#9/#10/#11) prepared; zero occurrences are admitted and Product remains false.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Acquire_ReconstructedArtist31470(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
		OutPreparation,
	std::string& strOutStatus)
{
	OutPreparation.reset();
	if (!Prepare_ReconstructedArtist31470(
		std::move(pDevice), std::move(pContext), strOutStatus))
	{
		return false;
	}
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		pCurrentEntry = CEffectCatalog::Find_RuntimeProgramEntry(
			std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation = g_ReconstructedArtist31470.pPreparation;
	if (nullptr == pCurrentEntry || nullptr == pPreparation ||
		pPreparation->Get_CatalogEntry().get() != pCurrentEntry.get() ||
		nullptr == g_ReconstructedArtist31470.pDocument ||
		nullptr == g_ReconstructedArtist31470.pPrepared)
	{
		strOutStatus =
			"Artist 31470 reconstructed prepared cache identity is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	OutPreparation = pPreparation;
	strOutStatus =
		"Artist 31470 reconstructed prepared cache acquired; Product remains false.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::
	Stage_ReconstructedArtist31470Preview(
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		std::string& strOutStatus)
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		pCurrentEntry = CEffectCatalog::Find_RuntimeProgramEntry(
			std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation = g_ReconstructedArtist31470.pPreparation;
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pPreparation ? nullptr : pPreparation->Get_Program();
	if (nullptr == pObject || nullptr == pExpectedPreparation ||
		nullptr == pCurrentEntry || nullptr == pPreparation ||
		pPreparation.get() != pExpectedPreparation.get() ||
		pPreparation->Get_CatalogEntry().get() != pCurrentEntry.get() ||
		nullptr == pProgram || pProgram->Admission.bRuntimeExecution ||
		pProgram->Admission.bProduct ||
		nullptr == g_ReconstructedArtist31470.pDocument ||
		nullptr == g_ReconstructedArtist31470.pPrepared)
	{
		strOutStatus =
			"Artist 31470 reconstructed preview cache identity is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	if (!pObject->Stage_ReconstructedSourceRuntime(
		*g_ReconstructedArtist31470.pDocument,
		g_ReconstructedArtist31470.pPrepared,
		pPreparation, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	strOutStatus =
		"Artist 31470 reconstructed preview attached from the shared prepared cache.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Spawn_ReconstructedArtist31470(
	const EFFECT_SPAWN_DESC& Desc,
	std::string& strOutStatus)
{
	const std::shared_ptr<CCharacter> pOwner = Desc.pOwner.lock();
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		pCurrentEntry = CEffectCatalog::Find_RuntimeProgramEntry(
			std::string(RECONSTRUCTED_ARTIST_31470_ASSET_ID));
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation = g_ReconstructedArtist31470.pPreparation;
	if (Desc.strEffectAssetId != RECONSTRUCTED_ARTIST_31470_ASSET_ID ||
		nullptr == pOwner || nullptr == pOwner->Get_BodyModel() ||
		nullptr == pCurrentEntry || nullptr == pPreparation ||
		pPreparation->Get_CatalogEntry().get() != pCurrentEntry.get() ||
		nullptr == g_ReconstructedArtist31470.pDocument ||
		nullptr == g_ReconstructedArtist31470.pPrepared ||
		Desc.strAnchorSlotId.empty() || Desc.strOccurrenceId.empty() ||
		0u == Desc.iActionStartTick ||
		!std::isfinite(Desc.fPlaybackRate) || Desc.fPlaybackRate <= 0.f ||
		Desc.fPlaybackRate > 16.f ||
		!std::isfinite(Desc.fInitialSampleTimeSeconds) ||
		Desc.fInitialSampleTimeSeconds < 0.f ||
		EFFECT_FOLLOW_POLICY::END == Desc.eFollowPolicy ||
		EFFECT_STOP_POLICY::END == Desc.eStopPolicy)
	{
		strOutStatus =
			"Artist 31470 reconstructed spawn descriptor/cache is invalid.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		pPreparation->Get_Program();
	if (nullptr == pProgram || pProgram->Admission.bRuntimeExecution ||
		pProgram->Admission.bProduct)
	{
		strOutStatus =
			"Artist 31470 reconstructed spawn cannot cross Product admission.";
		g_strStatus = strOutStatus;
		return false;
	}
	if (std::any_of(g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[&Desc, &pOwner](const ACTIVE_EFFECT& Effect)
		{
			return Effect.pOwner.lock() == pOwner &&
				Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
		}))
	{
		strOutStatus = "Duplicate Artist 31470 action edge ignored.";
		g_strStatus = strOutStatus;
		return false;
	}

	float4x4_t Anchor{};
	if (!Resolve_Anchor(pOwner, Desc.strAnchorSlotId, Anchor))
	{
		strOutStatus = "Artist 31470 root anchor is missing.";
		g_strStatus = strOutStatus;
		return false;
	}
	std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests =
		Collect_ReconstructedAnchorRequests(pPreparation);
	if (5u != SourceAnchorRequests.size() ||
		std::any_of(SourceAnchorRequests.begin(), SourceAnchorRequests.end(),
			[&pOwner](const SOURCE_ANCHOR_REQUEST& Request)
			{
				return !pOwner->Get_BodyModel()->Has_Bone(
					Request.strRuntimeBoneName.c_str());
			}))
	{
		strOutStatus =
			"Artist 31470 exact b_wp_1 source-anchor contract is unavailable.";
		g_strStatus = strOutStatus;
		return false;
	}
	const float4x4_t Root = Compose_Local(Desc.LocalTransform, Anchor);
	CEffectObject::EFFECT_OBJECT_DESC ObjectDesc{};
	ObjectDesc.RootWorld = Root;
	ObjectDesc.bAutoPlay = false;
	ObjectDesc.fPlaybackRate = Desc.fPlaybackRate;
	const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	const EFFECT_RENDER_PREWARM_PROBE ProbeBefore =
		CEffectDocumentRenderer::Get_PrewarmProbe();
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
		iLevelIndex, EFFECT_LAYER, &ObjectDesc, &pGameObject)))
	{
		strOutStatus = "Artist 31470 EffectObject clone failed.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<CEffectObject> pEffect =
		std::dynamic_pointer_cast<CEffectObject>(pGameObject);
	if (nullptr == pEffect ||
		!pEffect->Stage_ReconstructedSourceRuntime(
			*g_ReconstructedArtist31470.pDocument,
			g_ReconstructedArtist31470.pPrepared,
			pPreparation, strOutStatus))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		if (nullptr == pEffect)
			strOutStatus = "Artist 31470 EffectObject prototype type mismatch.";
		g_strStatus = strOutStatus;
		return false;
	}
	const EFFECT_RENDER_PREWARM_PROBE ProbeAfter =
		CEffectDocumentRenderer::Get_PrewarmProbe();
	if (ProbeAfter.iCoreBuildCount != ProbeBefore.iCoreBuildCount ||
		ProbeAfter.iModelDiskLoadCount != ProbeBefore.iModelDiskLoadCount ||
		ProbeAfter.iTextureDiskLoadCount != ProbeBefore.iTextureDiskLoadCount ||
		ProbeAfter.iVectorFieldDiskLoadCount !=
			ProbeBefore.iVectorFieldDiskLoadCount ||
		ProbeAfter.iSynchronousDocumentStageCount !=
			ProbeBefore.iSynchronousDocumentStageCount ||
		ProbeAfter.iPreparedAttachCount !=
			ProbeBefore.iPreparedAttachCount + 1u)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			iLevelIndex, EFFECT_LAYER, pGameObject);
		strOutStatus =
			"Artist 31470 action edge violated the prepared no-I/O contract.";
		g_strStatus = strOutStatus;
		return false;
	}
	pEffect->Set_SourceAnchorWorlds(
		Resolve_SourceAnchors(pOwner, SourceAnchorRequests));

	ACTIVE_EFFECT Active;
	Active.pObject = pEffect;
	Active.pOwner = pOwner;
	Active.iLevelIndex = iLevelIndex;
	Active.strEffectAssetId = Desc.strEffectAssetId;
	Active.strAnchorSlotId = Desc.strAnchorSlotId;
	Active.LocalTransform = Desc.LocalTransform;
	Active.eFollowPolicy = Desc.eFollowPolicy;
	Active.eStopPolicy = Desc.eStopPolicy;
	Active.iCueDurationMs = Desc.iCueDurationMs;
	Active.iActionStartTick = Desc.iActionStartTick;
	Active.iCueStartMs = Desc.iCueStartMs;
	Active.strOccurrenceId = Desc.strOccurrenceId;
	Active.fPlaybackRate = Desc.fPlaybackRate;
	Active.fElapsedCueTimeSeconds = Desc.fInitialSampleTimeSeconds;
	Active.fPendingInitialSampleTimeSeconds =
		Desc.fInitialSampleTimeSeconds;
	Active.SourceAnchorRequests = std::move(SourceAnchorRequests);
	g_ActiveEffects.push_back(std::move(Active));
	strOutStatus =
		"Spawned nonProduct Artist 31470 reconstructed effect at authoritative action age.";
	g_strStatus = strOutStatus;
	return true;
}

bool_t Client::CEffectPresentationService::Spawn(
    const EFFECT_SPAWN_DESC& Desc,
    std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn(
		Desc.strEffectAssetId, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<CCharacter> pOwner = Desc.pOwner.lock();
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
		CEffectCatalog::Find(Desc.strEffectAssetId);
	const bool_t bDescriptorValid = nullptr != pOwner && nullptr != pDocument &&
		!Desc.strAnchorSlotId.empty() && !Desc.strOccurrenceId.empty() &&
		std::isfinite(Desc.fPlaybackRate) && Desc.fPlaybackRate > 0.f &&
		Desc.fPlaybackRate <= 16.f &&
		std::isfinite(Desc.fInitialSampleTimeSeconds) &&
		Desc.fInitialSampleTimeSeconds >= 0.f &&
		EFFECT_FOLLOW_POLICY::END != Desc.eFollowPolicy &&
		EFFECT_STOP_POLICY::END != Desc.eStopPolicy &&
		(EFFECT_STOP_POLICY::CUE_END != Desc.eStopPolicy ||
			0u != Desc.iCueDurationMs);
	if (!bDescriptorValid)
	{
		strOutStatus = "Effect spawn descriptor is invalid or not admitted.";
		g_strStatus = strOutStatus;
		return false;
	}
	const std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pPrepared = CEffectDocumentRenderer::Find_Prepared(
			CEffectCatalog::Get_RuntimeRevision(),
			Desc.strEffectAssetId, *pDocument);
	if (nullptr == pPrepared)
	{
		strOutStatus =
			"Effect spawn rejected because its admitted animation target was not prewarmed.";
		g_strStatus = strOutStatus;
		return false;
	}
	const auto SameEdge = [&Desc, &pOwner](const auto& Effect)
	{
		return Effect.Desc.pOwner.lock() == pOwner &&
			Effect.Desc.iActionStartTick == Desc.iActionStartTick &&
			Effect.Desc.strOccurrenceId == Desc.strOccurrenceId;
	};
	const bool_t bActiveDuplicate = std::any_of(
		g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[&Desc, &pOwner](const ACTIVE_EFFECT& Effect)
		{
			return Effect.pOwner.lock() == pOwner &&
				Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
		});
	if (bActiveDuplicate ||
		std::any_of(g_PendingEffectSpawns.begin(),
			g_PendingEffectSpawns.end(), SameEdge))
	{
		strOutStatus = "Duplicate Effect cue edge ignored.";
		g_strStatus = strOutStatus;
		return false;
	}

	PENDING_EFFECT_SPAWN Pending;
	Pending.Desc = Desc;
	Pending.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	g_PendingEffectSpawns.push_back(std::move(Pending));
	strOutStatus = "Queued admitted Effect for post-update layer commit: " +
		Desc.strEffectAssetId;
	g_strStatus = strOutStatus;
	return true;
}

void Client::CEffectPresentationService::Commit_PendingSpawns()
{
	if (g_PendingEffectSpawns.empty())
		return;

	std::vector<PENDING_EFFECT_SPAWN> Pending =
		std::move(g_PendingEffectSpawns);
	g_PendingEffectSpawns.clear();
	const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
	for (PENDING_EFFECT_SPAWN& Request : Pending)
	{
		if (Request.iLevelIndex != iCurrentLevel)
		{
			g_strStatus =
				"Discarded queued Effect because its source level changed.";
			continue;
		}
		std::string Status;
		if (!Spawn_Immediate(Request.Desc, Status))
		{
			OutputDebugStringA((
				"Deferred Effect spawn failed after Object Manager update: " +
				Status + "\n").c_str());
		}
	}
}

bool_t Client::CEffectPresentationService::Spawn_Immediate(
    const EFFECT_SPAWN_DESC& Desc,
    std::string& strOutStatus)
{
	if (!CEffectReconstructedRuntimeBoundary::Admit_ProductSpawn(
		Desc.strEffectAssetId, strOutStatus))
	{
		g_strStatus = strOutStatus;
		return false;
	}
    const std::shared_ptr<CCharacter> pOwner = Desc.pOwner.lock();
    const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
        CEffectCatalog::Find(Desc.strEffectAssetId);
    if (nullptr == pOwner || nullptr == pDocument ||
        Desc.strAnchorSlotId.empty() ||
		Desc.strOccurrenceId.empty() ||
		!std::isfinite(Desc.fPlaybackRate) ||
		Desc.fPlaybackRate <= 0.f || Desc.fPlaybackRate > 16.f ||
        !std::isfinite(Desc.fInitialSampleTimeSeconds) ||
        Desc.fInitialSampleTimeSeconds < 0.f ||
        EFFECT_FOLLOW_POLICY::END == Desc.eFollowPolicy ||
        EFFECT_STOP_POLICY::END == Desc.eStopPolicy ||
        (EFFECT_STOP_POLICY::CUE_END == Desc.eStopPolicy &&
            0u == Desc.iCueDurationMs))
    {
        strOutStatus = "Effect spawn descriptor is invalid or not admitted.";
        g_strStatus = strOutStatus;
        return false;
    }
    const std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
        pPrepared = CEffectDocumentRenderer::Find_Prepared(
            CEffectCatalog::Get_RuntimeRevision(),
            Desc.strEffectAssetId, *pDocument);
    if (nullptr == pPrepared)
    {
        strOutStatus =
            "Effect spawn rejected because its admitted animation target was not prewarmed.";
        g_strStatus = strOutStatus;
        return false;
    }
    const bool Duplicate = std::any_of(
        g_ActiveEffects.begin(), g_ActiveEffects.end(),
        [&Desc, &pOwner](const ACTIVE_EFFECT& Effect)
        {
            return Effect.pOwner.lock() == pOwner &&
                Effect.iActionStartTick == Desc.iActionStartTick &&
				Effect.strOccurrenceId == Desc.strOccurrenceId;
        });
    if (Duplicate)
    {
        strOutStatus = "Duplicate Effect cue edge ignored.";
        g_strStatus = strOutStatus;
        return false;
    }

    float4x4_t Anchor{};
    if (!Resolve_Anchor(pOwner, Desc.strAnchorSlotId, Anchor))
    {
        strOutStatus = "Effect anchor is missing on the owner skeleton.";
        g_strStatus = strOutStatus;
        return false;
    }
    const float4x4_t Root = Compose_Local(Desc.LocalTransform, Anchor);
    std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests =
        Collect_SourceAnchorRequests(*pDocument);
    CEffectObject::EFFECT_OBJECT_DESC ObjectDesc{};
    ObjectDesc.pDocument = pDocument.get();
    ObjectDesc.pPreparedResources = pPrepared;
    ObjectDesc.RootWorld = Root;
    ObjectDesc.bAutoPlay = false;
    ObjectDesc.bRequirePreparedResources = true;
	ObjectDesc.fPlaybackRate = Desc.fPlaybackRate;
    const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
    const EFFECT_RENDER_PREWARM_PROBE ProbeBefore =
        CEffectDocumentRenderer::Get_PrewarmProbe();
    std::shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
        iLevelIndex, EFFECT_LAYER, &ObjectDesc, &pGameObject)))
    {
        strOutStatus = "EffectObject clone failed.";
        g_strStatus = strOutStatus;
        return false;
    }
    const std::shared_ptr<CEffectObject> pEffect =
        std::dynamic_pointer_cast<CEffectObject>(pGameObject);
    if (nullptr == pEffect)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevelIndex, EFFECT_LAYER, pGameObject);
        strOutStatus = "EffectObject prototype returned the wrong type.";
        g_strStatus = strOutStatus;
        return false;
    }
    const EFFECT_RENDER_PREWARM_PROBE ProbeAfter =
        CEffectDocumentRenderer::Get_PrewarmProbe();
    if (ProbeAfter.iCoreBuildCount != ProbeBefore.iCoreBuildCount ||
        ProbeAfter.iModelDiskLoadCount != ProbeBefore.iModelDiskLoadCount ||
        ProbeAfter.iTextureDiskLoadCount != ProbeBefore.iTextureDiskLoadCount ||
		ProbeAfter.iVectorFieldDiskLoadCount !=
			ProbeBefore.iVectorFieldDiskLoadCount ||
        ProbeAfter.iSynchronousDocumentStageCount !=
            ProbeBefore.iSynchronousDocumentStageCount ||
        ProbeAfter.iPreparedAttachCount !=
            ProbeBefore.iPreparedAttachCount + 1u)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevelIndex, EFFECT_LAYER, pGameObject);
        strOutStatus =
            "Effect spawn violated the prepared no-I/O resource contract.";
        g_strStatus = strOutStatus;
        return false;
    }

    ACTIVE_EFFECT Active;
    Active.pObject = pEffect;
    Active.pOwner = pOwner;
    Active.iLevelIndex = iLevelIndex;
    Active.strEffectAssetId = Desc.strEffectAssetId;
    Active.strAnchorSlotId = Desc.strAnchorSlotId;
    Active.LocalTransform = Desc.LocalTransform;
    Active.eFollowPolicy = Desc.eFollowPolicy;
    Active.eStopPolicy = Desc.eStopPolicy;
    Active.iCueDurationMs = Desc.iCueDurationMs;
    Active.iActionStartTick = Desc.iActionStartTick;
    Active.iCueStartMs = Desc.iCueStartMs;
	Active.strOccurrenceId = Desc.strOccurrenceId;
    Active.fPlaybackRate = Desc.fPlaybackRate;
    Active.fElapsedCueTimeSeconds = Desc.fInitialSampleTimeSeconds;
    Active.fPendingInitialSampleTimeSeconds =
        Desc.fInitialSampleTimeSeconds;
    Active.SourceAnchorRequests = std::move(SourceAnchorRequests);
    g_ActiveEffects.push_back(std::move(Active));
    strOutStatus = "Spawned admitted Effect: " + Desc.strEffectAssetId;
    g_strStatus = strOutStatus;
    return true;
}

void Client::CEffectPresentationService::Update(const f32_t fTimeDelta)
{
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        ACTIVE_EFFECT& Effect = g_ActiveEffects[iEffect];
        const std::shared_ptr<CCharacter> pOwner = Effect.pOwner.lock();
		if (nullptr != Effect.pObject &&
			Effect.pObject->Is_RenderFailureIsolated())
		{
			g_strStatus = Effect.pObject->Get_Status();
			OutputDebugStringA(("[Client][EffectPresentation] " +
				g_strStatus + "\n").c_str());
			Remove_At(iEffect);
			continue;
		}
        if (Effect.bPendingInitialSeek && nullptr != Effect.pObject &&
            !Effect.bFollowAnchorMissing)
        {
            Effect.pObject->Set_SampleTime(
                Effect.fPendingInitialSampleTimeSeconds);
            Effect.bPendingInitialSeek = false;
        }
        else if (nullptr != Effect.pObject && !Effect.bFollowAnchorMissing)
        {
            const f32_t fCueDelta =
                (std::max)(0.f, fTimeDelta) * Effect.fPlaybackRate;
            Effect.pObject->Advance_Preview(fCueDelta);
            Effect.fElapsedCueTimeSeconds += fCueDelta;
        }
        const bool bCueEnded =
            EFFECT_STOP_POLICY::CUE_END == Effect.eStopPolicy &&
            Effect.fElapsedCueTimeSeconds * 1000.f >= Effect.iCueDurationMs;
        if (nullptr == pOwner || nullptr == Effect.pObject ||
            Effect.iLevelIndex != iCurrentLevel || bCueEnded ||
            Effect.pObject->Is_Finished() || Effect.bFollowAnchorMissing)
        {
            Remove_At(iEffect);
            continue;
        }
    }
}

void Client::CEffectPresentationService::Synchronize_FollowAnchors()
{
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    for (ACTIVE_EFFECT& Effect : g_ActiveEffects)
    {
        const std::shared_ptr<CCharacter> pOwner = Effect.pOwner.lock();
        if (nullptr == pOwner || nullptr == Effect.pObject ||
            Effect.iLevelIndex != iCurrentLevel)
        {
            Effect.bFollowAnchorMissing = true;
            if (nullptr != Effect.pObject)
                Effect.pObject->Set_Visible(false);
            continue;
        }
        Effect.pObject->Set_SourceAnchorWorlds(
            Resolve_SourceAnchors(pOwner, Effect.SourceAnchorRequests));
        if (EFFECT_FOLLOW_POLICY::FOLLOW != Effect.eFollowPolicy &&
            !Effect.bPendingInitialSeek)
            continue;
        float4x4_t Anchor{};
        if (!Resolve_Anchor(pOwner, Effect.strAnchorSlotId, Anchor))
        {
            Effect.bFollowAnchorMissing = true;
            Effect.pObject->Set_Visible(false);
            continue;
        }
        Effect.pObject->Set_RootWorld(
            Compose_Local(Effect.LocalTransform, Anchor));
    }
}

void Client::CEffectPresentationService::Stop_Owner(
    const std::shared_ptr<CCharacter>& pOwner)
{
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[&pOwner](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.Desc.pOwner.lock() == pOwner;
		}), g_PendingEffectSpawns.end());
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].pOwner.lock() == pOwner)
            Remove_At(iEffect);
    }
}

void Client::CEffectPresentationService::Clear_Level(
    const uint32_t iLevelIndex)
{
	g_PendingEffectSpawns.erase(std::remove_if(
		g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
		[iLevelIndex](const PENDING_EFFECT_SPAWN& Pending)
		{
			return Pending.iLevelIndex == iLevelIndex;
		}), g_PendingEffectSpawns.end());
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].iLevelIndex == iLevelIndex)
            Remove_At(iEffect);
    }
}

void Client::CEffectPresentationService::Clear_All()
{
	g_PendingEffectSpawns.clear();
    while (!g_ActiveEffects.empty())
        Remove_At(g_ActiveEffects.size() - 1u);
    g_strStatus = "All runtime Effect instances cleared.";
}

void Client::CEffectPresentationService::Release_PreparedResources()
{
    Clear_All();
    g_ProductEffectTargets.clear();
	g_ReconstructedArtist31470 = {};
    CEffectDocumentRenderer::Clear_Prepared_Catalog();
    g_strStatus = "Effect product prewarm resources released.";
}

const std::string& Client::CEffectPresentationService::Get_Status()
{
    return g_strStatus;
}

