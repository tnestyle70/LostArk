#include "ValtanPatternTree.h"

#include "AnimationSkillBindingDocument.h"
#include "DataJson.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternEffectCueDocument.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>

namespace
{
	using Client::DATA_JSON_TYPE;
	using Client::DATA_JSON_VALUE;

	bool Read_TextDocument(
		const std::filesystem::path& Path,
		std::string& OutText,
		std::string& strOutError)
	{
		if (Path.empty())
		{
			strOutError = "path escaped the Data root";
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		OutText.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return true;
	}

	bool Parse_Document(
		const std::filesystem::path& Relative,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(
				Client::CProjectDataRoot::Resolve(Relative), Text, strOutError))
		{
			return false;
		}
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Relative.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	std::string Read_String(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::STRING != pValue->Get_Type() ?
			std::string{} : pValue->Get_String();
	}

	double Read_Number(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::NUMBER != pValue->Get_Type() ?
			0.0 : pValue->Get_Number();
	}

	bool_t Read_RequiredUInt32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		uint32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::floor(pValue->Get_Number()) != pValue->Get_Number() ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_RequiredFiniteFloat(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		f32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::abs(pValue->Get_Number()) > static_cast<double>(
				(std::numeric_limits<f32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_OptionalOrderedHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& OutOffsets)
	{
		OutOffsets.clear();
		const DATA_JSON_VALUE* pOffsets = Object.Find("hitOffsetsMs");
		if (nullptr == pOffsets)
			return true;
		if (!pOffsets->Is_Array() || pOffsets->Get_Array().empty() ||
			pOffsets->Get_Array().size() > 1000u)
		{
			return false;
		}

		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Value.Is_Number() || !std::isfinite(Value.Get_Number()) ||
				std::floor(Value.Get_Number()) != Value.Get_Number() ||
				Value.Get_Number() < 0.0 ||
				Value.Get_Number() > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				return false;
			}
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			OutOffsets.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	struct COMBAT_OBJECT_EFFECT_REFERENCE final
	{
		std::string strClientVisualId;
		std::string strEffectAssetId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
	};

	Client::VALTAN_CLIP_OCCURRENCE_VIEW Build_ClipOccurrenceView(
		const Client::BOSS_PATTERN_ANIMATION_CLIP& Clip)
	{
		Client::VALTAN_CLIP_OCCURRENCE_VIEW View;
		View.strClipOccurrenceId = Clip.strClipOccurrenceId;
		View.strClipName = Clip.strClipName;
		View.strMappingBasis = Clip.strMappingBasis;
		View.iSourceStartMs = Clip.iSourceStartMs;
		View.iPlayMs = Clip.iPlayMs;
		View.fPlayRate = Clip.fPlayRate;
		View.bLoop = Clip.bLoop;
		return View;
	}

	std::string Describe_FollowPolicy(
		const Client::EFFECT_FOLLOW_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_FOLLOW_POLICY::FOLLOW: return "follow";
		case Client::EFFECT_FOLLOW_POLICY::SNAPSHOT: return "snapshot";
		default: return {};
		}
	}

	std::string Describe_StopPolicy(
		const Client::EFFECT_STOP_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_STOP_POLICY::NATURAL: return "natural";
		case Client::EFFECT_STOP_POLICY::CUE_END: return "cue_end";
		default: return {};
		}
	}

	std::string Describe_RepeatPolicy(
		const Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE: return "once";
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP:
			return "each_loop";
		default: return {};
		}
	}
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_StageCount() const
{
	size_t iCount = 0u;
	for (const VALTAN_PATTERN_VIEW& Pattern : Gimmicks)
		iCount += Pattern.Stages.size();
	for (const VALTAN_PATTERN_VIEW& Pattern : Rotation)
		iCount += Pattern.Stages.size();
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_Effect();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectDocumentCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				iCount += Stage.Effects.size();
				iCount += Stage.CombatObjectEffects.size();
			}
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipBoundStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ClipBinding();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ProductCue();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipOccurrenceCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ClipOccurrences.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ProductCues.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_CombatObjectEffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.CombatObjectEffects.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

std::string Client::CValtanPatternTree::Build_StageEffectAssetId(
	const std::string& strPatternActionId,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (strPatternActionId.empty() || Stage.strActionId.empty())
		return {};
	/* valtan.attack.whirlwind -> whirlwind; the category is what the tree
	   already shows, so it is not repeated inside the asset id. */
	std::string strPattern = strPatternActionId;
	size_t iCut = strPattern.find('.');
	if (std::string::npos != iCut)
	{
		iCut = strPattern.find('.', iCut + 1u);
		if (std::string::npos != iCut)
			strPattern = strPattern.substr(iCut + 1u);
	}
	const std::string strPrefix = strPatternActionId + ".";
	std::string strStage;
	if (Stage.strActionId.starts_with(strPrefix) &&
		Stage.strActionId.size() > strPrefix.size())
	{
		strStage = Stage.strActionId.substr(strPrefix.size());
	}
	else
	{
		strStage = Stage.strStageId;
		std::transform(strStage.begin(), strStage.end(), strStage.begin(),
			[](const unsigned char Character)
			{
				return '_' == Character ? '-' :
					static_cast<char>(std::tolower(Character));
			});
	}
	if (strPattern.empty() || strStage.empty())
		return {};
	return "effect.valtan." + strPattern + "." + strStage;
}

bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	DATA_JSON_VALUE Encounter;
	std::string Error;
	const std::filesystem::path EncounterRelative =
		std::filesystem::path(L"Encounters") / L"Valtan" /
		L"ValtanEncounter.json";
	const std::filesystem::path BindingRelative =
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternbindings.json";
	if (!Parse_Document(EncounterRelative, Encounter, Error))
	{
		strOutStatus = "Valtan encounter load failed: " + Error;
		return false;
	}

