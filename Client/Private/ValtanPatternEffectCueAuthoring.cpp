#include "ValtanPatternEffectCueAuthoring.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <optional>

namespace
{
	using namespace Client;

	VALTAN_PATTERN_VIEW* FindPattern(
		VALTAN_PATTERN_TREE_VIEW& Tree,
		const std::string& strPatternId)
	{
		for (auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			const auto Iterator = std::find_if(
				pGroup->begin(), pGroup->end(),
				[&strPatternId](const VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (pGroup->end() != Iterator)
				return &*Iterator;
		}
		return nullptr;
	}

	VALTAN_STAGE_VIEW* FindStage(
		VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strStageId)
	{
		const auto Iterator = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&strStageId](const VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == strStageId;
			});
		return Pattern.Stages.end() == Iterator ? nullptr : &*Iterator;
	}

	bool_t IsStableAuthoringId(const std::string& Value)
	{
		return !Value.empty() && Value.size() <= 160u &&
			std::all_of(
				Value.begin(), Value.end(),
				[](const unsigned char Character)
				{
					return 0 != std::isalnum(Character) || '_' == Character ||
						'.' == Character || '-' == Character;
				});
	}

	bool_t EqualCue(
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Left,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Right)
	{
		return Left.strBindingId == Right.strBindingId &&
			Left.strOccurrenceId == Right.strOccurrenceId &&
			Left.strPatternId == Right.strPatternId &&
			Left.strStageId == Right.strStageId &&
			Left.strActionId == Right.strActionId &&
			Left.strClipOccurrenceId == Right.strClipOccurrenceId &&
			Left.strEffectAssetId == Right.strEffectAssetId &&
			Left.strV1EffectAssetId == Right.strV1EffectAssetId &&
			Left.strAnchorSlotId == Right.strAnchorSlotId &&
			Left.eFollowPolicy == Right.eFollowPolicy &&
			Left.eStopPolicy == Right.eStopPolicy &&
			Left.strFollowPolicy == Right.strFollowPolicy &&
			Left.strStopPolicy == Right.strStopPolicy &&
			Left.strRepeatPolicy == Right.strRepeatPolicy &&
			Left.eScalePolicy == Right.eScalePolicy &&
			Left.strScalePolicy == Right.strScalePolicy &&
			Left.vWorldScale.x == Right.vWorldScale.x &&
			Left.vWorldScale.y == Right.vWorldScale.y &&
			Left.vWorldScale.z == Right.vWorldScale.z &&
			Left.bHasExplicitScalePolicy == Right.bHasExplicitScalePolicy &&
			Left.bUsesStageClock == Right.bUsesStageClock &&
			Left.iStageOffsetMs == Right.iStageOffsetMs &&
			Left.iSourceStartMs == Right.iSourceStartMs &&
			Left.iSourceEndMs == Right.iSourceEndMs &&
			Left.iStageDurationMs == Right.iStageDurationMs &&
			Left.bHasSourceEnd == Right.bHasSourceEnd &&
			Left.LocalTransform.vPosition.x ==
				Right.LocalTransform.vPosition.x &&
			Left.LocalTransform.vPosition.y ==
				Right.LocalTransform.vPosition.y &&
			Left.LocalTransform.vPosition.z ==
				Right.LocalTransform.vPosition.z &&
			Left.LocalTransform.vRotationDegrees.x ==
				Right.LocalTransform.vRotationDegrees.x &&
			Left.LocalTransform.vRotationDegrees.y ==
				Right.LocalTransform.vRotationDegrees.y &&
			Left.LocalTransform.vRotationDegrees.z ==
				Right.LocalTransform.vRotationDegrees.z &&
			Left.LocalTransform.vScale.x == Right.LocalTransform.vScale.x &&
			Left.LocalTransform.vScale.y == Right.LocalTransform.vScale.y &&
			Left.LocalTransform.vScale.z == Right.LocalTransform.vScale.z;
	}

