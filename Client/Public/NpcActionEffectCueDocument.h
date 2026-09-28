#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* One original PlayParticleEffect notify of an NPC clip. The restored V1
   document owns the visual; this row owns only which document, when, and where
   it is placed. */
struct NPC_ACTION_EFFECT_CUE final
{
	std::string strCueId;
	std::string strEffectAssetId;
	std::string strClip;
	std::uint32_t iStartMs = 0u;
	std::uint32_t iDurationMs = 0u;
	std::string strBone;
	bool_t bFollowBone = false;
	/* Optional source sound notify of the same occurrence, version 2 only.
	   An empty event leaves the row visual only. The original LookInfo picks
	   one of two events by the actor's DLChar, so the alternate event is
	   selected when the owner model tag ends with the recorded suffix. */
	std::string strSoundClass;
	std::string strSoundEvent;
	std::string strSoundEventAlternate;
	std::string strSoundAlternateModelTagSuffix;
};

/* What one fired cue still owns after it was spawned. A positive stop age
   belongs to the source notify duration: the engine bounds the visual itself
   through fSourceLoopEndSeconds, while the sound cue is cut here. */
struct NPC_ACTION_EFFECT_LIVE_CUE final
{
	std::uint64_t iSoundHandle = 0u;
	f32_t fSoundStopAtSeconds = 0.f;
};

/* Per NPC occurrence cursor. The clip edge is the identity: the same clip
   restarted spawns a fresh occurrence, and a running one only advances. */
struct NPC_ACTION_EFFECT_PLAYBACK_STATE final
{
	std::string strClip;
	f32_t fElapsedSeconds = 0.f;
	std::size_t iNextCue = 0u;
	bool_t bActive = false;
	/* Cues that are still running. They outlive bActive, because the last
	   notify of a clip can start before its own stop age is reached. */
	std::vector<NPC_ACTION_EFFECT_LIVE_CUE> LiveCues;

	/* Clears the cursor only. The caller releases LiveCues first, because
	   this header must not depend on the presentation service. */
	void Reset();
};

/* Data/Effects/NpcActionCues/<archetypeId>.npcactioncues.json is projected
   from the .loa notify timeline. It is read once per archetype and cached;
   an archetype without a document simply has no cues, which leaves the
   existing V2 binding path in charge for that NPC. */
class CNpcActionEffectCueDocument final
{
public:
	static bool_t Load(const std::string& strArchetypeId, std::string& strOutStatus);
	static const std::vector<NPC_ACTION_EFFECT_CUE>& Get_Cues(
		const std::string& strArchetypeId);
	static bool_t Has_Clip(const std::string& strArchetypeId,
		const std::string& strClip);
};

NS_END