	/* The tree consumes the same typed, fail-closed documents as Product
	   runtime.  It does not maintain a second permissive JSON interpretation. */
	std::string BindingText;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT BindingDocument;
	if (!Read_TextDocument(CProjectDataRoot::Resolve(BindingRelative),
			BindingText, Error) ||
		!CValtanPatternAnimationBindingDocument::Parse_Text(
			BindingText, BindingDocument, Error))
	{
		strOutStatus = "Valtan pattern bindings load failed: " + Error;
		return false;
	}
	std::vector<std::string> DeclaredClipNames;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			DeclaredClipNames.push_back(Clip.strClipName);
	}
	if (!CValtanPatternAnimationBindingDocument::Validate(
			BindingDocument, "BOSS_VALTAN", DeclaredClipNames, Error))
	{
		strOutStatus = "Valtan pattern bindings validation failed: " + Error;
		return false;
	}

	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
	if (!CValtanPatternEffectCueDocument::Load_Source(CueDocument, Error))
	{
		strOutStatus = "Valtan Product Effect cue load failed: " + Error;
		return false;
	}

	/* Moving pattern visuals are owned by Server combat objects rather than
	   boss-root cues.  Join the encounter spawn action, combat-object owner,
	   and BossCatalog visual explicitly so the authoring tree cannot hide the
	   document under an unrelated "unmapped" bucket or double-own it. */
	DATA_JSON_VALUE BossCatalog;
	DATA_JSON_VALUE CombatObjects;
	if (!Parse_Document(std::filesystem::path(L"Actors") /
			L"BossCatalog.json", BossCatalog, Error) ||
		!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanCombatObjects.json", CombatObjects, Error))
	{
		strOutStatus = "Valtan combat-object authoring join failed: " + Error;
		return false;
	}
	std::map<std::string, COMBAT_OBJECT_EFFECT_REFERENCE, std::less<>>
		CombatObjectEffectsByArchetype;
	const DATA_JSON_VALUE* pBosses = BossCatalog.Find("bosses");
	const DATA_JSON_VALUE* pObjects = CombatObjects.Find("objects");
	if (nullptr == pBosses || !pBosses->Is_Array() ||
		nullptr == pObjects || !pObjects->Is_Array())
	{
		strOutStatus =
			"Valtan combat-object authoring documents have no rows.";
		return false;
	}
	const DATA_JSON_VALUE* pValtanBoss = nullptr;
	for (const DATA_JSON_VALUE& Boss : pBosses->Get_Array())
	{
		if (Boss.Is_Object() &&
			Read_String(Boss, "archetypeId") == "BOSS_VALTAN")
		{
			if (nullptr != pValtanBoss)
			{
				strOutStatus = "BossCatalog contains duplicate BOSS_VALTAN rows.";
				return false;
			}
			pValtanBoss = &Boss;
		}
	}
	const DATA_JSON_VALUE* pVisuals = nullptr == pValtanBoss ? nullptr :
		pValtanBoss->Find("combatObjectVisuals");
	if (nullptr == pVisuals || !pVisuals->Is_Array() ||
		pVisuals->Get_Array().empty())
	{
		strOutStatus = "BOSS_VALTAN has no combat-object Effect visuals.";
		return false;
	}
	for (const DATA_JSON_VALUE& Visual : pVisuals->Get_Array())
	{
		if (!Visual.Is_Object())
		{
			strOutStatus = "BOSS_VALTAN combat-object visual row is invalid.";
			return false;
		}
		const std::string strArchetypeId = Read_String(
			Visual, "combatObjectArchetypeId");
		COMBAT_OBJECT_EFFECT_REFERENCE Reference;
		Reference.strClientVisualId = Read_String(Visual, "clientVisualId");
		Reference.strEffectAssetId = Read_String(Visual, "effectAssetId");
		if (strArchetypeId.empty() || Reference.strClientVisualId.empty() ||
			Reference.strEffectAssetId.empty() ||
			!CombatObjectEffectsByArchetype.emplace(
				strArchetypeId, std::move(Reference)).second)
		{
			strOutStatus =
				"BOSS_VALTAN combat-object visual identity is invalid or duplicated.";
			return false;
		}
	}
	std::set<std::string, std::less<>> DescribedCombatObjects;
	for (const DATA_JSON_VALUE& Object : pObjects->Get_Array())
	{
		if (!Object.Is_Object())
			continue;
		const std::string strArchetypeId = Read_String(
			Object, "combatObjectArchetypeId");
		auto Reference = CombatObjectEffectsByArchetype.find(strArchetypeId);
		if (Reference == CombatObjectEffectsByArchetype.end())
			continue;
		const std::string strClientVisualId = Read_String(
			Object, "clientVisualId");
		Reference->second.strOwnerPatternId = Read_String(
			Object, "ownerPatternId");
		Reference->second.strOwnerStageActionId = Read_String(
			Object, "ownerStageActionId");
		if (strClientVisualId != Reference->second.strClientVisualId ||
			Reference->second.strOwnerPatternId.empty() ||
			Reference->second.strOwnerStageActionId.empty() ||
			!DescribedCombatObjects.insert(strArchetypeId).second)
		{
			strOutStatus =
				"Valtan combat-object owner or visual identity changed: " +
				strArchetypeId;
			return false;
		}
	}
	if (DescribedCombatObjects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"BossCatalog combat-object Effect visual has no Valtan owner row.";
		return false;
	}

	/* Effect bindings are optional: a freshly seeded stage document is not in
	   patterneffects.json yet, and that must not fail the whole tree. */
	std::map<std::string, std::pair<std::string, std::string>, std::less<>>
		EffectByAction;
	DATA_JSON_VALUE Effects;
	if (Parse_Document(std::filesystem::path(L"Animation") / L"Authored" /
			L"Valtan" / L"Valtan.patterneffects.json", Effects, Error))
	{
		const DATA_JSON_VALUE* pRows = Effects.Find("bindings");
		if (nullptr != pRows && pRows->Is_Array())
		{
			for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
			{
				if (!Row.Is_Object())
					continue;
				const std::string strAction = Read_String(Row, "actionId");
				if (strAction.empty())
					continue;
				EffectByAction[strAction] = {
					Read_String(Row, "effectAssetId"),
					Read_String(Row, "effectDocument") };
			}
		}
	}

	std::map<std::string, std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>, std::less<>>
		ClipsByAction;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		Occurrences.reserve(Binding.Clips.size());
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			Occurrences.push_back(Build_ClipOccurrenceView(Clip));
		ClipsByAction.emplace(Binding.strActionId, std::move(Occurrences));
	}

	std::map<std::string, std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW>,
		std::less<>>
		CueByAction;
	const size_t iCueCount = CueDocument.Cues.size();
	for (const VALTAN_PATTERN_EFFECT_CUE& SourceCue : CueDocument.Cues)
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
		Cue.strBindingId = SourceCue.strBindingId;
		Cue.strOccurrenceId = SourceCue.strOccurrenceId;
		Cue.strPatternId = SourceCue.strPatternId;
		Cue.strStageId = SourceCue.strStageId;
		Cue.strActionId = SourceCue.strActionId;
		Cue.strClipOccurrenceId = SourceCue.strClipOccurrenceId;
		Cue.strEffectAssetId = SourceCue.strEffectAssetId;
		Cue.strV1EffectAssetId = SourceCue.strV1EffectAssetId;
		Cue.strAnchorSlotId = SourceCue.strAnchorSlotId;
		Cue.LocalTransform = SourceCue.LocalTransform;
		Cue.eFollowPolicy = SourceCue.eFollowPolicy;
		Cue.eStopPolicy = SourceCue.eStopPolicy;
		Cue.strFollowPolicy = Describe_FollowPolicy(SourceCue.eFollowPolicy);
		Cue.strStopPolicy = Describe_StopPolicy(SourceCue.eStopPolicy);
		Cue.strRepeatPolicy = Describe_RepeatPolicy(SourceCue.eRepeatPolicy);
		Cue.iSourceStartMs = SourceCue.iStartMs;
		Cue.iSourceEndMs = SourceCue.iEndMs;
		Cue.iStageDurationMs = SourceCue.iStageDurationMs;
		Cue.bHasSourceEnd = SourceCue.bHasSourceEnd;
		CueByAction[Cue.strActionId].push_back(std::move(Cue));
	}

	const DATA_JSON_VALUE* pPatterns = Encounter.Find("patterns");
	if (nullptr == pPatterns || !pPatterns->Is_Array())
	{
		strOutStatus = "Valtan encounter has no patterns array.";
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW Staged;
	size_t iResolvedCueCount = 0u;
	std::set<std::string, std::less<>> ResolvedCombatObjectEffects;
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!PatternValue.Is_Object())
			continue;
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = Read_String(PatternValue, "patternId");
		Pattern.strDisplayName = Read_String(PatternValue, "displayName");
		Pattern.strActionId = Read_String(PatternValue, "actionId");
		Pattern.iMinimumHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "minimumHealthBar"));
		Pattern.iMaximumHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "maximumHealthBar"));
		Pattern.iTriggerHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "triggerHealthBar"));
		if (Pattern.strPatternId.empty())
			continue;

		const DATA_JSON_VALUE* pStages = PatternValue.Find("stages");
		if (nullptr != pStages && pStages->Is_Array())
		{
			for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
			{
				if (!StageValue.Is_Object())
					continue;
				VALTAN_STAGE_VIEW Stage;
				Stage.strStageId = Read_String(StageValue, "stageId");
				Stage.strActionId = Read_String(StageValue, "actionId");
				Stage.strStageKind = Read_String(StageValue, "stageKind");
				Stage.strHitShape = Read_String(StageValue, "hitShape");
				const bool_t bValidStageKind =
					"WINDUP" == Stage.strStageKind ||
					"ACTIVE" == Stage.strStageKind ||
					"RECOVERY" == Stage.strStageKind ||
					"GROGGY" == Stage.strStageKind ||
					"PART_BREAK" == Stage.strStageKind;
				const bool_t bValidHitShape =
					"NONE" == Stage.strHitShape ||
					"CIRCLE" == Stage.strHitShape ||
					"RING" == Stage.strHitShape ||
					"CONE" == Stage.strHitShape ||
					"BOX" == Stage.strHitShape ||
					"CROSS" == Stage.strHitShape ||
					"SIX_DIRECTIONS" == Stage.strHitShape;
				if (Stage.strStageId.empty() || Stage.strActionId.empty() ||
					!bValidStageKind || !bValidHitShape)
				{
					strOutStatus =
						"Valtan encounter stage identity is missing or invalid";
					return false;
				}
				if (!Read_RequiredUInt32(
						StageValue, "durationMs", Stage.iDurationMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitCount", Stage.iHitCount) ||
					!Read_RequiredUInt32(
						StageValue, "hitIntervalMs", Stage.iHitIntervalMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitDelayMs", Stage.iHitDelayMs) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitOuterRadius", Stage.fHitOuterRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitInnerRadius", Stage.fHitInnerRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitAngleDegrees", Stage.fHitAngleDegrees) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitLength", Stage.fHitLength) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitHalfWidth", Stage.fHitHalfWidth))
				{
					strOutStatus =
						"Valtan encounter stage numeric field is missing or invalid: " +
						Stage.strActionId;
					return false;
				}
				if (!Read_OptionalOrderedHitOffsets(
						StageValue, Stage.HitOffsetsMs))
				{
					strOutStatus =
						"Valtan encounter stage hitOffsetsMs is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.strServerDamageProfileId = Read_String(
					StageValue, "serverDamageProfileId");

				const bool_t bHasExplicitOffsets = !Stage.HitOffsetsMs.empty();
				const bool_t bValidExplicitSchedule = bHasExplicitOffsets &&
					Stage.HitOffsetsMs.size() == Stage.iHitCount &&
					0u == Stage.iHitIntervalMs && 0u == Stage.iHitDelayMs &&
					Stage.HitOffsetsMs.back() < Stage.iDurationMs;
				const bool_t bValidLegacySchedule = !bHasExplicitOffsets &&
					Stage.iHitCount > 0u &&
					(1u == Stage.iHitCount ? 0u == Stage.iHitIntervalMs :
						Stage.iHitIntervalMs > 0u) &&
					static_cast<uint64_t>(Stage.iHitDelayMs) +
						static_cast<uint64_t>(Stage.iHitCount - 1u) *
							Stage.iHitIntervalMs < Stage.iDurationMs;
				const bool_t bValidEmptySchedule = 0u == Stage.iHitCount &&
					!bHasExplicitOffsets && 0u == Stage.iHitIntervalMs &&
					0u == Stage.iHitDelayMs;
				if (!bValidExplicitSchedule && !bValidLegacySchedule &&
					!bValidEmptySchedule)
				{
					strOutStatus =
						"Valtan encounter stage hit schedule is invalid: " +
						Stage.strActionId;
					return false;
				}

				const DATA_JSON_VALUE* pActions = StageValue.Find("actions");
				if (nullptr != pActions && pActions->Is_Array())
				{
					for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
					{
						if (!Action.Is_Object() ||
							Read_String(Action, "kind") !=
								"SPAWN_COMBAT_OBJECT")
						{
							continue;
						}
						const std::string strTargetId = Read_String(
							Action, "targetId");
						const auto Reference =
							CombatObjectEffectsByArchetype.find(strTargetId);
						const double SpawnValue = Read_Number(Action, "value");
						if (Reference == CombatObjectEffectsByArchetype.end() ||
							Reference->second.strOwnerPatternId !=
								Pattern.strPatternId ||
							Reference->second.strOwnerStageActionId !=
								Stage.strActionId ||
							!std::isfinite(SpawnValue) || SpawnValue < 1.0 ||
							SpawnValue >
								static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
							std::floor(SpawnValue) != SpawnValue ||
							!ResolvedCombatObjectEffects.insert(strTargetId).second)
						{
							strOutStatus =
								"Valtan SPAWN_COMBAT_OBJECT authoring join changed: " +
								strTargetId;
							return false;
						}
						VALTAN_COMBAT_OBJECT_EFFECT_VIEW View;
						View.strCombatObjectArchetypeId = strTargetId;
						View.strClientVisualId =
							Reference->second.strClientVisualId;
						View.strEffectAssetId =
							Reference->second.strEffectAssetId;
						View.strTrigger = Read_String(Action, "trigger");
						View.iSpawnValue = static_cast<uint32_t>(SpawnValue);
						Stage.CombatObjectEffects.push_back(std::move(View));
					}
				}

				const auto ClipIterator = ClipsByAction.find(Stage.strActionId);
				if (ClipIterator != ClipsByAction.end())
				{
					Stage.ClipOccurrences = ClipIterator->second;
					Stage.RuntimeClipNames.reserve(
						Stage.ClipOccurrences.size());
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Stage.RuntimeClipNames.push_back(
							Occurrence.strClipName);
					}
					Stage.strRuntimeClipName =
						Stage.RuntimeClipNames.front();
				}

				const auto CueIterator = CueByAction.find(Stage.strActionId);
				if (CueIterator != CueByAction.end())
				{
					for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
						CueIterator->second)
					{
						if (Cue.strPatternId != Pattern.strPatternId ||
							Cue.strStageId != Stage.strStageId)
						{
							strOutStatus =
								"Valtan Product Effect cue pattern/stage tuple changed for " +
								Stage.strActionId + ".";
							return false;
						}
						const auto Clip = std::find_if(
							Stage.ClipOccurrences.begin(),
							Stage.ClipOccurrences.end(),
							[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
							{
								return Occurrence.strClipOccurrenceId ==
									Cue.strClipOccurrenceId;
							});
						if (Clip == Stage.ClipOccurrences.end())
						{
							strOutStatus =
								"Valtan Product Effect cue clip occurrence left its stage: " +
								Cue.strClipOccurrenceId;
							return false;
						}
						Clip->ProductCues.push_back(Cue);
						Stage.ProductCues.push_back(Cue);
						++iResolvedCueCount;

					}
					if (!Stage.ProductCues.empty())
						Stage.ProductCue = Stage.ProductCues.front();
				}

				const auto EffectIterator =
					EffectByAction.find(Stage.strActionId);
				if (EffectIterator != EffectByAction.end() &&
					!EffectIterator->second.first.empty() &&
					!EffectIterator->second.second.empty())
				{
					const auto Existing = std::find_if(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&EffectIterator](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId ==
								EffectIterator->second.first;
						});
					if (Existing != Stage.Effects.end())
					{
						Existing->bPatternEffectBinding = true;
					}
					else
					{
						VALTAN_STAGE_EFFECT_VIEW Bound;
						Bound.strEffectAssetId = EffectIterator->second.first;
						Bound.DocumentPath = CProjectDataRoot::Get() /
							std::filesystem::path(
								EffectIterator->second.second).lexically_normal();
						Bound.eOrigin =
							VALTAN_STAGE_EFFECT_ORIGIN::PATTERN_EFFECT_BINDING;
						Bound.bPatternEffectBinding = true;
						Stage.Effects.push_back(std::move(Bound));
					}
				}

				/* Product cues own the authoring entry. The naming rule is only
				   a fallback for a stage that has no Product cue, so the old
				   whirlwind seed cannot appear beside the admitted 420633 row. */
				if (!Stage.Has_ProductCue() &&
					Stage.CombatObjectEffects.empty())
				{
					const std::string strCandidate = Build_StageEffectAssetId(
						Pattern.strActionId, Stage);
					const bool_t bCandidateAlreadyBound = std::any_of(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&strCandidate](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId == strCandidate;
						});
					if (!strCandidate.empty() && !bCandidateAlreadyBound)
					{
						const std::filesystem::path Candidate =
							CProjectDataRoot::Resolve(
								std::filesystem::path(L"Effects") / L"Authored" /
								std::filesystem::path(
									strCandidate + ".effect.json"));
						std::error_code FileError;
						if (!Candidate.empty() && std::filesystem::exists(
								Candidate, FileError))
						{
							VALTAN_STAGE_EFFECT_VIEW Seeded;
							Seeded.strEffectAssetId = strCandidate;
							Seeded.DocumentPath = Candidate;
							Seeded.eOrigin =
								VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE;
							Stage.Effects.push_back(std::move(Seeded));
						}
					}
				}
				Pattern.Stages.push_back(std::move(Stage));
			}
		}
		if (Pattern.Is_Gimmick())
			Staged.Gimmicks.push_back(std::move(Pattern));
		else
			Staged.Rotation.push_back(std::move(Pattern));
	}

	if (iResolvedCueCount != iCueCount)
	{
		strOutStatus = "Valtan Product Effect cue action did not resolve into the encounter: resolved " +
			std::to_string(iResolvedCueCount) + " of " +
			std::to_string(iCueCount) + ".";
		return false;
	}
	if (ResolvedCombatObjectEffects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"Valtan combat-object Effect visual did not resolve into its owner stage.";
		return false;
	}
	if (Staged.Gimmicks.empty() && Staged.Rotation.empty())
	{
		strOutStatus = "Valtan encounter produced no patterns.";
		return false;
	}
	/* Gimmicks read as a timeline down the health bar, so order them the way
	   the fight actually presents them. */
	std::sort(Staged.Gimmicks.begin(), Staged.Gimmicks.end(),
		[](const VALTAN_PATTERN_VIEW& Left, const VALTAN_PATTERN_VIEW& Right)
		{
			return Left.iTriggerHealthBar > Right.iTriggerHealthBar;
		});

	/* A phase is the band of health bars that ends when its gimmick fires.
	   The encounter document has no phase field, so this band is derived from
	   triggerHealthBar and used only as a display grouping. */
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	/* A rotation pattern belongs to every band its own bar range overlaps.
	   Most are 1-160 and therefore appear in all of them; two are narrower
	   and that is exactly the information the band grouping exposes. */
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	const std::string strIntroPatternId = Read_String(
		Encounter, "introPatternId");
	if (!strIntroPatternId.empty())
	{
		const auto Intro = std::find_if(
			Staged.Rotation.begin(), Staged.Rotation.end(),
			[&strIntroPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strIntroPatternId;
			});
		if (Intro != Staged.Rotation.end())
		{
			Staged.iIntroRotationIndex = static_cast<size_t>(
				std::distance(Staged.Rotation.begin(), Intro));
		}
	}

	OutView = std::move(Staged);
	strOutStatus = "Valtan tree: " +
		std::to_string(OutView.Phases.size()) + " phases, " +
		std::to_string(OutView.Get_PatternCount()) + " patterns, " +
		std::to_string(OutView.Get_StageCount()) + " stages, " +
		std::to_string(OutView.Get_ClipBoundStageCount()) + " clip-bound (" +
		std::to_string(OutView.Get_ClipOccurrenceCount()) + " occurrences), " +
		std::to_string(OutView.Get_ProductCueStageCount()) + " cue stages (" +
		std::to_string(OutView.Get_ProductCueCount()) + " Product cues), " +
		std::to_string(OutView.Get_CombatObjectEffectCount()) +
		" combat-object visuals, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	return true;
}
