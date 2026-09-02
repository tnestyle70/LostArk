#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "PlayerHandGripTransform.h"

#include <cstdint>
#include <array>
#include <filesystem>
#include <optional>
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
	uint32_t iHitDelayMs = 0;
	/* Optional stage-relative contacts for source clips whose hit cadence is
	   not uniform. Empty preserves the legacy delay + interval schedule. */
	std::vector<uint32_t> hitOffsetsMs;
	bool_t bHasHitAnchor = false;
	std::string hitAnchorKind;
	f32_t fHitAnchorForwardOffsetM = 0.f;
	f32_t fHitAnchorRightOffsetM = 0.f;
	f32_t fHitAnchorYawOffsetDegrees = 0.f;
	bool_t bHasHitActivation = false;
	uint32_t iHitActivationStartMs = 0u;
	uint32_t iHitActivationLifetimeMs = 0u;
	std::string serverDamageProfileId;
	std::optional<PLAYER_HAND_GRIP_LOCAL_OFFSET> gripLocalOffset;
	/* Optional Server-authored stage refinements are retained by this read-only
	   reference so tools can diagnose the same contract that the Server loaded. */
	std::string partDamagePolicy;
	bool_t bHasCounterProxy = false;
	/* Retained only for Product fallback diagnostics.  Command-capable Logic
	   Flow still consumes the strictly joined authoring graph. */
	bool_t bHasCounterHitBranch = false;
	std::string counterProxyKind;
	f32_t fCounterProxyForwardOffsetM = 0.f;
	f32_t fCounterProxyRightOffsetM = 0.f;
	f32_t fCounterProxyRadiusM = 0.f;
	f32_t fCounterProxyArcDegrees = 0.f;
};

/* Projected Product motion retained for presentation consumers that need the
   admitted landing anchor. Runtime presentation must not re-open the split
   Valtan.gameplay/presentation authoring graph to recover this value. */
struct ENCOUNTER_PATTERN_SERVER_MOTION_REFERENCE final
{
	std::string kind;
	std::string anchorId;
	std::array<f32_t, 3u> landingPosition{};
	bool_t bMoveToAnchorBeforeTakeoff = false;
};

struct ENCOUNTER_PATTERN_REFERENCE final
{
	std::string patternId;
	std::string displayName;
	std::string actionId;
	std::string selectionMode;
	/* Retained so presentation documents can validate virtual target anchors
	   against the exact Server targeting contract instead of accepting a
	   target-looking stable ID as an arbitrary model bone. */
	std::string targetPolicy;
	std::string aimPolicy;
	uint32_t iTriggerHealthBar = 0;
	uint32_t iTotalDurationMs = 0;
	std::vector<uint32_t> sourceActionIds;
	std::vector<ENCOUNTER_STAGE_REFERENCE> stages;
	std::optional<ENCOUNTER_PATTERN_SERVER_MOTION_REFERENCE> serverMotion;
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
