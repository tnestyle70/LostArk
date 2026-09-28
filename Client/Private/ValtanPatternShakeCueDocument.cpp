#include "ValtanPatternShakeCueDocument.h"

#include "AnimationSkillBindingDocument.h"
#include "DataJson.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternTree.h"
#include <sstream>

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

	constexpr std::string_view SCHEMA = "lostark.valtan-pattern-shake-cues";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
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
		VALTAN_PATTERN_SHAKE_REPEAT_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("repeatPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("once" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SHAKE_REPEAT_POLICY::ONCE;
		else if ("each_loop" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP;
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
Client::CValtanPatternShakeCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternshakecues.json");
}

bool_t Client::CValtanPatternShakeCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	if (!Encounter.Is_Ready() ||
		Encounter.Get_BossArchetypeId() != OWNER_ARCHETYPE_ID ||
		AnimationBindings.strBossArchetypeId != OWNER_ARCHETYPE_ID)
	{
		strOutStatus =
			"Valtan pattern Shake cues require the validated Valtan encounter.";
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
		strOutStatus = "Valtan pattern Shake cue JSON is malformed: " +
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
		pCues->Get_Array().size() > MAX_CUE_COUNT)
	{
		strOutStatus = "Valtan pattern Shake cue header is invalid.";
		return false;
	}

	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Staged;
	Staged.iFormatVersion = iFormatVersion;
	Staged.strOwnerArchetypeId = pOwner->Get_String();
	Staged.Cues.reserve(pCues->Get_Array().size());
	std::unordered_set<std::string> BindingIds;
	std::unordered_set<std::string> OccurrenceIds;
	std::unordered_set<std::string> ActionClipOccurrenceTuples;
	size_t iSkippedUnimplementedPatternCount = 0u;
	size_t iSkippedSuppressedAnimationCount = 0u;
	for (const DATA_JSON_VALUE& CueValue : pCues->Get_Array())
	{
		if (!Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "shake",
				  "repeatPolicy", "startMs" }))
		{
			strOutStatus = "Valtan pattern Shake cue has unexpected properties.";
			return false;
		}

		VALTAN_PATTERN_SHAKE_CUE Cue;
		if (!Read_String(CueValue, "bindingId", Cue.strBindingId) ||
			!Read_String(CueValue, "occurrenceId", Cue.strOccurrenceId) ||
			!Read_String(CueValue, "patternId", Cue.strPatternId) ||
			!Read_String(CueValue, "stageId", Cue.strStageId) ||
			!Read_String(CueValue, "actionId", Cue.strActionId) ||
			!Read_String(CueValue, "clipOccurrenceId", Cue.strClipOccurrenceId) ||
			!Read_RepeatPolicy(CueValue, Cue.eRepeatPolicy) ||
			!Read_Unsigned(CueValue, "startMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iStartMs) ||
			!BindingIds.insert(Cue.strBindingId).second ||
			!OccurrenceIds.insert(Cue.strOccurrenceId).second)
		{
			strOutStatus =
				"Valtan pattern Shake cue identity or policy is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* pShake = CueValue.Find("shake");
		std::string ShakeStatus;
		if (nullptr == pShake || !pShake->Is_String() ||
			!CCameraShakeService::Parse_PayloadSpec(
				pShake->Get_String(), Cue.Spec, ShakeStatus))
		{
			strOutStatus = "Valtan pattern Shake cue spec is invalid: " +
				Cue.strBindingId + " (" + ShakeStatus + ")";
			return false;
		}

		Cue.strPayload = pShake->Get_String();

		/* Only an explicitly suppressed, known action (NONE + []) can leave a
		stale sound occurrence behind. Typos/unknown actions and broken active
		clip joins remain corrupt data and must preserve the previous document. */
		const BOSS_PATTERN_ANIMATION_BINDING* pAnimationBinding =
			Find_ActionBinding(AnimationBindings, Cue.strActionId);
		if (nullptr == pAnimationBinding)
		{
			strOutStatus = "Valtan pattern Shake cue action has no animation binding: " +
				Cue.strActionId;
			return false;
		}
		const bool_t isSuppressedAnimation =
			pAnimationBinding->bSuppressAnimation && pAnimationBinding->Clips.empty();
		if (pAnimationBinding->bSuppressAnimation && !pAnimationBinding->Clips.empty())
		{
			strOutStatus = "Valtan suppressed animation binding unexpectedly declares clips: " +
				Cue.strActionId;
			return false;
		}
		// Explicit NONE retires the entire animation tuple: its old encounter
		// stage can also have been removed (sequence.four.step-02).
		if (isSuppressedAnimation)
		{
			++iSkippedSuppressedAnimationCount;
			Staged.PreservedCues.push_back(std::move(Cue));
			continue;
		}
		const BOSS_PATTERN_ANIMATION_CLIP* pAnimationClip =
			Find_ClipOccurrence(*pAnimationBinding, Cue.strClipOccurrenceId);
		if (nullptr == pAnimationClip)
		{
			strOutStatus = "Valtan pattern Shake cue clip occurrence is not owned by its action: " +
				Cue.strOccurrenceId;
			return false;
		}
		if (VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !pAnimationClip->bLoop)
		{
			strOutStatus = "Valtan pattern Shake each_loop cue references a non-loop clip: " +
				Cue.strOccurrenceId;
			return false;
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
			OutputDebugStringA(("[Client][Valtan] pattern Shake cue skipped -- "
				"pattern not yet in Encounter: " + Cue.strPatternId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			Staged.PreservedCues.push_back(std::move(Cue));
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
			strOutStatus = "Valtan pattern Shake cue encounter tuple is invalid: " + Cue.strBindingId;
			return false;
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
				"Valtan pattern Shake cue source window is outside its clip segment: " +
				Cue.strOccurrenceId;
			return false;
		}

		const std::string Tuple = Cue.strActionId + "\n" +
			Cue.strClipOccurrenceId + "\n" + Cue.strOccurrenceId;
		if (!ActionClipOccurrenceTuples.insert(Tuple).second)
		{
			strOutStatus = "Duplicate Valtan action/clip/Shake cue occurrence tuple.";
			return false;
		}
		Staged.Cues.push_back(std::move(Cue));
	}

	std::sort(Staged.Cues.begin(), Staged.Cues.end(),
		[](const VALTAN_PATTERN_SHAKE_CUE& Left,
			const VALTAN_PATTERN_SHAKE_CUE& Right)
		{
			return std::tie(Left.strActionId,
				Left.strClipOccurrenceId, Left.iStartMs,
				Left.strOccurrenceId) <
				std::tie(Right.strActionId,
					Right.strClipOccurrenceId, Right.iStartMs,
					Right.strOccurrenceId);
		});
	Staged.strSourceBytes = std::string(Text);
	InOutDocument = std::move(Staged);
	strOutStatus = "Parsed " + std::to_string(InOutDocument.Cues.size()) +
		" clip-occurrence-qualified Valtan pattern Shake cue(s), skipped " +
		std::to_string(iSkippedUnimplementedPatternCount) +
		" not-yet-implemented-pattern cue(s), " +
		std::to_string(iSkippedSuppressedAnimationCount) +
		" explicitly suppressed-animation cue(s).";
	return true;
}

bool_t Client::CValtanPatternShakeCueDocument::Load_Source(
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& InOutDocument,
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
			"Missing Valtan pattern Shake cue document: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Staged;
	if (!Parse_Text(Text, Encounter, AnimationBindings, Staged, strOutStatus))
		return false;
	InOutDocument = std::move(Staged);
	return true;
}

namespace
{
	bool SerializeShakeDraft(const Client::VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document,
		std::string& candidate, std::string& status)
	{
		if (document.strOwnerArchetypeId != OWNER_ARCHETYPE_ID || document.iFormatVersion != FORMAT_VERSION ||
			document.Cues.size() + document.PreservedCues.size() > MAX_CUE_COUNT)
		{ status = "Camera Shake draft header or count is invalid."; return false; }
		std::unordered_set<std::string> bindings, occurrences;
		std::ostringstream out;
		out << "{\n  \"schema\": \"lostark.valtan-pattern-shake-cues\",\n  \"formatVersion\": 1,\n  \"ownerArchetypeId\": \"BOSS_VALTAN\",\n  \"cues\": [";
		bool first = true;
		for (const auto* rows : {&document.Cues, &document.PreservedCues}) for (const auto& cue : *rows)
		{
			Client::CAMERA_SHAKE_SPEC spec;
			if (!Is_StableId(cue.strBindingId) || !Is_StableId(cue.strOccurrenceId) || !Is_StableId(cue.strPatternId) ||
				!Is_StableId(cue.strStageId) || !Is_StableId(cue.strActionId) || !Is_StableId(cue.strClipOccurrenceId) ||
				!bindings.insert(cue.strBindingId).second || !occurrences.insert(cue.strOccurrenceId).second ||
				!Client::CCameraShakeService::Parse_PayloadSpec(cue.strPayload, spec, status) || cue.iStartMs > 600000u ||
				(cue.eRepeatPolicy != Client::VALTAN_PATTERN_SHAKE_REPEAT_POLICY::ONCE &&
				 cue.eRepeatPolicy != Client::VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP))
			{ status = "Camera Shake draft has an invalid or duplicate cue: " + cue.strOccurrenceId + ". " + status; return false; }
			const auto quote = [](const std::string& value) { return "\"" + Client::CDataJson::Escape(value) + "\""; };
			if (!first) out << ","; first = false;
			out << "\n    {\"bindingId\": " << quote(cue.strBindingId) << ", \"occurrenceId\": " << quote(cue.strOccurrenceId)
				<< ", \"patternId\": " << quote(cue.strPatternId) << ", \"stageId\": " << quote(cue.strStageId)
				<< ", \"actionId\": " << quote(cue.strActionId) << ", \"clipOccurrenceId\": " << quote(cue.strClipOccurrenceId)
				<< ", \"repeatPolicy\": \"" << (cue.eRepeatPolicy == Client::VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ? "each_loop" : "once")
				<< "\", \"startMs\": " << cue.iStartMs << ", \"shake\": " << quote(cue.strPayload) << "}";
		}
		out << "\n  ]\n}\n"; candidate = out.str(); return true;
	}
}

bool_t Client::CValtanPatternShakeCueDocument::Remove_StageDraft(
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document, const std::string& patternId,
	const std::string& stageId, std::string& status)
{
	return Remove_ClipDraft(document, patternId, stageId, {}, status);
}

bool_t Client::CValtanPatternShakeCueDocument::Remove_ClipDraft(
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document, const std::string& patternId,
	const std::string& stageId, const std::string& clipId, std::string& status)
{
	if (document.strSourceBytes.empty()) { status = "Camera Shake source baseline is unavailable; draft preserved."; return false; }
	std::size_t removed = 0u;
	for (auto* rows : {&document.Cues, &document.PreservedCues})
		removed += std::erase_if(*rows, [&](const auto& cue) {
			return cue.strPatternId == patternId && cue.strStageId == stageId &&
				(clipId.empty() || cue.strClipOccurrenceId == clipId);
		});
	if (removed) { document.bDraftDirty = true; ++document.iDraftGeneration; }
	return true;
}

bool_t Client::CValtanPatternShakeCueDocument::Append_StageCopyDraft(
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document, const std::vector<VALTAN_PATTERN_SHAKE_CUE>& copied,
	const VALTAN_STAGE_VIEW& source, const VALTAN_STAGE_VIEW& target, const std::string& patternId,
	const uint32_t stageIndex, std::string& status)
{
	if (copied.empty()) return true;
	if (document.strSourceBytes.empty() || source.ClipOccurrences.size() != target.ClipOccurrences.size())
	{ status = "Camera Shake copy source or clip mapping is unavailable; draft preserved."; return false; }
	auto candidate = document;
	for (auto cue : copied)
	{
		const auto clip = std::find_if(source.ClipOccurrences.begin(), source.ClipOccurrences.end(), [&](const auto& row) {
			return row.strClipOccurrenceId == cue.strClipOccurrenceId;
		});
		if (clip == source.ClipOccurrences.end() || cue.strStageId != source.strStageId || cue.strActionId != source.strActionId)
		{ status = "Camera Shake copy references a stale source clip: " + cue.strOccurrenceId; return false; }
		const auto& mapped = target.ClipOccurrences[static_cast<std::size_t>(clip - source.ClipOccurrences.begin())];
		cue.strPatternId = patternId; cue.strStageId = target.strStageId; cue.strActionId = target.strActionId;
		cue.strClipOccurrenceId = mapped.strClipOccurrenceId; cue.iStageIndex = stageIndex; cue.iStageDurationMs = target.iDurationMs;
		cue.strBindingId = "cue.shake.composition." + target.strActionId + "." + std::to_string(candidate.Cues.size() + 1u);
		cue.strOccurrenceId = cue.strBindingId + ".occurrence.1";
		candidate.Cues.push_back(std::move(cue));
	}
	std::string serialized;
	if (!SerializeShakeDraft(candidate, serialized, status)) return false;
	candidate.bDraftDirty = true; ++candidate.iDraftGeneration; document = std::move(candidate); return true;
}

bool_t Client::CValtanPatternShakeCueDocument::Prepare_Save(
	const VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document, std::string& baseline, std::string& candidate,
	uint64_t& generation, std::string& status)
{
	baseline.clear(); candidate.clear(); generation = document.iDraftGeneration;
	if (!document.bDraftDirty) return true;
	if (document.strSourceBytes.empty()) { status = "Camera Shake Save baseline is unavailable."; return false; }
	if (!SerializeShakeDraft(document, candidate, status)) return false;
	baseline = document.strSourceBytes; return true;
}

bool_t Client::CValtanPatternShakeCueDocument::Accept_Save(
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT& document, const uint64_t generation,
	const std::string& candidate, std::string& status)
{
	std::string current;
	if (document.iDraftGeneration != generation || !SerializeShakeDraft(document, current, status) || current != candidate)
	{ status = "Camera Shake draft changed while Save completed; local edits were preserved."; return false; }
	std::ifstream input(Resolve_Path(), std::ios::binary);
	const std::string disk{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
	if (!input || disk != candidate) { status = "Saved Camera Shake bytes changed before local accept; draft preserved."; return false; }
	document.strSourceBytes = candidate; document.bDraftDirty = false; return true;
}
