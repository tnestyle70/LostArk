#include "AnimationSkillBindingDocument.h"
#include "Effect_DocumentCodec.h"
#include "EncounterPatternReference.h"
#include "ValtanPatternSoundCueDocument.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

// AnimationSkillBindingDocument.cpp also owns Effect-tree staging. Keep that
// unrelated codec boundary fail-closed instead of pulling the 14k-line Effect
// codec back into this focused Sound contract executable.
bool_t Client::CEffectDocumentCodec::Load(
	const std::filesystem::path&,
	EFFECT_DOCUMENT_DESC&,
	std::string& outError)
{
	outError = "Effect document loading is outside the focused Valtan sound cue contract";
	return false;
}

namespace
{
	bool Require(const bool condition, const char* message)
	{
		if (!condition)
			std::cerr << "ValtanPatternSoundCueDocumentContracts: " << message << '\n';
		return condition;
	}

	std::string ReadText(const std::filesystem::path& path)
	{
		std::ifstream stream(path, std::ios::binary);
		return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
	}

	std::string SoundCueText(const Client::VALTAN_PATTERN_SOUND_CUE& cue)
	{
		std::ostringstream text;
		text << "{\"bindingId\":\"" << cue.strBindingId
			<< "\",\"occurrenceId\":\"" << cue.strOccurrenceId
			<< "\",\"patternId\":\"" << cue.strPatternId
			<< "\",\"stageId\":\"" << cue.strStageId
			<< "\",\"actionId\":\"" << cue.strActionId
			<< "\",\"clipOccurrenceId\":\"" << cue.strClipOccurrenceId
			<< "\",\"soundBank\":\"" << cue.strSoundBank
			<< "\",\"soundEvent\":\"" << cue.strSoundEvent
			<< "\",\"repeatPolicy\":\""
			<< (Client::VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == cue.eRepeatPolicy ? "each_loop" : "once")
			<< "\",\"startMs\":" << cue.iStartMs << '}';
		return text.str();
	}

	std::string SoundDocumentText(const std::string& rows)
	{
		return "{\"schema\":\"lostark.valtan-pattern-sound-cues\",\"formatVersion\":1,"
			"\"ownerArchetypeId\":\"BOSS_VALTAN\",\"cues\":[" + rows + "]}";
	}

