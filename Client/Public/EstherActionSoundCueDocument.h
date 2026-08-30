#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

enum class ESTHER_ACTION_SOUND_OWNER_KIND : std::uint8_t
{
	PLAYER_ACTION,
	NPC_ACTION,
	END
};

struct ESTHER_ACTION_SOUND_CUE final
{
	std::string strCueId;
	ESTHER_ACTION_SOUND_OWNER_KIND eOwnerKind =
		ESTHER_ACTION_SOUND_OWNER_KIND::END;
	std::string strOwnerId;
	std::string strActionId;
	std::string strCatalogOwnerId;
	std::string strSoundEvent;
	std::uint32_t iStartMs = 0u;
	std::uint32_t iLateToleranceMs = 0u;
	f32_t fVolume = 1.f;
	bool_t bOnce = true;
	std::string strTimingBasis;
};

/* Per replicated entity occurrence cursor. The Server actionStartTick is the
   identity: repeated snapshots advance due cues without replaying them, while
   a later authoritative action edge starts a fresh occurrence. */
struct ESTHER_ACTION_SOUND_PLAYBACK_STATE final
{
	ESTHER_ACTION_SOUND_OWNER_KIND eOwnerKind =
		ESTHER_ACTION_SOUND_OWNER_KIND::END;
	std::string strOwnerId;
	std::string strActionId;
	std::uint32_t iActionStartTick = 0u;
	std::unordered_set<std::string> AttemptedCueIds;

	void Reset();
};

/* Data/Sound/EstherActionSoundCues.json is intentionally separate from the NPC
   model/clip catalog. It can bind both a replicated player ESTHER_CAST edge
   and a replicated Esther NPC action without inventing an ActorCatalog sound
   lifecycle. Missing presentation remains non-authoritative and is isolated. */
class CEstherActionSoundCueDocument final
{
public:
	static bool_t Load(std::string& strOutStatus);
	static bool_t Is_Ready() { return s_bLoaded; }
	static const std::vector<ESTHER_ACTION_SOUND_CUE>& Get_Cues()
	{
		return s_Cues;
	}

	/* Scans one Server action occurrence, plays every newly due cue, and drops
	   cues whose explicit late tolerance has elapsed. Playback failure never
	   changes gameplay; false only reports a malformed runtime input. */
	static bool_t Play_Due(
		ESTHER_ACTION_SOUND_OWNER_KIND eOwnerKind,
		const std::string& strOwnerId,
		const std::string& strActionId,
		std::uint32_t iServerTick,
		std::uint32_t iActionStartTick,
		ESTHER_ACTION_SOUND_PLAYBACK_STATE& State,
		std::string& strOutStatus);

private:
	static std::vector<ESTHER_ACTION_SOUND_CUE> s_Cues;
	static std::uint32_t s_iFixedTickHz;
	static bool_t s_bLoaded;
};

NS_END
