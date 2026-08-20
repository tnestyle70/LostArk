#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* One authored stage of a boss pattern. iStartOffsetMs is derived at load time
   from the preceding stage durations and is never stored in the document. */
struct ENCOUNTER_STAGE_REFERENCE final
{
	std::string stageId;
	std::string actionId;
	std::string stageKind;
	uint32_t iDurationMs = 0;
	uint32_t iStartOffsetMs = 0;
	std::string hitShape;
	f32_t fHitOuterRadius = 0.f;
	f32_t fHitInnerRadius = 0.f;
	f32_t fHitAngleDegrees = 0.f;
	f32_t fHitLength = 0.f;
	f32_t fHitHalfWidth = 0.f;
	uint32_t iHitCount = 0;
	uint32_t iHitIntervalMs = 0;
	std::string serverDamageProfileId;
};

struct ENCOUNTER_PATTERN_REFERENCE final
{
	std::string patternId;
	std::string displayName;
	std::string actionId;
	std::string selectionMode;
	uint32_t iTriggerHealthBar = 0;
	uint32_t iTotalDurationMs = 0;
	std::vector<ENCOUNTER_STAGE_REFERENCE> stages;
};

/* Read-only view of Data/Encounters/<Boss>/<Boss>Encounter.json.

   The editable draft of that document is owned by CBalanceTool, which parses,
   edits and saves it. This class exists so authoring tools that only need to
   resolve stable patternId/stageId/actionId can do so without holding an
   editable draft, and it deliberately has no Save path. If the encounter
   schema changes, the exact key lists in the matching cpp must change in the
   same commit; unknown properties are rejected instead of ignored. */
class CEncounterPatternReference final
{
public:
	static constexpr uint32_t MAX_PATTERN_COUNT = 256u;
	static constexpr uint32_t MAX_STAGE_COUNT = 64u;
	static constexpr uint32_t MAX_STAGE_DURATION_MS = 60000u;
	static constexpr uint32_t MAX_FIXED_TICK_HZ = 240u;

public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	void Clear();

public:
	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_EncounterId() const { return m_EncounterId; }
	const std::string& Get_BossArchetypeId() const
	{
		return m_BossArchetypeId;
	}
	uint32_t Get_FixedTickHz() const { return m_iFixedTickHz; }
	const std::vector<ENCOUNTER_PATTERN_REFERENCE>& Get_Patterns() const
	{
		return m_Patterns;
	}
	const ENCOUNTER_PATTERN_REFERENCE* Find_Pattern(
		const std::string& patternId) const;

	static uint32_t To_ServerTick(
		uint32_t milliseconds,
		uint32_t fixedTickHz);

private:
	std::string m_EncounterId;
	std::string m_BossArchetypeId;
	uint32_t m_iFixedTickHz = 0;
	std::vector<ENCOUNTER_PATTERN_REFERENCE> m_Patterns;
	bool_t m_isReady = false;
};

NS_END