	bool_t RequireAdmission(
		const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
		const char* const pOperation,
		std::string& strOutStatus)
	{
		if (VALTAN_EFFECT_CUE_AUTHORING_ADMISSION::ADMITTED !=
				Context.eAdmission)
		{
			strOutStatus = std::string(pOperation) +
				" blocked: the split gameplay/presentation source is not ADMITTED. "
				"A stale-preserved graph is display-only.";
			return false;
		}
		if (!Context.QuerySourceMembership)
		{
			strOutStatus = std::string(pOperation) +
				" blocked: the admitted Effect source catalog query is absent.";
			return false;
		}
		return true;
	}

	bool_t ValidateCue(
		const VALTAN_PATTERN_TREE_VIEW& Tree,
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
		const bool_t bRequireCompositionNamespace,
		const std::string& strIgnoredCueId,
		const std::string& strIgnoredOccurrenceId,
		const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
		std::string& strOutStatus)
	{
		if (!Pattern.bAuthoringMasterManaged ||
			"WAIT" == Stage.strSequenceRole || Stage.bSuppressAnimation ||
			Stage.ClipOccurrences.empty())
		{
			strOutStatus =
				"Valtan Effect invocation requires one admitted non-WAIT animation Stage.";
			return false;
		}
		if (!IsStableAuthoringId(Cue.strBindingId) ||
			!IsStableAuthoringId(Cue.strOccurrenceId) ||
			!IsStableAuthoringId(Cue.strEffectAssetId) ||
			!IsStableAuthoringId(Cue.strClipOccurrenceId) ||
			Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId ||
			Cue.strActionId != Stage.strActionId || Cue.bUsesStageClock ||
			0u != Cue.iStageOffsetMs)
		{
			strOutStatus =
				"Valtan Effect invocation stable Pattern/Stage/Action/clip identity is invalid.";
			return false;
		}
		if (bRequireCompositionNamespace &&
			(0u != Cue.strBindingId.rfind("cue.valtan.composition.", 0u) ||
			 0u != Cue.strOccurrenceId.rfind(
				 Cue.strBindingId + ".occurrence.", 0u)))
		{
			strOutStatus =
				"A new Workbench Effect invocation requires cue.valtan.composition.* and its matching occurrence namespace.";
			return false;
		}
		if (!bRequireCompositionNamespace &&
			((!strIgnoredCueId.empty() &&
			  Cue.strBindingId != strIgnoredCueId) ||
			 (!strIgnoredOccurrenceId.empty() &&
			  Cue.strOccurrenceId != strIgnoredOccurrenceId)))
		{
			strOutStatus =
				"Valtan Effect invocation update cannot replace cueId or occurrenceId.";
			return false;
		}

		const auto Clip = std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
			{
				return Candidate.strClipOccurrenceId == Cue.strClipOccurrenceId;
			});
		if (Stage.ClipOccurrences.end() == Clip ||
			Cue.iSourceStartMs < Clip->iSourceStartMs ||
			Cue.iSourceStartMs > 600000u || Cue.iSourceEndMs > 600000u ||
			(Cue.bHasSourceEnd &&
			 Cue.iSourceEndMs <= Cue.iSourceStartMs) ||
			(!Cue.bHasSourceEnd && 0u != Cue.iSourceEndMs) ||
			(0u != Clip->iPlayMs &&
			 (Cue.iSourceStartMs >= Clip->iSourceStartMs + Clip->iPlayMs ||
			  (Cue.bHasSourceEnd &&
			   Cue.iSourceEndMs > Clip->iSourceStartMs + Clip->iPlayMs))))
		{
			strOutStatus =
				"Valtan Effect invocation source window escapes its exact animation occurrence.";
			return false;
		}

