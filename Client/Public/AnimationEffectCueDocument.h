#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "HitAreaWire.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class EFFECT_PRODUCT_CUE_ADMISSION_TOKEN;

enum class EFFECT_FOLLOW_POLICY : uint8_t
{
    FOLLOW,
    SNAPSHOT,
    END
};

enum class EFFECT_STOP_POLICY : uint8_t
{
    NATURAL,
    CUE_END,
    END
};

struct ANIMATION_EFFECT_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    uint32_t iEndMs = 0u;
    std::string strEffectAssetId;
    std::string strAnchorSlotId = "root";
    EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	std::shared_ptr<const EFFECT_PRODUCT_CUE_ADMISSION_TOKEN>
		pProductAdmissionToken;
};

struct ANIMATION_HIT_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    uint32_t iEndMs = 0u;
    uint32_t iRepeatCount = 1u;
    uint32_t iRepeatMs = 0u;
    HIT_AREA_SHAPE Shape{};
};

struct ANIMATION_EFFECT_CUE_DOCUMENT final
{
    uint32_t iFormatVersion = 5u;
    std::string strAnimationAssetId;
    std::vector<ANIMATION_EFFECT_CUE> Cues;
    std::vector<ANIMATION_HIT_CUE> Hits;
};

class CAnimationEffectCueDocument final
{
public:
    static bool_t Load(
        const std::string& strAnimationAssetId,
        const std::vector<std::string>& AvailableClips,
        ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
        std::string& strOutStatus);
};

NS_END

