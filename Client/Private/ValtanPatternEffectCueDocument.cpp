#include "ValtanPatternEffectCueDocument.h"

#include "AnimationSkillBindingDocument.h"
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
#include <unordered_map>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr std::string_view SCHEMA =
		"lostark.valtan-pattern-effect-cues";
	constexpr std::string_view V1_ALIAS_SCHEMA =
		"lostark.valtan-pattern-effect-v1-aliases";
	constexpr uint32_t V1_ALIAS_FORMAT_VERSION = 1u;
	constexpr uint32_t V2_FORMAT_VERSION = 2u;
	constexpr uint32_t V3_FORMAT_VERSION = 3u;
	constexpr uint32_t FORMAT_VERSION = 4u;
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr size_t MAX_CUE_COUNT = 512u;
	constexpr f32_t MAX_POSITION_MAGNITUDE = 100000.f;
	constexpr f32_t MAX_ROTATION_MAGNITUDE = 360000.f;
	constexpr f32_t MAX_SCALE = 1000.f;
	constexpr std::string_view PATTERN_TARGET_SNAPSHOT_ANCHOR =
		"pattern.target.snapshot";

	bool_t Is_LockedPatternTargetPolicy(const std::string_view Policy)
	{
		return Policy == "LOCK_NEAREST_ON_START" ||
			Policy == "LOCK_RANDOM_ALIVE_ON_START" ||
			Policy == "LOCK_RANDOM_ALIVE_BEHIND_ON_START";
	}

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

	bool_t Load_AnimationBindings(
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& OutBindings,
		std::string& strOutStatus)
	{
		const std::filesystem::path Path =
			CValtanPatternAnimationBindingDocument::Resolve_Path("Valtan");
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus =
				"Missing Valtan pattern animation binding document: " +
				Path.string();
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT Staged;
		if (!CValtanPatternAnimationBindingDocument::Parse_Text(
				Text, Staged, strOutStatus))
		{
			return false;
		}
		std::vector<std::string> DeclaredClips;
		for (const BOSS_PATTERN_ANIMATION_BINDING& Binding : Staged.Bindings)
		{
			for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
				DeclaredClips.push_back(Clip.strClipName);
		}
		if (!CValtanPatternAnimationBindingDocument::Validate(
				Staged, OWNER_ARCHETYPE_ID, DeclaredClips, strOutStatus))
		{
			return false;
		}
		OutBindings = std::move(Staged);
		return true;
	}

	bool_t Apply_OptionalV1Aliases(
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus)
	{
		const std::filesystem::path Path =
			CValtanPatternEffectCueDocument::Resolve_V1AliasPath();
		std::error_code FileSystemError;
		const bool_t bExists = std::filesystem::exists(
			Path, FileSystemError);
		if (FileSystemError)
		{
			strOutStatus =
				"Could not inspect the optional Valtan Material V1 alias document: " +
				FileSystemError.message() + "; V0 cues remain authoritative.";
			return true;
		}
		if (!bExists)
		{
			strOutStatus =
				"Optional Valtan Material V1 alias document is absent; V0 cues remain authoritative.";
			return true;
		}

		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus =
				"Could not open the Valtan Material V1 alias document: " +
				Path.string() + "; V0 cues remain authoritative.";
			return true;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE Root;
		std::string ParseError;
		DATA_JSON_PARSE_LIMITS Limits{};
		Limits.iMaximumBytes = 256u * 1024u;
		Limits.iMaximumDepth = 8u;
		Limits.iMaximumValues = 4096u;
		if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
			!Is_ExactObject(Root,
				{ "schema", "formatVersion", "ownerArchetypeId", "aliases" }))
		{
			strOutStatus =
				"Valtan Material V1 alias JSON is malformed and was omitted: " +
				ParseError + "; V0 cues remain authoritative.";
			return true;
		}

		const DATA_JSON_VALUE* pSchema = Root.Find("schema");
		const DATA_JSON_VALUE* pOwner = Root.Find("ownerArchetypeId");
		const DATA_JSON_VALUE* pAliases = Root.Find("aliases");
		uint32_t iFormatVersion = 0u;
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != V1_ALIAS_SCHEMA ||
			!Read_Unsigned(Root, "formatVersion", V1_ALIAS_FORMAT_VERSION,
				iFormatVersion) || iFormatVersion != V1_ALIAS_FORMAT_VERSION ||
			nullptr == pOwner || !pOwner->Is_String() ||
			pOwner->Get_String() != OWNER_ARCHETYPE_ID ||
			pOwner->Get_String() != InOutDocument.strOwnerArchetypeId ||
			nullptr == pAliases || !pAliases->Is_Array() ||
			pAliases->Get_Array().empty() ||
			pAliases->Get_Array().size() > MAX_CUE_COUNT)
		{
			strOutStatus =
				"Valtan Material V1 alias header is invalid and was omitted; V0 cues remain authoritative.";
			return true;
		}

		std::unordered_set<std::string> CueEffectAssetIds;
		for (const VALTAN_PATTERN_EFFECT_CUE& Cue : InOutDocument.Cues)
			CueEffectAssetIds.insert(Cue.strEffectAssetId);
		std::unordered_map<std::string, std::string> Aliases;
		std::unordered_set<std::string> V1EffectAssetIds;
		size_t iOmittedAliasCount = 0u;
		for (const DATA_JSON_VALUE& AliasValue : pAliases->Get_Array())
		{
			if (!Is_ExactObject(AliasValue,
					{ "effectAssetId", "v1EffectAssetId" }))
			{
				++iOmittedAliasCount;
				continue;
			}
			std::string EffectAssetId;
			std::string V1EffectAssetId;
			if (!Read_String(AliasValue, "effectAssetId", EffectAssetId) ||
				!Read_String(AliasValue, "v1EffectAssetId", V1EffectAssetId) ||
				EffectAssetId == V1EffectAssetId ||
				!V1EffectAssetId.ends_with(".v1.unified") ||
				!CueEffectAssetIds.contains(EffectAssetId) ||
				Aliases.contains(EffectAssetId) ||
				V1EffectAssetIds.contains(V1EffectAssetId))
			{
				++iOmittedAliasCount;
				continue;
			}
			Aliases.emplace(EffectAssetId, V1EffectAssetId);
			V1EffectAssetIds.insert(V1EffectAssetId);
		}

		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged = InOutDocument;
		for (VALTAN_PATTERN_EFFECT_CUE& Cue : Staged.Cues)
		{
			const auto Found = Aliases.find(Cue.strEffectAssetId);
			if (Aliases.end() != Found)
				Cue.strV1EffectAssetId = Found->second;
		}
		InOutDocument = std::move(Staged);
		strOutStatus = "Applied " + std::to_string(Aliases.size()) +
			" optional Valtan Material V1 Effect alias(es); omitted " +
			std::to_string(iOmittedAliasCount) +
			" invalid alias row(s). V0 cues remain authoritative.";
		return true;
	}

	bool_t Read_RepeatPolicy(const DATA_JSON_VALUE& Parent,
		VALTAN_PATTERN_EFFECT_REPEAT_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("repeatPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("once" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE;
		else if ("each_loop" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP;
		else
			return false;
		return true;
	}

	bool_t Read_ScalePolicy(
		const DATA_JSON_VALUE& Parent,
		VALTAN_PATTERN_EFFECT_CUE& OutCue)
	{
		const DATA_JSON_VALUE* pPolicy = Parent.Find("scalePolicy");
		if (nullptr == pPolicy)
			return true;
		if (!pPolicy->Is_Object())
			return false;
		const DATA_JSON_VALUE* pKind = pPolicy->Find("kind");
		if (nullptr == pKind || !pKind->Is_String())
			return false;

		if ("OWNER_RELATIVE" == pKind->Get_String())
		{
			if (!Is_ExactObject(*pPolicy, { "kind" }))
				return false;
			OutCue.eScalePolicy =
				VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
			OutCue.vWorldScale = { 1.f, 1.f, 1.f };
		}
		else if ("GAMEPLAY_FOOTPRINT" == pKind->Get_String() ||
			"ARENA_ABSOLUTE" == pKind->Get_String())
		{
			if (!Is_ExactObject(*pPolicy, { "kind", "worldScale" }) ||
				!Read_Float3(*pPolicy, "worldScale", MAX_SCALE,
					OutCue.vWorldScale) ||
				OutCue.vWorldScale.x <= 0.f ||
				OutCue.vWorldScale.y <= 0.f ||
				OutCue.vWorldScale.z <= 0.f)
			{
				return false;
			}
			OutCue.eScalePolicy =
				"GAMEPLAY_FOOTPRINT" == pKind->Get_String() ?
				VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT :
				VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE;
		}
		else
		{
			return false;
		}
		OutCue.bHasExplicitScalePolicy = true;
		return true;
	}

	bool_t Is_ManagedPatternId(const std::string_view PatternId)
	{
		return PatternId == "VALTAN_WHIRLWIND" ||
			PatternId == "VALTAN_DASH_CHARGE" ||
			PatternId == "VALTAN_FOUR_SLASH" ||
			PatternId == "VALTAN_FIST_IN_OUT" ||
			PatternId == "VALTAN_HIGH_JUMP" ||
			PatternId == "VALTAN_FLOOR_WIPE_130" ||
			PatternId == "VALTAN_ARENA_BREAK_109";
	}

	const BOSS_PATTERN_ANIMATION_BINDING* Find_ActionBinding(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& Document,
		const std::string_view ActionId)
	{
		const auto Found = std::find_if(Document.Bindings.begin(),
			Document.Bindings.end(),
			[ActionId](const BOSS_PATTERN_ANIMATION_BINDING& Binding)
			{
				return Binding.strActionId == ActionId;
			});
		return Document.Bindings.end() == Found ? nullptr : &*Found;
	}

	const BOSS_PATTERN_ANIMATION_CLIP* Find_ClipOccurrence(
		const BOSS_PATTERN_ANIMATION_BINDING& Binding,
		const std::string_view ClipOccurrenceId)
	{
		const auto Found = std::find_if(Binding.Clips.begin(),
			Binding.Clips.end(),
			[ClipOccurrenceId](const BOSS_PATTERN_ANIMATION_CLIP& Clip)
			{
				return Clip.strClipOccurrenceId == ClipOccurrenceId;
			});
		return Binding.Clips.end() == Found ? nullptr : &*Found;
	}
}

std::filesystem::path
Client::CValtanPatternEffectCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patterneffectcues.json");
}

