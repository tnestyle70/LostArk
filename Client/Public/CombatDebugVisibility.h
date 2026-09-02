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
	bool_t bCounterProxy = true;
	bool_t bPlayerSkillHitGeometry = true;
	std::uint64_t iRevision = 0u;

	bool_t Has_SameVisibility(
		const COMBAT_DEBUG_VISIBILITY_SNAPSHOT& Other) const
	{
		return bBossBodyCollider == Other.bBossBodyCollider &&
			bBossPatternHitPulse == Other.bBossPatternHitPulse &&
			bBossStageGeometry == Other.bBossStageGeometry &&
			bCounterProxy == Other.bCounterProxy &&
			bPlayerSkillHitGeometry == Other.bPlayerSkillHitGeometry;
	}

	static std::uint64_t Next_Revision(const std::uint64_t iCurrent)
	{
		return (std::numeric_limits<std::uint64_t>::max)() == iCurrent ?
			1u : iCurrent + 1u;
	}
};

NS_END
