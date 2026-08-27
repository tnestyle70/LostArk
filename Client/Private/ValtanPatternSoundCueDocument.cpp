#include "ValtanPatternSoundCueDocument.h"

#include "AnimationSkillBindingDocument.h"
#include "DataJson.h"
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

	constexpr std::string_view SCHEMA = "lostark.valtan-pattern-sound-cues";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
	/* Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json real cue count grew past the
	old 512 cap (519 as of the animation-tool-presentation-parity content), which fail-closed the
	entire document -- every Valtan pattern sound cue silently stopped playing at once instead of
	just the cues past the limit. Raised with real headroom instead of matching the count exactly. */
	constexpr size_t MAX_CUE_COUNT = 1024u;

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

	bool_t Read_RepeatPolicy(const DATA_JSON_VALUE& Parent,
		VALTAN_PATTERN_SOUND_REPEAT_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("repeatPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("once" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		else if ("each_loop" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
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
Client::CValtanPatternSoundCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternsoundcues.json");
}

bool_t Client::CValtanPatternSoundCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	if (!Encounter.Is_Ready() ||
		Encounter.Get_BossArchetypeId() != OWNER_ARCHETYPE_ID ||
		AnimationBindings.strBossArchetypeId != OWNER_ARCHETYPE_ID)
	{
		strOutStatus =
			"Valtan pattern Sound cues require the validated Valtan encounter.";
		return false;
	}

	DATA_JSON_VALUE Root;
	std::string ParseError;
	DATA_JSON_PARSE_LIMITS Limits{};
	Limits.iMaximumBytes = 512u * 1024u;
	Limits.iMaximumDepth = 8u;
	Limits.iMaximumValues = 8192u;
	if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
		!Is_ExactObject(Root,
			{ "schema", "formatVersion", "ownerArchetypeId", "cues" }))
	{
		strOutStatus = "Valtan pattern Sound cue JSON is malformed: " +
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
		strOutStatus = "Valtan pattern Sound cue header is invalid.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	Staged.iFormatVersion = iFormatVersion;
	Staged.strOwnerArchetypeId = pOwner->Get_String();
	Staged.Cues.reserve(pCues->Get_Array().size());
	std::unordered_set<std::string> BindingIds;
	std::unordered_set<std::string> OccurrenceIds;
	std::unordered_set<std::string> ActionClipOccurrenceTuples;
	size_t iSkippedUnimplementedPatternCount = 0u;
	for (const DATA_JSON_VALUE& CueValue : pCues->Get_Array())
	{
		if (!Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "soundBank", "soundEvent",
				  "repeatPolicy", "startMs" }))
		{
			strOutStatus = "Valtan pattern Sound cue has unexpected properties.";
			return false;
		}

		VALTAN_PATTERN_SOUND_CUE Cue;
		if (!Read_String(CueValue, "bindingId", Cue.strBindingId) ||
			!Read_String(CueValue, "occurrenceId", Cue.strOccurrenceId) ||
			!Read_String(CueValue, "patternId", Cue.strPatternId) ||
			!Read_String(CueValue, "stageId", Cue.strStageId) ||
			!Read_String(CueValue, "actionId", Cue.strActionId) ||
			!Read_String(CueValue, "clipOccurrenceId", Cue.strClipOccurrenceId) ||
			!Read_String(CueValue, "soundBank", Cue.strSoundBank) ||
			!Read_String(CueValue, "soundEvent", Cue.strSoundEvent) ||
			!Read_RepeatPolicy(CueValue, Cue.eRepeatPolicy) ||
			!Read_Unsigned(CueValue, "startMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iStartMs) ||
			!BindingIds.insert(Cue.strBindingId).second ||
			!OccurrenceIds.insert(Cue.strOccurrenceId).second)
		{
			strOutStatus =
				"Valtan pattern Sound cue identity or policy is invalid.";
			return false;
		}

		/* An action's animation binding can be swapped or emptied (playbackMode
		NONE, clips: []) by a later animation-side change without the
		sound-cue side being re-authored in the same change -- e.g.
		valtan.sequence.four.step-02 lost its clip-01 occurrence when its
		animation was swapped out. That is stale cross-layer content, not a
		corrupt document; skip only this cue instead of fail-closing every
		other real cue in the same document (same reasoning as the
		not-yet-implemented-pattern skip below). */
		const BOSS_PATTERN_ANIMATION_BINDING* pAnimationBinding =
			Find_ActionBinding(AnimationBindings, Cue.strActionId);
		if (nullptr == pAnimationBinding)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"action has no animation binding: " + Cue.strActionId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}
		const BOSS_PATTERN_ANIMATION_CLIP* pAnimationClip =
			Find_ClipOccurrence(*pAnimationBinding, Cue.strClipOccurrenceId);
		if (nullptr == pAnimationClip)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"clip occurrence not owned by its action: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}
		if (VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !pAnimationClip->bLoop)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"each_loop cue references a non-loop clip: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}

		/* A pattern authored at the animation/sound-cue layer but not yet wired into
		Data/Encounters/Valtan/ValtanEncounter.json (real Server hit shape/damage/motion
		not authored yet -- e.g. VALTAN_ARENA_BREAK_33/_84) is not a corrupt document,
		just content that has not reached that layer yet. Skip only this cue instead of
		fail-closing every other real cue in the same document. */
		const ENCOUNTER_PATTERN_REFERENCE* pPattern =
			Encounter.Find_Pattern(Cue.strPatternId);
		if (nullptr == pPattern)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"pattern not yet in Encounter: " + Cue.strPatternId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}
		const auto Stage = std::find_if(pPattern->stages.begin(),
			pPattern->stages.end(), [&Cue](const ENCOUNTER_STAGE_REFERENCE& Value)
			{
				return Value.stageId == Cue.strStageId;
			});
		if (pPattern->stages.end() == Stage ||
			Stage->actionId != Cue.strActionId || 0u == Stage->iDurationMs)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"encounter tuple is invalid: " + Cue.strBindingId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}
		Cue.iStageIndex = static_cast<uint32_t>(Stage - pPattern->stages.begin());
		Cue.iStageDurationMs = Stage->iDurationMs;

		const uint64_t iSegmentEndMs =
			static_cast<uint64_t>(pAnimationClip->iSourceStartMs) +
			static_cast<uint64_t>(pAnimationClip->iPlayMs);
		if (Cue.iStartMs < pAnimationClip->iSourceStartMs ||
			(0u != pAnimationClip->iPlayMs && Cue.iStartMs >= iSegmentEndMs))
		{
			strOutStatus =
				"Valtan pattern Sound cue source window is outside its clip segment: " +
				Cue.strOccurrenceId;
			return false;
		}

		const std::string Tuple = Cue.strActionId + "\n" +
			Cue.strClipOccurrenceId + "\n" + Cue.strOccurrenceId;
		if (!ActionClipOccurrenceTuples.insert(Tuple).second)
		{
			strOutStatus = "Duplicate Valtan action/clip/Sound cue occurrence tuple.";
			return false;
		}
		Staged.Cues.push_back(std::move(Cue));
	}

	std::sort(Staged.Cues.begin(), Staged.Cues.end(),
		[](const VALTAN_PATTERN_SOUND_CUE& Left,
			const VALTAN_PATTERN_SOUND_CUE& Right)
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
		" clip-occurrence-qualified Valtan pattern Sound cue(s), skipped " +
		std::to_string(iSkippedUnimplementedPatternCount) +
		" not-yet-implemented-pattern cue(s).";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Load_Source(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
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
			"Missing Valtan pattern Sound cue document: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	if (!Parse_Text(Text, Encounter, AnimationBindings, Staged, strOutStatus))
		return false;
	InOutDocument = std::move(Staged);
	return true;
}
