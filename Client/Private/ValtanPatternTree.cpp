#include "ValtanPatternTree.h"

#include "AnimationSkillBindingDocument.h"
#include "DataJson.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternEffectCueDocument.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
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

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Names)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Names.size())
			return false;
		for (const std::string_view Name : Names)
		{
			if (nullptr == Object.Find(Name))
				return false;
		}
		return true;
	}

	bool_t Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				'_' != Character && '-' != Character && '.' != Character)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_FiniteNumber(const DATA_JSON_VALUE* pValue)
	{
		return nullptr != pValue && pValue->Is_Number() &&
			std::isfinite(pValue->Get_Number());
	}

	bool_t Is_NonNegativeInteger(const DATA_JSON_VALUE* pValue)
	{
		return Is_FiniteNumber(pValue) && pValue->Get_Number() >= 0.0 &&
			std::floor(pValue->Get_Number()) == pValue->Get_Number() &&
			pValue->Get_Number() <= static_cast<double>(
				(std::numeric_limits<uint32_t>::max)());
	}

	bool_t Read_NullableStableToken(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue)
			return false;
		if (pValue->Is_Null())
		{
			OutValue.clear();
			return true;
		}
		if (!pValue->Is_String() || !Is_StableToken(pValue->Get_String()))
			return false;
		OutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_StageMotion(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		if (!Has_ExactProperties(*pValue, { "kind", "distance" }))
			return false;
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		Client::VALTAN_STAGE_MOTION_VIEW Motion;
		if (nullptr == pKind || pKind->Get_String().empty() ||
			!Read_RequiredFiniteFloat(*pValue, "distance", Motion.fDistance))
		{
			return false;
		}
		Motion.strKind = pKind->Get_String();
		Out = std::move(Motion);
		return true;
	}

	bool_t Read_StageActions(
		const DATA_JSON_VALUE* pValue,
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactProperties(Value,
					{ "trigger", "kind", "targetId", "value", "durationMs" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* pTrigger = Required(
				Value, "trigger", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pKind = Required(
				Value, "kind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTarget = Required(
				Value, "targetId", DATA_JSON_TYPE::STRING);
			Client::VALTAN_STAGE_ACTION_VIEW Action;
			if (nullptr == pTrigger || nullptr == pKind || nullptr == pTarget ||
				!Read_RequiredFiniteFloat(Value, "value", Action.fValue) ||
				!Read_RequiredUInt32(Value, "durationMs", Action.iDurationMs))
			{
				return false;
			}
			Action.strTrigger = pTrigger->Get_String();
			Action.strKind = pKind->Get_String();
			Action.strTargetId = pTarget->Get_String();
			Out.push_back(std::move(Action));
		}
		return true;
	}

	bool_t Read_StageBranches(
		const DATA_JSON_VALUE* pValue,
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;
		std::set<std::string, std::less<>> Outcomes;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactProperties(Value, { "outcome", "nextActionId" }))
				return false;
			const DATA_JSON_VALUE* pOutcome = Required(
				Value, "outcome", DATA_JSON_TYPE::STRING);
			std::string strNextActionId;
			if (nullptr == pOutcome || !Is_StableToken(pOutcome->Get_String()) ||
				!Outcomes.insert(pOutcome->Get_String()).second ||
				!Read_NullableStableToken(
					Value, "nextActionId", strNextActionId))
			{
				return false;
			}
			Client::VALTAN_STAGE_BRANCH_VIEW Branch;
			Branch.strOutcome = pOutcome->Get_String();
			if (!strNextActionId.empty())
				Branch.strNextActionId = std::move(strNextActionId);
			Out.push_back(std::move(Branch));
		}
		return true;
	}

	bool_t Read_RequiredHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& Out)
	{
		Out.clear();
		const DATA_JSON_VALUE* pOffsets = Required(
			Object, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffsets || pOffsets->Get_Array().size() > 1000u)
			return false;
		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Is_NonNegativeInteger(&Value))
				return false;
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			Out.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	bool_t Read_PatternServerMotion(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		if (!Has_ExactProperties(*pValue,
				{ "kind", "anchorId", "landingPosition", "apexHeight",
				  "travelStageId" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnchor = Required(
			*pValue, "anchorId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTravel = Required(
			*pValue, "travelStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pLanding = Required(
			*pValue, "landingPosition", DATA_JSON_TYPE::ARRAY);
		Client::VALTAN_PATTERN_SERVER_MOTION_VIEW Motion;
		if (nullptr == pKind || nullptr == pAnchor || nullptr == pTravel ||
			pKind->Get_String().empty() || pAnchor->Get_String().empty() ||
			pTravel->Get_String().empty() || nullptr == pLanding ||
			3u != pLanding->Get_Array().size() ||
			!Read_RequiredFiniteFloat(*pValue, "apexHeight", Motion.fApexHeight))
		{
			return false;
		}
		for (size_t i = 0u; i < Motion.LandingPosition.size(); ++i)
		{
			const DATA_JSON_VALUE& Coordinate = pLanding->Get_Array()[i];
			if (!Is_FiniteNumber(&Coordinate) ||
				std::abs(Coordinate.Get_Number()) > static_cast<double>(
					(std::numeric_limits<f32_t>::max)()))
			{
				return false;
			}
			Motion.LandingPosition[i] = static_cast<f32_t>(Coordinate.Get_Number());
		}
		Motion.strKind = pKind->Get_String();
		Motion.strAnchorId = pAnchor->Get_String();
		Motion.strTravelStageId = pTravel->Get_String();
		Out = std::move(Motion);
		return true;
	}

	struct MASTER_EFFECT_REFERENCE final
	{
		std::string strType;
		std::string strId;
		std::string strClipOccurrenceId;
		std::string strMappingBasis;
		uint32_t iSourceStartMs = 0u;
		uint32_t iSourceEndMs = 0u;
		bool_t bHasSourceEnd = false;
	};

	struct MASTER_STAGE final
	{
		std::string strStageId;
		std::string strSequenceRole;
		std::string strActionId;
		std::string strStageKind;
		uint32_t iDurationMs = 0u;
		uint32_t iRepeatCount = 0u;
		std::string strAnimationEndPolicy;
		std::string strHitShape;
		f32_t fHitOuterRadius = 0.f;
		f32_t fHitInnerRadius = 0.f;
		f32_t fHitAngleDegrees = 0.f;
		f32_t fHitLength = 0.f;
		f32_t fHitHalfWidth = 0.f;
		uint32_t iHitCount = 0u;
		uint32_t iHitIntervalMs = 0u;
		uint32_t iHitDelayMs = 0u;
		std::vector<uint32_t> HitOffsetsMs;
		std::string strServerDamageProfileId;
		f32_t fPushRangeM = 0.f;
		uint32_t iPushMs = 0u;
		bool_t bKnockdown = false;
		uint32_t iDownMs = 0u;
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW> Motion;
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW> Actions;
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> Branches;
		std::vector<Client::VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		std::vector<MASTER_EFFECT_REFERENCE> EffectReferences;
	};

	struct MASTER_PATTERN final
	{
		std::string strPatternId;
		std::string strCategory;
		uint32_t iMinimumPhase = 0u;
		uint32_t iMaximumPhase = 0u;
		std::string strTargetPolicy;
		std::string strAimPolicy;
		std::string strDisplayName;
		std::string strActionId;
		std::vector<uint32_t> SourceActionIds;
		std::string strSelectionMode;
		int32_t iMinimumHealthBar = 0;
		int32_t iMaximumHealthBar = 0;
		int32_t iTriggerHealthBar = 0;
		uint32_t iTriggerOrder = 0u;
		std::string strArmorRequirement;
		std::string strPhaseRequirement;
		bool_t bInvulnerableWhileRunning = false;
		uint32_t iSelectionWeight = 0u;
		uint32_t iMaximumConsecutiveUses = 0u;
		f32_t fMinimumRange = 0.f;
		f32_t fMaximumRange = 0.f;
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW> ServerMotion;
		uint32_t iSourceSequenceIndex = 0u;
		std::vector<Client::VALTAN_PRESENTATION_SOURCE_VIEW> PresentationSources;
		std::vector<Client::VALTAN_PATTERN_REACTION_VIEW> Reactions;
		std::vector<std::string> CameraCueIds;
		std::vector<Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW>
			WorldEventTriggerRefs;
		std::vector<MASTER_STAGE> Stages;
	};

	struct MASTER_DOCUMENT final
	{
		std::vector<std::string> RetiredPatternIds;
		Client::VALTAN_NORMAL_SELECTION_VIEW NormalSelection;
		std::vector<Client::VALTAN_COUNTER_REACTION_LAYER_VIEW>
			CounterReactionLayers;
		std::vector<Client::VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;
		std::vector<MASTER_PATTERN> Patterns;
	};

	bool_t Parse_MasterOccurrence(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_CLIP_OCCURRENCE_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "clipOccurrenceId", "clip", "mappingBasis",
				  "sourceStartMs", "playMs", "playRate",
				  "repeatUntilStageEnd" }))
		{
			strOutError = "master animation occurrence has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pOccurrenceId = Required(
			Value, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pClip = Required(
			Value, "clip", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pBasis = Required(
			Value, "mappingBasis", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = Value.Find("sourceStartMs");
		const DATA_JSON_VALUE* pPlay = Value.Find("playMs");
		const DATA_JSON_VALUE* pRate = Value.Find("playRate");
		const DATA_JSON_VALUE* pRepeat = Required(
			Value, "repeatUntilStageEnd", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pOccurrenceId || nullptr == pClip || nullptr == pBasis ||
			!Is_StableToken(pOccurrenceId->Get_String()) ||
			!Is_StableToken(pClip->Get_String()) || pBasis->Get_String().empty() ||
			!Is_NonNegativeInteger(pSourceStart) ||
			!Is_NonNegativeInteger(pPlay) || !Is_FiniteNumber(pRate) ||
			pRate->Get_Number() <= 0.05 || pRate->Get_Number() > 16.0 ||
			nullptr == pRepeat)
		{
			strOutError = "master animation occurrence is invalid";
			return false;
		}
		Out.strClipOccurrenceId = pOccurrenceId->Get_String();
		Out.strClipName = pClip->Get_String();
		Out.strMappingBasis = pBasis->Get_String();
		Out.iSourceStartMs = static_cast<uint32_t>(pSourceStart->Get_Number());
		Out.iPlayMs = static_cast<uint32_t>(pPlay->Get_Number());
		Out.fPlayRate = static_cast<f32_t>(pRate->Get_Number());
		Out.bLoop = pRepeat->Get_Boolean();
		return true;
	}

	bool_t Validate_MasterStageGameplayShape(
		const DATA_JSON_VALUE& Value,
		std::string& strOutError)
	{
		const std::initializer_list<std::string_view> NumericFields = {
			"hitOuterRadius", "hitInnerRadius", "hitAngleDegrees", "hitLength",
			"hitHalfWidth", "hitCount", "hitIntervalMs", "hitDelayMs",
			"pushRangeM", "pushMs", "downMs" };
		for (const std::string_view Field : NumericFields)
		{
			if (!Is_FiniteNumber(Value.Find(Field)))
			{
				strOutError = "master stage numeric field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		const DATA_JSON_VALUE* pOffsets = Required(
			Value, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* pMotion = Value.Find("motion");
		const DATA_JSON_VALUE* pActions = Required(
			Value, "actions", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffsets || nullptr == pKnockdown || nullptr == pMotion ||
			nullptr == pActions)
		{
			strOutError = "master stage gameplay fields are invalid";
			return false;
		}
		for (const DATA_JSON_VALUE& Offset : pOffsets->Get_Array())
		{
			if (!Is_NonNegativeInteger(&Offset))
			{
				strOutError = "master stage hitOffsetsMs is invalid";
				return false;
			}
		}
		if (!pMotion->Is_Null())
		{
			if (!Has_ExactProperties(*pMotion, { "kind", "distance" }) ||
				nullptr == Required(*pMotion, "kind", DATA_JSON_TYPE::STRING) ||
				!Is_FiniteNumber(pMotion->Find("distance")))
			{
				strOutError = "master stage motion is invalid";
				return false;
			}
		}
		for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
		{
			if (!Has_ExactProperties(Action,
					{ "trigger", "kind", "targetId", "value", "durationMs" }) ||
				nullptr == Required(Action, "trigger", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "kind", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "targetId", DATA_JSON_TYPE::STRING) ||
				!Is_FiniteNumber(Action.Find("value")) ||
				!Is_NonNegativeInteger(Action.Find("durationMs")))
			{
				strOutError = "master stage action is invalid";
				return false;
			}
		}
		return true;
	}

	bool_t Parse_MasterStage(
		const DATA_JSON_VALUE& Value,
		MASTER_STAGE& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "stageId", "sequenceRole", "actionId", "stageKind",
				  "durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				  "hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				  "hitIntervalMs", "hitDelayMs", "hitOffsetsMs",
				  "serverDamageProfileId", "pushRangeM", "pushMs", "knockdown",
				  "downMs", "motion", "actions", "branches", "animation",
				  "effectRefs" }))
		{
			strOutError = "master stage has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pStageId = Required(
			Value, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRole = Required(
			Value, "sequenceRole", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pKind = Required(
			Value, "stageKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDuration = Value.Find("durationMs");
		const DATA_JSON_VALUE* pShape = Required(
			Value, "hitShape", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDamage = Required(
			Value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnimation = Required(
			Value, "animation", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pBranches = Required(
			Value, "branches", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pEffects = Required(
			Value, "effectRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pStageId || nullptr == pRole || nullptr == pAction ||
			nullptr == pKind || !Is_NonNegativeInteger(pDuration) ||
			0.0 == pDuration->Get_Number() || nullptr == pShape ||
			nullptr == pDamage || nullptr == pAnimation || nullptr == pBranches ||
			nullptr == pEffects ||
			!Is_StableToken(pStageId->Get_String()) ||
			!Is_StableToken(pRole->Get_String()) ||
			!Is_StableToken(pAction->Get_String()))
		{
			strOutError = "master stage identity is invalid";
			return false;
		}
		if (!Validate_MasterStageGameplayShape(Value, strOutError))
			return false;

		if (!Has_ExactProperties(
				*pAnimation, { "repeatCount", "endPolicy", "occurrences" }))
		{
			strOutError = "master stage animation has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pRepeatCount = pAnimation->Find("repeatCount");
		const DATA_JSON_VALUE* pEndPolicy = Required(
			*pAnimation, "endPolicy", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOccurrences = Required(
			*pAnimation, "occurrences", DATA_JSON_TYPE::ARRAY);
		if (!Is_NonNegativeInteger(pRepeatCount) ||
			pRepeatCount->Get_Number() < 1.0 ||
			pRepeatCount->Get_Number() > 64.0 || nullptr == pOccurrences ||
			pOccurrences->Get_Array().empty() || nullptr == pEndPolicy ||
			("EXACT" != pEndPolicy->Get_String() &&
			 "HOLD_LAST_POSE" != pEndPolicy->Get_String() &&
			 "LOOP_TO_STAGE_END" != pEndPolicy->Get_String()) ||
			pOccurrences->Get_Array().size() > 64u)
		{
			strOutError = "master stage repeatCount/occurrences is invalid";
			return false;
		}

		Out.strStageId = pStageId->Get_String();
		Out.strSequenceRole = pRole->Get_String();
		Out.strActionId = pAction->Get_String();
		Out.strStageKind = pKind->Get_String();
		Out.iDurationMs = static_cast<uint32_t>(pDuration->Get_Number());
		Out.iRepeatCount = static_cast<uint32_t>(pRepeatCount->Get_Number());
		Out.strAnimationEndPolicy = pEndPolicy->Get_String();
		Out.strHitShape = pShape->Get_String();
		Out.strServerDamageProfileId = pDamage->Get_String();
		if (!Read_RequiredFiniteFloat(
				Value, "hitOuterRadius", Out.fHitOuterRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitInnerRadius", Out.fHitInnerRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitAngleDegrees", Out.fHitAngleDegrees) ||
			!Read_RequiredFiniteFloat(Value, "hitLength", Out.fHitLength) ||
			!Read_RequiredFiniteFloat(
				Value, "hitHalfWidth", Out.fHitHalfWidth) ||
			!Read_RequiredUInt32(Value, "hitCount", Out.iHitCount) ||
			!Read_RequiredUInt32(Value, "hitIntervalMs", Out.iHitIntervalMs) ||
			!Read_RequiredUInt32(Value, "hitDelayMs", Out.iHitDelayMs) ||
			!Read_RequiredHitOffsets(Value, Out.HitOffsetsMs) ||
			!Read_RequiredFiniteFloat(Value, "pushRangeM", Out.fPushRangeM) ||
			!Read_RequiredUInt32(Value, "pushMs", Out.iPushMs) ||
			!Read_RequiredUInt32(Value, "downMs", Out.iDownMs) ||
			!Read_StageMotion(Value.Find("motion"), Out.Motion) ||
			!Read_StageActions(Value.Find("actions"), Out.Actions) ||
			!Read_StageBranches(pBranches, Out.Branches))
		{
			strOutError = "master stage gameplay values are invalid";
			return false;
		}
		Out.bKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		std::set<std::string, std::less<>> OccurrenceIds;
		for (const DATA_JSON_VALUE& OccurrenceValue : pOccurrences->Get_Array())
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
			if (!Parse_MasterOccurrence(
					OccurrenceValue, Occurrence, strOutError) ||
				!OccurrenceIds.insert(Occurrence.strClipOccurrenceId).second)
			{
				if (strOutError.empty())
					strOutError = "master stage duplicates a clip occurrence";
				return false;
			}
			Out.Occurrences.push_back(std::move(Occurrence));
		}
		/* repeatCount is a semantic promise that one authored clip is repeated,
		   not merely a display label for an arbitrary occurrence list. Keeping
		   this check in the shared master admission makes both Tools fail closed
		   before they can build different timelines. */
		if (1u < Out.iRepeatCount)
		{
			if (Out.Occurrences.size() != Out.iRepeatCount)
			{
				strOutError =
					"master repeatCount must own exactly its explicit occurrences: " +
					Out.strActionId;
				return false;
			}
			const std::string& strRepeatedClip =
				Out.Occurrences.front().strClipName;
			if (std::any_of(Out.Occurrences.begin(), Out.Occurrences.end(),
				[&strRepeatedClip](
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
				{
					return Occurrence.strClipName != strRepeatedClip;
				}))
			{
				strOutError =
					"master repeatCount occurrences must use one clip: " +
					Out.strActionId;
				return false;
			}
		}
		std::set<std::string, std::less<>> EffectReferenceIds;
		for (const DATA_JSON_VALUE& EffectValue : pEffects->Get_Array())
		{
			const DATA_JSON_VALUE* pType = Required(
				EffectValue, "refType", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pId = Required(
				EffectValue, "refId", DATA_JSON_TYPE::STRING);
			if (nullptr == pType || nullptr == pId ||
				("CUE_BINDING" != pType->Get_String() &&
				 "INDEPENDENT_EFFECT" != pType->Get_String()) ||
				!Is_StableToken(pId->Get_String()) ||
				!EffectReferenceIds.insert(
					pType->Get_String() + ":" + pId->Get_String()).second)
			{
				strOutError = "master effect reference is invalid or duplicated";
				return false;
			}
			MASTER_EFFECT_REFERENCE Reference;
			Reference.strType = pType->Get_String();
			Reference.strId = pId->Get_String();
			if ("INDEPENDENT_EFFECT" == Reference.strType)
			{
				if (!Has_ExactProperties(EffectValue, { "refType", "refId" }))
				{
					strOutError =
						"master independent Effect reference has unexpected properties";
					return false;
				}
			}
			else
			{
				if (!Has_ExactProperties(
						EffectValue, { "refType", "refId", "cueProjection" }))
				{
					strOutError =
						"master cue reference has unexpected properties";
					return false;
				}
				const DATA_JSON_VALUE* pProjection = Required(
					EffectValue, "cueProjection", DATA_JSON_TYPE::OBJECT);
				if (nullptr == pProjection || !Has_ExactProperties(*pProjection,
						{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
						  "mappingBasis" }))
				{
					strOutError = "master cueProjection contract is invalid";
					return false;
				}
				const DATA_JSON_VALUE* pClipOccurrence = Required(
					*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pSourceStart =
					pProjection->Find("sourceStartMs");
				const DATA_JSON_VALUE* pSourceEnd =
					pProjection->Find("sourceEndMs");
				const DATA_JSON_VALUE* pMappingBasis = Required(
					*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
				if (nullptr == pClipOccurrence ||
					!Is_StableToken(pClipOccurrence->Get_String()) ||
					!Is_NonNegativeInteger(pSourceStart) ||
					nullptr == pSourceEnd ||
					(!pSourceEnd->Is_Null() &&
					 !Is_NonNegativeInteger(pSourceEnd)) ||
					nullptr == pMappingBasis ||
					pMappingBasis->Get_String().empty())
				{
					strOutError = "master cueProjection values are invalid";
					return false;
				}
				Reference.strClipOccurrenceId = pClipOccurrence->Get_String();
				Reference.iSourceStartMs = static_cast<uint32_t>(
					pSourceStart->Get_Number());
				Reference.bHasSourceEnd = !pSourceEnd->Is_Null();
				Reference.iSourceEndMs = Reference.bHasSourceEnd ?
					static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
				Reference.strMappingBasis = pMappingBasis->Get_String();
			}
			Out.EffectReferences.push_back(std::move(Reference));
		}
		return true;
	}

	bool_t Validate_MasterServerMotion(
		const DATA_JSON_VALUE& Pattern,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pMotion = Pattern.Find("serverMotion");
		if (nullptr == pMotion)
			return false;
		if (pMotion->Is_Null())
			return true;
		if (!Has_ExactProperties(*pMotion,
				{ "kind", "anchorId", "landingPosition", "apexHeight",
				  "travelStageId" }) ||
			nullptr == Required(*pMotion, "kind", DATA_JSON_TYPE::STRING) ||
			nullptr == Required(*pMotion, "anchorId", DATA_JSON_TYPE::STRING) ||
			nullptr == Required(
				*pMotion, "travelStageId", DATA_JSON_TYPE::STRING) ||
			!Is_FiniteNumber(pMotion->Find("apexHeight")))
		{
			strOutError = "master serverMotion is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pLanding = Required(
			*pMotion, "landingPosition", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pLanding || 3u != pLanding->Get_Array().size() ||
			std::any_of(pLanding->Get_Array().begin(),
				pLanding->Get_Array().end(),
				[](const DATA_JSON_VALUE& Coordinate)
				{
					return !Is_FiniteNumber(&Coordinate);
				}))
		{
			strOutError = "master serverMotion landingPosition is invalid";
			return false;
		}
		return true;
	}

	bool_t Parse_MasterPattern(
		const DATA_JSON_VALUE& Value,
		MASTER_PATTERN& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "patternId", "category", "minimumPhase", "maximumPhase",
				  "targetPolicy", "aimPolicy", "displayName", "actionId",
				  "sourceActionIds", "sourceSequenceIndex", "presentationSources",
				  "selectionMode",
				  "minimumHealthBar", "maximumHealthBar", "triggerHealthBar",
				  "triggerOrder", "armorRequirement", "phaseRequirement",
				  "invulnerableWhileRunning", "selectionWeight",
				  "maximumConsecutiveUses", "minimumRange", "maximumRange",
				  "serverMotion", "reactions", "cameraCueIds",
				  "worldEventTriggerRefs", "stages" }))
		{
			strOutError = "master pattern has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pPatternId = Required(
			Value, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceSequence = Value.Find("sourceSequenceIndex");
		const DATA_JSON_VALUE* pStages = Required(
			Value, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pPatternId || nullptr == pDisplay || nullptr == pAction ||
			!Is_StableToken(pPatternId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAction->Get_String()) ||
			!Is_NonNegativeInteger(pSourceSequence) ||
			0.0 == pSourceSequence->Get_Number() || nullptr == pStages ||
			pStages->Get_Array().empty())
		{
			strOutError = "master pattern identity is invalid";
			return false;
		}

		for (const std::string_view Field :
			{ "category", "targetPolicy", "aimPolicy", "selectionMode",
			  "armorRequirement", "phaseRequirement" })
		{
			const DATA_JSON_VALUE* pText = Required(
				Value, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pText || pText->Get_String().empty())
			{
				strOutError = "master pattern string field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		for (const std::string_view Field :
			{ "minimumPhase", "maximumPhase", "minimumHealthBar",
			  "maximumHealthBar", "triggerHealthBar", "triggerOrder",
			  "selectionWeight", "maximumConsecutiveUses" })
		{
			if (!Is_NonNegativeInteger(Value.Find(Field)))
			{
				strOutError = "master pattern integer field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		if (!Is_FiniteNumber(Value.Find("minimumRange")) ||
			!Is_FiniteNumber(Value.Find("maximumRange")) ||
			nullptr == Required(
				Value, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN) ||
			!Validate_MasterServerMotion(Value, strOutError))
		{
			if (strOutError.empty())
				strOutError = "master pattern selection/motion fields are invalid";
			return false;
		}

		const DATA_JSON_VALUE* pSourceActions = Required(
			Value, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pSourceActions || pSourceActions->Get_Array().empty() ||
			std::any_of(pSourceActions->Get_Array().begin(),
				pSourceActions->Get_Array().end(),
				[](const DATA_JSON_VALUE& SourceAction)
				{
					return !Is_NonNegativeInteger(&SourceAction) ||
						0.0 == SourceAction.Get_Number();
				}))
		{
			strOutError = "master sourceActionIds is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pPresentationSources = Required(
			Value, "presentationSources", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pPresentationSources ||
			pPresentationSources->Get_Array().empty())
		{
			strOutError = "master presentationSources is invalid";
			return false;
		}
		std::set<std::string, std::less<>> PresentationSourceIdentities;
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			if (!Has_ExactProperties(
					Source, { "sourceActionId", "sequenceIndex", "role" }) ||
				!Is_NonNegativeInteger(Source.Find("sourceActionId")) ||
				0.0 == Source.Find("sourceActionId")->Get_Number() ||
				!Is_NonNegativeInteger(Source.Find("sequenceIndex")) ||
				0.0 == Source.Find("sequenceIndex")->Get_Number() ||
				nullptr == Required(Source, "role", DATA_JSON_TYPE::STRING))
			{
				strOutError = "master presentation source row is invalid";
				return false;
			}
			const std::string Identity = std::to_string(static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number())) + ":" +
				std::to_string(static_cast<uint32_t>(
					Source.Find("sequenceIndex")->Get_Number())) + ":" +
				Required(Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			if (!PresentationSourceIdentities.insert(Identity).second)
			{
				strOutError = "master presentation source row is duplicated";
				return false;
			}
		}
		const DATA_JSON_VALUE* pReactions = Required(
			Value, "reactions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pCameraCues = Required(
			Value, "cameraCueIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pWorldRefs = Required(
			Value, "worldEventTriggerRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pReactions || nullptr == pCameraCues ||
			nullptr == pWorldRefs)
		{
			strOutError = "master reactions/cue references are invalid";
			return false;
		}
		std::set<std::string, std::less<>> ReactionIdentities;
		for (const DATA_JSON_VALUE& Reaction : pReactions->Get_Array())
		{
			const DATA_JSON_VALUE* pTrigger = Required(
				Reaction, "triggerKind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStage = Required(
				Reaction, "stageId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(Reaction, { "triggerKind", "stageId" }) ||
				nullptr == pTrigger || nullptr == pStage ||
				!Is_StableToken(pTrigger->Get_String()) ||
				!Is_StableToken(pStage->Get_String()) ||
				!ReactionIdentities.insert(
					pTrigger->Get_String() + ":" + pStage->Get_String()).second)
			{
				strOutError = "master reaction is invalid or duplicated";
				return false;
			}
			Client::VALTAN_PATTERN_REACTION_VIEW ReactionView;
			ReactionView.strTriggerKind = pTrigger->Get_String();
			ReactionView.strStageId = pStage->Get_String();
			Out.Reactions.push_back(std::move(ReactionView));
		}
		std::set<std::string, std::less<>> CameraCueIdentities;
		for (const DATA_JSON_VALUE& CameraCue : pCameraCues->Get_Array())
		{
			if (!CameraCue.Is_String() ||
				!Is_StableToken(CameraCue.Get_String()) ||
				!CameraCueIdentities.insert(CameraCue.Get_String()).second)
			{
				strOutError = "master cameraCueIds is invalid or duplicated";
				return false;
			}
			Out.CameraCueIds.push_back(CameraCue.Get_String());
		}
		std::set<std::string, std::less<>> WorldReferenceIdentities;
		for (const DATA_JSON_VALUE& WorldRef : pWorldRefs->Get_Array())
		{
			const DATA_JSON_VALUE* pWorldPattern = Required(
				WorldRef, "patternId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldStage = Required(
				WorldRef, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldTrigger = Required(
				WorldRef, "triggerKind", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(
					WorldRef, { "patternId", "stageId", "triggerKind" }) ||
				nullptr == pWorldPattern || nullptr == pWorldStage ||
				nullptr == pWorldTrigger ||
				!Is_StableToken(pWorldPattern->Get_String()) ||
				!Is_StableToken(pWorldStage->Get_String()) ||
				!Is_StableToken(pWorldTrigger->Get_String()) ||
				!WorldReferenceIdentities.insert(
					pWorldPattern->Get_String() + ":" +
					pWorldStage->Get_String() + ":" +
					pWorldTrigger->Get_String()).second)
			{
				strOutError =
					"master worldEventTriggerRefs is invalid or duplicated";
				return false;
			}
			Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW Reference;
			Reference.strPatternId = pWorldPattern->Get_String();
			Reference.strStageId = pWorldStage->Get_String();
			Reference.strTriggerKind = pWorldTrigger->Get_String();
			Out.WorldEventTriggerRefs.push_back(std::move(Reference));
		}

		Out.strPatternId = pPatternId->Get_String();
		Out.strCategory = Required(
			Value, "category", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumPhase = static_cast<uint32_t>(
			Value.Find("minimumPhase")->Get_Number());
		Out.iMaximumPhase = static_cast<uint32_t>(
			Value.Find("maximumPhase")->Get_Number());
		Out.strTargetPolicy = Required(
			Value, "targetPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strAimPolicy = Required(
			Value, "aimPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strActionId = pAction->Get_String();
		for (const DATA_JSON_VALUE& SourceAction : pSourceActions->Get_Array())
		{
			Out.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}
		Out.strSelectionMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumHealthBar = static_cast<int32_t>(
			Value.Find("minimumHealthBar")->Get_Number());
		Out.iMaximumHealthBar = static_cast<int32_t>(
			Value.Find("maximumHealthBar")->Get_Number());
		Out.iTriggerHealthBar = static_cast<int32_t>(
			Value.Find("triggerHealthBar")->Get_Number());
		Out.iTriggerOrder = static_cast<uint32_t>(
			Value.Find("triggerOrder")->Get_Number());
		Out.strArmorRequirement = Required(
			Value, "armorRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strPhaseRequirement = Required(
			Value, "phaseRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.bInvulnerableWhileRunning = Required(
			Value, "invulnerableWhileRunning",
			DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		Out.iSelectionWeight = static_cast<uint32_t>(
			Value.Find("selectionWeight")->Get_Number());
		Out.iMaximumConsecutiveUses = static_cast<uint32_t>(
			Value.Find("maximumConsecutiveUses")->Get_Number());
		Out.fMinimumRange = static_cast<f32_t>(
			Value.Find("minimumRange")->Get_Number());
		Out.fMaximumRange = static_cast<f32_t>(
			Value.Find("maximumRange")->Get_Number());
		if (!Read_PatternServerMotion(Value.Find("serverMotion"), Out.ServerMotion))
		{
			strOutError = "master serverMotion values are invalid";
			return false;
		}
		Out.iSourceSequenceIndex = static_cast<uint32_t>(
			pSourceSequence->Get_Number());
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			Client::VALTAN_PRESENTATION_SOURCE_VIEW Presentation;
			Presentation.iSourceActionId = static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number());
			Presentation.iSequenceIndex = static_cast<uint32_t>(
				Source.Find("sequenceIndex")->Get_Number());
			Presentation.strRole = Required(
				Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			Out.PresentationSources.push_back(std::move(Presentation));
		}
		std::set<std::string, std::less<>> StageIds;
		for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
		{
			MASTER_STAGE Stage;
			if (!Parse_MasterStage(StageValue, Stage, strOutError) ||
				!StageIds.insert(Stage.strStageId).second)
			{
				if (strOutError.empty())
					strOutError = "master pattern duplicates a stageId";
				return false;
			}
			Out.Stages.push_back(std::move(Stage));
		}
		for (const Client::VALTAN_PATTERN_REACTION_VIEW& Reaction :
			Out.Reactions)
		{
			if (StageIds.end() == StageIds.find(Reaction.strStageId))
			{
				strOutError = "master reaction targets an unknown stage: " +
					Reaction.strStageId;
				return false;
			}
		}
		for (const Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Reference :
			Out.WorldEventTriggerRefs)
		{
			if (Reference.strPatternId != Out.strPatternId ||
				StageIds.end() == StageIds.find(Reference.strStageId))
			{
				strOutError =
					"master world Event trigger leaves its managed pattern/stage: " +
					Reference.strPatternId + "/" + Reference.strStageId;
				return false;
			}
		}
		return true;
	}

	bool_t Parse_MasterIndependentEffect(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "independentEffectId", "displayName", "effectAssetId",
				  "ownership", "ownerPatternId", "ownerStageId",
				  "triggerPolicy", "combatObjectArchetypeId", "clientVisualId",
				  "effectCueBindingId", "cueProjection" }))
		{
			strOutError = "master independent Effect has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pId = Required(
			Value, "independentEffectId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAsset = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOwnership = Required(
			Value, "ownership", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = Required(
			Value, "ownerPatternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pStage = Required(
			Value, "ownerStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTrigger = Required(
			Value, "triggerPolicy", DATA_JSON_TYPE::STRING);
		if (nullptr == pId || nullptr == pDisplay || nullptr == pAsset ||
			nullptr == pOwnership || nullptr == pPattern || nullptr == pStage ||
			nullptr == pTrigger || !Is_StableToken(pId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAsset->Get_String()) ||
			!Is_StableToken(pPattern->Get_String()) ||
			!Is_StableToken(pStage->Get_String()))
		{
			strOutError = "master independent Effect identity is invalid";
			return false;
		}
		Out.strIndependentEffectId = pId->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strEffectAssetId = pAsset->Get_String();
		Out.strOwnership = pOwnership->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.strTriggerPolicy = pTrigger->Get_String();
		if (!Read_NullableStableToken(Value, "combatObjectArchetypeId",
				Out.strCombatObjectArchetypeId) ||
			!Read_NullableStableToken(Value, "clientVisualId",
				Out.strClientVisualId) ||
			!Read_NullableStableToken(Value, "effectCueBindingId",
				Out.strEffectCueBindingId))
		{
			strOutError = "master independent Effect owner identity is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pProjection = Value.Find("cueProjection");
		if (nullptr == pProjection)
		{
			strOutError = "master independent Effect cueProjection is missing";
			return false;
		}
		const bool_t bCombatObject = "SERVER_COMBAT_OBJECT" == Out.strOwnership;
		const bool_t bPatternStage = "SERVER_PATTERN_STAGE" == Out.strOwnership;
		if ((!bCombatObject && !bPatternStage) ||
			(bCombatObject && (Out.strCombatObjectArchetypeId.empty() ||
				Out.strClientVisualId.empty() ||
				!Out.strEffectCueBindingId.empty())) ||
			(bPatternStage && (!Out.strCombatObjectArchetypeId.empty() ||
				!Out.strClientVisualId.empty() ||
				Out.strEffectCueBindingId.empty())))
		{
			strOutError = "master independent Effect ownership tuple is invalid";
			return false;
		}
		if (bCombatObject)
		{
			if (!pProjection->Is_Null())
			{
				strOutError =
					"master combat-object Effect cannot project a boss-root cue";
				return false;
			}
			return true;
		}
		if (!pProjection->Is_Object() || !Has_ExactProperties(*pProjection,
				{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
				  "mappingBasis" }))
		{
			strOutError =
				"master pattern-stage Effect cueProjection contract is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pClipOccurrence = Required(
			*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = pProjection->Find("sourceStartMs");
		const DATA_JSON_VALUE* pSourceEnd = pProjection->Find("sourceEndMs");
		const DATA_JSON_VALUE* pMappingBasis = Required(
			*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
		if (nullptr == pClipOccurrence ||
			!Is_StableToken(pClipOccurrence->Get_String()) ||
			!Is_NonNegativeInteger(pSourceStart) || nullptr == pSourceEnd ||
			(!pSourceEnd->Is_Null() && !Is_NonNegativeInteger(pSourceEnd)) ||
			nullptr == pMappingBasis || pMappingBasis->Get_String().empty())
		{
			strOutError =
				"master pattern-stage Effect cueProjection values are invalid";
			return false;
		}
		Out.bHasCueProjection = true;
		Out.strCueClipOccurrenceId = pClipOccurrence->Get_String();
		Out.iCueSourceStartMs = static_cast<uint32_t>(
			pSourceStart->Get_Number());
		Out.bHasCueSourceEnd = !pSourceEnd->Is_Null();
		Out.iCueSourceEndMs = Out.bHasCueSourceEnd ?
			static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
		Out.strCueMappingBasis = pMappingBasis->Get_String();
		return true;
	}

	bool_t Parse_MasterNormalSelection(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_NORMAL_SELECTION_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(
				Value, { "selectionMode", "ranges", "patternIds" }))
		{
			strOutError = "master normalSelection has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRanges = Required(
			Value, "ranges", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatternIds = Required(
			Value, "patternIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pMode || "WEIGHTED_POOL" != pMode->Get_String() ||
			nullptr == pRanges || pRanges->Get_Array().empty() ||
			nullptr == pPatternIds || pPatternIds->Get_Array().empty())
		{
			strOutError = "master normalSelection is invalid";
			return false;
		}
		Out.strSelectionMode = pMode->Get_String();
		std::set<std::string, std::less<>> RangeIds;
		uint32_t iPreviousFromHealthBar =
			(std::numeric_limits<uint32_t>::max)();
		for (const DATA_JSON_VALUE& Range : pRanges->Get_Array())
		{
			const DATA_JSON_VALUE* pRangeId = Required(
				Range, "rotationId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pFrom = Range.Find("fromHealthBar");
			const DATA_JSON_VALUE* pTo = Range.Find("toHealthBar");
			if (!Has_ExactProperties(
					Range, { "rotationId", "fromHealthBar", "toHealthBar" }) ||
				nullptr == pRangeId || !Is_StableToken(pRangeId->Get_String()) ||
				!Is_NonNegativeInteger(pFrom) || !Is_NonNegativeInteger(pTo) ||
				0.0 == pFrom->Get_Number() || 0.0 == pTo->Get_Number())
			{
				strOutError = "master normalSelection range is invalid";
				return false;
			}
			Client::VALTAN_NORMAL_SELECTION_RANGE_VIEW RangeView;
			RangeView.strRotationId = pRangeId->Get_String();
			RangeView.iFromHealthBar = static_cast<uint32_t>(pFrom->Get_Number());
			RangeView.iToHealthBar = static_cast<uint32_t>(pTo->Get_Number());
			if (!RangeIds.insert(RangeView.strRotationId).second ||
				RangeView.iFromHealthBar <= RangeView.iToHealthBar ||
				RangeView.iFromHealthBar > iPreviousFromHealthBar)
			{
				strOutError = "master normalSelection range is duplicated or inverted";
				return false;
			}
			iPreviousFromHealthBar = RangeView.iFromHealthBar;
			Out.Ranges.push_back(std::move(RangeView));
		}
		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternId : pPatternIds->Get_Array())
		{
			if (!PatternId.Is_String() ||
				!Is_StableToken(PatternId.Get_String()) ||
				!PatternIds.insert(PatternId.Get_String()).second)
			{
				strOutError =
					"master normalSelection patternId is invalid or duplicated";
				return false;
			}
			Out.PatternIds.push_back(PatternId.Get_String());
		}
		return true;
	}

	bool_t Parse_MasterCounterReactionLayer(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "reactionLayerId", "admissionScope", "ownerPatternId",
				  "ownerStageId", "windowActionId", "successActionId",
				  "failureActionId" }))
		{
			strOutError =
				"master counter reaction layer has unexpected properties";
			return false;
		}
		const auto ReadStable = [&Value](const std::string_view Name)
			-> const DATA_JSON_VALUE*
		{
			const DATA_JSON_VALUE* pValue = Required(
				Value, Name, DATA_JSON_TYPE::STRING);
			return nullptr != pValue && Is_StableToken(pValue->Get_String()) ?
				pValue : nullptr;
		};
		const DATA_JSON_VALUE* pLayerId = ReadStable("reactionLayerId");
		const DATA_JSON_VALUE* pScope = Required(
			Value, "admissionScope", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = ReadStable("ownerPatternId");
		const DATA_JSON_VALUE* pStage = ReadStable("ownerStageId");
		const DATA_JSON_VALUE* pWindow = ReadStable("windowActionId");
		const DATA_JSON_VALUE* pSuccess = ReadStable("successActionId");
		const DATA_JSON_VALUE* pFailure = ReadStable("failureActionId");
		if (nullptr == pLayerId || nullptr == pScope ||
			"REFERENCE_ONLY_LEGACY" != pScope->Get_String() ||
			nullptr == pPattern || nullptr == pStage || nullptr == pWindow ||
			nullptr == pSuccess || nullptr == pFailure)
		{
			strOutError = "master counter reaction layer is invalid";
			return false;
		}
		Out.strReactionLayerId = pLayerId->Get_String();
		Out.strAdmissionScope = pScope->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.Window.strActionId = pWindow->Get_String();
		Out.Success.strActionId = pSuccess->Get_String();
		Out.Failure.strActionId = pFailure->Get_String();
		return true;
	}

	bool_t Parse_MasterDocument(
		const DATA_JSON_VALUE& Root,
		MASTER_DOCUMENT& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Root,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "previewPaths", "retiredPatternIds",
				  "normalSelection", "counterReactionLayers",
				  "independentEffects", "patterns" }) ||
			nullptr == Required(Root, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-master" !=
				Required(Root, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(Root.Find("formatVersion")) ||
			1.0 != Root.Find("formatVersion")->Get_Number() ||
			nullptr == Required(Root, "bossArchetypeId", DATA_JSON_TYPE::STRING) ||
			"BOSS_VALTAN" != Required(
				Root, "bossArchetypeId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "encounterId", DATA_JSON_TYPE::STRING) ||
			"ENCOUNTER_VALTAN" != Required(
				Root, "encounterId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "scope", DATA_JSON_TYPE::STRING) ||
			"PHASE_ONE" != Required(
				Root, "scope", DATA_JSON_TYPE::STRING)->Get_String())
		{
			strOutError = "Valtan.pattern.json root contract is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pPreviewPaths = Required(
			Root, "previewPaths", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pPreviewPaths || !Has_ExactProperties(*pPreviewPaths,
				{ "encounter", "animationBindings", "effectCues",
				  "combatObjects", "bossCatalog", "effectCatalog",
				  "damageProfiles", "cinematicCamera", "worldEvents",
				  "patternRotations", "sourceClipSequences" }))
		{
			strOutError = "Valtan.pattern.json previewPaths contract is invalid";
			return false;
		}
		for (const auto& [Name, Value] : pPreviewPaths->Get_Object())
		{
			(void)Name;
			if (!Value.Is_String() || !Value.Get_String().starts_with("Data/"))
			{
				strOutError = "Valtan.pattern.json preview path is invalid";
				return false;
			}
		}

		const DATA_JSON_VALUE* pRetired = Required(
			Root, "retiredPatternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pIndependent = Required(
			Root, "independentEffects", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pNormalSelection = Required(
			Root, "normalSelection", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pCounterReactions = Required(
			Root, "counterReactionLayers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatterns = Required(
			Root, "patterns", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pRetired || nullptr == pIndependent ||
			nullptr == pNormalSelection || nullptr == pCounterReactions ||
			pCounterReactions->Get_Array().empty() ||
			nullptr == pPatterns || pIndependent->Get_Array().empty() ||
			pPatterns->Get_Array().empty())
		{
			strOutError = "Valtan.pattern.json inventories are invalid";
			return false;
		}

		std::set<std::string, std::less<>> RetiredIds;
		for (const DATA_JSON_VALUE& Retired : pRetired->Get_Array())
		{
			if (!Retired.Is_String() || !Is_StableToken(Retired.Get_String()) ||
				!RetiredIds.insert(Retired.Get_String()).second)
			{
				strOutError = "master retiredPatternIds is invalid or duplicated";
				return false;
			}
			Out.RetiredPatternIds.push_back(Retired.Get_String());
		}
		if (!Parse_MasterNormalSelection(
				*pNormalSelection, Out.NormalSelection, strOutError))
		{
			return false;
		}
		std::set<std::string, std::less<>> CounterLayerIds;
		std::set<std::string, std::less<>> CounterOwnerStages;
		for (const DATA_JSON_VALUE& LayerValue :
			pCounterReactions->Get_Array())
		{
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer;
			if (!Parse_MasterCounterReactionLayer(
					LayerValue, Layer, strOutError) ||
				!CounterLayerIds.insert(Layer.strReactionLayerId).second ||
				!CounterOwnerStages.insert(
					Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId).second)
			{
				if (strOutError.empty())
					strOutError =
						"master counter reaction identity is duplicated";
				return false;
			}
			Out.CounterReactionLayers.push_back(std::move(Layer));
		}

		std::set<std::string, std::less<>> IndependentIds;
		std::set<std::string, std::less<>> IndependentAssets;
		for (const DATA_JSON_VALUE& IndependentValue : pIndependent->Get_Array())
		{
			Client::VALTAN_INDEPENDENT_EFFECT_VIEW Independent;
			if (!Parse_MasterIndependentEffect(
					IndependentValue, Independent, strOutError) ||
				!IndependentIds.insert(
					Independent.strIndependentEffectId).second ||
				!IndependentAssets.insert(Independent.strEffectAssetId).second)
			{
				if (strOutError.empty())
					strOutError = "master independent Effect is duplicated";
				return false;
			}
			Out.IndependentEffects.push_back(std::move(Independent));
		}

		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
		{
			MASTER_PATTERN Pattern;
			if (!Parse_MasterPattern(PatternValue, Pattern, strOutError) ||
				!PatternIds.insert(Pattern.strPatternId).second ||
				RetiredIds.contains(Pattern.strPatternId))
			{
				if (strOutError.empty())
					strOutError = "master pattern identity is duplicated or retired";
				return false;
			}
			Out.Patterns.push_back(std::move(Pattern));
		}
		std::set<std::string, std::less<>> ManagedNormalPatternIds;
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if ("NORMAL" == Pattern.strSelectionMode)
				ManagedNormalPatternIds.insert(Pattern.strPatternId);
		}
		const std::set<std::string, std::less<>> WeightedPatternIds(
			Out.NormalSelection.PatternIds.begin(),
			Out.NormalSelection.PatternIds.end());
		if (WeightedPatternIds != ManagedNormalPatternIds)
		{
			strOutError =
				"master normalSelection is not the exact managed normal pattern set";
			return false;
		}
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
			Out.CounterReactionLayers)
		{
			if (PatternIds.contains(Layer.strOwnerPatternId))
			{
				strOutError =
					"reference-only counter reaction admits a managed pattern";
				return false;
			}
		}
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

	Client::VALTAN_PATTERN_VIEW* Find_Pattern(
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			const auto Found = std::find_if(
				pGroup->begin(), pGroup->end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (Found != pGroup->end())
				return &*Found;
		}
		return nullptr;
	}

	Client::VALTAN_STAGE_VIEW* Find_Stage(
		Client::VALTAN_PATTERN_VIEW& Pattern,
		const std::string_view strStageId)
	{
		const auto Found = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[strStageId](const Client::VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == strStageId;
			});
		return Found == Pattern.Stages.end() ? nullptr : &*Found;
	}

	bool_t Equal_MasterOccurrence(
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Product,
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Master)
	{
		return Product.strClipOccurrenceId == Master.strClipOccurrenceId &&
			Product.strClipName == Master.strClipName &&
			Product.strMappingBasis == Master.strMappingBasis &&
			Product.iSourceStartMs == Master.iSourceStartMs &&
			Product.iPlayMs == Master.iPlayMs &&
			std::abs(Product.fPlayRate - Master.fPlayRate) <= 0.000001f &&
			Product.bLoop == Master.bLoop;
	}

	bool_t Equal_StageMotion(
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->fDistance == Right->fDistance);
	}

	bool_t Equal_StageActions(
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strTrigger != Right[i].strTrigger ||
				Left[i].strKind != Right[i].strKind ||
				Left[i].strTargetId != Right[i].strTargetId ||
				Left[i].fValue != Right[i].fValue ||
				Left[i].iDurationMs != Right[i].iDurationMs)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Equal_StageBranches(
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strOutcome != Right[i].strOutcome ||
				Left[i].strNextActionId != Right[i].strNextActionId)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Equal_MasterStageGameplay(
		const Client::VALTAN_STAGE_VIEW& Product,
		const MASTER_STAGE& Master)
	{
		return Product.strStageId == Master.strStageId &&
			Product.strActionId == Master.strActionId &&
			Product.strStageKind == Master.strStageKind &&
			Product.iDurationMs == Master.iDurationMs &&
			Product.strHitShape == Master.strHitShape &&
			Product.fHitOuterRadius == Master.fHitOuterRadius &&
			Product.fHitInnerRadius == Master.fHitInnerRadius &&
			Product.fHitAngleDegrees == Master.fHitAngleDegrees &&
			Product.fHitLength == Master.fHitLength &&
			Product.fHitHalfWidth == Master.fHitHalfWidth &&
			Product.iHitCount == Master.iHitCount &&
			Product.iHitIntervalMs == Master.iHitIntervalMs &&
			Product.iHitDelayMs == Master.iHitDelayMs &&
			Product.HitOffsetsMs == Master.HitOffsetsMs &&
			Product.strServerDamageProfileId ==
				Master.strServerDamageProfileId &&
			Product.fPushRangeM == Master.fPushRangeM &&
			Product.iPushMs == Master.iPushMs &&
			Product.bKnockdown == Master.bKnockdown &&
			Product.iDownMs == Master.iDownMs &&
			Equal_StageMotion(Product.Motion, Master.Motion) &&
			Equal_StageActions(Product.Actions, Master.Actions) &&
			Equal_StageBranches(Product.Branches, Master.Branches);
	}

	bool_t Equal_PatternServerMotion(
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->strAnchorId == Right->strAnchorId &&
			 Left->LandingPosition == Right->LandingPosition &&
			 Left->fApexHeight == Right->fApexHeight &&
			 Left->strTravelStageId == Right->strTravelStageId);
	}

	bool_t Equal_MasterPatternGameplay(
		const Client::VALTAN_PATTERN_VIEW& Product,
		const MASTER_PATTERN& Master)
	{
		return Product.strPatternId == Master.strPatternId &&
			Product.strCategory == Master.strCategory &&
			Product.iMinimumPhase == Master.iMinimumPhase &&
			Product.iMaximumPhase == Master.iMaximumPhase &&
			Product.strTargetPolicy == Master.strTargetPolicy &&
			Product.strAimPolicy == Master.strAimPolicy &&
			Product.strDisplayName == Master.strDisplayName &&
			Product.strActionId == Master.strActionId &&
			Product.SourceActionIds == Master.SourceActionIds &&
			Product.strSelectionMode == Master.strSelectionMode &&
			Product.iMinimumHealthBar == Master.iMinimumHealthBar &&
			Product.iMaximumHealthBar == Master.iMaximumHealthBar &&
			Product.iTriggerHealthBar == Master.iTriggerHealthBar &&
			Product.iTriggerOrder == Master.iTriggerOrder &&
			Product.strArmorRequirement == Master.strArmorRequirement &&
			Product.strPhaseRequirement == Master.strPhaseRequirement &&
			Product.bInvulnerableWhileRunning ==
				Master.bInvulnerableWhileRunning &&
			Product.iSelectionWeight == Master.iSelectionWeight &&
			Product.iMaximumConsecutiveUses ==
				Master.iMaximumConsecutiveUses &&
			Product.fMinimumRange == Master.fMinimumRange &&
			Product.fMaximumRange == Master.fMaximumRange &&
			Equal_PatternServerMotion(Product.ServerMotion, Master.ServerMotion);
	}

	bool_t Assign_MasterWallBudgets(
		Client::VALTAN_STAGE_VIEW& Stage,
		std::string& strOutError)
	{
		uint64_t iKnownWallMs = 0u;
		size_t iUnknownCount = 0u;
		size_t iUnknownIndex = 0u;
		for (size_t i = 0u; i < Stage.ClipOccurrences.size(); ++i)
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[i];
			Clip.iAuthoringWallMs = 0u;
			if (0u == Clip.iPlayMs)
			{
				++iUnknownCount;
				iUnknownIndex = i;
				continue;
			}
			const double WallMs = static_cast<double>(Clip.iPlayMs) /
				static_cast<double>(Clip.fPlayRate);
			if (!std::isfinite(WallMs) || WallMs <= 0.0 ||
				WallMs > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutError = "master animation wall budget is invalid: " +
					Clip.strClipOccurrenceId;
				return false;
			}
			Clip.iAuthoringWallMs = static_cast<uint32_t>(std::llround(WallMs));
			iKnownWallMs += Clip.iAuthoringWallMs;
		}
		if (1u < iUnknownCount || iKnownWallMs >
			static_cast<uint64_t>(Stage.iDurationMs) + 2u)
		{
			strOutError =
				"master stage cannot derive one deterministic animation wall budget: " +
				Stage.strActionId;
			return false;
		}
		const bool_t bUnknownLoops = 1u == iUnknownCount &&
			Stage.ClipOccurrences[iUnknownIndex].bLoop;
		if (bUnknownLoops &&
			"LOOP_TO_STAGE_END" != Stage.strAnimationEndPolicy)
		{
			strOutError =
				"repeatUntilStageEnd requires LOOP_TO_STAGE_END: " +
				Stage.strActionId;
			return false;
		}
		if ("LOOP_TO_STAGE_END" == Stage.strAnimationEndPolicy)
		{
			if (1u != iUnknownCount || !bUnknownLoops ||
				iKnownWallMs >= Stage.iDurationMs)
			{
				strOutError =
					"LOOP_TO_STAGE_END requires one looping remainder: " +
					Stage.strActionId;
				return false;
			}
			const uint64_t iRemaining =
				static_cast<uint64_t>(Stage.iDurationMs) - iKnownWallMs;
			Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
				static_cast<uint32_t>(iRemaining);
			iKnownWallMs += iRemaining;
		}
		else if ("HOLD_LAST_POSE" == Stage.strAnimationEndPolicy)
		{
			if (bUnknownLoops || iKnownWallMs >= Stage.iDurationMs + 2ull)
			{
				strOutError = "HOLD_LAST_POSE animation budget is invalid: " +
					Stage.strActionId;
				return false;
			}
			if (1u == iUnknownCount)
			{
				if (iKnownWallMs >= Stage.iDurationMs)
					return false;
				Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
			else if (iKnownWallMs < Stage.iDurationMs &&
				!Stage.ClipOccurrences.empty())
			{
				Stage.ClipOccurrences.back().iAuthoringWallMs +=
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
		}
		else if ("EXACT" == Stage.strAnimationEndPolicy)
		{
			const int64_t iDifference = static_cast<int64_t>(Stage.iDurationMs) -
				static_cast<int64_t>(iKnownWallMs);
			if (0u != iUnknownCount || std::abs(iDifference) > 2 ||
				Stage.ClipOccurrences.empty())
			{
				strOutError = "EXACT animation does not fill its Server stage: " +
					Stage.strActionId;
				return false;
			}
			const int64_t iCorrected = static_cast<int64_t>(
				Stage.ClipOccurrences.back().iAuthoringWallMs) + iDifference;
			if (iCorrected <= 0)
				return false;
			Stage.ClipOccurrences.back().iAuthoringWallMs =
				static_cast<uint32_t>(iCorrected);
			iKnownWallMs = Stage.iDurationMs;
		}
		else
		{
			strOutError = "master animation endPolicy is invalid: " +
				Stage.strActionId;
			return false;
		}
		if (iKnownWallMs != Stage.iDurationMs)
		{
			strOutError = "master animation wall budget did not commit exactly: " +
				Stage.strActionId;
			return false;
		}
		return true;
	}

	bool_t Apply_MasterDocument(
		const MASTER_DOCUMENT& Master,
		const DATA_JSON_VALUE& PatternRotations,
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		std::string& strOutError)
	{
		for (const std::string& strRetiredPatternId : Master.RetiredPatternIds)
		{
			if (nullptr != Find_Pattern(View, strRetiredPatternId))
			{
				strOutError = "retired master pattern still exists in Product: " +
					strRetiredPatternId;
				return false;
			}
		}

		std::map<std::string,
			const Client::VALTAN_INDEPENDENT_EFFECT_VIEW*, std::less<>>
			IndependentById;
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			IndependentById.emplace(
				Independent.strIndependentEffectId, &Independent);
		}
		std::set<std::string, std::less<>> ReferencedIndependentIds;

		for (const MASTER_PATTERN& MasterPattern : Master.Patterns)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(
				View, MasterPattern.strPatternId);
			if (nullptr == pPattern ||
				!Equal_MasterPatternGameplay(*pPattern, MasterPattern) ||
				pPattern->Stages.size() != MasterPattern.Stages.size())
			{
				strOutError = "master/Product pattern projection changed: " +
					MasterPattern.strPatternId;
				return false;
			}

			pPattern->iSourceSequenceIndex =
				MasterPattern.iSourceSequenceIndex;
			pPattern->PresentationSources = MasterPattern.PresentationSources;
			pPattern->Reactions = MasterPattern.Reactions;
			pPattern->CameraCueIds = MasterPattern.CameraCueIds;
			pPattern->WorldEventTriggerRefs =
				MasterPattern.WorldEventTriggerRefs;
			pPattern->bAuthoringMasterManaged = true;
			for (size_t iStage = 0u; iStage < MasterPattern.Stages.size();
				++iStage)
			{
				const MASTER_STAGE& MasterStage = MasterPattern.Stages[iStage];
				Client::VALTAN_STAGE_VIEW& Stage = pPattern->Stages[iStage];
				if (!Equal_MasterStageGameplay(Stage, MasterStage) ||
					Stage.ClipOccurrences.size() !=
						MasterStage.Occurrences.size())
				{
					strOutError = "master/Product stage projection changed: " +
						MasterPattern.strPatternId + "/" + MasterStage.strStageId;
					return false;
				}
				for (size_t iClip = 0u;
					iClip < MasterStage.Occurrences.size(); ++iClip)
				{
					if (!Equal_MasterOccurrence(
							Stage.ClipOccurrences[iClip],
							MasterStage.Occurrences[iClip]))
					{
						strOutError =
							"master/Product animation occurrence changed: " +
							MasterStage.Occurrences[iClip].strClipOccurrenceId;
						return false;
					}
				}
				Stage.strSequenceRole = MasterStage.strSequenceRole;
				Stage.iAuthoringRepeatCount = MasterStage.iRepeatCount;
				Stage.strAnimationEndPolicy =
					MasterStage.strAnimationEndPolicy;
				if (!Assign_MasterWallBudgets(Stage, strOutError))
					return false;

				for (const MASTER_EFFECT_REFERENCE& Reference :
					MasterStage.EffectReferences)
				{
					if ("CUE_BINDING" == Reference.strType)
					{
						const bool_t bFound = std::any_of(
							Stage.ProductCues.begin(), Stage.ProductCues.end(),
							[&Reference](
								const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
							{
								return Cue.strBindingId == Reference.strId &&
									Cue.strClipOccurrenceId ==
										Reference.strClipOccurrenceId &&
									Cue.iSourceStartMs == Reference.iSourceStartMs &&
									Cue.bHasSourceEnd == Reference.bHasSourceEnd &&
									(!Reference.bHasSourceEnd ||
									 Cue.iSourceEndMs == Reference.iSourceEndMs);
							});
						if (!bFound)
						{
							strOutError =
								"master cue reference left its Product stage: " +
								Reference.strId;
							return false;
						}
						continue;
					}
					const auto Independent = IndependentById.find(Reference.strId);
					if (Independent == IndependentById.end() ||
						Independent->second->strOwnerPatternId !=
							pPattern->strPatternId ||
						Independent->second->strOwnerStageId != Stage.strStageId)
					{
						strOutError = "master independent Effect reference is stale: " +
							Reference.strId;
						return false;
					}
					Stage.IndependentEffectIds.push_back(Reference.strId);
					ReferencedIndependentIds.insert(Reference.strId);
				}
			}
		}

		const DATA_JSON_VALUE* pRotationRows = Required(
			PatternRotations, "rotations", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactProperties(PatternRotations,
				{ "schema", "formatVersion", "encounterId", "bossArchetypeId",
				  "rotations" }) ||
			nullptr == Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-rotations" != Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(PatternRotations.Find("formatVersion")) ||
			2.0 != PatternRotations.Find("formatVersion")->Get_Number() ||
			nullptr == pRotationRows)
		{
			strOutError = "Valtan normal-selection Product is invalid";
			return false;
		}
		for (const Client::VALTAN_NORMAL_SELECTION_RANGE_VIEW& Range :
			Master.NormalSelection.Ranges)
		{
			const DATA_JSON_VALUE* pFound = nullptr;
			for (const DATA_JSON_VALUE& Row : pRotationRows->Get_Array())
			{
				const DATA_JSON_VALUE* pId = Required(
					Row, "rotationId", DATA_JSON_TYPE::STRING);
				if (nullptr != pId && pId->Get_String() == Range.strRotationId)
				{
					if (nullptr != pFound)
					{
						strOutError =
							"Valtan normal-selection range is duplicated in Product";
						return false;
					}
					pFound = &Row;
				}
			}
			const DATA_JSON_VALUE* pMode = nullptr == pFound ? nullptr :
				Required(*pFound, "selectionMode", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pFrom = nullptr == pFound ? nullptr :
				pFound->Find("fromHealthBar");
			const DATA_JSON_VALUE* pTo = nullptr == pFound ? nullptr :
				pFound->Find("toHealthBar");
			const DATA_JSON_VALUE* pPatternIds = nullptr == pFound ? nullptr :
				Required(*pFound, "patternIds", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pFound || !Has_ExactProperties(*pFound,
					{ "rotationId", "selectionMode", "fromHealthBar",
					  "toHealthBar", "patternIds" }) ||
				nullptr == pMode ||
				Master.NormalSelection.strSelectionMode != pMode->Get_String() ||
				!Is_NonNegativeInteger(pFrom) || !Is_NonNegativeInteger(pTo) ||
				Range.iFromHealthBar != static_cast<uint32_t>(pFrom->Get_Number()) ||
				Range.iToHealthBar != static_cast<uint32_t>(pTo->Get_Number()) ||
				nullptr == pPatternIds ||
				pPatternIds->Get_Array().size() !=
					Master.NormalSelection.PatternIds.size())
			{
				strOutError =
					"Valtan normal-selection range drifted from master: " +
					Range.strRotationId;
				return false;
			}
			for (size_t iPattern = 0u;
				iPattern < Master.NormalSelection.PatternIds.size(); ++iPattern)
			{
				const DATA_JSON_VALUE& PatternId =
					pPatternIds->Get_Array()[iPattern];
				if (!PatternId.Is_String() || PatternId.Get_String() !=
					Master.NormalSelection.PatternIds[iPattern])
				{
					strOutError =
						"Valtan normal-selection Product pool order changed";
					return false;
				}
			}
		}
		View.NormalSelection = Master.NormalSelection;
		for (const std::string& PatternId : View.NormalSelection.PatternIds)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(View, PatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				"NORMAL" != pPattern->strSelectionMode)
			{
				strOutError =
					"Valtan normal-selection pool left the managed normal set: " +
					PatternId;
				return false;
			}
		}

		std::set<std::string, std::less<>> ProductCounterOwnerStages;
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			for (Client::VALTAN_PATTERN_VIEW& Pattern : *pGroup)
			{
				for (Client::VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					const bool_t bCounterable = std::any_of(
						Stage.Actions.begin(), Stage.Actions.end(),
						[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
						{
							return "ENTER" == Action.strTrigger &&
								"SET_BOSS_FLAG" == Action.strKind &&
								"boss.flag.counterable" == Action.strTargetId &&
								1.f == Action.fValue;
						});
					if (bCounterable)
						ProductCounterOwnerStages.insert(
							Pattern.strPatternId + "/" + Stage.strStageId);
				}
			}
		}
		std::set<std::string, std::less<>> MasterCounterOwnerStages;
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& MasterLayer :
			Master.CounterReactionLayers)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, MasterLayer.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pWindow = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, MasterLayer.strOwnerStageId);
			const auto FindActionStage = [pOwnerPattern](
				const std::string& ActionId) -> Client::VALTAN_STAGE_VIEW*
			{
				if (nullptr == pOwnerPattern)
					return nullptr;
				Client::VALTAN_STAGE_VIEW* pFound = nullptr;
				for (Client::VALTAN_STAGE_VIEW& Stage : pOwnerPattern->Stages)
				{
					if (Stage.strActionId != ActionId)
						continue;
					if (nullptr != pFound)
						return nullptr;
					pFound = &Stage;
				}
				return pFound;
			};
			Client::VALTAN_STAGE_VIEW* pSuccess = FindActionStage(
				MasterLayer.Success.strActionId);
			Client::VALTAN_STAGE_VIEW* pFailure = FindActionStage(
				MasterLayer.Failure.strActionId);
			const bool_t bHasCounterEnter = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "ENTER" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						1.f == Action.fValue;
				});
			const bool_t bHasCounterExit = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "EXIT" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						0.f == Action.fValue;
				});
			const auto HasBranch = [pWindow](
				const std::string_view Outcome, const std::string& ActionId)
			{
				return nullptr != pWindow && std::any_of(
					pWindow->Branches.begin(), pWindow->Branches.end(),
					[Outcome, &ActionId](
						const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{
						return Branch.strOutcome == Outcome &&
							Branch.strNextActionId.has_value() &&
							*Branch.strNextActionId == ActionId;
					});
			};
			if (nullptr == pOwnerPattern || pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pWindow || pWindow->strActionId !=
					MasterLayer.Window.strActionId ||
				nullptr == pSuccess || nullptr == pFailure ||
				pWindow->ClipOccurrences.empty() ||
				pSuccess->ClipOccurrences.empty() ||
				pFailure->ClipOccurrences.empty() ||
				!bHasCounterEnter || !bHasCounterExit ||
				!HasBranch("COUNTER_HIT", MasterLayer.Success.strActionId) ||
				!HasBranch("TIMEOUT", MasterLayer.Failure.strActionId))
			{
				strOutError =
					"reference-only counter reaction Product join changed: " +
					MasterLayer.strReactionLayerId;
				return false;
			}
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer = MasterLayer;
			Layer.Window.ClipOccurrences = pWindow->ClipOccurrences;
			Layer.Success.ClipOccurrences = pSuccess->ClipOccurrences;
			Layer.Failure.ClipOccurrences = pFailure->ClipOccurrences;
			View.CounterReactionLayers.push_back(std::move(Layer));
			MasterCounterOwnerStages.insert(
				MasterLayer.strOwnerPatternId + "/" +
				MasterLayer.strOwnerStageId);
		}
		if (ProductCounterOwnerStages != MasterCounterOwnerStages)
		{
			strOutError =
				"counter reaction master does not cover exact Product counter stages";
			return false;
		}

		if (ReferencedIndependentIds.size() != Master.IndependentEffects.size())
		{
			strOutError =
				"master independent Effect inventory contains an unreferenced identity";
			return false;
		}
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, Independent.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pOwnerStage = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, Independent.strOwnerStageId);
			if (nullptr == pOwnerPattern || !pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pOwnerStage)
			{
				strOutError = "master independent Effect owner is stale: " +
					Independent.strIndependentEffectId;
				return false;
			}
			bool_t bOwnerJoined = false;
			if ("SERVER_COMBAT_OBJECT" == Independent.strOwnership)
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->CombatObjectEffects.begin(),
					pOwnerStage->CombatObjectEffects.end(),
					[&Independent](
						const Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect)
					{
						return Effect.strEffectAssetId ==
								Independent.strEffectAssetId &&
							Effect.strCombatObjectArchetypeId ==
								Independent.strCombatObjectArchetypeId &&
							Effect.strClientVisualId ==
								Independent.strClientVisualId;
					});
			}
			else
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->ProductCues.begin(),
					pOwnerStage->ProductCues.end(),
					[&Independent](
						const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
					{
						return Cue.strEffectAssetId == Independent.strEffectAssetId &&
							Cue.strBindingId == Independent.strEffectCueBindingId &&
							Independent.bHasCueProjection &&
							Cue.strClipOccurrenceId ==
								Independent.strCueClipOccurrenceId &&
							Cue.iSourceStartMs == Independent.iCueSourceStartMs &&
							Cue.bHasSourceEnd == Independent.bHasCueSourceEnd &&
							(!Independent.bHasCueSourceEnd ||
							 Cue.iSourceEndMs == Independent.iCueSourceEndMs);
					});
			}
			const std::filesystem::path EffectDocument =
				Client::CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") / L"Authored" /
					std::filesystem::path(
						Independent.strEffectAssetId + ".effect.json"));
			std::error_code FileError;
			if (!bOwnerJoined || EffectDocument.empty() ||
				!std::filesystem::is_regular_file(EffectDocument, FileError))
			{
				strOutError =
					"master independent Effect did not resolve to one Product owner/document: " +
					Independent.strIndependentEffectId;
				return false;
			}
		}

		View.IndependentEffects = Master.IndependentEffects;
		return true;
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

bool_t Client::CValtanPatternTree::Build_PreviewStagePath(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::vector<const VALTAN_STAGE_VIEW*>& OutStages,
	std::string& strOutStatus)
{
	if (VALTAN_PATTERN_PREVIEW_PATH::END == ePath || Pattern.Stages.empty())
	{
		strOutStatus = "Valtan preview path request is invalid.";
		return false;
	}

	std::map<std::string, size_t, std::less<>> StageByAction;
	for (size_t i = 0u; i < Pattern.Stages.size(); ++i)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[i];
		if (Stage.strActionId.empty() ||
			!StageByAction.emplace(Stage.strActionId, i).second)
		{
			strOutStatus = "Valtan preview path has a missing or duplicated action: " +
				Stage.strActionId;
			return false;
		}
		std::set<std::string, std::less<>> Outcomes;
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strOutcome.empty() ||
				!Outcomes.insert(Branch.strOutcome).second)
			{
				strOutStatus = "Valtan preview path has ambiguous branches: " +
					Stage.strActionId;
				return false;
			}
		}
	}
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strNextActionId.has_value() &&
				!StageByAction.contains(*Branch.strNextActionId))
			{
				strOutStatus =
					"Valtan preview path branches to an unknown action: " +
					*Branch.strNextActionId;
				return false;
			}
		}
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagedPath;
	std::set<std::string, std::less<>> VisitedActions;
	size_t iStage = 0u;
	bool_t bWallContactTaken = false;
	for (;;)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!VisitedActions.insert(Stage.strActionId).second)
		{
			strOutStatus = "Valtan preview path contains a cycle: " +
				Stage.strActionId;
			return false;
		}
		StagedPath.push_back(&Stage);

		if (Stage.Branches.empty())
		{
			if (iStage + 1u == Pattern.Stages.size())
				break;
			++iStage;
			continue;
		}

		const VALTAN_STAGE_BRANCH_VIEW* pSelected = nullptr;
		const auto SelectOutcome = [&Stage, &pSelected](
			const std::string_view strOutcome) -> bool_t
		{
			for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
			{
				if (Branch.strOutcome != strOutcome)
					continue;
				if (nullptr != pSelected)
					return false;
				pSelected = &Branch;
			}
			return true;
		};

		if (VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK == ePath &&
			bWallContactTaken)
		{
			if (!SelectOutcome("PART_DESTROYED"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous PART_DESTROYED edges.";
				return false;
			}
		}
		if (nullptr == pSelected &&
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL != ePath &&
			!bWallContactTaken)
		{
			if (!SelectOutcome("WALL_CONTACT"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous WALL_CONTACT edges.";
				return false;
			}
			bWallContactTaken = nullptr != pSelected;
		}
		if (nullptr == pSelected && !SelectOutcome("TIMEOUT"))
		{
			strOutStatus =
				"Valtan preview path has ambiguous TIMEOUT edges.";
			return false;
		}
		if (nullptr == pSelected)
		{
			strOutStatus = "Valtan preview path has no policy-selectable edge: " +
				Stage.strActionId;
			return false;
		}
		if (!pSelected->strNextActionId.has_value())
			break;
		const auto Next = StageByAction.find(*pSelected->strNextActionId);
		if (Next == StageByAction.end())
		{
			strOutStatus = "Valtan preview path branches to an unknown action: " +
				*pSelected->strNextActionId;
			return false;
		}
		iStage = Next->second;
	}

	OutStages = std::move(StagedPath);
	strOutStatus = "Valtan preview path resolved " +
		std::to_string(OutStages.size()) + " stages.";
	return true;
}

bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	DATA_JSON_VALUE Encounter;
	DATA_JSON_VALUE MasterRoot;
	DATA_JSON_VALUE PatternRotations;
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
	if (!Parse_Document(std::filesystem::path(L"Valtan") /
			L"Valtan.pattern.json", MasterRoot, Error))
	{
		strOutStatus = "Valtan authoring master load failed: " + Error;
		return false;
	}
	if (!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanPatternRotations.json", PatternRotations, Error))
	{
		strOutStatus = "Valtan normal-selection product load failed: " + Error;
		return false;
	}
	MASTER_DOCUMENT MasterDocument;
	if (!Parse_MasterDocument(MasterRoot, MasterDocument, Error))
	{
		strOutStatus = "Valtan authoring master validation failed: " + Error;
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
		Pattern.strCategory = Read_String(PatternValue, "category");
		Pattern.strTargetPolicy = Read_String(PatternValue, "targetPolicy");
		Pattern.strAimPolicy = Read_String(PatternValue, "aimPolicy");
		Pattern.strDisplayName = Read_String(PatternValue, "displayName");
		Pattern.strActionId = Read_String(PatternValue, "actionId");
		Pattern.strSelectionMode = Read_String(PatternValue, "selectionMode");
		Pattern.strArmorRequirement = Read_String(
			PatternValue, "armorRequirement");
		Pattern.strPhaseRequirement = Read_String(
			PatternValue, "phaseRequirement");
		uint32_t iMinimumHealthBar = 0u;
		uint32_t iMaximumHealthBar = 0u;
		uint32_t iTriggerHealthBar = 0u;
		const DATA_JSON_VALUE* pSourceActionIds = Required(
			PatternValue, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pInvulnerable = Required(
			PatternValue, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN);
		if (Pattern.strPatternId.empty() || Pattern.strCategory.empty() ||
			Pattern.strTargetPolicy.empty() || Pattern.strAimPolicy.empty() ||
			Pattern.strDisplayName.empty() || Pattern.strActionId.empty() ||
			Pattern.strSelectionMode.empty() ||
			Pattern.strArmorRequirement.empty() ||
			Pattern.strPhaseRequirement.empty() || nullptr == pInvulnerable ||
			nullptr == pSourceActionIds || pSourceActionIds->Get_Array().empty() ||
			!Read_RequiredUInt32(
				PatternValue, "minimumPhase", Pattern.iMinimumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumPhase", Pattern.iMaximumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "minimumHealthBar", iMinimumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumHealthBar", iMaximumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerHealthBar", iTriggerHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerOrder", Pattern.iTriggerOrder) ||
			!Read_RequiredUInt32(
				PatternValue, "selectionWeight", Pattern.iSelectionWeight) ||
			!Read_RequiredUInt32(PatternValue, "maximumConsecutiveUses",
				Pattern.iMaximumConsecutiveUses) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "minimumRange", Pattern.fMinimumRange) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "maximumRange", Pattern.fMaximumRange) ||
			!Read_PatternServerMotion(
				PatternValue.Find("serverMotion"), Pattern.ServerMotion))
		{
			strOutStatus =
				"Valtan encounter pattern gameplay projection is invalid: " +
				Pattern.strPatternId;
			return false;
		}
		Pattern.iMinimumHealthBar = static_cast<int32_t>(iMinimumHealthBar);
		Pattern.iMaximumHealthBar = static_cast<int32_t>(iMaximumHealthBar);
		Pattern.iTriggerHealthBar = static_cast<int32_t>(iTriggerHealthBar);
		Pattern.bInvulnerableWhileRunning = pInvulnerable->Get_Boolean();
		for (const DATA_JSON_VALUE& SourceAction :
			pSourceActionIds->Get_Array())
		{
			if (!Is_NonNegativeInteger(&SourceAction) ||
				0.0 == SourceAction.Get_Number())
			{
				strOutStatus = "Valtan encounter sourceActionIds is invalid: " +
					Pattern.strPatternId;
				return false;
			}
			Pattern.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}

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
				const DATA_JSON_VALUE* pKnockdown = Required(
					StageValue, "knockdown", DATA_JSON_TYPE::BOOLEAN);
				if (nullptr == Required(StageValue, "serverDamageProfileId",
						DATA_JSON_TYPE::STRING) ||
					nullptr == pKnockdown ||
					!Read_RequiredFiniteFloat(
						StageValue, "pushRangeM", Stage.fPushRangeM) ||
					!Read_RequiredUInt32(
						StageValue, "pushMs", Stage.iPushMs) ||
					!Read_RequiredUInt32(
						StageValue, "downMs", Stage.iDownMs) ||
					!Read_StageMotion(StageValue.Find("motion"), Stage.Motion) ||
					!Read_StageActions(StageValue.Find("actions"), Stage.Actions) ||
					!Read_StageBranches(StageValue.Find("branches"), Stage.Branches))
				{
					strOutStatus =
						"Valtan encounter stage gameplay projection is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.bKnockdown = pKnockdown->Get_Boolean();

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
	if (!Apply_MasterDocument(
			MasterDocument, PatternRotations, Staged, Error))
	{
		strOutStatus = "Valtan authoring master join failed: " + Error;
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
		std::to_string(OutView.IndependentEffects.size()) +
		" independent Effects, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	return true;
}