	bool VerifySoundCueIsolation(const std::filesystem::path& root)
	{
		using namespace Client;
		CEncounterPatternReference encounter;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT animation;
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT document;
		std::string status;
		const auto authored = root / "Data/Animation/Authored/Valtan";
		const bool loaded = encounter.Load(root / "Data/Encounters/Valtan/ValtanEncounter.json", status) &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				ReadText(authored / "Valtan.patternbindings.json"), animation, status) &&
			CValtanPatternSoundCueDocument::Parse_Text(
				ReadText(authored / "Valtan.patternsoundcues.json"), encounter, animation, document, status);
		if (!Require(loaded, status.c_str()))
			return false;
		const std::string realDocumentStatus = "real sound document admission mismatch: active=" +
			std::to_string(document.Cues.size()) + ", status=" + status;
		if (!Require(566u == document.Cues.size() &&
			status.find("0 explicitly suppressed-animation") != std::string::npos &&
			status.find("0 not-yet-implemented-pattern") != std::string::npos,
			realDocumentStatus.c_str()))
			return false;
		const auto committed = document;
		const auto& valid = committed.Cues.front();
		auto single = document;
		const bool validFixtureAccepted = CValtanPatternSoundCueDocument::Parse_Text(
			SoundDocumentText(SoundCueText(valid)), encounter, animation, single, status);
		if (!Require(validFixtureAccepted && 1u == single.Cues.size(),
			"minimal valid sound fixture was rejected before mutation tests"))
			return false;
		// Current published rows no longer contain stale NONE/unimplemented cues.
		// Keep both isolation branches covered independently of that content cleanup.
		const auto suppressible = std::find_if(committed.Cues.begin(), committed.Cues.end(),
			[&valid](const VALTAN_PATTERN_SOUND_CUE& cue)
			{ return cue.strActionId != valid.strActionId; });
		if (!Require(committed.Cues.end() != suppressible,
			"sound isolation fixture has no independent known action to suppress"))
			return false;
		auto isolationBindings = animation;
		for (auto& binding : isolationBindings.Bindings)
			if (binding.strActionId == suppressible->strActionId)
			{
				binding.bSuppressAnimation = true;
				binding.Clips.clear();
			}
		auto unimplemented = valid;
		unimplemented.strBindingId += ".unimplemented";
		unimplemented.strOccurrenceId += ".unimplemented";
		unimplemented.strPatternId = "VALTAN_UNIMPLEMENTED_SOUND_FIXTURE";
		auto isolated = single;
		if (!Require(CValtanPatternSoundCueDocument::Parse_Text(
			SoundDocumentText(SoundCueText(valid) + ',' + SoundCueText(*suppressible) + ',' +
				SoundCueText(unimplemented)), encounter, isolationBindings, isolated, status) &&
			1u == isolated.Cues.size() &&
			SoundCueText(isolated.Cues.front()) == SoundCueText(valid) &&
			status.find("1 explicitly suppressed-animation") != std::string::npos &&
			status.find("1 not-yet-implemented-pattern") != std::string::npos,
			"known NONE/unimplemented sound cues were not isolated from the valid cue"))
			return false;
		const auto preservesDocument = [&](const std::string& text,
			const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& bindings)
		{
			return !CValtanPatternSoundCueDocument::Parse_Text(text, encounter, bindings, document, status) &&
				document.Cues.size() == committed.Cues.size() &&
				SoundCueText(document.Cues.front()) == SoundCueText(committed.Cues.front()) &&
				SoundCueText(document.Cues.back()) == SoundCueText(committed.Cues.back());
		};
		auto invalid = valid;
		invalid.strBindingId += ".invalid";
		invalid.strOccurrenceId += ".invalid";
		invalid.strActionId = "valtan.unknown-action";
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(valid) + ',' + SoundCueText(invalid)), animation),
			"unknown action was skipped or partially replaced committed cues"))
			return false;
		invalid.strActionId = valid.strActionId;
		invalid.strClipOccurrenceId = "unknown-occurrence";
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(invalid)), animation),
			"unknown occurrence was skipped instead of fail-closing"))
			return false;
		invalid.strClipOccurrenceId = valid.strClipOccurrenceId;
		invalid.strStageId = "UNKNOWN_STAGE";
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(invalid)), animation),
			"malformed encounter tuple was accepted"))
			return false;
		invalid.strStageId = valid.strStageId;
		invalid.eRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		auto nonLoop = animation;
		for (auto& binding : nonLoop.Bindings)
			if (binding.strActionId == valid.strActionId)
				for (auto& clip : binding.Clips)
					clip.bLoop = false;
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(invalid)), nonLoop),
			"each_loop on non-loop clip was silently skipped"))
			return false;
		auto implicitEmpty = animation;
		for (auto& binding : implicitEmpty.Bindings)
			if (binding.strActionId == valid.strActionId)
				binding.Clips.clear();
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(valid)), implicitEmpty),
			"empty clips without explicit NONE was treated as suppression"))
			return false;
		auto malformedSuppression = animation;
		for (auto& binding : malformedSuppression.Bindings)
			if (binding.strActionId == valid.strActionId)
				binding.bSuppressAnimation = true;
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(valid)), malformedSuppression),
			"NONE with non-empty clips was accepted"))
			return false;
		if (!Require(preservesDocument(SoundDocumentText(SoundCueText(valid) + ',' + SoundCueText(valid)), animation) &&
			preservesDocument("{\"formatVersion\":99}", animation),
			"duplicate identity or malformed/versioned document did not roll back"))
			return false;
		return true;
	}
}

int Run_ValtanPatternSoundCueDocumentContractTests()
{
	if (!VerifySoundCueIsolation(std::filesystem::current_path()))
		return 1;
	std::cout << "Valtan pattern Sound cue document contracts: PASS\n";
	return 0;
}
