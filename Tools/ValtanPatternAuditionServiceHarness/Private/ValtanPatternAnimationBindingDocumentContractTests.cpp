#include "AnimationSkillBindingDocument.h"
#include "ClientReplication.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
	using namespace Client;

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

		bool_t Set(const std::filesystem::path& value)
		{
			return FALSE != SetEnvironmentVariableW(
				Name.c_str(), value.c_str());
		}

	private:
		std::wstring Name;
		std::wstring Previous;
		bool_t HadValue = false;
	};

	bool_t Require(const bool_t condition, const std::string& message)
	{
		if (!condition)
		{
			std::cerr <<
				"ValtanPatternAnimationBindingDocumentContracts: " <<
				message << '\n';
		}
		return condition;
	}

	bool_t VerifyAuthoritativeFreshnessGate()
	{
		CPrimaryValtanPresentationFreshnessGate gate;
		std::string status;
		if (!Require(gate.Can_Play(status),
			"fresh authoritative presentation gate rejected Complete Play"))
		{
			return false;
		}
		gate.Reject("authoritative cache reload failed");
		if (!Require(!gate.Can_Play(status) &&
			status.find("authoritative cache reload failed") != std::string::npos,
			"stale authoritative presentation gate did not retain its diagnostic"))
		{
			return false;
		}
		/* A despawn/no-consumer edge does not touch the gate. The original
		   rejection therefore remains latched until an explicit world reset or
		   successful primary-consumer reload/spawn. */
		if (!Require(!gate.Can_Play(status),
			"despawn/no-consumer lifecycle cleared a rejected freshness gate"))
		{
			return false;
		}
		gate.Admit("replicated world reset");
		if (!Require(gate.Can_Play(status),
			"world reset did not establish a fresh presentation lifetime"))
		{
			return false;
		}
		gate.Reject("next consumer reload failed");
		gate.Admit("authoritative primary spawn reloaded");
		return Require(gate.Can_Play(status),
			"successful authoritative spawn reload did not reopen Complete Play");
	}

	std::string ReadText(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		return { std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
	}

	bool_t WriteText(const std::filesystem::path& path,
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

	bool_t ReadValtanClipMap(
		const std::filesystem::path& path,
		std::vector<std::string>& outClips)
	{
		std::ifstream input(path, std::ios::binary);
		std::string magic;
		std::string asset;
		std::uint32_t version = 0u;
		std::size_t count = 0u;
		if (!input || !(input >> magic >> version >> std::quoted(asset) >> count) ||
			magic != "LOSTARK_CLIP_MAP" || 1u != version || asset != "Valtan" ||
			0u == count || count > 4096u)
		{
			return false;
		}

		std::unordered_set<std::string> unique;
		outClips.clear();
		outClips.reserve(count);
		for (std::size_t index = 0u; index < count; ++index)
		{
			std::string clip;
			if (!(input >> std::quoted(clip)) || clip.empty() ||
				!unique.insert(clip).second)
			{
				return false;
			}
			std::string remainder;
			std::getline(input, remainder);
			outClips.push_back(std::move(clip));
		}
		return outClips.size() == count;
	}

	void AddBoundModelClips(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
		std::vector<std::string>& inOutClips)
	{
		std::unordered_set<std::string> unique(
			inOutClips.begin(), inOutClips.end());
		for (const BOSS_PATTERN_ANIMATION_BINDING& binding : document.Bindings)
		{
			for (const BOSS_PATTERN_ANIMATION_CLIP& clip : binding.Clips)
			{
				if (unique.insert(clip.strClipName).second)
					inOutClips.push_back(clip.strClipName);
			}
		}
	}

	bool_t HasStagingArtifact(const std::filesystem::path& destination)
	{
		const std::string prefix = destination.filename().string() + ".tmp.";
		std::error_code iterationError;
		for (std::filesystem::directory_iterator iterator(
			destination.parent_path(), iterationError), end;
			!iterationError && iterator != end; iterator.increment(iterationError))
		{
			if (iterator->path().filename().string().starts_with(prefix))
				return true;
		}
		return false;
	}

	std::size_t FirstBindingWithClips(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
		const std::size_t minimumClipCount = 1u)
	{
		for (std::size_t index = 0u; index < document.Bindings.size(); ++index)
		{
			if (!document.Bindings[index].bSuppressAnimation &&
				document.Bindings[index].Clips.size() >= minimumClipCount)
			{
				return index;
			}
		}
		return document.Bindings.size();
	}

	bool_t HasIdentityRow(const std::string& sources,
		const std::string_view field, const std::string& value)
	{
		return std::string::npos != sources.find(
			"\"" + std::string(field) + "\": \"" + value + "\"");
	}

	std::size_t FirstBindingWithUnreferencedAction(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
		const std::string& dependentSources,
		const std::size_t excluded = (std::numeric_limits<std::size_t>::max)())
	{
		for (std::size_t index = 0u; index < document.Bindings.size(); ++index)
		{
			const BOSS_PATTERN_ANIMATION_BINDING& binding = document.Bindings[index];
			if (index != excluded && !binding.bSuppressAnimation &&
				!binding.Clips.empty() &&
				!HasIdentityRow(
					dependentSources, "actionId", binding.strActionId))
			{
				return index;
			}
		}
		return document.Bindings.size();
	}

	bool_t VerifyContract()
	{
		if (!VerifyAuthoritativeFreshnessGate())
			return false;
		const std::filesystem::path repository =
			std::filesystem::current_path();
		const std::filesystem::path source = repository /
			"Data/Animation/Authored/Valtan/Valtan.patternbindings.json";
		const std::filesystem::path clipMap = repository /
			"Data/Animation/Reference/Valtan/Valtan.clipmap";
		const std::filesystem::path encounterSource = repository /
			"Data/Encounters/Valtan/ValtanEncounter.json";
		const std::array<std::filesystem::path, 3u> dependentSources = {
			repository / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
			repository / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
			repository / "Data/Animation/Authored/Valtan/Valtan.patternshakecues.json" };
		const std::string sourceText = ReadText(source);
		const std::string encounterText = ReadText(encounterSource);
		std::string dependentSourceText;
		for (const std::filesystem::path& path : dependentSources)
			dependentSourceText += ReadText(path);
		std::string status;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT sourceDocument;
		if (!Require(!sourceText.empty() &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				sourceText, sourceDocument, status),
			"real Valtan binding source did not parse: " + status))
		{
			return false;
		}

		std::vector<std::string> availableClips;
		if (!Require(ReadValtanClipMap(clipMap, availableClips),
			"real Valtan clip-map contract did not parse"))
		{
			return false;
		}
		/* The Product model also carries death/respawn clips that are not part
		   of the read-only skill reference map. Preserve the real admitted
		   binding union while the runtime continues to supply this same vector
		   from CModel::Get_AnimationName. */
		AddBoundModelClips(sourceDocument, availableClips);
		std::unordered_map<std::string, f32_t> clipSourceDurations;
		clipSourceDurations.reserve(availableClips.size());
		for (const std::string& clip : availableClips)
			clipSourceDurations.emplace(clip, 120.f);
		if (!Require(CValtanPatternAnimationBindingDocument::Validate(
			sourceDocument, "BOSS_VALTAN", availableClips, status),
			"real Valtan binding/available-clip join failed: " + status))
		{
			return false;
		}

		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT parseTarget = sourceDocument;
		const std::string unknownField = ReplaceOnce(sourceText,
			"\"bindings\": [", "\"unknown\": true, \"bindings\": [");
		if (!Require(!CValtanPatternAnimationBindingDocument::Parse_Text(
			unknownField, parseTarget, status) && parseTarget == sourceDocument,
			"strict parse failure replaced the admitted in-memory document"))
		{
			return false;
		}

		const std::filesystem::path testRoot =
			std::filesystem::temp_directory_path() /
			("LostArkValtanPatternAnimationBindingHarness-" +
			 std::to_string(GetCurrentProcessId()) + "-" +
			 std::to_string(GetTickCount64()));
		SCOPED_TEST_DIRECTORY cleanup(testRoot);
		const std::filesystem::path dataRoot = testRoot / "Data";
		const std::filesystem::path destination = dataRoot /
			"Animation/Authored/Valtan/Valtan.patternbindings.json";
		std::error_code directoryError;
		std::filesystem::create_directories(
			destination.parent_path(), directoryError);
		const std::filesystem::path encounterDestination = dataRoot /
			"Encounters/Valtan/ValtanEncounter.json";
		std::filesystem::create_directories(
			encounterDestination.parent_path(), directoryError);
		if (!Require(!directoryError && WriteText(destination, sourceText) &&
			WriteText(encounterDestination, encounterText),
			"could not create isolated Valtan binding source"))
		{
			return false;
		}
		for (const std::filesystem::path& path : dependentSources)
		{
			if (!Require(WriteText(
				destination.parent_path() / path.filename(), ReadText(path)),
				"could not copy a dependent Valtan cue owner"))
			{
				return false;
			}
		}

		SCOPED_ENVIRONMENT_VARIABLE dataRootEnvironment(
			L"LOSTARK_PROJECT_DATA_ROOT");
		if (!Require(dataRootEnvironment.Set(dataRoot),
			"could not redirect the project Data root for the focused harness"))
		{
			return false;
		}

		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT loaded;
		std::string baselineSourceBytes;
		if (!Require(CValtanPatternAnimationBindingDocument::Load_ForAuthoring(
			"Valtan", "BOSS_VALTAN", availableClips, loaded,
			baselineSourceBytes, status) &&
			loaded == sourceDocument,
			"isolated real Valtan binding load failed: " + status))
		{
			return false;
		}

		const std::size_t editableIndex =
			FirstBindingWithUnreferencedAction(loaded, dependentSourceText);
		const std::size_t suppressionIndex =
			FirstBindingWithUnreferencedAction(
				loaded, dependentSourceText, editableIndex);
		if (!Require(editableIndex < loaded.Bindings.size() &&
			suppressionIndex < loaded.Bindings.size() &&
			!loaded.Bindings[suppressionIndex].bSuppressAnimation,
			"real Valtan fixture has no independent editable/suppressible rows"))
		{
			return false;
		}

		auto draft = loaded;
		BOSS_PATTERN_ANIMATION_CLIP& editedClip =
			draft.Bindings[editableIndex].Clips.back();
		const auto replacementClip = std::find_if(
			availableClips.begin(), availableClips.end(),
			[&editedClip](const std::string& clip)
			{
				return clip != editedClip.strClipName;
			});
		if (!Require(replacementClip != availableClips.end(),
			"real Valtan model has no alternate clip for draft coverage"))
		{
			return false;
		}
		editedClip.strClipName = *replacementClip;
		editedClip.strMappingBasis = "PROJECT_AUTHORED";
		editedClip.iSourceStartMs = 125u;
		editedClip.iPlayMs = 750u;
		editedClip.fPlayRate = 1.25f;
		editedClip.bLoop = false;
		draft.Bindings[suppressionIndex].Clips.clear();
		draft.Bindings[suppressionIndex].bSuppressAnimation = true;

		std::string committedSourceBytes;
		const bool_t initialSave =
			CValtanPatternAnimationBindingDocument::Save_Atomic(
			draft, "Valtan", "BOSS_VALTAN", availableClips,
			clipSourceDurations,
			baselineSourceBytes, committedSourceBytes, status);
		if (!Require(initialSave,
			"typed Valtan animation draft save failed: " + status))
		{
			return false;
		}
		baselineSourceBytes = committedSourceBytes;
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT reloaded;
		if (!Require(CValtanPatternAnimationBindingDocument::Load(
			"Valtan", "BOSS_VALTAN", availableClips, reloaded, status) &&
			reloaded == draft,
			"typed Valtan animation draft did not reload exactly: " + status))
		{
			return false;
		}
		const std::string admittedBytes = ReadText(destination);
		if (!Require(admittedBytes.find("\"playbackMode\": \"NONE\"") !=
				std::string::npos &&
			admittedBytes.find("\"sourceStartMs\": 125") !=
				std::string::npos &&
			admittedBytes.find("\"loop\": false") !=
				std::string::npos,
			"saved typed document omitted NONE or timing state"))
		{
			return false;
		}

		const auto rejectsSaveWithoutMutation =
			[&](const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& candidate,
				const std::vector<std::string>& candidateClips,
				const std::string& label)
			{
				const auto candidateBefore = candidate;
				std::string rejectedCommittedBytes;
				const bool_t rejected =
					!CValtanPatternAnimationBindingDocument::Save_Atomic(
						candidate, "Valtan", "BOSS_VALTAN",
						candidateClips, clipSourceDurations,
						baselineSourceBytes,
						rejectedCommittedBytes, status);
				BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT after;
				const bool_t loadPreserved =
					CValtanPatternAnimationBindingDocument::Load(
						"Valtan", "BOSS_VALTAN", availableClips,
						after, status) && after == draft;
				return Require(rejected && candidate == candidateBefore &&
					ReadText(destination) == admittedBytes && loadPreserved,
					label + " changed the file or in-memory document");
			};

		auto invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().strClipName = "bad/clip";
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "invalid clip ID"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().strClipName =
			"mesh_stale_missing";
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "stale model clip"))
			return false;
		invalid = draft;
		invalid.Bindings[1u].strActionId = invalid.Bindings[0u].strActionId;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "duplicate action ID"))
			return false;
		invalid = draft;
		const std::size_t otherClipIndex = FirstBindingWithClips(invalid,
			1u == invalid.Bindings[editableIndex].Clips.size() ? 2u : 1u);
		std::size_t occurrenceBinding = otherClipIndex;
		if (occurrenceBinding == editableIndex ||
			occurrenceBinding >= invalid.Bindings.size())
		{
			occurrenceBinding = editableIndex + 2u;
		}
		if (!Require(occurrenceBinding < invalid.Bindings.size() &&
			!invalid.Bindings[occurrenceBinding].Clips.empty(),
			"real Valtan fixture has no second clip occurrence"))
		{
			return false;
		}
		invalid.Bindings[occurrenceBinding].Clips.front().strClipOccurrenceId =
			invalid.Bindings[editableIndex].Clips.front().strClipOccurrenceId;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "duplicate occurrence ID"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().strClipOccurrenceId =
			"bad/occurrence";
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "invalid occurrence ID"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().iSourceStartMs = 60001u;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "invalid source start"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().iPlayMs = 60001u;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "invalid play duration"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].Clips.back().fPlayRate =
			std::numeric_limits<f32_t>::infinity();
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "non-finite play rate"))
			return false;
		invalid = draft;
		const std::size_t multiClipIndex = FirstBindingWithClips(invalid, 2u);
		if (!Require(multiClipIndex < invalid.Bindings.size(),
			"real Valtan fixture has no multi-clip loop contract"))
		{
			return false;
		}
		invalid.Bindings[multiClipIndex].Clips.front().bLoop = true;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "non-terminal loop clip"))
			return false;
		invalid = draft;
		invalid.iFormatVersion = 2u;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "lossy legacy version save"))
			return false;
		invalid = draft;
		invalid.Bindings[editableIndex].bSuppressAnimation = true;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "NONE with a non-empty clip chain"))
			return false;

		std::size_t requiredBindingIndex = draft.Bindings.size();
		for (std::size_t index = 0u; index < draft.Bindings.size(); ++index)
		{
			if (HasIdentityRow(
				encounterText, "actionId", draft.Bindings[index].strActionId))
			{
				requiredBindingIndex = index;
				break;
			}
		}
		if (!Require(requiredBindingIndex < draft.Bindings.size(),
			"real Valtan fixture has no Encounter-required action"))
		{
			return false;
		}
		invalid = draft;
		invalid.Bindings.erase(
			invalid.Bindings.begin() + requiredBindingIndex);
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "missing Encounter-required action"))
		{
			return false;
		}

		std::size_t linkedBindingIndex = draft.Bindings.size();
		std::size_t linkedClipIndex = 0u;
		for (std::size_t bindingIndex = 0u;
			bindingIndex < draft.Bindings.size(); ++bindingIndex)
		{
			const BOSS_PATTERN_ANIMATION_BINDING& binding =
				draft.Bindings[bindingIndex];
			if (binding.Clips.size() < 2u)
				continue;
			for (std::size_t clipIndex = 0u;
				clipIndex < binding.Clips.size(); ++clipIndex)
			{
				if (HasIdentityRow(dependentSourceText, "clipOccurrenceId",
					binding.Clips[clipIndex].strClipOccurrenceId))
				{
					linkedBindingIndex = bindingIndex;
					linkedClipIndex = clipIndex;
					break;
				}
			}
			if (linkedBindingIndex < draft.Bindings.size())
				break;
		}
		if (!Require(linkedBindingIndex < draft.Bindings.size(),
			"real Valtan fixture has no multi-clip dependent occurrence"))
		{
			return false;
		}
		invalid = draft;
		invalid.Bindings[linkedBindingIndex].Clips.erase(
			invalid.Bindings[linkedBindingIndex].Clips.begin() + linkedClipIndex);
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "cue-linked occurrence delete"))
		{
			return false;
		}
		invalid = draft;
		invalid.Bindings[linkedBindingIndex].Clips.clear();
		invalid.Bindings[linkedBindingIndex].bSuppressAnimation = true;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "cue-linked action NONE"))
		{
			return false;
		}
		invalid = draft;
		invalid.Bindings[linkedBindingIndex].Clips[linkedClipIndex].iSourceStartMs =
			60000u;
		if (!rejectsSaveWithoutMutation(
			invalid, availableClips, "cue-linked timing window"))
		{
			return false;
		}

		auto missingDurationCandidate = draft;
		BOSS_PATTERN_ANIMATION_CLIP& missingDurationClip =
			missingDurationCandidate.Bindings[editableIndex].Clips.back();
		missingDurationClip.fPlayRate += 0.125f;
		std::unordered_map<std::string, f32_t> missingClipDuration =
			clipSourceDurations;
		missingClipDuration.erase(missingDurationClip.strClipName);
		std::string missingDurationCommittedBytes;
		const auto missingDurationBefore = missingDurationCandidate;
		if (!Require(!CValtanPatternAnimationBindingDocument::Save_Atomic(
			missingDurationCandidate, "Valtan", "BOSS_VALTAN", availableClips,
			missingClipDuration, baselineSourceBytes,
			missingDurationCommittedBytes, status) &&
			missingDurationCandidate == missingDurationBefore &&
			ReadText(destination) == admittedBytes,
			"missing current model duration changed the file or draft"))
		{
			return false;
		}

		std::vector<std::string> staleAvailableClips = availableClips;
		staleAvailableClips.erase(std::remove(staleAvailableClips.begin(),
			staleAvailableClips.end(), editedClip.strClipName),
			staleAvailableClips.end());
		if (!rejectsSaveWithoutMutation(
			draft, staleAvailableClips, "stale available-clip revision"))
		{
			return false;
		}

		const auto rejectsJoinedOwnerMutationAtCommit =
			[&](const std::filesystem::path& joinedOwner,
				const std::string& label)
			{
				const std::string ownerBefore = ReadText(joinedOwner);
				auto raceCandidate = draft;
				raceCandidate.Bindings[editableIndex].Clips.back().fPlayRate +=
					0.25f;
				const auto raceCandidateBefore = raceCandidate;
				SCOPED_ENVIRONMENT_VARIABLE mutationHook(
					L"LOSTARK_TEST_VALTAN_BINDING_MUTATE_DEPENDENCY_BEFORE_COMMIT");
				if (!Require(mutationHook.Set(joinedOwner),
					"could not arm " + label + " CAS race hook"))
				{
					return false;
				}
				std::string raceCommittedBytes;
				const bool_t raceSave =
					CValtanPatternAnimationBindingDocument::Save_Atomic(
						raceCandidate, "Valtan", "BOSS_VALTAN",
						availableClips, clipSourceDurations,
						baselineSourceBytes, raceCommittedBytes, status);
				SetEnvironmentVariableW(
					L"LOSTARK_TEST_VALTAN_BINDING_MUTATE_DEPENDENCY_BEFORE_COMMIT",
					nullptr);
				const bool_t rejectedAndRetained = Require(!raceSave &&
					raceCandidate == raceCandidateBefore &&
					ReadText(destination) == admittedBytes &&
					ReadText(joinedOwner) == ownerBefore + " ",
					label +
					" commit-time CAS did not reject/retain an external mutation");
				const bool_t restored = Require(
					WriteText(joinedOwner, ownerBefore),
					"could not restore " + label +
					" after commit-time CAS coverage");
				return rejectedAndRetained && restored;
			};
		const std::filesystem::path soundDependency =
			destination.parent_path() / dependentSources[1u].filename();
		if (!rejectsJoinedOwnerMutationAtCommit(
				soundDependency, "dependent Sound owner") ||
			!rejectsJoinedOwnerMutationAtCommit(
				encounterDestination, "Encounter owner"))
		{
			return false;
		}

		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT loadTarget = draft;
		if (!Require(WriteText(destination, "{\"formatVersion\":99}"),
			"could not write invalid load fixture") ||
			!Require(!CValtanPatternAnimationBindingDocument::Load(
				"Valtan", "BOSS_VALTAN", availableClips, loadTarget, status) &&
				loadTarget == draft,
				"failed reload replaced the admitted in-memory draft") ||
			!Require(WriteText(destination, admittedBytes),
				"could not restore admitted bytes after failed-load coverage"))
		{
			return false;
		}

		const std::string externalMutation = admittedBytes + " ";
		std::string staleCommittedBytes;
		if (!Require(WriteText(destination, externalMutation),
			"could not stage an external source mutation") ||
			!Require(!CValtanPatternAnimationBindingDocument::Save_Atomic(
				draft, "Valtan", "BOSS_VALTAN", availableClips,
				clipSourceDurations,
				baselineSourceBytes, staleCommittedBytes, status) &&
				ReadText(destination) == externalMutation,
				"stale CAS Save overwrote an external source mutation") ||
			!Require(WriteText(destination, admittedBytes),
				"could not restore the admitted CAS baseline"))
		{
			return false;
		}

		auto blocked = draft;
		blocked.Bindings[editableIndex].Clips.back().fPlayRate = 1.5f;
		const HANDLE lock = CreateFileW(destination.c_str(), GENERIC_READ, 0u,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (!Require(INVALID_HANDLE_VALUE != lock,
			"could not lock the binding destination for replace-failure coverage"))
		{
			return false;
		}
		std::string blockedCommittedBytes;
		const bool_t blockedSave =
			CValtanPatternAnimationBindingDocument::Save_Atomic(
				blocked, "Valtan", "BOSS_VALTAN", availableClips,
				clipSourceDurations,
				baselineSourceBytes, blockedCommittedBytes, status);
		CloseHandle(lock);
		if (!Require(!blockedSave && ReadText(destination) == admittedBytes,
			"atomic replace failure changed the admitted binding bytes") ||
			!Require(!HasStagingArtifact(destination),
				"atomic replace failure left a sibling staging file"))
		{
			return false;
		}
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT afterFailure;
		if (!Require(CValtanPatternAnimationBindingDocument::Load(
			"Valtan", "BOSS_VALTAN", availableClips, afterFailure, status) &&
			afterFailure == draft,
			"atomic replace failure changed the reloadable binding document"))
		{
			return false;
		}

		if (!Require(CValtanPatternAnimationBindingDocument::Save_Atomic(
			blocked, "Valtan", "BOSS_VALTAN", availableClips,
			clipSourceDurations,
			baselineSourceBytes, blockedCommittedBytes, status),
			"save did not recover after destination lock release: " + status) ||
			!Require(CValtanPatternAnimationBindingDocument::Load(
				"Valtan", "BOSS_VALTAN", availableClips,
				afterFailure, status) && afterFailure == blocked,
				"post-failure binding save/reload did not commit exactly"))
		{
			return false;
		}
		baselineSourceBytes = blockedCommittedBytes;

		return true;
	}
}

int Run_ValtanPatternAnimationBindingDocumentContractTests()
{
	if (!VerifyContract())
		return 1;
	std::cout <<
		"Valtan pattern animation binding document contracts: PASS\n";
	return 0;
}