		const bool_t bFollowPolicyValid =
			("follow" == Cue.strFollowPolicy &&
			 Cue.eFollowPolicy == EFFECT_FOLLOW_POLICY::FOLLOW) ||
			("snapshot" == Cue.strFollowPolicy &&
			 Cue.eFollowPolicy == EFFECT_FOLLOW_POLICY::SNAPSHOT);
		const bool_t bStopPolicyValid =
			("natural" == Cue.strStopPolicy &&
			 Cue.eStopPolicy == EFFECT_STOP_POLICY::NATURAL &&
			 !Cue.bHasSourceEnd) ||
			("cue_end" == Cue.strStopPolicy &&
			 Cue.eStopPolicy == EFFECT_STOP_POLICY::CUE_END &&
			 Cue.bHasSourceEnd);
		const bool_t bRepeatPolicyValid = "once" == Cue.strRepeatPolicy ||
			("each_loop" == Cue.strRepeatPolicy && Clip->bLoop);
		if (!bFollowPolicyValid || !bStopPolicyValid || !bRepeatPolicyValid)
		{
			strOutStatus =
				"Valtan Effect invocation follow/stop/repeat policy is invalid for its selected animation occurrence.";
			return false;
		}

		const bool_t bTargetSnapshotAnchor =
			"pattern.target.snapshot" == Cue.strAnchorSlotId &&
			"snapshot" == Cue.strFollowPolicy &&
			("LOCK_NEAREST_ON_START" == Pattern.strTargetPolicy ||
			 "LOCK_RANDOM_ALIVE_ON_START" == Pattern.strTargetPolicy ||
			 "LOCK_RANDOM_ALIVE_BEHIND_ON_START" == Pattern.strTargetPolicy);
		const bool_t bArenaTargetFollow =
			"arena.center.target-follow" == Cue.strAnchorSlotId;
		const bool_t bArenaCenterAnchor =
			("arena.center" == Cue.strAnchorSlotId ||
			 "arena.center.facing" == Cue.strAnchorSlotId ||
			 bArenaTargetFollow) &&
			(bArenaTargetFollow ? "follow" : "snapshot") ==
				Cue.strFollowPolicy && Pattern.ServerMotion.has_value() &&
			"LEAP_TO_ANCHOR" == Pattern.ServerMotion->strKind &&
			Pattern.ServerMotion->bMoveToAnchorBeforeTakeoff &&
			("arena.center.facing" != Cue.strAnchorSlotId ||
			 ("LOCK_FACING_ON_START" == Pattern.strAimPolicy &&
			  "LOCK_RANDOM_ALIVE_ON_START" == Pattern.strTargetPolicy)) &&
			(!bArenaTargetFollow ||
			 ("TRACK_TARGET_EACH_TICK" == Pattern.strAimPolicy &&
			  "LOCK_RANDOM_ALIVE_ON_START" == Pattern.strTargetPolicy));
		if ("root" != Cue.strAnchorSlotId && !bTargetSnapshotAnchor &&
			!bArenaCenterAnchor)
		{
			strOutStatus =
				"Valtan Effect invocation anchor must be root, an admitted pattern.target.snapshot, or an admitted arena.center fixed/follow root.";
			return false;
		}

		const auto FiniteMagnitude = [](const float3_t& Value,
			const float fMaximum, const bool_t bPositive)
		{
			return std::isfinite(Value.x) && std::isfinite(Value.y) &&
				std::isfinite(Value.z) && std::abs(Value.x) <= fMaximum &&
				std::abs(Value.y) <= fMaximum &&
				std::abs(Value.z) <= fMaximum &&
				(!bPositive ||
				 (Value.x > 0.f && Value.y > 0.f && Value.z > 0.f));
		};
		if (!FiniteMagnitude(Cue.LocalTransform.vPosition, 100000.f, false) ||
			!FiniteMagnitude(
				Cue.LocalTransform.vRotationDegrees, 360000.f, false) ||
			!FiniteMagnitude(Cue.LocalTransform.vScale, 1000.f, true))
		{
			strOutStatus =
				"Valtan Effect invocation position, rotation, or positive scale is outside its authored bounds.";
			return false;
		}

