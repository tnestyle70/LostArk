#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;

struct EFFECT_SPAWN_DESC final
{
    std::string strEffectAssetId;
    std::weak_ptr<CCharacter> pOwner;
    std::string strAnchorSlotId = "root";
    EFFECT_TRANSFORM_DESC LocalTransform{};
    EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    uint32_t iCueDurationMs = 0u;
    uint32_t iActionStartTick = 0u;
    uint32_t iCueStartMs = 0u;
	std::string strOccurrenceId;
    f32_t fPlaybackRate = 1.f;
    f32_t fInitialSampleTimeSeconds = 0.f;
};

class CEffectPresentationService final
{
public:
    static bool_t Prepare_ProductCues(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<ANIMATION_EFFECT_CUE>& Cues,
        std::string& strOutStatus);
    static bool_t Reprepare_ProductTargets(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<std::string>& AdditionalEffectAssetIds,
        std::string& strOutStatus);
    static bool_t Spawn(
        const EFFECT_SPAWN_DESC& Desc,
        std::string& strOutStatus);
    static void Update(f32_t fTimeDelta);
    static void Synchronize_FollowAnchors();
    static void Stop_Owner(const std::shared_ptr<CCharacter>& pOwner);
    static void Clear_Level(uint32_t iLevelIndex);
    static void Clear_All();
    static void Release_PreparedResources();
    static const std::string& Get_Status();
};

NS_END

