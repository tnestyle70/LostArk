#pragma once

#include "AnimationSkillBindingDocument.h"
#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "HitAreaWire.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

enum class EFFECT_FOLLOW_POLICY : uint8_t
{
    FOLLOW,
    SNAPSHOT,
    END
};

/* Position sampling and rotational authority are independent.  Existing cues
   omit this field and retain their sampled anchor basis. */
enum class EFFECT_ORIENTATION_POLICY : uint8_t
{
	ANCHOR,
	ACTION_FACING,
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
	EFFECT_ORIENTATION_POLICY eOrientationPolicy =
		EFFECT_ORIENTATION_POLICY::ANCHOR;
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

/* A wav playback trigger, parsed from a "SOUND" .animevents row. strEventName
   is the payload's event name (bank prefix stripped, e.g. "PC_LanceMaster_F_
   Att_Battle1_1") -- the actual wav path(s) are resolved later at Character
   load time via CSoundCueCatalog, not stored here, so this struct stays a
   pure parse result independent of which variant file gets picked at
   playback. */
struct ANIMATION_SOUND_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    std::string strEventName;
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
	uint32_t iFormatVersion = 6u;
    std::string strAnimationAssetId;
    std::vector<ANIMATION_EFFECT_CUE> Cues;
	/* Structurally valid Product cues whose exact Effect target is absent from
	   the currently committed runtime catalog. They are isolated per target;
	   valid sibling cues remain available to the character and Effect Tool. */
	std::vector<std::string> UnavailableEffectAssetIds;
    std::vector<ANIMATION_HIT_CUE> Hits;
    std::vector<ANIMATION_PROJECTILE_CUE> Projectiles;
    std::vector<ANIMATION_SOUND_CUE> Sounds;
};

/* A Tool preview candidate is an exact join between one Product animevents
   cue and one ordered skillbinding clip. It never represents an inferred full
   skill chain. */
struct ANIMATION_EFFECT_PREVIEW_CANDIDATE final
{
    size_t iBoundClipOrdinal = 0u;
    size_t iStageIndex = 0u;
    size_t iStageClipIndex = 0u;
    ANIMATION_SKILL_CLIP Clip;
    ANIMATION_EFFECT_CUE Cue;
};

class CAnimationEffectCueDocument final
{
public:
	/* A Product occurrence belongs to exactly one half-open playable source
	   window.  playMs==0 means the remainder of the model clip. */
	static bool_t Is_CueStartInClipWindow(
		const ANIMATION_SKILL_CLIP& Clip,
		uint32_t iCueStartMs);

	/* Local TRS is applied to the sampled root once. ACTION_FACING preserves
	   sampled root translation/scale while replacing only its rotational basis
	   with the action-start yaw captured by the caller. */
	static bool_t Try_ComposeRootTransform(
		const EFFECT_TRANSFORM_DESC& Local,
		const float4x4_t& SampledRootAnchor,
		EFFECT_ORIENTATION_POLICY eOrientationPolicy,
		f32_t fActionFacingYawDegrees,
		float4x4_t& OutRoot);

    /* Joins Product cues to the binding in stage/clip order. A document with no
       exact Product mapping fails without modifying OutCandidates, allowing
       callers to keep the previous preview instead of expanding a skill chain. */
    static bool_t Resolve_PreviewCandidates(
        const ANIMATION_SKILL_BINDING& Binding,
        const std::vector<ANIMATION_EFFECT_CUE>& ProductCues,
        std::string_view strAuthoredEffectAssetId,
        std::vector<ANIMATION_EFFECT_PREVIEW_CANDIDATE>& OutCandidates,
        std::string& strOutStatus);

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

