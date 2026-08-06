#pragma once

#include "AnimationEffectCueDocument.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

NS_BEGIN(Client)

enum class EFFECT_CUE_PIVOT_KIND : uint8_t
{
	PLAYER_ROOT,
	WEAPON_SOCKET,
	MODEL_BONE,
	END
};

struct EFFECT_AUTHORING_CUE_TRANSFER final
{
	uint64_t iTargetGeneration = 0u;
	std::string strAnimationAssetId;
	std::string strClipName;
	uint32_t iTimeMs = 0u;
	uint32_t iDurationMs = 0u;
	std::string strEffectAssetId;
	EFFECT_CUE_PIVOT_KIND ePivotKind = EFFECT_CUE_PIVOT_KIND::END;
	std::string strAnchorSlotId = "root";
	EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
};

class CEffectAuthoringTransfer final
{
public:
	static void Publish(EFFECT_AUTHORING_CUE_TRANSFER&& Transfer)
	{
		s_Pending = std::move(Transfer);
	}

	static bool_t Consume(EFFECT_AUTHORING_CUE_TRANSFER& OutTransfer)
	{
		if (!s_Pending.has_value())
			return false;
		OutTransfer = std::move(*s_Pending);
		s_Pending.reset();
		return true;
	}

private:
	inline static std::optional<EFFECT_AUTHORING_CUE_TRANSFER> s_Pending;
};

NS_END