		const bool_t bOwnerScale =
			"OWNER_RELATIVE" == Cue.strScalePolicy &&
			Cue.eScalePolicy ==
				VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
		const bool_t bFootprintScale =
			("GAMEPLAY_FOOTPRINT" == Cue.strScalePolicy &&
			 Cue.eScalePolicy ==
				 VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT) ||
			("ARENA_ABSOLUTE" == Cue.strScalePolicy &&
			 Cue.eScalePolicy ==
				 VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE);
		if (!Cue.bHasExplicitScalePolicy ||
			(!bOwnerScale && !bFootprintScale) ||
			(bOwnerScale &&
			 (1.f != Cue.vWorldScale.x || 1.f != Cue.vWorldScale.y ||
			  1.f != Cue.vWorldScale.z)) ||
			(bFootprintScale &&
			 (1.5f != Cue.vWorldScale.x || 1.5f != Cue.vWorldScale.y ||
			  1.5f != Cue.vWorldScale.z)))
		{
			strOutStatus =
				"Valtan Effect invocation scalePolicy is invalid; world policies preserve the exact 1.5 footprint.";
			return false;
		}

		bool_t bSourceContains = false;
		std::string strCatalogStatus;
		if (0u != Cue.strEffectAssetId.rfind("effect.valtan.", 0u) ||
			!Context.QuerySourceMembership(
				Cue.strEffectAssetId, bSourceContains, strCatalogStatus) ||
			!bSourceContains)
		{
			strOutStatus =
				"Valtan Effect invocation requires one fresh direct-authored effect.valtan.* catalog row: " +
				strCatalogStatus;
			return false;
		}

		for (const auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			for (const VALTAN_PATTERN_VIEW& OtherPattern : *pGroup)
			{
				for (const VALTAN_STAGE_VIEW& OtherStage : OtherPattern.Stages)
				{
					for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Existing :
						OtherStage.ProductCues)
					{
						if (Existing.strBindingId == strIgnoredCueId &&
							Existing.strOccurrenceId == strIgnoredOccurrenceId)
						{
							continue;
						}
						if (Existing.strBindingId == Cue.strBindingId ||
							Existing.strOccurrenceId == Cue.strOccurrenceId)
						{
							strOutStatus =
								"Valtan Effect invocation cueId or occurrenceId already exists in the joined graph.";
							return false;
						}
					}
				}
			}
		}
		return true;
	}
}

bool_t Client::CValtanPatternEffectCueAuthoring::Validate_Mirrors(
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus)
{
	std::size_t iNestedCount = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
		Stage.ClipOccurrences)
	{
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
			Occurrence.ProductCues)
		{
			++iNestedCount;
			const auto Flat = std::find_if(
				Stage.ProductCues.begin(), Stage.ProductCues.end(),
				[&Cue](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
				{
					return Candidate.strBindingId == Cue.strBindingId &&
						Candidate.strOccurrenceId == Cue.strOccurrenceId;
				});
			if (Stage.ProductCues.end() == Flat || !EqualCue(*Flat, Cue) ||
				Cue.strClipOccurrenceId != Occurrence.strClipOccurrenceId)
			{
				strOutStatus =
					"Valtan Effect invocation flat/clip occurrence mirror is inconsistent.";
				return false;
			}
		}
	}
	if (iNestedCount != Stage.ProductCues.size())
	{
		strOutStatus =
			"Valtan Effect invocation mirror count is inconsistent.";
		return false;
	}
	return true;
}

