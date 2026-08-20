#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "HitAreaWire.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

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

/* An object the clip spawns (from <Asset>.projectiles.json): where the Server
   really judges the hits. The client only predicts it for the Debug wire; the
   authoritative judgement, timing and damage stay on the Server. */
struct ANIMATION_PROJECTILE_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    /* 0 MISSILE, 1 FIXAREA, 2 GRENADE, 3 TRACE */
    uint32_t iKind = 0u;
    /* 0 on the caster (offset forward/right in metres), 1 at the aim point */
    uint32_t iOrigin = 0u;
    f32_t fOffsetForward = 0.f;
    f32_t fOffsetRight = 0.f;
    f32_t fSpeed = 0.f;
    f32_t fMinDistance = 0.f;
    f32_t fMaxDistance = 0.f;
    uint32_t iLifeMs = 0u;
    f32_t fRadius = 0.f;
    /* Shapes the object applies, in the same cm units as HIT rows. */
    std::vector<HIT_AREA_SHAPE> Shapes;
};

struct ANIMATION_EFFECT_CUE_DOCUMENT final
{
    uint32_t iFormatVersion = 5u;
    std::string strAnimationAssetId;
    std::vector<ANIMATION_EFFECT_CUE> Cues;
    std::vector<ANIMATION_HIT_CUE> Hits;
    std::vector<ANIMATION_PROJECTILE_CUE> Projectiles;
};

class CAnimationEffectCueDocument final
{
public:
    /* Loading-level path. It discovers referenced clips while parsing the
       authored rows once, so no live CModel or second animevents read is
       required just to register Product prewarm targets. */
    static bool_t Load_ForProductPrewarm(
        const std::string& strAnimationAssetId,
        ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
        std::string& strOutStatus);
    /* Reads <Asset>.projectiles.json beside the event document. A missing file
       is an empty list; a present but invalid one fails so the caller keeps its
       previous document. */
    static bool_t Load_Projectiles(
        const std::string& strAnimationAssetId,
        const std::vector<std::string>& AvailableClips,
        std::vector<ANIMATION_PROJECTILE_CUE>& OutProjectiles,
        std::string& strOutStatus,
        bool_t bFilterToAvailableClips = false);

    static bool_t Load(
        const std::string& strAnimationAssetId,
        const std::vector<std::string>& AvailableClips,
        ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
        std::string& strOutStatus,
        bool_t bFilterToAvailableClips = false);
	static bool_t Load_FromText(
		const std::string& strAnimationAssetId,
		const std::string_view Text,
		const std::vector<std::string>& AvailableClips,
		ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
		std::string& strOutStatus,
		bool_t bFilterToAvailableClips = false);

private:
	static bool_t Load_FromText(
		const std::string& strAnimationAssetId,
		const std::string_view Text,
		const std::vector<std::string>& AvailableClips,
		ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
		std::string& strOutStatus,
		bool_t bFilterToAvailableClips,
		std::vector<std::string>* pOutReferencedClips);
};

NS_END

