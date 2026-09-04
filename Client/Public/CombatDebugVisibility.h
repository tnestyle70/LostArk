#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <limits>

NS_BEGIN(Client)

/* One process-wide Debug presentation contract.  Every Client replication
   instance consumes a revisioned snapshot, so changing Levels cannot silently
   restore an older mixture of boss/player geometry switches. */
struct COMBAT_DEBUG_VISIBILITY_SNAPSHOT final
{
	bool_t bBossBodyCollider = false;
	bool_t bBossPatternHitPulse = true;
	bool_t bBossStageGeometry = true;
	bool_t bCombatObjectHit = true;
	bool_t bCounterProxy = true;
	bool_t bPlayerSkillHitGeometry = true;
	std::uint64_t iRevision = 0u;

	bool_t Has_SameVisibility(
		const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Other) const
	{
		return bBossBodyCollider == Other.bBossBodyCollider &&
			bBossPatternHitPulse == Other.bBossPatternHitPulse &&
			bBossStageGeometry == Other.bBossStageGeometry &&
			bCombatObjectHit == Other.bCombatObjectHit &&
			bCounterProxy == Other.bCounterProxy &&
			bPlayerSkillHitGeometry == Other.bPlayerSkillHitGeometry;
	}

	static std::uint64_t Next_Revision(const std::uint64_t iCurrent)
	{
		return (std::numeric_limits<std::uint64_t>::max)() == iCurrent ?
			1u : iCurrent + 1u;
	}
};

/* Pure display clock for Server-owned combat-object geometry. CONTACT shapes
   remain live with the replicated object. TIMED rows mirror the authored
   Server pulse clock and stay visible for one short inspection window after
   each pulse; neither path performs Client-side hit judgment. */
class COMBAT_OBJECT_HIT_DEBUG_CLOCK final
{
public:
	static bool_t Is_Visible(
		const bool_t bContact,
		const std::uint32_t iCurrentServerTick,
		const std::uint32_t iSpawnTick,
		const std::uint32_t iAtMs,
		const std::uint32_t iRepeatCount,
		const std::uint32_t iRepeatIntervalMs,
		const std::uint32_t iFixedTickHz = 30u,
		const std::uint32_t iPulseWindowMs = 300u)
	{
		if (0u == iCurrentServerTick || 0u == iSpawnTick ||
			0u == iFixedTickHz || 0u == iRepeatCount ||
			static_cast<std::int32_t>(
				iCurrentServerTick - iSpawnTick) < 0)
		{
			return false;
		}
		if (bContact)
			return true;

		const std::uint64_t iAgeTicks =
			iCurrentServerTick - iSpawnTick;
		const std::uint64_t iPulseWindowTicks =
			Milliseconds_ToTicks(iPulseWindowMs, iFixedTickHz);
		for (std::uint32_t iPulse = 0u;
			iPulse < iRepeatCount; ++iPulse)
		{
			const std::uint64_t iDueMs =
				static_cast<std::uint64_t>(iAtMs) +
				static_cast<std::uint64_t>(iPulse) * iRepeatIntervalMs;
			const std::uint64_t iDueTicks =
				Milliseconds_ToTicks(iDueMs, iFixedTickHz);
			if (iAgeTicks >= iDueTicks &&
				iAgeTicks <= iDueTicks + iPulseWindowTicks)
			{
				return true;
			}
		}
		return false;
	}

private:
	static std::uint64_t Milliseconds_ToTicks(
		const std::uint64_t iMilliseconds,
		const std::uint32_t iFixedTickHz)
	{
		return (iMilliseconds * iFixedTickHz + 999u) / 1000u;
	}
};

NS_END