std::filesystem::path
Client::CValtanPatternEffectCueDocument::Resolve_V1AliasPath()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patterneffectv1aliases.json");
}

bool_t Client::CValtanPatternEffectCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	return Parse_Text(Text, Encounter, AnimationBindings,
		InOutDocument, strOutStatus);
}

bool_t Client::CValtanPatternEffectCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	if (!Encounter.Is_Ready() ||
		Encounter.Get_BossArchetypeId() != OWNER_ARCHETYPE_ID ||
		AnimationBindings.strBossArchetypeId != OWNER_ARCHETYPE_ID)
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
			iFormatVersion) ||
		(iFormatVersion != 1u && iFormatVersion != V2_FORMAT_VERSION &&
		 iFormatVersion != V3_FORMAT_VERSION &&
		 iFormatVersion != FORMAT_VERSION) ||
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
	std::vector<std::string> DeclaredClips;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		AnimationBindings.Bindings)
	{
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			DeclaredClips.push_back(Clip.strClipName);
	}
	std::string AnimationStatus;
	if (!CValtanPatternAnimationBindingDocument::Validate(
			AnimationBindings, OWNER_ARCHETYPE_ID, DeclaredClips,
			AnimationStatus))
	{
		strOutStatus =
			"Valtan pattern Effect cue animation join is invalid: " +
			AnimationStatus;
		return false;
	}

	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged;
	Staged.iFormatVersion = iFormatVersion;
	Staged.strOwnerArchetypeId = pOwner->Get_String();
	Staged.Cues.reserve(pCues->Get_Array().size());
	std::unordered_set<std::string> BindingIds;
	std::unordered_set<std::string> OccurrenceIds;
	std::unordered_set<std::string> EncounterTuples;
	std::unordered_set<std::string> EffectAssetIds;
	std::unordered_set<std::string> ActionClipOccurrenceTuples;
	for (const DATA_JSON_VALUE& CueValue : pCues->Get_Array())
	{
		const bool_t bLegacy = 1u == iFormatVersion;
		const bool_t bStageClock =
			nullptr != CueValue.Find("timingBasis");
		const bool_t bUsesScalePolicySchema =
			iFormatVersion == V3_FORMAT_VERSION ||
			iFormatVersion == FORMAT_VERSION;
		const bool_t bHasScalePolicy =
			nullptr != CueValue.Find("scalePolicy");
		if ((bStageClock && FORMAT_VERSION != iFormatVersion) ||
			(bLegacy && !Is_ExactObject(CueValue,
				{ "bindingId", "patternId", "stageId", "actionId",
				  "effectAssetId", "anchorSlotId", "followPolicy",
				  "stopPolicy", "startMs", "endMs", "localTransform" })) ||
			(V2_FORMAT_VERSION == iFormatVersion && !Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "effectAssetId",
				  "anchorSlotId", "followPolicy", "stopPolicy", "repeatPolicy",
				  "sourceStartMs", "sourceEndMs", "localTransform" })) ||
			(bUsesScalePolicySchema && !bStageClock &&
				((!bHasScalePolicy && !Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "effectAssetId",
				  "anchorSlotId", "followPolicy", "stopPolicy", "repeatPolicy",
				  "sourceStartMs", "sourceEndMs", "localTransform" })) ||
				 (bHasScalePolicy && !Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "effectAssetId",
				  "anchorSlotId", "followPolicy", "stopPolicy", "repeatPolicy",
				  "sourceStartMs", "sourceEndMs", "localTransform",
				  "scalePolicy" })))) ||
			(FORMAT_VERSION == iFormatVersion && bStageClock &&
				((!bHasScalePolicy && !Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "timingBasis", "stageOffsetMs",
				  "effectAssetId", "anchorSlotId", "followPolicy",
				  "stopPolicy", "repeatPolicy", "localTransform" })) ||
				 (bHasScalePolicy && !Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "timingBasis", "stageOffsetMs",
				  "effectAssetId", "anchorSlotId", "followPolicy",
				  "stopPolicy", "repeatPolicy", "localTransform",
				  "scalePolicy" })))))
		{
			strOutStatus =
				"Valtan pattern Effect cue has unexpected properties.";
			return false;
		}

		VALTAN_PATTERN_EFFECT_CUE Cue;
		const DATA_JSON_VALUE* pTimingBasis =
			CueValue.Find("timingBasis");
		if (!Read_String(CueValue, "bindingId", Cue.strBindingId) ||
			(!bLegacy &&
			 !Read_String(CueValue, "occurrenceId", Cue.strOccurrenceId)) ||
			!Read_String(CueValue, "patternId", Cue.strPatternId) ||
			!Read_String(CueValue, "stageId", Cue.strStageId) ||
			!Read_String(CueValue, "actionId", Cue.strActionId) ||
			(!bLegacy && !bStageClock &&
			 !Read_String(CueValue, "clipOccurrenceId",
				Cue.strClipOccurrenceId)) ||
			(bStageClock &&
			 (nullptr == pTimingBasis || !pTimingBasis->Is_String() ||
			  "STAGE_CLOCK" != pTimingBasis->Get_String())) ||
			!Read_String(CueValue, "effectAssetId", Cue.strEffectAssetId) ||
			!Read_String(CueValue, "anchorSlotId", Cue.strAnchorSlotId) ||
			!Read_FollowPolicy(CueValue, Cue.eFollowPolicy) ||
			!Read_StopPolicy(CueValue, Cue.eStopPolicy) ||
			(!bLegacy && !Read_RepeatPolicy(CueValue,
				Cue.eRepeatPolicy)) ||
			(bUsesScalePolicySchema &&
			 !Read_ScalePolicy(CueValue, Cue)) ||
			!Read_Unsigned(CueValue,
				bLegacy ? "startMs" :
					(bStageClock ? "stageOffsetMs" : "sourceStartMs"),
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iStartMs) || !Read_Transform(CueValue, Cue.LocalTransform) ||
			!BindingIds.insert(Cue.strBindingId).second)
		{
			strOutStatus =
				"Valtan pattern Effect cue identity, policy, or transform is invalid.";
			return false;
		}
		if (bLegacy)
			Cue.strOccurrenceId = Cue.strBindingId;
		Cue.bUsesStageClock = bStageClock;
		if (bStageClock &&
			(EFFECT_STOP_POLICY::NATURAL != Cue.eStopPolicy ||
			 VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE != Cue.eRepeatPolicy))
		{
			strOutStatus =
				"STAGE_CLOCK Valtan Effect cue requires natural/once policies.";
			return false;
		}
		if (bUsesScalePolicySchema &&
			Is_ManagedPatternId(Cue.strPatternId) &&
			!Cue.bHasExplicitScalePolicy)
		{
			strOutStatus =
				"Managed Valtan pattern Effect cue requires explicit scalePolicy: " +
				Cue.strBindingId;
			return false;
		}
		if (!OccurrenceIds.insert(Cue.strOccurrenceId).second)
		{
			strOutStatus =
				"Duplicate Valtan pattern Effect cue occurrence identity.";
			return false;
		}

		const BOSS_PATTERN_ANIMATION_BINDING* pAnimationBinding =
			Find_ActionBinding(AnimationBindings, Cue.strActionId);
		if (nullptr == pAnimationBinding ||
			(bLegacy && 1u != pAnimationBinding->Clips.size()) ||
			(bStageClock &&
			 (!pAnimationBinding->bSuppressAnimation ||
			  !pAnimationBinding->Clips.empty())) ||
			(!bStageClock && pAnimationBinding->bSuppressAnimation))
		{
			strOutStatus = bStageClock ?
				"STAGE_CLOCK Valtan Effect cue requires an explicit NONE animation binding: " +
					Cue.strActionId : bLegacy ?
				"Valtan pattern Effect cue v1 cannot migrate an ordered multi-clip action: " +
					Cue.strActionId :
				"Valtan pattern Effect cue action has no animation binding: " +
					Cue.strActionId;
			return false;
		}
		if (bLegacy)
		{
			Cue.strClipOccurrenceId =
				pAnimationBinding->Clips.front().strClipOccurrenceId;
			Cue.bUsesLegacyStageWallTime = true;
		}
		const BOSS_PATTERN_ANIMATION_CLIP* pAnimationClip = bStageClock ?
			nullptr : Find_ClipOccurrence(*pAnimationBinding,
				Cue.strClipOccurrenceId);
		if (!bStageClock && nullptr == pAnimationClip)
		{
			strOutStatus =
				"Valtan pattern Effect cue clip occurrence is not owned by its action: " +
				Cue.strOccurrenceId;
			return false;
		}
		if (!bLegacy && !bStageClock &&
			VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !pAnimationClip->bLoop)
		{
			strOutStatus =
				"Valtan each_loop Effect cue references a non-loop clip: " +
				Cue.strOccurrenceId;
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
		if (Cue.strAnchorSlotId.starts_with("pattern.target.") &&
			Cue.strAnchorSlotId != PATTERN_TARGET_SNAPSHOT_ANCHOR)
		{
			strOutStatus =
				"Valtan pattern Effect cue uses an unknown reserved target anchor: " +
				Cue.strBindingId;
			return false;
		}
		if (Cue.strAnchorSlotId == PATTERN_TARGET_SNAPSHOT_ANCHOR &&
			(Cue.eFollowPolicy != EFFECT_FOLLOW_POLICY::SNAPSHOT ||
			 !Is_LockedPatternTargetPolicy(pPattern->targetPolicy)))
		{
			strOutStatus =
				"Valtan pattern target Effect cue requires snapshot follow and a locked Server target: " +
				Cue.strBindingId;
			return false;
		}
		const auto Stage = std::find_if(pPattern->stages.begin(),
			pPattern->stages.end(), [&Cue](const ENCOUNTER_STAGE_REFERENCE& Value)
			{
				return Value.stageId == Cue.strStageId;
			});
		if (pPattern->stages.end() == Stage ||
			Stage->actionId != Cue.strActionId ||
			0u == Stage->iDurationMs ||
			((bLegacy || bStageClock) &&
			 Cue.iStartMs >= Stage->iDurationMs))
		{
			strOutStatus = "Valtan pattern Effect cue encounter tuple is invalid: " +
				Cue.strBindingId;
			return false;
		}
		Cue.iStageIndex = static_cast<uint32_t>(
			Stage - pPattern->stages.begin());
		Cue.iStageDurationMs = Stage->iDurationMs;

		const char_t* pEndKey = bLegacy ? "endMs" : "sourceEndMs";
		const DATA_JSON_VALUE* pEndMs = bStageClock ?
			nullptr : CueValue.Find(pEndKey);
		if (bStageClock)
		{
			Cue.iEndMs = Cue.iStartMs;
			Cue.bHasSourceEnd = false;
		}
		else if (EFFECT_STOP_POLICY::NATURAL == Cue.eStopPolicy)
		{
			if (nullptr == pEndMs || !pEndMs->Is_Null())
			{
				strOutStatus =
					"Natural Valtan pattern Effect cue requires a null source end.";
				return false;
			}
			Cue.iEndMs = Cue.iStartMs;
			Cue.bHasSourceEnd = false;
		}
		else if (nullptr == pEndMs || !Read_Unsigned(CueValue, pEndKey,
				bLegacy ? Stage->iDurationMs :
					CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iEndMs) ||
			Cue.iEndMs <= Cue.iStartMs)
		{
			strOutStatus =
				"cue_end Valtan pattern Effect cue has an invalid source end.";
			return false;
		}
		else
		{
			Cue.bHasSourceEnd = true;
		}

		if (!bLegacy && !bStageClock)
		{
			const uint64_t iSegmentEndMs =
				static_cast<uint64_t>(pAnimationClip->iSourceStartMs) +
				static_cast<uint64_t>(pAnimationClip->iPlayMs);
			if (Cue.iStartMs < pAnimationClip->iSourceStartMs ||
				(0u != pAnimationClip->iPlayMs &&
				 (Cue.iStartMs >= iSegmentEndMs ||
				  (Cue.bHasSourceEnd && Cue.iEndMs > iSegmentEndMs))))
			{
				strOutStatus =
					"Valtan pattern Effect cue source window is outside its clip segment: " +
					Cue.strOccurrenceId;
				return false;
			}
			const std::string Tuple = Cue.strActionId + "\n" +
				Cue.strClipOccurrenceId + "\n" + Cue.strOccurrenceId;
			if (!ActionClipOccurrenceTuples.insert(Tuple).second)
			{
				strOutStatus =
					"Duplicate Valtan action/clip/cue occurrence tuple.";
				return false;
			}
		}
		else if (bLegacy)
		{
			const std::string Tuple = Cue.strPatternId + "\n" +
				Cue.strStageId + "\n" + Cue.strActionId;
			if (!EncounterTuples.insert(Tuple).second ||
				!EffectAssetIds.insert(Cue.strEffectAssetId).second)
			{
				strOutStatus =
					"Duplicate Valtan pattern Effect v1 encounter or target identity.";
				return false;
			}
		}
		else
		{
			const std::string Tuple = Cue.strActionId + "\nSTAGE_CLOCK\n" +
				Cue.strOccurrenceId;
			if (!ActionClipOccurrenceTuples.insert(Tuple).second)
			{
				strOutStatus =
					"Duplicate Valtan action/stage-clock/cue occurrence tuple.";
				return false;
			}
		}
		Staged.Cues.push_back(std::move(Cue));
	}

	std::sort(Staged.Cues.begin(), Staged.Cues.end(),
		[](const VALTAN_PATTERN_EFFECT_CUE& Left,
			const VALTAN_PATTERN_EFFECT_CUE& Right)
		{
			return std::tie(Left.strActionId,
				Left.strClipOccurrenceId, Left.iStartMs,
				Left.strOccurrenceId) <
				std::tie(Right.strActionId,
					Right.strClipOccurrenceId, Right.iStartMs,
					Right.strOccurrenceId);
		});
	InOutDocument = std::move(Staged);
	strOutStatus = "Parsed " + std::to_string(InOutDocument.Cues.size()) +
		" presentation-qualified Valtan pattern Effect cue(s).";
	return true;
}

