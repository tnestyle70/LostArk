#include "ValtanPatternEffectCueDocument.h"

#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr std::string_view SCHEMA =
		"lostark.valtan-pattern-effect-cues";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr size_t MAX_CUE_COUNT = 512u;
	constexpr f32_t MAX_POSITION_MAGNITUDE = 100000.f;
	constexpr f32_t MAX_ROTATION_MAGNITUDE = 360000.f;
	constexpr f32_t MAX_SCALE = 1000.f;

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<const char_t*> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(),
			[&Value](const char_t* pKey)
			{
				return nullptr != Value.Find(pKey);
			});
	}

	bool_t Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 160u &&
			std::all_of(Value.begin(), Value.end(),
				[](const char_t Character)
				{
					const unsigned char Value =
						static_cast<unsigned char>(Character);
					return 0 != std::isalnum(Value) || Character == '_' ||
						Character == '-' || Character == '.';
				});
	}

	bool_t Read_String(const DATA_JSON_VALUE& Parent,
		const char_t* pKey, std::string& strOutValue)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find(pKey);
		if (nullptr == pValue || !pValue->Is_String() ||
			!Is_StableId(pValue->Get_String()))
		{
			return false;
		}
		strOutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_Unsigned(const DATA_JSON_VALUE& Parent,
		const char_t* pKey, const uint32_t iMaximum, uint32_t& iOutValue)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find(pKey);
		if (nullptr == pValue || !pValue->Is_Number())
			return false;
		const double Number = pValue->Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>(iMaximum) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		iOutValue = static_cast<uint32_t>(Number);
		return true;
	}

	bool_t Read_Float3(const DATA_JSON_VALUE& Parent,
		const char_t* pKey, const f32_t fMaximumMagnitude,
		float3_t& vOutValue)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find(pKey);
		if (nullptr == pValue || !pValue->Is_Array() ||
			3u != pValue->Get_Array().size())
		{
			return false;
		}
		f32_t Components[3]{};
		for (size_t iComponent = 0u; iComponent < 3u; ++iComponent)
		{
			const DATA_JSON_VALUE& Component =
				pValue->Get_Array()[iComponent];
			if (!Component.Is_Number() ||
				!std::isfinite(Component.Get_Number()) ||
				std::abs(Component.Get_Number()) > fMaximumMagnitude)
			{
				return false;
			}
			Components[iComponent] = static_cast<f32_t>(
				Component.Get_Number());
		}
		vOutValue = { Components[0], Components[1], Components[2] };
		return true;
	}

	bool_t Read_Transform(const DATA_JSON_VALUE& Parent,
		EFFECT_TRANSFORM_DESC& OutTransform)
	{
		const DATA_JSON_VALUE* pTransform = Parent.Find("localTransform");
		if (nullptr == pTransform || !Is_ExactObject(*pTransform,
				{ "position", "rotationDegrees", "scale" }) ||
			!Read_Float3(*pTransform, "position", MAX_POSITION_MAGNITUDE,
				OutTransform.vPosition) ||
			!Read_Float3(*pTransform, "rotationDegrees", MAX_ROTATION_MAGNITUDE,
				OutTransform.vRotationDegrees) ||
			!Read_Float3(*pTransform, "scale", MAX_SCALE,
				OutTransform.vScale))
		{
			return false;
		}
		return OutTransform.vScale.x > 0.f &&
			OutTransform.vScale.y > 0.f && OutTransform.vScale.z > 0.f;
	}

	bool_t Read_FollowPolicy(const DATA_JSON_VALUE& Parent,
		EFFECT_FOLLOW_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("followPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("follow" == pValue->Get_String())
			eOutPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
		else if ("snapshot" == pValue->Get_String())
			eOutPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
		else
			return false;
		return true;
	}

	bool_t Read_StopPolicy(const DATA_JSON_VALUE& Parent,
		EFFECT_STOP_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("stopPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("natural" == pValue->Get_String())
			eOutPolicy = EFFECT_STOP_POLICY::NATURAL;
		else if ("cue_end" == pValue->Get_String())
			eOutPolicy = EFFECT_STOP_POLICY::CUE_END;
		else
			return false;
		return true;
	}

	bool_t Load_EncounterReference(CEncounterPatternReference& OutEncounter,
		std::string& strOutStatus)
	{
		return OutEncounter.Load(CProjectDataRoot::Resolve(
			std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json"), strOutStatus);
	}
}

std::filesystem::path
Client::CValtanPatternEffectCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patterneffectcues.json");
}

bool_t Client::CValtanPatternEffectCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	if (!Encounter.Is_Ready() ||
		Encounter.Get_BossArchetypeId() != OWNER_ARCHETYPE_ID)
	{
		strOutStatus =
			"Valtan pattern Effect cues require the validated Valtan encounter.";
		return false;
	}

	DATA_JSON_VALUE Root;
	std::string ParseError;
	DATA_JSON_PARSE_LIMITS Limits{};
	Limits.iMaximumBytes = 1024u * 1024u;
	Limits.iMaximumDepth = 16u;
	Limits.iMaximumValues = 16384u;
	if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
		!Is_ExactObject(Root,
			{ "schema", "formatVersion", "ownerArchetypeId", "cues" }))
	{
		strOutStatus = "Valtan pattern Effect cue JSON is malformed: " +
			ParseError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pOwner = Root.Find("ownerArchetypeId");
	const DATA_JSON_VALUE* pCues = Root.Find("cues");
	uint32_t iFormatVersion = 0u;
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != SCHEMA ||
		!Read_Unsigned(Root, "formatVersion", FORMAT_VERSION,
			iFormatVersion) || iFormatVersion != FORMAT_VERSION ||
		nullptr == pOwner || !pOwner->Is_String() ||
		pOwner->Get_String() != OWNER_ARCHETYPE_ID ||
		pOwner->Get_String() != Encounter.Get_BossArchetypeId() ||
		nullptr == pCues || !pCues->Is_Array() ||
		pCues->Get_Array().empty() ||
		pCues->Get_Array().size() > MAX_CUE_COUNT)
	{
		strOutStatus = "Valtan pattern Effect cue header is invalid.";
		return false;
	}

	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged;
	Staged.iFormatVersion = iFormatVersion;
	Staged.strOwnerArchetypeId = pOwner->Get_String();
	Staged.Cues.reserve(pCues->Get_Array().size());
	std::unordered_set<std::string> BindingIds;
	std::unordered_set<std::string> EncounterTuples;
	std::unordered_set<std::string> EffectAssetIds;
	for (const DATA_JSON_VALUE& CueValue : pCues->Get_Array())
	{
		if (!Is_ExactObject(CueValue,
				{ "bindingId", "patternId", "stageId", "actionId",
				  "effectAssetId", "anchorSlotId", "followPolicy",
				  "stopPolicy", "startMs", "endMs", "localTransform" }))
		{
			strOutStatus =
				"Valtan pattern Effect cue has unexpected properties.";
			return false;
		}

		VALTAN_PATTERN_EFFECT_CUE Cue;
		if (!Read_String(CueValue, "bindingId", Cue.strBindingId) ||
			!Read_String(CueValue, "patternId", Cue.strPatternId) ||
			!Read_String(CueValue, "stageId", Cue.strStageId) ||
			!Read_String(CueValue, "actionId", Cue.strActionId) ||
			!Read_String(CueValue, "effectAssetId", Cue.strEffectAssetId) ||
			!Read_String(CueValue, "anchorSlotId", Cue.strAnchorSlotId) ||
			!Read_FollowPolicy(CueValue, Cue.eFollowPolicy) ||
			!Read_StopPolicy(CueValue, Cue.eStopPolicy) ||
			!Read_Unsigned(CueValue, "startMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iStartMs) || !Read_Transform(CueValue, Cue.LocalTransform) ||
			!BindingIds.insert(Cue.strBindingId).second ||
			!EffectAssetIds.insert(Cue.strEffectAssetId).second)
		{
			strOutStatus =
				"Valtan pattern Effect cue identity, policy, or transform is invalid.";
			return false;
		}

		const ENCOUNTER_PATTERN_REFERENCE* pPattern =
			Encounter.Find_Pattern(Cue.strPatternId);
		if (nullptr == pPattern)
		{
			strOutStatus = "Valtan pattern Effect cue pattern is unknown: " +
				Cue.strPatternId;
			return false;
		}
		const auto Stage = std::find_if(pPattern->stages.begin(),
			pPattern->stages.end(), [&Cue](const ENCOUNTER_STAGE_REFERENCE& Value)
			{
				return Value.stageId == Cue.strStageId;
			});
		if (pPattern->stages.end() == Stage ||
			Stage->actionId != Cue.strActionId ||
			0u == Stage->iDurationMs || Cue.iStartMs >= Stage->iDurationMs)
		{
			strOutStatus = "Valtan pattern Effect cue encounter tuple is invalid: " +
				Cue.strBindingId;
			return false;
		}
		Cue.iStageIndex = static_cast<uint32_t>(
			Stage - pPattern->stages.begin());
		Cue.iStageDurationMs = Stage->iDurationMs;

		const DATA_JSON_VALUE* pEndMs = CueValue.Find("endMs");
		if (EFFECT_STOP_POLICY::NATURAL == Cue.eStopPolicy)
		{
			if (nullptr == pEndMs || !pEndMs->Is_Null())
			{
				strOutStatus =
					"Natural Valtan pattern Effect cue requires null endMs.";
				return false;
			}
			Cue.iEndMs = Cue.iStartMs;
		}
		else if (nullptr == pEndMs || !Read_Unsigned(CueValue, "endMs",
				Stage->iDurationMs, Cue.iEndMs) ||
			Cue.iEndMs <= Cue.iStartMs)
		{
			strOutStatus =
				"cue_end Valtan pattern Effect cue has an invalid endMs.";
			return false;
		}

		const std::string Tuple = Cue.strPatternId + "\n" + Cue.strStageId +
			"\n" + Cue.strActionId;
		if (!EncounterTuples.insert(Tuple).second)
		{
			strOutStatus = "Duplicate Valtan pattern Effect encounter tuple.";
			return false;
		}
		Staged.Cues.push_back(std::move(Cue));
	}

	std::sort(Staged.Cues.begin(), Staged.Cues.end(),
		[](const VALTAN_PATTERN_EFFECT_CUE& Left,
			const VALTAN_PATTERN_EFFECT_CUE& Right)
		{
			return std::tie(Left.strActionId, Left.iStartMs,
				Left.strBindingId) <
				std::tie(Right.strActionId, Right.iStartMs,
					Right.strBindingId);
		});
	InOutDocument = std::move(Staged);
	strOutStatus = "Parsed " + std::to_string(InOutDocument.Cues.size()) +
		" action-qualified Valtan pattern Effect cue(s).";
	return true;
}

bool_t Client::CValtanPatternEffectCueDocument::Load_Source(
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	CEncounterPatternReference Encounter;
	if (!Load_EncounterReference(Encounter, strOutStatus))
		return false;
	const std::filesystem::path Path = Resolve_Path();
	std::ifstream Input(Path, std::ios::binary);
	if (Path.empty() || !Input)
	{
		strOutStatus =
			"Missing Valtan pattern Effect cue document: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	return Parse_Text(Text, Encounter, InOutDocument, strOutStatus);
}

bool_t Client::CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged;
	if (!Load_Source(Staged, strOutStatus))
		return false;
	for (const VALTAN_PATTERN_EFFECT_CUE& Cue : Staged.Cues)
	{
		if (!CEffectCatalog::Contains(Cue.strEffectAssetId))
		{
			strOutStatus =
				"Valtan pattern Effect cue target is absent from the runtime catalog: " +
				Cue.strEffectAssetId;
			return false;
		}
		if (!CEffectCatalog::Admit_ProductSpawn(
				Cue.strEffectAssetId, nullptr, strOutStatus))
		{
			return false;
		}
	}
	InOutDocument = std::move(Staged);
	strOutStatus = "Loaded " + std::to_string(InOutDocument.Cues.size()) +
		" runtime-admitted Valtan pattern Effect cue(s).";
	return true;
}
