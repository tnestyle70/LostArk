#include "Effect_PresentationService.h"

#include "Character.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr const wchar_t* EFFECT_LAYER = L"Layer_Effect";

    struct SOURCE_ANCHOR_REQUEST final
    {
        std::string strRuntimeAnchorSlotId;
        std::string strRuntimeBoneName;
        Client::EFFECT_TRANSFORM_DESC SocketLocalTransform{};
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
        f32_t fElapsedSeconds = 0.f;
        bool_t bFollowAnchorMissing = false;
        std::vector<SOURCE_ANCHOR_REQUEST> SourceAnchorRequests;
    };

    std::vector<ACTIVE_EFFECT> g_ActiveEffects;
    std::string g_strStatus = "Effect presentation service is idle.";

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
                pModel->Get_BoneMatrix(Request.strRuntimeBoneName.c_str()) *
                OwnerWorld);
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

bool_t Client::CEffectPresentationService::Spawn(
    const EFFECT_SPAWN_DESC& Desc,
    std::string& strOutStatus)
{
    const std::shared_ptr<CCharacter> pOwner = Desc.pOwner.lock();
    const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument =
        CEffectCatalog::Find(Desc.strEffectAssetId);
    if (nullptr == pOwner || nullptr == pDocument ||
        Desc.strAnchorSlotId.empty() ||
		!std::isfinite(Desc.fPlaybackRate) ||
		Desc.fPlaybackRate <= 0.f || Desc.fPlaybackRate > 16.f ||
        EFFECT_FOLLOW_POLICY::END == Desc.eFollowPolicy ||
        EFFECT_STOP_POLICY::END == Desc.eStopPolicy ||
        (EFFECT_STOP_POLICY::CUE_END == Desc.eStopPolicy &&
            0u == Desc.iCueDurationMs))
    {
        strOutStatus = "Effect spawn descriptor is invalid or not admitted.";
        g_strStatus = strOutStatus;
        return false;
    }
    const bool Duplicate = std::any_of(
        g_ActiveEffects.begin(), g_ActiveEffects.end(),
        [&Desc, &pOwner](const ACTIVE_EFFECT& Effect)
        {
            return Effect.pOwner.lock() == pOwner &&
                Effect.iActionStartTick == Desc.iActionStartTick &&
                Effect.iCueStartMs == Desc.iCueStartMs &&
                Effect.strEffectAssetId == Desc.strEffectAssetId &&
                Effect.strAnchorSlotId == Desc.strAnchorSlotId;
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
    CEffectObject::EFFECT_OBJECT_DESC ObjectDesc{};
    ObjectDesc.pDocument = pDocument.get();
    ObjectDesc.RootWorld = Root;
    ObjectDesc.bAutoPlay = true;
	ObjectDesc.fPlaybackRate = Desc.fPlaybackRate;
    const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
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
    Active.SourceAnchorRequests = Collect_SourceAnchorRequests(*pDocument);
    pEffect->Set_SourceAnchorWorlds(
        Resolve_SourceAnchors(pOwner, Active.SourceAnchorRequests));
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
        Effect.fElapsedSeconds += (std::max)(0.f, fTimeDelta);
        const std::shared_ptr<CCharacter> pOwner = Effect.pOwner.lock();
        const bool bCueEnded =
            EFFECT_STOP_POLICY::CUE_END == Effect.eStopPolicy &&
            Effect.fElapsedSeconds * 1000.f >= Effect.iCueDurationMs;
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
        if (EFFECT_FOLLOW_POLICY::FOLLOW != Effect.eFollowPolicy)
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
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].pOwner.lock() == pOwner)
            Remove_At(iEffect);
    }
}

void Client::CEffectPresentationService::Clear_Level(
    const uint32_t iLevelIndex)
{
    for (size_t iEffect = g_ActiveEffects.size(); iEffect-- > 0u;)
    {
        if (g_ActiveEffects[iEffect].iLevelIndex == iLevelIndex)
            Remove_At(iEffect);
    }
}

void Client::CEffectPresentationService::Clear_All()
{
    while (!g_ActiveEffects.empty())
        Remove_At(g_ActiveEffects.size() - 1u);
    g_strStatus = "All runtime Effect instances cleared.";
}

const std::string& Client::CEffectPresentationService::Get_Status()
{
    return g_strStatus;
}

