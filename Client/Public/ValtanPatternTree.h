#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstddef>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Where a stage's Effect document came from. A stage can carry both: the
   evidence-backed binding for the 420633 canary and the seeded document that
   the naming rule finds on disk. Neither hides the other. */
enum class VALTAN_STAGE_EFFECT_ORIGIN : uint8_t
{
	PATTERN_EFFECT_BINDING,
	NAMING_RULE,
	END
};

struct VALTAN_STAGE_EFFECT_VIEW final
{
	std::string strEffectAssetId;
	std::filesystem::path DocumentPath;
	VALTAN_STAGE_EFFECT_ORIGIN eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::END;
	/* Filled only after the user opens or plays the document. The tree must
	   never decode JSON just to draw a row. */
	uint32_t iElementCount = 0u;
	bool_t bParsed = false;
};

/* The stage is where a Valtan pattern actually becomes work: it owns the
   clip that plays, the milliseconds it lasts, the shape the Server tests for
   damage, and the Effect that should be visible while it runs. Those four
   facts live in four different documents joined by the stage actionId. */
struct VALTAN_STAGE_VIEW final
{
	std::string strStageId;
	std::string strActionId;
	std::string strStageKind;
	uint32_t iDurationMs = 0u;

	/* Server owns damage. These are copied for display so the person
	   authoring the Effect can see the window they are filling. */
	std::string strHitShape;
	f32_t fHitOuterRadius = 0.f;
	f32_t fHitInnerRadius = 0.f;
	f32_t fHitAngleDegrees = 0.f;
	f32_t fHitLength = 0.f;
	f32_t fHitHalfWidth = 0.f;
	uint32_t iHitCount = 0u;
	uint32_t iHitIntervalMs = 0u;
	std::string strServerDamageProfileId;

	std::string strRuntimeClipName;
	std::vector<VALTAN_STAGE_EFFECT_VIEW> Effects;

	bool_t Has_Effect() const { return !Effects.empty(); }
	bool_t Has_HitShape() const
	{
		return !strHitShape.empty() && "NONE" != strHitShape;
	}
};

struct VALTAN_PATTERN_VIEW final
{
	std::string strPatternId;
	std::string strDisplayName;
	std::string strActionId;
	int32_t iMinimumHealthBar = 0;
	int32_t iMaximumHealthBar = 0;
	int32_t iTriggerHealthBar = 0;
	std::vector<VALTAN_STAGE_VIEW> Stages;

	/* A pattern pinned to one health bar is a scripted gimmick; the rest are
	   selected from the rotation while the bar range allows it. */
	bool_t Is_Gimmick() const { return 0 != iTriggerHealthBar; }
};

/* Valtan has no phase field. What it has is 160 health bars and gimmicks
   pinned to one of them, so a phase is the band of bars that ends when its
   gimmick fires. The band is derived, never stored as an id. */
struct VALTAN_PHASE_VIEW final
{
	static constexpr size_t INVALID_INDEX =
		(std::numeric_limits<size_t>::max)();

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTopHealthBar = 0;
	int32_t iBandBottomHealthBar = 0;
	/* Empty on the final band: nothing gates the bars below the last gimmick. */
	std::string strGatePatternId;
	int32_t iGateTriggerHealthBar = 0;

	/* Indices into VALTAN_PATTERN_TREE_VIEW, never copies. The same rotation
	   pattern shows under several phases and must stay one object. */
	std::vector<size_t> GimmickIndices;
	std::vector<size_t> RotationIndices;
};

struct VALTAN_PATTERN_TREE_VIEW final
{
	std::vector<VALTAN_PATTERN_VIEW> Gimmicks;
	std::vector<VALTAN_PATTERN_VIEW> Rotation;
	std::vector<VALTAN_PHASE_VIEW> Phases;
	/* introPatternId resolved into Rotation, or INVALID_INDEX. */
	size_t iIntroRotationIndex = VALTAN_PHASE_VIEW::INVALID_INDEX;

	size_t Get_PatternCount() const
	{
		return Gimmicks.size() + Rotation.size();
	}
	size_t Get_StageCount() const;
	size_t Get_EffectCount() const;
	size_t Get_EffectDocumentCount() const;
};

/* Read-only join of ValtanEncounter.json, Valtan.patternbindings.json and
   Valtan.patterneffects.json. The Effect Tool renders the result; nothing
   here writes, because the encounter document is Server authority. */
class CValtanPatternTree final
{
public:
	static bool_t Load(
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus);
	/* effect.valtan.<pattern-slug>.<stage-slug>, the same rule
	   Tools/EffectPipeline/build_valtan_stage_effects.py emits. */
	static std::string Build_StageEffectAssetId(
		const std::string& strPatternActionId,
		const VALTAN_STAGE_VIEW& Stage);
};

NS_END