bool_t Client::CValtanPatternEffectCueAuthoring::Add(
	VALTAN_PATTERN_TREE_VIEW& InOutTree,
	const std::string& strPatternId,
	const std::string& strStageId,
	const std::string& strActionId,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
	bool_t& bOutChanged,
	std::string& strOutStatus)
{
	bOutChanged = false;
	if (!RequireAdmission(Context, "Valtan Effect invocation add", strOutStatus))
		return false;

	VALTAN_PATTERN_TREE_VIEW Staged = InOutTree;
	VALTAN_PATTERN_VIEW* const pPattern = FindPattern(Staged, strPatternId);
	VALTAN_STAGE_VIEW* const pStage = nullptr == pPattern ? nullptr :
		FindStage(*pPattern, strStageId);
	if (nullptr == pPattern || nullptr == pStage ||
		pStage->strActionId != strActionId ||
		pStage->ProductCues.size() >= 128u)
	{
		strOutStatus =
			"Valtan Effect invocation add rejected: Pattern/Stage/Action is stale or the Stage cue bound was reached.";
		return false;
	}
	if (!ValidateCue(
			Staged, *pPattern, *pStage, Cue, true, {}, {}, Context,
			strOutStatus))
	{
		return false;
	}

	const auto Clip = std::find_if(
		pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
		[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
		{
			return Candidate.strClipOccurrenceId == Cue.strClipOccurrenceId;
		});
	if (pStage->ClipOccurrences.end() == Clip)
	{
		strOutStatus =
			"Valtan Effect invocation add lost its exact clip occurrence while staging.";
		return false;
	}
	pStage->ProductCues.push_back(Cue);
	Clip->ProductCues.push_back(Cue);
	pStage->ProductCue = pStage->ProductCues.front();
	if (!Validate_Mirrors(*pStage, strOutStatus))
		return false;

	InOutTree = std::move(Staged);
	bOutChanged = true;
	strOutStatus = "Staged ADD_EFFECT_CUE for " + Cue.strOccurrenceId +
		". Save + Validate + Publish commits Valtan.presentation.json.";
	return true;
}

bool_t Client::CValtanPatternEffectCueAuthoring::Update(
	VALTAN_PATTERN_TREE_VIEW& InOutTree,
	const std::string& strPatternId,
	const std::string& strStageId,
	const std::string& strActionId,
	const std::string& strCueId,
	const std::string& strOccurrenceId,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
	bool_t& bOutChanged,
	std::string& strOutStatus)
{
	bOutChanged = false;
	if (!RequireAdmission(
			Context, "Valtan Effect invocation update", strOutStatus))
	{
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW Staged = InOutTree;
	VALTAN_PATTERN_VIEW* const pPattern = FindPattern(Staged, strPatternId);
	VALTAN_STAGE_VIEW* const pStage = nullptr == pPattern ? nullptr :
		FindStage(*pPattern, strStageId);
	if (nullptr == pPattern || nullptr == pStage ||
		pStage->strActionId != strActionId)
	{
		strOutStatus =
			"Valtan Effect invocation update rejected: Pattern/Stage/Action is stale.";
		return false;
	}
	const auto Current = std::find_if(
		pStage->ProductCues.begin(), pStage->ProductCues.end(),
		[&](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
		{
			return Candidate.strBindingId == strCueId &&
				Candidate.strOccurrenceId == strOccurrenceId;
		});
	if (pStage->ProductCues.end() == Current)
	{
		strOutStatus =
			"Valtan Effect invocation update rejected: exact cue predecessor is stale.";
		return false;
	}
	if (0u != Current->strBindingId.rfind("cue.valtan.composition.", 0u) &&
		(Current->strScalePolicy != Cue.strScalePolicy ||
		 Current->eScalePolicy != Cue.eScalePolicy))
	{
		strOutStatus =
			"Valtan Effect invocation update cannot change the canonical scalePolicy kind of an admitted legacy cue.";
		return false;
	}
	if (EqualCue(*Current, Cue))
	{
		strOutStatus = "Valtan Effect invocation is unchanged.";
		return true;
	}
	if (!ValidateCue(
			Staged, *pPattern, *pStage, Cue, false, strCueId,
			strOccurrenceId, Context, strOutStatus))
	{
		return false;
	}

	std::size_t iNestedMatches = 0u;
	for (VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
		pStage->ClipOccurrences)
	{
		for (auto Iterator = Occurrence.ProductCues.begin();
			Iterator != Occurrence.ProductCues.end();)
		{
			if (Iterator->strBindingId == strCueId &&
				Iterator->strOccurrenceId == strOccurrenceId)
			{
				Iterator = Occurrence.ProductCues.erase(Iterator);
				++iNestedMatches;
			}
			else
			{
				++Iterator;
			}
		}
	}
	if (1u != iNestedMatches)
	{
		strOutStatus =
			"Valtan Effect invocation update rejected: exact nested predecessor mirror is missing or duplicated.";
		return false;
	}
	const auto TargetClip = std::find_if(
		pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
		[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
		{
			return Candidate.strClipOccurrenceId == Cue.strClipOccurrenceId;
		});
	if (pStage->ClipOccurrences.end() == TargetClip)
	{
		strOutStatus =
			"Valtan Effect invocation update lost its selected clip while staging.";
		return false;
	}
	*Current = Cue;
	TargetClip->ProductCues.push_back(Cue);
	pStage->ProductCue = pStage->ProductCues.front();
	if (!Validate_Mirrors(*pStage, strOutStatus))
		return false;

	InOutTree = std::move(Staged);
	bOutChanged = true;
	strOutStatus = "Staged UPDATE_EFFECT_CUE for " + strOccurrenceId +
		". Save + Validate + Publish commits Valtan.presentation.json.";
	return true;
}

bool_t Client::CValtanPatternEffectCueAuthoring::Remove(
	VALTAN_PATTERN_TREE_VIEW& InOutTree,
	const std::string& strPatternId,
	const std::string& strStageId,
	const std::string& strActionId,
	const std::string& strCueId,
	const std::string& strOccurrenceId,
	const std::string& strEffectAssetId,
	const std::string& strClipOccurrenceId,
	const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
	bool_t& bOutChanged,
	std::string& strOutStatus)
{
	bOutChanged = false;
	if (!RequireAdmission(
			Context, "Valtan Effect invocation remove", strOutStatus))
	{
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW Staged = InOutTree;
	VALTAN_PATTERN_VIEW* const pPattern = FindPattern(Staged, strPatternId);
	VALTAN_STAGE_VIEW* const pStage = nullptr == pPattern ? nullptr :
		FindStage(*pPattern, strStageId);
	if (nullptr == pPattern || nullptr == pStage ||
		pStage->strActionId != strActionId ||
		"WAIT" == pStage->strSequenceRole)
	{
		strOutStatus =
			"Valtan Effect invocation remove rejected: Pattern/Stage/Action is stale or the Stage is WAIT.";
		return false;
	}
	const auto Current = std::find_if(
		pStage->ProductCues.begin(), pStage->ProductCues.end(),
		[&](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
		{
			return Candidate.strBindingId == strCueId &&
				Candidate.strOccurrenceId == strOccurrenceId &&
				Candidate.strEffectAssetId == strEffectAssetId &&
				Candidate.strClipOccurrenceId == strClipOccurrenceId;
		});
	if (pStage->ProductCues.end() == Current)
	{
		strOutStatus =
			"Valtan Effect invocation remove rejected: exact cue predecessor CAS did not match.";
		return false;
	}
	pStage->ProductCues.erase(Current);

	std::size_t iNestedMatches = 0u;
	for (VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
		pStage->ClipOccurrences)
	{
		const std::size_t iBefore = Occurrence.ProductCues.size();
		std::erase_if(
			Occurrence.ProductCues,
			[&](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
			{
				return Candidate.strBindingId == strCueId &&
					Candidate.strOccurrenceId == strOccurrenceId &&
					Candidate.strEffectAssetId == strEffectAssetId &&
					Candidate.strClipOccurrenceId == strClipOccurrenceId;
			});
		iNestedMatches += iBefore - Occurrence.ProductCues.size();
	}
	if (1u != iNestedMatches)
	{
		strOutStatus =
			"Valtan Effect invocation remove rejected: exact nested predecessor mirror is missing or duplicated.";
		return false;
	}
	pStage->ProductCue = pStage->ProductCues.empty() ?
		std::nullopt :
		std::optional<VALTAN_PRODUCT_EFFECT_CUE_VIEW>{
			pStage->ProductCues.front() };
	if (!Validate_Mirrors(*pStage, strOutStatus))
		return false;

	InOutTree = std::move(Staged);
	bOutChanged = true;
	strOutStatus = "Staged REMOVE_EFFECT_CUE for " + strOccurrenceId +
		" with exact predecessor CAS.";
	return true;
}