bool_t Client::CValtanPatternEffectCueDocument::Load_ReadOnlyProduct(
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
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged;
	if (!Parse_Text(Text, Encounter, AnimationBindings,
			Staged, strOutStatus))
	{
		return false;
	}
#ifdef _DEBUG
	std::string V1AliasStatus;
	Apply_OptionalV1Aliases(Staged, V1AliasStatus);
	if (!V1AliasStatus.empty())
	{
		OutputDebugStringA((
			"[Client][Valtan] optional Material V1 alias stage: " +
			V1AliasStatus + "\n").c_str());
	}
#endif
	InOutDocument = std::move(Staged);
	return true;
}

bool_t Client::CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Staged;
	if (!Load_ReadOnlyProduct(Staged, strOutStatus))
		return false;
	size_t iOmittedV1AliasCount = 0u;
	for (VALTAN_PATTERN_EFFECT_CUE& Cue : Staged.Cues)
	{
		if (!CEffectCatalog::Contains(Cue.strEffectAssetId))
		{
			strOutStatus =
				"Valtan pattern Effect cue target is absent from the runtime catalog: " +
				Cue.strEffectAssetId;
			return false;
		}
		if (!Cue.strV1EffectAssetId.empty() &&
			!CEffectCatalog::Contains(Cue.strV1EffectAssetId))
		{
			OutputDebugStringA((
				"[Client][Valtan] optional Material V1 alias omitted because its target is absent from the runtime catalog: " +
				Cue.strV1EffectAssetId + "; V0 remains authoritative.\n").c_str());
			Cue.strV1EffectAssetId.clear();
			++iOmittedV1AliasCount;
		}
	}
	InOutDocument = std::move(Staged);
	const size_t iV1AliasCount = static_cast<size_t>(std::count_if(
		InOutDocument.Cues.begin(), InOutDocument.Cues.end(),
		[](const VALTAN_PATTERN_EFFECT_CUE& Cue)
		{
			return !Cue.strV1EffectAssetId.empty();
		}));
	strOutStatus = "Loaded " + std::to_string(InOutDocument.Cues.size()) +
		" runtime-admitted Valtan pattern Effect cue(s) with " +
		std::to_string(iV1AliasCount) + " Material V1 alias(es); omitted " +
		std::to_string(iOmittedV1AliasCount) +
		" unavailable optional alias target(s).";
	return true;
}
