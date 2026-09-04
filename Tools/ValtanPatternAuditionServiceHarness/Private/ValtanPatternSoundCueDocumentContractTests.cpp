#include "AnimationSkillBindingDocument.h"
#include "Effect_DocumentCodec.h"
#include "EncounterPatternReference.h"
#include "SoundCueCatalog.h"
#include "ValtanCombatObjectSoundCueDocument.h"
#include "ValtanPatternSoundCueDocument.h"

#include <Windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

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
	class SCOPED_TEST_DIRECTORY final
	{
	public:
		explicit SCOPED_TEST_DIRECTORY(std::filesystem::path path)
			: Path(std::move(path))
		{
		}
		~SCOPED_TEST_DIRECTORY()
		{
			std::error_code cleanupError;
			std::filesystem::remove_all(Path, cleanupError);
		}

	private:
		std::filesystem::path Path;
	};

	class SCOPED_ENVIRONMENT_VARIABLE final
	{
	public:
		explicit SCOPED_ENVIRONMENT_VARIABLE(const wchar_t* name)
			: Name(name)
		{
			const DWORD required = GetEnvironmentVariableW(
				Name.c_str(), nullptr, 0u);
			if (0u == required)
				return;
			std::vector<wchar_t> value(required);
			const DWORD copied = GetEnvironmentVariableW(
				Name.c_str(), value.data(), required);
			if (0u != copied && copied < required)
			{
				HadValue = true;
				Previous.assign(value.data(), copied);
			}
		}
		~SCOPED_ENVIRONMENT_VARIABLE()
		{
			SetEnvironmentVariableW(Name.c_str(),
				HadValue ? Previous.c_str() : nullptr);
		}

		bool Set(const std::filesystem::path& value)
		{
			return FALSE != SetEnvironmentVariableW(
				Name.c_str(), value.c_str());
		}

	private:
		std::wstring Name;
		std::wstring Previous;
		bool HadValue = false;
	};

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

	std::size_t CountToken(const std::string_view text,
		const std::string_view token)
	{
		std::size_t count = 0u;
		std::size_t offset = 0u;
		while (!token.empty() &&
			std::string_view::npos != (offset = text.find(token, offset)))
		{
			++count;
			offset += token.size();
		}
		return count;
	}

	bool WriteText(const std::filesystem::path& path,
		const std::string& text)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		output.write(text.data(), static_cast<std::streamsize>(text.size()));
		return output.good();
	}

	std::string ReplaceOnce(std::string text,
		const std::string& needle, const std::string& replacement)
	{
		const std::size_t offset = text.find(needle);
		if (std::string::npos != offset)
			text.replace(offset, needle.size(), replacement);
		return text;
	}

	std::string ReplaceCatalogEventVariantsWithEmpty(
		std::string text,
		const std::string_view eventName)
	{
		const std::string eventPrefix =
			"\"" + std::string(eventName) + "\": [";
		const std::size_t event = text.find(eventPrefix);
		if (std::string::npos == event)
			return text;
		const std::size_t arrayBegin = event + eventPrefix.size() - 1u;
		const std::size_t arrayEnd = text.find(']', arrayBegin);
		if (std::string::npos == arrayEnd)
			return text;
		text.replace(arrayBegin, arrayEnd - arrayBegin + 1u, "[]");
		return text;
	}

	bool HasStagingArtifact(const std::filesystem::path& destination)
	{
		const std::string prefix = destination.filename().string() + ".tmp.";
		std::error_code iterationError;
		for (std::filesystem::directory_iterator iterator(
			destination.parent_path(), iterationError), end;
			!iterationError && iterator != end;
			iterator.increment(iterationError))
		{
			if (iterator->path().filename().string().starts_with(prefix))
				return true;
		}
		return false;
	}

	bool CopyFixture(const std::filesystem::path& source,
		const std::filesystem::path& destination)
	{
		std::error_code error;
		std::filesystem::create_directories(destination.parent_path(), error);
		if (error)
			return false;
		std::filesystem::copy_file(source, destination,
			std::filesystem::copy_options::overwrite_existing, error);
		return !error;
	}

	bool LoadValtanModelClipSourceDurations(
		const std::filesystem::path& root,
		std::unordered_map<std::string, f32_t>& outDurations)
	{
		std::istringstream input(ReadText(root /
			"Data/Animation/Reference/Valtan/Valtan.animnotify"));
		std::unordered_map<std::string, f32_t> staged;
		std::string line;
		while (std::getline(input, line))
		{
			if (line.empty() || '"' != line.front())
				continue;
			const std::size_t nameEnd = line.find('"', 1u);
			const std::size_t durationBegin = line.find(" len=", nameEnd);
			if (std::string::npos == nameEnd ||
				std::string::npos == durationBegin)
			{
				return false;
			}
			const std::size_t valueBegin = durationBegin + 5u;
			const std::size_t valueEnd = line.find(' ', valueBegin);
			try
			{
				const f32_t duration = std::stof(line.substr(valueBegin,
					std::string::npos == valueEnd ? std::string::npos :
					valueEnd - valueBegin));
				if (!std::isfinite(duration) || duration <= 0.f ||
					!staged.emplace(line.substr(1u, nameEnd - 1u),
						duration).second)
				{
					return false;
				}
			}
			catch (...)
			{
				return false;
			}
		}
		if (staged.empty())
			return false;
		outDurations = std::move(staged);
		return true;
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
		const std::string soundSource =
			ReadText(authored / "Valtan.patternsoundcues.json");
		const std::size_t authoredCueCount =
			CountToken(soundSource, "\"bindingId\"");
		const bool loaded = encounter.Load(root / "Data/Encounters/Valtan/ValtanEncounter.json", status) &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				ReadText(authored / "Valtan.patternbindings.json"), animation, status) &&
			CValtanPatternSoundCueDocument::Parse_Text(
				soundSource, encounter, animation, document, status);
		if (!Require(loaded, status.c_str()))
			return false;
		const std::string realDocumentStatus = "real sound document admission mismatch: active=" +
			std::to_string(document.Cues.size()) + ", authored=" +
			std::to_string(authoredCueCount) + ", status=" + status;
		if (!Require(0u != authoredCueCount &&
			authoredCueCount == document.Cues.size() &&
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

	bool VerifyAuthoringSave(const std::filesystem::path& root)
	{
		using namespace Client;
		const std::filesystem::path testRoot =
			std::filesystem::temp_directory_path() /
			("LostArkValtanPatternSoundAuthoringHarness-" +
			 std::to_string(GetCurrentProcessId()) + "-" +
			 std::to_string(GetTickCount64()));
		SCOPED_TEST_DIRECTORY cleanup(testRoot);
		const std::filesystem::path dataRoot = testRoot / "Data";
		const std::filesystem::path destination = dataRoot /
			"Animation/Authored/Valtan/Valtan.patternsoundcues.json";
		const std::filesystem::path bindingPath = dataRoot /
			"Animation/Authored/Valtan/Valtan.patternbindings.json";
		const std::filesystem::path encounterPath = dataRoot /
			"Encounters/Valtan/ValtanEncounter.json";
		const std::filesystem::path catalogPath = dataRoot /
			"Sound/CharacterSoundCatalog.json";
		if (!Require(CopyFixture(root /
				"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
				destination) &&
			CopyFixture(root /
				"Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
				bindingPath) &&
			CopyFixture(root /
				"Data/Encounters/Valtan/ValtanEncounter.json", encounterPath) &&
			CopyFixture(root /
				"Data/Sound/CharacterSoundCatalog.json", catalogPath),
			"could not create isolated pattern Sound authoring fixture"))
		{
			return false;
		}

		SCOPED_ENVIRONMENT_VARIABLE dataEnvironment(
			L"LOSTARK_PROJECT_DATA_ROOT");
		SCOPED_ENVIRONMENT_VARIABLE resourceEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		if (!Require(dataEnvironment.Set(dataRoot) &&
			resourceEnvironment.Set(root / "Client/Bin/Resources"),
			"could not redirect Data/Resources roots for Sound authoring"))
		{
			return false;
		}

		std::string status;
		const bool catalogLoaded = CSoundCueCatalog::Load(status);
		if (!Require(catalogLoaded,
			("real Valtan Sound catalog did not load: " + status).c_str()))
		{
			return false;
		}
		std::unordered_map<std::string, f32_t> modelClipDurations;
		if (!Require(LoadValtanModelClipSourceDurations(
				root, modelClipDurations) &&
			modelClipDurations.contains("mesh_att_battle_1_01") &&
			modelClipDurations.contains("mesh_att_battle_1_02"),
			"could not load real Valtan model-like source durations"))
		{
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT loaded;
		std::string authoringBaseline;
		const bool sourceLoaded =
			CValtanPatternSoundCueDocument::Load_ForAuthoring(
				loaded, authoringBaseline, status);
		const std::size_t authoredCueCount =
			CountToken(authoringBaseline, "\"bindingId\"");
		if (!Require(sourceLoaded && 0u != authoredCueCount &&
			authoredCueCount == loaded.Cues.size() &&
			authoringBaseline == ReadText(destination),
			("strict full Valtan pattern Sound load failed: " + status).c_str()))
		{
			return false;
		}
		const std::string initialSourceBytes = authoringBaseline;
		auto failedLoadDocument = loaded;
		failedLoadDocument.strOwnerArchetypeId = "sentinel.owner";
		const auto failedLoadDocumentBefore = failedLoadDocument;
		std::string failedLoadBaseline = "sentinel-baseline";
		if (!Require(WriteText(destination, "{}\n") &&
			!CValtanPatternSoundCueDocument::Load_ForAuthoring(
				failedLoadDocument, failedLoadBaseline, status) &&
			failedLoadDocument == failedLoadDocumentBefore &&
			failedLoadBaseline == "sentinel-baseline" &&
			WriteText(destination, initialSourceBytes),
			"failed CAS authoring load changed its document/baseline outputs"))
		{
			return false;
		}

		VALTAN_PATTERN_SOUND_CUE_DOCUMENT runtimeLoaded;
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT runtimeReceipt;
		if (!Require(CValtanPatternSoundCueDocument::Load_Source(
				runtimeLoaded, runtimeReceipt, status) &&
			runtimeReceipt.Is_Valid() &&
			runtimeReceipt.iBytes == initialSourceBytes.size(),
			("runtime Sound source receipt load failed: " + status).c_str()))
		{
			return false;
		}
		const std::size_t unresolvedRuntimeRows =
			static_cast<std::size_t>(std::count_if(
				runtimeLoaded.Cues.begin(), runtimeLoaded.Cues.end(),
				[](const VALTAN_PATTERN_SOUND_CUE& cue)
				{
					return cue.strSoundEvent ==
						"G_Voltan1_Attack13_Loop1" &&
						cue.ResolvedAssetIds.empty();
				}));
		const std::size_t expectedUnresolvedRuntimeRows =
			static_cast<std::size_t>(std::count_if(
				loaded.Cues.begin(), loaded.Cues.end(),
				[](const VALTAN_PATTERN_SOUND_CUE& cue)
				{
					return cue.strSoundEvent ==
						"G_Voltan1_Attack13_Loop1";
				}));
		const bool hasPinnedRuntimeAsset = std::any_of(
			runtimeLoaded.Cues.begin(), runtimeLoaded.Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return !cue.ResolvedAssetIds.empty();
			});
		if (!Require(0u != expectedUnresolvedRuntimeRows &&
			expectedUnresolvedRuntimeRows == unresolvedRuntimeRows &&
			hasPinnedRuntimeAsset,
			"runtime Sound load did not preserve unresolved cues while pinning resolved assets"))
		{
			return false;
		}
		{
			CValtanPatternSoundSourceReadAdmission playbackAdmission;
			VALTAN_PATTERN_SOUND_SOURCE_RECEIPT lockedReceipt;
			if (!Require(playbackAdmission.Acquire(lockedReceipt, status) &&
				lockedReceipt == runtimeReceipt,
				("playback Sound source admission did not reproduce the runtime receipt: " +
					status).c_str()))
			{
				return false;
			}
			const HANDLE conflictingWriter = CreateFileW(
				destination.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
				nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE != conflictingWriter)
				CloseHandle(conflictingWriter);
			if (!Require(INVALID_HANDLE_VALUE == conflictingWriter,
				"playback Sound source admission allowed a concurrent destination writer"))
			{
				return false;
			}
		}
		if (!Require(WriteText(destination, initialSourceBytes),
			"Pattern Sound destination remained locked after playback admission release"))
		{
			return false;
		}

		const auto countRows = [&loaded](const std::string_view patternId,
			const std::string_view stageId, const std::string_view actionId,
			const std::string_view clipOccurrenceId)
		{
			return static_cast<std::size_t>(std::count_if(
				loaded.Cues.begin(), loaded.Cues.end(),
				[=](const VALTAN_PATTERN_SOUND_CUE& cue)
				{
					return cue.strPatternId == patternId &&
						cue.strStageId == stageId &&
						cue.strActionId == actionId &&
						cue.strClipOccurrenceId == clipOccurrenceId;
				}));
		};
		if (!Require(2u == countRows("VALTAN_TRASH", "STEP_07",
				"valtan.sequence.center-trash-rush-if.step-07",
				"valtan.sequence.center-trash-rush-if.step-07.clip-01") &&
			3u == countRows("VALTAN_TRASH", "CATCH_COUNTER",
				"valtan.sequence.center-trash-rush-if.catch-counter",
				"valtan.sequence.center-trash-rush-if.catch-counter.clip-01") &&
			2u == countRows("VALTAN_TRASH_CATCH_IF", "STEP_07",
				"valtan.sequence.rush-if.step-07",
				"valtan.sequence.rush-if.step-07.clip-01") &&
			3u == countRows("VALTAN_TRASH_CATCH_IF", "CATCH_COUNTER",
				"valtan.sequence.rush-if.catch-counter",
				"valtan.sequence.rush-if.catch-counter.clip-01"),
			"Trash/catch-if counter-window Sound occurrence rows drifted"))
		{
			return false;
		}

		CEncounterPatternReference encounter;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT animation;
		if (!Require(encounter.Load(encounterPath, status) &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				ReadText(bindingPath), animation, status),
			("real Valtan timing dependencies failed: " + status).c_str()))
		{
			return false;
		}
		const auto fullStructuralDocument = loaded;
		const bool fullStrongAdmission =
			CValtanPatternSoundCueDocument::Validate_Joined(
				loaded, encounter, animation, modelClipDurations, status);
		if (!Require(!fullStrongAdmission && loaded == fullStructuralDocument,
			"structural full-row load did not remain separate from strong runtime admission"))
		{
			return false;
		}

		const auto editable = std::find_if(
			loaded.Cues.begin(), loaded.Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return
					"cue.sound.valtan.sequence.center-trash-rush-if.step-07.clip-01.01.occurrence.01" ==
					cue.strOccurrenceId;
			});
		const auto nonLoopEditable = std::find_if(
			loaded.Cues.begin(), loaded.Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return
					"cue.sound.valtan.attack.swing.active.clip.01.01.occurrence.01" ==
					cue.strOccurrenceId;
			});
		const auto loopEditable = std::find_if(
			loaded.Cues.begin(), loaded.Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return
					"cue.sound.valtan.attack.swing.active.clip.02.02.occurrence.01" ==
					cue.strOccurrenceId;
			});
		const auto legacyInvalidEditable = std::find_if(
			loaded.Cues.begin(), loaded.Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return
					"cue.sound.valtan.attack.backstep.windup.clip.01.02.occurrence.01" ==
					cue.strOccurrenceId;
			});
		if (!Require(loaded.Cues.end() != editable &&
			loaded.Cues.end() != nonLoopEditable &&
			loaded.Cues.end() != loopEditable &&
			loaded.Cues.end() != legacyInvalidEditable,
			"real Sound fixture lost focused strong/grandfather rows"))
		{
			return false;
		}
		const std::size_t editableIndex = static_cast<std::size_t>(
			editable - loaded.Cues.begin());
		const std::size_t nonLoopEditableIndex = static_cast<std::size_t>(
			nonLoopEditable - loaded.Cues.begin());
		const std::size_t loopEditableIndex = static_cast<std::size_t>(
			loopEditable - loaded.Cues.begin());
		const std::size_t legacyInvalidEditableIndex =
			static_cast<std::size_t>(legacyInvalidEditable - loaded.Cues.begin());

		const std::vector<std::string> eventNames =
			CSoundCueCatalog::Collect_EventNames("Valtan");
		const auto alternateEvent = std::find_if(eventNames.begin(),
			eventNames.end(), [](const std::string& eventName)
			{
				return eventName.starts_with("G_Voltan1_");
			});
		if (!Require(eventNames.end() != alternateEvent,
			"Valtan catalog has no alternate Voltan1 Sound event"))
		{
			return false;
		}

		auto draft = loaded;
		draft.Cues[editableIndex].strSoundBank = "S_Mob_G_Voltan1";
		draft.Cues[editableIndex].strSoundEvent = *alternateEvent;
		draft.Cues[editableIndex].iStartMs = 100u;
		draft.Cues[loopEditableIndex].eRepeatPolicy =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		if (!Require(CValtanPatternSoundCueDocument::Save_Atomic(
			draft, modelClipDurations, authoringBaseline, status),
			("typed Valtan pattern Sound save failed: " + status).c_str()))
		{
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT reloaded;
		if (!Require(CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == draft,
			("typed Valtan pattern Sound reload drifted: " + status).c_str()))
		{
			return false;
		}

		VALTAN_PATTERN_SOUND_CUE_ADD_ROW addRow;
		addRow.strPatternId = draft.Cues[editableIndex].strPatternId;
		addRow.strStageId = draft.Cues[editableIndex].strStageId;
		addRow.strActionId = draft.Cues[editableIndex].strActionId;
		addRow.strClipOccurrenceId =
			draft.Cues[editableIndex].strClipOccurrenceId;
		addRow.strSoundBank = "S_Mob_G_Voltan1";
		addRow.strSoundEvent = *alternateEvent;
		addRow.eRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		addRow.iStartMs = 150u;
		auto addedDocument = draft;
		VALTAN_PATTERN_SOUND_CUE_ROW_ID addedRowId;
		if (!Require(CValtanPatternSoundCueDocument::Add_AuthoringRow(
				addedDocument, addRow, modelClipDurations, addedRowId, status) &&
			addedDocument.Cues.size() == draft.Cues.size() + 1u &&
			addedRowId.strBindingId == "cue.sound.authoring." +
				addRow.strClipOccurrenceId + ".0001" &&
			addedRowId.strOccurrenceId ==
				addedRowId.strBindingId + ".occurrence.01",
			("typed Valtan pattern Sound Add Row failed: " + status).c_str()) ||
			!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				addedDocument, modelClipDurations, authoringBaseline, status),
				("typed added Sound row did not save: " + status).c_str()) ||
			!Require(CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == addedDocument,
				("typed added Sound row did not reload exactly: " + status).c_str()))
		{
			return false;
		}

		auto wrongRemoveDocument = reloaded;
		VALTAN_PATTERN_SOUND_CUE_ROW_ID wrongRemoveId = addedRowId;
		wrongRemoveId.strOccurrenceId += ".stale";
		if (!Require(!CValtanPatternSoundCueDocument::Remove_AuthoringRow(
				wrongRemoveDocument, wrongRemoveId, status) &&
			wrongRemoveDocument == reloaded,
			"partial Sound row key removed a non-exact row"))
		{
			return false;
		}
		auto removedDocument = reloaded;
		if (!Require(CValtanPatternSoundCueDocument::Remove_AuthoringRow(
				removedDocument, addedRowId, status) &&
			removedDocument == draft,
			("typed Valtan pattern Sound Remove Row failed: " + status).c_str()) ||
			!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				removedDocument, modelClipDurations, authoringBaseline, status),
				("typed removed Sound row did not save: " + status).c_str()) ||
			!Require(CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == draft,
				("typed removed Sound row did not reload exactly: " + status).c_str()))
		{
			return false;
		}
		const std::string admittedBytes = ReadText(destination);
		if (!Require(authoringBaseline == admittedBytes &&
			admittedBytes.find("\"repeatPolicy\": \"each_loop\"") !=
				std::string::npos &&
			admittedBytes.find("\"startMs\": 100") != std::string::npos &&
			admittedBytes.find(*alternateEvent) != std::string::npos,
			"saved Sound source omitted edited event/timing/repeat state"))
		{
			return false;
		}

		const auto rejectsWithDurationsWithoutMutation =
			[&](const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& candidate,
				const std::unordered_map<std::string, f32_t>& durations,
				const std::string& label)
			{
				const auto candidateBefore = candidate;
				const std::string baselineBefore = authoringBaseline;
				const bool rejected =
					!CValtanPatternSoundCueDocument::Save_Atomic(
						candidate, durations, authoringBaseline, status);
				VALTAN_PATTERN_SOUND_CUE_DOCUMENT after;
				const bool preserved =
					CValtanPatternSoundCueDocument::Load_AuthoringSource(
						after, status) && after == draft;
				return Require(rejected && candidate == candidateBefore &&
					authoringBaseline == baselineBefore &&
					ReadText(destination) == admittedBytes && preserved &&
					!HasStagingArtifact(destination),
					(label + " changed the file/input or left staging state").c_str());
			};
		const auto rejectsWithoutMutation =
			[&](const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& candidate,
				const std::string& label)
			{
				return rejectsWithDurationsWithoutMutation(
					candidate, modelClipDurations, label);
			};
		const auto rejectsAddWithoutMutation =
			[&](const VALTAN_PATTERN_SOUND_CUE_ADD_ROW& candidateRow,
				const std::unordered_map<std::string, f32_t>& durations,
				const std::string& label)
			{
				auto candidateDocument = draft;
				const auto documentBefore = candidateDocument;
				VALTAN_PATTERN_SOUND_CUE_ROW_ID created{
					"sentinel.binding", "sentinel.occurrence" };
				const auto createdBefore = created;
				const bool rejected =
					!CValtanPatternSoundCueDocument::Add_AuthoringRow(
						candidateDocument, candidateRow, durations,
						created, status);
				return Require(rejected && candidateDocument == documentBefore &&
					created == createdBefore && authoringBaseline == admittedBytes &&
					ReadText(destination) == admittedBytes,
					(label + " changed Add Row outputs or source bytes").c_str());
			};

		auto invalidAdd = addRow;
		invalidAdd.strClipOccurrenceId =
			"valtan.sequence.center-trash-rush-if.step-07.clip-stale";
		if (!rejectsAddWithoutMutation(invalidAdd, modelClipDurations,
			"Add Row with no owned clip occurrence"))
		{
			return false;
		}
		invalidAdd = addRow;
		invalidAdd.strSoundBank = "S_Mob_G_Voltan2";
		invalidAdd.strSoundEvent = "G_Voltan2_UnknownWorkbenchEvent";
		if (!rejectsAddWithoutMutation(invalidAdd, modelClipDurations,
			"Add Row with no catalog event"))
		{
			return false;
		}
		invalidAdd.strPatternId = draft.Cues[loopEditableIndex].strPatternId;
		invalidAdd.strStageId = draft.Cues[loopEditableIndex].strStageId;
		invalidAdd.strActionId = draft.Cues[loopEditableIndex].strActionId;
		invalidAdd.strClipOccurrenceId =
			draft.Cues[loopEditableIndex].strClipOccurrenceId;
		invalidAdd.strSoundBank = draft.Cues[loopEditableIndex].strSoundBank;
		invalidAdd.strSoundEvent = draft.Cues[loopEditableIndex].strSoundEvent;
		invalidAdd.eRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		invalidAdd.iStartMs = 2500u;
		if (!rejectsAddWithoutMutation(invalidAdd, modelClipDurations,
			"Add Row outside a playMs-zero model source window"))
		{
			return false;
		}
		std::unordered_map<std::string, f32_t> noAddDurations;
		if (!rejectsAddWithoutMutation(addRow, noAddDurations,
			"Add Row without current model durations"))
		{
			return false;
		}

		auto duplicateAddedDocument = draft;
		VALTAN_PATTERN_SOUND_CUE_ROW_ID duplicateAddedRowId;
		if (!Require(CValtanPatternSoundCueDocument::Add_AuthoringRow(
				duplicateAddedDocument, addRow, modelClipDurations,
				duplicateAddedRowId, status),
			("could not stage duplicate Add Row coverage: " + status).c_str()))
		{
			return false;
		}
		const auto duplicateAddedRow = std::find_if(
			duplicateAddedDocument.Cues.begin(),
			duplicateAddedDocument.Cues.end(),
			[&duplicateAddedRowId](const VALTAN_PATTERN_SOUND_CUE& cue)
			{
				return cue.strBindingId == duplicateAddedRowId.strBindingId;
			});
		if (!Require(duplicateAddedDocument.Cues.end() != duplicateAddedRow,
			"staged Add Row lost its deterministic ID before duplicate coverage"))
		{
			return false;
		}
		duplicateAddedDocument.Cues.push_back(*duplicateAddedRow);
		if (!rejectsWithoutMutation(
			duplicateAddedDocument, "duplicate added stable row identity"))
		{
			return false;
		}

		auto removedDependencyDocument = draft;
		const VALTAN_PATTERN_SOUND_CUE_ROW_ID existingRowId{
			draft.Cues[editableIndex].strBindingId,
			draft.Cues[editableIndex].strOccurrenceId };
		if (!Require(CValtanPatternSoundCueDocument::Remove_AuthoringRow(
				removedDependencyDocument, existingRowId, status),
			("could not stage removed-row duration coverage: " + status).c_str()) ||
			!rejectsWithDurationsWithoutMutation(removedDependencyDocument,
				noAddDurations, "removed row without current model durations"))
		{
			return false;
		}

		auto invalid = draft;
		invalid.Cues[editableIndex].strSoundBank = "S_Mob_G_Voltan2";
		invalid.Cues[editableIndex].strSoundEvent =
			"G_Voltan2_UnknownWorkbenchEvent";
		if (!rejectsWithoutMutation(invalid, "unknown Sound event"))
			return false;
		invalid = draft;
		invalid.Cues[editableIndex].strSoundBank = "S_Mob_G_Voltan9";
		if (!rejectsWithoutMutation(invalid, "unknown Sound bank identity"))
			return false;
		invalid = draft;
		invalid.Cues[editableIndex].strActionId = "valtan.stale.sound-action";
		if (!rejectsWithoutMutation(invalid, "stale action identity"))
			return false;
		invalid = draft;
		invalid.Cues[editableIndex].strClipOccurrenceId =
			"valtan.stale.sound-action.clip-01";
		if (!rejectsWithoutMutation(invalid, "stale clip occurrence identity"))
			return false;
		invalid = draft;
		invalid.Cues[1u].strBindingId = invalid.Cues[0u].strBindingId;
		if (!rejectsWithoutMutation(invalid, "duplicate binding identity"))
			return false;
		invalid = draft;
		invalid.Cues[1u].strOccurrenceId = invalid.Cues[0u].strOccurrenceId;
		if (!rejectsWithoutMutation(invalid, "duplicate occurrence identity"))
			return false;
		invalid = draft;
		invalid.Cues[loopEditableIndex].iStartMs = 2500u;
		if (!rejectsWithoutMutation(invalid,
			"playMs-zero cue at the model source end"))
			return false;
		invalid = draft;
		invalid.Cues[nonLoopEditableIndex].eRepeatPolicy =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		if (!rejectsWithoutMutation(invalid, "each_loop on non-loop occurrence"))
			return false;
		invalid = draft;
		++invalid.Cues[legacyInvalidEditableIndex].iStartMs;
		if (!rejectsWithoutMutation(invalid,
			"editing a grandfathered runtime-isolated cue"))
		{
			return false;
		}

		std::unordered_map<std::string, f32_t> invalidDurations;
		if (!rejectsWithDurationsWithoutMutation(draft, invalidDurations,
			"duration-free strong save"))
		{
			return false;
		}
		auto timingCandidate = draft;
		timingCandidate.Cues[loopEditableIndex].iStartMs = 201u;
		invalidDurations = modelClipDurations;
		invalidDurations.erase("mesh_att_battle_1_01");
		if (!rejectsWithDurationsWithoutMutation(
			timingCandidate, invalidDurations,
			"missing prior model clip duration"))
		{
			return false;
		}
		invalidDurations = modelClipDurations;
		invalidDurations["mesh_att_battle_1_02"] =
			(std::numeric_limits<f32_t>::quiet_NaN)();
		if (!rejectsWithDurationsWithoutMutation(
			timingCandidate, invalidDurations,
			"non-finite model clip duration"))
		{
			return false;
		}
		invalidDurations = modelClipDurations;
		invalidDurations["mesh_att_battle_1_01"] = 5.5f;
		if (!rejectsWithDurationsWithoutMutation(
			timingCandidate, invalidDurations,
			"prior clip wall pushes the cue beyond the stage"))
		{
			return false;
		}

		const std::string animationBindingBytes = ReadText(bindingPath);
		const std::string clipIdentityNeedle =
			"\"clipOccurrenceId\": \"" + addRow.strClipOccurrenceId + "\"";
		const std::string staleAnimationBindings = ReplaceOnce(
			animationBindingBytes, clipIdentityNeedle,
			"\"clipOccurrenceId\": \"" + addRow.strClipOccurrenceId +
			".stale\"");
		const std::string dependencyBaselineBefore = authoringBaseline;
		const auto dependencyDraftBefore = draft;
		if (!Require(staleAnimationBindings != animationBindingBytes &&
			WriteText(bindingPath, staleAnimationBindings),
			"could not stage stale Animation dependency coverage"))
		{
			return false;
		}
		const bool staleDependencySave =
			CValtanPatternSoundCueDocument::Save_Atomic(
				draft, modelClipDurations, authoringBaseline, status);
		if (!Require(!staleDependencySave && draft == dependencyDraftBefore &&
			authoringBaseline == dependencyBaselineBefore &&
			ReadText(destination) == admittedBytes &&
			!HasStagingArtifact(destination) &&
			WriteText(bindingPath, animationBindingBytes),
			"stale Animation dependency changed Sound source/draft/CAS baseline"))
		{
			return false;
		}

		/* The dependency may change after every semantic/timing check has passed
		   but before the Sound ReplaceFile. The harness hook fires at that exact
		   boundary; both joined owners must retain the Sound destination and CAS
		   baseline while their external bytes are restored for the next case. */
		{
			SCOPED_ENVIRONMENT_VARIABLE dependencyMutationEnvironment(
				L"LOSTARK_TEST_VALTAN_SOUND_MUTATE_DEPENDENCY_BEFORE_COMMIT");
			const auto rejectsMidSaveDependencyMutation =
			[&](const std::filesystem::path& dependencyPath,
				const std::string& dependencyBytes,
				const char* label)
			{
				const std::string soundBytesBefore = ReadText(destination);
				const std::string baselineBefore = authoringBaseline;
				const auto draftBefore = draft;
				if (!dependencyMutationEnvironment.Set(dependencyPath))
					return Require(false, label);
				const bool saved = CValtanPatternSoundCueDocument::Save_Atomic(
					draft, modelClipDurations, authoringBaseline, status);
				const std::string mutatedDependencyBytes =
					ReadText(dependencyPath);
				const bool restored =
					WriteText(dependencyPath, dependencyBytes);
				return Require(!saved && draft == draftBefore &&
					authoringBaseline == baselineBefore &&
					ReadText(destination) == soundBytesBefore &&
					mutatedDependencyBytes == dependencyBytes + " " &&
					restored && !HasStagingArtifact(destination) &&
					status.find("stale joined owners at commit") !=
						std::string::npos,
					label);
			};
			if (!rejectsMidSaveDependencyMutation(
					bindingPath, animationBindingBytes,
					"mid-save Animation dependency mutation changed Sound source/draft/CAS baseline") ||
				!rejectsMidSaveDependencyMutation(
					encounterPath, ReadText(encounterPath),
					"mid-save Encounter dependency mutation changed Sound source/draft/CAS baseline"))
			{
				return false;
			}
		}

		const std::string catalogBytes = ReadText(catalogPath);
		const std::vector<std::string> editedVariants =
			CSoundCueCatalog::Find_Variants(
				"Valtan", draft.Cues[editableIndex].strSoundEvent);
		if (!Require(!editedVariants.empty(),
			"edited Sound event has no variant for missing-asset coverage"))
		{
			return false;
		}
		const std::string unresolvedReferencedCatalog =
			ReplaceCatalogEventVariantsWithEmpty(catalogBytes,
				draft.Cues[editableIndex].strSoundEvent);
		if (!Require(unresolvedReferencedCatalog != catalogBytes &&
			WriteText(catalogPath, unresolvedReferencedCatalog) &&
			CSoundCueCatalog::Load(status),
			"could not preserve an unresolved catalog event for consumer validation"))
		{
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT unresolvedCandidate;
		if (!Require(CValtanPatternSoundCueDocument::Load_AuthoringSource(
				unresolvedCandidate, status) &&
			ReadText(destination) == admittedBytes &&
			std::any_of(unresolvedCandidate.Cues.begin(),
				unresolvedCandidate.Cues.end(),
				[&draft, editableIndex](const VALTAN_PATTERN_SOUND_CUE& cue)
				{
					return cue.strSoundEvent ==
						draft.Cues[editableIndex].strSoundEvent &&
						cue.ResolvedAssetIds.empty();
				}),
			"referenced unresolved Valtan Sound event was not preserved as an isolated no-op"))
		{
			return false;
		}
		if (!Require(WriteText(catalogPath, catalogBytes) &&
			CSoundCueCatalog::Load(status),
			"could not restore the resolved Sound catalog after unresolved-event rejection"))
		{
			return false;
		}
		const std::string invalidAssetId =
			"Sound/Valtan/workbench-missing-pattern-sound.wav";
		const std::string brokenCatalog = ReplaceOnce(catalogBytes,
			editedVariants.front(), invalidAssetId);
		if (!Require(brokenCatalog != catalogBytes &&
			WriteText(catalogPath, brokenCatalog) &&
			CSoundCueCatalog::Load(status),
			"could not stage catalog missing-asset coverage"))
		{
			return false;
		}
		const auto assetCandidateBefore = draft;
		const bool assetSave =
			CValtanPatternSoundCueDocument::Save_Atomic(
				draft, modelClipDurations, authoringBaseline, status);
		if (!Require(!assetSave && draft == assetCandidateBefore &&
			authoringBaseline == admittedBytes &&
			ReadText(destination) == admittedBytes &&
			!HasStagingArtifact(destination),
			"missing catalog asset changed the admitted Sound source"))
		{
			return false;
		}
		if (!Require(WriteText(catalogPath, catalogBytes) &&
			CSoundCueCatalog::Load(status) &&
			CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == draft,
			"could not restore the valid Sound catalog after asset rejection"))
		{
			return false;
		}

		/* A second Workbench instance commits a valid editable change after this
		first instance loaded. The first instance must reject its now-stale draft
		without overwriting either the external bytes or its own baseline. */
		auto externalDocument = draft;
		externalDocument.Cues[editableIndex].iStartMs = 101u;
		std::string externalBaseline = authoringBaseline;
		if (!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				externalDocument, modelClipDurations, externalBaseline, status),
			("could not stage a second-editor Sound save: " + status).c_str()))
		{
			return false;
		}
		const std::string externalBytes = ReadText(destination);
		const std::string staleBaselineBefore = authoringBaseline;
		const auto staleDraftBefore = draft;
		const bool staleSave = CValtanPatternSoundCueDocument::Save_Atomic(
			draft, modelClipDurations, authoringBaseline, status);
		if (!Require(!staleSave && draft == staleDraftBefore &&
			authoringBaseline == staleBaselineBefore &&
			externalBaseline == externalBytes &&
			ReadText(destination) == externalBytes &&
			!HasStagingArtifact(destination),
			"stale Sound draft overwrote external editable bytes or advanced its baseline"))
		{
			return false;
		}
		if (!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				draft, modelClipDurations, externalBaseline, status) &&
			externalBaseline == admittedBytes &&
			ReadText(destination) == admittedBytes,
			("could not restore admitted bytes after stale-save coverage: " +
				status).c_str()))
		{
			return false;
		}

		/* Mutate the destination after the sibling temp appears. The second exact
		baseline comparison immediately before commit must preserve that writer's
		bytes, reject this save, remove the temp, and leave this baseline stale. */
		auto midSaveCandidate = draft;
		midSaveCandidate.Cues[editableIndex].iStartMs = 202u;
		std::atomic_bool sawSiblingTemp{ false };
		std::atomic_bool wroteExternalBytes{ false };
		std::thread externalWriter([&]()
			{
				const auto deadline = std::chrono::steady_clock::now() +
					std::chrono::seconds(10);
				while (std::chrono::steady_clock::now() < deadline)
				{
					if (HasStagingArtifact(destination))
					{
						sawSiblingTemp.store(true, std::memory_order_release);
						wroteExternalBytes.store(
							WriteText(destination, externalBytes),
							std::memory_order_release);
						return;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			});
		const std::string midSaveBaselineBefore = authoringBaseline;
		const bool midSave = CValtanPatternSoundCueDocument::Save_Atomic(
			midSaveCandidate, modelClipDurations, authoringBaseline, status);
		externalWriter.join();
		if (!Require(!midSave &&
			sawSiblingTemp.load(std::memory_order_acquire) &&
			wroteExternalBytes.load(std::memory_order_acquire) &&
			authoringBaseline == midSaveBaselineBefore &&
			ReadText(destination) == externalBytes &&
			!HasStagingArtifact(destination),
			"mid-save Sound mutation was overwritten or advanced the stale baseline"))
		{
			return false;
		}
		std::string adoptedExternalBaseline = externalBytes;
		if (!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				draft, modelClipDurations, adoptedExternalBaseline, status) &&
			adoptedExternalBaseline == admittedBytes &&
			ReadText(destination) == admittedBytes,
			("could not recover admitted Sound bytes after mid-save mutation: " +
				status).c_str()))
		{
			return false;
		}
		authoringBaseline = std::move(adoptedExternalBaseline);

		auto blocked = draft;
		blocked.Cues[editableIndex].iStartMs = 201u;
		const HANDLE lock = CreateFileW(destination.c_str(), GENERIC_READ,
			FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (!Require(INVALID_HANDLE_VALUE != lock,
			"could not lock pattern Sound destination for replace failure"))
		{
			return false;
		}
		const bool blockedSave =
			CValtanPatternSoundCueDocument::Save_Atomic(
				blocked, modelClipDurations, authoringBaseline, status);
		CloseHandle(lock);
		if (!Require(!blockedSave && authoringBaseline == admittedBytes &&
			ReadText(destination) == admittedBytes &&
			!HasStagingArtifact(destination),
			"locked atomic replace changed source or left a temp file") ||
			!Require(CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == draft,
			"locked replace failure changed reloadable Sound state"))
		{
			return false;
		}
		if (!Require(CValtanPatternSoundCueDocument::Save_Atomic(
				blocked, modelClipDurations, authoringBaseline, status) &&
			authoringBaseline == ReadText(destination) &&
			CValtanPatternSoundCueDocument::Load_AuthoringSource(
				reloaded, status) && reloaded == blocked,
			("Sound save did not recover after lock release: " + status).c_str()))
		{
			return false;
		}
		return true;
	}

	bool VerifyCombatObjectSoundEventIdentity()
	{
		using namespace Client;
		constexpr std::string_view product = R"json({
  "schema": "lostark.valtan-combat-objects",
  "formatVersion": 1,
  "encounterId": "ENCOUNTER_VALTAN",
  "objects": [
    {
      "combatObjectArchetypeId": "combatobject.valtan.contract",
      "hits": [{ "hitId": "hit.valtan.contract.01" }],
      "presentationEvents": [
        { "presentationEventId": "pulse.valtan.contract.expire", "atMs": 10 }
      ]
    }
  ]
})json";
		constexpr std::string_view valid = R"json({
  "schema": "lostark.valtan-combat-object-sound-cues",
  "formatVersion": 1,
  "ownerArchetypeId": "BOSS_VALTAN",
  "cues": [
    {
      "bindingId": "cue.contract.hit",
      "combatObjectArchetypeId": "combatobject.valtan.contract",
      "hitId": "hit.valtan.contract.01",
      "soundBank": "S_Mob_G_Voltan2",
      "soundEvent": "G_Voltan2_Attack09_ProjExp1"
    },
    {
      "bindingId": "cue.contract.presentation",
      "combatObjectArchetypeId": "combatobject.valtan.contract",
      "presentationEventId": "pulse.valtan.contract.expire",
      "soundBank": "S_Mob_G_Voltan2",
      "soundEvent": "G_Voltan2_Attack09_ProjExp2"
    }
  ]
})json";
		VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT admitted;
		std::string status;
		if (!Require(CValtanCombatObjectSoundCueDocument::Parse_Text(
				valid, product, admitted, status) && 2u == admitted.Cues.size(),
			("combat-object hit/presentation Sound union was rejected: " +
				status).c_str()))
		{
			return false;
		}
		const auto hit = std::find_if(admitted.Cues.begin(), admitted.Cues.end(),
			[](const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue)
			{
				return !cue.strHitId.empty();
			});
		const auto presentation = std::find_if(
			admitted.Cues.begin(), admitted.Cues.end(),
			[](const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue)
			{
				return !cue.strPresentationEventId.empty();
			});
		if (!Require(admitted.Cues.end() != hit &&
			admitted.Cues.end() != presentation &&
			hit->strPresentationEventId.empty() &&
			presentation->strHitId.empty(),
			"combat-object Sound source union did not preserve exactly one identity"))
		{
			return false;
		}

		const auto RejectsWithoutMutation =
			[&](const std::string_view candidate,
				const std::string_view candidateProduct,
				const char* label)
		{
				const std::size_t cueCountBefore = admitted.Cues.size();
				const std::string ownerBefore = admitted.strOwnerArchetypeId;
				return Require(!CValtanCombatObjectSoundCueDocument::Parse_Text(
					candidate, candidateProduct, admitted, status) &&
					cueCountBefore == admitted.Cues.size() &&
					ownerBefore == admitted.strOwnerArchetypeId,
					label);
			};
		constexpr std::string_view both = R"json({
  "schema":"lostark.valtan-combat-object-sound-cues","formatVersion":1,
  "ownerArchetypeId":"BOSS_VALTAN","cues":[{
    "bindingId":"cue.contract.both",
    "combatObjectArchetypeId":"combatobject.valtan.contract",
    "hitId":"hit.valtan.contract.01",
    "presentationEventId":"pulse.valtan.contract.expire",
    "soundBank":"S_Mob_G_Voltan2","soundEvent":"G_Voltan2_Attack09_ProjExp1"
  }]})json";
		constexpr std::string_view neither = R"json({
  "schema":"lostark.valtan-combat-object-sound-cues","formatVersion":1,
  "ownerArchetypeId":"BOSS_VALTAN","cues":[{
    "bindingId":"cue.contract.neither",
    "combatObjectArchetypeId":"combatobject.valtan.contract",
    "soundBank":"S_Mob_G_Voltan2","soundEvent":"G_Voltan2_Attack09_ProjExp1"
  }]})json";
		constexpr std::string_view missing = R"json({
  "schema":"lostark.valtan-combat-object-sound-cues","formatVersion":1,
  "ownerArchetypeId":"BOSS_VALTAN","cues":[{
    "bindingId":"cue.contract.missing",
    "combatObjectArchetypeId":"combatobject.valtan.contract",
    "presentationEventId":"pulse.valtan.contract.missing",
    "soundBank":"S_Mob_G_Voltan2","soundEvent":"G_Voltan2_Attack09_ProjExp1"
  }]})json";
		constexpr std::string_view wrongVersion = R"json({
  "schema":"lostark.valtan-combat-object-sound-cues","formatVersion":2,
  "ownerArchetypeId":"BOSS_VALTAN","cues":[{
    "bindingId":"cue.contract.version",
    "combatObjectArchetypeId":"combatobject.valtan.contract",
    "hitId":"hit.valtan.contract.01",
    "soundBank":"S_Mob_G_Voltan2","soundEvent":"G_Voltan2_Attack09_ProjExp1"
  }]})json";
		constexpr std::string_view malformedProduct = R"json({
  "schema":"lostark.valtan-combat-objects","formatVersion":1,
  "encounterId":"ENCOUNTER_VALTAN","objects":[{
    "combatObjectArchetypeId":"combatobject.valtan.contract",
    "hits":[],"presentationEvents":{}
  }]})json";
		return RejectsWithoutMutation(both, product,
			"combat-object Sound cue admitted both hitId and presentationEventId") &&
			RejectsWithoutMutation(neither, product,
				"combat-object Sound cue admitted no Server event identity") &&
			RejectsWithoutMutation(missing, product,
				"combat-object Sound cue admitted a missing presentation event") &&
			RejectsWithoutMutation(wrongVersion, product,
				"combat-object Sound cue admitted an unsupported version") &&
			RejectsWithoutMutation(valid, malformedProduct,
				"combat-object Sound cue admitted malformed Product events");
	}
}

int Run_ValtanPatternSoundCueDocumentContractTests()
{
	if (!VerifySoundCueIsolation(std::filesystem::current_path()))
		return 1;
	if (!VerifyAuthoringSave(std::filesystem::current_path()))
		return 1;
	if (!VerifyCombatObjectSoundEventIdentity())
		return 1;
	std::cout << "Valtan pattern Sound cue document contracts: PASS\n";
	return 0;
}
