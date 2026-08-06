#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"

#include <cstdint>
#include <memory>
#include <string>

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
};

class CEffectPresentationService final
{
public:
    static bool_t Spawn(
        const EFFECT_SPAWN_DESC& Desc,
        std::string& strOutStatus);
    static void Update(f32_t fTimeDelta);
    static void Synchronize_FollowAnchors();
    static void Stop_Owner(const std::shared_ptr<CCharacter>& pOwner);
    static void Clear_Level(uint32_t iLevelIndex);
    static void Clear_All();
    static const std::string& Get_Status();
};

NS_END

